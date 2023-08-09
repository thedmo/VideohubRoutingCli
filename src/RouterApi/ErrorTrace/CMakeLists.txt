add_library(ErrorTraceLib STATIC
        ErrorTrace.cpp
        )

target_include_directories(ErrorTraceLib PUBLIC
        "./"
        )

# install library files
install(TARGETS
        ErrorTraceLib
        EXPORT ErrorTraceLib
        ARCHIVE DESTINATION ${COMMON_LIB_DIR}/libs
        LIBRARY DESTINATION ${COMMON_LIB_DIR}/libs)

#install header files
set(ERRORTRACELIB_HEADERS 
        ErrorTrace.hpp
)

install(FILES ${ERRORTRACELIB_HEADERS}
        DESTINATION ${COMMON_LIB_DIR}/includes)