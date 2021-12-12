#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "mpi.h"

#define STACJA 1
#define START 2
#define RUCH 3
#define KONIEC_RUCHU 4
#define KATASTROFA 5
#define CHARGING 5000

int ENERGIA = 5000;
int ZATRZYMAJ = 1, NIE_ZATRZYMAJ = 0;
int liczba_procesow;
int nr_procesu;
int ilosc_pociagow;
int ilosc_torow_kolejowych = 4;
int ilosc_zajetych_torow = 0;
int tag = 1;
int wyslij[2];
int odbierz[2];
MPI_Status mpi_status;


void Wyslij(int nr_pociagu, int stan)
{
    wyslij[0] = nr_pociagu;
    wyslij[1] = stan;
    MPI_Send( & wyslij, 2, MPI_INT, 0, tag, MPI_COMM_WORLD);
    sleep(1);
}
void Stacja(int liczba_procesow) {
    int nr_pociagu, status;
    ilosc_pociagow = liczba_procesow - 1;
    printf("Zyczymy Panstwu, przyjemnej podrozy \n \n \n");
    printf("Dysponujemy %d pasami torów kolejowych\n", ilosc_torow_kolejowych);
    sleep(2);
    while (ilosc_torow_kolejowych <= ilosc_pociagow) 
    {
        MPI_Recv( & odbierz, 2, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, & mpi_status);
        
        nr_pociagu = odbierz[0];
        status = odbierz[1];
        
        if (status == 1) 
        {
            printf("Pociag %d stoi w bazie\n", nr_pociagu);
        }
        if (status == 2) {
            printf("Pociag %d pozwolenie na start z torow nr %d\n", nr_pociagu, ilosc_zajetych_torow);
            ilosc_zajetych_torow--;
        }
        if (status == 3) {
            printf("Pociag %d wyruszyl\n", nr_pociagu);
        }
        if (status == 4) {
            if (ilosc_zajetych_torow < ilosc_torow_kolejowych) 
            {
                ilosc_zajetych_torow++;
                MPI_Send( & ZATRZYMAJ, 1, MPI_INT, nr_pociagu, tag, MPI_COMM_WORLD);
            } 
            else 
            {
                MPI_Send( & NIE_ZATRZYMAJ, 1, MPI_INT, nr_pociagu, tag, MPI_COMM_WORLD);
            }
        }
        if (status == 5) 
        {
            ilosc_pociagow--;
            printf("Ilosc pociagow: %d\n", ilosc_pociagow);
        }
    }
    printf("Program zakonczyl dzialanie:)\n");
}
void Pociag() 
{
    int stan, suma, i;
    stan = RUCH;
    while (1) 
    {
        if (stan == 1) 
        {
            if (rand() % 2 == 1) 
            {
                stan = START;
                ENERGIA = CHARGING;
                printf("Prosze o pozwolenie na start, pociag: %d\n", nr_procesu);
                Wyslij(nr_procesu, stan);
            } 
            else 
            {
                Wyslij(nr_procesu, stan);
            }
        } 
        else if (stan == 2) 
        {
            printf("Wystartowalem, pociag %d\n", nr_procesu);
            stan = RUCH;
            Wyslij(nr_procesu, stan);
        } 
        else if (stan == 3) 
        {
            ENERGIA -= rand() % 500; 
            if (ENERGIA <= 500) 
            {
                stan = KONIEC_RUCHU;
                printf("Prosze o pozwolenie na postój w bazie.\n");
                Wyslij(nr_procesu, stan);
            } 
            else 
            {
                for (i = 0; rand() % 10000; i++);
            }
        } 
        else if (stan == 4) 
        {
            int temp;
            MPI_Recv( & temp, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, & mpi_status);
            if (temp == ZATRZYMAJ) 
            {
                stan = STACJA;
                printf("Zatrzymalem sie w bazie, pociag %d\n", nr_procesu);
            } 
            else 
            {
                ENERGIA -= rand() % 500;
                if (ENERGIA > 0) 
                {
                    Wyslij(nr_procesu, stan);
                } 
                else 
                {
                    stan = KATASTROFA;
                    printf("Katastrona, pociag %d\n", nr_procesu);
                    Wyslij(nr_procesu, stan);
                    return;
                }
            }
        }
    }
}
int main(int argc, char * argv[]) 
{
    MPI_Init( & argc, & argv);
    MPI_Comm_rank(MPI_COMM_WORLD, & nr_procesu);
    MPI_Comm_size(MPI_COMM_WORLD, & liczba_procesow);
    srand(time(NULL));
    if (nr_procesu == 0) 
        Stacja(liczba_procesow);
    else
        Pociag();
    MPI_Finalize();
    return 0;
}