if (CMAKE_SYSTEM MATCHES "FreeBSD" OR ARCH_32)
    option (USE_INTERNAL_GPERFTOOLS_LIBRARY "Set to FALSE to use system gperftools (tcmalloc) library instead of bundled" OFF)
else ()
    option (USE_INTERNAL_GPERFTOOLS_LIBRARY "Set to FALSE to use system gperftools (tcmalloc) library instead of bundled" ${NOT_UNBUNDLED})
endif ()
option (ENABLE_LIBTCMALLOC "Set to TRUE to enable libtcmalloc" ON)
option (DEBUG_LIBTCMALLOC "Set to TRUE to use debug version of libtcmalloc" OFF)

if (ENABLE_LIBTCMALLOC)
    if (NOT EXISTS "${ClickHouse_SOURCE_DIR}/contrib/gperftools/src/tcmalloc.h")
        message (WARNING "submodule contrib/gperftools is missing. to fix try run: \n git submodule update --init --recursive")
        set (USE_INTERNAL_GPERFTOOLS_LIBRARY 0)
        set (MISSING_INTERNAL_GPERFTOOLS_LIBRARY  1)
    endif ()

    #contrib/libtcmalloc doesnt build debug version, try find in system
    if (DEBUG_LIBTCMALLOC OR NOT USE_INTERNAL_GPERFTOOLS_LIBRARY)
        find_package (Gperftools)
    endif ()

    if (NOT MISSING_INTERNAL_GPERFTOOLS_LIBRARY AND NOT (GPERFTOOLS_FOUND AND GPERFTOOLS_INCLUDE_DIR AND GPERFTOOLS_TCMALLOC_MINIMAL) AND NOT (CMAKE_SYSTEM MATCHES "FreeBSD" OR ARCH_32))
        set (USE_INTERNAL_GPERFTOOLS_LIBRARY 1)
        set (GPERFTOOLS_INCLUDE_DIR "${ClickHouse_BINARY_DIR}/contrib/gperftools/include")
    endif ()

    if (GPERFTOOLS_FOUND OR USE_INTERNAL_GPERFTOOLS_LIBRARY)
        set (USE_TCMALLOC 1)
    endif ()

    message (STATUS "Using tcmalloc=${USE_TCMALLOC}: ${GPERFTOOLS_INCLUDE_DIR} : ${GPERFTOOLS_TCMALLOC_MINIMAL}")
endif ()

macro (target_include_gperftools target)
    if (USE_TCMALLOC)
       add_dependencies (${target} gperftools)
       target_include_directories (${target} BEFORE PRIVATE ${GPERFTOOLS_INCLUDE_DIR})
    endif ()
endmacro ()
