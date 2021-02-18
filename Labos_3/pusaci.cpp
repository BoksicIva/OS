#include <iostream>
#include <sys/types.h>
#include <sys/shm.h>
#include <cstdlib>
#include <sys/wait.h>
#include <unistd.h>
#include <ctime>
#include <sys/sem.h>
#include <cstring>
#include <csignal>

#include <semaphore.h>

using namespace std;

int brojPusaca=3;
int ID;
int r1, r2;
int *s1 , *s2 ; //sastojci 1 i 2
string sastojci[3] = {"papir", "duhan", "sibice"};
sem_t *KO;           //semafor za ulazak u kriticni odsjecak
sem_t *p1, *p2, *p3; //semafori pusaca
sem_t *stol_prazan; //semafor koji provjerava je li stol prazan

bool na_stolu_sastojci(int sastojak1, int sastojak2)
{
   if ((*s1 == sastojak1 && *s2 == sastojak2)||(*s1==sastojak2 && *s2==sastojak1))
      return true;
   else
      return false;
}

void stavi_sastojke_na_stol(int sastojak1, int sastojak2)
{
   *s1 = sastojak1;
   *s2 = sastojak2;
   
}

void smotaj_zapali_pusi(int p)
{
   cout << "Pusac " << p << ": uzima sastojke i mota cigaretu pali ju i pusi." << endl<<endl;
  
}

void uzmi_sastojke()
{
   *s1 = 0;
   *s2 = 0;
}

void Trgovac()
{
   while (true)
   {
      // 1.) (s1, s2) = nasumice_odaberi_dva_različita_sastojka
      int sastojak1 = (rand() % 3) + 1; //nasumicno odabran prvi sastojak
      int sastojak2;
      while (true)
      {
         sastojak2 = (rand() % 3) + 1; //nasumicno odabran drugi sastojak
         if (sastojak2 != sastojak1)              //drugi sastojak se odabire sve dok ne bude razlicit od prvog
            break;
      }

      //ČekajBSEM(KO)
      sem_wait(KO);

      //stavi_sastojke_na_stol(s1, s2)
      cout << "Trgovac stavlja: " << sastojci[sastojak1 - 1] << " i " << sastojci[sastojak2 - 1] << endl;
      stavi_sastojke_na_stol(sastojak1, sastojak2);
     

      //PostaviBSem(KO)
      sem_post(KO);
     
      /*PostaviBSem(p1)  PostaviBSem(p2) PostaviBSem(p3)*/
      
      sem_post(p1);
      sem_post(p2);
      sem_post(p3);

      //ČekajBSem(stol_prazan)
      sem_wait(stol_prazan);
   }
}

void Pusac(int p)
{
   //(r1, r2) = sastojci_koje_pušač_nema(p)
   if (p == 1)
   {
      r1 = 2;
      r2 = 3;
   }
   else if (p == 2)
   {
      r1 = 1;
      r2 = 3;
   }
   else
   {
      r1 = 1;
      r2 = 2;
   }

   while (true)
   {
      //ČekajBSem(p)
      if (p == 1)
         sem_wait(p1);
      if (p == 2)
         sem_wait(p2);
      if (p == 3)
         sem_wait(p3);

      //ČekajBSEM(KO)
      sem_wait(KO);

      //ako(na_stolu_sastojci(r1, r2) = DA){
      if (na_stolu_sastojci(r1, r2))
      {
         uzmi_sastojke();
         smotaj_zapali_pusi(p);

         //PostaviBSem(KO)
         sem_post(KO);

         
         //PostaviBSem(stol_prazan)
         sem_post(stol_prazan);
         
         sleep(5);
      }
      else
      {
         //PostaviBSem(KO)
         sem_post(KO);
      }
   }
}

void brisi_memoriju(int id){
   sem_destroy(KO);
   
   sem_destroy(p1);
   sem_destroy(p2);
   sem_destroy(p3);
   sem_destroy(stol_prazan);
   shmdt(KO);
}



int main(void)
{
   cout << "Pusac 1: ima papir" << endl;
   cout << "Pusac 2: ima duhan" << endl;
   cout << "Pusac 3: ima sibice" << endl;

   srand((unsigned)time(0));

   sigset(SIGINT,brisi_memoriju);

   ID = shmget(IPC_PRIVATE, 5*sizeof(sem_t) + 2*sizeof(int), 0600);
   KO =(sem_t *)shmat(ID, NULL, 0);
   shmctl(ID, IPC_RMID, NULL); //moze odmah ovdje, nakon shmat, ili na kraju nakon shmdt

   p1=KO+1;
   p2=p1+1;
   p3=p2+1;
   stol_prazan=p3+1;

   s1=(int *)(stol_prazan+1);
   s2=(int *)(s1+1);
   *s1=0;
   *s2=0;

   sem_init(KO, 1, 1); //početna vrijednost = 1, 1=>za procese
   sem_init(p1, 1, 0);
   sem_init(p2, 1, 0);
   sem_init(p3, 1, 0);
   sem_init(stol_prazan, 1, 0);

   

   switch (fork()){
      case 0:
         Trgovac();
         exit(0);
      case -1:
         cout << "Nije moguće stvaranje novog procesa" << endl;
         exit(1);
      }

     for (int i = 1; i <= brojPusaca; i++){
      switch (fork())
      {
      case 0:
         Pusac(i);
         exit(0);
      case -1:
         cout << "Nije moguće stvaranje novog procesa" << endl;
         exit(1);
      }
   }

   for(int i =0;i <4;i++)
      wait(NULL);


   
   //brisi_memoriju(0);//zapravo ne treba jer se program beskonacno vrti i mogemo ga jedino s SIGINTom prekinuti
   return 0;
}