drop table if exists sql_test;

-- 1. create table without engine
create table sql_test (timestamp UInt32, event UInt32, str VARCHAR(42));
insert into sql_test values (1, 1000, 'string!');
insert into sql_test values (CURRENT_TIMESTAMP() - CURRENT_TIMESTAMP(), 1001, 'string2');
--insert into sql_test values (CURRENT_TIMESTAMP, 1003, 'string3');
select * from sql_test where timestamp <= 1000;
drop table sql_test;
