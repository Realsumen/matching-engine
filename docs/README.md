# Matching Engine Project

本项目实现了一个基于双链表，和哈希表的订单配对引擎系统，包含了订单接收处理，订单簿管理和交易记录生成的核心功能。

<img src="assets/README/diagram.png" alt="project structure" width="500">

<img src="assets/README/data-structure.png" alt="project structure" width="500">

## Usage

## 模块职责

1. **Gateway（网关）**
   - 客户信息网关：负责监听外部系统的数据输入与输出，通过协议解析与格式转换，为订单管理器和撮合引擎提供可靠的数据交互接口，同时支持多种通信协议（TCP, FIX）以确保系统的兼容性和稳定性。
   - 行情信息网关：从 MatchingEngine 或 OrderBook 收集处理后的数据，成交回报：如订单状态、成交价格、成交数量。行情信息：如当前的最优买卖价（BBO）或完整的深度行情（Level 2 数据）将这些信息推送给前端用户或外部市场数据消费者。

2. **MessageQueue（订单流管道）**
   - 提供接口给前段接收订单信息

3. **OrderManager（订单管理器）**
   - 从订单流管道接收message
   - 进行基本的验证与预处理，调用`MatchingEngine`的操作

4. **MatchingEngine（配对引擎）**
   - 负责接收来自 `OrderManager` 的新增订单，执行订单匹配和成交操作。在撮合过程中，`MatchingEngine` 会根据订单的匹配情况生成交易记录，并实时更新最后成交价格。如果订单部分或完全未成交，`MatchingEngine` 将调用 `OrderBook` 的接口，将剩余部分添加至订单簿。同时，`MatchingEngine` 通过观察者模式通知相关模块（如止损单触发器或行情接口）更新最新的成交价和行情信息。

5. **OrderBook（订单簿）**
   - 负责管理限价单和止损单的结构化存储，包括维护价格层次、订单链表结构以及价格到订单节点的映射。`OrderBook` 提供接口供 `OrderManager` 用于订单的增删改操作，并供 `MatchingEngine` 用于添加未成交订单。`OrderBook` 确保订单状态的高效管理，为撮合引擎提供可靠的数据支持。

5. **DataGenerator（数据生成器）**
   - 负责生成行情信息，并持久化逐笔订单数据。
