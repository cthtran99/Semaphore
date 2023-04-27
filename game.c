#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "dungeon_info.h"
#include <semaphore.h>
#include <errno.h>
#include <string.h>

sem_t *sem_lever1;
sem_t *sem_lever2;
struct Dungeon *dungeon;
pid_t barbarian_pid, wizard_pid, rogue_pid;
void handle_semaphore_signal(int signum);
void handle_release_lever_signal(int signum);
#define RELEASE_LEVER_SIGNAL SIGUSR1


int main() {
    int shm_fd = shm_open(dungeon_shm_name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    ftruncate(shm_fd, sizeof(struct Dungeon));
    struct Dungeon *dungeon = (struct Dungeon *)mmap(NULL, sizeof(struct Dungeon), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    // NULL is the default value for the semaphore in treasure and spoils.
    strcpy(dungeon->treasure, "NULL");
    // Initialize the dungeon struct here

    // Fork and exec the Barbarian process
    barbarian_pid = fork();
    if (barbarian_pid == 0) {
        execl("./barbarian", "barbarian", NULL);
    }

    // Fork and exec the Wizard process
    wizard_pid = fork();
    if (wizard_pid == 0) {
        execl("./wizard", "wizard", NULL);
    }

    // Fork and exec the Rogue process
    rogue_pid = fork();
    if (rogue_pid == 0) {
        execl("./rogue", "rogue", NULL);
    }

    sem_unlink(dungeon_lever_one);
    sem_unlink(dungeon_lever_two);
   
    // Create the semaphores
    sem_lever1 = sem_open(dungeon_lever_one, O_CREAT, 0666, 0);
    sem_lever2 = sem_open(dungeon_lever_two, O_CREAT, 0666, 0);
    // Check if semaphores are created successfully
    if (sem_lever1 == SEM_FAILED || sem_lever2 == SEM_FAILED) {
        perror("Semaphore initialization failed");
        fprintf(stderr, "ERRNO: %d\n", errno);
    }

    struct sigaction sa;
    sa.sa_handler = handle_semaphore_signal;
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        printf("ERROR: sigaction\n");
        perror("sigaction");
        return 1;
    }


    // Register signal handlers
    //sigaction(SEMAPHORE_SIGNAL, handle_semaphore_signal);
    signal(RELEASE_LEVER_SIGNAL, handle_release_lever_signal);

    // Launch the dungeon using the PIDs of the character processes
    RunDungeon(wizard_pid, rogue_pid, barbarian_pid);


    // Wait for the child processes to exit
    waitpid(barbarian_pid, NULL, 0);
    waitpid(wizard_pid, NULL, 0);
    waitpid(rogue_pid, NULL, 0);

    // Cleanup
    munmap(dungeon, sizeof(struct Dungeon));
    close(shm_fd);
    shm_unlink(dungeon_shm_name);

    // Close and unlink semaphores
    sem_close(sem_lever1);
    sem_close(sem_lever2);
    sem_unlink(dungeon_lever_one);
    sem_unlink(dungeon_lever_two);
    return 0;
}

void handle_release_lever_signal(int signum) {
    printf("Received release lever signal\n");
    if (signum == RELEASE_LEVER_SIGNAL) {
        // Release the semaphores
        sem_post(sem_lever1);
        sem_post(sem_lever2);
    }
}

void handle_semaphore_signal(int signum) {
    if (signum == SIGUSR1) {
        printf("Received SEMAPHORE_SIGNAL\n");
        // Wait for both semaphores
        sem_wait(sem_lever1);
        sem_wait(sem_lever2);

        // Copy the treasure to the spoils field
        char treasure[4];
        for (int i = 0; i <= 3; i++) {
            treasure[i] = dungeon->treasure[i];
            printf("Test %c", treasure[i]);
            sleep(1);
        }
        strncpy(dungeon->spoils, treasure, 4);

        // Release the semaphores
        kill(rogue_pid, RELEASE_LEVER_SIGNAL);
        kill(wizard_pid, RELEASE_LEVER_SIGNAL);
        kill(barbarian_pid, RELEASE_LEVER_SIGNAL);    
    }
}
