SET send_logs_level = 'none';

DROP TABLE IF EXISTS test.quorum1;
DROP TABLE IF EXISTS test.quorum2;

CREATE TABLE test.quorum1(x UInt32, y Date) ENGINE ReplicatedMergeTree('/clickhouse/tables/test/quorum', '1') ORDER BY x PARTITION BY y;
CREATE TABLE test.quorum2(x UInt32, y Date) ENGINE ReplicatedMergeTree('/clickhouse/tables/test/quorum', '2') ORDER BY x PARTITION BY y;

SET insert_quorum=2;
SET select_sequential_consistency=1;

SET insert_quorum_timeout=0;

SYSTEM STOP FETCHES test.quorum1;

INSERT INTO test.quorum2 VALUES (1, '2018-11-15'); -- { serverError 319 }

SELECT count(*) FROM test.quorum1;
SELECT count(*) FROM test.quorum2;

SET select_sequential_consistency=0;

SELECT x FROM test.quorum2 ORDER BY x;
SET select_sequential_consistency=1;

SET insert_quorum_timeout=100;

SYSTEM START FETCHES test.quorum1;
SYSTEM SYNC REPLICA test.quorum1;
 
SELECT x FROM test.quorum1 ORDER BY x;
SELECT x FROM test.quorum2 ORDER BY x;

DROP TABLE IF EXISTS test.quorum1;
DROP TABLE IF EXISTS test.quorum2;
