add_library(DataHandlerLib STATIC
        DataHandler.cpp
    )

target_link_libraries(DataHandlerLib PUBLIC 
        DeviceDataLib
        SqlAccessHandlerLib
    )

target_include_directories(DataHandlerLib PUBLIC 
        "./" 
    )

# install library files
install(TARGETS
        DataHandlerLib
        EXPORT DataHandlerLib
        ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
        LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs)

#install header files
set(DATAHANDLERLIB_HEADERS 
        DataHandler.hpp
)

install(FILES ${DATAHANDLERLIB_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)