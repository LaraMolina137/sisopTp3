#include <sys/types.h>  // socket(), bind()
#include <sys/socket.h> // socket(), bind(), inet_addr()
#include <netinet/in.h> // inet_addr()
#include <arpa/inet.h>  // inet_addr()
#include <string.h>     // bzero()
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define FALSE 0
#define TRUE 1
#define TAMBUF 1024
#define MAXQ 1
//#define PUERTO 5001
#define FAIL -1
#define MAX_INTENTOS 6
#define CANT_CARACTERES 2 // indica cantidad de caracteres a enviar en el SEND
#define MAX_PALABRA 22    // largo de palabra
int jugando = FALSE;
int socketServidor;
int cliente;
int pidServer;
int PUERTO;

void ayuda();
void terminar(int);
void cerrar();
void cliente_murio(int);
void sin_control_c(int);

char *procesar_letra(char *, char *, char *, int *, int *);
int levantarServidorPara1Conexion(int, int);
void manejarServidorPara1Conexion(int);
void cerrarSocket(int);

int main(int argc, char *argv[])
{

    // controlar seniales
    //	  	SIGUSR1: finaliza si no hay partida
    //		  SIGINT: ignorarla
    //	levantar el servidor
    //	atender el cliente
    //		  asignar palabra
    //		  control del juego

    // signal(SIGINT, sin_control_c);
    signal(SIGUSR1, terminar);
    pidServer = getpid();

    if (argc == 2)
    {
        if (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0)
        {
            ayuda();
            exit(1);
        }
    }
    if (argc > 2)
    {
        printf("Cantidad de Parametros recibidos incorrecta, para ejecutar la ayuda introduzca ./servidor -h , o ./servidor -? , o ./servidor -help\n");
        exit(1);
    }

    printf("comienzo programa\n");
    printf("Server corriendo con PID = %d\n", pidServer);
    printf("Por favor, ingrese un puerto: \n");

    PUERTO = strtol(argv[1], NULL, 10);
    printf("entrada: %d\n", PUERTO);

    socketServidor = levantarServidorPara1Conexion(PUERTO, MAXQ);

    while (TRUE)
    {
        printf("Esperando conexion con el cliente.....\n");
        manejarServidorPara1Conexion(socketServidor);
    }

    cerrarSocket(socketServidor);

    printf("Se desconectó\n");
    return 0;
}

// ***************************************************************************************
// *                                      FUNCIONES                                      *
// ***************************************************************************************

// *******************************
// *     Creacion del socket  	 *
// *******************************
int newSocket()
{
    int fileDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (fileDescriptor == FAIL)
        printf("[ERROR] Funcion socket");
    return fileDescriptor;
}

// *******************************************
// *     Asociar una dirección al socket  	 *
// *******************************************
struct sockaddr_in asociarSocket(int sockfd, int puerto)
{
    struct sockaddr_in miDireccion;
    miDireccion.sin_family = AF_INET;
    miDireccion.sin_port = htons(puerto);
    miDireccion.sin_addr.s_addr = 0; // htonl(INADDR_ANY); // Usa mi direccion IP
    memset(&(miDireccion.sin_zero), '\0', 8);

    // Si el puerto esta en uso, lanzamos error
    int enUso = 1;
    int estaEnUso = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&enUso, sizeof(enUso));
    if (estaEnUso == FAIL)
        printf("[ERROR] No es posible reutilizar el socket\n");

    // Funcion bind
    int funcionBind = bind(sockfd, (struct sockaddr *)&miDireccion, sizeof(struct sockaddr));
    if (funcionBind == FAIL)
        printf("[ERROR] Funcion BIND: No se pudo asociar con el puerto\n");

    return miDireccion;
}

// ***************************************************
// *   Habilitar el socket para recibir conexiones	 *
// ***************************************************
void escucharSocket(int sockfd, int conexionesEntrantesPermitidas)
{
    int funcionListen = listen(sockfd, conexionesEntrantesPermitidas);
    if (funcionListen == FAIL)
        printf("[ERROR] Funcion listen\n");
}

// *******************************
// *      Cerrar el socket    	 *
// *******************************
void cerrarSocket(int sockfd)
{
    int funcionClose = close(sockfd);
    if (funcionClose == FAIL)
        printf("[ERROR] Funcion CLOSE: Error al cerrar el file descriptor\n");
    char *mensaje = malloc(50);
    sprintf(mensaje, "Socket %d cerrado\n", sockfd);
    printf(mensaje);
    free(mensaje);
}

int aceptarConexionSocket(int sockfd)
{
    struct sockaddr_storage cliente;
    unsigned int addres_size = sizeof(cliente);
    int fdCliente = accept(sockfd, (struct sockaddr *)&cliente, &addres_size);
    if (fdCliente == FAIL)
        printf("[ERROR] Funcion accept\n");

    jugando = TRUE;
    return fdCliente;
}

char *obtener_palabra()
{
    printf("obtener_palabra\n");
    FILE *archivo;
    char *line = NULL;

    int i = 0;
    size_t len = 0;
    ssize_t read;

    archivo = fopen("palabras.txt", "r");

    // archivo = fopen("/home/ubuntu/Escritorio/Facultad/SO/C/ej5/palabras.txt", "r");
    if (NULL == archivo)
        exit(EXIT_FAILURE);

    fseek(archivo, 0L, SEEK_END);
    long tam = ftell(archivo);
    rewind(archivo);
    char *palabra[tam];

    while ((read = getline(&line, &len, archivo)) != -1)
    {
        palabra[i] = (char *)malloc(strlen(line));
        memset(palabra[i], 0, strlen(line));
        memcpy(palabra[i], line, strlen(line));
        palabra[i][strlen(line)] = '\0';
        // strcpy(palabra[i], line);
        i++;
    }
    srand(time(NULL));
    int r = (rand() % i);
    fclose(archivo);
    char *palabraRetorna = (char *)malloc(strlen(palabra[r]));
    memset(palabraRetorna, 0, strlen(palabra[r]));
    memcpy(palabraRetorna, palabra[r], strlen(palabra[r]));
    palabraRetorna[strlen(palabra[r])] = '\0';

    // liberar espacio de palabra[i]
    int j = 0;
    while (j < i)
    {
        free(palabra[j]);
        j++;
    }
    return palabraRetorna;
}

char *procesar_letra(char *resultadoLetra, char *palabra, char *letra, int *cantIntentos, int *cantAciertos)
{
    printf("procesar_letra\n");
    static int pos, len;
    pos = strcspn(palabra, letra);
    len = strlen(palabra) - 1;

    if (pos >= len)
    {
        // la letra no está en la palabra
        (*cantIntentos)--;
    }
    else
    {
        for (int i = 0; i < len; i++)
        {
            if (palabra[i] == *letra)
            {
                resultadoLetra[i] = *letra;
                (*cantAciertos)++;
            }
        }
    }
    return resultadoLetra;
}

void atenderCliente(int fdClienteNuevo)
{
    printf("atenderCliente\n");
    //		  asignar palabra
    // armar mensaje cant.letras, cant intentos
    char *palabra = obtener_palabra();
    int cantLetras = strlen(palabra);
    int cantIntentos = MAX_INTENTOS;
    int cantAciertos = 0;
    int desplazamiento = 0;
    printf("---palabra: %s\n", palabra);
    int tam = sizeof(pidServer) + sizeof(cantIntentos) + sizeof(cantLetras) + cantLetras;
    char *resultadoLetra = malloc(cantLetras);
    for (int i = 0; i < cantLetras; i++)
    {
        resultadoLetra[i] = '-';
    }
    resultadoLetra[cantLetras - 1] = '\0';
    //		  control del juego
    // envio longitud de palabra al cliente
    // armo primer mensaje  ----------------------------------------------- listo
    // estructura: cantIntentos + cantLetras + resultadoLetra
    void *mensaje = malloc(tam);
    memset(mensaje, 0, tam);
    memcpy(mensaje + desplazamiento, &pidServer, sizeof(pidServer));
    desplazamiento += sizeof(pidServer);
    memcpy(mensaje + desplazamiento, &cantIntentos, sizeof(cantIntentos));
    desplazamiento += sizeof(cantIntentos);
    memcpy(mensaje + desplazamiento, &cantLetras, sizeof(cantLetras));
    desplazamiento += sizeof(cantLetras);
    memcpy(mensaje + desplazamiento, resultadoLetra, cantLetras);
    desplazamiento += cantLetras;
    int bytesEnv = send(fdClienteNuevo, mensaje, desplazamiento, 0);
    free(mensaje);
    //--------------------------------------------------------------------------
    int respuesta = 1;
    while (cantIntentos > 0 && (cantLetras - 1) != cantAciertos && respuesta > 0)
    {
        // espero letra
        // int tamEntrada = sizeof(char);
        int tamEntrada = 27;
        char *letra = malloc(tamEntrada);
        ;
        respuesta = recv(fdClienteNuevo, &letra, tamEntrada, 0);
        printf("Respuesta: %d\n", respuesta);
        // letra = tolower(letra);
        letra[respuesta + 1] = '\0';
        printf("Letra recibida: %s\n", letra);

        // validar la letra recibida si acerto o no
        resultadoLetra = procesar_letra(resultadoLetra, palabra, &letra, &cantIntentos, &cantAciertos);

        // armo resultado a enviar
        //  estructura: cantIntentos + cantAciertos + cantLetras + resultadoLetra
        desplazamiento = 0;
        tam = sizeof(cantIntentos) + sizeof(cantAciertos) + sizeof(cantLetras) + cantLetras;
        void *mensaje = malloc(tam);
        memset(mensaje, 0, tam);
        memcpy(mensaje + desplazamiento, &cantIntentos, sizeof(cantIntentos));
        desplazamiento += sizeof(cantIntentos);
        memcpy(mensaje + desplazamiento, &cantAciertos, sizeof(cantAciertos));
        desplazamiento += sizeof(cantAciertos);
        memcpy(mensaje + desplazamiento, &cantLetras, sizeof(cantLetras));
        desplazamiento += sizeof(cantLetras);
        memcpy(mensaje + desplazamiento, resultadoLetra, cantLetras);
        desplazamiento += cantLetras;
        int bytesEnv = send(fdClienteNuevo, mensaje, desplazamiento, 0);
        free(mensaje);
    }

    if (cantIntentos == 0)
    {
        tam = sizeof(cantLetras) + cantLetras;
        desplazamiento = 0;
        void *mensaje = malloc(tam);
        memset(mensaje, 0, tam);
        memcpy(mensaje + desplazamiento, &cantLetras, sizeof(cantLetras));
        desplazamiento += sizeof(cantLetras);
        memcpy(mensaje + desplazamiento, palabra, cantLetras);
        desplazamiento += cantLetras;
        send(fdClienteNuevo, mensaje, desplazamiento, 0);
        free(mensaje);
    }
}

// Retorna el socket servidor creado
int levantarServidorPara1Conexion(int puerto, int cant_max_conexiones)
{
    int socket;

    socket = newSocket();
    asociarSocket(socket, puerto);
    escucharSocket(socket, cant_max_conexiones);

    return socket;
}

void manejarServidorPara1Conexion(int socketServidor)
{
    cliente = aceptarConexionSocket(socketServidor);

    atenderCliente(cliente);

    cerrarSocket(cliente);
    jugando = FALSE;
}

void cliente_murio(int s)
{
    puts("El cliente se ha desconectado. Abortando...\n");
    signal(SIGINT, SIG_DFL);
    cerrar();
    exit(EXIT_SUCCESS);
}

void terminar(int signalNum)
{
    puts("TERMINAR\n");
    if (signalNum != SIGUSR1)
        return;

    if (jugando)
    {
        puts("Partida en curso, no interrumpir\n");
        return;
    }

    puts("SIGUSR1 recibido fuera de partida, saliendo...\n");
    cerrarSocket(socketServidor);
    exit(EXIT_SUCCESS);
}

void sin_control_c(int signum)
{
    /*     puts("salgo...");
        puts("pero porque yo quiero eh!!");
        exit(EXIT_SUCCESS);
     */
}

void cerrar()
{
    cerrarSocket(socketServidor);
    cerrarSocket(cliente);
}

void ayuda()
{
    printf("\n********************************AYUDA******************************\n");
    printf("Juego del ahorcado (Hangman) \n");
    printf("Se genera una comunicación cliente-servidor mediante socket.\n");
    printf("El servidor debe recibir como parámetro el puerto que escuchara y el\n");
    printf("cliente recibirá la IP y puerto del servidor a donde deberá conectarse.\n");
    printf("Se debe ejecutar primero el servidor y a continuacion el cliente podra\n");
    printf("realizar la solicitud.\n");
    printf("Para ejecutar el servidor ingrese: ./servidor PUERTO\n");
    printf("Para ejecutar el cliente ingrese: ./cliente PUERTO DIRECCIONIP\n");
}