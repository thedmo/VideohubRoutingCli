#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <whereami++.h>
#include <iostream>

struct device_data {
  std::string ip;
  std::string name;
  std::string version ;
  int source_count;
  int destination_count;
  std::string source_labels;
  std::string destination_labels;
  std::string routing;
  std::string prepared_routes;
  std::string locks;
};

class vdb {
public:
  static const int SQL_OK = 0;


  vdb();
  ~vdb();

  int check_if_exists(std::string ip);
  int insert_device_into_db(std::unique_ptr<device_data> &data);
  int select_device(std::string ip);
  int remove_selected_device_from_db();


  static std::vector<std::string> GetErrorMessages();

private:


  const std::string DEVICES_TABLE = "routers";
  const std::string ROUTINGS_TABLE = "routings";

  char *m_err;
  sqlite3 *m_db;

  int sql_query(std::string query, int &rows);
  int sql_query(std::string query);

  static std::vector<std::string> m_err_msgs;
  void AddToTrace(std::string s);
};
