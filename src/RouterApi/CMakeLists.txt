# Router Api LIBRARY
add_subdirectory(SqlAccessHandler)
add_subdirectory(DataHandler)
add_subdirectory(TelnetClient)
add_subdirectory(ErrorTrace)
add_subdirectory(DeviceData)


# SQLITE3 Library
FetchContent_Declare(
            sqlite3
            URL https://www.sqlite.org/2022/sqlite-amalgamation-3400000.zip
        )
FetchContent_MakeAvailable(sqlite3)

add_library(sqlite3_lib STATIC 
            ${sqlite3_SOURCE_DIR}/sqlite3.c
        )

target_include_directories(sqlite3_lib PUBLIC 
            ${sqlite3_SOURCE_DIR}
        )


# WHEREAMI Library
FetchContent_Declare(
            whereami
            GIT_REPOSITORY https://github.com/gpakosz/whereami.git
            GIT_TAG 7cdf4e9257d24166777eea68a7c4c7dc48e239b3 # From C++ branch
        )
FetchContent_MakeAvailable(whereami)

add_library(whereami_lib STATIC 
            ${whereami_SOURCE_DIR}/src/whereami++.cpp
        )

target_include_directories(whereami_lib PUBLIC 
            ${whereami_SOURCE_DIR}/src
        )


# RouterModel Library
add_library(RouterApi_lib STATIC
            RouterApi.cpp
        )

target_link_libraries(RouterApi_lib PUBLIC
            TelnetClient_lib
            DeviceDataLib
            ErrorTraceLib
            DataHandlerLib
        )

target_include_directories(RouterApi_lib PUBLIC
            "./"            
            "ErrorTrace"
            "DeviceData"
            "TelnetClient"
        )

# install library files
install(TARGETS 
            RouterApi_lib
            EXPORT RouterModel
            ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
            LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs
        )

#install header files
set(ROUTERMODEL_HEADERS 
            RouterApi.hpp
        )

install(FILES ${ROUTERMODEL_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)