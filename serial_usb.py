import serial
import serial.tools.list_ports

# Vendor ID and Product ID of the USB device
vendor_id = 1055  # vendor ID
product_id = 22300  # product ID

#
ports = serial.tools.list_ports.comports()

#
device_port = None

for port in ports:
    if port.vid == vendor_id and port.pid == product_id:
        device_port = port.device

#
if device_port is not None:
    print("Device found")
else:
    raise Exception("Device not found")


ser = serial.Serial(device_port)
print(ser.name)

ser.close()

# Define your message
message = b"\x20\x02\x00\x12\x00Hello Worldoo"


"""
with open("file.elf", "rb") as file:
    elf_data = file.read()

# Split the ELF data into 1KB chunks
chunk_size = 1024
chunks = [elf_data[i : i + chunk_size] for i in range(0, len(elf_data), chunk_size)]

# Send the chunks to the USB device
for chunk in chunks:
    device.write(1, chunk, 100)  # Endpoint 1, data chunk, timeout of 100ms

"""

"""
import crcmod

# Create function for the CRC-32
crc_func = crcmod.mkCrcFun(0x104C11DB7, initCrc=0xFFFFFFFF, xorOut=0x00, rev=False)

# Input data in hexadecimal format
data_hex = [0x11223344, 0xAABBCCDD, 0x66775533]

# Convert hex to bytes so as to be processed
data_bytes = b"".join(bytes(x.to_bytes(4, "big")) for x in data_hex)

# Verify the conversion
#hex_array = [hex(byte) for byte in data_bytes]
#print(hex_array)

# Calculate CRC value
crc_value = crc_func(data_bytes)
print("CRC value: 0x{:02X}".format(crc_value))

"""
