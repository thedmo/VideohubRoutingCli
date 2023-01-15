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
  } else if (line == ("ACK")) {
    type = information_type::ack;
  } else {
    return AddToTrace("Type could not be extracted from: " + line + ".");
  }
  return Vapi::ROUTER_API_OK;
}

int Vapi::ExtractInformation(std::string info, std::unique_ptr<device_data> &_data) {
  information_type type = information_type::none;

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
        break;
      case information_type::outputs_labels:
        _data->destination_labels += line + "\n";
        break;
      case information_type::routing:
        _data->routing += line + "\n";
        break;
      case information_type::locks:
        _data->locks += line + "\n";
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

  m_database.insert_device_into_db(router_data);
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
  return AddToTrace("Not implemented yet");
}
int Vapi::GetSources(std::string &callback) {
  return AddToTrace("Not Implemented yet");
}

int Vapi::RenameDestination(int channel_number, const std::string new_name) {
  return AddToTrace("Not implemented yet");
}
int Vapi::GetDestinations(std::string &callback) {
  return AddToTrace("Not implemented yet");
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

  // Get device data with prepared routes from database
  result = m_database.GetSelectedDeviceData(current_device);
  if (result) return AddToTrace("Could not not get prepared routes", m_database.GetErrorMessages());

  // Create socket
  TelnetClient tc(current_device->ip, VIDEOHUB_TELNET_PORT, response, result);
  if (result) return AddToTrace("Could not create socket", tc.GetErrorMessages());

  // send Command to take routes to connected device
  std::string msg = "VIDEO OUTPUT ROUTING:\n" + current_device->prepared_routes + '\n';
  result = tc.SendMsgToServer(msg);
  if (result) return AddToTrace("Could not take prepared routes", tc.GetErrorMessages());

  // reset entry for prepared routes in database
  result = m_database.clean_prepared_routes();
  if (result) return AddToTrace("could not reset prepared routes", m_database.GetErrorMessages());

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
  // TODO update values of selected router
  result = m_database.update_selected_device_data(current_device);
  if (result) return AddToTrace("Could not update device data of selected router in database", m_database.GetErrorMessages());

  return ROUTER_API_OK;
}
int Vapi::LockRoutes(unsigned int destination) {
  return AddToTrace("ROUTER_API: Not implemented yet");
}
int Vapi::GetRoutes() {
  return AddToTrace("ROUTER_API: Not implemented yet");
}
int Vapi::SaveRoutes(const std::string destinations) {
  return AddToTrace("ROUTER_API: Not implemented yet");
}
int Vapi::GetSavedRoutes() {
  return AddToTrace("ROUTER_API: Not implemented yet");
}
int Vapi::LoadRoutes(std::string name) {
  return AddToTrace("ROUTER_API: Not implemented yet");
}

// Error Handling
std::vector<std::string> Vapi::m_err_msgs;
int Vapi::AddToTrace(std::string s) {
  m_err_msgs.push_back("ROUTER_API: " + s);
  return 1;
}

// TODO reduce errorcheck to if(result) return AddToTrace(err, err_list);
int Vapi::AddToTrace(std::string err, std::vector<std::string> err_list) {
  m_err_msgs.push_back("ROUTER_API: " + err);
  for (std::string e : err_list) {
    m_err_msgs.push_back(e);
  }
  return ROUTER_API_NOT_OK;
}

std::vector<std::string> Vapi::GetErrorMessages() {
  return m_err_msgs;
}