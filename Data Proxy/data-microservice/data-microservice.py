from signal import signal, SIGINT
from sys import exit
import paho.mqtt.client as mqtt_client
import aiocoap as coap
import aiocoap.resource as resource
import asyncio
import influxdb_client
from influxdb_client.client.write_api import ASYNCHRONOUS


# mqtt parameters
broker = "localhost"
broker_port = 1883
topic = "/sensor-data"
client_id = "data-microservice-client-1"

# coap parameters
coap_server = "localhost"
coap_port = 8080


# influx parameters
bucket = "sensor-data"
org = "sembrava_qualcuno"
token = "HfiRuB99Z4opkWAeV85Q85TyqDnTc23rIPXjez0Ekh82E3CKC5Y5jMoGtYAbkeTAQfBWIhnO26y1kMwe7AxX8w=="
influx_url = "http://localhost:8086"
influx_client = influxdb_client.InfluxDBClient(
    url=influx_url,
    token=token,
    org=org)
write_api = influx_client.write_api(write_options=ASYNCHRONOUS)


# This function parses received data and sends it to the INFLUX db


def parse_and_send(measurement: str) -> bool:
    data_list = measurement.split(",")

    # Check that data is complete
    if not len(data_list) == 8:
        print("Received data is not complete\nCannot send data to influx!\n")
        return False

    # Check that data is well formatted
    try:
        int(data_list[0])
        int(data_list[3-7])
        float(data_list[1-2])
    except ValueError:
        print("Received data are not well formatted, cannot send data to influx!")
        return False

    # Create Influx point and return

    p = influxdb_client.Point("sensor-data2").tag("device-id", int(data_list[0])).tag("GPS", str(data_list[1])+","+str(data_list[2])).field(
        "temp", int(data_list[3])).field("hum", int(data_list[4])).field("gas", int(data_list[5])).field("AQI", int(data_list[6])).field("RSSI", int(data_list[7]))
    print("Influx record: ", p)
    write_api.write(bucket=bucket, org=org, record=p)
    print("Data successfully sent to Influx")

    return True

###################################### MQTT #########################################

# Create mqtt connection


def connect_mqtt() -> mqtt_client:
    # On connect callback
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!\n")
        else:
            print("Failed to connect, return code %d\n", rc)

    # Create the mqtt client
    client = mqtt_client.Client(client_id)
    client.on_connect = on_connect
    client.connect(broker, broker_port)
    return client

# Let the created client subscribe to the /sensor-data topic and receive data


def subscribe(client: mqtt_client):
    # Define the on message callback to receive data
    def on_message(client, userdata, msg):
        data = msg.payload.decode("utf-8")
        print("Received %s from %s topic" % (data, msg.topic))
        parse_and_send(data)

    # Subscribe to the topic
    client.subscribe(topic)
    client.on_message = on_message

# mqtt main


def run_mqtt():
    client = connect_mqtt()
    subscribe(client)
    # Creates new thread that loops on the mqtt connection
    client.loop_start()

###################################### COAP #########################################

# Create CoAP resource for IoT Device


class IoTDeviceResource(resource.Resource):
    # This resource supports POST method on /sensor-data URL
    def __init__(self):
        super().__init__()

    async def render_post(self, request):
        data = request.payload.decode("utf-8")
        print('Fresh data arrived: %s' % data)

        if parse_and_send(data) is False:
            print("Parse error. Data discarded...\n")
            # Return 4.00 BAD_REQUEST
            return coap.Message(code=128)
        else:
            # Return 2.03 VALID
            return coap.Message(code=67)


def main():
    # Resource tree creation
    print("Starting CoAP server...")
    root = resource.Site()
    root.add_resource(['sensor-data'], IoTDeviceResource())

    # CoAP server creation
    asyncio.Task(coap.Context.create_server_context(
        root, bind=(coap_server, coap_port)))

    # Run MQTT subscriber
    print("Setting up MQTT subscriber")
    run_mqtt()

    try:
        asyncio.get_event_loop().run_forever()
    except KeyboardInterrupt:
        print("\nExiting...")
        exit(0)


if __name__ == "__main__":
    main()
