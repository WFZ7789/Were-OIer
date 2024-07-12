# Were-OIer
## 目录
- [环境](#环境)
- [运行方法](#运行方法)
- 简要说明
## 环境
目前已经测试了如下环境：
- 系统：Windows 10 1809 64-bit
- 编译器：TDM-GCC 4.9.2 64-bit Release（DEV_C++ 5.11, with `-std=c++14`）
- 终端：旧版控制台（[怎样打开？](#附录A-旧版控制台)）
  
MacOSX版本测试了如下：
- macOS Monterey 12.7.5 M1
- 编译器：Homebrew GCC 14.1.0_1（with `-std=c++17`）
- 原生 zsh Shell
## 运行方法
将对应版本的 `main.cpp` 文件复制到本地，编译运行。或者先用 DEV-C++ 编译，再在 `cmd` 中执行 `./main.exe seed`。其中 `./main.exe` 是你的程序路径，`seed` 是种子，只能是 32 位整型。
GNU_Linux / MacOSX
使用你的编译器编译（`GNU g++` 最好，`clang` 没有 `stdc++.h`)，然后终端输入 `./main seed` 运行。其中 `./main` 是你的程序路径，`seed` 是种子，只能是 32 位整型。
## 附录
### 附录A 旧版控制台
![image](https://github.com/user-attachments/assets/9f41711c-94d7-4606-8fa3-60e6a1f20890)
控制台上方右键，打开“xxx属性”，勾选“使用旧版控制台(U)”框。
