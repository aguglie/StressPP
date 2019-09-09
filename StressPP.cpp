//
// Created by andrea on 24/05/19.
//

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>
#include <zconf.h>
#include "StressPP.h"

//On destruction, stop every running thread.
StressPP::~StressPP() {
    stop();
}

StressPP::StressPP() :
//If no value default to number of cpu threads.
        StressPP(std::thread::hardware_concurrency()) {
}

StressPP::StressPP(unsigned int cpuThreads) :
        _stressCpuThreads(cpuThreads) {
}

std::vector<StressCpuThread> *StressPP::getStressCpuThreads() {
    return &_stressCpuThreads;
}

//Stop and join every started thread.
void StressPP::stop() {
    _stressCpuThreads.clear(); // Stop StressCPU Threads
    stopSyncStress();
    stopMallocStress();
    stopWriteStress();
}

void StressPP::stopSyncStress() {
    if (_stressingSync) {
        _stressingSync = false;
        for (auto &t : _syncThreads) {
            t.join();
        }
    }
}

void StressPP::stopMallocStress() {
    if (_stressingMalloc) {
        _stressingMalloc = false;
        for (auto &t : _mallocThreads) {
            t.join();
        }
    }
}

void StressPP::stopWriteStress() {
    if (_stressingWrite) {
        _stressingWrite = false;
        for (auto &t : _writeThreads) {
            t.join();
        }
    }
}


void StressPP::syncStress(int howManyThreads) {
    _stressingSync = true;
    for (int i = 0; i < howManyThreads; i++)
        _syncThreads.push_back(std::thread([this]() {
            while (_stressingSync)
                sync();
        }));
    std::cout << "Created " << howManyThreads << " new threads stressing I/O with Sync" << "\n";
}

void StressPP::mallocStress(int howManyThreads, int vmChunks, long long vmBytes) {
    _stressingMalloc = true;
    for (int i = 0; i < howManyThreads; i++)
        _mallocThreads.push_back(std::thread([this, vmChunks, vmBytes]() {
            long long j, k;
            int retval = 0;
            char **ptr;

            while (_stressingMalloc) {
                ptr = (char **) malloc(vmChunks * sizeof(int));
                for (j = 0; vmChunks == 0 || j < vmChunks; j++) {
                    if ((ptr[j] = (char *) malloc(vmBytes * sizeof(char)))) {
                        for (k = 0; k < vmBytes; k++)
                            ptr[j][k] = 'Z';    /* Ensure that COW happens.  */
                    } else {
                        ++retval;
                        std::cerr << "malloc failed, waiting then retrying..." << "\n";
                        usleep(10000);
                        continue;
                    }
                }
                if (retval == 0) {
                    for (j = 0; vmChunks == 0 || j < vmChunks; j++) {
                        free(ptr[j]);
                    }
                    free(ptr);
                    continue;
                }
            }
        }));
    std::cout << "Created " << howManyThreads << " new threads stressing Virtual Memory malloc" << "\n";
}


void StressPP::writeStress(int howManyThreads, bool clean, long long files, long long bytes) {
    _stressingWrite = true;
    for (int i = 0; i < howManyThreads; i++)
        _writeThreads.push_back(std::thread([this, clean, files, bytes]() {
            long long i, j;
            int fd;
            int chunk = (1024 * 1024) - 1;    /* Minimize slow writing.  */
            char buff[chunk];

            for (i = 0; i < chunk - 1; i++) {
                j = rand();
                j = (j < 0) ? -j : j;
                j %= 95;
                j += 32;
                buff[i] = j;
            }
            buff[i] = '\n';

            while (_stressingWrite) {
                for (i = 0; i < files; i++) {
                    char name[] = "/tmp/stress.XXXXXX";

                    if ((fd = mkstemp(name)) < 0) {
                        perror("mkstemp");
                        std::cerr << "mkstemp failed" << "\n";
                        break;
                    }

                    for (j = 0;
                         bytes == 0 || j + chunk < bytes;
                         j += chunk) {
                        if (write(fd, buff, chunk) !=
                            chunk) {
                            std::cerr << "fast write failed\n";
                            break;
                        }
                    }

                    for (; bytes == 0 || j < bytes - 1; j++) {
                        if (write(fd, "Z", 1) != 1) {
                            std::cerr << "slow write failed\n";
                            break;
                        }
                    }
                    if (write(fd, "\n", 1) != 1) {
                        std::cerr << "write failed\n";
                        break;
                    }
                    ++j;

                    close(fd);

                    if (clean) {
                        if (unlink(name)) {
                            std::cerr << "unlink failed\n";
                            break;
                        }
                    }
                }
            }
        }));
    std::cout << "Created " << howManyThreads << " new threads stressing Disk with writes" << "\n";
}