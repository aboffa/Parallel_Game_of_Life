#include <thread>
#include <chrono>

#include "include/game_of_life.hpp"


int main(int argc, char *argv[]) {
    if(argc < 4){
        std::cout << "USAGE: " << argv[0] << " n m nw -rand seed ratio" << std::endl;
        std::cout << "OR (there will be some bug): " << argv[0] << " n (>70) m (>250) nw -file filename" << std::endl;
        return 1;
    }
    uint n = atoi(argv[1]); // number of rows
    uint m = atoi(argv[2]); // number of columns
    uint nw = atoi(argv[3]);   // number of workers

    std::string n_str = std::to_string(n);
    std::string m_str = std::to_string(m);
    std::string nw_str = std::to_string(nw);

    std::chrono::system_clock::time_point start;

    if (strcmp(argv[4], "-rand") == 0) {
        int seed = atoi(argv[5]); // seed
        float ratio = atof(argv[6]); // ratio
        std::cout << "type,n_iter,n,m,nw,time(ms),speed-up,scalability" << std::endl;
        uint n_inter = 10;
        long millisec_seq = 0;
        Game_of_Life gol(n, m, seed, ratio);
        start = std::chrono::system_clock::now();
        gol.step_seq(n_inter);
        std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
        millisec_seq = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();

        std::cout << "sequential," << n_inter << "," << n << "," << m << "," << "1," << millisec_seq << ",1,1"
                  << std::endl;
        std::cout << std::endl;
        std::cout << "type,n_iter,n,m,nw,time(ms),speed-up,scalability" << std::endl;
        gol.reset();
        long millisec_open1 = 0;
        for (auto my_nw = 1; my_nw < nw; ++my_nw) {
            start = std::chrono::system_clock::now();
            gol.step_par_omp(my_nw, n_inter);
            std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
            if (my_nw == 1) {
                millisec_open1 = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                std::cout << "openMP," << n_inter << "," << n << "," << m << "," << "1," << millisec_open1 << ","
                          << double(millisec_seq) / double(millisec_open1) << ",1"
                          << std::endl;
            } else {
                auto millisec_open = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                std::cout << "openMP," << n_inter << "," << n << "," << m << "," << my_nw << "," << millisec_open << ","
                          << double(millisec_seq) / double(millisec_open) << ","
                          << double(millisec_open1) / double(millisec_open) << std::endl;
            }
            gol.reset();
        }
        std::cout << std::endl;
        std::cout << "type,n_iter,n,m,nw,time(ms),speed-up,scalability" << std::endl;
        long millisec_th1 = 0;
        for (auto my_nw = 1; my_nw < nw; ++my_nw) {
            start = std::chrono::system_clock::now();
            gol.step_par_th(my_nw, n_inter);
            std::chrono::duration<double> elapsed = std::chrono::system_clock::now() - start;
            if (my_nw == 1) {
                millisec_th1 = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                std::cout << "pthread," << n_inter << "," << n << "," << m << "," << "1," << millisec_th1 << ","
                          << double(millisec_seq) / double(millisec_th1)
                          << ",1" << std::endl;
            } else {
                auto millisec_th = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
                std::cout << "pthread," << n_inter << "," << n << "," << m << "," << my_nw << "," << millisec_th << ","
                          << double(millisec_seq) / double(millisec_th) << ","
                          << double(millisec_th1) / double(millisec_th) << std::endl;
            }
            gol.reset();
        }

    } else {
        std::string filename = std::string(argv[5]);
        std::string filename_and_path = "../pattern_files/" + filename + ".txt";
        Game_of_Life gol(filename_and_path, n, m);
        for (auto i = 0; i < 500; ++i) {
            gol.print();
            gol.step_par_th(2);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    return 0;
}
