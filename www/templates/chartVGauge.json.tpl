{
    "type": "gauge",
    "marginBottom": 5,
    "marginTop": 5,
    "startDuration": 2,
    "accessible": false,
    "fontSize": 13,
    "theme": "dark",
    "creditsPosition": "bottom-left",
    "arrows": [
        {
            "id": "Voltage",
            "innerRadius": "70%",
	    "radius": "105%",
            "startWidth": 8,
            "value": {$last[0].U}
        }
    ],
    "axes": [
        {
            "axisThickness": 1,
            "topText": "{$last[0].U} {$msg_stat.U}",
            "topTextYOffset": 30,
            "bottomText": "{$msg_stat.voltage}",
	    "bottomTextYOffset": 5,
            "endValue": 260,
            "fontSize": 14,
            "id": "voltage-last",
            "startValue": 190,
            "valueInterval": 10,
            "bands": [
            {
                "alpha": 0.7,
                "color": "#00CC00",
                "endValue": 240,
                "id": "NormalV",
                "startValue": 210
                },
            {
                "alpha": 0.7,
                "color": "#ffac29",
                "endValue": 210,
                "id": "lowV",
                "startValue": 190
            },
            {
                "alpha": 0.7,
                "color": "#EA3838",
                "endValue": 260,
                "gradientRatio": [],
                "id": "HighV",
                "innerRadius": "95%",
                "startValue": 240
            }
            ]
        }
    ],
    "allLabels": [],
    "balloon": {},
    "titles": []
}
