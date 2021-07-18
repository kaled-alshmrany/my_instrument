#!/usr/bin/env python3
import os.path
import os
import sys


print('In your System (sys.byteorder)=', sys.byteorder)

byte_order_in = str(input('1: little \n2: big \nelse system\n'))
byte_order = sys.byteorder
if byte_order_in == '1' : 
	byte_order = 'little'
elif byte_order_in == '2' : 
	byte_order = 'big'

print ('Your choice:', byte_order)
num_bytes = ''
while len(num_bytes) == 0 :
	num_bytes = input('Enter number of bytes:')
num_bytes = int(num_bytes)
seed_filename =''
while len(seed_filename) == 0 :
	seed_filename = str(input('Enter seed file_name:'))
lst = []
while True:
	num = str(input('Enter number or xx to end:'))
	if num == 'xx' : break
	if len(num) == 0 : continue
	lst.append(int(num))

with  open(seed_filename, 'wb') as seed_f:
	for n in lst:
		seed_f.write((n).to_bytes(num_bytes, byteorder=byte_order, signed=True))
command = 'xxd  -c ' + str(num_bytes) + ' -b ' + seed_filename
print(command)
os.system(command)
print('Done....')

