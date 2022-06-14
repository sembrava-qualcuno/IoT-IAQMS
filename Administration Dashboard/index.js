const express = require('express')
const bodyParser = require('body-parser')
const cors = require('cors')
const axios = require('axios')

const url = "http://influxdb:8086"
const token = "LSDJ8tdf7oJLybsrsApPyHb-Kf4zy7CWcuIH1v8xsFZgjOaqR39shBPgKaw5ADAXhC8S8FY5npcI5fDL2iL52Q=="
const org = "sembrava_qualcuno"

const grafanaUrl = "http://grafana:3000"
const grafanaToken = "eyJrIjoiODVyT0ZzZmw3azlZMXBUaHo4YWpnZjJUd1hYaGdmVUIiLCJuIjoiQWRtaW5pc3RyYXRpb24gRGFzaGJvYXJkIiwiaWQiOjF9"

const Influx = require('@influxdata/influxdb-client')
const { error } = require('console')
const { DashboardsAPI } = require('@influxdata/influxdb-client-apis')

const queryApi = new Influx.InfluxDB({url, token}).getQueryApi(org)

// Initialize REST API
const app = express()
const port = process.env.PORT ? process.env.PORT : 3002

app.use(cors({   origin: '*'}))

// Configuring body parser middleware
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())

app.use(express.static('public'));

let devices = []

var grafanaFolderId

// Api errors
const apiErrorDevice = {"code": 1, "message": "The device id must be a positive integer"}
const apiErrorDeviceAlreadyExists = {"code": 2, "message": "The device already exists"}
const apiErrorDeviceNotExists = {"code": 3, "message": "The device not exists"}

const apiErrorSyntaxError = {"code": 5, "message": "Syntax error"}

// Check if there is a JSON parsing error
app.use((err, req, res, next) => {
    if (err instanceof SyntaxError && err.status === 400 && 'body' in err) {
        return res.status(400).json(apiErrorSyntaxError);
    }
    next();
})

app.post('/device', (req, res) => {
    const deviceId = parseInt(req.body.deviceId)

    if(typeof deviceId != 'undefined' && deviceId > 0) {
        if(devices.includes(deviceId)) {
            console.log("Error: ", apiErrorDeviceAlreadyExists)
            res.status(403).json(apiErrorDeviceAlreadyExists)
        }
        else {            
            axios
                .post(grafanaUrl + "/api/dashboards/db", {
                        "dashboard": {
                            "panels": [
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": [
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "A"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Actual"
                                                    }
                                                ]
                                            },
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "B"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Forecasted"
                                                    }
                                                ]
                                            }
                                        ]
                                    },
                                    "gridPos": {
                                        "h": 8,
                                        "w": 9,
                                        "x": 0,
                                        "y": 0
                                    },
                                    "id": 2,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": `from(bucket: \"sensor-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._measurement == \"IoT-Device\" and\r\n    r.device_id == \"${deviceId}\" and\r\n    r._field == \"temp\"\r\n  )`,
                                            "refId": "A"
                                        },
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "hide": false,
                                            "query": "from(bucket: \"data-analytics\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._field == \"temp\"\r\n  )",
                                            "refId": "B"
                                        }
                                    ],
                                    "title": "Temperature",
                                    "type": "timeseries"
                                },
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": [
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "B"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Forecasted"
                                                    }
                                                ]
                                            },
                                            {
                                                "__systemRef": "hideSeriesFrom",
                                                "matcher": {
                                                    "id": "byNames",
                                                    "options": {
                                                        "mode": "exclude",
                                                        "names": [
                                                            "Actual"
                                                        ],
                                                        "prefix": "All except:",
                                                        "readOnly": true
                                                    }
                                                },
                                                "properties": [
                                                    {
                                                        "id": "custom.hideFrom",
                                                        "value": {
                                                            "legend": false,
                                                            "tooltip": false,
                                                            "viz": false
                                                        }
                                                    }
                                                ]
                                            },
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "A"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Actual"
                                                    }
                                                ]
                                            }
                                        ]
                                    },
                                    "gridPos": {
                                        "h": 8,
                                        "w": 8,
                                        "x": 9,
                                        "y": 0
                                    },
                                    "id": 4,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": `from(bucket: \"sensor-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._measurement == \"IoT-Device\" and\r\n    r.device_id == \"${deviceId}\" and\r\n    r._field == \"hum\"\r\n  )`,
                                            "refId": "A"
                                        },
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "hide": false,
                                            "query": "from(bucket: \"data-analytics\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._field == \"hum\"\r\n  )",
                                            "refId": "B"
                                        }
                                    ],
                                    "title": "Humidity",
                                    "type": "timeseries"
                                },
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": [
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "B"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Forecasted"
                                                    }
                                                ]
                                            },
                                            {
                                                "matcher": {
                                                    "id": "byFrameRefID",
                                                    "options": "A"
                                                },
                                                "properties": [
                                                    {
                                                        "id": "displayName",
                                                        "value": "Actual"
                                                    }
                                                ]
                                            }
                                        ]
                                    },
                                    "gridPos": {
                                        "h": 8,
                                        "w": 7,
                                        "x": 17,
                                        "y": 0
                                    },
                                    "id": 6,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": `from(bucket: \"sensor-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._measurement == \"IoT-Device\" and\r\n    r.device_id == \"${deviceId}\" and\r\n    r._field == \"gas\"\r\n  )`,
                                            "refId": "A"
                                        },
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "hide": false,
                                            "query": "from(bucket: \"data-analytics\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._field == \"gas\"\r\n  )",
                                            "refId": "B"
                                        }
                                    ],
                                    "title": "Gas",
                                    "type": "timeseries"
                                },
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": []
                                    },
                                    "gridPos": {
                                        "h": 6,
                                        "w": 12,
                                        "x": 0,
                                        "y": 8
                                    },
                                    "id": 8,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": `from(bucket: \"sensor-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._measurement == \"IoT-Device\" and\r\n    r.device_id == \"${deviceId}\" and\r\n    r._field == \"AQI\"\r\n  )`,
                                            "refId": "A"
                                        }
                                    ],
                                    "title": "AQI",
                                    "type": "timeseries"
                                },
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": []
                                    },
                                    "gridPos": {
                                        "h": 6,
                                        "w": 12,
                                        "x": 12,
                                        "y": 8
                                    },
                                    "id": 10,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": `from(bucket: \"sensor-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._measurement == \"IoT-Device\" and\r\n    r.device_id == \"${deviceId}\" and\r\n    r._field == \"RSSI\"\r\n  )`,
                                            "refId": "A"
                                        }
                                    ],
                                    "title": "RSSI",
                                    "type": "timeseries"
                                },
                                {
                                    "datasource": {
                                        "type": "influxdb",
                                        "uid": "rFoKsy97z"
                                    },
                                    "fieldConfig": {
                                        "defaults": {
                                            "color": {
                                                "mode": "palette-classic"
                                            },
                                            "custom": {
                                                "axisLabel": "",
                                                "axisPlacement": "auto",
                                                "barAlignment": 0,
                                                "drawStyle": "line",
                                                "fillOpacity": 0,
                                                "gradientMode": "none",
                                                "hideFrom": {
                                                    "legend": false,
                                                    "tooltip": false,
                                                    "viz": false
                                                },
                                                "lineInterpolation": "linear",
                                                "lineWidth": 1,
                                                "pointSize": 5,
                                                "scaleDistribution": {
                                                    "type": "linear"
                                                },
                                                "showPoints": "auto",
                                                "spanNulls": false,
                                                "stacking": {
                                                    "group": "A",
                                                    "mode": "none"
                                                },
                                                "thresholdsStyle": {
                                                    "mode": "off"
                                                }
                                            },
                                            "mappings": [],
                                            "thresholds": {
                                                "mode": "absolute",
                                                "steps": [
                                                    {
                                                        "color": "green",
                                                        "value": null
                                                    },
                                                    {
                                                        "color": "red",
                                                        "value": 80
                                                    }
                                                ]
                                            }
                                        },
                                        "overrides": []
                                    },
                                    "gridPos": {
                                        "h": 8,
                                        "w": 12,
                                        "x": 0,
                                        "y": 14
                                    },
                                    "id": 12,
                                    "options": {
                                        "legend": {
                                            "calcs": [],
                                            "displayMode": "list",
                                            "placement": "bottom"
                                        },
                                        "tooltip": {
                                            "mode": "single",
                                            "sort": "none"
                                        }
                                    },
                                    "targets": [
                                        {
                                            "datasource": {
                                                "type": "influxdb",
                                                "uid": "rFoKsy97z"
                                            },
                                            "query": "from(bucket: \"outdoor-temperature-data\")\r\n  |> range(start: v.timeRangeStart, stop:v.timeRangeStop)\r\n  |> filter(fn: (r) =>\r\n    r._field == \"tavg\"\r\n  )",
                                            "refId": "A"
                                        }
                                    ],
                                    "title": "Average Outdoor Temperature Data",
                                    "type": "timeseries"
                                }
                            ],
                            "refresh": "5s",
                            "schemaVersion": 36,
                            "style": "dark",
                            "tags": [],
                            "time": {
                                "from": "now-1h",
                                "to": "now+5m"
                            },
                            "title": `Device ${deviceId}`
                        },
                        "folderId": grafanaFolderId
                }, {
                    headers: {
                        'Authorization': `Bearer ${grafanaToken}`
                    }
                })
                .then(res => {
                    if(res.status == 200)
                        console.log("Grafana dashboard created: ", res.data.url)                
                })
                .catch(error => {
                    console.error(error)
                })

                console.log("Device added: ", deviceId)
                devices.push(deviceId)
                res.send()   
        }
    }
    else {
        console.log("Error: ", apiErrorDevice)
        res.status(400).json(apiErrorDevice)
    }
})

app.get('/devices', (req, res) => {
    res.json(devices)
})

app.get('/devices/:deviceId', (req, res) => {
    const deviceId = parseInt(req.params.deviceId)

    if(typeof deviceId == 'undefined') {
        res.status(400).json(apiErrorDevice)
        return
    }

    if(deviceId <= 0) {
        res.status(400).json(apiErrorDevice)
        return
    }
    
    if(!devices.includes(deviceId)) {
        res.status(404).json(apiErrorDeviceNotExists)
        return
    }

    const fluxQuery = `from(bucket: "sensor-data")\
                        |> range(start: 0)\
                        |> filter(fn: (r) => r["_measurement"] == "IoT-Device")\
                        |> filter(fn: (r) => r["device_id"] == "${deviceId}")\
                        |> last()`
        
    var GPS
    queryApi.queryRows(fluxQuery, {
        next(row, tableMeta) {
            const o = tableMeta.toObject(row)
            GPS = o.GPS
            
            //console.log(`${o.GPS}`)
        },
        error(error) {
            console.error(error)
            console.log('Finished ERROR')
        },
        complete() {
            console.log('Finished SUCCESS')

            var device = {}
            device.deviceId = deviceId

            if(typeof GPS != 'undefined') {
                var coordinates  = GPS.split(",")
                device.GPS = {lat: coordinates[0], long: coordinates[1]}
            }
            else
                device.GPS = null

            //Retrieve Grafana Dashboard
            axios
                .get(grafanaUrl + "/api/search?type=dash-db", {
                    headers: {
                        'Authorization': `Bearer ${grafanaToken}`
                    }
                })
                .then(resGrafana => {
                    if(resGrafana.status == 200) {
                        dashboard = resGrafana.data.filter(dashboard => dashboard.folderId == grafanaFolderId && dashboard.title == `Device ${deviceId}`)
                        device.link = grafanaUrl + dashboard[0].url
                        res.json(device)
                    }
                })
                .catch(error => {
                    device.link = null
                    res.json(device)
                })
        },
    })
})

//Create GRAFANA folder
axios
    .post(grafanaUrl + "/api/folders", {
        "title": "Administration Dashboard"
    }, {
        headers: {
            'Authorization': `Bearer ${grafanaToken}`
        }
    })
    .then(res => {
        if(res.status == 200) {
            grafanaFolderId = res.data.id
            console.log("Grafana folder created, id: ", grafanaFolderId)
        }
    })
    .catch(error => {
        if(error.response.status == 409) {
            console.log("Grafana folder already exists")
            axios
                .get(grafanaUrl + "/api/folders", {
                    headers: {
                        'Authorization': `Bearer ${grafanaToken}`
                    }
                })
                .then(res => {
                    if(res.status == 200) {
                        folder = res.data.filter(dashboard => dashboard.title === "Administration Dashboard")
                        grafanaFolderId = folder[0].id
                        console.log("Grafana folder created, id: ", grafanaFolderId)
                    }
                })
        }
        else
            console.error(error);
    });

app.listen(port, '0.0.0.0', () => console.log(`Device configuration API listening on port ${port}!`))
