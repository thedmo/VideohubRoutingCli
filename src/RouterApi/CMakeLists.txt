# Router Api LIBRARY
add_subdirectory(sql_interface)
add_subdirectory(TelnetClient)
add_subdirectory(ErrorTrace)
add_subdirectory(DeviceData)

add_library(RouterApi_lib STATIC
            RouterApi.cpp
        )

target_link_libraries(RouterApi_lib PUBLIC
            sql_interface_lib
            TelnetClient_lib
            ErrorTraceLib
            DeviceDataLib
        )

target_include_directories(RouterApi_lib PUBLIC
            "./"            
            "ErrorTrace"
            "DeviceData"
            "sql_interface"
            "TelnetClient"
        )

# install library files
install(TARGETS 
            RouterApi_lib 
            TelnetClient_lib
            EXPORT RouterApiLib
            ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
            LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs
        )

#install header files
set(ROUTER_API_HEADERS 
            RouterApi.hpp 
            headers/device_data.hpp
            TelnetClient/TelnetClient.hpp
        )

install(FILES ${ROUTER_API_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)