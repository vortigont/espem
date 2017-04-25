<?php

// Gather data for month graph and stat tables

// year stat
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



$smarty->display('pmeter.tpl');

?>