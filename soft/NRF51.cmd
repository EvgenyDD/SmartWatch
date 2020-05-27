openocd -f "H:/WORKSPACE/NRF51/SmartWatch/NRF51_flash_script.tcl"


goto comments

start openocd -f interface/stlink-v2.cfg -f target/nrf51_stlink.tcl ::-d3
telnet 127.0.0.1 4444
color 0A 

   
Commands:
>reg - registers
>flash info 0 - data information
>nrf51 mass_erase

> flash banks  - to flash (only use bank #0 starting from address 0)

> flash write_image erase PATH-TO-YOUR-BINARY/YOUR-BINARY.bin 0
wrote xxx bytes from file PATH-TO-YOUR-BINARY/YOUR-BINARY.bin in xx.yys (x.yyy KiB/s)

flash write_image erase "C:/Users/Evgeny/workspace/nRF51_START/bin/Debug/nRF51_START.hex" 0


> reset

:comments