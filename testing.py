from redpitaya.overlay.mercury import mercury as FPGA
import IPython.display as ipd
import time
import math
import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
import random
import gc

def initialise(sampleRate):
    print('Beginning Initialisation...')
    fpga = FPGA()

    # Initialisation of inputs and outputs (oscilliscopes and generators)
    out1 = fpga.gen(0) # Output to X-Axis K-Cube
    out2 = fpga.gen(1) # Output to Y-Axis K-Cube
    in1 = fpga.osc(0, 1.0) # Instance of oscilliscope object for the input
    # in2 = fpga.osc(1, 1.0) # Unused input

    # Initialise output as sinusoid with no frequency or amplitude, as the output will be
    # controlled by manipulating the offset as the program runs, this initialisation is
    # the same for both outputs
    out1.waveform = out1.sin()
    out1.frequency = 0
    out1.amplitude = 0.0
    out1.offset = 0
    out1.enable = True

    out2.waveform = out2.sin()
    out2.frequency = 0
    out2.amplitude = 0.0
    out2.offset = 0
    out2.enable = True

    in1.decimation = int(125000000/sampleRate) # 125Msps / 125Ksps = 1000
    in1.enable = True
    # dir(in1)

    # out2.sync_src = fpga.sync_src['gen0']
    print('System Initialised :)')
    #print(fpga.overlay)

    return in1, out1, out2

def demo(waitTime):
    print('Beginning Initialisation...')
    fpga = FPGA()

    # Initialisation of inputs and outputs (oscilliscopes and generators)
    out1 = fpga.gen(0) # Output to X-Axis K-Cube
    out2 = fpga.gen(1) # Output to Y-Axis K-Cube
    in1 = fpga.osc(0, 1.0) # Instance of oscilliscope object for the input
    # in2 = fpga.osc(1, 1.0) # Unused input

    # Initialise output as sinusoid with no frequency or amplitude, as the output will be
    # controlled by manipulating the offset as the program runs, this initialisation is
    # the same for both outputs
    out1.waveform = out1.sin()
    out1.frequency = 0.2
    out1.amplitude = 1.0
    out1.offset = 0
    out1.enable = True

    out2.waveform = out2.sin()
    out2.frequency = 0.2
    out2.amplitude = 1.0
    out2.offset = 0
    out2.enable = True

    in1.decimation = 65536 # 125Msps / 125Ksps = 1000
    in1.enable = True
    
    in1.start()
    out1.start()
    out2.start()
    out1.trigger()
    out2.trigger()
    
    time.sleep(waitTime)
    
    in1.stop()
    out1.stop()
    out2.stop()
    print('System Initialised :)')
    #print(fpga.overlay)
    
    return in1.data()

def run(sampleRate, runTime, size, climb):
    """Runs system for set period of time

    Parameters:
    sampleRate (int): Number of samples per second (to pass onto initialise)
    runTime (int): Number of seconds to run for
    size (int): Size of output array
    climb (bool): Boolean to determine whether or not to use the climbing algorithm
    
    Returns:
    output (np.array): Output of last (size) values read from input
    """
    in1, out1, out2 = initialise(sampleRate)

    in1.start()
    out1.start()
    out2.start()
    out1.trigger()
    out2.trigger()

    output = np.zeros(size)
    inputx = np.zeros(size)
    inputy = np.zeros(size)
    
    x_val = 0
    y_val = 0
    out1.offset = x_val
    out2.offset = y_val
    time.sleep(0.5)
    
    count = 1
    correct = 1
    
    perIteration = 0.005
    
    print('Running')
    start = time.perf_counter()
    instanceStart = time.perf_counter()
    while((time.perf_counter() - start) <= runTime):
        #x_val = math.sin(time.perf_counter()*math.pi)
        #y_val += 0.002
        direction = 0
        valueX = 0
        valueY = 0
        if climb:
            direction = random.random()*2*math.pi
            valueX = math.cos(direction)*0.005
            valueY = math.sin(direction)*0.005
        
        x_val += valueX
        y_val += valueY
        
        if (x_val > 1):
            x_val = 1
        if (x_val < -1):
            x_val = -1
        if (y_val > 1):
            y_val = 1
        if (y_val < -1):
            y_val = -1
        
        out1.offset = x_val
        out2.offset = y_val
        
        while ((time.perf_counter() - instanceStart) < perIteration): pass
        # print(time.perf_counter() - instanceStart)
        # NOTE: Without the append, the loop takes 0.37s for 10,000 iterations, which are outputted correctly
        #       this means that the maximum frequency of the system without maths is ~27,000Hz
        instanceStart = time.perf_counter()

        output[count%size] = np.average(in1.data(int(sampleRate/maxRate*perIteration*100000000)))
        inputx[count%size] = x_val
        inputy[count%size] = y_val

        if (output[count%size] < output[(count-1)%size]):
            x_val -= 2*valueX
            y_val -= 2*valueY
            correct += 1
        count += 1

    end = time.perf_counter() - start

    print("Time taken: {:.2f}s, done".format(end))
    print("Count: {}".format(count))
    print("Corrections: {:2f}%".format((correct*100)/count))
    print("System Frequency: {:.2f}Hz".format(count/(end)))
    
    in1.stop()    
    out1.stop()
    out2.stop()
    
    return output, inputx, inputy

frequency = 200
seconds = 20
maxRate = 125000000
sampleRate = 1250000

output, inputx, inputy = run(sampleRate, seconds, seconds*frequency, True)
output2, inputx2, inputy2 = run(sampleRate, seconds, seconds*frequency, False)

fig,ax=plt.subplots(nrows=3,ncols=1,sharex=True, figsize=(5,8))
t=np.array(range(0,len(output)))/200
ax[0].plot(t, inputx)
ax[0].plot(t, inputy)
ax[0].plot(t, output)
ax[0].set_ylim(-1, 1)
ax[0].set_title('Climb')
ax[0].set_ylabel('Voltage (V)')
ax[0].legend(['X','Y','Out'])
#plt.xlim(50,60)

ax[1].plot(t, inputx2)
ax[1].plot(t, inputy2)
ax[1].plot(t, output2)
ax[1].set_ylim(-1, 1)
ax[1].set_title('No Climb')
ax[1].set_ylabel('Voltage (V)')
ax[1].legend(['X','Y','Out'])

ax[2].plot(t, output)
ax[2].plot(t, output2)
ax[2].set_ylim(0.0, 0.5)
ax[2].set_title('Comparison')
ax[2].set_ylabel('Voltage (V)')
ax[2].set_xlabel('Time (s))')
ax[2].legend(['Climb', 'No Climb'])
#plt.xlim(50,60)

print('Climb Avg: {:2f}V'.format(np.average(output)))
print('No Climb Avg: {:2f}V'.format(np.average(output2)))

plt.tight_layout()
plt.show()
