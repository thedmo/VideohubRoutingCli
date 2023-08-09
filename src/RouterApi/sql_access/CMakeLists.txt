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



# install library files
install(TARGETS
        sql_access_lib
        EXPORT SqlAccessLib
        ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
        LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs)

#install header files
set(SQLACCESSLIB_HEADERS 
        sql_access.hpp
)

install(FILES ${SQLACCESSLIB_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)