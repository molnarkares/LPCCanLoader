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



#define SW_VERSION  1.0

static ulong_t check_ihex(ihex_recordset_t *);
extern int download_ihex(uint8_t *, int,char*,uint8_t,char*);

static void fusage();


int main(int argc, char *argv[])
{



    int c;
    opterr = 0;
    char *cvalue = NULL;
    char* canspeed = "125K";
    char* LibraryPath="libcanfestival_can_virtual.so";
    uint8_t nodeid   = 0x7d;
    int tmparg;

    while ((c = getopt (argc, argv, "l:s:n:v")) != -1)
    {
        switch (c)
        {
        case 'v':
            printf("LPC via CAN programmer version %.01f\n",SW_VERSION);
            printf("(c) Lyorak 2013\n");
            return EXIT_SUCCESS;

        case 's':
        	canspeed = optarg;
            printf("Setting CAN speed to %sbps.\n",canspeed);
            break;
        case 'l':
        	LibraryPath = optarg;
            printf("Using CAN driver library %s.\n",LibraryPath);
            break;
        case 'n':
            cvalue = optarg;
            tmparg = (int)strtol(cvalue, NULL, 16);
            if((tmparg >= 0) && (tmparg <=0xff))
            {
                nodeid= (uint8_t)tmparg;
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
    	uint8_t * databuffer;
    	ihex_recordset_t * ihexrec;
    	ulong_t record_size,downloaded_size;
        printf("parsing file %s...\n",filename);
        ihexrec = ihex_rs_from_file(filename);
        record_size = check_ihex(ihexrec);
        databuffer = malloc(record_size);
        if((record_size == 0 ) || (databuffer == NULL))
        {
        	printf("Unable to proceed. Exiting.\n");
        	return EXIT_FAILURE;
        }

		downloaded_size = download_ihex(databuffer,
										(int)record_size,
										canspeed,
										nodeid,
										LibraryPath);
		if(downloaded_size != record_size)
		{
			printf("Unable to download HEX record.\n");
			return EXIT_FAILURE;
		}else
		{
			printf("Download successful. Reset your device.\n");
			return EXIT_SUCCESS;
		}
    }else
    {
        fusage();
    }

    return EXIT_SUCCESS;
}

static ulong_t check_ihex(ihex_recordset_t * rset)
{
	uint_t no_records = rset->ihrs_count;
	ulong_t size = 0;
	ihex_addr_t last_address = 0;
	int ctr;
	if(rset == NULL)
	{
		printf("Invalid HEX record file.\n");
		return 0;
	}
	size = ihex_rs_get_size(rset);
	last_address = rset->ihrs_records[0].ihr_address;
	printf("Verifying record continuity...\n");
	for(ctr = 0; ctr < no_records;ctr++)
	{
		if(rset->ihrs_records[ctr].ihr_type == IHEX_DATA)
		{
			if(last_address != rset->ihrs_records[ctr].ihr_address)
			{
				printf("Hex record is not continuous between address 0x%04x and 0x%04x.\n",
						last_address,
						rset->ihrs_records[ctr].ihr_address);
				return 0;
			}
			last_address += rset->ihrs_records[ctr].ihr_length;
		}
	}
	printf("Record is continuous.\n");
	printf("Record start address is 0x%04x.\n", (int)rset->ihrs_records[0].ihr_address);
	printf("Record contains %d bytes of ROM data.\n", (int)size);
	return size;
}

static void fusage() {
    printf("LPC via CAN programmer\n");
    printf("(c) Lyorak 2013\n");
    printf("Using 3rd party software. See licenses\n");
    printf("Usage: lpc2can [-s speed] [-n node] file.hex\n");
    printf("Example: lpc2can -s 125 -n 7d sample.hex\n");
}

