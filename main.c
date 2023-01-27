#include <openssl/sha.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <math.h>

u_int32_t shaWrapper(u_int64_t nonce, int bitSize);
double calculateVariance(double*, double);

int main(int argc, char **argv) {
    
    if (strcmp(argv[1], "-p") == 0) {
        int bitSize = atoi(argv[2]);
        FILE* random = fopen("/dev/random", "r");
        u_int64_t initNonce;
        read(fileno(random), (unsigned char*)&initNonce, 8);
        // print our every number, then print out average, high and low
        // save each number of attempts 
        unsigned int counter = 0;
        int match = 0;
        u_int32_t currVal = 0;
        double min = 50;
        double max = 0;
        double attempts[50];
        u_int32_t target = 0;
        u_int64_t newNonce = read(fileno(random), (unsigned char*)&newNonce, 8);
        for (int i = 0; i < 50; ++i) {
            double attemptNum = 0;
            u_int32_t target = shaWrapper(initNonce + i, bitSize);
            currVal = shaWrapper(newNonce + i, bitSize);
            match = 0;
            while (!match) {
                currVal = shaWrapper(newNonce + counter, bitSize);
                if (target == currVal) {
                    printf("Found match on attempt: %f\n", attemptNum);
                    attempts[i] = attemptNum;
                    match = 1;
                }
                ++attemptNum;
                if (attemptNum > max) {
                    max = attemptNum;
                }
                else if (attemptNum < min) {
                    min = attemptNum;
                }
                ++counter;  
            }
        }
        int numAttempts = 0;
        for (int i = 0; i < 50; ++i) {
            numAttempts += attempts[i];
        }
        double expected = pow(2.0, (double)bitSize);
        double average = numAttempts / 50;
        double variance = calculateVariance(attempts, average);
        printf("average number of attempts: %f\n", average);
        printf("min attempts: %f\n", min);
        printf("max attempts: %f\n", max);
        printf("variance: %f\n", variance);
        FILE* fpt = fopen("preimageData.csv", "w+");
        fprintf(fpt, "bitSize, average, expected, min, max, variance");
        fprintf(fpt,"%d, %f, %d, %f, %f, %f\n", bitSize, average, expected, min, max, variance);
        close(fpt);
    }
    else if (strcmp(argv[1], "-c") == 0) {
        int bitSize = atoi(argv[2]);
        int numPossible = 1 << bitSize;
        FILE* random = fopen("/dev/random", "r");
        u_int64_t nonce;
        read(fileno(random), (unsigned char*)&nonce, 8);
        // print our every number, then print out average, high and low
        // save each number of attempts 
        int collision = 0;
        unsigned int counter = 0;
        double min = 50;
        double max = 0;
        double attempts[50];
        u_int32_t currVal = 0;
        for (int i = 0; i < 50; ++i) {
            int total = 0;
            currVal = shaWrapper(nonce + i, bitSize);
            //printf("init currVal: %d\n", currVal);
            u_int32_t totalHashes[numPossible];
            totalHashes[total] = currVal;
            double attemptNum = 0;
            collision = 0;
            bzero(totalHashes, numPossible);
            while (!collision) {
                ++counter;
                currVal = shaWrapper(nonce + counter, bitSize);
                //printf("currVal: %d\n", currVal);
                ++attemptNum;
                for (int j = 0; j < numPossible; ++j) {
                    if (currVal == totalHashes[j]) {
                        printf("collision on attempt: %f\n", attemptNum);
                        if (attemptNum > max) {
                            max = attemptNum;
                        }
                        else if (attemptNum < min) {
                            min = attemptNum;
                        }
                        attempts[i] = attemptNum;
                        collision = 1;
                        break;
                    }
                }
                totalHashes[total] = currVal;
                ++total;
            }
        }
        int numAttempts = 0;
        for (int i = 0; i < 50; ++i) {
            numAttempts += attempts[i];
        }
        double expected = pow(2.0, ((double)bitSize/2));
        double average = numAttempts / 50;
        double variance = calculateVariance(attempts, average);
        printf("average number of attempts: %f\n", average);
        printf("min attempts: %f\n", min);
        printf("max attempts: %f\n", max);
        printf("variance: %f\n", variance);
         FILE* fpt = fopen("collisionData.csv", "w+");
        fprintf(fpt, "bitSize, average, expected, min, max, variance");
        fprintf(fpt,"%d, %f, %d, %f, %f, %f\n", bitSize, average, expected, min, max, variance);
        close(fpt);
    }
    return 0;
}

u_int32_t shaWrapper(u_int64_t nonce, int bitSize) {
    unsigned char obuf[20];

    SHA1((char*)&nonce, bitSize, obuf);

    u_int32_t forMask = 0;
    for (int i = 0; i < 4; ++i) {
        forMask = forMask << 8;
        forMask += obuf[16 + i];
    }
    int mask = (1 << bitSize) - 1;
    forMask = forMask & mask;

    return forMask;
}

double calculateVariance(double* attempts, double average) {

    double difference[50];
    for (int i = 0; i < 50; ++i) {
        difference[i] = attempts[i] - average;
    }
    
    double squares[50];
    for (int i = 0; i < 50; ++i) {
        squares[i] = difference[i] * difference[i];
    }

    double sumOfSquares = 0;
    for (int i = 0; i < 50; ++i) {
        sumOfSquares += squares[i];
    }

    return sumOfSquares / 50;

}

