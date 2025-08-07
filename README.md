Jack's Test Branch for New PCB/Code Implementations

Steps for STM32 Development Setup With PlatformIO:

1. Install the PlatformIO IDE extension within VSCode
2. Using Zadig and having the board you want to program connected via USB, install drivers to use DFU programming
3. When creating a new PlatformIO project, enable the following flags in the platformio.ini file:
        
        [env:genericSTM32F411CE]
        platform = ststm32
        board = genericSTM32F411CE
        framework = arduino
        upload_protocol = dfu
        monitor_speed = 9600
        build_flags = 
	        -D ENABLE_USB_SERIAL
	        -D PIO_FRAMEWORK_ARDUINO_ENABLE_CDC
        lib_deps = 
            ....
            ....

Extra Important Notes:

 - When creating a new PlatformIO project, you should be able to select a specific version of STM32, make sure you select the board version you are using, and make sure it is provided in the platformio.ini file. I'm using a STM32F411CE, so that is why it is written in the env and board fields.

 - The monitor_speed parameter should be adjust to whatever monitor speed you wish to use and should match whatever speed you provide in Serial.begin(...).

 - If you want multiple monitors:
    Select the COM port you wish to monitor
    Go to the Terminal tab, click the drop down next to the plus on the right hand side (its near the running tasks window)
    Click 'Run Task' and then click the 'PlatformIO: Monitor' task that should have appeared at the top of the window

- If you want to add external libraries: click the PlatformIO extension button on the left, then click  libraries to search for what you want. Adding libraries should add the library's name in the lib_deps section in the platformio.ini file. 