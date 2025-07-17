#include <iostream>    
#include <cstdlib>        
#include <string>         
#include <unistd.h>       
#include "csapp.h"        
#include "message.h"     
#include "message_serialization.h" 
#include "exceptions.h"   

int main(int argc, char **argv) {
  if ( argc != 6 && (argc != 7 || std::string(argv[1]) != "-t") ) {
    std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
    std::cerr << "Options:\n";
    std::cerr << "  -t      execute the increment as a transaction\n";
    return 1;
  }

  int count = 1;

  bool use_transaction = false;
  if ( argc == 7 ) {
    use_transaction = true;
    count = 2;
  }

  std::string hostname = argv[count++];
  std::string port = argv[count++];
  std::string username = argv[count++];
  std::string table = argv[count++];
  std::string key = argv[count++];

  // TODO: implement
  int clientfd;
  rio_t in;
  std::string encoded_msg;
  Message msg;

  try {
    clientfd = open_clientfd(hostname.c_str(), port.c_str());
  } catch (...) {
    std::cerr << "Error: could not connect to server\n";
    return 1;
  }
  Rio_readinitb(&in, clientfd);

  
}
