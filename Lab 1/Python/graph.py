import matplotlib.pyplot as plt
import matplotlib.animation as animation

# Parameters
x_len = 300         # Number of points to display
y_range = [10, 40]  # Range of possible Y values to display

# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)

plt.xlim(300,0)
#xs = list(range(300))
xs = [0] * 300
ys = [0] * x_len
ax.set_ylim(y_range)

# Initialize communication with TMP102
#tmp102.init()

# Create a blank line. We will update the line in animate
#line, = ax.plot(xs, ys)

# Add labels
plt.title('TMP102 Temperature over Time')
plt.xlabel('Samples')
plt.ylabel('Temperature (deg C)')

datax = [300,299,298,297,200]
datay = [10,11,12,13,14]

plt.plot(datax, datay)
plt.show

# This function is called periodically from FuncAnimation
#def animate(i, ys):
    
    # Read temperature (Celsius) from TMP102
    #temp_c = round(123.456, 2)
   # temp_c = 34.6

    # Add y to list
   # ys.append(temp_c)


    # Limit y list to set number of items
#ys = ys[-x_len:]
    #ys = ys[datax:]

    # Update line with new Y values
   # line.set_ydata(ys)
    #line.set_ydata(datay)

   # return line,

# Set up plot to call animate() function periodically
#ani = animation.FuncAnimation(fig,
 ##  fargs=(ys,),
   # interval=50,
    #blit=True)

plt.show()
