SELECT intDiv(number, 100) AS k, length(groupArray(number)) FROM (SELECT * FROM system.numbers LIMIT 1000000) GROUP BY k WITH TOTALS ORDER BY k LIMIT 10;

DROP TABLE IF EXISTS test.numbers_mt;
CREATE TABLE test.numbers_mt (number UInt64) ENGINE = Log;
INSERT INTO test.numbers_mt SELECT * FROM system.numbers LIMIT 1, 1000000;

SELECT count(), sum(ns), max(ns) FROM (SELECT intDiv(number, 100) AS k, groupArray(number) AS ns FROM test.numbers_mt GROUP BY k) ARRAY JOIN ns;
SELECT count(), sum(toUInt64(ns)), max(toUInt64(ns)) FROM (SELECT intDiv(number, 100) AS k, groupArray(toString(number)) AS ns FROM test.numbers_mt GROUP BY k) ARRAY JOIN ns;
SELECT count(), sum(toUInt64(ns[1])), max(toUInt64(ns[1])), sum(toUInt64(ns[2]))/10 FROM (SELECT intDiv(number, 100) AS k, groupArray([toString(number), toString(number*10)]) AS ns FROM test.numbers_mt GROUP BY k) ARRAY JOIN ns;
SELECT count(), sum(ns[1]), max(ns[1]), sum(ns[2])/10 FROM (SELECT intDiv(number, 100) AS k, groupArray([number, number*10]) AS ns FROM test.numbers_mt GROUP BY k) ARRAY JOIN ns;

SELECT count(), sum(ns), max(ns) FROM (SELECT intDiv(number, 100) AS k, groupArray(number) AS ns FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k) ARRAY JOIN ns;
SELECT count(), sum(toUInt64(ns)), max(toUInt64(ns)) FROM (SELECT intDiv(number, 100) AS k, groupArray(toString(number)) AS ns FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k) ARRAY JOIN ns;
SELECT count(), sum(toUInt64(ns[1])), max(toUInt64(ns[1])), sum(toUInt64(ns[2]))/10 FROM (SELECT intDiv(number, 100) AS k, groupArray([toString(number), toString(number*10)]) AS ns FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k) ARRAY JOIN ns;

DROP TABLE test.numbers_mt;
CREATE TABLE test.numbers_mt (number UInt64) ENGINE = Log;
INSERT INTO test.numbers_mt SELECT * FROM system.numbers LIMIT 1, 1048575;

SELECT '';
SELECT roundToExp2(number) AS k, length(groupArray(1)(number AS i)), length(groupArray(1024)(i)), length(groupArray(65536)(i)) AS s FROM test.numbers_mt GROUP BY k ORDER BY k LIMIT 9, 11;
SELECT roundToExp2(number) AS k, length(groupArray(1)(hex(number) AS i)), length(groupArray(1024)(i)), length(groupArray(65536)(i)) AS s FROM test.numbers_mt GROUP BY k ORDER BY k LIMIT 9, 11;
SELECT roundToExp2(number) AS k, length(groupArray(1)([hex(number)] AS i)), length(groupArray(1024)(i)), length(groupArray(65536)(i)) AS s FROM test.numbers_mt GROUP BY k ORDER BY k LIMIT 9, 11;

SELECT '';
SELECT roundToExp2(number) AS k, length(groupArray(1)(number AS i)), length(groupArray(1500)(i)), length(groupArray(70000)(i)) AS s FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k ORDER BY k LIMIT 9, 11;
SELECT roundToExp2(number) AS k, length(groupArray(1)(hex(number) AS i)), length(groupArray(1500)(i)), length(groupArray(70000)(i)) AS s FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k ORDER BY k LIMIT 9, 11;
SELECT roundToExp2(number) AS k, length(groupArray(1)([hex(number)] AS i)), length(groupArray(1500)(i)), length(groupArray(70000)(i)) AS s FROM remote('127.0.0.{1,2}', 'test', 'numbers_mt') GROUP BY k ORDER BY k LIMIT 9, 11;

DROP TABLE test.numbers_mt;

-- Check binary compability:
-- clickhouse-client -h old -q "SELECT arrayReduce('groupArrayState', [['1'], ['22'], ['333']]) FORMAT RowBinary" | clickhouse-local --input-format RowBinary --structure "d AggregateFunction(groupArray2, Array(String))" -q "SELECT groupArray2Merge(d) FROM table"
