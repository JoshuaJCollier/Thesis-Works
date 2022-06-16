// Imports from standard functions
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "redpitaya/rp.h"

// Import for Red Pitaya
#include "gradientAscentC.h"

static const int OSC_BASE_ADDR = 0x40100000;
static const int OSC_BASE_SIZE = 0x30000;
static const int OSC_CHA_OFFSET = 0x10000;

// Save data
int save(int size, int indexNum, float* inx, float* iny, float* out) {
    char fileName[64];
    sprintf(fileName, "/root/RedPitaya/joshThesis/saves/saveData%i.csv", indexNum);
    FILE *file = fopen(fileName, "w");

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

    // Initialise Buffer Stuff
    //int fd = open("/dev/mem", O_RDWR | O_SYNC);
    //uint32_t* virtual_address = mmap(NULL, 65535, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x40100000);
    //uint32_t* buff = (uint32_t*)(virtual_address + OSC_CHA_OFFSET);
    //float* buff = (float*)(mmap(NULL, OSC_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OSC_BASE_ADDR + OSC_CHA_OFFSET);

    // Start
    rp_GenOutEnable(RP_CH_1);
    rp_GenOutEnable(RP_CH_2);
    rp_AcqStart();

    uint32_t buff_array[4] = {1028, 2048, 4096, 8192};
    float frequencies[4] = {2000, 1000, 500, 250};

    //for (int j = 0; j < in.runNum; j++) {
    int j = 0;

    float frequency = frequencies[2];
    uint32_t buff_size = buff_array[2]; //16384;
    float *buff = (float *)malloc(buff_size * sizeof(float));
    in.size = frequency*in.runTime*0.998;
    sleep(0.5);

    float inputx[in.size];
    float inputy[in.size];
    float output[in.size];
    float x_change = 0;
    float y_change = 0;
    float last_out = 0;
    float x_val = 0;
    float y_val = 0;
    float curr_out = 0;
    float sum = 0;
    clock_t start, end, instance_time;
    int pos = 0;
    int count = 0;
    start = clock();
    
    while (((double) (clock() - start)) / CLOCKS_PER_SEC < in.runTime) {
        last_out = curr_out; // Define last value
        pos = count % in.size;
        
        x_change = 0;//(2.0*((float)rand()/(float)RAND_MAX)-1.0)*in.stepSize;
        y_change = 0;//(2.0*((float)rand()/(float)RAND_MAX)-1.0)*in.stepSize;

        x_val += x_change;
        y_val += y_change;

        if (x_val > 1.0) {
            x_val = 1.0;
        } 
        if (x_val < -1.0) {
            x_val = -1.0;
        } 
        if (y_val > 1.0) {
            y_val = 1.0;
        } 
        if (y_val < -1.0) {
            y_val = -1.0;
        }

        // Set generator values
        rp_GenOffset(RP_CH_1, x_val);
        rp_GenOffset(RP_CH_2, y_val);

        // Wait for values from photodetector
        while ((((double) (clock() - instance_time)) / (double)CLOCKS_PER_SEC) < (1.0/frequency)) {continue;}
        instance_time = clock();

        // Get current photodetector value
        rp_AcqGetLatestDataV(RP_CH_1, &buff_size, buff);
        sum = 0.0;
        for (int i = 0; i < buff_size; ++i) {
            sum += buff[i];
        }
        
        curr_out = sum/buff_size;
        inputx[pos] = x_val;
        inputy[pos] = y_val;
        output[pos] = curr_out;

        if ((curr_out + in.stepSize/20) < last_out) {
            x_val -= 2*x_change;
            y_val -= 2*y_change;
        } else if (curr_out < last_out) {
            x_val -= x_change;
            y_val -= y_change;
        }   

        count += 1;
    }

    end = ((double) (clock() - start)) / CLOCKS_PER_SEC;

    printf("Run: %i, done\n", j);
    printf("Time taken: %lis\n", end);
    printf("Count: %i\n", count);
    printf("Output: %f\n", curr_out);
    printf("X Val: %f\n", x_val);
    printf("Y Val: %f\n", y_val);
    printf("\n");

    save(in.size, j, inputx, inputy, output);
    free(buff);
    sleep(1);
    //}
    
    rp_AcqStop();
    //free(buff);
    rp_Release();
    return 0;
}

int main(int argc, char *argv[]) {
    float frequency = 500.0;
    float time = 10.0;
    float stepSize = 0.025*250/frequency;
    int sampleFrequency = 125000000;
    int count = frequency*time*0.998;
    int iterations = 1;
    bool change = false;
    struct run_in in = {sampleFrequency, count, iterations, change, stepSize, time, frequency};
    run(in);

    return 0;
}

