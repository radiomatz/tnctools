
/* Copyright: You are granted to share this source to anyone, which needs it,
   as long my name is showed here: Matthias Huber, DM2HR, dm2hr@hmro.de */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <stdint.h>

#define FROM "N0CALL"
#define KISSUTILHOST "localhost"


#define LNSIZE 128
#define TOSIZE 30

extern char *optarg;
extern int optind, opterr, optopt;


int main ( int argc, char *argv[] ) {
FILE *fp;
int outfd = 1;
char line[LNSIZE];
int i, len, opt, debug = 0;
char *comma = "";
char to[TOSIZE];
char tosave[TOSIZE];
char pipecmd[128];
char *from = FROM;
char *host = KISSUTILHOST;
uint8_t msgnum = 42;

	memset(to, 0, TOSIZE);
	memset(tosave, 0, TOSIZE);
	memset(line, 0, LNSIZE);

        while ((opt = getopt (argc, argv, "h:f:d")) != -1) {
                switch (opt) {
                        case 'h':
                                host = optarg;
                                break;

                        case 'f':
                                from = optarg;
                                for(i = 0; i < strlen(from); i++) 
                                        from[i] = toupper(from[i]);
                                break;

                        case 'd':
				debug = 1;
                                break;

                         default:
                                fprintf(stderr, "\nusage:\n    %s [-h kissutil-host] [-f <from-callsign-x]  [-d(debug)] [VIA [VIA...]]\n\n", argv[0]);
                                break;
                }
        }

	if ( debug == 0 ) {
		strcpy(pipecmd, "kissutil -h ");
		strcat(pipecmd, host);
		strcat(pipecmd, " >/dev/null");
		fprintf(stderr, "pipe cmd=%s\n", pipecmd);
		fp = popen(pipecmd, "w");
	} else {
		fp = stdout;
	}
	if ( fp == NULL ) {
		perror("popen kissutil failed");
		exit(1);
	}
	

	do {
		if ( feof(stdin) )
			break;

		memset(to, 0, TOSIZE);
		printf("\n(%s) >", *tosave ? tosave : "(EMAIL,EMAIL-2,MAIL,SAMAIL,SMS,APTDAB,<CALLSIGN-X>)");
		fgets(to, TOSIZE, stdin);

		len = strlen(to);
		for ( i = 0; i < len; i++ )
			if ( to[i] == 0x0d || to[i] == 0x0a )
				to[i] = 0;
		len = strlen(to);
		for ( i = 0; i < len; i++ )
			to[i] = toupper(to[i]);
		len = strlen(to);
		for ( i = 0; i < len; i++ )
			to[i] = toupper(to[i]);
		strcpy(tosave, to); // saving for later laziness
		
		memset(line, 0, LNSIZE);
		printf("message>");
		fgets(line, LNSIZE, stdin);
		for ( i = 0; i < strlen(line); i++ )
			if ( line[i] == 0x0d || line[i] == 0x0a )
				line[i] = 0;

		fputs("[0] ", fp);
		fputs(from, fp);
		fputc('>', fp);
		fputs(to, fp);
		comma = ",";
		// maybe, we gave some vias in cmdline, possibly needed here!
		if ( optind < argc ) {
			for ( i = optind; i < argc; i++ ) {
				fputs(comma, fp);
				for ( int j = 0; j < strlen(argv[i]); j++ )
				        fputc(toupper(argv[i][j]), fp);
				comma = ",";
			}
		}		

		// to := EMAIL,EMAIL-2,MAIL,SAMAIL,SMS,APTDAB
		fputs("::", fp);
		fprintf(fp, "%-9.9s", to);

		// comment (text)
		fputc(':', fp);
		fputs(line, fp);

		// Message Number (fake)
                for(i = 0; i < strlen(from); i++)
                        msgnum += from[i]; 
                for(i = 0; i < strlen(to); i++)
                        msgnum += to[i]; 
                for(i = 0; i < strlen(line); i++)
                        msgnum += line[i]; 
		fprintf(fp, "{%d}\n", msgnum);

		fflush(fp);
	} while ( strlen(line) > 0 );

	fflush(fp);
	fclose(fp);

	exit(0);
}
