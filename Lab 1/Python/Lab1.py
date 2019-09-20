import serial
import time

arduino = serial.Serial('COM3', 9600, timeout=0)

print("Serial initialized...")

#test = 1
#arduino.write(test)
#test = arduino.read()

#print(test)

while True:
    # print(arduino.is_open)
 #   print(arduino.readline())
    try:
        b = arduino.readline()
        string_n = b.decode()
        decoded_bytes = string_n.rstrip()
        print(decoded_bytes)
        time.sleep(0.1)
    except Exception as e:
        print(e)
        break
        
    