if (NOT USE_INTERNAL_BOOST_LIBRARY)
	set (Boost_USE_STATIC_LIBS ${USE_STATIC_LIBRARIES})
	set (BOOST_ROOT "/usr/local")
	find_package (Boost 1.55 COMPONENTS program_options system filesystem regex thread)
	if (NOT Boost_FOUND)
		# Try to find manually.
		set (BOOST_PATHS "")
		find_library (Boost_PROGRAM_OPTIONS_LIBRARY boost_program_options PATHS ${BOOST_PATHS})
		find_library (Boost_SYSTEM_LIBRARY boost_system PATHS ${BOOST_PATHS})
		find_library (Boost_FILESYSTEM_LIBRARY boost_filesystem PATHS ${BOOST_PATHS})
	endif ()
	include_directories (${Boost_INCLUDE_DIRS})
endif ()

if (NOT Boost_SYSTEM_LIBRARY)
	set (Boost_PROGRAM_OPTIONS_LIBRARY boost_program_options_internal)
	set (Boost_SYSTEM_LIBRARY boost_system_internal)
	set (Boost_FILESYSTEM_LIBRARY boost_filesystem_internal)
	set (Boost_INCLUDE_DIRS "${ClickHouse_SOURCE_DIR}/contrib/libboost/boost_1_62_0/")
	include_directories (BEFORE ${Boost_INCLUDE_DIRS})
endif ()

message(STATUS "Using Boost: ${Boost_INCLUDE_DIRS} : ${Boost_PROGRAM_OPTIONS_LIBRARY},${Boost_SYSTEM_LIBRARY},${Boost_FILESYSTEM_LIBRARY}")
