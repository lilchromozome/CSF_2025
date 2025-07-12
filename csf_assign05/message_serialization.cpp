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
}
