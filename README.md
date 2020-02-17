# Mozart++ Template Library made with ❤️

[![Action Status](https://github.com/covariant-institute/mozart/workflows/build/badge.svg)](https://github.com/covariant-institute/mozart/actions)
[![GitHub top language](https://img.shields.io/github/languages/top/covariant-institute/mozart.svg)](https://github.com/covariant-institute/mozart)
[![license](https://img.shields.io/github/license/covariant-institute/mozart.svg?colorB=000000)](https://github.com/covariant-institute/mozart)


[English](./README.md)

[中文](./README-zh.md)

## What is Mozart++
Mozart++ is a template library written in Modern C++ designed to make up for some missing tools in STL.

Mozart++ was born in our daily development. We usually build all kinds of "wheels" in a project, but the process is just so trivial, as a result, we decided to put these "wheels" out as a separate template library.

Currently our project is written in C++14 because our main project which Mozart++ originally aimed to support was written in C++14.

## Code Convension
Mozart++ has two `namespace`s, `mpp` and `mpp_impl`.
Usually developers only need to use `mpp`. We promise that all implementations will be hidden in `mpp_impl`.

## What does Mozart++ have now?
Currently we have these tools listed below (ordered alphabetically):

* A
  * `mpp::any`: A super effective `std::any`.
  * `mpp::allocator_type`: Memory allocator.
* C
  * `mpp::codecvt`: Wide string and string converter.
* E
  * `mpp::event_emitter`: A NodeJS-like, non-invasive EventEmitter (two implementations):
    * `mpp::event_emitter_fast` (for release)
    * `mpp::event_emitter_attentive` (for debug, with more debug information)
* F
  * `mpp::function`: An alias for `std::function`.
  * `mpp::function_parser`: A function type trait for extracting all information from a callable type.
  * `mpp::function_type`: An alias for parsed function type (aka `mpp::function`).
* I
  * `mpp::iterator_range`: A range adapter for a pair of iterators, wrapping two iterators into range-compatible interface.
* O
  * `mpp::optional`: Something like `std::optional`
* R
  * `mpp::runtime_error`: Customized runtime exception.
* S
  * `mpp::stream`: Stream API support.
  * `mpp::string_ref`: String wrapper with plenty of useful string manipulation methods.
* T
  * `mpp::timer`: Time operation wrapper.
  * `mpp::typelist`: Compile-time list holding types as its elements.
  * `mpp::throw_ex()`: Exception thrower which triggers global event emitter.

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

