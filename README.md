# A comparison between my Game of Life parallel implementations.

My sequential and parallel implementation of the [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) game.

<p align="center">
  <img width="460" height="300" src="https://github.com/aboffa/Parallel_Game_of_Life/blob/master/plots/bigun.png">
</p>

I use the library [sdsl-lite](https://github.com/simongog/sdsl-lite) in order to use their bit-vector, just because I am used to use it.
To install it:

```

git submodule update --init --recursive

cd ./lib/sdsl-lite

./install .

```

I implemented a single *C++* class **Game_of_Life** with 3 main methods:

1. step_sequential( number_of_iteration )
2. step_parallel_open_mp( number_of_iteration, number_of_workers )
3. step_parallel_pthread( number_of_iteration, number_of_workers )

The first one is just a simple implementation of a game of life step, that is: compute the number of
neighbors, change the “aliveness” of the corresponding cell in a temporary grid. In the end I swap
the two grid.

The second one is trivial, it uses [OpenMP](https://www.openmp.org/). I just copy and paste *step_sequential( )* and add:

 ```
#pragma omp parallel for num_threads(nw) schedule(auto)
```
 
in front of the right for loop.

The third one is more involving, it uses [Pthread](https://pubs.opengroup.org/onlinepubs/7908799/xsh/pthread.h.html). I use two condition variable and a mutex to make the thread wait the completion of all the others at the end of each step. Then the main does the swap. Done the swap all the threads are awakened ( *notify_all( )* )
and they can continue the new iteration.

I tested the sequential implementation and then the two parallel implementations for each number of worker between 1 and 64.


Here there is the plots of the speed-up of the implementation with Open MP: 

<p align="center">
  <img width="560" height="400" src="https://github.com/aboffa/Parallel_Game_of_Life/blob/master/plots/open-mp.png">
</p>

Here there is the plots of the speed-up of the implementation with Pthread: 

<p align="center">
  <img width="560" height="400" src="https://github.com/aboffa/Parallel_Game_of_Life/blob/master/plots/pthread.png">
</p>
