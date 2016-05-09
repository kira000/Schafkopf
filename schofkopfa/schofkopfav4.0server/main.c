#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>
#include <winsock2.h>
#include <string.h>

#define PORT 1283
#define BUF 512

typedef int socklen_t;

typedef struct doubleint{
    int a;
    int b;
} DI;

// schau ganz obe bevorsd wosd machsd
// hob i da mei angsd hoid si in grenzn :P

void mischen_austeilen (char *alle_spieler[4][8], char karten[4][8][16], int vergeben[4][8]);
int mitspieler_bestimmen(DI spielerteam, char *alle_spieler[4][8], int gespielte_sau, char karten[4][8][16]);
int rundespielen (char *alle_spieler[4][8], char karten[4][8][16], char *stiche[8][5], int aktueller_ausspieler, int runde, DI spielerteam, int gespielte_sau, DI spieltyp, int davolaufa, int anfaenger[], int *clientsock);
void anzeigen_karten_spieler (char *alle_spieler[8]);
void anzeigen_spielbare_karten_spieler (DI spieltyp,char *gespielte_karte,char karten[4][8][16], int j, int gespielte_farbe, int gespielter_schlag, int aktueller_ausspieler, char *alle_spieler[4][8],int sau_farbe, int nur_pruefen, int davolaufa, int anfaenger[],int* client_sock);
char* rechtbestimmen(char* hoechste, char* neugespielte, char karten[4][8][16], DI spieltyp);
void karten_sortieren(char *alle_spieler[4][8],char karten[4][8][16], DI spielertyp);
DI kartenposbest(char* karte, char karten[4][8][16]);
DI spielerposbest(char* karte, char* alle_spieler[4][8]);
DI punktezaehlen(DI spielerteam, char *stiche[8][5], char karten[4][8][16], int rrsp[8]);
int farbe_holen(char *gespielte_karte, char karten[4][8][16]); //ma
int schlag_holen(char *gespielte_karte,char karten[4][8][16]); //ma
DI sauabfragefkt(char *alle_spieler[4][8], char karten[4][8][16], DI spielerteam, int spielanfang_ausspieler, int *client_sock);
DI solofarbabfragefkt(char *alle_spieler[4][8], char karten[4][8][16], int spielanfang_ausspieler, DI spielerteam, int *client_sock);
//DI wenzabfragefkt(char *alle_spieler[4][8], char karten[4][8][16], int spielanfang_ausspieler, DI spielerteam, SOCKET *sock);
DI richtig_gspeid(DI spieltyp,char *gespielte_karte,char karten[4][8][16], int j,int gespielte_farbe, int gespielter_schlag, int aktueller_ausspieler,char *alle_spieler[4][8],int sau_farbe,int nur_pruefen,int davolaufa, int *clientsock);
int zugegeben(int j,int gespielte_farbe,int trumpf_hand,int farbe_hand,int farbe,int trumpf, int gespielter_schlag, DI spieltyp, int trumpf_farbe, int nur_pruefen, int *clientsock, int aktuellerausspieler);
int davolaufabestfkt(char *mitspielerkarten[8],char karten[4][8][16],int gespielte_sau, int *cliensock);

// ab do netzwerkfkts

void cleanup (void);
void error_exit(char *error_message);
void senden (int *sock, char *buffer, size_t bufferlaenge);
//void sendencls(int *sock);
//void sendenpause(int *sock);
void sendentext(int *client_sock, int spieler, char *text);
void sendentexts(int *client_sock, int spieler, char *text, int ausgabeserver);
void sendenschleife(int *client_sock, char *text, int ausgebenserver);
void empfangenschleife(int *client_sock);
void sendenpause(int *client_sock);
char empfangentext(int *client_sock, int spieler);
void anzeigenKartenSpielerNetzwerk(int *client_sock, char *alle_spieler[4][8], int spieler);
void anzeigenKartenAllenSpielern(int *client_sock, char *alle_spieler[4][8]);
char eingaben(int *client_sock, char *text, int spieler);
void sendenpauseeinzel(int *client_sock, int spieler);

int main()
{
    char karten[4][8][16]={ {"schellen_7","schellen_8","schellen_9","schellen_unter","schellen_ober","schellen_koenig","schellen_10","schellen_sau"},

                            {"herz_7","herz_8","herz_9","herz_unter","herz_ober","herz_koenig","herz_10","herz_sau"},

                            {"gras_7","gras_8","gras_9","gras_unter","gras_ober","gras_koenig","gras_10","gras_sau"},

                            {"eichel_7","eichel_8","eichel_9","eichel_unter","eichel_ober","eichel_koenig","eichel_10","eichel_sau"}};

    char *alle_spieler[4][8], gespielt, *stiche[8][5]={}, anf='n';
    char bla;

    int i=0, j=0, k=0, vergeben[4][8]={}, aktueller_ausspieler, spielanfang_ausspieler, gespielte_sau, runde=0, davolaufa=0, anfaenger[4]={};
    int rrsp[8]={}; //rrsp=rundenrechtspieler=spieler die in der jeweiligen runde 0-7 gestochen haben

    DI punkte={}, spielerteam={}, spieltyp={1,1};   // spieltyp.a gibt an typ o(sole,wenz,...), spieltyp.b gibt bei wenz und geier an gspuidn schlog o
                                                    // bei solo sauspui de gspuide forb (also bei sauspui imma 1=herz)

    SOCKET sock1, sock2/*, sock3*/;
    int ready, sock_max, max=-1;
    int client_sock[3];          // int client_sock[FD_SETSIZE]; FD_SETSIZE = maximale anzahl von deskriptoren pro prozess und ist abhängig vom betriebssystem
    fd_set gesamt_sock, lese_sock;
    char *buffer = (char*) malloc (BUF);/*, *buffer2 = (char*) malloc (BUF), *buffer3 = (char*) malloc (BUF)*/
    WORD winsockversion;
    WSADATA wsadata;
    struct sockaddr_in server, client;
    int len=0;

    /*
    for(i=0;i<8;i++){
        for(j=0;j<5;j++){
            stiche[i][j]=NULL;
            if(stiche[i][j]==NULL){
            }
        }
    }
    */

    /*
    time_t stempel;                                         // zeit auslesen vielleicht brauchd mas no
    struct tm *currentTime = localtime ( &stempel );
    int sec  = currentTime -> tm_sec;
    int min  = currentTime -> tm_min;
    int hour = currentTime -> tm_hour;
    printf("%d\n",hour);
    */
    winsockversion = MAKEWORD(2,0);                          // word für winsockverion 2,0 kreiren
    if (WSAStartup(winsockversion, &wsadata)!= 0){          // winsock initialisieren und auf fehler abfragen -1
        printf("fehler beim initialisieren von winsock");
        exit (0);
    }
    else{
        printf("winsock initialisiert\n\n");
    }

    sock_max = sock1 = socket(AF_INET, SOCK_STREAM, 0);     // socket anlegen rückgabewert ist ein socketdescriptor
                                                            // af_inat=ipv4, sock_stream=übertragungsart tcp, 0=standardprotokoll tcp
    atexit(cleanup);                                        // beim beenden des programms sockets freigeben



    memset( &server, 0, sizeof (server));                   // server auf 0 setzn
    server.sin_family = AF_INET;
    server.sin_port = htons (PORT);                         // port für socket festlegen; htons=host to network short,  umwandlung in network byte order
    server.sin_addr.s_addr = htonl( INADDR_ANY );           // beliebige ip-addressen annehmen testen !!!; htnl=host to network long

    if (bind(sock1, (struct sockaddr*) &server, sizeof(server)) <0){    // socket festlegen
        printf("fehler beim bind");
    }


    if (listen( sock1,3)==-1){            // wartet auf verbindungswünsche von clients 3 gibt die max anzahl an verbinungen in der schlange an
        printf("fehler beim listen");
    }
    len = sizeof(client);


    for( i=0; i<3; i++){
        client_sock[i] = -1;
    }

    FD_ZERO(&gesamt_sock);                // alle deskriptoren im gesamt_sock initialisieren
    FD_SET(sock1, &gesamt_sock);          // sock1 dem gesamt_sock hinzufügen

    while(1){
    lese_sock = gesamt_sock;
    while(client_sock[2]<0){
        ready = select( sock_max+1, &lese_sock, NULL, NULL, NULL );
        if( FD_ISSET(sock1, &lese_sock)) {
            sock2 = accept( sock1, (struct sockaddr*) &client, &len );
            if (sock2 == INVALID_SOCKET){
            printf("fehler beim accept");
            printf("%d",sock2);
            WSAGetLastError();
            }
            else{
                printf("verbunden\n\n");
            }
            /* freien Platz für (Socket-)Deskriptor
            * in client_sock suchen und vergeben */
            for( i=0; i< 3; i++){
                if(client_sock[i] < 0) {
					//printf("i: %d\n",i);
                    client_sock[i] = sock2;
					//printf("socknr: %d\n",client_sock[i]);
                    //system("pause");
                    break;
                }
            }
            FD_SET(sock2, &gesamt_sock);                        /* den neuen (Socket-)Deskriptor zur
                                                                 * (Gesamt)Menge hinzufügen */

            if( sock2 > sock_max ){                             /* select() benötigt die höchste*/
                sock_max = sock2;                               /* (Socket-)Deskriptor-Nummer. */
            }

            if( i > max ){                                      /* höchster Index für client_sock*/
                max = i;                                        /* für die anschließende Schleife benötigt */
            }
        }
    }

    spielanfang_ausspieler = 0;
    aktueller_ausspieler = 0;

    for(j=0;j<4;j++){

        sendenschleife(client_sock, "cls", 0);
        sendenschleife(client_sock, "\n\n\n\n\n\n\n\n\n\t\t\tGib mir einen Euro\n", 1);
        sendenschleife(client_sock, "cls", 0);
    }

    //Beep(13000,1000);

    for(i=0;i<4;i++){
        if(i!=3){
            sendentext(client_sock, i, "username");
        }
        else{
            system("if %username% == Robert echo Hallo Meister");
            system("if %username% == rkiesbauer echo Hallo Meister");
            system("if %username% == mauer echo Hallo Meister2");
            //system("pause");
            //system("cls");
        }
    }


    //sendenpause(client_sock);

    sendenschleife(client_sock, "\n\n\t\t\t\tSCHOFKOPFA\n\n\t\t     presented by R.K. &&  M.A.\n\n\n\n\n\n\n\n\n\n", 1);
    sendenpause(client_sock);
    //sendenschleife(client_sock, "cls", 0);

    do{
        do{
            mischen_austeilen (alle_spieler,karten,vergeben);
            karten_sortieren(alle_spieler,karten,spieltyp);


            gespielt='n';
            spielerteam.a=spielanfang_ausspieler;
            k=0;
            spielerteam.a=spielerteam.a-1;
            do
            {
                spielerteam.a++;
                do
                {
                    //sendenschleife(client_sock, "cls", 0);

                    sprintf(buffer, "\n\nSpieler%d: nicht Spielbare Karten kennzeichnen? (j/n)", spielerteam.a+1);
                    sendentext(client_sock, spielerteam.a, buffer);

                    anf = eingaben(client_sock, "eingabechar", spielerteam.a);
                    /*if(spielerteam.a<3){
                        sendentext(&client_sock[spielerteam.a], "eingabechar", spielerteam.a);
                        anf = empfangentext(client_sock[spielerteam.a]);
                    }
                    else{
                        scanf("%c", &anf);
                        fflush(stdin);
                    }*/

                    if(anf!='n' && anf!='N' && anf!='j' && anf!='J'){

                        sendentext(client_sock, spielerteam.a, "do mechd i da extra heifa und dia s'lebn einfacher macha");
                        sendentext(client_sock, spielerteam.a, "und du mechsd mas damit danga dasd wosd eigibsd wos i da ned vorgebn hob");
                        sendentext(client_sock, spielerteam.a, "kannsd gern versuacha oba i loss di ned weida mocha bisd ned \"j\" oda \"n\" eigebn hosd");
                        sendentext(client_sock, spielerteam.a, "bisd ned \"j\" oda \"n\" eigebn hosd\n");
                        sendentext(client_sock, spielerteam.a, "also machs gscheid oda du wirsd von deine mitspieler gschlogn");
                    }

                    if(anf=='j' || anf=='J')
                    {
                        anfaenger[spielerteam.a]=1;
                    }

                }while(anf!='n' && anf!='N' && anf!='j' && anf!='J');

            }while(spielerteam.a<3);


            while(gespielt!='j' && k<4){
                sendenschleife(client_sock, "cls", 0);
                anzeigenKartenAllenSpielern(client_sock, alle_spieler);
                do{
                    spielerteam.a++;
                    if(spielerteam.a==4){spielerteam.a=0;}

                    //anzeigen_karten_spieler(alle_spieler[spielerteam.a]);
                    sprintf(buffer, "\nspieler%d spielst du?(j/n)", spielerteam.a+1);
                    sendentext(client_sock, spielerteam.a, buffer);
					gespielt = eingaben(client_sock, "eingabechar", spielerteam.a);
                    //printf("\n\n");
                    //scanf("%c",&gespielt);
                    //fflush( stdin );
                    if(gespielt !='j' && gespielt != 'n'){
                        sendentext(client_sock, spielerteam.a, "bitte nur j oder n eingeben wenn is da scho vorgib ");
                        sendentext(client_sock, spielerteam.a, "und überleg da vorher obst ned vielleicht gsperrt bisd ");
                        sendentext(client_sock, spielerteam.a, "\noda bisd du vielleicht oana der ned lesn kann dann ");
                        sendentext(client_sock, spielerteam.a, "frog i mi allerdings warum du a textbasierts schofkopfa ");
                        sendentext(client_sock, spielerteam.a, "spuin maechsd\n\n");
                        sendenpauseeinzel(client_sock, spielerteam.a);
                        //eingaben(&client_sock[spielerteam.a], "eingabechar", spielerteam.a);
                        spielerteam.a--;
                    }
                }while(gespielt !='j' && gespielt != 'n');

                k++;
            }

            if(gespielt!='j'){
                sendenschleife(client_sock, "\n\nkeiner spielt es wird neu gemischt\n\n", 1);
                sendenpause(client_sock);
                spielanfang_ausspieler++;
                if (spielanfang_ausspieler == 4){
                    spielanfang_ausspieler = 0;
                }
            }

        }while(gespielt!='j');
        do
        {

            sendentext(client_sock, spielerteam.a, "cls");

            sendentext(client_sock, spielerteam.a, "was spielsd du\n\n");
            sendentext(client_sock, spielerteam.a, "1 fuer sauspui\n");
            sendentext(client_sock, spielerteam.a, "2 fuer solo \n");
            sendentext(client_sock, spielerteam.a, "3 fuer wenz \n");
            sendentext(client_sock, spielerteam.a, "4 fuer geier \n");
            sendentext(client_sock, spielerteam.a, "5 fuer sie gibs auf du hoassd koa sie und wenn glaub is da ned\nbech ghabt\n");
            fflush( stdin );
            spieltyp.a = (int)(eingaben(client_sock, "eingabeint", spielerteam.a));
            //printf("\n\t%d\n", spieltyp.a);

            if(spieltyp.a==5){sendentext(client_sock, spielerteam.a,"blurg");sendenpauseeinzel(&client_sock[spielerteam.a], spielerteam.a);}
            if(spieltyp.a<1 || spieltyp.a>4){
                sendentext(client_sock, spielerteam.a, "farbwenz oda farbgeier gibs einfach ned und wos andas als des wos om stehd mog i ned spuin");
                sendentext(client_sock, spielerteam.a, "also gib wos gscheids ei oda i ruaf bei da sonderschui das da an blotz freihoidn soin weilsd ned a richtige zahl eitippn kannsd");
                sendenpauseeinzel(client_sock,spielerteam.a);
            }
        }while(spieltyp.a<1 || spieltyp.a>4);

        switch (spieltyp.a){
            case 1: spieltyp=sauabfragefkt(alle_spieler, karten, spielerteam, spielanfang_ausspieler, client_sock);
                    if(spieltyp.a==-1)
                    {
                        spielanfang_ausspieler=spieltyp.b;
                    }
                    else{
                        spielerteam.b = mitspieler_bestimmen(spielerteam, alle_spieler, spieltyp.b, karten);
                        gespielte_sau=spieltyp.b; spieltyp.b=1;   //di spieltyp moi kurz missbrauchd umd sau zum übergebn dannoch and richtige variable übergebn und forb eigsetzt (herz)
                        davolaufa=davolaufabestfkt(alle_spieler[spielerteam.b],karten,gespielte_sau,client_sock);
                    };
                    break;

            case 2: spieltyp=solofarbabfragefkt(alle_spieler, karten, spielanfang_ausspieler, spielerteam, client_sock); break;
            case 3: spieltyp.a=3;spieltyp.b=3; break;
            case 4: spieltyp.a=4;spieltyp.b=4; break;
        }

    }while(spieltyp.a==-1);

    if(spieltyp.a != 1){
        spielerteam.b=spielerteam.a;
    }
    fflush(stdin);
    sscanf("%c",&bla);
    sendenschleife(client_sock, "cls", 0);
    //printf("(spieler%d spielt mit spieler%d)\n\n",spielerteam.a+1,spielerteam.b+1);
    aktueller_ausspieler = spielanfang_ausspieler;
     karten_sortieren(alle_spieler,karten,spieltyp);

    for(runde=0;runde<8;runde++){
        aktueller_ausspieler = rundespielen (alle_spieler, karten, stiche, aktueller_ausspieler, runde, spielerteam, gespielte_sau, spieltyp, davolaufa, anfaenger, client_sock);
        rrsp[runde]=aktueller_ausspieler;
    }

    for(runde=0;runde<8;runde++){
        sprintf(buffer,"runde %d\n",runde+1);
        sendenschleife(client_sock,buffer,0);
        for (i=0;i<4;i++){
            if(stiche[runde][i]!=NULL){
                sprintf(buffer,"karte %d: %15s\t",i+1,stiche[runde][i]);
                sendenschleife(client_sock,buffer,0);
                if(runde==0){
                    if(spielanfang_ausspieler+i<4){
                        printf("von spieler%d\n", spielanfang_ausspieler+1+i);
                    }
                    else{
                        printf("von spieler%d\n", spielanfang_ausspieler-3+i);
                    }
                }
                else{
                    if(rrsp[runde]+i<4){
                        printf("von spieler%d\n", rrsp[runde]+1+i);
                    }
                    else{
                        printf("von spieler%d\n", rrsp[runde]-3+i);
                    }
                }
            }

        }
        printf("gestochen hat spieler%d mit der karte %s\n\n",(rrsp[runde])+1,stiche[runde][4]);
    }

     punkte=punktezaehlen(spielerteam, stiche, karten, rrsp);
     if(punkte.a<punkte.b){
         sprintf(buffer,"oooohhh s`spielerteam hod verlorn %d zu %d \n",punkte.a,punkte.b);
         sendenschleife(client_sock,buffer,0);
         sendenschleife(client_sock,"wenn mas ned kann dann soid mas a bleibn lossn",0);
     }
     else{
         if(punkte.a==punkte.b){
             sendenschleife(client_sock,"o mei o mei a unentschieden (60:60) wos sandn des für spieler",0);
         }
         else{
             if(punkte.a==120){sendenschleife(client_sock,"jawoi schworz so mog i des endlich moi oi des wert sand mi zum nutzn\n",0);
                 sendenschleife(client_sock,"hoffentlich wars koa ramsch den den kann i ned",0);
             }
             else{
                 sendenschleife(client_sock,"a sieg is des mindesteste wos ma von am gscheidn spieler erwarten kann\n",0);
                 sprintf(buffer,"a %d zu %d sieg",punkte.a,punkte.b);
                 sendenschleife(client_sock,buffer,0);
             }
         }
     }
    /* printf("\n\nspieler %d, %d gegen de andan\n",spielerteam.a+1,spielerteam.b+1);


    printf("\n\n");
    system("pause");
    */    free(buffer);
    printf("Hello world!\n");
    scanf("%c",&bla);
    return 0;
    }
}

DI sauabfragefkt(char *alle_spieler[4][8], char karten[4][8][16], DI spielerteam, int spielanfang_ausspieler, int *client_sock)
{
  int l;
  DI sau;
    char *buffer = (char*) malloc (BUF);
        l=0;
        do{
            do{
                sendentext(client_sock,spielerteam.a,"cls");
                sprintf(buffer,"\nspieler%d mit wem spielst du?\n",spielerteam.a+1);
                anzeigenKartenSpielerNetzwerk(client_sock,alle_spieler,spielerteam.a);
                sendentext(client_sock,spielerteam.a,"\n(1) fuer \"i spui mit da hundsgficktn\"");
                sendentext(client_sock,spielerteam.a,"\n(2) fuer \"i spui mit da blaun\"");
                sendentext(client_sock,spielerteam.a,"\n(3) fuer \"i spui mit da oidn\"\n\n");


                sau.b=(int)(eingaben(client_sock,"eingabeint",spielerteam.a));
                fflush( stdin );
                if(sau.b<1 || sau.b>3){
                    sendentext(client_sock,spielerteam.a,"i hobs doch higschriebn 1, 2 oda 3 und du maechsd de lila sau oda wia? \n\n\n\t\t\t\tNOMOI!!!!\n\n\n\n");
                    sendenpauseeinzel(client_sock,spielerteam.a);
                }
            }while(sau.b<1 || sau.b>3);

            spielerteam.b=0;
            spielerteam.b = mitspieler_bestimmen(spielerteam, alle_spieler, sau.b, karten);
            if(spielerteam.b==-1){
                sendentext(client_sock,spielerteam.a,"man kann blos mit na sau spuin wosd a a forb hosd! bass bessa auf\n");
                sendenpauseeinzel(client_sock,spielerteam.a);            }
            else{
                if(spielerteam.a==spielerteam.b){
                    sendentext(client_sock,spielerteam.a,"solo headsd vorher auswein miasn du depp\n");
                    sendenpauseeinzel(client_sock,spielerteam.a);
                }
            }
            l++;
            if (l==4){
                spielanfang_ausspieler++;
                if (spielanfang_ausspieler == 4){
                    spielanfang_ausspieler = 0;
                }

                    system("cls");
                    sendentext(client_sock,spielerteam.a,"also entweder hosd du so a dussl und kriagsd nur trumpf und/oda saun ");
                    sendentext(client_sock,spielerteam.a,"dann frog i mi warum du ned um geid spuisd sondern di mit so am deppardn ");
                    sendentext(client_sock,spielerteam.a,"programm rumschlogsd des ned moi geier no wenz  konn\n\noda du bisd ");
                    sendentext(client_sock,spielerteam.a,"einfach nur a reiner voidepp ders nochm 4. moi imma no ned gschafft hod ");
                    sendentext(client_sock,spielerteam.a,"de richtige sau zum spuin eizumgebn\nin dem foi frog i mi warum mei programmiera ");
                    sendentext(client_sock,spielerteam.a,"mi an die eigentlich weida gebn hod");
                    sendentext(client_sock,spielerteam.a,"\n\n\nes wird nei gmischd\n\n");
                    system("pause");
                    sau.a=-1; sau.b=spielanfang_ausspieler;
                    return sau;

            }
            system("cls");

        }while((spielerteam.a==spielerteam.b || spielerteam.b==-1) && l<4);
        sau.a=1;        free(buffer);
        return sau;
}

DI solofarbabfragefkt(char *alle_spieler[4][8], char karten[4][8][16], int spielanfang_ausspieler, DI spielerteam, int *client_sock)
{
    int l=0;
    DI spieltyp;
    char *buffer = (char*) malloc (BUF);
    spieltyp.a=2;

    do{

        sendentext(client_sock,spielerteam.a,"cls");
        sprintf(buffer,"\nspieler%d wos fia a Solo spielst du?\n",spielerteam.a+1);
        sendentext(client_sock,spielerteam.a,buffer);
        anzeigenKartenSpielerNetzwerk(client_sock,alle_spieler,spielerteam.a);
        sendentext(client_sock,spielerteam.a,"\n(1) fuer \"a normals(herz)\"");
        sendentext(client_sock,spielerteam.a,"\n(2) fuer \"a schein Solo\"");
        sendentext(client_sock,spielerteam.a,"\n(3) fuer \"a gross Solo\"");
        sendentext(client_sock,spielerteam.a,"\n(4) fuer \"a oache Solo\"\n\n");
        spieltyp.b=(int)(eingaben(client_sock,"eingabeint",spielerteam.a));
        fflush(stdin);
        if(spieltyp.b <1 || spieltyp.b > 4)
            {
                sendentext(client_sock,spielerteam.a,"\nDuad ma leid aba beim Schofkopfa gibds grod 4 Farbn\n");
                sendentext(client_sock,spielerteam.a,"Und du speisd mid oana fo dene!\nhosd mi");
                sendenpauseeinzel(client_sock,spielerteam.a);
            }
        sendentext(client_sock,spielerteam.a,"cls");
        l++;
        if (l==4){
            spielanfang_ausspieler++;
            if (spielanfang_ausspieler == 4){
                spielanfang_ausspieler = 0;
            }

                sendenschleife(client_sock,"cls",0);
                sendentext(client_sock,spielerteam.a,"wennsd zbled bisd um di fuer a forb zum entscheidn dann bisd seiba schuid");
                sendentext(client_sock,spielerteam.a,"\n\n\nes wird nei gmischd\n\n");
                sendenpauseeinzel(client_sock,spielerteam.a);
                spieltyp.a=-1; spieltyp.b=spielanfang_ausspieler;
                return spieltyp;
            }

    }while(spieltyp.b <1 || spieltyp.b > 4);

    switch(spieltyp.b)
            {
                case 1: spieltyp.b=1;break;
                case 2: spieltyp.b=0;break;
                case 3: spieltyp.b=2;break;
                case 4: spieltyp.b=3;break;
            }    free(buffer);
    return spieltyp;

}



/*    if(spielerteam.a>60)
    {
            if(spielerteam.a>=120)
            {
                printf("\nDa Spieler hod eich schwarz abzong");
            }
            else
            {
                if(spielerteam.a>=90)
                {
                    printf("\nDA Spieler gwingd");
                    printf("\nund seine Gegner hand Schneider");
                }
                else
                {
                    printf("\nDa Spieler hod gwunga");
                }
            }
    }
    else
    {
        if(spielerteam.a==60)
        {
                printf("\nPunktgleichheit: Da spieler verliert");
        }
        else
        if(spielerteam.a<60)
        {
            if(spielerteam.a==0)
            {
                printf("\nDes war moi überhaupt nix");
                printf("\nschwarz bisd");
            }
            else
            {
                printf("\nDa Spieler hod de Rundn woi verlorn");
            }
        }
    }
}
*/
DI punktezaehlen(DI spielerteam, char *stiche[8][5], char karten[4][8][16], int rrsp[8]){

        int runde,i, rundenpunkte;
        DI punkte={};
        for(runde=0;runde<8;runde++){
            rundenpunkte=0;
            for(i=0;i<4;i++){
                switch ((kartenposbest(stiche[runde][i], karten)).b){
                    case 0:
                    case 1:
                    case 2:break;
                    case 3:rundenpunkte +=2;break;
                    case 4:rundenpunkte +=3;break;
                    case 5:rundenpunkte +=4;break;
                    case 6:rundenpunkte +=10;break;
                    case 7:rundenpunkte +=11;break;
                }
            }
                if(rrsp[runde]==spielerteam.a || rrsp[runde]==spielerteam.b){
                    punkte.a+=rundenpunkte;
                }
                else{
                    punkte.b+=rundenpunkte;
                }
        }
        return punkte;
}

void mischen_austeilen (char *alle_spieler[4][8], char karten[4][8][16], int vergeben[4][8]){

    int i=0, j=0, farbe=0, schlag=0;

    srand( time(0));        //Zufahlszahl mithilfe der systemzeit initialisieren
    for(i=0;i<4;i++){       // spielerarray initiallisieren
        for(j=0;j<8;j++){
            vergeben[i][j]=0;
        }
    }
    for (j=0;j<4;j++){
        for (i=0;i<8;i++){
            schlag = rand()%8;
            farbe = rand()%4 ;
            while (vergeben[farbe][schlag]==1){
                schlag = rand()%8;
                farbe = rand()%4;
            }
            alle_spieler[j][i]=karten[farbe][schlag];
            vergeben[farbe][schlag]=1;
        }
    }
}

void anzeigen_karten_spieler (char *alle_spieler[8]){


    int i;
    for(i=0;i<8;i++){
        if(alle_spieler[i]!=NULL){
            printf("%d) %s\n",i+1,alle_spieler[i]);
        }
    }
}

void anzeigen_spielbare_karten_spieler (DI spieltyp,char *gespielte_karte,char karten[4][8][16], int j, int gespielte_farbe, int gespielter_schlag, int aktueller_ausspieler, char *alle_spieler[4][8],
                                        int sau_farbe,int nur_pruefen, int davolaufa, int anfaenger[],int *client_sock){

    int i;
    char *buffer = (char*) malloc (BUF);

    for(i=0;i<8;i++){
        if(alle_spieler[aktueller_ausspieler][i]!=NULL){
            if(richtig_gspeid(spieltyp,alle_spieler[aktueller_ausspieler][i],karten,j,gespielte_farbe,gespielter_schlag,aktueller_ausspieler,
                               alle_spieler,sau_farbe,nur_pruefen,davolaufa,client_sock).a==-1){
                if(anfaenger[aktueller_ausspieler]==1)
                {
                    sprintf(buffer,"X %d) %s",i+1,alle_spieler[aktueller_ausspieler][i]);
                    sendentext(client_sock,aktueller_ausspieler, buffer);
                }

            }
            else
            {
                sprintf(buffer,"%d) %s",i+1,alle_spieler[aktueller_ausspieler][i]);
                sendentext(client_sock,aktueller_ausspieler,buffer);
            }

        }
    }    free(buffer);
}

int mitspieler_bestimmen(DI spielerteam, char *alle_spieler[4][8], int gespielte_sau,char karten[4][8][16]){

    char *gesuchte_sau;
    int i=0,j=0, hodaned=1;
    DI pos={};

    switch(gespielte_sau){
    case 1: gesuchte_sau=karten[0][7];j=0;break;
    case 2: gesuchte_sau=karten[2][7];j=2;break;
    case 3: gesuchte_sau=karten[3][7];j=3;break;
    }

    for(i=0;i<8;i++){
        pos=kartenposbest(alle_spieler[spielerteam.a][i],karten);
        if(j==pos.a && pos.b!=3 && pos.b!=4){
            hodaned=0;
        }
    }
    if(hodaned==1){
        return -1;
    }

    pos=spielerposbest(gesuchte_sau, alle_spieler);
    return pos.a;
}

DI spielerposbest(char* karte, char* alle_spieler[4][8]){
    int i=0, j=0;
    DI pos={};
            i=-1;
            do{
                i++;
                j=0;
                while(j<8 && karte!=alle_spieler[i][j]){
                    j++;
                }
                if(j==8){
                    j=0;
                }
            }while(i<4 && karte!=alle_spieler[i][j]);
            if(karte!=alle_spieler[i][j]){
                printf("ebba hod de kortn klaud! wer wars?");
                pos.a=-1; pos.b=-1; return pos;
            }
    pos.a=i;
    pos.b=j;
    return pos;
}

int rundespielen (char *alle_spieler[4][8], char karten[4][8][16], char *stiche[8][5], int aktueller_ausspieler, int runde, DI spielerteam, int gespielte_sau, DI spieltyp, int davolaufa, int anfaenger[], int *clientsock){

    int i=0 , j=0, karte=0, neu_ausspieler=0, nur_pruefen=0;
    int gespielte_farbe=0,gespielter_schlag=0, sau_farbe; //ma
    char *hoechste, *neugespielte;
    DI gespielte_karte;
    hoechste = NULL;
    char *buffer = (char*) malloc (BUF);

    for(j=0;j<4;j++){

        do{

            switch(spieltyp.a)
            {
                case 1:
                    sprintf(buffer,"spieler%d spielt mit da ",spielerteam.a+1);
                    sendenschleife(clientsock,buffer,1);
                    switch(gespielte_sau){
                        case 1: sendenschleife(clientsock,"hundsgficktn\n\n\n", 1); sau_farbe=0; break;
                        case 2: sendenschleife(clientsock,"blaun\n\n\n",1); sau_farbe=2; break;
                        case 3: sendenschleife(clientsock,"oidn\n\n\n",1); sau_farbe=3; break;
                    }
                    break;

                case 2:
                    sprintf(buffer,"spieler%d spielt ein ",spielerteam.a+1);
                    sendenschleife(clientsock,buffer,1);
                    switch(spieltyp.b){
                        case 0: sendenschleife(clientsock, "schellen-solo\n\n\n",1);break;
                        case 1: sendenschleife(clientsock, "herz-solo\n\n\n",1);break;
                        case 2: sendenschleife(clientsock, "grass-solo\n\n\n", 1);break;
                        case 3: sendenschleife(clientsock, "eichel-solo\n\n\n", 1);break;
                    }
                    break;

                case 3:
                    sprintf(buffer,"spieler%d spielt einen wenz",spielerteam.a+1);
                    sendenschleife(clientsock,buffer,1);
                    break;

                case 4:
                    sprintf(buffer,"spieler%d spielt einen geier ",spielerteam.a+1);
                    sendenschleife(clientsock,buffer,1);
                    break;
            }


            sprintf(buffer,"\n\nspieler%d was spielsd du aus?\n\n",aktueller_ausspieler+1);
            sendentext(clientsock,aktueller_ausspieler,buffer);
            sendenschleife(clientsock,"---------------------------\n",1);
            sendenschleife(clientsock,"bis jetzt gespielte karten:\n\n",1);

            for (i=0;i<4;i++){
                if(stiche[runde][i]!=NULL){
                    sprintf(buffer,"%s\n",stiche[runde][i]);                    sendenschleife(clientsock,buffer,1);
                }
            }

            sendenschleife(clientsock,"---------------------------\n\n",1);            for(i=0;i<4;i++){                sprintf(buffer,"spieler%d deine karten\n", i+1);                sendentext(clientsock,i,buffer);                if(i==aktueller_ausspieler && anfaenger[aktueller_ausspieler]==1){
                    nur_pruefen=1;                    sprintf(buffer,"(X: gesperrte karten)\n\n");
                    sendentext(clientsock,aktueller_ausspieler,buffer);                    anzeigen_spielbare_karten_spieler(spieltyp,alle_spieler[aktueller_ausspieler][karte],karten, j,gespielte_farbe,gespielter_schlag,                                              aktueller_ausspieler, alle_spieler, sau_farbe, nur_pruefen, davolaufa, anfaenger, clientsock);
                }                else{                    anzeigenKartenSpielerNetzwerk(clientsock, alle_spieler, i);                }            }

            karte=0;

            karte= (int)eingaben(clientsock, "eingabeint",aktueller_ausspieler);
                                //lol bei eingabe "1 1 1 1 2 2 2 2 3 3 3 3 4 4 4 4 5 5 5 5 6 6 6 6 7 7 7 7 8 8 8 8" :D ohne fflush
                                // de eingabe gehd e nimma wega der scheiß zugabeabfrage und durchs netzwerk erst recht ned

            karte--;
            if(karte<0 || karte>7 || alle_spieler[aktueller_ausspieler][karte]==NULL){
                sendentext(clientsock,aktueller_ausspieler,"de Kortn hosd scho gspuid oda de gibs ned");
                sendentext(clientsock,aktueller_ausspieler," spui a andre du Depp\n\n");
                sendenpauseeinzel(clientsock,aktueller_ausspieler);
                sendentext(clientsock,aktueller_ausspieler,"cls");
            }
            //ma

            if(karte>=0 && karte<=7 && alle_spieler[aktueller_ausspieler][karte]!=NULL)
            {
            nur_pruefen=0;
            gespielte_karte=richtig_gspeid(spieltyp,alle_spieler[aktueller_ausspieler][karte],karten, j,gespielte_farbe,gespielter_schlag, aktueller_ausspieler,
                                           alle_spieler, sau_farbe, nur_pruefen, davolaufa, clientsock);
            //ma
            }
            if(j==0 && gespielte_karte.a!=-1)
            {
                gespielte_farbe=gespielte_karte.a;
                gespielter_schlag=gespielte_karte.b;
            }
        }while(karte<0 || karte>7 || alle_spieler[aktueller_ausspieler][karte]==NULL || gespielte_karte.a==-1);


        stiche[runde][j]=alle_spieler[aktueller_ausspieler][karte];
        neugespielte=alle_spieler[aktueller_ausspieler][karte];
        alle_spieler[aktueller_ausspieler][karte]=NULL;

        hoechste = rechtbestimmen(hoechste, neugespielte, karten, spieltyp);
        if(neugespielte==hoechste){
            neu_ausspieler=aktueller_ausspieler;
        }

        aktueller_ausspieler++;
        if (aktueller_ausspieler==4){
            aktueller_ausspieler=0;
        }
        sendenschleife(clientsock,"cls",0);
    }
    stiche[runde][4]=hoechste;

    free(buffer);
    return neu_ausspieler;

}

DI kartenposbest(char* karte, char karten[4][8][16]){

    DI pos={};          //ma

    for(pos.a=0;pos.a<4;pos.a++)
        {
            for(pos.b=0;pos.b<8;pos.b++)
                {
                    if (karte!=NULL && karte==karten[pos.a][pos.b])
                        {
                            return pos;                                                // farb und schlog wird zruggem
                        }
                }
        }
        pos.a=-1;
        pos.b=-1;
  return pos;
}

char* rechtbestimmen(char* hoechste, char* neugespielte, char karten[4][8][16], DI spieltyp){

    int i=0, j=0, l=0, m=0, farbe=spieltyp.b;
    DI pos;

        if(neugespielte==NULL){
            printf("karte schon gespielt");
            return hoechste;
        }
        if(hoechste==NULL){
            return neugespielte;
        }
        else{
            pos=kartenposbest(hoechste, karten);
            l=pos.a;
            m=pos.b;

            pos=kartenposbest(neugespielte, karten);
            i=pos.a;
            j=pos.b;

/*  1. fall vorher farbe nachher gleiche höhere farbe

    2. fall vorher farbe nachher gleiche niedrigere farbe

    3. fall vorher farbe nachher andere farbe

    4. fall vorher farbe nachher trumpf

    5. fall vorher trumpf nachher farbe

    6. fall vorher trumpf nachher höherer trumpf
        vorher ober nachher höherer ober
        vorher unter nachher ober, höherer unter
        vorher herz nachher ober, unter, höheres herz

    7. fall vorher trumpf nachher niedrigerer trumpf
        vorher ober nachher niedriger ober, kein ober
        vorher unter nachher niedriger unter, herz
*/    if(spieltyp.a == 1 || spieltyp.a==2)
        {                                       //  herz == trumpffarbe
            if (l!=farbe && m!=3 && m!=4){      //  vorher farbe
                if(l==i){                   //  vorher und nachher gleiche farbe
                    if(j!=3 && j!=4){       //  nachher kein trumpf
                        if(j>m){            //  nachher höhere farbe
                            return neugespielte;
                        }
                        else{
                            return hoechste;    //  nachher niedrigere farbe
                        }
                    }
                    else{                   //  nachher trumpf
                        return neugespielte;
                    }
                }
                else{
                    if(i!=farbe && j!=3 && j!=4){   //  nachher kein trumpf
                        return hoechste;        //  vorher und nachher unterschiedliche farbe
                    }
                    else{                       // vorher farbe nachher trumpf
                        return neugespielte;
                    }
                }
            }
            else{                           //  vorher trumpf
                if(i!=farbe && j!=3 && j!=4){   //  nachher kein trumpf
                    return hoechste;
                }                           //  fälle beide trumpf
                else{                       //  vorher und nachher trumpf
                    if(m==4){               //  vorher ober
                        if(j!=4){           //  nachher kein ober
                            return hoechste;
                        }
                        else{               //  vorher ober nachher ober
                            if(i>l){        //  nachher ober höher
                                return neugespielte;
                            }
                            else{           //  vorher ober höher
                                return hoechste;
                            }
                        }
                    }
                    else{
                        if(m==3){               //  vorher unter
                            if(j==4){           //  vorher unter nachher ober
                                return neugespielte;
                            }
                            else{
                                if(j==3){       //  vorher unter nachher unter
                                    if(i>l){    //  unter nachher höher
                                        return neugespielte;
                                    }
                                    else{       //  unter vorher höher
                                        return hoechste;
                                    }
                                }
                                else{           //  vorher unter nachher herz oder niedriger
                                    //if(i==1){
                                        return hoechste;
                                    //}
                                }
                            }
                        }
                        else{                   // vorher herz
                            if(j==3 || j==4){           //  vorher herz nachher höher(unter,ober)
                                return neugespielte;
                            }
                            else{
                                if(i==farbe){               //  vorher herz nachher herz
                                    if(j>m){            //  herz vorher niedriger (nachher inklusive ober und unter)
                                        return neugespielte;
                                    }
                                    else{           //  herz nachher niedriger
                                        return hoechste;
                                    }
                                }
                                else{
                                    return hoechste;
                                }
                            }
                        }
                    }
                }
            }
        }

        else
        {
            if(m==spieltyp.b)
            {
                if(j==spieltyp.b)
                {
                    if(i>l)         //i is nei
                    {
                        return neugespielte;
                    }
                    else
                    {
                        return hoechste;
                    }

                }
                else
                {
                    return hoechste;
                }
            }
            else
            {
                if(j==spieltyp.b)
                {
                    return neugespielte;
                }
                else
                {
                    if(l==i)
                    {
                        if(m>j)
                        {
                            return hoechste;
                        }
                        else
                        {
                            return neugespielte;
                        }
                    }
                    else
                    {
                        return hoechste;
                    }
                }
            }
        }
    printf("fall nicht abgefangen\nfehler\n");
    return hoechste;
}
}

void karten_sortieren(char *alle_spieler[4][8],char karten[4][8][16], DI spieltyp){

    int tausch=0,i=0,j=0;
    char *hoehere;


    for(i=0;i<4;i++){
        tausch=1;
        while (tausch==1){
            tausch=0;
            for(j=0;j<7;j++){
                hoehere = rechtbestimmen(alle_spieler[i][j], alle_spieler[i][j+1], karten, spieltyp);
                if(hoehere==alle_spieler[i][j+1]){
                    alle_spieler[i][j+1] = alle_spieler[i][j];
                    alle_spieler[i][j] = hoehere;
                    tausch=1;
                }
            }
        }
    }

    for(i=0;i<4;i++){
        tausch=1;
        while (tausch==1){
            tausch=0;
            for(j=0;j<7;j++){
                if(kartenposbest(alle_spieler[i][j],karten).a != spieltyp.b && kartenposbest(alle_spieler[i][j],karten).b !=3 && kartenposbest(alle_spieler[i][j],karten).b!=4){
                    if (kartenposbest(alle_spieler[i][j],karten).a < kartenposbest(alle_spieler[i][j+1],karten).a){
                        hoehere=alle_spieler[i][j+1];
                        alle_spieler[i][j+1]=alle_spieler[i][j];
                        alle_spieler[i][j]=hoehere;
                        tausch=1;
                    }
                    else{
                        if(kartenposbest(alle_spieler[i][j],karten).a == kartenposbest(alle_spieler[i][j+1],karten).a){
                            if(kartenposbest(alle_spieler[i][j],karten).b < kartenposbest(alle_spieler[i][j+ 1],karten).b){
                                hoehere=alle_spieler[i][j+1];
                                alle_spieler[i][j+1]=alle_spieler[i][j];
                                alle_spieler[i][j]=hoehere;
                                tausch=1;
                            }
                        }
                    }
                }
            }
        }
    }

}

DI richtig_gspeid(DI spieltyp,char *gespielte_karte,char karten[4][8][16], int j, int gespielte_farbe, int gespielter_schlag, int aktueller_ausspieler,
                  char *alle_spieler[4][8], int sau_farbe, int nur_pruefen, int davolaufa, int *clientsock)
{
    int i,fehler=0, trumpf_farbe=1;
    DI karte;
    int farbe,trumpf, farbe_hand, spielerfarbe=0, sau_zugeben=0,trumpf_hand=0, hod_sau=0;

    farbe=kartenposbest(gespielte_karte, karten).a;                                             //lesd de nochgspuide forb und schlog aus
    trumpf=kartenposbest(gespielte_karte,karten).b;

    if(j==0)
    {
        gespielte_farbe=kartenposbest(gespielte_karte,karten).a;                            // gibd de gspeide farb zrug
        gespielter_schlag=kartenposbest(gespielte_karte,karten).b;                          // bzw. lesd farb und schlog vom ausspieler aus
    }

    if(spieltyp.a==1 || spieltyp.a==2){

        trumpf_farbe = spieltyp.b;


        if(j==0)
        {

            if(spieltyp.a==1){
                for(i=0;i<8;i++){
                    if(kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).a == sau_farbe &&
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b == 7){

                        hod_sau=1;
                    }
                }
                if(farbe==sau_farbe && trumpf != 7 && davolaufa==0 && hod_sau==1 && trumpf !=3 && trumpf != 4){
                    if(nur_pruefen==0){
                        sendentext(clientsock,aktueller_ausspieler,"\na wennsd da scho ind hosn mochsd,\n");
                        sendentext(clientsock,aktueller_ausspieler,"derfsd beim schofkopfa trotzdem erst davolaufa\n");
                        sendentext(clientsock,aktueller_ausspieler,"wennsd mind. 4 moi d'saufoarb hosd\n\n");
                        sendentext(clientsock,aktueller_ausspieler,"ALSO SEI NED SO A FEIGLING UND SUACH DI SEIBA\n\n");
                        sendentext(clientsock,aktueller_ausspieler,"oda spui a andre foarb wennsd di ned drausd\n\n");
                        sendenpauseeinzel(clientsock,aktueller_ausspieler);
                        sendentext(clientsock,aktueller_ausspieler,"cls");
                    }
                    fehler=1;
                }
            }
        }
        else
        {
            for(i=0;i<8;i++)
            {
                if(gespielte_farbe==kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).a &&
                    kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b!=3 &&
                    kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b!=4)           //schaud ob der spieler de gspeide farb auf da hend hod
                {
                    farbe_hand=1;                                                               //wenn ja wird der schoida farbe_hand umgschoidn
                }
            }
        }

        if(spieltyp.a==1)                                                                       //wenns a sauspui is
        {

            if(j!=0 && farbe_hand==1)                                                           //er ned da ausspieler is und foarb auf da hend hod
            {
                for(i=0;i<8;i++)
                {
                    if(sau_farbe==kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).a &&
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b !=3 &&
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b !=4)
                    {                                                                           //prüft ob erd saufoarb hod
                        spielerfarbe++;                                                         //wenn dann wird de anzahl von der forb zeid
                        if(kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b==7)
                        {
                            sau_zugeben=1;                                                      //wennd sau dabei is wird sau_zuagebn gsetzt
                        }
                    }
                }
            }
        }
        if(j!=0)                                                                                //wenn ned ausspieler
        {
            if(gespielte_farbe==trumpf_farbe || gespielter_schlag==3 || gespielter_schlag==4)   //wenn a  "herz" || a unter || a ober gspeid is
            {
                for(i=0;i<8;i++)
                {
                    if(kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b == 3 ||
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b == 4 ||
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).a == trumpf_farbe)      // wird gschaud ob ober oder unter auf da hend hand
                    {
                        trumpf_hand=1;                                                          //if ja schalter trumpf_hand=1
                    }
                }
            }
        }

        if(spieltyp.a==1)
        {
            if(gespielte_farbe==sau_farbe && sau_zugeben==1 && (trumpf!=7 || farbe!=sau_farbe) && davolaufa!=1 &&
               gespielter_schlag!=3 && gespielter_schlag!=4)
            {
                if(nur_pruefen!=1){
                    sendentext(clientsock,aktueller_ausspieler,"Du muasd dei Sau spein du Depp!");
                    sendentext(clientsock,aktueller_ausspieler,"\nOder glaubsd du dasd du davo laufa kannsd?");
                    sendentext(clientsock,aktueller_ausspieler,"\nKannsd naemlich ned!");
                    sendentext(clientsock,aktueller_ausspieler,"\nIatz machs nomoi gscheid!\n");
                    sendenpauseeinzel(clientsock,aktueller_ausspieler);
                    sendentext(clientsock,aktueller_ausspieler,"cls");
                }
                fehler=1;
            }
        }

        if(fehler!=1){
            fehler=zugegeben(j,gespielte_farbe,trumpf_hand,farbe_hand,farbe,trumpf,gespielter_schlag,spieltyp,trumpf_farbe,nur_pruefen,clientsock,aktueller_ausspieler);
        }
    }
    else
    {


        if(j!=0)
        {
            if(j!=0 && gespielter_schlag==spieltyp.b)
            {
                for(i=0; i<8; i++)
                {
                    if(kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b==spieltyp.b)
                    {
                        trumpf_hand=1;
                    }

                }
            }
            else
            {
                for(i=0;i<8;i++)
                {
                    if(kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).a==gespielte_farbe &&
                       kartenposbest(alle_spieler[aktueller_ausspieler][i],karten).b!=spieltyp.b)
                    {
                        farbe_hand=1;
                    }
                }




            }

           fehler=zugegeben(j,gespielte_farbe,trumpf_hand,farbe_hand,farbe,trumpf,gespielter_schlag,spieltyp,trumpf_farbe,nur_pruefen,clientsock,aktueller_ausspieler);
        }


    }
    if(j==0 && fehler !=1)
    {
        karte.a=gespielte_farbe;
        karte.b=gespielter_schlag;
    }
    else
    {
        if(fehler==1)
        {
            karte.a=-1;
        }
    }

    return karte;
}

int zugegeben(int j,int gespielte_farbe,int trumpf_hand,int farbe_hand,int farbe,int trumpf,int gespielter_schlag, DI spieltyp, int trumpf_farbe,
              int nur_pruefen, int *clientsock, int aktueller_ausspieler)
{

    int wg_trumpf=0,karte=0;

    wg_trumpf=spieltyp.b;

    if(spieltyp.a==1 || spieltyp.a==2)                                                  // wenn sauspui oda solo
    {
        if(j!=0 && gespielte_farbe==trumpf_farbe && (trumpf_hand==1 || farbe_hand==1) && gespielte_farbe!=farbe && trumpf!=3 && trumpf!=4)
        {                                                                               //"herz" is ausgspeid
            if(nur_pruefen!=1){
                sendentext(clientsock,aktueller_ausspieler,"\nbscheisn deama fei ned\n");                                   //weder "herz" noch ober noch unter nochgspeid
                sendentext(clientsock,aktueller_ausspieler,"gib trumpf zua\n\n");                                           //aber auf da hend
                sendenpauseeinzel(clientsock,aktueller_ausspieler);
                sendentext(clientsock,aktueller_ausspieler,"cls");
            }
            karte=1;                                                                    // wird karte auf 1 gsetzt
        }
        if(j!=0 && karte!=1)
        {
            if(gespielter_schlag==3 || gespielter_schlag==4 )                           //ober oder unter ausgspeid
            {
                if(j!=0 && trumpf_hand==1 && farbe!= trumpf_farbe && trumpf!=3 && trumpf!=4)
                {
                    if(nur_pruefen!=1){
                        sendentext(clientsock,aktueller_ausspieler,"\nbscheisn deama fei ned\n");                           //weder "herz" noch ober noch unter nochgspeid
                        sendentext(clientsock,aktueller_ausspieler,"gib trumpf zua\n\n");                                   //aber auf da hend
                        sendenpauseeinzel(clientsock,aktueller_ausspieler);
                        sendentext(clientsock,aktueller_ausspieler,"cls");
                    }
                    karte=1;
                }
            }
        }
        if(karte!=1 && j!=0 && gespielte_farbe!=trumpf_farbe && gespielter_schlag!=3 && gespielter_schlag!=4 && farbe_hand==1) //farb ausgspeid
        {                                                                               //spieler hod farb auf da hend
            if(trumpf == 3 || trumpf == 4)                                              //gibt oba ober oder unter zua
            {
                if(nur_pruefen!=1){
                    sendentext(clientsock,aktueller_ausspieler,"\nbscheisn deama fei ned\n");                               //a farb ned nochgspeid
                    sendentext(clientsock,aktueller_ausspieler,"gib de Farb zua\n\n");
                    sendenpauseeinzel(clientsock,aktueller_ausspieler);
                    sendentext(clientsock,aktueller_ausspieler,"cls");
                }
                karte=1;

            }
            else                                                                        //immer no farb auf da hend
            {
                if(gespielte_farbe!=farbe)                                              //gibt oba de foische farb zua (inklusive trumpfforb)
                {
                    if(nur_pruefen!=1){
                        sendentext(clientsock,aktueller_ausspieler,"\nbscheisn deama fei ned\n");                           //de farb ned nochgspeid
                        sendentext(clientsock,aktueller_ausspieler,"gib de Farb zua\n\n");
                        sendenpauseeinzel(clientsock,aktueller_ausspieler);
                        sendentext(clientsock,aktueller_ausspieler,"cls");
                    }
                    karte=1;
                }
            }
        }
    }
    else
    {
        if (j!=0)
        {
            if(gespielter_schlag==wg_trumpf)
            {
                if(trumpf!=gespielter_schlag && trumpf_hand==1)
                {
                    if(nur_pruefen!=1)
                    {
                        sendentext(clientsock,aktueller_ausspieler,"\nA Wenz wa gspeid und do wa oana in deina Hend \n");
                        sendentext(clientsock,aktueller_ausspieler,"Iatz spei na a");
                        sendenpauseeinzel(clientsock,aktueller_ausspieler);
                        sendentext(clientsock,aktueller_ausspieler,"cls");
                        karte=1;
                    }

                }
            }
            else
            {
                if(farbe_hand==1 && (gespielte_farbe!=farbe || trumpf==wg_trumpf))
                {
                    if(nur_pruefen!=1){
                        sendentext(clientsock,aktueller_ausspieler,"\nbscheisn deama fei ned\n");                           //de farb ned nochgspeid
                        sendentext(clientsock,aktueller_ausspieler,"gib de Farb zua\n\n");
                        sendenpauseeinzel(clientsock,aktueller_ausspieler);
                        sendentext(clientsock,aktueller_ausspieler,"cls");
                    }
                    karte=1;
                }
            }
        }
    }

  return karte;
}

int davolaufabestfkt(char *mitspielerkarten[8],char karten[4][8][16],int gespielte_sau, int* clientsock ){

    int i, sau_farbe, saufarben=0, davolaufa=0;
    DI kartenpos={};

    switch(gespielte_sau){
        case 1:  sau_farbe=0; break;
        case 2:  sau_farbe=2; break;
        case 3:  sau_farbe=3; break;
    }

    for(i=0;i<8;i++){
        kartenpos = kartenposbest(mitspielerkarten[i],karten);
        if(kartenpos.a==sau_farbe && kartenpos.b != 3 && kartenpos.b != 4){
            saufarben++;
        }
    }

    if(saufarben>3){
        davolaufa=1;
    }
    else{
        davolaufa=0;
    }
    return davolaufa;
}

// netzwerkfkts

void cleanup(void){                                 //Cleanup Winsock
    int i;
    //closesocket (sockverb1);
    for(i=0;i<200;i++){
        //printf("%d ",i) ;
        printf("socket %d beenden\n",i);
        closesocket (i);
    }
    WSACleanup();
    printf("Aufraeuumarbeiten erledigt ...\n");
    system("pause");
}

void error_exit(char *error_message) {              // bei fehler meldung
    fprintf(stderr,"%s: %d\n", error_message, WSAGetLastError());
    exit(EXIT_FAILURE);
}

void senden (int *sock, char *buffer, size_t bufferlaenge){

    SOCKET sock2 = *sock;
    Sleep(20);

    if( send (sock2 , buffer, bufferlaenge, 0) == SOCKET_ERROR ){
        fprintf(stderr,"Fehler bei send(): %d\n", WSAGetLastError());
        exit(EXIT_FAILURE);

    }
}

/*void sendencls(int *sock){

    char *buffer*buffer = (char*) malloc (BUF);
    SOCKET sock2 = *sock;
    Sleep(1);
    buffer=("cls");
    senden (&sock2,buffer,strlen(buffer));
}

void sendenpause(int *sock){

    char *buffer = (char*) malloc (BUF);
    SOCKET sock2 = *sock;
    Sleep(1);
    buffer=("pause");
    senden (&sock2,buffer,strlen(buffer));
}*/

void sendentext(int *client_sock, int spieler, char *text){

    char *buffer /*= (char*) malloc (BUF)*/;
    //SOCKET sock2 = *sock;
    buffer=text;
    //printf("%s\n\n", buffer);
    if(spieler<3){
        senden (&client_sock[spieler],buffer,strlen(buffer));
    }
    else{
        if(strcmp(text,"cls")!=0 && strcmp(text,"pause")!=0){
            printf("%s\n",text);
        }
        else{
            system(text);
        }
    }
    buffer = NULL;
    free(buffer);           //sinnfrei?
}

void sendentexts(int *client_sock, int spieler, char *text, int ausgabeserver){

    char *buffer = (char*) malloc (BUF);
    //SOCKET sock2 = *sock;
    buffer=text;
    //printf("%s\n\n", buffer);
    if(spieler<3){
        senden (&client_sock[spieler],buffer,strlen(buffer));
    }
    else{
        if(ausgabeserver==1){
            printf("%s\n",text);
        }
        if(strcmp(text,"cls")==0 || strcmp(text,"pause")==0){
            system(text);
        }
    }
    buffer = NULL;
    free(buffer);           //sinnfrei ??
}

void sendenschleife(int *client_sock, char *text, int ausgabeserver){
    int i=0;
    for(i=0;i<4;i++){
        sendentexts(client_sock, i, text, ausgabeserver);
    }
}

void empfangenschleife(int *client_sock){

    char *buffer = (char*) malloc (BUF);
    //int laeng=0, ready=0, bereit=0;
    int i;
    size_t buflaeng;
    buffer[0] = '\0';
    buflaeng=BUF-1;
//    fd_set lese_sock;
    SOCKET sock2/*, sock_max*/;

   /* FD_ZERO(&lese_sock);
    for(i=0;i<3;i++){
        sock2 = client_sock[i];
        if(sock_max < sock2){
            sock_max = sock2;
        }
        FD_SET(sock2, &lese_sock);
    }

    while (bereit<3){

        ready = select( sock_max+1, &lese_sock, NULL, NULL, NULL );
        for(i=0;i<3;i++){
            sock2 = client_sock[i];
            if( FD_ISSET(sock2, &lese_sock)){
                laeng = recv (sock2, buffer, buflaeng, 0);
                if( laeng > 0 || laeng != SOCKET_ERROR ){
                    bereit++;
                }
                else{
                    fprintf(stderr,"Fehler bei recv(): %d\n", WSAGetLastError());
                    system("pause");
                    exit(EXIT_FAILURE);
                }
            }
        }
        printf("%d", ready);
        printf("%d", bereit);
    }
    printf("noch da readyschleifn");*/
    /*for(i=0;i<3;i++){

        sock2 =client_sock[i];
        laeng = recv (sock2, buffer, buflaeng, 0);

        if( laeng > 0 || laeng != SOCKET_ERROR ){
            buffer[laeng] = '\0';
        }
        else{
            fprintf(stderr,"Fehler bei recv(): %d\n", WSAGetLastError());
            system("pause");
            exit(EXIT_FAILURE);
        }
    }*/
    for(i=0;i<3;i++){
        sock2=client_sock[i];
        recv (sock2, buffer, buflaeng, 0);
    }
    free(buffer);
    //printf("noch da forschleifn ende");
}

void sendenpause(int *client_sock){

    char dummy;

    sendenschleife(client_sock, "Zum Fortfahren Enter druecken", 0);
    sendenschleife(client_sock, "eingabechar", 0);
    empfangenschleife(client_sock);
    printf("Clients sind bereit\n\nZum Fortfahren Enter druecken");
    fflush(stdin);
    scanf("%c",&dummy);                                     // fkt. evtl noch nicht später noch prüfen
    //system("pause");
}

void sendenpauseeinzel(int *client_sock, int spieler){

    //char dummy;

    //if(spieler<3){

    sendentext(client_sock, spieler, "Zum Fortfahren Enter druecken");
    sendentexts(client_sock, spieler, "eingabechar",0);
    //empfangentext(*sock, spieler);
    //eingaben(client_sock,"eingabechar",spieler);
    //printf("Clients sind bereit\n\nZum Fortfahren Enter druecken");
    fflush(stdin);

    //scanf("%c",&dummy);                                     // fkt. evtl noch nicht später noch prüfen
    //system("pause");
}

char empfangentext(int *client_sock, int spieler){

    int laeng;
    char *buffer = (char*) malloc (BUF);
    SOCKET sock2=client_sock[spieler];
    size_t buflaeng=BUF-1;

    laeng = recv (sock2, buffer, buflaeng, 0);

        if( laeng > 0 || laeng != SOCKET_ERROR ){
            buffer[laeng] = '\0';
        }
        else{
            fprintf(stderr,"Fehler bei recv(): %d\n", WSAGetLastError());
            system("pause");
            exit(EXIT_FAILURE);
        }

    return *buffer;
}

void anzeigenKartenAllenSpielern(int *client_sock, char *alle_spieler[4][8]){
    int i;

    for(i=0;i<4;i++){
        anzeigenKartenSpielerNetzwerk(client_sock, alle_spieler, i);
    }
}

void anzeigenKartenSpielerNetzwerk(int *client_sock, char *alle_spieler[4][8], int spieler){

    int i;
    char *buffer = (char*) malloc (BUF);

    if(spieler != 3){
        for(i=0;i<8;i++){
            if(alle_spieler[spieler][i]!=NULL){
                sprintf(buffer, "%d) %s\n",i+1,alle_spieler[spieler][i]);
                sendentext(client_sock, spieler, buffer);
            }
        }
    }
    else{
        anzeigen_karten_spieler(alle_spieler[spieler]);
    }    free(buffer);
}

char eingaben(int *client_sock, char *text, int spieler){

    char zeich='n';
    //int zahl=0;

    if(spieler<3){
        sendentext(client_sock, spieler, text);
        zeich = empfangentext(client_sock, spieler);
    }
    else{
		fflush(stdin);
        if(strcmp(text,"eingabechar")==0){
            scanf("%c", &zeich);
        }
        else{
            scanf("%c", &zeich);
            //sprintf(&zeich,"%d",zahl);
            // else no weg
        }
    }
    if(strcmp(text,"eingabeint")==0 ){
        zeich=zeich - '0';

    }
    //printf("\t\t\t%c", zeich);
    //printf("\t\t\t\t%d", (int)zeich);
    return zeich;
}



/*einzelpause(int *client_sock, spieler){

    char *buffer = (char*) malloc (BUF);
    int i=0;
    buffer = "pause";

    for(i=0;i<3;i++)
    eingaben(client_sock[spieler], buffer, spieler);

}*/

//  copyright by Robert Kiesenbauer
//  any illegal copying will result in death


// schofkopfa version 3.2(solo)       31.12.2012	21:14:03,71

