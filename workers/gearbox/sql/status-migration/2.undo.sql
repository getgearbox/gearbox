CREATE TABLE versioncopy (
       version INTEGER NOT NULL
);
INSERT INTO versioncopy SELECT version FROM version;
DROP TABLE version;
ALTER TABLE versioncopy RENAME TO version;
