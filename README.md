# Distributed Meeting Scheduler

## Project Overview

This project implements a distributed meeting scheduling system using socket programming in C++. The system consists of a client, a main server (serverM), and two backend servers (serverA and serverB). It efficiently finds common time slots for multiple participants while maintaining privacy of individual schedules.

## System Architecture

- **Client**: Initiates requests and displays results
- **Main Server (serverM)**: Coordinates communication between client and backend servers
- **Backend Servers (serverA, serverB)**: Store user availability data and perform intersection calculations

## Key Features

- Distributed architecture for scalability and privacy
- TCP and UDP socket programming for inter-process communication
- Efficient algorithms for time interval intersection
- Dynamic port allocation for flexible deployment

## Technical Implementation

### Communication Protocols

- Client <-> Main Server: TCP
- Main Server <-> Backend Servers: UDP

### Data Structures

- Availability data: Vector of time intervals `vector<vector<int>>`
- User-to-server mapping: Unordered map `unordered_map<string, char>`

### Algorithms

- Time interval intersection: Custom algorithm to find overlapping time slots
- Load balancing: Distribution of users across backend servers

### Distributed Implementation

- Parallel processing of availability data on multiple backend servers
- Aggregation of results on the main server for final intersection

## Socket Implementation

The system utilizes both TCP and UDP sockets for communication between different components:

### TCP Sockets

- **Client-Main Server Communication**:
  The client establishes a TCP connection with the main server. It uses the `socket()`, `connect()`, `send()`, and `recv()` functions to create a socket, connect to the server, send requests, and receive responses. The client uses a dynamically assigned port for this connection.

- **Main Server-Client Communication**:
  The main server uses `socket()`, `bind()`, `listen()`, and `accept()` functions to create a TCP socket, bind it to a specific port, listen for incoming connections, and accept client connections. It then uses `send()` and `recv()` to communicate with the client.

### UDP Sockets

- **Main Server-Backend Server Communication**:
  For communication with backend servers, the main server creates UDP sockets using `socket()` and binds them to specific ports with `bind()`. It uses `sendto()` to send requests to backend servers and `recvfrom()` to receive responses.

- **Backend Server-Main Server Communication**:
  Backend servers (A and B) create UDP sockets with `socket()` and `bind()` functions. They use recvfrom() to receive requests from the main server and `sendto()` to send responses back.

### Error Handling

Robust error handling is implemented for all socket operations. This includes checking return values of socket functions and using perror() to print descriptive error messages if any operation fails.

### Dynamic Port Assignment

The client uses dynamically assigned ports for TCP connections. After connecting to the main server, it retrieves its local port number using `getsockname()` function.

### IPv6 Support

The system is designed to work with both IPv4 and IPv6. It uses the `AI_PASSIVE` flag with `getaddrinfo()` to handle both address families. A custom `get_in_addr()` function is implemented to extract the correct address structure regardless of the IP version used.

## Code Structure

- `client.cpp`: Client implementation
- `serverM.cpp`: Main server coordination logic
- `serverA.cpp` & `serverB.cpp`: Backend server implementations
- `header.h`: Common definitions and includes

## Build and Execution

```bash
make all
./serverM
./serverA
./serverB
./client
```

## Usage

1. Start the servers in the following order:

   ```
   ./serverM
   ./serverA
   ./serverB
   ```

2. Launch the client:

   ```
   ./client
   ```

3. When prompted, enter usernames to check schedule availability:

   ```
   Please enter the usernames to check schedule availability:
   alice bob charlie
   ```

4. The system will process the request and display available time slots:

   ```
   Time intervals [[2,3],[17,18]] works for alice, bob, charlie.
   ```

5. For a new request, enter more usernames when prompted:

   ```
   -----Start a new request-----
   Please enter the usernames to check schedule availability:
   ```

6. To exit the program, use `Ctrl+C` in each terminal window.

Note: Ensure that the input files `a.txt` and `b.txt` are present in the same directory as the executables for the backend servers to read user availability data.

## Performance Considerations

- Efficient UDP communication for backend server queries
- Optimized data structures for quick lookups and intersections
- Scalable to handle multiple concurrent client requests

## Future Enhancements

- Integration with calendar systems
- Support for recurring meetings
- Web-based frontend for improved accessibility
