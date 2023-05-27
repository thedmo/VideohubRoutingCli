#include <SqliteInterface.hpp>

int vdb::last_row_num;

vdb::vdb() {
  sql_access _sql;

  std::string device_query_str = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + 
                                            " (ip VARCHAR PRIMARY KEY, "+
                                            "name VARCHAR, "+ 
                                            "version VARCHAR, "+ 
                                            "source_count INT, "+ 
                                            "destination_count INT, "+ 
                                            "source_labels VARCHAR, "+
                                            "destination_labels VARCHAR, "+
                                            "routing VARCHAR, "+
                                            "prepared_routes VARCHAR, "+
                                            "locks VARCHAR, "+
                                            "selected_router VARCHAR, "+
                                            "marked_for_saving VARCHAR)";

  _sql.Query(_sql.GetStatement(device_query_str));

  std::string routing_query_str = "CREATE TABLE IF NOT EXISTS " + ROUTINGS_TABLE + 
                                      " (ip VARCHAR, name VARCHAR, "+
                                      "routing VARCHAR, "+
                                      "FOREIGN KEY(ip) REFERENCES " + 
                                      DEVICES_TABLE + "(ip))";

  sqlite3_stmt *routing_table_statement = _sql.GetStatement(routing_query_str);
  _sql.Query(routing_table_statement);
}

vdb::~vdb() {
}

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

int vdb::check_if_device_exists(std::string ip) {
  std::string query_str = "SELECT ip FROM " + DEVICES_TABLE + " WHERE ip =?;";
  std::vector<std::string> bind_values = { ip };

  sql_access _sql;
  int result = _sql.Query(_sql.BindValues(bind_values, _sql.GetStatement(query_str)));

  if (result != SQL_OK) return AddToTrace("Error: " + _sql.GetLastErrorMsg());

  if (_sql.GetLastRowNum() == 1) return AddToTrace("Device with ip: '" + ip + "' already in list");
  return SQL_OK;
}

int vdb::check_if_devicetable_empty() {
  std::string query_str = "SELECT * FROM " + DEVICES_TABLE + ";";

  sql_access _sql;
  int result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("could not check devicetable");

  if (_sql.GetLastRowNum() == 0) return AddToTrace("devices table is empty");

  return SQL_OK;
}

int vdb::insert_device_into_db(std::unique_ptr<device_data> &data) {
  std::string query_str = "INSERT INTO " + DEVICES_TABLE + "(ip, name, version, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES (?,'" + data->name + "','" + data->version + "'," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + ",?,?,'" + data->routing + "','" + data->locks + "');";
  std::vector<std::string> args = { data->ip, data->source_labels, data->destination_labels };

  sql_access _sql;
  int result = _sql.Query(_sql.BindValues(args, _sql.GetStatement(query_str)));

  if (result) {
    AddToTrace("could not insert new device into table");
    return 1;
  }

  result = select_device(data->ip);
  if (result) {
    AddToTrace("Could not select device in table");
    return 1;
  }

  return SQL_OK;
}

int vdb::select_device(std::string ip) {
  int result = check_if_device_exists(ip);
  if (result == 0) return AddToTrace("Device with ip: " + ip + " does not exist in database. Add it first before selecting.");

  std::string query = "UPDATE " + DEVICES_TABLE + " SET selected_router='o';";

  sql_access _sql;
  result = _sql.Query(_sql.GetStatement(query));
  if (result) return AddToTrace(std::to_string(result) + ": could not reset router selection...");

  query = "UPDATE " + DEVICES_TABLE + " SET selected_router='x' WHERE ip=?;";

  std::vector<std::string> args = { ip };
  result = _sql.Query(_sql.BindValues(args, _sql.GetStatement(query)));
  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());

  return SQL_OK;
}

int vdb::remove_selected_device_from_db() {
  int result = check_if_devicetable_empty();
  if (result) return AddToTrace("could not remove device from table, seems to be empty");


  sql_access _sql;
  std::string query_str = "DELETE FROM " + DEVICES_TABLE + " WHERE selected_router='x'";

  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("could not remove device from table");

  return SQL_OK;
}

int vdb::get_data_of_selected_device() {
  int result;
  result = check_if_devicetable_empty();
  if (result) return AddToTrace("Could not get data:");

  std::string query = "SELECT * FROM " + DEVICES_TABLE + " WHERE selected_router='x';";

  sql_access _sql;
  result = _sql.Query(_sql.GetStatement(query));
  if (result) return AddToTrace("no selected device found");

  result = SetLocalDeviceDataNew(_sql.GetLastQueryResult());
  if (result) return AddToTrace("could not set devicedata");

  return SQL_OK;
}

int vdb::add_to_prepared_routes(int destination, int source) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");

  std::string prepared_routes = m_device.prepared_routes;

  std::string new_route = std::to_string(destination) + " " + std::to_string(source) + "\n";

  prepared_routes += new_route;

  std::string query_str = "UPDATE " + DEVICES_TABLE + " SET prepared_routes='" + prepared_routes + "' WHERE selected_router='x';";

  sql_access _sql;
  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("could not add new route to prepared routes");

  return SQL_OK;
}

// TODO remove function, can be made with update_device_data function
// int vdb::clean_prepared_routes() {
//   int result = check_if_devicetable_empty();
//   if (result) return AddToTrace("no devices in router table");

//   std::string query_str = "UPDATE " + DEVICES_TABLE + " SET prepared_routes='' WHERE selected_router='x';";

//   sql_access _sql;
//   result = _sql.Query(_sql.GetStatement(query_str));
//   if (result) return AddToTrace("query did not work");

//   return SQL_OK;
// }

int vdb::mark_route_for_saving(int destination) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");

  std::string marked_routes_str = m_device.marked_for_saving;
  std::string marked_route;

  std::stringstream routes_stream(m_device.routing);
  std::string route_line;
  std::string destination_string = std::to_string(destination);
  const char *destination_char = destination_string.c_str();

  while (std::getline(routes_stream, route_line)) {
    auto line_stream = std::stringstream{ route_line };
    std::string line_destination_string;
    std::getline(line_stream, line_destination_string, ' ');

    if (destination_string == line_destination_string) {
      marked_route = route_line;
      break;
    }
  }

  std::stringstream marked_routes_stream(marked_routes_str);
  std::string marked_line;
  while (std::getline(marked_routes_stream, marked_line)) {
    if (destination_string == marked_line) {
      return AddToTrace("Route already marked");
      break;
    }
  }

  marked_routes_str += marked_route + '\n';

  sql_access _sql;
  std::string query_str = "UPDATE " + DEVICES_TABLE + " SET marked_for_saving=? WHERE selected_router='x';";
  std::vector<std::string> args = {marked_routes_str};

  result = _sql.Query(_sql.BindValues(args, _sql.GetStatement(query_str)));
  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());

  return SQL_OK;
}

// TODO remove function, can be made with update_device_data function
int vdb::clean_marked_routes() {
  int result;
  std::string query_str = "UPDATE " + DEVICES_TABLE + " SET marked_for_saving='' WHERE selected_router='x';";
  sql_access _sql;

  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());

  return SQL_OK;
}

int vdb::get_saved_routing_names(std::string ip, std::vector<std::string> &names) {
  int result;
  std::string query_str = "SELECT * FROM " + ROUTINGS_TABLE + " WHERE ip='" + ip + "';";
  sql_access _sql;
  
  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());
  if (_sql.GetLastQueryResult().size() < 1) return AddToTrace("no routings in list");

  for (unsigned int i = 1; i < _sql.GetLastQueryResult().size(); i++) {
    std::string current_name = _sql.GetLastQueryResult()[i][1];
    names.push_back(current_name);
  }

  return SQL_OK;
}

int vdb::get_saved_routing_names(std::string ip, std::vector<std::string> &names, QueryResult& query_result){
  int result;
  std::string query_str = "SELECT * FROM " + ROUTINGS_TABLE + " WHERE ip='" + ip + "';";
  sql_access _sql;
  
  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());
  if (_sql.GetLastQueryResult().size() < 1) return AddToTrace("no routings in list");

  for (unsigned int i = 1; i < _sql.GetLastQueryResult().size(); i++) {
    std::string current_name = _sql.GetLastQueryResult()[i][1];
    names.push_back(current_name);
  }
  
  query_result = _sql.GetLastQueryResult();  
  
  return SQL_OK;
}


int vdb::save_routing(const std::string name, std::unique_ptr<device_data> &data) {
  int result;

  std::vector<std::string> saved_routing_names;
  result = get_saved_routing_names(data->ip, saved_routing_names);

  for (auto saved_routing_name : saved_routing_names) {
    if (name == saved_routing_name) {
      return AddToTrace("Routing with name: " + name + " already in list of saved routings");
    }
  }

  sql_access _sql;
  std::string query_str = "INSERT INTO " + ROUTINGS_TABLE + "(ip, name, routing) VALUES (?,?,?);";
  std::vector<std::string> args = {data->ip, name, data->marked_for_saving};
  result = _sql.Query(_sql.BindValues(args, _sql.GetStatement(query_str)));

  if (result) return AddToTrace("Error: " + _sql.GetLastErrorMsg());

  result = clean_marked_routes();
  if (result) return AddToTrace("Could not clean marked routes: ");

  return SQL_OK;
}


int vdb::get_routing_by_name(const std::string name, std::string &routes_str) {
  int result;
  result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get routing names: ");
  QueryResult query_result;

  std::vector<std::string> saved_routing_names;
  result = get_saved_routing_names(m_device.ip, saved_routing_names, query_result);
  if (result) return AddToTrace("could not get routings list: ");

  for (size_t i = 1; i < query_result.size(); i++) {
    if (m_last_query_result[i][1] == name) {
      routes_str = m_last_query_result[i][2];
      return SQL_OK;
    }
  }

  return AddToTrace("routing with name: " + name + " does not exist for selected device.");
}

int vdb::update_selected_device_data(std::unique_ptr<device_data> &data) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data of selected device");

  std::string query_str = "UPDATE " + DEVICES_TABLE + " SET version=?, "+
                                                            "source_labels=?, "+
                                                            "destination_labels=?, "+
                                                            "routing='" + data->routing +
                                                            "', locks = '" + data->locks +
                                                            "prepared_routes"+
                                                            "marked_for_saving"+ 
                                                            "' WHERE selected_router='x';";
                                                            
  sql_access _sql;
  std::vector<std::string> args = {
    data->version, data->source_labels, 
    data->destination_labels, 
    data->prepared_routes, 
    data->marked_for_saving
    };

  result = _sql.Query(_sql.BindValues(args, _sql.GetStatement(query_str)));

  if (result) return AddToTrace("Query did not work: " + _sql.GetLastErrorMsg());

  return SQL_OK;
}

int vdb::GetSelectedDeviceData(std::unique_ptr<device_data> &device) {
  int result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data from selected device");

  *device = m_device;
  return SQL_OK;
}

int vdb::GetDevices(std::string &device_str) {
  
  int result;
  std::string query_str = "SELECT ip, name, version FROM " + DEVICES_TABLE + ";";
  sql_access _sql;

  result = _sql.Query(_sql.GetStatement(query_str));
  if (result) return AddToTrace("could not get Devices List");

  // Compose response string
  for (auto row : _sql.GetLastQueryResult()) {
    const auto line = std::format("| {0:<20} | {1:<20} | {2:<20} |\n", row[0], row[1], row[2]);
    device_str += line;
  }

  return SQL_OK;
}

int vdb::get_saved_routings(std::string &routings_str) {
  int result;

  // Get data of selected device
  result = get_data_of_selected_device();
  if (result) return AddToTrace("could not get data from selected device");

  // query for saved routings of selected device
  std::string ip_str = m_device.ip;
  std::string query = std::format("SELECT * FROM {0} WHERE ip='{1}';", ROUTINGS_TABLE, ip_str);
  sql_access _sql;
  
  result = _sql.Query(_sql.GetStatement(query));
  if (result) return AddToTrace(std::format("query did not work: {}", _sql.GetLastErrorMsg()));

  // Compose response string
  for (size_t i = 1; i < _sql.GetLastQueryResult().size(); i++) {
    auto row = m_last_query_result[i];
    const auto line = std::format("IP: {0}\nName: {1}\nRouting:\n{2}\n", row[0], row[1], row[2]);
    routings_str += line;
    std::cout << line << std::endl;
  }

  return SQL_OK;
}










// Error Handling
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














// SQL_ACCESS

int sql_access::last_row_num;

sql_access::sql_access() {
  std::string path = whereami::getExecutablePath().dirname();
  path = path + "/router.db";
  sqlite3_open(path.c_str(), &m_db);
}

sql_access::~sql_access() {
  sqlite3_close_v2(m_db);
}

sqlite3_stmt *sql_access::GetStatement(std::string query) {
  sqlite3_stmt *statement;
  sqlite3_prepare_v2(m_db, query.c_str(), -1, &statement, 0);
  return statement;
}

int sql_access::Query(sqlite3_stmt *statement) {
  m_last_query_result.clear();
  int result;
  last_row_num = 0;
  std::vector<std::string> row_content;

  do {
    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
      break;
    }

    // column names
    if (m_last_query_result.size() == 0) {
      row_content.clear();
      for (int i = 0; i < sqlite3_data_count(statement); i++) {
        const char *col_name = (const char *)sqlite3_column_name(statement, i);
        std::string column(col_name);

        row_content.push_back(column);
      }
      m_last_query_result.push_back(row_content);
    }

    row_content.clear();
    // Fields
    for (int j = 0; j < sqlite3_data_count(statement); j++) {
      auto field = (const char *)(sqlite3_column_text(statement, j)) ? (const char *)(sqlite3_column_text(statement, j)) : "";

      row_content.push_back(field);
    }

    m_last_query_result.push_back(row_content);
    last_row_num++;
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);
  if (result != SQLITE_OK) return 1;

  return SQL_ACCESS_OK;
}


sqlite3_stmt *sql_access::BindValues(std::vector<std::string> args, sqlite3_stmt *statement) {
  for (size_t i = 0; i < args.size(); i++) {
    sqlite3_bind_text(statement, i + 1, args[i].c_str(), -1, SQLITE_TRANSIENT);
  }

  return statement;
}


QueryResult sql_access::GetLastQueryResult() {
  return m_last_query_result;
}

int sql_access::GetLastRowNum() {
  return last_row_num;
}

std::string sql_access::GetLastErrorMsg(){
  return sqlite3_errmsg(m_db);
}