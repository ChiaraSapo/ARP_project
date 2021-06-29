#!/bin/sh
# ----Modify the file name to make it point to your configuration file path:----
# Source the configuration file and read it 
if [ -r /home/sapo/ARP/config_file.cfg ]
then
. /home/sapo/ARP/config_file.cfg
else
echo "Problems with the configuration File"
exit 99
fi

gcc G.c -o G
gcc main.c -o main
./main "$pathlog" "$IP_ADDRESS" "$RF" "$process_G_path" "$number_of_ports" "$DEFAULT_TOKEN" "$waiting_time" "$Pid_P_path" "$Pid_G_path"
