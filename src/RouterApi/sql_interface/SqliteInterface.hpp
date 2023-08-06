#ifndef SqliteInterface
#define SqliteInterface


// #include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <sstream>
#include <format>
#include <DeviceData.hpp>
#include <ErrorTrace.hpp>
#include <sql_access.hpp>



// Aliases
using QueryResult = std::vector<std::vector<std::string>>;
using namespace sqlAccess;

// SQL Handler
class vdb {
public:
	static const int SQL_OK = 0;

	vdb();
	~vdb();

	int check_if_device_exists(std::string ip);
	int check_if_devicetable_empty();

	int insert_device_into_db(std::unique_ptr<device_data>& data);

	int select_device(std::string ip);
	int update_selected_device_data(std::unique_ptr<device_data>& data);
	int remove_selected_device_from_db();

	int add_to_prepared_routes(int destination, int source);
	int mark_route_for_saving(int destination);
	int save_routing(const std::string name, std::unique_ptr<device_data>& data);

	int [[deprecated("use update_device_data instead")]] clean_marked_routes();

	int GetSelectedDeviceData(std::unique_ptr<device_data>& device);
	int GetDevices(std::string& device_str);
	int get_routing_by_name(const std::string name, std::string& routes_str);
	int get_saved_routings(std::string& routings_str);

	static std::vector<std::string> GetErrorMessages();

private:

	const std::string DEVICES_TABLE = "routers";
	const std::string ROUTINGS_TABLE = "routings";

	char* m_err;
	device_data m_device;

	std::shared_ptr<sql_access> _sql;

	int SetLocalDeviceData(device_data& device, sqlite3_stmt* statement);
	int SetLocalDeviceDataNew(const std::vector<std::vector<std::string>>& query_result);

	int get_saved_routing_names(std::string ip, std::vector<std::string>& names);
	int get_saved_routing_names(std::string ip, std::vector<std::string>& names, QueryResult& query_result);

	int [[deprecated("use new function instead to mitigate sql injection")]] sql_query(std::string query);

	int get_data_of_selected_device();
};

#endif // !SqliteInterface