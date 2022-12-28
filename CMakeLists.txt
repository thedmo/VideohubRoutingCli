cmake_minimum_required(VERSION 3.0.0)
project(vhub_cli VERSION 0.1.0)

include(CTest)

include(FetchContent)

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(exec_name videohub_cli )
set(main_lib_name lib_vhub )

add_subdirectory(src)
add_subdirectory(app)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
