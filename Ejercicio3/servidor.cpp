#include <iostream>
#include <unistd.h> //abrir y cerrar fifos
#include <sstream> // funciones de string
#include <fstream> //manejo de archivos
#include <list>

#include <string>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sched.h>

using namespace std;

typedef struct {
    int id;
	string descripcion;
    int precio;
    int costo;
    int stock;
} Producto;

void signal_handler(int signal_num) { 
   cout << "Se recibió la señal (" << signal_num << "). Se interrumpe el proceso. \n"; 
   unlink("./fifo/clienteServidor");
   exit(signal_num);   
} 


string ejecutarSTOCK(string sentencia, char* nombreArchivo) {

    string delimiter = " ";
    string datoId = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    ifstream file(nombreArchivo);
    string linea, id, descripcion, precio, costo, stock;

    while (getline(file, linea))
	{
		stringstream input_stringstream(linea);
		getline(input_stringstream, id, ';');
		getline(input_stringstream, descripcion, ';');
		getline(input_stringstream, precio, ';');
		getline(input_stringstream, costo, ';');
        getline(input_stringstream, stock, ';');

        if((strcmp(id.c_str(),datoId.c_str())) == 0){
            file.close();
            return "\n" + descripcion + " " + stock + "u";
        }
	}

    file.close();

    return "No se encontro el producto";
}

string ejecutarSIN_STOCK(char* nombreArchivo) {

    ifstream file(nombreArchivo);
    string linea, id, descripcion, precio, costo, stock, resultado;
    int index=0;

    while (getline(file, linea))
	{
		stringstream input_stringstream(linea);
		getline(input_stringstream, id, ';');
		getline(input_stringstream, descripcion, ';');
		getline(input_stringstream, precio, ';');
		getline(input_stringstream, costo, ';');
        getline(input_stringstream, stock, ';');

        if((strcmp(stock.c_str(),"0")) == 0){
            index++;
            resultado +=  id + " " + descripcion + " $" + costo + "\n"; 
        }
	}

    file.close();

    return "\n" + resultado;
}

string ejecutarREPO(string sentencia, char* nombreArchivo) {

    string delimiter = " ";
    string cantidad = sentencia.substr(sentencia.find(delimiter) + delimiter.length());

    ifstream file(nombreArchivo);
    string linea, id, descripcion, precio, costo, stock, resultado;
    int calculo;

    while (getline(file, linea))
	{
		stringstream input_stringstream(linea);
		getline(input_stringstream, id, ';');
		getline(input_stringstream, descripcion, ';');
		getline(input_stringstream, precio, ';');
		getline(input_stringstream, costo, ';');
        getline(input_stringstream, stock, ';');

        if((strcmp(stock.c_str(),"0")) == 0){
            calculo = stoi(costo,nullptr,10) * stoi( cantidad,nullptr, 10);
            resultado += id + " " 
            + "$" +  to_string(calculo) + "\n"; 
        }
	}

    file.close();

    return "\n" + resultado;
}

string ejecutarLIST(char* nombreArchivo) {

    ifstream file(nombreArchivo);
    string linea, id, descripcion, precio, costo, stock, resultado;
    int index=0;

    while (getline(file, linea))
	{
		stringstream input_stringstream(linea);
		getline(input_stringstream, id, ';');
		getline(input_stringstream, descripcion, ';');
		getline(input_stringstream, precio, ';');
		getline(input_stringstream, costo, ';');
        getline(input_stringstream, stock, ';');

        if(strcmp(id.c_str(), "ID") != 0){
            index++;
            resultado += id + " " + descripcion + " $" + precio + "\n"; 
        }

	}

    file.close();

    return "\n" + resultado;
}

void ejecutarQUIT() {
    exit(0);
}

string realizarAccion(string comando, char* nombreArchivo) {

    stringstream registro(comando);
    string accion;

    getline(registro, accion, ' '); //  obtiene la primer palabra del comando
    
    if(accion == "STOCK") { return ejecutarSTOCK(comando, nombreArchivo); }
    if(accion == "SIN_STOCK") { return ejecutarSIN_STOCK(nombreArchivo); }
    if(accion == "REPO") { return ejecutarREPO(comando, nombreArchivo); }
    if(accion == "LIST") { return ejecutarLIST(nombreArchivo); }
    if(accion == "QUIT") ejecutarQUIT(); 

    return "Ha ocurrido un error inesperado, por favor intentelo mas tarde nuevamente";
}

void help()
{
    cout << "------------------------------------------------------------------------" << endl;
    cout << "------------------------------------------------------------------------" << endl;
    cout << "Servidor que ejecuta comandos recibidos mediante un cliente" << endl;
    cout << "Los comandos aceptados " << endl;
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
    cout << "El servidor se ejecuta de la siguiente manera:" << endl;
    cout << "./servidor NombreArchivo" << endl;
    cout << "------------------------------------------------------------------------" << endl;
}

int main(int argc, char *argv[]){

    if (argc != 2) {
        cout << "La cantidad de parametros es incorrecta" << endl;
        return 1;
    }
    else if (!strcmp(argv[1],"--help") || !strcmp(argv[1], "-h")) {
        help();
        return 0;
    }
    

    //Señal 
    signal( SIGUSR1 , signal_handler);
    //signal(SIGINT, signal_callback_handler);
    //crea otra tuberia con el mismo nombre que la del cliente
    mkfifo("/tmp/clienteServidor",0666);

    char* nombreArchivo = argv[1];

    signal(SIGINT, SIG_IGN);
    while (1) {
        char contenido[1024];

        int fifoClienteServidor = open("/tmp/clienteServidor", O_RDONLY);
        read(fifoClienteServidor,contenido,sizeof(contenido));
        close(fifoClienteServidor);

        string respuesta = realizarAccion(string(contenido), argv[1]);

        fifoClienteServidor = open("/tmp/clienteServidor", O_WRONLY);
        write(fifoClienteServidor,respuesta.c_str(),strlen(respuesta.c_str())+1);
        close(fifoClienteServidor);
    }
    
    return EXIT_SUCCESS;
}