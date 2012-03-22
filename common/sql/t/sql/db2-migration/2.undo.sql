CREATE TABLE test1copy (
       col1 INTEGER NOT NULL
);
INSERT INTO test1copy SELECT col1 FROM test1;
DROP TABLE test1;
ALTER TABLE test1copy RENAME TO test1;
