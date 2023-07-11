#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
//#include "Commands.h"
#include "smash.h"
#include "signals.h"
#include "CommandsHistory.h"
#include "JobsList.h"

// added
#include <csignal>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h> // TODO: NEW for open() function
#include <cstdio> // TODO: NEW for BUFSIZ
#include <iomanip> // TODO: NEW for setw() function

namespace smash{
	

    int SyscallResult;

    class Command {
		
		int killWrapper(pid_t Pid, int SigNum) {
			int SyscallResult = kill(Pid, SigNum);
			if(SyscallResult < 0)
				perror("smash error: kill failed");
			return SyscallResult;
		}
	
		
		
        string CmdLine;
        string CmdName, Arg1, Arg2;
        size_t NumArgs;

        int SwitchedFd, StdFdCopy;

        JobState RequestedState;
        bool WaitForMe;
        pid_t PidToWaitFor;

        void splitCommandLine() {
            NumArgs = -1;

            string Words[3] = {"", "", ""};
            size_t WordStart, WordEnd;
            WordEnd = 0;
            for(int i = 0; i < 3; i++) {
                WordStart = CmdLine.find_first_not_of(" ", WordEnd);
                if(WordStart == string::npos)
                    break;
                WordEnd = CmdLine.find_first_of(" ", WordStart);
                Words[i] = CmdLine.substr(WordStart, WordEnd - WordStart);
                ++NumArgs;
                if(NumArgs >= 2)
                    break;
            }
            CmdName = Words[0];
            Arg1 = Words[1];
            Arg2 = Words[2];
        }

    public:
        Command(string CmdLine)
        : CmdLine(CmdLine) {
            splitCommandLine();
            WaitForMe = false;
            SwitchedFd = -1;
        }
        Command(const Command &other) = delete;
        Command &operator=(const Command &other) = delete;
        ~Command() {}

        void switchChannel(FILE* StdFile, int NewFd){
            //prepare a copy to restore the channel afterwards
            this->SwitchedFd = fileno(StdFile);
            this->StdFdCopy = dup(this->SwitchedFd);
            if(this->StdFdCopy == -1)
                perror("smash error: dup failed");
            //override the current standard channel
            SyscallResult = dup2(NewFd, this->SwitchedFd);
            if(SyscallResult == -1)
                perror("smash error: dup2 failed");
            //close the input NewFd, because it has been duplicated and won't
            // serve any other process
            SyscallResult = close(NewFd);
            if(SyscallResult == -1)
                perror("smash error: close failed");
        }
        void revertChannel(){
            SyscallResult = close(this->SwitchedFd);
            if(SyscallResult == -1)
                perror("smash error: close failed");
            //override the temporary fd with the old one
            SyscallResult = dup2(this->StdFdCopy, this->SwitchedFd);
            if(SyscallResult == -1)
                perror("smash error: dup2 failed");
            //clear temporary fd
            close(this->StdFdCopy);
            if(SyscallResult == -1)
                perror("smash error: close failed");

            //mark that there is no need to revert again
            this->SwitchedFd = -1;
        }
        void closeFiles(){
            if(this->SwitchedFd != -1)
                revertChannel();
        }

        void execute(){
            executeInternal();
        }

        void executeInternal() {
            // Handle the 'pwd' command (1/10):
            if (!CmdName.compare("pwd")) {
                char CwdStr[MAX_WORD_SIZE];
                if (getcwd(CwdStr, sizeof(CwdStr)) == NULL)
                    perror("smash error: getcwd failed");
                else
                    cout << CwdStr << endl;
                return;
            }

            // Handle the 'cd' command (2/10):
            if (!CmdName.compare("cd")) {

                if (NumArgs == 0) {
                    cout << "smash error: cd: missing directory argument\n";
                    return;
                }
                if (NumArgs > 1) {
                    cout << "smash error: cd: too many arguments\n";
                    return;
                }

                // Deal with "-"
                if (Arg1 == "-") {
                    if (strlen(LastFolder) == 0) {
                        cout << "smash error: cd: OLDPWD not set" << "\n";
                        return;
                    }

                    char TempFolder[MAX_WORD_SIZE];
                    strcpy(TempFolder, LastFolder);
                    if (getcwd(LastFolder, sizeof(LastFolder)) == NULL)
                        perror("smash error: getcwd failed");
                    if (chdir(TempFolder) != 0)
                        perror("smash error: chdir failed");
                    return;
                }

                // Apply chdir and log history.
                const char *DirStr = Arg1.c_str();
                if (getcwd(LastFolder, sizeof(LastFolder)) == NULL)
                    perror("smash error: getcwd failed");
                if (chdir(DirStr) != 0)
                    perror("smash error: chdir failed");
                return;
            }

            // Handle the 'history' command (3/10).
            if (!CmdName.compare("history")) {
                for (unsigned i = 0; i < HistoryCursor; i++) {
                    cout << right << setw(5) << Counters[i] << " " << History[i]
                    << endl;
                }
                return;
            }


            // Handle the 'jobs' internal command (4/10).
            if (!CmdName.compare("jobs")) {
                Jobs.printJobsList();
                return;
            }


            // Handle the kill command (5/10):
            if (!CmdName.compare("kill")) {

                char char_StrLine[CmdLine.size() + 1];
                strcpy(char_StrLine, CmdLine.c_str());

                // Retrieve signal number string.
                char *TempString = strtok(char_StrLine, " ");
                TempString = strtok(NULL, " ");    //Arg1

                // Retrieve job ID string.
                char *StrWord = strtok(NULL, " "); //Arg2



                int SignalNumber;
                // Convert TempString string to int: signalNumber
                if (sscanf(TempString, "%d", &SignalNumber) < 1) {
                    cout << "smash error: kill: invalid arguments\n";
                    return;
                }

                if (SignalNumber > 0) {
                    cout << "smash error: kill: invalid arguments\n";
                    return;
                }

                SignalNumber = SignalNumber * (-1);

                int JobID;
                // Convert StrWord string to int: JobID
                if (sscanf(StrWord, "%d", &JobID) < 1) {
                    cout << "smash error: kill: invalid arguments\n";
                    return;
                }

                JobsList::JobEntry *Entry = Jobs.getJobById(JobID);
                if (Entry == NULL) {
                    cout << "smash error: kill: job-id "
                         << JobID << " does not exist" << endl;
                    return;
                }
                pid_t pid = Entry->getProcessId();

                if(killWrapper(pid, SignalNumber) >=0)
                    cout << "signal number "
                        << SignalNumber << " was sent to pid " << pid << endl;

                return;
            }


            // Handle the 'showpid' command (6/10).
            if (!CmdName.compare("showpid")) {
                pid_t pid = getpid();
                if(pid < 0)
                    perror("smash error: getpid failed");
                else
                    cout << "smash pid is " << pid << endl;
                return;
            }
      //      cerr << "debug0" << endl;
            // Handle the fg, bg commands (7/10) + (8/10).
            if (!CmdName.compare("fg") || !CmdName.compare("bg")) {
                //use RequestedState to distinguish the 2 similiar commands
        //        cerr << "debug1" << endl;
                if (!CmdName.compare("fg"))
                    RequestedState = JOB_STATE_FOREGROUND;
                else //bg
                    RequestedState = JOB_STATE_BACKGROUND;

                //find the relevant job
                JobsList::JobEntry *Entry;
                if (NumArgs > 1) {
                    cout << "smash error: " << CmdName << ": "
                         << "invalid arguments" << endl;
                    return;
                } else if (NumArgs == 0) {
                    //find job
                    if (RequestedState == JOB_STATE_FOREGROUND) { //fg
                        Entry = Jobs.getLastJob(true);
                    } else {//bg
                        Entry = Jobs.getLastJob(false);
                    }
                    //handle an error
                    if (Entry == NULL) {
                        cout << "smash error: " << CmdName << ": ";
                        if (RequestedState == JOB_STATE_FOREGROUND) //fg
                            cout << "jobs list is empty";
                        else //bg
                            cout << "there is no stopped jobs to resume";
                        cout << endl;
                        return;
                    }
                } else if (NumArgs == 1) {
                    unsigned sequentialId;
                    try {
                        sequentialId = stoi(Arg1, nullptr, 0);
                    }
                    catch (const invalid_argument &ia) {
                        cout << "smash error: " << CmdName << ": "
                             << "invalid arguments" << endl;
                        return;
                    }
                    Entry = Jobs.getJobById(sequentialId);
                    if (Entry == NULL) {
                        cout << "smash error: " << CmdName << ": "
                             << "job-id "
                             << sequentialId
                             << " does not exist" << endl;
                        return;
                    } else if (RequestedState == JOB_STATE_BACKGROUND
                               and Entry->isInBackground()) {
                        cout << "smash error: bg: "
                             << "job-id " << sequentialId
                             << " is already running in the background" << endl;
                        return;
                    }
                }

                pid_t pid = Entry->getProcessId();
                Entry->printCommandLineAndPid();
                Entry->resumeOnForeground();
                if (RequestedState == JOB_STATE_FOREGROUND) { //fg
                    if (Entry->isStopped())
                        killWrapper(pid, SIGCONT);
                    Jobs.setForegroundJob(Entry);
                    WaitForMe = true;
                    PidToWaitFor = pid;
                } else { //bg
                    Entry->resumeOnBackground();
                    killWrapper(pid, SIGCONT);
                }

                Jobs.removeFinishedJobs();
                return;
            }


            // Handle the quit command (9/10):
            if (!CmdName.compare("quit")) {
                if (NumArgs > 0 and !Arg1.compare("kill")) {
                    cout << "smash: sending SIGKILL signal to "
                         << Jobs.getCount()
                         << " jobs:"
                         << endl;
                    Jobs.killAllJobs();
                }
                exit(0);
            }

            // Handle the cp command (10/10):
            if (!CmdName.compare("cp")) {
				
				char char_StrLine[CmdLine.size() + 1];
				strcpy(char_StrLine, CmdLine.c_str());

                // Retrieve the old file path string.
                char *TempString = strtok(char_StrLine, " ");
                TempString = strtok(NULL, " ");    //Arg1

                // Retrieve the new file path string.
                char *StrWord = strtok(NULL, " "); //Arg2
				
				// Buffer for transfering data.
				char buffer[BUFSIZ];
				size_t size;
				
				// Open the file we want to copy
				int source = open(TempString, O_RDONLY, 0);
				if (source < 0) {
					perror("smash error: open failed");
					return;
				}
				
				// Delete the destination file if it already exists.
				if (remove (StrWord) < -1) {
					perror("smash error: remove failed");
					return;
				}
				
				// Create the file we want to write to.
				int destination = open(StrWord, O_WRONLY | O_CREAT, 0644);
				if (destination < 0) {
					perror("smash error: open failed");
					return;
				}
							
				// Copy the data from the source file to the destination file.
				while ( (size = read(source, buffer, BUFSIZ) ) > 0) {
					
					if ( write(destination, buffer, size) < 0 ) {
						perror("smash error: write failed");
						return;
					}
					
				}
				
				// close the files.
				if ( close (source) < 0 ) {
					perror("smash error: close failed");
					return;
				}
				
				if ( close (destination) < 0 ) {
					perror("smash error: close failed");
					return;
				}
				
				// print success message
				cout << "smash: "
                << TempString
                << " was copied to "
				<< StrWord
                << endl;

				return;

            }

            // Handle another command just for debugging (11/10):
            if (!CmdName.compare("Delay2Ctrl")) {
                int Signum;
                if(Arg1[0] == 'Z') //Ctrl+Z
                    Signum = SIGTSTP;
                else  //Ctrl+C
                    Signum = SIGINT;

                pid_t pid = fork();
                if (pid == -1)
                    perror("smash error: fork failed");

                else if (pid == 0) { //child process
                    sleep(2);
                    kill(getppid(), Signum);
                    exit(0);
                }
                return;
            }


            return executeExternal();
        }
        void executeExternal(){
            // Parse the command line to check for terminating '&' character.
            size_t AmpersandPos = CmdLine.find_first_of("&", 0);
            RequestedState = JOB_STATE_BACKGROUND;
            if (AmpersandPos == string::npos) {
                RequestedState = JOB_STATE_FOREGROUND;
            }

            // Execute external command
            pid_t pid = fork();
            if (pid == -1)
                perror("smash error: fork failed");
            else if (pid == 0) { //child process
                //make sure the child won't recieve signals sent to his daddy
                SyscallResult = setpgrp();
                if(SyscallResult < 0)
                    perror("smash error: setpgrp failed");
                //remove '&' from end if it exists
                CmdLine = CmdLine.substr(0, AmpersandPos);
                //arrange execv arguments
                size_t BashLen(strlen("/bin/bash") + 1),
                        FlagLen(strlen("/bin/bash") + 1),
                        CmdLen(CmdLine.length() + 1);
                char Bash[BashLen], Flag[FlagLen], CmdCharArr[CmdLen];
                memcpy(Bash, "/bin/bash\0", BashLen);
                memcpy(Flag, "-c\0", FlagLen);
                memcpy(CmdCharArr, CmdLine.c_str(), CmdLen);
                char *const argv[4] = {Bash, Flag, CmdCharArr, NULL};

                execv(Bash, argv);
                perror("smash error: execv failed");
                exit(0);
            } else { //parent
                Jobs.addJob(CmdLine, pid, RequestedState);
                if (RequestedState == JOB_STATE_FOREGROUND) {
                    //in case the user did not write &
                    WaitForMe = true;
                    PidToWaitFor = pid;
                }
                return;
            }
        }

        void waitForTermination(){
            if(this->needsWaiting()){
                // wait till the job terminates or gets stopped
     /*           cerr << "debugA: start waiting for " << this->CmdLine << " "
                                                                         "but"
                                                                         " for "
                <<
                this->PidToWaitFor
                << endl;
                Jobs.printJobsList();
*/
                waitpid(this->PidToWaitFor, NULL, WUNTRACED);
         //       cerr << "debugB: finished waiting" << endl;
                //afterwards
                Jobs.setForegroundJob(NULL);
            }
        }
        bool needsWaiting() const{
            return WaitForMe;
        }
    };
}

#endif //SMASH_COMMAND_H_
