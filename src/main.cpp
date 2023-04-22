#include <iostream>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include "config.hpp"
using namespace std;

typedef struct {
    bool success;
    int data;
} batt_percentage;

typedef struct {
    bool success;
    char data;
} batt_status;

batt_percentage read_batt_percentage () {
    batt_percentage data;
    const char* pathToFile = PATH_TO_BATT_CAPACITY;
    char* percentage = new char[5];
    FILE * pFile;
    pFile = fopen(pathToFile, "r");
    if (pFile == NULL) {
        data.success = false;
        data.data = 0;
    } else {
        while (!feof(pFile)){
            if (fgets(percentage, 5, pFile) == NULL) break;
        }
        fclose(pFile);
        data.success = true;
        data.data = atoi(percentage);
    }
    return data;
}

batt_status read_batt_status () {
    batt_status data;
    const char* pathToFile = PATH_TO_BATT_STATUS;
    char status [25];
    FILE * pFile;
    pFile = fopen(pathToFile, "r");
    if (pFile == NULL) {
        data.success = false;
        data.data = '\0';
    } else {
        while (!feof(pFile)){
            if (fgets(status, 25, pFile) == NULL) break;
        }
        fclose(pFile);
        int len = strcspn(status, "\n"); 
        strncpy(status + len, "", 1); 
        data.success = true;
        data.data = *status;
    }
    return data;
}

int spawn_process(char* args[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(args[0], args);
        cerr << "Failed to execute " << args[0] << std::endl;
        return 1;
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        cout << "Child process exited with status " << status << std::endl;
    } else {
        cerr << "Failed to fork" << std::endl;
        return 1;
    }
    return 0;
}

int main () {
    bool running = true;
    while (running == true) {
        batt_percentage battPercentageData = read_batt_percentage();
        batt_status battStatusData = read_batt_status();
        if (battStatusData.success == false or battPercentageData.success == false){
            running = false;
            break;
        }
        char message[100];
        sprintf(message, NOTIFICATION, battPercentageData.data);

        const char * spawn_notif[] = {"/usr/bin/notify-send", "--app-name=Battery", "-u", "critical", "-t", "10000", message, NULL};
        const int nargs = sizeof(spawn_notif) / sizeof(spawn_notif[0]) - 1;
        char* cargs[nargs];
        for (int i = 0; i < nargs; ++i) {
            cargs[i] = const_cast<char*>(spawn_notif[i]);
        }
        switch (battStatusData.data){
            case 'C':
                cout << "Charging!\nsleeping for : " << SLEEP_TIME_LONG << endl;
                sleep(SLEEP_TIME_LONG);
                break;
            case 'D':
                cout << "Discharging!\n";
                cout << battPercentageData.data << "% ";
                switch (battPercentageData.data) {
                    case BATT_LOW ... 100:
                        cout << "sleeping for : " << SLEEP_TIME_LONG << endl;
                        sleep(SLEEP_TIME_LONG);
                        break;
                    case BATT_CRITICAL ... BATT_LOW-1:
                        cout << "sleeping for : " << SLEEP_TIME_FAST << endl;
                        sleep(SLEEP_TIME_FAST);
                        break;
                    case 1 ... BATT_CRITICAL-1 :
                        cout << "sleeping for : " << SLEEP_TIME_NORMAL << endl;
                        spawn_process(cargs);
                        sleep(SLEEP_TIME_NORMAL);
                        break;
                    default:
                        break;
                }
                break;
            case 'F':
                cout << "Full!\nsleeping for : " << SLEEP_TIME_LONG << endl;
                sleep(SLEEP_TIME_LONG);
                break;
            default:
                cout << "Unknown\nsleeping for : " << SLEEP_TIME_NORMAL << endl;
                sleep(SLEEP_TIME_NORMAL);
                break;
        }
    }
    return 0;
}
