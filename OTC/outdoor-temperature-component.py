import os
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import datetime
from meteostat import Point, Daily
import pandas as pd

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
    influx_url = "http://influxdb:8086"


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


class InfluxInfo:
    def __init__(self, device_id):
        super().__init__()
        query = ' from(bucket:"{}")\
                    |> range(start: -30d, stop: now())\
                    |> filter(fn:(r) => r.device_id == "{}")\
                    |> first() '.format(read_bucket, device_id)
        result = query_api.query(org=org, query=query)
        for table in result:
            for record in table.records:
                gps = record.values.get("GPS")
                self.first_dt = record.get_time()
        query = ' from(bucket:"{}")\
                    |> range(start: -30d, stop: now())\
                    |> filter(fn:(r) => r.device_id == "{}")\
                    |> last() '.format(read_bucket, device_id)
        result = query_api.query(org=org, query=query)
        for table in result:
            for record in table.records:
                self.last_dt = record.get_time()

        print("InfluxInfo initialized! Found\nGPS: %s\nFirst DateTime: %s\nLast DateTime: %s" % (
            gps, self.first_dt, self.last_dt))

        gps_list = gps.split(",")
        self.lat = float(gps_list[0])
        self.long = float(gps_list[1])

    def _getLat(self) -> float:
        return self.lat

    def _getLong(self) -> float:
        return self.long

    def _getFirstDT(self) -> datetime.date:
        return datetime.datetime(self.first_dt.year, self.first_dt.month, self.first_dt.day)

    def _getLastDT(self) -> datetime.date:
        return datetime.datetime(self.last_dt.year, self.last_dt.month, self.last_dt.day)

# Function to send the otc fetched data to influxdb bucket


def send_otc_data(data):
    data.reset_index(inplace=True)
    data["_time"] = pd.to_datetime(data["time"])
    data.set_index("_time", inplace=True)

    data.drop(columns=["time", "tmin", "tmax", "prcp", "snow",
              "wdir", "wspd", "wpgt", "pres", "tsun"], inplace=True)

    write_api.write(bucket=write_bucket, org=org, record=data,
                    data_frame_measurement_name="otc")
    print("Successfully sent data to influxdb!")


def main():
    print("Gathering info from influxdb bucket: %s" % read_bucket)
    info = InfluxInfo(1)

    # Getting daily values for the chosen locality from 15 days before the first gathered data
    print("Getting outdoor temperature value through meteostat")
    data = Daily(Point(info._getLat(), info._getLong()),
                 info._getFirstDT() - datetime.timedelta(15), info._getLastDT()).fetch()

    print("Sending fetched data to influxdb")
    send_otc_data(data)


if __name__ == "__main__":
    main()
