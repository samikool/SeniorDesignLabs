import serial
import time

arduino = serial.Serial('COM3', 9600, timeout=0)

print("Serial initialized...")
#string = 'hello\n'
#string = string.encode('utf-8')
#test = 1
#arduino.write(test)
#test = arduino.read()
#arduino.write(string)
#print(test)

while True:
    #print(arduino.is_open)
    #print(arduino.readline())
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
        break
        
    