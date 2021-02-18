#include <iostream>
#include <cmath>
#include <signal.h>
#include <unistd.h>
#include <csignal>
#include <sys/time.h>
#include <cstdlib>
#include <ctime>

using namespace std;

#define _XOPEN_SOURCE
#define _XOPEN_SOURCE_EXTENDED

int signali[] = {SIGUSR1, SIGUSR2, SIGPIPE, SIGTRAP}; //svi osim SIGINT
int pid = 0;

void prekidna_rutina(int sig)
{
   kill(pid, SIGKILL); /* pošalji SIGKILL procesu 'pid'*/
   exit(0);
}

int main(int argc, char *argv[])
{
   int odabrani_signal;
   srand((unsigned)time(NULL));

   pid = atoi(argv[1]);

   sigset(SIGINT, prekidna_rutina); //kad se uhvati SIGINT posalji SIGKILL preko prekidne rutine

   while (1)
   {
      sleep(rand() % 3 + 3);                 /* odspavaj 3-5 sekundi */
      odabrani_signal = signali[rand() % 5]; /* slučajno odaberi jedan signal (od 4) */
      kill(pid, odabrani_signal);            /* pošalji odabrani signal procesu 'pid' funkcijom kill*/
   }
   return 0;
}