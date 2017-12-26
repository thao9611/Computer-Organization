/***************************************************************/
/*                                                             */
/*   MIPS-32 Instruction Level Simulator                       */
/*                                                             */
/*   CS311 KAIST                                               */
/*   run.c                                                     */
/*                                                             */
/***************************************************************/

#include <stdio.h>

#include "util.h"
#include "run.h"

//int nextPC = MEM_TEXT_START+4;

/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction* get_inst_info(uint32_t pc) { 
    return &INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

void process_j(instruction* instr){
	if (instr->opcode == 2 ){ // jump instruction
		CURRENT_STATE.PC = (instr->r_t.target)*4;
	}
	else {// jal
		CURRENT_STATE.REGS[31] = CURRENT_STATE.PC + 8;
		CURRENT_STATE.PC 	= (instr->r_t.target)*4;
		//printf("DM: %d\n", CURRENT_STATE.PC);
	}
}

void process_r(instruction* instr){
	int jr = CURRENT_STATE.PC + 4;
	short opcode = instr->func_code;
	unsigned char rs= instr->r_t.r_i.rs;
	unsigned char rt = instr->r_t.r_i.rt;
	unsigned char rd = instr->r_t.r_i.r_i.r.rd;
	unsigned char shamt = instr->r_t.r_i.r_i.r.shamt;
	if ( opcode == 33){//ADDU
		CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]+ CURRENT_STATE.REGS[rt];
	}
	else if(opcode == 8){//JR
		jr = CURRENT_STATE.REGS[rs]; 
		//printf("DM: %d %d\n", rs, jr);
	}
	else if (opcode == 37){//OR
		CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]|CURRENT_STATE.REGS[rt];
	}
	else if(opcode == 43){//SLTU
		if (CURRENT_STATE.REGS[rs] < CURRENT_STATE.REGS[rt]) {
			CURRENT_STATE.REGS[rd] =1;
		}
		else {
			CURRENT_STATE.REGS[rd] = 0;
		}
	}
	else if (opcode ==39 ){//NOR
		CURRENT_STATE.REGS[rd] = ~(CURRENT_STATE.REGS[rs]|CURRENT_STATE.REGS[rt]);
	}
	else if (opcode == 0){//SLL
		CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt]<<shamt;
	}
	else if( opcode == 2){// SRL
		CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rt]>>shamt;
	}
	else if ( opcode == 36){
		CURRENT_STATE.REGS[rd] = (CURRENT_STATE.REGS[rs]&CURRENT_STATE.REGS[rt]);
	}
	else{
		CURRENT_STATE.REGS[rd] = CURRENT_STATE.REGS[rs]- CURRENT_STATE.REGS[rt];
	}
	CURRENT_STATE.PC = jr;
}

void process_i(instruction* instr){
	int offset = 4;
	short opcode = instr->opcode;
	unsigned char rs = instr->r_t.r_i.rs;
	unsigned char rt = instr->r_t.r_i.rt;
	short imm = instr->r_t.r_i.r_i.imm;
	if (opcode == 4){
		if (CURRENT_STATE.REGS[rs] == CURRENT_STATE.REGS[rt]) offset += imm*4;
	}
	if (opcode == 5){
		if (CURRENT_STATE.REGS[rs] != CURRENT_STATE.REGS[rt]) offset += imm*4;
	}
	if (opcode == 43){
		mem_write_32(CURRENT_STATE.REGS[rs]+imm,CURRENT_STATE.REGS[rt]);
	}
	if (opcode == 35){
		CURRENT_STATE.REGS[rt] = mem_read_32(CURRENT_STATE.REGS[rs]+imm);
	}
	if (opcode == 11){
		if (CURRENT_STATE.REGS[rs]<imm) CURRENT_STATE.REGS[rt]=1; else CURRENT_STATE.REGS[rt]=0;
	}
	if (opcode == 9){
		CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] + imm;
	}
	if (opcode == 12){
		CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] & imm;
	}
	if (opcode == 13){
		CURRENT_STATE.REGS[rt] = CURRENT_STATE.REGS[rs] | imm;
	}
	if (opcode ==  15){
		CURRENT_STATE.REGS[rt] = imm << 16;
	}
	CURRENT_STATE.PC += offset;
}


/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
/*The `process_instruction()` function is used by the `cycle()` instruction to execute the instruction at the current PC.
Your internal register/memory state should be changed according to the instruction that is pointed to by the current PC.
*/
void process_instruction(){
	/** Implement this function */
	instruction* instr = get_inst_info(CURRENT_STATE.PC);
	short opcode = instr->opcode;
	short f_code = instr->func_code;
	if (((CURRENT_STATE.PC - MEM_TEXT_START) >> 2) >= NUM_INST) {
		RUN_BIT=FALSE;
		return;
	}
	//printf("\nDM: %d %d %d \n", instr->value, NUM_INST, (CURRENT_STATE.PC - MEM_TEXT_START) >> 2);
	if (opcode == 2 || opcode == 3){ // J type
		process_j(instr);
	} else if (opcode == 0){ //R type
		process_r(instr);
	} else { // I type
		process_i(instr);
	}
}

