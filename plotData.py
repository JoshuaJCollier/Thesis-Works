from redpitaya.overlay.mercury import mercury as FPGA
#import numpy as np
import matplotlib.pyplot as plt
from gradientAscent import *

frequency = 222
seconds = 20
sampleRate = 1250000
stepSize = 0.025
perIteration = 0.0045
output, inputx, inputy = run(sampleRate, seconds, seconds*frequency, True, stepSize, perIteration)
outputNoClimb, inputxNoClimb, inputyNoClimb = run(sampleRate, seconds, seconds*frequency, False, stepSize, perIteration)

print('Climb Avg: {:2f}V'.format(np.average(output)))
print('No Climb Avg: {:2f}V'.format(np.average(outputNoClimb)))

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
ax[2].set_ylim(0.4, 1.2)
ax[2].set_title('Comparison')
ax[2].set_ylabel('Voltage (V)')
ax[2].set_xlabel('Time (s))')
ax[2].legend(['Climb', 'No Climb'], loc='lower right')

plt.tight_layout()
#plt.rcParams["figure.figsize"] = (20,10)
plt.show()