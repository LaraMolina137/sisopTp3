#include <iostream>
#include <cstring>
#include <sstream>
#include <iterator>
#include <fcntl.h> // para el manejo de fifo
#include <signal.h>
#include <unistd.h>

using namespace std;

bool isValidSTOCK (string sentencia){

    string delimiter = " ";
    string dato = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    cout << dato ;

    if(dato.empty()){ //Si esta vacio necesita el dato
        return false;
    }
                                                                    
    return true;
}

bool isValidSIN_STOCK (string sentencia){

    string delimiter = " ";
    string dato = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    if(dato.empty()){ //Si esta vacio necesita el dato
        return true;
    }
                                                                    
    return false;
}

bool isValidREPO (string sentencia){

    string delimiter = " ";
    string dato = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    if(dato.empty()){ //Si esta vacio necesita el dato
        return false;
    }
                                                                    
    return true;
}

bool isValidLIST (string sentencia){

    string delimiter = " ";
    string dato = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    if(dato.empty()){ //Si esta vacio necesita el dato
        return true;
    }
                                                                    
    return false;
}

bool isValidQUIT (string sentencia){

    string delimiter = " ";
    string dato = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    if(dato.empty()){ //Si esta vacio necesita el dato
        return true;
    }
                                                                    
    return false;
}

bool isValidSentence (string sentencia){

    stringstream registro(sentencia);
    string accion;

    getline(registro, accion, ' ');

    if(accion == "STOCK") { return isValidSTOCK(sentencia); }
    if(accion == "SIN_STOCK") { return isValidSIN_STOCK(sentencia); }
    if(accion == "REPO") { return isValidREPO(sentencia); }
    if(accion == "LIST") { return isValidLIST(sentencia); }
    if(accion == "QUIT") isValidQUIT(sentencia); 

    return false;
}

void help()
{
    cout << "---------------------------------------------------------------------------------------" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;
    cout << "Este script recibe los comandos que van a ser enviados al servidor, para consultar en un archivo de productos" << endl;
    cout << " - Los comandos permitidos para consultar en el archivo son los siguientes:" << endl;
    cout << "\t - STOCK producto_id" << endl;
    cout << "\t\t Muestra DESCRIPCION y STOCK para un producto dado." << endl;
    cout << "\t - SIN_STOCK" << endl;
    cout << "\t\t Muestra ID, DESCRIPCION y COSTO de los productos con STOCK cero." << endl;
    cout << "\t - REPO cantidad" << endl;
    cout << "\t\t Muestra el costo total de reponer una cantidad dada para cada producto sin stock." << endl;
    cout << "\t - LIST" << endl;
    cout << "\t\t Muestra ID, DESCRIPCION y PRECIO de todos los productos existentes." << endl;
    cout << "\t - QUIT" << endl;
    cout << "\t\t Finaliza la ejecucion." << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;
}
    
int main(int argc, char *argv[]){

    if (argc != 1)
    {
        if (argc > 2) {
            
            cout << "La cantidad de parametros es incorrecta" << endl;
            return 1;
        }
        else if (!strcmp(argv[1],"--help") || !strcmp(argv[1], "-h")) {
            help();
            return 0;
        }
        else {
            cout << "El parametro ingresado es incorrecto" << endl;
            return 1;
        }
    }

    string comando;

    cout << "INGRESE COMANDO: ";
    getline(cin, comando);

    while(true){

        if(isValidSentence(comando)){

            //INICIANDO LA CONEXION DE FIFO
            char respuesta[1000];
      
            // Abro tuberia para escribir 
            int fifoClienteServidor = open("/tmp/clienteServidor", O_WRONLY);
            write(fifoClienteServidor,comando.c_str(),strlen(comando.c_str())+1);
            close(fifoClienteServidor);

            if(comando == "QUIT"){
                exit(0);
            }

            // Abro tuberia para leer
            fifoClienteServidor = open("/tmp/clienteServidor", O_RDONLY);
            read(fifoClienteServidor,respuesta,sizeof(respuesta));
            close(fifoClienteServidor);


            cout << "Mensaje recibido del Servidor: " << respuesta << endl;
            //FIN DE LA CONEXION

        } else {
           cout << "La Sintaxis es incorrecta, consultar ayuda" << endl;
        }

        cout << "INGRESE COMANDO: ";
        getline(cin, comando);
        
    }

    return 0;
}