#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

#define TRYS 5000000
#define THREADS 4

static int throw() {
  double x, y;
  x = (double)rand() / (double)RAND_MAX;
  y = (double)rand() / (double)RAND_MAX;
  if ((x*x + y*y) <= 1.0) return 1;
    
  return 0;
}

int main(int argc, char **argv) {
  int globalCount = 0, globalSamples=TRYS;
  int threadRes[THREADS];

  #pragma omp parallel num_threads(THREADS)
  {
        threadRes[omp_get_thread_num()] = 0;

  	#pragma omp for reduction(+:globalCount)
  	for(int i = 0; i < globalSamples; ++i) {
			int wurf = throw();
			threadRes[omp_get_thread_num()] += wurf;
			globalCount += wurf;
  	}

	#pragma omp for
        for(int i = 0; i < THREADS; ++i) {
                        printf("Thread %d: treffer %d \n ", omp_get_thread_num(), threadRes[omp_get_thread_num()]);
        }
  }

  double pi = 4.0 * (double)globalCount / (double)(globalSamples);
 
  printf("pi is %.9lf\n", pi);
  
  return 0;
}
