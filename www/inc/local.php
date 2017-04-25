<?php

require_once( 'config.php' );

require_once(SMARTY_DIR . 'Smarty.class.php');
$smarty = new Smarty();

$smarty->debugging = false;
$smarty->caching   = false;
//$smarty->cache_lifetime = 120;

$smarty->template_dir = DOC_ROOT . '/templates/';
$smarty->compile_dir = DOC_ROOT . '/templates/templates_c/';
$smarty->config_dir = DOC_ROOT . '/templates/configs/';
$smarty->cache_dir = DOC_ROOT . '/templates/cache/';

$lang = substr($_SERVER['HTTP_ACCEPT_LANGUAGE'], 0, 2);
// include trans file
switch ($lang){
    case "ru":
	require_once( 'lang_ru.php' );
        break;
    default:
        //Setting Default";
	require_once( 'lang_en.php' );
        break;
}

// lang resources
$smarty->assignByRef( 'msg_stat', $msg_stat );

// set device id if provided
if (is_numeric($_REQUEST['devid'])) $devid=$_REQUEST['devid'];
$smarty->assign('devid', $devid);

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
        $dbengine = 'sqlite';   //make sure to redefine def engine
        $pdo = new PDO("sqlite:$sqlitedb", null, null, $pdoopt);
}

// include reqs
require( "$dbengine.reqs.php" );

// get meters list
$stmt = $pdo->prepare($sql['devlist']);
$stmt->execute();
$devlist = $stmt->fetchAll();
$smarty->assignByRef('devlist', $devlist );

// find current meter hostname
$rid = array_search($devid, array_column($devlist, 'id'));
$meterhost = $devlist[$rid]['hostname'];
$smarty->assign('esphost', $meterhost);
