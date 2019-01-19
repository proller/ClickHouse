if (NOT EXISTS "${ClickHouse_SOURCE_DIR}/contrib/aerospike/src/main/aerospike/_bin.c")
    message (WARNING "submodule contrib/aerospike is missing. to fix try run: \n git submodule update --init --recursive")
else()
    if (NOT EXISTS "${ClickHouse_SOURCE_DIR}/contrib/aerospike")
        message (WARNING "submodule contrib/aerospike is missing. to fix try run: \n git submodule update --init --recursive")
    else()
        set (AEROSPIKE_INCLUDE_DIR ${ClickHouse_SOURCE_DIR}/contrib/aerospike/src/include)
        set (AEROSPIKE_LIBRARY aerospike)
        set (USE_AEROSPIKE 1)
    endif()
endif ()
