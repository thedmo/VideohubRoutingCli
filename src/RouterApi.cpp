#include <RouterApi.hpp>
#include <sstream>

#include <iostream>

Vapi::Vapi() {
}

int Vapi::GetInformationType(std::string line, information_type &type) {

  if (line == "PROTOCOL PREAMBLE:")
  {
    type = information_type::preamble;
  }
  else if (line == ("VIDEOHUB DEVICE:"))
  {
    type = information_type::device;
  }
  else if (line == ("INPUT LABELS:"))
  {
    type = information_type::inputs_labels;
  }
  else if (line == ("OUTPUT LABELS:"))
  {
    type = information_type::outputs_labels;
  }
  else if (line == ("VIDEO OUTPUT ROUTING:"))
  {
    type = information_type::routing;
  }
  else if (line == ("VIDEO OUTPUT LOCKS:"))
  {
    type = information_type::locks;
  }
  else {
    AddToTrace("Type could not be extracted from: " + line);
    return Vapi::ROUTER_API_NOT_OK;
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
    if (line.empty())
    {
      type = information_type::none;
      continue;
    }

    //determine information type
    if (type == information_type::none)
    {
      result = GetInformationType(line, type);

      if (result != Vapi::ROUTER_API_OK)
      {
        AddToTrace("Could not determine information type");
        return Vapi::ROUTER_API_NOT_OK;
      }
      continue;
    }

    // extract information
    switch (type)
    {
      case information_type::preamble:
        result = GetDeviceInformation(line, _data);
        break;
      case information_type::device:
        result = GetDeviceInformation(line, _data);
        break;
      case information_type::inputs_labels:
        _data->source_labels += line + "\\n";
        break;
      case information_type::outputs_labels:
        _data->destination_labels += line + "\\n";
        break;
      case information_type::routing:
        _data->routing += line + "\\n";
        break;
      case information_type::locks:
        _data->locks += line + "\\n";
        break;

      default:
        AddToTrace("empty information type was passed");
        return Vapi::ROUTER_API_NOT_OK;
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

  if (words.empty())
  {
    AddToTrace("empty string was given to GetDeviceInformation Function");
    return Vapi::ROUTER_API_NOT_OK;
  }

  if (words[0].empty() || words[1].empty())
  {
    AddToTrace("empty string in extracted words from line");
    return Vapi::ROUTER_API_NOT_OK;
  }

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


int Vapi::GetStatus(std::string ip, std::unique_ptr<device_data> &_data) {
  int result;
  std::string response;
  TelnetClient _telnet(ip, VIDEOHUB_TELNET_PORT, response, result);

  if (result != TelnetClient::TELNET_OK)
  {
    for (std::string err : _telnet.GetErrorMessages()) {
      AddToTrace(err);
    }
    return ROUTER_API_NOT_OK;
  }

  result = ExtractInformation(response, _data);
  if (result != Vapi::ROUTER_API_OK)
  {
    AddToTrace("could not extract information");
    return Vapi::ROUTER_API_NOT_OK;
  }

  _data->ip = ip;

  return ROUTER_API_OK;
}






int Vapi::AddRouter(std::string ip) {

  std::unique_ptr<device_data> router_data = std::make_unique<device_data>();

  // TODO: make sqlite object member of static class
  vdb m_sqlite;

  int result = GetStatus(ip, router_data);
  if (result != ROUTER_API_OK)
  {
    for (std::string s : Vapi::GetErrorMessages()) {
      m_err_msgs.push_back(s);
    }
    return ROUTER_API_NOT_OK;
  }

  result = m_sqlite.check_if_exists(ip);
  if (result != vdb::SQL_OK) {
    for (std::string s : vdb::GetErrorMessages())
    {
      m_err_msgs.push_back(s);
    }
    return ROUTER_API_NOT_OK;
  }

  m_sqlite.insert_device_into_db(router_data);
  return ROUTER_API_OK;
}

// TODO select router in database
int Vapi::SelectRouter(std::string ip, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}

// TODO Remove router from database
int Vapi::RemoveSelectedRouter(std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::GetDevices(std::string &callback, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::RenameSource(int channel_number, const std::string new_name, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::GetSources(std::string &callback, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}

int Vapi::RenameDestination(int channel_number, const std::string new_name, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::GetDestinations(std::string &callback, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::TakePreparedRoutes(std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::LockRoutes(unsigned int destination, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::GetRoutes(std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::SaveRoutes(const std::string destinations, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::GetSavedRoutes(std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
int Vapi::LoadRoutes(std::string name, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}

//Error Handling
std::vector<std::string> Vapi::m_err_msgs;
void Vapi::AddToTrace(std::string s) {
  m_err_msgs.push_back("ROUTER_API: " + s);
}

std::vector<std::string> Vapi::GetErrorMessages() {
  return m_err_msgs;
}