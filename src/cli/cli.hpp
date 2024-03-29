#include <RouterApi.hpp>
#include <ErrorTrace.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

using namespace RouterModel;

class cli {
public:
  cli() {};
  ~cli() {};

  int Evaluate(const int argc, const char *argv[]);

private:
 // TODO: Add desciption with examples
  struct Option {
    int option_number;
    std::string short_name, long_name;
  };

  enum Flags {
    value,
    help,
    add_router,
    remove_router,
    select_router,
    list_devices,
    rename_source,
    list_sources,
    rename_destination,
    list_destinations,
    new_route,
    take_routes,
    lock_route,
    unlock_route,
    list_routes,
    //mark_route_for_saving_old,
    mark_route_for_saving,
    save_routing,
    list_saved_routes,
    load_routes
  };

  std::vector<Option> m_options = {
      {Flags::help, "-h", "--help"},
      {Flags::add_router, "-a", "--add_router"},
      {Flags::remove_router, "-rm", "--remove_router"},
      {Flags::select_router, "-s", "--select_router"},
      {Flags::list_devices, "-LD", "--list_devices"},
      {Flags::rename_source, "-rs", "--rename_source"},
      {Flags::list_sources, "-Ls", "--list_sources"},
      {Flags::rename_destination, "-rd", "--rename_destination"},
      {Flags::list_destinations, "-Ld", "--list_destinations"},
      {Flags::new_route, "-nr", "--new_route"},
      {Flags::take_routes, "-t", "--take"},
      {Flags::lock_route, "-l", "--lock"},
      {Flags::unlock_route, "-u", "--unlock"},
      {Flags::list_routes, "-Lr", "--list_routes"},
      //{Flags::mark_route_for_saving_old, "-m2", "--mark2"},
      {Flags::mark_route_for_saving, "-m", "--mark"},
      {Flags::save_routing, "-sr", "--save_routes"},
      {Flags::list_saved_routes, "-lsr", "--list_saved_routings"},
      {Flags::load_routes, "-lr", "--load_routes"} };

  void PrintHelp();
  int CompareToOptions(std::string comp_str);
  int CheckArgCount(const int argc, const int current_index, std::string &err_msg);

  void [[deprecated("This behaviour only i valid for one argument at a time an thus very slow. use new method instead.")]] invoke_methods(const int argc, const char *argv[]);
  void invoke_methods_2(const int argc, const char *argv[]);

  void AddRouter(int argc, const char *argv[], int &current_argument_index);
  void RemoveRouter();
  void SelectRouter(int argc, const char *argv[], int &current_argument_index);
  void ListDevices();
  void RenameSource(int argc, const char *argv[], int &current_argument_index);
  void ListSources();
  void RenameDestination(int argc, const char *argv[], int &current_argument_index);
  void ListDestinations();
  void PrepareNewRoute(int argc, const char *argv[], int &current_argument_index);
  void TakePreparedRoutes();
  void LockRoute(int argc, const char *argv[], int &current_argument_index);
  void UnlockRoute(int argc, const char *argv[], int &current_argument_index);
  void ListRoutes();
  //void MarkRoutForSavingOld(int argc, const char *argv[], int &current_argument_index);
  void MarkRoutForSaving(int argc, const char *argv[], int &current_argument_index);
  void SaveRouting(int argc, const char *argv[], int &current_argument_index);
  void ListSavedRoutings();
  void LoadRouting(int argc, const char *argv[], int &current_argument_index);

  std::string m_err_msg;
  int m_result;

  static const int NOT_OK = 1;
  static const int OK = 0;

  void PrintErrors(std::vector<std::string>);
};
