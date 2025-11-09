%These are dummy Values to be changed later 
%tHESE EQUATIONS HERE are fro textas instruments 
clear all;
clc;

Iin_max = 15; %I had to include this because in my appkication i dont thinki need select the Iout_Max but rather 
                %Calculate it based on the Input and Output current
                %relationship based on the current that i get from the PV
                %panel

%VIN_min=20;
%VIN_max=45;
VOUT=60;

VIN_power=20;

fs= 100e3  %100kHz
Ts = 1/fs;
%fs is the minimum switching freq

n=95/100; %estimated efficiency


%DUTY CYCLE CALCULATION FOR THE WORST CASE SCENARION OF MAX CURRENT 
D_cycle = 1 - ((VIN_power*n)/VOUT)

IOUT_max= Iin_max*(1-D_cycle)*n       %calculated IOUT_MAX

%Inductor Selection

IL_ripple_p_p_estimated = 0.1*(IOUT_max)*(VOUT/VIN_power)  %use this to estimate the ripple so as to calculate Current!!

L= (VIN_power * (VOUT -VIN_power)) / (IL_ripple_p_p_estimated *fs*VOUT)

wanted_ripple=0.1;  

%percentage_wanted_ripple = (wanted_ripple / Iin_max )*100


%L_2= (VIN_power*D_cycle)/(fs*wanted_ripple)

%Calculation of Maximum Switch Current 


IL_ripple_p_p =  (VIN_power*D_cycle)/(fs*L)  %here  i calculate the real inductor currrent








%ILIMmin=10; %%need to consider this

%I_MAXOUT = (ILIMmin - (IL_ripple_p_p/2)) * (1-D_cycle)  %this i hve to check  with I_sw_max has to be higher

I_sw_max_peak = (IL_ripple_p_p/2) + Iin_max                 %( IOUT_max/((1-D_cycle)*n))   %max switch current of the application 

Ripple_Percentge = ( IL_ripple_p_p / (IOUT_max/(1-D_cycle)) )*100  %percentage ripple 

%Diode calculation this is only for if i wanna use the diode instaed of
%uisng a second switch for the power stage 
V_f = 0.7 ; 

I_f= IOUT_max;
P_d = I_f*V_f  %Power dissipation from the diode 



%Output  Capacitor
VOUT_ripple = 10/100;
COUT_min = (IOUT_max * D_cycle) / (fs*VOUT_ripple)

%The ESR FROM CAPACITORS ADDS SOME RIPPLE ALSO SO WE HAVE TO ADD CONSIDER
%IT AND CHOOSE SOMETHING WITH LESS ESR

% ESR=0.02;
% VOUT_ripple_esr = ESR *  ( (IOUT_max/(1-D_cycle)) + (IL_ripple_p_p/2) )


%Output Voltage Setting Voltage devider network 







