CREATE TABLE IF NOT EXISTS `users` (
  `id` INTEGER,
  `name` TEXT,
  `age` INTEGER,
  `city` TEXT,
  `is_member` BOOLEAN
);

INSERT INTO `users` (`id`, `name`, `age`, `city`, `is_member`) VALUES (1, 'Alice', 30, 'New York', TRUE);
INSERT INTO `users` (`id`, `name`, `age`, `city`, `is_member`) VALUES (2, 'Bob', 25, 'Los Angeles', FALSE);
INSERT INTO `users` (`id`, `name`, `age`, `city`, `is_member`) VALUES (3, 'Charlie', 35, 'Chicago', TRUE);
INSERT INTO `users` (`id`, `name`, `age`, `city`) VALUES (4, 'David', 40, 'Houston');
INSERT INTO `users` (`id`, `name`, `age`, `city`, `is_member`) VALUES (5, 'Eve', 28, 'Phoenix', TRUE);
