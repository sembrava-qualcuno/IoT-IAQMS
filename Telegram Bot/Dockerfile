FROM python:3.8-slim-buster
WORKDIR /bot
COPY requirements.txt requirements.txt
RUN pip3 install -r requirements.txt
COPY ./bot.py ./bot.py
CMD  [ "python3", "-u", "bot.py" ]
