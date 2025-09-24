#include <unistd.h>

#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>

int main() {
    std::string line;
    char ch;
    while (true) {
        ssize_t n = read(STDIN_FILENO, &ch, 1);
        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (n == 0) {
            // EOF
            if (line.empty()) {
                break;
            }
        } else {
            if (ch != '\n') {
                line.push_back(ch);
                continue;
            }
        }

        std::vector<float> nums;
        std::string cur;
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];
            if (c == ' ') {
                if (!cur.empty()) {
                    nums.push_back(std::atof(cur.c_str()));
                    cur.clear();
                }
            } else {
                cur.push_back(c);
            }
        }
        if (!cur.empty()) {
            nums.push_back(std::atof(cur.c_str()));
            cur.clear();
        }

        if (!nums.empty()) {
            float sum = 0;
            for (size_t i = 0; i < nums.size(); ++i) {
                sum += nums[i];
            }
            std::string out = std::to_string(sum) + "\n";
            size_t woff = 0;
            while (woff < out.size()) {
                ssize_t w = write(STDOUT_FILENO, out.c_str() + woff,
                                  out.size() - woff);
                if (w < 0) {
                    if (errno == EINTR) {
                        continue;
                    }
                    perror("write");
                    return EXIT_FAILURE;
                }
                woff += w;
            }
        }

        line.clear();
        if (n == 0) {
            break;
        }
    }

    exit(EXIT_SUCCESS);
}
