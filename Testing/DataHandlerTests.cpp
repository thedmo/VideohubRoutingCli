#include <catch2/catch.hpp>
#include <DataHandler.hpp>
#include <device_data.hpp>

class DHTestValues {
public:
	static inline std::string
		ip1 = "127.0.0.1",
		ip2 = "127.0.0.2",
		ip3 = "127.0.0.3",

		name1 = "Smart1",
		name2 = "Smart2",
		name3 = "Smart3",

		version1 = "v1.1",
		version2 = "v1.2",
		version3 = "v1.3";

	static inline int
		sourceCount1 = 20,
		sourceCount2 = 40,
		sourceCount3 = 60,

		destinationCount1 = 20,
		destinationCount2 = 40,
		destinationCount3 = 60;

	static inline std::vector<std::string>
		sourceLabelsList1 = { "one, two, three" },
		sourceLabelsList2 = { "four, five, six" },
		sourceLabelsList3 = { "seven, eight, nine" },

		destinationsLabelsList1 = { "one, two, three" },
		destinationsLabelsList2 = { "four, five, six" },
		destinationsLabelsList3 = { "seven, eight, nine" },

		locksList1 = { "lone", "ltwo", "lthree" },
		locksList2 = { "lone", "ltwo", "lthree" },
		locksList3 = { "lone", "ltwo", "lthree" };

	static inline std::vector<std::pair<int, int>>
		routesMarkedList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesMarkedList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesMarkedList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} },

		routesPreparedList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesPreparedList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesPreparedList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} },

		routesList1 = { std::pair{1,2}, std::pair{3,4}, std::pair{5,6      } },
		routesList2 = { std::pair{7,8}, std::pair{9,10}, std::pair{11,12   } },
		routesList3 = { std::pair{13,14}, std::pair{15,16}, std::pair{17,18} };


	static inline std::unique_ptr<device_data> device1 = std::make_unique<device_data>();
	static inline std::unique_ptr<device_data> device2 = std::make_unique<device_data>();
	static inline std::unique_ptr<device_data> device3 = std::make_unique<device_data>();

	static void InitializeDevices() {
		device1->ip = ip1;
		device1->name = name1;
		device1->version = version1;
		device1->source_count = sourceCount1;
		device1->destination_count = destinationCount1;
		device1->sourceLabelsList = sourceLabelsList1;
		device1->destinationsLabelsList = destinationsLabelsList1;
		device1->locksList = locksList1;
		device1->routesMarkedList = routesMarkedList1;
		device1->routesPreparedList = routesPreparedList1;
		device1->routesList = routesList1;
		
		device2->ip = ip2;
		device2->name = name2;
		device2->version = version2;
		device2->source_count = sourceCount2;
		device2->destination_count = destinationCount2;
		device2->sourceLabelsList = sourceLabelsList2;
		device2->destinationsLabelsList = destinationsLabelsList2;
		device2->locksList = locksList2;
		device2->routesMarkedList = routesMarkedList2;
		device2->routesPreparedList = routesPreparedList2;
		device2->routesList = routesList2;
				
		device3->ip = ip3;
		device3->name = name3;
		device3->version = version3;
		device3->source_count = sourceCount3;
		device3->destination_count = destinationCount3;
		device3->sourceLabelsList = sourceLabelsList3;
		device3->destinationsLabelsList = destinationsLabelsList3;
		device3->locksList = locksList3;
		device3->routesMarkedList = routesMarkedList3;
		device3->routesPreparedList = routesPreparedList3;
		device3->routesList = routesList3;
	}
};

bool isInitialized = false;

void Initialize() {
	if (isInitialized) {
		return;
	}
	isInitialized = true;

	DHTestValues::InitializeDevices();
	DataHandler::InitializeStorage();
}

void DropTablesDataHandler() {
	int result = 0;

	result = SqlCom::ConnectToDatabase(DataHandler::DB_NAME);

	std::string queryStr = "DROP TABLE IF EXISTS "+ DataHandler::DEVICES_TABLE +";";
	auto statement = SqlCom::GetStatement(queryStr, result);
	result = SqlCom::Query(statement);

	queryStr = "DROP TABLE IF EXISTS "+ DataHandler::ROUTINGS_TABLE +";";
	statement = SqlCom::GetStatement(queryStr, result);
	result = SqlCom::Query(statement);

	result = SqlCom::CloseConnection();
}

void RemoveDevicesFromTable(std::unique_ptr<device_data> &device) {
	DataHandler::RemoveDevice(device);
}

TEST_CASE("Add New Devices to database") {
	Initialize();

	RemoveDevicesFromTable(DHTestValues::device1);
	RemoveDevicesFromTable(DHTestValues::device2);
	RemoveDevicesFromTable(DHTestValues::device3);

	REQUIRE(DataHandler::AddDevice(DHTestValues::device1) == 0);
	REQUIRE(DataHandler::AddDevice(DHTestValues::device2) == 0);
	REQUIRE(DataHandler::AddDevice(DHTestValues::device3) == 0);
}

TEST_CASE("Add already existing device to storage") {
	REQUIRE(DataHandler::AddDevice(DHTestValues::device1) == 1);
}