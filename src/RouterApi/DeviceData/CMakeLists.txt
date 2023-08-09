add_library(DeviceDataLib STATIC
        DeviceData.cpp
        )

target_include_directories(DeviceDataLib PUBLIC
        "./"
        )

        # install library files
install(TARGETS
        DeviceDataLib
        EXPORT DeviceDataLib
        ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
        LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs)

#install header files
set(DEVICEDATALIB_HEADERS 
        DeviceData.hpp
)

install(FILES ${DEVICEDATALIB_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)