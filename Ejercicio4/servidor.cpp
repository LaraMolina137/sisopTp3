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

using namespace std;

const char* keyAlta = "/tmp/ejercicio4ditommasoalta";
const char* keyBaja = "/tmp/ejercicio4ditommasobaja";
const char* keyConsulta = "/tmp/ejercicio4ditommasoconsulta";


struct gato{
    char nombre[20];
    char raza[20];
    char sexo;
    bool castrado;
};


int main(int argc, const char * argv[]) {
    while(true){ 
    //ALTA

    int shmidA = -1,shmidB = -1,shmidC = -1; 
    // ftok to generate unique key
     
    key_t key;
    
    if (-1 != open(keyAlta, O_CREAT, 0777)) {
        key = ftok(keyAlta, 'R');
     } else {
        perror("open");
        exit(1);
    }
    // shmget returns an identifier in shmid
    struct gato *str;
    if(( shmidA = shmget(key,sizeof(struct gato),0644) ) == -1 ) {
         perror("shmget");
    }else{
        // shmat to attach to shared memory
        cout << shmidA << endl ; 
        str = (gato*) shmat(shmidA,(void*)0,0);
        if (str == (gato *)(-1)) {
            perror("shmat");
        }
        cout << "sexo:  " << str->sexo << endl;
        cout << "castrado:  " << str->castrado << endl ;
        cout << "nombre:  " << str->nombre << endl;
        cout << "raza:  " << str->raza << endl << std::flush ;

        ofstream outfile;

        outfile.open("dbgatitos.txt", ios::out | ios::app);

        outfile << str->nombre << ","<< str->raza << "," << str->sexo<< "," << str->castrado<< endl;
        
        //detach from shared memory 
        if (shmdt(str) == -1) {
            perror("shmdt");
        }
        // destroy the shared memory
        if(shmctl(shmidA,IPC_RMID,NULL) == -1){
            perror("shmctl");
        }
    }


    //BAJA
    key_t keyB ;
    if (-1 != open(keyBaja, 0100, 0777)) {
                    keyB = ftok(keyBaja, 0);
                } else {
                    perror("open");
                    exit(1);
                }
    // shmget returns an identifier in shmid
    char* nombreGato;
    if(( shmidB = shmget(keyB,1024,0644|IPC_CREAT) ) == -1 ) {
         perror("shmget");
    }else{
    // shmat to attach to shared memory
    nombreGato = (char*) shmat(shmidB,(void*)0,0);
    if (nombreGato == (char *)(-1)) {
        perror("shmat");
    }else{
        string line;
        ifstream iFile;
        cout << nombreGato << endl;
        iFile.open("dbgatitos.txt");
        unsigned int curLine = 0;
        while(getline(iFile, line)) {
                curLine++;
            if (line.find(nombreGato, 0) != string::npos) {
               // cout << "found: " << nombreGato << "line: " << curLine << endl;
            }
        }
         ofstream outfile;
         outfile.open("dbgatitos.txt", ios::out | ios::app);
    }   
    //detach from shared memory 
    if (shmdt(nombreGato) == -1) {
        perror("shmdt");
    }
    
    // destroy the shared memory
    if(shmctl(shmidB,IPC_RMID,NULL) == -1){
        perror("shmctl");
    }
    }
    
    

    //CONSULTA

    key_t keyC = ftok(keyConsulta,'R');
    
    // shmget returns an identifier in shmid
    char* consulta;
    if((shmidC = shmget(keyC,1024,0644|IPC_CREAT) ) == -1 ) {
         perror("shmget");
    }else{
            // shmat to attach to shared memory
        consulta = (char*) shmat(shmidC,(void*)0,0);
        if (consulta == (char *)(-1)) {
            perror("shmat");
        }

        //detach from shared memory 
        if (shmdt(consulta) == -1) {
            perror("shmdt");
            }
         // destroy the shared memory
        if(shmctl(shmidC,IPC_RMID,NULL) == -1){
            perror("shmctl");
            exit(1);
        }     
    }
    
   
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
}
    return EXIT_SUCCESS;  
}