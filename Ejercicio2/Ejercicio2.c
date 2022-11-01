#define _GNU_SOURCE

#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <errno.h>

static FILE *logfp = NULL;

static unsigned int num_adder_threads = 1;
static unsigned int num_remover_threads = 1;
static unsigned int num_inotify_instances = 1;
static char *working_dir = "";
static char * file_name = "log.txt";

static pthread_attr_t attr;
static pthread_mutex_t wait_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wait_var = PTHREAD_COND_INITIALIZER;
static int wait;
#define WAIT_CHILD do {\
		pthread_mutex_lock(&wait_mutex); \
		if (wait == 0) \
			pthread_cond_wait(&wait_var, &wait_mutex); \
		wait = 0; \
		pthread_mutex_unlock(&wait_mutex); \
	} while (0);
#define WAKE_PARENT do {\
		pthread_mutex_lock(&wait_mutex); \
		wait = 1; \
		pthread_cond_signal(&wait_var); \
		pthread_mutex_unlock(&wait_mutex); \
	} while (0);

struct adder_struct {
	int inotify_fd;
	int file_num;
};

struct operator_struct {
	int inotify_fd;
};

struct thread_data {
	int inotify_fd;
	pthread_t *adders;
	pthread_t *removers;
};

static int stopped = 0;

static void sigfunc(int sig_num)
{
	if (sig_num == SIGINT)
		stopped = 1;
	else
		printf("Ni puta idea que señal es!\n");
}

static void *__add_watches(void *ptr)
{
	struct adder_struct *adder_arg = ptr;
	int file_num = adder_arg->file_num;
	int notify_fd = adder_arg->inotify_fd;
	int ret;
	char filename[50];

	fprintf(logfp, "Creando un hilo watcher, notify_fd=%d filenum=%d\n",
		notify_fd, file_num);

	snprintf(filename, 50, "%s/%d", working_dir, file_num);

	WAKE_PARENT;

	while (!stopped) {
		ret = inotify_add_watch(notify_fd, filename, IN_MODIFY | IN_CREATE | IN_DELETE);
		if (ret < 0 && errno != ENOENT)
			perror("inotify_add_watch");
		sched_yield(); 
	}

	return NULL;
}

static int start_watch_creation_threads(struct thread_data *td)
{
	struct adder_struct ws;
	unsigned int i, j;
	int rc;
	pthread_t *adders;

	ws.inotify_fd = td->inotify_fd;

	adders = calloc(num_adder_threads, sizeof(*adders));
	if (!adders){
        perror("Error al reservar memoria");
	    exit(EXIT_FAILURE);
    }
	td->adders = adders;

	for (i = 0; i < num_adder_threads; i++) {
		ws.file_num = i;
		for (j = 0; j < 1; j++) {
			rc = pthread_create(&adders[i + j], &attr, __add_watches, &ws);
			if (rc)
			{
                perror("Error");
	            exit(EXIT_FAILURE);
            }
			WAIT_CHILD;
		}
	}

	return 0;
}

static void *__remove_watches(void *ptr)
{
	struct operator_struct *operator_arg = ptr;
	int inotify_fd = operator_arg->inotify_fd;
	int i;

	fprintf(logfp, "Crando un hilo para remover los watches\n");

	WAKE_PARENT;

	while (!stopped) {
		for (i = 0; i < INT_MAX; i++)
			inotify_rm_watch(inotify_fd, i);
		sched_yield(); 
	}
	return NULL;
}

static int start_watch_removal_threads(struct thread_data *td)
{
	struct operator_struct os;
	int rc;
	unsigned int i, j;
	pthread_t *removers;

	os.inotify_fd = td->inotify_fd;

	removers = calloc(num_remover_threads, sizeof(*removers));
	if (!removers)
    {
        perror("Error al reservar memoria");
	    exit(EXIT_FAILURE);
    }
	td->removers = removers;

	for (i = 0; i < num_remover_threads; i++) {
		for (j = 0; j < 1; j++) {
			rc = pthread_create(&removers[i + j], &attr, __remove_watches, &os);
			if (rc)
            {
                	perror("Creando el hilo para remover el watcher");
	                exit(EXIT_FAILURE);
            }
			WAIT_CHILD;
		}
	}
	return 0;
}

static int join_threads(struct thread_data *td)
{
	unsigned int i, j;
	void *ret;
	pthread_t *to_join;

	to_join = td->adders;
    pthread_join(to_join[0], &ret);

	to_join = td->removers;
    pthread_join(to_join[0], &ret);

	return 0;
}

int mostrar_ayuda(int cantPar, char *cad)
{

    if (cantPar == 2 && (!strcmp(cad, "-h") || !strcmp(cad, "--help")))
    {
        printf("HELP\n");
        printf("Ejercicio2.exe - Monitorea una carpetas y subcarpetas\n");
        printf("SYNOPSIS:\n");
        printf("    ejercicio2 [folder]  monitorear \n");
        printf("Ejemplo\n");
        printf("    ejercicio2 yyy \n");
        return 1;
    }
    return 0;
}

static int validar_args(int argc, char *argv[])
{
    if (mostrar_ayuda(argc, argv[1]))
        exit(1);    
    working_dir = optarg;
	return 0;
}

int main(int argc, char *argv[])
{
	struct thread_data *td;
	int rc;
	unsigned int i;
	struct sigaction setmask;

	rc = validar_args(argc, argv);
	if (rc)
		printf("error procesando argumentos");

    logfp = fopen(file_name, "w+");
    if (logfp == NULL)
        printf("fopen error al abrir");
    setbuf(logfp, NULL);//en null

    printf("open file\n");

    /* Interrupcion con (Ctrl-C) */
    
	sigemptyset( &setmask.sa_mask );
	setmask.sa_handler = sigfunc;
	setmask.sa_flags   = 0;
	sigaction( SIGINT,  &setmask, (struct sigaction *) NULL );  

	pthread_attr_init(&attr);
	pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN*2);
	td = calloc(num_inotify_instances, sizeof(*td));
    if (!td)
    {
        perror("Error en calloc inotify td vector");
	    exit(EXIT_FAILURE);
    }

	/*Creando inotify instancias*/
	for (i = 0; i < num_inotify_instances; i++) {
		struct thread_data *t;
		int fd;
		fd = inotify_init1(O_NONBLOCK);

		t = &td[i];
		t->inotify_fd = fd;

		start_watch_creation_threads(t);
		start_watch_removal_threads(t);
	}

	for (i = 0; i < num_inotify_instances; i++)
		join_threads(&td[i]);

	for (i = 0; i < num_inotify_instances; i++) {
		free(td[i].adders);
		free(td[i].removers);
	}
	free(td);
	exit(EXIT_SUCCESS);
}