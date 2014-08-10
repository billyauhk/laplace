laplace
=======
This repository contains several short code which I am using and testing.
They all computes the Laplace limit occurs in the field ofbasic orbital dynamics.
At the meanwhile I am still evaluating the relative speed of them.

Legalese readers: No warranty, not fit for particular purpose...

Performance
===========
Tested on Fourtuna, initial value with 2 d.p.
Requested length 200 d.p.
For laplace_mpfr, argument 800 is used to match the 200 d.p.
laplace.dc	0m31.342s	0m31.105s	0m31.348s
laplace_mpfr	0m0.043s	0m0.036s	0m0.033s

Another equation is used which seems faster (by 3 times or so)
Tested on Fourtuna, initial value with 5 d.p.
Requested argument 10000, output should be 2500 d.p.
laplace_mpfr2	0m6.531s	0m6.555s	0m6.520s
laplace_mpfr3	0m6.539s	0m6.559s	0m6.505s
(The only difference between mpfr2 and mpfr3 is the interrupt handling.)

Planning to test on Fourtuna, initial value with 5 d.p.
Requested length 2495 d.p.
laplace_quad.bc	8m28.860s

