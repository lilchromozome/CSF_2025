#include <iostream>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include "exceptions.h"
#include "client_connection.h"

ClientConnection::ClientConnection( Server *server, int client_fd )
  : m_server( server )
  , m_client_fd( client_fd )
  , m_has_stack(false)
{
  Rio_readinitb( &m_fdbuf, m_client_fd );
}

ClientConnection::~ClientConnection()
{
  Close(m_client_fd);
}

//Ask server to create named table
void ClientConnection::create(Message req, std::string encoded_msg){
  std::string name = req.get_table();
  m_server->create_table(name);
  send_ok(encoded_msg);
  return;
}

// Push a value onto the operand stack
void ClientConnection::push(Message req, std::string encoded_msg){
  if (!m_has_stack) {
    m_stack = ValueStack();
    m_has_stack = true;
    // send_failed("no active key", encoded_msg);
  } 
  m_stack.push(req.get_value());
  send_ok(encoded_msg);
  return;
}

//Pop (discard) the top value from the operand stack
void ClientConnection::pop(std::string encoded_msg){
  if (!m_has_stack) {
    send_failed("no active key", encoded_msg);
  } else {
    try {
      m_stack.pop();
      send_ok(encoded_msg);
    } catch (const std::exception &e) {
      send_failed("stack empty", encoded_msg);
    }
  }
  return;
}

//	Retrieve the top value from the operand stack
void ClientConnection::top(std::string encoded_msg){
  if (!m_has_stack) {
    send_failed("no active key", encoded_msg);
  } else {
    try {
      std::string val = m_stack.get_top();
      send_data(val, encoded_msg);
    } catch (const std::exception &e) {
      send_failed("stack empty", encoded_msg);
    }
  }
  return;
}

// Set value of tuple named by key in table to the value popped from the operand stack
void ClientConnection::set(const Message &req, std::string encoded_msg){
  std::string table = req.get_table();
  std::string key = req.get_key();
  Table *t = m_server->find_table(table);

  if (!m_has_stack) {
    send_failed("no active key", encoded_msg);
  } 
  if (!t) {
    send_failed("no such table", encoded_msg);
    return;
  }

  //m_table = table;
  //m_key = key;
  //m_has_stack = true;

  //std::string final_val = m_stack.get_top();
  //m_stack.pop();


  if (m_in_transaction) {
    if(m_locked_tables.insert(t).second){
      if (!t->trylock()) {
        rollback_transaction();
        throw FailedTransaction("could not lock table");
      }
    }
     //t->set(m_key, final_val);
     
  } else {
    t->lock();
    //t->set(m_key, final_val);
    //t->commit_changes(); 
    //t->unlock();
  }

  try{
    std::string val = m_stack.get_top();
    m_stack.pop();
    t->set(key, val);

    if (!m_in_transaction) {
      t->commit_changes(); // commit changes immediately
      t->unlock();
    }

    m_table = table;
    m_key = key;
    m_has_stack = true;
    send_ok(encoded_msg);
  }catch (const std::exception &e) {
    if (!m_in_transaction) {
      t->unlock();
    }
    send_failed("set failed: " + std::string(e.what()), encoded_msg);
    return;
  }

  return;
}

// Push value of tuple named by key in table onto the operand stack
void ClientConnection::get(Message req, std::string encoded_msg){
    std::string table = req.get_table();
    std::string key   = req.get_key();
    Table *t = m_server->find_table(table);

    if (!t) {
      send_failed("no such table",encoded_msg);
      return;
    } 
    if (m_in_transaction) {
      if (m_locked_tables.insert(t).second){
        if (!t->trylock()) {
          rollback_transaction();
          throw FailedTransaction("could not lock table");
      }
    } else{
        t->lock();
    }
      // if (!t-> trylock()) throw FailedTransaction("could not lock table");
      // m_locked_tables.insert(t);
    }
    //if (!t->has_key(key)) {
      //if (!m_in_transaction) t->unlock();
      //send_failed("no such key", encoded_msg);
      //return;
    //}
    try {
      std::string val = t->get(key);
      m_stack = ValueStack(); // reset stack
      m_stack.push(val);
      m_table = table;
      m_key   = key;
      m_has_stack = true;

      if (!m_in_transaction) {
        t->unlock();
      }
      send_ok(encoded_msg);
    }catch (const OperationException &e) {
      if (m_in_transaction) {
        rollback_transaction();                        // rollback if inside transaction
        throw FailedTransaction("no such key");        // MUST throw
      } else {
        t->unlock();                                   // just unlock if no transaction
      send_failed("no such key", encoded_msg);
      }
      return;
    }
    
    // // load into working stack
    // t->lock();
    // std::string val = t->get(key);
    // t->unlock();

    //std::string val = t->get(key);
    //if (!m_in_transaction) t->unlock();

    //m_stack = ValueStack();             // reset stack
    //m_stack.push(val);
    //m_table = table;
    //m_key   = key;
    //m_has_stack = true;
    //send_ok(encoded_msg);
    //return;
}

// Pop two integers from operand stack, add them, push sum
// Pop two integers from operand stack, multiply them, push product
// Pop right and left integers from operand stack, subtract right from left, push difference
// Pop right and left integers from operand stack, divide left by right, push quotient
bool ClientConnection::arithmetic(Message req, std::string encoded_msg){
  if (!m_has_stack) {
    send_failed("no active key", encoded_msg);
  } else {
    // need two operands
    try {
      std::string s1 = m_stack.get_top();
      m_stack.pop();
      std::string s2 = m_stack.get_top();
      m_stack.pop();

      int a = std::stoi(s1);
      int b = std::stoi(s2);
      double result;
      switch (req.get_message_type()) {
        case MessageType::ADD: result = b + a; break;
        case MessageType::SUB: result = b - a; break;
        case MessageType::MUL: result = b * a; break;
        case MessageType::DIV:
          if (a == 0) { send_failed("divide by zero", encoded_msg); return true; }
          result = b / a;
          break;
        default: 
          send_error("unexpected operation", encoded_msg);
          return true;
      }
      m_stack.push(std::to_string(result));
      send_ok(encoded_msg);
    } catch (const std::invalid_argument &) {
      send_failed("non-integer operand", encoded_msg);
    } catch (const std::out_of_range &) {
      send_failed("integer overflow", encoded_msg);
    } catch (const std::exception &) {
      send_failed("stack empty", encoded_msg);
    }
  }
  return false;
}

// Client begins a transaction
void ClientConnection::begin(std::string encoded_msg){
  // make sure not in transaction
  if (m_in_transaction){
    send_failed("already in transaction", encoded_msg);
  } else {
    m_in_transaction = true;
    m_locked_tables.clear(); 
    send_ok(encoded_msg);
  }
  return;
}

// Client commits a transaction
void ClientConnection::commit(std::string encoded_msg){
   // TODO
  if (!m_in_transaction) {
    send_failed("not in transaction", encoded_msg);
  }
  try {
    for (Table *table : m_locked_tables) {
      table->commit_changes();  
    }
    for (Table *table : m_locked_tables) {
      table->unlock();
    }

    m_locked_tables.clear();
    m_in_transaction = false;
    send_ok(encoded_msg);
  } catch (const std::exception &e) {
    throw FailedTransaction("commit failed");
  }
}

void ClientConnection::rollback_transaction() {
  if (m_in_transaction) {
    for (Table *t : m_locked_tables) {
      t->rollback_changes();  // revert
    }
    for (Table *t : m_locked_tables) {
      t->unlock();
    }
    m_locked_tables.clear();
    m_in_transaction = false;
  }
}


void ClientConnection::chat_with_client()
{
  // TODO: implement
  char buf[Message::MAX_ENCODED_LEN];
  std::string encoded_msg;
  while (true) {
    if (Rio_readlineb(&m_fdbuf, buf, sizeof(buf)) <= 0)
      break; // client closed

    Message req;
    MessageSerialization::decode(buf, req);
    if (!req.is_valid()) {
      send_error("invalid request", encoded_msg);
      continue;
    }

    try{
      
    switch (req.get_message_type()) {
      // Client logs in
      case MessageType::LOGIN: {
        // Always ok?
        send_ok(encoded_msg);
        break;
      }
      case MessageType::CREATE: {
        create(req, encoded_msg);
        break;
      }
      case MessageType::PUSH: {
        push(req, encoded_msg);
        break;
      }
      case MessageType::POP: {
        pop(encoded_msg);
        break;
      }
      case MessageType::TOP: {
        top(encoded_msg);
        break;
      }
      case MessageType::SET: {
        set(req, encoded_msg);
        break;
      }
      case MessageType::GET: {
        get(req, encoded_msg);
        break;
      }
      case MessageType::ADD:    // Pop two integers from operand stack, add them, push sum
      case MessageType::SUB:    // Pop two integers from operand stack, multiply them, push product
      case MessageType::MUL:    // Pop right and left integers from operand stack, subtract right from left, push difference
      case MessageType::DIV:    // Pop right and left integers from operand stack, divide left by right, push quotient
      {
        if(arithmetic(req, encoded_msg)) continue;
        break;
      }
      case MessageType::BEGIN: {
        // TODO
        begin(encoded_msg);
        break;
      }
      case MessageType::COMMIT: {
        commit(encoded_msg);
        break;
      }
      // Client logs out (end of connection)
      case MessageType::BYE: {
        send_ok(encoded_msg);
        return;  // break loop and close
      }
      // Error?
      default: {
        // TODO
        send_error("unrecognized command", encoded_msg);
        break;
      }
    }
  } catch (const FailedTransaction &e) {
      send_failed(e.what(), encoded_msg);
      rollback_transaction();
    }
  }
}

// TODO: additional member functions



void ClientConnection::send_ok(std::string encoded_msg) {
  Message resp(MessageType::OK);
  MessageSerialization::encode(resp, encoded_msg);
  Rio_writen(m_client_fd, encoded_msg.c_str(), encoded_msg.size());
}

void ClientConnection::send_failed(const std::string &what,std::string encoded_msg) {
  Message resp(MessageType::FAILED, {what});
  MessageSerialization::encode(resp, encoded_msg);
  Rio_writen(m_client_fd, encoded_msg.c_str(), encoded_msg.size());
}

void ClientConnection::send_error(const std::string &what, std::string encoded_msg) {
  Message resp(MessageType::ERROR, {what});
  MessageSerialization::encode(resp, encoded_msg);
  Rio_writen(m_client_fd, encoded_msg.c_str(), encoded_msg.size());
}

void ClientConnection::send_data(const std::string &data, std::string encoded_msg) {
  Message resp(MessageType::DATA, {data});
  MessageSerialization::encode(resp, encoded_msg);
  Rio_writen(m_client_fd, encoded_msg.c_str(), encoded_msg.size());
}