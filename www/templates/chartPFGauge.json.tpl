{
    "type": "gauge",
    "marginTop": 0,
    "marginBottom": 0,
    "marginRight": 0,
    "marginLeft": 0,
    "startDuration": 2,
    "theme": "dark",
    "creditsPosition": "bottom-left",
    "arrows": [
        {
            "alpha": 1,
            "id": "PF",
            "innerRadius": "95%",
            "nailRadius": 0,
            "radius": "170%",
            "startWidth": 5,
            "value": {$last[0].PF}
        }
    ],
    "axes": [
        {
            "axisColor": "#31d6ea",
            "axisThickness": 1,
            "bottomText": "",
            "centerY": "30%",
            "endAngle": 90,
            "endValue": 100,
            "id": "GaugeAxis-1",
            "radius": "80%",
            "startAngle": -90,
            "labelFrequency": 0,
            "tickColor": "#67b7dc",
            "topText": "{$msg_stat.PF} {$last[0].PF}%",
            "topTextFontSize": 20,
            "topTextYOffset": 25,
            "bands": [
                {
                    "color": "#0080ff",
                    "endValue": 100,
                    "gradientRatio": [
                        0.5,
                        0,
                        -0.5
                    ],
                    "id": "GaugeBand-1",
                    "innerRadius": "105%",
                    "radius": "170%",
                    "startValue": 0
                },
                {
                    "color": "#3cd3a3",
                    "endValue": {$last[0].PF},
                    "gradientRatio": [
                        0.5,
                        0,
                        -0.5
                    ],
                    "id": "GaugeBand-2",
                    "innerRadius": "105%",
                    "radius": "170%",
                    "startValue": 0
                }
            ]
        }
    ],
    "allLabels": [],
    "balloon": {},
    "titles": []
}