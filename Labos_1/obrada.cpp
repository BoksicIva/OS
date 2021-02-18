#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <csignal>

using namespace std;

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

#define N 6 //broj razina

int oznaka_cekanja[N];
int prioritet[N];
int tekuci_prioritet;

int signali[] = {SIGUSR1, SIGUSR2, SIGPIPE, SIGTRAP, SIGINT}; //proizvoljan izbor signala

void zabrani_prekidanje()
{
   for (int i = 0; i < 5; i++) //zabrani prekidanje svim signalima
      sighold(signali[i]);
}

void dozvoli_prekidanje()
{
   for (int i = 0; i < 5; i++) //dozvoli prekidanje svim signalima
      sigrelse(signali[i]);
}

void prikaz_signala(int signal, char x) //zadano u pripremi
{
   switch (signal)
   {
   case 0:
      cout << x << " - - - - - " << endl;
      break;
   case 1:
      cout << "- " << x << " - - - -" << endl;
      break;
   case 2:
      cout << "- - " << x << " - - -" << endl;
      break;
   case 3:
      cout << "- - - " << x << " - -" << endl;
      break;
   case 4:
      cout << "- - - - " << x << " -" << endl;
      break;
   case 5:
      cout << "- - - - - " << x << endl;
      break;
   }
}

void obrada_signala(int signal)
{
   prikaz_signala(signal, 'P'); //pocetak signala
   for (int i = 1; i <= 5; i++)
   {
      prikaz_signala(signal, i + 48); //trajanje signala 5 sek uz prikaz svake sekunde
      sleep(1);
   }
   prikaz_signala(signal, 'K'); //kraj signala
}

void prekidna_rutina(int signal)
{
   int n;
   zabrani_prekidanje();
   for (int i = 0; i < N; i++)  //prikaz pocetka prekida signala
   {                            //ali ne i cijele njegove obrade
      if (signali[i] == signal) //jer ovisi o prioritetu
      {
         prikaz_signala(i + 1, 'X');
         n = i + 1;
         break;
      }
   }
   oznaka_cekanja[n] += 1;

   int x;
   do
   {
      x = 0;
      for (int j = tekuci_prioritet + 1; j < N; j++)
         if (oznaka_cekanja[j] != 0)
            x = j; //trazenje indeksa signala najveceg prioriteta koji ceka na obradu

      if (x > 0)
      {
         oznaka_cekanja[x] -= 1;          //umanjuje se jer x signal upravo krecemo obavljati vise ne ceka na red
         prioritet[x] = tekuci_prioritet; //pohrana "konteksta" -> tekuci prioritet
         tekuci_prioritet = x;            //pamcenje trenutnog najveceg prioriteta
         dozvoli_prekidanje();
         obrada_signala(x); //obavit obradu prekida razine x
         zabrani_prekidanje();
         tekuci_prioritet = prioritet[x]; //obnavljanje konteksta
      }
   } while (x > 0); //obrada svih signala veceg prioriteta od onog koji se obraduje
}

int main(void)
{
   sigset(SIGUSR1, prekidna_rutina);
   sigset(SIGUSR2, prekidna_rutina);
   sigset(SIGPIPE, prekidna_rutina);
   sigset(SIGTRAP, prekidna_rutina);
   sigset(SIGINT, prekidna_rutina);

   cout << "Proces obrade prekida, PID=" << getpid() << endl;
   cout << "G 1 2 3 4 5" << endl;
   cout << "-----------" << endl;

   /* troši vrijeme da se ima šta prekinuti - 10 s */

   while (true) //beskonacna petlja
   {
      int i = 0;
      while (i < 10)
      {
         prikaz_signala(0, i + 48); //asci 0=48, prikaz brojeva na zaslonu
         sleep(1);
         i++;
      }
   }
   cout << "Zavrsio osnovni program\n";
   return 0;
}