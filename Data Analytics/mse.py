import numpy as np
from numpy import False_
import pandas as pd
import time
from datetime import datetime
from prophet import Prophet
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import os
from prophet.diagnostics import cross_validation
from prophet.diagnostics import performance_metrics
from prophet.plot import plot_cross_validation_metric
import chart_studio.plotly as py
import plotly.io as pio
pio.renderers.default = "browser"

# influx parameters
if "INFLUX_WRITE_BUCKET" in os.environ:
    write_bucket = os.environ["INFLUX_WRITE_BUCKET"]
else:
    write_bucket = "data-analytics"

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
    token = "-2mg-iFxwmOqYxW59M5K-O2JSOU0docZjopGuK3IHQuv5JCDY-giEEYwIi-fAjYM_7VFZT0kiIacA6MP6BCn5A=="

if "INFLUX_URL" in os.environ:
    influx_url = os.environ["INFLUX_URL"]
else:
    influx_url = "http://localhost:8086"

if "DEVICE_ID" in os.environ:
    device_id = os.environ["DEVICE_ID"]
else:
    device_id = "1"

print("Attempting to connect to: %s\nOrg: %s\nBucket: %s" %
      (influx_url, org, read_bucket))
influx_client = InfluxDBClient(
    url=influx_url,
    token=token,
    org=org)
print("Created read client for influx connection!")
query_api = influx_client.query_api()
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

def main():
    query = 'from(bucket:"{}")' \
        ' |> range(start: 2022-06-12T10:20:00.000Z, stop: 2022-06-12T23:00:00.000Z)'\
        ' |> filter(fn: (r) => r.device_id == "{}")' \
        ' |> filter(fn: (r) => r._measurement == "IoT-Device")'\
        ' |> filter(fn: (r) => r._field == "{}")'.format(read_bucket, device_id, "temp")
    
    result = influx_client.query_api().query(org=org, query=query)

    raw = []
    for table in result:
        for record in table.records:
            raw.append((record.get_value(), record.get_time()))

    print()
    print("=== influxdb query into dataframe ===")
    print()

    df = pd.DataFrame(raw, columns=['y','ds'], index=None)
    df['ds'] = df['ds'].values.astype('<M8[s]')

    print(df)

    m = Prophet()
    m.fit(df)

    df_cv = cross_validation(m, initial = '4 hours', horizon = '60 min')

    df_p = performance_metrics(df_cv)
    print(df_cv)
    print(df_p)

    fig = plot_cross_validation_metric(df_cv, metric='mse')
    fig

    tosend = pd.DataFrame()
    tosend["horizon"] = df_p["horizon"]
    tosend.reset_index(inplace=True)
    tosend["mse"] = df_p["mse"]

    np.savetxt(r'performance.txt', tosend.values, fmt='%s %s %s')

if __name__ == "__main__":
    main()
