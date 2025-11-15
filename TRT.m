
syms D Ron D_prime Rout Vin Vd

A=[-D*Ron  -D_prime
   D_prime      -1/Rout]

B=[1 -D_prime
   0  0]                         

C=[1  0];

D=[0 0];

U=[Vin
   Vd]

%X= -inv(A)   %*B*U

Z = [1 2 -1
    -2 0 1
    1 -1 0]

transpose(Z)*det(Z)

inv(Z)