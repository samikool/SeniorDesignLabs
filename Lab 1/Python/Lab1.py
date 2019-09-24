import serial
import time

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
    while arduino.in_waiting == 0:
                time.sleep(.001)
    b = arduino.readline()
    string_n = b.decode()
    decoded_bytes = string_n.rstrip()
    print(decoded_bytes)
    return decoded_bytes

#arduino = serial.Serial('COM3', 9600, timeout=0)

while True:
    
    #graph null points
    #possibly break if able to open port
    if not arduino.is_open:

        arduino = serial.Serial('COM3', 9600, timeout=0)
        
    #graph data from sensor
    else:
        string = 'hello\n'
        string = string.encode('utf-8')
        arduino.write(string)
        arduino.flush()
        while arduino.out_waiting != 0:
            print("Sending data")
        
        try:
            while arduino.in_waiting == 0:
                time.sleep(.001)
            b = arduino.readline()
            string_n = b.decode()
            decoded_bytes = string_n.rstrip()
            print(decoded_bytes)

            time.sleep(.1)
        except Exception as e:
            print(e)

        
    