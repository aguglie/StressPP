# StressPP
StessPP is a synthetic workload generator based on [stress](https://linux.die.net/man/1/stress).
With Stress*PP* you can pressure on disk i/o, memory and cpu.

## Link to the presentation [here](https://raw.githubusercontent.com/Guglio95/StressPP/master/docs/Presentation.pdf)

## TLDR; Examples:
### CPU

Run 5 threads to stress CPU.
> ./stresspp -c 5
<p align="center">
<img src="docs/stress_cpu_0.png?raw=true">
</p>


Run 2 threads and stick the former to core 0, the latter to core 3.
> ./stresspp --cpu-affinity 1000,0001
<p align="center">
<img src="docs/stress_cpu_1.png?raw=true">
</p>


### Virtual Memory
Run 9 threads which are allocating and randomly accessing 2 x 500Mb chunks. 
> ./stresspp -m 9 --vm-chunks 2 --vm-bytes 536870912


### I/O
Run 2 threads stressing I/O with sync().
> ./stresspp -i 2

### Disk
Run a thread continuously writing and deleting 1Gb file.
> ./stresspp -d 1

### Complete list of allowed options
```
Allowed options:
  -h, --help                     Produce help message
  -t, --timeout arg (=30)        timeout (stop) after n seconds
  -m, --vm arg (=0)              spawn n threads spinning on malloc()
  --vm-chunks arg (=1)           malloc c chunks (default is 1)
  --vm-bytes arg (=268435456)    malloc chunks of b bytes (default is 256MB)
  -d, --hdd arg (=0)             spawn n threads spinning on write()
  --hdd-noclean arg (=0)         do not unlink file to which random data written
  --hdd-files arg (=1)           write to f files (default is 1)
  --hdd-bytes arg (=1073741824)  write b bytes (default is 1GB)
  -i, --io arg (=0)              spawn n threads spinning on sync()
  -c, --cpu arg (=0)             spawn n threads spinning on sqrt(). You can omit this option if using --cpu-affinity.
  --cpu-affinity arg             Cpu Affinity Matrix to stick a thread to a CPU. Rows are threads, cols are CPUs, comma separated. Ex: 1000,0100,0010,0001
```

## Write your own script
Instead of using command-line you can write your own script:
```cpp
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
    return 0;
}
```
