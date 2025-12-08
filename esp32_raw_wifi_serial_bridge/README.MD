Burn the pre-built firmware to ESP32
=========================================
Download from here the files: bootloader.bin, partition.bin, firmware.bin
Erase flash: esptool.py --port /dev/ttyUSB0 erase_flash
Burn firmware:
esptool.py \
    --chip esp32 \
    -p /dev/ttyUSB0 \
    -b 460800 \
    --before default_reset \
    --after hard_reset \
    write_flash \
    -z \
    --flash_mode dio \
    --flash_freq 40m \
    --flash_size detect \
    0x1000 bootloader.bin \
    0x8000 partition.bin \
    0x10000 firmware.bin

Next build KLIPPER for SERIAL communication
-------------------------------------------
git clone https://github.com/KevinOConnor/klipper  
cd klipper  
make menuconfig 
=> Communication Interface (UART0 on GPIO0/GPIO1)  
=> (500000) Baud rate for serial port   
make  
*Note* - you might need for make command: sudo apt install gcc-arm-none-eabi  

keep pressing BOOT button when you power the pico and it will boot in usb mass storage mode.   
copy ./klipper/out/klipper.uf2 ===> usb mass storage  
it dissapears = ok  

Now for KLIPPER connect ESP32 to your MCU running Klipper pins (the ones you select in klipper)  
Connect RX->TX,TX->RX and 5v, GND to ESP32

Serial to TCP bridge on host running klipper - SOCAT
====================================================
manual for testing
--------------------
/usr/bin/socat -d -d \
    pty,raw,echo=0,mode=666,link=/tmp/espbridge \
    tcp:192.168.4.1:8888,nodelay

service permanent use
----------------------
copy espbridge.service to /etc/systemd/system/
sudo copy espbridge.service /etc/systemd/system/
sudo systemctl enable espbridge
sudo systemctl start espbridge

klipper config:
[mcu]
serial: /tmp/espbridge

Manual build the firmware:
===================================
Install platformio
---------------------
phyton3 -m venv platformio  
source platformio/bin/activate  
pip install platformio  

Create platformio project
------------------------
mkdir esp_raw_bridge  
cd esp_raw_bridge  
pio init --board esp32dev  
copy to src/  - main.cpp  

Compile then upload  
pio run -e esp32_klipper_bridge  
pio run -e esp32_klipper_bridge -t upload --upload-port /dev/ttyUSB0  
*change ttyUSB0 to what your device is - ussualy is this, can be 1,2 etc if you have more usb-serial devices connected

Test it
-------------------------
connect to 3D_Printer AP, password: CNCTinker --- see main.cpp source code.  
ip a   
	-- should show 192.168.4.2  
telnet 192.168.4.1 8888  
Open serial - I prefer CuteCom GUI, but can use a console serial like  
screen /dev/ttyUSB0 500000  
*Note: you need a Serial-to-USB converter for this connected to pins 16,17 of esp32 (see source code).  
Now type something in telnet and serial, and you should see it on the other side.
