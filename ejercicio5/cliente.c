#include <sys/types.h>	// socket(), bind()
#include <sys/socket.h> // socket(), bind(), inet_addr()
#include <netinet/in.h> // inet_addr()
#include <arpa/inet.h>	// inet_addr()
#include <string.h>		// bzero(), strerror()
#include <stdio.h>
#include <errno.h> // variable global errno
#include <stdlib.h>
#include <signal.h>

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

void *iniciarJuego(int);
void jugar(int, struct formato *);
void cerrarSocket(int);

int main(int argc, const char *argv[])
{

	/*  - cliente
		controlar seniales
			SIGINT: finalizar el juego avisando al servidor
		conectar con el servidor
		iniciar el juego
		jugar */

	// Conectar cliente con el servidor
	signal(SIGINT, salir);

	if (argc == 3)
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
	printf("entrada: %d\n", PUERTO);
	IPDESTINO = argv[2];
	printf("entrada: %d\n", IPDESTINO);

	int socketServidor = conectarCliente(IPDESTINO, PUERTO);

	struct formato *controlJuego = (struct formato *)iniciarJuego(socketServidor);

	jugar(socketServidor, controlJuego);

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

void *iniciarJuego(int sockfd)
{
	// Primer mensaje para comenzar el juego
	struct formato *controlJuego = malloc(sizeof(struct formato));
	// se recibe respuesta del servidor
	// estructura del recv: pidServer + intentos + acertados + tamPalabra + palabraAdivinar
	recv(sockfd, &pidServer, sizeof(pidServer), 0);

	int cantIntentos = 0;
	recv(sockfd, &cantIntentos, sizeof(cantIntentos), 0);

	int tamPalabra = 0;
	recv(sockfd, &tamPalabra, sizeof(tamPalabra), 0);

	char *palabraAdivinar = malloc(tamPalabra);
	recv(sockfd, palabraAdivinar, tamPalabra, 0);

	controlJuego->forCantLetras = tamPalabra - 1;
	controlJuego->forCantIntentos = cantIntentos;
	printf("pidServer: %d\n", pidServer);
	printf("Comienza el juego HANGMAN\n");
	printf("Palabra a adivinar: %s\t\t", palabraAdivinar);
	printf("Cantidad de intentos: %d\n", controlJuego->forCantIntentos);

	free(palabraAdivinar);

	return controlJuego;
}

void jugar(int socketServidor, struct formato *controlJuego)
{
	// ciclo
	// ingresa letra
	// validar que sea letra valida
	//   convierto a mayuscula
	//     - si ok: envio la letra al servidor
	//     - si not ok: solicito nueva letra
	// recibe estado de la palabra

	printf("\nfuncion jugar-------------\n ");
	int tamEntrada = sizeof(char); // + sizeof(char) + sizeof(char) + sizeof(char) + sizeof(char);
	char *letra = malloc(tamEntrada);

	while (controlJuego->forCantIntentos > 0 && controlJuego->forCantLetras != controlJuego->forCantAciertos)
	{
		// se recibe y envia mensaje al servidor
		printf("\nIngrese comando: ");
		fgets(letra, 40, stdin); // Leyendo el comando ingresado
		printf("sole - letra: %s", letra);
		printf("sole - tamLetra: %d\n", strlen(letra));
		char *cadena = malloc(tamEntrada);
		strcpy(cadena, letra); // hago copia
		char delimitador[] = " ";
		char *comando = strtok(letra, delimitador);
		int tamComando = strlen(comando);

		while (strcmp(comando, "ALTA") != 0 && strcmp(comando, "BAJA") != 0 && strcmp(comando, "CONSULTA") != 0)
		{
			printf("Comando invalido. Debe ingresar un comando: ");
			fgets(letra, 40, stdin);
			strcpy(cadena, letra);
			comando = strtok(letra, delimitador);
		}

		if (!strcmp(comando, "ALTA"))
		{
			printf("sole - son iguales alta - %s\n", comando);
			printf("sole - cadena: %s\n", cadena);
			int tam = strlen(cadena);
			cadena[tam + 1] = '\0';
			// estructura: comando + nombre + raza + sexo(M-H) + CA(castrado)/SC(sin castrar)
			// estructura: ALTA SnowBall siamés M CA
			int bytesEnv = send(socketServidor, cadena, strlen(cadena), 0);
			// int bytesEnv = send(socketServidor, cadena, 40, 0);
			printf("sole - cadena: %d\n", bytesEnv);
			printf("sole - fin alta\n");
		}
		if (!strcmp(comando, "BAJA"))
		{
			printf("sole - son iguales baja - %s\n", comando);
		}
		if (!strcmp(comando, "CONSULTA"))
		{
			printf("sole - son iguales consulta - %s\n", comando);
		}

		// se recibe respuesta del servidor
		// estructura: intentos + acertados + tamPalabra + palabraAdivinar
		// int cantIntentos = 0;
		// recv(socketServidor, &cantIntentos, sizeof(cantIntentos), 0);

		// int cantAciertos = 0;
		// recv(socketServidor, &cantAciertos, sizeof(cantAciertos), 0);

		// int tamPalabra = 0;
		// recv(socketServidor, &tamPalabra, sizeof(tamPalabra), 0);

		// char *palabraAdivinar = malloc(tamPalabra);
		// recv(socketServidor, palabraAdivinar, tamPalabra, 0);

		// if (controlJuego->forCantIntentos == cantIntentos)
		//{
		//	printf("Letra acertada\n");
		// }
		// else
		//	printf("Letra erronea\n");

		// printf("\nPalabra a adivinar: %s\t\t", palabraAdivinar);
		// printf("Cantidad de intentos: %d\n", cantIntentos);
		// controlJuego->forCantIntentos = cantIntentos;
		// controlJuego->forCantAciertos = cantAciertos;
		// free(palabraAdivinar);
	}

	if (controlJuego->forCantIntentos > 0)
	{
		printf("FIN DEL JUEGO: GANASTE!!\n\n");
	}
	else
	{
		int tamPalabra = 0;
		recv(socketServidor, &tamPalabra, sizeof(tamPalabra), 0);

		char *palabraAdivinar = malloc(tamPalabra);
		recv(socketServidor, palabraAdivinar, tamPalabra, 0);
		printf("\nFIN DEL JUEGO: PERDISTE!!\t");
		printf("La palabra era: %s\n", palabraAdivinar);
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
	printf("Juego del ahorcado (Hangman) \n");
	printf("Se genera una comunicación cliente-servidor mediante socket.\n");
	printf("El servidor debe recibir como parámetro el puerto que escuchara y el\n");
	printf("cliente recibirá la IP y puerto del servidor a donde deberá conectarse.\n");
	printf("Se debe ejecutar primero el servidor y a continuacion el cliente podra\n");
	printf("realizar la solicitud.\n");
	printf("Para ejecutar el servidor ingrese: ./servidor PUERTO\n");
	printf("Para ejecutar el cliente ingrese: ./cliente PUERTO DIRECCIONIP\n");
}