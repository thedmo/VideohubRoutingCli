# SQLITE3
FetchContent_Declare(
    sqlite3
    URL https://www.sqlite.org/2022/sqlite-amalgamation-3400000.zip
)
FetchContent_MakeAvailable(sqlite3)

add_library(sqlite3_lib STATIC 
    ${sqlite3_SOURCE_DIR}/sqlite3.c
    )

message(STATUS "sqlite3_SOURCE_DIR: ${sqlite3_SOURCE_DIR}")

target_include_directories(sqlite3_lib PUBLIC 
    ${sqlite3_SOURCE_DIR}
    )


# WHEREAMI
FetchContent_Declare(
    whereami
    GIT_REPOSITORY https://github.com/gpakosz/whereami.git
    GIT_TAG 7cdf4e9257d24166777eea68a7c4c7dc48e239b3 # From C++ branch
)
FetchContent_MakeAvailable(whereami)

add_library(whereami_lib STATIC 
    ${whereami_SOURCE_DIR}/src/whereami++.cpp
    )

message(STATUS "whereami_SOURCE_DIR: ${whereami_SOURCE_DIR}")

target_include_directories(whereami_lib PUBLIC 
    ${whereami_SOURCE_DIR}/src
    )



# SQL ACCESS
add_library(sql_access_lib STATIC
    sql_access.cpp
    )

target_link_libraries(sql_access_lib PUBLIC 
    whereami_lib
    sqlite3_lib 
    )

target_include_directories(sql_access_lib PUBLIC 
    "./" 
    ${whereami_SOURCE_DIR}/src
    ${sqlite3_SOURCE_DIR}
    )
