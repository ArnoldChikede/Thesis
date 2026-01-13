Design A PCB Board Prototype with the following things to consider



* Temperature sensor incorporated...will also check if things kike humidity are  required
* Wifi and cloud capabilities (Covered under the esp32 chip) ---actually THE WIFI PART IS MORE OR LESS Set-Up
* Led Display (Still yet to decide which one to use) --ILI9341 (2.4–2.8″) , ILI9488 (3.5″)
* Boost converter circuit obviously and its components 
* input Terminals Header for V\_Supply also same header for Output Terminals 
* External Headers For debugging Obviously Signals going to MCU 
* Zero Ohm Resistor in each and every Line just to Disconnect Later On if Any mistakes happen  
* Probably a four-layer board(With bottom and top as ground poured for shielding)..To decide on this 
* Consider  heat sinks for cooling also...Might not be necessary will see 
* We might need an input filter for the ADC measured signals, of course, if there is noise lingering in the measured signals..To consider this...something with a simple comms protocol obv 
* For the ESP32  "The ESP32 ADC is sensitive to noise, leading to large discrepancies in ADC readings. Depending on the usage scenario, you may need to connect a bypass capacitor (e.g., a 100 nF ceramic capacitor) to the ADC input pad in use, to minimize noise. Besides, multisampling may also be used to further mitigate the effects of noise."



We shall come back to the top point when we are now designing the PCB etc



https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user\_guide.html#get-started-esp32-devkitc-board-front
Dimenisons for the ESP32 DEVKIT V4 board







