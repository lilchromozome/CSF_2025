#ifndef TABLE_H
#define TABLE_H

#include <map>
#include <string>
#include <pthread.h>

class Table {
private:
  std::string m_name;
  // TODO: add member variables
  pthread_mutex_t m_lock;
  std::map<std::string, std::string> m_data; // Key-value pairs
  std::map<std::string, std::string> m_temp_data; // Temporary changes
  // copy constructor and assignment operator are prohibited
  Table( const Table & ) = delete;
  Table &operator=( const Table & ) = delete;

public:
  Table( const std::string &name );
  ~Table();

  std::string get_name() const { return m_name; }

  void lock();
  void unlock();
  bool trylock();

  // Note: these functions should only be called while the
  // table's lock is held!
  void set( const std::string &key, const std::string &value );
  bool has_key( const std::string &key );
  std::string get( const std::string &key );
  void commit_changes();
  void rollback_changes();
};

#endif // TABLE_H
