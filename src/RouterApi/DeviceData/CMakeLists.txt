add_library(DeviceDataLib STATIC
        DeviceData.cpp
        )

target_include_directories(DeviceDataLib PUBLIC
        "./"
        )