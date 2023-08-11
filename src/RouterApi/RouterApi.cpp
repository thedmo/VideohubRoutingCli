#include <RouterApi.hpp>
#include <format>

using namespace RouterModel;

Vapi::Vapi() {
}

int Vapi::GetInformationType(std::string line, information_type& type) {
	if (line == "PROTOCOL PREAMBLE:") {
		type = information_type::preamble;
	}
	else if (line == ("VIDEOHUB DEVICE:")) {
		type = information_type::device;
	}
	else if (line == ("INPUT LABELS:")) {
		type = information_type::inputs_labels;
	}
	else if (line == ("OUTPUT LABELS:")) {
		type = information_type::outputs_labels;
	}
	else if (line == ("VIDEO OUTPUT ROUTING:")) {
		type = information_type::routing;
	}
	else if (line == ("VIDEO OUTPUT LOCKS:")) {
		type = information_type::locks;
	}
	else if (line == ("CONFIGURATION:")) {
		type = information_type::configuration;
	}
	else if (line == ("ACK")) {
		type = information_type::ack;
	}
	else if (line == "END PRELUDE:") {
		type = information_type::end_prelude;
	}

	else {
		return ET::Collector::Add("Type could not be extracted from: " + line + ".");
	}
	return 0;
}

int Vapi::ExtractInformation(std::string info, std::unique_ptr<device_data>& _data) {
	information_type type = information_type::none;

	_data = std::make_unique<device_data>();

	std::string line;

	int result;

	std::stringstream info_stream(info);
	while (std::getline(info_stream, line)) {
		// continue and reset type upon empty string
		if (line.empty()) {
			type = information_type::none;
			continue;
		}

		// determine information type
		if (type == information_type::none) {
			result = GetInformationType(line, type);

			if (result != 0) {
				return ET::Collector::Add("Could not determine information type");
			}
			continue;
		}

		std::string informationStr;
		std::pair<int, int> routePair;

		// extract information
		switch (type) {
		case information_type::preamble:
			result = GetDeviceInformation(line, _data);
			break;
		case information_type::device:
			result = GetDeviceInformation(line, _data);
			break;
		case information_type::inputs_labels:
			_data->source_labels += line + "\n";
			result = Converter::StringLineToData(line, informationStr);
			_data->sourceLabelsList.push_back(informationStr);
			break;
		case information_type::outputs_labels:
			_data->destination_labels += line + "\n";
			result = Converter::StringLineToData(line, informationStr);
			_data->destinationsLabelsList.push_back(informationStr);
			break;
		case information_type::routing:
			_data->routing += line + "\n";
			result = Converter::RouteLineToRoutePair(line, routePair);
			_data->routesList.push_back(routePair);
			break;
		case information_type::locks:
			_data->locks += line + "\n";
			result = Converter::StringLineToData(line, informationStr);
			_data->locksList.push_back(informationStr);
			break;
		case information_type::configuration:
			// TODO Feature: configuration of device via library
			break;
		case information_type::end_prelude:
			break;

		default:
			return ET::Collector::Add("empty information type was passed");
			break;
		}
	}
	return 0;
}

int Vapi::GetDeviceInformation(std::string line, std::unique_ptr<device_data>& _data) {
	std::stringstream line_stream(line);

	std::string word;

	std::vector<std::string> words;
	while (std::getline(line_stream, word, ':')) {
		words.push_back(word);
	}

	if (words.empty()) return ET::Collector::Add("empty string was given to GetDeviceInformation Function");

	if (words[0].empty() || words[1].empty()) return ET::Collector::Add("empty string in extracted words from line");

	std::string key = words[0];
	std::string value = words[1];
	value.erase(0, 1);

	if (key.compare("Version") == 0) {
		_data->version = value;
		return 0;
	}
	else if (key.compare("Friendly name") == 0) {
		_data->name = value;
		return 0;
	}
	else if (key.compare("Video inputs") == 0) {
		_data->source_count = std::stoi(value);
		return 0;
	}
	else if (key.compare("Video outputs") == 0) {
		_data->destination_count = std::stoi(value);
		return 0;
	}

	return 0;
}

int Vapi::GetStatus(std::string ip, std::unique_ptr<device_data>& _data) {
	int result;
	std::string response;

	TelnetClient _telnet(ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("could not get status...", ET::Collector::GetErrorMessages());

	result = ExtractInformation(response, _data);
	if (result) return ET::Collector::Add("could not extract information");

	_data->ip = ip;

	return 0;
}

int Vapi::AddRouter(std::string ip) {
	std::unique_ptr<device_data> router_data = std::make_unique<device_data>();

	int result = GetStatus(ip, router_data);
	if (result) return ET::Collector::Add("Could not add router...", ET::Collector::GetErrorMessages());

	result = DataHandler::AddDevice(router_data);
	if (result) return ET::Collector::Add("could not add router to Storage");

	result = DataHandler::SelectDevice(router_data);
	if (result) return ET::Collector::Add("could not select router in Storage");

	result = DataHandler::UpdateSelectedDeviceData(router_data);
	if (result) return ET::Collector::Add("could not update data in Storage");

	return 0;
}

int Vapi::SelectRouter(std::string ip) {
	int result = 0;

	auto device = std::make_unique<device_data>();
	device->ip = ip;
	result = DataHandler::SelectDevice(device);
	if (result) return ET::Collector::Add("could not select router...", ET::Collector::GetErrorMessages());

	return 0;
}

int Vapi::RemoveSelectedRouter() {
	int result = 0;

	auto device = std::make_unique<device_data>();
	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("removing device did not work.", ET::Collector::GetErrorMessages());

	result = DataHandler::RemoveDevice(device);
	if (result) return ET::Collector::Add("removing device did not work.", ET::Collector::GetErrorMessages());

	return 0;
}

int Vapi::GetDevicesList(std::vector<std::string>& entries) {
	int result = 0;

	std::vector<std::unique_ptr<device_data>> devices;
	result = DataHandler::GetEntries(devices);
	if (result) return ET::Collector::Add("could not get Device listings: ", ET::Collector::GetErrorMessages());

	for (auto& device : devices) {
		const auto entry = std::format("| {0:<20} | {1:<20} | {2:<20} |", device->ip, device->name, device->version);
		entries.push_back(entry);
	}

	return 0;
}

int Vapi::RenameSource(int channel_number, const std::string new_name) {
	int result = 0;
	auto currentDevice = std::make_unique<device_data>();
	std::string response;

	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) ET::Collector::Add("Could not get device data: ");
	
	// Prüfen, ob Kanal Nummer nicht zu tief oder zu hoch --> neue private Funktion dafür erstellen
	if (channel_number < 0 || channel_number >= currentDevice->source_count ) return ET::Collector::Add("Problem with channel number: ");

	// Socket erstellen und öffnen
	TelnetClient tc(currentDevice->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not establish conection to device:");

	// Nachricht zusammenstellen und an Videohub schicken
	std::string message = "INPUT LABELS:\n" + std::to_string(channel_number) + " " + new_name + "\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("sending message did not work: ");

	// Liste mit Sourcenamen anfordern
	message = "INPUT LABELS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("sending message did not work: ");

	// get last response from socket and extract information
	auto deviceDataSources = std::make_unique<device_data>();
	response = tc.GetLastDataDump();
	currentDevice->source_labels = "";
	result = ExtractInformation(response, deviceDataSources);
	if (result) return ET::Collector::Add("Could extract routing from response");

	currentDevice->sourceLabelsList = deviceDataSources->sourceLabelsList;

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	return 0;
}

int Vapi::RenameDestination(int channel_number, const std::string new_name) {
	int result;
	auto currentDevice = std::make_unique<device_data>();
	std::string response;

	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("Could nod get data of selected device");

	// Prüfen, ob Kanal Nummer nicht zu tief oder zu hoch --> neue private Funktion dafür erstellen
	if (channel_number < 0 || channel_number >= currentDevice->destination_count) return ET::Collector::Add("Problem with channel number: ");

	// Socket erstellen und öffnen
	response;
	TelnetClient tc(currentDevice->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not establish conection to device:");

	// Nachricht zusammenstellen und an Videohub schicken
	std::string message = "OUTPUT LABELS:\n" + std::to_string(channel_number) + " " + new_name + "\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("sending message did not work: ");

	// Liste mit Sourcenamen anfordern
	message = "OUTPUT LABELS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("sending message did not work, device online? ");

	// get last response from socket and extract information into new device data object

	auto deviceDataDestinations = std::make_unique<device_data>();
	response = tc.GetLastDataDump();
	currentDevice->destination_labels = "";
	result = ExtractInformation(response, deviceDataDestinations);
	if (result) return ET::Collector::Add("Could extract data from response");

	currentDevice->destinationsLabelsList = deviceDataDestinations->destinationsLabelsList;

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	return 0;
}

int Vapi::GetSourcesList(std::vector<std::string>& sourcesList) {
	std::unique_ptr<device_data> device;
	std::string ip;
	int result = 0;

	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("could not get data of selected device");

	sourcesList = device->sourceLabelsList;

	return 0;
}

int Vapi::GetDestinations(std::vector<std::string>& destinationsList) {
	std::unique_ptr<device_data> device;
	std::string ip;
	int result = 0;

	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("could not get data of selected device");

	destinationsList = device->destinationsLabelsList;
	return 0;
}

int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source) {
	int result = 0;
	auto device = std::make_unique<device_data>();

	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("Could not get data of selected router from storage");

	device->routesPreparedList.push_back({ destination, source });

	result = DataHandler::UpdateSelectedDeviceData(device);
	if (result) return ET::Collector::Add("Could not update device data in storage");

	return 0;
}

int Vapi::TakePreparedRoutes() {
	std::string response;
	int result;
	std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

	result = DataHandler::GetDataOfSelectedDevice(current_device);
	if (result) return ET::Collector::Add("Could not not get prepared routes");

	// Create socket send ip from acquired device data
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket");

	// Composit command
	std::string prepRouteStr;
	for (auto route : current_device->routesPreparedList)
	{
		prepRouteStr += (std::to_string(route.first) + " " + std::to_string(route.second) + '\n');
	}

	// send Command to take routes to connected device
	std::string msg = "VIDEO OUTPUT ROUTING:\n" + prepRouteStr + '\n';
	result = tc.SendMsgToServer(msg);
	if (result) return ET::Collector::Add("Could not take prepared routes");

	// reset prepared routes in device_data
	current_device->routesPreparedList.clear();

	// get current routes from connected device (response gets saved in member variable)
	msg = "VIDEO OUTPUT ROUTING:\n\n";
	result = tc.SendMsgToServer(msg);
	if (result) return ET::Collector::Add("Could not get routing from device");

	// get response from member variable and fill in routing of device data
	response = tc.GetLastDataDump();
	current_device->routing = "";

	auto deviceRouting = std::make_unique<device_data>();
	result = ExtractInformation(response, deviceRouting);
	if (result) return ET::Collector::Add("Could extract routing from response");

	current_device->routesList = deviceRouting->routesList;
	result = DataHandler::UpdateSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database");

	return 0;
}

int Vapi::GetRoutes(std::vector<std::pair<int, int>>& routesList) {
	std::unique_ptr<device_data> device;
	std::string ip;
	int result = 0;

	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("could not get data of selected device");

	routesList = device->routesList;
	return 0;
}

int Vapi::LockRoute(unsigned int destination) {
	int result;
	auto currentDevice = std::make_unique<device_data>();
	auto deviceLocks = std::make_unique<device_data>();

	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("could not get devicedata from selected device");

	if (destination < 0 || destination >= currentDevice->destination_count) return ET::Collector::Add("Channel Number not in range");

	// create socket
	std::string response;
	TelnetClient tc(currentDevice->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket");

	// generate message and send to device
	std::string message = "VIDEO OUTPUT LOCKS:\n" + std::to_string(destination) + " L\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("Could not send message");

	// get lock information from device
	message = "VIDEO OUTPUT LOCKS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("Could not send message");

	response = tc.GetLastDataDump();

	result = ExtractInformation(response, deviceLocks);

	// NEW
	currentDevice->locksList = deviceLocks->locksList;
	if (result) return ET::Collector::Add("could not extract data from response of connected device");

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("could not update data of selected device in storage");
	
	return 0;
}

int Vapi::UnlockRoute(unsigned int destination) {
	int result;
	auto currentDevice = std::make_unique<device_data>();
	//auto current_device = std::make_unique<device_data>();
	std::unique_ptr<device_data> deviceLocks;

	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("could not get device data: ");

	if (destination < 0 || destination >= currentDevice->destination_count) return ET::Collector::Add("Channel Number not in range");

	// create socket
	std::string response;
	TelnetClient tc(currentDevice->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket");

	// generate message and send to device
	std::string message = "VIDEO OUTPUT LOCKS:\n" + std::to_string(destination) + " U\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("Could not send message");

	// get lock information from device
	message = "VIDEO OUTPUT LOCKS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("Could not send message");

	response = tc.GetLastDataDump();
	result = ExtractInformation(response, deviceLocks);

	currentDevice->locksList = deviceLocks->locksList;
	if (result) return ET::Collector::Add("Could not extract information of response from connecte device");

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);

	return 0;
}

std::pair<int, int> Vapi::GetRouteFromDestination(int destination, std::unique_ptr<device_data>& deviceData) {
	for (auto route : deviceData->routesList) {
		if (route.first == destination) {
			return route;
		}
	}
}

int Vapi::MarkRoutes2(std::vector<int> destinations) {
	int result = 0;
	std::unique_ptr<device_data> selectedDevice;
	std::unique_ptr<device_data> connectedDevice;
	std::string response;
	std::string ip;

	result = DataHandler::GetDataOfSelectedDevice(selectedDevice);
	if (result) return ET::Collector::Add("could not get data of selected device from storage");

	ip = selectedDevice->ip;
	result = GetStatus(ip, connectedDevice);
	if (result) ET::Collector::Add("Could not get data from connected device");

	for (int destination : destinations) {
		if (destination < 0 || destination >= connectedDevice->destination_count) return ET::Collector::Add("Destination out of range: " + std::to_string(destination));

		connectedDevice->routesMarkedList.push_back(GetRouteFromDestination(destination, connectedDevice));
	}
	selectedDevice->routesMarkedList = connectedDevice->routesMarkedList;

	// update device data with new deviceData object to update the markedforsavings list in storage
	DataHandler::UpdateSelectedDeviceData(selectedDevice);

	return 0;
}

int Vapi::SaveRoutes(std::string routing_name) {
	int result;
	std::unique_ptr<device_data> selectedDevice;

	result = DataHandler::GetDataOfSelectedDevice(selectedDevice);
	if (result) return ET::Collector::Add("Could not get data of selected device from storage");

	if (selectedDevice->routesMarkedList.empty()) return ET::Collector::Add("now marked routes to store as saved routes");

	auto markedRoutes = selectedDevice->routesMarkedList;
	DataHandler::Routing markedRouting = { routing_name, markedRoutes };
	result = DataHandler::StoreRoutingForSelected(markedRouting);
	if (result) return ET::Collector::Add("Could not store routing in storage");

	selectedDevice->routesMarkedList.clear();
	result = DataHandler::UpdateSelectedDeviceData(selectedDevice);
	if (result) return ET::Collector::Add("Could not update device data in storage");

	return 0;
}

int Vapi::GetSavedRoutes(std::vector < std::pair<std::string, std::vector<std::pair<int, int>>>> &routingsList) {
	int result;

	result = DataHandler::GetRoutesFromSelected(routingsList);
	if (result) return ET::Collector::Add("could not acquire routings from storage");

	return 0;
}

int Vapi::LoadRoutes(std::string name) {
	int result;
	std::string response;
	std::string routes;
	DataHandler::Routing routing;
	DataHandler::RoutingsList routingsList;
	std::unique_ptr<device_data> selectedDevice;
	std::unique_ptr<device_data> connectedDevice;

	result = DataHandler::GetDataOfSelectedDevice(selectedDevice);
	if (result) return ET::Collector::Add("Could not get data of selected device");

	result = DataHandler::GetRoutesFromSelected(routingsList);
	if (result) return ET::Collector::Add("could not get saved routes of selected device from storage");

	TelnetClient tc(selectedDevice->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket", ET::Collector::GetErrorMessages());

	for (DataHandler::Routing routing : routingsList) {
		if (routing.first != name) continue;

		for (std::pair<int, int> route : routing.second) {
			routes += std::to_string(route.first) + " " + std::to_string(route.second) + '\n';
		}
	}

	// send Command to take routes to connected device
	std::string route_command = "VIDEO OUTPUT ROUTING:\n" + routes + '\n';
	result = tc.SendMsgToServer(route_command);
	if (result) return ET::Collector::Add("Could not load routes", ET::Collector::GetErrorMessages());

	// get current routes from connected device (response gets saved in member variable)
	route_command = "VIDEO OUTPUT ROUTING:\n\n";
	result = tc.SendMsgToServer(route_command);
	if (result) return ET::Collector::Add("Could not get routing from device", ET::Collector::GetErrorMessages());

	// get response from member variable and fill in routing of device data
	response = tc.GetLastDataDump();
	result = ExtractInformation(response, connectedDevice);
	if (result) return ET::Collector::Add("Could extract routing from response");

	selectedDevice->routesList = connectedDevice->routesList;
	result = DataHandler::UpdateSelectedDeviceData(selectedDevice);
	if (result) return ET::Collector::Add("could not update data in storage");

	return 0;
}