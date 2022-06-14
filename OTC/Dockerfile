FROM python:3.8-slim-buster
LABEL org.opencontainers.image.source="https://github.com/sembrava-qualcuno/IoT-IAQMS"
WORKDIR /otc
COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
COPY ./outdoor-temperature-component.py ./outdoor-temperature-component.py
CMD  [ "python3", "-u", "outdoor-temperature-component.py" ]
