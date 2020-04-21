#pragma clang diagnostic push
#pragma ide diagnostic ignored "openmp-use-default-none"

#include <unistd.h>
#include <fstream>
#include <condition_variable>
#include <mutex>

#include <sdsl/bit_vectors.hpp>
#include "utils.hpp"

class Game_of_Life {
    uint32_t n;
    uint32_t m;
    uint32_t seed;
    float ratio;

    std::vector<sdsl::bit_vector> grid;
    std::vector<sdsl::bit_vector> grid_tmp;

public:
    Game_of_Life(uint32_t _n, uint32_t _m, uint32_t _seed, float _ratio) : n(_n), m(_m), seed(_seed),
                                                                           ratio(_ratio) {
        grid = *new std::vector<sdsl::bit_vector>(n);
        for (auto i = 0; i < n; ++i) {
            grid[i] = *new sdsl::bit_vector(m, 0);
        }

        grid_tmp = *new std::vector<sdsl::bit_vector>(n);
        for (auto i = 0; i < n; ++i) {
            grid_tmp[i] = *new sdsl::bit_vector(m, 0);
        }

        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dis(0, 10);
        for (auto i = 1; i < n - 1; ++i) {
            for (auto j = 1; j < m - 1; ++j) {
                if (dis(gen) < (10 * ratio)) {
                    grid[i][j] = 1;
                }
            }
        }
    }

    Game_of_Life(std::string &filename, uint32_t _n, uint32_t _m) : n(_n), m(_m) {
        grid = *new std::vector<sdsl::bit_vector>(n);
        for (auto i = 0; i < n; ++i) {
            grid[i] = *new sdsl::bit_vector(m, 0);
        }

        grid_tmp = *new std::vector<sdsl::bit_vector>(n);
        for (auto i = 0; i < n; ++i) {
            grid_tmp[i] = *new sdsl::bit_vector(m, 0);
        }

        std::ifstream file(filename);
        std::vector<std::pair<int, int>> pairs;
        if (file.is_open()) {
            std::string line;

            while (std::getline(file, line)) {
                std::vector<std::string> splitted;
                split1(line, splitted);
                int x = atoi(splitted[0].c_str());
                int y = atoi(splitted[1].c_str());
                // TODO change n and m accordingly if not big enough
                pairs.emplace_back(std::make_pair(x + m / 2, y + n / 2));
                //std::cout <<  line << std::endl;
            }
            file.close();
        }

        for (auto pair : pairs) {
            grid[pair.second][pair.first] = 1;
        }
    }

    void print() {
        for (auto i = 0; i < n; ++i) {
            for (auto j = 0; j < m; ++j) {
                if (grid[i][j])
                    std::cout << "O";
                else
                    std::cout << "_";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
        for (auto i = 0; i < m + 10; ++i) {
            std::cout << "-";
        }
        std::cout << std::endl;
    }

    void step_seq(uint32_t n_step = 1) {
        for (auto step = 0; step < n_step; ++step) {
            for (auto i = 1; i < n - 1; ++i) {
                for (auto j = 1; j < m - 1; ++j) {
                    compute_step(i, j);
                }
            }
            grid.swap(grid_tmp);
        }
    }

    using pair = std::pair<size_t, size_t>;

    void step_par_th(uint32_t nw, uint32_t n_step = 1) {
        std::vector<pair> ranges(nw);                     // vector to compute the ranges
        uint delta{n / nw};
        for (int i = 0; i < nw; ++i) {                     // split the rows into pieces
            ranges[i].first = i * delta;
            ranges[i].second = (i != (nw - 1) ? ((i + 1) * delta - 1) : (n - 1));
        }
        ranges.front().first++;
        ranges.back().second--;

        std::vector<std::thread> tids;
        std::mutex mutex;
        std::condition_variable cond_var_thread;
        std::condition_variable cond_var_main;
        uint done_threads = 0;

        auto compute_chunk = [&](pair p) {
            for (auto step = 0; step < n_step; ++step) {
                for (auto i = p.first; i <= p.second; ++i) {
                    for (auto j = 1; j < m - 1; ++j) {
                        compute_step(i, j);
                    }
                }
                {
                    std::unique_lock<std::mutex> lock(mutex);
                    done_threads++;
                    cond_var_main.notify_one();
                    cond_var_thread.wait(lock);
                }
            }
        };

        for (auto i = 0; i < nw; ++i) {                     // assign chuncks to threads
            tids.push_back(std::thread(compute_chunk, ranges[i]));
        }

        for (auto step = 0; step < n_step; ++step) {
            {
                std::unique_lock<std::mutex> lock(mutex);
                while (done_threads < nw)
                    cond_var_main.wait(lock);
                done_threads = 0;
            }
            grid.swap(grid_tmp);
            cond_var_thread.notify_all();
        }
        cond_var_thread.notify_all();

        for (std::thread &t: tids) {
            t.join();
        }

        tids.clear();
    }


    void step_par_omp(uint32_t nw, uint32_t n_step = 1) {
        for (auto step = 0; step < n_step; ++step) {
#pragma omp parallel for num_threads(nw) schedule(auto)
            for (auto i = 1; i < n - 1; ++i) {
                for (auto j = 1; j < m - 1; ++j) {
                    compute_step(i, j);
                }
            }
            grid.swap(grid_tmp);
        }
    }

    __attribute__((__always_inline__)) void compute_step(uint32_t i, uint32_t j) {
        int8_t n_neighbours = 0;
        n_neighbours += grid[i - 1][j - 1] + grid[i - 1][j] + grid[i - 1][j + 1];
        n_neighbours += grid[i][j - 1] + grid[i][j + 1];
        n_neighbours += grid[i + 1][j - 1] + grid[i + 1][j] + grid[i + 1][j + 1];

        if (n_neighbours == 2) {
            grid_tmp[i][j] = grid[i][j];
        } else if (n_neighbours == 3) {
            grid_tmp[i][j] = 1;
        } else {
            grid_tmp[i][j] = 0;
        }
    }

    void reset() {
        std::mt19937 gen(seed);
        std::uniform_int_distribution<> dis(0, 10);
        for (auto i = 1; i < n - 1; ++i) {
            for (auto j = 1; j < m - 1; ++j) {
                if (dis(gen) < (10 * ratio)) {
                    grid[i][j] = 1;
                }
            }
        }
    }

};

#pragma clang diagnostic pop