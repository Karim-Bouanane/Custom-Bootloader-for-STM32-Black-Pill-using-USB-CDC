import usb.core

# USB device vendor and product IDs
vendor_id = 0x1234  # Replace with the correct vendor ID
product_id = 0x5678  # Replace with the correct product ID

# Find the USB device
device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

# Check if the device is found
if device is not None:
    print("USB device found.")

    # Set the active configuration
    device.set_configuration()

    # Find the IN endpoint
    endpoint_in = None
    for cfg in device:
        for intf in cfg:
            if (
                usb.util.endpoint_direction(intf.bEndpointAddress)
                == usb.util.ENDPOINT_IN
            ):
                endpoint_in = intf[0]

    # Check if the IN endpoint is found
    if endpoint_in is not None:
        print("IN endpoint found.")

        # Read data from the device
        try:
            while True:
                # Read data from the IN endpoint
                data = device.read(
                    endpoint_in.bEndpointAddress, endpoint_in.wMaxPacketSize
                )

                # Process and print the received data
                print("Received:", data)
        except KeyboardInterrupt:
            # Handle keyboard interrupt (Ctrl+C)
            print("USB reading interrupted.")
    else:
        print("IN endpoint not found.")
else:
    print("USB device not found.")


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
