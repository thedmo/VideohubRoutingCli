#include <RouterApi.hpp>

int Vapi::AddRouter(std::string ip, std::string &errmsg) {
  errmsg = "Not implemented yet";
  return NOT_OK;

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