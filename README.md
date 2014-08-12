laplace
=======
This repository contains several short code which I am using and testing.
They all computes the Laplace limit occurs in the field of basic orbital dynamics.
At the meanwhile I am still evaluating the relative speed of them.

Legalese readers: No warranty, not fit for particular purpose...

Performance
===========
Tested on Fourtuna, initial value with 2 d.p.
Requested length 200 d.p.
For laplace_mpfr, argument 800 is used to match the 200 d.p.
|laplace.dc|0m31.342s|0m31.105s|0m31.348s|
|laplace_mpfr|0m0.043s|0m0.036s|0m0.033s|

Tested on Fourtuna, initial value with 5 d.p.
Requested argument 10000, output should be 2500 d.p.
|laplace_mpfr|0m18.276s|0m18.069s|0m18.078s|
|laplace_mpfr2|0m6.531s|0m6.555s|0m6.520s|
|laplace_mpfr3|0m6.539s|0m6.559s|0m6.505s|
|laplace_mpfr4|0m0.028s|0m0.022s|0m0.021s|
|laplace_mpfr5|0m0.054s|0m0.062s|0m0.059s|
mpfr->mpfr2: Another equation is used which is faster
mpfr2->mpfr3: Interrupt handling is added and now it could save and restore from continue.dat
mpfr3->mpfr4: A end-order root finding method is used. Per iteration time is also shortened.
mpfr5: Trying to use a 4-th order root finding method, but seems no improvement in total runtime. Will do parallelization.

Tested on Fourtuna, initial value with 5 d.p.
Requested length 2495 d.p.
|laplace_2ndConv.bc|1m26.226s|13m29.706s|
|laplace_4thConv.bc|1m33.619s|17m35.945s|
