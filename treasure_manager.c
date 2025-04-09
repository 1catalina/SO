#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <errno.h>
#include <string.h>


typedef struct gps
{
  float lat;
  float longt;
}gps;

typedef struct treasure{
  char id[5];
  char user[20];
  gps coord;
  char clue[10];
  int value;
}treasure;

void build_path(char *dest, char *dir)
{
  strcpy(dest, dir);
  strcat(dest, "/treasures.dat");
}
void build_logged_path(char *dest, char *dir)
{
  build_path(dest, dir);
  strcat(dest, "/logged_hunt");
}
void create_sim(char *old, char *dest, char *dir)
{
  symlink(old, build_logged_path(dest, dir));
}
void add_treasure(int argc, char **argv)
{
  if(argc != 9)
    {
      printf("argumente insuficiente\n");
      exit(-1);
    }
  char *id=argv[2];
  treasure t;
  strcpy(t.id, argv[3]);
  strcpy(t.user, argv[4]);
  t.coord.lat=atof(argv[5]);
  t.coord.longt=atof(argv[6]);
  strcpy(t.clue, argv[7]);
  t.value=atoi(argv[8]);

  mkdir(id, 0755);

  char path[512];
  build_path(path, id);

  char log_path[512];
  build_logged_path(log_path, id);

  int f;
  if((f=open(path, O_WRONLY | O_CREAT | O_APPEND, 0644))==-1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }
  write(f, &t, sizeof(treasure));
  close(f);
  int f;
  if((f=open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644))==-1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }
  write(f, "add\n", 4*sizeof(char));
  close(f);
   
}

void list_treasures(char *hunt_id)
{
  char path[512];
  build_path(path, hunt_id);
  int f;
  if((f=open(path, O_RDONLY)) ==-1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }
    struct stat st;
    stat(path, &st);

    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", st.st_size);
    printf("Last modified: %ld\n", st.st_mtime);
    printf("---------------------------\n");

    treasure t;
    while((read(f, &t, sizeof(treasure))) == sizeof(treasure))
      {
	printf("id: %s - username: %s - gps: %.4f latitude, %.4f longitude\n", t.id, t.user, t.coord.lat, t.coord.longt);
	printf("clue: %s - value: %d\n", t.clue, t.value);
	printf("---------------------------\n");
      }
    close(f);
}
void view(char *hunt, char *id)
{
  char path[512];
  build_path(path, hunt);
  int f;
  if((f=open(path, O_RDONLY)) == -1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }
  struct stat st;
  stat(path, &st);

  treasure t;
  while(((read(f, &t, sizeof(treasure))) == sizeof(treasure)) && (strcmp(t.id, id))==1)
  printf("id: %s - username: %s - gps: %.4f latitude, %.4f longitude\n", t.id, t.user, t.coord.lat, t.coord.longt);
	printf("clue: %s - value: %d\n", t.clue, t.value);
	printf("---------------------------\n");
  
}
int main(int argc, char **argv)
{
  if(argc < 2)
    {
      printf("argumente insuficiente\n");
      exit(-1);
    }
  if(strcmp(argv[1], "add")==0)
    {
      add_treasure(argc, argv);
    }
  else
    if(strcmp(argv[1], "list")==0)
      {
	list_treasures(argv[2]);
      }
    else
      if(strcmp(argv[1], "view")==0)
	{
	  view(argv[2], argv[3]);
	}
    else
      {
	printf("comanda invalida\n");
      }
  
  return 0;
}
