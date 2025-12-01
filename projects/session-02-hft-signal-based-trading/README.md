### 1.) Which signal triggered the most orders?

#### To answer this, I made a counter variable for each of the signals as a private member variable. Then, whenever the singal was true and changed the buy from false to true, I incremented the counter. After the full simulation, it looks to be the casethat signal 2 (Deviation from average) signal caused the most trades.

### 2.) What could I optimize farther?

#### I could improve the tick to trade latency, though most of the data structures used were close to optimal.

### 3.) How would the code behave with 10x the data?

#### I ran the simulation with 10x the data, and it seemed to me that there was still no bottleneck yet. The runtime went from 118 to 1171 ms, which is practically a linear increase when 10x the data. 

