import serial
import time
import numpy as np
from twilio.rest import Client
import pyrebase
import matplotlib.pyplot as plt

def saveFile(data):
    file = open("data.dat", "w+")
    for i in range(len(data)):
        file.write(str(data[i]) + "\n")
    file.close()

def loadFile(data):
    file = open("data.dat", "r")
    content = file.readlines()
    for i in content:
        if(i != "\n"):
            data.append(int(i))
    file.close()
    return data

def send(message):
    arduino.write(message.encode())
    arduino.flush()

def recieve():
    while arduino.in_waiting < 7:
        time.sleep(.001)
    b = arduino.readline()
    string_n = b.decode()
    decoded_bytes = string_n.rstrip()
    print(decoded_bytes)
    arduino.reset_input_buffer()
    return decoded_bytes

def sendText(message):
    # Your Account Sid and Auth Token from twilio.com/console
    # DANGER! This is insecure. See http://twil.io/secure
    account_sid = 'AC073d9e745d5a5da8e7fbe7bbfba84102'
    auth_token = 'd2031f6ed8a3aeb15b1666628c076ae9'
    client = Client(account_sid, auth_token)

    message = client.messages.create(body=message,from_='+13092048749',to='+13092359608')

    print(message.sid)

def graphPoints(datax, temperatures):
    plt.xlim(300,0)
    plt.ylim(10,50)
    plt.show
    plt.autoscale(False)
    plt.title('TMP102 Temperature over Time')
    plt.xlabel('Samples')
    plt.ylabel('Temperature (deg C)')
    plt.plot(datax, temperatures)
    plt.draw()
    plt.pause(0.0001)
    plt.clf()
#Firebase Setup
config = {
  "apiKey": "AIzaSyB2mbXG7qK3_o3q8tZNY-FbHTwrK972iyQ",
  "authDomain": "seniordesignlab1-2099a.firebaseapp.com",
  "databaseURL": "https://seniordesignlab1-2099a.firebaseio.com",
  "storageBucket": "",
  "serviceAccount": "credentials.json"
}
firebase = pyrebase.initialize_app(config)
database = firebase.database()

temperatures = []
#for i in range(0,300,1):
#    temperatures.append(-127)

datax = []
#for i in range(0,300,1):
#    datax.append(i)

count = 0
connected = False
state = 1

while True:    
    try:
        if connected == False:
            if serial.Serial('COM3', 9600, timeout=0).isOpen():
                arduino = serial.Serial('COM3', 9600, timeout=0)
                connected = True
                time.sleep(3)
    except Exception as e:
        print(e)
        connected = False
        time.sleep(.5)
    
    #send state to arduino
    send(str(state))

    #read temperature in C & add x value
    measurement = str(recieve())
    if(measurement == "00000" or measurement == "85.00"):
        measurement = np.nan
        temperatures.insert(0, float(measurement))
    else:
        temperatures.insert(0, float(measurement))
    datax.append(count)
    
    #graph data
    graphPoints(datax,temperatures)

    #Graph point
    count += 1
    if(count == 300):
        count = 0