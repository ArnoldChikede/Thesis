clc;
clear all;
close all
%syms  kp ki Immp D_cycle D_prime s Vout Gdo Ggo Wz Q Wo Rout Cout vg_hat L ZC ZR ZL Z_CR  Iout vout_hat L_ESR d_hat I Ztotal_series V_q I_q 
  
%%%%%%%%%%%%%%%%%$%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  

       %%%%%%%%%%%%%%% ARNOLD ALWAYS SYNCHRONISE VARIABLES%%%%%%%%%%%%%%%%%%


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%INITIALISAZTION AND ASSIGNMENT OF VARIABLES 

Vmppt = 17.6 ;  
Imppt = 3.69  ;
Vout = 60;
Rmppt = Vmppt/Imppt;
Rboost = Rmppt;

fs = 80e3  ;
Ts = 1/fs ;
  
Rout = Rboost/((Vmppt/Vout)^2 )

D_cycle= 1- (sqrt(Rboost/Rout))
D_prime = 1-D_cycle;

Cout= 14e-06 ;
L_ESR=1.38e-3;
L = 150e-06;

I_q =Imppt;


fo = (D_prime)/(2*pi*sqrt(L*Cout))    %Resonant Freq
Wo = (D_prime)/(sqrt(L*Cout)) 

Zeta = L*Wo/(2*Rout*(D_prime)^2)  % The L_eff seen = L/Dprime^2    thats why we have the Dprime appering in teh Damping Ratio Equation  
 
Wd = Wo*sqrt(1-Zeta^2)

f_RHPZ_hz = (Rout*(D_prime)^2)/(2*pi*L)
f_RHPZ_rad = (Rout*(D_prime)^2)/(L)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Time constants and GAIN values

TL_2=L/Rout*(D_prime)^2
T_L = L/L_ESR       %  *(D_prime)^2;
T_C = Rout*Cout;
Kp_L=0.001;
Ki_L= Kp_L/T_L; 

TAU_EXP = 1/(1.0e+04 * 0.0556)



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%



% Gc_id_wthout_kp_num = [T_L 1];
% Gc_id_wthout_kp_den = [T_L];
% Gc_id_wthout_kp_TF = tf(Gc_id_wthout_kp_num, Gc_id_wthout_kp_den )

% figure('Name','Bode For Gc_id_TF ')
% bode(Gc_id_TF)
% figure('Name','Root Locus For Gc_id_TF ')
% rlocus(Gc_id_TF)
% figure('Name','pzmap For Gc_id_TF ')
% pzmap(Gc_id_TF )    %poles and zeros 
% figure('Name','Nyquist For Gc_id_TF ')
% nyquist(Gc_id_TF )



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Duty to Inductor Current Transfer Functions 



%Gid_num = [Vout*Cout     Vout/Rout+D_prime*I_q ];
%Gid_den =  [Cout*L    L/Rout  D_prime^2];
%Gid_TF = tf(Gid_num , Gid_den)


Gid_main = 2*Vout/((1-D_cycle)^2*Rout)
Gid_num = [Gid_main*(Rout*Cout/2)  Gid_main];
Gid_den = [L*Cout/(1-D_cycle)^2  L/(Rout*(1-D_cycle)^2)  1 ];
Gid_TF =tf(Gid_num , Gid_den)


%step(Gid_TF)




% step(Gid_TF)
% figure('Name','Bode For Gid_TF')
% bode(Gid_TF)
%figure('Name','Root Locus For Gid_TF ')
 %rlocus(Gid_TF )
 figure('Name','pzmap For Gid_TF ')
 pzmap(Gid_TF )    %poles and zeros 
% figure('Name','Nyquist For  Gid_TF ')
% nyquist(Gid_TF )
% grid on 

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Delays

Total_delay_num = [ 1]
Total_delay_den = [1.5*Ts  1]
Total_delay_TF = tf(Total_delay_num, Total_delay_den);


% figure('Name','Bode For Total_delay_TF')
% bode(Total_delay_TF)
% figure('Name','Root Locus For Total_delay_TF ')
% rlocus(Total_delay_TF)
 figure('Name','pzmap For Total_delay_TF ')
pzmap(Total_delay_TF )    %poles and zeros 
% figure('Name','Nyquist For  Total_delay_TF ')
% nyquist(Total_delay_TF )
% grid on 


% controlSystemDesigner(Gid_TF_Total_delay)
% pole(Gid_TF_Total_delay)
% zero(Gid_TF_Total_delay)
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Controller 

kp = 0.0376;
ki =0; 

Gc_id_TF_num = [kp ki]
Gc_id_TF_den = [1 0]
Gc_id_TF = tf(Gc_id_TF_num, Gc_id_TF_den);

%-
% ,pzmap(Gc_id_TF)

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Gid Combined with delay 

Gid_TF_Total_delay = Gid_TF*Total_delay_TF;

k= 0:0.0001:25;

% Gid_Controller = PI_TF*Gid_TF
%Gid_Controller= simplifyFraction(Vout*Cout*s  + Vout/Rout+D_prime*I_q / (Cout*L*s^2 + s^2*L/Rout +D_prime^2 ))


% figure('Name','Root Locus For   For Gid + Total_delay_TF ')
% rlocus( Gid_TF_Total_delay,k)
% figure('Name','Bode For   For Gid + Total_delay_TF ')
% bode( Gid_TF_Total_delay)
% 
%figure('Name','Step For   For Gid + Total_delay_TF + Gc_id_TF ')
%step(Gid_TF_Total_delay*Gc_id_TF ) 
% figure
% step(Gid_TF_Total_delay ) 
% figure
% pzmap(Gid_TF_Total_delay )
% controlSystemDesigner(Gid_TF_Total_delay )
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Open and Closed Loop Gain 


LOOP_GAIN_OPEN_LOOP_TF = Gc_id_TF*Gid_TF*Total_delay_TF ;
% 
% 
CLOSED_LOOP_TF= LOOP_GAIN_OPEN_LOOP_TF /(1 + LOOP_GAIN_OPEN_LOOP_TF);
% 
% 
figure('Name','Bode For Open Loop Gain ')
bode(LOOP_GAIN_OPEN_LOOP_TF )
% 
 % figure('Name','Bode For Closed Loop Gain ')
 % bode(CLOSED_LOOP_TF )

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%







%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%Keywords from toolbox

% figure
% rlocus(H)
% figure
% pzmap(H)    %poles and zeros 
% figure 
% nyquist(H)
% grid on 
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
