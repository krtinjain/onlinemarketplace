#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <thread>
#include <sstream>
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
#include <arpa/inet.h>
#include "project.h"
#include <vector>
#include <ctime>
#define PORT "8000"
#define MAXDATASIZE 1000
#define BACKLOG 10 



using namespace std;


int main(int argc, char* argv[]) { 

     setenv("TZ", "/usr/share/zoneinfo/America/Los_Angeles", 1);
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
     
     vector<TCP_Message> vect;
     
     while(i < N)
     {
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
      TCP_Message msg;
      strcpy(msg.name, pring);
      msg.price = amt;
      vect.push_back(msg);
      i++;
     }
     
     /*for(int k = 0; k < N; k++)
     {
         cout<< vect[k].name<<endl;
     }*/
     
     string str1, str2; 
     struct addrinfo hints, *servinfo;
     struct sockaddr_storage;
     int sockfd; 
     int num, numbytes;
     
     memset(&hints, 0, sizeof hints);
     hints.ai_family = AF_UNSPEC;
     hints.ai_socktype = SOCK_STREAM;

     num = getaddrinfo("localhost", PORT, &hints, &servinfo);
     if (num != 0) 
     {
       return 1;
     }

     sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
     if (sockfd < 0) 
     {
       return 1;
     }

     num = connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
     
     if (num < 0) 
     {
       close(sockfd);
       return 1;
     }
     struct sockaddr_in *addr;
    addr = (struct sockaddr_in *)servinfo->ai_addr; 
     time_t t2 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
     cout << std::put_time(std::localtime(&t2), "%c %Z") << ": "<< "Connected to " <<inet_ntoa((struct in_addr)addr->sin_addr) << " on port "<< PORT << "."<< endl;
     

     freeaddrinfo(servinfo);
     for(int j = 0; j < N; j++)
     {
        time_t t3 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
     cout << std::put_time(std::localtime(&t3), "%c %Z") << ": "<< "Sent "<< vect[j].name << " for sale for "<< vect[j].price << "$."<< endl;
        vect[j].price = htonl(vect[j].price);
        num = send(sockfd, &vect[j], 24, 0);
        if (num < 0) 
        {
          cerr << "seller: send error" << endl;
          return 1;
        }
        this_thread::sleep_for(10ms);
        
     }
     close(sockfd);
     
     


  return 0;
} 
