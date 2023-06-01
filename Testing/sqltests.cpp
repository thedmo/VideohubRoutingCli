#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <filesystem>

#include <SqliteInterface.hpp>
#include <sql_access.hpp>
#include <device_data.hpp>


//// sql_access Functions


TEST_CASE("SqliteInterface constructor creates database file", "[sql_access]") {

  std::shared_ptr<sql_access> db = std::make_shared<sql_access>("test");

  REQUIRE(std::filesystem::exists("test.db"));
}

// Unit test für GetStatement Function
TEST_CASE("GetStatement returns a valid sqlite3_stmt pointer", "[GetStatement]") {
  std::string DEVICES_TABLE = "routers";
  sql_access db("test");
  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + 
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
  sqlite3_stmt *statement = db.GetStatement(query);
  REQUIRE(statement != nullptr);
}


// Unit test for GetLastQuerqResult() Function
TEST_CASE("GetLastQueryResult returns a valid QueryResult", "[GetLastQueryResult]") {
  std::string DEVICES_TABLE = "routers";
  sql_access db("test");
  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + 
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
  sqlite3_stmt *statement = db.GetStatement(query);
  db.Query(statement);
  REQUIRE(db.GetLastQueryResult().size() == 0);
}


// Unit test for GetLastRowNum() Function
TEST_CASE("GetLastRowNum returns a valid row number", "[GetLastRowNum]") {
  std::string DEVICES_TABLE = "routers";
  sql_access db("test");
  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + 
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
  sqlite3_stmt *statement = db.GetStatement(query);
  db.Query(statement);
  REQUIRE(db.GetLastRowNum() == 0);
}


// Unit test for GetLastErrorMsg() Function
TEST_CASE("GetLastErrorMsg returns a valid error message", "[GetLastErrorMsg]") {
  std::string DEVICES_TABLE = "routers";
  sql_access db("test");
  std::string query = "SELECT * FROM " + DEVICES_TABLE + " WHERE ip = 127.0.0.1";
  sqlite3_stmt *statement = db.GetStatement(query);
  db.Query(statement);

  std::cout << db.GetLastErrorMsg() << std::endl;

  REQUIRE(db.GetLastErrorMsg() == "near \".0\": syntax error");
}


// Unit test for BindValues() Function
TEST_CASE("BindValues returns a valid sqlite3_stmt pointer", "[BindValues]") {
  std::string DEVICES_TABLE = "routers";
  sql_access db("test");
  std::string query = "CREATE TABLE IF NOT EXISTS " + DEVICES_TABLE + 
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
  sqlite3_stmt *statement = db.GetStatement(query);
  db.Query(statement);

  std::shared_ptr<device_data> data = std::make_shared<device_data>();
  data->ip = "127.0.0.1";
  data->name = "test";
  data->version = "1.0";
  data->source_count = 1;
  data->destination_count = 1;
  data->source_labels = "test";
  data->destination_labels = "test";
  data->routing = "test";
  data->prepared_routes = "test";
  data->locks = "test";
  data->marked_for_saving = "test";

  // Create sql statement fo add a new device to the database
  std::string query_str = "INSERT INTO " + DEVICES_TABLE + "(ip, name, version, source_count, destination_count, source_labels, destination_labels, routing, locks) VALUES (?,'" + data->name + "','" + data->version + "'," + std::to_string(data->source_count) + "," + std::to_string(data->destination_count) + ",?,?,'" + data->routing + "','" + data->locks + "');";
  sqlite3_stmt *bind_statement = db.GetStatement(query_str);

  // Create Vector with values to bind
  std::vector<std::string> args = { data->ip, data->source_labels, data->destination_labels };

  // Bind values to statement
  db.BindValues(args, bind_statement);
  REQUIRE(bind_statement != nullptr);
}

//// SqliteInterface Functions

// Unit test for GetDevices Function
TEST_CASE("GetDevices returns 0, if not, database or Devices table missing or general sql error", "[GetDevices]") {
  vdb db;
  std::string device_str;
  REQUIRE(db.GetDevices(device_str) == 0);
}


