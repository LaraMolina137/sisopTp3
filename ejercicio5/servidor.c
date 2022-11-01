#include <sys/types.h>  // socket(), bind()
#include <sys/socket.h> // socket(), bind(), inet_addr()
#include <netinet/in.h> // inet_addr()
#include <arpa/inet.h>  // inet_addr()
#include <string.h>     // bzero()
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define OK 0
#define ALTA 1
#define BAJA 2
#define CONSULTA 3
#define ERROR_DATOS_ALTA 1
#define ERROR_DATO_RAZA 2
#define ERROR_DATO_SEXO 3
#define ERROR_DATO_CASTRAR 4
#define ERROR_NOMBRE_REPETIDO 5
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

void mensaje_inicio(int);
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

void mensaje_inicio(int fdClienteNuevo)
{
    int desplazamiento = 0;
    int tam = sizeof(pidServer);
    void *mensaje = malloc(tam);
    memset(mensaje, 0, tam);
    memcpy(mensaje + desplazamiento, &pidServer, sizeof(pidServer));
    desplazamiento += sizeof(pidServer);
    int bytesEnv = send(fdClienteNuevo, mensaje, desplazamiento, 0);
    free(mensaje);
}

char *desserializar_recibir(int fdClienteNuevo)
{
    int tamEntrada = sizeof(int);
    int respuesta = 0;
    // ------recibo tamanio mensaje-----
    tamEntrada = sizeof(int);
    int tamMensaje = 0;
    respuesta = recv(fdClienteNuevo, &tamMensaje, tamEntrada, 0);
    // ------recibo mensaje-----
    char *mensaje = malloc(tamMensaje + 1);
    memset(mensaje, 0, tamMensaje + 1);
    respuesta = recv(fdClienteNuevo, mensaje, tamMensaje, 0);
    mensaje[tamMensaje] = '\0';
    printf("mensaje recibida: %s\n", mensaje);
    return mensaje;
}

char *obtener_substring(char *origen, int offset, int tam)
{
    char *palabra = malloc(tam + 1);
    memset(palabra, 0, tam + 1);
    memcpy(palabra, origen + offset, tam);
    palabra[tam] = '\0';
    return palabra;
}

char *obtener_dato(int tam_mje, char *entrada, int tam_offset)
{
    int tam_mje2 = tam_mje - tam_offset;
    char *subtring = obtener_substring(entrada, tam_offset, tam_mje2);
    char delimitador[] = " ";
    char *salida = strtok(subtring, delimitador);
    return salida;
}

int validar_datos_alta(char *entrada)
{
    // 1: faltan datos para el alta
    // 2: falta dato raza para el alta
    // 3: falta dato sexo para el alta
    // 4: falta dato castrar para el alta
    printf("---validar_datos_alta\n");
    printf("---entrada: %s\n", entrada);
    int tam_mje = strlen(entrada);
    int tam_offset = 0;
    char *copia = obtener_substring(entrada, tam_offset, tam_mje);
    char *nombre = obtener_dato(tam_mje, copia, tam_offset);
    printf("---nombre: %s\n", nombre);
    if (nombre == NULL)
    {
        return ERROR_DATOS_ALTA;
    }
    else
    {
        printf("---falta validar nombre duplicado\n");
    }

    int tamNombre = strlen(nombre) + 1;
    tam_offset += tamNombre;
    char *raza = obtener_dato(tam_mje, copia, tam_offset);
    printf("---raza: %s\n", raza);
    if (raza == NULL)
    {
        return ERROR_DATO_RAZA;
    }

    int tamRaza = strlen(raza) + 1;
    tam_offset += tamRaza;
    char *sexo = obtener_dato(tam_mje, copia, tam_offset);
    printf("---sexo: %s\n", sexo);
    if (sexo == NULL)
    {
        return ERROR_DATO_SEXO;
    }

    int tamSexo = strlen(sexo) + 1;
    tam_offset += tamSexo;
    char *castrar = obtener_dato(tam_mje, copia, tam_offset);
    printf("---castrar: %s", castrar);
    if (castrar == NULL)
    {
        return ERROR_DATO_CASTRAR;
    }
    int tamCastrar = strlen(castrar);
    printf("---fin validar_datos_alta\n");
}

void evaluo_error(valido)
{
    switch (valido)
    {
    case ERROR_DATOS_ALTA:
        printf("---ERROR_DATOS_ALTA\n");
        break;
    case ERROR_DATO_RAZA:
        printf("---ERROR_DATO_RAZA\n");
        break;
    case ERROR_DATO_SEXO:
        printf("---ERROR_DATO_SEXO\n");
        break;
    case ERROR_DATO_CASTRAR:
        printf("---ERROR_DATO_CASTRAR\n");
        break;
    case ERROR_NOMBRE_REPETIDO:
        printf("---ERROR_NOMBRE_REPETIDO\n");
        break;

    default:
        break;
    }
}

int grabo_archivo(char *mensaje)
{
    FILE *archivo;
    char *line = NULL;

    int i = 0;
    size_t len = 0;
    ssize_t read;

    archivo = fopen("abmGatos.txt", "a");

    if (NULL == archivo)
        exit(EXIT_FAILURE);

    fprintf(archivo, mensaje);
    fclose(archivo);
}

int trato_alta(char *mensaje)
{
    int tam_mje_total = strlen(mensaje);
    // revisar de sacar el numero 4 y obtenerlo de otra manera
    int tam_mje = tam_mje_total - 4; // 5: "ALTA "
    char *subcadena = obtener_substring(mensaje, 4, tam_mje);
    int valido = validar_datos_alta(subcadena);
    if (valido != OK)
    {
        evaluo_error(valido);
    }
    // revisar que cuando graba lo hace asi " SnowBall siames M CA", sacarle el espacio adelante
    grabo_archivo(subcadena);
}

char existeProducto(char *nombre)
{
    FILE *archivo;
    char existe = 0;
    char *linea = NULL;
    size_t len = 0;
    /* Abre el archivo en modo lectura */
    archivo = fopen("abmGatos.txt", "r");
    if (archivo != NULL)
    {
        getline(&linea, &len, archivo);
        while (!feof(archivo))
        {
            int tam_mje = strlen(linea);
            int tam_offset = 0;
            char *copia = obtener_substring(linea, tam_offset, tam_mje);
            char *campoNombre = obtener_dato(tam_mje, copia, tam_offset);
            if (strcmp(campoNombre, nombre))
            {
                existe = 1;
                break;
            }
            getline(&linea, &len, archivo);
        }
        /* Cierra el archivo */
        fclose(archivo);
    }
    // existe = 1/ no existe = 0
    return existe;
}

int eliminarProducto(char *nombre)
{
    FILE *archivo;
    FILE *temporal;
    char elimina = 0;
    char *linea = NULL;
    size_t len = 0;

    archivo = fopen("abmGatos.txt", "r");
    temporal = fopen("temporal.txt", "w");

    if (archivo == NULL || temporal == NULL)
    {
        elimina = 0;
    }
    else
    {
        /* Se copia en el archivo temporal los registros válidos */
        getline(&linea, &len, archivo);
        while (!feof(archivo))
        {
            int tam_mje = strlen(linea);
            int tam_offset = 0;
            char *copia = obtener_substring(linea, tam_offset, tam_mje);
            char *campoNombre = obtener_dato(tam_mje, copia, tam_offset);
            int cmp = strcmp(nombre, campoNombre);
            if (strcmp(nombre, campoNombre))
            {
                fprintf(temporal, linea);
            }
            getline(&linea, &len, archivo);
        }
        /* Se cierran los archivos antes de borrar y renombrar */
        fclose(archivo);
        fclose(temporal);

        remove("abmGatos.txt");
        rename("temporal.txt", "abmGatos.txt");

        elimina = 1;
    }
    return elimina;
}

int trato_baja(char *mensaje)
{
    int tam_mje_total = strlen(mensaje);
    // revisar de sacar el numero 4 y obtenerlo de otra manera
    int tam_mje = tam_mje_total - 5; // 5: "ALTA "
    char *nombre = obtener_substring(mensaje, 5, tam_mje);
    nombre[strlen(nombre) - 1] = '\0';
    // int valido = validar_datos_alta(subcadena);
    // if (valido != OK)
    //{
    //     evaluo_error(valido);
    // }
    /* Se verifica que el gato a buscar, exista */
    if (existeProducto(nombre))
    {
        if (eliminarProducto(nombre))
        {
            printf("\n\tProducto eliminado satisfactoriamente.\n");
        }
        else
        {
            printf("\n\tEl producto no pudo ser eliminado\n");
        }
    }
    else
    {
        /* El gato no existe */
        printf("\n\tEl gato con nombre %s no existe.\n", nombre);
    }
    return 1;
}

void consulta_general()
{
    printf("---consulta_general\n");
}

void consulta_particular()
{
    printf("---consulta_particular\n");
}

void trato_consulta(char *mensaje)
{
    // printf("---mensaje: %s\n", mensaje);
    int tam_mje = strlen(mensaje);
    int tam_offset = 0;
    char *copia = obtener_substring(mensaje, tam_offset, tam_mje);
    char *nombre = obtener_dato(tam_mje, copia, tam_offset);
    // printf("---nombre: %s\n", nombre);
    if (nombre == NULL)
    {
        consulta_general();
    }
    else
    {
        consulta_particular(nombre);
    }
}

void atenderCliente(int fdClienteNuevo)
{
    printf("atenderCliente\n");
    //		  asignar palabra
    int tam = sizeof(pidServer);
    int desplazamiento = 0;
    mensaje_inicio(fdClienteNuevo);

    int respuesta = 1;
    while (TRUE)
    {
        // ------recibo comando-----
        int tamEntrada = sizeof(int);
        int comando = 0;

        respuesta = recv(fdClienteNuevo, &comando, tamEntrada, 0);
        if (comando != 0)
        {
            char *mensaje = desserializar_recibir(fdClienteNuevo);

            switch (comando)
            {
            case ALTA:
                printf("-----ENTRO POR ALTA\n");
                trato_alta(mensaje);
                break;
            case BAJA:
                printf("-----ENTRO POR BAJA\n");
                trato_baja(mensaje);
                break;
            case CONSULTA:
                printf("-----ENTRO POR CONSULTA\n");
                trato_consulta(mensaje);
                break;
            default:
                break;
            }
        }
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
    printf("Refugio de gatos \n");
    printf("Se lleva un registro mínimo de los ingresos (rescates) y los egresos (adopciones)\n");
    printf("Se puede tomar los siguientes comandos: \n");
    printf("ALTA: Datos a ingresar: nombre, raza, sexo (M-H), CA (castrado)/SC(sin castrar)\n");
    printf("Por ejemplo: ALTA SnowBall siamés M CA\n");
    printf("BAJA: Datos a ingresar: nombre\n");
    printf("Por ejemplo: BAJA SnowBall\n");
    printf("CONSULTA: Datos opcional: nombre, si no se ingresa muestra todos los gatos rescatados.\n");
    printf("Por ejemplo: CONSULTA SnowBall\n");
    printf("             CONSULTA\n");
    printf("La salida por pantalla en el cliente serían todos los datos de cada gato.\n");
    printf("          \n");
    printf("El servidor debe recibir como parámetro el puerto que escuchara y el\n");
    printf("cliente recibirá la IP y puerto del servidor a donde deberá conectarse.\n");
    printf("Se debe ejecutar primero el servidor y a continuacion el cliente podra\n");
    printf("realizar la solicitud.\n");
    printf("Para ejecutar el servidor ingrese: ./servidor PUERTO\n");
    printf("Para ejecutar el cliente ingrese: ./cliente PUERTO DIRECCIONIP\n");
}