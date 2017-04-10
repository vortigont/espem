--
-- Table structure for table `data`
--

DROP TABLE `meters`;
DROP TABLE `data`;

CREATE TABLE `meters` (
    `id` INTEGER PRIMARY KEY,
    `name` TEXT,
    `descr` TEXT,
    `hostname` TEXT
);


-- set `dtime` default to localtime (datetime('now','localtime')
CREATE TABLE `data` (
  `id` INTEGER PRIMARY KEY,
  `devid` INT  	    NOT NULL default '1' references meters(id),
  `dtime` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `U` DECIMAL(4,1)  NOT NULL default '0.0',
  `I` DECIMAL(4,2)  NOT NULL default '0.0',
  `P` SMALLINT      NOT NULL default '0',
  `W` MEDIUMINT     NOT NULL
);

CREATE INDEX dtime ON data (dtime);

INSERT INTO meters VALUES ('1','MainMeter', 'Default PowerMeter', 'esp-pzem01');
