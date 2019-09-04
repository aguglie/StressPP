#include <vector>
#include <chrono>
#include "StressPP.h"


void sleep(int i);

using namespace std;


int main(int argc, char **argv) {
    int howManyCpuWorkers = 3;
    StressPP stressPP(howManyCpuWorkers); // Instantiates StressPP with 3 stress-cpu threads ready to start.

    //Start CPU Workers.
    for (int i = 0; i < howManyCpuWorkers; i++)
        stressPP.getStressCpuThreads()->at(i).start();

    sleep(2000); // Sleep 2s.

    //Stick all CPU Workers to core 0 and 1
    for (int i = 0; i < howManyCpuWorkers; i++) {
        stressPP.getStressCpuThreads()->at(i).addAffinity(0);
        stressPP.getStressCpuThreads()->at(i).addAffinity(1);
    }

    sleep(2000); // Sleep 2s.

    //Stick all CPU Workers to core 2 and 3
    for (int i = 0; i < howManyCpuWorkers; i++) {
        stressPP.getStressCpuThreads()->at(i).resetAffinity();
        stressPP.getStressCpuThreads()->at(i).addAffinity(2);
        stressPP.getStressCpuThreads()->at(i).addAffinity(3);
    }

    //Meanwhile stress disk:
    //1 thread, cleaning file at exit, 2 files, 5Mb
    stressPP.writeStress(1, true, 2, 1024 * 1024 * 5);

    //Meanwhile stress VM:
    //1 thread, writing 2 chunks of 5Mb
    stressPP.mallocStress(1, 2, 1024 * 1024 * 5);

    //Meanwhile stress IO
    //1 thread
    stressPP.syncStress(1);

    //Stop CPU Workers.
    for (int i = 0; i < howManyCpuWorkers; i++)
        stressPP.getStressCpuThreads()->at(i).stop();

    //Stop other stressers:
    stressPP.stopWriteStress();
    stressPP.stopMallocStress();
    stressPP.stopSyncStress();

    //We could have stopped 'em all with just:
    stressPP.stop();

}

void sleep(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}


void split(const string &s, char c, vector<string> &v) {
    string::size_type i = 0;
    string::size_type j = s.find(c);

    if (j == string::npos) {
        v.push_back(s);
        return;
    }

    while (j != string::npos) {
        v.push_back(s.substr(i, j - i));
        i = ++j;
        j = s.find(c, j);

        if (j == string::npos)
            v.push_back(s.substr(i, s.length()));
    }
}

