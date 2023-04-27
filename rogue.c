#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "dungeon_info.h"
#include <errno.h>

float binary_search_pick(float low, float high, float target) {
    float mid;
    while (high - low > 1e-6) {
        mid = (low + high) / 2;
        if (mid < target) {
            low = mid;
        } else {
            high = mid;
        }
    }
    return mid;
}

void rogue_signal_handler(int sig) {
    int shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
    struct Dungeon *dungeon = (struct Dungeon *)mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    
    float target_pick = dungeon->trap.locked ? 0.0 : 1.0;
    dungeon->rogue.pick = binary_search_pick(0.0, 1.0, target_pick);

    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
}

// int main() {
//     struct sigaction sa;
//     sa.sa_handler = rogue_signal_handler;
//     sigemptyset(&sa.sa_mask);
//     sa.sa_flags = 0;

//     if (sigaction(DUNGEON_SIGNAL, &sa, NULL) == -1) {
//         perror("sigaction");
//         exit(1);
//     }

//     // Get the Dungeon process ID
//     int shm_fd = shm_open(dungeon_shm_name, O_RDWR, 0666);
//     if (shm_fd == -1) {
//         perror("shm_open");
//         exit(EXIT_FAILURE);
//     }

//     struct Dungeon *dungeon = (struct Dungeon *)mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
//     if (dungeon == MAP_FAILED) {
//         perror("mmap");
//         exit(EXIT_FAILURE);
//     }
//     pid_t dungeon_pid = dungeon->dungeonPID;
//     munmap(dungeon, sizeof(struct Dungeon));
//     close(shm_fd);

//     while (1) {
//         pause(); // Wait for a signal to start picking the lock
//     }

//     return 0;
// }
