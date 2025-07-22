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

    switch (req.get_message_type()) {
      // Client logs in
      case MessageType::LOGIN: {
        // Always ok?
        send_ok(encoded_msg);
        break;
      }
      //Ask server to create named table
      case MessageType::CREATE: {
        std::string name = req.get_table();
        m_server->create_table(name);
        send_ok(encoded_msg);
        break;
      }
      // Push a value onto the operand stack
      case MessageType::PUSH: {
        if (!m_has_stack) {
          send_failed("no active key", encoded_msg);
        } else {
          m_stack.push(req.get_value());
          send_ok(encoded_msg);
        }
        break;
      }
      //Pop (discard) the top value from the operand stack
      case MessageType::POP: {
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
        break;
      }
      //	Retrieve the top value from the operand stack
      case MessageType::TOP: {
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
        break;
      }
      // Set value of tuple named by key in table to the value popped from the operand stack
      case MessageType::SET: {
        if (!m_has_stack) {
          send_failed("no active key", encoded_msg);
        } else {
          Table *t = m_server->find_table(m_table);
          assert(t);
          std::string final_val = m_stack.get_top();
          t->set(m_key, final_val);
          send_ok(encoded_msg);
        }
        break;
      }
      // Push value of tuple named by key in table onto the operand stack
      case MessageType::GET: {
        std::string table = req.get_table();
        std::string key   = req.get_key();
        Table *t = m_server->find_table(table);
        if (!t) {
          send_failed("no such table",encoded_msg);
        } else if (!t->has_key(key)) {
          send_failed("no such key", encoded_msg);
        } else {
          // load into working stack
          std::string val = t->get(key);
          m_stack = ValueStack();             // reset stack
          m_stack.push(val);
          m_table = table;
          m_key   = key;
          m_has_stack = true;
          send_ok(encoded_msg);
        }
        break;
      }
      case MessageType::ADD:    // Pop two integers from operand stack, add them, push sum
      case MessageType::SUB:    // Pop two integers from operand stack, multiply them, push product
      case MessageType::MUL:    // Pop right and left integers from operand stack, subtract right from left, push difference
      case MessageType::DIV:    // Pop right and left integers from operand stack, divide left by right, push quotient
      {
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
                if (a == 0) { send_failed("divide by zero", encoded_msg); continue; }
                result = b / a;
                break;
              default: result = 0; break;
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
        break;
      }
      // Client begins a transaction
      case MessageType::BEGIN: {
        // TODO
        send_ok(encoded_msg);
        break;
      }
      // Client commits a transaction
      case MessageType::COMMIT: {
        // TODO
        send_ok(encoded_msg);
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