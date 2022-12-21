#include <RouterApi.hpp>

#include <iostream>

Vapi::Vapi() {
}

std::vector<std::string> Vapi::m_err_msgs;

int Vapi::GetStatus(std::string ip, std::unique_ptr<device_data> &_data) {
  int result;
  std::string response;
  TelnetClient _telnet(ip, VIDEOHUB_TELNET_PORT, response, result);

  if (result != TelnetClient::TELNET_OK)
  {
    for (std::string err : _telnet.GetErrorMessages()) {
      m_err_msgs.push_back("TELNET_CLIENT: " + err);
    }
    return ROUTER_API_NOT_OK;
  }

  // TODO: create method to get information out of response string to fill in router fields
  std::cout << response << std::endl;

  // TODO: remove mockup values
  _data->ip = ip;
  _data->name = "MOCKUP_NAME";
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
    m_err_msgs.push_back("ROUTER_API: Could not Get Status, check trace for reason.");
    return ROUTER_API_NOT_OK;
  }

  result = m_sqlite.check_if_exists(ip);
  if (result != vdb::SQL_OK) {
    m_err_msgs.push_back("ROUTER_API: router with ip: " + ip + " is already in list of known devices");
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

std::vector<std::string> Vapi::GetErrorMessages() {
  return m_err_msgs;
}