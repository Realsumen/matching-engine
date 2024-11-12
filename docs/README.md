# Matching Engine Project

本项目实现了一个基于双链表和哈希表的订单配对引擎系统，包含了订单处理、订单簿管理和交易记录生成的核心功能。

## TODO LIST

1. 添加matchingEngine 完善lastTradedPrice的功能
2. 在main.cpp中做前端的简单模拟
3. 测试
4. 为止损单做准备，观察者模块的设计
5. 止损单的逻辑实现，在 `OrderBook` 中引入回调接口
6. 测试
7. 行情接口
8. 异步设计，性能监控，并发设计，内存管理优化，精细化订单流模拟

## 模块职责

1. **MessageQueue（订单流管道）**
   - 提供接口给前段接收订单信息

2. **OrderManager（订单管理器）**
   - 从订单流管道接收message
   - 进行基本的验证与预处理，调用`MatchingEngine`的操作

3. **MatchingEngine（配对引擎）**
   - 负责接收来自 `OrderManager` 的新增订单，执行订单匹配和成交操作。在撮合过程中，`MatchingEngine` 会根据订单的匹配情况生成交易记录，并实时更新最后成交价格。如果订单部分或完全未成交，`MatchingEngine` 将调用 `OrderBook` 的接口，将剩余部分添加至订单簿。同时，`MatchingEngine` 通过观察者模式通知相关模块（如止损单触发器或行情接口）更新最新的成交价和行情信息。

4. **OrderBook（订单簿）**
   - 负责管理限价单和止损单的结构化存储，包括维护价格层次、订单链表结构以及价格到订单节点的映射。`OrderBook` 提供接口供 `OrderManager` 用于订单的增删改操作，并供 `MatchingEngine` 用于添加未成交订单。`OrderBook` 确保订单状态的高效管理，为撮合引擎提供可靠的数据支持。