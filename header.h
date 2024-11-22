#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <errno.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <arpa/inet.h> 
#include <sys/wait.h> 
#include <signal.h>
#include "stdbool.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <unordered_map>
#include <set>

#define MAXDATASIZE 100000
#define MAXBUFLEN 10000

const char SERVERAPORT[] = "21849";
const char SERVERBPORT[] = "22849";
const char SERVERUDPPORT[] = "23849";
const char SERVERTCPPORT[] = "24849";