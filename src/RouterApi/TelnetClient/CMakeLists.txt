add_library(TelnetClient_lib STATIC
        TelnetClient.cpp
        )

target_link_libraries(TelnetClient_lib PUBLIC
        ws2_32
        ErrorTraceLib
        )

target_include_directories(TelnetClient_lib PUBLIC "./")