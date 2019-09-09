#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <chrono>
#include "StressPP.h"

#include "popl/popl.hpp"

void sleep(int i);

void split(const string &s, char c, vector<string> &v);

using namespace std;
using namespace popl;


int main(int argc, char **argv) {
    OptionParser op("Allowed options");

    //General
    auto help_option = op.add<Switch>("h", "help", "Produce help message");
    auto timeout_seconds = op.add<Value<int>>("t", "timeout", "timeout (stop) after n seconds", 30);


    //Malloc
    auto malloc_howMany = op.add<Value<int>>("m", "vm", "spawn n threads spinning on malloc()", 0);
    auto malloc_chunks = op.add<Value<int>>("", "vm-chunks", "malloc c chunks (default is 1)", 1);
    auto malloc_vmBytes = op.add<Value<long long>>("", "vm-bytes", "malloc chunks of b bytes (default is 256MB)",
                                                   256 * 1024 * 1024);

    //Disk
    auto disk_howMany = op.add<Value<int>>("d", "hdd", "spawn n threads spinning on write()", 0);
    auto disk_noClean = op.add<Value<bool>>("", "hdd-noclean", "do not unlink file to which random data written",
                                            false);
    auto disk_files = op.add<Value<long long>>("", "hdd-files", "write to f files (default is 1)", 1);
    auto disk_bytes = op.add<Value<long long>>("", "hdd-bytes", "write b bytes (default is 1GB)", 1024 * 1024 * 1024);

    //IO
    auto io_howMany = op.add<Value<int>>("i", "io", "spawn n threads spinning on sync()", 0);

    //CPU
    auto cpu_howMany = op.add<Value<int>>("c", "cpu",
                                          "spawn n threads spinning on sqrt(). You can omit this option if using --cpu-affinity.",
                                          0);
    auto cpu_aff_matrix = op.add<Value<string>>("", "cpu-affinity",
                                                "Cpu Affinity Matrix to stick a thread to a CPU. Rows are threads, cols are CPUs, comma separated. Ex: 1000,0100,0010,0001\nYou can also set this via env 'CPU_AFFINITY'",
                                                "");
    op.parse(argc, argv);


    //If CPU_AFFINITY env is set use it as param.
    if (const char *env_p = std::getenv("CPU_AFFINITY"))
        cpu_aff_matrix->set_value(env_p);

    //If no useful param was provided show help.
    if (!malloc_howMany->is_set() && !disk_howMany->is_set() && !io_howMany->is_set() &&
        (!cpu_howMany->is_set() && !cpu_aff_matrix->is_set())) {
        cout << op << "\n";
        return 0;
    }

    //CPU part:

    //Split rows of the affinity matrix:
    vector<string> cpu_aff_matrix_rows;
    split(cpu_aff_matrix->value(), ',', cpu_aff_matrix_rows);

    // Calculate how many CPU threads are needed as Math.max(Cpu Affinity Rows, cpu_howMany)
    int cpu_howManyWorkers = (cpu_aff_matrix_rows.size() > cpu_howMany->value() && cpu_aff_matrix->is_set())
                             ? cpu_aff_matrix_rows.size() // Rows of the affinity matrix (if present)
                             : cpu_howMany->value(); // Value passed via '-c' param.

    StressPP stressPP(cpu_howManyWorkers); // Create a new StressPP instance with given threads.

    //If the affinity matrix is present, set for each thread its affinity.
    if (cpu_aff_matrix->is_set())
        for (int threadRow = 0; threadRow < cpu_aff_matrix_rows.size(); ++threadRow) {
            cout << "Affinity row " << threadRow << ": " << cpu_aff_matrix_rows[threadRow] << '\n';
            for (std::string::size_type cpuColumn = 0; cpuColumn < cpu_aff_matrix_rows[threadRow].size(); ++cpuColumn) {
                if (cpu_aff_matrix_rows[threadRow][cpuColumn] == '1') {
                    stressPP.getStressCpuThreads()->at(threadRow).addAffinity(cpuColumn);
                }
            }
        }

    //Start every CPU worker.
    for (int i = 0; i < cpu_howManyWorkers; i++)
        stressPP.getStressCpuThreads()->at(i).start();

    //  / End of CPU part.


    //Stress disk if required.
    if (disk_howMany->value() > 0)
        stressPP.writeStress(disk_howMany->value(), !disk_noClean->value(), disk_files->value(), disk_bytes->value());

    //Stress VM if required
    if (malloc_howMany->value() > 0)
        stressPP.mallocStress(malloc_howMany->value(), malloc_chunks->value(), malloc_vmBytes->value());

    //Stress IO if required
    if (io_howMany->value() > 0)
        stressPP.syncStress(io_howMany->value());

    //Sleep if required.
    if (timeout_seconds->value() > 0) {
        sleep(timeout_seconds->value() * 1000);
        stressPP.stop();
    }

}

/**
 * Sleep utility.
 */
void sleep(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}


/**
 * Utility to split string.
 * @param s string to split
 * @param c delimiter
 * @param v vector of tokens.
 */
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

