#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "dungeon_info.h"
#include <errno.h>


void barbarian_signal_handler(int sig) {
    int shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
    struct Dungeon *dungeon = (struct Dungeon *)mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    dungeon->barbarian.attack = dungeon->enemy.health;

    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
}

// int main(int argc, char *argv[]) {
//     signal(DUNGEON_SIGNAL, barbarian_signal_handler);

//     pid_t my_pid = getpid();
//     pid_t dungeon_pid = atoi(argv[1]);

//     if (kill(dungeon_pid, 0) == -1) {
//         perror("Failed to find dungeon process");
//         exit(1);
//     }

//     RunDungeon(0, 0, my_pid);

//     return 0;
// }
