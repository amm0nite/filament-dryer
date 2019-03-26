import time
import smbus
import struct

i2c_channel = 1
i2c_address = 0x08

bus = smbus.SMBus(i2c_channel)
print dir(bus)

while True:
	data = bus.read_i2c_block_data(i2c_address, 0, 6)
	data = bytearray(data)
	parsed = struct.unpack('fh', data)
	print("temp", parsed[0])
	print("state", parsed[1])
	time.sleep(1)