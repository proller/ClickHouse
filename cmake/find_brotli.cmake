option (ENABLE_BROTLI "Enable brotli" ON)

if (ENABLE_BROTLI)

option (USE_INTERNAL_BROTLI_LIBRARY "Set to FALSE to use system libbrotli library instead of bundled" ${NOT_UNBUNDLED})

if (NOT EXISTS "${ClickHouse_SOURCE_DIR}/contrib/brotli/c/include/brotli/decode.h")
    if (USE_INTERNAL_BROTLI_LIBRARY)
        message (WARNING "submodule contrib/brotli is missing. to fix try run: \n git submodule update --init --recursive")
        set (USE_INTERNAL_BROTLI_LIBRARY 0)
    endif ()
    set (MISSING_INTERNAL_BROTLI_LIBRARY 1)
endif ()

if(NOT USE_INTERNAL_BROTLI_LIBRARY)
    find_library(BROTLI_LIBRARY_COMMON brotlicommon)
    find_library(BROTLI_LIBRARY_DEC brotlidec)
    find_library(BROTLI_LIBRARY_ENC brotlienc)
    find_path(BROTLI_INCLUDE_DIR NAMES brotli/decode.h brotli/encode.h brotli/port.h brotli/types.h PATHS ${BROTLI_INCLUDE_PATHS})
    if(BROTLI_LIBRARY_DEC AND BROTLI_LIBRARY_ENC AND BROTLI_LIBRARY_COMMON)
        set(BROTLI_LIBRARY ${BROTLI_LIBRARY_DEC} ${BROTLI_LIBRARY_ENC} ${BROTLI_LIBRARY_COMMON})
    endif()
endif()

if (BROTLI_LIBRARY AND BROTLI_INCLUDE_DIR)
    set (USE_BROTLI 1)
elseif (NOT MISSING_INTERNAL_BROTLI_LIBRARY)
    set (BROTLI_INCLUDE_DIR ${ClickHouse_SOURCE_DIR}/contrib/brotli/c/include)
    set (USE_INTERNAL_BROTLI_LIBRARY 1)
    set (BROTLI_LIBRARY brotli)
    set (USE_BROTLI 1)
endif ()

endif()

message (STATUS "Using brotli=${USE_BROTLI}: ${BROTLI_INCLUDE_DIR} : ${BROTLI_LIBRARY}")
