#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <atomic>

using namespace std;
int ID;
atomic<int> *A;
atomic<int> *PRAVO;
int brojPonavljanja;
atomic<int> *ZASTAVICA; //ZASTAVICA[2]

void udi_u_KO(int i, int j)
{
   ZASTAVICA[i] = 1;
   while (ZASTAVICA[j] != 0)
   {
      if (*PRAVO == j)
      {
         ZASTAVICA[i] = 0;
         while (*PRAVO == j)
            ;
         ZASTAVICA[i] = 1;
      }
   }
}

void izadi_iz_KO(int i, int j)
{
   *PRAVO = j;
   ZASTAVICA[i] = 0;
}

void Proces(int i)
{
   int j;
   if (i == 0)
      j = 1;
   else
      j = 0;
   for (int x = 0; x < brojPonavljanja; x++)
   {
      udi_u_KO(i, j); //udi u kritični odsječak
      *A = *A + 1;
      //cout<<*A<<endl;    -----> provjera povecava li se A pravilno odnosno 1+1+1...
      izadi_iz_KO(i, j); //izadi iz  kritičnog odsječka
   }
}

void brisi_memoriju(int signal) //f-ja za brisanje zauzete memorije s funkcijom shmget
{
   shmdt((char *)A);
   shmdt((char *)ZASTAVICA);
   shmdt((char *)PRAVO);
   shmctl(ID, IPC_RMID, NULL);
}

int main(int argc, char *avrg[])
{

   brojPonavljanja = atoi(avrg[1]); // broj ponavljanja se cita iz terminala

   ID = shmget(IPC_PRIVATE, sizeof(atomic<int>) * 4, 0600); //zauzimanje zajednicke memorije

   if (ID == -1)
   {
      cout << "Nije moguće zauzeti memoriju" << endl;
      exit(1);
   }

   ZASTAVICA = (atomic<int> *)shmat(ID, NULL, 0); //zauzimanje memorije za polje ZASTAVICA
   *ZASTAVICA = 0;                                //inicijalizacija vrijednosti
   *(ZASTAVICA + 1) = 0;

   PRAVO = ZASTAVICA + 2; //+2 jer je ZASTAVICA "polje" duljine 2
   A = ZASTAVICA + 3;

   *PRAVO = 0; //inicijalizacija vrijednosti
   *A = 0;

   sigset(SIGINT, brisi_memoriju); //u slucaju prekida signalom SIGINT brisi memoriju

   for (int i = 0; i < 2; i++) //stvoriti 2 procesa
   {
      switch (fork())
      {
      case 0:
         Proces(i); //stvaranje procesa
         exit(0);
      case -1:
         cout << "Nije moguće stvaranje novog procesa" << endl;
         exit(1);
      }
   }

   for (int i = 0; i < 2; i++)
      wait(NULL); //cekanje na zavrsetak procesa

   cout << "A=" << *A << endl; //ispis trazene vrijednosti
   brisi_memoriju(0);          //olobadanje zauzetih resursa i ako se nije dogodio prekid
   return 0;
}
