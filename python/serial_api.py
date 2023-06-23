
import struct
import crcmod
import serial


# Command/Response Size
CMD_SIZE                    = 7
RESP_SIZE                   = 3

# Command Response Status
CMD_RESP_STATUS_OK          = 0
CMD_RESP_STATUS_ERROR       = 1
CMD_RESP_STATUS_INVALID     = 2

# Packet Response Status 
PACKET_RESP_ACK             = 0
PACKET_RESP_NACK            = 1
PACKET_RESP_INVALID         = 2

# Commands
CMD_ID_ACK				    = 0x10
CMD_ID_PACKET			    = 0x20
CMD_ID_PACKET_ACK		    = 0x30
CMD_ID_PACKET_NACK		    = 0x40
CMD_ID_ERROR			    = 0x50
CMD_ID_EXECUTE			    = 0x60
CMD_ID_ERASE_APP		    = 0x70
CMD_ID_DOWNLOAD_FW		    = 0x80

CMD_NAME_LIST = {

    CMD_ID_ACK          : 'CMD_ACK',
    CMD_ID_PACKET       : 'PACKET',
    CMD_ID_PACKET_ACK   : 'PACKET_ACK',
    CMD_ID_PACKET_NACK  : 'PACKET_NACK',
    CMD_ID_ERROR        : 'ERROR',
    CMD_ID_EXECUTE      : 'EXECUTE',
    CMD_ID_ERASE_APP    : 'ERASE_APP',
    CMD_ID_DOWNLOAD_FW  : 'DOWNLOAD_FW'
}

# Errors
BL_CHKS_MISMATCH			= 0x7F 		# Application checksum incorrect
BL_CMD_INVALID              = 0x80		# Invalid command
BL_INVALID_STATE            = 0x81		# Invalid state
BL_RECEIVE_TIMEOUT          = 0x82		# Receive timeout reached
BL_DOWNLOAD_FAILED          = 0x83		# Firmware download failed
BL_NO_USER_APP              = 0x84		# No user application found

ERROR_NAME_LIST = {
    
    BL_CHKS_MISMATCH    : "CHECKSUM MISMATH",
    BL_CMD_INVALID      : "INVALID COMMAND",
    BL_INVALID_STATE    : "INVALID BOOTLOADER STATE", 
    BL_RECEIVE_TIMEOUT  : "RECEIVE TIMEOUT",
    BL_DOWNLOAD_FAILED  : "DOWNLOAD FAILED",
    BL_NO_USER_APP      : "USER APPLICATION NOT FOUND"
}


"""
Function: bytes_to_hex
Description: Converts bytes data to a hex string representation.
@param bytes_data: The bytes data to be converted.
@return: The hex string representation of the bytes data.
"""
def bytes_to_hex(bytes_data):
    return ' '.join(['0x{:02X}'.format(byte) for byte in bytes_data])


"""
Function: displayBinaryFile
Description: Displays binary file data in a formatted manner.
@param data: The binary file data.
@return: None
"""
def displayBinaryFile(data):
    #
    words = [data[i:i+4][::-1] for i in range(0, len(data), 4)]
    # Display words in four columns
    for i in range(0, len(words), 4):
        row = words[i:i+4]
        formatted_row = [word.hex() for word in row]
        print("\t".join(formatted_row))


"""
Function: displayPacket
Description: Displays a packet in a formatted manner.
@param packet: The packet data to be displayed.
@return: None
"""
def displayPacket(packet):
    for i in range(0, len(packet), 4):
        if i != 0 and i % 16 == 0:
            print()
        print((packet[i:i+4])[::-1].hex(), end="\t")
    print()


"""
Function: Connect
Description: Establishes a serial connection to the specified COM port.
@param com_port: The COM port to connect to.
@return: The serial port object.
"""
def Connect(com_port):
    # Serial port settings
    baudrate = 115200
    bytesize = serial.EIGHTBITS
    stopbits = serial.STOPBITS_ONE
    parity = serial.PARITY_NONE
    read_timeout = 10                    # value in seconds

    # Create a serial port object with the specified settings
    ser = serial.Serial(com_port, baudrate=baudrate, timeout=read_timeout, 
                        bytesize=bytesize, parity=parity, stopbits=stopbits)

    return ser


"""
Function: SendCMD
Description: Sends a command packet over the serial port and waits for acknowledgment.
@param serial_port: The serial port object.
@param cmd: The command to send.
@param data: Optional data to include in the command packet.
@return: 
"""
def SendCMD(serial_port, cmd, LOG):

    if type(cmd) == int:
        cmd_packet = bytes([cmd] + [0]*6)
    else:
        cmd_packet = cmd + bytes(7 - len(cmd))

    cmd_id = cmd_packet[0]
    LOG("Send " + CMD_NAME_LIST[cmd_id] + " Command")
    
    print("Sending Command: ", bytes_to_hex(cmd_packet))

    try:
        #
        serial_port.reset_input_buffer()
        serial_port.reset_output_buffer()
        #
        serial_port.write(cmd_packet)
        return ReceiveCmdResp(serial_port, cmd_id, LOG)
        
    except serial.SerialException as e:
        LOG("Serial Exception while sending CMD: " + str(e))

    return CMD_RESP_STATUS_INVALID


"""
Function: ReceiveCmdResp
Description: Receives and verifies the acknowledgment response for a command.
@param serial_port: The serial port object.
@param cmd: The command for which to receive the response.
@return: 
"""
def ReceiveCmdResp(serial_port, cmd, LOG):
    #
    response = serial_port.read(RESP_SIZE) 
    print("Response: ", bytes_to_hex(response))

    if len(response) == RESP_SIZE:

        if response[0] == CMD_ID_ACK and response[1] == cmd:
            LOG("Command acknowledgment received.")
            return CMD_RESP_STATUS_OK
        
        elif response[0] == CMD_ID_ERROR:
            error_id = response[1]
            LOG("Received Error: " + ERROR_NAME_LIST[error_id])
            return CMD_RESP_STATUS_ERROR
    
    LOG("Invalid Response Packet")
    return CMD_RESP_STATUS_INVALID


"""
Function: SendPacket
Description: Sends a packet over the serial port and waits for acknowledgment.
@param serial_port: The serial port object.
@param payload: The packet payload to send.
@param number: The packet number for acknowledgment.
@return: True if the packet is sent and acknowledged successfully, False otherwise.
"""
def SendPacket(serial_port, payload, packet_num, LOG):
    
    #displayPacket(packet_payload)
    try_nb = 3

    # Attempt to send the packet and receive acknowledgment
    while try_nb > 0 :

        LOG("> Send Packet n°:" + str(packet_num))
        serial_port.write(payload)
        status = ReceivePacketResp(serial_port, packet_num, LOG)

        if status == PACKET_RESP_NACK:
            try_nb -= 1
        else:
            break
    
    # Check if the maximum number of attempts is reached
    if status != PACKET_RESP_ACK:
        return False
    
    return True


"""
Function: ReceivePacketAck
Description: Receives acknowledgment for a packet over the serial port.
@param serial_port: The serial port object.
@param number: The packet number for acknowledgment.
@return: True if the acknowledgment is received successfully, False otherwise.
"""
def ReceivePacketResp(serial_port, packet_num, LOG):

    response = serial_port.read(RESP_SIZE)
    print("Packet n°:", packet_num, ". Resp length:", len(response) , ". Resp: ", bytes_to_hex(response))


    if len(response) == RESP_SIZE:
        resp_cmd_id = response[0]
        resp_packet_number = response[1] + ((response[2] << 8) & 0xFF00)
    
        if (resp_cmd_id == CMD_ID_PACKET_ACK) and (resp_packet_number == packet_num) :
            LOG("< Packet n°:" + str(packet_num) + " acknowledged")  
            return PACKET_RESP_ACK

        elif (resp_cmd_id == CMD_ID_PACKET_NACK) and (resp_packet_number == packet_num) :
            LOG("< Packet n°:" + str(packet_num) + " not acknowledged")  
            return PACKET_RESP_NACK

    LOG("< Invalid Response for Packet n°:" + str(packet_num))    
    return PACKET_RESP_INVALID


"""
Function: SendBinaryFile
Description: Sends a binary file over the serial port in packets.
@param serial_port: The serial port object used for communication.
@param path_to_file: The path to the binary file to be sent.
@param LOG: The logging function to display messages.
@return: None
"""
def SendBinaryFile(serial_port, path_to_file, LOG):

    packet_size = 64

    try:
        file_data = []

        # Read the file
        with open(path_to_file, "rb") as file:
            file_data = file.read()

            file_size = len(file_data)

            # Add padding to make it multiple of 64
            padding_size = 64 - (file_size % 64)
            padding_data = bytes([0] * padding_size)
            file_data += padding_data

            # Count number of packets of size 64 bytes
            total_packets = len(file_data) // packet_size
            total_packets_inBytes = struct.pack('<H', total_packets)

            # Calculate the CRC32 value of the file data
            crc32_value = calculateCRC32(file_data)
            crc32_value_inBytes = struct.pack('<I', crc32_value)

            # Prepare the command data to send
            cmd_packet = bytes([CMD_ID_DOWNLOAD_FW]) + total_packets_inBytes + crc32_value_inBytes
            
            LOG("")
            LOG("--------------- Info ---------------")
            LOG("Orginal file size \t\t\t: " + str(file_size))
            LOG("Max packet size \t\t\t: " + str(packet_size))
            LOG("Total packets to send: " + str(total_packets))
            LOG("CRC value \t\t\t: 0x{:02X}".format(crc32_value))
            LOG("-------------------------------------\n")

            # Send the command to start downloading firmware
            if SendCMD(serial_port, cmd_packet, LOG) != CMD_RESP_STATUS_OK:
                return

            LOG("Start Downloading ....")

            for packet_num in range(0, total_packets):
                # Extract the next payload
                packet_payload = file_data[packet_num * packet_size : (packet_num+1) * packet_size]

                # Send the packet payload
                if SendPacket(serial_port, packet_payload, packet_num, LOG) == False:
                    LOG("Download FW Aborted.")
                    return
            
            LOG("Firmware Successfully Flashed.")

        
    except IOError as e:
        LOG("Error while sending binary file: " + str(e))


"""
Function: calculateCRC32
Description: Calculates the CRC32 checksum of the given data.
@param data: The data for which CRC32 checksum is to be calculated.
@return: The CRC32 checksum value.
"""
def calculateCRC32(data):

    # Organize the data into 4-byte chunks in big-endian order
    data = b"".join([data[i:i+4][::-1] for i in range(0, len(data), 4)])

    # Create a CRC32 function object with the specified parameters
    crc32_func = crcmod.mkCrcFun(0x104C11DB7, initCrc=0xFFFFFFFF, xorOut=0x00, rev=False)
    
    # Calculate the CRC32 checksum of the data
    crc32_value = crc32_func(data)

    return crc32_value
