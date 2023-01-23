#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <whereami++.h>
#include <iostream>
#include <sstream>

struct device_data {
  std::string ip;
  std::string name;
  std::string version;
  int source_count;
  int destination_count;
  std::string source_labels;
  std::string destination_labels;
  std::string routing;
  std::string prepared_routes;
  std::string locks;
  std::string marked_for_saving;
};

class vdb {
public:
  static const int SQL_OK = 0;

  vdb();
  ~vdb();


  int check_if_device_exists(std::string ip);
  int check_if_devicetable_empty();
  int insert_device_into_db(std::unique_ptr<device_data> &data);
  int update_selected_device_data(std::unique_ptr<device_data> &data);

  int select_device(std::string ip);
  int remove_selected_device_from_db();
  int add_to_prepared_routes(int destination, int source);
  int clean_prepared_routes();
  int mark_route_for_saving(int destination);
  int clean_marked_routes();
  int save_routing(const std::string name, std::unique_ptr<device_data> &data);
  int get_routing_by_name(const std::string name, std::string &routes_str);

  static std::vector<std::string> GetErrorMessages();

  int GetSelectedDeviceData(std::unique_ptr<device_data> &device);
  int GetDevices(std::string &device_str);

private:

  const std::string DEVICES_TABLE = "routers";
  const std::string ROUTINGS_TABLE = "routings";

  char *m_err;
  sqlite3 *m_db;
  device_data m_device;
  static int last_row_num;
  std::vector<std::vector<std::string>> m_last_query_result;

  int SetLocalDeviceData(device_data &device, sqlite3_stmt *statement);
  int SetLocalDeviceDataNew(const std::vector<std::vector<std::string>> &query_result);

  int get_saved_routing_names(std::string ip, std::vector<std::string> &names);

  int sql_query(std::string query);
  sqlite3_stmt *GetStatement(std::string query);
  int sql_query(sqlite3_stmt *statement);

  static std::vector<std::string> m_err_msgs;
  int AddToTrace(std::string s);

  int get_data_of_selected_device();
};
