<?php
require_once 'db.php';

DB::pdo()->exec("CREATE TABLE IF NOT EXISTS `users` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `username` varchar(30) COLLATE utf8_bin NOT NULL,
  `first_name` varchar(60) COLLATE utf8_bin NOT NULL,
  `last_name` varchar(60) COLLATE utf8_bin NOT NULL,
  `email` varchar(60) COLLATE utf8_bin NOT NULL,
  `password` char(64) COLLATE utf8_bin NOT NULL,
  `type` enum('normal','teacher','admin') COLLATE utf8_bin NOT NULL DEFAULT 'normal',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  KEY (`type`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE IF NOT EXISTS `session` (
  `id` char(10) COLLATE utf8_bin NOT NULL,
  `data` text COLLATE utf8_bin NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE IF NOT EXISTS `tasks` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `name` VARCHAR(128) NOT NULL,
  `checker` varchar(32) NOT NULL DEFAULT 'default',
  `added` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `privileges` enum('admin','teacher','all') COLLATE utf8_bin NOT NULL DEFAULT 'all',
  PRIMARY KEY (`id`),
  KEY (`added`),
  KEY (`checker`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE IF NOT EXISTS `submissions` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `user_id` int unsigned NOT NULL,
  `round_id` int unsigned NOT NULL,
  `task_id` int unsigned NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `status` enum('ok','error','c_error','waiting') DEFAULT NULL COLLATE utf8_bin,
  `queued` timestamp NOT NULL,
  `points` int unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY (`user_id`),
  KEY (`round_id`),
  KEY (`task_id`),
  KEY (`status`,`queued`),
  KEY (`time`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE IF NOT EXISTS `submissions_to_rounds` (
  `round_id` int unsigned NOT NULL,
  `submission_id` int unsigned NOT NULL,
  `user_id` int unsigned NOT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`round_id`, `user_id`,`submission_id`),
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;

CREATE TABLE IF NOT EXISTS `rounds` (
  `id` int unsigned NOT NULL AUTO_INCREMENT,
  `item` int unsigned NOT NULL,
  `visible` BOOLEAN NOT NULL DEFAULT FALSE,
  `author` int unsigned NOT NULL,
  `name` VARCHAR(128) NOT NULL,
  `parent` int unsigned NOT NULL DEFAULT 1,
  `begin_time` timestamp DEFAULT NULL,
  `full_judge_time` timestamp DEFAULT NULL,
  `end_time` timestamp DEFAULT NULL,
  `privileges` enum('admin','teacher','all') COLLATE utf8_bin NOT NULL DEFAULT 'all',
  `task_id` int unsigned DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY (`parent`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
INSERT IGNORE INTO `rounds` (`id`,`name`,`parent`) VALUES(1,'',0);

CREATE TABLE IF NOT EXISTS `ranks` (
  `user_id` int unsigned NOT NULL,
  `round_id` int unsigned NOT NULL,
  `points` int unsigned DEFAULT NULL,
  PRIMARY KEY (`user_id`,`round_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_bin;
");

?>
