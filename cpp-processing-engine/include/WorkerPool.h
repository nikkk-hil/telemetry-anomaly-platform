#pragma once

#include <queue>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <unordered_map>
#include <iostream>
#include "ThreadSafeQueue.h"
#include "json.hpp"

using json = nlohmann::json;


class WorkerPool{
    private:
        std::mutex mtx;
        std::vector<std::thread> th; 
        ThreadSafeQueue<std::string>* queue;
        std::function<void(std::string)> alert;
        std::unordered_map <std::string, std::queue<long long>> ipFreq;

        void dataProcessing() {
            while(true){
                static int count = 0;
                if (++count % 1000 == 0) {
                    std::cout << "[QueueMonitor] approx size: " << queue->size() << std::endl;
                }

                std::string data; 
                if (!queue->wait_and_pop(data))  //shutdown condition.
                    break;
                
                try {
                // 1. Convert the dumb string into a smart JSON object
                json parsedData = json::parse(data);

                // 2. Access the keys! (The library automatically converts the types)
                std::string userIp = parsedData["ip"];
                long long timestamp = parsedData["timestamp"];
                bool shouldAlert = false;
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    auto& q = ipFreq[userIp];
                    while (!q.empty() && timestamp - q.front() > 10000) q.pop();
                    q.push(timestamp);
                    if (q.size() >= 50) {
                        shouldAlert = true;
                        while (!q.empty()) q.pop();
                    }
                }
                if (shouldAlert) alert(userIp);   // outside the lock
            }
                
                catch (const json::parse_error& e) {
                // SECURITY BONUS: If a hacker sends garbage text instead of JSON, 
                // json::parse will throw an error. We catch it here so the C++ engine doesn't crash!
                std::cout << "Dropped invalid network packet." << std::endl;
            }
                
            }
        }

    public:
        WorkerPool(int N, ThreadSafeQueue<std::string>* targetQueue, std::function<void(std::string)> func): queue(targetQueue), alert(func) {
            for(int i = 0; i < N; i++){
                th.emplace_back([this]() {dataProcessing();}); //telling each thread to run dataProcessing function with THIS specific WorkPool obj
            }
        }

        ~WorkerPool(){
            //prevent deadlock by waking up all the threads
            queue->shutdown();
            for (auto& t: th){
                if (t.joinable())    
                    t.join();  //it will block the destructor execution untill the threads are executed completly
            }
        }


};
