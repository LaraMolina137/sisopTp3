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
#include <csignal>
#include <syslog.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>

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


#define ISRUNNING "\
#!/bin/bash  \n\
 val=$(ps -ef | grep serverdito4 | wc -l) \n\
 exit $val \n\
"

sem_t *mutexBaja, *mutexAlta, *mutexConsulta, *mutexRespBaja, *mutexRespAlta, *mutexRespConsulta;

struct gato
{
    char nombre[20];
    char raza[20];
    char sexo;
    char castrado[2];
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

void signalHandler( int signum ) {  
   if(signum == SIGUSR1){
     exit(signum);  
   }
}

sem_t *openSem(const char *semName,int value);
int getSharedMemorySegmentId(const char *keyName, int size, int mode);



void makeDaemon(){
     /////////////////////////////////////////////////////////////////////////////////
     pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);

    /* Catch, ignore and handle signals */
    //TODO: Implement a working signal handler */
    signal(SIGINT, signalHandler);  
    signal(SIGUSR1,signalHandler);

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/tmp");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }

    /* Open the log file */
    //openlog ("firstdaemon", LOG_PID, LOG_DAEMON);

}


int getProcIdByName(string procName)
{
    int currPid = ::getpid();
    int pid = -1;
    // Open the /proc directory
    DIR *dp = opendir("/proc");
    if (dp != NULL)
    {
        // Enumerate all entries in directory until process found
        struct dirent *dirp;
        while (pid < 0 && (dirp = readdir(dp)))
        {
            // Skip non-numeric entries
            int id = atoi(dirp->d_name);
            if (id > 0)
            {
                // Read contents of virtual /proc/{pid}/cmdline file
                string cmdPath = string("/proc/") + dirp->d_name + "/cmdline";
                ifstream cmdFile(cmdPath.c_str());
                string cmdLine;
                getline(cmdFile, cmdLine);
                if (!cmdLine.empty())
                {
                    // Keep first cmdline item which contains the program path
                    size_t pos = cmdLine.find('\0');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(0, pos);
                    // Keep program name only, removing the path
                    pos = cmdLine.rfind('/');
                    if (pos != string::npos)
                        cmdLine = cmdLine.substr(pos + 1);
                    // Compare against requested process name
                    if (procName == cmdLine && id != currPid)
                        pid = id;
                }
            }
        }
       return pid;
    }

    closedir(dp);

    return pid;
}


int main(int argc, const char *argv[])
{   
    if(getProcIdByName("serverdito4") > 0){
        cout << "Ya hay una instancia en ejecucion" << endl;
        exit(EXIT_FAILURE);
    }
    makeDaemon();
    mutexBaja = sem_open(SEM_BAJA, O_CREAT, 0644, 1);
    sem_destroy(mutexBaja);
    while (true)
    {
        // ALTA

        int shmidA = -1, shmidB = -1, shmidC = -1;
        mutexAlta = openSem(SEM_ALTA,1);
        sem_wait(mutexAlta);
        mutexRespAlta = openSem(SEM_RESP_ALTA,0);
        shmidA = getSharedMemorySegmentId(keyAlta,sizeof(struct gato),0644);
        if (shmidA != -1)
        {
            gato* str = (gato *)shmat(shmidA, (void *)0, 0);
            if (str == (gato *)(-1))
            {
                perror("shmat");
            }
            
            int shmidRespA = getSharedMemorySegmentId(keyRespAlta,1024,0644 | IPC_CREAT);
            char* respAlta = (char*) shmat(shmidRespA,(void * )0,0);

            string line;
            ifstream iFile;
            unsigned int foundit = 0;

            iFile.open("dbgatitos.txt");
            while (foundit == 0 && getline(iFile, line))
            {
                string tofind = str->nombre;
                if (line.find(tofind + ",", 0) != string::npos)
                {
                    foundit = 1;
                }
            }
            iFile.close();
            if (foundit == 1)
            {
                strcpy(respAlta, "Ya existe el gatito");
            }else{
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
            }
            detachAndDestroy<gato *>(str, shmidA);
            if (shmdt(respAlta) == -1)
            {
                perror("shmdt");
            }
            sem_post(mutexRespAlta);
        }
        sem_post(mutexAlta);

        ///////// BAJA ///////////////
        key_t keyB;
        char *nombreGato;
        mutexBaja = openSem(SEM_BAJA,1);
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
                mutexRespBaja = openSem(SEM_RESP_BAJA,1);
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
        mutexConsulta = openSem(SEM_CONSULTA,1);
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
            mutexRespConsulta = openSem(SEM_RESP_CONSULTA,1);
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

sem_t *openSem(const char *semName,int value)
{
    sem_t *mutex;
    if ((mutex = sem_open(semName, O_CREAT, 0644, value)) == SEM_FAILED)
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