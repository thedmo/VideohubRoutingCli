#ifndef DeviceData
#define DeviceData

#include <iostream> 
#include <vector>

class device_data {
public:
	device_data() {};
	~device_data() {};

	// Copy Constructor
	device_data(device_data& device) :
		ip(device.ip),
		name(device.name),
		version(device.version),
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
		version;

	int
		source_count,
		destination_count;

	//TODO: to replace simple variables in device DataVector

	// Storable as blobs in database
	std::vector<std::string>
		sourceLabelsList,
		destinationsLabelsList,
		locksList;

	std::vector<std::pair<int, int>>
		routesMarkedList,
		routesPreparedList,
		routesList;

	bool Equals(const device_data* other) {
		return
			this->ip == other->ip &&
			this->name == other->name &&
			this->version == other->version &&
			this->source_count == other->source_count &&
			this->destination_count == other->destination_count &&
			this->sourceLabelsList == other->sourceLabelsList &&
			this->destinationsLabelsList == other->destinationsLabelsList &&
			this->locksList == other->locksList &&
			this->routesMarkedList == other->routesMarkedList &&
			this->routesPreparedList == other->routesPreparedList &&
			this->routesList == other->routesList;
	}
};

#endif // !DeviceData
