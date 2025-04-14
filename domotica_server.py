from flask import Flask, request, current_app
import requests
import json
import base64
import logging

app = Flask(__name__)

API_URL = 'http://localhost:18083/api/v5/publish'

@app.route("/domotica")
def hello():
    return "<h1 style='color:blue'>Domotica server!</h1>"

@app.route("/domotica/api/mqtt", methods=['GET','POST'])
def mqtt():
    #app.logger.warning("message recived!!")
    if request.method == 'GET':
        username = request.args.get('username')
        password= request.args.get('password')
        payload = request.args.get('payload')
        topic = request.args.get('topic')

    elif request.method == 'POST':
        content_type = request.headers.get('Content-Type')
        if (content_type == 'application/json'):
            json_payload = request.json
            username = json_payload.get('username')
            password = json_payload.get('password')
            payload = json_payload.get('payload')
            topic = json_payload.get('topic')
        else:
            return 'Content-Type not supported!'
    if(username is None or password is None or payload is None or topic is None):
        return 'Missing parameters', 400

    auth_header = "Basic " + base64.b64encode((username + ":" + password).encode()).decode()
    headers = {'Authorization': auth_header,
               'Content-Type': 'application/json'}
    data = {
        'topic': topic,
        'payload': payload
    }
    response = requests.post(API_URL, json=data, headers=headers)
    return response.text, response.status_code

if __name__ == "__main__":
    gunicorn_logger = logging.getLogger('gunicorn.error')
    app.logger.handlers = gunicorn_logger.handlers
    app.logger.setLevel(gunicorn_logger.level)
    app.run(host='192.168.1.250', debug=False)
