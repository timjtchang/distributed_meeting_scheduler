
#include "header.h"

#define MYPORT SERVERAPORT
#define PORT SERVERUDPPORT


using namespace std;

// the interval time msg to main server:  IA[[time,time],[time,time],[time,time]]

/*
*   Reference from Beej's guide ---------
*
*/

void *get_in_addr(struct sockaddr *sa) {

    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr); 
}

int udp_listener( vector<string>& list ){ // receive requested name list from main server using UDP

    int sockfd;
    struct addrinfo hints, *servinfo, *p; 
    int rv;
    int numbytes;
    struct sockaddr_storage their_addr; 
    char buf[MAXBUFLEN];
    socklen_t addr_len;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, MYPORT, &hints, &servinfo)) != 0) { 

        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)); 
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
    // printf("listener: waiting to recvfrom...\n");
    addr_len = sizeof their_addr;

    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) { 
            
        perror("recvfrom");
        exit(1);
    }

    // printf("listener: got packet from %s\n", inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s));
    // printf("listener: packet is %d bytes long\n", numbytes); 
    buf[numbytes] = '\0';
    // printf("listener: packet contains \"%s\"\n", buf);

    string str = "";

    for( int i=3 ; i<numbytes ; i++ ){ // parse the string sent from main server and store it into list

        if( buf[i] == ',' || buf[i] == ']' ){

            list.push_back( str ); //vector
            str = "";
        }else{

            str.push_back(buf[i]); //vector
        }
        
    }

    close(sockfd); 

    return 0;

}

int sendMsg( char* msg, int numbytes ){ // send message to main server using UDP

    int sockfd;
    struct addrinfo hints, *servinfo, *p; 
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; // set to AF_INET to use IPv4 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { 

        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {

        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) { 
            
            perror("talker: socket"); 
            continue;
        }
        break; 
    }

    if (p == NULL) {

        fprintf(stderr, "talker: failed to create socket\n"); 
        return 2;
    }


    if ((numbytes = sendto(sockfd, msg, numbytes, 0, p->ai_addr, p->ai_addrlen)) == -1) {
        
        perror("talker: sendto"); 
        exit(1);
    } 
    freeaddrinfo(servinfo);
    // printf("talker: sent %d bytes to port %s\n", numbytes, PORT ); 

    close(sockfd); 

    return 0;

}

/*
*-----------------------------------------------------------------------------------
*/

string decorate( vector< vector<int>> data ){ // convert interval time from 2-D array to a string

    string results;

    results.push_back('['); //vector

    for( const auto& vt:data )results = results+"["+ to_string(vt[0]) +","+to_string(vt[1])+"],"; //string

    if( results.size() !=1) results.pop_back(); //string
    
    results.push_back(']'); //vector

    return results;
}

void getFile( unordered_map< string, vector< vector<int> >>& data){ // get input file and store it into a map

    ifstream file;
    string input;

    file.open("input_files/a.txt");  //fstream

    if (file.is_open()){        //fstream
        
        while ( getline(file,input) ){
            
            string name;

            int left = 0;
            int right = 0;

            string numA = "";
            string numB = "";

            bool name_flag = 0;

            for( int i=0 ; i<input.size()-1 ; i++ ){ // parse the string

                if( input[i] == ' ') continue;
                else if( input[i] == ';' ){

                    i++;
                    name_flag = true;

                    if( data.count(name) != 0 ) break;
                    continue;
                }else if( !name_flag ) name.push_back(input[i]); //vector
                else{
                    
                    if( input[i] == ',' || input[i] == '[' || input[i] == ']' || input[i] == ' ' ) continue;

                    for( ; input[i]!=','; i++) if( input[i]!='['||input[i]!=']') numA.push_back( input[i] ); //vector
                    i++;
                    for( ; input[i]!=']'; i++) if( input[i]!='['||input[i]!=']') numB.push_back( input[i] ); //vector

                    data[name].push_back( {stoi(numA), stoi(numB)}); //vector, string

                    numA = "";
                    numB = "";

                }
            }
            
        }
        file.close();
    }

    // for( const auto& str:data ){

    //     cout<<str.first<<" : ";
    //     for( const auto& num_vt:str.second ) cout<<"["<<num_vt[0]<<","<<num_vt[1]<<"]"<<endl;
    // }

}

// go through all the names requested from main server, map these name to the array for interval time, 
// and compare each interval name array to get a intersection array.

void getInterval( vector<string>& list, unordered_map< string, vector< vector<int> >>& data, vector<vector<int>>& results ){
    
    bool flag = false;
    vector<vector<int>> buf; // store the current intersection time in the buf

    for( const auto& name_vt:list ){  // iterate all name  <vector>

        if( !flag ){
            
            buf = data[name_vt];
            flag = true;
            
        }else{
            
            for( const auto& d_vt:data[name_vt] ){ // iterate all interval time mapped to the name <vector>

                for( int i=0 ; i<buf.size() ; i++  ){

                    if( buf[i][0]>=d_vt[0] && buf[i][1]<=d_vt[1] )results.push_back( buf[i] );  //vector
                    else if( buf[i][0]<=d_vt[0] && buf[i][1]>=d_vt[1] ){    // if one interval time totally cover another
                        
                        if( d_vt[1]!= buf[i][1]) buf.push_back( { d_vt[1], buf[i][1] } ); //vector
                        results.push_back( {d_vt[0], d_vt[1]} ); //vector

                        buf[i][0] = -1;
                        buf[i][1] = -1;
                        
                    }else if( buf[i][0]<=d_vt[0] && buf[i][1]<=d_vt[1] && buf[i][1]>d_vt[0] ){ //if left side is longer

                        results.push_back( {d_vt[0], buf[i][1]} ); //vector
                        buf[i][0] = -1;
                        buf[i][1] = -1;

                    } 
                    else if( buf[i][0]>=d_vt[0] && buf[i][1]>=d_vt[1] && buf[i][0]<d_vt[1] ){ // if right side is longer

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

            buf = results;
            results = {};
        }
        
    }
    
    results = buf;
   
}


int main(void) {
    
    std::cout<< "ServerA is up and running using UDP on port "<<SERVERAPORT<<"."<<endl;

    unordered_map< string, vector< vector<int> >> data;

    getFile( data );

    string name_list = "FA[";
    
    for( const auto& pr:data ){ //unordered_map
  
        name_list+=pr.first;
        name_list+=",";
    }

    name_list.pop_back();
    name_list+="]";

    char msg[ name_list.length()+1 ];
    strcpy( msg, name_list.c_str()) ;
    sendMsg( msg, sizeof(msg) );

    std::cout<<"ServerA finished sending a list of usernames to Main Server."<<endl;

    while(1){

        vector<string> requestedList;

        udp_listener( requestedList );
        std::cout<<"Server A received the usernames from Main Server using UDP over port "<<SERVERAPORT<<"."<<endl;

        vector<vector<int>> interval_time;

        getInterval( requestedList, data, interval_time ); // get interval time

        string decorated_time = decorate( interval_time );

        std::cout<<"Found the intersection result:"<<decorated_time<<" for ";
        for( int i=0 ; i<requestedList.size() ; i++ ){

            std::cout<<requestedList[i];
            if( i!=requestedList.size()-1 ) std::cout<<", ";
            else std::cout<<"."<<endl;
        }

        decorated_time = "IA"+decorated_time;
        msg[ decorated_time.length()+1 ];
        strcpy( msg, decorated_time.c_str()) ;
        sendMsg( msg, sizeof(msg) );

        std::cout<< "Server A finished sending the response to Main Server."<<endl;

    }
    
    return 0;

}