/*
 * candownload.c
 *
 *  Created on: Oct 12, 2013
 *      Author: kares
 */

#include <stdint.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "canfestival.h"
#include "lpc11cxx_bl.h"

static void catch_signal(int sig);
static void InitNodes(CO_Data* d, UNS32 id);
static void Exit(CO_Data* d, UNS32 id);

void lpc11cxx_bl_stopped(CO_Data* d)
{
	printf("LPC11Cxx CANOPEN stack stopped\n");
}

void lpc11cxx_bl_operational(CO_Data* d)
{
	printf("LPC11Cxx CANOPEN stack operational\n");
}

void lpc11cxx_bl_preOperational(CO_Data* d)
{
	 printf("LPC11Cxx CANOPEN stack pre-operational\n");
}

void lpc11cxx_bl_initialisation(CO_Data* d)
{
	printf("LPC11Cxx CANOPEN stack initialized\n");
}

int download_ihex(uint8_t * buf, int size,char* canspeed,uint8_t nodeid, char* lpath)
{
	CAN_PORT MasterPort;
	s_BOARD MasterBoard;
	LIB_HANDLE CanDriverHandle;
	UNS8 ret;
	MasterBoard.baudrate = canspeed;
	MasterBoard.busname="LPCMaster";
  /* install signal handler for manual break */
	signal(SIGTERM, catch_signal);
	signal(SIGINT, catch_signal);
	TimerInit();
	CanDriverHandle = LoadCanDriver(lpath);
	if( CanDriverHandle == NULL)
	{
		printf("Unable to load CAN driver library %s\n",lpath);
		return 0;
	}
	//lpc11cxx_bl_Data.heartbeatError = lpc11cxx_bl_heartbeatError;
	lpc11cxx_bl_Data.initialisation = lpc11cxx_bl_initialisation;
	lpc11cxx_bl_Data.preOperational = lpc11cxx_bl_preOperational;
	lpc11cxx_bl_Data.operational 	= lpc11cxx_bl_operational;
	lpc11cxx_bl_Data.stopped		= lpc11cxx_bl_stopped;
//	lpc11cxx_bl_Data.post_TPDO		= lpc11cxx_bl_post_TPDO;

	MasterPort = canOpen(&MasterBoard,&lpc11cxx_bl_Data);
	if(MasterPort==NULL)
	{
		printf("Cannot open Master node.\n");
		return 0;
	}
	// Start timer thread
	StartTimerLoop(&InitNodes);
	sleep(1);
	setState(&lpc11cxx_bl_Data,Operational);
	ret = readNetworkDict(&lpc11cxx_bl_Data,nodeid,0x1000,0x0,uint32,0);
	if(ret == 0xFF)
	{
		printf("Serial ID read attempt returned with error.\n");
		return 0;
	}else if (ret == 0xfe)
	{
		printf("Unable too connect to LPC11Cxx node.\n");
		return 0;
	}

	sleep(3);
	// Start timer thread
	StopTimerLoop(&Exit);
	canClose(&lpc11cxx_bl_Data);
	UnLoadCanDriver(CanDriverHandle);
	return size;
}

static void catch_signal(int sig)
{
  signal(SIGTERM, catch_signal);
  signal(SIGINT, catch_signal);

  printf("Got Signal \"%s\"\n",strsignal(sig));
}

/***************************  INIT  *****************************************/
static void InitNodes(CO_Data* d, UNS32 id)
{
		/* init */
	/* callbacks appear to be buggy and do not pass parameters properly. */
	/* so we must use a global variable for accessing CO_DATA. */
	setState(&lpc11cxx_bl_Data, Initialisation);
}

static void Exit(CO_Data* d, UNS32 id)
{
	/* callbacks appear to be buggy and do not pass parameters properly. */
	/* so we must use a global variable for accessing CO_DATA. */
    //Stop master
	setState(&lpc11cxx_bl_Data, Stopped);

}
