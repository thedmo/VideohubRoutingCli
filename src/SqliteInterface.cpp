#include <SqliteInterface.hpp>

vdb::vdb() {

  std::string path = whereami::getExecutablePath().dirname();

  path = path + "/router.db";

  sqlite3_open(path.c_str(), &m_db);

  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + " (ip VARCHAR PRIMARY KEY, name VARCHAR, version VARCHAR, source_count INT, destination_count INT, source_labels VARCHAR, destination_labels VARCHAR, routing VARCHAR, prepared_routes VARCHAR, locks VARCHAR, selected_router VARCHAR)";
  sql_query(query);

  query = "CREATE TABLE IF NOT EXISTS " + ROUTINGS_TABLE + " (ip VARCHAR, name VARCHAR, routing VARCHAR, FOREIGN KEY(ip) REFERENCES " + DEVICES_TABLE + "(ip))";
  sql_query(query);
}

vdb::~vdb() {
  sqlite3_close_v2(m_db);
}

// TODO: secure against sql injection with bind function --> change passed argument type to statement or create new overloaded function
int vdb::sql_query(std::string query, int &rows) {
  sqlite3_stmt *statement;
  int result;
  rows = 0;

  result = sqlite3_prepare_v2(m_db, query.c_str(), -1, &statement, nullptr);
  if (result != SQLITE_OK) {
    AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
  }

  do {
    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
      break;
    }
    for (auto i = 0; i < sqlite3_data_count(statement); i++) {
      const char *col_name = (const char *)sqlite3_column_name(statement, i);
      std::string column(col_name);
      int column_value_int = sqlite3_column_int(statement, i) ? sqlite3_column_int(statement, i) : 0;
      const char *col_val = (const char *)sqlite3_column_text(statement, i) ? (char *)sqlite3_column_text(statement, i) : "";
      std::string column_value_str(col_val);
      rows++;

      if (column == "source_count") {
        m_device.source_count = column_value_int;
      }
      else if (column == "destination_count") {
        m_device.destination_count = column_value_int;
      }
      if (column == "ip") {
        m_device.ip = column_value_str;
      }
      else if (column == "name") {
        m_device.name = column_value_str;
      }
      else if (column == "version") {
        m_device.version = column_value_str;
      }
      else if (column == "destination_labels") {
        m_device.destination_labels = column_value_str;
      }
      else if (column == "source_labels") {
        m_device.source_labels = column_value_str;
      }
      else if (column == "routing") {
        m_device.routing = column_value_str;
      }
      else if (column == "prepared_routes") {
        m_device.prepared_routes = column_value_str;
      }
      else if (column == "locks") {
        m_device.locks = column_value_str;
      }
    }
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);

  if (result != SQLITE_OK) {
    AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
    return 1;
  }

  return SQL_OK;
}

// TODO secure againtst sql injection with bind function
int vdb::sql_query(std::string query) {

  int result = sqlite3_exec(m_db, query.c_str(), 0, 0, nullptr);
  if (result != SQLITE_OK) {
    AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
    return 1;
  }

  return SQL_OK;
}

int vdb::check_if_exists(std::string ip) {
  int rows;
  std::string query = "SELECT ip FROM " + DEVICES_TABLE + " WHERE ip = '" + ip + "';";

  int result = sql_query(query, rows);
  if (result != SQL_OK)
  {
    AddToTrace("could not add device to list.");
    return 1;
  }


  if (rows == 1) {
    AddToTrace("Device found in list");
    return 1;
  }
  return SQL_OK;
}

int vdb::insert_device_into_db(std::unique_ptr<device_data> &data) {
  std::string query = "INSERT INTO " + DEVICES_TABLE + "(ip, name, version, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES ('" + data->ip + "','" + data->name + "','" + data->version + "'," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + ",'" + data->source_labels + "','" + data->destination_labels + "','" + data->routing + "','" + data->locks + "');";

  int result = sql_query(query);
  if (result != SQL_OK)
  {
    AddToTrace("could not insert new device into table");
    return 1;
  }

  result = select_device(data->ip);
  if (result != SQL_OK)
  {
    AddToTrace("Could not select device in table");
    return 1;
  }

  return SQL_OK;
}

int vdb::select_device(std::string ip) {
  int result = check_if_exists(ip);
  if (result == 0)
  {
    AddToTrace("Device with ip: " + ip + " does not exist in database. Add it first before selecting.");
    return 1;
  }

// deselect currently selected router
  std::string query = "UPDATE " + DEVICES_TABLE + " SET selected_router='o';";
  result = sql_query(query);
  if (result != SQL_OK)
  {
    AddToTrace(std::to_string(result) + ": could not reset router selection...");
    return 1;
  }

  query = "UPDATE " + DEVICES_TABLE + " SET selected_router='x' WHERE ip='" + ip + "';";
  result = sql_query(query);
  if (result != SQL_OK)
  {
    AddToTrace("could not select router...");
    return 1;
  }

  return SQL_OK;
}

int vdb::remove_selected_device_from_db() {
  std::string query = "DELETE FROM " + DEVICES_TABLE + " WHERE selected_router='x'";

  int result = sql_query(query);
  if (result != SQL_OK)
  {
    AddToTrace("could not remove device from table");
    return 1;
  }


  return SQL_OK;
}

int vdb::get_data_of_selected_device() {
  std::string query = "SELECT * FROM " + DEVICES_TABLE + " WHERE selected_router='x';";

  int rows = 0;

  int result = sql_query(query, rows);
  if (result != SQL_OK)
  {
    AddToTrace("no selected device found");
    return 1;
  }
  return SQL_OK;
}

// TODO Add public function to reset prepared routes

int vdb::add_to_prepared_routes(int destination, int source) {

  int result = get_data_of_selected_device();
  if (result != SQL_OK) {
    AddToTrace("could not get data of selected device");
    return 1;
  }

  std::string prepared_routes = m_device.prepared_routes;

  std::string new_route = std::to_string(destination) + " " + std::to_string(source) + "\n";

  prepared_routes += new_route;

  std::string query = "UPDATE " + DEVICES_TABLE + " SET prepared_routes='" + prepared_routes + "' WHERE selected_router='x';";
  result = sql_query(query);
  if (result != SQL_OK)
  {
    AddToTrace("could not add new route to prepared routes");
    return 1;
  }

  return SQL_OK;
}

// TODO Add function to update values of selected router

// int vdb::get_prepared_routes(std::string &routes) {
//   int result = get_data_of_selected_device();
//   if (result != SQL_OK) {
//     AddToTrace("could not get data of selected device");
//     return 1;
//   }

//   routes = m_device.prepared_routes;
//   return SQL_OK;
// }

int vdb::GetSelectedDeviceData(std::unique_ptr<device_data> &device){
  int result = get_data_of_selected_device();
  if(result) return AddToTrace("could not get data from selected device");
  
  *device = m_device;
  return SQL_OK;
}

std::vector<std::string> vdb::m_err_msgs;
int vdb::AddToTrace(std::string s) {
  m_err_msgs.push_back("SQLITE_INTERFACE: " + s);
  return 1;
}

std::vector<std::string> vdb::GetErrorMessages() {
  return m_err_msgs;
}
