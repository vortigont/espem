<?php

// Create DB connection

$pdoopt = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    PDO::ATTR_EMULATE_PREPARES   => false,
];

switch ($dbengine) {
    case 'mysql':
        $dsn = "mysql:host=$dbhost;dbname=$dbname;charset=utf8";
        $pdo = new PDO($dsn, $dbuser, $dbpass, $pdoopt);
        break;
    default:
	$dbengine = 'sqlite';	//make sure to redefine def engine
        $pdo = new PDO("sqlite:$sqlitedb", null, null, $pdoopt);
}

// include reqs
require( "$dbengine.reqs.php" );

$stmt = $pdo->prepare($sql['ystat']);
$stmt->execute();

$ystat = $stmt->fetchAll();

$smarty->assignByRef('ystat', $ystat );


// month stat
$stmt = $pdo->prepare($sql['mstat']);
$stmt->execute();
$mstat = $stmt->fetchAll();

$smarty->assignByRef('mstat', $mstat );

// last values rom DB
$stmt = $pdo->prepare($sql['last']);
$stmt->execute();
$last = $stmt->fetchAll();
$smarty->assignByRef('last', $last );

$smarty->assign('esphost', $meterhost);

$smarty->display('pmeter.tpl');

?>