/*
* Brief: Used to open a file and write text into the file. Exits appropriately in case of errors
* Author: Deekshith Reddy Patil, patil.deekshithreddy@colorado.edu
* Reference: Syslog, https://linux.die.net/man/3/syslog
*/

#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>

#define TOTAL_NUMBER_OF_ARGUMENTS         3

#define SYSLOG_PREFIX_TO_LOGS             NULL
#define SYSLOG_OPTION                     0
#define SYSLOG_FACILITY                   LOG_USER

#define SYSLOG_PRIORITY_ERROR             LOG_ERR
#define SYSLOG_PRIORITY_DEBUG             LOG_DEBUG

int main(int argc, char *argv[])
{

  openlog(SYSLOG_PREFIX_TO_LOGS,SYSLOG_OPTION,SYSLOG_FACILITY);

  if(argc != TOTAL_NUMBER_OF_ARGUMENTS)
  {
    printf("Invalid number of arguments recieved!\n");
    syslog(SYSLOG_PRIORITY_ERROR,"Invalid number of arguments recieved!\n");
    exit(1);
  }

  const char * path = argv[1];
  const char * text = argv[2];

  FILE *fp = fopen(path,"w"); //opens a file in write mode. If it already exists, contents are discarded

  if(fp == NULL)
  {
    printf("Error opening the file!\n");
    syslog(SYSLOG_PRIORITY_ERROR,"Error opening the file!\n");
    exit(0);
  }

  fprintf(fp, "%s\n",text);
  syslog(SYSLOG_PRIORITY_DEBUG,"Writing '%s' to '%s'\n",text,path);

  return 0;
}
