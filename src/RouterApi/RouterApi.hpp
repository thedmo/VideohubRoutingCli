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

	/// <summary>
	/// Gets Information of router by ip and stores it into dataobject
	/// </summary>
	/// <param name="ip">ip as string</param>
	/// <param name="_data">reference to deviceData Object</param>
	/// <returns>int; 0 = OK</returns>
	static int GetStatus(std::string ip, std::unique_ptr<device_data>& _data);

	/// <summary>
	/// Adds a Router by ip address
	/// </summary>
	/// <param name="ip">ip address as string</param>
	/// <returns>int; 0 = ok</returns>
	static int AddRouter(std::string ip);

	/// <summary>
	/// Selects a router from all devices to work on by ip
	/// </summary>
	/// <param name="ip">ip as string</param>
	/// <returns>int; 0 = OK</returns>
	static int SelectRouter(std::string ip);

	/// <summary>
	/// Removes selected Device from storage if it exists
	/// </summary>
	/// <returns>int; 0 = ok</returns>
	static int RemoveSelectedRouter();

	/// <summary>
	/// Gets List of entries in Storage. stores informmation into list of formattet strings
	/// </summary>
	/// <param name="entries">reference to list of strings</param>
	/// <returns>int; 0 = ok</returns>
	static int GetDevicesList(std::vector<std::string> &entries);

	/// <summary>
	/// Rename sourcechannel from connected device
	/// </summary>
	/// <param name="channel_number">number of channel</param>
	/// <param name="new_name">new name for channel</param>
	/// <returns>int; 0 = OK</returns>
	static int RenameSource(int channel_number, const std::string new_name);

	/// <summary>
	/// Rename destination channel of selected router
	/// </summary>
	/// <param name="channel_number">number of channel</param>
	/// <param name="new_name">new name for channel</param>
	/// <returns>int; 0 = ok</returns>
	static int RenameDestination(int channel_number, const std::string new_name);
	
	/// <summary>
	/// Gets List of sources from selected device
	/// </summary>
	/// <param name="sourcesList">vector of strings</param>
	/// <returns>int; 0 = ok</returns>
	static int GetSourcesList(std::vector<std::string>& sourcesList);
	
	/// <summary>
	/// Gets List of destinations from selected device
	/// </summary>
	/// <param name="destinationsList">vector of strings</param>
	/// <returns>int; 0 = ok</returns>
	static int GetDestinations(std::vector<std::string>& destinationsList);

	/// <summary>
	/// prepares new route and adds ist to list of prepared routes. use Take to send new routes to device
	/// </summary>
	/// <param name="destination">number of destionation channel</param>
	/// <param name="source">number if sourcechannel</param>
	/// <returns>int; 0 = OK</returns>
	static int PrepareNewRoute(unsigned int destination, unsigned int source);

	/// <summary>
	/// Send all prepared routes to selected device
	/// </summary>
	/// <returns>int; 0= OK</returns>
	static int TakePreparedRoutes();
	
	/// <summary>
	/// Get List of routing from selecte device
	/// </summary>
	/// <param name="routesList">vector of pairs with ints</param>
	/// <returns>int; 0 = ok</returns>
	static int GetRoutes(std::vector<std::pair<int,int>>& routesList);

	/// <summary>
	/// Enables lock on certain channel
	/// </summary>
	/// <param name="destination"></param>
	/// <returns></returns>
	static int LockRoute(unsigned int destination);

	/// <summary>
	/// Unlock route from selected device
	/// </summary>
	/// <param name="destination">channelnumber to unlock</param>
	/// <returns>int; 0 = OK</returns>
	static int UnlockRoute(unsigned int destination);

	/// <summary>
	/// Mark routes for saving
	/// </summary>
	/// <param name="destinations">list with destinations to add to marked routes</param>
	/// <returns>int; 0 = ok</returns>
	static int MarkRoutes2(std::vector<int> destinations);

	/// <summary>
	/// Saves marked routes from selected device in storage as new routing
	/// </summary>
	/// <param name="routing_name">name for new routing to store</param>
	/// <returns>int; 0 = ok</returns>
	static int SaveRoutes(std::string routing_name);

	/// <summary>
	/// Gets List of all saved routings for selected device in storage
	/// </summary>
	/// <param name="routingsList">list to store routings into</param>
	/// <returns>int; 0 = ok</returns>
	static int GetSavedRoutes(std::vector < std::pair<std::string, std::vector<std::pair<int, int>>>> &routingsList);

	/// <summary>
	/// loads routing from storage of selected device and sends it to connected device
	/// </summary>
	/// <param name="name">name of routing</param>
	/// <returns>int; 0 = ok</returns>
	static int LoadRoutes(std::string name);

private:
	/// <summary>
	/// Checks given string for certain words to determine current informationblock
	/// </summary>
	/// <param name="line">line to ckeck for word</param>
	/// <param name="type">reference to object to store current type into</param>
	/// <returns>int; 0 = ok</returns>
	static int GetInformationType(std::string line, information_type& type);

	/// <summary>
	/// Converts response string from connected device to a device data object
	/// </summary>
	/// <param name="infromationStr">string conaining response from connected device</param>
	/// <param name="_data">reference to device data object for storing information into</param>
	/// <returns>int; 0 = ok</returns>
	static int ExtractInformation(std::string infromationStr, std::unique_ptr<device_data>& _data);

	/// <summary>
	/// converts the data from a line where the key and the value are on the same line and stores it into given device_data object
	/// </summary>
	/// <param name="line">line with data</param>
	/// <param name="_data">reference to device_data object to store data into</param>
	/// <returns>int; 0 = ok</returns>
	static int GetDeviceInformation(std::string line, std::unique_ptr<device_data>& _data);
	
	/// <summary>
	/// Gets a pair of ints from given devices data object by channelnumber
	/// </summary>
	/// <returns>pair of ints with route in it</returns>
	static std::pair<int, int> GetRouteFromDestination(int destination, std::unique_ptr<device_data>& deviceData);
};

} // RouterModel