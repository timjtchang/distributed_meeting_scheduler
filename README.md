# Distributed Meeting Scheduler

## Overview

This project implements a distributed system for managing and querying name-time interval data across multiple servers using socket programming. It consists of a main server (ServerM), two auxiliary servers (ServerA and ServerB), and a client application. The system leverages both TCP and UDP sockets for inter-server communication and client-server interaction.

## System Architecture

- **ServerM**: Central coordinator
  - TCP Socket: Port 24849 (for client connections)
  - UDP Socket: Port 23849 (for inter-server communication)
- **ServerA**: Auxiliary data server
  - UDP Socket: Port 21849
- **ServerB**: Auxiliary data server
  - UDP Socket: Port 22849
- **Client**: User interface
  - TCP Socket: Dynamically assigned port

## Key Features

- Multi-socket management (TCP and UDP)
- Concurrent socket handling using `select()`
- Socket-based distributed data storage and retrieval
- Real-time client query processing over TCP sockets
- Inter-server communication via UDP sockets

## Technical Implementation

### ServerM

1. Initializes both TCP and UDP sockets
2. Implements a UDP listener for ServerA and ServerB communications
3. Uses `select()` for managing multiple socket connections
4. Processes client requests via TCP socket
5. Orchestrates data retrieval from ServerA and ServerB using UDP sockets

### ServerA and ServerB

1. File I/O for data ingestion (a.txt and b.txt respectively)
2. UDP socket initialization for communication with ServerM
3. Implements `sendto()` and `recvfrom()` for UDP-based messaging

### Client

1. TCP socket initialization and connection to ServerM
2. Utilizes `send()` and `recv()` for data exchange with ServerM

## Socket Programming Highlights

- **Non-blocking I/O**: Implemented in ServerM to handle multiple connections
- **Socket Address Structures**: Use of `sockaddr_in` for IPv4 addressing
- **Error Handling**: Robust error checking for all socket operations
- **Buffer Management**: Careful handling of send and receive buffers to prevent overflow
- **Connection Management**: Proper socket closure and resource cleanup

## Communication Protocols

- **TCP**: Used for reliable, ordered communication between Client and ServerM
- **UDP**: Employed for lightweight, fast communication between ServerM and ServerA/B

## Performance Considerations

- Asynchronous UDP communication for reduced latency in inter-server data exchange
- Efficient socket buffer management to optimize data transfer rates
- Use of `select()` in ServerM for improved I/O multiplexing

## Dependencies

- Standard C++ libraries: `<string>`, `<fstream>`, `<vector>`, `<set>`
- Socket programming headers: `<sys/socket.h>`, `<netinet/in.h>`, `<arpa/inet.h>`
- Custom `header.h` for system-wide configurations

## Usage

### Compilation

Compile each component using the following commands:

```bash
g++ -o serverM serverM.cpp -lsocket -lnsl
g++ -o serverA serverA.cpp -lsocket -lnsl
g++ -o serverB serverB.cpp -lsocket -lnsl
g++ -o client client.cpp -lsocket -lnsl
```

### Execution

1. Start the servers in the following order:

   ```bash
   ./serverM
   ./serverA
   ./serverB
   ```

2. Once all servers are running, start the client:

   ```bash
   ./client
   ```

### Using the Client

1. When prompted, enter a list of names separated by spaces:

   ```
   Enter names: lilith jameson zein
   ```

2. The system will process your request and return the available time slots for the given names.

3. If a name doesn't exist in the system, you'll receive a notification:

   ```
   tttt does not exist.
   ```

4. To make another query, simply enter a new list of names when prompted.

5. To exit the client, use the keyboard interrupt (Ctrl+C).

### Example Session

```
$ ./client
Enter names: lilith jameson
Available time slots: [2,3], [5,7]
Enter names: zein callie
Available time slots: [1,4], [6,8]
Enter names: lilith ererer
ererer does not exist.
Available time slots for lilith: [1,3], [4,6], [7,9]
Enter names: ^C
$
```

### Notes

- Ensure that the input files (a.txt and b.txt) are in the same directory as ServerA and ServerB executables.
- The system uses localhost (127.0.0.1) for all connections. Modify the source code if you need to use different IP addresses.
- The servers will continue running until manually terminated. Use Ctrl+C to stop each server when you're done.

## Future Enhancements

- Implement SSL/TLS for secure socket communications
- Explore asynchronous I/O libraries for improved socket performance
- Implement socket-level compression for optimized data transfer
