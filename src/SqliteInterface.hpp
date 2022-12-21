#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>

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

class vdb {
public:
  static const int SQL_OK = 0;


  vdb();
  ~vdb();

  int check_if_exists(std::string ip);
  //   std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + " (ip VARCHAR PRIMARY KEY, name VARCHAR, source_count INT, destination_count INT, source_labels VARCHAR, destination_labels VARCHAR, routing VARCHAR, prepared_routes VARCHAR, locks VARCHAR, selected_router VARCHAR)";

  void insert_device_into_db(std::unique_ptr<device_data> &data);

  static std::vector<std::string> GetErrorMessages();

private:


  const std::string DEVICES_TABLE = "routers";
  const std::string ROUTINGS_TABLE = "routings";

  char *m_err;
  sqlite3 *m_db;

  int sql_query(std::string query);

  static std::vector<std::string> m_err_msgs;
  void AddToTrace(std::string s);
};
