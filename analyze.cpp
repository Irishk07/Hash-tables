#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>


typedef struct {
    const char* name;
    const char* filename;
    long int ticks[5];
    int n;
} Analyze_t;


int Comaparator(const void* a, const void* b) {
    assert(a);
    assert(b);

    long int num1 = *(const long int*)a;
    long int num2 = *(const long int*)b;
    return (num1 > num2) - (num1 < num2);
}

void Analyze(long int *arr, int n, float *avg, float *stddev) {
    assert(arr);
    assert(avg);
    assert(stddev);
    
    if (n <= 2) { 
        *avg = 0; 
        *stddev = 0; 
        return; 
    }

    long long int sum = 0;
    for (int i = 0; i < n; ++i)
        sum += arr[i];
    *avg = sum / n;

    float var = 0.0;
    for (int i = 0; i < n; i++) {
        float diff = (float)arr[i] - *avg;
        var += diff * diff;
    }

    *stddev = sqrtf(var / n);
}

int main() {
    Analyze_t tests[] = {
        {"base_O0", "times/base_O0.txt", {0}, 0},
        {"base_O3", "times/base_O3.txt", {0}, 0},
        {"crc32",   "times/crc32.txt",   {0}, 0},
        {"strlen",  "times/strlen.txt",  {0}, 0},
        {"strcmp",  "times/strcmp.txt",  {0}, 0},
        {"pgo",     "times/pgo.txt",     {0}, 0}
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; ++i) {
        FILE* file = fopen(tests[i].filename, "r");

        long long val = 0;
        while (tests[i].n < 3 && fscanf(file, "%lld", &val) == 1) {
            tests[i].ticks[tests[i].n] = (long int)val;
            tests[i].n++;
        }
        
        fclose(file);
    }

    printf("%-20s | %-15s | %-15s | %-10s\n", "Test Name", "Average", "StdDev", "RelError");
    printf("--------------------------------------------------------------------------\n");

    for (int i = 0; i < num_tests; ++i) {
        float avg = 0.0, 
              stddev = 0.0;
        Analyze(tests[i].ticks, tests[i].n, &avg, &stddev);
        
        float rel_err = (avg > 0) ? (stddev / avg * 100.0) : 0.0;

        printf("%-20s | %-15.2e | %-15.2e | %.2f%%\n", 
               tests[i].name, avg, stddev, rel_err);
    }

    return 0;
}