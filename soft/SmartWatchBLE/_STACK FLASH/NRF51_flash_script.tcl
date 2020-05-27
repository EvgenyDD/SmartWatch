source [find interface/stlink-v2.cfg]
source [find target/nrf51_stlink.tcl]

init    
halt

flash write_image erase "s110.hex" 0
verify_image "s110.hex"

reset run 
sleep 20
shutdown