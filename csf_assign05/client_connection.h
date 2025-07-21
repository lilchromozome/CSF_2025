#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <set>
#include "message.h"
#include "value_stack.h"
#include "csapp.h"

class Server; // forward declaration
class Table; // forward declaration

class ClientConnection {
private:
  Server *m_server;
  int m_client_fd;
  rio_t m_fdbuf;

  // copy constructor and assignment operator are prohibited
  ClientConnection( const ClientConnection & );
  ClientConnection &operator=( const ClientConnection & );

  void send_ok(std::string encoded_msg);
  void send_failed(const std::string &what, std::string encoded_msg);
  void send_error(const std::string &what, std::string encoded_msg);
  void send_data(const std::string &data, std::string encoded_msg);

  ValueStack m_stack;    
  std::string m_table;     
  std::string m_key;   
  bool m_has_stack;   

public:
  ClientConnection( Server *server, int client_fd );
  ~ClientConnection();

  void chat_with_client();

  // TODO: additional member functions
};

#endif // CLIENT_CONNECTION_H
