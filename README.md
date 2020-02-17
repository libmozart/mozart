<div align="center">

   <img width="160" src="./logo/libmozart.jpg" alt="logo"></br>
----
[![Action Status](https://github.com/covariant-institute/mozart/workflows/build/badge.svg)](https://github.com/covariant-institute/mozart/actions)
[![GitHub top language](https://img.shields.io/github/languages/top/covariant-institute/mozart.svg)](https://github.com/covariant-institute/mozart)
[![license](https://img.shields.io/github/license/covariant-institute/mozart.svg?colorB=000000)](https://github.com/covariant-institute/mozart)

    <br/>
    <h1>Mozart++ Template Library made with ❤️</h1>
    <br/>

[English](./README.md)
[中文](./README-zh.md)

</div>

## What is Mozart++
Mozart++ is a cross-platform template library written in Modern C++ designed to make up for essential but missing components in STL.

Mozart++ was born in our daily development. We usually build all kinds of "wheels" in a project, but the process is just so trivial, as a result, we decided to put these "wheels" out as a separate template library.

Currently our project is written in C++14 because our main project which Mozart++ originally aimed to support was written in C++14.

## Supported Compilers
Compiler|Version|Tested Platform|Status
:---:|:---:|:---:|:---:|
gcc|8.1.0-x86_64|Ubuntu 18.04|:white_check_mark:
gcc|7.4.0-x86_64|WSL Ubuntu 18.04|:white_check_mark:
Apple Clang|11.0.0|macOS Catalina|:white_check_mark:
mingw-gcc|8.1.0 (x86_64-posix-seh-rev0)|Windows 10 Pro 1903|:white_check_mark:
msvc|19.24.28316|Windows 10 Pro 1903|:white_check_mark:

## Code Convension
Mozart++ has two `namespace`s, `mpp` and `mpp_impl`.
Usually developers only need to use `mpp`. We promise that all implementations will be hidden in `mpp_impl`.

## What does Mozart++ have now?
Currently we have these tools listed below (ordered alphabetically):

* A
  * `mpp::any`: A super effective `std::any`.
  * `mpp::allocator_type`: Memory allocator.
* C
  * `mpp::codecvt`: Wide string and string converter supports ascii, utf-8 and GBK.
* E
  * `mpp::event_emitter`: A NodeJS-like, non-invasive EventEmitter (two implementations):
    * `mpp::event_emitter_fast` (for release)
    * `mpp::event_emitter_attentive` (for debug, with more debug information)
* F
  * `mpp::function`: An alias for `std::function`.
  * `mpp::function_parser`: A function type trait for extracting all information from a callable type.
  * `mpp::function_type`: An alias for parsed function type (aka `mpp::function`).
  * `mpp::fdistream`: Wrap a C file descriptor and Windows File Handle into C++ std::istream.
  * `mpp::fdostream`: Wrap a C file descriptor and Windows File Handle into C++ std::ostream.
* I
  * `mpp::iterator_range`: A range adapter for a pair of iterators, wrapping two iterators into range-compatible interface.
* O
  * `mpp::optional`: Something like `std::optional`
* P
  * `mpp::process`: Cross-platform process interacting library.
* R
  * `mpp::runtime_error`: Customized runtime exception.
* S
  * `mpp::stream`: Stream API support.
  * `mpp::string_ref`: String wrapper with plenty of useful string manipulation methods.
* T
  * `mpp::timer`: Time operation wrapper.
  * `mpp::typelist`: Compile-time list holding types as its elements.
  * `mpp::throw_ex()`: Exception thrower which triggers global event emitter.

## How to use Mozart++?
#### For cmake-based project
First of all, open your terminal (or powershell, or cmd)
```bash
$ cd /path/to/your/project
$ git submodule init
$ git submodule add https://github.com/libmozart/mozart.git third-party/mozart
```

Then, add the following lines to your root `CMakeLists.txt`:
```cmake
add_subdirectory(third-party/mozart)
include_directories(third-party/mozart)
```

NOTICE: Do not forget to link `mozart++` to your target.
Please make sure your `CMakeLists.txt` contains the following line:
```cmake
target_link_libraries(<your-target> mozart++)
```

#### For other build tools
We are currently working on it. If you have any idea,
feel free to create issues or pull requests.

## Demo
* Event Emitter
    ```cpp
    mpp::event_emitter e;

    // register events
    e.on("player-login", [](mpp::string_ref username) {
        checkUsername(username);
    });

    e.on("player-move", [](point from, point to) {
        checkMovement(from, to);
    });

    // some other places
    e.emit("player-login", username);
    e.emit("player-move", {1, 2}, {3, 4});
    ```

* Stream
    ```cpp
    stream<int>::iterate(1, [](int x) { return x * 2; })
            .map([](int x) { return x - 1; })
            .filter([](int x) { return x > 1000; })
            .drop_while([](int x) { return x <= 100000; })
            .drop(5)
            .take_while([](int x) { return x <= 5000000; })
            .for_each([](int x) { printf("%d\n", x); });
    ```

* Optional
    ```cpp
    int sum(const mpp::optional<std::vector<int>> &nums) {
        return nums.apply_or<int>(0, [](auto &&v) -> int {
            int s = 0;
            for (auto &&e : v) {
                s += e;
            }
            return s;
        });
    }
    ```

* Process
    ```cpp
    using mpp::process;

    process p = process::exec("/bin/bash");
    p.in() << "ls /" << std::endl;
    p.in() << "exit" << std::endl;
    p.wait_for();

    std::string s;
    while (std::getline(p.out(), s)) {
        printf("%s\n", s.c_str());
    }
    ```

## Contributions
Contributions and ideas are welcomed through issues and PRs.

## Developers
[![](https://github.com/mikecovlee.png?size=50)](https://github.com/mikecovlee)
[![](https://github.com/imkiva.png?size=50)](https://github.com/imkiva)

## License
```
MIT License

Copyright (c) 2020 Covariant Institute
```

