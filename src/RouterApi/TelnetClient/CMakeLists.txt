add_library(TelnetClient_lib STATIC
        TelnetClient.cpp
        )

target_link_libraries(TelnetClient_lib PUBLIC
        ws2_32
        ErrorTraceLib
        )

target_include_directories(TelnetClient_lib PUBLIC "./")

        # install library files
install(TARGETS
        TelnetClient_lib
        EXPORT TelnetClientLib
        ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
        LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs)

#install header files
set(TELNETCLIENTLIB_HEADERS 
        TelnetClient.hpp
)

install(FILES ${TELNETCLIENTLIB_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes
)