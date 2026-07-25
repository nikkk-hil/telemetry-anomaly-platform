#pragma once   //This line tells the compiler to load this file once irrespective of how many files ask for it.

#include<queue>
#include<mutex>
#include<condition_variable>   //allows threads to suspend execution untill another thread notifies.



template <typename T>
class ThreadSafeQueue {
    private:
        std::queue<T> q;
        std::mutex mtx;
        std::condition_variable cv;
        atomic<bool> isShuttingDown{false};

    public:

        void push(T value){
            {
                std::lock_guard<std::mutex> lock(mtx);   //lock mutex
                q.push(value);  //unlock mutex if this line crashes
            }                   //lock automatically unlocks when scope ends.

            //notifying one waiting thread
            cv.notify_one();
        }

        bool wait_and_pop(T &value){

            std::unique_lock<std::mutex> lock(mtx);   //unlock mutex when thread goes to sleep and lock when thread is awake.
            
            // A precaution what if OS might accidentally wakes up consumer thread
            cv.wait(lock, [this]() {return (!q.empty() || isShuttingDown);});  //q is not empty awake thread otherwise sleep

            if (q.empty()) return false;    //when engine is shutting down and q.empty just tell the worker thread break the loop.

            // if (!q.empty()){      //can't use this method in place of wait because the thread will continously ran consumes CPU
            value = q.front();
            q.pop();
            return true;
            // }
        }

        void shutdown(){
            isShuttingDown = true;
            cv.notify_all();
        }

};