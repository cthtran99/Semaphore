#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include "dungeon_info.h"
#include <errno.h>

void decode_caesar_cipher(char *input, char *output, int key) {
    char c;
    for (int i = 0; input[i]; i++) {
        c = input[i];
        if ('A' <= c && c <= 'Z') {
            output[i] = ((c - 'A' + key) % 26) + 'A';
        } else if ('a' <= c && c <= 'z') {
            output[i] = ((c - 'a' + key) % 26) + 'a';
        } else {
            output[i] = c;
        }
    }
    output[strlen(input)] = '\0';
}

void wizard_signal_handler(int sig) {
    int shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
    struct Dungeon *dungeon = (struct Dungeon *)mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    int key = dungeon->barrier.spell[0] - 'A';
    decode_caesar_cipher(dungeon->barrier.spell, dungeon->wizard.spell, key);

    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
}

// Add the main function here
// int main() {
//     signal(DUNGEON_SIGNAL, wizard_signal_handler);

//     pid_t wizard_pid = getpid();
//     pid_t rogue_pid = 0; // replace with the rogue's process ID
//     pid_t barbarian_pid = 0; // replace with the barbarian's process ID

//     RunDungeon(wizard_pid, rogue_pid, barbarian_pid);

//     while (1) {
//         pause(); // wait for signals
//     }

//     return 0;
// }
