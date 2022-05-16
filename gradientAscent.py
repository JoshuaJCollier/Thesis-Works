from redpitaya.overlay.mercury import mercury as FPGA
#from numba import jit
import time
import math
import numpy as np
import random

def initialise(sampleRate):
    """Initialise the generators and the oscilliscope

    Parameters:
    sampleRate (int): Number of samples per second
    
    Returns:
    in1: Oscilliscope 1 object
    out1: Generator 1 object
    out2: Generator 2 object
    """
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
     
    maxRate = 125000000
    in1.decimation = int(maxRate/sampleRate) # 125Msps / 125Ksps = 1000
    in1.enable = True
    # dir(in1)

    # out2.sync_src = fpga.sync_src['gen0']
    print('System Initialised :)')
    #print(fpga.overlay)

    return in1, out1, out2

def demo(waitTime):
    """Demonstration function to test outputs, generate slow sinusoid 

    Parameters:
    waitTime (int): Number of seconds to run demonstration
    
    Returns:
    in1.data() (float[]): Output of oscilliscope
    """
    print('Beginning Demonstration...')
    fpga = FPGA()

    # Initialisation of inputs and outputs (oscilliscopes and generators)
    out1 = fpga.gen(0) # Output to X-Axis K-Cube
    out2 = fpga.gen(1) # Output to Y-Axis K-Cube
    in1 = fpga.osc(0, 1.0) # Instance of oscilliscope object for the input
    # in2 = fpga.osc(1, 1.0) # Unused input

    # Output 1 and 2 output as sinusoids with 0.2Hz 1V amplitude
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

    # Setting input
    in1.decimation = 65536 
    in1.enable = True
    
    # Start
    in1.start()
    out1.start()
    out2.start()
    out1.trigger()
    out2.trigger()
    
    time.sleep(waitTime)
    
    in1.stop()
    out1.stop()
    out2.stop()
    print('Demonstration finished.')
    
    return in1.data()

#@jit(nopython=True)
def run(sampleRate, runTime, size, climb, stepSize, perIteration):
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
    maxRate = 125000000
    halfStep = stepSize/2
    fullStep = stepSize

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
        
    print('Running')
    start = time.perf_counter()
    instanceStart = time.perf_counter()
    
    #avgLength = int(sampleRate/maxRate*perIteration*1000000) # int(perIteration/(0.000131*sampleRate/maxRate))
    #print('AvgLen: {}'.format(avgLength))
    currOut = 0
    
    while((time.perf_counter() - start) <= runTime):
        direction = 0
        valueX = 0
        valueY = 0
        lastOut = currOut
        currentPos = count%size
        
        if climb:
            valueX = (2*random.random()-1)*stepSize
            valueY = (2*random.random()-1)*stepSize
        
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

        inputx[currentPos] = x_val
        inputy[currentPos] = y_val
        currOut = np.average(in1.data())
        
        #print(in1.pointer)

        output[currentPos] = currOut
        
        #print(output[count%size] - output[(count-1)%size])
        if ((currOut+0.005) < lastOut):
            x_val -= 2*valueX
            y_val -= 2*valueY
        elif currOut < lastOut:
            x_val -= valueX
            y_val -= valueY
           
        '''
        if currentPos > 20:
            recentAvg = np.average(output[currentPos-20:currentPos])
            if ((currOut > (recentAvg - 0.005)) and (currOut < (recentAvg + 0.005))):
               stepSize = halfStep
            else:
               stepSize = fullStep
        '''
        count += 1

    end = time.perf_counter() - start

    print("Time taken: {:.2f}s, done".format(end))
    print("Count: {}".format(count))
    # print("System Frequency: {:.2f}Hz".format(count/(end)))
    in1.stop()    
    out1.stop()
    out2.stop()
    
    return output, inputx, inputy
