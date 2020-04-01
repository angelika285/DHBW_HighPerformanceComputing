#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <omp.h>

#define calcIndex(width, x,y)  ((y)*(width) + (x))

long TimeSteps = 100;

void writeVTK2(long timestep, double *data, char prefix[1024], int w, int h, int threadx, int thready) {

  char filename[2048];  
  int x,y; 
  
  int threads = threadx*thready;
  float deltax = 1.0;
  long  nxy = w * h * sizeof(float)/threads;

  int sizex = w / threadx;
  int sizey = h / thready;

  #pragma omp parallel num_threads(threads)
  {
   int number = omp_get_thread_num();
    int indexy = (number / threadx);
    int indexx = number % threadx;
      
    snprintf(filename, sizeof(filename), "%d:%s-%05ld%s" ,number, prefix, timestep, ".vti");
    FILE* fp = fopen(filename, "w");
    //out/%d:%s-%05ld%s
    //"%s-%05ld%s"

    fprintf(fp, "<?xml version=\"1.0\"?>\n");
    fprintf(fp, "<VTKFile type=\"ImageData\" version=\"0.1\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
    fprintf(fp, "<ImageData WholeExtent=\"%d %d %d %d %d %d\" Origin=\"%d %d 0\" Spacing=\"%le %le %le\">\n",0, sizex, 0, sizey,0 ,0, indexx*sizex+sizex, indexy*sizey+sizey, deltax, deltax, 0.0);
    fprintf(fp, "<CellData Scalars=\"%s\">\n", prefix);
    fprintf(fp, "<DataArray type=\"Float32\" Name=\"%s\" format=\"appended\" offset=\"0\"/>\n", prefix);
    fprintf(fp, "</CellData>\n");
    fprintf(fp, "</ImageData>\n");
    fprintf(fp, "<AppendedData encoding=\"raw\">\n");
    fprintf(fp, "_");
    fwrite((unsigned char*)&nxy, sizeof(long), 1, fp);

    for (y = indexy*sizey; y < (indexy*sizey)+sizey; y++) {
      for (x = indexx*sizex; x < (indexx*sizex)+sizex; x++) {
        float value = data[calcIndex(h ,x,y)];
        fwrite((unsigned char*)&value, sizeof(float), 1, fp);
      }
    }
    
    fprintf(fp, "\n</AppendedData>\n");
    fprintf(fp, "</VTKFile>\n");
    fclose(fp);
  }
}

void show(double* currentfield, int w, int h) {
  printf("\033[H");
  int x,y;
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) printf(currentfield[calcIndex(w, x,y)] ? "\033[07m  \033[m" : "  ");
    //printf("\033[E");
    printf("\n");
  }
  fflush(stdout);
}
 
 int checkNeighbors (double* currentfield, int w, int x, int y) {
  int neighbors = 0;
  neighbors += currentfield[calcIndex(w, x-1, y)];
  neighbors += currentfield[calcIndex(w, x+1, y)];
  neighbors += currentfield[calcIndex(w, x, y-1)];
  neighbors += currentfield[calcIndex(w, x, y+1)];
  neighbors += currentfield[calcIndex(w, x-1, y-1)];
  neighbors += currentfield[calcIndex(w, x-1, y+1)];
  neighbors += currentfield[calcIndex(w, x+1, y-1)];
  neighbors += currentfield[calcIndex(w, x+1, y+1)];
  return neighbors;
}

void evolve(double* currentfield, double* newfield, int w, int h, int threadx, int thready) {
  int x,y;
  
  int sizex= w / threadx;
  int sizey= h / thready;
  int threads = threadx*thready;

  #pragma omp parallel shared(newfield) num_threads(threads)
  {
    int num = omp_get_thread_num();
    int indexy = num/(threadx);
    int indexx = num % threadx;
      //printf("\nindexy: %d, indexx: %d\n", indexy, indexx);

    for (y = sizey*indexy; y < sizey*indexy+sizey; y++) {
      for (x = sizex*indexx; x < sizex*indexx+sizex; x++) {
      
      //TODO FIXME impletent rules and assign new value


        int neighbors = checkNeighbors(currentfield, w, x, y);

        if (currentfield[calcIndex(w,x,y)] == 0 && neighbors == 3) {
          newfield[calcIndex(w, x,y)] = 1;
        }
        if (currentfield[calcIndex(w,x,y)] == 0 && neighbors < 3 || neighbors > 3) {
          newfield[calcIndex(w, x,y)] = 0;
        }
        if (currentfield[calcIndex(w,x,y)] == 1 && neighbors < 2 || neighbors > 3) {
          newfield[calcIndex(w, x,y)] = 0;
        }
        if (currentfield[calcIndex(w,x,y)] == 1 && neighbors == 2 || neighbors == 3) {
          newfield[calcIndex(w, x,y)] = 1;
        }
      }
    }
  }
}
 
void readfile(double* currentfield, int w, int h) {
  int i = 0;
  int c;
  FILE *file;
  file = fopen("initial.txt", "r");
  if (file) {
    while ((c=getc(file)) != EOF){
      if(c == 48) {
        currentfield[i] = 0;
        i++;
      }
      if(c == 49) {
        currentfield[i] = 1;
        i++;
      }    
    }
    fclose(file);
  }
}

void filling(double* currentfield, int w, int h) {
  int i;
  for (i = 0; i < h*w; i++) {
    currentfield[i] = (rand() < RAND_MAX / 10) ? 1 : 0; ///< init domain randomly
  }
}
 
void game(int w, int h, int threadx, int thready) {
  double *currentfield = calloc(w*h, sizeof(double));
  double *newfield     = calloc(w*h, sizeof(double));
  
  //printf("size unsigned %d, size long %d\n",sizeof(float), sizeof(long));
  
  //filling(currentfield, w, h);
  readfile(currentfield, w, h);
  long t;
  for (t=0;t<TimeSteps;t++) {
    show(currentfield, w, h);
    evolve(currentfield, newfield, w, h, threadx, thready);
    printf("%ld timestep\n",t);
    writeVTK2(t,currentfield, "gol", w, h, threadx, thready);    
    
    usleep(200000);

    //SWAP
    double *temp = currentfield;
    currentfield = newfield;
    newfield = temp;
  }
  
  free(currentfield);
  free(newfield);
}
 
int main(int c, char **v) {
  int w = 0, h = 0, threadx=2, thready=2;
  if (c > 1) w = atoi(v[1]); ///< read width
  if (c > 2) h = atoi(v[2]); ///< read height
  if (c > 3) threadx = atoi(v[3]);
  if (c > 4) thready = atoi(v[4]);
  if (w <= 0) w = 10; ///< default width
  if (h <= 0) h = 10; ///< default height
  game(w, h, threadx, thready);
}
