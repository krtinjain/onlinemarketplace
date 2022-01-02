#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <thread>
#include <chrono>
#include <vector>
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
#include <sys/time.h>
#include <signal.h>
#include "project.h"
#define MAXDATASIZE 1000
#define BACKLOG 10 

using namespace std;
using namespace chrono_literals;
struct sockaddr_storage client_addr2; 
socklen_t client_addr_len2;

int main(int argc, char* argv[]) { 

     setenv("TZ", "/usr/share/zoneinfo/America/Los_Angeles", 1);
     //std::this_thread::sleep_for(3s);
     string str;
     ifstream infile; 
     if (argc > 1) {
        infile.open(argv[1]);
     }
     else
     {
        return 0;
     }
     getline(infile, str);
     
     int N;
     stringstream ss(str);
     ss >> N;
     time_t t1 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
     cout << std::put_time(std::localtime(&t1), "%c %Z") << ": "<< "Reading "<< N << " items from "<< argv[1]<< "."<< endl;
     int i = 0;
     vector<UDP_Message> vect;
     
     while(i < N)
     {
      //uint32_t act;
      uint32_t amt;
      char pring[20];
      string tring, fname; 
      getline(infile, str);
      int space = str.find(" ");
      tring  = str.substr(space+1);
      strcpy(pring, tring.c_str());
      stringstream ss1(str);
      ss1 >> amt >> fname;
      //cout << amt << " "<<pring<<endl;
      UDP_Message msg;
      strcpy(msg.name, pring);
      msg.price = htonl(amt);
      msg.action = htonl(OFFER);
      vect.push_back(msg);
      i++;
     }
     
//_______________________________________________________________________________________________
     
     struct addrinfo hints, *servinfo;

     int sockfd;
     int rv, numbytes;

     memset(&hints, 0, sizeof hints);
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_DGRAM;

     rv = getaddrinfo("localhost", UDP_PORT, &hints, &servinfo);
     if (rv != 0) {
       cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
       return 1;
     }

     sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
     if (sockfd == -1) {
       cerr << "buyer: socket" << endl;
       return 1;
     }
     struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec =  500000;
     setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
     //freeaddrinfo(servinfo);
     int j = 0;
     while(1)
     {
         
             time_t t1 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
             cout << std::put_time(std::localtime(&t1), "%c %Z") << ": "<< "Sent bid "<< ntohl(vect[j].price) << "$ on "<< vect[j].name << "."<< endl;
             //vect[j].price = htonl(vect[j].price);
             //vect[j].action = htonl(vect[j].action);

             numbytes = sendto(sockfd, &vect[j], sizeof(vect[j]), 0, servinfo->ai_addr, servinfo->ai_addrlen);
             if (numbytes < 0) 
             {
               cerr << "buyer: sentto" << endl;
               return 1;
             }
             std::this_thread::sleep_for(200ms);
          
          
          time_t now1 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
          struct sockaddr_storage client_addr;
          client_addr2 = client_addr; 
          socklen_t client_addr_len = sizeof(client_addr);
          client_addr_len2 = client_addr_len;
          vector<UDP_Message> vect2;
          int xy = 0;
          
             UDP_Message msg2;
             time_t t;
             //time_t now2 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
             //auto secs = std::chrono::duration_cast<std::chrono::seconds>(now2 - now1);
             
             rv = recvfrom(sockfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&client_addr, &client_addr_len);
             if (rv < 0) {
               //cerr << "server: recvfrom" << endl;
               continue;
             }
             else
             {
                 
                 msg2.price = ntohl(msg2.price);
                 msg2.action = ntohl(msg2.action);
                 if(msg2.action == 0)
                 {
                   t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                   cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bidding not open." << endl;
                 }
                 else if(msg2.action == 2)
                 {
                    t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                    cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bid " << msg2.price << "$ on "<< msg2.name << " accepted." <<endl;
                 }
                 else if(msg2.action == 3)
                 {
                    t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                    cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bid on " << msg2.name << " rejected, current bid price "<< msg2.price << "$." <<endl;
                 }
                 else
                 {
                   cout<<"Invalid return!"<<endl;
                 }
                 //vect2.push_back(msg2);
                 xy++;
                 j++;
                 if(j == N)
                 {
                   break;
                 }
                 
            }
            
          
            
          
     }
     while(1)
     {
            UDP_Message msg2;
             time_t t;
             //time_t now2 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
             //auto secs = std::chrono::duration_cast<std::chrono::seconds>(now2 - now1);
             
             rv = recvfrom(sockfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&client_addr2, &client_addr_len2);
             if (rv < 0) {
               //cerr << "server: recvfrom" << endl;
               continue;
             }
             else
             {
                 
                 msg2.price = ntohl(msg2.price);
                 msg2.action = ntohl(msg2.action);
                 if(msg2.action == 0)
                 {
                   t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                   cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bidding not open." << endl;
                 }
                 else if(msg2.action == 2)
                 {
                    t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                    cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bid " << msg2.price << "$ on "<< msg2.name << " accepted." <<endl;
                 }
                 else if(msg2.action == 3)
                 {
                    t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                    cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bid on " << msg2.name << " rejected, current bid price "<< msg2.price << "$." <<endl;
                 }
                 else
                 {
                   cout<<"Invalid return!"<<endl;
                 }
             }
        }
  freeaddrinfo(servinfo);
  close(sockfd);

  return 0;
     
} 
