#include <cassert>
#include "table.h"
#include "exceptions.h"
#include "guard.h"

Table::Table(){
  
}

Table::Table( const std::string &name )
  : m_name( name )
  // TODO: initialize additional member variables
{
  // TODO: implement
  pthread_mutex_init(&m_lock, nullptr);
}

Table::~Table()
{
  // TODO: implement
  pthread_mutex_destroy(&m_lock);
}

void Table::lock()
{
  // TODO: implement
  pthread_mutex_lock(&m_lock);
}

void Table::unlock()
{
  // TODO: implement
  pthread_mutex_unlock(&m_lock);
}

bool Table::trylock()
{
  // TODO: implement
  return pthread_mutex_trylock(&m_lock) == 0;
}

void Table::set( const std::string &key, const std::string &value )
{
  // TODO: implement
  
  m_temp_data[key] = value;
 
}

std::string Table::get( const std::string &key )
{
  // TODO: implement
  
  auto it = m_temp_data.find(key);
  if (it != m_temp_data.end()) {
    return it->second;
  }

  it = m_data.find(key);
  if (it != m_data.end()) {
    return it->second;
  }

  
  throw OperationException("Key not found: " + key);
}

bool Table::has_key( const std::string &key )
{
  // TODO: implement
  return ((m_temp_data.count(key) > 0 ) || (m_data.count(key) > 0));
}

void Table::commit_changes()
{
  // TODO: implement
  for (const auto &pair : m_temp_data) {
    m_data[pair.first] = pair.second;
  }
  m_temp_data.clear();
}

void Table::rollback_changes()
{
  // TODO: implement
  m_temp_data.clear();
}
