#include <iostream>
#include <cstring>
#include <sstream>
#include <iterator>
#include <fcntl.h> // para el manejo de fifo
#include <signal.h>


using namespace std;

void help();
    
int main(int argc, char *argv[]){

    if (argc != 1)
    {
        if (argc > 1) {
            cout << "La cantidad de parametros es incorrecta";
            return 1;
        }
        else if (!strcmp(argv[1],"--help") || !strcmp(argv[1], "-h")) {
            help();
            return 0;
        }
        else {
            cout << "El parametro ingresado es incorrecto";
            return 1;
        }
    }

    string comando;

    //LOGICA DEL CLIENTE
    cout << "INGRESE COMANDO: ";
    getline(cin, comando);
    
    //cout << comando << endl;

    // string delimiter = " ";
    // string comando = entrada.substr(0, entrada.find(delimiter));
    // string valor_comando = entrada.substr(entrada.find(delimiter) + delimiter.length());

    while(comando != "QUIT"){

        //if(isValidSentence(accion)){

            //INICIANDO LA CONEXION DE FIFO
            char respuesta[1000];
      
            // Abro tuberia para escribir 
            int fifoClienteServidor = open("/tmp/clienteServidor", O_WRONLY);
            write(fifoClienteServidor,comando.c_str(),strlen(comando.c_str())+1);
            close(fifoClienteServidor);

            // Abro tuberia para leer
            fifoClienteServidor = open("/tmp/clienteServidor", O_RDONLY);
            read(fifoClienteServidor,respuesta,sizeof(respuesta));
            close(fifoClienteServidor);

            cout << "Mensaje recibido del SERVER: " << respuesta << endl;
            //FIN DE LA CONEXION

        //} else {
          //  cout << "Sintaxis incorrecta" << endl;
        //}

        cout << "INGRESE COMANDO: ";
        getline(cin, comando);
        // comando = entrada.substr(0, entrada.find(delimiter));
        // valor_comando = entrada.substr(entrada.find(delimiter) + delimiter.length());
    }

    return 0;
}

void help()
{
    cout << "---------------------------------------------------------------------------------------" << endl;
    cout << "---------------------------------------------------------------------------------------" << endl;
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