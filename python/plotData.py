from gradientAscent import *
from redpitaya.overlay.mercury import mercury as FPGA
from mpl_toolkits.mplot3d import Axes3D
from scipy.stats import multivariate_normal
import matplotlib.pyplot as plt
import numpy as np
import math

seconds = 5
sampleRate = 125000000
stepSize = 0.025
perIteration = 0.0043
frequency = 1/perIteration

outArray = []
outNoClimbArray = []

for i in range(10):
    output, inputx, inputy = run(sampleRate, seconds, int((seconds)*frequency), True, stepSize, perIteration)
    outArray.append(output)    
    print('Climb Avg: {:2f}V'.format(np.average(output)))

for i in range(1):
    outputNoClimb, inputxNoClimb, inputyNoClimb = run(sampleRate, seconds, int((seconds)*frequency), False, stepSize, perIteration)
    outNoClimbArray.append(outputNoClimb)
    print('No Climb Avg: {:2f}V'.format(np.average(outputNoClimb)))
    
def plotTypeOne():
    fig,ax=plt.subplots(nrows=3,ncols=1,sharex=True, figsize=(5,8))
    t=np.array(range(0,len(output)))/frequency
    ax[0].plot(t, inputx, color="blue")
    ax[0].plot(t, inputy, color="green")
    ax[0].plot(t, output, color="red")
    ax[0].set_ylim(-1.2, 1.2)
    ax[0].set_title('Climb')
    ax[0].set_ylabel('Voltage (V)')
    ax[0].legend(['X','Y','Out'], loc='lower right')

    ax[1].plot(t, inputxNoClimb, color="blue")
    ax[1].plot(t, inputyNoClimb, color="green")
    ax[1].plot(t, outputNoClimb, color="orange")
    ax[1].set_ylim(-1.2, 1.2)
    ax[1].set_title('No Climb')
    ax[1].set_ylabel('Voltage (V)')
    ax[1].legend(['X','Y','Out'], loc='lower right')

    ax[2].plot(t, output, color="red")
    ax[2].plot(t, outputNoClimb, color="orange")
    ax[2].set_ylim(0, 1.1)
    ax[2].set_title('Comparison')
    ax[2].set_ylabel('Voltage (V)')
    ax[2].set_xlabel('Time (s))')
    ax[2].legend(['Climb', 'No Climb'], loc='lower right')

    plt.tight_layout()
    plt.show()

def plotTypeTwo():
    t=np.array(range(0,len(outArray[0])))/frequency
    for i in range(5):
        plt.plot(t, outArray[i])

    plt.ylim(0, 1.2)
    plt.title('Climb')
    plt.ylabel('Voltage (V)')
    plt.xlabel('Time (s))')
    plt.legend(['Out0','Out1','Out2', 'Out3','Out4'], loc='lower right')

    plt.tight_layout()
    plt.show()

finalAvg = []
for i in range(10):
    finalAvg.append(np.average(outArray[i]))
    print('Climb Avg [{}]: {:2f}V'.format(i, np.average(outArray[i])))
print()
print('Climb Tot Avg: {:2f}V'.format(np.average(finalAvg)))
    
for i in range(1):
    print('Non Climb Avg: {:2f}V'.format(np.average(outNoClimbArray[i])))

def plot3D():
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d',alpha=0.1)

    x, y = np.mgrid[-1.0:1.0:30j, -1.0:1.0:30j]
    xy = np.column_stack([x.flat, y.flat])

    mu = np.array([0.0, 0.0])
    sigma = np.array([0.2, 0.2])
    covariance = np.diag(sigma**2)

    z = multivariate_normal.pdf(xy, mean=mu, cov=covariance)
    z = z.reshape(x.shape)*1/np.max(z)*sum(output[int(len(output)/2):])/(len(output)/2)
    x = x + sum(inputx[int(len(inputx)/2):])/(len(inputx)/2)
    y = y + sum(inputy[int(len(inputy)/2):])/(len(inputy)/2)
    ax.plot_wireframe(x,y,z,color='grey')
    length = len(inputx)

    # plotting directly
    ax.plot(inputx[0:length-2], inputy[0:length-2], output[0:length-2], color='red')

    # plotting via scatter
    # ax.scatter(inputx, inputy, output, s=1)

    # plotting via quivers
    '''
    for i in range(length-1):
        if (output[i] < sum(output)/length):
            soa = np.array([[inputx[i], inputy[i], output[i], inputx[i+1]-inputx[i], inputy[i+1]-inputy[i], output[i+1]-output[i]]])
            dist = math.sqrt((inputx[i+1]-inputx[i])**2 + (inputy[i+1]-inputy[i])**2 + (output[i+1]-output[i])**2)
            
            X, Y, Z, U, V, W = zip(*soa)
            ax.quiver(X, Y, Z, U, V, W,color='r',length=dist)
    '''

    plt.show()
