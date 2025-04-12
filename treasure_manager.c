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
  strcpy(dest, dir);
  strcat(dest, "/logged_hunt.txt");
  }
void create_simlink(char *hunt_id)
{
   char old[256], new[256];
  strcpy(new, "logged_hunt-");
  strcat(new, hunt_id);
  build_logged_path(old, hunt_id);
  symlink(old, new);
}
void log_operation(char *hunt_id, char *op)
{
  char path[512];
  build_logged_path(path, hunt_id);
  int f;
  if((f=open(path, O_WRONLY | O_CREAT | O_APPEND, 0644))==-1)
    {
      printf("eroare logare operatie: %s", op);
      exit(-1);
    }
    char out[128];
    strcpy(out, "operatie: ");
    strcat(out, op);
    strcat(out, "\n");
    write(f, out, strlen(out));
    close(f);
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
  int f;
  if((f=open(path, O_WRONLY | O_CREAT | O_APPEND, 0644))==-1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }
  write(f, &t, sizeof(treasure));
  close(f);

  log_operation(id, "add");
  create_simlink(id);
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
    
    ///-------------------------------------
    
    treasure t;
    while((read(f, &t, sizeof(treasure))) == sizeof(treasure))
      {
	printf("id: %s - username: %s - gps: %.4f latitude, %.4f longitude\n", t.id, t.user, t.coord.lat, t.coord.longt);
	printf("clue: %s - value: %d\n", t.clue, t.value);
	printf("---------------------------\n");
      }
    close(f);
   
    log_operation(hunt_id, "list");
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
  while((read(f, &t, sizeof(treasure))) == sizeof(treasure))
    if(strcmp(t.id, id)==0)
      {
	printf("id: %s - username: %s - gps: %.4f latitude, %.4f longitude\n", t.id, t.user, t.coord.lat, t.coord.longt);
	printf("clue: %s - value: %d\n", t.clue, t.value);
	printf("---------------------------\n");
	break;
      }
  close(f);

  log_operation(hunt, "view");
 
}

void remove_treasure(char *hunt_id, char *id)
{
  char path[512];
  build_path(path, hunt_id);

  int f=open(path, O_RDONLY);
  if(f==-1)
    {
      printf("eroare deschidere fisier\n");
      exit(-1);
    }

  treasure *buffer = malloc(1024 *sizeof(treasure));
  int cnt=0;
  treasure t;

  while(read(f, &t, sizeof(treasure))==sizeof(treasure))
    {
      if(strcmp(t.id, id)!=0)
	{
	  buffer[cnt++]=t;
	}
    }
  close(f);

  if((f=open(path, O_WRONLY | O_TRUNC))==-1)
    {
       printf("eroare deschidere fisier\n");
       exit(-1);
    }
  
  write(f, buffer, cnt*sizeof(treasure));
    close(f);
    free(buffer);

    log_operation(hunt_id, "remove_treasure");
}
 void remove_hunt(char *hunt_id)
 {
  char path[512];
    build_path(path, hunt_id);
    unlink(path); 

    build_logged_path(path, hunt_id);
    unlink(path); 

    rmdir(hunt_id); 

    char symlink_name[256];
    sprintf(symlink_name, "logged_hunt-%s", hunt_id);
    unlink(symlink_name);  
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
	 if (argc != 3)
	   {
	     	printf("argumente insuficiente\n");
		exit(-1);
	    }
	list_treasures(argv[2]);
      }
    else
      if(strcmp(argv[1], "view")==0)
	{
	  if(argc != 4)
	    {
	      	printf("argumente insuficiente\n");
		exit(-1);
	    }
	  view(argv[2], argv[3]);
	}
    else
      if (strcmp(argv[1], "remove_treasure") == 0)
	{
	   if(argc!=4)
	      {
		printf("argumente insuficiente\n");
		exit(-1);
	      }
	  remove_treasure(argv[2], argv[3]);
	}
      else
	if (strcmp(argv[1], "remove_hunt") == 0)
	  {
	    if(argc!=3)
	      {
		printf("argumente insuficiente\n");
		exit(-1);
	      }
	    remove_hunt(argv[2]);
           }
       else
        {
          printf("comanda invalida\n");
        }
  
  return 0;
}
