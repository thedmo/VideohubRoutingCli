#pragma once

#include <filesystem>
#include <config.h>

class VhubMockup {


public:
	static void StartServer() {
		std::string path = VHUB_MOCK_DIR;
		std::string mock_path = path + "/BmdVideoHub.exe";
		std::string command = "start " + mock_path;

		std::system(command.c_str());
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
};