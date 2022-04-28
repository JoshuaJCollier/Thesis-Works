print('Starting...')

from redpitaya.overlay.mercury import mercury as FPGA
import IPython.display as ipd
import time
import math
import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import random

fpga = FPGA()

out1 = fpga.gen(0)
out2 = fpga.gen(1)
in1 = fpga.osc(0, 1.0) #Instance of oscilliscope object for the input
in2 = fpga.osc(1, 1.0) 
#First arguement is the channel index, either 0 or 1 (this is In1=0, In2=1)
#Second argument is the voltage range, either 1.0 (default) or 20.0

print(type(out1))
# buffer waveform and sample timing
out1.waveform      = out1.sin()
out1.frequency = 0

# set output amplitude, offset and enable it
out1.amplitude     = 0.0

# buffer waveform and sample timing
out2.waveform      = out1.sin()
out2.frequency = 0

# set output amplitude, offset and enable it
out2.amplitude     = 0.0

in1.decimation = int(65536/2) # 125Msps / 65536 = 125Ksps

out2.sync_src = fpga.sync_src['gen0']

out1.offset = 0
out2.offset = 0
out1.enable        = True
out2.enable        = True

finalArray = np.array([0])
start = time.perf_counter()
out1.start()
out2.start()
out1.trigger()
output = []

for i in range(100):
    out1.offset        = 0.02*i-1
    #out2.offset        = 0.02*i-1

    
    output.append(np.average(in1.data(60)))
    
    time.sleep(0.004)
    # NOTE: Without the append, the loop takes 0.37s for 10,000 iterations, which are outputted correctly
    #       this means that the maximum frequency of the system without maths is ~27,000Hz
    
#print(help(in1.data(60)))
end = time.perf_counter() - start
print("Time taken: {:.2f}s, done".format(end))
out1.enable = False
out2.enable = False
