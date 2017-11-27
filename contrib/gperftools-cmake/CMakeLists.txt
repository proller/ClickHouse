if (GPERFTOOLS_USE_CONFIGURE)


set (GPERFTOOLS_ROOT ${ClickHouse_SOURCE_DIR}/contrib/gperftools)
set (GPERFTOOLS_BUILD ${ClickHouse_BINARY_DIR}/contrib/gperftools)

set (CC_FULL "${CMAKE_C_COMPILER_LAUNCHER} ${CMAKE_C_COMPILER}")
set (CXX_FULL "${CMAKE_CXX_COMPILER_LAUNCHER} ${CMAKE_CXX_COMPILER}")
get_property (C_LAUNCH GLOBAL PROPERTY RULE_LAUNCH_COMPILE)

include (ExternalProject)
ExternalProject_Add (
    gperftools
    DOWNLOAD_COMMAND ""
    CONFIGURE_COMMAND autoreconf -i ${GPERFTOOLS_ROOT} && env "CC=${C_LAUNCH} ${CMAKE_C_COMPILER}" "CXX=${C_LAUNCH} ${CMAKE_CXX_COMPILER}" CFLAGS=${FULL_C_FLAGS} CXXFLAGS=${FULL_CXX_FLAGS} ${GPERFTOOLS_ROOT}/configure --enable-minimal --with-pic --enable-shared=no --prefix=${GPERFTOOLS_BUILD}
    BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -j `nproc || sysctl -n hw.ncpu || echo 4`
)

set (GPERFTOOLS_TCMALLOC_MINIMAL ${ClickHouse_BINARY_DIR}/contrib/gperftools/lib/libtcmalloc_minimal.a CACHE INTERNAL "")
set (GPERFTOOLS_TCMALLOC_MINIMAL_DEBUG ${ClickHouse_BINARY_DIR}/contrib/gperftools/lib/libtcmalloc_minimal_debug.a CACHE INTERNAL "")


else () 


SET(LIBRARY_DIR ${ClickHouse_SOURCE_DIR}/contrib/gperftools/src)

#message (STATUS "Building: tcmalloc_minimal_internal")
add_library (tcmalloc_minimal
${LIBRARY_DIR}/malloc_hook.cc
${LIBRARY_DIR}/base/spinlock_internal.cc
${LIBRARY_DIR}/base/spinlock.cc
${LIBRARY_DIR}/base/dynamic_annotations.c
${LIBRARY_DIR}/base/linuxthreads.cc
${LIBRARY_DIR}/base/elf_mem_image.cc
${LIBRARY_DIR}/base/vdso_support.cc
${LIBRARY_DIR}/base/sysinfo.cc
${LIBRARY_DIR}/base/low_level_alloc.cc
${LIBRARY_DIR}/base/thread_lister.c
${LIBRARY_DIR}/base/logging.cc
${LIBRARY_DIR}/base/atomicops-internals-x86.cc
${LIBRARY_DIR}/memfs_malloc.cc
${LIBRARY_DIR}/tcmalloc.cc
${LIBRARY_DIR}/malloc_extension.cc
${LIBRARY_DIR}/thread_cache.cc
${LIBRARY_DIR}/symbolize.cc
${LIBRARY_DIR}/page_heap.cc
${LIBRARY_DIR}/maybe_threads.cc
${LIBRARY_DIR}/central_freelist.cc
${LIBRARY_DIR}/static_vars.cc
${LIBRARY_DIR}/sampler.cc
${LIBRARY_DIR}/internal_logging.cc
${LIBRARY_DIR}/system-alloc.cc
${LIBRARY_DIR}/span.cc
${LIBRARY_DIR}/common.cc
${LIBRARY_DIR}/debugallocation.cc
${LIBRARY_DIR}/stacktrace.cc
${LIBRARY_DIR}/stack_trace_table.cc
${LIBRARY_DIR}/heap-checker.cc
${LIBRARY_DIR}/heap-checker-bcad.cc
${LIBRARY_DIR}/heap-profile-table.cc
${LIBRARY_DIR}/raw_printer.cc
${LIBRARY_DIR}/memory_region_map.cc
)

target_compile_options (tcmalloc_minimal
    PRIVATE
    -DNO_TCMALLOC_SAMPLES
    -DNDEBUG
    -DNO_FRAME_POINTER
    -Wwrite-strings
    -Wno-sign-compare
    -Wno-unused-result
    -Wno-deprecated-declarations
    -Wno-unused-function
    -Wno-unused-private-field

     PUBLIC
    -fno-builtin-malloc
    -fno-builtin-free
    -fno-builtin-realloc
    -fno-builtin-calloc
    -fno-builtin-cfree
    -fno-builtin-memalign
    -fno-builtin-posix_memalign
    -fno-builtin-valloc
    -fno-builtin-pvalloc
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.9)
    target_compile_options(tcmalloc_minimal PUBLIC -Wno-dynamic-exception-spec )
endif ()

if (CMAKE_SYSTEM MATCHES "FreeBSD" AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(tcmalloc_minimal PUBLIC -Wno-unused-but-set-variable)
endif ()

if (CMAKE_SYSTEM MATCHES "FreeBSD")
    target_compile_definitions(tcmalloc_minimal PUBLIC _GNU_SOURCE)
endif ()

#target_include_directories (tcmalloc_minimal PUBLIC include)
#target_include_directories (tcmalloc_minimal PRIVATE src)

target_include_directories (tcmalloc_minimal PRIVATE BEFORE ${ClickHouse_SOURCE_DIR}/contrib/gperftools-cmake)
target_include_directories (tcmalloc_minimal PUBLIC ${LIBRARY_DIR})

find_package (Threads)
target_link_libraries (tcmalloc_minimal ${CMAKE_THREAD_LIBS_INIT})

set (GPERFTOOLS_TCMALLOC_MINIMAL tcmalloc_minimal CACHE INTERNAL "")
set (GPERFTOOLS_TCMALLOC_MINIMAL_DEBUG tcmalloc_minimal_debug CACHE INTERNAL "") # TODO


endif ()
