from numpy import False_
import pandas as pd
import time
from datetime import datetime
from prophet import Prophet
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import os
import schedule

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
    influx_url = "http://influxdb:8086"

if "FORECASTING_FREQUENCY" in os.environ:
    forecasting_frequency = os.environ["FORECASTING_FREQUENCY"]
else:
    forecasting_frequency = "1min"

if "FORECASTING_PERIODS" in os.environ:
    forecasting_periods = os.environ["FORECASTING_PERIODS"]
else:
    forecasting_periods = 1

if "DEVICE_ID" in os.environ:
    device_id = os.environ["DEVICE_ID"]
else:
    device_id = "1"

if "DATA_FRESHNESS" in os.environ:
    data_freshness = os.environ["DATA_FRESHNESS"]
else:
    data_freshness = "-1d"

print("Attempting to connect to: %s\nOrg: %s\nBucket: %s" %
      (influx_url, org, read_bucket))
influx_client = InfluxDBClient(
    url=influx_url,
    token=token,
    org=org)
print("Created read client for influx connection!")
query_api = influx_client.query_api()
write_api = influx_client.write_api(write_options=SYNCHRONOUS)



def update_forecast(): 
    # Calculate temperature forecasting
    do_forecast("temp")

    # Calculate humidity forecasting
    do_forecast("hum")

    # Calculate gas forecasting
    do_forecast("gas")

def do_forecast(sensor):
    df = get_data(sensor)

    m = Prophet()
    m.fit(df)

    future = m.make_future_dataframe(periods=forecasting_periods, freq=forecasting_frequency, include_history=False)

    forecast = m.predict(future)
    
    print(forecast[['ds', 'yhat', 'yhat_lower', 'yhat_upper']])

    send_forecasted_results(forecast, sensor)



def get_data(sensor):
    print("Get data for {}".format(sensor))

    query = 'from(bucket:"{}")' \
        ' |> range(start:{}, stop: now())'\
        ' |> filter(fn: (r) => r.device_id == "{}")' \
        ' |> filter(fn: (r) => r._measurement == "IoT-Device")'\
        ' |> filter(fn: (r) => r._field == "{}")'.format(read_bucket, data_freshness, device_id, sensor)

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

    return df

def send_forecasted_results(data, sensor):
    result = pd.DataFrame()
    result['_time'] = data['ds']
    result[sensor] = data['yhat']
    result.set_index("_time", inplace=True)

    write_api.write(bucket=write_bucket, org=org, record=result,
                    data_frame_measurement_name="data-analysis")
    print("Successfully sent data to influxdb!")

schedule.every(1).minutes.do(update_forecast)

def main():
    update_forecast()

    while True:
        schedule.run_pending()
        time.sleep(1)

if __name__ == "__main__":
    main()