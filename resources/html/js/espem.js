function rawdata_cb(obj) {
    console.log('Got my raw data:', obj);
    var frame = obj.block;
    if (!obj.block){
        console.log('Message has no data block!');
        return;
    }

    for (var i = 0; i < frame.length; i++) if (typeof frame[i] == "object") {
        if (frame[i].id === "U"){
            var element = document.getElementById("gaugeV");
            GVchart.arrows[0].setValue(frame[i].value);
            GVchart.axes[0].setTopText(frame[i].value.toFixed(0) + ' Volts');
        }
        if (frame[i].id === "Pf"){
            var element = document.getElementById("gaugePF");
            GPFchart.arrows[0].setValue(frame[i].value * 100);
            GPFchart.axes[0].setTopText('PF ' + (frame[i].value * 100).toFixed(0) + '%');
            GPFchart.axes[0].bands[0].setEndValue(frame[i].value * 100);
            GPFchart.axes[0].bands[1].setStartValue(frame[i].value * 100);
        }

        // requred for left-menu renderer
        frame[i].html = true;
    }

    // pass data to the renderer to make it available under Menu area
    rdr.value(obj);
}

var GVchart = null;
var GPFchart = null;
var Gsminichart = null;

function mkchart(id, param){
    var element = document.getElementById(id);

    if (!element){
        console.log('Element with id', id, ' not found!');
        return
    }
    console.log('Attempt to build el id', id);

    switch (id) {
        case "gaugeV" : GVchart = AmCharts.makeChart(element, {
            "type": "gauge",
            "marginBottom": -10,
            "marginTop": 25,
            "startDuration": 2,
            "accessible": false,
            "creditsPosition": "bottom-left",
            "fontSize": 13,
            "theme": "dark",
            "arrows": [
                {
                    "id": "Voltage",
                    "innerRadius": "80%",
                    "radius": "110%",
                    "startWidth": 8,
                    "value": 190
                }
            ],
            "axes": [
                {
                    "axisThickness": 1,
                    "bottomText": "Voltage",
                    "bottomTextYOffset": 5,
                    "endValue": 270,
                    "fontSize": 14,
                    "id": "voltage-last",
                    "startValue": 190,
                    "topText": "---",
                    "topTextYOffset": 30,
                    "valueInterval": 20,
                    "bands": [
                        {
                            "alpha": 0.7,
                            "color": "#F7D442",
                            "endValue": 210,
                            "id": "lowV",
                            "innerRadius": "108%",
                            "startValue": 190
                        },
                        {
                            "alpha": 0.7,
                            "color": "#00CC00",
                            "endValue": 250,
                            "id": "NormalV",
                            "innerRadius": "108%",
                            "startValue": 210
                        },
                        {
                            "alpha": 0.7,
                            "color": "#EA3838",
                            "endValue": 270,
                            "gradientRatio": [],
                            "id": "HighV",
                            "innerRadius": "108%",
                            "startValue": 250
                        }
                    ]
                }
            ],
                "allLabels": [], "balloon": {}, "titles": []
            });
                    console.log('Created chart ', element);
                    break;
        case "gaugePF" :
            GPFchart = AmCharts.makeChart(element, {
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
                        "value": 0
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
                        "topText": "PF 0%",
                        "topTextFontSize": 20,
                        "topTextYOffset": 25,
                        "bands": [
                            {
                                "color": "#47ea19",
                                "startValue": 0,
                                "endValue": 100,
                                "gradientRatio": [
                                    0.5,
                                    0,
                                    -0.5
                                ],
                                "id": "GaugeBand-1",
                                "innerRadius": "105%",
                                "radius": "170%"
                            },
                            {
                                "color": "#b5f0fc",
                                "endValue": 100,
                                "gradientRatio": [
                                    0.5,
                                    0,
                                    -0.5
                                ],
                                "id": "GaugeBand-2",
                                "innerRadius": "105%",
                                "radius": "170%",
                                "startValue": 100
                            }
                        ]
                    }
                ],
                "allLabels": [],
                "balloon": {},
                "titles": []
            });
            console.log('Created chart ', element);
            break;

        case "gsmini" : Gsminichart = AmCharts.makeChart(element, {
                "type": "serial",
                "dataLoader": { "url" : "http://"+location.host+"/samples.json", "reload": 15},
                "categoryField": "t",
                "sequencedAnimation": false,
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
                    "title": "Power",
                    "type": "step",
                    "valueAxis": "vaP",
                    "valueField": "P"
                },
                {
                    "id": "gPF",
                    "lineColor": "#23EF23",
                    "lineThickness": 2,
                    "title": "Power Factor}",
                    "type": "smoothedLine",
                    "valueAxis": "vaPF",
                    "valueField": "pF"
                }
                ],
                "valueAxes": [
                {
                    "id": "vaP",
                    "unit": "W",
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
                } });
            
                //Gsminichart.dataLoader.url = "http://"+location.host+"/samples";

        default :
    }

}
  