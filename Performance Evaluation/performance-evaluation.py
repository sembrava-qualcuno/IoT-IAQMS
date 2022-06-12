from sys import exit
import os, sys
from tokenize import String
import paho.mqtt.client as mqtt_client
import argparse
from socket import gaierror

# mqtt parameters
if "MQTT_BROKER_HOST" in os.environ:
    broker = os.environ["MQTT_BROKER_HOST"]
else:
    broker = "mosquitto"

if "MQTT_BROKER_PORT" in os.environ:
    broker_port = int(os.environ["MQTT_BROKER_PORT"])
else:
    broker_port = 1883

if "MQTT_ACTIVATE_TOPIC" in os.environ:
    activate_topic = os.environ["MQTT_ACTIVATE_TOPIC"]
else:
    activate_topic = "/performance/activate"

if "MQTT_RESULT_TOPIC" in os.environ:
    result_topic = os.environ["MQTT_RESULT_TOPIC"]
else:
    result_topic = "/performance/evaluation"

if "DEVICE_ID" in os.environ:
    device_id = os.environ["DEVICE_ID"]
else:
    device_id = "1"

if "NUM_PACKETS" in os.environ:
    num_packets = os.environ["NUM_PACKETS"]
else:
    num_packets = 20

client_id = "performance-evaluator"

parser = argparse.ArgumentParser(description='Perform a performance evaluation on the specified device')
parser.add_argument('-d', '--deviceID', metavar='deviceID', type=int,
                    help='the device id')
parser.add_argument('-n', '--npackets', metavar='packets', type=int,
                    help='the number of the packets used in the evaluation')
parser.add_argument('--host', metavar='host',
                    help='the mqtt broker hostname')
parser.add_argument('-p', '--port', metavar='port',
                    help='the mqtt broker port')

args = parser.parse_args()
if(args.deviceID != None):
    device_id = args.deviceID
if(args.npackets != None):
    num_packets = args.npackets
if(args.host != None):
    broker = args.host
if(args.port != None):
    broker_port = args.port

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

# Let the created client subscribe to the /performance/evaluation topic and receive data
def subscribe(client: mqtt_client):
    # Define the on message callback to receive data
    def on_message(client, userdata, msg):
        data = msg.payload.decode("utf-8")
        #print("Received %s from %s topic" % (data, msg.topic))
        evaluation = data.split(",")
        
        if(len(evaluation) != 2):
            print("Error: too much arguments for the evaluation results")
            os._exit(1)

        try:
            average_delay = float(evaluation[0])
        except ValueError:
            print("Error: the average delay must be a number")
            os._exit(2)

        try:
            packet_delivery_ratio = float(evaluation[1])
        except ValueError:
            print("Error: the packet delivery ratio must be a number")
            os._exit(2)
    
        print("Average delay: {:.2f} ms".format(average_delay))
        print("Packet delivery ratio: {:.2f} %".format(packet_delivery_ratio))
        os._exit(0)        

    # Subscribe to the topic
    client.subscribe("/devices/" + str(device_id) + result_topic)
    client.on_message = on_message

def main():
    # Run MQTT subscriber
    print("Setting up MQTT subscriber\n Connecting to %s:%s" %
          (broker, broker_port))
    try:
        client = connect_mqtt()
    except gaierror:
        print("Broker name or service not known")
        sys.exit(3)
    subscribe(client)
    # Creates new thread that loops on the mqtt connection
    client.loop_start()

    client.publish("/devices/" + str(device_id) + activate_topic, payload=num_packets)

    while(True):
        pass

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print('Interrupted')
        try:
            sys.exit(0)
        except SystemExit:
            os._exit(0)
