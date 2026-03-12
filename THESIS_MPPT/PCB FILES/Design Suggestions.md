Design A PCB Board Prototype with the following things to consider



* Temperature sensor incorporated...will also check if things kike humidity are  required
* Wifi and cloud capabilities (Covered under the esp32 chip) ---actually THE WIFI PART IS MORE OR LESS Set-Up
* Led Display (Still yet to decide which one to use) --ILI9341 (2.4–2.8″) , ILI9488 (3.5″)......3.5TFT LCD WITH TOUCH  --Might Not Need this
* Boost converter circuit obviously and its components
* input Terminals Header for V\_Supply also same header for Output Terminals
* External Headers For debugging Obviously Signals going to MCU
* Zero Ohm Resistor in each and every Line just to Disconnect Later On if Any mistakes happen
* Probably a four-layer board(With bottom and top as ground poured for shielding)..To decide on this
* Consider  heat sinks for cooling also...Might not be necessary will see
* We might need an input filter for the ADC measured signals, of course, if there is noise lingering in the measured signals..To consider this...something with a simple comms protocol obv
* For the ESP32  "The ESP32 ADC is sensitive to noise, leading to large discrepancies in ADC readings. Depending on the usage scenario, you may need to connect a bypass capacitor (e.g., a 100 nF ceramic capacitor) to the ADC input pad in use, to minimize noise. Besides, multisampling may also be used to further mitigate the effects of noise."
* might need fuses
* current and voltage sensors best guess ICs already made for that
  Reverse Protectiom mosfet
*  



We shall come back to the top point when we are now designing the PCB etc







Inductor selections and things to watchout

* My inductor is huge okay 200 uH
* i will need some core and litz wire

IHV20BZ200









Inuctor important parameters ]

Lets take a toroidal core

Lets maybe have "Black" or "Sendust" Ring

Powder Cores: They saturate gradually. If you hit 7A, the inductance might drop by 20% (to 120 µH), but it won't crash to zero instantly. This gives you a safety net.

Ferrite Cores: (Used in transformers) saturate instantly. They are very dangerous if you miscalculate.

Coecivity  also another important parameter of indductors because it shows howhard it  is to  demagnetize an inductor and this is what you would want for your Indcutor i believe ...and you want it to be low as possible for the Powerr applications

Sendust Advantages to look at



DTMSS-40/0.15/15-V    better for high current but faster dynamics because its smaller but higer ripple  same size with lower version -- poor inductance curve with hiher currents   -- 46.5mm

DTMSS-40/0,22/10-V   stronger option for now  coz of higher inductance so less ripple  same size with upper version -- But Inductance still higer han my min value at 5Amps    45.5mm







**DTSM-47/0,47/6,3   51mm  higer RDC  0.102**

DTSM-47/0,33/10    53        RDC = 0.051

DTSM-47/0,22/13    53        RDC = 0.035



https://www.micros.com.pl/en/inductors-chokes-emi-filters/inductors/toroidal-tht-inductors/





https://www.linkedin.com/pulse/comprehensive-pcb-design-guidelines-dc-dc-converters-dileep-chacko-7zu8e/









https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user\_guide.html#get-started-esp32-devkitc-board-front
Dimenisons for the ESP32 DEVKIT V4 board





https://onlinelibrary.wiley.com/doi/10.1155/2018/3286479  
Herr we have selection of te Input Capacitor

