FROM python:3.8-buster
WORKDIR /data-analytics
COPY requirements.txt requirements.txt
RUN export CXX=g++
RUN export CC=gcc
RUN pip3 install -r requirements.txt
RUN pip3 install prophet==1.0.1
COPY ./data-analytics.py ./data-analytics.py
CMD  [ "python3", "-u", "data-analytics.py" ]
