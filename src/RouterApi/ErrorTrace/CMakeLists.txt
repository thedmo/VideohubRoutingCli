add_library(ErrorTraceLib STATIC
        ErrorTrace.cpp
        )

target_include_directories(ErrorTraceLib PUBLIC
        "./"
        )