clc;
clear all;
close all
syms  kp ki Immp D_cycle D_prime s Vout Gdo Ggo Wz Q Wo Rout Cout vg_hat L ZC ZR ZL Z_CR  Iout vout_hat L_ESR d_hat I Ztotal_series V_q I_q 
        
  

%Initialisaztion and Assignment of Variables 
% 
% D_cycle = 0.671;
% D_prime = 1-D_cycle;
% Cout=  250e-06;
% Rout=12;
% L_ESR=1.38e-3;
% L = 200e-06;
% Vout = 60;
% Gdo= Vout/D_prime;
% Ggo=1/D_prime;
% Wz=(Rout*D_prime^2)/L ;
% Q=  D_prime*Rout*sqrt(Cout/L);
% Wo= D_prime/sqrt(L*Cout);
% Immp=15 ;
% I_q =Immp;
% fs = 25e3
% Ts = 1/fs

% 

% TOTAL_I = (Vout/D_prime - Vout)/(L/D_prime^2*s + L_ESR) + Immp +  Vout/(L/D_prime^2*s + L_ESR)
% TOTAL_I= simplify(TOTAL_I)

% TOTAL_I_num=[Immp*L   Immp*L_ESR*D_prime^2+Vout*D_prime]
% TOTAL_I_den= [L  L_ESR*D_prime^2 ]
% TOTAL_I_TF = tf(TOTAL_I_num,TOTAL_I_den)
% step(TOTAL_I_TF)









% %Delays
% 
% Total_delay_num = [ 1]
% Total_delay_den = [1.5*Ts  1]
% Total_delay_TF = tf(Total_delay_num, Total_delay_den);



%Inductor Current Transfer Functions 

%Gig (Line to Inductor Current Transfer Function)Calculation
ZC = 1/(s*Cout)
ZR = Rout
ZL = (L*s) / D_prime^2
Z_RC = (ZC*ZR)/(ZC+ZR)
Ztotal_series = Z_RC + ZL;
IL =  (vg_hat/D_prime) / Ztotal_series ; 
Gig = simplify(IL / vg_hat);







%Gid(Control to output Voltage TF) Calculation ..used superposition for the final circuit

% i_hat_voltage_source = (V_q*d_hat/D_prime) / (Ztotal_series);
% %i_hat_current_source = (I*d_hat)* (1/ZL)/(1/ZL + 1/ZC + I/Rout)
% 
% i_hat_current_source = (I_q*d_hat)*(Z_RC/(ZL+Z_RC))    ;        % (1/ZL)/(1/ZL + 1/ZC + I/Rout)
% 
% i_hat= i_hat_voltage_source + i_hat_current_source;
% Gid = simplifyFraction(i_hat / d_hat)                         TO
% EVALUATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


 %Time constants and GAIN values
%L/Rout*(D_prime)^2
 
 T_L = L/L_ESR                  %      *(D_prime)^2;
 T_C = Rout*Cout;
 Kp_L=5;
 Ki_L= Kp_L/T_L; 

k= 0:0.1:10;

% 
% Gc_id_num=[Kp_L  Ki_L];
% Gc_id_den= [1];
% Gc_id_TF =tf(Gc_id_num, Gc_id_den )
% 
% 
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

% 
% Gid_num = [Vout*Cout     Vout/Rout+D_prime*I_q ];
% Gid_den =  [Cout*L    L/Rout  D_prime^2];
% Gid_TF = tf(Gid_num , Gid_den)
% % pole(Gid_TF)



% step(Gid_TF)
% 
% figure('Name','Bode For Gid_TF')
% bode(Gid_TF)
 % figure('Name','Root Locus For Gid_TF ')
 %  rlocus(Gid_TF )
% figure('Name','pzmap For Gid_TF ')
% pzmap(Gid_TF )    %poles and zeros 
% figure('Name','Nyquist For  Gid_TF ')
% nyquist(Gid_TF )
% grid on 


% figure('Name','Bode For Total_delay_TF')
% bode(Total_delay_TF)
% figure('Name','Root Locus For Total_delay_TF ')
 % rlocus(Total_delay_TF)
% figure('Name','pzmap For Total_delay_TF ')
% pzmap(Total_delay_TF )    %poles and zeros 
% figure('Name','Nyquist For  Total_delay_TF ')
% nyquist(Total_delay_TF )
% grid on 

% 
% Gid_TF_Total_delay = Gid_TF*Total_delay_TF
% Gid_Controller = PI_TF*Gid_TF
%Gid_Controller= simplifyFraction(Vout*Cout*s  + Vout/Rout+D_prime*I_q / (Cout*L*s^2 + s^2*L/Rout +D_prime^2 ))



% controlSystemDesigner(Gid_TF*Total_delay_TF)
% pole(Gid_TF*Total_delay_TF)

% figure('Name','Bode For Gid_TF_Total_delay ')
% bode(Gid_TF_Total_delay )
% figure('Name','Root Locus For   For Gid + Total_delay_TF ')
% rlocus(Gid_TF*Total_delay_TF,k)
% % 





%Output Voltage Inductor Current Transfer Functions 


%Gid (Control to Inductor Current TF) calculation 



%Gvg (Line to output Voltage TF) Calculation 
% vout_hat =  (vg_hat/D_prime)*(Z_RC/(ZL+Z_RC));
% Gvg = simplifyFraction (vout_hat / vg_hat)
% 
% 
% 








%Gvd = tf( [-Gdo/Wz Gdo], [ (1/Wo)^2  1/(Q*Wo)  1] )

% Gvd = ( Gdo - (Gdo*s)/Wz) / (1+ s/(Q*Wo)+ (s/Wo)^2 )
% Gid = simplifyFraction(Gvd /Rout)
% 
% 
% Gvg = ( Ggo) / (1+ s/(Q*Wo)+ (s/Wo)^2 )
% Gig = simplifyFraction(Gvg /Rout)



% %bode(Gvd, {2*pi ,30e3*2*pi} )
% figure;
% nyquist(Gvd)








% 
% %ROOT LOCUS 
% b= 1.5
% num = [1];
% den = [ 1 1*b 1]
% 
% 
% 
% H = tf(num,den);
% 
% figure
% rlocus(H)
% figure
% pzmap(H)    %poles and zeros 
% figure 
% nyquist(H)
% grid on 
