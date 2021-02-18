#include <pthread.h>
#include <iostream>
#include <csignal>
#include <ctime>
#include <sys/wait.h>
#include <unistd.h>
#include <malloc.h>

using namespace std;

pthread_mutex_t soba;//monitor
pthread_cond_t red[3];  // red uvjeta prvi za studente, druga dva za partibrejkera
int studentiUSobi=0;
int brojStudenata;
int partibrejkerUSobi=0;
int ukBrojUlazaka;


void *student(void *K){
    //spavaj X milisekundi; // X je slučajan broj između 100 i 500
    int x=rand()%400+100;
    usleep(x*1000);

    //ponavljaj 3 puta 
    for(int i =0; i < 3;i++){

        int rbStudenta=*((int *)K)+1;
        
        pthread_mutex_lock(&soba);

        //uđi u sobu za zabavu ako u sobi nema partibrejkera;
        while(partibrejkerUSobi==1)
            pthread_cond_wait(&red[0],&soba);

        studentiUSobi++;
        
        cout<< "Student "<<rbStudenta<<" je ušao u sobu"<<endl;
        
        pthread_mutex_unlock(&soba);

        if(studentiUSobi>=3)
            pthread_cond_signal(&red[1]);

        
      
        //zabavi se; // spavaj X milisekundi gdje je X slučajan broj između 1000 i 2000
        //u slucaju da je partibrejker u sobi student zeli sto prije izaci pa je manje vrijeme zabave 
        //(profesor Jelenkovic spomenio u videu da mozemo tako napraviti)
        
        if(partibrejkerUSobi==1){
            int x=rand()%500;
            usleep(x*1000);
        }else{
            int x=rand()%1000+1000;
            usleep(x*1000);
        }
        
        //izađi iz sobe za zabavu;
        pthread_mutex_lock(&soba);

        cout<< "Student "<<rbStudenta<<" je izašao iz sobe"<<endl;
        studentiUSobi--;
        ukBrojUlazaka--;
        pthread_mutex_unlock(&soba);

        if(studentiUSobi==0)//ako nema vise studenata u sobi partibrejker izlazi
            pthread_cond_signal(&red[2]);


        //odspavaj X milisekundi; // X je slučajan broj između 1000 i 2000
        x=rand()%1000+1000;
        usleep(x*1000);
   }
   return NULL;
}



void *partibrejker(void *z)
{
   while(ukBrojUlazaka>0){
        //spavaj X milisekundi; // X je slučajan broj između 100 i 1000
        int x=rand()%900+100;
        usleep(x*1000);

        pthread_mutex_lock(&soba);

        //uđi u sobu za zabavu ako su u sobi 3 ili više studenata;
        while(studentiUSobi<3)
               pthread_cond_wait(&red[1],&soba);


        cout<< "Partibrejker je ušao u sobu"<<endl;
        partibrejkerUSobi=1;
        
        pthread_mutex_unlock(&soba);

        //izađi iz sobe ako u sobi nema više studenata;
        
        pthread_mutex_lock(&soba);
        while(studentiUSobi>0)
              pthread_cond_wait(&red[2],&soba);

        cout<< "Partibrejker je izašao iz sobe"<<endl;
        partibrejkerUSobi=0;
        pthread_cond_broadcast(&red[0]);

        pthread_mutex_unlock(&soba);
    }
    return NULL;
}



int main(int argc,char *argv[]){
    srand((unsigned)time(0));
    

    brojStudenata=atoi(argv[1]);//citanje vrijednosti broja studenata iz terminala
    ukBrojUlazaka=brojStudenata*3;//svaki student zeli 3 puta uci na zabavu pa ukupni broj ulazaka je broj studenata * 3
   

    pthread_t dretva_student[brojStudenata];
    pthread_t dretva_partibrejker;

    int br[brojStudenata];//polje velicine broja studenata koje sa

    pthread_mutex_init(&soba,NULL);
    pthread_cond_init(&red[0],NULL);
    pthread_cond_init(&red[1],NULL);
    pthread_cond_init(&red[2],NULL);


    if(pthread_create(&dretva_partibrejker,NULL,partibrejker,NULL)){//stvaranje dretvi
        cout<<"Nije moguce stvaranje nove dretve."<<endl;
        exit(1);
    }

    for(int i=0;i<brojStudenata;i++){
        br[i]=i;
        if(pthread_create(&dretva_student[i],NULL,student,&br[i])){//stvaranje dretvi
        cout<<"Nije moguce stvaranje nove dretve."<<endl;
        exit(1);
        }
    }

    for(int i=0;i<brojStudenata;i++){
        pthread_join(dretva_student[i],NULL);//cekanje kraja svih  dretvi
    }
    pthread_join(dretva_partibrejker,NULL);

    pthread_mutex_destroy(&soba);
    pthread_cond_destroy(&red[0]);
    pthread_cond_destroy(&red[1]);
    pthread_cond_destroy(&red[2]);


    
    return 0;
}