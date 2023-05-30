#ifndef EE918193_380A_4CAC_B16D_81B97DBCC972
#define EE918193_380A_4CAC_B16D_81B97DBCC972

#include <iostream> 

class device_data {
public:
    std::string ip;
    std::string name;
    std::string version;
    int source_count;
    int destination_count;
    std::string source_labels;
    std::string destination_labels;
    std::string routing;
    std::string prepared_routes;
    std::string locks;
    std::string marked_for_saving;

    //TODO: to replace simple variables in device Data
    std::vector<std::tuple<int, int>> marked_routes;
    std::vector<std::tuple<int, int>> video_output_routing;
};

#endif /* EE918193_380A_4CAC_B16D_81B97DBCC972 */
