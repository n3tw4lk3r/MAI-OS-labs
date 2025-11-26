#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <cstring>
#include <string>

#include "SharedData.hpp"

int main() {
    int shared_memory_file_descriptor = shm_open("/calc_shm", O_CREAT | O_RDWR, 0666);
    if (shared_memory_file_descriptor == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(shared_memory_file_descriptor, sizeof(SharedData)) == -1) {
        perror("ftruncate");
        shm_unlink("/calc_shm");
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    SharedData* shared_data = (SharedData*)mmap(NULL, sizeof(SharedData), 
                                               PROT_READ | PROT_WRITE, 
                                               MAP_SHARED, shared_memory_file_descriptor, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        shm_unlink("/calc_shm");
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    memset(shared_data, 0, sizeof(SharedData));
    shared_data->is_ready = false;
    shared_data->is_processed = false;

    sem_t* semaphore_server = sem_open("/calc_sem_server", O_CREAT, 0666, 0);
    sem_t* semaphore_client = sem_open("/calc_sem_client", O_CREAT, 0666, 0);
    if (semaphore_server == SEM_FAILED || semaphore_client == SEM_FAILED) {
        perror("sem_open");
        munmap(shared_data, sizeof(SharedData));
        shm_unlink("/calc_shm");
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        munmap(shared_data, sizeof(SharedData));
        shm_unlink("/calc_shm");
        sem_close(semaphore_server);
        sem_close(semaphore_client);
        sem_unlink("/calc_sem_server");
        sem_unlink("/calc_sem_client");
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // child
        munmap(shared_data, sizeof(SharedData));
        close(shared_memory_file_descriptor);
        
        execl("./child", "./child", (char*)NULL);
        perror("execl");
        _exit(EXIT_FAILURE);
    }
    
    // parent
    std::string filename;
    char ch;
    
    write(STDOUT_FILENO, "Enter filename: ", 16);
    
    while (read(STDIN_FILENO, &ch, 1) > 0) {
        if (ch == '\n') {
            break;
        }
        filename.push_back(ch);
    }

    if (filename.empty()) {
        return EXIT_FAILURE;
    }

    strncpy(shared_data->filename, filename.c_str(), sizeof(shared_data->filename) - 1);
    shared_data->filename[sizeof(shared_data->filename) - 1] = '\0';
    shared_data->is_processed = false;

    sem_post(semaphore_client);

    sem_wait(semaphore_server);

    if (strlen(shared_data->result) > 0) {
        write(STDOUT_FILENO, "Result: ", 8);
        write(STDOUT_FILENO, shared_data->result, strlen(shared_data->result));
        write(STDOUT_FILENO, "\n", 1);
    }

    strncpy(shared_data->filename, "", sizeof(shared_data->filename));
    shared_data->is_processed = false;
    sem_post(semaphore_client);

    waitpid(pid, NULL, 0);

    munmap(shared_data, sizeof(SharedData));
    close(shared_memory_file_descriptor);
    shm_unlink("/calc_shm");
    sem_close(semaphore_server);
    sem_close(semaphore_client);
    sem_unlink("/calc_sem_server");
    sem_unlink("/calc_sem_client");

    return EXIT_SUCCESS;
}
