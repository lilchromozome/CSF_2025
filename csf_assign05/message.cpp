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

bool Message::is_valid() const
{
  // TODO: implement
  switch (m_message_type) {
    // 1 args
    case MessageType::LOGIN:
    case MessageType::PUSH:
    case MessageType::CREATE:
    case MessageType::FAILED:
    case MessageType::ERROR:
    case MessageType::DATA:
      return m_args.size()==1;

    // 2 args
    case MessageType::SET:
    case MessageType::GET:
      return m_args.size() == 2;

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
  }
  return false;
}
