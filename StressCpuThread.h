//
// Created by andrea on 17/08/19.
//

#ifndef UNTITLED_STRESSCPUTHREAD_H
#define UNTITLED_STRESSCPUTHREAD_H


#include <thread>
#include <vector>
#include <mutex>


class StressCpuThread {
public:
    virtual ~StressCpuThread();

private:
    volatile bool _running = false;
    std::thread _thread;
    cpu_set_t _cpuset; // Affinity set.
    int _myId; // Debugging purpose.
    std::mutex _rand_mutex;  // protects rand() used in threads.
public:
    StressCpuThread();

    void start();

    void addAffinity(unsigned int);

    void resetAffinity();

    void stop();
};


#endif //UNTITLED_STRESSCPUTHREAD_H
