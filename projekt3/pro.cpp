#include <cstdlib>
#include <cmath>

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include <mpi.h>

#include <time.h>

#define TAG 0
#define TAG_I 1
#define TAG_S 2
#define TAG_W 3
#define TAG_K 4
#define TAG_END 7
#define TAG_V 8
#define TAG_N 9

#define TEST false

using namespace std;

class Node {
private:
    int rank;
    string node;
 
public:
    void setRank(int rank);
    int getRank();
    void setNode(string node);
    string getNode();
};

void Node::setRank(int rank)
{
    this->rank = rank;
}

int Node::getRank()
{
    return this->rank;
}

void Node::setNode(string node)
{
    this->node = node;
}

string Node::getNode()
{
    return this->node;
}

class Edge {
private:
    int id;
    string start;
    string end;
    
public:
    static int ids;
    void setID(int id);
    void setStart(string start);
    void setEnd(string end);
    string getStart();
    string getEnd();
    int getID();
};

void Edge::setID(int id)
{
    this->id = id;
}

void Edge::setStart(string start)
{
    this->start = start;
}

void Edge::setEnd(string end)
{
    this->end = end;
}

string Edge::getStart()
{
    return this->start;
}

string Edge::getEnd()
{
    return this->end;
}

int Edge::getID()
{
    return this->id;
}


class Soused{
private:
    int edge;
    int reverse;

public:
    void setEdge(int edge);
    void setReverse(int reverse);
    int getEdge();
    int getReverse();

};

void Soused::setEdge(int edge)
{
    this->edge = edge;
}

void Soused::setReverse(int reverse)
{
    this->reverse = reverse;
}

int Soused::getEdge()
{
    return this->edge;
}

int Soused::getReverse()
{
    return this->reverse;
}


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

/**
 * Jenom funkce na vypis seznamu sousednosti
 * @param sousedi ukazatel na seznam (vector) sousednosti
 */
void printSousedy(vector< vector<Soused> > *sousedi)
{
    for (int i = 0; i < sousedi->size(); i++)
    {
        for (int j = 0; j < sousedi->at(i).size(); j++)
        {
            cout << "[" << sousedi->at(i).at(j).getEdge() << "][" << sousedi->at(i).at(j).getReverse() << "] -> ";
        }
        cout << endl;
    }
}

/**
 * Jenom pomocna testovaci funkce na vypis vektoru ID hran
 * @param eges Vektor ID hran
 */
void printEdgesID(vector<int> *edges)
{
    for (int i = 0; i < edges->size(); i++)
        cout << edges->at(i) << " ";
    
    cout << endl;  
}

/**
 * Prohledava vector edges, hleda hrany ktere z daneho uzlu (me), vedou do a z rodice (parent)
 * nasledne je prohazuje (z edge na reverse a naopak) a vraci v objektu soused.
 * 
 * @param edges Seznam doted vytvorenych hran
 * @param soused Vracejici se objekt s uzly do a od rodice
 * @param parent Rodic daneho uzlu
 * @param me Dany uzel
 * @return Vzdy true
 */
bool getParent(vector<Edge> *edges, Soused *soused, string parent, string me)
{   
    for (int i = 0; i < edges->size(); i++)
    {
        string testStart = edges->at(i).getStart();
        string testEnd = edges->at(i).getEnd();
        
        if (parent == testStart && me == testEnd) //moje reverzni hrana
            soused->setReverse(edges->at(i).getID());
        else if (me == testStart && parent == testEnd) // moje hrana, ktera jde odemne k rodici
            soused->setEdge(edges->at(i).getID());
        else
            continue;
    }
    
    true; //nikdy by se nemelo stat zeby se rodic nenasel... ty hrany uz davno musi v tom vektoru byt
    
}

/**
 * Funkce co vytvori  seznam (vector) sousednosti. Prochazi seznam uzlu, vytvari pro ne hrany (edges),
 * a pak pro ne vytvari objekt Soused, do ktereho se ukladaji id danych hran.
 * Vysledne objekty Soused se vkladaji do vetoru cimz imituji seznam.
 * Po zpracovani uzlu se vysledny vektor vlozi to vectoru vectoru (sousedi).
 * 
 * @param uzly Retezec uzlu, ktere se maji zpracovat
 * @param edges Vector orientovanych hran
 * @param sousedi Vector vectoru objektu Soused, svazany s vectorem edges
 */
void createSousedy(string uzly, vector<Edge> *edges, vector< vector<Soused> > *sousedi)
{
    for(int i = 0; i < uzly.length(); i++)
    {
        string start(1, uzly.at(i));
        int endL = (i + 1) * 2; //levy potomek
        int endP = ((i + 1) * 2) + 1; //pravy potomek
        
        string endUzelL = "";
        string endUzelP = "";
        
        string parent = "";
        int par = (i + 1) / 2;
        
        if (endL <= uzly.length())
            endUzelL.assign(1, uzly.at(endL-1));
        
        if (endP <= uzly.length())
            endUzelP.assign(1, uzly.at(endP-1));
        
        if (par <= uzly.length() && par >= 1) 
            parent.assign(1, uzly.at(par-1));
        
       
        bool add = false;
        Edge edgeL;
        Edge reverseL;
        
        Soused sousedL;
        
        if (endUzelL != "") // uzel ma leveho potomka, vytvor hranu
        {
            add = true;
            edgeL.setID(Edge::ids);
            Edge::ids++;
            edgeL.setStart(start);
            edgeL.setEnd(endUzelL);
            
            reverseL.setID(Edge::ids);
            Edge::ids++;
            reverseL.setStart(endUzelL);
            reverseL.setEnd(start);
            
            edges->push_back(edgeL);
            edges->push_back(reverseL);
            
            sousedL.setEdge(edgeL.getID());
            sousedL.setReverse(reverseL.getID()); 
        }
        
        Edge edgeP;
        Edge reverseP;
        
        Soused sousedP;
        
        if (endUzelP != "") //uzel ma praveho potomka, tvorim pro neho hrany
        {
            add = true;
            edgeP.setID(Edge::ids);
            Edge::ids++;
            edgeP.setStart(start);
            edgeP.setEnd(endUzelP);
            
            reverseP.setID(Edge::ids);
            Edge::ids++;
            reverseP.setStart(endUzelP);
            reverseP.setEnd(start);
            
            edges->push_back(edgeP);
            edges->push_back(reverseP);
            
            sousedP.setEdge(edgeP.getID());
            sousedP.setReverse(reverseP.getID());
        }
        
        Soused myParent;
        
        if (parent != "")
        {
            getParent(edges, &myParent, parent, start);
        }
        
        
        
        
        ///////////////////////////////////////////////////////////////
        vector<Soused> sousediUzel;
        
        if (parent != "") // ma to rodice
            sousediUzel.push_back(myParent);
        
        
        if (endUzelL != "" && endUzelP != "") //uzel ma leveho i praveho potomka
        {
                sousediUzel.push_back(sousedL);
                sousediUzel.push_back(sousedP);
        }
        else if (endUzelL != "" && endUzelP == "") //uzel ma jenom leveho potomka
        {
                sousediUzel.push_back(sousedL);
        }
        else if (endUzelL == "" && endUzelP != "") //uzel ma jenom praveho potomka
        {
                sousediUzel.push_back(sousedP);
        }
        else
            ; // nema potomky
        
        sousedi->push_back(sousediUzel);
    }
}

/**
 * Funkce prevede vektor sousedu na normalni pole.
 * 
 * @param array Ukazatel na pole
 * @param vektor Ukazatel na vektor
 */
void vector2arrayS(int *array, vector<Soused> *vektor)
{
    int index = 0;
    for (int i = 0; i < vektor->size(); i++)
    {
        array[index] = vektor->at(i).getEdge();
        array[index + 1] = vektor->at(i).getReverse();
        
        index += 2;
    }
}

/**
 * Prevede pole na vektor sousedu
 * @param array Prijate pole
 * @param size Velikost pole
 * @param vektor Vysledny vektor
 */
void array2vectorS(int *array, int size, vector<Soused> *vektor)
{
    for (int i = 0; i < size; i++)
    {
        Soused soused;
        soused.setEdge(array[i]);
        soused.setReverse(array[i + 1]);
        
        vektor->push_back(soused);
        
        i++; //preskocime 1 pvek, uz jsme ho dostali do dvojice
    }
    
}


/**
 * Prevede vektor hran na pole, ve vyslednem poli pak budou jenom ID danych hran
 * 
 * @param array Vysledne pole
 * @param vektor Vsutpni vektor hran
 */
void vector2arrayE(int *array, vector<Edge> *vektor)
{
    //1 polozka, 1 ID, 1 index
    for (int i = 0; i < vektor->size(); i++)
        array[i] = vektor->at(i).getID(); 
}

/**
 * Prevede pole na vektor hran (ve ktutecnosti to bude jenom vektor int, proces dostane jenom ID 
 * dane hrany, pouze 0 procesor bude vedet co presne s danym ID souvisi)
 * @param array Prijate pole
 * @param size Velikost pole
 * @param vektor Vysledny vektor
 */
void array2vectorE(int *array, int size, vector<int> *vektor)
{
    for (int i = 0; i < size; i++)
        vektor->push_back(array[i]);
}

/**
 * Vsem procesum odesle vektor vektoru sousedu. Vektor se rozdeli na jednotlive vnitrni vektory.
 * Ty se prvedou na pole int hodnot, kde kazda dvojice symbolizuje jednu hodnotu Soused.
 * NEjdrive se odesle velikost daneho pole, nasledne samotne pole hodnot
 * 
 * @param sousedi Vektor vektoru sousedu co se odesila
 * @param size Pocet procesu, kterym se maji zpravy odeslat (odesila se i samo sobe 0->0)
 */
void sendSousedy(vector< vector<Soused> > *sousedi, int size)
{
    int allSize = sousedi->size();
    //kazdemu procesu se musi odeslat cele vektor sousedu
    for (int i = 0; i < size; i++) //PROCESY
    {
        //potrebuju jeste poslat kolik poli se mi bude posilat
        MPI_Send(&allSize, 1, MPI_INT, i, TAG_V, MPI_COMM_WORLD);
        
        //cely vektor vektoru ale poslat nelze - posilato po jednotlivych vektorech + vektor prevest na polse intu
        for (int vec = 0; vec < allSize; vec++) //VEKTOR
        {
            int vecSize = sousedi->at(vec).size() * 2; //1 prvek ma v sobe 2 hrany, proto *2
            int *vectorPart = new int[vecSize];
            vector2arrayS(vectorPart, &sousedi->at(vec));
            
            //procesum se posle velikost daneho vektoru
            MPI_Send(&vecSize, 1, MPI_INT, i, TAG_N, MPI_COMM_WORLD);
            //procesum se posle dany vektor
            MPI_Send(vectorPart, vecSize, MPI_INT, i, TAG, MPI_COMM_WORLD);
            
            delete vectorPart;
        }
    }
}

/**
 * Procesy prijmou od 0. procesu cely vektor sousednosti
 * @param sousedi Zrekonstruovany vektor sousednosti
 */
void recvSousedy(vector< vector<Soused> > *sousedi)
{
    MPI_Status stat;
    int allSize;
    int vecSize;
    
    //kolikrat mi jeste nasledujici dvojice prijde
    MPI_Recv(&allSize, 1, MPI_INT, 0, TAG_V, MPI_COMM_WORLD, &stat);
    
    for (int i = 0; i < allSize; i++) //budu opakovat dokud nedostanu cely vektor vektoru...
    {
        //velikost pole co mi proces 0 posila
        MPI_Recv(&vecSize, 1, MPI_INT, 0, TAG_N, MPI_COMM_WORLD, &stat);
    
        int *vectorPart = new int[vecSize];
    
        //pole co mi proces 0 posila ... preved na vektor, vloz do celkoveho vektoru
        MPI_Recv(vectorPart, vecSize, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
        
        vector<Soused> soused;
        
        array2vectorS(vectorPart, vecSize, &soused);
        
        sousedi->push_back(soused);
        
        delete vectorPart;
    }
}

/**
 * Ve procesum odesle  ID uzlu
 * 
 * @param edges Vektor uzlu
 * @param size pocet procesu
 */
void sendEdges(vector<Edge> *edges, int size)
{
    for (int i = 0; i < size; i++) //poslat vsem procesum
    {
        int id = edges->at(i).getID();
        //ono pocet procesoru je stejny jako pocet hran... takze to bude ok to tahle posila
        MPI_Send(&id, 1, MPI_INT, i, TAG, MPI_COMM_WORLD); //odeslat ID hrany
    } 
}

/**
 * Procesy prijmou od procesu 0 ID hrany
 * 
 * @param idHrany vprijate id
 */
void recvEdges(int *idHrany)
{
    MPI_Status stat;
    
    MPI_Recv(idHrany, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat); //id hrany
}

/**
 * procesy odeslou danemu procesu svou vypocitanou hodnotu
 * 
 * @param hrana Vypocitana hodnota
 * @param rank jakemu procesu se zprava zasila
 *  
 */
void sendEulerTour(int hrana,  int rank)
{
    MPI_Send(&hrana, 1, MPI_INT, rank, TAG, MPI_COMM_WORLD); //odeslat ID hrany  
}

/**
 * Procesy dostanou od daneho procesu vypocitanou hodnotu z eulerovy cesty
 * @param hrana
 * @param rank
 */
void recvEulerTour(int *hrana, int rank)
{
    MPI_Status stat;
    
    MPI_Recv(hrana, 1, MPI_INT, rank, TAG, MPI_COMM_WORLD, &stat); //id hrany  
}

/**
 * Funkce je casti tvorby eulerovy cesty, dle algortimu z prednasky (25s)
 * @param hrana
 * @param sousedi
 * @return id hrany jako vysledek euler tour algoritmu
 */
int eulerTour(int hrana, vector< vector<Soused> > *sousedi)
{
//    pro danou hranu
//        -hledame v sousedech, kde dana hrana jako obyc hrana
//        -kdyz uz ji mame, tak ziskame jeji reverzni hranu (jsou ve stejnem obj)
//            
//        -hledame v sousedech reverni hranu, kde je na pozici obyc hrany
//        -kdyz ji najdeme, koukeneme se jestli je tam nejaka dalsi hrana (jestli v danem podvektoru jsou dalsi objekty)
//        -jestli tam je, tak VYSLEDKEM je id te nasleduji hrany 
//        -jestli tam je null, tak vzsledkem je id hrany, ktera je na zacatku seznamu... trochu komplikovanejsi
    int reverse;
    for(int i = 0; i < sousedi->size(); i++)
    {
        for (int j = 0; j < sousedi->at(i).size(); j++)
        {
            if (hrana == sousedi->at(i).at(j).getEdge()) //nasli jsme danou hranu, ziskame jeji reverzni
            {
                reverse = sousedi->at(i).at(j).getReverse();   
            }
        }
    }
    
    //opet prohledavat sousedy, tentokrat pro reverzni hranu, kde je na pozici hlavni hrany
    for(int i = 0; i < sousedi->size(); i++)
    {
        for (int j = 0; j < sousedi->at(i).size(); j++)
        {
            if (reverse == sousedi->at(i).at(j).getEdge()) //nasli jsme kde reverzni hrana je na pozici hlavni hrany
            {
                //je tam tesetovani jestli obsahuje ukazaatel na dalsi prvek v podseznamu... tady ale nemam ukazatele, ale veektor objektu....
                //AKA jestli je tam dalsi prvek, tak to rika, ze prvke ma ukazatel na dalsi prvek <> null
                //test jestli dany prvek ve vektoru ma nekoho za sebou (posledni prvek nema nikoho, velikost je o 1 vetsi)
                if (j < sousedi->at(i).size() - 1) //rika ze tam jeste nekdo je, dokaz neni null... vratim ID hlavni hrany co je za nim
                {
                    return sousedi->at(i).at(j + 1).getEdge();
                    
                }
                else //nikdo tam uz neni, odkaz je null
                {
                    //vraci se hlavni hrana co je jako prvni ve vektoru
                    return sousedi->at(i).at(0).getEdge();
                }
            }
        }
    }
}

/**
 * Funkce pro eulerovu cescu zavede koren/prerizne eulerovu cestu (27s)
 * @param eulerTour Vector s eulerovou cestou
 */
void fixEulerTour(vector<int> *eulerTour, vector<Edge> *edges)
{
    //korenem je start 1. hrany
    string koren;
    int id;
    
    //hleda se hrana ktera vede do korene jako posledni (takze to bude hodnota konce)
    for (int i = 0; i < edges->size(); i++)
    {
        if (i == 0)
            koren = edges->at(0).getStart();
        
        if (koren == edges->at(i).getEnd())
            id = edges->at(i).getID();
    }
    
    //id je praktickz index + 1, nahradime to samym sebou
    eulerTour->at(id - 1) = id;  
}

/**
 * Funkce spocita vahu dane hrany, pokud je dopredna, tak ma vahu 1, jinac 0
 * 
 * @param hrana Hrana pro kterou se pocita vaha
 * @param sousedi Seznam sousedu - i z neho lze pocitat dopredne hrany
 * @return Vaha dane hrany
 */
int getWeight(int hrana, vector< vector<Soused> > *sousedi)
{
    for(int i = 0; i < sousedi->size(); i++)
    {
        for (int j = 0; j < sousedi->at(i).size(); j++)
        {
            //jdeme poporade... kontrolujeme jak hlavni tak i reverse hrany
            if (hrana == sousedi->at(i).at(j).getEdge())
                return 1;
            else if (hrana == sousedi->at(i).at(j).getReverse())
                return 0;
            else 
                continue;
        }
    }
}

/**
 * Funkce pro vypocet sumy suffixu, AKA jeden velky bordel
 * 
 * @param hrana Ukazatel na hranu ze seznamu sousedu
 * @param weight Ukazatel na hodntou vahy, reprezentuje jednu hodnotu z pole hodnot
 * @param rank Rank daneho procesu
 * @param size Pocet vsech procesu
 */
void suffixSum(int *hrana, int *weight, int rank, int size)
{
    bool konec = false; //prvek, ktery je jako posledni hranou, tak musi poslani hodnoty provest 2x
    //rank zcina od 0 ne od 1, takze k rank musime pridat 1
    int originalW = *weight;
    if (*hrana == (rank + 1))
    {
        *weight = 0;
        konec = true;
    }
    
    double n = (double)size;
    int repeat = ceil(log(n));
    
    //je tam vzorec, kolikrat bude muset koncovy poslat svoje hodnoty...
    // 2 3 5 9 17 33 ... (x * 2) - 1
    int opak = 2;
    int komu;
    
    MPI_Status stat;
    
    bool ptaSe = true; //jestli se me nekdo pta na hodntu
    
    for (int k = 0; k <= repeat; k++)
    {
        //potrebuju svoji hodnotu weight a svoji hodnotu hrany = MAM OK
        //potrebuju kontaktovat proces, jehoz ID(rank) je hodnota moji hrany
            //ten by mi mel vratit svoji hodnotu weight
            //a taky budu potrebovat od neho i jeho hodnotu hrany
        
        komu = *hrana - 1;
        
        MPI_Send(hrana, 1, MPI_INT, komu, TAG_I, MPI_COMM_WORLD); //informuju proces, ze chci jeho hodnoty
        
        int prijato;
    
        if (rank != 0 && ptaSe) //1. (0.) by se nikdo nemel nikdy ptat - seznam sousedu je preriznuty a nahrazeny koncem
        {
            MPI_Recv(&prijato, 1, MPI_INT, MPI_ANY_SOURCE, TAG_I, MPI_COMM_WORLD, &stat); //od nekoho dostanu info, ze chce moje hodnoty
            //poslu mu svoje hodnoty, nejdriv hodnotu hrany a pak hodnotu vahy
            
            MPI_Send(hrana, 1, MPI_INT, stat.MPI_SOURCE, TAG_S, MPI_COMM_WORLD); // poslu mu svoji hranu
            MPI_Send(weight, 1, MPI_INT, stat.MPI_SOURCE, TAG_W, MPI_COMM_WORLD); // poslu mu svoji vahu
        }
        
        if (konec) //koncovy proces musi odpovedet vicekrat
        {
            opak = opak - 1; //uz jednou odpovedel
            for (int i = 0; i < opak; i++) //odpovime i tem ostatnim
            {
                

                MPI_Recv(&prijato, 1, MPI_INT, MPI_ANY_SOURCE, TAG_I, MPI_COMM_WORLD, &stat); //od nekoho dostanu info, ze chce moje hodnoty
                //poslu mu svoje hodnoty, nejdriv hodnotu hrany a pak hodnotu vahy
                
                MPI_Send(hrana, 1, MPI_INT, stat.MPI_SOURCE, TAG_S, MPI_COMM_WORLD); // poslu mu svoji hranu
                MPI_Send(weight, 1, MPI_INT, stat.MPI_SOURCE, TAG_W, MPI_COMM_WORLD); // poslu mu svoji vahu   
            }
            
            opak = opak + 1;
            opak = (opak * 2) - 1;
            
            //nekdy je ovsem prvku mene nez jak vychazi z posloupnosti - musi se to upravi
            if (opak > size)
                opak = size;
        }
        
        int novaHrana;
        int novaVaha;
        
        MPI_Recv(&novaHrana, 1, MPI_INT, komu, TAG_S, MPI_COMM_WORLD, &stat); //od procesoru dostanu jeho hranu
        MPI_Recv(&novaVaha, 1, MPI_INT, komu, TAG_W, MPI_COMM_WORLD, &stat); //od procesoru dostanu jeho vahu
        
        //korekce pro to aby se mi nejaky proces nezasekl
        if (rank != 0) // poslu 0 procesu, svoje hodnoty, ten pak informuje proccesy o tom, ktere uz nemaji ocekavat ze se je uz nekdo bude ptat
        {
            MPI_Send(&novaHrana, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD); // poslu mu svoji hranu
        }
        else if (rank == 0) // jsem 0 proces,
        {
            int pomoc;
            vector<int> korekce(size);
            korekce.at(0) = novaHrana - 1;
            
            for (int i = 1; i < size; i++)
            {
                MPI_Recv(&pomoc, 1, MPI_INT, MPI_ANY_SOURCE, TAG, MPI_COMM_WORLD, &stat);
                
                korekce.at(stat.MPI_SOURCE) = pomoc - 1;
            }
            
            //poslat vsem procesum. zda maji pocitat s tim, ze se jich v dalsim kole nekdo zepta na jejich hodnotu
            //1. (0.) se posilat nic nemusi, toho se nikdo nikdy nezepta na hodnotu
            int ano = 1;
            int ne = 0;
            
            for (int i = 1; i < size; i++)
            {
                if(find(korekce.begin(), korekce.end(), i) != korekce.end())
                    MPI_Send(&ano, 1, MPI_INT, i, TAG, MPI_COMM_WORLD); 
                else
                    MPI_Send(&ne, 1, MPI_INT, i, TAG, MPI_COMM_WORLD); 
            }
        }
        
        if (rank != 0) //prijmu informaci jestli v dalsim cyklu mum pocitat s tim ze se me nekdo zepta na mou hodnotu
        {
            int ok;
            MPI_Recv(&ok, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, &stat);
            
            if (ok == 1) // nekdo se me jeste bude ptat
                ptaSe = true;
            else if (ok == 0) //nikdo se me ptat nebude
                ptaSe = false; 
        }
        
        MPI_Barrier(MPI_COMM_WORLD); //donutim vsechny procesy se tady zastavit a pockat na ostatni, aby nedoslo k prepsani hodnot jeste pred tim nez budou odeslane
        
        *weight = *weight + novaVaha;
        *hrana = novaHrana;    
    }
    
//    cout << "Ja: "  << rank + 1 << " hrana: " << *hrana << " vaha: " << *weight << endl;
    
    //ikdyz teoreticky. ta posledni hrana bude vzdy opacna, takze vzdy bude mit 0, takze je tento krok zbytecny... nech to pro splneni alg.
    if (konec)
    {
        int ano = 1; 
        int ne = 0;
        for (int i = 0; i < size; i++)
        {
            if (originalW != 0) //musi se provesto korekce. nejdriv zprava o tom ze proces ma ocekavat hodnotu pro korekci, pak samotna hodnota
            {
                MPI_Send(&ano, 1, MPI_INT, i, TAG_I, MPI_COMM_WORLD);
                
                MPI_Send(&originalW, 1, MPI_INT, i, TAG_K, MPI_COMM_WORLD); 
            }
            else
            {
                MPI_Send(&ne, 1, MPI_INT, i, TAG_I, MPI_COMM_WORLD);
            }
        }
    }
    
    int oprava;
    MPI_Recv(&oprava, 1, MPI_INT, MPI_ANY_SOURCE, TAG_I, MPI_COMM_WORLD, &stat);
    
    if (oprava == 1) //provadi se korekce, musi prijmout jeste jednu zpravu
    {
        int korekce;
        MPI_Recv(&korekce, 1, MPI_INT, stat.MPI_SOURCE, TAG_K, MPI_COMM_WORLD, &stat);
        
        *weight = *weight + korekce;
    }
    
}

int preOrder(int weight, int size)
{
    return (size - weight + 1);
}

int Edge::ids = 1; // inicializace  tridni promenne podle ktere se generuji id hrany

int main(int argc, char** argv) {
    
    string uzly = argv[1];
    
    MPI_Status stat;
    int rank;
    int size;
    vector<Edge> edges;
    vector< vector<Soused> > sousedi;
    
    timespec pocatek, konec;
    
    //MPI INIT
    MPI_Init(&argc,&argv);                          // inicializace MPI 
    MPI_Comm_size(MPI_COMM_WORLD, &size);           // zjistíme, kolik procesů běží 
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);           // zjistíme id svého procesu 

//////////////PRIPRAVA, TVORBA GRAFU////////////////
    if (rank == 0) // 1. proces spocita seznam sousednosti
    {
        
#if TEST
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &pocatek);
#endif
        
        createSousedy(uzly, &edges, &sousedi);
        //printSousedy(&sousedi);
        sendEdges(&edges, size);
        sendSousedy(&sousedi, size);
    }
    
    vector< vector<Soused> > newSousedi;
    int idHrany;
    
    recvEdges(&idHrany);
    recvSousedy(&newSousedi);
    
//////////////////EULEROVA CESTA///////////////////

    int euler = eulerTour(idHrany, &newSousedi);
    
/////////////////VAHA HRANY//////////////////////////
    
    int weight = getWeight(idHrany, &newSousedi);
    bool forward;
    if (weight == 1)
        forward = true;
    else
        forward = false;

////////////////OPRAVA EULEROVY CESTY a PRIPRAVA NA SUMU SUFFIXU//////////////////    
    vector<int> newEuler(edges.size());
    
    for (int i = 1; i < size; i++) //odeslu 0 procesu vypocitanou hodnotu euler, aby ji mohl upravit
    {
        if (rank == i)
        {
            sendEulerTour(euler, 0);
        }
        else if (rank == 0)
        {
            int hrana;
            recvEulerTour(&hrana, i);
            newEuler.at(i) = hrana;
        }
    }
    
    if (rank == 0)
    {
        newEuler.at(0) = euler;
        fixEulerTour(&newEuler, &edges); // 0 procesor jako jediny ma plny seznam hran
        for (int i = 0; i < size; i++)
        {
            sendEulerTour(newEuler.at(i),  i);
            
        }
    }
    
    recvEulerTour(&euler, 0);
    
//////////////////SUMA SUFFIXU///////////////////
    suffixSum(&euler, &weight, rank, size);
    
    
////////////////korekce pro PREORDER///////////////
    
    int poradi;
    int ano = 1; 
    int ne = 0;
    if (forward)
    {
        poradi = preOrder(weight, size);
        MPI_Send(&ano, 1, MPI_INT, 0, TAG_END, MPI_COMM_WORLD); //odeslat svoje poradi
        MPI_Send(&poradi, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD); //odeslat svoje poradi
    }
    else
    {
        MPI_Send(&ne, 1, MPI_INT, 0, TAG_END, MPI_COMM_WORLD); //0 symbolizuje ze 
    }
    
    if (rank == 0)
    {
        int finale;
        MPI_Status stat;
        int tisk;

        vector<Node> nodes;
        Node newNode;
        newNode.setRank(1);
        newNode.setNode(edges.at(0).getStart());
        nodes.push_back(newNode);
        
        for (int i = 0; i < size; i++)
        {
            MPI_Recv(&tisk, 1, MPI_INT, i, TAG_END, MPI_COMM_WORLD, &stat);
            
            if (tisk == 1)
            {
                MPI_Recv(&finale, 1, MPI_INT, i, TAG, MPI_COMM_WORLD, &stat);
                Node help;
                help.setRank(finale);
                help.setNode(edges.at(i).getEnd());
                nodes.push_back(help);
            } 
        }
        
        //TISK VYSLEDKU
        // trochu problematictejsi, v tom vectoru nodes, nejsou uzly ulozene podle toho jaky maji rank, ale podle toho v jakem
        //poradi prisly do procesu 0, takze se to musi cele prohledavat
        for (int i = 1; i <= size; i++)
        {
            for (int j = 0; j < nodes.size(); j++)
            {
                if (i == nodes.at(j).getRank())
                {
                    cout << nodes.at(j).getNode();
                    break;
                }
            }
        }
        
        cout << endl;
        
#if TEST
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &konec);
        timespec cas = rozdil(pocatek, konec);
        cout << cas.tv_sec << ":" << cas.tv_nsec << endl;
#endif
    }

    MPI_Finalize();

    return 0;
}

