#include <iostream>    
#include <cstdlib>        
#include <string>         
#include <unistd.h>       
#include "csapp.h"        
#include "message.h"     
#include "message_serialization.h" 
#include "exceptions.h"   


// helper fxn to accept input
void handle_message(const Message &m, int clientfd, rio_t in, std::string encoded_msg, Message msg ) {
    MessageSerialization::encode(m, encoded_msg);
    Rio_writen(clientfd, encoded_msg.c_str(), encoded_msg.size());
    char buf[Message::MAX_ENCODED_LEN];                   
    Rio_readlineb(&in, buf, sizeof(buf));
    MessageSerialization::decode(buf, msg);
    if (msg.get_message_type() != MessageType::OK) {
        std::cerr << "Error: " << msg.get_quoted_text() << "\n";
        close(clientfd);
        std::exit(1);
    }
};

int main(int argc, char **argv)
{
  if ( argc != 6 ) {
    std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2];
  std::string username = argv[3];
  std::string table = argv[4];
  std::string key = argv[5];

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

  //Login
  handle_message(Message(MessageType::LOGIN, {username}), clientfd, in, encoded_msg, msg);
  //Get
  handle_message(Message(MessageType::GET, {table, key}), clientfd, in, encoded_msg, msg);

  //Top
  {
    Message m(MessageType::TOP);  
    MessageSerialization::encode(m, encoded_msg);
    Rio_writen(clientfd, encoded_msg.c_str(), encoded_msg.size());
    char buf[Message::MAX_ENCODED_LEN];                       
    Rio_readlineb(&in, buf, sizeof(buf));                    
    MessageSerialization::decode(buf, msg);
    if (msg.get_message_type() == MessageType::DATA) {
        std::cout << msg.get_value() << "\n";  
    } else {
        std::cerr << "Error: expected DATA, got " << static_cast<int>(msg.get_message_type()) << "\n";
        close(clientfd);
        return 1;
    }
  }

  //Bye
  {
    Message m(MessageType::BYE);
    MessageSerialization::encode(m, encoded_msg);
    Rio_writen(clientfd, encoded_msg.c_str(), encoded_msg.size());
  }

  close(clientfd);
  return 0; 
}
