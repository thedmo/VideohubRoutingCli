# Router Api LIBRARY
add_subdirectory(sql_interface)
add_subdirectory(TelnetClient)

add_library(RouterApi_lib STATIC
        RouterApi.cpp
        )

target_link_libraries(RouterApi_lib PUBLIC
            sql_interface_lib
            TelnetClient_lib
        )

target_include_directories(sql_interface_lib PUBLIC
        "./"
        "../headers/"
        )