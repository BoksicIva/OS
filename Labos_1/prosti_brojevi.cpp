#include <iostream>
#include <cmath>
#include <signal.h>
#include <unistd.h>
#include <csignal>
#include <sys/time.h>
#include <cstdlib>

using namespace std;

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

int pauza = 0;
unsigned long long int broj = 1000000001;
unsigned long long int zadnjiprost = 1000000001;

int prost(unsigned long long n) // funkcija koja ispituje je
{                               // li poslani broj kao parametar
   unsigned long max = sqrt(n); // prost i vraca 1 ako je

   if (n % 2 == 0)
      return 0;

   for (unsigned long int i = 3; i <= max; i += 2)
      if ((n % i) == 0)
         return 0;

   return 1;
}

void periodicki_ispis(int signal) //periodicki ispisuje proste brojeve
{
   cout << "zadnji prost broj = " << zadnjiprost << endl;
}

void postavi_pauzu(int signal)
{
   pauza = 1 - pauza;
   if (pauza == 1)
      cout << "Pauzirano" << endl;
   else
      cout << "Nastavljeno" << endl;
}

void prekini(int signal) //poziiva se kad se  "uhvati" signal SIGTERM
{
   cout << "SIGTERM : ";     //ispisuje prosti broj
   periodicki_ispis(signal); //te prekida izvrsavanje programa
   exit(0);
}

int main(void)
{

   sigset(SIGINT, postavi_pauzu); //povezati signale s funkcijama
   sigset(SIGTERM, prekini);
   sigset(SIGALRM, periodicki_ispis); // postavi periodicki alarm

   struct itimerval t;

   t.it_value.tv_sec = t.it_interval.tv_sec = 4;   // periodicki postignuto ispisivanje svakih 4sek
   t.it_value.tv_usec = t.it_interval.tv_usec = 0; //u pripremi nije navedeno koliko tocno moramo uzeti

   setitimer(ITIMER_REAL, &t, NULL); //pokretanje sata

   while (1)
   {
      if (prost(broj) == 1) //ispituje je li broj prost
         zadnjiprost = broj;
      broj++;
      while (pauza == 1)
      {
         pause(); // pauzira
      }
   }

   return 0;
}