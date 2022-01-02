#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <future>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <vector>
#include <stdio.h>
#include <unistd.h>
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
#include "project.h"
#include <arpa/inet.h>

#define PORT "8000"
#define MAXDATASIZE 1000
#define BACKLOG 10 



using namespace std;
using namespace chrono_literals;
auto tt1 = std::chrono::high_resolution_clock::now();

struct bidder{
  uint32_t max_bid;
  char name[20];
  struct sockaddr_storage netaddress;
  socklen_t netaddress_len; 

};

inline bool operator==(const UDP_Message& lhs, const UDP_Message& rhs)
{
    return lhs.price == rhs.price && strcmp(lhs.name,rhs.name) == 0;
}
inline bool operator==(const bidder& lhs, const bidder& rhs)
{
    return lhs.max_bid == rhs.max_bid && strcmp(lhs.name, rhs.name) == 0;
}


vector<vector<TCP_Message>> big_vect;
mutex m;
pthread_mutex_t mut;

void push_back_unq(vector<UDP_Message> vectr, UDP_Message msg)
{
    if(vectr.size() == 0)
    {
      vectr.push_back(msg);
      return;
    }
    else 
    {
      for(int i = 0; i < vectr.size(); i++)
      {
        if(vectr[i] == msg)
        {
          return;
        }

      }
      vectr.push_back(msg);
    }
    return;

}
void push_back_unq_bid(vector<bidder> vectr, bidder msg)
{
    if(vectr.size() == 0)
    {
      vectr.push_back(msg);
      return;
    }
    else 
    {
      for(int i = 0; i < vectr.size(); i++)
      {
        if(vectr[i] == msg)
        {
          return;
        }

      }
      vectr.push_back(msg);
    }
    return;

}

int buyer();

void seller_thread(int new_socket, char strng[INET_ADDRSTRLEN], unsigned short int xyz){
 
  int numbytes = 0;
  vector<TCP_Message> vect;
  while (1) {  
    //struct timeval tv;
    TCP_Message msg;
     numbytes = recv(new_socket, &msg, MAXDATASIZE-1, 0);
     if (numbytes <= 0) 
     {
       tt1 = std::chrono::high_resolution_clock::now();
       break;
     }
     this_thread::sleep_for(10ms);
     {
       std::lock_guard<std::mutex> lg {m}; 
       time_t t3 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
       //tt1 = t3;
       msg.price = ntohl(msg.price);
       cout << std::put_time(std::localtime(&t3), "%c %Z") << ": "<< "Received  "<< msg.name << " for sale for "<< msg.price << "$ from " << strng << ":"<< xyz << "(thread: " << this_thread::get_id()<< ")." <<  endl;
       vect.push_back(msg);

     }
    
  }
  //pthread_mutex_lock(&mut);
  big_vect.push_back(vect);
  //pthread_mutex_unlock(&mut);


  //buyer();
  return;
}

//________________________________________________________________________________________________________________

int seller()
{
  setenv("TZ", "/usr/share/zoneinfo/America/Los_Angeles", 1);
  //cout << "server" <<endl; 
  int sockfd, new_socket;             
  struct addrinfo hints, *servinfo;
  struct sockaddr_storage client_addr;
  socklen_t sin_size;
  int num; 

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  num = getaddrinfo(NULL, PORT, &hints, &servinfo);
  if (num != 0) 
  {
    return 1;
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (sockfd < 0) 
  {
    cerr << "server: socket" << endl;
    return 1;
  }

  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  
  
  num = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if (num == -1) 
  {
    close(sockfd);
    cerr << "server: bind" << endl;
    return 1;
  }
 time_t t1 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
 freeaddrinfo(servinfo);
  
  num = listen(sockfd, BACKLOG);
  if (num < 0) 
  {
    cerr << "server: listen" << endl;
    return 1;
  }
  //cout << "server: waiting for connections..." << endl;
  cout << std::put_time(std::localtime(&t1), "%c %Z") << ": "<< "Waiting for connection on port "<< PORT << "."<< endl;

  sin_size = sizeof client_addr;
 
  thread nq(buyer);
  nq.detach();
  while(1)
  {
 
     new_socket = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
     if (new_socket < 0) 
     {
       cerr << "server: accept error" << endl;
       return 1;
     }
     struct timeval tv;
     //tv.tv_sec = 3;
     //tv.tv_usec = 0;
     //setsockopt(new_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
     
     time_t t2 = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
     struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&client_addr;
     struct in_addr ipAddr = pV4Addr->sin_addr;
     unsigned short int xyz = pV4Addr->sin_port;
     char strng[INET_ADDRSTRLEN];
     inet_ntop( AF_INET, &ipAddr, strng, INET_ADDRSTRLEN );
     cout << std::put_time(std::localtime(&t2), "%c %Z") << ": "<< "Accepted a new TCP connection from "<< strng << ":"<< xyz << "."<< endl;
     
     thread t(seller_thread, new_socket, strng, xyz);
     t.detach();
  }
  
  close(new_socket); 
  close(sockfd);
  return 0;
}


//_____________________________________________________________________________________________________________________________-


int buyer()
{
  setenv("TZ", "/usr/share/zoneinfo/America/Los_Angeles", 1);
  int sockfd;              
  struct addrinfo hints, *servinfo;
  int rv, numbytes;
  vector<bidder> biddy;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;            // <<<---- First Change
  hints.ai_flags = AI_PASSIVE;
  rv = getaddrinfo(NULL, UDP_PORT, &hints, &servinfo);

  if (rv != 0) {
    cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
    return 1;
  }

  sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (sockfd == -1) {
    cerr << "server: socket" << endl;
    return 1;
  }
 
  // updated line
  int yes = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  struct timeval tv;
  tv.tv_sec = 8;
  tv.tv_usec =  0;
  //setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
 
  rv = bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen);
  if (rv == -1) {
    close(sockfd);
    cerr << "server: bind" << endl;
    return 1;
  }

  freeaddrinfo(servinfo); // all done with this structure
 
  //cout << "server: waiting for connections..." << endl;
  while (1) {
      
       
       struct sockaddr_storage client_addr; 
       socklen_t client_addr_len = sizeof(client_addr);
       int numbyte;
       vector<UDP_Message> vect;
       vector<UDP_Message> messages;
       UDP_Message msg; 
       char      ipstr[INET6_ADDRSTRLEN];
       char      ipstr2[INET6_ADDRSTRLEN];
       uint16_t  port;   
       rv = recvfrom(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, &client_addr_len);
       if (rv == -1) {
         //cerr << "server: recvfrom" << endl;
         break;
       }
       time_t t = std::chrono::system_clock::to_time_t(chrono::system_clock::now());
      
       if (client_addr.ss_family == AF_INET) {
         struct sockaddr_in *ipv4 = (struct sockaddr_in *)&client_addr;
         inet_ntop(AF_INET, &(ipv4->sin_addr), ipstr, sizeof ipstr);
         port = ntohs(ipv4->sin_port);

       }
       else {
         struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&client_addr;
         inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipstr, sizeof ipstr);
         port = ntohs(ipv6->sin6_port);

       }
       cout << std::put_time(std::localtime(&t), "%c %Z") << ": "<< "Received bid " << ntohl(msg.price) << "$ on "<< msg.name << " from "<< ipstr << ":"<< port << "."<<endl;
       bidder bid1;
       msg.price = ntohl(msg.price);
       msg.action = ntohl(msg.price);
       bid1.max_bid = msg.price;
       strcpy(bid1.name, msg.name);
       bid1.netaddress = client_addr;
       bid1.netaddress_len= client_addr_len;
       UDP_Message msg2;
          msg2.price = 9999999;
          msg2.action = 1;
          for(int i = 0; i < big_vect.size(); i++)
          {
            for(int j = 0; j < big_vect[i].size(); j++)
            {
               //cout << msg.name << endl;
              if(strcmp(msg.name,big_vect[i][j].name) == 0)
              {
                //cout<< big_vect[i][j].name << endl;
                if(msg2.price >= big_vect[i][j].price)
                {
                  msg2.price = big_vect[i][j].price;
                  
                }
                strcpy(msg2.name, big_vect[i][j].name);
              }
            }
          }
          push_back_unq(messages, msg2);
          int flag = 0;
          for(int i = 0; i < big_vect.size(); i++)
          {
            for(int j = 0; j < big_vect[i].size(); j++)
            {
              if(strcmp(msg.name,big_vect[i][j].name) == 0)
              {
                flag = 1;
              }
            }
          }
          auto tt2 = std::chrono::high_resolution_clock::now();
          auto timeinterval = std::chrono::duration_cast<std::chrono::milliseconds>(tt2 - tt1);
          double ms = timeinterval.count();
          //cout<< ms<<endl;
          if(flag == 0 || ms < 2900)
          {
            msg.price = htonl(msg.price);
            msg.action = htonl(0);
            numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, client_addr_len);
            if (numbytes == -1) {
            cerr << "server: senttobuyer error" << endl;
            return 1;
            }
            time_t noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
            cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bidding not open to " << ipstr << ":"<< port << "."<<endl;
            continue;
          }
          else if(flag == 1 && ms >= 2900)
          {
              if(bid1.max_bid < msg2.price)
              { 
                msg.price = htonl(msg2.price);
                msg.action = htonl(3);
                numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, client_addr_len);
                if (numbytes == -1) {
                  cerr << "server: senttobuyer error" << endl;
                  return 1;
                }
                time_t noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bid " << bid1.max_bid << "$ on "<< msg.name << " rejected to "<< ipstr << ":"<< port << "."<<endl;
                continue;
              }
              else
              {
                if(biddy.size() == 0)
                {
                  biddy.push_back(bid1);
                }
                else
                {
                  int flag2 = 0;
                  for(int i = 0; i < biddy.size(); i++)
                  {
                    if(strcmp(bid1.name, biddy[i].name) == 0)
                    {
                      flag2 = 1;
                      if(bid1.max_bid < biddy[i].max_bid)
                      {
                        msg.price = htonl(biddy[i].max_bid);
                        msg.action = htonl(3);
                        numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, client_addr_len);
                        if (numbytes == -1) {
                          cerr << "server: senttobuyer error" << endl;
                          return 1;
                        }
                        time_t noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                        cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bid " << ntohl(msg.price) << "$ on "<< msg.name << " rejected to "<< ipstr << ":"<< port << "."<<endl;
                        continue;
                      }
                      else
                      {
                        msg.price = htonl(bid1.max_bid);
                        msg.action = htonl(2);
                        numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, client_addr_len);
                        if (numbytes == -1) {
                          cerr << "server: senttobuyer error" << endl;
                          return 1;
                        }
                        time_t noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                        cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bid " << ntohl(msg.price) << "$ on "<< msg.name << " accepted to "<< ipstr << ":"<< port << "."<<endl;
                        struct sockaddr_in *coolip  = (struct sockaddr_in *)&(biddy[i].netaddress);
                        inet_ntop(AF_INET, &(coolip->sin_addr), ipstr2, sizeof ipstr2);
                        uint16_t port2 = ntohs(coolip->sin_port);
                        msg.price = htonl(bid1.max_bid);
                        msg.action = htonl(3);
                        numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&(biddy[i].netaddress), biddy[i].netaddress_len);
                        if (numbytes == -1) {
                          cerr << "server: senttobuyer error" << endl;
                          return 1;
                        }
                        noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                        cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bid " << ntohl(msg.price) << "$ on "<< msg.name << " rejected to "<< ipstr2 << ":"<< port2 << "."<<endl;
                        biddy.erase(biddy.begin() + i);
                        biddy.push_back(bid1);
                        continue;
                      }

                    }

                  }
                  if(flag2 == 0)
                  {
                    msg.price = htonl(bid1.max_bid);
                    msg.action = htonl(2);
                    numbytes = sendto(sockfd, &msg, sizeof(msg), 0, (struct sockaddr *)&client_addr, client_addr_len);
                    if (numbytes == -1) {
                      cerr << "server: senttobuyer error" << endl;
                      return 1;
                    }
                    time_t noww =  std::chrono::system_clock::to_time_t(chrono::system_clock::now());
                    cout << std::put_time(std::localtime(&noww), "%c %Z") << ": "<< "Sent bid " << ntohl(msg.price) << "$ on "<< msg.name << " accepted to "<< ipstr << ":"<< port << "."<<endl;
                    biddy.push_back(bid1);
                    continue;

                  }
                }

              }
          }  
       
       //this_thread::sleep_for(200ms);
    

  }
   

  return 0;
   
}


//___________________________________________________________________________________________________________


int main() 
{ 
   int x;
   seller();
   
   //this_thread::sleep_for(3s);
   //thread t4(buyer);
   //t4.join();
  
} 
