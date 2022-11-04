set(SQLITE_INCLUDES
    "./"
    )

add_library(lib_sqlite STATIC 
    sqlite3.c
    )

target_include_directories(lib_sqlite PUBLIC 
    ${SQLITE_INCLUDES}
    )