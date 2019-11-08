import tkinter as tk
import sys 
import time

on = False

def screen_button():
   global on
   print(on)
   if(on == True):
      screen_button_text.set("Turn On")
      on = False
   else:
      screen_button_text.set("Turn Off")
      on = True
 
    
root = tk.Tk()

root.title("Arduino Controller")

canvas = tk.Canvas(root, height = 400, width = 400)
canvas.pack()

frame = tk.Frame(root, bg= '#80d4ff', bd =5)

frame.place(relx=0.5, rely=0.1, relwidth=0.75, relheight=0.2, anchor = 'n')

screen_button_text = tk.StringVar()
screen_button_text.set("Turn on")
screen_button = tk.Button(frame, textvariable=screen_button_text, font=20, bg = 'Blue', command=screen_button)
screen_button.place(relx=.25, relheight=1, relwidth=.5)

root.after(1000, 'Lab1.py')
root.mainloop()