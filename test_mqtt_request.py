# send_mqtt_message.py
import requests
import base64
import json
API_URL = 'http://localhost:18083/api/v5/publish'

def test_mqtt_get_message(url, username, password, topic, payload):

    url += "?username={username}&password={password}&topic={topic}&payload={payload}".format(
        username=username,
        password=password,
        topic=topic,
        payload=json.dumps(payload)
    )
    print(url)
    response = requests.get(url)
    print(f"Status Code: {response.status_code}")
    print(response.text)

def send_mqtt_post_message(url, username, password, topic, payload):
    data = {
        'username' : username,
        'password' : password,
        'topic': topic,
        'payload': json.dumps(payload)
    }
    headers = { 'Content-Type': 'application/json'}

    response = requests.post(url, json=data, headers=headers)
    print(f"Status Code: {response.status_code}")
    print(response.text)

if __name__ == "__main__":
    url = 'https://< ADDRESS >/domotica/api/mqtt'
    topic = 'mqttx/simulate/temp-data/response/'
    payload_dict = {"status": "off", "message": "Turn off the air conditioning"}
    USERNAME = 'f2694b80b370cb67'
    PASSWORD = '9C0ayjPLDB4wmXVDTaP9C7pFQmjnNSZFh9Cy9Cn3eOu6jvE'
    test_mqtt_get_message(url, USERNAME, PASSWORD, topic, payload_dict)
    send_mqtt_post_message(url, USERNAME, PASSWORD, topic, payload_dict)