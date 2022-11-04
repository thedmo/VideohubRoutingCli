#include <sqlite3.h>
#include <string>
#include <iostream>

// TODO: Remove iostream
class vdb
{
public:
    vdb(){};
    ~vdb(){};



private:
    const std::string ROUTERS = "routers";
    const std::string CHANNELS = "channels";

    char *m_err;
    sqlite3 *m_db;
    void sql_query(std::string query, std::string table);
};
