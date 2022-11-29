#include <SqliteInterface.hpp>

vdb::vdb() {
  sqlite3_open("router_db", &m_db);

  std::cout << "Creating devices table" << std::endl;
  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + " (ip VARCHAR PRIMARY KEY, name VARCHAR, source_count INT, destination_count INT, source_labels VARCHAR, destination_labels VARCHAR, routing VARCHAR, prepared_routes VARCHAR, locks VARCHAR, selected_router VARCHAR)";
  sql_query(query);

  std::cout << "Creating routings table" << std::endl;
  query = "CREATE TABLE IF NOT EXISTS " + ROUTINGS_TABLE + " (ip VARCHAR, name VARCHAR, routing VARCHAR, FOREIGN KEY(ip) REFERENCES " + DEVICES_TABLE + "(ip))";
  sql_query(query);
}

vdb::~vdb() {
  sqlite3_close_v2(m_db);
}

int vdb::sql_query(std::string query) {
  sqlite3_stmt* statement;
  int result;
  int rows = 0;

  result = sqlite3_prepare_v2(m_db, query.c_str(), -1, &statement, nullptr);
  if (result != SQLITE_OK) {
    std::cout << "Error: " << sqlite3_errmsg(m_db) << std::endl;
  }

  do {
    std::cout << "stepping..." << std::endl;
    result = sqlite3_step(statement);

    if (result != SQLITE_ROW) {
      break;
    }
    for (auto i = 0; i < sqlite3_data_count(statement); i++) {
      rows++;
      if (sqlite3_column_type(statement, i) == SQLITE_INTEGER) {
        std::cout << "got an Integer" << std::endl;
      } else if (sqlite3_column_type(statement, i) == SQLITE3_TEXT) {
        std::cout << "got some Text" << std::endl;
      }
    }
  } while (result != SQLITE_DONE);

  result = sqlite3_finalize(statement);

  if (result != SQLITE_OK) {
    std::cout << sqlite3_errmsg(m_db) << std::endl;
  }

  return rows;
}

bool vdb::check_if_exists(std::string ip) {
  int rows;
  std::string query = "SELECT ip FROM " + DEVICES_TABLE + ";";

  rows = sql_query(query);

  if (rows == 0) {
    return false;
    std::cout << "no devices in List" << std::endl;
  }
  return true;
}

void vdb::insert_device_into_db(device_data* data) {
  std::cout << "inserting new device" << std::endl;

  std::string query = "INSERT INTO " + DEVICES_TABLE + "(ip, name, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES (" + data->ip + "," + data->name + "," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + "," + data->source_labels + "," + data->destination_labels + "," + data->routing + "," + data->locks + ")";
  sql_query(query);
}

// sql_query(query, ROUTERS);

// query = "INSERT INTO routers VALUES (0, '10.30.50.70', 'Smart Hub', 40, 40);";

// sql_query(query, ROUTERS);
