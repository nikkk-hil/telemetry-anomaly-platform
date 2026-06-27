#pragma once

#include <vector>
#include <thread>
#include "ThreadSafeQueue.h"

class WokerPool{
    private:
        ThreadSafeQueue<std::string>* queue;
        std::vector<std::thread> th; 

    public:
        WokerPool(int N, ThreadSafeQueue<std::string>* targetQueue){
            queue = targetQueue;
            for(int i = 0; i < N; i++){
                th.emplace_back([this]() {dataProcessing();}); //telling each thread to run dataProcessing function with THIS specific WorkPool obj
            }
        }

        void dataProcessing(){
            while(true){
                std::string data;//created a data variable I don't know the datatype 
                if (!queue->wait_and_pop(data))  //shutdown condition.
                    break;
                // process(data);  //process the data
            }
        }

        ~WokerPool(){

            //prevent deadlock by waking up all the threads
            queue->shutdown();
            for (auto& t: th){
                if (t.joinable())    
                    t.join();  //it will block the destructor execution untill the threads are executed completly
            }
        }


};