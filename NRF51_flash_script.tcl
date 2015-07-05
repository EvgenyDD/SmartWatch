source [find interface/stlink-v2.cfg]
source [find target/nrf51_stlink.tcl]

init    
halt

flash write_image erase "H:/WORKSPACE/NRF51/SmartWatch/bin/Debug/SmartWatch.elf" 0
verify_image "H:/WORKSPACE/NRF51/SmartWatch/bin/Debug/SmartWatch.elf"

reset run 
sleep 20
shutdown