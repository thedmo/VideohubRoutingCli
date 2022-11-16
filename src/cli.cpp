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
  //   std::cout << "Options count: " << m_options.size() << std::endl;

  //   std::cout << "Option: " << comp_str << '\n';

  for (size_t i = 0; i < m_options.size(); i++) {
    if ((comp_str == m_options[i].short_name) || (comp_str == m_options[i].long_name)) {
      //   std::cout << " , Option number: " << std::to_string(m_options[i].option_number) << std::endl;
      return m_options[i].option_number;
    }

    // std::cout << m_options[i].option_number << ", " << m_options.at(i).short_name << ", " << m_options.at(i).long_name << ", " << std::endl;
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
  if (Vapi::AddRouter(argv[++current_argument_index], m_err_msg) != Vapi::OK) {
    std::cout << "Add Router Error: " << m_err_msg << '\n';
    return;
  }
}

void cli::RemoveRouter() {
  if (Vapi::RemoveSelectedRouter(m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
}

void cli::SelectRouter(int argc, const char *argv[], int &current_argument_index) {
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  if (Vapi::SelectRouter(argv[++current_argument_index], m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::ListDevices() {
  std::string device_list;

  if (Vapi::GetDevices(device_list, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  std::cout << device_list << std::endl;
};

void cli::RenameSource(int argc, const char *argv[], int &current_argument_index) {
  std::string new_name;
  int channel_number;

  // Check first argument
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  new_name = argv[++current_argument_index];

  // check second Argument
  if (CheckArgCount(argc, current_argument_index + 1, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  channel_number = std::stoi(argv[++current_argument_index]);

  if (Vapi::RenameSource(new_name, channel_number, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::ListSources(){

};

void cli::RenameDestination(int argc, const char *argv[], int &current_argument_index) {
  std::string new_name;
  int channel_number;

  // Check first argument
  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  new_name = argv[++current_argument_index];

  // check second Argument
  if (CheckArgCount(argc, current_argument_index + 1, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  // TODO: Check, if integer
  channel_number = std::stoi(argv[++current_argument_index]);

  if (Vapi::RenameDestination(new_name, channel_number, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  };
}

void cli::ListDestinations() {
  std::string destination_list;
  if (Vapi::GetDestinations(destination_list, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
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
  // TODO: Check, if integer
  temp_destination = std::stoi(argv[++current_argument_index]);

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
  // TODO: Check, if integer
  temp_source = std::stoi(argv[++current_argument_index]);

  if (Vapi::PrepareNewRoute(temp_destination, temp_source, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::TakePreparedRoutes() {
  if (Vapi::TakePreparedRoutes(m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::LockRoute(int argc, const char *argv[], int &current_argument_index) {
  int temp_destination;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
  // TODO: Check, if integer
  temp_destination = std::stoi(argv[++current_argument_index]);

  if (Vapi::LockRoutes(temp_destination, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::ListRoutes() {
  if (Vapi::GetRoutes(m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::SaveRouting(int argc, const char *argv[], int &current_argument_index) {
  std::string temp_routes;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  temp_routes = argv[++current_argument_index];

  if (Vapi::SaveRoutes(temp_routes, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::ListSavedRoutings() {
  if (Vapi::GetSavedRoutes(m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }
};

void cli::LoadRouting(int argc, const char *argv[], int &current_argument_index) {
  std::string temp_argument;

  if (CheckArgCount(argc, current_argument_index, m_err_msg) != OK) {
    std::cout << "Error: " << m_err_msg << '\n';
    return;
  }

  temp_argument = argv[++current_argument_index];

  if (Vapi::LoadRoutes(temp_argument, m_err_msg) != Vapi::OK) {
    std::cout << "Error: " << m_err_msg << '\n';
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
        case (Flags::list_routes): {
          std::cout << "listing routes\n";
          ListRoutes();
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
  } else {
    // helping
    std::cout << "No Argument passed." << std::endl;
    PrintHelp();
  }

  return 0;
}
