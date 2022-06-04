FROM python:3.8-slim-buster
WORKDIR /data-microservice
COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
COPY ./data-microservice.py ./data-microservice.py
CMD  [ "python3", "-u", "data-microservice.py" ]
