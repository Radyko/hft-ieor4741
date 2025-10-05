### Observation: 

CRTP ~ free and virtual is a few percent slower

CRTP allows for inlining while virtual introduces an inderect branch and blocks inlining. 

We want to use CRTP when we know the types at compile times. This is really fitting for HFT systems.

### Results


Generating 10000000 ticks, iters=1...

free_function       time: 18.621 ms  sink=0.000000

virtual_call        time: 18.994 ms  sink=0.000000

crtp_call           time: 18.709 ms  sink=0.000000

### Summary

free_function       ns/tick: 1.862  ticks/sec: 537.03 M

virtual_call        ns/tick: 1.899  ticks/sec: 526.49 M

crtp_call           ns/tick: 1.871  ticks/sec: 534.50 M
