#include <winsock2.h>
#include <ws2tcpip.h> // Required for modern IP string conversions
#pragma comment(lib, "ws2_32.lib") // Tells Visual Studio to link the network library

#include <mutex>
#include <thread>
#include <string>
#include <cstdint>
#include <unordered_set>
#include <condition_variable>
#include "ThreadSafeQueue.h"
#include "MessageFramer.h"

class TCPServer{
    private:
        int port;
        std::string ip;
        std::mutex mtx;
        std::thread thread;
        SOCKET serverSocket;
        std::unordered_set<SOCKET> clients;
        ThreadSafeQueue<std::string>* queue;

        void startAccepting() {   //Creating per connection per thread. System failure at massive scale.
            while (true){
                SOCKET clientSocket = accept(serverSocket, nullptr, nullptr); //a blocking system call freeze the thread untill client calls stores client

                if (clientSocket == INVALID_SOCKET)  //when server socket is destroyed
                    break;

                {
                    std::lock_guard<std::mutex> lock(mtx);
                    clients.insert(clientSocket);
                }

                std::thread recieve([this, clientSocket]() {
                    MessageFramer mf(queue);
                    while(true){
                        char buffer[1024];
                        int bytesReceived = recv(clientSocket, buffer, 1024, 0);  //recv = 0 nodejs hang up the connection  recv < 0 connection dropped                

                        if (bytesReceived <= 0){
                            closesocket(clientSocket);   //free up memory by closing file decriptor 
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                clients.erase(clientSocket);
                            }
                            
                            break;
                        }

                        // std::cout << "bytes received: " << bytesReceived << std::endl;
                        mf.framing_and_append(buffer, bytesReceived);
                    
                    }
                });

                if (recieve.joinable()){
                    recieve.detach();
                }

                
            }
        }

    public:
        TCPServer(int port, std::string ip, ThreadSafeQueue<std::string>* q){
            this->queue = q;
            this->port = port;
            this->ip = ip;

            //Initialize Windows Networking
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);

            //Create the Socket (IPv4, TCP)
            serverSocket = socket(AF_INET, SOCK_STREAM, 0);  //system call to create a network endpoint  (opens a file descriptor) #Receptionist
            
            //Pack the IP and Port into the C-struct
            sockaddr_in serverAddr;
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(port); // Flip the bytes for the network card
            serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

            bind(serverSocket, (struct sockaddr*)& serverAddr, sizeof(serverAddr));    //bind ip and port to that endpoint
            listen(serverSocket, SOMAXCONN);     //listening to that endpoint let wait clients (OS MAX Limit) while receptionist is busy. TCP Backlog Limit = SOMAXCONN
            
            this->thread = std::thread([this]() {startAccepting();});
            

        }

        void alertBot(std::string ip){
            std::string payload = "BLOCK:" + ip;

            {
                std::lock_guard<std::mutex> lock(mtx);
                for (auto clientSocket: clients){
                    sendFramed(clientSocket, payload);
                }
            }

            std::cout << "[ALERT] Sent BLOCK command to all connected Node.js gateways for IP: " << ip << std::endl;
        }

        void sendFramed(SOCKET clientSocket, const std::string& message){

            std::uint32_t messageLen = message.size();
            const char* prefix_byte_ptr = reinterpret_cast<const char*>(&messageLen);
            const char* message_byte_ptr = message.data();

            send(clientSocket, prefix_byte_ptr, sizeof(messageLen), 0);
            send(clientSocket, message_byte_ptr, message.size(), 0);

        }

        ~TCPServer(){
            closesocket(serverSocket);  //prevents deadlock by focefully destroying the socket

            if (thread.joinable()) //thread finished execution must join to release memory
                thread.join();

            WSACleanup(); // Shut down Windows networking drivers
        }

};