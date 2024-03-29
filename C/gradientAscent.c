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
#include "gradientAscent.h"

#define formatBool(b) ((b) ? "true" : "false")

static const uint32_t OSC_BASE_ADDR = 0x40100000;
static const int OSC_BASE_SIZE = 0x30000;
static const int OSC_CHA_OFFSET = 0x10000;

// Save data
int save(struct run_in in, int length, float* inx, float* iny, float* out) {
    printf("Saving...\n");
    char fileName[150];
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    float outAvg = 0;
    // make the name of the save file relative to datetime and in
    sprintf(fileName, "/root/RedPitaya/joshThesis/saves/saveData_Freq%iHz_Time%is_on_%02d_%02d_%02d_at_%02d_%02d.csv", (int)in.frequency, (int)in.runTime, tm.tm_mday, tm.tm_mon + 1,tm.tm_year + 1900, tm.tm_hour, tm.tm_min);
    FILE *file = fopen(fileName, "w");

    if(file == NULL) {
        printf("Unable to create file.\n");
        exit(EXIT_FAILURE);
    }
    
    fprintf(file, "Index,X Values,Y Values,Output\n");

    for (int i = 0; i < length; i++) {
        fprintf(file, "%f,%f,%f,%f\n", (float) i/in.frequency, inx[i], iny[i], out[i]);
        outAvg += out[i];
    }
    outAvg = outAvg/length;

    printf("Output Average: %f\n", outAvg);
    /* Close file to save file data */
    fclose(file);
    printf("Saved.\n");

    return 0;
}

int initialise() {
    printf("Initalising...\n");
    // Check API connection
    if(rp_Init() != RP_OK){
        fprintf(stderr, "Red Pitaya API init failed!\n");
    }

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
    
    // Start
    rp_GenOutEnable(RP_CH_1);
    rp_GenOutEnable(RP_CH_2);
    rp_AcqStart();
    printf("Initialised.\n");

    return 0;
}

void generateMove(float stepSize, float* x_val, float* y_val, float* x_change, float* y_change, bool climbGrad) {
    if (climbGrad) {
        float angle = M_PI*(2.0*((float)rand()/(float)RAND_MAX)-1.0);
        *x_change = cos(angle)*stepSize;
        *y_change = sin(angle)*stepSize;
    } else {
        *x_change = (2.0*((float)rand()/(float)RAND_MAX)-1.0)*stepSize;
        *y_change = (2.0*((float)rand()/(float)RAND_MAX)-1.0)*stepSize;
    }
    
    *x_val += *x_change;
    *y_val += *y_change;
    
    if (*x_val > 1.0) {
        *x_val = 1.0;
    } 
    if (*x_val < -1.0) {
        *x_val = -1.0;
    } 
    if (*y_val > 1.0) {
        *y_val = 1.0;
    } 
    if (*y_val < -1.0) {
        *y_val = -1.0;
    }
}

void correctOut(float* currOut, float lastOut, struct run_in in, float* x_val, float* y_val, float x_change, float y_change) {
    if (in.climbGrad) {
        *x_val += x_change/in.stepSize*(*currOut-lastOut)*in.climbGradLearning;
        *y_val += y_change/in.stepSize*(*currOut-lastOut)*in.climbGradLearning;
    } else if (in.climbSmart) {
        // Take step back if we arent progressing forward,
        if (*currOut < lastOut) {
            *x_val -= x_change;
            *y_val -= y_change;
            *currOut = lastOut;
        }
    } else if (in.climbMulti) {
        // Make climbMultiNum moves.
        // ----------------- WORK IN PROGRESS -----------------
        *x_val += x_change;
        *y_val += y_change;
    } else {
        // Without gradient consideration, either make big or small step back
        if ((*currOut + in.stepSize/20) < lastOut) {
            *x_val -= 2*x_change;
            *y_val -= 2*y_change;
        } else if (*currOut < lastOut) {
            *x_val -= x_change;
            *y_val -= y_change;
        }
    }

    if (*x_val > 1.0) {
        *x_val = 1.0;
    } 
    if (*x_val < -1.0) {
        *x_val = -1.0;
    } 
    if (*y_val > 1.0) {
        *y_val = 1.0;
    } 
    if (*y_val < -1.0) {
        *y_val = -1.0;
    }
}

// Main run function
int run(struct run_in in, int nth) {
    srand(time(NULL));

    initialise();
    printf("Running...\n");

    // ----------------- WORK IN PROGRESS -----------------
    /* MMAP Buffer Attempt */
    // int fd = open("/dev/mem", O_RDWR | O_SYNC);
    // float* buff = (float*)(mmap(NULL, OSC_BASE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, OSC_BASE_ADDR + OSC_CHA_OFFSET));
    

    // Initialise Buffer
    uint32_t buff_size = in.buffSize; //16384 max
    float *buff = (float *)malloc(buff_size * sizeof(float));

    int finalCount = in.size/nth;
    in.size = finalCount;
    
    // Initialise Variables
    float inputx[in.size];
    float inputy[in.size];
    float output[in.size];
    float x_val = 0;
    float y_val = 0;
    float x_change = 0;
    float y_change = 0;
    float currOut = 0;
    float lastOut = 0;
    float sum = 0;

    // Position
    int pos = 0;
    int count = 0;

    // Timing
    clock_t start, end, instance_time;
    start = clock();

    // Run Loop
    while (((double) (clock() - start)) / CLOCKS_PER_SEC < in.runTime) {
        // Init for loop
        lastOut = currOut; // Save last value
        pos = (count/nth % in.size);
        sum = 0.0;
        
        if (in.climb) {
            generateMove(in.stepSize, &x_val, &y_val, &x_change, &y_change, in.climbGrad);
        }

        // Set generator values
        rp_GenOffset(RP_CH_1, x_val);
        rp_GenOffset(RP_CH_2, y_val);

        // Synchronise System (bring it in like with frequency set)
        while ((((double) (clock() - instance_time)) / (double)CLOCKS_PER_SEC) < (1.0/in.frequency)) {continue;}
        instance_time = clock();

        // Get current photodetector value
        rp_AcqGetLatestDataV(RP_CH_1, &buff_size, buff);
        for (int i = 0; i < buff_size; ++i) {
            sum += buff[i];
        }
        currOut = sum/buff_size;
        
        inputx[pos] = x_val;
        inputy[pos] = y_val;
        output[pos] = currOut;

        correctOut(&currOut, lastOut, in, &x_val, &y_val, x_change, y_change);
        
        count += 1;
    }

    end = ((double) (clock() - start)) / CLOCKS_PER_SEC;

    printf("\n");
    printf("Run Finished with freq: %.2fHz, runTime: %.2fs, climb: %s, gradAsc: %s, smart: %s done\n", in.frequency, in.runTime, formatBool(in.climb), formatBool(in.climbGrad), formatBool(in.climbSmart));
    printf("Time taken: %lis\n", end);
    printf("Count: %i\n", count);
    printf("Last Output: %04f\n", currOut);
    printf("Last X Value: %04f\n", x_val);
    printf("Last Y Value: %04f\n", y_val);
    printf("\n");

    finalCount = (int) count/nth;
    save(in, finalCount, inputx, inputy, output);
    rp_AcqStop();
    rp_Release();
    free(buff);
    return 0;
}

int main(int argc, char *argv[]) {
    // Variable Defaults
    float frequency = 500.0;
    float runTime = 10.0;
    uint32_t buffSize = 4096;
    bool climb = true; // Used to tell it to use a climbing algorithm (otherwise it is just a test)
    bool climbGrad = false; // Gradient Proportional Ascent
    bool climbSmart = false; // Consideration of back data
    bool climbMulti = false; // Make multiple moves before deciding where to end up
    float climbGradLearning = 0.5; // Learning factor for gradient proportional ascent
    int climbMultiNum = 3; //
    float stepSize = 0.025*250/frequency;
    int nth = 1;

    // Flags
    for (int i = 0; i < argc; i++) {
        printf("argv[%i]: %s\n", i, argv[i]);
        
        if (strcmp(argv[i],"-freq") == 0) {
            frequency = atof(argv[i+1]);
        } else if (strcmp(argv[i],"-time") == 0) {
            runTime = atof(argv[i+1]);
        } else if (strcmp(argv[i],"-step") == 0) {
            stepSize = atof(argv[i+1]);
        } else if (strcmp(argv[i],"-test") == 0) {
            climb = false;
        } else if (strcmp(argv[i],"-grad") == 0) {
            climbGrad = true;
            if (i < argc-1) {
                if (argv[i+1][0] == '-') {
                    climbGradLearning = atof(argv[i+1]);
                }
            }
        } else if (strcmp(argv[i],"-smart") == 0) {
            climbSmart = true;
        } else if (strcmp(argv[i],"-multi") == 0) {
            climbMulti = true;
            if (i < argc-1) {
                if (argv[i+1][0] == '-') {
                    climbMultiNum = atoi(argv[i+1]);
                }
            }
        } else if (strcmp(argv[i],"-buff") == 0) {
            buffSize = (uint32_t) atoi(argv[i+1]);
        }
    }

    int runReturnSize = frequency*runTime;
    printf("Run Return Size: %i\n", runReturnSize);
    int sampleFrequency = 125000000;
    struct run_in in = {sampleFrequency, runReturnSize, climb, climbGrad, climbSmart, climbMulti, climbGradLearning, climbMultiNum, stepSize, runTime, frequency, buffSize};
    run(in, nth);

    return 0;
}
