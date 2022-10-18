#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <string.h>
#include <unistd.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * EVENT_SIZE)

int watcher();
int validarParametros(int, char *);
int mostrarAyuda(int, char *);

int main(int argc, char **argv)
{
  if (mostrarAyuda(argc, argv[1]))
    exit(1);
  if (argc != 1)
  {
    if (!validarParametros(argc, argv[1]))
    {
      printf("Error de parametros\n");
      printf("Para mostrar la ayuda : ejercicio2 [options]\n");
      printf("Options:\n");
      printf("-h\n");
      printf("--help\n");
      exit(1);
    }
  }
  watcher();

  return 0;
}

// FUNCTIONS
int watcher()
{
  char buffer[BUF_LEN];

  int file_descriptor = inotify_init();
  if (file_descriptor < 0)
    perror("inotify_init");

  int watch_descriptor = inotify_add_watch(file_descriptor, "/Home/strike", IN_MODIFY | IN_CREATE | IN_DELETE);

  int length = read(file_descriptor, buffer, BUF_LEN);
  if (length < 0)
    perror("read");

  int offset = 0;

  while (offset < length)
  {
    struct inotify_event *event = (struct inotify_event *)&buffer[offset];

    if (event->len)
    {

      if (event->mask & IN_CREATE)
      {
        (event->mask & IN_ISDIR) ? printf("La carpeta %s fue creada.\n", event->name):
                                   printf("El archivo %s fue creado.\n", event->name);
      }
      else if (event->mask & IN_DELETE)
      {
        (event->mask & IN_ISDIR) ? printf("La carpeta %s fue eliminada.\n", event->name):
                                   printf("El archivo %s fue eliminado.\n", event->name);
      }
      else if (event->mask & IN_MODIFY)
      {
        (event->mask & IN_ISDIR) ? printf("La carpeta %s fue modificada.\n", event->name):
                                   printf("El archivo %s fue modificado.\n", event->name);
      }
    }
    offset += sizeof(struct inotify_event) + event->len;
  }

  inotify_rm_watch(file_descriptor, watch_descriptor);
  close(file_descriptor);

  return EXIT_SUCCESS;
}
int mostrarAyuda(int cantPar, char *cad)
{

  if (cantPar == 2 && (!strcmp(cad, "-h") || !strcmp(cad, "--help")))
  {
    printf("HELP\n");
    printf("Ejercicio2.exe - Monitorea una carpetas y subfolders\n");
    printf("SYNOPSIS:\n");
    printf("    ejercicio2 [folder]  monitorear \n");
    return 1;
  }
  return 0;
}

int validarParametros(int cantParam, char *cad)
{
  return (cantParam != 2 && (!strcmp(cad, "-h") || !strcmp(cad, "--help")));
}