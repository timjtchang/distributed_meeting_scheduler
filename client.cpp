#include "header.h"

#define PORT SERVERTCPPORT

using namespace std;

string MYPORT = "";

/*
*   Reference from Beej's guide
*
*/
void* get_in_addr( struct sockaddr* sa ){

    if( sa->sa_family == AF_INET ){

        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr );

}

int main( int argc, char* argv[] ){

    cout<<"Client is up and running."<<endl;

    /*
    *   Reference from Beej's guide ------------
    *
    */

    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;

    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(( rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) !=0 ){

        fprintf( stderr, "getaddrinfo: %s\n", gai_strerror(rv) );
        return 1;
    }

    for( p = servinfo ; p != NULL; p=p->ai_next ){

        if((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1 ){

            //perror("client: socket");
            continue;
        }

        if( connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 ){

            close(sockfd);
            //perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {

        fprintf(stderr, "client: failed to connect\n"); 
        return 2;
    }else{

        struct sockaddr_in mysock;
        socklen_t len = sizeof(mysock);

        if (getsockname(sockfd, (struct sockaddr *)&mysock, &len) == -1)
            perror("getsockname");
        else
            MYPORT = to_string( ntohs(mysock.sin_port));  // from libraries string
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);

    /**
     * Get port
     * 
    */



    freeaddrinfo(servinfo); // all done with this structure

    /*
    *   -----------------------------------------------------------
    *
    * 
    * */
    

    while(1){

        cout<<"Please enter the usernames to check schedule availability:"<<endl;
        string msg;
        getline(cin, msg );

        if (send(sockfd, msg.c_str(), msg.length()+1, 0) == -1){  // send name list

            perror("send"); 
            close(sockfd);
            exit(0); 
        }

        cout<<"Client finished sending the usernames to Main Server."<<endl;

        char buf[MAXDATASIZE];
        int numbytes;

        while(1){

            if ((numbytes = recv( sockfd, buf, MAXDATASIZE-1, 0)) == -1) { 
                perror("recv");
                exit(1); 
            }

            buf[numbytes] = '\0';

            if( buf[0]=='#' ) break;  // if all names does not exist, the main server will send #, and the loop will be breaked.
        
            cout<<"Client received the reply from the Main Server using TCP over port "<<MYPORT<<"."<<endl;

            cout<<buf<<endl;
            
            if( buf[0] == 'T' && buf[1] == 'i') break;

        }

        cout<<"-----Start a new request-----"<<endl;
       
    } 

    close(sockfd);
    return 0; 
}