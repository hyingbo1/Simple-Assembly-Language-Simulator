/*
 *  cpu.c
 *  Contains APEX cpu pipeline implementation
 *
 *  Author :
 *  Gaurav Kothari (gkothar1@binghamton.edu)
 *  State University of New York, Binghamton
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"

/* Set this flag to 1 to enable debug messages */
int ENABLE_DEBUG_MESSAGES = 1;


int comparator(APEX_CPU* cpu, int r_name, int *rs_value){
  for(int i=MEM1; i<=WB; i++){
    CPU_Stage* stage = &cpu->stage[i];
    if(stage->rd == r_name){
      if( strcmp(stage->opcode,"MOVC") == 0
        || strcmp(stage->opcode,"ADDL")== 0 || strcmp(stage->opcode,"ADD")== 0 
        ||strcmp(stage->opcode,"SUBL")== 0 || strcmp(stage->opcode,"SUB")== 0
        || strcmp(stage->opcode,"AND")== 0 || strcmp(stage->opcode,"OR")== 0
        || strcmp(stage->opcode,"EX-OR")== 0|| strcmp(stage->opcode,"MUL")== 0){
        
        *rs_value = stage->buffer; //forwarding
        return 1;
      }
    }
  }
  CPU_Stage* stage = &cpu->stage[WB];
  if(stage->rd == r_name){
    if(strcmp(stage->opcode,"LOAD") == 0 || strcmp(stage->opcode,"LDR") == 0){
      *rs_value = stage->buffer; //forwarding
      return 1;
    }
  }  
  return 0;
}

int comparator_z(APEX_CPU* cpu, int* z){
  CPU_Stage* stage = &cpu->stage[EX2];
  if(strcmp(stage->opcode,"ADD") == 0 || strcmp(stage->opcode,"ADDl") == 0
    || strcmp(stage->opcode,"SUB") == 0 || strcmp(stage->opcode,"SUBL") == 0
    || strcmp(stage->opcode,"MUL") == 0 ){
    return 0;
    }
  for(int i=MEM1; i<WB; i++){
    CPU_Stage* stage = &cpu->stage[i];
    if(strcmp(stage->opcode,"ADD") == 0 || strcmp(stage->opcode,"ADDl") == 0
      || strcmp(stage->opcode,"SUB") == 0 || strcmp(stage->opcode,"SUBL") == 0
      || strcmp(stage->opcode,"MUL") == 0 ){
      
      if(stage->buffer == 0){
        *z = 1;
        
      }
      else{
        *z = 0;
        
      }
      return 1;
    }
  }
  return 0;

}
/*
 * This function creates and initializes APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *                 implementation
 */
APEX_CPU*
APEX_cpu_init(const char* filename)
{
  if (!filename) {
    return NULL;
  }

  APEX_CPU* cpu = malloc(sizeof(*cpu));
  if (!cpu) {
    return NULL;
  }

  /* Initialize PC, Registers and all pipeline stages */
  cpu->end = 0;
  cpu->pc = 4000;
  memset(cpu->regs, 0, sizeof(int) * 32);
  memset(cpu->regs_valid, 1, sizeof(int) * 32);
  memset(cpu->stage, 0, sizeof(CPU_Stage) * NUM_STAGES );
  memset(cpu->data_memory, 0, sizeof(int) * 4000);

  /* Parse input file and create code memory */
  cpu->code_memory = create_code_memory(filename, &cpu->code_memory_size);

  if (!cpu->code_memory) {
    free(cpu);
    return NULL;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    fprintf(stderr,
            "APEX_CPU : Initialized APEX CPU, loaded %d instructions\n",
            cpu->code_memory_size);
    fprintf(stderr, "APEX_CPU : Printing Code Memory\n");
    printf("%-9s %-9s %-9s %-9s %-9s %-9s\n", "opcode", "rd", "rs1", "rs2","rs3", "imm");

    for (int i = 0; i < cpu->code_memory_size; ++i) {
      printf("%-9s %-9d %-9d %-9d %-9d %-9d\n",
             cpu->code_memory[i].opcode,
             cpu->code_memory[i].rd,
             cpu->code_memory[i].rs1,
             cpu->code_memory[i].rs2,
             cpu->code_memory[i].rs3,
             cpu->code_memory[i].imm);
    }
  }

  /* Make all stages busy except Fetch stage, initally to start the pipeline */
  for (int i = 1; i < NUM_STAGES; ++i) {
    cpu->stage[i].busy = 1;
  }

  cpu->clock = 1;
  return cpu;
}

/*
 * This function de-allocates APEX cpu.
 *
 * Note : You are free to edit this function according to your
 *                 implementation
 */
void
APEX_cpu_stop(APEX_CPU* cpu)
{
  free(cpu->code_memory);
  free(cpu);
}

/* Converts the PC(4000 series) into
 * array index for code memory
 *
 * Note : You are not supposed to edit this function
 *
 */
int
get_code_index(int pc)
{
  return (pc - 4000) / 4;
}

static void
print_instruction(CPU_Stage* stage)
{
  if (strcmp(stage->opcode, "HALT") == 0) {
    printf("%s ", stage->opcode);
  }
  if (strcmp(stage->opcode, "STORE") == 0 ) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rs1, stage->rs2, stage->imm);
  }

  if (strcmp(stage->opcode, "MOVC") == 0) {
    printf("%s,R%d,#%d ", stage->opcode, stage->rd, stage->imm);
  }

  if (strcmp(stage->opcode, "LOAD") == 0 || strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0 ) {
    printf(
      "%s,R%d,R%d,#%d ", stage->opcode, stage->rd, stage->rs1, stage->imm);
  }
  
  if (strcmp(stage->opcode, "STR") == 0 || strcmp(stage->opcode, "LDR") == 0|| strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR" ) == 0 || strcmp(stage->opcode, "MUL") == 0) {
    printf(
      "%s,R%d,R%d,R%d ", stage->opcode, stage->rd, stage->rs1, stage->rs2 );
  }

  if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 ) {
    printf("%s,#%d ",stage->opcode, stage->imm);
  }

  if (strcmp(stage->opcode, "JUMP") == 0) {
    printf("%s,R%d,#%d",stage->opcode,stage->rs1, stage->imm);
  }

}

/* Debug function which dumps the cpu stage
 * content
 *
 * Note : You are not supposed to edit this function
 *
 */
static void
print_stage_content(char* name, CPU_Stage* stage)
{
  printf("%-15s: pc(%d) ", name, stage->pc);
  print_instruction(stage);
  printf("\n");
}

/*
 *  Fetch Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
fetch(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[F];
  if(get_code_index(cpu->pc) > cpu->code_memory_size){ // pc excess code size or there was a HALT
    memset(&cpu->stage[F], 0, sizeof(CPU_Stage)); // stop fetch new code line
    cpu->stage[DRF] = cpu->stage[F];
    if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Fetch", stage);
    }
    return 0;
  }

  if (!stage->busy ) {
    /* Store current PC in fetch latch */
    stage->pc = cpu->pc;

    /* Index into code memory using this pc and copy all instruction fields into
     * fetch latch
     */
    
    APEX_Instruction* current_ins = &cpu->code_memory[get_code_index(cpu->pc)];
    strcpy(stage->opcode, current_ins->opcode);
    stage->rd = current_ins->rd;
    stage->rs1 = current_ins->rs1;
    stage->rs2 = current_ins->rs2;
    stage->rs3 = current_ins->rs3;
    stage->imm = current_ins->imm;
    stage->rd = current_ins->rd;

    

    /* Update PC for next instruction */
    cpu->pc += 4;
    printf("cpu -> pc%d",cpu->pc);
  }
  if(!stage->stalled){
    stage->busy = 0;
    cpu->stage[DRF] = cpu->stage[F];
  }
  if(stage->stalled){
    stage->busy = 1;
  }

  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Fetch", stage);
  }
  return 0;
}

/*
 *  Decode Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
decode(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[DRF];
  if (!stage->busy && !stage->stalled) {

    /* Read data from register file for store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      
      if((cpu->regs_valid[stage->rs1] == 1 || comparator(cpu,stage->rs1,&stage->rs1_value) == 1) 
      && (cpu->regs_valid[stage->rs2] == 1 || comparator(cpu,stage->rs2,&stage->rs2_value) == 1)){//all rs are valid
        if(comparator(cpu,stage->rs1,&stage->rs1_value) == 0){
          stage->rs1_value = cpu->regs[stage->rs1];
        }
        if(comparator(cpu,stage->rs2,&stage->rs2_value) == 0){
          stage->rs2_value = cpu->regs[stage->rs2];
        }
        cpu->stage[F].stalled = 0;
      }
      else{ // stall stage before decode.
        cpu->stage[F].stalled = 1;
      }
    }

    /* No Register file read needed for MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      if(comparator(cpu,stage->rs1,&stage->rs1_value) == 1){
        cpu->stage[F].stalled = 0;
      }
      else if(cpu->regs_valid[stage->rs1] == 1){
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "STR") == 0) {
      // if(comparator(cpu,stage->rs1,&stage->rs1_value) == 1 && comparator(cpu,stage->rs2,&stage->rs2_value)==1
      //   && comparator(cpu,stage->rs3,&stage->rs3_value) == 1){
      //   cpu->stage[F].stalled = 0;
      // }
      if((cpu->regs_valid[stage->rs1] == 1 || comparator(cpu,stage->rs1,&stage->rs1_value) == 1) 
        && (cpu->regs_valid[stage->rs2] == 1 || comparator(cpu,stage->rs2,&stage->rs2_value)==1) 
        && (cpu->regs_valid[stage->rs3] == 1 || comparator(cpu,stage->rs3,&stage->rs3_value) == 1)){
        if(comparator(cpu,stage->rs1,&stage->rs1_value) == 0){
          stage->rs1_value = cpu->regs[stage->rs1];
        }
        if(comparator(cpu,stage->rs2,&stage->rs2_value) == 0){
          stage->rs2_value = cpu->regs[stage->rs2];
        }
        if(comparator(cpu,stage->rs3,&stage->rs3_value) == 0){
          stage->rs3_value = cpu->regs[stage->rs3];
        }
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "LDR") == 0){
      if((cpu->regs_valid[stage->rs1] == 1 || comparator(cpu,stage->rs1,&stage->rs1_value) == 1) 
      && (cpu->regs_valid[stage->rs2] == 1 || comparator(cpu,stage->rs2,&stage->rs2_value) == 1)){//all rs are valid
        if(comparator(cpu,stage->rs1,&stage->rs1_value) == 0){
          stage->rs1_value = cpu->regs[stage->rs1];
        }
        if(comparator(cpu,stage->rs2,&stage->rs2_value) == 0){
          stage->rs2_value = cpu->regs[stage->rs2];
        }
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
    }
    if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0) {
      if(comparator(cpu,stage->rs1,&stage->rs1_value) == 1 ){
        cpu->stage[F].stalled = 0;
      }
      else if(cpu->regs_valid[stage->rs1] == 1){
        stage->rs1_value = cpu->regs[stage->rs1];
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 || strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "MUL") == 0) {
      if((cpu->regs_valid[stage->rs1] == 1 || comparator(cpu,stage->rs1,&stage->rs1_value) == 1) 
      && (cpu->regs_valid[stage->rs2] == 1 || comparator(cpu,stage->rs2,&stage->rs2_value) == 1)){//all rs are valid
        if(comparator(cpu,stage->rs1,&stage->rs1_value) == 0){
          stage->rs1_value = cpu->regs[stage->rs1];
        }
        if(comparator(cpu,stage->rs2,&stage->rs2_value) == 0){
          stage->rs2_value = cpu->regs[stage->rs2];
        }
        cpu->stage[F].stalled = 0;
      }
      else if(cpu->regs_valid[stage->rs1] == 1 && cpu->regs_valid[stage->rs2] ==1){
        stage->rs1_value = cpu->regs[stage->rs1];
        stage->rs2_value = cpu->regs[stage->rs2];
        cpu->stage[F].stalled = 0;
        
      }
      else{
        cpu->stage[F].stalled = 1;
      
      }
    }

    if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0) {
      if(comparator_z(cpu,&stage->z)==1){
        cpu->stage[F].stalled = 0;
        stage->z_valid = 1;
      }
      else if(cpu->z_valid == 1){
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
      
    }

    if (strcmp(stage->opcode, "JUMP") == 0){
      if(cpu->regs_valid[stage->rs1] == 1){
        stage->rs1_value = cpu->regs[stage->rs1];
        //printf("%d",stage->rs1_value);
        cpu->stage[F].stalled = 0;
      }
      else{
        cpu->stage[F].stalled = 1;
      }
    }

    if (strcmp(stage->opcode, "HALT") == 0) {
      int rest_code_size = 1;
      for(int i=EX2; i<=WB; i++){
        if(strcmp(cpu->stage[i].opcode, "") != 0){
          rest_code_size++;
        }
      }
      cpu->ins_completed = cpu->code_memory_size - rest_code_size;
      cpu->stage[EX1] = cpu->stage[DRF];
      memset(cpu->stage,0,sizeof(CPU_Stage));
      cpu->stage[F].stalled = 1;
      cpu->stage[F].busy = 1;
    }

    /* Copy data from decode latch to execute latch*/
    if(cpu->stage[F].stalled == 0){
      cpu->stage[EX1] = cpu->stage[DRF];
    }
    else if(strcmp(stage->opcode, "HALT") != 0){
      memset(&cpu->stage[EX1],0,sizeof(CPU_Stage));
    }




  }
  if (ENABLE_DEBUG_MESSAGES) {
    print_stage_content("Decode/RF", stage);
  }
  return 0;
}

/*
 *  Execute Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
execute1(APEX_CPU* cpu) //keep rd's and z's status to invalid
{
  CPU_Stage* stage = &cpu->stage[EX1];
  if (!stage->busy && !stage->stalled) {

    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs_valid[stage->rd] = 0;
    }

    /* Store */
    else if (strcmp(stage->opcode, "STORE") == 0) {
       
    }

    else if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "STR") == 0) {
      
    }

    else if (strcmp(stage->opcode, "LDR") == 0){
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
  
    }

    else if(strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "MUL") == 0){
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 || strcmp(stage->opcode, "JUMP") == 0) {
      printf("%d",stage->rs1_value);
    }

    else if (strcmp(stage->opcode, "HALT") == 0) {
      cpu->stage[F].stalled = 1;
      cpu->stage[F].busy = 1;
      memset(&cpu->stage[DRF],0,sizeof(CPU_Stage));
    }
    /* Copy data from decode latch to execute latch*/
   
    cpu->stage[EX2] = cpu->stage[EX1];
    
    /* Copy data from Execute latch to Memory latch*/

    
  }
  if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute1", stage);
  }
  return 0;
}

int
execute2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[EX2];
  if (!stage->busy && !stage->stalled) {


    /* MOVC */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      stage->buffer = stage->imm;
      cpu->regs_valid[stage->rd] = 0;
    }

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      stage->mem_address = stage->rs2_value + stage->imm;
    }

    if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->mem_address = stage->rs1_value + stage->imm;
      cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "STR") == 0) {
      stage->mem_address = stage->rs2_value + stage->rs3_value;
    }

    if (strcmp(stage->opcode, "LDR") == 0){
      stage->mem_address = stage->rs1_value + stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
    }

    if (strcmp(stage->opcode, "ADDL") == 0 ) {
      stage->buffer = stage->rs1_value + stage->imm;
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    if(strcmp(stage->opcode, "SUBL") == 0){
      stage->buffer = stage->rs1_value - stage->imm;
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    if (strcmp(stage->opcode, "ADD") == 0 ) {
      stage->buffer = stage->rs1_value + stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    if(strcmp(stage->opcode, "SUB") == 0){

      stage->buffer = stage->rs1_value - stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    
    }

    if(strcmp(stage->opcode, "AND") == 0){
      stage->buffer = stage->rs1_value & stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
    }

    if(strcmp(stage->opcode, "OR") == 0){
      stage->buffer = stage->rs1_value | stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
    }

    if(strcmp(stage->opcode, "EX-OR") == 0){
      stage->buffer = stage->rs1_value ^ stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
    }

    if(strcmp(stage->opcode, "MUL") == 0){
      stage->buffer = stage->rs1_value * stage->rs2_value;
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    if (strcmp(stage->opcode, "BZ") == 0 ) {
      stage->buffer = stage->imm;
      if(stage->z_valid == 1){
        if(stage->z == 1){
          memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
          cpu->pc = stage->pc + stage->buffer;
          cpu->code_memory_size -= (stage->buffer/4);
          
          cpu->stage[F].busy = 1;
        }
      }
      else if(cpu->z == 1){
        memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
       
        cpu->pc = stage->pc + stage->buffer;
        cpu->code_memory_size -= (stage->buffer/4);
       
        cpu->stage[F].busy = 1;
      }
    }

    if(strcmp(stage->opcode, "BNZ") == 0){
      stage->buffer = stage->imm;
      if(stage->z_valid == 1){
        if(stage->z == 0){
          memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
          cpu->pc = stage->pc + stage->buffer;
          cpu->code_memory_size -= (stage->buffer/4);
          
          cpu->stage[F].busy = 1;
        }
      }
      else if(cpu->z != 1){
        memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
        cpu->pc = cpu->pc + stage->buffer;
        cpu->code_memory_size -= (stage->buffer/4);
        cpu->stage[F].busy = 1;
      }
    }

    if (strcmp(stage->opcode, "JUMP") == 0){
      stage->buffer = stage->rs1_value + stage->imm;

      memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
      //cpu->code_memory_size += (cpu->pc - stage->buffer)/4 -4;
      cpu->pc = stage->buffer;

      //printf("%d,%d",stage->rs1, stage->imm);
      cpu->stage[F].busy = 1;
    }

    if (strcmp(stage->opcode, "HALT") == 0) {
      memset(cpu->stage, 0, sizeof(CPU_Stage) * 3); //memset f,d,ex1 stage
      cpu->stage[F].stalled = 1;
      cpu->stage[F].busy = 1;
    }

    /* Copy data from Execute latch to Memory latch*/
    cpu->stage[MEM1] = cpu->stage[EX2];

    
  }
  if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Execute2", stage);
  }
  return 0;
}

/*
 *  Memory Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
memory1(APEX_CPU* cpu) // almost do nothing,keep rd's and z's status to invalid
{
  CPU_Stage* stage = &cpu->stage[MEM1];
  if (!stage->busy && !stage->stalled) {
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs_valid[stage->rd] = 0;
    }

    /* Store */
    else if (strcmp(stage->opcode, "STORE") == 0) {
       
    }

    else if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "STR") == 0) {
      
    }

    else if (strcmp(stage->opcode, "LDR") == 0){
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    else if(strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 || strcmp(stage->opcode, "MUL") == 0){
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 || strcmp(stage->opcode, "JUMP") == 0) {
      
    }

    else if (strcmp(stage->opcode, "HALT") == 0) {
    }
    /* Copy data from decode latch to execute latch*/
    cpu->stage[MEM2] = cpu->stage[MEM1];

    
  }
  if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory1", stage);
  }
  return 0;
}

int
memory2(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[MEM2];
  if (!stage->busy && !stage->stalled) {

    /* Store */
    if (strcmp(stage->opcode, "STORE") == 0) {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    /* MOVC */
    else if (strcmp(stage->opcode, "LOAD") == 0) {
      stage->buffer = cpu->data_memory[stage->mem_address]; //for forwarding
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "STR") == 0) {
      cpu->data_memory[stage->mem_address] = stage->rs1_value;
    }

    else if (strcmp(stage->opcode, "LDR") == 0) {
      stage->buffer = cpu->data_memory[stage->mem_address]; //for forwarding
      cpu->regs_valid[stage->rd] = 0;
    }

    else if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ) {
      cpu->regs_valid[stage->rd] = 0;
      cpu->z_valid = 0;
    }

    /* Copy data from decode latch to execute latch*/
    cpu->stage[WB] = cpu->stage[MEM2];

    
  }
  if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Memory2", stage);
  }
  return 0;
}

/*
 *  Writeback Stage of APEX Pipeline
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
writeback(APEX_CPU* cpu)
{
  CPU_Stage* stage = &cpu->stage[WB];
  if (!stage->busy && !stage->stalled) {

    /* Update register file */
    if (strcmp(stage->opcode, "MOVC") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    else if (strcmp(stage->opcode, "STORE") == 0) {
       
    }

    else if (strcmp(stage->opcode, "LOAD") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    else if (strcmp(stage->opcode, "STR") == 0) {
      
    }

    else if (strcmp(stage->opcode, "LDR") == 0){
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    else if (strcmp(stage->opcode, "ADDL") == 0 || strcmp(stage->opcode, "SUBL") == 0) {
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
      cpu->z_valid = 1;
      if (stage->buffer == 0){
        cpu->z = 1;
      }
      else{
        cpu->z = 0;
      }
    }

    else if (strcmp(stage->opcode, "ADD") == 0 || strcmp(stage->opcode, "SUB") == 0 ||strcmp(stage->opcode, "MUL") == 0 ) {
      cpu->regs_valid[stage->rd] = 1;
      cpu->regs[stage->rd] = stage->buffer;
      cpu->z_valid = 1;
      if (stage->buffer == 0){
        cpu->z = 1;
      }
      else{
        cpu->z = 0;
      }
    }

    else if(strcmp(stage->opcode, "AND") == 0 || strcmp(stage->opcode, "OR") == 0 || strcmp(stage->opcode, "EX-OR") == 0 ){
      cpu->regs[stage->rd] = stage->buffer;
      cpu->regs_valid[stage->rd] = 1;
    }

    else if (strcmp(stage->opcode, "BZ") == 0 || strcmp(stage->opcode, "BNZ") == 0 || strcmp(stage->opcode, "JUMP") == 0) {
      
    }
    
    
    //cpu->stage[FIN] = cpu->stage[WB];
    if(strcmp(stage->opcode, "") != 0){
      cpu->ins_completed++;
    }
    if(strcmp(stage->opcode, "HALT") == 0){
      cpu->end = 1;
    }
    printf("CPUpc : %d, code : %d",get_code_index(cpu->pc),cpu->code_memory_size);
    if(get_code_index(stage->pc) == cpu->code_memory_size -1 ){
      printf("opcode : %s, %d",stage->opcode, stage->rs1);
      printf("pc : %d, code : %d",get_code_index(stage->pc),cpu->code_memory_size);
      cpu->end = 1;
    }

  }
  if (ENABLE_DEBUG_MESSAGES) {
      print_stage_content("Writeback", stage);
  }
  return 0;
}

/*
 *  APEX CPU simulation loop
 *
 *  Note : You are free to edit this function according to your
 *                  implementation
 */
int
APEX_cpu_run(APEX_CPU* cpu, int mode, int cycle)
{
  if(mode == 0){ // stimulate
    ENABLE_DEBUG_MESSAGES = 0;
  }
  else if(mode == 1){ //display
    ENABLE_DEBUG_MESSAGES = 1;
  }
  while (1) {

    /* All the instructions committed, so exit */
    if (cpu->end == 1) {
      printf("(apex) >> Simulation Complete");
      break;
    }

    if(cycle < cpu->clock){
      break;
    }

    if (ENABLE_DEBUG_MESSAGES) {
      printf("--------------------------------\n");
      printf("Clock Cycle #: %d\n", cpu->clock);
      printf("--------------------------------\n");
    }

    

    writeback(cpu);
    memory2(cpu);
    memory1(cpu);
    execute2(cpu);
    execute1(cpu);
    decode(cpu);
    fetch(cpu);
    cpu->clock++;

  }
  display_reg_file(cpu);
  display_data_memory(cpu);
  return 0;
}


void display_reg_file(APEX_CPU* cpu){
  printf("=============== STATE OF ARCHITECTURAL REGISTER FILE ==========\n");
  for(int i=0; i<16; i++){
    if(cpu->regs_valid[i] == 1){
      printf("|     REG[%2d]    |      Value=%6d     |     Status=Valid      \n",i,cpu->regs[i]);
    } 
    else if(cpu->regs_valid[i] == 0){
      printf("|     REG[%2d]    |      Value=%6d     |     Status=Invalid    \n",i,cpu->regs[i]);
    } 
    else{
      printf("|     REG[%2d]    |      Value=%6d     |     Status=Valid      \n",i,cpu->regs[i]);
    }
  }
}

void display_data_memory(APEX_CPU* cpu){
  printf("============== STATE OF DATA MEMORY =============\n");
  for(int i=0; i<100; i++){
    printf("|     MEM[%2d]     |     Data Value = %6d     |\n",i,cpu->data_memory[i]);
  }
}