// override variable with ESPEM's API version
app_jsapi = 1;

// default samples chart len
var samples_len = 600;

// raw data coming from the EmbUI handled here
unknown_pkg_callback = function (obj) {
    let frame = obj.block;
    if (!obj.block){
        console.log('Message has no data block!');
        return;
    }
    //console.log('Process raw data:', frame);
    let U, I, Pf, P, W, vals = [];
    for (var i = 0; i != frame.length; i++) if (typeof frame[i] == "object") {
        if (frame[i].stale === true){   // we have stale data for some reason
            GVchart.axes[0].setTopText('Error');
            GPFchart.axes[0].setTopText('Error');
            // set err value for display widgets
            //delete frame[i];
            frame.push({"id":"cur", "value": "err", 'html':true});
            frame.push({"id":"pwr", "value": "err", 'html':true});
            frame.push({"id":"enrg", "value": "err", 'html':true});
            frame.shift();
            rdr.value(obj);
            return;
        }

        // обновить график с новым значением шкалы
        if (frame[i].scntr && Gsminichart){
            samples_len = frame[i].scntr;
            AmCharts.loadFile("/samples.json?scntr=" + frame[i].scntr, {async: true}, function(data) { Gsminichart.dataProvider = AmCharts.parseJSON(data); } );
            Gsminichart.validateData();
            return;
        }

        frame[i].U /= 10
        frame[i].I /= 1000 // normalize to Amps
        frame[i].P /= 10
        frame[i].W /= 1000
        frame[i].freq /= 10
        W = frame[i].W
        U = frame[i].U
        I = frame[i].I
        P = frame[i].P

        GVchart.arrows[0].setValue(frame[i].U.toFixed(0));
        GVchart.axes[0].setTopText(frame[i].U + ' Volts');

        GPFchart.arrows[0].setValue(frame[i].Pf);
        GPFchart.axes[0].setTopText('PF ' + frame[i].Pf + '%');
        GPFchart.axes[0].bands[0].setEndValue(frame[i].Pf);
        GPFchart.axes[0].bands[1].setStartValue(frame[i].Pf);
        frame[i].Pf /= 100
        Pf = frame[i].Pf

        // for left bar panel
        vals.push({'id':'U', 'value': frame[i].U, 'html':true})
        vals.push({'id':'Pf', 'value': frame[i].Pf, 'html':true})
        vals.push({"id":"I", "value":frame[i].I, 'html':true})
        vals.push({"id":"P", "value":frame[i].P, 'html':true})
        vals.push({"id":"W", "value":frame[i].W, 'html':true})
        vals.push({"id":"freq", "value":frame[i].freq, 'html':true})
        // for widgets
        vals.push({"id":"cur", "value":frame[i].I, 'html':true})
        vals.push({"id":"pwr", "value":frame[i].P, 'html':true})
        vals.push({"id":"enrg", "value":frame[i].W, 'html':true})
    }

    // skip if there is no Voltage data
    if (!U) return;

    // overwrite with new vals
    obj.block = vals;

    if (Gsminichart == null) return;
    // Power metrics graph
    if (Gsminichart.dataProvider.length > samples_len) Gsminichart.dataProvider.shift();
    Gsminichart.dataProvider.push( { "t": Math.floor(Date.now()), "U": U, "I": I, "P": P, "W": W, "pF": Pf } );
    Gsminichart.validateData();

    // pass data to the renderer to make it available under Menu/display area
    rdr.value(obj);
}


var GVchart = null;
var GPFchart = null;
var Gsminichart = null;
var minichart_tier = 1;     // chart tier level

function mkgauge(id, param){
    let element = document.getElementById(id);
    if (!element){
        console.log('Element with id', id, ' not found!');
        return
    }
    console.log('Building gauge id:', id);
    switch (id) {
        case "gaugeV" : GVchart = AmCharts.makeChart(element, {
            "type": "gauge",
            "marginTop": 25,
            "marginBottom": -10,
            "startDuration": 1,
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
                    "valueInterval": 10,
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
                    console.log('Created chart ', id);
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
                        "id": "PF",
                        "alpha": 1,
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
                                "gradientRatio": [0.5, 0, -0.5],
                                "id": "GaugeBand-1",
                                "innerRadius": "105%",
                                "radius": "160%"
                            },
                            {
                                "color": "#b5f0fc",
                                "endValue": 100,
                                "gradientRatio": [0.5,0,-0.5],
                                "id": "GaugeBand-2",
                                "innerRadius": "105%",
                                "radius": "160%",
                                "startValue": 100
                            }
                        ]
                    }
                ],
                "allLabels": [],
                "balloon": {},
                "titles": []
            });
            console.log('Created chart ', id);
            break;
            default :
        }
}

function mkchart(obj){
    let id = obj.block[0].id;
    let scnt = obj.block[0].scnt;
    let element = document.getElementById(id);

    if (obj.block[0].tier) minichart_tier = obj.block[0].tier;


    if (!element){
        console.log('Element id:', id, ' not found!');
        return
    }
    console.log('Building chart id:', id);

    Gsminichart = AmCharts.makeChart(element,
        {
        "type": "serial",
        "categoryField": "t",
        "sequencedAnimation": false,
        "backgroundColor": "#000000",
        "borderColor": "#111111",
        "theme": "black",
        "creditsPosition": "top-right",
        "dataLoader": {
            "url" : "/samples.json?tsid=" + minichart_tier + "&scnt=" + scnt,
            "showErrors": false,
            "load": function( options, Gsminichart ) {
                    var pwrGraph = new AmCharts.AmGraph();
                    pwrGraph.valueField = "P";
                    pwrGraph.type = "step";
                    pwrGraph.title = "Power";
                    pwrGraph.lineColor = "#FF0000";
                    pwrGraph.lineThickness = 2;
                    Gsminichart.addGraph( pwrGraph );

                    var pfGraph = new AmCharts.AmGraph();
                    pfGraph.valueField = "pF";
                    pfGraph.type = "smoothedLine";
                    pfGraph.valueAxis = "vaPF";
                    pfGraph.type = "step";
                    pfGraph.title = "Power";
                    pfGraph.lineColor = "#12DE12";
                    pfGraph.lineThickness = 2;
                    Gsminichart.addGraph( pfGraph );
            },
        },
        "categoryAxis": {
            "gridPosition": "start",
            "minPeriod": "ss",
            "parseDates": true},
/*
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
*/
        "valueAxes": [
        {
            "id": "vaP",
            "unit": "W",
            "position": "right",
            "axisColor": "#FF0000",
            "axisThickness": 2,
            "color": "#FF0000",
            "dashLength": 0
        },
        {
            "id": "vaPF",
            "maximum": 1,
            "minimum": 0,
            "position": "left",
            "strictMinMax": true,
            "color": "#23EF23"
        }
        ],
        "legend": {
        "enabled": false
        },
        "export": {
        "enabled": false
        } }
    );

    console.log("created gsmini", element);
}
