#include <sql_access_handler.hpp>
#include <device_data.hpp>

using namespace SqliteHandler;

class DataHandler : private DataGetter, private DataSetter, private DbMod, private TableInfo {

	using StringList = std::vector<std::string>;
	using IntList = std::vector<int>;
	using IntPairList = std::vector<std::pair<int, int>>;
	using StringMatrix = std::vector<std::vector<std::string>>;
	using IntMatrix = std::vector<std::vector<int>>;
	using IntPairMatrix = std::vector<std::vector<std::pair<int, int>>>;

public:
	/// <summary>
	/// std::pair with string as name and vector with pairs of ints as routes
	/// </summary>
	using Routing = std::pair<std::string, std::vector<std::pair<int,int>>>;

	/// <summary>
	/// std::vector with Routing objects
	/// </summary>
	using RoutingsList = std::vector<Routing>;

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

private:
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

	/// <summary>
	/// Load Data from selected device in table.  
	/// </summary>
	/// <typeparam name="Storage">Type of Container given into method</typeparam>
	/// <param name="propName">name of property to load</param>
	/// <param name="dataStorage">container to load data into. has to be a vector</param>
	/// <returns>int; 0 = OK </returns>
	template <typename Storage>
	static int LoadFromSelected(const std::string propName, Storage& dataStorage) {
		int result = 0;

		result = DataGetter::LoadData(DB_NAME, DEVICES_TABLE, propName, COLUMN_SELECTED, "x", dataStorage);
		if (result) return 1;

		return 0;
	}

	/// <summary>
	/// Stores data in selected device
	/// </summary>
	/// <typeparam name="Storage"></typeparam>
	/// <param name="propName">name of property to store</param>
	/// <param name="dataStorage">container to load data into. has to be a vector</param>
	/// <returns>int; 0 = OK </returns>
	template <typename Storage>
	static int StoreInSelected(	const std::string propName,	Storage& dataStorage) {
		return DataSetter::StoreData( 
			DB_NAME,
			DEVICES_TABLE,
			propName,
			dataStorage,
			COLUMN_SELECTED,
			"x"
		);
	}

public:
	/// <summary>
	/// Initializes Storage for devices and routings if it not already exists
	/// </summary>
	/// <returns>int; 0 = OK</returns>
	static int InitializeStorage() {
		int result;

		// Create Devicedatabase if not exists
		result = DbMod::CreateTableWithPrimaryKey(DB_NAME, DEVICES_TABLE, COLUMN_IP, TEXT);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, TEXT);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_NAME, TEXT);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_VERSION, TEXT);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_COUNT, INTEGER);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_DESTINATION_COUNT, INTEGER);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_LABELS, BLOB);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_DESTINATION_LABELS, BLOB);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_ROUTES, BLOB);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_PREPARED, BLOB);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_LOCKS, BLOB);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, DEVICES_TABLE, COLUMN_MARKED, BLOB);
		if (result) return 1;

		// create routingdatabase if not exists
		result = DbMod::CreateTableWithForeignKey(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, TEXT, DEVICES_TABLE, COLUMN_IP);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, ROUTINGS_TABLE, COLUMN_NAME, TEXT);
		if (result) return 1;
		result = DbMod::AddColumn(DB_NAME, ROUTINGS_TABLE, COLUMN_ROUTES, BLOB);
		if (result) return 1;

		return 0;
	}

	/// <summary>
	/// Adds device to Storage. Only adds device with ip. Update device data with the ref object after adding it
	/// </summary>
	/// <param name="deviceData">reference to object that holds the data of the device</param>
	/// <returns>int; 0 = OK</returns>
	static int AddDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = DbMod::InsertRow(DB_NAME, DEVICES_TABLE, COLUMN_IP, deviceData->ip);
		if (result) return 1;

		return 0;
	}

	/// <summary>
	/// Removes specified device 
	/// </summary>
	/// <param name="deviceData">reference to object that holds the ip (identifier) of the device to remove</param>
	/// <returns>int; 0 = OK</returns>
	static int RemoveDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = DbMod::RemoveRow(DB_NAME, DEVICES_TABLE, COLUMN_IP, deviceData->ip);
		if (result)	return 1;

		if (TableInfo::CheckIfRowExists(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, deviceData->ip)) {
			result = DbMod::RemoveRow(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, deviceData->ip);
			if (result)	return 1;
		}

		return 0;
	}

	/// <summary>
	/// Updates entry in storage with values from references device data object
	/// </summary>
	/// <param name="deviceData">reference to unique_ptr of device data object</param>
	/// <returns>int; 0 = OK </returns>
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

	/// <summary>
	/// Gets List with all added entries in Storage
	/// </summary>
	/// <param name="deviceDataVector">reference to vector that can hold unique pointers to device_data objects. can be empty</param>
	/// <returns>int; 0 = OK </returns>
	static int GetEntries(std::vector<std::unique_ptr<device_data>>& deviceDataVector) {
		int result = 0;

		StringList ipList;
		StringList nameList;
		IntList sourceCountList;

		result = DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_IP, ipList);
		if (result) return 1;

		result = DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_NAME, nameList);
		if (result) return 1;

		result = DataGetter::LoadData(DB_NAME, DEVICES_TABLE, COLUMN_SOURCE_COUNT, sourceCountList);
		if (result) return 1;

		for (size_t i = 0; i < ipList.size(); i++) {
			deviceDataVector.push_back(std::make_unique<device_data>());
			deviceDataVector[i]->ip = ipList[i];
			deviceDataVector[i]->name = nameList[i];
			deviceDataVector[i]->source_count = sourceCountList[i];
		}

		return 0;
	}

	/// <summary>
	/// Selects device in storage
	/// </summary>
	/// <param name="deviceData">unique_ptr for device data object. only needs ip address</param>
	/// <returns>int; 0 = OK </returns>
	static int SelectDevice(std::unique_ptr<device_data>& deviceData) {
		int result;

		result = DataSetter::StoreData(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, "", COLUMN_SELECTED, "x");
		if (result)	return 1;

		result = DataSetter::StoreData(DB_NAME, DEVICES_TABLE, COLUMN_SELECTED, "x", COLUMN_IP, deviceData->ip);
		if (result)	return 1;

		return 0;
	}

	/// <summary>
	/// Gets information of selected device from storage
	/// </summary>
	/// <param name="deviceData">reference to unique_ptr object for device_data. Can be uninitialized</param>
	/// <returns></returns>
	static int GetDataOfSelectedDevice(std::unique_ptr<device_data>& deviceData) {
		int result;
		StringList stringList;
		IntList intList;
		IntPairList intPairList;
		StringMatrix stringMatrix;
		IntMatrix intMatrix;
		IntPairMatrix intPairMatrix;

		deviceData = std::make_unique<device_data>();

		// ip
		result = LoadFromSelected(COLUMN_IP, stringList);
		if (stringList.empty()) return 1;
		deviceData->ip = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// name
		result = LoadFromSelected(COLUMN_NAME, stringList);
		if (stringList.empty()) return 1;
		deviceData->name = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// version
		result = LoadFromSelected(COLUMN_VERSION, stringList);
		if (stringList.empty()) return 1;
		deviceData->version = stringList[0];
		stringList.clear();
		if (result)	return 1;

		// source_labels
		result = LoadFromSelected(COLUMN_SOURCE_LABELS, stringMatrix);
		if (stringMatrix.empty()) return 1;
		deviceData->sourceLabelsList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// destination_labels
		result = LoadFromSelected(COLUMN_DESTINATION_LABELS, stringMatrix);
		if (stringMatrix.empty()) return 1;
		deviceData->destinationsLabelsList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// source_count
		result = LoadFromSelected(COLUMN_SOURCE_COUNT, intList);
		if (intList.empty()) return 1;
		deviceData->source_count = intList[0];
		intList.clear();
		if (result)	return 1;

		// destination_count
		result = LoadFromSelected(COLUMN_DESTINATION_COUNT, intList);
		if (intList.empty()) return 1;
		deviceData->destination_count = intList[0];
		intList.clear();
		if (result)	return 1;

		// routing
		result = LoadFromSelected(COLUMN_ROUTES, intPairMatrix);
		if (intPairMatrix.empty()) return 1;
		deviceData->routesList = intPairMatrix[0];
		intPairMatrix.clear();
		if (result)	return 1;

		// locks
		result = LoadFromSelected(COLUMN_LOCKS, stringMatrix);
		if (stringMatrix.empty()) return 1;
		deviceData->locksList = stringMatrix[0];
		stringMatrix.clear();
		if (result)	return 1;

		// prepared routes
		result = LoadFromSelected(COLUMN_PREPARED, intPairMatrix);
		if (intPairMatrix.empty()) return 1;
		deviceData->routesPreparedList = intPairMatrix[0];
		intPairMatrix.clear();
		if (result)	return 1;

		// routes marked for saving
		result = LoadFromSelected(COLUMN_MARKED, intPairMatrix);
		if (intPairMatrix.empty()) return 1;
		deviceData->routesMarkedList = intPairMatrix[0];
		intPairMatrix.clear();
		if (result)	return 1;

		return 0;
	}

	// get list of routings for selected device
	static std::pair<StringList, IntPairMatrix> GetRoutesFromSelected(int& result) {
		StringList routingNames;
		IntPairMatrix routings;

		StringList ipList;
		result = LoadFromSelected(COLUMN_IP, ipList);

		result = DataGetter::LoadData(
			DB_NAME,
			ROUTINGS_TABLE,
			COLUMN_NAME,
			COLUMN_IP,
			ipList[0],
			routingNames
		);

		result = DataGetter::LoadData(
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
		result = LoadFromSelected(COLUMN_IP, ipList);
		if (result) return 1;

		result = DbMod::InsertRow(DB_NAME, ROUTINGS_TABLE, COLUMN_IP, ipList[0]);
		if (result) return 1;

		result = DataSetter::StoreData(DB_NAME, ROUTINGS_TABLE, COLUMN_NAME, routing.first, COLUMN_IP, ipList[0]);
		if (result) return 1;

		result = DataSetter::StoreData(DB_NAME, ROUTINGS_TABLE, COLUMN_ROUTES, routing.second, COLUMN_IP, ipList[0]);
		if (result) return 1;

		return 0;
	}
};