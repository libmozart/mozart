# Mozart++ Template Library made with ❤️

[![Action Status](https://github.com/covariant-institute/mozart/workflows/build/badge.svg)](https://github.com/covariant-institute/mozart/actions)
[![GitHub top language](https://img.shields.io/github/languages/top/covariant-institute/mozart.svg)](https://github.com/covariant-institute/mozart)
[![license](https://img.shields.io/github/license/covariant-institute/mozart.svg?colorB=000000)](https://github.com/covariant-institute/mozart)


[English](./README.md)

[中文](./README-zh.md)

## Mozart++ 是什么？
Mozart++ 是一个用现代 C++ 写成的模版库，用来补足 STL 的短板。

Mozart++ 诞生于我们的日常开发中。通常我们需要在每个项目中造各种各样的“轮子”来满足需求，但造轮子的过程十分痛苦和繁琐，所以，我们决定把这些“轮子”单独抽出来做成一个公共的模版库。

目前为止，我们的项目采用 C++14 语言标准，因为 Mozart++ 最初用来支持的项目就是用 C++14 写成的。

## 支持的编译器
编译器|版本|测试平台|状态
:---:|:---:|:---:|:---:|
gcc|8.1.0-x86_64|Ubuntu 18.04|:white_check_mark:
gcc|7.4.0-x86_64|WSL Ubuntu 18.04|:white_check_mark:
Apple Clang|11.0.0|macOS Catalina|:white_check_mark:
mingw-gcc|8.1.0 (x86_64-posix-seh-rev0)|Windows 10 Pro 1903|:white_check_mark:
msvc|19.24.28316|Windows 10 Pro 1903|:white_check_mark:

## 代码约定
Mozart++ 有两个 `namespace`, `mpp` 和 `mpp_impl`.
通常情况下开发者只要使用 `mpp` 中的组建，我们保证所有的实现细节都会隐藏在 `mpp_impl` 中，不用担心命名空间污染.

## Mozart++ 现在有什么？
现在我们有这些东西（按字母表排序）:

* A
  * `mpp::any`: 效率极高的 `std::any`
  * `mpp::allocator_type`: 通用内存分配器具
* C
  * `mpp::codecvt`: 宽字符串和普通字符串的互相转换器，支持ASCII、UTF-8以及GBK编码
* E
  * `mpp::event_emitter`: 像 NodeJS 那样的并且无侵入性的 EventEmitter (两个实现):
    * `mpp::event_emitter_fast` (release 构建使用)
    * `mpp::event_emitter_attentive` (debug 构建使用, 带有更详细的调试信息)
* F
  * `mpp::function`: `std::function` 的别名
  * `mpp::function_parser`: 函数类型萃取器，支持所有 `callble` 类型
  * `mpp::function_type`: 对使用函数类型萃取器得到的函数类型的别名 (即 `mpp::function`)
  * `mpp::fdistream`: 将 C 的文件描述符和 Windows 的文件句柄包装成 std::istream
  * `mpp::fdostream`: 将 C 的文件描述符和 Windows 的文件句柄包装成 std::ostream
* I
  * `mpp::iterator_range`: 将 `iterator` 包装成支持 `ranged-for` 的对象
* O
  * `mpp::optional`: 像 C++17 中的 `std::optional`
* P
  * `mpp::process`: 跨平台进程交互库拓展库
* R
  * `mpp::runtime_error`: 运行时异常
* S
  * `mpp::stream`: 流式 API 支持
  * `mpp::string_ref`: 字符串的包装类，提供大量字符串操作方法
* T
  * `mpp::timer`: 包装与时间操作相关的 API
  * `mpp::typelist`: 编译期的类型列表，其元素均为类型变量
  * `mpp::throw_ex()`: 异常抛出，但会触发全局的一个 event emitter.

## 如何使用 Mozart++?
#### 基于 CMake 的项目
首先，打开你的终端（或者 cmd，或者 powershell）
```bash
$ cd /path/to/your/project
$ git submodule init
$ git submodule add https://github.com/covmozart/mozart.git third-party/mozart
```

然后，添加下面这行到项目根目录下的 `CMakeLists.txt` 中:
```cmake
add_subdirectory(third-party/mozart)
include_directories(third-party/mozart)
```

重要提示: 不要忘记将 `mozart++` 连接到你的目标上.
请保证 `CMakeLists.txt` 中含有下面这行代码：
```cmake
target_link_libraries(<your-target> mozart++)
```

#### 基于其他构建工具的项目
目前我们正在让 Mozart++ 支持其他构建工具，如果您有任何建议，
欢迎来提交 issue 或者 pull request

## 示例代码
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

## 贡献
任何贡献都欢迎通过 issue 和 PR 提交！

## 开发者
[![](https://github.com/mikecovlee.png?size=50)](https://github.com/mikecovlee)
[![](https://github.com/imkiva.png?size=50)](https://github.com/imkiva)

## 开源协议
```
MIT License

Copyright (c) 2020 Covariant Institute
```

