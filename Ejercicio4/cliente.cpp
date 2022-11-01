///////////////////////////////////////////////////////
//////////Ejercicio 4//////////////////////////////////
/////////////////////////////////////////////////////
///Integrantes : 
//         Di Tommaso, Giuliano   38695645            
//
//
//
//          Entrega: 1
/////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <cstring>
#include <fcntl.h>
#include <sys/sem.h>
#include <semaphore.h>
#include <sys/stat.h>

using namespace std;

struct gato
{
    char nombre[20];
    char raza[20];
    char sexo;
    char castrado[2];
};

#define SEM_ALTA "/semaltaditommaso"
#define SEM_BAJA "/sembajaditommaso"
#define SEM_CONSULTA "/semconsultaditommaso"
#define SEM_RESP_ALTA "/respaltaditommaso"
#define SEM_RESP_BAJA "/respbajaditommaso"
#define SEM_RESP_CONSULTA "/respconsultaditommaso"

#define keyAlta "/tmp/ejercicio4ditommasoalta"
#define keyBaja "/tmp/ejercicio4ditommasobaja"
#define keyConsulta "/tmp/ejercicio4ditommasoconsulta"
#define keyRespAlta "/tmp/ejercicio4ditommasorespalta"
#define keyRespBaja "/tmp/ejercicio4ditommasorespbaja"
#define keyRespConsulta "/tmp/ejercicio4ditommasorespconsulta"

sem_t *mutex_baja, *mutexAlta, *mutexConsulta, *mutexRespBaja, *mutexRespAlta, *mutexRespConsulta;

sem_t *openSem(const char *semName, int value);
int getSharedMemorySegmentId(const char *keyName, int size, int mode);
void deleteCat(char *nombreGato);
void consultar(char *nombreGato);
void recibirRespuestaBaja();
void recibirRespuestaConsulta();

void help()
{
    cout << "Hay 3 formas de ejecucion " << endl;
    cout << "./ejercicio4 ALTA [nombreGato] [razaGato] [sexo(H|M)] [castrado(CA)|sinCastrar(SC)]" << endl;
    cout << "./ejercicio4 BAJA [nombreGato]" << endl;
    cout << "./ejercicio4 CONSULTA [nombreGato(OPCIONAL)]" << endl;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        cout << "Parametros insuficientes" << std::flush;
        return 0;
    }
    else
    {
        string par1 = argv[1];
        if (argc == 2)
        {
            if (par1.compare("-h") == 0)
            {   
                help();
                return EXIT_SUCCESS;
            }
            else
            {
                if (par1.compare("BAJA") != 0 && par1.compare("CONSULTA") != 0)
                {
                    cout << "Parametros incorrectos" << endl;
                    return EXIT_FAILURE;
                }
            }
        }
        else
        {
            if (par1.compare("ALTA") != 0 && par1.compare("CONSULTA") != 0 && par1.compare("BAJA") != 0)
            {
                cout << "Parametros incorrectos" << endl;
                return EXIT_FAILURE;
            }
        }
        if (par1.compare("ALTA") == 0)
        {   
            string sexo = argv[4];
            if(sexo.compare("M") != 0 && sexo.compare("H") != 0){
                cout << "Parametros incorrectos.Utilice -h para conocer el funcionamiento" << endl;
                exit(1);
            }
            string castrado = argv[5];
            if(castrado.compare("CA") != 0 && castrado.compare("SC") != 0){
                cout << "Parametros incorrectos.Utilice -h para conocer el funcionamiento" << endl;
                exit(1);
            }
            struct gato *str;
            key_t key;
            mutexAlta = openSem(SEM_ALTA, 1);
            sem_wait(mutexAlta);
            int shmid = getSharedMemorySegmentId(keyAlta, sizeof(struct gato), 0644 | IPC_CREAT);
            if (shmid != -1)
            {
                str = (gato *)shmat(shmid, (void *)0, 0);
                if (str == (gato *)(-1))
                {
                    perror("shmat");
                    exit(1);
                }

                strcpy(str->nombre, argv[2]);
                strcpy(str->raza, argv[3]);
                str->sexo = argv[4][0];
                strcpy(str->castrado,argv[5]);

                // detach from shared memory
                if (shmdt(str) == -1)
                {
                    perror("shmdt");
                    exit(1);
                }
            }
            sem_post(mutexAlta);
            mutexRespAlta = openSem(SEM_RESP_ALTA, 0);
            sem_wait(mutexRespAlta);
            int shmidRespA = getSharedMemorySegmentId(keyRespAlta, 1024, 0644);
            char *respAlta = (char *)shmat(shmidRespA, (void *)0, 0);
            cout << respAlta << endl;
            if (shmdt(respAlta) == -1)
            {
                perror("shmdt");
            }
            // destroy the shared memory
            if (shmctl(shmidRespA, IPC_RMID, NULL) == -1)
            {
                perror("shmctl");
            }
        }
        else if (par1.compare("BAJA") == 0)
        {
            key_t keyB;
            int shmidB;
            mutex_baja = openSem(SEM_BAJA, 1);
            sem_wait(mutex_baja);
            deleteCat(argv[2]);
            sem_post(mutex_baja);
            recibirRespuestaBaja();
        }
        else if (par1.compare("CONSULTA") == 0)
        {
            mutexConsulta = openSem(SEM_CONSULTA, 1);
            sem_wait(mutexConsulta);
            consultar(argv[2]);
            sem_post(mutexConsulta);
            mutexRespConsulta = openSem(SEM_RESP_CONSULTA, 0);
            sem_wait(mutexRespConsulta);
            recibirRespuestaConsulta();
            sem_post(mutexRespConsulta);
        }
    }

    return EXIT_SUCCESS;
}

sem_t *openSem(const char *semName, int value)
{
    sem_t *mutex;
    if ((mutex = sem_open(semName, O_CREAT, 0644, value)) == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }
    return mutex;
}

int getSharedMemorySegmentId(const char *keyName, int size, int mode)
{
    int shmid;
    key_t key;
    if (-1 != open(keyName, 0100, 0777))
    {
        key = ftok(keyName, 0);
    }
    else
    {
        perror("open");
        return -1;
    }
    if ((shmid = shmget(key, size, mode)) == -1)
    {
        perror("shmget");
        return -1;
    }
    return shmid;
}

void deleteCat(char *nombreGato)
{
    cout << "Iniciando baja" << endl;
    int shmidB = getSharedMemorySegmentId(keyBaja, 1024, 0644 | IPC_CREAT);
    char *str = (char *)shmat(shmidB, (void *)0, 0);
    if (str == (char *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    strcpy(str, nombreGato);
}

void recibirRespuestaBaja()
{
    cout << "Esperando respuesta" << endl;
    mutexRespBaja = openSem(SEM_RESP_BAJA, 0);
    sem_wait(mutexRespBaja);
    int shmid = getSharedMemorySegmentId(keyRespBaja, 1024, 0644);

    char *str = (char *)shmat(shmid, (void *)0, 0);
    if (str == (char *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    cout << str << endl;
    sem_post(mutex_baja);
}

void consultar(char *nombreGato)
{
    int shmidB = getSharedMemorySegmentId(keyConsulta, 1024, 0644 | IPC_CREAT);
    char *str = (char *)shmat(shmidB, (void *)0, 0);
    if (str == (char *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    if (nombreGato != NULL)
    {
        strcpy(str, nombreGato);
    }
    else
    {
        strcpy(str, "NOSEPASONOMBRE");
    }
    cout << "BUSCANDO: " << str << endl;
}

void recibirRespuestaConsulta()
{
    mutexRespConsulta = openSem(SEM_RESP_CONSULTA, 0);
    sem_wait(mutexRespConsulta);
    int shmid = getSharedMemorySegmentId(keyRespConsulta, 1024, 0644 | IPC_CREAT);

    char *str = (char *)shmat(shmid, (void *)0, 0);
    if (str == (char *)(-1))
    {
        perror("shmat");
        exit(1);
    }
    char *ptr = strtok(str, ";");
    while (ptr != NULL)
    {
        cout << ptr << endl;
        ptr = strtok(NULL, ";");
    }
    if (shmdt(str) == -1)
    {
        perror("shmdt");
    }
    // destroy the shared memory
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
    }
}