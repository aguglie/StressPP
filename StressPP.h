//
// Created by andrea on 24/05/19.
//

#ifndef UNTITLED_STRESSPP_H
#define UNTITLED_STRESSPP_H

#include <iostream>
#include "StressCpuThread.h"

using namespace std;

class StressPP {
private:
    std::vector<StressCpuThread> _stressCpuThreads;

    std::vector<std::thread> _syncThreads;
    std::vector<std::thread> _mallocThreads;
    std::vector<std::thread> _writeThreads;

    volatile bool _stressingSync = false;
    volatile bool _stressingMalloc = false;
    volatile bool _stressingWrite = false;

public:
    StressPP();

    StressPP(unsigned int cpuThreads);

    std::vector<StressCpuThread> *getStressCpuThreads();

    void syncStress(int howManyThreads);

    void stopSyncStress();

    void mallocStress(int howManyThreads, int vmChunks, long long vmBytes);

    void stopMallocStress();

    void writeStress(int howManyThreads, bool clean, long long files, long long bytes);

    void stopWriteStress();

    void stop();

    virtual ~StressPP();

};


#endif //UNTITLED_STRESSPP_H
