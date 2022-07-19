# CPPM: C++ Program Progress Monitor

`cppm` is a  **c++ monitor** for monitoring progress of loops, inspired by [tqdm](https://github.com/tqdm/tqdm). The [official](https://github.com/tqdm/tqdm.cpp) c++ port lacks a lot of features and is not being maintained. `cppm` aims to provide an easy-to-use header-only file to be dropped in any c++ project for monitoring program progress.

This is useful if you need to run some process and would like to monitor the progress of some loop. For most cases, it will display:

- speed of iterations per second
- current percentage
- progress bar
- elapsed time
- estimated remaining time

`cppm` uses syntax upto C++11.

# Quick install

You can use the following in your CMakeLists.txt
```cmake
include(FetchContent)
# ==================================================
set (EXT_CPPM "ext_cppm")
	FetchContent_Declare (
    ${EXT_CPPM}

    PREFIX         ${EXT_CPPM}
		GIT_REPOSITORY https://github.com/soraxas/cppm
		GIT_TAG        2e946ee1261b61b955d55b5d405410f34fb8ce4a
		GIT_SHALLOW    ON

		BUILD_ALWAYS   OFF
    INSTALL_DIR    ${CMAKE_CURRENT_BINARY_DIR}/ext/${EXT_CPPM}

		CMAKE_CACHE_ARGS
			-DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

	)
FetchContent_MakeAvailable(${EXT_CPPM})
# ==================================================

# ......
# ......

target_link_libraries(${PROJECT_NAME} cppm)
target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:cppm,INTERFACE_INCLUDE_DIRECTORIES>)
```


# Usage

`cppm` can be used as a monitor without explicit size, of which it will display the speed of the iterations per second.

```c++
auto pbar = cppm::pm(n);
for (...) {
  ...
  pbar.update();
}
pbar.finish();
```

# Acknowledgement

The main `cppm`'s header only structure is based upon [cpptqdm](https://github.com/aminnj/cpptqdm), including the theme of the fancy progress bar. In addition, the wrapping of iterable container is inspired by [tqdm-cpp](https://gitlab.com/miguelraggi/tqdm-cpp).
