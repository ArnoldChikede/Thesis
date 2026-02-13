%we firstly try to design the converter for the operating point that we
%want to work in 

clc; 
clear all ;


Vmppt = 17.6  %17.6;   %This is out Vin
Imppt = 3.69  %3.69; 
Rmppt = Vmppt/Imppt;
Rboost = Rmppt   %This condition allows for operating at max power  point
                   %here we use only one curve later on we can use a lower
                   %graph but might not be necessary 
n= 80/100;



fsw = 80e3;

Rout = Rboost/((Vmppt/60)^2 )   %%CAN HAVE IT TRIAL AND ERROR OR CALCULATE it depends with the condiiton 

D= 1- (sqrt(Rboost/Rout))
%D=0.69391
Vout = Vmppt/(1-D)

%Lmin = (D/(n*fsw))*Rboost 

L__new= (Vmppt*D)/ (fsw*2*0.15*Imppt)

%IL_ripple_p_p =  (Vmppt*D)/(fsw*Lmin) 

%Isw_max = Imppt + (IL_ripple_p_p/2)


Cin_min = 1/(8*fsw*Rboost) %These  miniminum values we can change them later to higer values 

%%Cout_min = D/(n*fsw*Rout)

Cout__new= (Vout*D)/(2*Rout*0.005*Vout*fsw)

% cvout = (75e-06*(2*Rout*0.02*Vout*fsw))/D 


%wanted_fsw = (Vmppt*D)/(3*Lmin) % for 10 percent ripple 

% syms a b c d y u s 
% 
% eq1 = a == u - (2*c)/(s+2);
% eq2 = b == a/(s+1);
% eq3 = c == b - 3*d;
% eq4 = d == c/s;
% eq5 = y == d + b;
% 
% sol = solve([eq1 eq2 eq3 eq4 eq5], [a b c d y]);  % Solve for everything
% H = simplify(sol.y / u) 
% 
% b_val = 0:0.5:1;
% num = [5 1];
% 
% 
%     b = 5
%     den = [-2 -7*b -7];
%     TFF = tf(num, den);
%     figure
%     rlocus(TFF)
   
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%EXTRA CODE FROM Transfer Functions script


% Gdo= Vout/D_prime;
% Ggo=1/D_prime;
% Wz=(Rout*D_prime^2)/L ;
% Q=  D_prime*Rout*sqrt(Cout/L);
% Wo= D_prime/sqrt(L*Cout);


% TOTAL_I = (Vout/D_prime - Vout)/(L/D_prime^2*s + L_ESR) + Immp +  Vout/(L/D_prime^2*s + L_ESR)
% TOTAL_I= simplify(TOTAL_I)

% TOTAL_I_num=[Immp*L   Immp*L_ESR*D_prime^2+Vout*D_prime]
% TOTAL_I_den= [L  L_ESR*D_prime^2 ]
% TOTAL_I_TF = tf(TOTAL_I_num,TOTAL_I_den)
% step(TOTAL_I_TF)




%Gig (Line to Inductor Current Transfer Function)Calculation
% ZC = 1/(s*Cout)
% ZR = Rout
% ZL = (L*s) / D_prime^2
% Z_RC = (ZC*ZR)/(ZC+ZR)
% Ztotal_series = Z_RC + ZL;
% IL =  (vg_hat/D_prime) / Ztotal_series ; 
% Gig = simplify(IL / vg_hat);
% 






%Gid(Control to output Voltage TF) Calculation ..used superposition for the final circuit

% i_hat_voltage_source = (V_q*d_hat/D_prime) / (Ztotal_series);
% %i_hat_current_source = (I*d_hat)* (1/ZL)/(1/ZL + 1/ZC + I/Rout)
% 
% i_hat_current_source = (I_q*d_hat)*(Z_RC/(ZL+Z_RC))    ;        % (1/ZL)/(1/ZL + 1/ZC + I/Rout)
% 
% i_hat= i_hat_voltage_source + i_hat_current_source;
% Gid = simplifyFraction(i_hat / d_hat)                         TO
% EVALUATE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!





%Output Voltage Inductor Current Transfer Functions 


%Gid (Control to Inductor Current TF) calculation 



%Gvg (Line to output Voltage TF) Calculation 
% vout_hat =  (vg_hat/D_prime)*(Z_RC/(ZL+Z_RC));
% Gvg = simplifyFraction (vout_hat / vg_hat)
% 
% 
% 







% 
% Gvd = tf( [-Gdo/Wz Gdo], [ (1/Wo)^2  1/(Q*Wo)  1] )
% %controlSystemDesigner(Gvd)
% figure
% step(Gvd)
% figure
% rlocus(Gvd)
% figure
% bode(Gvd)

% Gvd = ( Gdo - (Gdo*s)/Wz) / (1+ s/(Q*Wo)+ (s/Wo)^2 )
% Gid = simplifyFraction(Gvd /Rout)
% 
% 
% Gvg = ( Ggo) / (1+ s/(Q*Wo)+ (s/Wo)^2 )
% Gig = simplifyFraction(Gvg /Rout)



% %bode(Gvd, {2*pi ,30e3*2*pi} )
% figure;
% nyquist(Gvd)











