drop table if exists IOPoint;
drop table if exists RTU;

create table IOPoint 
(
    parent integer,
    name varchar(32),
    id int default 0,
    digital boolean default 0,
    analog boolean default 0,
    --
    -- Direction 0 is input
    --
    direction boolean default 0
);

create table RTU (
    idx integer primary key,
    name varchar(32)
);

insert into IOPoint values (0,'LED',13,1,0,1);
insert into IOPoint values (0,'ON',0,1,0,0);
insert into IOPoint values (0,'OFF',1,1,0,0);
