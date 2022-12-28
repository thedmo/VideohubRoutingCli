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

int vdb::sql_query(std::string query) {
  sqlite3_stmt *statement;
  int result;
  int rows = 0;

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
      rows++;
      if (sqlite3_column_type(statement, i) == SQLITE_INTEGER) {
      }
      else if (sqlite3_column_type(statement, i) == SQLITE3_TEXT) {
      }
    }
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);

  if (result != SQLITE_OK) {
    AddToTrace("Error: " + std::string(sqlite3_errmsg(m_db)));
  }

  return rows;
}

int vdb::check_if_exists(std::string ip) {
  int rows;
  std::string query = "SELECT ip FROM " + DEVICES_TABLE + " WHERE ip = '" + ip + "';";

  rows = sql_query(query);

  if (rows == 1) {
    AddToTrace("Device was already in list");
    return 1;
  }
  return SQL_OK;
}

void vdb::insert_device_into_db(std::unique_ptr<device_data> &data) {
  std::string query = "INSERT INTO " + DEVICES_TABLE + "(ip, name, version, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES ('" + data->ip + "','" + data->name + "','" + data->version + "'," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + ",'" + data->source_labels + "','" + data->destination_labels + "','" + data->routing + "','" + data->locks + "');";

  sql_query(query);
}

std::vector<std::string> vdb::m_err_msgs;
void vdb::AddToTrace(std::string s) {
  m_err_msgs.push_back("SQLITE_INTERFACE: " + s);
}

std::vector<std::string> vdb::GetErrorMessages() {
  return m_err_msgs;
}
