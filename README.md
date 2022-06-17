# Thesis-Works

C code compiled with make file, using either "make all" or "make gradientAscent"
To run code, ./gradientAscent -flags

# gradientAscent (1) - Personal Linux man page
## **Name**
**gradientAscent** - Control piezo electric actuators to move mirror in response to changing coupled power
  
## **Synopsis**
**gradientAscent** \[\-**test**] \[\-**smart**] \[\-**freq** _frequency_] \[\-**time** _run\_time_] \[\-**step** _step\_size_] \[\-**grad** _learning\_rate_] \[\-**multi** _step\_number_] \[\-**buff** _buffer\_length_] 

## **Description**
**gradientAscent** is a C program written for this thesis that is used to operate 

###### The options are as follows:
&nbsp;&nbsp;&nbsp;&nbsp;\-**test** \
&nbsp;&nbsp;&nbsp;&nbsp;Forces **gradientAscent** into test mode (no climbing)

&nbsp;&nbsp;&nbsp;&nbsp;\-**smart** \
&nbsp;&nbsp;&nbsp;&nbsp;Forces **gradientAscent** into smart mode which avoids over back-stepping and thus makes smarter local decisions

&nbsp;&nbsp;&nbsp;&nbsp;\-**freq** _frequency_ \
&nbsp;&nbsp;&nbsp;&nbsp;Sets the control frequency of the system to _frequency_ Hz. Without this flag the default value is 500 Hz.

&nbsp;&nbsp;&nbsp;&nbsp;\-**time** _run\_time_ \
&nbsp;&nbsp;&nbsp;&nbsp;Runs a test for _run\_time_ s and will save data after this time. Without this flag the default value is 10s.

&nbsp;&nbsp;&nbsp;&nbsp;\-**step** _step\_size_ \
&nbsp;&nbsp;&nbsp;&nbsp;The size of each step taken is set to _step\_size_ V. Without this flag the default is a function of frequency.

&nbsp;&nbsp;&nbsp;&nbsp;\-**grad** _learning\_rate_ \
&nbsp;&nbsp;&nbsp;&nbsp;Applies the gradient adaptive algorithm. The _learning\_rate_ is a multiplier to the step size of the secondary step used to optimise movement towards goal. This flag does not need a _learning\_rate_, if none are used the default of 0.5 is used.

&nbsp;&nbsp;&nbsp;&nbsp;\-**multi** _step\_number_ \
&nbsp;&nbsp;&nbsp;&nbsp;Applies the multi step algorithm. The _step\_number_ is used to determine how many pseudo steps are taken in order to calculate the optimal move. This flag does not need a _step\_number_, if none are used the default of 3 is used.

&nbsp;&nbsp;&nbsp;&nbsp;\-**buff** _buffer\_length_ \
&nbsp;&nbsp;&nbsp;&nbsp;Sets the buffer size that is taken in for each iteration, set by _buffer\_length_. Without this flag the default value is 2048.
