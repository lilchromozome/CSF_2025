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

  //Login
  handle_message(Message(MessageType::LOGIN, {username}), clientfd, in, encoded_msg, msg);

  if (use_transaction) handle_message(Message(MessageType::BEGIN), clientfd, in, encoded_msg, msg);

  handle_message(Message(MessageType::GET, {table, key}), clientfd, in, encoded_msg, msg);  
  handle_message(Message(MessageType::PUSH, {"1"}), clientfd, in, encoded_msg, msg);    
  handle_message(Message(MessageType::ADD), clientfd, in, encoded_msg, msg);       
  handle_message(Message(MessageType::SET, {table, key}), clientfd, in, encoded_msg, msg);

  if (use_transaction) handle_message(Message(MessageType::COMMIT, {table, key}), clientfd, in, encoded_msg, msg);

  //Bye
  {
    Message m(MessageType::BYE);
    MessageSerialization::encode(m, encoded_msg);
    Rio_writen(clientfd, encoded_msg.c_str(), encoded_msg.size());
  }
  close(clientfd);
  return 0; 
}
