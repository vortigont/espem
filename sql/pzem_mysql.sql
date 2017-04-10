CREATE DATABASE /*!32312 IF NOT EXISTS*/ `pzem` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `pzem`;

--
-- Table structure for table `meters`
--

DROP TABLE IF EXISTS `meters`;
CREATE TABLE `meters` (
    `id` SMALLINT unsigned NOT NULL AUTO_INCREMENT,
    `name` VARCHAR(20),
    `descr` VARCHAR(255),
    `hostname` VARCHAR(64),
    PRIMARY KEY (`id`),
    KEY  (`name`)
) ENGINE=INNODB CHARACTER SET=UTF8;

INSERT INTO `meters` VALUES (1,'MainMeter','Default PowerMeter', 'esp-pzem01');

--
-- Table structure for table `data`
--
DROP TABLE IF EXISTS `data`;
CREATE TABLE `data` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `devid` SMALLINT unsigned NOT NULL default '1',
  `dtime` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `U` DECIMAL(4,1) unsigned NOT NULL default '0.0',
  `I` DECIMAL(4,2) unsigned NOT NULL default '0.0',
  `P` SMALLINT  unsigned NOT NULL default '0',
  `W` MEDIUMINT  unsigned NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`devid`) REFERENCES meters(id)
    ON UPDATE CASCADE,
  KEY  (`dtime`)
) ENGINE=INNODB;
