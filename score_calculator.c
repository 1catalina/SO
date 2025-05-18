#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct gps
{
    float lat;
    float longt;
} gps;

typedef struct treasure
{
    char id[5];
    char user[20];
    gps coord;
    char clue[10];
    int value;
} treasure;

typedef struct user_score
{
    char user[20];
    int score;
    struct user_score *next;
} user_score;

void add_score(user_score **head, const char *user, int value)
{
    user_score *current = *head;
    while (current)
    {
        if (strcmp(current->user, user) == 0)
        {
            current->score += value;
            return;
        }
        current = current->next;
    }
    // Adaugă un nou utilizator
    user_score *new_user = malloc(sizeof(user_score));
    strcpy(new_user->user, user);
    new_user->score = value;
    new_user->next = *head;
    *head = new_user;
}

void free_scores(user_score *head)
{
    user_score *current = head;
    while (current)
    {
        user_score *tmp = current;
        current = current->next;
        free(tmp);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Utilizare: %s <hunt_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char path[512];
    snprintf(path, sizeof(path), "%s/treasures.dat", argv[1]);

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        perror("Eroare la deschiderea fișierului treasures.dat");
        return EXIT_FAILURE;
    }

    treasure t;
    user_score *scores = NULL;

    while (read(fd, &t, sizeof(treasure)) == sizeof(treasure))
    {
        add_score(&scores, t.user, t.value);
    }
    close(fd);

    user_score *current = scores;
    while (current)
    {
        printf("Utilizator: %s - Scor: %d\n", current->user, current->score);
        current = current->next;
    }

    free_scores(scores);
    return EXIT_SUCCESS;
}

