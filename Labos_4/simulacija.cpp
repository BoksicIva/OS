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


using namespace std;

int t; //simulacija vremena (trenutno vrijeme)
int ima; //koliko ukupno ima dretvi koje ce doci tijekom vremena
int brojacDretviUSustavu; //broji koliko je dretvi u polju P tj. koliko ih je treuntuno u sustavu u obradi ili cekaju obradu

struct dretva {
	int id; //1, 2, 3, ...
	int p;  //preostalo vrijeme rada
	int prio; //prioritet
	int rasp; //način raspoređivanja (za opcionalni zadatak)
};

struct dretva *nova_dretva;


#define MAX_DRETVI	5
struct dretva *P[MAX_DRETVI]; //red pripravnih dretvi, P[0] = aktivna dretva

/* podaci o događajima pojave novih poslova - dretvi */
#define BR_DRETVI	6
int nove[BR_DRETVI][5] =
{
	/* trenutak dolaska, id, p, prio, rasp (prio i rasp se koriste ovisno o rasporedivacu) */
	{ 1,  3, 5, 3, 1 }, /* rasp = 0 => PRIO+FIFO; 1 => PRIO+RR */
	{ 3,  5, 6, 5, 1 },
	{ 7,  2, 3, 5, 0 },
	{ 12, 1, 5, 3, 0 },
	{ 20, 6, 3, 6, 1 },
	{ 20, 7, 4, 7, 1 },
};

void ispis_stanja ( int ispisi_zaglavlje )
{
	int i;
	if ( !ispisi_zaglavlje ) {  // ako je nulta sekunda onda treba i zaglavlje ispisati
		cout<<"  t    AKT" ;
		for ( i = 1; i < MAX_DRETVI; i++ )
			cout<< "     PR"<< i ;
		cout<<"\n" ;
	}

	printf("%3d",t);                
	for ( i = 0; i < MAX_DRETVI; i++ )
		if ( P[i] != NULL )
			cout<<"   "<<P[i]->id<<"/"<< P[i]->prio<<"/"<< P[i]->p ;
		else
			cout<<"   -/-/-" ;
	cout<<endl;
}

void dodaj_u_pripravne(struct dretva *nd){ //f-ja za dodavanje u red pripravnih
    P[brojacDretviUSustavu]=nd; // za pocetak dodajemo na kraj reda, ali odmah potom sortiramo po prioritetu
    brojacDretviUSustavu++; // povecava se broj dretvi u sustavu 
    struct dretva* pom; // pomocna dretva koja nam sluzi za sortiranje
    for(int i=0; i< brojacDretviUSustavu-1;i++){
         for(int j=i+1; j< brojacDretviUSustavu;j++){  // sortiranje dretvi prema prioritetu, najveci prioritet "najjaci"
             if(P[i]->prio < P[j]->prio){  //boouble sort
                    pom=P[i];
                    P[i]=P[j];              
                    P[j]=pom;
                }
         }
         }
    ispis_stanja(1); //  ispisuje stanje nakon svakog dodavanja nove dretve 
    
}


void ispis_nove(struct dretva *nd){// f-ja za ispis da je dosla nova dretva u vremenu t
    printf("%3d",t);
    cout<<" -- nova dretva id="<<nd->id<<", p="<<nd->p<<", prio="<<nd->prio<<endl;
}

void dretva_zavrsila(){
    cout<<"Dretva "<<P[0]->id<<" zavrsila."<<endl; // moze zavsiti samo aktivna dretva
    free(P[0]);
    for(int i =0;i<brojacDretviUSustavu-1;i++)  
        P[i]=P[i+1];      // posmice se cijeli red za 1 ulijevo
    P[brojacDretviUSustavu-1]=nullptr; //zadnje dretve nema jer je prva zavrsila i sve su se pomakle ulijevo
    brojacDretviUSustavu--;//smanjuje se brojac dretvi
    ima--;//smanjuje se brojac koliko ukupno dretvi ocekujemo 
}

void brisi_memoriju(int sig){
     for(int i =0;i< brojacDretviUSustavu;i++)
        free(P[i]);
    exit(0);
}


int main (void) {
    
    ima=BR_DRETVI;  // stavila sam da ce ukupno biti dretvi koliko je zadano u polju pa mi program i zavrsava kad zavrsi zadnja dretva
    t=0;
    brojacDretviUSustavu=0; //na pocetku programa pretpostavljamo da nema dretvi
    nova_dretva=nullptr;
    
    sigset(SIGINT,brisi_memoriju);

    while(ima >0 ){

        ispis_stanja(t);

       
        for(int i=0;i< BR_DRETVI;i++){
            if(nove[i][0]==t){
            //dodaj novu dretvu:
                nova_dretva=(struct dretva*)malloc(sizeof(struct dretva));
                nova_dretva->id=nove[i][1];
                nova_dretva->p=nove[i][2];
                nova_dretva->prio=nove[i][3];
                nova_dretva->rasp=nove[i][4];
                ispis_nove(nova_dretva);
                dodaj_u_pripravne(nova_dretva);
            }
           
        }
        

        if(P[0] != nullptr){
            P[0]->p--; //samanjujem vrijeme trajanja aktivne dretve
            if(P[0]->p==0)   // je li gotova dretva
                dretva_zavrsila();
        }

        sleep(1);
        t++; // povećavanje brojača proteklog vremena
    }
    
   
    brisi_memoriju(0);

    return 0;
    
}