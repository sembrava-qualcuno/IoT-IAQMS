FROM python:3.8-slim-buster
LABEL org.opencontainers.image.source="https://github.com/sembrava-qualcuno/IoT-IAQMS"
WORKDIR /performance
COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
COPY *.py .
ENTRYPOINT [ "python3", "-u", "performance-evaluation.py" ]
