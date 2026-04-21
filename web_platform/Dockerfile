FROM python:3.12-slim

WORKDIR /app

RUN pip install numpy regex requests

COPY . .

WORKDIR /app/web_platform/backend

EXPOSE 8000

CMD ["python", "server.py"]
