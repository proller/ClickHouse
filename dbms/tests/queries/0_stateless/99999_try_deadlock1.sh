#!/usr/bin/env bash

set -e


CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. $CURDIR/../shell_config.sh

for n in {1..10}; do
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/00693_max_block_size_system_tables_columns.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00076_system_columns_bytes.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00140_rename.sql &
     #sleep 0.1
done

for n in {1..20}; do
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/00693_max_block_size_system_tables_columns.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00076_system_columns_bytes.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00140_rename.sql &
     sleep 0.1
done

wait
