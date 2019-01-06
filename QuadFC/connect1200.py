import serial
ser = serial.Serial('/dev/ttyACM0', 1200, timeout=1)
x = ser.read()          # read one byte
ser.close()
