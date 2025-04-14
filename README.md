PlatformIO Notes:
- Install PIO via. VSCode and follow standard installation instructions
- In the platformio.ini file, change the following;
- upload_port to whatever port the Serial to UART for the UPDI is connected to
- monitor_port to whatever port the Serial to UART the actual serial monitor is connected to

hopefully, in the new board design, Serial/UPDI will be a toggle DIP switch or something

TODO BY 4/20/25:
- change C7/C8 for proper matching with new potential antenna
- reference PN532 RF application design guide for all total changes
- build an actual layout for a board
