#!/usr/bin/env bash

set -e

CURDIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
. $CURDIR/../shell_config.sh

for n in {1..10}; do
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/00722_inner_join.sql &
     #$CURDIR/00838_system_tables_drop_table_race.sh &
     #${CLICKHOUSE_CLIENT} -m -n < $CURDIR/00693_max_block_size_system_tables_columns.sql &
     #${CLICKHOUSE_CLIENT} -m -n < $CURDIR/00080_show_tables_and_system_tables.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00140_rename.sql &
     ${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00147_global_in_aggregate_function.sql &
     #${CLICKHOUSE_CLIENT} -m -n < $CURDIR/../1_stateful/00063_loyalty_joins.sql &
     #sleep 0.1
done

wait
