if(USE_STATIC_LIBRARIES AND NOT UNBUNDLED)
    option(ENABLE_MUSL "Enable Musl" 1)
endif()

if(ENABLE_MUSL)
    set(MUSL_ROOT "/usr/local/opt/icu4c" CACHE STRING "")
    set(MUSL_INCLUDE_PATHS /usr/include/x86_64-linux-musl)
    if(USE_STATIC_LIBRARIES)
        list(APPEND MUSL_LIBRARIES_PATHS /usr/lib/x86_64-linux-musl) # .a
    else()
        list(APPEND MUSL_LIBRARIES_PATHS /lib/x86_64-linux-musl) # .so
    endif()
    find_path(MUSL_INCLUDE_DIR NAMES malloc.h PATHS ${MUSL_INCLUDE_PATHS} NO_DEFAULT_PATH)
    find_library(MUSL_M_LIBRARY m PATHS ${MUSL_LIBRARIES_PATHS} NO_DEFAULT_PATH)
    find_library(MUSL_C_LIBRARY c PATHS ${MUSL_LIBRARIES_PATHS} NO_DEFAULT_PATH)
    set(MUSL_LIBRARIES ${MUSL_M_LIBRARY} ${MUSL_C_LIBRARY})
    if(MUSL_LIBRARIES AND MUSL_INCLUDE_DIR)
        set(MUSL_FOUND 1)
    endif()

    if(MUSL_FOUND)
        set(USE_MUSL 1)
    endif()
endif()

message(STATUS "Using musl=${USE_MUSL}: ${MUSL_INCLUDE_DIR} : ${MUSL_LIBRARIES}")
