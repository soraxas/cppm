#include "yapm.hpp"



std::vector<int> get_vector(int size)
{
    std::vector<int> A(size);
    std::iota(A.begin(), A.end(), 1000);
    //     std::generate(A.begin(), A.end(), []() {return rand(); } );

    return A;
}

int main() {

    int N = 20000;

    std::cout << "Progress monitor without total:" << std::endl;
    yapm::pm bar1;
    // if you want to output the progress monitor to some log file:
    // bar1.setOutFilename("/tmp/yadm.log");
    for (int i = 0; i < N; i++)
    {
        usleep(.1);
        bar1.update();
        // include extra info 
        bar1 << "loss=" << i;
    }
    bar1.finish();
    
    std::cout << "Overhead of loop only:" << std::endl;
    yapm::pm bar2;
    for (int i = 0; i < N; i++)
    {
        bar2.update();
        // include extra info 
        bar2 << "loss=" << i;
    }
    bar2.finish();
    
    std::cout << "Progress monitor with given total:" << std::endl;
    yapm::pm bar3(N);
    for (int i = 0; i < N; i++)
    {
        usleep(.1);
        bar3.update();
        // include extra info 
        bar3 << "loss=" << i;
    }
    bar3.finish();
    
    std::cout << "YAPM as a timer with an estimated time-to-finish:" << std::endl;
    yapm::pm_timer bar4(N * 500 * 1e-6);  // estimated time to finish
    for (int i = 0; i < N; i++)
    {
        usleep(500);
        bar4.update();
    }
    bar4.finish();
    
    std::cout << "YAPM with wrapping some iterator (lvalue):" << std::endl;
    auto list = get_vector(5000000);
    auto bar5 = yapm::iter(list);
    for (auto &&t: bar5)
    {
        // bar5.update();  // SHOULD NOT USE UPDATE
        bar5 << t;
    }
    bar5.finish();
    
    std::cout << "YAPM with wrapping some iterator (rvalue):" << std::endl;
    auto bar6 = yapm::iter(get_vector(5000000));
    for (auto &&t: bar6)
    {
        bar6 << t;
    }
    bar6.finish();
    
    std::cout << "YAPM with easy range (only end):" << std::endl;
    for (auto &&t : yapm::range(100))
    {}
    
    std::cout << "YAPM with easy range (start & end):" << std::endl;
    for (auto &&t : yapm::range(10, 100))
    {}
    
    std::cout << "YAPM with easy range (start & end & step):" << std::endl;
    for (auto &&t : yapm::range(10, 100, 5))
    {}
    
    std::cout << "YAPM with easy range (negative step):" << std::endl;
    auto bar7 = yapm::range(1.4e2, -100.2, -5.5);
    for (auto &&t : bar7)
    {
        // bar7.update();  // SHOULD NOT USE UPDATE
        bar7 << "val=" << t;
    }
    bar7.finish();

    std::cout << "===== All themes =====" << std::endl;
    std::cout << "Basic:" << std::endl;
    yapm::pm bar;
    bar.reset();
    bar.set_theme_basic();
    bar.set_total(N);
    for(int i = 0; i < N; i++) {
        // bar.progress(i, N);
        bar.update();
        usleep(100);
        bar << "loss=" << i;
    }
    bar.finish();

    std::cout << "Braille:" << std::endl;
    bar.reset();
    bar.set_theme_braille();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
        usleep(300);
    }
    bar.finish();

    std::cout << "Line:" << std::endl;
    bar.reset();
    bar.set_theme_line();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
        usleep(300);
    }
    bar.finish();

    std::cout << "Circles:" << std::endl;
    bar.reset();
    bar.set_theme_circle();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
        usleep(300);
    }
    bar.finish();

    bar.reset();
    std::cout << "Vertical bars:" << std::endl;
    bar.reset();
    bar.set_theme_vertical();
    for(int i = 0; i < N; i++) {
        bar.progress(i, N);
        usleep(300);
    }
    bar.finish();

    return 0;
}
