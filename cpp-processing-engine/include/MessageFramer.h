#pragma once

#include <string>
#include <cstring>
#include <cstdint>
#include "ThreadSafeQueue.h"

class MessageFramer
{
private:
    std::string message;
    std::uint8_t prefixBuffer[4];
    size_t remainingPrefixBytes;
    size_t remainingMessageBytes;
    ThreadSafeQueue<std::string>* queue;


public:
    MessageFramer(ThreadSafeQueue<std::string>* q): queue(q) {
        remainingMessageBytes = 0;
        remainingPrefixBytes = 4;
    }

    void framing_and_append(char buffer[], int totalBytes){
        size_t usedBytes = 0;
        
        while(usedBytes != totalBytes){
            if (remainingMessageBytes == 0 && remainingPrefixBytes != 0){
                size_t toCopy = std::min(remainingPrefixBytes, totalBytes-usedBytes);
                std::memcpy(prefixBuffer + (4-remainingPrefixBytes), buffer + usedBytes, toCopy);
                remainingPrefixBytes -= toCopy;
                usedBytes += toCopy;

                if (remainingPrefixBytes == 0){
                    uint32_t messageLength;
                    std::memcpy(&messageLength, prefixBuffer, 4);
                    remainingMessageBytes = messageLength;
                }
            }

            else if (remainingMessageBytes != 0){
                size_t toCopy = std::min(remainingMessageBytes, totalBytes-usedBytes);
                message.append(buffer + usedBytes, toCopy);
                usedBytes += toCopy;
                remainingMessageBytes -=toCopy;
            }
        
            if (remainingPrefixBytes == 0 && remainingMessageBytes == 0){
                queue->push(message);
                remainingPrefixBytes = 4;
                message.clear();
            }

        }
    }

};