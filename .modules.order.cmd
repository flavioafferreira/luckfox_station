cmd_/home/flavio/drivers/lora_station/modules.order := {   echo /home/flavio/drivers/lora_station/station.ko; :; } | awk '!x[$$0]++' - > /home/flavio/drivers/lora_station/modules.order
