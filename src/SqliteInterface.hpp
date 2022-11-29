#include <sqlite3.h>

#include <iostream>
#include <string>

struct device_data {
  std::string ip;
  std::string name;
  int source_count;
  int destination_count;
  std::string source_labels;
  std::string destination_labels;
  std::string routing;
  std::string prepared_routes;
  std::string locks;
};

// TODO: Remove iostream
class vdb {
 public:
  vdb();
  ~vdb();

  bool check_if_exists(std::string ip);
  //   std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + " (ip VARCHAR PRIMARY KEY, name VARCHAR, source_count INT, destination_count INT, source_labels VARCHAR, destination_labels VARCHAR, routing VARCHAR, prepared_routes VARCHAR, locks VARCHAR, selected_router VARCHAR)";

  void insert_device_into_db(device_data*);

 private:
  const std::string DEVICES_TABLE = "routers";
  const std::string ROUTINGS_TABLE = "routings";

  char *m_err;
  sqlite3 *m_db;

  int sql_query(std::string query);
  //   int callback(void *NotUsed, int argc, char **argv, char **azColName);
};
