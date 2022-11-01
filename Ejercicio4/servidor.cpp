#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <chrono>
#include <thread>
#include <fcntl.h>
#include <filesystem>
#include <semaphore.h>
#include <sys/stat.h>
#include <cstring>

using namespace std;

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

sem_t *mutexBaja, *mutexAlta, *mutexConsulta, *mutexRespBaja, *mutexRespAlta, *mutexRespConsulta;

struct gato
{
    char nombre[20];
    char raza[20];
    char sexo;
    bool castrado;
};

template <typename T>

void detachAndDestroy(T toDetach, int toDestroy)
{
    // detach from shared memory
    if (shmdt(toDetach) == -1)
    {
        perror("shmdt");
    }
    // destroy the shared memory
    if (shmctl(toDestroy, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
    }
}

sem_t *openSem(const char *semName);
int getSharedMemorySegmentId(const char *keyName, int size, int mode);
// void detachAndDestroy(char* toDetach,int toDestroy);

int main(int argc, const char *argv[])
{
    mutexBaja = sem_open(SEM_BAJA, O_CREAT, 0644, 1);
    sem_destroy(mutexBaja);
    while (true)
    {
        // ALTA

        int shmidA = -1, shmidB = -1, shmidC = -1;
        // ftok to generate unique key

        if ((mutexAlta = sem_open(SEM_BAJA, O_CREAT, 0644, 1)) == SEM_FAILED)
        {
            perror("sem_open");
        }
        sem_wait(mutexAlta);
        key_t key;

        if (-1 != open(keyAlta, O_CREAT, 0777))
        {
            key = ftok(keyAlta, 0);
        }
        else
        {
            perror("open");
        }
        // shmget returns an identifier in shmid
        struct gato *str;
        if ((shmidA = shmget(key, sizeof(struct gato), 0644)) == -1)
        {
            perror("shmget");
        }
        else
        {
            // shmat to attach to shared memory
            cout << shmidA << endl;
            str = (gato *)shmat(shmidA, (void *)0, 0);
            if (str == (gato *)(-1))
            {
                perror("shmat");
            }
            cout << "sexo:  " << str->sexo << endl;
            cout << "castrado:  " << str->castrado << endl;
            cout << "nombre:  " << str->nombre << endl;
            cout << "raza:  " << str->raza << endl
                 << std::flush;

            ofstream outfile;

            outfile.open("dbgatitos.txt", ios::out | ios::app);

            outfile << str->nombre << "," << str->raza << "," << str->sexo << "," << str->castrado << ";" << endl;
            outfile.flush();
            outfile.close();
            detachAndDestroy<gato *>(str, shmidA);
            // detach from shared memory
        }
        sem_post(mutexAlta);

        ///////// BAJA ///////////////
        key_t keyB;
        char *nombreGato;
        mutexBaja = openSem(SEM_BAJA);
        sem_wait(mutexBaja);
        shmidB = getSharedMemorySegmentId(keyBaja, 1024, 0644);

        // shmget returns an identifier in shmid
        if (shmidB != -1)
        {
            // shmat to attach to shared memory
            nombreGato = (char *)shmat(shmidB, (void *)0, 0);
            if (nombreGato == (char *)(-1))
            {
                perror("shmat");
            }
            else
            {
                string line;
                ifstream iFile;
                ofstream outfile;
                outfile.open("dbgatitostmp.txt", ios::out | ios::app);
                iFile.open("dbgatitos.txt");
                unsigned int curLine = 0;
                unsigned int foundit = 0;
                while (getline(iFile, line))
                {
                    curLine++;
                    string tofind = nombreGato;
                    if (line.find(tofind + ",", 0) != string::npos)
                    {
                        cout << "found: " << nombreGato << "line: " << curLine << endl;
                        foundit = 1;
                    }
                    else
                    {
                        outfile << line << endl;
                    }
                }
                iFile.close();
                outfile.close();
                remove("dbgatitos.txt");
                rename("dbgatitostmp.txt", "dbgatitos.txt");
                mutexRespBaja = openSem(SEM_RESP_BAJA);
                int shmidRespB = getSharedMemorySegmentId(keyRespBaja, 1024, 0644);
                char *respBaja = (char *)shmat(shmidRespB, (void *)0, 0);
                if (respBaja == (char *)(-1))
                {
                    perror("shmat");
                }
                if (foundit == 1)
                {
                    strcpy(respBaja, "BORRADO");
                }
                else
                {
                    strcpy(respBaja, "NO EXISTE");
                }
                sem_post(mutexRespBaja);
            }
            detachAndDestroy<char *>(nombreGato, shmidB);
        }
        sem_post(mutexBaja);

        ////////// CONSULTA //////////////////////////////////////

        key_t keyC;
        char *consulta;
        mutexConsulta = openSem(SEM_CONSULTA);
        sem_wait(mutexConsulta);
        shmidC = getSharedMemorySegmentId(keyConsulta, 1024, 0644);
        if (shmidC != -1)
        {
            // shmat to attach to shared memory
            consulta = (char *)shmat(shmidC, (void *)0, 0);
            if (consulta == (char *)(-1))
            {
                perror("shmat");
            }
            key_t keyRespCons;
            char *respConsulta;
            mutexRespConsulta = openSem(SEM_RESP_CONSULTA);
            int shmidRespC = getSharedMemorySegmentId(keyRespConsulta, 1024, 0644 | IPC_CREAT);
            if (shmidRespC != -1)
            {

                respConsulta = (char *)shmat(shmidRespC, (void *)0, 0);
                if (respConsulta == (char *)(-1))
                {
                    perror("shmat");
                }
                string totalFile;
                string line;
                ifstream iFile;
                iFile.open("dbgatitos.txt");
                string cons = consulta;
                if (cons.compare("NOSEPASONOMBRE") != 0)
                {
                    unsigned int foundit = 0;
                    while (foundit == 0 && getline(iFile, line))
                    {
                        string tofind = consulta;
                        if (line.find(tofind + ",", 0) != string::npos)
                        {
                            totalFile.append(line);
                            foundit = 1;
                        }
                    }
                    if (foundit == 0)
                    {
                        totalFile = "No se encontro al gatito";
                    }
                }
                else
                {
                    while (getline(iFile, line))
                    {
                        totalFile.append(line);
                    }
                    cout << totalFile.c_str() << endl;
                }
                strcpy(respConsulta, totalFile.c_str());
                iFile.close();
                if (shmdt(respConsulta) == -1)
                {
                    perror("shmdt");
                }
            }
            detachAndDestroy<char *>(consulta, shmidC);
            sem_post(mutexRespConsulta);
        }
        sem_post(mutexConsulta);
        cout << "------------------" << endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
    return EXIT_SUCCESS;
}

sem_t *openSem(const char *semName)
{
    sem_t *mutex;
    if ((mutex = sem_open(semName, O_CREAT, 0644, 1)) == SEM_FAILED)
    {
        perror("sem_open");
        return NULL;
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

/* void detachAndDestroy(char* toDetach,int toDestroy){
    // detach from shared memory
    if (shmdt(toDetach) == -1)
    {
        perror("shmdt");
    }
    // destroy the shared memory
    if (shmctl(toDestroy, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
    }
} */