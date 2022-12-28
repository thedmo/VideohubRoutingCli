#include <RouterApi.hpp>
#include <sstream>

// #include <iostream>

Vapi::Vapi() {
}

int Vapi::GetInformationType(std::string line, information_type &type) {

  // std::cout << line << std::endl;

  if (line.find("PROTOCOL PREAMBLE:"))
  {
    type = information_type::preamble;
  }
  else if (line.find("VIDEOHUB DEVICE:"))
  {
    type = information_type::device;
  }
  else if (line.find("INPUT LABELS:"))
  {
    type = information_type::inputs_labels;
  }
  else if (line.find("OUTPUT LABELS:"))
  {
    type = information_type::outputs_labels;
  }
  else if (line.find("VIDEO OUTPUT ROUTING:"))
  {
    type = information_type::routing;
  }
  else if (line.find("VIDEO OUTPUT LOCKS:"))
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
      int result = GetInformationType(line, type);

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
        // TODO GetDeviceInformation()
        break;
      case information_type::device:
        // TODO GetDeviceInformation()
        break;
      case information_type::inputs_labels:
        // TODO GetInputLabels()
        break;
      case information_type::outputs_labels:
      // TODO GetOutputLabels()
        break;
      case information_type::routing:
      // TODO GetRouting()
        break;
      case information_type::locks:
      // TODO GetLocks()
        break;

      default:
        AddToTrace("empty information type was passed");
        return Vapi::ROUTER_API_NOT_OK;
        break;
    }
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

  // TODO: create method to get information out of response string to fill in router fields
  // std::cout << response << std::endl;
  result = ExtractInformation(response, _data);
  if (result != Vapi::ROUTER_API_OK)
  {
    AddToTrace("could not extract information");
    return Vapi::ROUTER_API_NOT_OK;
  }

  // TODO: remove mockup values
  _data->ip = ip;
  _data->name = "MOCKUP_NAME";
  _data->version = "2.586735892";
  _data->source_count = 999;
  _data->destination_count = 999;
  _data->source_labels = "SOURCE_LABELS";
  _data->destination_labels = "DESTINATION_LABELS";
  _data->routing = "ROUTING";
  _data->locks = "LOCKS";

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
int Vapi::SelectRouter(std::string ip, std::string &errmsg) {
  errmsg = "ROUTER_API: Not implemented yet";
  return ROUTER_API_NOT_OK;
}
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

// Switch order of arguments: 1. Channelnumber, 2. New Name
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