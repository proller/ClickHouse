if (CLICKHOUSE_SPLIT_BINARY)
    set (TEST_USE_BINARIES CLICKHOUSE_TESTS_SERVER_BIN_PATH=${ClickHouse_BINARY_DIR}/dbms/programs/clickhouse-server CLICKHOUSE_TESTS_CLIENT_BIN_PATH=${ClickHouse_BINARY_DIR}/dbms/programs/clickhouse-client)
else ()
    set (TEST_USE_BINARIES CLICKHOUSE_TESTS_SERVER_BIN_PATH=${ClickHouse_BINARY_DIR}/dbms/programs/clickhouse CLICKHOUSE_TESTS_CLIENT_BIN_PATH=${ClickHouse_BINARY_DIR}/dbms/programs/clickhouse)
endif ()

find_program (DOCKER_CMD docker)
find_program (DOCKER_COMPOSE_CMD docker-compose)
find_program (PYTEST_CMD pytest)
find_program (SUDO_CMD sudo)

# will mount only one binary to docker container - build with .so cant work
if (MAKE_STATIC_LIBRARIES AND DOCKER_CMD AND DOCKER_COMPOSE_CMD AND PYTEST_CMD)
    #add_test (NAME integration WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND env ${TEST_USE_BINARIES} "CLICKHOUSE_TESTS_BASE_CONFIG_DIR=${ClickHouse_SOURCE_DIR}/dbms/programs/server/" ${PYTEST_STARTER} ${PYTEST_CMD} ${PYTEST_OPT})
    add_test (NAME integration WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND ${SUDO_CMD} ./runner --binary ${ClickHouse_BINARY_DIR}/dbms/programs/clickhouse --configs-dir ${ClickHouse_SOURCE_DIR}/dbms/programs/server/)
    message(STATUS "Using tests in docker DOCKER=${DOCKER_CMD}; DOCKER_COMPOSE=${DOCKER_COMPOSE_CMD}; PYTEST=${PYTEST_STARTER} ${PYTEST_CMD} ${PYTEST_OPT}")
endif()
