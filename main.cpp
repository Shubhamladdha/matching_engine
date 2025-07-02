#include "matching_engine.h"
#include <random>
#include <thread>
#include <chrono>
#include <iostream>

void testClient(int clientId, MatchingEngine& engine) {
    SafeMsgQueue msgQueue;
    engine.registerClient(clientId, &msgQueue);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> priceDist(95, 105);
    std::uniform_int_distribution<> amountDist(1, 10);
    std::uniform_int_distribution<> sideDist(0, 1);
    for (int i = 0; i < 5; ++i) {
        int orderId = clientId * 100 + i;
        int price = priceDist(gen);
        int amount = amountDist(gen);
        Side side = static_cast<Side>(sideDist(gen));
        engine.placeOrder(clientId, {orderId, price, amount, side, clientId});
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (msgQueue.pop_print(clientId));
    }
    for (int i = 0; i < 2; ++i) {
        int cancelId = clientId * 100 + i;
        engine.cancelOrder(clientId, cancelId);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        while (msgQueue.pop_print(clientId));
    }
    while (msgQueue.pop_print(clientId));
}

int main() {
    MatchingEngine engine;
    std::thread t1(testClient, 1, std::ref(engine));
    std::thread t2(testClient, 2, std::ref(engine));
    t1.join();
    t2.join();
    return 0;
}
