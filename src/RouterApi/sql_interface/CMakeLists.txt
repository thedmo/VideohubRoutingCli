add_subdirectory(sql_access)


# SQL INTERFACE LIBRARY
add_library(sql_interface_lib STATIC
        SqliteInterface.cpp
        )

target_link_libraries(sql_interface_lib PUBLIC
        sql_access_lib
        )

target_include_directories(sql_interface_lib PUBLIC
        "./"
        "../headers/"
        )
