
/* Copyright: You are granted to share this source to anyone, which needs it,
   as long my name is showed here: Matthias Huber, DM2HR, dm2hr@hmro.de */


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LNSIZE 128
#define TOSIZE 30

char *postag_ns  = "4822.17N";
char *postag_ew  = "01043.09E";

#define FROM "DM2HR-1"
#define KISSUTILHOST "localhost"

char *symbol    = "/-"; // House QTH VHF


int main ( int argc, char *argv[] ) {
FILE *fp;
int outfd = 1;
char line[LNSIZE];
int i, j, len, opt, debug = 0, uppercase = 0;
char to[TOSIZE];
char tosave[TOSIZE];
char pipecmd[128];
char *from = FROM;
char *host = KISSUTILHOST;

	memset(to, 0, TOSIZE);
	memset(tosave, 0, TOSIZE);
	memset(line, 0, LNSIZE);

        while ((opt = getopt (argc, argv, "h:f:du")) != -1) {
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

                        case 'u':
				uppercase = 1;
                                break;

                         default:
                                fprintf(stderr, "\nusage:\n    %s [-h kissutil-host] [-f <from-callsign-x] [-d(debug)] [-u(uppercase message)] [VIA [VIA...]]\n\n", argv[0]);
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
	
	do {
		if ( feof(stdin) )
			break;

		memset(to, 0, TOSIZE);
		printf("\nto(%s)>", tosave);
		fgets(to, TOSIZE, stdin);
		
		len = strlen(to);
		if ( ( *to == '\r' || *to == '\n' ) && strlen(tosave) > 0 ) { // no input, so save is input copied back
			strcpy(to, tosave);
			len = strlen(tosave);
		}
		for ( i = 0; i < len; i++ )
			if ( to[i] == 0x0d || to[i] == 0x0a )
				to[i] = 0;
		len = strlen(to);
		for ( i = 0; i < len; i++ )
			to[i] = toupper(to[i]);
		strcpy(tosave, to); // saving for later laziness
	
		memset(line, 0, LNSIZE);
		printf("message>");
		fgets(line, LNSIZE, stdin);
		if ( uppercase == 1 ) {
		        for ( i = 0; i < strlen(line); i++ )
			        line[i] = toupper(line[i]);
                }
                
		fputs("[0] ", fp);
		fputs(from, fp);
		fputc('>', fp);
		fputs(to, fp);
                
		// maybe, we gave some vias in cmdline
		if ( optind < argc ) {
			for ( i = optind; i < argc; i++ ) {
				fputc(',', fp);
				for ( j = 0; j < strlen(argv[i]); j++ )
				        fputc(toupper(argv[i][j]), fp);
			}
		}		

		fputc(':', fp);
		fputc('=', fp);

		// position N/S
		fputs(postag_ns, fp);

		// symbol 1st byte
		fputc(symbol[0], fp);

		// position N/S
		fputs(postag_ew, fp);

		// symbol 2nd byte
		fputc(symbol[1], fp);

		// one letter for better readability
		fputc(' ', fp);

		// comment (text)
		len = strlen(line);
		for ( i = 0; i < len; i++ )
			fputc(line[i], fp);
		fflush(fp);
	} while ( strlen(line) > 0 );

	fflush(fp);
	fclose(fp);

	exit(0);
}
