#include <RouterApi.hpp>

using namespace RouterModel;

Vapi::Vapi() {
}
vdb Vapi::m_database;

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
	return Vapi::ROUTER_API_OK;
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

			if (result != Vapi::ROUTER_API_OK) {
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
	return Vapi::ROUTER_API_OK;
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

	std::cout << key << " " << value << std::endl;

	if (key.compare("Version") == 0) {
		_data->version = value;
		return Vapi::ROUTER_API_OK;
	}
	else if (key.compare("Friendly name") == 0) {
		_data->name = value;
		return Vapi::ROUTER_API_OK;
	}
	else if (key.compare("Video inputs") == 0) {
		_data->source_count = std::stoi(value);
		return Vapi::ROUTER_API_OK;
	}
	else if (key.compare("Video outputs") == 0) {
		_data->destination_count = std::stoi(value);
		return Vapi::ROUTER_API_OK;
	}

	return Vapi::ROUTER_API_OK;
}

/// <summary>
/// Gets Information of router by ip and stores it into dataobject
/// </summary>
/// <param name="ip">ip as string</param>
/// <param name="_data">reference to deviceData Object</param>
/// <returns>int; 0 = OK</returns>
int Vapi::GetStatus(std::string ip, std::unique_ptr<device_data>& _data) {
	int result;
	std::string response;

	TelnetClient _telnet(ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("could not get status...", ET::Collector::GetErrorMessages());

	result = ExtractInformation(response, _data);
	if (result) return ET::Collector::Add("could not extract information");

	_data->ip = ip;

	return ROUTER_API_OK;
}

/// <summary>
/// Adds a Router by ip address
/// </summary>
/// <param name="ip">ip address as string</param>
/// <returns>int; 0 = ok</returns>
int Vapi::AddRouter(std::string ip) {
	std::unique_ptr<device_data> router_data = std::make_unique<device_data>();

	int result = GetStatus(ip, router_data);
	if (result) return ET::Collector::Add("Could not add router...", ET::Collector::GetErrorMessages());


	// OLD
	result = m_database.check_if_device_exists(ip);
	if (result) return ET::Collector::Add("could not check if device exists... ", ET::Collector::GetErrorMessages());

	result = m_database.insert_device_into_db(router_data);
	if (result) return ET::Collector::Add("could not add router to Storage");
	// old


	// NEW
	result = DataHandler::AddDevice(router_data);
	if (result) return ET::Collector::Add("could not add router to Storage");

	result = DataHandler::SelectDevice(router_data);
	if (result) return ET::Collector::Add("could not select router in Storage");

	result = DataHandler::UpdateSelectedDeviceData(router_data);
	if (result) return ET::Collector::Add("could not update data in Storage");

	return ROUTER_API_OK;
}

/// <summary>
/// Selects a router from all devices to work on by ip
/// </summary>
/// <param name="ip">ip as string</param>
/// <returns>int; 0 = OK</returns>
int Vapi::SelectRouter(std::string ip) {
	int result = 0;


	// OLD
	result = m_database.select_device(ip);
	if (result) return ET::Collector::Add("could not select router...", ET::Collector::GetErrorMessages());
	// old


	// NEW
	auto device = std::make_unique<device_data>();
	device->ip = ip;
	result = DataHandler::SelectDevice(device);
	if (result) return ET::Collector::Add("could not select router...", ET::Collector::GetErrorMessages());

	return 0;
}

/// <summary>
/// Removes selected Device from storage if it exists
/// </summary>
/// <returns>int; 0 = ok</returns>
int Vapi::RemoveSelectedRouter() {
	int result = 0;

	result = m_database.remove_selected_device_from_db();
	if (result) return ET::Collector::Add("removing device did not work.", ET::Collector::GetErrorMessages());

	auto device = std::make_unique<device_data>();
	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("removing device did not work.", ET::Collector::GetErrorMessages());

	result = DataHandler::RemoveDevice(device);
	if (result) return ET::Collector::Add("removing device did not work.", ET::Collector::GetErrorMessages());

	return 0;
}

[[deprecated("will be replaced by GetDevicesList")]]
int Vapi::GetDevices(std::string& callback) {
	int result = m_database.GetDevices(callback);
	if (result) return ET::Collector::Add("could not get Device listings: ", ET::Collector::GetErrorMessages());

	return 0;
}

/// <summary>
/// Gets List of entries in Storage. stores informmation into list of formattet strings
/// </summary>
/// <param name="entries">reference to list of strings</param>
/// <returns>int; 0 = ok</returns>
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

/// <summary>
/// Rename sourcechannel from connected device
/// </summary>
/// <param name="channel_number">number of channel</param>
/// <param name="new_name">new name for channel</param>
/// <returns>int; 0 = OK</returns>
int Vapi::RenameSource(int channel_number, const std::string new_name) {
	int result = 0;
	auto currentDevice = std::make_unique<device_data>();
	std::string response;

	//// OLD

	//// get Device information for IP
	//result = m_database.GetSelectedDeviceData(currentDevice);
	//if (result) ET::Collector::Add("Could not get device data: ");
	//// old


	// NEW
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

	// OLD

	// update values in database with device data
	result = m_database.update_selected_device_data(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());
	// old


	// NEW
	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	return 0;
}

[[deprecated("use devicedata from get status instead")]]
int Vapi::GetSources(std::string& callback) {
	int result;

	// get Device information for IP
	auto current_device = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) ET::Collector::Add("Could not get device data: ");

	// Socket erstellen und öffnen
	std::string response;
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not establish conection to device:");

	// Nachrich an socket schicken
	std::string message = "INPUT LABELS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("could not send message to device ");

	// Letzte Anwort von socket anfordern und in string speichern
	callback = tc.GetLastDataDump();

	return ROUTER_API_OK;
}

/// <summary>
/// Rename destination channel of selected router
/// </summary>
/// <param name="channel_number">number of channel</param>
/// <param name="new_name">new name for channel</param>
/// <returns>int; 0 = ok</returns>
int Vapi::RenameDestination(int channel_number, const std::string new_name) {
	int result;
	auto currentDevice = std::make_unique<device_data>();
	std::string response;



	//// OLD
	//// get Device information for IP
	//result = m_database.GetSelectedDeviceData(currentDevice);
	//if (result) ET::Collector::Add("Could not get device data: ");
	//// old
	


	// NEW
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

	// OLD
	// update values in database with device data
	result = m_database.update_selected_device_data(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());
	// old


	// NEW
	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	return ROUTER_API_OK;
}

[[deprecated("use devicedata from get status instead")]]
int Vapi::GetDestinations(std::string& callback) {
	int result;

	// get Device information for IP
	auto current_device = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) ET::Collector::Add("Could not get device data: ");

	// Socket erstellen und öffnen
	std::string response;
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not establish conection to device:");

	// Nachrich an socket schicken
	std::string message = "OUTPUT LABELS:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("could not send message to device ");

	// Letzte Anwort von socket anfordern und in string speichern
	callback = tc.GetLastDataDump();

	return ROUTER_API_OK;
}

/// <summary>
/// prepares new route and adds ist to list of prepared routes. use Take to send new routes to device
/// </summary>
/// <param name="destination">number of destionation channel</param>
/// <param name="source">number if sourcechannel</param>
/// <returns>int; 0 = OK</returns>
int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source) {
	int result = 0;
	auto device = std::make_unique<device_data>();

	/// OLD
	result = m_database.add_to_prepared_routes(destination, source);
	if (result) return ET::Collector::Add("Could not add new route");
	// old


	// NEW
	result = DataHandler::GetDataOfSelectedDevice(device);
	if (result) return ET::Collector::Add("Could not get data of selected router from storage");

	// HIER WEITER
	device->routesPreparedList.push_back({ destination, source });

	result = DataHandler::UpdateSelectedDeviceData(device);
	if (result) return ET::Collector::Add("Could not update device data in storage");

	return 0;
}

/// <summary>
/// Send all prepared routes to selected device
/// </summary>
/// <returns>int; 0= OK</returns>
int Vapi::TakePreparedRoutes() {
	std::string response;
	int result;
	std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

	//// OLD

	//// Get device data from database
	//result = m_database.GetSelectedDeviceData(current_device);
	//if (result) return ET::Collector::Add("Could not not get prepared routes");
	//// old


	// NEW
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
	current_device->prepared_routes = "";
	current_device->routesPreparedList.clear();
	// if (result) return ET::Collector::Add("could not reset prepared routes", ET::Collector::GetErrorMessages());

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


	// OLD
	// update values in database with device data
	result = m_database.update_selected_device_data(current_device);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database");
	// old


	// NEW
	current_device->routesList = deviceRouting->routesList;
	result = DataHandler::UpdateSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database");

	return ROUTER_API_OK;
}

/// <summary>
/// Enables lock on certain channel
/// </summary>
/// <param name="destination"></param>
/// <returns></returns>
int Vapi::LockRoute(unsigned int destination) {
	int result;
	auto currentDevice = std::make_unique<device_data>();
	
	//// OLD
	//// Check Channelnumber
	//result = check_channel_number(destination);
	//if (result) return ET::Collector::Add("something wrong with the channelnumber: ");
	//// old
	
	// NEW
	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("could not get devicedata from selected device");

	if (destination < 0 || destination >= currentDevice->destination_count) return ET::Collector::Add("Channel Number not in range");




	// OLD
	// get device data
	auto current_device = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("could not get device data: ");
	// old




	// create socket
	std::string response;
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
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




	// OLD
	// update device data in database
	current_device->locks = "";
	result = ExtractInformation(response, current_device);
	m_database.update_selected_device_data(current_device);
	//old




	// NEW
	currentDevice->locksList.clear();
	result = ExtractInformation(response, currentDevice);
	if (result) return ET::Collector::Add("could not extract data from response of connected device");

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);
	if (result) return ET::Collector::Add("could not update data of selected device in storage");
	
	return ROUTER_API_OK;
}

/// <summary>
/// Unlock route from selected device
/// </summary>
/// <param name="destination">channelnumber to unlock</param>
/// <returns>int; 0 = OK</returns>
int Vapi::UnlockRoute(unsigned int destination) {
	int result;
	auto currentDevice = std::make_unique<device_data>();

	// Check Channelnumber
	result = check_channel_number(destination);
	if (result) return ET::Collector::Add("something wrong with the channelnumber: ");




	// OLD
	// get device data
	auto current_device = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("could not get device data: ");
	// old

	// NEW
	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("could not get device data: ");



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



	// OLD
	// update device data in database
	current_device->locks = "";
	response = tc.GetLastDataDump();
	result = ExtractInformation(response, current_device);
	m_database.update_selected_device_data(current_device);
	// old



	// NEW
	currentDevice->locksList.clear();
	response = tc.GetLastDataDump();
	result = ExtractInformation(response, currentDevice);
	if (result) return ET::Collector::Add("Could not extract information of response from connecte device");

	result = DataHandler::UpdateSelectedDeviceData(currentDevice);

	return ROUTER_API_OK;
}

[[deprecated("use getstatus instead")]]
int Vapi::GetRoutes(std::string& callback) {
	int result;

	// get Device information for IP
	auto current_device = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) ET::Collector::Add("Could not get device data: ");

	// Socket erstellen und öffnen
	std::string response;
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not establish conection to device:");

	// Nachrich an socket schicken
	std::string message = "VIDEO OUTPUT ROUTING:\n\n";
	result = tc.SendMsgToServer(message);
	if (result) return ET::Collector::Add("could not send message to device ");

	// Letzte Anwort von socket anfordern und in string speichern
	callback = tc.GetLastDataDump();

	// update device data in database
	current_device->routing = "";
	response = tc.GetLastDataDump();
	result = ExtractInformation(response, current_device);
	m_database.update_selected_device_data(current_device);

	return ROUTER_API_OK;
}

[[deprecated("use MarkRoutes2 instead")]]
int Vapi::MarkRouteForSaving(int destination) {
	std::string response;
	int result;
	std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

	// Get device data from database
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("Could not not get device data from database: ", ET::Collector::GetErrorMessages());

	// Create socket
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket: ", ET::Collector::GetErrorMessages());

	// get current routes from connected device (response gets saved in member variable)
	std::string msg = "VIDEO OUTPUT ROUTING:\n\n";
	result = tc.SendMsgToServer(msg);
	if (result) return ET::Collector::Add("Could not get routing from device: ", ET::Collector::GetErrorMessages());

	// get response from member variable and fill in routing of device data
	response = tc.GetLastDataDump();
	current_device->routing = "";
	result = ExtractInformation(response, current_device);
	if (result) return ET::Collector::Add("Could not extract routing from response: ");

	// update values in database with device data
	result = m_database.update_selected_device_data(current_device);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	result = m_database.mark_route_for_saving(destination);
	if (result) return ET::Collector::Add("could not mark route: ", ET::Collector::GetErrorMessages());

	return ROUTER_API_OK;
}

std::pair<int, int> Vapi::GetRouteFromDestination(int destination, std::unique_ptr<device_data>& deviceData) {
	for (auto route : deviceData->routesList) {
		if (route.first == destination) {
			return route;
		}
	}
}

/// <summary>
/// Mark routes for saving
/// </summary>
/// <param name="destinations">list with destinations to add to marked routes</param>
/// <returns>int; 0 = ok</returns>
int Vapi::MarkRoutes2(std::vector<int> destinations) {
	int result = 0;
	auto currentDevice = std::make_unique<device_data>() ;
	std::string response;

	result = DataHandler::GetDataOfSelectedDevice(currentDevice);
	if (result) return ET::Collector::Add("could not get data of selected device from storage");

	auto markedRoutes = currentDevice->routesMarkedList;

	std::string ip = currentDevice->ip;
	result = GetStatus(ip, currentDevice);
	if (result) ET::Collector::Add("Could not get data from connected device");

	bool isAlreadyInList = false;
	for (size_t i = 0; i < destinations.size(); i++)
	{
		for (size_t j = 0; i < markedRoutes.size(); j++) {
			if (markedRoutes[j].first == destinations[i]) {
				isAlreadyInList = true;
				break;
			}

			if (isAlreadyInList) continue;

			auto route = GetRouteFromDestination(destinations[i], currentDevice);
			currentDevice->routesMarkedList.push_back(route);
		}
	}

	// update device data with new deviceData object to update the markedforsavings list in storage
	DataHandler::UpdateSelectedDeviceData(currentDevice);

	return ROUTER_API_OK;
}

int Vapi::SaveRoutes(std::string routing_name) {
	// Get marked routes of selected device from database
	int result;
	std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

	// Get device data with prepared routes from database
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("Could not not get marked routes", ET::Collector::GetErrorMessages());

	if (current_device->marked_for_saving.empty()) return ET::Collector::Add("No marked route to save present in selected device");

	// Insert new row for routing of selected device in routings table: ip, name of routing, routing as text
	result = m_database.save_routing(routing_name, current_device);
	if (result) return ET::Collector::Add("Could not save routing: ", ET::Collector::GetErrorMessages());



	return ROUTER_API_OK;
}

// TODO
int Vapi::GetSavedRoutes(std::string& callback) {
	int result;
	DataHandler::RoutingsList routings;

	result = DataHandler::GetRoutesFromSelected(routings);


	// get routing
	result = m_database.get_saved_routings(callback);
	if (result) return ET::Collector::Add("Could not not get saved routings: ", ET::Collector::GetErrorMessages());

	return ROUTER_API_OK;
}

int Vapi::LoadRoutes(std::string name) {
	std::unique_ptr<device_data> current_device = std::make_unique<device_data>();
	std::string response;
	std::string routes;
	int result;

	// send request to database api to get saved routes by name
	result = m_database.get_routing_by_name(name, routes);
	if (result) return ET::Collector::Add("could not get saved routes:");

	// Get device data from database
	result = m_database.GetSelectedDeviceData(current_device);
	if (result) return ET::Collector::Add("Could not not get prepared routes", ET::Collector::GetErrorMessages());

	// Create socket
	TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
	if (result) return ET::Collector::Add("Could not create socket", ET::Collector::GetErrorMessages());

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
	current_device->routing = "";
	result = ExtractInformation(response, current_device);
	if (result) return ET::Collector::Add("Could extract routing from response");

	// update values in database with device data
	result = m_database.update_selected_device_data(current_device);
	if (result) return ET::Collector::Add("Could not update device data of selected router in database", ET::Collector::GetErrorMessages());

	return ROUTER_API_OK;
}

int Vapi::check_channel_number(int num) {
	int result;

	// Device information von Datenbank anfordern
	auto device_info = std::make_unique<device_data>();
	result = m_database.GetSelectedDeviceData(device_info);
	if (result) return ET::Collector::Add("could not acquire device information");

	// Kanalnummer mit anzahl Kanäle in device information vergleichen
	if (num > device_info->source_count) {
		return ET::Collector::Add("Channel number too high. Max value: " + std::to_string(device_info->destination_count - 1));
	}
	else if (num < 0) {
		return ET::Collector::Add("Channel number under 0 not possible. Min value: 0");
	}
	return ROUTER_API_OK;
}