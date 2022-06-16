// Struct for input to run function
typedef struct run_in {
    int sampleRate;
    int size;
    bool climb; // Use the climbing algorithm
    bool climbGrad; // Use the gradient aspect in climbing algorithm
    bool climbSmart; // Use past output values for maths
    bool climbMulti; // Use for multiple moves per iteration
    float climbGradLearning;
    int climbMultiNum;
    float stepSize;
    double runTime;
    double frequency;
    uint32_t buffSize;
} run_in;

int initialise();
int save(struct run_in in, float* inx, float* iny, float* out);
void generateMove(float stepSize, float* x_val, float* y_val, float* x_change, float* y_change);
void correctOut(float* currOut, float lastOut, bool climbGrad, bool climbSmart, float stepSize, float* x_val, float* y_val, float x_change, float y_change);
int run(struct run_in in);
