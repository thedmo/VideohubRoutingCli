#include <RouterApi.hpp>

int Vapi::AddRouter(std::string ip, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;

    return OK;
}
int Vapi::SelectRouter(std::string ip, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::RemoveSelectedRouter(std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::GetDevices(std::string &callback, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::RenameSource(const std::string new_name, int channel_number, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::GetSources(std::string &callback, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::RenameDestination(const std::string new_name, int channel_number, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::GetDestinations(std::string &callback, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::PrepareNewRoute(unsigned int destination, unsigned int source, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::TakePreparedRoutes(std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::LockRoutes(unsigned int destination, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::GetRoutes(std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::SaveRoutes(const std::vector<int> destinations, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::GetSavedRoutes(std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}
int Vapi::LoadRoutes(std::string name, std::string &errmsg)
{
    errmsg = "Not implemented yet";
    return NOT_OK;
}