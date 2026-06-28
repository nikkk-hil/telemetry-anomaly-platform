#include <winsock2.h>
#include <ws2tcpip.h> // Required for modern IP string conversions
#pragma comment(lib, "ws2_32.lib") // Tells Visual Studio to link the network library

#include <string>
#include <condition_variable>
#include <ThreadSafeQueue.h>
#include <thread>

class TCPServer{
    private:
        int port;
        std::string ip;
        std::thread thread;
        SOCKET serverSocket;
        ThreadSafeQueue<std::string>* queue;

        void startAccepting() const {   //Creating per connection per thread. System failure at massive scale.
            while (true){
                SOCKET clientSocket = accept(serverSocket, nullptr, nullptr); //a blocking system call freeze the thread untill client calls stores client

                if (clientSocket == INVALID_SOCKET)  //when server socket is destroyed
                    break;

                std::thread recieve([this, clientSocket]() {
                    while(true){
                        char buffer[1024];
                        int byteRecieved = recv(clientSocket, buffer, 1024, 0);  //recv = 0 nodejs hang up the connection  recv < 0 connection dropped
                        
                        if (byteRecieved <= 0){
                            closesocket(clientSocket);   //free up memory by closing file decriptor 
                            break;
                        }

                        std::string data(buffer, byteRecieved);
                        queue->push(data);
                    
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

        ~TCPServer(){
            closesocket(serverSocket);  //prevents deadlock by focefully destroying the socket

            if (thread.joinable()) //thread finished execution must join to release memory
                thread.join();

            WSACleanup(); // Shut down Windows networking drivers
        }

};