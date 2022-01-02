#ifndef PROJECT_H 
#define PROJECT_H

#define TCP_PORT "8000"
#define UDP_PORT "8001"
#define NO_BIDDING 0
#define OFFER 1
#define ACCEPTED 2
#define REJECTED 3


struct TCP_Message {
  uint32_t  price;
  char      name[20];
};

struct UDP_Message {
  uint32_t  action;
  uint32_t  price;
  char      name[20];
};



#endif /* PROJECT_H */
