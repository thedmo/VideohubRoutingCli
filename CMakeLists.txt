cmake_minimum_required(VERSION 3.26.0)
project(vhub_cli VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(CMAKE_CXX_EXTENSIONS Off)

include(CTest)

include(FetchContent)

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(exec_name videohub_cli )
set(main_lib_name lib_vhub )

add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(Testing)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_custom_target(run_tests
    COMMAND tests
    DEPENDS tests
    COMMENT "Running tests"
)
