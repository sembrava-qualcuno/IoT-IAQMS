import os
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import datetime
from meteostat import Point, Hourly
import pandas as pd
import schedule
import time
from datetime import timezone

# influx parameters
if "INFLUX_WRITE_BUCKET" in os.environ:
    write_bucket = os.environ["INFLUX_WRITE_BUCKET"]
else:
    write_bucket = "outdoor-temperature-data"

if "INFLUX_READ_BUCKET" in os.environ:
    read_bucket = os.environ["INFLUX_READ_BUCKET"]
else:
    read_bucket = "sensor-data"

if "INFLUX_ORG" in os.environ:
    org = os.environ["INFLUX_ORG"]
else:
    org = "sembrava_qualcuno"

if "INFLUX_TOKEN" in os.environ:
    token = os.environ["INFLUX_TOKEN"]
else:
    token = "dN1RDF-P_up739OqDg4LuYRvZK4-9EVfrXyhR3ZPIgsJfHVuX6doNSYBHgMD9y3mfB0yXIJOEav8rJFOi0xk0g=="

if "INFLUX_URL" in os.environ:
    influx_url = os.environ["INFLUX_URL"]
else:
    influx_url = "http://localhost:8086"

if "MEASUREMENT" in os.environ:
    measurement = os.environ["MEASUREMENT"]
else:
    measurement = "IoT-Device"

if "DEVICEID" in os.environ:
    device_id = os.environ["DEVICEID"]
else:
    device_id = 1

print("Attempting to connect to: %s\nOrg: %s\nBucket: %s" %
      (influx_url, org, read_bucket))
influx_client = influxdb_client.InfluxDBClient(
    url=influx_url,
    token=token,
    org=org)
print("Created read client for influx connection!")
query_api = influx_client.query_api()
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

# Class that contains GPS values and first and last timestamps of Influx data


def get_GPS() -> str:
    query = ' from(bucket:"{}")\
                    |> range(start: 0)\
                    |> filter(fn:(r) => r.device_id == "{}")\
                    |> filter(fn:(r) => r._measurement == "{}")\
                    |> last() '.format(read_bucket, device_id, measurement)
    result = query_api.query(org=org, query=query)
    for table in result:
        for record in table.records:
            gps = record.values.get("GPS")

    print("Got \nGPS: %s\n" % gps)
    return gps


print("Gathering gps from influxdb bucket: %s" % read_bucket)
gps = get_GPS()

# Function to send the otc fetched data to influxdb bucket


def send_otc_data(data):
    tosend = pd.DataFrame()
    tosend["temp"] = data["temp"]
    data.reset_index(inplace=True)
    tosend["_time"] = data["time"].values.astype('<M8[s]')

    tosend.set_index("_time", inplace=True)

    write_api.write(bucket=write_bucket, org=org, record=tosend,
                    data_frame_measurement_name="otc2")
    print("Successfully sent data to influxdb!")


def fetch_and_send():
    gps_list = gps.split(",")
    latitude = float(gps_list[0])
    longitude = float(gps_list[1])

    # Getting last hour hourly values for the chosen locality
    print("Getting last hour outdoor temperature value through meteostat")
    data = Hourly(Point(latitude, longitude),
                  datetime.datetime.now() - datetime.timedelta(hours=1), datetime.datetime.now(), timezone="Europe/Rome").fetch()
    print("Sending fetched data to influxdb")
    print(data)
    send_otc_data(data)


schedule.every().hour.do(fetch_and_send)


def main():

    fetch_and_send()

    while True:
        schedule.run_pending()
        time.sleep(1)


if __name__ == "__main__":
    main()
