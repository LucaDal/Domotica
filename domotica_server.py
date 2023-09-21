from flask import Flask
app = Flask(__name__)



@app.route("/domotica/api/get/fingerprint")
def get_SH1_pubkey():
    with open("/home/droidian/Projects/Scripts/fingerprint_certbot/fingerprint.txt",'r') as file:
        return file.read()
    

@app.route("/domotica/api/<token>")
def request(token):
    return token



@app.route("/domotica")
def hello():
    return "<h1 style='color:blue'>Domotica server!</h1>"

if __name__ == "__main__":
    app.run(host='192.168.1.250')
