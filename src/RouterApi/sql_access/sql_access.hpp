#ifndef SQL_ACCESS_HPP
#define SQL_ACCESS_HPP


#include <string>
#include <vector>
#include <whereami++.h>
#include <sqlite3.h>

namespace sqlAccess {
	// Aliases
	using QueryResult = std::vector<std::vector<std::string>>;

	// Database Communication
	class sql_access
	{
	public:
		static const int SQL_ACCESS_OK = 0;

		sql_access(std::string db_name);
		sql_access() {};
		~sql_access();

		sqlite3_stmt* GetStatement(std::string query);
		sqlite3_stmt* BindValues(std::vector<std::string> args, sqlite3_stmt* statement);
		int Query(sqlite3_stmt* statement);

		std::string GetLastErrorMsg();
		QueryResult GetLastQueryResult();
		int GetLastRowNum();
		int GetRowCount();

	private:
		std::string m_db_name;
		sqlite3* m_db;
		QueryResult m_last_query_result;
		static int last_row_num;
		int rowCount;
	};
} // namespace sqlAccess

#endif /* SQL_ACCESS_HPP */
