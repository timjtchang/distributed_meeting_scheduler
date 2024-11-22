#include "header.h"

#define TCPPORT SERVERTCPPORT // the
#define UDPPORT SERVERUDPPORT
#define BACKLOG 10 // how many pending connections queue will hold


/*     format
*   interval time msg from server A, B:  IA[[time,time],[time,time],[time,time]]
*   fowarding list msg form server A,B: FA[string,stirng ];
*/

using namespace std;


/* Container
*/

unordered_map< char, vector<vector<int>> > interval_time;
unordered_map< string, char > forwarding_table;

void update_forwarding_table( string& input ){    // store names list to a map where name map to server A or B

    for( int i=2 ; i<input.size() ; i++ ){

        if( input[i] == ']') break;
        else if( input[i] == '[' || input[i] == ' ' ) continue;

        string str = "";
        while(1){

            if( i>=input.size() || input[i] == ',' || input[i] == ']') break;
            else str.push_back( input[i] );

            i++;
        } 

        forwarding_table[ str ] = input[1];
    }

}

void update_interval_time( string& input ){  // store the intveral time sent from server A and B where [0] is A and [1] is B
    
    vector< vector< int>> results;
    for( int i=2 ; i<input.size()-1 ; i++ ){
        
        if( input[i]=='[' || input[i]==']' || input[i]==',' ) continue;
        else{
            
            string num_a = "";
            string num_b = "";
            
            for( ; input[i]!=',' ; i++ ) num_a.push_back( input[i] );
            i++;
            for( ; input[i]!=']' ; i++ ) num_b.push_back( input[i] );
            
            results.push_back( {stoi(num_a), stoi(num_b)} );
            
        }
    }

    interval_time[ input[1] ] = results;

}

string decorate( vector< vector<int>> data ){  // convert the time from 2-D array to a string

    string results;

    results.push_back('[');

    for( const auto& vt:data )results = results+"["+ to_string(vt[0]) +","+to_string(vt[1])+"],";

    if( results.size() != 1 ) results.pop_back();
    
    results.push_back(']');

    return results;
}


/*
*   Reference from Beej's guide -----------------------------------------------
*
*/


void sigchld_handler(int s) {

    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno; 

}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {

        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr); 

}

int udp_listener(){  // receive data using udp 

    int sockfd;
    struct addrinfo hints, *servinfo, *p; 
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr; 
    char buf[MAXBUFLEN] ={};
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, UDPPORT, &hints, &servinfo)) != 0) { 

        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            
            //perror("listener: socket"); 
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
            
            close(sockfd);
            //perror("listener: bind");
            continue; 
        }
        break; 
    }

    if (p == NULL) {

        fprintf(stderr, "listener: failed to bind socket\n"); return 2;
    }

    freeaddrinfo(servinfo);
    //printf("listener: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) { 
            
        perror("recvfrom");
        exit(1);
    }

    // printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
    // printf("listener: packet is %d bytes long\n", numbytes); 
    // buf[numbytes] = '\0';
    // printf("listener: packet contains \"%s\"\n", buf);

    vector<vector<int>> results;
    string input = string(buf);
    vector<string> list;

    if( input[0] == 'I') update_interval_time( input );  // I stands for interval time sent from Server A and Server B
    else if( input[0] == 'F' ){ // I stands for name list sent from server A and B

        update_forwarding_table( input );
        cout<< "Main Server received the username list from server "<< input[1] <<" using UDP over port "<<SERVERUDPPORT<<"."<<endl;
    } 

     
    close(sockfd); 

    return 0;

}

/*
*------------------------------------------------------------------------------------------------------
*
*/

void print_interval_time(){

    for( const auto& pr:interval_time ){

        cout<<pr.first;
        cout<<"[";
        for( const auto& vt:pr.second ){

            cout<<"["<<vt[0]<<","<<vt[1]<<"]"<<",";
        }

        cout<<"]"<<endl;;
    }
}

void print_forwarding_table(){

    vector<string> listA;
    vector<string> listB;

    for( const auto& pr:forwarding_table ){

        if( pr.second == 'A') listA.push_back(pr.first); //vector
        else if( pr.second == 'B' ) listB.push_back( pr.first ); //vector
    }

    cout<<"A: ";
    for( const auto& str:listA )cout<<str<<" ";
    cout<<endl;
    cout<<"B: ";
    for( const auto& str:listB )cout<<str<<" ";
    cout<<endl;
}

/*
*   Reference from Beej's guide -----------------------------------------------------------
*
*/

int udp_sender( char* msg, int numbytes, const char* PORT ){ // send data to server A and B using udp

    int sockfd;
    struct addrinfo hints, *servinfo, *p; 
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { 

        //fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) { 
            
            //perror("talker: socket"); 
            continue;
        }
        break; 
    }

    if (p == NULL) {

        //fprintf(stderr, "talker: failed to create socket\n"); 
        return 2;
    }


    if ((numbytes = sendto(sockfd, msg, numbytes, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        
        //perror("talker: sendto"); 
        exit(1);
    } 
    freeaddrinfo(servinfo);
    //printf("talker: sent %d bytes to localhost\n", numbytes ); 

    close(sockfd); 

    return 0;

}

/*
*   ------------------------------------------------------------------------------------------
*
*/

void getInterval( vector<vector<int>>& results ){   // compare interval time sent from A and B, and store the new interval time in results
     
    if( interval_time['A'].size() == 0 ) results = interval_time['B'];
    else if( interval_time['B'].size() == 0 ) results = interval_time['A'];

    vector<vector<int>> buf;
    buf = interval_time['A'];

    for( const auto& d_vt:interval_time['B'] ){  

        for( int i=0 ; i<buf.size() ; i++  ){

            if( buf[i][0]>=d_vt[0] && buf[i][1]<=d_vt[1] )results.push_back( buf[i] ); //vector
            else if( buf[i][0]<=d_vt[0] && buf[i][1]>=d_vt[1] ){  // if one interval time can cover another
                
                if( d_vt[1]!= buf[i][1]) buf.push_back( { d_vt[1], buf[i][1] } ); // vector
                results.push_back( {d_vt[0], d_vt[1]} ); //vector

                buf[i][0] = -1;
                buf[i][1] = -1;
                
            }else if( buf[i][0]<=d_vt[0] && buf[i][1]<=d_vt[1] && buf[i][1]>d_vt[0] ){ // if only left side is longer 

                results.push_back( {d_vt[0], buf[i][1]} ); //vector
                buf[i][0] = -1;
                buf[i][1] = -1;

            } 
            else if( buf[i][0]>=d_vt[0] && buf[i][1]>=d_vt[1] && buf[i][0]<d_vt[1] ){ // if only right side is longer

                results.push_back( {buf[i][0], d_vt[1]} ); //vector
                buf[i][0] = -1;
                buf[i][1] = -1;
            }                     
            
        }
    }
    
    set< vector<int>> tmp;
    for( const auto& vt:results ) tmp.insert( vt ); //set
    results = {};
    for( const auto& st:tmp ) results.push_back(st); //vector
   
}


int main(void) {

    // Container

    unordered_map< char, vector< vector<int>>> list;  

    cout<<"Main Server is up and running."<<endl;

    /*
    *   Reference from Beej's guide --------------------------------------------
    *
    */

    int sockfd, new_fd; // listen on sock_fd, new connection on new_fd 
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information 
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints); 
    hints.ai_family = AF_UNSPEC; 
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, TCPPORT, &hints, &servinfo)) != 0) { 
        
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
    
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) { 
            perror("server: socket"); continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {

            perror("setsockopt");
            exit(1); 
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) { 
            
            close(sockfd);
            perror("server: bind");
            continue; 
        }
        break; 
    }

    freeaddrinfo(servinfo); // all done with this structure
    if(p==NULL) {

        fprintf(stderr, "server: failed to bind\n"); 
        exit(1);
    }

    udp_listener();
    udp_listener();
    

    if (listen(sockfd, BACKLOG) == -1) { 
        
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes 
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        
        perror("sigaction"); 
        exit(1);
    }

    /*
    * -------------------------------------------------------------------------------------------
    *
    */

    while(1) { // main accept() loop

        /*
        *   Reference from Beej's guide
        *
        */

        interval_time = {};

        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size); 
        
        if (new_fd == -1) {
            perror("accept");
            continue; 


        }

        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);

        /*
        *   -----------------------------------------------------------------------------
        */

        while(1){

            char buf[MAXDATASIZE];
            int numbytes;

            // Handle some process here 

            if ((numbytes = recv( new_fd, buf, MAXDATASIZE-1, 0)) == -1) { 
                perror("recv");
                exit(1); 
            }

            buf[numbytes] = '\0';


            /*
            *           request list form client using TCP
            *
            * */
            cout<<"Main Server received the request from the client using TCP over port "<<SERVERTCPPORT<<"."<<endl;

            string listA = "LM[";
            string listB = "LM[";
            string notExist = "";
            string str = "";

            vector<string> valid_names = {};

            int a_num = 0;
            int b_num = 0;

            /*
            *       Process the list from client
            *
            * */

            for( int i=0 ; i<numbytes ; i++ ){  // using forwarding table to build two strings to send request to server A and B

                if( buf[i] == '\0' || buf[i] == ' ' ){

                    if( forwarding_table[str] == 'A' ){

                        if( listA.size()>3) listA += ',';
                        listA += str;
                        
                        valid_names.push_back(str); //vector
                        a_num++;
                    
                    }else if( forwarding_table[str] == 'B' ){

                        if( listB.size()>3 ) listB += ',';
                        listB += str;

                        valid_names.push_back(str); //vector
                        b_num++;

                    }else{

                        if( notExist.size() ) notExist += ',';
                        notExist += str;
                    }

                    str = "";

                }else str.push_back( buf[i] );

            }

            if( listA[ listA.size()-1] == ',' ) listA.pop_back(); //vector
            if( listB[ listB.size()-1] == ',' ) listB.pop_back(); //vector
            if( notExist[ notExist.size()-1 ] == ',') notExist.pop_back(); //vector

            listA.push_back(']');
            listB.push_back(']');


            if( notExist.size() || ( a_num==0 && b_num==0) ){     // send not exist list to client
                
                notExist+=" do not exist.\0";
                cout<<notExist<<" Send a reply to the client."<<endl;

                if (send(new_fd, notExist.c_str(), notExist.length()+1, 0) == -1){

                    perror("send"); 
                    close(new_fd);
                }

            }


            /*
            *       Send request for interval time to A and B
            *
            * */


            if( a_num==0 && b_num==0 ){ // if all name not exist
                
                for( int i=0 ; i<100000000 ; i++ ) int b=0;

                char msg[] = "#\0";

                if (send(new_fd, msg, sizeof(msg), 0) == -1){

                    perror("send"); 
                    close(new_fd);
                }

                continue;
            
            } 

            if( a_num ){

                cout<<"Found ";
                for( int i=2 ; i<listA.size() ; i++ ){

                    if( listA[i]=='[' || listA[i]==']' ) continue;
                    else cout<<listA[i];
                } 

                cout<<" located at Server A. Send to Server A."<<endl;

            }
            
            if( b_num ){

                cout<<"Found ";
                for( int i=2 ; i<listB.size() ; i++ ){

                    if( listB[i]=='[' || listB[i]==']' ) continue;
                    else cout<<listB[i];
                } 

                cout<<" located at Server B. Send to Server B."<<endl;

            }

            if( a_num ){                

                char msg[ listA.length()+1 ];
                strcpy( msg, listA.c_str()) ;  //string
                udp_sender( msg, sizeof(msg), SERVERAPORT );
                udp_listener();

                cout<<"Main Server received from server A the intersection result using UDP over port "<<SERVERUDPPORT<<"."<<endl;
                cout<<"[";
                for( int i=0 ; i<interval_time['A'].size() ; i++ ){

                    cout<<"["<<interval_time['A'][i][0]<<","<<interval_time['A'][i][1]<<"]";
                    if( i!=interval_time['A'].size()-1 ) cout<<",";
                }
                cout<<"]."<<endl;
            }

            if( b_num ){

                char msg[ listB.length()+1 ];
                strcpy( msg, listB.c_str()) ;
                udp_sender( msg, sizeof(msg), SERVERBPORT );
                udp_listener();

                cout<<"Main Server received from server B the intersection result using UDP over port "<<SERVERUDPPORT<<"."<<endl;
                cout<<"[";
                for( int i=0 ; i<interval_time['B'].size() ; i++ ){

                    cout<<"["<<interval_time['B'][i][0]<<","<<interval_time['B'][i][1]<<"]";
                    if( i!=interval_time['B'].size()-1 ) cout<<",";
                }
                cout<<"]."<<endl;

            }

            vector<vector<int>> new_time;
            getInterval( new_time );  // get new interval time

            /*
            *       Process new interval time from A and B
            *
            * */

            string tmp_time = "";
            if( new_time.size() ){

                tmp_time+="[";
                for( const auto& vt:new_time ) tmp_time = tmp_time+"["+to_string(vt[0])+","+to_string(vt[1])+"],"; //string
                tmp_time.pop_back();
                tmp_time+="]";
    
            }else tmp_time = "[]";

            cout<<"Found the intersection between the results from server A and B:"<<endl;
            cout<<tmp_time<<"."<<endl;

            string tmp_name = "";
            for( const auto& str:valid_names ) tmp_name = tmp_name+str+", ";
            if( tmp_name.size() ){
                
               tmp_name.pop_back();
               tmp_name.pop_back();
               tmp_name+=".";
            } 

            /*
            *   Send new interval time to client
            *
            * */

            string msg_string = "Time intervals "+tmp_time+" works for "+tmp_name;

            //cout<<msg_string<<endl;

            char msg[ msg_string.length()+1 ];
            strcpy( msg, msg_string.c_str()) ; //string
            
            msg[msg_string.length()] = '\0';

            // Handle some process here 

            if (send(new_fd, msg, sizeof( msg ), 0) == -1){

                perror("send"); 
                close(new_fd);
                exit(0); 
            }

            cout<<"Main Server sent the result to the client"<<endl;
        
        }

        close( new_fd );

    }
    return 0; 

}