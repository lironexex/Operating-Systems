#ifndef SMASH__SIGNALS_H_
#define SMASH__SIGNALS_H_

#include <iostream>
#include <signal.h>
#include "smash.h"

using namespace std;
namespace smash{
    void ctrlZHandler(int SigNum) {
    //    cerr << "debug: got ctrl-Z" << "\n";
        cout << "smash: got ctrl-Z" << "\n";
        pid_t pid = Jobs.killForegroundJob(SIGSTOP, false);
    //    cerr << "debug: " << pid << endl;
        if(pid != -1)
            cout << "smash: process "
                 << pid
                 << " was stopped"
                 << endl;
    }

    void ctrlCHandler(int SigNum) {
        cout << "smash: got ctrl-C" << "\n";
        pid_t pid = Jobs.killForegroundJob(SIGKILL, true);
        if(pid != -1)
            cout << "smash: process "
                 << pid
                 << " was killed"
                 << endl;
    }

}

#endif //SMASH__SIGNALS_H_
