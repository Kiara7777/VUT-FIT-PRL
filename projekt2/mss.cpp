/*
 * Projekt: Projekt 2 do predmetu PRL - Merge-splitting sort
 * Autor: Sara Skutova, xskuto00@stud.fit.vutbr.cz
 * Datum: 26.03.2018
 */


#include <cstdlib>
#include <cmath>
#include <fstream>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm> 

#include <mpi.h>

#include <time.h>

using namespace std;

#define DEBUG false
#define TEST false

#define TAG 0
#define TAG_S 9

const string fileName = "numbers";

/*
 * Cast kodu prevzata a upravena z http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
 * Vypocte a vrati rozdil mezi dvema casovymi udaji
 * @param struktura s prvnim casem
 * @param struktura s druhym casem
 * @return casovy rozdil
 */
timespec rozdil(timespec pocatek, timespec konec)
{
    timespec vysledek;
    
    if ((konec.tv_nsec - pocatek.tv_nsec) >= 0) 
    {
        vysledek.tv_sec = konec.tv_sec - pocatek.tv_sec;
        vysledek.tv_nsec = konec.tv_nsec - pocatek.tv_nsec;
    } 
    else 
    {
        vysledek.tv_sec = konec.tv_sec - pocatek.tv_sec - 1;
        vysledek.tv_nsec = 1000000000 + konec.tv_nsec - pocatek.tv_nsec;
    }
    
    return vysledek;
}

/*
 * Ze souboru numbers nacte jednotliva cisla
 * @param vektor nactenych cislic
 * @return Uspech cteni,true pokud bez problemu, false pokud se soubor neotevrel
 */
bool getNumbers(vector<int> *numbers)
{
    int number;                                    
    fstream fin;
        
    fin.open(fileName.c_str(), ios::in); 
        
    if (!fin.is_open())
    {
        cerr << "Soubor " + fileName + " se nepodarilo otevrit" << endl;
        return false;
    }
    
    while(fin.good()){
        number = fin.get();
        
        if(!fin.good()) //kdys se nacte eof, tak vyskoc
            break;
        
        numbers->push_back(number);
    }
    
    fin.close();
    
    return true; 
}

/*
 * Na stdout vytiskne cisla v pozadovanem formatu - na jednom radku, oddelene mezerou
 * @param numbers, vektor cisel co se maji vypsat
 */
void printNumbers(vector<int> numbers)
{
    for (int i = 0; i < numbers.size(); i++)
    {
        cout << numbers[i];
        if (i < (numbers.size() - 1))
            cout << ' ';
    }
    
    cout << endl << flush;
}


/**
 * Pomocna funkce pro testovani, vypise pole
 * @param array Pole hodnot co se ma vypsat
 * @param size VElikost pole
 */
void printArray(int *array, int size)
{
    for (int i = 0; i < size; i++)
        cout << array[i] << " ";
    cout << endl;
}

/*
 * Hlavni funkce progemu
 */
int main(int argc, char** argv) {
    
    //getNumbers();
    
    int rank;
    int size;
    int skup;
    int kolikPridano = 0;
    MPI_Status stat;
    
    vector<int> numbers;
    
    timespec pocatek, konec;
    
    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &size);           // zjistíme, kolik procesů běží 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);           // zjistíme id svého procesu 

    
    if (rank == 0) // 0 procesor, vsechno nacte, a rozesle hodnoty sobe i vsem ostatnim
    {
        
        vector<int> numbers;
        getNumbers(&numbers);

        printNumbers(numbers);

        
        int zbytek = numbers.size() % size;
        
        if (zbytek != 0) //algoritmus potrebuje aby se pocet cislic mohl delit poctem procesor;
        {
            kolikPridano = size - zbytek; //kolik poterbuju pridat neceho... asi tam dam 0, at je to na zacatku
            for (int i = 0; i < kolikPridano; i++)
                numbers.push_back(0);      
        }
        
        skup = numbers.size() / size;
        
        int *procNumbers = new int[skup];
        
        //vsem procesorum, i mastrovi se rozeslou cisla, ktera budou radit
        int poradi = 0;

#if TEST
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pocatek);
#endif

        for (int i = 0; i < size; i++)
        {
            //posle vsem ostatnim velikost skupiny - slo by to resit i pres parametry prikazove radky
            MPI_Send(&skup, 1, MPI_INT, i, TAG_S, MPI_COMM_WORLD);
            
            for (int j = 0; j < skup; j++) //dostane hodnoty z vektoru do pole, poradi zajistuje, ze se pro kazdy procesor veme spravna posloupnout
            {
                procNumbers[j] = numbers[poradi + j];
            }
            poradi += skup;
            
            //buffer,velikost,typ,rank prijemce,tag,komunikacni skupina
            //posila vsem ostatnim i sobe cisla
            MPI_Send(procNumbers, skup, MPI_INT, i, TAG, MPI_COMM_WORLD);
        }
        delete procNumbers; 
    }
    
    //ziskam velikost skupiny
    MPI_Recv(&skup, 1, MPI_INT, 0, TAG_S, MPI_COMM_WORLD, &stat);

    int *recvNumbers = new int[skup];
    
    //buffer,velikost,typ,rank odesilatele,tag, skupina, stat
    //ziskam skupinu
    MPI_Recv(recvNumbers, skup, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);

#if DEBUG
    cout << "ID: " << rank << ": "; //DEBUG
    printArray(recvNumbers, skup);
#endif

    //1. setridit sekvencnim algoritmem
    vector<int> myNumbers(recvNumbers, recvNumbers + skup); //pole na vektor, potrebuju vektor pro sort
    sort(myNumbers.begin(), myNumbers.end());
    copy(myNumbers.begin(), myNumbers.end(), recvNumbers); //setridene hodnoty ratim zpet do meho pole
    
#if DEBUG
    cout << "Serazeno, ID: " << rank << ": ";
    printArray(recvNumbers, myNumbers.size());
#endif
    
    //2. Faze spojovani a rozdelovani
    int polovina = size / 2;
    int limita = 2*(size/2);
    int limitaLiche = 2*(size/2)-1;                 
    int limitaSude = 2*((size-1)/2); 
    
    int ceilCyklus = ceil(double(size) / 2);
    int limitaCPU = 2 * floor(double(size) / 2);
    
    int *helpnumbers = new int[skup];

    for (int i = 1; i <= ceilCyklus; i++)
    {
        //1
        if ((rank == 0 || !(rank%2)) && (rank < limitaLiche)) // suda
        {
            MPI_Send(recvNumbers, skup, MPI_INT, rank+1, TAG, MPI_COMM_WORLD); //svemu pravemu (lichemu) sousedovi poslu cisla
            MPI_Recv(recvNumbers, skup, MPI_INT, rank+1, TAG, MPI_COMM_WORLD, &stat); // od sveho praveho, licheho  prijmu hodnoty po merge split

            
        }
        else if (rank <= limitaLiche) //licha
        {
            MPI_Recv(helpnumbers, skup, MPI_INT, rank-1, TAG, MPI_COMM_WORLD, &stat); //ja lichy, prijmu hodnotu co mi poslal sudy
     
            vector<int> mergeHelp(2*skup);
            // udelam merge = cisla co mam, a cisla co jsem dostal
            merge(recvNumbers, recvNumbers + skup, helpnumbers, helpnumbers + skup, mergeHelp.begin());
            
            
#if DEBUG
            cout << "Merge, ID: " << rank << ": ";
            printArray(mergeHelp.data(), mergeHelp.size());
#endif
            
            //vysledny vektor pak rozdelim na 2 casti, levou poslu svemu levemu (sudemu) sousedovi, prava se stane moji hodnotou
            size_t const polVec = mergeHelp.size() / 2;
            vector<int> lower(mergeHelp.begin(), mergeHelp.begin() + polVec);
            vector<int> higher(mergeHelp.begin() + polVec, mergeHelp.end());
            
    
            MPI_Send(lower.data(), skup, MPI_INT, rank-1, TAG, MPI_COMM_WORLD); //poslu nove hodnoty svemu levemu sousedovi, sudemu sousedovi
             
            //prekopiruju hodnoty z pomocneho vektoru do sveho hlavniho pole hodnot
            copy(higher.begin(), higher.end(), recvNumbers);

        }
        
        //2
        if ((rank%2) && (rank < limitaSude)) //licha
        {

            MPI_Send(recvNumbers, skup, MPI_INT, rank+1, TAG, MPI_COMM_WORLD); //svemu pravemu (sudemu) sousedovi poslu cisla
            MPI_Recv(recvNumbers, skup, MPI_INT, rank+1, TAG, MPI_COMM_WORLD, &stat); // od sveho praveho, sudeho  prijmu hodnoty po merge split

        }
        else if ((rank != 0) && rank <= limitaSude) //suda
        {
            MPI_Recv(helpnumbers, skup, MPI_INT, rank-1, TAG, MPI_COMM_WORLD, &stat); //ja sudy, prijmu hodnotu co mi poslal lichy

            vector<int> mergeHelp(2*skup);
            // udelam merge = cisla co mam, a cisla co jsem dostal
            merge(recvNumbers, recvNumbers + skup, helpnumbers, helpnumbers + skup, mergeHelp.begin());
                    
#if DEBUG
            cout << "Merge ID: " << rank << ": ";
            printArray(mergeHelp.data(), mergeHelp.size());
#endif
            
            //vysledny vektor pak rozdelim na 2 casti, levou poslu svemu levemu (sudemu) sousedovi, prava se stane moji hodnotou
            size_t const polVec = mergeHelp.size() / 2;
            vector<int> lower(mergeHelp.begin(), mergeHelp.begin() + polVec);
            vector<int> higher(mergeHelp.begin() + polVec, mergeHelp.end());
            

            
            MPI_Send(lower.data(), skup, MPI_INT, rank-1, TAG, MPI_COMM_WORLD); //poslu nove hodnoty svemu levemu sousedovi, sudemu sousedovi
             
            //prekopiruju hodnoty z pomocneho vektoru do sveho hlavniho pole hodnot
            copy(higher.begin(), higher.end(), recvNumbers);

                
        }     
    }

    //prenos vsech hodnot do 0 procesoru - pro konecny vypis
    int *vysledek = new int[skup*size];
    int *help = new int[skup];
    
    for (int i = 1; i < size; i++)
    {
        if (rank == i)
            MPI_Send(recvNumbers, skup, MPI_INT, 0, TAG, MPI_COMM_WORLD);
        if (rank == 0)
        {
            MPI_Recv(help, skup, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
            
            for (int j = 0; j < skup; j++)
                vysledek[(i*skup) + j] = help[j];
        }     
    }
    
    //procsor 0 prida svoji hodnotu a vypise vysledek
    if (rank == 0)
    {
#if TEST
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &konec);
        timespec cas = rozdil(pocatek, konec);
        cout << cas.tv_sec << ":" << cas.tv_nsec << endl;
#endif
        for (int i = 0; i < skup; i++)
            vysledek[i] = recvNumbers[i];
        
        //kolikPridano pocet kolik se pridalo pro korecki nedestatku cisel
        //normalne to bude 0, jinac nejake cislo, urcuje kolik se pridalo prvku navic
        //pridavaji se 0, takze budou na zacatku pole
        //zaroven to taky muze figurovat na kolikatem indexu zacinaji prave hodnoty

        for (int i = kolikPridano; i < skup*size; i++) 
            cout << vysledek[i] << endl;

    }
    
    
    MPI_Finalize();
    
    delete[] recvNumbers;
    delete[] helpnumbers;
    delete[] vysledek;
    delete[] help;
    
    
    return 0;
}

