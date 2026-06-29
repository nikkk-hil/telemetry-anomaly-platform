
#include <iostream>
#include <string>

#include "../include/ThreadSafeQueue.h"
#include "../include/WorkerPool.h"
#include "../include/TCPServer.h"


int main() {
    ThreadSafeQueue<std::string> sharedQ;

    TCPServer server(8080, "127.0.0.1", &sharedQ);
    
    WorkerPool workers(4, &sharedQ, ([&server](std::string ip) {
        server.alertBot(ip);
    }));

    std::cout << "=========================================" << std::endl;
    std::cout << "Engine is LIVE at IP: 127.0.0.1 and Port: 8080. Type 'exit' to shutdown." << std::endl;
    std::cout << "=========================================" << std::endl;

    std::string s;
    while(std::cin >> s){
        if (s == "exit")
            break;
        else
            std::cout << "[ERROR] Invalid command, type 'exit' to shutdown."<< std::endl;
    }

    std::cout << "[System] Intiating shutdown....." << std::endl;

    return 0;
}