
#
import os
import datetime
import subprocess
#
from serial.tools import list_ports
#
import tkinter as tk
from tkinter import filedialog
#
from serial_api import *


''' Global variables '''

actual_connected_port = ""
serial_port = None
file_path = ''


''' Functions '''

"""
Function: LOG
Description: Inserts a log message with a timestamp into the log_text Text widget.
@param message: The log message to be displayed.
@return: None
"""
def LOG(message):

    # Get the current time and format it as a string
    current_time = '(' + datetime.datetime.now().strftime("%H:%M:%S") + ')\t'

    # Insert the log message with the timestamp into the log_text Text widget
    log_text.insert(tk.END, current_time + message + '\n')


"""
Function: scan_ports
Description: Scans for available COM ports and add them to the COM port menu.
@return: None
"""
def scan_ports():
    # Clear the existing options in the COM port menu
    com_port_menu["menu"].delete(0, "end")

    # Scan for available COM ports
    ports = list(list_ports.comports())

    # Add the scanned ports to the COM port menu 
    for port in ports:
        com_port_menu["menu"].add_command(label=port.device, command=tk._setit(var_com_port, port.device))


"""
Function: connect
Description: Connects to the selected COM port.
@return: None
"""
def connect():
    
    global actual_connected_port 
    global serial_port

    # Get the selected port from the COM port menu
    selected_port = var_com_port.get() 

    # Check if already connected to the selected port
    if actual_connected_port != "" and actual_connected_port == selected_port:
        LOG("Already connected to the PORT: " + selected_port)

    # Check if no port is selected
    elif selected_port == "":
        LOG("Please scan ports, then select your desired port.")
    
    else:
        try:
            # Connect to the selected port
            serial_port = Connect(selected_port)
            actual_connected_port = selected_port
            LOG("Serial Port " + selected_port + " successfully opened.")

        except serial.SerialException as e:
            serial_port = None
            LOG("Error opening serial port: " + str(e))


"""
Function: disconnect
Description: Disconnects from the currently connected COM port.
@return: None
"""
def disconnect():

    global actual_connected_port 
    global serial_port

    # Get the selected port from the COM port menu
    selected_port = var_com_port.get()

    # Check if no port is connected
    if actual_connected_port == '' or serial_port == None:
        LOG("No port is connected")

    else:
        try:
            # Close the serial port connection
            serial_port.close()
            LOG("Disconnected successfully from PORT: " +  selected_port)

        except Exception as e:
            LOG("Error occurred during serial port disconnection: " +  str(e))

    # Reset the COM port menu and variables
    var_com_port.set('')
    actual_connected_port = ''
    serial_port = None


"""
Function: import_file
Description: Allows the user to import a file and display its path in the file entry widget.
@return: None
"""
def import_file():

    global file_path

    # Functionality to import file
    file_path = filedialog.askopenfilename(filetypes=[("ELF Files", "*.elf"), ("Binary Files", "*.bin")])
    
    # Check if no file is selected
    if file_path == "":
        LOG("No file is selected")
    else:
        LOG("Selected file: " + file_path)

    # Clear and update the file entry widget
    file_entry.delete(0, tk.END)
    file_entry.insert(tk.END, file_path)


"""
Function: flash
Description: Flashes the selected binary file onto the STM32.
@return: None
"""
def flash():

    global file_path

    # Check if no port is connected
    if actual_connected_port == '' or serial_port == None:
        LOG("No port is connected")
    
    # Check if no binary file is selected
    elif file_path == "":
        LOG("Please select the binary file you want to flash onto the STM32.") 

    else:
        # Check if the file is in ELF format
        if file_path.endswith('.elf'):
            file_name = os.path.basename(file_path)
            file_name = file_name[:-4] + '.bin'
            
            print("file path:", file_path)
            print("file_name:", file_name)

            LOG('Converting the imported ELF file to BIN ...')

            elf_to_bin_program_path = os.path.dirname(os.path.abspath(__file__)) + "\\objcopy.exe"
            
            try:
                result = subprocess.run([elf_to_bin_program_path, '-O', 'binary', file_path, file_name], check=True)
            
                if result.returncode == 0:
                    LOG("Conversion from ELF to BIN was successful")
                    file_path = file_name
                else:
                    LOG("Failed to convert ELF to BIN with an error code:" + result.returncode)
                    return

            except FileNotFoundError as e:
                LOG("Error: The ARM toolchain objcopy program for the conversion could not be found.")
                return
            
            except Exception as e:
                LOG("Failed to Convert ELF to BIN with an exception:" + str(e))
                return

        LOG("Flashing onto STM32 the binary file: ./" + file_path)

        # Flash the binary file onto the STM32 
        SendBinaryFile(serial_port, file_path, LOG)


"""
Function: erase
Description: Sends an erase command to the bootloader to erase the user application.
@return: None
"""
def erase():

    global actual_connected_port 
    global serial_port

    # Check if no serial connection is established
    if actual_connected_port == '' or serial_port == None:
        LOG("No serial connection established")
        
    elif SendCMD(serial_port, CMD_ID_ERASE_APP, LOG) == CMD_RESP_STATUS_OK:
        LOG("Bootloader Successfully Erased User Application")


"""
Function: execute
Description: Sends the EXECUTE command to the bootloader to start executing the user application.
@return: None
"""
def execute():

    global actual_connected_port 
    global serial_port

    if actual_connected_port == '' or serial_port == None:
        LOG("No serial connection established")

    else:
        status = SendCMD(serial_port, CMD_ID_EXECUTE, LOG) 
        if status == CMD_RESP_STATUS_OK:
            LOG("Bootloader Executing User Application")


"""
Function: clear
Description: Clears the log display by deleting all the text in the log_text Text widget.
@return: None
"""
def clear():
    log_text.delete('1.0', tk.END)


''' Main '''

# Create the main window
window = tk.Tk()
window.title("Bootloader Command Interface")
window.geometry("800x500")

window.maxsize(width=800, height=500)
window.minsize(width=600, height=500)

# Create the left frame
left_frame = tk.Frame(window)
left_frame.pack(side=tk.LEFT, padx=10)

# PORT group
port_frame = tk.LabelFrame(left_frame, text="PORT")
port_frame.pack(fill=tk.BOTH, padx=10, pady=10, ipady=5)

com_port_label = tk.Label(port_frame, text="COM Port:")
com_port_label.pack()

#
var_com_port = tk.StringVar(window)
com_port_menu = tk.OptionMenu(port_frame, var_com_port, "")
com_port_menu.pack(pady=5)

# Create the scan ports button
scan_button = tk.Button(port_frame, text="Scan Ports", command=scan_ports, width=12)
scan_button.pack()

connect_button = tk.Button(port_frame, text="Connect", command=connect, width=12)
connect_button.pack(pady=10)

disconnect_button = tk.Button(port_frame, text="Disconnect", command=disconnect, width=12)
disconnect_button.pack()

# FILE group
file_frame = tk.LabelFrame(left_frame, text="FILE")
file_frame.pack(fill=tk.BOTH, padx=10, pady=10, ipady=5)

file_label = tk.Label(file_frame, text="File:")
file_label.pack()

file_entry = tk.Entry(file_frame)
file_entry.pack(padx=5, pady=5)

import_button = tk.Button(file_frame, text="Import", command=import_file, width=12)
import_button.pack()

# BOOTLOADER group
bootloader_frame = tk.LabelFrame(left_frame, text="BOOTLOADER")
bootloader_frame.pack(fill=tk.BOTH, padx=10, pady=10)

flash_button = tk.Button(bootloader_frame, text="FLASH", command=flash, width=12)
flash_button.pack(pady=5)

erase_button = tk.Button(bootloader_frame, text="ERASE", command=erase, width=12)
erase_button.pack(pady=5)

execute_button = tk.Button(bootloader_frame, text="EXECUTE", command=execute, width=12)
execute_button.pack(pady=5)

# Create the right frame with scrolling text box
right_frame = tk.Frame(window)
right_frame.pack(side=tk.RIGHT, padx=10)

# Create the log display label
log_label = tk.Label(right_frame, text="LOG DISPLAY")
log_label.pack(pady=5)

# 
scrollbar = tk.Scrollbar(right_frame)
scrollbar.pack(side=tk.RIGHT, fill=tk.Y)

log_text = tk.Text(right_frame, yscrollcommand=scrollbar.set)
log_text.pack(fill=tk.BOTH, padx=10, pady=10)

scrollbar.config(command=log_text.yview)

# Create the CLEAR button
clear_button = tk.Button(right_frame, text="CLEAR", command=clear)
clear_button.pack(side=tk.BOTTOM, padx=10, pady=5)

# Start the main loop
window.mainloop()

