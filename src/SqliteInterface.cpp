#include <SqliteInterface.hpp>

void vdb::sql_query(std::string query, std::string table)
{
    int rc = sqlite3_exec(m_db, query.c_str(), NULL, NULL, &m_err);
    if (rc != SQLITE_OK)
    {
        std::cout << "Error: " << m_err << std::endl; // TODO: return via String.
    }
}

    // sqlite3_open("router_db", &db);

    // std::string query = "CREATE TABLE IF NOT EXISTS " + ROUTERS + " (id INT, ip VARCHAR, name VARCHAR, inputs INT, outputs INT)";

    // sql_query(query, ROUTERS);

    // query = "INSERT INTO routers VALUES (0, '10.30.50.70', 'Smart Hub', 40, 40);";

    // sql_query(query, ROUTERS);

    // sqlite3_close(db);