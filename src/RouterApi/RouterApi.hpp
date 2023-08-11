#include <SqliteInterface.hpp>
#include <DataHandler.hpp>
#include <vector>

#include <TelnetClient.hpp>
#include <DeviceData.hpp>
#include <ErrorTrace.hpp>
#include <iostream>
#include <sstream>

const int VIDEOHUB_TELNET_PORT = 9990;

namespace RouterModel {

class Converter {
private:
	static int GetLineContents(std::string line, std::vector<std::string>& lineContentVec) {
		int result = 0;
		std::string word;

		if (line.back() == '\n') {
			line.pop_back();
		}
		std::stringstream line_stream(line);

		while (std::getline(line_stream, word, ' ')) {
			lineContentVec.push_back(word);
		}
		if (lineContentVec.empty()) return 1;

		return 0;
	}


public:
	static int RouteLineToRoutePair(std::string line, std::pair<int, int>& route) {
		int result = 0;
		std::vector<std::string> words;
		result = GetLineContents(line, words);
		if (result) return 1;

		if (words.size() < 2) return 1;

		route.first = std::stoi(words[0]);
		route.second = std::stoi(words[1]);

		return 0;
	}

	static int StringLineToData(std::string line, std::string& lineData) {
		int result = 0;
		std::vector<std::string> words;
		result = GetLineContents(line, words);
		if (result) return 1;

		if (words.size() < 2) return 1;

		for (size_t i = 1; i < words.size(); i++)
		{
			lineData += words[i];

			if (i >= words.size()-1) break;

			lineData += " ";
		}

		return 0;
	}
};

class Vapi {
public:
	enum information_type {
		none,
		preamble,
		device,
		inputs_labels,
		outputs_labels,
		routing,
		locks,
		configuration,
		ack,
		end_prelude
	};

	Vapi();
	~Vapi() {};

	static int AddRouter(std::string ip);
	static int SelectRouter(std::string ip);
	static int RemoveSelectedRouter();
	static int GetDevicesList(std::vector<std::string> &entries);
	static int RenameSource(int channel_number, const std::string new_name);
	static int GetSourcesList(std::vector<std::string>& sourcesList);
	static int GetDestinations(std::vector<std::string>& destinationsList);
	static int GetRoutes(std::vector<std::pair<int,int>>& routesList);
	static int RenameDestination(int channel_number, const std::string new_name);
	static int PrepareNewRoute(unsigned int destination, unsigned int source);
	static int TakePreparedRoutes();
	static int LockRoute(unsigned int destination);
	static int UnlockRoute(unsigned int destination);
	static int MarkRoutes2(std::vector<int> destinations);
	static int SaveRoutes(std::string routing_name);
	static int GetSavedRoutes(std::vector < std::pair<std::string, std::vector<std::pair<int, int>>>> &routingsList);
	static int LoadRoutes(std::string name);

	static int GetStatus(std::string ip, std::unique_ptr<device_data>& _data);

	static std::vector<std::string> GetErrorMessages();

	static const int ROUTER_API_NOT_OK = 1;
	static const int ROUTER_API_OK = 0;

private:

	static int GetInformationType(std::string line, information_type& type);
	static int ExtractInformation(std::string, std::unique_ptr<device_data>& _data);

	static int GetDeviceInformation(std::string, std::unique_ptr<device_data>& _data);

	static std::pair<int, int> GetRouteFromDestination(int destination, std::unique_ptr<device_data>& deviceData);
};

} // RouterModel