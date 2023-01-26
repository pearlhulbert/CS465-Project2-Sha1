#include <openssl/sha.h>
#include <stdio.h>
#include <sys/types.h>

u_int32_t shaWrapper(u_int64_t nonce, int bitSize);
void collision(u_int64_t nonce, int bitSize);

int main(int argc, char **argv) {
    
    if (strcmp(argv[1], "-p") == 0) {
        int bitSize = atoi(argv[2]);
        FILE* random = fopen("/dev/random", "r");
        u_int64_t initNonce;
        read(fileno(random), (unsigned char*)&initNonce, 8);
        // print our every number, then print out average, high and low
        // save each number of attempts 
        u_int32_t target = shaWrapper(initNonce, bitSize);
        u_int64_t newNonce = read(fileno(random), (unsigned char*)&newNonce, 8);
        double counter = 0;
        u_int32_t retVal = 0;
        int min = 50;
        int max = 0;
        double attempts[50];
        for (int i = 0; i < 50; ++i) {
            while (retVal != target) {
                retVal = shaWrapper(newNonce + counter, bitSize);
                if (counter > max) {
                    max = counter;
                }
                else if (counter < min) {
                    min = counter;
                }
                ++counter;
                attempts[i] = counter;
                if (target == retVal) {
                    printf("Found match on attempt: %d\n", counter);
                    break;
                }
            }
        }
        int numAttempts = 0;
        for (int i = 0; i < 50; ++i) {
            numAttempts += attempts[i];
        }
        double average = numAttempts / 50;
        printf("average number of attempts: %d", average);
    }
    else if (strcmp(argv[1], "-c") == 0) {
        int bitSize = atoi(argv[2]);
        int numPossible = 1 << bitSize;
        u_int32_t totalHashes[numPossible];
        FILE* random = fopen("/dev/random", "r");
        u_int64_t nonce;
        read(fileno(random), (unsigned char*)&nonce, 8);
        // print our every number, then print out average, high and low
        // save each number of attempts 
        int collision = 0;
        double counter = 0;
        for (int i = 0; i < 50; ++i) {
            u_int32_t retVal = shaWrapper(nonce + counter, bitSize);
            totalHashes[i] = retVal;
            while (1) {
                for (int j = 0; j < numPossible; ++j) {
                    if (retVal == totalHashes[j]) {
                        break;
                        collision = 1;
                    }
                }
                if (!collision) {
                    break;
                }
            }
            ++counter;
        }
        
        
    }

    return 0;
}

u_int32_t shaWrapper(u_int64_t nonce, int bitSize) {
    unsigned char ibuf[] = "compute sha1";
    unsigned char obuf[20];

    SHA1((char*)&nonce, bitSize, obuf);

    u_int32_t forMask = 0;
    for (int i = 0; i < 4; ++i) {
        forMask = forMask << 8;
        forMask += obuf[16 + i];
    }
    int mask = (1 << bitSize) - 1;
    forMask = forMask & mask;

    // int i;
    // for (i = 16; i < 20; i++) {
    //     printf("%02x ", obuf[i]);
    // }
    // printf("\n");

    return forMask;
}

