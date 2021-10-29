#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <signal.h>


#define NB_MAX_MOTS 30
#define NB_MAX_CAR 4096

#define UNUSED(x) (void)(x)


static void
lit_et_analyse_ligne (char *ligne_analysee, char *commande[])
{

    *ligne_analysee = '\0';


    while (fgets (ligne_analysee, NB_MAX_MOTS, stdin) == NULL)
    {
        if (feof (stdin))
        {
            printf ("\n");
            exit (0);
        }
    }

    char *debut = ligne_analysee;

    while (*debut && isspace (*debut))
        debut++;

    int i = 0;
    while (*debut && i < NB_MAX_MOTS - 1)	// on laisse la place pour le NULL final
    {
        commande[i] = debut;

        while (*debut && !isspace (*debut))
            debut++;

        if (*debut && isspace (*debut))
        {
            *debut = '\0';
            debut++;
            while (*debut && isspace (*debut))
                debut++;
        }

        ++i;
    }

    commande[i] = NULL;
}

static void
affiche_invite (void)
{
    char *prompt, *ptr, *ptr2 = NULL;

    prompt = get_current_dir_name ();
    ptr2 = strrchr (prompt, '/');

    ptr = ptr2;
    ptr++;

    putchar('\n');
    printf ("%s>", ptr);
    fflush (stdout);

    free(prompt);
}

static void
traite_signal (int signal_recu)
{

    UNUSED(signal_recu);
    affiche_invite();
}


static void
initialiser_gestion_signaux (struct sigaction *sig)
{
    sig->sa_flags = SA_NOCLDSTOP;
    sigemptyset (&sig->sa_mask);


    sig->sa_handler = traite_signal;
    sigaction(SIGINT, sig, NULL);
}


static void
execute_commande (char *commande[], struct sigaction *sig)
{

    if (commande[0] == NULL)
    {
        ;
    }

    else if (!strcmp (commande[0], "cd"))
    {
        int argc = 0;

        while (commande[argc])
        	argc++;

       if (argc <= 2)
       {
            if (argc == 1)
            {
                if (chdir (getenv ("HOME")) == -1)
                {
                    perror ("mini-shell: cd");
                }
            }
            else if (argc == 2)
            {
                if ((strcmp (commande[1], "~") == 0) && getenv ("HOME") != NULL)
                {
                    strcpy (commande[1], getenv ("HOME"));
                }
                if (chdir (commande[1]) == -1)
                {
                    perror ("mini-shell: cd");
                    if (errno == ENOENT)
                    {
                        fprintf (stderr,
                                 "Conseil : vÃ©rifiez que tous les rÃ©pertoires du chemin existent\n");
                    }
                    else if (errno == EACCES)
                    {
                        fprintf (stderr,
                                 "Conseil : vÃ©rifiez que vous pouvez accÃ©der Ã  tous les rÃ©pertoires du chemin\n");
                    }
                }

            }
        }
        else
        {
            fprintf (stderr, "Erreur: trop d'arguments\n");
        }
    }
    else
    {
	    pid_t res_f;
	    int statut;

            sig->sa_handler = SIG_IGN;
	    sigaction(SIGINT, sig, NULL);

            res_f = fork ();

        if (res_f == -1)
        {
            perror ("mini-shell: fork");
            exit (errno);
        }

        if (res_f == 0)
        {
            sig->sa_handler = SIG_DFL;
	    sigaction(SIGINT, sig, NULL);

            if (execvp (commande[0], commande) == -1)
            {
                perror ("mini-shell: execvp");
                exit (errno);
            }
        }
        else
        {
            if (waitpid (res_f, &statut, 0) == -1)
            {
                perror ("mini-shell: waitpid");
                exit (errno);
            }

            sig->sa_handler = traite_signal;
	    sigaction(SIGINT, sig, NULL);
        }

    }
}


int
main (void)
{
    char ligne[NB_MAX_CAR];
    char *commande[NB_MAX_MOTS + 1];
    struct sigaction m_sig;


    initialiser_gestion_signaux (&m_sig);

    while (true)
    {
        affiche_invite ();
        lit_et_analyse_ligne (ligne, commande);
        execute_commande (commande, &m_sig);
    }

    return EXIT_SUCCESS;
}
