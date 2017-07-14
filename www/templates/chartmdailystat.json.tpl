{
"type": "serial",
"categoryField": "date",
"columnSpacing": 2,
"columnWidth": 0.49,
"dataDateFormat": "YYYY-MM-DD",
"maxSelectedSeries": 40,
"angle": 30,
"depth3D": 20,
"maxZoomFactor": 1,
"sequencedAnimation": false,
"accessible": false,
"accessibleTitle": "",
"creditsPosition": "top-right",
"handDrawScatter": 5,
"handDrawThickness": 11,
"precision": 1,
"startDuration": 0,
"theme": "black",
"export": {
    "enabled": true
},
"categoryAxis": {
    "dateFormats": [
        {
            "period": "mm",
            "format": "DD JJ:NN"
        },
        {
            "period": "hh",
            "format": "MMM DD"
        },
        {
            "period": "DD",
            "format": "MMM DD"
        },
        {
            "period": "WW",
            "format": "MMM DD"
        },
        {
            "period": "MM",
            "format": "MMM"
        }
    ],
    "equalSpacing": true,
    "forceShowField": "",
    "gridPosition": "start",
    "minPeriod": "hh",
    "parseDates": true,
    "firstDayOfWeek": 0,
    "labelFrequency": 2,
    "markPeriodChange": false,
    "title": "",
    "titleBold": false
},
"graphs": [
    {
        "balloonText": "[[value]] {$msg_stat.kWh}",
        "dateFormat": "MMM DD",
        "fillAlphas": 1,
        "fillColors": "#E94C23",
        "id": "EnrgDay",
        "lineThickness": 0,
        "maxBulletSize": 51,
        "title": "{$msg_stat.Day}",
        "type": "column",
        "valueField": "EnrgDay"
    },
    {
        "balloonText": "[[value]] {$msg_stat.kWh}",
        "dateFormat": "MMM DD",
        "fillAlphas": 1,
        "fillColors": "#BBBBBB",
        "fillToGraph": "AmGraph-1",
        "id": "EnrgNight",
        "legendValueText": "",
        "lineColor": "#666161",
        "lineThickness": 0,
        "title": "{$msg_stat.Night}",
        "type": "column",
        "valueField": "EnrgNight"
    },
    {
        "id": "PF",
        "lineColor": "#00CC00",
        "lineThickness": 3,
        "title": "{$msg_stat.PowerFactor}",
        "type": "smoothedLine",
        "valueAxis": "PF",
        "valueField": "PFavg"
    },
    {
        "balloonText": "[[value]] {$msg_stat.W}",
        "id": "PWR",
        "lineColor": "#9400D3",
        "lineThickness": 2,
        "title": "{$msg_stat.PwrAVG}",
        "type": "step",
        "valueAxis": "pwr",
        "valueField": "Pavg"
    }
],
"valueAxes": [
    {
        "axisFrequency": 0,
        "id": "Enrg",
        "precision": 1,
        "stackType": "regular",
        "labelOffset": -20,
        "totalText": "",
        "unit": "",
        "title": "{$msg_stat.kWh}"
    },
    {
        "id": "PF",
        "maximum": 100,
        "minimum": 0,
        "position": "left",
        "precision": 0,
        "strictMinMax": true,
        "color": "#00CC00",
        "gridColor": "#00CC00",
        "labelOffset": -20,
        "inside": false,
        "offset": 50,
        "title": "{$msg_stat.PFavg}",
        "titleColor": "#00CC00",
        "titleRotation": -90
    },
    {
        "id": "pwr",
        "position": "top",
        "axisThickness": 2,
        "color": "#9400D3",
        "gridColor": "#9400D3",
        "gridThickness": 0,
        "labelOffset": -20,
        "title": "{$msg_stat.PwrAVG} {$msg_stat.kW}",
        "titleColor": "#9400D3"
    }
],
"allLabels": [],
"balloon": {},
"legend": {
    "enabled": true,
    "markerType": "diamond",
    "useGraphSettings": true
},
"titles": [
    {
        "id": "mdailystat",
        "size": 15,
        "text": "{$msg_stat.tDaily}"
    }
],
"dataProvider": [
    {foreach key=row item=data from=$mstat}
    {
        {foreach key=hdr item=val from=$data}
            "{$hdr}": "{$val}{if $val@last}"{else}",{/if}
        {/foreach}
    {if $data@last}{rdelim}{else}{rdelim},{/if}
    {/foreach}
]
}
