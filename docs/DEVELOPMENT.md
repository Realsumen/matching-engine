# Development Log

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