#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <omp.h>

int main(int argc, char **argv) {
  //int i = 10;
  #pragma omp parallel shared(i) num_threads(2)
  {
    #pragma omp for 
      for (int i=0; i<10; i++) {
        printf("thrad: %d, i: %d \n", omp_get_thread_num(), i);
      }
  }

  /*
  #pragma omp parallel
  omp_set_num_threads(2);
  #pragma omp parallel for

  for (int i=0; i<10; i++) {
    printf("thread: %d durchgang: %d \n", omp_get_thread_num(), i);
  }
  */
  /*
  int a, b = 0;
  #pragma omp parallel for shared(b,a)
  for (a=0; a<5; ++a) {
    #pragma omp atomic
    b += a;
    printf("thread: %d durchgang: %d b: %d\n", omp_get_thread_num(), a, b);
  }
  return 0;
  */
  /*
  #pragma omp parallel for
  for (int i=0; i<4; i++) {
    printf("Hello World from thread %d of %d\n", omp_get_thread_num(), omp_get_num_threads());
  }
  return 0;
  */
}
