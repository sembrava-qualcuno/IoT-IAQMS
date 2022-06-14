function addDevice() {
    device_id = document.getElementById('device_id').value;
    console.log(device_id)
    var xhr = new XMLHttpRequest();

    xhr.open("POST", "/device/", true);
    xhr.setRequestHeader("Content-Type", "application/json");
    var data = JSON.stringify({ "deviceId": device_id });
    xhr.send(data);

    window.location.reload();
}

function loadBooks() {
    const xhttp = new XMLHttpRequest();

    xhttp.open("GET", "/devices", false);
    xhttp.send();

    const devices = JSON.parse(xhttp.responseText);

    for (let device of devices) {
        const x = `
    <div class="col-4">
        <div class="card">
            <div class="card-body">
                <h5 class="card-title">Device ${device}</h5>
    
                <hr>
    
                <button types="button" class="btn btn-primary" data-toggle="modal" data-target="#editBookModal"
                    onClick="showDevice(${device})">
                    Show Device Info
                </button>
            </div>
        </div>
    </div>
    `

        document.getElementById('devices').innerHTML = document.getElementById('devices').innerHTML + x;
    }
}

function showDevice(device_id) {
    window.location.replace("/device.html?deviceId=" + device_id);
}

function showDeviceInfo(device_id) {
    var xhr = new XMLHttpRequest();

    xhr.open("GET", "/devices/" + device_id, true);
    var data = JSON.stringify({ "deviceId": device_id });
    xhr.onload = function (e) {
        if (xhr.readyState === 4) {
            if (xhr.status === 200) {
                const device = JSON.parse(xhr.responseText);
                if (device.GPS !== null) {
                    var map = L.map('deviceMap').setView([device.GPS.lat, device.GPS.long], 13);

                    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
                        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
                    }).addTo(map);

                    L.marker([device.GPS.lat, device.GPS.long]).addTo(map)
                        .bindPopup('Device ' + device_id)
                        .openPopup();
                }
                else{
                    document.getElementById('deviceMap').innerText = "No GPS position available for this device!"
                }
                if(device.link !== null)
                    document.getElementById('grafana').innerHTML = '<a href=' + device.link + '>Go to the Grafana Dashboard</a>'
                else
                document.getElementById('grafana').innerText = "No Grafana Dashboard available for this device!"
            } else {
                console.error(xhr.statusText);
            }
        }
    };
    xhr.send(data);

    
}

function sendParameters() {    
    protocol = $("input[type=radio][name=inlineRadioOptions]:checked").val()
    sampleFrequency = document.getElementById('sampleFrequency').value
    minGasValue = document.getElementById('minGasValue').value
    maxGasValue = document.getElementById('maxGasValue').value
    device_id = urlParams.get("deviceId")


    var data = {}

    data.protocol = protocol
    if(sampleFrequency != '')
        data.sampleFrequency = sampleFrequency
    if(minGasValue != '')
        data.minGasValue = minGasValue
    if(maxGasValue != '')
        data.maxGasValue = maxGasValue

    console.log(data)

    var xhr = new XMLHttpRequest();
    
    xhr.open("PUT", "http://configuration-microservice:3001/devices/" + device_id, true);
    xhr.setRequestHeader("Content-Type", "application/json");

    xhr.send(JSON.stringify(data));    
}
