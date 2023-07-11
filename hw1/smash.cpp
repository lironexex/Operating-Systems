// Default code skeleton
#include "Command.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


#define MAX_HISTORY_COUNT 50

using namespace std;


namespace smash {


    // Log history.
    void logHistoryInsert(string StrLine) {
		
		if (HistoryCursor >= MAX_HISTORY_COUNT) {
			for (int i = 0; i < MAX_HISTORY_COUNT - 1; i ++) {
				History[i] = History[i+1];
				Counters[i] = Counters[i+1];
			}
			HistoryCursor = MAX_HISTORY_COUNT - 1;
		}
		
		// counts the number of commands that have been typed.
		Counters[HistoryCursor] = CommandsCounter;
        History[HistoryCursor] = StrLine;
        HistoryCursor++;
        if (HistoryCursor >= HISTORY_SIZE) {
            HistoryCursor = 0;
        }
    }


    const string WHITESPACE = " \n\r\t\f\v";
    string _ltrim(const std::string& s) {
        size_t start = s.find_first_not_of(WHITESPACE);
        return (start == std::string::npos) ? "" : s.substr(start);
    }
    string _rtrim(const std::string& s) {
        size_t end = s.find_last_not_of(WHITESPACE);
        return (end == std::string::npos) ? "" : s.substr(0, end + 1);
    }
    string _trim(const std::string& s) {
        return _rtrim(_ltrim(s));
    }


    void smashLoop(){


        // Register signals SIGINT and SIGTSTP signal handler

        __sighandler_t syscallAttempt;
        syscallAttempt = signal(SIGTSTP, ctrlZHandler);
        if(syscallAttempt == SIG_ERR)
            perror("smash error: failed to set ctrl-Z handler");

        syscallAttempt = signal(SIGINT, ctrlCHandler);
        if(syscallAttempt == SIG_ERR)
            perror("smash error: failed to set ctrl-C handler");

        while(true) {
            cout << "smash> ";

            string CmdLine;
            getline(cin, CmdLine);
			
            if (!CmdLine.compare(""))
                continue;
			
			CommandsCounter++;
						
			if (HistoryCursor == 0) {
				logHistoryInsert(CmdLine);
			}
			// Check if the last command is the same as the current command.
			else {
				if (CmdLine.compare(History[HistoryCursor - 1]) != 0 ) {
					logHistoryInsert(CmdLine);
				}
				
				else {
					Counters[HistoryCursor - 1] = CommandsCounter;
				}
			}

            CmdLine = _trim(CmdLine);


            size_t Bonus = CmdLine.find_first_of(">|");
            if(Bonus == string::npos){ //no pipe, no redirection
                Command Cmd(CmdLine);
                Cmd.execute();
                Cmd.waitForTermination();
       //         cerr << "debug6" << endl;

            } else if(CmdLine[Bonus] == '>'){ //redirection
                bool IsAppending = CmdLine[Bonus + 1] == '>';
                string Line1 = CmdLine.substr(0, Bonus);
                string Line2 = CmdLine.substr(Bonus + 1 + IsAppending, Bonus);
                Line1 = _trim(Line1);
                Line2 = _trim(Line2);
                Command Cmd(Line1);

				// Fd for checking errors
                int Fd;
                if(IsAppending) { //redirection appending ">>"
                    Fd = open(Line2.c_str(), O_WRONLY | O_CREAT | O_APPEND,
                            0644);
                } else { //redirection overriding ">"
                    Fd = open(Line2.c_str(), O_WRONLY | O_CREAT,
                            0644);
                }
                if (Fd < 0) {
                    perror("smash error: open failed");
                    continue;
                }
                Cmd.switchChannel(stdout, Fd);
                Cmd.execute();

                Cmd.revertChannel();
                Cmd.waitForTermination();

            } else { //pipe
                bool IsErrPipe = CmdLine[Bonus + 1] == '&';
                string Line1 = CmdLine.substr(0, Bonus);
                string Line2 = CmdLine.substr(Bonus + 1 + IsErrPipe, Bonus);
                Line1 = _trim(Line1);
                Line2 = _trim(Line2);
                Command Cmd1(Line1), Cmd2(Line2);
                int PipeFds[2];
                pipe(PipeFds);
                if(IsErrPipe) { //err pipe "|&"
                    Cmd1.switchChannel(stderr, PipeFds[1]);
                } else { //out pipe "|"
                    Cmd1.switchChannel(stdout, PipeFds[1]);
                }
                Cmd1.execute();
                Cmd1.revertChannel();

                Cmd2.switchChannel(stdin, PipeFds[0]);
                Cmd2.execute();
                Cmd2.revertChannel();

                Cmd1.waitForTermination();
                Cmd2.waitForTermination();
            }
        }

    }
}

int main () {
    smash::smashLoop();

    return 0;
}
