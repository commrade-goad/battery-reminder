#include <iostream>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include "config.hpp"
using namespace std;

const int AUDIO_FREQUENCY = 44100;
const int AUDIO_CHANNELS = 2;
const int AUDIO_CHUNKSIZE = 1024;

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

int play_audio(const char* pathToFile) {
    if (!filesystem::exists(pathToFile)) return 1;
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        cout << "ERROR : Failed to initialize SDL audio system: " << SDL_GetError() << endl;
        return 1;
    }
        if (Mix_OpenAudio(AUDIO_FREQUENCY, MIX_DEFAULT_FORMAT, AUDIO_CHANNELS, AUDIO_CHUNKSIZE) < 0) {
        cout << "ERROR : Failed to open audio device: " << Mix_GetError() << endl;
        return 1;
    }

    const char* audioExt = strchr(pathToFile, '.');
    if (strstr(audioExt, "mp3")) {
        Mix_Music* audio = Mix_LoadMUS(pathToFile);
        if (!audio) {
            cout << "ERROR : Failed to load audio file: " << Mix_GetError() << endl;
            return 1;
        }
        Mix_PlayMusic(audio, 0);
        while (Mix_PlayingMusic()) {
            SDL_Delay(100);
        }
        Mix_FreeMusic(audio);
    } else if (strstr(audioExt, "wav")) {
        Mix_Chunk* audio = Mix_LoadWAV(pathToFile);
        if (!audio) {
            cout << "ERROR : Failed to load audio file: " << Mix_GetError() << endl;
            return 1;
        }
        Mix_PlayChannel(-1, audio, 0);
        while (Mix_Playing(-1)) {
            SDL_Delay(100);
        }
        Mix_FreeChunk(audio);
    } else {
        cout << "ERROR : Unsupported audio format " << audioExt << endl;
        return 1;
    }
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
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
    if (play_audio(PATH_TO_AUDIO_FILE) !=0) cout << "Failed to play audio!\n";
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
                        if (play_audio(PATH_TO_AUDIO_FILE) !=0) cout << "Failed to play audio!\n";
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
