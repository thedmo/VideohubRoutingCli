#ifndef EE918193_380A_4CAC_B16D_81B97DBCC972
#define EE918193_380A_4CAC_B16D_81B97DBCC972

#include <iostream> 

class device_data {
public:
	device_data() {};

	// Copy Constructor
	device_data(device_data& device) :
		ip(device.ip),
		name(device.name),
		version(device.version),
		source_labels(device.source_labels),
		destination_labels(device.destination_labels),
		routing(device.routing),
		prepared_routes(device.prepared_routes),
		locks(device.locks),
		marked_for_saving(device.marked_for_saving),
		source_count(device.source_count),
		destination_count(device.destination_count),
		sourceLabelsList(device.sourceLabelsList),
		destinationsLabelsList(device.destinationsLabelsList),
		locksList(device.locksList),
		routesMarkedList(device.routesMarkedList),
		routesPreparedList(device.routesPreparedList),
		routesList(device.routesList)
	{ }

	std::string
		ip,
		name,
		version,
		source_labels,
		destination_labels,
		routing,
		prepared_routes,
		locks,
		marked_for_saving;

	int
		source_count,
		destination_count;

	//TODO: to replace simple variables in device Data

	// Storable as blobs in database
	std::vector<std::string>
		sourceLabelsList,
		destinationsLabelsList,
		locksList;

	std::vector<std::pair<int, int>>
		routesMarkedList,
		routesPreparedList,
		routesList;

	bool Equals(const device_data& other) {
		return
			this->ip == other.ip &&
			this->name == other.name &&
			this->version == other.version &&
			this->source_labels == other.source_labels &&
			this->destination_labels == other.destination_labels &&
			this->routing == other.routing &&
			this->prepared_routes == other.prepared_routes &&
			this->locks == other.locks &&
			this->marked_for_saving == other.marked_for_saving &&
			this->source_count == other.source_count &&
			this->destination_count == other.destination_count &&
			this->sourceLabelsList == other.sourceLabelsList &&
			this->destinationsLabelsList == other.destinationsLabelsList &&
			this->locksList == other.locksList &&
			this->routesMarkedList == other.routesMarkedList &&
			this->routesPreparedList == other.routesPreparedList &&
			this->routesList == other.routesList;
	}
};


#endif /* EE918193_380A_4CAC_B16D_81B97DBCC972 */
