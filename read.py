import time
import smbus

i2c_channel = 1
i2c_address = 0x08

bus = smbus.SMBus(i2c_channel)

while True:
	val = bus.read_i2c_block_data(i2c_address, 0x00)
	print("val", val)
	time.sleep(1)