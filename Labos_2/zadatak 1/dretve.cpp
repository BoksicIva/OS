#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <cstdio>
#include <csignal>
#include <cstdlib>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int brojDretvi, brojPonavljanja;
int A = 0;

void *Dretva(void *x)
{
   for (int j = 0; j < brojPonavljanja; j++)
      A++; //povecavanje vrijednosti
}

int main(int argc, char *argv[])
{

   brojDretvi = atoi(argv[1]); //citanje vrijednosti iz terminala
   brojPonavljanja = atoi(argv[2]);

   pthread_t dretva_id[brojDretvi];

   for (int i = 0; i < brojDretvi; i++)
   {
      if (pthread_create(&dretva_id[i], NULL, Dretva, NULL)) //stvaranje dretvi
      {
         cout << "Nije moguce stavranje nove dretve." << endl;
         exit(1);
      }
   }

   for (int i = 0; i < brojDretvi; i++)
   {
      pthread_join(dretva_id[i], NULL); //cekanje kraja dretvi
   }

   cout << "A=" << A << endl;
   return 0;
}
