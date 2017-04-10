{
    "type": "serial",
    "dataLoader": {
        "url": "http://{$esphost}/samples"
    },
    "categoryField": "t",
    "mouseWheelZoomEnabled": true,
    "sequencedAnimation": false,
    "startDuration": 1,
    "startEffect": "easeOutSine",
    "backgroundColor": "#000000",
    "borderColor": "#111111",
    "theme": "black",
    "titles": [
	{
	    "id": "Title-1",
	    "size": 15,
	    "text": "Power chart"
	}
    ],
    "categoryAxis": {
	"gridPosition": "start",
	"minPeriod": "ss",
	"parseDates": true
    },
    "chartCursor": {
	"enabled": true,
	"categoryBalloonAlpha": 0.8,
	"categoryBalloonDateFormat": "J:NN:SS",
	"cursorColor": "#79F4F0",
	"valueLineAlpha": 0.4,
	"valueLineBalloonEnabled": true,
	"valueLineEnabled": true
    },
    "chartScrollbar": {
	"enabled": true,
	"autoGridCount": true,
	"backgroundAlpha": 0,
	"color": "#888888",
	"graph": "gPWR",
	"graphFillAlpha": 0,
	"graphLineAlpha": 0.2,
	"graphLineColor": "#c2c2c2",
	"graphType": "smoothedLine",
	"gridAlpha": 0,
	"hideResizeGrips": true,
	"offset": 5,
	"scrollbarHeight": 55,
	"scrollDuration": 0,
	"selectedBackgroundAlpha": 0.1,
	"selectedBackgroundColor": "#888888",
	"selectedGraphFillAlpha": 0,
	"selectedGraphLineAlpha": 1,
	"selectedGraphLineColor": "#888888"
    },
    "graphs": [
	{
	    "balloonText": "[[value]]{$msg_stat.U}",
	    "id": "Voltage",
	    "lineColor": "#B69F30",
	    "title": "{$msg_stat.voltage|capitalize}",
	    "type": "step",
	    "valueAxis": "vaU",
	    "valueField": "U"
	},
	{
	    "balloonText": "[[value]]{$msg_stat.P}",
	    "id": "gPWR",
	    "lineColor": "#FF0000",
	    "lineThickness": 2,
	    "title": "{$msg_stat.power|capitalize}",
	    "type": "step",
	    "valueAxis": "vaP",
	    "valueField": "P"
	},
	{
	    "balloonText": "{$msg_stat.PF} [[value]]",
	    "id": "gPF",
	    "lineColor": "#23EF23",
	    "lineThickness": 2,
	    "title": "{$msg_stat.PowerFactor|capitalize}",
	    "type": "smoothedLine",
	    "valueAxis": "vaPF",
	    "valueField": "pF"
	}
    ],
    "valueAxes": [
	{
	    "id": "vaU",
	    "unit": "{$msg_stat.U}",
	    "axisColor": "#FCD202",
	    "color": "#FCD202",
	    "gridThickness": 0,
	    "offset": 60
	},
	{
	    "id": "vaP",
	    "unit": "{$msg_stat.P}",
	    "axisColor": "#FF0000",
	    "axisThickness": 2,
	    "color": "#FF0000",
	    "dashLength": 0
	},
	{
	    "id": "vaPF",
	    "maximum": 1,
	    "minimum": 0,
	    "position": "right",
	    "strictMinMax": true,
	    "color": "#23EF23"
	}
    ],
    "legend": {
	"enabled": true,
	"useGraphSettings": true
    },
    "export": {
	"enabled": true
    }
}