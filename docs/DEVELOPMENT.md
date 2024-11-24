# Development

## 
```
<!-- Using AEAN for sanity check --> -->
rm -rf build
mkdir build
cd build
cmake .. -DUSE_ASAN=ON
make

<!-- Active debug mode -->
rm -rf build
mkdir build
cd build
cmake .. -DDEBUG=ON
make

<!-- Using LLDB for debugging -->
lldb ./tests

breakpoint matchingEnging.cpp:1
b matchingEnging::matchOrder
b list
p variable_name

run
frame variable
next
step
bt

quit
```

## Development Log

### [2024-11-24] 错误修复
工作内容

1. 使用 LLDB 和 AddressSanitizer (ASAN) 修复内存错误：
    - 调试和核心跟踪内存問题。
    - 优化内存释放过程，避免使用释放后的指针。
2. 更改了修改计划单的逻辑：
    - 处理修改限价单导致价格交叉的问题，确保交易登记和匹配正确。

### [2024-11-19] 修复 CMake 配置问题（编译过程调试）
工作内容
1. 修复 CMake 配置问题:
    - 修复 `utility` 和 `MatchingEngine` 之间的依赖错误，可执行文件要添加工具库的头文件路径，并且不应该 include 使用绝对路径
2. 修改代码中的 Bug：
    - 修正部分代码中的逻辑错误，使项目可以通过编译。
3. 测试框架跑通：
    - 成功运行 Google Test 框架进行测试用例。

### [2024-11-15] Preperation for server class testing
- Reconfigre CMake file: Integrate Google Test and spdlog libraries
- Remove the header file and definition of logger.h
- Change TODO List