#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <cstring>
#include <fcntl.h>

using namespace std;

struct gato{
    char nombre[20];
    char raza[20];
    char sexo;
    bool castrado;
};

const char* keyAlta = "/tmp/ejercicio4ditommasoalta";
const char* keyBaja = "/tmp/ejercicio4ditommasobaja";
const char* keyConsulta = "/tmp/ejercicio4ditommasoconsulta";

string help(){
    return "Ejecutelo de la siguiente manera: ./ejercicio4 ";
}

int main(int argc,char *argv[]) {   
    int shmid;
    
    if(argc == 1){
        cout << "Parametros insuficientes" << std::flush;
        return 0;
    }else{
        string par1 = argv[1];
        if(argc == 2){
            if(par1.compare("-h") == 0){
                cout << help() << endl;
                return EXIT_SUCCESS;
            }else{
                if(par1.compare("BAJA") != 0 && par1.compare("CONSULTA") != 0){
                    cout << "Parametros incorrectos" << endl;
                    return EXIT_FAILURE;
                }
            }
        }else{
           if(par1.compare("ALTA") != 0 && par1.compare("CONSULTA") != 0 && par1.compare("BAJA") != 0){
                cout << "Parametros incorrectos" << endl;
                return EXIT_FAILURE;
           }
        }
        if(par1.compare("ALTA") == 0){
                        struct gato *str;
                key_t key ;
                 if (-1 != open(keyAlta, 0100, 0777)) {
                    key = ftok(keyAlta, 0);
                } else {
                    perror("open");
                    exit(1);
                }
                if( (shmid = shmget(key,sizeof(struct gato),0644|IPC_CREAT) ) == -1 ){
                    perror("shmget");
                    exit(1);
                }

                str = (gato*) shmat(shmid,(void*)0,0);
                if (str == (gato *)(-1)) {
                    perror("shmat");
                    exit(1);
                }
            
                strncpy(str->nombre, argv[2],strlen(argv[2]));
                strncpy(str->raza, argv[3],strlen(argv[3]));
                str->sexo = argv[4][0];
                str->castrado = argv[5];

                //detach from shared memory 
            if (shmdt(str) == -1) {
                    perror("shmdt");
                    exit(1);
                }
        }else if (par1.compare("BAJA") == 0){
             key_t keyB;
             int shmidB;
             if (-1 != open(keyBaja, 0100, 0777)) {
                    keyB = ftok(keyBaja, 0);
                } else {
                    perror("open");
                    exit(1);
                }
            if( (shmidB = shmget(keyB,1024,0644|IPC_CREAT) ) == -1 ){
                perror("shmget");
                exit(1);
            }
            cout << shmidB << endl;
            char* str;
             str = (char*) shmat(shmidB,(void*)0,0);
                if (str == (char *)(-1)) {
                    perror("shmat");
                    exit(1);
                }
             strncpy(str,argv[2],strlen(argv[2]));   
                //detach from shared memory 
            if (shmdt(str) == -1) {
                    perror("shmdt");
                    exit(1);
                }
        }else if(par1.compare("CONSULTA") == 0){
            key_t key = ftok(keyConsulta,'R');
        }
    }

    return EXIT_SUCCESS;
}
