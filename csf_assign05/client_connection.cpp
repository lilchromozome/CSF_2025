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


static bool is_valid_identifier(const std::string &u){
  if (u.empty()) {
    return false;
  }
  // An identifier must start with letter or _
  if (!std::isalpha(u[0]) && u[0] != '_') {
    return false;
  }

  // All the parts of the identifier must be alphanumeric or _
  for (size_t i = 1; i < u.size(); ++i) {
    if (!std::isalnum(u[i]) && u[i] != '_') {
      return false;
    }
  }
  return true;
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
      //std::cout << e.what() << std::endl;
      send_failed("stack empty", encoded_msg);
    }
  }
  return;
}

// Set value of tuple named by key in table to the value popped from the operand stack
void ClientConnection::set(const Message &req, std::string encoded_msg){
  std::string table = req.get_table();
  std::string key = req.get_key();

  if (!is_valid_identifier(key)){
    //throw OperationException("invalid key name");
    send_failed("invalid key name", encoded_msg);
    return;
  }

  Table *t = m_server->find_table(table);

  if (!m_has_stack) {
    send_failed("no active key", encoded_msg);
    return;
  } 
  if (!t) {
    send_failed("no such table", encoded_msg);
    return;
  }

  try {
    std::string val = m_stack.get_top();
    m_stack.pop();

    if (m_in_transaction) {
      // first-time lock if needed
      if (m_locked_tables.insert(t).second) {
        if (!t->trylock()) {
          rollback_transaction();
          throw FailedTransaction("could not lock table");
        }
      }
      // store in transaction buffer only
      m_transaction_buffer[table][key] = val;
    } else {
      t->lock();
      t->set(key, val);
      t->commit_changes();
      t->unlock();
    }

    m_table = table;
    m_key = key;
    m_has_stack = true;
    send_ok(encoded_msg);
  } catch (const std::exception &e) {
    if (!m_in_transaction) {
      t->unlock();
    }
    send_failed("set failed: " + std::string(e.what()), encoded_msg);
  }
}

// Push value of tuple named by key in table onto the operand stack
void ClientConnection::get(Message req, std::string encoded_msg){
  std::string table = req.get_table();
  std::string key   = req.get_key();
  //std::cout << "get " << table << ", " << key << std::endl;
  Table *t = m_server->find_table(table);

  if (!t) {
    send_failed("no such table",encoded_msg);
    return;
  } 
  if (m_in_transaction) {
    // std::cout << m_in_transaction << " in transaction" << std::endl;
    if (m_locked_tables.insert(t).second){
      if (!t->trylock()) {
        rollback_transaction();
        throw FailedTransaction("could not lock table");
      }
    } //else{
      //t->lock();
    //}
  }

  try {
    std::string val;

    if (m_in_transaction && m_transaction_buffer.count(table) && m_transaction_buffer[table].count(key)) {
      val = m_transaction_buffer[table][key];
    } else {
      if (!t->has_key(key)) {
        throw OperationException("no such key");
      }
      val = t->get(key);
    }

    m_stack.push(val);
    m_table = table;
    m_key   = key;
    m_has_stack = true;

    if (!m_in_transaction) {
      t->unlock();
    }
    send_ok(encoded_msg);
  } catch (const OperationException &e) {
    // std::cout << e.what() << std::endl;
    if (m_in_transaction) {
      rollback_transaction();                        // rollback if inside transaction
      throw FailedTransaction("no such key");        // MUST throw
    } else {
      t->unlock();                                   // just unlock if no transaction
      send_failed(e.what(), encoded_msg);
    }
    return;
  }
}

// Pop two integers from operand stack, add them, push sum
// Pop two integers from operand stack, multiply them, push product
// Pop right and left integers from operand stack, subtract right from left, push difference
// Pop right and left integers from operand stack, divide left by right, push quotient
void ClientConnection::arithmetic(Message req, std::string encoded_msg){
  if(m_stack.size() < 2){
    throw OperationException("not enough operands");
    return;
  }
  if (m_stack.is_empty()||m_stack.get_top() == "") {
    send_failed("stack empty", encoded_msg);
    return;
  }

  std::string s1 = m_stack.get_top();
  m_stack.pop();

  if (m_stack.is_empty() || m_stack.get_top() == "") {
    m_stack.push(s1);
    send_failed("Insufficient values for operation", encoded_msg);
    return;
  }

  std::string s2 = m_stack.get_top();
  m_stack.pop();

  int a,b;
  size_t pos1, pos2;

  try {
    a = std::stoi(s1, &pos1);
    if (pos1 != s1.size() || s1.find('.') != std::string::npos) {
      m_stack.push(s2);
      m_stack.push(s1);
      send_error("non-integer operand", encoded_msg);
      return;
    }
  } catch (const std::invalid_argument &) {
    m_stack.push(s2);
    m_stack.push(s1);
    throw OperationException("non-integer operand");
    return;
  } catch (const std::out_of_range &) {
    m_stack.push(s2);
    m_stack.push(s1);
    throw OperationException("integer overflow");
    return;
  }

  try {
    b = std::stoi(s2, &pos2);
    if (pos2 != s2.size() || s2.find('.') != std::string::npos) {
      m_stack.push(s1);
      m_stack.push(s2);
      send_error("non-integer operand", encoded_msg);
      return;
    }
  } catch (const std::invalid_argument &) {
    m_stack.push(s1);
    m_stack.push(s2);
    throw OperationException("non-integer operand");
    return;
  } catch (const std::out_of_range &) {
    m_stack.push(s2);
    m_stack.push(s1);
    throw OperationException("integer overflow");
    return;
  }

  if (req.get_message_type() == MessageType::DIV && b == 0) {
    m_stack.push(s1);
    m_stack.push(s2);
    send_error("Division by zero is not allowed", encoded_msg);
    return;
  }

  int result = 0;
  switch (req.get_message_type()) {
    case MessageType::ADD: result = b + a; break;
    case MessageType::SUB: result = b - a; break;
    case MessageType::MUL: result = b * a; break;
    case MessageType::DIV:
      result = b / a;
      break;
    default: 
      m_stack.push(s2);
      m_stack.push(s1);
      throw InvalidMessage("unexpected operation");
      return;
  }
  m_stack.push(std::to_string(result));
  send_ok(encoded_msg);
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
  if (!m_in_transaction) {
    send_failed("not in transaction", encoded_msg);
    return;
  }

  try {
    // Apply all buffered writes
    for (const auto &[table_name, keyval_map] : m_transaction_buffer) {
      Table *t = m_server->find_table(table_name);
      if (!t) continue; // Table might have been deleted
      for (const auto &[key, val] : keyval_map) {
        t->set(key, val);
      }
    }

    // Commit and unlock all locked tables
    for (Table *t : m_locked_tables) {
      t->commit_changes();
    }
    for (Table *t : m_locked_tables) {
      t->unlock();
    }

    m_transaction_buffer.clear();
    m_locked_tables.clear();
    m_in_transaction = false;

    send_ok(encoded_msg);
  } catch (const std::exception &e) {
    rollback_transaction();
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
    try{
    if (Rio_readlineb(&m_fdbuf, buf, sizeof(buf)) <= 0)
      throw CommException("I/O error"); // client closed
      // break;

    Message req;
    MessageSerialization::decode(buf, req);

    if(!m_has_logged_in && req.get_message_type() != MessageType::LOGIN){
      throw InvalidMessage("first operation must be LOGIN");
      // send_error("first operation must be LOGIN", encoded_msg);
      return;
    }
    // if (!req.is_valid()) {
    //   throw InvalidMessage("invalid request");
    //   return;
    // }

    req.print_message();

    switch (req.get_message_type()) {
      // Client logs in
      case MessageType::LOGIN: {
        // Always ok?
        const std::string &user = req.get_username();
        if (!is_valid_identifier(user))
          throw InvalidMessage("invalid username");
        m_has_logged_in = true;
      
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
        arithmetic(req, encoded_msg);
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
        throw InvalidMessage("invalid request");
        break;
      }
    }
  }
  catch(const InvalidMessage &e){
    send_error(e.what(), encoded_msg);
    //break; //end session
    return;
  } 
  catch(const CommException &e){
    //break; //drop client
    return;
  } 
  catch(const OperationException &e){
    send_failed(e.what(), encoded_msg);
    if(m_in_transaction) rollback_transaction();
  }
  catch(const FailedTransaction &e) {
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