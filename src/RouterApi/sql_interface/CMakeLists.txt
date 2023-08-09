# SQL INTERFACE LIBRARY
add_library(sql_interface_lib STATIC
        SqliteInterface.cpp
        )

target_link_libraries(sql_interface_lib PUBLIC
        DeviceDataLib
        sql_access_lib
        ErrorTraceLib
        )

target_include_directories(sql_interface_lib PUBLIC
        "./"
        )

# install library files
install(TARGETS 
sql_interface_lib
EXPORT SqlLib
ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs
)

#install header files
set(SQLITEINTERFACE_HEADERS 
        SqliteInterface.hpp
)

install(FILES ${SQLITEINTERFACE_HEADERS} 
DESTINATION ${COMMON_LIB_DIR}/includes)