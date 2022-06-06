FROM python:3.8-slim-buster
WORKDIR /data-analytics
COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
COPY ./data-analytics.py ./data-analytics.py
CMD  [ "python3", "-u", "data-analytics.py" ]
