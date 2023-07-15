#include <sql_access_handler.hpp>
#include <device_data.hpp>

class DataHandler : private SqliteHandler::DataGetter, private SqliteHandler::DataSetter, private SqliteHandler::DatabaseChanger {

	using StringList = std::vector<std::string>;
	using IntList = std::vector<int>;
	using IntPairList = std::vector<std::pair<int, int>>;
	using StringMatrix = std::vector<std::vector<std::string>>;
	using IntMatrix = std::vector<std::vector<int>>;
	using IntPairMatrix = std::vector<std::vector<std::pair<int, int>>>;

	const static inline std::string 
		DB_NAME = "RouterDb",
		DEVICES_TABLE = "routers",
		ROUTINGS_TABLE = "routings";

	const static inline std::string
		COLUMN_IP = "ip",
		COLUMN_SELECTED = "selected_device",
		COLUMN_NAME = "name",
		COLUMN_VERSION = "version",
		COLUMN_SOURCE_COUNT = "source_count",
		COLUMN_SOURCE_LABELS = "source_labels",
		COLUMN_DESTINATION_COUNT = "destination_count",
		COLUMN_DESTINATION_LABELS = "destination_labels",
		COLUMN_ROUTES = "routes",
		COLUMN_PREPARED = "prepared_routes",
		COLUMN_LOCKS = "locks",
		COLUMN_MARKED = "marked_for_saving";

	// checks for empty data
	template <typename T>
	static bool Empty(T object) {
		return false;
	}

	template <>
	static bool Empty<std::string>(std::string object) {
		return object.empty();
	}

	template <>
	static bool Empty<int>(int object) {
		if (object == 0)
		{
			return true;
		}
		return false;
	}

	template <typename T>
	static bool Empty(std::vector<T> object) {
		return object.empty();
	}

	template <typename Storage>
	static int LoadFromSelected(const std::string column, int& result, Storage& dataStorage) {
		result = SqliteHandler::DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_PREPARED, COLUMN_SELECTED, "x", dataStorage);

		return 0;
	}

	template <typename Storage>
	static int StoreInSelected(
		const std::string column, 
		Storage& dataStorage) {

		// return normally if empty
		if (Empty(dataStorage)) {
			return 0;
		}

		return SqliteHandler::DataSetter::StoreData(
			DB_NAME, 
			DEVICES_TABLE, 
			column, 
			dataStorage, 
			COLUMN_SELECTED, 
			"x"
		);
	}

public:
	static int InitializeStorage() {
		int result;

		// Create Devicedatabase if not exists
		result = SqliteHandler::DatabaseChanger::CreateTableWithPrimaryKey(DB_NAME, DEVICES_TABLE, COLUMN_IP, SqliteHandler::DatabaseChanger::V_CHAR);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, SqliteHandler::DatabaseChanger::V_CHAR);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_NAME, SqliteHandler::DatabaseChanger::V_CHAR);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_VERSION, SqliteHandler::DatabaseChanger::V_CHAR);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_COUNT, SqliteHandler::DatabaseChanger::INT);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_DESTINATION_COUNT, SqliteHandler::DatabaseChanger::INT);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_LABELS, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_DESTINATION_LABELS, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_ROUTES, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_PREPARED, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_LOCKS, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_MARKED, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;

		// create routingdatabase if not exists
		result = SqliteHandler::DatabaseChanger::CreateTableWithForeignKey(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, SqliteHandler::DatabaseChanger::V_CHAR, DEVICES_TABLE, COLUMN_IP);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, ROUTINGS_TABLE, COLUMN_NAME, SqliteHandler::DatabaseChanger::V_CHAR);
		if (result) return 1;
		result = SqliteHandler::DatabaseChanger::AddColumn(DB_NAME, ROUTINGS_TABLE, COLUMN_ROUTES, SqliteHandler::DatabaseChanger::BLOB);
		if (result) return 1;

		return 0;
	}

	// add device with device data
	static int AddDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = SqliteHandler::DataSetter::InsertRow(DB_NAME, DEVICES_TABLE, COLUMN_IP, deviceData->ip);

		if (result) {
			return 1;
		}

		return 0;
	}

	// remove device and its routing Data
	static int RemoveDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = SqliteHandler::DataSetter::RemoveRow(DB_NAME, DEVICES_TABLE, COLUMN_IP, deviceData->ip);
		if (result)
		{
			return 1;
		}

		result = SqliteHandler::DataSetter::RemoveRow(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, deviceData->ip);
		if (result)	return 1;

		return 0;
	}

	// get list of devices ip, name and channelcount
	static std::vector<std::unique_ptr<device_data>> GetEntries(int& result) {
		std::vector<std::unique_ptr<device_data>> deviceDataVector;

		StringList ipList;
		StringList nameList;
		IntList sourceCountList;

		result = SqliteHandler::DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_IP, ipList);
		result = SqliteHandler::DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_NAME, nameList);
		result = SqliteHandler::DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_COUNT, sourceCountList);

		for (unsigned int i = 0; i > ipList.size(); i++) {
			deviceDataVector.push_back(std::make_unique<device_data>());
			deviceDataVector[i]->ip = ipList[i];
			deviceDataVector[i]->name = nameList[i];
			deviceDataVector[i]->source_count = sourceCountList[i];
		}

		return deviceDataVector;
	}

	// select device by ip
	static int SelectDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = SqliteHandler::DataSetter::StoreData(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, "", COLUMN_SELECTED, "x");
		if (result)	return 1;

		result = SqliteHandler::DataSetter::StoreData(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, "x", COLUMN_IP, deviceData->ip);
		if (result)	return 1;

		return 0;
	}

	// get device data
	static int GetDataOfSelectedDevice(std::unique_ptr<device_data>& deviceData) {
		int result;
		StringList stringList;
		IntList intList;
		IntPairList intPairList;
		StringMatrix stringMatrix;
		IntMatrix intMatrix;
		IntPairMatrix intPairMatrix;
		

		// ip
		result = LoadFromSelected(COLUMN_IP, result, stringList);
		deviceData->ip = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// name
		result = LoadFromSelected(COLUMN_NAME, result, stringList);
		deviceData->name = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// version
		result = LoadFromSelected(COLUMN_VERSION, result, stringList);
		deviceData->version = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// source_labels
		result = LoadFromSelected(COLUMN_SOURCE_LABELS, result, stringMatrix);
		deviceData->sourceLabelsList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// destination_labels
		result = LoadFromSelected(COLUMN_DESTINATION_LABELS, result, stringMatrix);
		deviceData->destinationsLabelsList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// source_count
		result = LoadFromSelected(COLUMN_SOURCE_COUNT, result, intList);
		deviceData->source_count = intList[0];
		stringMatrix.clear();
		if (result)	return 1;

		// destination_count
		result = LoadFromSelected(COLUMN_DESTINATION_COUNT, result, intList);
		deviceData->destination_count = intList[0];
		stringMatrix.clear();
		if (result)	return 1;

		// routing
		result = LoadFromSelected(COLUMN_ROUTES, result, intPairMatrix);
		deviceData->routesList = intPairMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// locks
		result = LoadFromSelected(COLUMN_LOCKS, result, stringMatrix);
		deviceData->locksList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;
		
		// prepared routes
		result = LoadFromSelected(COLUMN_PREPARED, result, intPairMatrix);
		deviceData->routesPreparedList = intPairMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// routes marked for saving
		result = LoadFromSelected(COLUMN_MARKED, result, intPairMatrix);
		deviceData->routesMarkedList = intPairMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		return 0;
	}

	// set device data
	static int UpdateSelectedDeviceData(std::unique_ptr<device_data>& deviceData) {
		int result;

		// ip
		result = StoreInSelected(COLUMN_IP, deviceData->ip);
		if (result)	return 1;

		// name
		result = StoreInSelected(COLUMN_NAME, deviceData->name);
		if (result)	return 1;

		// version
		result = StoreInSelected(COLUMN_VERSION, deviceData->version);
		if (result)	return 1;

		// source_labels
		result = StoreInSelected(COLUMN_SOURCE_LABELS, deviceData->sourceLabelsList);
		if (result)	return 1;

		// destination_labels
		result = StoreInSelected(COLUMN_DESTINATION_LABELS, deviceData->destinationsLabelsList);
		if (result)	return 1;

		// source_count
		result = StoreInSelected(COLUMN_SOURCE_COUNT, deviceData->source_count);
		if (result)	return 1;

		// destination_count
		result = StoreInSelected(COLUMN_DESTINATION_COUNT, deviceData->destination_count);
		if (result)	return 1;

		// routing
		result = StoreInSelected(COLUMN_ROUTES, deviceData->routesList);
		if (result)	return 1;

		// locks
		result = StoreInSelected(COLUMN_LOCKS, deviceData->locksList);
		if (result)	return 1;

		// prepared routes
		result = StoreInSelected(COLUMN_PREPARED, deviceData->routesPreparedList);
		if (result)	return 1;

		// routes marked for saving
		result = StoreInSelected(COLUMN_MARKED, deviceData->routesMarkedList);
		if (result)	return 1;

		return 0;
	}

	// get list of routings for selected device
	static std::pair<StringList, IntPairMatrix> GetRoutesFromSelected(int& result) {
		StringList routingNames;
		IntPairMatrix routings;

		StringList ipList;
		result = LoadFromSelected(COLUMN_IP, result, ipList);

		result = SqliteHandler::DataGetter::LoadData(
			DB_NAME,
			ROUTINGS_TABLE,
			COLUMN_NAME,
			COLUMN_IP,
			ipList[0],
			routingNames
		);

		result = SqliteHandler::DataGetter::LoadData(
			DB_NAME,
			ROUTINGS_TABLE,
			COLUMN_ROUTES,
			COLUMN_IP,
			ipList[0],
			routings
		);

		return { routingNames, routings };
	}

	// save routing for selected device in routings table
	static int StoreRoutingForSelected(std::pair<std::string, IntPairList> routing) {
		int result;
		
		StringList ipList;
		result = LoadFromSelected(COLUMN_IP, result, ipList);
		if (result) return 1;

		result = SqliteHandler::DataSetter::InsertRow(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, ipList[0]);
		if (result) return 1;

		result = SqliteHandler::DataSetter::StoreData(DB_NAME, ROUTINGS_TABLE, COLUMN_NAME, routing.first, COLUMN_IP, ipList[0]);
		if (result) return 1;

		result = SqliteHandler::DataSetter::StoreData(DB_NAME, ROUTINGS_TABLE, COLUMN_ROUTES, routing.second, COLUMN_IP, ipList[0]);
		if (result) return 1;

		return 0;
	}
};