#include <SqliteInterface.hpp>
#include <vector>

class Vapi {
 public:
  Vapi(){};
  ~Vapi(){};

  static int AddRouter(std::string ip, std::string &errmsg);
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

  static const int NOT_OK = 1;
  static const int OK = 0;
 private:
};