// Imports from standard functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>

// Import for Red Pitaya
#include "redpitaya/rp.h"
//#include "common.h"
//#include "oscilloscope.h"
//#include "rp_cross.h"
#include "gradientAscentC.h"

// Save data
int save(int size, float* inx, float* iny, float* out) {
    FILE * file;
    file = fopen("/root/RedPitaya/joshThesis/saveData.csv", "w");

    if(file == NULL) {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < size; i++) {
        fprintf(file, "%i,%f,%f,%f\n", i, inx[i], iny[i], out[i]);
    }

    /* Close file to save file data */
    fclose(file);

    return 0;
}

// Main run function
int run(struct run_in in) {
    printf("Running...\n");
    // Check API connection
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Rp api init failed!\n");
    }

    // Seed Random
    srand(time(NULL));

    // Reset Generator and Acquisition
    rp_GenReset();
    rp_AcqReset();

    // Initialise Generator 1 (for x axis)
    rp_GenFreq(RP_CH_1, 0.0);
    rp_GenAmp(RP_CH_1, 0.0);
    rp_GenWaveform(RP_CH_1, RP_WAVEFORM_SINE);
    rp_GenOffset(RP_CH_1, 0.0);
    rp_GenOutEnable(RP_CH_1);

    // Initialise Generator 2 (for y axis)
    rp_GenFreq(RP_CH_2, 0.0);
    rp_GenAmp(RP_CH_2, 0.0);
    rp_GenWaveform(RP_CH_2, RP_WAVEFORM_SINE);
    rp_GenOffset(RP_CH_2, 0.0);
    rp_GenOutEnable(RP_CH_2);

    // Initialise Acquisition (for photodetector)
    rp_AcqSetDecimation(RP_DEC_1);
    
    // Initialise Variables
    //int maxRate = 125000000;
    //double halfStep = in.stepSize/2;
    double fullStep = in.stepSize;

    float inputx[in.size];
    float inputy[in.size];
    float output[in.size];
    float x_val, y_val, x_change, y_change, last_out, curr_out, sum;
    clock_t start, end, instance_time;
    int pos, i, count;

    uint32_t buff_size = 2048; //16384;
    float *buff = (float *)malloc(buff_size * sizeof(float));

    sleep(0.5);

    // Start
    rp_GenOutEnable(RP_CH_1);
    rp_GenOutEnable(RP_CH_2);
    rp_AcqStart();
    start = clock();
    
    while (((double) (clock() - start)) / CLOCKS_PER_SEC < in.runTime) {
        last_out = curr_out; // Define last value
        pos = count % in.size;
        
        x_change = (2.0*((float)rand()/(float)RAND_MAX)-1.0)*fullStep;
        y_change = (2.0*((float)rand()/(float)RAND_MAX)-1.0)*fullStep;

        x_val += x_change;
        y_val += y_change;

        if (x_val > 1.0) {
            x_val = 1.0;
        } if (x_val < -1.0) {
            x_val = -1.0;
        } if (y_val > 1.0) {
            y_val = 1.0;
        } if (y_val < -1.0) {
            y_val = -1.0;
        }

        // Set generator values
        rp_GenOffset(RP_CH_1, x_val);
        rp_GenOffset(RP_CH_2, y_val);

        // Wait for values from photodetector
        while ((((double) (clock() - instance_time)) / (double)CLOCKS_PER_SEC) < (1.0/in.frequency)) {continue;}
        instance_time = clock();

        // Get current photodetector value
        rp_AcqGetLatestDataV(RP_CH_1, &buff_size, buff);
        //buff = (float*)((char*)mmap(NULL, OSC_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, 0, (OSC_BASE_ADDR >> 20) * sysconf(_SC_PAGESIZE)) + OSC_CHA_OFFSET);
        printf("%f", buff[0]);

        sum = 0.0;
        for (i = 0; i < buff_size; ++i) {
            sum += buff[i];
        }

        curr_out = sum/buff_size;
        inputx[pos] = x_val;
        inputy[pos] = y_val;
        output[pos] = curr_out;

        if ((curr_out + fullStep/10) < last_out) {
            x_val -= 2*x_change;
            y_val -= 2*y_change;
        } else {
            x_val -= x_change;
            y_val -= y_change;
        }

        count += 1;
    }

    end = ((double) (clock() - start)) / CLOCKS_PER_SEC;

    printf("Time taken: %lis, done\n", end);
    printf("Count: %i\n", count);
    printf("Output: %f\n", curr_out);
    printf("X Val: %f\n", x_val);
    printf("Y Val: %f\n", y_val);

    save(in.size, inputx, inputy, output);
    rp_AcqStop();
    free(buff);
    rp_Release();
    return 0;
}

int main(int argc, char *argv[]) {
    float frequency = 1000.0;
    struct run_in in = {125000000, 4987, true, 0.025*250/frequency, 5.0, frequency};
    run(in);
    return 0;
}
