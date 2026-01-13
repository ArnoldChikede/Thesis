Inductor

SCF19XV-220-1R6A004JV



Notes on inductors to take note

* Importance of the core  in guiding the field
* the core you choose also has  a role because we all know that the Xl --reactance (ohms) = 2pifL changes with freq so we have to get a core material that will try to keep these losses very low esp at higher freq



Selection Criteria for Inductors

When designing a DC-DC converter, selecting the right inductor is critical. Here are some key factors to consider:

Inductance Value: The inductance value affects the energy storage capacity and the output current ripple. A higher inductance value typically results in lower ripple but may slow down the response time of the converter.



* Saturation Current: This is the maximum current the inductor can handle before its inductance drops significantly. Choosing an inductor with a saturation current higher than the maximum load current is essential to prevent performance degradation.

 	It is crucial to choose one that can handle the maximum ripple current without saturating. The inductor should 	have a current rating higher than the peak inductor current, which includes the average current plus half of the 	ripple current.





* DC Resistance (DCR): Lower DCR is preferred to minimize power losses and heat generation within the inductor. This contributes to the overall efficiency of the DC-DC converter.



* Core Material: The choice of core material (e.g., ferrite, powdered iron) affects the inductor's performance, including its efficiency and frequency response. Ferrite cores are commonly used for high-frequency applications due to their low losses.





Link to selection of inductors



https://www.ti.com/lit/an/slva157/slva157.pdf





MOSFET SELECTION 

Importantly we  need to focus on the VGS so that we can manage to stay safe especially for the off state \\



VGS also important and also dependable on the current thats gonna need to flow through the channel since it sets the conductive channel capabilities for that moment



continuos drain current -- max current it can carry when it conducts 



RDs On for calculation of losses 

This parameter actually increases with the increase in VDS



IDSS -drain leakage current 

evaluated at VGS = 0

as small as possible 

Depends on Juntion temp and VDs 



Body diode intermsof inductive loads(3rd quadrant usage )

Its voltage is VSD  -- source to drain voltage 











