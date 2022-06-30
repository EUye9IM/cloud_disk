DROP TABLE IF EXISTS node;
CREATE TABLE node(
path VARCHAR(512) NOT NULL,
hash char(50),
mtime int(11),
PRIMARY KEY (path)
);

INSERT INTO node VALUES ('/',NULL,NULL);

DROP TABLE IF EXISTS file;
CREATE TABLE file(
hash char(50),
size bigint(22) NOT NULL,
PRIMARY KEY (hash)
);

DROP TABLE IF EXISTS userinfo;
CREATE TABLE userinfo(
name char(255) NOT NULL,
pass char(50),
PRIMARY KEY (name)
);