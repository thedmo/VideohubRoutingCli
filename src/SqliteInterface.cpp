#include <SqliteInterface.hpp>

int vdb::last_row_num;

vdb::vdb() {
  std::string path = whereami::getExecutablePath().dirname();

  path = path + "/router.db";

  sqlite3_open(path.c_str(), &m_db);

  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + " (ip VARCHAR PRIMARY KEY, name VARCHAR, version VARCHAR, source_count INT, destination_count INT, source_labels VARCHAR, destination_labels VARCHAR, routing VARCHAR, prepared_routes VARCHAR, locks VARCHAR, selected_router VARCHAR, marked_for_saving VARCHAR)";
  sql_query(query);

  query = "CREATE TABLE IF NOT EXISTS " + ROUTINGS_TABLE + " (ip VARCHAR, name VARCHAR, routing VARCHAR, FOREIGN KEY(ip) REFERENCES " + DEVICES_TABLE + "(ip))";
  sql_query(query);
}

vdb::~vdb() {
  sqlite3_close_v2(m_db);
}

// // Todo: replace with method to extract from Vector
// int vdb::SetLocalDeviceData(device_data &device, sqlite3_stmt *statement) {
//   for (auto i = 0; i < sqlite3_data_count(statement); i++) {
//     const char *col_name = (const char *)sqlite3_column_name(statement, i);
//     std::string column(col_name);

//     int column_value_int = sqlite3_column_int(statement, i) ? sqlite3_column_int(statement, i) : 0;

//     const char *col_val = (const char *)sqlite3_column_text(statement, i) ? (char *)sqlite3_column_text(statement, i) : "";
//     std::string column_value_str(col_val);

//     last_row_num++;

//     if (column == "source_count") {
//       device.source_count = column_value_int;
//     }
//     else if (column == "destination_count") {
//       device.destination_count = column_value_int;
//     }
//     if (column == "ip") {
//       device.ip = column_value_str;
//     }
//     else if (column == "name") {
//       device.name = column_value_str;
//     }
//     else if (column == "version") {
//       device.version = column_value_str;
//     }
//     else if (column == "destination_labels") {
//       device.destination_labels = column_value_str;
//     }
//     else if (column == "source_labels") {
//       device.source_labels = column_value_str;
//     }
//     else if (column == "routing") {
//       device.routing = column_value_str;
//     }
//     else if (column == "prepared_routes") {
//       device.prepared_routes = column_value_str;
//     }
//     else if (column == "locks") {
//       device.locks = column_value_str;
//     }
//   }

//   return SQL_OK;
// }

int vdb::SetLocalDeviceDataNew(const std::vector<std::vector<std::string>> &query_result) {
  // iterating through cols
  for (size_t i = 0; i < query_result[0].size(); i++) {

    std::string column = query_result[0][i];
    std::string field = query_result[1][i];

    if (column == "source_count") {
      m_device.source_count = std::stoi(field);
    }
    else if (column == "destination_count") {
      m_device.destination_count = std::stoi(field);
    }
    if (column == "ip") {
      m_device.ip = field;
    }
    else if (column == "name") {
      m_device.name = field;
    }
    else if (column == "version") {
      m_device.version = field;
    }
    else if (column == "destination_labels") {
      m_device.destination_labels = field;
    }
    else if (column == "source_labels") {
      m_device.source_labels = field;
    }
    else if (column == "routing") {
      m_device.routing = field;
    }
    else if (column == "prepared_routes") {
      m_device.prepared_routes = field;
    }
    else if (column == "locks") {
      m_device.locks = field;
    }
    else if (column == "marked_for_saving") {
      m_device.marked_for_saving = field;
    }
  }

  return SQL_OK;
}

// DEPRECATED do not use anymore
int vdb::sql_query(std::string query) {
  sqlite3_stmt *statement;
  int result;
  last_row_num = 0;

  result = sqlite3_prepare_v2(m_db, query.c_str(), -1, &statement, nullptr);
  if (result != SQLITE_OK) {
    return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
  }

  do {
    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
      break;
    }
    last_row_num++;

    // result = SetLocalDeviceData(m_device, statement);
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);

  if (result != SQLITE_OK) {
    AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
    return 1;
  }

  return SQL_OK;
}

sqlite3_stmt *vdb::GetStatement(std::string query) {
  sqlite3_stmt *statement;
  sqlite3_prepare_v2(m_db, query.c_str(), -1, &statement, 0);
  return statement;
}

int vdb::sql_query(sqlite3_stmt *statement) {
  m_last_query_result.clear();
  int result;
  last_row_num = 0;
  std::vector<std::string> row_content;

  do {
    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
      break;
    }

    //column names
    if (m_last_query_result.size() == 0)
    {
      row_content.clear();
      for (int i = 0; i < sqlite3_data_count(statement); i++)
      {
        const char *col_name = (const char *)sqlite3_column_name(statement, i);
        std::string column(col_name);

        row_content.push_back(column);
      }
      m_last_query_result.push_back(row_content);
    }

    row_content.clear();
    //Fields
    for (int j = 0; j < sqlite3_data_count(statement); j++) {
      char *field = (char *)(sqlite3_column_text(statement, j)) ? (char *)(sqlite3_column_text(statement, j)) : "";

      row_content.push_back(field);
    }


    m_last_query_result.push_back(row_content);
    last_row_num++;
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);

  if (result != SQLITE_OK) return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));

  return SQL_OK;
}

int vdb::check_if_device_exists(std::string ip) {
  sqlite3_stmt *statement = GetStatement("SELECT ip FROM " + DEVICES_TABLE + " WHERE ip =?;");

  int result = sqlite3_bind_text(statement, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
  if (result != SQL_OK) return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));

  result = sql_query(statement);
  if (result != SQL_OK) return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));

  if (last_row_num == 1) return AddToTrace("Device with ip: '" + ip + "' already in list");
  return SQL_OK;
}

int vdb::check_if_devicetable_empty() {
  std::string query = "SELECT * FROM " + DEVICES_TABLE + ";";
  int result = sql_query(query);
  if (result) return AddToTrace("could not check devicetable");

  if (last_row_num == 0) return AddToTrace("devices table is empty");

  return SQL_OK;
}

int vdb::insert_device_into_db(std::unique_ptr<device_data> &data) {
  std::string query = "INSERT INTO " + DEVICES_TABLE + "(ip, name, version, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES (?,'" + data->name + "','" + data->version + "'," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + ",?,?,'" + data->routing + "','" + data->locks + "');";
  sqlite3_stmt *statement = GetStatement(query);
  sqlite3_bind_text(statement, 1, data->ip.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, data->source_labels.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 3, data->destination_labels.c_str(), -1, SQLITE_TRANSIENT);
  int result = sql_query(statement);

  if (result != SQL_OK) {
    AddToTrace("could not insert new device into table");
    return 1;
  }

  result = select_device(data->ip);
  if (result != SQL_OK) {
    AddToTrace("Could not select device in table");
    return 1;
  }

  return SQL_OK;
}

int vdb::select_device(std::string ip) {
  int result = check_if_device_exists(ip);
  if (result == 0) return AddToTrace("Device with ip: " + ip + " does not exist in database. Add it first before selecting.");

  std::string query = "UPDATE " + DEVICES_TABLE + " SET selected_router='o';";
  sqlite3_stmt *statement = GetStatement(query);
  result = sql_query(statement);
  if (result) return AddToTrace(std::to_string(result) + ": could not reset router selection...");

  query = "UPDATE " + DEVICES_TABLE + " SET selected_router='x' WHERE ip=?;";
  statement = GetStatement(query);
  result = sqlite3_bind_text(statement, 1, ip.c_str(), -1, SQLITE_TRANSIENT);
  if (result) return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));

  result = sql_query(statement);
  if (result) return AddToTrace("could not select router...");

  return SQL_OK;
}

int vdb::remove_selected_device_from_db() {
  int result = check_if_devicetable_empty();
  if (result) return AddToTrace("could not remove device from table, seems to be empty");

  std::string query = "DELETE FROM " + DEVICES_TABLE + " WHERE selected_router='x'";

  result = sql_query(query);
  if (result) return AddToTrace("could not remove device from table");

  return SQL_OK;
}

int vdb::get_data_of_selected_device() {
  std::string query = "SELECT * FROM " + DEVICES_TABLE + " WHERE selected_router='x';";
  sqlite3_stmt *statement = GetStatement(query);
  int result = sql_query(statement);
  if (result) return AddToTrace("no selected device found");

  result = SetLocalDeviceDataNew(m_last_query_result);
  if (result) return AddToTrace("could not set devicedata");

  return SQL_OK;
}

int vdb::add_to_prepared_routes(int destination, int source) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");

  std::string prepared_routes = m_device.prepared_routes;

  std::string new_route = std::to_string(destination) + " " + std::to_string(source) + "\n";

  prepared_routes += new_route;

  std::string query = "UPDATE " + DEVICES_TABLE + " SET prepared_routes='" + prepared_routes + "' WHERE selected_router='x';";
  result = sql_query(query);
  if (result) return AddToTrace("could not add new route to prepared routes");

  return SQL_OK;
}

int vdb::clean_prepared_routes() {
  int result = check_if_devicetable_empty();
  if (result) return AddToTrace("no devices in router table");

  sqlite3_stmt *statement = GetStatement("UPDATE " + DEVICES_TABLE + " SET prepared_routes='' WHERE selected_router='x';");
  result = sql_query(statement);
  if (result) return AddToTrace("query did not work");

  return SQL_OK;
}

int vdb::mark_route_for_saving(int destination){
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");
  
  // TODO hier weiter
  sqlite3_stmt *statement = GetStatement("");

  return SQL_OK;
}

int vdb::update_selected_device_data(std::unique_ptr<device_data> &data) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");

  sqlite3_stmt *statement = GetStatement("UPDATE " + DEVICES_TABLE + " SET version=?, source_labels=?, destination_labels=?, routing='" + data->routing + "', locks = '" + data->locks + "';");
  sqlite3_bind_text(statement, 1, data->version.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 2, data->source_labels.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(statement, 3, data->destination_labels.c_str(), -1, SQLITE_TRANSIENT);
  if (result != SQLITE_OK) return AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));

  result = sql_query(statement);
  if (result) return AddToTrace("Query did not work: " + std::string(sqlite3_errmsg(m_db)));

  return SQL_OK;
}

int vdb::GetSelectedDeviceData(std::unique_ptr<device_data> &device) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data from selected device");

  *device = m_device;
  return SQL_OK;
}

int vdb::GetDevices(std::string &device_str) {
  sqlite3_stmt *statement = GetStatement("SELECT ip, name, version FROM " + DEVICES_TABLE + ";");
  int result = sql_query(statement);
  if (result) return AddToTrace("could not get Devices List");

  for (auto row : m_last_query_result) {
    for (auto field : row) {
      device_str += field + " ";
    }

    device_str += '\n';
  }

  return SQL_OK;
}

std::vector<std::string> vdb::m_err_msgs;
int vdb::AddToTrace(std::string s) {
  m_err_msgs.push_back("SQLITE_INTERFACE: " + s);
  return 1;
}

std::vector<std::string> vdb::GetErrorMessages() {
  std::vector<std::string> temp = m_err_msgs;
  m_err_msgs.clear();
  return temp;
}
