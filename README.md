# keyTime
*keyTime, analyse timing leakage from ECUs over the CAN bus network* :car:

### Overview
This program was created for my final year project at university. It is to send a seed request to the electronic control unit (ECU)
followed by replying with a key, specifically this work was done with diagnostic IDs. The amount of time it takes for the ECU to
respond to whether the key is correct or not is measured. With the amount of time it takes that is possibly the source of timing leakage
and as a result could be susceptible to an attack which would grant the user access to that ECU and possible other ECUs on the network
to send codes to that would affect the vehicle. This project was set up to analyse this time difference. The seeds will be different each attempt but in this case the key will always be the same `06 27 02 00 00 00 00` .

---

### Flow
The flow of the program goes as follows:
* Reset the ECU - Too many failed key attempts may lock you out for a couple of seconds, this is an attempt to get around it.
* Put the ECU into programming mode - This is how the ECU prepares for seed and key exchanges.
* Send a seed request - Ask the ECU for a seed to be able to gain access.
* Save the seed value for later - Will be written to a csv file.
* Send the key to the ECU - In this situation the key we send will always be the same, unlike the seed.
* Start the timer - Start the time at the moment the sending of the key has happened.
* Wait on a response from the ECU - The ECU will respond with or a success or fail, we don't expect to ever get a succes with this key but you could keep changing the key to your liking.
* Stop the timer - As soon as we have the response the timer will stop.
* Output the seed and the time difference to a CSV.

---

### Installation
`git clone https://github.com/Shmink/CAN-bus-analyse-timing-leakage.git && cd CAN-bus-analyse-timing-leakage && make`

If you need to test it virtually first you can run `chmod +x makeVCAN0.sh && ./makeVCAN0.sh` to save you some time.

---

### Usage
`./keyTime` should show you all you need to know.

---

### My set up
* This was installed onto a Raspberry Pi 3 Model B (@ 1.2 GHz) with a PiCAN2 CAN-Bus board.
* The OS for it was Raspbian Stretch Lite and the kernel patched to Real-Time Linux kernel (Went from a max latency of ~500us to 72us)
* Wired from the 4 screw terminal on the PiCAN2 board to the pins of the OBD-2 port (Pins: 6, 14, 5) ???
* The ECU was from a Fiat Punto ????. The bitrate of which was 500 kb/s.

### Mentions
* Thanks to `SocketCAN` for making this work easier to deal with. [Documentation](https://www.kernel.org/doc/Documentation/networking/can.txt).
* Thanks to `can-utils` for a great resource of open source work. [Repository](https://github.com/linux-can/can-utils).
