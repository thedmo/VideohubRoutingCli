#include <sql_access.hpp>

// SQL_ACCESS
int sql_access::last_row_num;

sql_access::sql_access(std::string db_name) {
  m_db_name = db_name;

  std::string path = whereami::getExecutablePath().dirname();
  path = path + "/" + m_db_name+ ".db";
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