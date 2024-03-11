#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <ctype.h>

//---------------------- Cacher mot de passe par '*' ---------------------
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>

char getch()
{
    struct termios oldt, newt;
    char ch;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    return ch;
}
#endif

#define LONGUEUR_MAX_LOGIN 10
#define LONGUEUR_MAX_MDP 10
#define MAX_STUDENTS_PER_CLASS 50

// Codes d'échappement ANSI pour les couleurs du texte
#define RESET "\x1B[0m"
#define RED "\x1B[31m"
#define GREEN "\x1B[32m"
#define YELLOW "\x1B[33m"
#define BLUE "\x1B[34m"
#define MAGENTA "\x1B[35m"
#define CYAN "\x1B[36m"
#define WHITE "\x1B[37m"

typedef struct
{
    int jour;
    int mois;
    int annee;
} Date;

typedef struct
{
    char login[LONGUEUR_MAX_LOGIN];
    char motDePasse[LONGUEUR_MAX_MDP];
} Identifiants;

typedef struct
{
    char matricule[10];
    char motdepasse[10];
    char prenom[20];
    char nom[20];
    char classe[6];
    Date dateNaiss;
    int etat;
} Apprenant;

// Définition de la structure pour un étudiant
struct Etudiant
{
    char matricule[20];
    char prenom[50];
    char nom[50];
    char nom_classe[20];
};

// Définition de la structure pour un message
struct Message
{
    char expediteur[50];
    char date[20];
    char contenu[500];
};

Identifiants identifiantsAdmin;
int nombreIdentifiantsAdmin = 1;

// recuperer les infos d'un etudiant par matricule
int obtenirInfosEtudiantParMatricule(char *prenom, char *nom, char *matricule)
{
    FILE *fichier = fopen("file-etudiant.txt", "r");

    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
        return 0; // Retourner 0 pour indiquer une erreur
    }

    int trouve = 0;
    Apprenant etudiant;

    while (fscanf(fichier, " %s %s %s", etudiant.prenom, etudiant.nom, etudiant.matricule) != EOF)
    {

        if (strcmp(etudiant.matricule, matricule) == 0)
        {

            // Étudiant trouvé, copier le prénom et le nom
            strcpy(prenom, etudiant.prenom);
            strcpy(nom, etudiant.nom);
            trouve = 1;
            break;
        }
    }

    fclose(fichier);

    if (trouve == 0)
    {
        printf("Étudiant de matricule %s non trouvé.\n", matricule);

        return 0; // Retourner 0 pour indiquer que l'étudiant n'a pas été trouvé
    }

    return 1; // Retourner 1 pour indiquer le succès
}

// enregistrer la presence
void enregistrerPresence(char *prenom, char *nom, char *matricule)
{
    FILE *fichier = fopen("file-presence.txt", "a");
    if (fichier == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    // Récupérer la date et l'heure  actuelle
    time_t now = time(NULL);
    struct tm *timeinfo = localtime(&now);
    // Écrire dans le fichier la date et l'heure
    fprintf(fichier, "%s %s %s %d/%d/%d \n", prenom, nom, matricule, timeinfo->tm_mday, timeinfo->tm_mon + 1,
            timeinfo->tm_year + 1900);
    // %dh%dmn%ds , timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec

    fclose(fichier);
}

// marquer presence
void marquerPresence()
{
    char choix[10];
    printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
    scanf("%s", choix);
    while (strcmp(choix, "Q") != 0 && strcmp(choix, "q") != 0)
    {
        FILE *fichierPresence = fopen("file-presence.txt", "r");
        if (fichierPresence == NULL)
        {
            printf("Erreur lors de l'ouverture du fichier de présence.\n");
            return;
        }

        int present = 0;
        char matricule[10];
        char date[20];
        char prenom[20];
        char nom[20];
        while (fscanf(fichierPresence, "%s %s", matricule, date) != EOF)
        {
            if (strcmp(matricule, choix) == 0)
            {
                printf(RED "\n--- L'étudiant de matricule %s est déjà marqué présent aujourd'hui.\n" RESET, choix);
                present = 1;
            }
        }
        fclose(fichierPresence);

        if (!present)
        {
            // Vérifier la date du dernier marquage
            FILE *fichierDernierMarquage = fopen("dernier-marquage.txt", "r");
            if (fichierDernierMarquage == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier du dernier marquage.\n");
                return;
            }

            char dernierMatricule[10];
            char dernierDate[20];
            while (fscanf(fichierDernierMarquage, "%s %s", dernierMatricule, dernierDate) != EOF)
            {
                if (strcmp(dernierMatricule, choix) == 0)
                {
                    // Vérifier si la date est la même que la date actuelle
                    time_t now = time(NULL);
                    struct tm *timeinfo = localtime(&now);
                    char dateActuelle[20];
                    sprintf(dateActuelle, "%01d/%01d/%04d", timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    if (strcmp(dernierDate, dateActuelle) == 0)
                    {
                        printf(RED "\n--- Vous avez déjà marqué votre présence aujourd'hui.\n" RESET);
                        fclose(fichierDernierMarquage);
                        break;
                    }
                }
            }

            fclose(fichierDernierMarquage);

            FILE *fichier = fopen("file-etudiant.txt", "r+");
            if (fichier == NULL)
            {
                printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                return;
            }

            while (fscanf(fichier, "%s", matricule) != EOF)
            {
                if (strcmp(matricule, choix) == 0)
                {
                    obtenirInfosEtudiantParMatricule(prenom, nom, matricule);
                    // Enregistrer la présence dans le fichier
                    // Etudiant e = rechercheEtudiant(matr)
                    enregistrerPresence(prenom, nom, matricule);

                    // Mettre à jour la date du dernier marquage
                    FILE *fichierDernierMarquage = fopen("dernier-marquage.txt", "a");
                    if (fichierDernierMarquage == NULL)
                    {
                        printf("Erreur lors de l'ouverture du fichier du dernier marquage.\n");
                        fclose(fichier);
                        return;
                    }

                    time_t now = time(NULL);
                    struct tm *timeinfo = localtime(&now);
                    fprintf(fichierDernierMarquage, "%s %01d/%01d/%04d\n", choix, timeinfo->tm_mday, timeinfo->tm_mon + 1, timeinfo->tm_year + 1900);

                    fclose(fichierDernierMarquage);

                    printf(GREEN "\n ✅ Presence marquee pour l'etudiant de matricule %s\n" RESET, choix);
                    present = 1;
                    break;
                }
            }
            fclose(fichier);
        }

        if (!present)
        {
            printf(RED "---  Matricule invalide. Veuillez reessayer ('Q' pour quitter) : " RESET);
        }
        else
        {
            printf("\n--- Entrez le matricule de l'etudiant à marquer present ('Q' pour quitter) : ");
        }

        scanf("%s", choix);
    }
}

// Fonctions du menu admin

int menuAdmin()
{
    int choix = 0;
    do
    {
        printf("------------------------------------------------------------\n");
        printf("\t\t\tBienvenue dans le menu de l'administrateur:\n");
        printf("------------------------------------------------------------\n");
        printf("1 - Gestion des étudiants\n");
        printf("2 - Génération de fichiers\n");
        printf("3 - Marquer les présences\n");
        printf("4 - Envoyer un message\n");
        printf("5 - Paramètres\n");
        printf("6 - Deconnexion\n");
        printf("\n--- Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 6)
        {
            printf(RED "Choix invalide. Veuillez entrer un choix entre 1 et 2.\n" RESET);
        }
    } while (choix != 6);
    return choix;
}

// fonctions du menu etudiant
int menuEtudiant()
{
    // Définition du menu de l'étudiant
    int choix = 0;
    do
    {
        printf("----------------------------------------------------------\n");
        printf("\t\t\tBienvenue dans le menu de l'apprenant :\n");
        printf("----------------------------------------------------------\n");
        printf("1 - GESTION DES ÉTUDIANTS\n");
        printf("2 - GÉNÉRATION DE FICHIERS\n");
        printf("3 - MARQUER SA PRÉSENCE\n");
        printf("4 - Message (0)\n");
        printf("5 - Déconnexion\n");
        printf("\n--- Entrez votre choix : ");
        scanf("%d", &choix);
        if (choix < 1 || choix > 5)
        {
            printf(RED "Choix invalide. Veuillez entrer un choix entre  1 et 5.\n" RESET);
        }
    } while (choix < 1 || choix > 5);
    return choix;
}

// Fonction pour vérifier les identifiants de connexion
int verifierIdentifiants(Identifiants *identifiants, int nombreIdentifiants, char *login, char *motDePasse)
{
    for (int i = 0; i < nombreIdentifiants; i++)
    {
        if (strcmp(identifiants[i].login, login) == 0 && strcmp(identifiants[i].motDePasse, motDePasse) == 0)
        {
            return 1; // Identifiants valides
        }
    }
    return 0; // Identifiants invalides
}

// fonction pour vider le buffer
void viderBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ; // Lire et ignorer les caractères jusqu'à la fin de la ligne ou la fin du fichier
}

//------------------------------------2 GENERATION DE FICHIERS--------------------------------------------------------


void genererFichierToutesPresences()
{
    FILE *fichierPresence = fopen("file-presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    FILE *fichierSortie = fopen("fichier-toutes-presences.txt", "w, ccs=UTF-8");
    if (fichierSortie == NULL)
    {
        printf("Erreur lors de la création du fichier de sortie.\n");
        fclose(fichierPresence);
        return;
    }

    // Obtention de la date actuelle
    time_t t = time(NULL);
    struct tm tm_info = *localtime(&t);
    char dateGeneration[20];
    strftime(dateGeneration, sizeof(dateGeneration), "%d/%m/%Y", &tm_info);

    fprintf(fichierSortie, "FICHIER DE TOUTES LES PRÉSENCES\n");
    fprintf(fichierSortie, "Date de génération : %s\n\n", dateGeneration);

    char date[20];
    char matricule[10];
    char nom[20];
    char prenom[20];

    fprintf(fichierSortie, "+-------------------+------------------+------------+\n");
    fprintf(fichierSortie, "| Prénom            | Nom              | Matricule  |\n");
    fprintf(fichierSortie, "+-------------------+------------------+------------+\n");

    while (fscanf(fichierPresence, "%s %s %s %s", prenom, nom, matricule, date) != EOF)
    {
        fprintf(fichierSortie, "| %-17s | %-16s | %-10s |\n", prenom, nom, matricule);
        fprintf(fichierSortie, "+-------------------+------------------+------------+\n");
    }

    fclose(fichierPresence);
    fclose(fichierSortie);

    printf("Fichier généré avec succès : fichier-toutes-presences.txt\n");
}





     void genererFichierParDate()
 {
     char dateRecherche[20];
    int jour, mois, annee;
    char garbage; // Pour stocker les caractères indésirables

     printf("Veuillez entrer la date au format JJ/MM/AAAA : ");

     // Vérification de la saisie de la date
     if (scanf("%d/%d/%d%c", &jour, &mois, &annee, &garbage) != 4 || garbage != '\n')
     {
         printf(RED "Saisie invalide. Veuillez entrer une date au format JJ/MM/AAAA.\n" RESET);
         // Nettoyer le buffer d'entrée
         while (getchar() != '\n');
         return;
     }

     // Validation de la date
     if (jour < 1 || jour > 31 || mois < 1 || mois > 12 || annee < 1)
     {
         printf(RED "Date invalide. Veuillez entrer une date au format valide.\n" RESET);
         return;
     }

     sprintf(dateRecherche, "%01d/%01d/%04d", jour, mois, annee);

     FILE *fichierPresence = fopen("file-presence.txt", "r");
     if (fichierPresence == NULL)
     {
         printf("Erreur lors de l'ouverture du fichier de présence.\n");
         return;
     }

     char nomFichierSortie[50];
     sprintf(nomFichierSortie, "resultat-%01d-%01d-%04d.txt", jour, mois, annee);
     FILE *fichierSortie = fopen(nomFichierSortie, "w,  ccs=UTF-8");
     if (fichierSortie == NULL)
     {
         printf("Erreur lors de la création du fichier de sortie.\n");
         fclose(fichierPresence);
         return;
     }

     int presenceTrouvee = 0;
     char date[20];
     char heure[10];
     char matricule[10];
     char nom[20];
     char prenom[20];

     fprintf(fichierSortie, MAGENTA "Date de présence : %s\n", dateRecherche);
     fprintf(fichierSortie, "+-------------------+------------------+------------+\n");
     fprintf(fichierSortie, "| Prénom            | Nom              | Matricule  |\n");
     fprintf(fichierSortie, "+-------------------+------------------+------------+\n");

     while (fscanf(fichierPresence, "%s %s %s %s %s", prenom, nom, matricule, date, heure) != EOF)
     {
         if (strcmp(date, dateRecherche) == 0)
         {
             presenceTrouvee = 1;
             fprintf(fichierSortie, "| %-17s | %-16s | %-10s |\n", prenom, nom, matricule);
         }
     }
     fprintf(fichierSortie, "+-------------------+------------------+------------+\n\n");


     fclose(fichierPresence);
     fclose(fichierSortie);

     if (!presenceTrouvee)
     {
         printf("Pas de liste de présence à cette date. Aucun fichier genénéré . \n");
         remove(nomFichierSortie);
     }
     else{
    
         printf("Le tableau a été généré dans le fichier resultat-%02d-%02d-%04d.txt\n",jour, mois, annee);
     }

     printf("Tapez 0 pour revenir au menu précédent.\n");
 }



/* void genererFichierParDate()
{
    char dateRecherche[20];
    int jour, mois, annee;
    char garbage; // Pour stocker les caractères indésirables

    printf("Veuillez entrer la date au format JJ/MM/AAAA : ");

    // Vérification de la saisie de la date
    if (scanf("%d/%d/%d%c", &jour, &mois, &annee, &garbage) != 4 || garbage != '\n')
    {
        printf("Saisie invalide. Veuillez entrer une date au format JJ/MM/AAAA.\n");
        // Nettoyer le buffer d'entrée
        while (getchar() != '\n');
        return;
    }

    // Validation de la date
    if (jour < 1 || jour > 31 || mois < 1 || mois > 12 || annee < 1)
    {
        printf("Date invalide. Veuillez entrer une date au format valide.\n");
        return;
    }

    sprintf(dateRecherche, "%02d/%02d/%04d", jour, mois, annee);

    FILE *fichierPresence = fopen("file-presence.txt", "r");
    if (fichierPresence == NULL)
    {
        printf("Erreur lors de l'ouverture du fichier de présence.\n");
        return;
    }

    char nomFichierSortie[50];
    sprintf(nomFichierSortie, "resultat-%02d-%02d-%04d.txt", jour, mois, annee);
    FILE *fichierSortie = fopen(nomFichierSortie, "w,  ccs=UTF-8");
    if (fichierSortie == NULL)
    {
        printf("Erreur lors de la création du fichier de sortie.\n");
        fclose(fichierPresence);
        return;
    }

    int presenceTrouvee = 0;
    char date[20];
    char heure[10];
    char matricule[10];
    char nom[20];
    char prenom[20];

    fprintf(fichierSortie, "Date de présence : %s\n", dateRecherche);
    fprintf(fichierSortie, "+-------------------+------------------+-----------+\n");
    fprintf(fichierSortie, "| Prénom            | Nom              | Matricule |\n");
    fprintf(fichierSortie, "+-------------------+------------------+-----------+\n");

    printf("\nListe des étudiants présents à la date %02d/%02d/%04d :\n", jour, mois, annee);
    printf("+-------------------+------------------+------------+\n");
    printf("| Prénom            | Nom              | Matricule  |\n");
    printf("+-------------------+------------------+------------+\n");

    while (fscanf(fichierPresence, "%s %s %s %s %s", prenom, nom, matricule, date, heure) != EOF)
    {
        if (strcmp(date, dateRecherche) == 0)
        {
            presenceTrouvee = 1;
            fprintf(fichierSortie, "| %-17s | %-16s | %-10s |\n", prenom, nom, matricule);
            fprintf(fichierSortie, "+-------------------+------------------+------------+\n\n");

            // Affichage à l'écran
            printf("| %-17s | %-16s | %-10s |\n", prenom, nom, matricule);
        }
    }
    printf("| %-17s | %-16s | %-10s |\n", prenom, nom, matricule);
    printf("+-------------------+------------------+------------+\n\n");

    fclose(fichierPresence);
    fclose(fichierSortie);

    if (!presenceTrouvee)
    {
        printf("Pas de liste de présence à cette date. Aucun fichier généré.\n");
        remove(nomFichierSortie);
    }
    else
    {
        printf("Le tableau a été généré dans le fichier resultat-%02d-%02d-%04d.txt\n", jour, mois, annee);
    }

    printf("Tapez 0 pour revenir au menu précédent.\n");
} */

//------------------------------FIN GENERATION DE FICHIERS------------------------------------------------------------






// --------------------------------------------GESTION MESSAGES--------------------------------------------------------------------



// Fonction pour lire les étudiants à partir d'un fichier

void lireEtudiants(struct Etudiant etudiants[], int *nombreEtudiants)
{
    // Ouvrir le fichier en mode lecture
    FILE *fichier = fopen("odc.txt", "r");

    // Vérifier si le fichier est ouvert avec succès
    if (fichier == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    // Lire le nombre d'étudiants du fichier
    fscanf(fichier, "%d", nombreEtudiants);

    // Lire les détails de chaque étudiant
    for (int i = 0; i < *nombreEtudiants; ++i)
    {
        fscanf(fichier, "%s %s %s %s",
            etudiants[i].matricule,
            etudiants[i].prenom,
            etudiants[i].nom,
            etudiants[i].nom_classe);
    }

    // Fermer le fichier
    fclose(fichier);
}

// Fonction pour lire les messages à partir d'un fichier
void lireMessages(struct Message messages[], int *nombreMessages)
{
    // Ouvrir le fichier en mode lecture
    FILE *fichier = fopen("messages.txt", "r");

    // Vérifier si le fichier est ouvert avec succès
    if (fichier == NULL)
    {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    // Lire le nombre de messages du fichier
    fscanf(fichier, "%d", nombreMessages);

    // Lire les détails de chaque message
    for (int i = 0; i < *nombreMessages; ++i)
    {
        fscanf(fichier, "%s %s %[^\n]",
            messages[i].expediteur,
            messages[i].date,
            messages[i].contenu);
    }

    // Fermer le fichier
    fclose(fichier);
}

// Fonction pour envoyer un message de diffusion
void envoyerMessageDiffusion(struct Etudiant etudiants[], int nombreEtudiants, char contenu[])
{
    for (int i = 0; i < nombreEtudiants; ++i)
    {
        // Afficher le message pour chaque étudiant
        printf("Message envoyé à l'étudiant %s %s (Matricule: %s, Classe: %s)\n",
            etudiants[i].prenom, etudiants[i].nom,
            etudiants[i].matricule, etudiants[i].nom_classe);

        // Ajouter du code pour sauvegarder le message dans la boîte de réception de chaque étudiant
        FILE *boiteReception = fopen(etudiants[i].matricule, "a");
        if (boiteReception == NULL)
        {
            perror("Erreur lors de l'ouverture de la boîte de réception");
            // Ajouter le code pour afficher un message d'échec de l'envoi du message
            printf("Échec de l'envoi du message à l'étudiant %s %s (Matricule: %s)\n",
                etudiants[i].prenom, etudiants[i].nom, etudiants[i].matricule);
        }
        else
        {
            fprintf(boiteReception, "De: Admin\nDate: [Ajoutez la date actuelle]\n\n%s\n\n", contenu);
            fclose(boiteReception);
            // Ajouter le code pour afficher un message de réussite de l'envoi du message
            printf("Message sauvegardé dans la boîte de réception de l'étudiant %s %s (Matricule: %s)\n",
                etudiants[i].prenom, etudiants[i].nom, etudiants[i].matricule);
        }
    }
}

// Fonction pour envoyer un message à une classe spécifique
void envoyerMessageClasse(struct Etudiant etudiants[], int nombreEtudiants, char nom_classe[], char contenu[])
{
    int messageEnvoye = 0; // Pour vérifier si au moins un message a été envoyé à la classe

    // Parcourir tous les étudiants
    for (int i = 0; i < nombreEtudiants; ++i)
    {
        // Vérifier si l'étudiant appartient à la classe spécifiée
        if (strcmp(etudiants[i].nom_classe, nom_classe) == 0)
        {
            // Afficher le message pour chaque étudiant de la classe spécifiée
            printf("Message envoyé à l'étudiant %s %s (Matricule: %s, Classe: %s)\n",
                etudiants[i].prenom, etudiants[i].nom,
                etudiants[i].matricule, etudiants[i].nom_classe);

            // Ajouter du code pour sauvegarder le message dans la boîte de réception de chaque étudiant
            FILE *boiteReception = fopen(etudiants[i].matricule, "a");
            if (boiteReception == NULL)
            {
                perror("Erreur lors de l'ouverture de la boîte de réception");
                // Ajouter le code pour afficher un message d'échec de l'envoi du message
                printf("Échec de l'envoi du message à l'étudiant %s %s (Matricule: %s)\n",
                    etudiants[i].prenom, etudiants[i].nom, etudiants[i].matricule);
            }
            else
            {
                fprintf(boiteReception, "De: Admin\nDate: [Ajoutez la date actuelle]\n\n%s\n\n", contenu);
                fclose(boiteReception);
                messageEnvoye = 1; // Un message a été envoyé avec succès à la classe
            }
        }
    }

    // Vérifier si au moins un message a été envoyé
    if (messageEnvoye)
    {
        // Ajouter le code pour afficher un message de réussite de l'envoi du message
        printf("Messages envoyés avec succès à la classe %s\n", nom_classe);
    }
    else
    {
        // Ajouter le code pour afficher un message d'échec de l'envoi du message
        printf("Aucun étudiant trouvé dans la classe %s\n", nom_classe);
    }
}

// Fonction pour envoyer un message à un etudiant
void envoyerMessagePrive(struct Etudiant etudiants[], int nombreEtudiants, char matricule[], char contenu[])
{
    int etudiantTrouve = 0; // Pour vérifier si l'étudiant avec le matricule spécifié a été trouvé

    // Parcourir tous les étudiants
    for (int i = 0; i < nombreEtudiants; ++i)
    {
        // Vérifier si l'étudiant a le matricule spécifié
        if (strcmp(etudiants[i].matricule, matricule) == 0)
        {
            // Afficher le message pour l'étudiant spécifié
            printf("Message envoyé à l'étudiant %s %s (Matricule: %s, Classe: %s)\n",
                etudiants[i].prenom, etudiants[i].nom,
                etudiants[i].matricule, etudiants[i].nom_classe);

            // Ajouter du code pour sauvegarder le message dans la boîte de réception de l'étudiant
            FILE *boiteReception = fopen(matricule, "a");
            if (boiteReception == NULL)
            {
                perror("Erreur lors de l'ouverture de la boîte de réception");
                // Ajouter le code pour afficher un message d'échec de l'envoi du message
                printf("Échec de l'envoi du message à l'étudiant %s %s (Matricule: %s)\n",
                    etudiants[i].prenom, etudiants[i].nom, etudiants[i].matricule);
            }
            else
            {
                fprintf(boiteReception, "De: Admin\nDate: [Ajoutez la date actuelle]\n\n%s\n\n", contenu);
                fclose(boiteReception);
                etudiantTrouve = 1; // L'étudiant a été trouvé, et le message a été envoyé avec succès
            }
        }
    }

    // Vérifier si l'étudiant avec le matricule spécifié a été trouvé
    if (etudiantTrouve)
    {
        // Ajouter le code pour afficher un message de réussite de l'envoi du message
        printf("Message envoyé avec succès à l'étudiant avec le matricule %s\n", matricule);
    }
    else
    {
        // Ajouter le code pour afficher un message d'échec de l'envoi du message
        printf("Aucun étudiant trouvé avec le matricule %s\n", matricule);
    }
}

// Fonction pour afficher les messages
void afficherMessages(struct Message messages[], int nombreMessages)
{
    // Vérifier s'il y a des messages à afficher
    if (nombreMessages > 0)
    {
        // Parcourir tous les messages
        for (int i = 0; i < nombreMessages; ++i)
        {
            // Afficher les détails du message
            printf("De: %s\nDate: %s\n\n%s\n\n", messages[i].expediteur, messages[i].date, messages[i].contenu);
        }
    }
    else
    {
        // Aucun message à afficher
        printf("Aucun message disponible dans la boîte de réception.\n");
    }
}





//-------------------------------------------------------- Main -------------------------------------------------------







int main()
{

    struct Etudiant etudiants[500]; // Supposons un maximum de 500 étudiants
    struct Message messages[100];   // Supposons un maximum de 100 messages


    int nombreEtudiants = 0;
    int nombreMessages = 0;


    // Charger les étudiants et les messages depuis les fichiers correspondants
    lireEtudiants(etudiants, &nombreEtudiants);
    lireMessages(messages, &nombreMessages);






    // Création des fichiers pour stocker les identifiants
    FILE *fichierAdmin = fopen("admins.txt", "r");
    FILE *fichierEtudiant = fopen("etudiants.txt", "r");

    if (fichierAdmin == NULL || fichierEtudiant == NULL)
    {
        printf("Erreur lors de l'ouverture des fichiers.\n");
        return 1;
    }

    // Variables pour stocker les identifiants
    Identifiants identifiantsAdmin[100];    // Pour stocker jusqu'à 100 identifiants d'administrateur
    Identifiants identifiantsEtudiant[100]; // Pour stocker jusqu'à 100 identifiants d'étudiant

    int nombreIdentifiantsAdmin = 0;
    int nombreIdentifiantsEtudiant = 0;

    // Lecture des identifiants de l'admin
    while (fscanf(fichierAdmin, "%s %s", identifiantsAdmin[nombreIdentifiantsAdmin].login, identifiantsAdmin[nombreIdentifiantsAdmin].motDePasse) == 2)
    {
        nombreIdentifiantsAdmin++;
    }
    fclose(fichierAdmin);

    // Lecture des identifiants de l'étudiant
    while (fscanf(fichierEtudiant, "%s %s", identifiantsEtudiant[nombreIdentifiantsEtudiant].login, identifiantsEtudiant[nombreIdentifiantsEtudiant].motDePasse) == 2)
    {
        nombreIdentifiantsEtudiant++;
    }
    fclose(fichierEtudiant);

    int choix = 0;
    int choixMenu;
    char saisieLogin[LONGUEUR_MAX_LOGIN];
    char *saisieMotDePasse = malloc(LONGUEUR_MAX_MDP * sizeof(char)); // Allocation mémoire

    // Authentification
    do
    {
        // system("clear");
        printf("---------------- Connexion ----------------\n\n");
        saisieLogin[LONGUEUR_MAX_LOGIN] = '\0';
        printf("----- login : ");
        fgets(saisieLogin, LONGUEUR_MAX_LOGIN, stdin);
        saisieLogin[strcspn(saisieLogin, "\n")] = 0; // Supprime le caractère de nouvelle ligne
        if (strlen(saisieLogin) == 0)
        {
            printf("\nVous avez laissé le champ vide. Veuillez rentrer votre login.\n");
            continue;
        }

        printf("----- Mot de passe : ");

        int i = 0, c;
        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n')
        {
            if (c == 127)
            { // ASCII value for backspace
                if (i > 0)
                {
                    printf("\b \b"); // Effacer le caractère précédent
                    i--;
                }
            }
            else
            {
                saisieMotDePasse[i++] = c;
                printf("*");
            }
        }
        saisieMotDePasse[i] = '\0';

        if (strlen(saisieMotDePasse) == 0)
        {
            printf("\nVous avez laissé le champ vide. Veuillez entrer votre mot de passe.\n");
            continue;
        }

        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) && !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
        {
            printf("\nLogin ou mot de passe invalides.\n");
        }
        if ((verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)))
        {
            do
            {

                system("clear");
                printf("\n--------------------------------------------------------------------------\n");
                printf( GREEN"\t\t\tBienvenue dans le menu de l'administrateur:\n" RESET);
                printf("--------------------------------------------------------------------------\n");
                printf("\t1 --->Gestion des étudiants\n");
                printf("\t2 --->Génération de fichiers\n");
                printf("\t3 ---> Marquer les présences\n");
                printf("\t4 --->Envoyer  messages\n");
                printf("\t5---> Paramètres\n");
                printf("\t6 ---> Deconnexion\n");
                printf("\n--- Entrez votre choix : ");
                scanf("%d", &choix);

                if (choix == 2)
                {
                    int sousChoixFichiers = 0;
                    do
                    {
                        printf("\n--- Génération de fichiers ---\n");
                        printf("1 ---> Toutes les présences\n");
                        printf("2 ---> Par date\n");
                        printf("0---> Retour au menu principal\n");
                        printf("\n--- Entrez votre choix : ");
                        scanf("%d", &sousChoixFichiers);

                        if (sousChoixFichiers == 1)
                        {
                            // Génération du fichier de toutes les présences
                            genererFichierToutesPresences();
                        }
                        else if (sousChoixFichiers == 2)
                        {
                            // Génération du fichier par date
                            genererFichierParDate();
                        }
                        else if (sousChoixFichiers != 0)
                        {
                            printf("Choix invalide. Veuillez entrer 0, 1 ou 2.\n");
                        }

                    } while (sousChoixFichiers != 0);
                }

                if (choix == 3)
                {
                    system("clear");
                    marquerPresence();
                    do
                    {
                        viderBuffer();
                        printf("---> Mot de passe : ");
                        int i = 0, c;
                        while (i < LONGUEUR_MAX_MDP - 1 && (c = getch()) != '\n')
                        {
                            if (c == 127)
                            {
                                if (i > 0)
                                {
                                    printf("\b \b"); // Effacer le caractère précédent
                                    i--;
                                }
                            }
                            else
                            {
                                saisieMotDePasse[i++] = c;
                                printf("*");
                            }
                        }
                        saisieMotDePasse[i] = '\0';

                        if (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)))
                        {
                            printf("\n---->⛔ Vous n'êtes pas l'admin !!!\n");
                            getchar();
                            system("clear");
                            marquerPresence();
                        }
                        else
                        { // Appel de la fonction après la saisie du mot de passe correct
                        }
                    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)));
                }
                if (choix == 4)
                {
                    
                                printf("1. Envoyer un message par diffusion\n");
                                printf("2. Envoyer un message par classe\n");
                                printf("3. Envoyer un message privé\n");

                                int choixAdmin;
                                scanf("%d", &choixAdmin);

                                if (choixAdmin == 1)
                                {
                                    char contenu[500];
                                    printf("Entrez le contenu du message de diffusion: ");
                                    getchar(); // Pour consommer le caractère de nouvelle ligne restant
                                    fgets(contenu, sizeof(contenu), stdin);
                                    envoyerMessageDiffusion(etudiants, nombreEtudiants, contenu);
                                }

                                else if (choixAdmin == 2)
                                {
                                    // Envoyer un message par classe
                                    char nomClasse[20];
                                    char contenu[500];

                                    // Demander le nom de la classe et le contenu du message
                                    printf("Entrez le nom de la classe : ");
                                    scanf("%s", nomClasse);
                                    getchar(); // Pour consommer le caractère de nouvelle ligne restant
                                    printf("Entrez le contenu du message : ");
                                    fgets(contenu, sizeof(contenu), stdin);

                                    // Appeler la fonction pour envoyer le message par classe
                                    envoyerMessageClasse(etudiants, nombreEtudiants, nomClasse, contenu);
                                }

                                else if (choixAdmin == 3)
                                {
                                    // Envoyer un message privé
                                    char matriculeDestinataire[20];
                                    char contenu[500];

                                    // Demander le matricule de l'étudiant destinataire et le contenu du message
                                    printf("Entrez le matricule de l'étudiant destinataire : ");
                                    scanf("%s", matriculeDestinataire);
                                    getchar(); // Pour consommer le caractère de nouvelle ligne restant
                                    printf("Entrez le contenu du message : ");
                                    fgets(contenu, sizeof(contenu), stdin);

                                    // Appeler la fonction pour envoyer le message privé
                                    envoyerMessagePrive(etudiants, nombreEtudiants, matriculeDestinataire, contenu);
                                }

                                else
                                {
                                    printf("Option invalide\n");
                                }
                }
                
                if (choix == 6)
                {
                    printf("Vous êtes déconnecté !\n");
                }
                if (choix < 1 || choix > 6)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre 1 et 2.\n");
                }
            } while (choix != 6);
        }
        if ((verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)))
        {
            int choix = 0;
            do
            {
                system("clear");
                printf("\n--------------------------------------------------------------------------\n");
                printf( GREEN"\t\t\tBienvenue dans le menu de l'apprenant :\n" RESET);
                printf("--------------------------------------------------------------------------\n");
                printf("\t1 ----> MARQUER MA PRESENCE\n");
                printf("\t2 ----> NOMBRES DE MINUTES D'ABSENCES\n");
                printf("\t3 ----> MARQUER SA PRÉSENCE\n");
                printf("\t4 ----> MES MESSAGES (0)\n");
                printf("\t5 ---->  Déconnexion\n");
                printf("\n--- Entrez votre choix : ");
                scanf("%d", &choix);
                if (choix < 1 || choix > 5)
                {
                    printf("Choix invalide. Veuillez entrer un choix entre  1 et 5.\n");
                    system("clear");
                }
                if (choix == 1)
                {

                    //----------------------- Doublons & Présence ---------------------------------------------------------
                    FILE *fichierPresence = fopen("file-presence.txt", "r");
                    if (fichierPresence == NULL)
                    {
                        printf("Erreur lors de l'ouverture du fichier de présence.\n");
                        return 1;
                    }

                    int present = 0;
                    char matricule[10];
                    char prenom[20];
                    char nom[20];
                    while (fscanf(fichierPresence, "%s", matricule) != EOF)
                    {
                        if (strcmp(matricule, matricule) == 0)
                        {
                            printf(RED "\n---  L'étudiant de matricule %s est déjà marqué présent.\n" RESET, matricule);
                            present = 1;
                            break;
                        }
                    }

                    fclose(fichierPresence);

                    if (!present)
                    {
                        FILE *fichier = fopen("file-etudiant.txt", "r+");
                        if (fichier == NULL)
                        {
                            printf("Erreur lors de l'ouverture du fichier d'etudiants.\n");
                            return 1;
                        }

                        while (fscanf(fichier, "%s", matricule) != EOF)
                        {
                            if (strcmp(matricule, matricule) == 0)
                            {
                                // Enregistrer la présence dans le fichier
                                enregistrerPresence(prenom, nom, matricule);
                                printf(GREEN "\n--- ✅ Presence marquee pour l'etudiant de matricule %s\n" RESET, matricule);
                                present = 1;
                                break;
                            }
                        }

                        fclose(fichier);
                    }
                    //----------------------------------------  Fin -----------------------------------------------------
                }
                
                if (choix == 4)
                {
                                printf("1. Consulter mes messages\n");

                                int choixEtudiant;
                                scanf("%d", &choixEtudiant);

                                if (choixEtudiant == 1)
                                {
                                    // Consulter les messages
                                    afficherMessages(messages, nombreMessages);
                                }
                                else
                                {
                                    printf("Option invalide\n");
                                }
                }
                
                
                if (choix == 5)
                {
                    system("clear");
                    printf("Vous êtes déconnecté !\n");
                    saisieLogin[LONGUEUR_MAX_LOGIN] = 'a';
                }
            } while (choix != 5);
        }
    } while (!(verifierIdentifiants(identifiantsAdmin, nombreIdentifiantsAdmin, saisieLogin, saisieMotDePasse)) || !(verifierIdentifiants(identifiantsEtudiant, nombreIdentifiantsEtudiant, saisieLogin, saisieMotDePasse)));

    return 0;
}
