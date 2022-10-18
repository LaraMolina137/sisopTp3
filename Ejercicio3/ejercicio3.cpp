#include <iostream>
#include <cstring>
#include <sstream>
#include <iterator>

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

    string entrada;

    //LOGICA DEL CLIENTE
    cout << "INGRESE COMANDO: ";
    getline(cin, entrada);

    //Separa el comando de consulta
    stringstream input_stringstream(entrada); 
    string comando, sigue_comando;
    char delimitador =' ';
    getline(input_stringstream, comando, delimitador);
    getline(input_stringstream, sigue_comando, delimitador);

    //validar comando de consulta


    // while(accion != "quit"){

    //     if(isValidSentence(accion)){

    //         //INICIANDO LA CONEXION DE FIFO
    //         char respuesta[1000];

    //         int fifoClienteServidor = open("/tmp/clienteServidor", 01);
    //         write(fifoClienteServidor,accion.c_str(),strlen(accion.c_str())+1);
    //         close(fifoClienteServidor);

    //         fifoClienteServidor = open("/tmp/clienteServidor", 00);
    //         read(fifoClienteServidor,respuesta,sizeof(respuesta));
    //         close(fifoClienteServidor);

    //         cout << "Mensaje recibido del SERVER: " << respuesta << endl;
    //         //FIN DE LA CONEXION

    //     } else {
    //         cout << "Sintaxis incorrecta" << endl;
    //     }

    //     cout << "INGRESE COMANDO: ";
    //     getline(cin, accion);
    // }

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