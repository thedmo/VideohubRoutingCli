#include <RouterApi.hpp>

Vapi::Vapi() {
  // vdb m_sqlite;
}

int Vapi::AddRouter(std::string ip, std::string &errmsg) {
  // TODO: make sqlite object member of static class
   vdb m_sqlite;
  if (m_sqlite.check_if_exists(ip)) {
    errmsg = "router with ip: " + ip + " was already added to devices";
    return NOT_OK;
  };

  // TODO: Get data from device

  device_data *new_router = new device_data{
      ip, "'dummy'", 9999, 9999, "'dummy'", "'dummy'", "'dummy'", "'dummy'", "'dummy'"};

  m_sqlite.insert_device_into_db(new_router);

  return OK;
}
int Vapi::SelectRouter(std::string ip, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::RemoveSelectedRouter(std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::GetDevices(std::string &callback, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::RenameSource(int channel_number, const std::string new_name, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::GetSources(std::string &callback, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}

// Switch order of arguments: 1. Channelnumber, 2. New Name
int Vapi::RenameDestination(int channel_number, const std::string new_name, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::GetDestinations(std::string &callback, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::TakePreparedRoutes(std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::LockRoutes(unsigned int destination, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::GetRoutes(std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::SaveRoutes(const std::string destinations, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::GetSavedRoutes(std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}
int Vapi::LoadRoutes(std::string name, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;
}