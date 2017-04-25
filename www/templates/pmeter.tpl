<!DOCTYPE html><html class=''>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">

<title>PZEM - PowerMeter stat page</title>

<link type="text/css" rel="stylesheet" href="jcss/style.css">

<script src='jcss/modernizr-custom.js' type='text/javascript'></script>
<script type='text/javascript'>
    try { Modernizr } catch(e) { document.write('<script src="https://modernizr.com/download/?-cssall-cssfilters-csstransforms-csstransforms3d-csstransitions-textalignlast-wrapflow-setclasses"><\/script>')
    console.log("CDN Modernizer loaded!"); }
</script>

<!--Chart Styles -->
<style>
#chartdiv {
    width               : 100%;
    height              : 500px;
    font-size   : 11px;
}
</style>

<!-- amCharts javascript sources -->
<script src="http://www.amcharts.com/lib/3/amcharts.js" type="text/javascript"></script>
<script src="http://www.amcharts.com/lib/3/serial.js" type="text/javascript"></script>
<script src="http://www.amcharts.com/lib/3/themes/dark.js" type="text/javascript"></script>
<script src="http://www.amcharts.com/lib/3/themes/black.js" type="text/javascript"></script>
<script src="http://cdn.amcharts.com/lib/3/gauge.js" type="text/javascript"></script>
<!-- amCharts plugins -->
<script src="http://www.amcharts.com/lib/3/plugins/export/export.min.js" type="text/javascript"></script>
<script src="http://www.amcharts.com/lib/3/plugins/dataloader/dataloader.min.js" type="text/javascript"></script>

<link href="http://www.amcharts.com/lib/3/plugins/export/export.css" rel="stylesheet">


<!-- amCharts - Voltage Gauge -->
<script type="text/javascript">
    AmCharts.makeChart("GaugeV", {include file="chartVGauge.json.tpl"} );
</script>


<!-- amCharts - PF Gauge -->
<script type="text/javascript">
    AmCharts.makeChart("GaugePF", {include file="chartPFGauge.json.tpl"} );
</script>

<!-- amCharts - Samples minichart -->
<script type="text/javascript">
    AmCharts.makeChart("SamplesMini", {include file="chartsamplesmini.json.tpl"} );
</script>


<!-- amCharts Daily stat for 1 month graph -->
<script type="text/javascript">
    AmCharts.makeChart("monthstat", {include file="chartmdailystat.json.tpl"} );
</script>

</head>
<body bgcolor="#ffffff">

<p class="header-title center">Энергометр - Статистика</p>
<br>

<div id="mainpage" style="width: 95%; position:relative; margin: auto;" >

<div class="statusMenu">
    <div class="item sel">
        <form action="" method="get">
        <select name="devid" id="devselect" onchange="this.form.submit()">
        {foreach key=row item=data from=$devlist}
        <option {if $data.id eq $devid}selected{/if} value="{$data.id}">{$data.name}</option>
        {/foreach}
        </select></form>
    </div>
    <div class="item"><label for="inpgr">Graphs</label></div>
    <div class="item"><label for="inptbl">Tables</label></div>
    <div class="item"><a class="menu" href="?s=psamples&devid={$devid}">Samples</a></div>
</div>

<input type="checkbox" id="inpgr" class="hidden">
<div id="Gauges" class="graphs" >
    <div id="GaugeV"  class="graph" style="float:left; width: 200px; height: 180px; " ></div>
    <div id="GaugePF" class="graph" style="width: 250px; height: 160px; " ></div>
    <div id="SamplesMini" class="graph" style="flex: 1; height: 180px;" ></div>
</div>

<br>

<div id="monthstat" class="graphs" style="width: 99%; height: 400px; " ></div>

<div id="stat" style="clear:both; margin: 0 auto;">

<input type="checkbox" id="inptbl" class="hidden" checked>
<div id="stattables">
<p class="page-text">Расход электроэнергии по месяцам</p>

<table class="table table-header-rotated center">
<thead><tr>
{foreach key=hdr item=noitem from=$ystat[0]}
    <th class="rotate"><div><span>{$msg_stat.$hdr}</span></div></th>
{/foreach}
</tr></thead>
<tbody>
{foreach key=row item=data from=$ystat}
    <tr>
	{foreach key=hdr item=val from=$data}
	    <td>{$val}</td>
	{/foreach}
    </tr>
{/foreach}
</tbody>
</table>


<p class="page-text">Расход электроэнергии по дням за месяц</p>

<table class="table table-header-rotated center">
<thead><tr>
{foreach key=hdr item=noitem from=$mstat[0]}
    <th class="rotate"><div><span>{$msg_stat.$hdr}</span></div></th>
{/foreach}
</tr></thead>
<tbody>
{foreach key=row item=data from=$mstat}
    <tr>
	{foreach key=hdr item=val from=$data}
	    <td>{$val}</td>
	{/foreach}
    </tr>
{/foreach}
</tbody>
</table>
</div>


</div>
</div>

</body>
</html>