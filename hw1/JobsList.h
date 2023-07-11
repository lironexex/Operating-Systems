
#ifndef JOBSLIST_H
#define JOBSLIST_H

#include "/usr/include/c++/7/vector"
#include "/usr/include/c++/7/string"
#include <time.h>
#include <signal.h>
#include "smash.h"
#include <iostream>
#include <sys/wait.h>

using namespace std;


namespace smash{
    int killWrapper(pid_t Pid, int SigNum);

    enum JobState {
        JOB_STATE_FOREGROUND,
        JOB_STATE_BACKGROUND,
        JOB_STATE_STOPPED,
    };

    class JobsList {
    public:
        static const unsigned MAX_JOBS_NUM = 100;
        class JobEntry {
            string CmdLine;
            pid_t ProcessId;
            unsigned SequentialId;
            JobState State;
            time_t TimeStamp;
        public:
            JobEntry(string CmdLine, pid_t ProcessId, unsigned SequentialId)
                    : CmdLine(CmdLine), ProcessId(ProcessId),
                      SequentialId(SequentialId) {
                    this->State = JOB_STATE_BACKGROUND;
                    this->TimeStamp = time(NULL);
            }
            JobEntry(const JobEntry& other) {
                this->CmdLine =     other.CmdLine;
                this->ProcessId =   other.ProcessId;
                this->SequentialId = other.SequentialId;
                this->State =     other.State;
                this->TimeStamp =   other.TimeStamp;
            }
            JobEntry& operator=(const JobEntry& other) {
                this->CmdLine =     other.CmdLine;
                this->ProcessId =   other.ProcessId;
                this->SequentialId = other.SequentialId;
                this->State =     other.State;
                this->TimeStamp =   other.TimeStamp;
                return *this;
            }
            ~JobEntry() {}

            void stop(){
                this->State = JOB_STATE_STOPPED;
                this->TimeStamp = time(NULL);
            }
            void resumeOnBackground(){
                this->State = JOB_STATE_BACKGROUND;
                this->TimeStamp = time(NULL);
            }
            void resumeOnForeground(){
                this->State = JOB_STATE_FOREGROUND;
            }

            const pid_t getProcessId() const {return this->ProcessId;}
            const unsigned getSequentialId() const {return this->SequentialId;}
            const string& getCommandLine() const { return this->CmdLine;}
            time_t getElapsedTime() const{
                return difftime(time(NULL), this->TimeStamp);
            }

            void printJob(){
                cout << "[" << this->SequentialId << "] "
                     << this->CmdLine
                     << " : " << this->ProcessId << " "
                     << this->getElapsedTime() << " secs";
                if(this->isStopped())
                    cout << " (stopped)";
                cout << endl;
            }
            void printCommandLineAndPid(){
                cout << this->CmdLine
                     << " : "
                     << this->ProcessId
                     << endl;
            }

            bool isTerminated() const{
                int result = waitpid(this->ProcessId, NULL, WNOHANG);
                return result != 0;

                /*//for debugging
                if(result == -1 || result > 0){
                    char PidStr[6];   // ex. 34567
                    sprintf(PidStr, "%d", this->ProcessId);
                    cout << "Process " << PidStr << " found terminated by " <<
                    result << endl;
                    return true;
                }
                else { // if(result == 0)
                    char PidStr[6];   // ex. 34567
                    sprintf(PidStr, "%d", this->ProcessId);
                    cout << "Process " << PidStr << " still running by " <<
                         result << endl;
                    return false;
                }*/
            }

            bool isInBackground() const{
                return this->State == JOB_STATE_BACKGROUND;
            }
            bool isStopped() const{
                return this->State == JOB_STATE_STOPPED;
            }
            bool operator==(const JobEntry* other) const {
                return (this->ProcessId == other->ProcessId)
                    and (this->SequentialId == other->SequentialId);
            }
        };

        vector<JobEntry> entries;
        JobEntry* ForegroundJob;

        unsigned getNextId(){
            removeFinishedJobs();
            unsigned Max = 0;
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it) {
                if(it->getSequentialId() > Max)
                    Max = it->getSequentialId();
            }
            return Max + 1;
        }

    public:
        JobsList() {
            this->entries = vector<JobEntry>();
            this->ForegroundJob = NULL;
        }
        JobsList(JobsList const&)      = delete;
        void operator=(JobsList const&)  = delete;
        ~JobsList() {}

        void addJob(string CmdLine, pid_t ProcessID,
                JobState State = JOB_STATE_BACKGROUND) {
            JobEntry Entry(CmdLine, ProcessID, this->getNextId());
            vector<JobEntry>::iterator it =
                    this->entries.insert(this->entries.end(), Entry);

            if(State == JOB_STATE_STOPPED)
                it->stop();
            else if(State == JOB_STATE_FOREGROUND){
                this->setForegroundJob(&*it);
                it->resumeOnForeground();
            }
        }

        void printJobsList(){
            this->removeFinishedJobs();
            for (JobEntry Entry : this->entries) {
                Entry.printJob();
            }

        }

        void killAllJobs(){
            pid_t pid;
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ) {
                if(!it->isTerminated()){
                    pid = it->getProcessId();
                    if(killWrapper(pid, SIGKILL) >= 0)
                        cout << pid << ": "
                            << it->getCommandLine()
                            << endl;
                    it = this->entries.erase(it);
                }
                else
                    if(it != this->entries.end())
                        ++it;
            }
        }

        void removeFinishedJobs() {
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); )
                if(it->isTerminated()){
                    it = this->entries.erase(it);
                }
                else
                    if(it != this->entries.end())
                        ++it;
        }

        JobEntry* getJobById(unsigned jobId){
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it)
                if(it->getSequentialId() == jobId)
                    return &*it;
            return NULL;
        }
        void removeJob(JobEntry* ptrEntry){
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it) {
                if(&*it == ptrEntry){
                    this->entries.erase(it);
                    return;
                }
            }
        }
        void removeJobById(unsigned jobId){
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it) {
                if(it->getSequentialId() == jobId){
                    this->entries.erase(it);
                    return;
                }
            }
        }
        JobEntry* getLastJob(bool NevermindTheState){
            JobEntry* Last = NULL;
            unsigned MaxId = 0;
            for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it) {
                if(NevermindTheState || it->isStopped()){
                    if(it->getSequentialId() > MaxId) {
                        MaxId = it->getSequentialId();
                        Last = &*it;
                    }
                }
            }
            return Last;
        }

        size_t getCount(){
            return this->entries.size();
        }
        JobEntry* getForegroundJob() {return this->ForegroundJob;}
        void setForegroundJob(JobEntry* ptr) {
            this->ForegroundJob = ptr;
            if(ptr != NULL)
                ptr->resumeOnForeground();
        }

        pid_t killForegroundJob(int Signal, bool RemoveJob) {
            if(this->ForegroundJob == NULL)
                return -1;
            pid_t pid = this->ForegroundJob->getProcessId();


            killWrapper(pid, Signal);

     //       cerr << "debug:" << " kill " << pid << ' ' << Signal << endl;

            if(RemoveJob){
                for(vector<JobEntry>::iterator it = this->entries.begin();
                it != this->entries.end(); ++it){
                    if(*it == this->ForegroundJob) {
                        this->entries.erase(it);
                        break;
                    }
                }
            }
            else  //just stop, don't remove from this list
                this->ForegroundJob->stop();

            this->setForegroundJob(NULL);
            return pid;
        }
    };

    
}


#endif //JOBSLIST_H
