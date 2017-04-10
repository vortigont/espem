{
    "type": "serial",
    "dataLoader": {
        "url": "http://{$esphost}/samples"
    },
    "categoryField": "t",
    "sequencedAnimation": false,
    "startDuration": 2,
    "startEffect": "easeOutSine",
    "backgroundColor": "#000000",
    "borderColor": "#111111",
    "theme": "black",
    "creditsPosition": "bottom-right",
    "categoryAxis": {
	"gridPosition": "start",
	"minPeriod": "ss",
	"parseDates": true
    },
    "graphs": [
	{
	    "id": "gPWR",
	    "lineColor": "#FF0000",
	    "lineThickness": 2,
	    "title": "{$msg_stat.power|capitalize}",
	    "type": "step",
	    "valueAxis": "vaP",
	    "valueField": "P"
	},
	{
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
	"enabled": false
    },
    "export": {
	"enabled": false
    }
}