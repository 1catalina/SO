#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <dirent.h>

pid_t monitor_pid = -1;
int monitor_running = 0;
int monitor_stopping = 0;

void handle_sigchld(int sig)
{
    int status;
    waitpid(monitor_pid, &status, 0);
    printf("Monitorul s-a incheiat cu statusul: %d\n", WEXITSTATUS(status));
    monitor_running = 0;
    monitor_pid = -1;
    monitor_stopping = 0;
}

void send_cmnd(char *cmnd_line, int signal)
{
    int fd = open("command.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1)
    {
        printf("Eroare deschidere fisier command.txt\n");
        return;
    }

    if (write(fd, cmnd_line, strlen(cmnd_line)) == -1)
    {
        printf("Eroare la scrierea in fisier\n");
        close(fd);
        return;
    }

    if (write(fd, "\n", 1) == -1)
    {
        printf("Eroare la scriere newline in fisier\n");
        close(fd);
        return;
    }

    close(fd);

    if (kill(monitor_pid, signal) == -1)
    {
        printf("Eroare la trimiterea semnalului catre monitor\n");
    }
}

void start_monitor(void)
{
    if (monitor_running)
    {
        printf("Monitorul este deja pornit\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid < 0)
    {
        printf("Eroare fork\n");
        exit(-1);
    }

    if (monitor_pid == 0)
    {
        execl("./treasure_manager", "treasure_manager", NULL);
        perror("Eroare execl\n");
        exit(-1);
    }

    monitor_running = 1;
    monitor_stopping = 0;   ///
    printf("Monitor pornit (PID: %d)\n", monitor_pid);
}

void stop_monitor(void)
{
    if (!monitor_running)
    {
        printf("Monitorul nu ruleaza\n");
        return;
    }

    send_cmnd("stop_monitor", SIGUSR2);
    monitor_stopping = 1;
    printf("Monitor oprit\n");
}
void calculate_score()
{
    DIR *d = opendir(".");
    if (!d)
    {
        perror("Eroare deschidere director curent\n");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(d)) != NULL)
    {
        if (entry->d_type == DT_DIR &&
            strcmp(entry->d_name, ".") != 0 &&
            strcmp(entry->d_name, "..") != 0)
        {
            char path[512];
            snprintf(path, sizeof(path), "%s/treasures.dat", entry->d_name);

            int f = open(path, O_RDONLY);
            if (f < 0)
                continue;
            close(f);

            int pipefd[2];
            if (pipe(pipefd) == -1)
            {
                perror("Eroare la crearea pipe-ului\n");
                continue;
            }

            pid_t pid = fork();
            if (pid == -1)
            {
                perror("Eroare fork\n");
                close(pipefd[0]);
                close(pipefd[1]);
                continue;
            }

            if (pid == 0)
            {
                close(pipefd[0]);
                dup2(pipefd[1], STDOUT_FILENO); 
                close(pipefd[1]);

                execlp("./score_calculator", "score_calculator", entry->d_name, NULL);
                perror("Eroare la exec\n");
                exit(EXIT_FAILURE);
            }
            else
            {
                close(pipefd[1]); 

                char buffer[1024];
                ssize_t bytes_read;
                printf("Scoruri pentru hunt-ul %s:\n", entry->d_name);
                while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
                {
                    buffer[bytes_read] = '\0';
                    printf("%s", buffer);
                }
                close(pipefd[0]);
                waitpid(pid, NULL, 0);
            }
        }
    }
    closedir(d);
}
int main()
{
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGCHLD, &sa, NULL);

    char command[256];

    while (1)
    {
        printf(">>> ");
        fflush(stdout);
        if (!fgets(command, sizeof(command), stdin))
	  {
	    break;
	  }
        command[strcspn(command, "\n")] = '\0';

        if (monitor_stopping)   ///
        {
            printf("Monitorul se opreste. Nu mai poti trimite comenzi\n");
            continue;
        }

        if (strcmp(command, "exit") == 0)
        {
            if (monitor_running)
            {
                printf("Monitorul inca ruleaza. Foloseste stop_monitor inainte de exit\n");
            }
            else
            {
                break;
            }
        }
        else if (strcmp(command, "start_monitor") == 0)
        {
            start_monitor();
        }
        else if (strcmp(command, "stop_monitor") == 0)
        {
            stop_monitor();
        }
        else if (strncmp(command, "list_hunts", 10) == 0)
        {
            send_cmnd(command, SIGUSR1);
        }
        else if (strncmp(command, "list_treasures", 14) == 0)
        {
            send_cmnd(command, SIGUSR1);
        }
        else if (strncmp(command, "view_treasure", 13) == 0)
        {
            send_cmnd(command, SIGUSR1);
        }
	else if (strcmp(command, "calculate_score") == 0)
        {
	    calculate_score();
	}
        else
        {
            printf("Comanda invalida\n");
        }
    }

    return 0;
}
