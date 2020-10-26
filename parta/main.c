/*
 *  main.c
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "cpu.h"

int
main(int argc, char** argv)
{
  // if (argc != 2) {
  //   fprintf(stderr, "APEX_Help : Usage %s <input_file>\n", argv[0]);
  //   exit(1);
  // }

  APEX_CPU* cpu = APEX_cpu_init(argv[1]);
  if (!cpu) {
    fprintf(stderr, "APEX_Error : Unable to initialize CPU\n");
    exit(1);
  }
  int cycle = INT_MAX;
  int mode = 0;
  if(strcmp(argv[2], "simulate") == 0){
    mode = 0;
  }
  else if(strcmp(argv[2], "display") == 0){
    mode = 1;
  }
  else{
    printf("for second parameter, please enter \"simulate\" or \"display\".\n");
    return 0;
  }
  if(argc >= 4){
    cycle = atoi(argv[3]);
  }

  APEX_cpu_run(cpu, mode, cycle);
  APEX_cpu_stop(cpu);
  return 0;
}