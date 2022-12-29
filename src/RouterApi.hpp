#include <SqliteInterface.hpp>
#include <vector>

#include <TelnetClient.hpp>

const int VIDEOHUB_TELNET_PORT = 9990;

class Vapi {
public:
  enum information_type {
    none,
    preamble,
    device,
    inputs_labels,
    outputs_labels,
    routing,
    locks
  };

  Vapi();
  ~Vapi() {};

  static int AddRouter(std::string ip);
  static int SelectRouter(std::string ip, std::string &errmsg);
  static int RemoveSelectedRouter(std::string &errmsg);
  static int GetDevices(std::string &callback, std::string &errmsg);
  static int RenameSource(int channel_number, const std::string new_name, std::string &errmsg);
  static int GetSources(std::string &callback, std::string &errmsg);
  static int RenameDestination(int channel_number, const std::string new_name, std::string &errmsg);
  static int GetDestinations(std::string &callback, std::string &errmsg);
  static int PrepareNewRoute(unsigned int destination, unsigned int source, std::string &errmsg);
  static int TakePreparedRoutes(std::string &errmsg);
  static int LockRoutes(unsigned int destination, std::string &errmsg);
  static int GetRoutes(std::string &errmsg);
  static int SaveRoutes(const std::string destinations, std::string &errmsg);
  static int GetSavedRoutes(std::string &errmsg);
  static int LoadRoutes(std::string name, std::string &errmsg);

  static int GetStatus(std::string ip, std::unique_ptr<device_data> &_data);

  static std::vector<std::string> GetErrorMessages();

  static const int ROUTER_API_NOT_OK = 1;
  static const int ROUTER_API_OK = 0;

private:
  static std::vector<std::string> m_err_msgs;
  static void AddToTrace(std::string);

  static int Vapi::GetInformationType(std::string line, information_type &type);
  static int ExtractInformation(std::string, std::unique_ptr<device_data> &_data);

  static int GetDeviceInformation(std::string, std::unique_ptr<device_data> &_data);
};