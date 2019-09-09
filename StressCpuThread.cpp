//
// Created by andrea on 17/08/19.
//

#include "StressCpuThread.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

static int counter = 0;

StressCpuThread::~StressCpuThread() {
    stop();
}

StressCpuThread::StressCpuThread() {
    resetAffinity();
    _myId = counter++;
}

void StressCpuThread::start() {
    if (!_running) {
        std::cout << "Starting StressCpuThread(), ID=" << _myId << "\n";
        _thread = std::thread([this] {
            volatile double d;
            while (_running) {
                d = sqrt(rand());
            }
        });
        _running = true;
    }
}

/**
 * Note: Resets affinity matrix to all zeros but does not apply it.
 */
void StressCpuThread::resetAffinity() {
    CPU_ZERO(&_cpuset);
}

/**
 * Adds affinity to a physical thread.
 * @param cpuId
 */
void StressCpuThread::addAffinity(unsigned int cpuId) {
    unsigned num_cpus = std::thread::hardware_concurrency();

    if (cpuId > num_cpus) {
        std::cerr << "Please choose a physical thread between 0 and " << (num_cpus - 1) << "\n";
        return;
    }

    if (!_running) start();

    CPU_SET(cpuId, &_cpuset);
    int rc = pthread_setaffinity_np(_thread.native_handle(),
                                    sizeof(cpu_set_t), &_cpuset);
    if (rc != 0) {
        std::cerr << "Error calling pthread_setaffinity_np: " << rc << "\n";
    } else {
        std::cout << "Setting affinity StressCpuThread() " << _myId << " to CPU " << cpuId << "\n";
    }
}

void StressCpuThread::stop() {
    if (_running) {
        std::cout << "Stopping StressCpuThread()" << "\n";
        _running = false;
        _thread.join();
    }
}




