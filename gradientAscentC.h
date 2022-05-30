// Struct for input to run function
typedef struct run_in {
    int sampleRate;
    int size;
    bool climb;
    float stepSize;
    double runTime;
    double frequency;
} run_in;

int save(int size, float[] inx, float[] iny, float[] out);
int[] run(struct run_in in);