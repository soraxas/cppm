#pragma once

#include <unistd.h>
#include <chrono>
#include <ctime>
#include <numeric>
#include <ios>
#include <string>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <signal.h>
#include <vector>
#include <math.h>
#include <algorithm>
// for getting terminal size
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#include <unistd.h>    // for STDOUT_FILENO

namespace pm
{
    unsigned int terminal_width = 80;

    void flush_stdout(int sig)
    { // can be called asynchronously
        printf("\n");
        fflush(stdout);
        signal(sig, SIG_DFL);
        raise(sig);
    }

    void update_terminal_width(int sig = -1)
    { // can be called asynchronously
        struct winsize size;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
        terminal_width = size.ws_col;
    }

    const char *COLOR_RESET = "\033[0m\033[32m\033[0m\015";
    const char *COLOR_RED = "\033[1m\033[31m";  // with bold
    const char *COLOR_BLUE = "\033[1m\033[34m"; // with bold
    const char *COLOR_LIME = "\033[32m";

    void hsv_to_rgb(float h, float s, float v, int &r, int &g, int &b)
    {
        if (s < 1e-6)
        {
            v *= 255., r = v, g = v, b = v;
        }
        int i = (int)(h * 6.0);
        float f = (h * 6.) - i;
        int p = (int)(255.0 * (v * (1. - s)));
        int q = (int)(255.0 * (v * (1. - s * f)));
        int t = (int)(255.0 * (v * (1. - s * (1. - f))));
        v *= 255;
        i %= 6;
        int vi = (int)v;
        if (i == 0)
            r = vi, g = t, b = p;
        else if (i == 1)
            r = q, g = vi, b = p;
        else if (i == 2)
            r = p, g = vi, b = t;
        else if (i == 3)
            r = p, g = q, b = vi;
        else if (i == 4)
            r = t, g = p, b = vi;
        else if (i == 5)
            r = vi, g = p, b = q;
    }

    class yapm
    {
    protected:
        // time, iteration counters and deques for rate calculations
        std::chrono::time_point<std::chrono::system_clock> t_first = std::chrono::system_clock::now();
        std::chrono::time_point<std::chrono::system_clock> t_old = std::chrono::system_clock::now();
        int n_old = 0;
        std::vector<double> deq_t;
        std::vector<int> deq_n;
        int nupdates = 0;
        int total_ = 0;
        int cur_ = 0;
        int period = 1;
        unsigned int smoothing = 50;
        bool use_ema = true;
        bool has_total_it = false;
        bool print_bar = false;
        float alpha_ema = 0.1;
        const float min_update_time = 0.15;
        std::stringstream suffix_;

        // short terminal_width = 80;
        std::vector<const char *> bars = {" ", "▏", "▎", "▍", "▌", "▋", "▊", "▉", "█"};

        bool in_screen = (system("test $STY") == 0);
        bool in_tmux = (system("test $TMUX") == 0);
        bool is_tty = isatty(1);
        bool use_colors = true;
        bool color_transition = true;
        bool enable_speed_stats = true;
        int bar_width = 40;

        /////////////////////////////////////
        double __tmp_pct = 0.;
        double __tmp_avgrate = 0.;
        double __tmp_remain_t = 0.;
        double __tmp_dt_tot = 0.;
        /////////////////////////////////////

        std::string left_pad = "▕";
        std::string right_pad = "▏";
        std::string label = "";

    protected:
        /////////////////////////////////////
        // formatting
        /////////////////////////////////////
        inline void _print_color(const char *color)
        {
            if (use_colors)
                printf(color);
        }
        inline void _print_bar()
        {
            double pct = __tmp_pct > 1 ? 1 : __tmp_pct;
            double fills = (pct * bar_width);
            int ifills = fills;

            if (use_colors)
            {
                if (color_transition)
                {
                    // red (hue=0) to green (hue=1/3)
                    int r = 255, g = 255, b = 255;
                    hsv_to_rgb(0.0 + pct / 3, 0.65, 1.0, r, g, b);
                    printf("\033[38;2;%d;%d;%dm", r, g, b);
                }
                else
                    printf(COLOR_LIME);
            }
            printf("%s", left_pad.c_str());
            for (int i = 0; i < ifills; i++)
                std::cout << bars[8];
            if (!in_screen and (pct < 1.0))
                printf("%s", bars[(int)(8.0 * (fills - ifills))]);
            for (int i = 0; i < bar_width - ifills - 1; i++)
                std::cout << bars[0];
            printf("%s", right_pad.c_str());
        }
        inline void _format_speed(std::ostringstream &oss, const double &avgrate)
        {
            if (!enable_speed_stats)
                return;

            const char *unit = "Hz";
            double div = 1.;
            if (avgrate > 1e6)
                unit = "MHz", div = 1.0e6;
            else if (avgrate > 1e3)
                unit = "kHz", div = 1.0e3;

            oss << std::fixed << std::setprecision(1) << avgrate / div << unit;
        }
        virtual inline void _sstream_progress_text(std::ostringstream &oss)
        {
            oss << cur_ << "/" << total_;
        }
        inline void _format_simplify_time_(std::ostringstream &oss, int seconds)
        {
            int hours, minutes, days;
            hours = minutes = days = -1;
            if (seconds >= 60)
            {
                minutes = seconds / 60;
                seconds = seconds % 60;
                if (minutes >= 60)
                {
                    hours = minutes / 60;
                    minutes = minutes % 60;
                    if (hours >= 24)
                    {
                        days = hours / 24;
                        hours = hours % 24;
                    }
                }
            }
            if (days >= 0)
                oss << std::setw(2) << std::setfill('0') << days << "d";
            if (hours >= 0)
                oss << std::setw(2) << std::setfill('0') << hours << "h";
            if (minutes >= 0)
                oss << std::setw(2) << std::setfill('0') << minutes << "m";
            oss << std::setw(2) << std::setfill('0') << seconds << "s";
        }
        /////////////////////////////////////
        // internal output and house keeping
        /////////////////////////////////////
        inline void _print_progress()
        {
            printf("\015"); // clear line
            // label and pct
            std::ostringstream pbar_pct;
            std::ostringstream pbar_suf;

            pbar_pct << label << std::fixed << std::setprecision(1) << std::setw(5) << std::setfill(' ') << __tmp_pct * 100 << "%";
            std::string pbar_pct_str = pbar_pct.str();

            auto suffix = suffix_.str();
            if (suffix.length() > 0)
                suffix.insert(0, " ");
            if (has_total_it || print_bar)
            {
                // percentage
                _sstream_progress_text(pbar_suf);
                pbar_suf << " [", _format_speed(pbar_suf, __tmp_avgrate);
                pbar_suf << "|", _format_simplify_time_(pbar_suf, __tmp_dt_tot);
                pbar_suf << "<", _format_simplify_time_(pbar_suf, __tmp_remain_t);
                pbar_suf << "]" << suffix;

                std::string pbar_suf_str = pbar_suf.str();

                compute_pbar_size(pbar_pct_str.length() + pbar_suf_str.length() + 2);
                _print_color(COLOR_RED);
                printf("%s", pbar_pct_str.c_str());
                _print_bar();
                _print_color(COLOR_BLUE);
                printf("%s", pbar_suf_str.c_str());
            }
            else
            {
                _print_color(COLOR_BLUE);
                pbar_suf << "[", _format_speed(pbar_suf, __tmp_avgrate);
                pbar_suf << "|", _format_simplify_time_(pbar_suf, __tmp_dt_tot);
                pbar_suf << "]" << suffix;

                std::string pbar_suf_str = pbar_suf.str();
                printf("%4dit %s", cur_,
                       pbar_suf_str.c_str());
            }

            // finish printing
            _print_color(COLOR_RESET);
            // if(!has_total_it || (total_ - cur_) > period) fflush(stdout);
            fflush(stdout);
        }
        inline void _internal_update_end()
        {
            suffix_.str("");
        }
        virtual inline void _compute_total()
        {
            if (has_total_it)
            {
                __tmp_remain_t = (total_ - cur_) / __tmp_avgrate;
                __tmp_pct = (double)cur_ / (total_);
                // last small chunk of percentage.
                if ((total_ - cur_) <= period)
                {
                    __tmp_pct = 1.;
                    __tmp_avgrate = total_ / __tmp_dt_tot;
                    // cur_ = total_;
                    __tmp_remain_t = 0;
                }
            }
        }
        inline bool _internal_update()
        {
            bool closed_to_finish = has_total_it && total_ - cur_ < 2; // will finish loop soon
            if (is_tty && (cur_ % period == 0 || closed_to_finish))
            {
                auto now = std::chrono::system_clock::now();
                float dt = ((std::chrono::duration<double>)(now - t_old)).count();
                nupdates++;

                // do nothing if last refresh time is too recent.
                if (!closed_to_finish && dt < min_update_time)
                    return false;

                __tmp_dt_tot = ((std::chrono::duration<double>)(now - t_first)).count();
                int dn = cur_ - n_old;
                n_old = cur_;
                t_old = now;
                if (deq_n.size() >= smoothing)
                    deq_n.erase(deq_n.begin());
                if (deq_t.size() >= smoothing)
                    deq_t.erase(deq_t.begin());
                deq_t.push_back(dt);
                deq_n.push_back(dn);

                __tmp_avgrate = 0.;
                if (use_ema)
                {
                    __tmp_avgrate = deq_n[0] / deq_t[0];
                    for (unsigned int i = 1; i < deq_t.size(); i++)
                    {
                        double r = 1.0 * deq_n[i] / deq_t[i];
                        __tmp_avgrate = alpha_ema * r + (1.0 - alpha_ema) * __tmp_avgrate;
                    }
                }
                else
                {
                    double dtsum = std::accumulate(deq_t.begin(), deq_t.end(), 0.);
                    int dnsum = std::accumulate(deq_n.begin(), deq_n.end(), 0.);
                    __tmp_avgrate = dnsum / dtsum;
                }

                // learn an appropriate period length to avoid spamming stdout
                // and slowing down the loop, shoot for ~25Hz and smooth over 3 seconds
                if (nupdates > 10)
                {
                    period = (int)(std::min(std::max((1.0 / 25) * cur_ / __tmp_dt_tot, 1.0), 5e5));
                    smoothing = 25 * 3;
                }
                _compute_total();
                return true;
            }
            return false;
        }
        inline void compute_pbar_size(const int &other_length)
        {
            bar_width = terminal_width - other_length;
        }

    public:
        yapm()
        {
            if (in_screen)
                set_theme_basic(), color_transition = false;
            else if (in_tmux)
                color_transition = false;
            update_terminal_width();
            signal(SIGINT, flush_stdout);            // flush stdout when program is exiting
            signal(SIGWINCH, update_terminal_width); // flush stdout when program is exiting
        }
        yapm(const int total) : yapm()
        {
            total_ = total;
            has_total_it = true;

            // compute_pbar_size();
        }
        template <class T>
        yapm &operator<<(const T &t)
        {
            suffix_ << t;
            return *this;
        }
        void reset()
        {
            t_first = std::chrono::system_clock::now();
            t_old = std::chrono::system_clock::now();
            n_old = 0;
            deq_t.clear();
            deq_n.clear();
            period = 1;
            nupdates = 0;
            total_ = 0;
            has_total_it = false;
            label = "";
            update_terminal_width();
        }

        ///////////////////////////////////////////////////////////////
        void update()
        {
            /* Called to increment internal counter */
            ++cur_;
            if (_internal_update())
                _print_progress();
            _internal_update_end();
        }
        void progress(int curr, int tot)
        {
            /* Called directly set current counter and total */
            cur_ = curr, total_ = tot;
            has_total_it = true;
            if (_internal_update())
                _print_progress();
            _internal_update_end();
        }

        ///////////////////////////////////////////////////////////////

        void set_theme_line() { bars = {"─", "─", "─", "╾", "╾", "╾", "╾", "━", "═"}; }
        void set_theme_circle() { bars = {" ", "◓", "◑", "◒", "◐", "◓", "◑", "◒", "#"}; }
        void set_theme_braille() { bars = {" ", "⡀", "⡄", "⡆", "⡇", "⡏", "⡟", "⡿", "⣿"}; }
        void set_theme_braille_spin() { bars = {" ", "⠙", "⠹", "⠸", "⠼", "⠴", "⠦", "⠇", "⠿"}; }
        void set_theme_vertical() { bars = {"▁", "▂", "▃", "▄", "▅", "▆", "▇", "█", "█"}; }
        void set_theme_basic()
        {
            bars = {" ", " ", " ", " ", " ", " ", " ", " ", "#"};
            left_pad = "|";
            right_pad = "|";
        }
        void set_label(std::string label_) { label = label_.append(" "); }
        void set_total(const int total)
        {
            total_ = total;
            has_total_it = true;
        }
        void disable_colors()
        {
            color_transition = use_colors = false;
        }
        void finish()
        {
            if (has_total_it)
                cur_ = total_;
            _print_progress();
            // progress(total_,total_);
            printf("\n");
            fflush(stdout);
        }
    };

    class yapm_timer : public yapm
    {
    protected:
        double total_seconds_ = 0.;

        inline void _compute_total()
        {
            auto now = std::chrono::system_clock::now();

            double passed_time = ((std::chrono::duration<double>)(now - t_first)).count();
            __tmp_remain_t = total_seconds_ - passed_time;
            __tmp_pct = passed_time / total_seconds_;
        }

        virtual inline void _sstream_progress_text(std::ostringstream &oss)
        {
            oss << cur_ << "it";
        }

    public:
        yapm_timer() = delete;
        yapm_timer(const int seconds) : yapm_timer((double)seconds) {}
        yapm_timer(const double seconds) : yapm(), total_seconds_(seconds)
        {
            print_bar = true;
        }
        void progress(int curr, int tot) = delete;
    };
}; // end namespace pm
// #endif
