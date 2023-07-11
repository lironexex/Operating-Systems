
#ifndef OS_SMASH_H
#define OS_SMASH_H

#include "JobsList.h"


namespace smash{
    JobsList Jobs;

    const unsigned MAX_WORD_SIZE = 100;
    const unsigned HISTORY_SIZE = 51;

    std::string History[HISTORY_SIZE];
    unsigned HistoryCursor = 0;
	unsigned int CommandsCounter = 0;
	int Counters[HISTORY_SIZE];

    char LastFolder[MAX_WORD_SIZE];

    int killWrapper(pid_t Pid, int SigNum){
        int SyscallResult = kill(Pid, SigNum);
        if(SyscallResult < 0)
            perror("smash error: kill failed");
        return SyscallResult;
    }
}

#endif //OS_SMASH_H
