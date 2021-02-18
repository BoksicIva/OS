#include <signal.h>
#include <iostream>
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

int ID; //id zajednickog spremista
int *A;
int brojPonavljanja, brojProcesa;

void Proces(int i)
{
   for (int j = 0; j < brojPonavljanja; j++)
      *A += 1; //povecavanje vrijedosti preko pokazivaca
               //cout<<*A<<endl;   ---->  provjera kako se povecava vrijednost A
}

void brisi_memoriju(int signal) //f-ja za brisanje zauzete memorije koristenjem f-ja shmget
{
   shmdt((char *)A);
   shmctl(ID, IPC_RMID, NULL);
}

int main(int argc, char *avrg[])
{

   brojProcesa = atoi(avrg[1]); //citanje vrijednosti iz terminala
   brojPonavljanja = atoi(avrg[2]);
   ID = shmget(IPC_PRIVATE, sizeof(int), 0600); //zauzimanje zajednicke memorije

   if (ID == -1)
   {
      cout << "Nije moguće zauzeti memoriju" << endl;
      exit(1);
   }

   A = (int *)shmat(ID, NULL, 0);
   *A = 0; //nuliranje vrijednsti zajednice varijabe

   sigset(SIGINT, brisi_memoriju); //u slucaju prekida signalom SIGINT brisi memoriju

   for (int i = 0; i < brojProcesa; i++)
   {
      switch (fork())
      {
      case 0:
         Proces(i); //stavranje procesa
         exit(0);
      case -1:
         cout << "Nije moguće stvaranje novog procesa" << endl;
         exit(1);
      }
   }

   for (int i = 0; i < brojProcesa; i++)
      wait(NULL); //cekanje na zavrsetak svih procesa i tek onda zavrsiti s glavnim procesom -> main funkcija

   cout << "A=" << *A << endl;
   brisi_memoriju(0); //ako nije bilo prekida svejdno obrisi memoriju
   return 0;
}
