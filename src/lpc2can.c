/*
 ============================================================================
 Name        : LPC2CAN.c
 Author      : Karoly Molnar
 Version     : 1.0
 Copyright   : Lyorak 2013
 Description : LPC12Cxx programmer via CAN
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <getopt.h>
#include "cintelhex.h"
#include "canfestival.h"
#include "lpc11cxx_bl.h"

#define SW_VERSION  1.0

static void fusage() {
    printf("LPC via CAN programmer\n");
    printf("(c) Lyorak 2013\n");
    printf("Using 3rd party software. See licenses\n");
    printf("Usage: lpc2can [-s speed] [-n node] file.hex\n");
    printf("Example: lpc2can -s 125 -n 7d sample.hex\n");
}

int main(int argc, char *argv[])
{
    int c,index;
    opterr = 0;
    char *cvalue = NULL;
    int canspeed = 125000;
    int nodeid   = 0x7d;
    int tmparg;

    while ((c = getopt (argc, argv, "s:n:v")) != -1)
    {
        switch (c)
        {
        case 'v':
            printf("LPC via CAN programmer version %.01f\n",SW_VERSION);
            printf("(c) Lyorak 2013\n");
            return EXIT_SUCCESS;

        case 's':
            cvalue = optarg;
            tmparg = (int)strtol(cvalue, NULL, 10);
            if((tmparg >= 125) && (tmparg <=1000))
            {
                canspeed = tmparg*1000;
                printf("Setting CAN speed to %d kbps\n",tmparg);
            }else
            {
                fprintf (stderr, "Invalid speed argument %s\n",cvalue);
            }
            break;
        case 'n':
            cvalue = optarg;
            tmparg = (int)strtol(cvalue, NULL, 16);
            if((tmparg >= 0) && (tmparg <=0xff))
            {
                nodeid= tmparg;
                printf("Setting Node ID to 0x%x\n",tmparg);
            }else
            {
                fprintf (stderr, "Invalid speed argument %s\n",cvalue);
            }
            break;
        case '?':
            if (optopt == 's' || optopt == 'n')
            {
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            }
            else if (isprint (optopt))
            {
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            }
            else
            {
                fprintf (stderr,"Unknown option character `\\x%x'.\n",optopt);
            }
            return EXIT_FAILURE;
        default:
            abort ();
        }
    }
    if(optind < argc)
    {
    	char * filename = argv[optind];
    	ihex_recordset_t * ihexrec;
    	ulong_t size = 0;
        printf("parsing file %s...\n",filename);
        ihexrec = ihex_rs_from_file(filename);
        if(ihexrec != NULL)
        {
        	size = ihex_rs_get_size(ihexrec);
        }
        if(size == 0)
        {
        	printf("Invalid file!\n");
        	return EXIT_FAILURE;
        }else
        {
        	uint_t no_records = ihexrec->ihrs_count;
        	printf("File contains %d bytes of ROM data in %d records.\n", (int)size, no_records);
        }
    }else
    {
        fusage();
    }

    return EXIT_SUCCESS;
}

