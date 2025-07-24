BOARD_TAG = electroniccats:mbed_rp2040:bombercat
# BOARD_TAG = arduino:mbed_rp2040:pico
# BOARD_TAG = rp2040:rp2040:generic
MONITOR_PORT = /dev/cu.usbmodem1101

compile:
	arduino-cli compile --fqbn $(BOARD_TAG)

upload:
	arduino-cli upload -p $(MONITOR_PORT) --fqbn $(BOARD_TAG) --verbose

monitor:
	arduino-cli monitor -p $(MONITOR_PORT)

clean:
	arduino-cli cache clean

wait:
	sleep 2

all: compile upload wait monitor