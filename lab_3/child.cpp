#include <cstdio>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "SharedData.hpp"

int main() {
    int shared_memory_file_descriptor = shm_open("/calc_shm", O_RDWR, 0666);
    if (shared_memory_file_descriptor == -1) {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    SharedData* shared_data = (SharedData*) mmap(NULL, sizeof(SharedData), 
                                               PROT_READ | PROT_WRITE, 
                                               MAP_SHARED, shared_memory_file_descriptor, 0);
    if (shared_data == MAP_FAILED) {
        perror("mmap");
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    sem_t* semaphore_server = sem_open("/calc_sem_server", 0);
    sem_t* semaphore_client = sem_open("/calc_sem_client", 0);
    if (semaphore_server == SEM_FAILED || semaphore_client == SEM_FAILED) {
        perror("sem_open");
        munmap(shared_data, sizeof(SharedData));
        close(shared_memory_file_descriptor);
        exit(EXIT_FAILURE);
    }

    while (true) {
        sem_wait(semaphore_client);

        if (strlen(shared_data->filename) == 0) {
            break;
        }

        int file_descriptor = open(shared_data->filename, O_RDONLY);
        if (file_descriptor < 0) {
            snprintf(shared_data->result, sizeof(shared_data->result), "Error: cannot open file");
        } else {
            char buffer[BUFSIZ];
            std::string content;
            ssize_t bytes_read;
            
            while ((bytes_read = read(file_descriptor, buffer, sizeof(buffer))) > 0) {
                content.append(buffer, bytes_read);
            }
            close(file_descriptor);

            std::string output;
            std::string line;
            size_t pos = 0;
            
            while (pos < content.size()) {
                size_t end_line = content.find('\n', pos);
                if (end_line == std::string::npos) {
                    line = content.substr(pos);
                    pos = content.size();
                } else {
                    line = content.substr(pos, end_line - pos);
                    pos = end_line + 1;
                }
                
                if (line.empty()) {
                    continue;
                }
                
                std::vector<float> numbers;
                std::string current_number;
                float sum = 0;
                int count = 0;
                
                for (size_t i = 0; i <= line.size(); ++i) {
                    char c = (i < line.size()) ? line[i] : ' ';
                    
                    if (c == ' ' || c == '\t') {
                        if (!current_number.empty()) {
                            float num = strtof(current_number.c_str(), nullptr);
                            sum += num;
                            count++;
                            current_number.clear();
                        }
                    } else {
                        current_number.push_back(c);
                    }
                }
                
                if (!current_number.empty()) {
                    float number = strtof(current_number.c_str(), nullptr);
                    sum += number;
                    count++;
                }
                
                if (count > 0) {
                    char number_string[64];
                    snprintf(number_string, sizeof(number_string), "%.2f", sum);
                    
                    if (!output.empty()) {
                        output += " ";
                    }
                    output += number_string;
                }
            }
            
            strncpy(shared_data->result, output.c_str(), sizeof(shared_data->result) - 1);
            shared_data->result[sizeof(shared_data->result) - 1] = '\0';
        }

        shared_data->is_processed = true;
        sem_post(semaphore_server);
    }

    sem_close(semaphore_server);
    sem_close(semaphore_client);
    munmap(shared_data, sizeof(SharedData));
    close(shared_memory_file_descriptor);

    return EXIT_SUCCESS;
}
