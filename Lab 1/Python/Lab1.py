import serial
import time
from twilio.rest import Client
import matplotlib.pyplot as plt
import matplotlib.animation as animation

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
    arduino.write(message)
    arduino.flush()
    while arduino.out_waiting != 0:
        print("Sending" + message)

def recieve():
    while arduino.in_waiting < 7:
            time.sleep(.001)
    b = arduino.readline()
    string_n = b.decode()
    decoded_bytes = string_n.rstrip()
    print(decoded_bytes)
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
    plt.ylim(-20,40)
    plt.show
    plt.autoscale(False)
    plt.title('TMP102 Temperature over Time')
    plt.xlabel('Samples')
    plt.ylabel('Temperature (deg C)')
    plt.plot(datax, temperatures)
    plt.draw()
    plt.pause(0.0001)
    plt.clf()

#Setting up graph
plt.ion
plt.xlim(300,0)
plt.ylim(-20,40)
plt.show
plt.autoscale(False)
plt.title('TMP102 Temperature over Time')
plt.xlabel('Samples')
plt.ylabel('Temperature (deg C)')


try:
    if serial.Serial('COM3', 9600, timeout=0).isOpen():
        arduino = serial.Serial('COM3', 9600, timeout=0)
except Exception as e:
    print(e)

temperatures = []
#for i in range(0,300,1):
#    temperatures.append(-127)

datax = []
#for i in range(0,300,1):
#    datax.append(i)

count = 0
connected = False

while True:
    #graph null points
    #possibly break if able to open port
    
    try:
        if connected == False and serial.Serial('COM3', 9600, timeout=0).isOpen():
            arduino = serial.Serial('COM3', 9600, timeout=0)
            connected = True
    except Exception as e:
        print(e)
        time.sleep(.5)
    
    #graph data from sensor
    else:
        temperatures.insert(0,float(recieve()))
        datax.append(count)

        graphPoints(datax,temperatures)

        count = count + 1
        if(count == 300):
            count = 0
            
        try:
            time.sleep(.1)
        except Exception as e:
            print(e)

        
    