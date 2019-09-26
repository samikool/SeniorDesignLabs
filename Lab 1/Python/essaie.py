import tkinter as tk

def allumer_instrument():
    print("L'instrument est allumé")

def changer_tempUnit():
        print("La temperature est %s 45 F")

root = tk.Tk()


canvas = tk.Canvas(root, height =600, width= 400)
canvas.pack()

frame = tk.Frame(root, bg= '#80d4ff', bd =10)

frame.place(relx=0.5, rely=0.1, relwidth=0.75, relheight=0.1, anchor = 'n')
button = tk.Button(frame, text ="Switch to celcius", bg= '#66ff66', font =30, command = changer_tempUnit)
button.place(relx=0.25, relheight=0.5, relwidth=0.5)
lower_frame= tk.Frame(root, bg = '#80c1ff', bd=15)
lower_frame.place(relx =0.5, rely=0.25, relwidth=0.75, relheight=0.6, anchor='n')

button = tk.Button(lower_frame, text ="Turn on", bg= '#66ff66', font =30, command = allumer_instrument)
button.place(relx=0.25, relheight=0.25, relwidth=0.5)

root.mainloop()

