SELECT toDateTime('2017-10-30 08:18:19') + INTERVAL 1 DAY + INTERVAL 1 MONTH - INTERVAL 1 YEAR;
SELECT toDateTime('2017-10-30 08:18:19') + INTERVAL 1 HOUR + INTERVAL 1000 MINUTE + INTERVAL 10 SECOND;
SELECT toDateTime('2017-10-30 08:18:19') + INTERVAL 1 DAY + INTERVAL number MONTH FROM system.numbers LIMIT 20;
SELECT toDateTime('2016-02-29 01:02:03') + INTERVAL number YEAR, toDateTime('2016-02-29 01:02:03') + INTERVAL number MONTH FROM system.numbers LIMIT 16;
