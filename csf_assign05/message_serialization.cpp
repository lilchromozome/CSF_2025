#include <utility>
#include <sstream>
#include <cassert>
#include <map>
#include "exceptions.h"
#include "message_serialization.h"

void MessageSerialization::encode( const Message &msg, std::string &encoded_msg )
{
  // TODO: implement
  std::ostringstream oss;

  MessageType type = msg.get_message_type();
  switch (type) {
    case MessageType::LOGIN:
      oss << "LOGIN"; 
      break;
    case MessageType::CREATE:
      oss << "CREATE";
      break;
    case MessageType::PUSH:
      oss << "PUSH";
      break;
    case MessageType::POP:
      oss <<"POP";
      break;
    case MessageType::TOP:
      oss << "TOP";
      break;
    case MessageType::SET:
      oss << "SET";
      break;
    case MessageType::GET:
      oss << "GET";
      break;
    case MessageType::ADD:
      oss << "ADD";
      break;
    case MessageType::SUB:
      oss << "SUB";
      break;
    case MessageType::MUL:
      oss << "MUL";
      break;
    case MessageType::DIV:
      oss << "DIV";
      break;
    case MessageType::BEGIN:
      oss << "BEGIN";
      break;
    case MessageType::COMMIT:
      oss << "COMMIT";
      break;
    case MessageType::BYE:
      oss << "BYE";
      break;
    case MessageType::OK:
      oss << "OK";
      break;
    case MessageType::FAILED:
      oss << "FAILED";
      break;
    case MessageType::ERROR:
      oss << "ERROR";
      break;
    case MessageType::DATA:
      oss << "DATA";
      break;
    default:
      encoded_msg = "";
      return;
  }

  for (unsigned i = 0; i < msg.get_num_args(); ++i) {
    oss << ' ';
    std::string arg = msg.get_arg(i);
    if((type == MessageType::FAILED || type == MessageType::ERROR || type == MessageType::DATA) && i == 0){
      oss << "\"" << arg << "\"";
    } else {
      oss << arg;
    }
  }
  oss << "\n";
  encoded_msg = oss.str();
}

void MessageSerialization::decode( const std::string &encoded_msg_, Message &msg )
{
  // TODO: implement
  std::string encoded_msg = encoded_msg_;
  if (!encoded_msg.empty() && encoded_msg.back() == '\n') {
    encoded_msg.pop_back(); // Remove trailing newline
  }
  std::istringstream iss(encoded_msg);
  std::string token;
  std::vector<std::string> tokens;

  // if token starts with "", it is a quoted string
  while (iss >> token) {
    if (token[0] == '"') {
      // Handle quoted strings
      std::string quoted_token = token;
      while (!quoted_token.empty() && quoted_token.back() != '"') {
        std::string next_token;
        if (!(iss >> next_token)) {
          break;
        }
        quoted_token += ' ' + next_token;
      }
      quoted_token = quoted_token.substr(1, quoted_token.size() - 2); // Remove quotes
      tokens.push_back(quoted_token);
    } else {
      tokens.push_back(token);
    }
  }

  static const std::map<std::string, MessageType> type_map = {
    {"LOGIN", MessageType::LOGIN},
    {"CREATE", MessageType::CREATE},
    {"PUSH", MessageType::PUSH},
    {"POP", MessageType::POP},
    {"TOP", MessageType::TOP},
    {"SET", MessageType::SET},
    {"GET", MessageType::GET},
    {"ADD", MessageType::ADD},
    {"SUB", MessageType::SUB},
    {"MUL", MessageType::MUL},
    {"DIV", MessageType::DIV},
    {"BEGIN", MessageType::BEGIN},
    {"COMMIT", MessageType::COMMIT},
    {"BYE", MessageType::BYE},
    {"OK", MessageType::OK},
    {"FAILED", MessageType::FAILED},
    {"ERROR", MessageType::ERROR},
    {"DATA", MessageType::DATA}
  };

  if (tokens.empty())
  {
    throw InvalidMessage("Empty message received");
  }

  auto it = type_map.find(tokens[0]);
  if (it == type_map.end())
  {
    throw InvalidMessage("Unknown message type: " + tokens[0]);
  }
  msg.set_message_type(it->second);
  for (size_t i = 1; i < tokens.size(); ++i)
  {
    msg.push_arg(tokens[i]);
  }
}
