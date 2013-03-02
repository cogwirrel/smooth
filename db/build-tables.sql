-- USAGE: In order to create the database run the following command.
--        Note that this does not work if a file already exists.
--
--        sqlite3 experiments.db < build-tables.sql
CREATE TABLE experiment (
     experimentid INTEGER PRIMARY KEY,
     sha CHARACTER(40),
     description text
   );

CREATE TABLE data (
     dataid INTEGER PRIMARY KEY,
     experimentid INTEGER,
     time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
     stdout text,
     stderr text,
     FOREIGN KEY(experimentid) REFERENCES experiment(experimentid)
     -- TODO Add auto-computed columns in the future as fit
     -- e.g. num cpu cores, min quality, exec time, etc.
   );
