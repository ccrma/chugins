"rave_chafe_data_rt.ts" => string model;
"forward" => string method;

Rave r1, r2, r3;
(model, method) => r1.init;


model => r2.model => r3.model;
method => r2.method => r3.method;

<<< r1.model(), r1.method() >>>;
<<< r2.model(), r2.method() >>>;
<<< r3.model(), r3.method() >>>;
