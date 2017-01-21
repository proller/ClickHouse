if (NOT READLINE_PATHS)
	set (READLINE_PATHS "/usr/local/opt/readline/lib")
	if (USE_STATIC_LIBRARIES)
		find_library (READLINE_LIB NAMES libreadline.a PATHS ${READLINE_PATHS})
	else ()
		find_library (READLINE_LIB NAMES readline PATHS ${READLINE_PATHS})
	endif ()

	if (USE_STATIC_LIBRARIES)
		find_library (TERMCAP_LIB NAMES libtermcap.a termcap)
	else ()
		find_library (TERMCAP_LIB NAMES termcap)
	endif ()

	if (USE_STATIC_LIBRARIES)
		find_library (EDIT_LIB NAMES libedit.a)
	else ()
		find_library (EDIT_LIB NAMES edit)
	endif ()

	set(READLINE_INCLUDE_PATHS "/usr/local/opt/readline/include")
	if (READLINE_LIB)
		find_path (READLINE_INCLUDE_DIR NAMES readline.h PATH_SUFFIXES readline PATHS ${READLINE_INCLUDE_PATHS})
		add_definitions (-D USE_READLINE)
		set (LINE_EDITING_LIBS ${READLINE_LIB} ${TERMCAP_LIB})
		message (STATUS "Using line editing libraries (readline): ${READLINE_INCLUDE_DIR} : ${LINE_EDITING_LIBS}")
	elseif (EDIT_LIB)
		if (USE_STATIC_LIBRARIES)
			find_library (CURSES_LIB NAMES libcurses.a)
		else ()
			find_library (CURSES_LIB NAMES curses)
		endif ()
		add_definitions (-D USE_LIBEDIT)
		find_path (READLINE_INCLUDE_DIR NAMES readline.h PATH_SUFFIXES editline PATHS ${READLINE_INCLUDE_PATHS})
		set (LINE_EDITING_LIBS ${EDIT_LIB} ${CURSES_LIB} ${TERMCAP_LIB})
		message (STATUS "Using line editing libraries (edit): ${READLINE_INCLUDE_DIR} : ${LINE_EDITING_LIBS}")
	else ()
		message (STATUS "Not using any library for line editing.")
	endif ()
	if (READLINE_INCLUDE_DIR)
		include_directories (${READLINE_INCLUDE_DIR})
	endif ()

endif ()
