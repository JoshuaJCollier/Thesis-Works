from redpitaya.overlay.mercury import mercury as FPGA
import numpy as np
import matplotlib.pyplot as plt
from gradientAscent import *

frequency = 200
seconds = 20
sampleRate = 1250000

output, inputx, inputy = run(sampleRate, seconds, seconds*frequency, True)
outputNoClimb, inputxNoClimb, inputyNoClimb = run(sampleRate, seconds, seconds*frequency, False)

fig,ax=plt.subplots(nrows=3,ncols=1,sharex=True, figsize=(5,8))
t=np.array(range(0,len(output)))/200
ax[0].plot(t, inputx)
ax[0].plot(t, inputy)
ax[0].plot(t, output)
ax[0].set_ylim(-1, 1)
ax[0].set_title('Climb')
ax[0].set_ylabel('Voltage (V)')
ax[0].legend(['X','Y','Out'])

ax[1].plot(t, inputxNoClimb)
ax[1].plot(t, inputyNoClimb)
ax[1].plot(t, outputNoClimb)
ax[1].set_ylim(-1, 1)
ax[1].set_title('No Climb')
ax[1].set_ylabel('Voltage (V)')
ax[1].legend(['X','Y','Out'])

ax[2].plot(t, output)
ax[2].plot(t, outputNoClimb)
ax[2].set_ylim(0.0, 0.5)
ax[2].set_title('Comparison')
ax[2].set_ylabel('Voltage (V)')
ax[2].set_xlabel('Time (s))')
ax[2].legend(['Climb', 'No Climb'])

print('Climb Avg: {:2f}V'.format(np.average(output)))
print('No Climb Avg: {:2f}V'.format(np.average(outputNoClimb)))

plt.tight_layout()
plt.show()
