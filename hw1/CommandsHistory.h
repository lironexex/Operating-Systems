#ifndef OS_COMMANDSHISTORY_H
#define OS_COMMANDSHISTORY_H


class CommandsHistory {
protected:
    class CommandHistoryEntry {
        // TODO: Add your data members
    };
    // TODO: Add your data members
public:
    CommandsHistory();
    ~CommandsHistory() {}
    void addRecord(const char* cmd_line);
    void printHistory();
};

#endif //OS_COMMANDSHISTORY_H
