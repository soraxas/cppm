# CPPM: C++ Program Progress Monitor

`cppm` is a  **c++ monitor** for monitoring progress of loops, inspired by [tqdm](https://github.com/tqdm/tqdm). The [official](https://github.com/tqdm/tqdm.cpp) c++ port lacks a lot of features and is not being maintained. `cppm` aims to provide an easy-to-use header-only file to be dropped in any c++ project for monitoring program progress.

This is useful if you need to run some process and would like to monitor the progress of some loop. For most cases, it will display:

- speed of iterations per second
- current percentage
- progress bar
- elapsed time
- estimated remaining time

`cppm` uses syntax upto C++11.


# Usage

`cppm` can be used as a monitor without explicit size, of which it will display the speed of the iterations per second.


# Acknowledgement

The main `cppm`'s header only structure is based upon [cpptqdm](https://github.com/aminnj/cpptqdm), including the theme of the fancy progress bar. In addition, the wrapping of iterable container is inspired by [tqdm-cpp](https://gitlab.com/miguelraggi/tqdm-cpp).