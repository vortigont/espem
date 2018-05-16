<?php
#
# PZEM power-meter http poller for cacti/SQL DB data collection
#
# polls data via ESP-board's web server connected to PZEM power-meter
#
# created by Emil Muratov
#

#
# Usage: pzem_poller.php <host> [port] [url] [meter_id]
# e.x. pzem_poller.php mymeter.example.com 80 getpmdata 10
#

# include configuration
require_once( 'config.php' );

//ini_set('error_reporting', 1);


//
isset($argv[1]) ? $host = $argv[1] : die('Error: <hostname> parameter required');
isset($argv[2]) ? $port = $argv[2] : $port = '80';
isset($argv[3]) ? $path = $argv[3] : $path = 'getpmdata';
isset($argv[4]) ? $devid =$argv[4];


$url = 'http://' . $host . ":" . $port . '/' . $path;

$curl = curl_init();
curl_setopt($curl, CURLOPT_URL,$url);
curl_setopt($curl, CURLOPT_RETURNTRANSFER,true);
curl_setopt($curl, CURLOPT_TIMEOUT, 5);
curl_setopt($curl, CURLOPT_CONNECTTIMEOUT, 5);

$iter=0;
do {
    $iter++;
    $response = curl_exec($curl);
    $httpcode = curl_getinfo($curl, CURLINFO_HTTP_CODE);
}
while ( ($httpcode != 200 or $response === false) and $iter < $retries);


curl_close($curl);

if ($httpcode != 200){
    if ( $log_enable ) { file_put_contents($logfile, "HTTP_error: $httpcode\n", FILE_APPEND); }
    die("HTTP_error: $httpcode\n");
}

#chop trailing zeroes (cacti doesn't like it)
$data = str_replace('.00', '', $response);


# print raw data to STD_OUT for cacti
print $data;

#Save data to log file
if ($log_enable) {
    $dateTime = date_create('now')->format('Y-m-d H:i:s');
    $logentry = "$dateTime - $meter_id - $data\n";

    file_put_contents($logfile, $logentry, FILE_APPEND);
}

#Save data to DB
if (!$db_enable) { exit(0);}

$datavals = explode(' ', $data);

foreach ($datavals as $item) {
    list($key, $value) = explode(':', $item);
    if (!is_numeric($value) || $value<0 ){ die("Val is out of range: $value\n"); }
    $meters[$key]=$value;
}
$meters['devid']=$devid;


$pdoopt = [
    PDO::ATTR_ERRMODE            => PDO::ERRMODE_EXCEPTION,
#    PDO::ATTR_DEFAULT_FETCH_MODE => PDO::FETCH_ASSOC,
    PDO::ATTR_EMULATE_PREPARES   => false,
];


    switch ($dbengine) {
	case 'mysql':
	    $dsn = "mysql:host=$dbhost;dbname=$dbname;charset=utf8";
	    $pdo = new PDO($dsn, $dbuser, $dbpass, $pdoopt);
	    break;
    default:
	$pdo = new PDO("sqlite:$sqlitedb", null, null, $pdoopt);
    }

// array keys must match with column names in DB
$sql = "INSERT INTO $datatable (" . implode(',',array_keys($meters) ) . ') VALUES (:' . implode(',:',array_keys($meters)) . ')';
$stmt = $pdo->prepare($sql);
$stmt->execute($meters);

?>