#include <iostream>
#include <set>
#include <map>
#include <regex>
#include <cassert>
#include "message.h"

Message::Message()
  : m_message_type(MessageType::NONE)
{
}

Message::Message( MessageType message_type, std::initializer_list<std::string> args )
  : m_message_type( message_type )
  , m_args( args )
{
}

Message::Message( const Message &other )
  : m_message_type( other.m_message_type )
  , m_args( other.m_args )
{
}

Message::~Message()
{
}

Message &Message::operator=( const Message &rhs )
{
  // TODO: implement
  if (this != &rhs) {
    m_message_type = rhs.m_message_type;
    m_args = rhs.m_args;
  }
  return *this;
}

MessageType Message::get_message_type() const
{
  return m_message_type;
}

void Message::set_message_type(MessageType message_type)
{
  m_message_type = message_type;
}

std::string Message::get_username() const
{
  // TODO: implement
  if(m_message_type == MessageType::LOGIN && m_args.size()==1){
    return m_args[0];
  }
  return "";
}

std::string Message::get_table() const
{
  // TODO: implement
  if ((m_message_type == MessageType::CREATE && m_args.size() == 1)
    || ((m_message_type == MessageType::SET || m_message_type == MessageType::GET) && m_args.size() == 2)){
      return m_args[0];
  }
  return "";
}

std::string Message::get_key() const
{
  // TODO: implement
  if ((m_message_type == MessageType::SET || m_message_type == MessageType::GET) && m_args.size() >= 2){
    return m_args[1];
  }
  return "";
}

std::string Message::get_value() const
{
  // TODO: implement
  if ((m_message_type == MessageType::PUSH || m_message_type == MessageType::DATA) && m_args.size() == 1){
    return m_args[0];
  }
  return "";
}

std::string Message::get_quoted_text() const
{
  // TODO: implement
  if ((m_message_type == MessageType::FAILED || m_message_type == MessageType::ERROR) && m_args.size() == 1){
    return m_args[0];
  }
  return "";
}

void Message::push_arg( const std::string &arg )
{
  m_args.push_back( arg );
}

// helper function to check if a string is a valid identifier
bool Message::is_valid_identifier(const std::string &identifier){
  if (identifier.empty()) {
    return false;
  }

  // An identifier must start with letter or _
  if (!std::isalpha(identifier[0]) && identifier[0] != '_') {
    return false;
  }

  // All the parts of the identifier must be alphanumeric or _
  for (size_t i = 1; i < identifier.size(); ++i) {
    if (!std::isalnum(identifier[i]) && identifier[i] != '_') {
      return false;
    }
  }
  return true;
}

bool Message::is_valid() const
{
  // TODO: implement
  switch (m_message_type) {
    // 1 args
    case MessageType::LOGIN:
    case MessageType::PUSH:
    case MessageType::FAILED:
    case MessageType::ERROR:
    case MessageType::DATA:
      return m_args.size()== 1;

    case MessageType::CREATE:
      return m_args.size() == 1 && is_valid_identifier(m_args[0]);

    // 2 args
    case MessageType::SET:
    case MessageType::GET:
      return m_args.size() == 2 && is_valid_identifier(m_args[0]) && is_valid_identifier(m_args[1]);    

    // 0 args
    case MessageType::POP:
    case MessageType::TOP:
    case MessageType::ADD:
    case MessageType::MUL:
    case MessageType::SUB:
    case MessageType::DIV:
    case MessageType::BEGIN:
    case MessageType::COMMIT:
    case MessageType::BYE:
    case MessageType::OK:
      return m_args.empty();

    case MessageType::NONE:
    default:
      return false;
  }
}

void Message::print_message() {
  std::string type_str;
  switch (m_message_type) {
    case MessageType::NONE:    type_str = "NONE"; break;
    case MessageType::LOGIN:   type_str = "LOGIN"; break;
    case MessageType::CREATE:  type_str = "CREATE"; break;
    case MessageType::PUSH:    type_str = "PUSH"; break;
    case MessageType::POP:     type_str = "POP"; break;
    case MessageType::TOP:     type_str = "TOP"; break;
    case MessageType::SET:     type_str = "SET"; break;
    case MessageType::GET:     type_str = "GET"; break;
    case MessageType::ADD:     type_str = "ADD"; break;
    case MessageType::SUB:     type_str = "SUB"; break;
    case MessageType::MUL:     type_str = "MUL"; break;
    case MessageType::DIV:     type_str = "DIV"; break;
    case MessageType::BEGIN:   type_str = "BEGIN"; break;
    case MessageType::COMMIT:  type_str = "COMMIT"; break;
    case MessageType::BYE:     type_str = "BYE"; break;
    case MessageType::OK:      type_str = "OK"; break;
    case MessageType::FAILED:  type_str = "FAILED"; break;
    case MessageType::ERROR:   type_str = "ERROR"; break;
    case MessageType::DATA:    type_str = "DATA"; break;
    default:                   type_str = "UNKNOWN"; break;
  }

  // Print the type and arguments
  std::cout << type_str;
  for (const auto &arg : m_args) {
    std::cout << " " << arg << " ";
  }
  std::cout << std::endl;
}
