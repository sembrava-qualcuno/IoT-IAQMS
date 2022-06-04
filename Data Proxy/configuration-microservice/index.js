const express = require('express')
const bodyParser = require('body-parser')
const cors = require('cors')
const mqtt = require('mqtt')

// Initialize mqtt
const broker = process.env.MQTT_BROKER ? process.env.MQTT_BROKER : 'localhost'
const mqttClient = mqtt.connect(`mqtt://${broker}`, {clientId:'configuration-microservice'}) 
// TODO mqtt username & password?

// Initialize REST API
const app = express()
const port = process.env.PORT ? process.env.PORT : 3000

app.use(cors())

// Configuring body parser middleware
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())

// Supported protocols with encoding
const supportedProtocols = {
    'coap': 0,
    'mqtt': 1
}

// Parameters encoding function
function encodeParameters(parameters) {
    return `${typeof parameters.sampleFrequency == 'undefined' ? '' : parameters.sampleFrequency},${typeof parameters.minGasValue == 'undefined' ? '' : parameters.minGasValue},${typeof parameters.maxGasValue == 'undefined' ? '' : parameters.maxGasValue}`
}

// Api errors
const apiErrorNegative = {"code": 1, "message": "The sample frequency, min and max gas value must be a positive integer"}
const apiErrorProtocol = {"code": 2, "message": "Protocol not supported"}
const apiErrorParameter = {"code": 3, "message": "Invalid parameter"}
const apiErrorEmptyRequest = {"code": 4, "message": "Empty request"}
const apiErrorSyntaxError = {"code": 5, "message": "Syntax error"}

// Check if there is a JSON parsing error
app.use((err, req, res, next) => {
    if (err instanceof SyntaxError && err.status === 400 && 'body' in err) {
        return res.status(400).json(apiErrorSyntaxError);
    }
    next();
})

app.put('/devices/:deviceId', (req, res) => {
    const deviceId = req.params.deviceId
    const parameters = req.body
    var validParameters = false
    var resultParameters = {}

    // Check if there is the sample frequency and if it is positive
    if(typeof parameters.sampleFrequency != 'undefined') {
        if(parameters.sampleFrequency <= 0)
            res.status(400).json(apiErrorNegative)
        else {
            validParameters = true
            resultParameters.sampleFrequency = parameters.sampleFrequency
        }
    }

    // Check if there is the min gas value and if it is not negative
    if(typeof parameters.minGasValue != 'undefined') {
        if(parameters.minGasValue < 0)
            res.status(400).json(apiErrorNegative)
        else {
            validParameters = true
            resultParameters.minGasValue = parameters.minGasValue
        }
    }

    // Check if there is the max gas value and if it is not negative
    if(typeof parameters.maxGasValue != 'undefined') {
        if(parameters.maxGasValue < 0)
            res.status(400).json(apiErrorNegative)
        else {
            validParameters = true
            resultParameters.maxGasValue = parameters.maxGasValue
        }
    }

    if(validParameters) // At least one parameter is valid
        mqttClient.publish(`/devices/${deviceId}/parameters`, encodeParameters(resultParameters), {"qos":1});

    // Check if there is the protocol and if it is a supported protocol
    if(typeof parameters.protocol != 'undefined') {
        if(!(parameters.protocol in supportedProtocols))
            res.status(400).json(apiErrorProtocol)
        else {
            validParameters = true;
            mqttClient.publish(`/devices/${deviceId}/protocol`, String(supportedProtocols[parameters.protocol]), {"qos":1})
        }
    }

    if(Object.keys(parameters).length == 0) // The body request is empty
        res.status(400).json(apiErrorEmptyRequest)
    else if(!validParameters) // There are no valid parameter
        res.status(400).json(apiErrorParameter)
    else // At least one parameter is valid
        res.send();
})

app.listen(port, () => console.log(`Device configuration API listening on port ${port}!`))
