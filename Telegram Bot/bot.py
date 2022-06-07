from telnetlib import NOP
from telegram.ext import CallbackContext, Application
import os
import influxdb_client
from influxdb_client.client.write_api import SYNCHRONOUS
import statistics
from telegram import Update
from telegram.ext import CallbackContext, CommandHandler

# influx parameters
if "INFLUX_READ_BUCKET" in os.environ:
    read_bucket = os.environ["INFLUX_READ_BUCKET"]
else:
    read_bucket = "sensor-data"

if "INFLUX_ORG" in os.environ:
    org = os.environ["INFLUX_ORG"]
else:
    org = "sembrava_qualcuno"

if "INFLUX_TOKEN" in os.environ:
    influx_token = os.environ["INFLUX_TOKEN"]
else:
    influx_token = "4Ec-_u1v87yRHcp9mW6bxZndumKBTerQbY7zzyBWhReKtOuTBnhC0Ln_pHKLIk-zlLrZj2INXtvWYPq7JCj5_w=="

if "INFLUX_URL" in os.environ:
    influx_url = os.environ["INFLUX_URL"]
else:
    influx_url = "http://influxdb:8086"

# telegram parameters
if "BOT_TOKEN" in os.environ:
    bot_token = os.environ["BOT_TOKEN"]
else:
    bot_token = "5590317237:AAEo-xY2OvfplF3giEEJg9VJRVJHNQr-CKY"

if "CHAT_ID" in os.environ:
    chat_id = os.environ["CHAT_ID"]
else:
    chat_id = "-1001795240933"

if "BOT_INTERVAL" in os.environ:
    bot_interval = os.environ["BOT_INTERVAL"]
else:
    bot_interval = 60

if "DATA_FRESHNESS" in os.environ:
    data_freshness = os.environ["DATA_FRESHNESS"]
else:
    data_freshness = "-1d"

print("Attempting to connect to: %s\nOrg: %s\nBucket: %s" %
      (influx_url, org, read_bucket))
influx_client = influxdb_client.InfluxDBClient(
    url=influx_url,
    token=influx_token,
    org=org)
print("Created read client for influx connection!")
query_api = influx_client.query_api()
write_api = influx_client.write_api(write_options=SYNCHRONOUS)

def get_average(sensor):
    query = 'from(bucket: "{}")\
                |> range(start: {}, stop: now())\
                |> filter(fn: (r) => r["_field"] == "{}")'.format(read_bucket, data_freshness, sensor)
    result = query_api.query(org=org, query=query)
    values = []
    for record in result[0].records:
        values.append(record.get_value())

    return statistics.fmean(values)

def get_averages_message():
    print("Get average values")

    temp_average = get_average("temp")
    hum_average = get_average("hum")
    gas_average = get_average("gas")

    return "Average values:\nTemperature: {:.1f}\nHumidity: {:.1f}\nGas: {:.1f}".format(temp_average, hum_average, gas_average)

async def callback_minute(context: CallbackContext):
    await context.bot.send_message(chat_id=chat_id, text=get_averages_message())

async def get_average_command(update: Update, context: CallbackContext.DEFAULT_TYPE):
    await context.bot.send_message(chat_id=update.effective_chat.id, text=get_averages_message())

def main():
    application = Application.builder().token(bot_token).build()
    job_queue = application.job_queue

    job_minute = job_queue.run_repeating(callback_minute, interval=bot_interval, first=10)

    average_handler = CommandHandler('averages', get_average_command)
    application.add_handler(average_handler)

    application.run_polling()

if __name__ == "__main__":
    main()
