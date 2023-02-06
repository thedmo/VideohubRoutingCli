#include <cli.hpp>

// Usage
void cli::PrintHelp() {
  std::string usage = "Usage:\n";
  for (size_t i = 0; i < m_options.size(); i++) {
    // TODO: add description with examples
    usage += (m_options[i].short_name + "  " + m_options[i].long_name) + '\n';
  }

  std::cout << usage << std::endl;
}

// EVALUATION =====================

int cli::CompareToOptions(const std::string comp_str) {
  for (size_t i = 0; i < m_options.size(); i++) {
    if ((comp_str == m_options[i].short_name) || (comp_str == m_options[i].long_name)) {
      return m_options[i].option_number;
    }
  }

  std::cout << comp_str << " not found." << std::endl;
  return Flags::help;
}

int cli::CheckArgCount(const int argc, const int current_index, std::string &err_msg) {
  if (argc <= (current_index + 1)) {
    err_msg = "Too few arguments.\n";
    return NOT_OK;
  }

  return OK;
}

// CLI METHODS ======================

void cli::AddRouter(int argc, const char *argv[], int &current_argument_index) {
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Argc error: " << m_err_msg << '\n';
    return;
  }
  //   current_argument_index += 1;
  if (Vapi::AddRouter(argv[++current_argument_index]) != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
}

void cli::RemoveRouter() {
  if (Vapi::RemoveSelectedRouter() != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
}

void cli::SelectRouter(int argc, const char *argv[], int &current_argument_index) {
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  };

  if (Vapi::SelectRouter(argv[++current_argument_index]) != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
}

void cli::ListDevices() {
  std::string device_list;

  if (Vapi::GetDevices(device_list) != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }

  std::cout << device_list << std::endl;
};

void cli::RenameSource(int argc, const char *argv[], int &current_argument_index) {
  std::string new_name;
  int channel_number;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    channel_number = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::exception &exception) {
    std::cout << "Error: " << exception.what() << std::endl;
    return;
  }

  // check second Argument
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  new_name = argv[++current_argument_index];

  if (Vapi::RenameSource(channel_number, new_name) != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::ListSources() {
  std::string device_text;

  int result = Vapi::GetSources(device_text);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }

  std::cout << device_text << std::endl;
};

void cli::RenameDestination(int argc, const char *argv[], int &current_argument_index) {
  std::string new_name;
  int channel_number = 0;

  // Check first argument
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    channel_number = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::invalid_argument &exception) {
    std::cout << "Error, not a valid integer: " << exception.what() << std::endl;
    return;
  }

  // check second Argument
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  new_name = argv[++current_argument_index];

  int result = Vapi::RenameDestination(channel_number, new_name);
  if (result) PrintErrors(Vapi::GetErrorMessages());
}

void cli::ListDestinations() {
  std::string destination_list;

  int result = Vapi::GetDestinations(destination_list);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }

  std::cout << destination_list << std::endl;
};

void cli::PrepareNewRoute(int argc, const char *argv[], int &current_argument_index) {
  int temp_destination;
  int temp_source;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    temp_destination = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::invalid_argument &exception) {
    std::cout << "Error, not a valid integer: " << exception.what() << std::endl;
  }

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    temp_source = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::invalid_argument &exception) {
    std::cout << "Error, not a valid integer: " << exception.what() << std::endl;
  }
  if (Vapi::PrepareNewRoute(temp_destination, temp_source) != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::TakePreparedRoutes() {
  if (Vapi::TakePreparedRoutes() != Vapi::ROUTER_API_OK) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::LockRoute(int argc, const char *argv[], int &current_argument_index) {
  int temp_destination;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    temp_destination = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::invalid_argument &exception) {
    std::cout << "Error, not a valid integer: " << exception.what() << std::endl;
  }
  int result = Vapi::LockRoute(temp_destination);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::UnlockRoute(int argc, const char *argv[], int &current_argument_index) {
  int temp_destination;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    temp_destination = std::stoi(argv[++current_argument_index]);
  }
  catch (const std::invalid_argument &exception) {
    std::cout << "Error, not a valid integer: " << exception.what() << std::endl;
  }
  int result = Vapi::UnlockRoute(temp_destination);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::ListRoutes() {
  std::string routes_str;

  int result = Vapi::GetRoutes(routes_str);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }

  std::cout << routes_str << std::endl;
};

void cli::MarkRoutForSaving(int argc, const char *argv[], int &current_argument_index) {
  int destination;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  try {
    // std::cout << "current argument: " << std::stoi(argv[1+ current_argument_index]) << std::endl;
    destination = std::stoi(argv[++current_argument_index]);
  }
  catch (std::exception e) {
    std::cout << e.what() << std::endl;
  }

  int result = Vapi::MarkRouteForSaving(destination);
  if (result) PrintErrors(Vapi::GetErrorMessages());
}

void cli::SaveRouting(int argc, const char *argv[], int &current_argument_index) {
  std::string name;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  name = argv[++current_argument_index];

  int result = Vapi::SaveRoutes(name);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

void cli::ListSavedRoutings() {
  std::string callback;

  int result = Vapi::GetSavedRoutes(callback);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }

  std::cout << callback << std::endl;
};

void cli::LoadRouting(int argc, const char *argv[], int &current_argument_index) {
  std::string routing_name;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  routing_name = argv[++current_argument_index];

  int result = Vapi::LoadRoutes(routing_name);
  if (result) {
    PrintErrors(Vapi::GetErrorMessages());
    return;
  }
};

//===============================

int cli::Evaluate(const int argc, const char *argv[]) {
  if (argc > 1) {
    for (int i = 1; i < argc; ++i) {
      // unsigned int increment = 0;
      int option = CompareToOptions(argv[i]);

      switch (option) {
        case (Flags::add_router): {
            std::cout << "Adding Router" << std::endl;
            AddRouter(argc, argv, i);
            break;
          }
        case (Flags::remove_router): {
            std::cout << "Removing selected router" << std::endl;
            RemoveRouter();
            break;
          }
        case (Flags::select_router): {
            std::cout << "selecting router\n";
            SelectRouter(argc, argv, i);
            break;
          }
        case (Flags::list_devices): {
          // list routers
            std::cout << "listing devices" << '\n';
            ListDevices();
            break;
          }
        case (Flags::rename_source): {
            std::cout << "Renaming source\n";
            RenameSource(argc, argv, i);
            break;
          }
        case (Flags::list_sources): {
            std::cout << "Listing sources\n";
            ListSources();
            break;
          }
        case (Flags::rename_destination): {
            std::cout << "Renaming destination\n";
            RenameDestination(argc, argv, i);
            break;
          }
        case (Flags::list_destinations): {
            std::cout << "Listing destination\n";
            ListDestinations();
            break;
          }
        case (Flags::new_route): {
            std::cout << "Preparing new route\n";
            PrepareNewRoute(argc, argv, i);
            break;
          }
        case (Flags::take_routes): {
            std::cout << "Taking prepared routes\n";
            TakePreparedRoutes();
            break;
          }
        case (Flags::lock_route): {
            std::cout << "locking route\n";
            LockRoute(argc, argv, i);
            break;
          }
        case (Flags::unlock_route): {
            std::cout << "unlocking route\n";
            UnlockRoute(argc, argv, i);
            break;
          }
        case (Flags::list_routes): {
            std::cout << "listing routes\n";
            ListRoutes();
            break;
          }
        case (Flags::mark_route_for_saving): {
            std::cout << "Marking route for saving\n";
            MarkRoutForSaving(argc, argv, i);
            break;
          }
        case (Flags::save_routing): {
            std::cout << "saving routes\n";
            SaveRouting(argc, argv, i);
            break;
          }
        case (Flags::list_saved_routes): {
            std::cout << "Listing saved routings\n";
            ListSavedRoutings();
            break;
          }
        case (Flags::load_routes): {
            std::cout << "Loading saved routes\n";
            LoadRouting(argc, argv, i);
            break;
          }
        case (Flags::help): {
          // help
            PrintHelp();
            break;
          }
        default:
          PrintHelp();
          break;
      }
    }
  }
  else {
 // helping
    std::cout << "No Argument passed." << std::endl;
    PrintHelp();
  }

  return 0;
}

void cli::PrintErrors(std::vector<std::string> err_msgs) {
  for (std::string s : err_msgs) {
    std::cout << s << '\n';
  }
}
