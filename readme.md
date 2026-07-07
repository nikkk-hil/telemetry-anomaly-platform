graph TD
    Client["Web Browser / Attacker"]
    Gateway["Node.js API Gateway"]
    TCPServer["C++ TCP Server"]
    Queue[("Thread-Safe Queue")]
    Workers["C++ Worker Pool"]

    Client -->|"HTTP Request"| Gateway
    
    Gateway -->|"1. Check Blocklist"| Bouncer{"Is IP Blocked?"}
    Bouncer -->|"Yes"| Drop["Return 403 Forbidden"]
    Bouncer -->|"No"| Serve["Serve Webpage"]
    
    Bouncer -.->|"2. Fire & Forget TCP"| TCPServer
    
    TCPServer -->|"Push String"| Queue
    
    Queue -->|"Pop & Parse JSON"| Workers
    
    Workers -->|"Sliding Window DSA"| Math{"> 50 req in 10s?"}
    Math -->|"Yes"| Alert["Trigger Pub/Sub Alert"]
    
    Alert -.->|"Broadcast BLOCK: IP"| TCPServer
    TCPServer -.->|"Send TCP Alert"| Gateway