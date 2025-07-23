#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <map>
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

  bool m_in_transaction = false;
  std::map<std::string, std::map<std::string, std::string>> m_transaction_buffer;

  // copy constructor and assignment operator are prohibited
  ClientConnection( const ClientConnection & );
  ClientConnection &operator=( const ClientConnection & );

  std::set<Table*> m_locked_tables;

  void send_ok(std::string encoded_msg);
  void send_failed(const std::string &what, std::string encoded_msg);
  void send_error(const std::string &what, std::string encoded_msg);
  void send_data(const std::string &data, std::string encoded_msg);

  void create(Message req, std::string encoded_msg);
  void push(Message req, std::string encoded_msg);
  void pop(std::string encoded_msg);
  void top(std::string encoded_msg);
  void set(std::string encoded_msg);
  void get(Message req, std::string encoded_msg);
  bool arithmetic(Message req, std::string encoded_msg);
  void begin(std::string encoded_msg);
  void commit(std::string encoded_msg);


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
