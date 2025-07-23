#include <iostream>
#include <cassert>
#include <memory>
#include "csapp.h"
#include "exceptions.h"
#include "guard.h"
#include "server.h"

Server::Server()
  // TODO: initialize member variables
{
  // TODO: implement
  pthread_mutex_init(&tables_mutex, nullptr);
}

Server::~Server()
{
  // TODO: implement
  pthread_mutex_destroy(&tables_mutex);
}

void Server::listen( const std::string &port )
{
  // TODO: implement
  listen_fd = Open_listenfd(port.c_str());
}

void Server::create_table(const std::string &name) {
  Guard g(tables_mutex);
  if (tables.count(name) == 0)
    tables[name] = std::make_unique<Table>(); 
  //Only for test
    tables[name]->set("default_key", "0");
}

Table *Server::find_table(const std::string &name) {
  Guard g(tables_mutex);
  auto it = tables.find(name);
  if (it == tables.end()) return nullptr;
  return it->second.get(); 
}


void Server::server_loop()
{
  // TODO: implement

  // Note that your code to start a worker thread for a newly-connected
  // client might look something like this:
/*
  ClientConnection *client = new ClientConnection( this, client_fd );
  pthread_t thr_id;
  if ( pthread_create( &thr_id, nullptr, client_worker, client ) != 0 )
    log_error( "Could not create client thread" );
*/
  while (true) {
    struct sockaddr_storage clientaddr;
    socklen_t clientlen = sizeof(clientaddr);
    int client_fd = Accept(listen_fd, (SA*)&clientaddr, &clientlen);

    ClientConnection *client = new ClientConnection(this, client_fd);
    pthread_t thr_id;
    if (pthread_create(&thr_id, nullptr, Server::client_worker, client) != 0) {
      log_error("Could not create client thread");
      delete client;
    } else {
      pthread_detach(thr_id);
    }
  }
}


void *Server::client_worker( void *arg )
{
  // TODO: implement

  // Assuming that your ClientConnection class has a member function
  // called chat_with_client(), your implementation might look something
  // like this:
/*
  std::unique_ptr<ClientConnection> client( static_cast<ClientConnection *>( arg ) );
  client->chat_with_client();
  return nullptr;
*/

  std::unique_ptr<ClientConnection> client(static_cast<ClientConnection *>(arg));
  client->chat_with_client();  
  return nullptr;
}

void Server::log_error( const std::string &what )
{
  std::cerr << "Error: " << what << "\n";
}

// TODO: implement member functions

