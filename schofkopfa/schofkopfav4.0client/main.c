#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string.h>

#define PORT 1283
#define BUF 1024
#define IP 127.0.0.1

typedef int socklen_t;

void cleanup(void);

int main()
{
    WORD winsockversion;                                    // verwendete winsoch version
    WSADATA wsadata;                                        // wsadata?? in dieser struktur finden sich informationen zur winsockversion??
    SOCKET socketdesc=0;    //, sockverb1=0;                // socketdescriptor
    //socklen_t len;
    struct sockaddr_in server;
    char *buffer = (char*) malloc (BUF), e_ip[16]="127.0.0.1";
    char eingabechar, eingabeint;
    int laeng;
    unsigned long addr;
    size_t bla;

    printf("\nWIllkommen beim Schofkopfprogramm v4.beta\n\n");
    memset( &server, 0, sizeof (server));                   // server auf 0 setzn
    server.sin_family = AF_INET;
    server.sin_port = htons (PORT);                         // port für socket festlegen; htons=host to network short,  umwandlung in network byte order

    printf("bitte ip-adresse des servers eingeben\n\n");
    //scanf("%s",e_ip);
    fflush ( stdin );
    addr=inet_addr(e_ip);
    memcpy((char *) &server.sin_addr, &addr, sizeof(addr)) ;           // beliebige ip-addressen annehmen testen !!!; htnl=host to network long

    winsockversion = MAKEWORD(2,0);                         // word für winsockverion 2,0 kreiren
    if (WSAStartup(winsockversion, &wsadata)!= 0){          // winsock initialisieren und auf fehler abfragen -1
        printf("fehler beim initialisieren von winsock\n");
        exit (0);
    }
    else{
        printf("winsock initialisiert\n");
    }

    atexit(cleanup);

    socketdesc = socket(AF_INET, SOCK_STREAM, 0);           // socket anlegen rückgabewert ist ein socketdescriptor
                                                            // af_inat=ipv4, sock_stream=übertragungsart tcp, 0=standardprotokoll tcp
    if (socketdesc < 0){
        printf("fehler beim anlegen eines sockets\n");
    }

    if (connect (socketdesc, (struct sockaddr*) &server, sizeof(server))<0){
        printf("fehler beim connectn\n");
    }
    //system("pause");

    do {
        do{
            buffer[0] = '\0';
            bla=BUF-1;
            laeng = recv (socketdesc, buffer, bla, 0);

            if( laeng > 0 || laeng != SOCKET_ERROR ){
                buffer[laeng] = '\0';
            }
            else{
                fprintf(stderr,"Fehler bei recv(): %d\n", WSAGetLastError());
                system("pause");
                exit(EXIT_FAILURE);
            }

            /*if (strcmp (buffer, "ende") == 0){
                break;
            }*/

            if(!(strcmp(buffer,"eingabeint")==0 || strcmp(buffer,"eingabechar")==0)){
                if(strcmp(buffer,"cls")==0){
                    system("cls");
                    continue;
                }

                if(strcmp(buffer,"pause")==0){
                    system("pause");
                    continue;
                }

                if(strcmp(buffer,"username")==0){
                    system("if %username% == Robert echo Hallo Meister");
                    system("if %username% == rkiesbauer echo Hallo Meister");
                    system("if %username% == mauer echo Hallo Meister2");
                    //system("pause");
                    //system("cls");
                    continue;
                }

                printf("%s\n",buffer);                          // solang im buffer ned eingabeint, eingabechar, cls oda pause stehd soid a an buffer ausgebn
                                                                // und de nächste nachricht empfanga
            }
        /*if(strcmp(buffer, "eingabechar")==0){
            printf("is null");
        }
        printf("\t\t%s\n", buffer);*/
        //printf("in da leseschleifn\n");

        }while(!(strcmp (buffer, "eingabeint")==0 || strcmp(buffer,"eingabechar")==0));

        //printf("\t\t%s\n\n",buffer);
        /*if (strcmp (buffer, "ende") == 0){
            printf("im break");
            break;
        }*/

        //printf ("Nachricht erhalten: %s\n", buffer);
        //karten auslesen frage auslesen frage stellen beenden break

        if(strcmp(buffer,"eingabeint")==0){
			fflush(stdin);
            scanf("%c",&eingabeint);
            sprintf(buffer,"%c",eingabeint);       //=!atoi
        }
        else{
			fflush(stdin);
            scanf("%c",&eingabechar);
            sprintf(buffer,"%c",eingabechar);       //=!atoi
        }
        //zu string konvertieren und versenden
        //fgets (buffer, BUF, stdin);

        if( send (socketdesc, buffer, strlen (buffer), 0) == SOCKET_ERROR ){
            fprintf(stderr,"Fehler bei send(): %d\n", WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        fflush(stdin);
        //printf("Warten auf andere Spieler ...\n");

    } while ( 1 /*strcmp (buffer, "quit\n") != 0*/);

    closesocket (socketdesc);

    printf("Hello world!\n");
    return 0;
}

void cleanup(void){
   // winsocks beenden
   WSACleanup();
   printf("\nwinsock ende\n");
   system("pause");
}
