#define REUSE 1
#define REVDNS 1

#define chName 0
#define getCanauxNames 1
#define joinCanal 2
#define sendMess 3

#include "assert.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <time.h>
#ifdef REVDNS
#include <netdb.h>
#include <stdlib.h>
#endif
struct message{
    int type;
    char sender[20];
    char canalName[20];
    char message[120];
};
struct retour{
    int type;
    int taille;
};
struct users{
    char nom[21];
    int adresse;
    struct users *ptrUserSuivante;
    struct userCanals *userscanaux;
};
struct canaux{
    char nom[21];
    struct usersConn *usersconn;
    struct canaux *nextCanal;
};
struct usersConn{
    struct users *user;
    int status;
    struct usersConn *nextUserConnected;
};
struct userCanals{
    struct canaux *canal;
    struct userCanals *nextUserCanal;
};
struct error{ //TODO change to retour
    int type; //-1 car retour
    int isError; // 0 pour pas erreur 1 pour erreur
    char message[40];
};

void ljust(char *str, int length, char fill_char){
    int str_len = strlen(str);

    if (str_len < length) {
        int fill_len = length - str_len;
        if (fill_len > 0) {
            for (int i = str_len; i < length; ++i) {
                str[i] = fill_char;
            }
            str[length] = '\0';
        }
    }
}
int isUserInChannel(struct usersConn **liste,struct users *u){
    struct usersConn *uc = *liste;
    while (uc!=NULL) {
        if (uc->user == u) {
            return 1;
        } else {
            uc = uc->nextUserConnected;
        }
    }
    return 0;
}

void generateUserName(char *name){

}

void nouvelleUser(struct users **liste, char *nom, int adresse) {
    struct users *newuser = malloc(sizeof(struct users));
    strncpy(newuser->nom, nom, sizeof(newuser->nom));
    newuser->adresse = adresse;
    newuser->ptrUserSuivante = NULL;
    newuser->userscanaux=NULL;
    if (*liste == NULL) {
        *liste = newuser;
    }else {
        struct users *dernierUser = *liste;
        while (dernierUser->ptrUserSuivante != NULL) {
            dernierUser = dernierUser->ptrUserSuivante;
        }

        // Liaison de la nouvelle personne à la fin de la liste
        dernierUser->ptrUserSuivante = newuser;
    }
}
int isCanalNameUsed(struct canaux *liste,  char *nom,struct canaux **c){
    printf("entrée dans la fct\n");
    struct canaux *dernierCanal = liste;
    while (dernierCanal != NULL) {
        if(strcmp(dernierCanal->nom, nom) == 0){ // erreur ici
            *c=dernierCanal;
            return 1;
            //return -1;
        }else {
            dernierCanal = dernierCanal->nextCanal;
        }
    }
    printf("sortie de la fct\n");
    c=NULL;
    return 0;
}
int nouveuCanal(struct canaux **liste, char *nom, struct users *creator) {
    struct canaux *c;
    if(isCanalNameUsed(*liste,nom,c)==0) {
        struct canaux *newCanal = malloc(sizeof(struct users));
        strncpy(newCanal->nom, nom, sizeof(newCanal->nom));
        struct usersConn *newUserConn = malloc(sizeof(struct usersConn));
        newUserConn->status = 1;
        newUserConn->user = creator;
        newUserConn->nextUserConnected=NULL;
        newCanal->usersconn = newUserConn;
        newCanal->nextCanal = NULL;
        if (*liste == NULL) {
            *liste = newCanal;
        } else {
            struct canaux *dernierCanal = *liste;
            while (dernierCanal->nextCanal != NULL) {

                dernierCanal = dernierCanal->nextCanal;
            }
            dernierCanal->nextCanal = newCanal;
        }
        return 0;
    }else{
        return 1;
    }
}
int addrAvecNom(struct users *liste,  char *nom){
    struct users *dernierUser = liste;
    while (dernierUser != NULL) {
        if(strcmp(dernierUser->nom, nom) == 0){ // erreur ici
            return dernierUser->adresse;
            //return -1;
        }else {
            dernierUser = dernierUser->ptrUserSuivante;
        }
    }
    printf("not find\n");
    return -1;
}
int getAllCanalsName(struct canaux **liste, char **nom) {
    struct canaux *dernierCanal = *liste;
    int tailleTotale = 0;
    while (dernierCanal != NULL) {
        tailleTotale += strlen(dernierCanal->nom)+1;
        dernierCanal = dernierCanal->nextCanal;
    }
    *nom = (char*)malloc(sizeof(char) * (tailleTotale));
    if (*nom == NULL) {
        return -1;
    }
    (*nom)[0] = '\0';
    dernierCanal = *liste;
    while (dernierCanal != NULL) {
        strcat(*nom, dernierCanal->nom);
        if(dernierCanal->nextCanal!=NULL) {
            strcat(*nom, "|");
        }
        dernierCanal = dernierCanal->nextCanal;
    }
    //(*nom)[tailleTotale]=' ';
    return tailleTotale; //
}
int joinCanale(struct canaux *canale,struct users *user){
    //TODO: verifier que l'utilisateur ne soit pas déjà connecté
    assert(canale!=NULL);
    assert(user!=NULL);

    struct usersConn *newConn = malloc(sizeof(struct usersConn));
    //struct userConn **newConnlist = (struct userConn **) &(canale->usersconn);
    assert(newConn!=NULL);
    newConn->user=user;
    newConn->status=0;
    assert(newConn->user!=NULL);
    newConn->nextUserConnected= canale->usersconn;
    canale->usersconn = newConn;

    fflush(stdout);//permet d'assurer que le printf passe
    // on peut utlisé la double étoile si pointeur sur pointeur

    printf("debug 1.8\n");
    //userCanalCourant->nextUserConnected=newConn;
    printf("debug 2\n");
    struct userCanals *newUserCanal = malloc(sizeof(struct userCanals));
    printf("debug 2.5\n");
    newUserCanal->canal=canale;
    newUserCanal->nextUserCanal=user->userscanaux;
    user->userscanaux=newUserCanal;
    /*struct usersConn *userCourant = (struct usersConn *) &(user->userscanaux);
    printf("debug 3");
    while (userCourant!=NULL){
        userCourant=userCourant->nextUserConnected;
    }
    //userCanalCourant->nextUserConnected=newUserCanal;
    printf("debug 3");*/
    return 1;
}
int changerNom(struct users **liste,   char *nom, int adresse){ //0 si nom utilisé, 1 si c'est bon
    struct users *dernierUser = *liste;
    if(addrAvecNom(*liste,nom)==-1) {
        while (dernierUser!= NULL) {
            if (dernierUser->adresse == adresse){
                strncpy(dernierUser->nom, "                      ", sizeof(&dernierUser->nom));
                strncpy(dernierUser->nom, nom, sizeof(&dernierUser->nom));

                //dernierUser->nom[sizeof(dernierUser->nom) - 1] = '\0'; // Assurer la terminaison de la chaîne
                return 1;
            } else {
                dernierUser = dernierUser->ptrUserSuivante;
            }
        }

        return 0;

    }
    return -1;
}
void sendResponse(char *error,int adresse,int isError){
    printf("LOG | %s\n",error);
    printf("%lu\n",strlen(error));
    struct error e1;
    if (strlen(error)<=40) {
        assert(adresse!=NULL);
        printf("%d\n",adresse);
        e1.type = -1;
        e1.isError = isError;
        strncpy(e1.message, error, sizeof(e1.message));
        printf("%lu\n",strlen(error));
        printf("debug1\n");
        ljust(e1.message, 40, ' ');
        printf("debug2\n");
        printf("%lu\n", sizeof(e1));
        fflush(stdout);
        int l = write(adresse, &e1, sizeof(e1));
        printf("n = %d\n",l);
        printf("debug3\n");
        fflush(stdout);
        printf("debug4\n");
        fflush(stdout);
    }else{
        printf("Log | erreur lors de l'envoie de l'erreur : %s\n",error );
    }
    printf("sortie");
    fflush(stdout);
}
void sendMessage(struct usersConn **liste,char *sender,char *message,char *nomCanal){
    struct message m;
    m.type=7;
    strncpy(m.sender, sender, sizeof(m.sender));
    strncpy(m.canalName, nomCanal, sizeof(m.canalName));
    strncpy(m.message, message, sizeof(m.message));
    printf("entrée\n");
    assert(liste!=NULL);
    struct usersConn *currUserConn = *liste;
    printf("debut boucle\n");
    while(currUserConn!=NULL){
        assert(currUserConn!=NULL);
        printf("debug100\n");
        assert(currUserConn->user!=NULL);
        struct users* u = currUserConn->user;
        printf("debug100.5\n");
        assert(u!=NULL);
        write(u->adresse,&m,sizeof(m)); // bug : passage de l'adresse
        printf("debug101\n");
        currUserConn = currUserConn->nextUserConnected;
        printf("User conn\n");
    }
    printf("sortie\n");
}


int main() {
    int s, c, n, clilen;
    struct sockaddr_in srv, cli;
    char error[20];
    uint8_t buf[512];
    printf(" LOG |  démarage du server\n");


#ifdef REUSE
    int optval;
#endif
    char namebuf[128],portbuf[32];
#ifdef REVDNS

#endif

    /* Create socket */
    s=socket(AF_INET,SOCK_STREAM,0);

#ifdef REUSE
    /* Set option REUSEADDR to allow direct reconnection on socket when client closed */
    optval=1;
    setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(void *)&optval,sizeof(optval));
#endif

    /* Bind to port, Address = ANY (0.0.0.0) */
    bzero(&srv,sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_addr.s_addr = htonl(INADDR_ANY);
    srv.sin_port = htons(6000);
    if (bind(s,(struct sockaddr *) &srv,sizeof(srv))<0) {
        printf("LOG |  error bind\n");
    }

    /* Put socket in Listen mode, max backlog = 1 */
    if (listen(s,1)!=0) {
        printf("LOG | error listen\n");
    }



#ifdef REVDNS
    getnameinfo((struct sockaddr *) &cli, sizeof(cli),
                namebuf,sizeof(namebuf),
                portbuf,sizeof(portbuf), /* Or NULL,0 for no port info */
                0); /* NI_NOFQDN, NI_NUMERIHOST */
    printf("Hostname : %s\n",namebuf);
#endif
    //TODO: rajouter des assert(cond) dans le code
    struct users *premieruser = NULL;
    struct canaux *premierCanal = NULL;
    int nUser =0;
    fd_set readfds;

    struct   timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    int highsock =-1;
    int i=0;
    struct users *u = malloc(sizeof(struct users));
    u->adresse=1;
    char nom[21]={"test1"};
    ljust(nom,20,' ');
    strncpy(u->nom, nom ,sizeof (u->nom));
    u->ptrUserSuivante=NULL;
    strncpy(nom, "test1", sizeof(nom));
    ljust(nom,20,' ');
    nouveuCanal(&premierCanal,nom,u);
    strncpy(nom, "test2", sizeof(nom));
    ljust(nom,20,' ');
    nouveuCanal(&premierCanal,nom,u);

    while(i<100) {
        FD_ZERO(&readfds);
        FD_SET(s, &readfds);
        int max_sd = s;
        clilen=sizeof(cli);
        struct users *userCourant = premieruser;
        printf(" LOG | Liste des users connectés : \n");
        while (userCourant != NULL) {
            FD_SET(userCourant->adresse, &readfds);
            if(userCourant->adresse>max_sd){
                max_sd=userCourant->adresse;
            }
            struct userCanals *canalCourant=userCourant->userscanaux;
            printf("user %s adresse : %d\n",userCourant->nom,userCourant->adresse);
            printf("Canaux : \n");
            while (canalCourant != NULL) {
                printf("%s\n",canalCourant->canal->nom);
                canalCourant=canalCourant->nextUserCanal;
            }

            userCourant = userCourant->ptrUserSuivante;
        }
        struct canaux *canalCourant = premierCanal;
        printf(" LOG | Liste des canaux : \n");
        while (canalCourant != NULL) {
            printf("Nom : %s\n",canalCourant->nom);

            struct usersConn *userCourant=canalCourant->usersconn;
            printf("User : \n");
            fflush(stdout);
            assert(userCourant!=NULL);
            while (userCourant != NULL) {
                printf("debut boucle");
                fflush(stdout);
                if(userCourant!=NULL) {
                    printf("%s\n", userCourant->user->nom);
                }
                userCourant=userCourant->nextUserConnected;
            }
            printf("test\n");
            fflush(stdout);
            canalCourant = canalCourant->nextCanal;
        }
        printf(" ");
        int activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0))
        {
            printf("LOG | select error");
        }

            if (FD_ISSET(s, &readfds)) {
                c = accept(s, (struct sockaddr *) &cli, &clilen);
                printf("LOG |  Connexion from %s adresse %d \n", inet_ntoa(cli.sin_addr), c);

                nUser++;
                char nom[20];
                char characters[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9',
                                     'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
                                     'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                                     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
                                     'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
                //nom = (char*)malloc(sizeof(char) * (21));
                srand(time(NULL));
                for(int i=0;i<strlen(nom);i++){
                    nom[i]=characters[rand() % 60 + 0];
                }
                ljust(nom,20,' ');
                printf("Long dans acc %lu",strlen(nom));
                nouvelleUser(&premieruser, nom, c);
                write(c, nom, strlen(nom));
                //FD_SET(c, &readfds);
                if (c > highsock) {
                    highsock = c;
                }
            }
            userCourant = premieruser;
            while (userCourant != NULL) {

                if (FD_ISSET(userCourant->adresse, &readfds)) {
                    // Socket prêt pour la lecture

                    n = read(userCourant->adresse, buf, sizeof(buf));
                    buf[n] = '\0';
                    if (n <= 0) {
                        printf("Déconnexion du client : %s\n",userCourant->nom);
                        close(userCourant->adresse);
                        // TODO: Supprimer le client de la liste
                    } else {
                        int v1;
                        memcpy(&v1, &buf[0], sizeof(int));
                        if(n!=v1){
                            sendResponse("error de taille1",userCourant->adresse,1);
                        }else {
                            int v2;
                            memcpy(&v2, &buf[sizeof(int)],sizeof(int)); // ou v2= * (int*) (buf+sizeof(int)); ou v2 = *((int*) buf)+1);
                            printf("debug0\n");
                            switch (v2) {
                                case chName:
                                    if(n== sizeof(int)*2+sizeof(char)*21) {
                                        char mot[20];
                                        memcpy(&mot, &buf[sizeof(int) * 2], 21);
                                        printf("LOG | Changement du nom de %s\n",userCourant->nom);
                                        if(changerNom(&premieruser, mot, userCourant->adresse)>0) {
                                            printf("LOG | Nouveau nom : %s\n",mot);
                                            sendResponse("ok",userCourant->adresse,0);
                                        }else{
                                            printf("LOG | Erreur, nom déja réservé : %s\n",mot);
                                            sendResponse(userCourant->nom,userCourant->adresse,1);
                                        }
                                    }else{
                                        sendResponse("error de taille2",userCourant->adresse,1);
                                    }
                                    break;
                                case getCanauxNames:
                                    if(n== sizeof(int)*2) {
                                        char* retour;
                                        int taille = getAllCanalsName(&premierCanal,&retour);

                                        struct retour r;
                                        printf("taille retournée = %d\n",taille);
                                        r.type=1;
                                        r.taille=taille;
                                        write(userCourant->adresse, &r, sizeof(r));
                                        printf("%s,%lu\n",retour,sizeof(retour));
                                        write(userCourant->adresse,retour,taille);


                                    }else{
                                        sendResponse("error de taille2",userCourant->adresse,1);
                                    }
                                    break;
                                case joinCanal:
                                    if(n== sizeof(int)*2+sizeof(char)*21) {
                                        char nom[20];
                                        struct canaux *c;

                                        memcpy(&nom, &buf[sizeof(int) * 2], 21);

                                        if(isCanalNameUsed(premierCanal,nom,&c)==1){

                                            if (c==NULL) {
                                                printf("LOG | erreur c est null");
                                                sendResponse("Error canal not found",userCourant->adresse,1);
                                            }else {
                                                if(isUserInChannel(&(c->usersconn),userCourant)==0) {
                                                    joinCanale(c, userCourant);
                                                    sendResponse("ok", userCourant->adresse, 0);
                                                    printf("LOG | %s rejoint le canal : %s", userCourant->nom, c->nom);
                                                }else{
                                                    printf("LOG | %s est déjà dans le canal : %s",userCourant-> nom,c->nom);
                                                    sendResponse("Vous etes deja dans le canal",userCourant->adresse,1);
                                                }

                                            }
                                        }else{
                                            nouveuCanal(&premierCanal,nom,userCourant);
                                            sendResponse("nouveau",userCourant->adresse,0);
                                            printf("LOG | nouveau canal : %s ",c->nom);
                                        }
                                    }else{
                                        sendResponse("error de taille2",userCourant->adresse,1);
                                    }
                                    break;
                                case sendMess:


                                    if(n== sizeof(int)*2+sizeof(char)*21+ sizeof(char)*121) {
                                        char nom[20];
                                        char message[120];
                                        struct canaux *c;
                                        printf("%s\n",buf);
                                        memcpy(&nom, &buf[sizeof(int) * 2], 21);
                                        memcpy(&message, &buf[(sizeof(int) * 2)+ sizeof(char)*21], 121);
                                        printf("Nom du canal : %s\n",nom);
                                        if(isCanalNameUsed(premierCanal,nom,&c)==1){

                                            if (c==NULL) {
                                                printf("LOG | erreur c est null");
                                                //sendResponse("Error canal not found",userCourant->adresse,1);
                                            }else {
                                                if(isUserInChannel(&(c->usersconn),userCourant)==1) {
                                                    sendMessage(&(c->usersconn),userCourant->nom,message,c->nom);
                                                    //sendResponse("ok",userCourant->adresse,0);
                                                    printf("LOG | %s envoie un message sur  le canal : %s\n", userCourant->nom, c->nom);
                                                }else{
                                                    printf("LOG | %s n'est pas dans le canal : %s\n",userCourant-> nom,c->nom);
                                                    sendResponse("Vous n'etes pas dans le canal",userCourant->adresse,1);
                                                }

                                            }
                                        }else{

                                            //sendResponse("Canal not found",userCourant->adresse,1);
                                            printf("LOG | Canal not found  ");
                                        }
                                    }else{
                                        //sendResponse("error de taille2",userCourant->adresse,1);
                                    }
                                    break;
                                default:
                                    sendResponse("erreur de type de packet",userCourant->adresse,1);
                                    break;
                            }

                        }
                    }
                }
                userCourant = userCourant->ptrUserSuivante;

            }

        i++;

        /*
        int v1,v2;
        char mot[21];
        printf("taille de buf  : %lu\n", sizeof(buf));
        printf("Message du client %s : %s\n", userCourant->nom, buf);
        memcpy(&v1, &buf[0], sizeof(int));
        printf("v1 : %d\n",v1);
        if(sizeof(buf)==v1) {
            memcpy(&v2, &buf[sizeof(int)], sizeof(int)); // ou v2= * (int*) (buf+sizeof(int)); ou v2 = *((int*) buf)+1);
            printf("v2 : %d\n", v2);
            memcpy(&mot, &buf[sizeof(int) * 2], 21);
            printf("mot : %s\n", mot);
            write(userCourant->adresse, buf, n);
            printf("message sent");
        }else{
            printf("erreur de comm");
        }*/






























    }
    printf("LOG | extinction du server");
    close(s);
    return 0;
}



/* getaddrinfo : flags = AI_PASSIVE => si adresse = NULL, utiliser INADDR_ANY
 * Permet de retrouver le numÃ©ro de port Ã  partir du protocole. */
