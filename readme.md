graph TD
    Client[Web Browser or Attacker]
    Gateway[Node.js API Gateway]
    TCPServer[C++ TCP Server]
    Queue[(Thread-Safe Queue)]
    Workers[C++ Worker Pool]

    Client -->|HTTP Request| Gateway
    
    Gateway -->|1. Check Blocklist| Bouncer{Is IP Blocked?}
    Bouncer -->|Yes| Drop[Return 403 Forbidden]
    Bouncer -->|No| Serve[Serve Webpage]
    
    Bouncer -.->|2. Fire and Forget TCP| TCPServer
    
    TCPServer -->|Push String| Queue
    
    Queue -->|Pop and Parse JSON| Workers
    
    Workers -->|Sliding Window DSA| Math{Exceeds 50 requests?}
    Math -->|Yes| Alert[Trigger PubSub Alert]
    
    Alert -.->|Broadcast BLOCK Alert| TCPServer
    TCPServer -.->|Send TCP Alert| Gateway