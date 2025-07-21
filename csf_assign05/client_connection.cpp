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
  // TODO: implement
  Close(m_client_fd);
}

void ClientConnection::chat_with_client()
{
  // TODO: implement

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