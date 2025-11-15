%we firstly try to design the converter for the operating point that we
%want to work in 

clc; 
clear all ;

Vmppt = 20;   %This is out Vin
Imppt = 15; 
Rmppt = Vmppt/Imppt;
Rboost = Rmppt   % This condition allows for operating at max power  point
                   %here we use only one curve later on we can use a lower
                   %graph but might not be necessary 
n= 80/100;
fsw =20e3;

Rout = Rboost/((Vmppt/60)^2 )   %%CAN HAVE IT TRIAL AND ERROR OR CALCULATE it depends with the condiiton 

D= 1- (sqrt(Rboost/Rout))
%D=0.69391
Vout = Vmppt/(1-D)

Lmin = (D/(n*fsw))*Rboost 

IL_ripple_p_p =  (Vmppt*D)/(fsw*Lmin) 

Isw_max = Imppt + (IL_ripple_p_p/2)


Cin_min = 1/(8*fsw*Rboost) %These  miniminum values we can change them later to higer values 

Cout_min = D/(n*fsw*Rout)


%wanted_fsw = (Vmppt*D)/(3*Lmin) % for 10 percent ripple 


