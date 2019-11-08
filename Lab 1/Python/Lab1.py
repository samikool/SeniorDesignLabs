import serial
import time
import numpy as np
from twilio.rest import Client
import pyrebase
import matplotlib.pyplot as plt
import tkinter as tk
import sys 
import time

arduino = 0
minTemp = -100
maxTemp = 200
textSent = False
account_sid = ''
auth_token = ''
fromPhone = ''
toPhone = ''
connected = False

##################################################
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
##################################################
def send(message):
    try:
        global arduino
        global connected
        arduino.write(message.encode())
        arduino.flush()
        print(message + " sent")
    except Exception as e:
        print(e)
        connected = False
        

def recieve():
    global arduino
    global connected
    try:
        while arduino.in_waiting < 7:
            time.sleep(.001)
        b = arduino.readline()
        string_n = b.decode()
        decoded_bytes = string_n.rstrip()
        print(decoded_bytes)
        arduino.reset_input_buffer()
        return decoded_bytes
    except Exception as e:
        print(e)
        connected = False
        return np.nan
###################################################
def sendText(message):
    # Your Account Sid and Auth Token from twilio.com/console
    # DANGER! This is insecure. See http://twil.io/secure
    global account_sid
    global auth_token
    global fromPhone
    global toPhone
    client = Client(account_sid, auth_token)

    message = client.messages.create(body=message,from_=fromPhone,to=toPhone)

    print(message.sid)
###################################################
def graphPoints(datax, temperatures):
    plt.xlim(300,0)
    plt.ylim(10,50)
    plt.show
    plt.autoscale(False)
    plt.title('Temperature over Time')
    plt.xlabel('Seconds Passed Since Measurement')
    plt.ylabel('Temperature (*C)')
    plt.plot(datax, temperatures)
    plt.draw()
    plt.pause(0.0001)
    plt.clf()
##################################################################
# Firebase Setup
config = {
  "apiKey": "AIzaSyB2mbXG7qK3_o3q8tZNY-FbHTwrK972iyQ",
  "authDomain": "seniordesignlab1-2099a.firebaseapp.com",
  "databaseURL": "https://seniordesignlab1-2099a.firebaseio.com",
  "storageBucket": "",
  "serviceAccount": "credentials.json"
}
firebase = pyrebase.initialize_app(config)
database = firebase.database()
displayC = True
##################################################################
def mainLoop():
    temperatures = []
    #for i in range(0,300,1):
    #    temperatures.append(-127)

    datax = []
    #for i in range(0,300,1):
    #    datax.append(i)

    count = 0
    global connected
    state = 0
    global arduino
    global temperature_display_text
    global textSent
    global minTemp
    global maxTemp
    
    while True:
        try:
            if connected == False:
                if serial.Serial('COM3', 9600, timeout=0).isOpen():
                    arduino = serial.Serial('COM3', 9600, timeout=0)
                    connected = True
                    time.sleep(5)
                    send("1")
        except Exception as e:
            print(e)
            connected = False
            time.sleep(.85)
        
        #send state to arduino
        #send(str(state))

        #read temperature in C & add x value
        if(connected):
            measurement = str(recieve())
            if(measurement == "00000" or measurement == "85.00"):
                measurement = np.nan
                if(len(datax) == 300):
                    temperatures.pop(299)
                    temperatures.insert(0, float(measurement))
                else:
                    temperatures.insert(0, float(measurement))
                    datax.append(count)
                temperature_display_text.set("Probe Disconnected")
            elif (measurement == "00001"):
                measurement = np.nan
                if(len(datax) == 300):
                    temperatures.pop(299)
                    temperatures.insert(0, float(measurement))
                else:
                    temperatures.insert(0, float(measurement))
                    datax.append(count)
                temperature_display_text.set("No Data")
            else:
                if(len(datax) == 300):
                    temperatures.pop(299)
                    temperatures.insert(0, float(measurement))
                else:
                    temperatures.insert(0, float(measurement))
                    datax.append(count)
                if(displayC):
                    temperature_display_text.set("Temperature: " + measurement + " *C")
                else:
                    measurement = str(round(float(measurement) * 9/5 + 32,2))
                    temperature_display_text.set("Temperature: " + measurement + " *F")

                if(not textSent):
                    if(float(measurement) < minTemp):
                        sendText("TEMPERATURE IS TOO COLD!")
                        textSent = True
                    elif(float(measurement) > maxTemp):
                        sendText("TEMPERATURE IS TOO HOT!")
                        textSent = True
            #graph data
            graphPoints(datax,temperatures)
            if(count < 300):
                count += 1
        else:
            measurement = np.nan
            if(len(datax) == 300):
                temperatures.pop(299)
                temperatures.insert(0, float(measurement))
            else:
                temperatures.insert(0, float(measurement))
                datax.append(count)
            graphPoints(datax,temperatures)
            if(count < 300):
                count += 1
                

on = False

def screen_button():
    global on
    print(on)
    if(on == True):
        screen_button_text.set("Turn On LCD")
        on = False
        send("0")
    else:
        screen_button_text.set("Turn Off LCD")
        on = True
        send("2")
 
def scale_button():
    global displayC
    print(displayC)
    if(displayC):
        displayC = False
        switch_temperature_scale_text.set("F->C")
    else:
        displayC = True
        switch_temperature_scale_text.set("C->F")

def set_min_max():
    global minTemp 
    global maxTemp 

    if(min_entry.get() != ""):
        try:
            minTemp = float(min_entry.get())
        except Exception as e:
            minTemp = -100
    if(max_entry.get() != ""):
        try:
            maxTemp = float(max_entry.get())
        except Exception as e:
            maxTemp = 100


    print("Set min to " + str(minTemp))
    print("Set high to " + str(maxTemp))

def set_number():
    global account_sid
    global auth_token
    global fromPhone
    global toPhone
    global textSent

    if(change_phone_entry.get() == ('3198999264')):
        account_sid = 'ACa77a0e8b75884a4ff3ab693eb54f1c60'
        auth_token = '60ecf338694800b9f79043e930d57ec5'
        fromPhone = '+1' + '9073181770'
        toPhone = '+1' + '3198999264'
        print("Phone set to +1 319-899-9264")
    elif(change_phone_entry.get() == ('3196711709')):
        account_sid = 'ACc3703a7b5aa799609b2152c354587241'
        auth_token = 'caabf8a7c686a7d9fc258ca307fbd1e0'
        fromPhone = '+1' + '9073181809'
        toPhone = '+1' + '3196711709'
        print("Phone set to +1 319-671-1709")

    textSent = False


root = tk.Tk()

root.title("Arduino Controller")

canvas = tk.Canvas(root, height = 600, width = 400)
canvas.pack()

frameButton = tk.Frame(root, bg='#80d4ff', bd=5)
frameButton.place(relx=0.5, rely=0.1, relwidth=0.75, relheight=0.2, anchor = 'n')

screen_button_text = tk.StringVar()
screen_button_text.set("Turn on LCD")
screen_button = tk.Button(frameButton, textvariable=screen_button_text, font=20, bg = 'Blue', command=screen_button)
screen_button.place(relx=0, relheight=1, relwidth=1)

frameDisplay = tk.Frame(root, bg='#80d4ff', bd=5)
frameDisplay.place(relx=0.5, rely=0.3, relwidth=0.75, relheight=0.2, anchor='n')

switch_temperature_scale_text = tk.StringVar()
switch_temperature_scale_text.set("C->F")
switch_temperature_scale = tk.Button(frameDisplay, textvariable=switch_temperature_scale_text, font=20, bg='Green', command=scale_button)
switch_temperature_scale.place(relx=.8, relheight=1,relwidth=.20)

temperature_display_text = tk.StringVar()
temperature_display_text.set("No Data")
temperature_display = tk.Label(frameDisplay, textvariable=temperature_display_text, font=20, bg='Red')
temperature_display.place(relx=0, relheight=1, relwidth=.80)

min_max_frame = tk.Frame(root, bg='#80d4ff', bd=5)
min_max_frame.place(relx=0.5, rely=0.5, relwidth=0.75, relheight=0.075, anchor='n')

min_label = tk.Label(min_max_frame, text="Min:", font=20, bg='LightBlue')
min_label.place(relx=0, relheight=1, relwidth=.20)

min_entry = tk.Entry(min_max_frame, font=20)
min_entry.place(relx=.20, rely=0, relheight=1, relwidth=.20)

max_label = tk.Label(min_max_frame, text="Max:", font=20, bg='Pink')
max_label.place(relx=.40, relheight=1, relwidth=.20)

max_entry = tk.Entry(min_max_frame, font=20)
max_entry.place(relx=.60, rely=0, relheight=1, relwidth=.20)

min_max_set_button = tk.Button(min_max_frame, text="set", font=20, bg="LightGreen", command=set_min_max)
min_max_set_button.place(relx=.80, rely=0, relheight=1, relwidth=.20)

change_phone_frame_label = tk.Frame(root, bg='#80d4ff', bd=5)
change_phone_frame_label.place(relx=0.5, rely=.575, relwidth=0.75, relheight=0.075, anchor='n')

change_phone_label = tk.Label(change_phone_frame_label, text="Enter Phone Number:", font=20, bg='purple')
change_phone_label.place(relx=0, rely=0, relheight=1, relwidth=1)

change_phone_frame = tk.Frame(root, bg='#80d4ff', bd=5)
change_phone_frame.place(relx=0.5, rely=.650, relwidth=0.75, relheight=0.075, anchor='n')

change_phone_area_label = tk.Label(change_phone_frame, text="+1", font=20, bg = "LightGreen")
change_phone_area_label.place(relx=0.0, rely=0, relwidth=.15, relheight=1)

change_phone_entry = tk.Entry(change_phone_frame, font=20)
change_phone_entry.place(relx=.15, rely=0, relwidth=.65, relheight=1)

change_phone_button = tk.Button(change_phone_frame, text="set", font=20, bg="LightGreen", command=set_number)
change_phone_button.place(relx=.75, rely=0, relwidth=.25, relheight=1)



root.after(1000, mainLoop())
root.mainloop()