<?php

require_once( 'config.php' );

require_once(SMARTY_DIR . 'Smarty.class.php');
$smarty = new Smarty();

$smarty->debugging = false;
$smarty->caching   = true;
$smarty->cache_lifetime = 120;

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