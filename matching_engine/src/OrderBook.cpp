#include <utility>
#include <iostream>
#include <limits>
#include "OrderBook.h"
#include "Order.h"

double POS_INF = std::numeric_limits<double>::infinity();
double NEG_INF = -std::numeric_limits<double>::infinity();


OrderBook::OrderBook(std::string instrument) : instrument(std::move(instrument)), bestBidLevel(nullptr), bestAskLevel(nullptr)
{

    try {
        constexpr int preallocatePriceLevelSize = 1000;
        constexpr int preallocateOrderNodeSize = 10000;
        for (size_t i = 0; i < preallocateOrderNodeSize; i++)
        {
            emptyOrderNodeStack.push(new OrderNode());
        }
        for (size_t i = 0; i < preallocatePriceLevelSize; i++)
        {
            emptyPriceLevelStack.push(new PriceLevel());
        }
    } catch(const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << '\n';
        while (!emptyOrderNodeStack.empty()) {
            delete emptyOrderNodeStack.top();
            emptyOrderNodeStack.pop();
        }
        while (!emptyPriceLevelStack.empty()) {
            delete emptyPriceLevelStack.top();
            emptyPriceLevelStack.pop();
        }
        throw; // Continue to throw an error
    }

    // Initializes timer or other resources for cleanup
}

void OrderBook::setCrossCallback(CrossCallback callback) {
    crossCallback = std::move(callback);
}

OrderBook::~OrderBook()
{
    // release all the OrderNode and PriceLevel in the buySide
    for (auto &pair : priceToPriceLevel)
    {
        PriceLevel *level = pair.second;

        OrderNode *currentNode = level->headOrder;
        while (currentNode != nullptr)
        {
            OrderNode *nextNode = currentNode->next;
            delete currentNode->order;
            delete currentNode;
            currentNode = nextNode;
        }

        delete level;
    }

    // Release nullptr in emptyOrderNodeStack
    while (!emptyOrderNodeStack.empty())
    {
        delete emptyOrderNodeStack.top();
        emptyOrderNodeStack.pop();
    }

    // Release nullptr in emptyPriceStack
    while (!emptyPriceLevelStack.empty())
    {
        delete emptyPriceLevelStack.top();
        emptyPriceLevelStack.pop();
    }
}

void OrderBook::cancelLimitOrder(unsigned int orderId)
{
    // Look up the order node associated with the given order ID
    auto it = orderIdToOrderNode.find(orderId);
    if (it == orderIdToOrderNode.end())
    {
        std::cerr << "Order ID " << orderId << " not found.\n";
        return; // If the order is not found, return immediately
    }

    OrderNode *orderNode = it->second; // Get the pointer to the order node
    Order *order = orderNode->order;

    removeOrderNodeFromBook(orderNode);
    delete order;
    orderIdToOrderNode.erase(it);
}

void OrderBook::cancelStopOrder(unsigned int orderId)
{
    // TODO: Cancel StopOrder logic
}

void OrderBook::modifyLimitOrder(unsigned int orderId, double newPrice, int newQuantity)
{
    auto it = orderIdToOrderNode.find(orderId);
    if (it == orderIdToOrderNode.end()){
        // If orderId does not exist in orderIdToOrderNode, return
        std::cerr << "OrderId " << orderId << " does not exist";
        return;
    }

    OrderNode* orderNode = it->second;
    Order* order = orderNode->order;

    if ((newPrice <= 0.0) || (newQuantity < 0)){
        // Input validation
        throw std::invalid_argument("Invalid input, newPrice and newQuantity must be greater than 0.");
    } else if (newQuantity == 0){
        // If the newQuantity is zero, call function cancelOrder
        cancelLimitOrder(orderId);
        return;
    }

    
    if (newPrice == order->getPrice()){
       // If the price is unchanged, just update the quantity of the order and the priceLevel
        double oldPrice = order->getPrice();
        int oldQuantity = order->getQuantity();
    
        order->setQuantity(newQuantity);
        PriceLevel* priceLevel = priceToPriceLevel[oldPrice];
        priceLevel->totalQuantity += (newQuantity - oldQuantity);
        return;
    }

    // Price is changed, we cancel the original order and create a new one
    std::string asset =  order->getAsset();
    bool isBuy = order->isBuy();
    cancelLimitOrder(orderId);
    Order *newOrder = Order::CreateLimitOrder(orderId, asset, newPrice, newQuantity, isBuy);
    if (crossCallback) 
    { 
        std::cout << "this is a call back" << std::endl;
        crossCallback(newOrder);
    }
}

void OrderBook::modifyStopOrder(unsigned int orderId, double newPrice, int newQuantity)
{
    // TODO: Modify StopOrder Logic
}

Order *OrderBook::getBestBid() const
{
    if (bestBidLevel == nullptr)
    {
        return nullptr;
    }
    return bestBidLevel->headOrder->order;
}

Order *OrderBook::getBestAsk() const
{
    if (bestAskLevel == nullptr)
    {
        return nullptr;
    }
    return bestAskLevel->headOrder->order;
}

void OrderBook::printOrderBook() const
{
    std::cout << "****----- Order Book for " << instrument << " ------****\n";

    // Print all the ask levels, from low to high;
    std::cout << "Asks:\n";
    const PriceLevel* askLevel = bestAskLevel;
    while (askLevel != nullptr) {
        printPriceLevel(askLevel, 1);
        askLevel = askLevel -> nextPrice;
    }

    // Print all the bid levels, from high to low;
    std::cout << "----------------------------\n";
    std::cout << "Bids:\n";
    std::cout << "----------------------------\n";

    const PriceLevel* bidLevel = bestBidLevel;
    while (bidLevel != nullptr) {
        printPriceLevel(bidLevel, 1);
        bidLevel = bidLevel -> nextPrice;
    }

    std::cout << "----------------------------\n";
}

void OrderBook::printOrderBook(double minPrice, double maxPrice) const
{
    std::cout << "----- Order Book for " << instrument << " from " << minPrice << " to " << maxPrice << " ------\n";

    std::cout << "Asks:\n";
    const PriceLevel* askLevel = bestAskLevel;
    while (askLevel != nullptr) {
        if (askLevel->price >= minPrice && askLevel->price <= maxPrice) {
            printPriceLevel(askLevel, 1);
        }
        askLevel = askLevel -> nextPrice;
    }

    std::cout << "----------------------------\n";
    std::cout << "Bids:\n";
    std::cout << "----------------------------\n";

    const PriceLevel* bidLevel = bestBidLevel;
    while (bidLevel != nullptr) {
        if (bidLevel->price >= minPrice && bidLevel->price <= maxPrice) {
            printPriceLevel(bidLevel, 1);
        }
        bidLevel = bidLevel -> nextPrice;
    }
    
    std::cout << "----------------------------\n";
}

void OrderBook::printOrderBook(int depth) const
{
    std::cout << "----- Order Book for " << instrument << " ------\n";

    std::cout << "Asks:\n";
    unsigned int currentAskDepth = 1;
    const PriceLevel* askLevel = bestAskLevel;
    while ((askLevel != nullptr) && (currentAskDepth <= depth)) {
        printPriceLevel(askLevel, 1);
        askLevel = askLevel -> nextPrice;
        ++currentAskDepth;
    }

    std::cout << "----------------------------\n";
    std::cout << "Bids:\n";
    std::cout << "----------------------------\n";

    unsigned int currentBidDepth = 1;
    const PriceLevel* bidLevel = bestBidLevel;
    while ((bidLevel != nullptr) && (currentBidDepth <= depth)) {
        printPriceLevel(bidLevel, 1);
        bidLevel = bidLevel -> nextPrice;
        ++currentBidDepth;
    }

    std::cout << "----------------------------\n";
}

void OrderBook::addPriceLevel(PriceLevel *priceLevel, PriceLevel *&bestLevel) // NOLINT(*-convert-member-functions-to-static)
{
    PriceLevel *prevPrice = nullptr;
    PriceLevel *nextPrice = bestLevel;

    bool isBuy = (priceLevel->side) == Side::BUY;

    // This while loop is to find the appropriate prePrice and nextPrice for our new priceLevel
    while ((nextPrice != nullptr) && ((isBuy && nextPrice->price > priceLevel->price) ||
                                    (!isBuy && nextPrice->price < priceLevel->price)))
    {
        prevPrice = nextPrice;
        nextPrice = nextPrice->nextPrice;
    }

    priceLevel->nextPrice = nextPrice;
    priceLevel->prevPrice = prevPrice;
    
    if (prevPrice == nullptr)
    {
        // The new priceLevel get inserted at the beginning of the chain
        bestLevel = priceLevel;
    }
    else
    {
        prevPrice->nextPrice = priceLevel;
    }

    if (nextPrice != nullptr)
    {
        // The new priceLevel get inserted at the ending of the chain
        nextPrice->prevPrice = priceLevel;
    }

}

void OrderBook::addLimitOrderToBook(Order *order)
{
    // Get a orderNode for the incoming order
    OrderNode *orderNode = getOrderNode();
    orderNode->order = order;

    // Add to the orderIdToOrderNode
    orderIdToOrderNode[order->getId()] = orderNode;

    // Get the priceLevel
    double price = order->getPrice();
    auto it = priceToPriceLevel.find(price);

    if (it != priceToPriceLevel.end())
    {   
        // Add the OrderNode to the book
        PriceLevel *priceLevel = it->second;
        OrderNode *tailNode = priceLevel->tailOrder;
        tailNode->next = orderNode;
        orderNode->prev = tailNode;
        priceLevel->tailOrder = orderNode;

        // Update totalQuantity of the priceLevel
        priceLevel->totalQuantity += order->getQuantity();

    } else
    {
        // If the priceLevel for the incoming node does not exist.
        
        PriceLevel *priceLevel = getPriceLevel();
        priceLevel->price = price;
        priceLevel->totalQuantity = order->getQuantity();
        priceLevel->headOrder = orderNode;
        priceLevel->tailOrder = orderNode;
        priceToPriceLevel[price] = priceLevel;

        if (order->isBuy())
        {
            priceLevel->side = Side::BUY;
            addPriceLevel(priceLevel, bestBidLevel);
        } 
        else
        {
            priceLevel->side = Side::SELL;
            addPriceLevel(priceLevel, bestAskLevel);
        }
    }
}

void OrderBook::addStopOrderToBook(Order *order)
{
    // Add a new StopOrder Logic 
}

void OrderBook::removeOrderNodeFromBook(OrderNode *orderNode)
{
    // Get the price of the order
    double price = orderNode->order->getPrice();
    PriceLevel *priceLevel = priceToPriceLevel[price];

    // Remove the order node from the price level's linked list
    if (orderNode->prev)
    {
        orderNode->prev->next = orderNode->next;
    }
    else
    {
        // If it is the head node of the price level
        if (orderNode->next)
        {
            priceLevel->headOrder = orderNode->next; // Update the head pointer
        }
        else
        {
            priceLevel->headOrder = nullptr; // If there are no other orders, clear the head pointer
        }
    }

    if (orderNode->next)
    {
        orderNode->next->prev = orderNode->prev;
    }
    else
    {
        // If it is the tail node of the price level
        priceLevel->tailOrder = orderNode->prev;
    }

    // Update Level Quantity
    priceLevel->totalQuantity -= orderNode->order->getQuantity();

    if (priceLevel->totalQuantity == 0)
    {
        removePriceFromBook(priceLevel);
    }
    else if (priceLevel->totalQuantity < 0)
    {
        throw std::logic_error("totalQuantity is lower than 0 after removing the order form book");
    }

    releaseOrderNode(orderNode);
}

void OrderBook::removePriceFromBook(PriceLevel *priceLevel)
{
    if (priceLevel == nullptr)
    {
        std::cerr << "Attempted to remove a null PriceLevel.\n";
        return;
    }

    if (priceLevel->totalQuantity != 0)
    {
        throw std::logic_error("Can not remove a priceLevel where TotalQuantity is not zero");
    }

    // Remove from the price level map
    auto it = priceToPriceLevel.find(priceLevel->price);
    if (it != priceToPriceLevel.end() && it->second == priceLevel)
    {
        priceToPriceLevel.erase(it);
    }

    // Remove from the doubly linked list
    if (priceLevel->prevPrice)
    {
        priceLevel->prevPrice->nextPrice = priceLevel->nextPrice;
    }
    else
    {
        // If it's the head of the list
        if (priceLevel->side == Side::BUY)
        // If this is the only level in that side of the book, it will set the bestLevel to a nullptr
        {   
            bestBidLevel = priceLevel->nextPrice;
        }
        else
        {
            bestAskLevel = priceLevel->nextPrice;
        }
    }

    if (priceLevel->nextPrice)
    {
        priceLevel->nextPrice->prevPrice = priceLevel->prevPrice;
    }
    else
    {
        // No update required for nextPrice's prevPrice since this is the last price level.
        // If this price level is the only one on its side of the order book,
        // the Best Bid or Best Ask is set to nullptr when handling prevPrice.
    }

    releasePriceLevel(priceLevel);
}

void OrderBook::updateBestPrices()
{
    std::cout << "This method will iterate through all the priceLevels in the book, " 
          << "which is highly resource-intensive." << std::endl;
    
    auto it = priceToPriceLevel.begin();
    double bestBidPrice = NEG_INF;
    double bestAskPrice = POS_INF;
    while (it != priceToPriceLevel.end())
    {   
        PriceLevel *priceLevel = it->second;
        if (priceLevel->side == Side::BUY) 
        {
            if (priceLevel->price > bestBidPrice)
            {
                bestBidPrice = priceLevel->price;
                bestBidLevel = priceLevel;
            }
        } else
        {
            if (priceLevel->price < bestAskPrice)
            {
                bestAskPrice = priceLevel->price;
                bestAskLevel = priceLevel;
            }
        }
        ++it;
    }
}

OrderBook::OrderNode *OrderBook::getOrderNode()
{   
    if (emptyOrderNodeStack.empty()) 
    {
        for (size_t i = 0; i < 2000; i++)
        {
            emptyOrderNodeStack.push(new OrderNode());
        }
    }
    OrderNode *emptyOrderNode = emptyOrderNodeStack.top();
    emptyOrderNodeStack.pop();
    return emptyOrderNode;
}

void OrderBook::releaseOrderNode(OrderNode *node)
{
    // This function will not delete any order.
    // It will only manage memory orderNode
    node->next = nullptr;
    node->prev = nullptr;
    node->order = nullptr;
    emptyOrderNodeStack.push(node);
}

OrderBook::PriceLevel *OrderBook::getPriceLevel()
{
    if (emptyPriceLevelStack.empty()) 
    {
        for (size_t i = 0; i < 200; i++)
        {
            emptyPriceLevelStack.push(new PriceLevel());
        }
    }
    PriceLevel *emptyPriceLevel = emptyPriceLevelStack.top();
    emptyPriceLevelStack.pop();
    return emptyPriceLevel;
}

void OrderBook::releasePriceLevel(PriceLevel *level)
{
    level->price = -1;
    level->totalQuantity = -1;
    level->side = Side::UNDEFINED;
    level->headOrder = nullptr;
    level->tailOrder = nullptr;
    level->nextPrice = nullptr;
    level->prevPrice = nullptr;
    emptyPriceLevelStack.push(level);
}

void OrderBook::cleanup()
{
    constexpr int minOrderNodeSize = 5000;
    constexpr int maxOrderNodeSize = 20000;
    constexpr int minPriceLevelSize = 1000;
    constexpr int maxPriceLevelSize = 2000;

    while(emptyOrderNodeStack.size() < minOrderNodeSize) {
        emptyOrderNodeStack.push(new OrderNode);
    }

    while (emptyOrderNodeStack.size() > maxOrderNodeSize)
    {
        delete emptyOrderNodeStack.top();
        emptyOrderNodeStack.pop();
    }

    while (emptyPriceLevelStack.size() < minPriceLevelSize) 
    {
        emptyPriceLevelStack.push(new PriceLevel);
    }

    while (emptyPriceLevelStack.size() > maxPriceLevelSize)
    {
        delete emptyPriceLevelStack.top();
        emptyPriceLevelStack.pop();
    }
    

}

void OrderBook::printPriceLevel(const PriceLevel *start, int depth) const// NOLINT(*-convert-member-functions-to-static)
{   
    // This function is used to print depth levels from the start.
    const PriceLevel* currentLevel = start; // Start with the incoming price layer
    int printedLevels = 0;

    while (currentLevel != nullptr && printedLevels < depth) {
        std::cout << "Price Level: " << currentLevel->price << "\n";
        std::cout << "Total Quantity: " << currentLevel->totalQuantity << "\n";
        std::cout << "Orders at this level:\n";

        OrderNode* currentOrderNode = currentLevel -> headOrder;
        while (currentOrderNode != nullptr) {
            Order* order = currentOrderNode->order;
            order->displayOrderInfo(); // display info of the node
            currentOrderNode = currentOrderNode->next;
        }

        // Move to the next priceLevel
        currentLevel = currentLevel -> prevPrice;
        printedLevels++;
    }

    // If the level is insufficient, the prompt ends
    if (printedLevels < depth)
    {
        std::cout << "No more price levels to display." << std::endl;
    }
}
