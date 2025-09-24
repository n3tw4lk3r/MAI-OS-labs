#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>

#include <string>

int main() {
    const size_t BUF_SIZE = 4096;
    std::string filename;
    char buf[BUF_SIZE];

    while (true) {
        ssize_t n = read(STDIN_FILENO, buf, 1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            break; // got EOF
        }

        if (buf[0] == '\n') {
            break;
        }
        filename.push_back(buf[0]);
        if (filename.size() >= BUF_SIZE) {
            const char *msg = "Filename is too long.\n";
            write(STDERR_FILENO, msg, strlen(msg));
            exit(EXIT_FAILURE);
        }
    }

    if (filename.empty()) {
        const char *msg = "Please provide filename.\n";
        write(STDERR_FILENO, msg, strlen(msg));
        exit(EXIT_SUCCESS);
    }

    int fd_in = open(filename.c_str(), O_RDONLY);
    if (fd_in < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    int pipefd[2];
    if (pipe(pipefd) != 0) {
        perror("pipe");
        close(fd_in);
        exit(EXIT_FAILURE);
    }

    const pid_t pid = fork();
    switch (pid) {
    case -1: {
        perror("fork");
        close(fd_in);
        close(pipefd[0]);
        close(pipefd[1]);
        exit(EXIT_FAILURE);
    } break;

    case 0: {
        // child
        if (dup2(fd_in, STDIN_FILENO) < 0) {
            _exit(EXIT_FAILURE);
        }
        if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
            _exit(EXIT_FAILURE);
        }
        if (dup2(pipefd[1], STDERR_FILENO) < 0) {
            _exit(EXIT_FAILURE);
        }

        close(fd_in);
        close(pipefd[0]);
        close(pipefd[1]);

        execl("./child", "./child", (char*)NULL);
        perror("execl");
        _exit(EXIT_FAILURE);
    } break;

    default: {
        // parent
        close(fd_in);
        close(pipefd[1]);

        const size_t RBUF = 4096;
        char buf[RBUF];
        bool had_error = false;
        while (true) {
            ssize_t r = read(pipefd[0], buf, RBUF);
            if (r < 0) {
                if (errno == EINTR) {
                    continue;
                }
                perror("read");
                had_error = true;
                break;
            }
            if (r == 0) {
                break; // got EOF
            }

            ssize_t woff = 0;
            while (woff < r) {
                ssize_t w = write(STDOUT_FILENO, buf + woff, r - woff);
                if (w < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    perror("write");
                    had_error = true;
                    break;
                }
                woff += w;
            }
            if (had_error) {
                break;
            }
        }

        close(pipefd[0]);

        int status = 0;
        if (waitpid(pid, &status, 0) < 0) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (had_error) {
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } break;
    }
}
