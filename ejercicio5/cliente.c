#include <sys/types.h>	// socket(), bind()
#include <sys/socket.h> // socket(), bind(), inet_addr()
#include <netinet/in.h> // inet_addr()
#include <arpa/inet.h>	// inet_addr()
#include <string.h>		// bzero(), strerror()
#include <stdio.h>
#include <errno.h> // variable global errno
#include <stdlib.h>
#include <signal.h>

#define TRUE 1
#define TAM_ENTRADA 50
#define TAMBUF 1024
//#define PUERTO 5001
#define MEM_SIZE 1024
char *IPDESTINO; // = "127.0.0.1";
#define FAIL -1
int pidServer;
int PUERTO;

struct formato
{
	int forCantLetras;
	int forCantIntentos;
	int forCantAciertos;
};

void ayuda();
void salir(int);
int conectarCliente(const char *, int);

void iniciar_abm(int);
void serializar_enviar(int, int, char *);
void abm_gatos(int);
void cerrarSocket(int);

int main(int argc, const char *argv[])
{

	/*  - cliente
		controlar seniales
			SIGINT: finalizar el juego avisando al servidor
		conectar con el servidor
		iniciar el juego
		abm_gatos */

	// Conectar cliente con el servidor
	signal(SIGINT, salir);

	if (argc == 2)
	{
		if (strcmp(argv[1], "-help") == 0 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-?") == 0)
		{
			ayuda();
			exit(1);
		}
	}
	if (argc > 3)
	{
		printf("Cantidad de Parametros recibidos incorrecta, para ejecutar la ayuda introduzca ./cliente -h , o ./cliente -? , o ./cliente -help\n");
		exit(1);
	}

	PUERTO = strtol(argv[1], NULL, 10);
	printf("puerto: %d\n", PUERTO);
	IPDESTINO = argv[2];
	printf("ipdestino: %s\n", IPDESTINO);

	int socketServidor = conectarCliente(IPDESTINO, PUERTO);

	iniciar_abm(socketServidor);
	abm_gatos(socketServidor);

	cerrarSocket(socketServidor);

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
		printf("Funcion socket");
	return fileDescriptor;
}

int conectarSocket(int sockfd, const char *ipDestino, int puerto)
{
	struct sockaddr_in datosServidor;

	// Se le asigna una direccion al socket
	datosServidor.sin_family = AF_INET;
	datosServidor.sin_port = htons(puerto);
	datosServidor.sin_addr.s_addr = inet_addr(ipDestino);
	memset(&(datosServidor.sin_zero), '\0', 8);

	// se conecta con el servidor
	int funcionConnect = connect(sockfd, (struct sockaddr *)&datosServidor, sizeof(struct sockaddr));
	if (funcionConnect == FAIL)
		return FAIL;
	return 0;
}

int conectarCliente(const char *ipDestino, int puerto)
{
	int socket;

	socket = newSocket();

	conectarSocket(socket, ipDestino, puerto);

	return socket;
}

void iniciar_abm(int sockfd)
{
	// Primer mensaje para comenzar
	// se recibe respuesta del servidor
	// estructura del recv: pidServer
	recv(sockfd, &pidServer, sizeof(pidServer), 0);

	printf("pidServer: %d\n", pidServer);
	printf("ABM - Refugio de Gatos\n");

	return;
}

void serializar_enviar(int socketServidor, int comando, char *cadena)
{
	// printf("sole - funcion serializar_enviar\n");
	int tamCadena = strlen(cadena) + 1;
	// estructura: comando + nombre + raza + sexo(M-H) + CA(castrado)/SC(sin castrar)
	// estructura: ALTA SnowBall siames M CA
	// estructura send: int (comando) + int (long cadena) +  mensaje
	int desplazamiento = 0;
	int tamMensaje = sizeof(comando) + sizeof(tamCadena) + tamCadena;
	void *mensaje = malloc(tamMensaje);
	memset(mensaje, 0, tamMensaje);
	memcpy(mensaje + desplazamiento, &comando, sizeof(comando));
	desplazamiento += sizeof(comando);
	memcpy(mensaje + desplazamiento, &tamCadena, sizeof(tamCadena));
	desplazamiento += sizeof(tamCadena);
	memcpy(mensaje + desplazamiento, cadena, tamCadena);
	desplazamiento += tamCadena;

	int bytesEnv = send(socketServidor, mensaje, desplazamiento, 0);
	free(mensaje);
	// printf("sole - bytesEnv: %d\n", bytesEnv);
	// printf("sole - funcion serializar_enviar fin\n");
}

void abm_gatos(int socketServidor)
{
	// ciclo
	// ingresa comando
	// validar que sea comando valido
	//   convierto a mayuscula
	//     - si ok: envio el comando al servidor
	//     - si not ok: solicito nuevo comando
	// recibe estado de la operacion

	char *letra = malloc(TAM_ENTRADA);

	while (TRUE)
	{
		// se recibe y envia mensaje al servidor
		printf("\nIngrese comando: ");
		fgets(letra, 40, stdin); // Leyendo el comando ingresado
		int tamLetra = strlen(letra);
		letra[tamLetra] = '\0';

		int tam_mje_total = strlen(letra);

		char *cadena = malloc(tam_mje_total + 1);
		memset(cadena, 0, tam_mje_total + 1);
		memcpy(cadena, letra + 0, tam_mje_total);
		cadena[tam_mje_total] = '\0';

		char delimitador[] = " ";
		char *comando = strtok(cadena, delimitador);

		int tamComando = strlen(comando);
		comando[tamComando] = '\0';
		tamComando = strlen(comando);

		// Valido comando ingresado
		while (strcmp(comando, "ALTA") != 0 && strcmp(comando, "BAJA") != 0 && strcmp(comando, "CONSULTA") != 0)
		{
			printf("Comando invalido. Debe ingresar un comando: ");
			fgets(letra, 40, stdin);
			strcpy(cadena, letra);
			comando = strtok(letra, delimitador);
		}

		if (!strcmp(comando, "ALTA"))
		{
			int comandoNum = 1;
			serializar_enviar(socketServidor, comandoNum, letra);
			// recibo respuesta alta ok o nok
			// si nok evaluar errores e informar
		}
		if (!strcmp(comando, "BAJA"))
		{
			int comandoNum = 2;
			serializar_enviar(socketServidor, comandoNum, letra);
			// recibo respuesta baja ok o nok
			// si nok evaluar errores e informar
		}
		if (!strcmp(comando, "CONSULTA"))
		{
			int comandoNum = 3;
			// printf("sole - son iguales consulta - %s\n", letra);
			serializar_enviar(socketServidor, comandoNum, letra);
			// recibo respuesta consulta ok o nok
			// si nok evaluar errores e informar
		}
	}
	free(letra);
}

// *******************************
// *      Cerrar el socket    	 *
// *******************************
void cerrarSocket(int sockfd)
{
	int funcionClose = close(sockfd);
	if (funcionClose == FAIL)
		printf("[ERROR] Funcion CLOSE: Error al cerrar el file descriptor. \n");
	char *mensaje = malloc(50);
	sprintf(mensaje, "Socket %d cerrado\n", sockfd);
	printf(mensaje);
	free(mensaje);
}

void salir(int s)
{
	printf("\nSeñal recibida = %s\n", strsignal(s));
	// printf("Mando SIGUSR2 = %d al pid %d\n", SIGUSR2, pidServer);
	// kill(pidServer, SIGUSR2);
	signal(SIGINT, SIG_DFL);
	exit(EXIT_SUCCESS);
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