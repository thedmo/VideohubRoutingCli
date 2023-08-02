#include <RouterApi.hpp>
#include <iostream>
#include <sstream>

Vapi::Vapi() {
}
vdb Vapi::m_database;

int Vapi::GetInformationType(std::string line, information_type &type) {
  if (line == "PROTOCOL PREAMBLE:") {
    type = information_type::preamble;
  } else if (line == ("VIDEOHUB DEVICE:")) {
    type = information_type::device;
  } else if (line == ("INPUT LABELS:")) {
    type = information_type::inputs_labels;
  } else if (line == ("OUTPUT LABELS:")) {
    type = information_type::outputs_labels;
  } else if (line == ("VIDEO OUTPUT ROUTING:")) {
    type = information_type::routing;
  } else if (line == ("VIDEO OUTPUT LOCKS:")) {
    type = information_type::locks;
  } else if (line == ("CONFIGURATION:")) {
    type = information_type::configuration;
  } else if (line == ("ACK")) {
    type = information_type::ack;
  } else if (line == "END PRELUDE:"){
    type = information_type::end_prelude;
  }
  
  else {
    return AddToTrace("Type could not be extracted from: " + line + ".");
  }
  return Vapi::ROUTER_API_OK;
}

int Vapi::ExtractInformation(std::string info, std::unique_ptr<device_data> &_data) {
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
        return AddToTrace("Could not determine information type");
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
        return AddToTrace("empty information type was passed");
        break;
    }
  }
  return Vapi::ROUTER_API_OK;
}

int Vapi::GetDeviceInformation(std::string line, std::unique_ptr<device_data> &_data) {
  std::stringstream line_stream(line);

  std::string word;

  std::vector<std::string> words;
  while (std::getline(line_stream, word, ':')) {
    words.push_back(word);
  }

  if (words.empty()) return AddToTrace("empty string was given to GetDeviceInformation Function");

  if (words[0].empty() || words[1].empty()) return AddToTrace("empty string in extracted words from line");

  std::string key = words[0];
  std::string value = words[1];
  value.erase(0, 1);

  std::cout << key << " " << value << std::endl;

  if (key.compare("Version") == 0) {
    _data->version = value;
    return Vapi::ROUTER_API_OK;
  } else if (key.compare("Friendly name") == 0) {
    _data->name = value;
    return Vapi::ROUTER_API_OK;
  } else if (key.compare("Video inputs") == 0) {
    _data->source_count = std::stoi(value);
    return Vapi::ROUTER_API_OK;
  } else if (key.compare("Video outputs") == 0) {
    _data->destination_count = std::stoi(value);
    return Vapi::ROUTER_API_OK;
  }

  return Vapi::ROUTER_API_OK;
}

int Vapi::GetStatus(std::string ip, std::unique_ptr<device_data> &_data) {
  int result;
  std::string response;

  TelnetClient _telnet(ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("could not get status...", _telnet.GetErrorMessages());

  result = ExtractInformation(response, _data);
  if (result) return AddToTrace("could not extract information");

  _data->ip = ip;

  return ROUTER_API_OK;
}

int Vapi::AddRouter(std::string ip) {
  std::unique_ptr<device_data> router_data = std::make_unique<device_data>();

  int result = GetStatus(ip, router_data);
  if (result) return AddToTrace("Could not add router...", Vapi::GetErrorMessages());

  result = m_database.check_if_device_exists(ip);
  if (result) return AddToTrace("could not check if device exists... ", vdb::GetErrorMessages());

  result = m_database.insert_device_into_db(router_data);
  if (result) return AddToTrace("could not add router to Storage");

  result = DataHandler::AddDevice(router_data);
  if (result) return AddToTrace("could not add router to Storage");

  return ROUTER_API_OK;
}

int Vapi::SelectRouter(std::string ip) {
  int result = m_database.select_device(ip);
  if (result) return AddToTrace("could not select router...", vdb::GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::RemoveSelectedRouter() {
  int result = m_database.remove_selected_device_from_db();
  if (result) return AddToTrace("removing device did not work.", vdb::GetErrorMessages());

  return ROUTER_API_OK;
}
int Vapi::GetDevices(std::string &callback) {
  int result = m_database.GetDevices(callback);
  if (result) return AddToTrace("could not get Device listings: ", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::RenameSource(int channel_number, const std::string new_name) {
  int result;

  // Prüfen, ob Kanal Nummer nicht zu tief oder zu hoch --> neue private Funktion dafür erstellen
  result = check_channel_number(channel_number);
  if (result) return AddToTrace("Problem with channel number: ");

  // get Device information for IP
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) AddToTrace("Could not get device data: ");

  // Socket erstellen und öffnen
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not establish conection to device:");

  // Nachricht zusammenstellen und an Videohub schicken
  std::string message = "INPUT LABELS:\n" + std::to_string(channel_number) + " " + new_name + "\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("sending message did not work: ");

  // Liste mit Sourcenamen anfordern
  message = "INPUT LABELS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("sending message did not work: ");

  // get last response from socket and extract information
  response = tc.GetLastDataDump();
  current_device->source_labels = "";
  result = ExtractInformation(response, current_device);
  if (result) return AddToTrace("Could extract routing from response");

  // update values in database with device data
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::GetSources(std::string &callback) {
  int result;

  // get Device information for IP
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) AddToTrace("Could not get device data: ");

  // Socket erstellen und öffnen
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not establish conection to device:");

  // Nachrich an socket schicken
  std::string message = "INPUT LABELS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("could not send message to device ");

  // Letzte Anwort von socket anfordern und in string speichern
  callback = tc.GetLastDataDump();

  return ROUTER_API_OK;
}

int Vapi::RenameDestination(int channel_number, const std::string new_name) {
  int result;

  // Prüfen, ob Kanal Nummer nicht zu tief oder zu hoch --> neue private Funktion dafür erstellen
  result = check_channel_number(channel_number);
  if (result) return AddToTrace("Problem with channel number: ");

  // get Device information for IP
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) AddToTrace("Could not get device data: ");

  // Socket erstellen und öffnen
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not establish conection to device:");

  // Nachricht zusammenstellen und an Videohub schicken
  std::string message = "OUTPUT LABELS:\n" + std::to_string(channel_number) + " " + new_name + "\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("sending message did not work: ");

  // Liste mit Sourcenamen anfordern
  message = "OUTPUT LABELS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("sending message did not work: ");

  // get last response from socket and extract information
  response = tc.GetLastDataDump();
  current_device->destination_labels = "";
  result = ExtractInformation(response, current_device);
  if (result) return AddToTrace("Could extract data from response");

  // update values in database with device data
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::GetDestinations(std::string &callback) {
  int result;

  // get Device information for IP
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) AddToTrace("Could not get device data: ");

  // Socket erstellen und öffnen
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not establish conection to device:");

  // Nachrich an socket schicken
  std::string message = "OUTPUT LABELS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("could not send message to device ");

  // Letzte Anwort von socket anfordern und in string speichern
  callback = tc.GetLastDataDump();

  return ROUTER_API_OK;
}

int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source) {
  int result = m_database.add_to_prepared_routes(destination, source);
  if (result) return AddToTrace("Could not add new route", vdb::GetErrorMessages());
  return ROUTER_API_OK;
}

int Vapi::TakePreparedRoutes() {
  std::string response;
  int result;
  std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

  // Get device data from database
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("Could not not get prepared routes", m_database.GetErrorMessages());

  // Create socket send ip from acquired device data
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket", tc.GetErrorMessages());

  // send Command to take routes to connected device
  std::string msg = "VIDEO OUTPUT ROUTING:\n" + current_device->prepared_routes + '\n';
  result = tc.SendMsgToServer(msg);
  if (result) return AddToTrace("Could not take prepared routes", tc.GetErrorMessages());

  // reset prepared routes in device_data
  // result = m_database.clean_prepared_routes();
  current_device->prepared_routes = "";
  // if (result) return AddToTrace("could not reset prepared routes", m_database.GetErrorMessages());

  // get current routes from connected device (response gets saved in member variable)
  msg = "VIDEO OUTPUT ROUTING:\n\n";
  result = tc.SendMsgToServer(msg);
  if (result) return AddToTrace("Could not get routing from device", tc.GetErrorMessages());

  // get response from member variable and fill in routing of device data
  response = tc.GetLastDataDump();
  current_device->routing = "";
  result = ExtractInformation(response, current_device);
  if (result) return AddToTrace("Could extract routing from response");

  // update values in database with device data
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::LockRoute(unsigned int destination) {
  int result;

  // Check Channelnumber
  result = check_channel_number(destination);
  if (result) return AddToTrace("something wrong with the channelnumber: ");

  // get device data
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("could not get device data: ");

  // create socket
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket", tc.GetErrorMessages());

  // generate message and send to device
  std::string message = "VIDEO OUTPUT LOCKS:\n" + std::to_string(destination) + " L\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("Could not send message", tc.GetErrorMessages());

  // get lock information from device
  message = "VIDEO OUTPUT LOCKS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("Could not send message", tc.GetErrorMessages());

  // update device data in database
  current_device->locks = "";
  response = tc.GetLastDataDump();
  result = ExtractInformation(response, current_device);
  m_database.update_selected_device_data(current_device);

  return ROUTER_API_OK;
}

int Vapi::UnlockRoute(unsigned int destination) {
  int result;

  // Check Channelnumber
  result = check_channel_number(destination);
  if (result) return AddToTrace("something wrong with the channelnumber: ");

  // get device data
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("could not get device data: ");

  // create socket
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket", tc.GetErrorMessages());

  // generate message and send to device
  std::string message = "VIDEO OUTPUT LOCKS:\n" + std::to_string(destination) + " U\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("Could not send message", tc.GetErrorMessages());

  // get lock information from device
  message = "VIDEO OUTPUT LOCKS:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("Could not send message", tc.GetErrorMessages());

  // update device data in database
  current_device->locks = "";
  response = tc.GetLastDataDump();
  result = ExtractInformation(response, current_device);
  m_database.update_selected_device_data(current_device);

  return ROUTER_API_OK;
}

int Vapi::GetRoutes(std::string &callback) {
  int result;

  // get Device information for IP
  auto current_device = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) AddToTrace("Could not get device data: ");

  // Socket erstellen und öffnen
  std::string response;
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not establish conection to device:");

  // Nachrich an socket schicken
  std::string message = "VIDEO OUTPUT ROUTING:\n\n";
  result = tc.SendMsgToServer(message);
  if (result) return AddToTrace("could not send message to device ");

  // Letzte Anwort von socket anfordern und in string speichern
  callback = tc.GetLastDataDump();

  // update device data in database
  current_device->routing = "";
  response = tc.GetLastDataDump();
  result = ExtractInformation(response, current_device);
  m_database.update_selected_device_data(current_device);

  return ROUTER_API_OK;
}

int Vapi::MarkRouteForSaving(int destination) {
  std::string response;
  int result;
  std::unique_ptr<device_data> current_device = std::make_unique<device_data>();
    // std::cout << "current argument: " << destination << std::endl;
  // Get device data from database
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("Could not not get device data from database: ", m_database.GetErrorMessages());

  // Create socket
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket: ", tc.GetErrorMessages());

  // get current routes from connected device (response gets saved in member variable)
  std::string msg = "VIDEO OUTPUT ROUTING:\n\n";
  result = tc.SendMsgToServer(msg);
  if (result) return AddToTrace("Could not get routing from device: ", tc.GetErrorMessages());

  // get response from member variable and fill in routing of device data
  response = tc.GetLastDataDump();
  current_device->routing = "";
  result = ExtractInformation(response, current_device);
  if (result) return AddToTrace("Could not extract routing from response: ");

  // update values in database with device data
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  result = m_database.mark_route_for_saving(destination);
  if (result) return AddToTrace("could not mark route: ", vdb::GetErrorMessages());

  return ROUTER_API_OK;
}

// TODO Hier weiter
int Vapi::MarkRoutes2(std::vector<int> destinations) {
  device_data device;



  return ROUTER_API_OK;
}

int Vapi::SaveRoutes(std::string routing_name) {
  // Get marked routes of selected device from database
  int result;
  std::unique_ptr<device_data> current_device = std::make_unique<device_data>();

  // Get device data with prepared routes from database
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("Could not not get marked routes", m_database.GetErrorMessages());

  if (current_device->marked_for_saving.empty()) return AddToTrace("No marked route to save present in selected device");

  // Insert new row for routing of selected device in routings table: ip, name of routing, routing as text
  result = m_database.save_routing(routing_name, current_device);
  if (result) return AddToTrace("Could not save routing: ", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

// TODO
int Vapi::GetSavedRoutes(std::string &callback) {
  int result;

  // get routing
  result = m_database.get_saved_routings(callback);
  if (result) return AddToTrace("Could not not get saved routings: ", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

int Vapi::LoadRoutes(std::string name) {
  std::unique_ptr<device_data> current_device = std::make_unique<device_data>();
  std::string response;
  std::string routes;
  int result;

  // send request to database api to get saved routes by name
  result = m_database.get_routing_by_name(name, routes);
  if (result) return AddToTrace("could not get saved routes:");

  // Get device data from database
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("Could not not get prepared routes", m_database.GetErrorMessages());

  // Create socket
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket", tc.GetErrorMessages());

  // send Command to take routes to connected device
  std::string route_command = "VIDEO OUTPUT ROUTING:\n" + routes + '\n';
  result = tc.SendMsgToServer(route_command);
  if (result) return AddToTrace("Could not load routes", tc.GetErrorMessages());

  // get current routes from connected device (response gets saved in member variable)
  route_command = "VIDEO OUTPUT ROUTING:\n\n";
  result = tc.SendMsgToServer(route_command);
  if (result) return AddToTrace("Could not get routing from device", tc.GetErrorMessages());

  // get response from member variable and fill in routing of device data
  response = tc.GetLastDataDump();
  current_device->routing = "";
  result = ExtractInformation(response, current_device);
  if (result) return AddToTrace("Could extract routing from response");

  // update values in database with device data
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}

// Error Handling
std::vector<std::string> Vapi::m_err_msgs;
int Vapi::AddToTrace(std::string s) {
  m_err_msgs.push_back("ROUTER_API: " + s);
  return 1;
}

int Vapi::AddToTrace(std::string err, std::vector<std::string> err_list) {
  m_err_msgs.push_back("ROUTER_API: " + err);
  for (std::string e : err_list) {
    m_err_msgs.push_back(e);
  }
  return ROUTER_API_NOT_OK;
}

std::vector<std::string> Vapi::GetErrorMessages() {
  std::vector<std::string> temp = m_err_msgs;
  m_err_msgs.clear();
  return temp;
}

int Vapi::check_channel_number(int num) {
  int result;

  // Device information von Datenbank anfordern
  auto device_info = std::make_unique<device_data>();
  result = m_database.GetSelectedDeviceData(device_info);
  if (result) return AddToTrace("could not acquire device information");

  // Kanalnummer mit anzahl Kanäle in device information vergleichen
  if (num > device_info->source_count) {
    return AddToTrace("Channel number too high. Max value: " + std::to_string(device_info->destination_count - 1));
  } else if (num < 0) {
    return AddToTrace("Channel number under 0 not possible. Min value: 0");
  }
  return ROUTER_API_OK;
}