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


/***************************************************************/
/*                                                             */
/* Procedure: get_inst_info                                    */
/*                                                             */
/* Purpose: Read insturction information                       */
/*                                                             */
/***************************************************************/
instruction get_inst_info(uint32_t pc) { 
    return INST_INFO[(pc - MEM_TEXT_START) >> 2];
}

void Init(){
	CURRENT_STATE.LUHazard = FALSE;
	CURRENT_STATE.PIPE_STALL[ID_STAGE] = PRE_STATE.PIPE_STALL[IF_STAGE];
	CURRENT_STATE.PIPE_STALL[EX_STAGE] = PRE_STATE.PIPE_STALL[ID_STAGE];
	CURRENT_STATE.PIPE_STALL[MEM_STAGE] = PRE_STATE.PIPE_STALL[EX_STAGE];
	CURRENT_STATE.PIPE_STALL[WB_STAGE] = PRE_STATE.PIPE_STALL[MEM_STAGE];
}

void ID_Stage(){
	//printf("ID OK\n");
	//PIPE
	//PIPE_STALL
	//INST
	//ins
	//NPC

	//PIPE
	//PIPE_STALL
	//uint32_t ID_EX_NPC;
	//uint32_t ID_EX_REG1;
	//uint32_t ID_EX_REG2;
	//short ID_EX_IMM;
	//instruction* ID_EX_ins;
	CURRENT_STATE.JUMP_PC = 0;
	CURRENT_STATE.ID_EX_JUMP = FALSE;
	CURRENT_STATE.PIPE[ID_STAGE] = PRE_STATE.PIPE[IF_STAGE];
	//printf("%d\n",PRE_STATE.REGS[(PRE_STATE.IF_ID_ins).r_t.r_i.rs]);
	if (PRE_STATE.PIPE_STALL[ID_STAGE] == FALSE && CURRENT_STATE.BRANCH_PC == 0){
		CURRENT_STATE.ID_EX_REG1 = PRE_STATE.REGS[(PRE_STATE.IF_ID_ins).r_t.r_i.rs];
		CURRENT_STATE.ID_EX_REG2 = PRE_STATE.REGS[(PRE_STATE.IF_ID_ins).r_t.r_i.rt];
		CURRENT_STATE.ID_EX_IMM = (PRE_STATE.IF_ID_ins).r_t.r_i.r_i.imm;
		CURRENT_STATE.ID_EX_ins = PRE_STATE.IF_ID_ins;
		CURRENT_STATE.ID_EX_NPC = PRE_STATE.IF_ID_NPC;

		if (PRE_STATE.IF_ID_ins.opcode == 2) {
			CURRENT_STATE.JUMP_PC = PRE_STATE.IF_ID_ins.r_t.target*4;
			CURRENT_STATE.ID_EX_JUMP = TRUE;
		}
		if (PRE_STATE.IF_ID_ins.opcode == 3) {
			CURRENT_STATE.JUMP_PC = PRE_STATE.IF_ID_ins.r_t.target*4;
			if (END_BIT != TRUE)CURRENT_STATE.REGS[31] = PRE_STATE.IF_ID_NPC+4;
			CURRENT_STATE.ID_EX_JUMP = TRUE;
		}
		if (PRE_STATE.IF_ID_ins.opcode == 0 && PRE_STATE.IF_ID_ins.func_code == 8) {
			CURRENT_STATE.JUMP_PC = PRE_STATE.REGS[CURRENT_STATE.ID_EX_ins.r_t.r_i.rs];
			CURRENT_STATE.ID_EX_JUMP = TRUE;
		}
	}
}

void EX_Stage(){
	//printf("EX OK\n");
	//ID_EX_REG1;
	//ID_EX_REG2;
	//ID_EX_IMM;

	//EX_MEM_ALU_OUT;
	//EX_MEM_W_VALUE;
	//EX_MEM_BR_TARGET;
	//EX_MEM_BR_TAKE;
	CURRENT_STATE.PIPE[EX_STAGE] = PRE_STATE.PIPE[ID_STAGE];
	if (PRE_STATE.PIPE_STALL[EX_STAGE] == FALSE && CURRENT_STATE.BRANCH_PC == 0){
		uint32_t imm = PRE_STATE.ID_EX_IMM;
		CURRENT_STATE.EX_MEM_IMM = imm;
		int rt = PRE_STATE.ID_EX_ins.r_t.r_i.rt;
		int rs = PRE_STATE.ID_EX_ins.r_t.r_i.rs;
		uint32_t s = PRE_STATE.REGS[rs];
		uint32_t t = PRE_STATE.REGS[rt];
		int rd = PRE_STATE.ID_EX_ins.r_t.r_i.r_i.r.rd;
		int h = PRE_STATE.ID_EX_ins.r_t.r_i.r_i.r.shamt;
		uint32_t out = 0;
		switch (PRE_STATE.ID_EX_ins.opcode){
			//Type I
			case 0x9:		//(0x001001)ADDIU
				out = s + imm;
				if (END_BIT != TRUE)CURRENT_STATE.REGS[rt] = out;
				break;
			case 0xc:		//(0x001100)ANDI
				out = s & imm;
				if (END_BIT != TRUE)CURRENT_STATE.REGS[rt] = out;
				break;
			case 0xf:		//(0x001111)LUI
				out = imm << 16;
				if (END_BIT != TRUE)CURRENT_STATE.REGS[rt] = out;
				break;
			case 0xd:		//(0x001101)ORI
				out = s | imm;
				if (END_BIT != TRUE)CURRENT_STATE.REGS[rt] = out;
				break;
			case 0xb:		//(0x001011)SLTIU
				if (imm < 0) imm = (~imm)+1;
				out = (s < imm);
				//printf("\nSLTIU: %d %d\n",s,imm);
				if (END_BIT != TRUE)CURRENT_STATE.REGS[rt] = out;
				break;
			case 0x23:		//(0x100011)LW	
				out = s + imm;
				break;
			case 0x2b:		//(0x101011)SW ....
				out = s + imm;
				break;
			case 0x4:		//(0x000100)BEQ
				//rintf("\nBEQ: %d %d\n",rs,rt);
				CURRENT_STATE.EX_MEM_BR_TAKE = (s==t);
				CURRENT_STATE.EX_MEM_BR_TARGET = PRE_STATE.ID_EX_NPC + imm*4;
				break;
			case 0x5:		//(0x000101)BNE 
				CURRENT_STATE.EX_MEM_BR_TAKE = (s!=t);
				CURRENT_STATE.EX_MEM_BR_TARGET = PRE_STATE.ID_EX_NPC + imm*4;
				break;
			case 2:
			case 3:
				CURRENT_STATE.PIPE_STALL[EX_STAGE] = TRUE;
				break;

			//TYPE R
			case 0x0:		//(0x000000)ADDU, AND, NOR, OR, SLTU, SLL, SRL, SUBU, JR
				switch (PRE_STATE.ID_EX_ins.func_code){
					case 33: 	//addu
						out = s + t;
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 36: 	//and
						out = s & t;
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 39: 	//nor
						out = ~(s | t);
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 37: 	//or
						out = s | t;
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 43: 	//sltu
						out = (s < t);
						//printf("\nSLTU: %d %d\n",s,t);
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 0: 	//sll
						out = t << h;
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 2: 	//srl
						out = t >> h;
						//printf("\nSRL: %d %d\n",t,out);
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 35: 	//subu
						out = s - t;
						//if (s<0) s= (~s)+1;
						//if (t<0) t= (~t)+1;
						if (END_BIT != TRUE)CURRENT_STATE.REGS[rd] = out;
						break;
					case 8:
						CURRENT_STATE.PIPE_STALL[EX_STAGE] = TRUE;
						break;
				}
				break;
		}
		CURRENT_STATE.EX_MEM_ALU_OUT = out;
		CURRENT_STATE.EX_MEM_NPC = PRE_STATE.ID_EX_NPC;
		CURRENT_STATE.EX_MEM_ins = PRE_STATE.ID_EX_ins;

		//LOAD USE HAZARD
		if (PRE_STATE.ID_EX_ins.opcode == 0x23) {
			//Register t
			if (PRE_STATE.IF_ID_ins.r_t.r_i.rt == PRE_STATE.ID_EX_ins.r_t.r_i.rt)
				if (PRE_STATE.IF_ID_ins.opcode == 0 && (PRE_STATE.IF_ID_ins.func_code == 0 || PRE_STATE.IF_ID_ins.func_code == 2)){
					CURRENT_STATE.LUHazard = TRUE;
				}

			//Register s & t
			if (PRE_STATE.IF_ID_ins.r_t.r_i.rt == PRE_STATE.ID_EX_ins.r_t.r_i.rt || PRE_STATE.IF_ID_ins.r_t.r_i.rs == PRE_STATE.ID_EX_ins.r_t.r_i.rt)
				if (PRE_STATE.IF_ID_ins.opcode == 0){
					switch (PRE_STATE.IF_ID_ins.func_code){
						case 33:
						case 36:
						case 39:
						case 37:
						case 41:
						case 35:
							CURRENT_STATE.LUHazard = TRUE;
							break;
					}
				}

			//Register s
			if (PRE_STATE.IF_ID_ins.r_t.r_i.rs == PRE_STATE.ID_EX_ins.r_t.r_i.rt){
				switch (PRE_STATE.IF_ID_ins.opcode){
					case 0xb:
					case 0xc:
					case 0xd:
					case 0x9:
						CURRENT_STATE.LUHazard = TRUE;
						break;
				}
			}
		}
	}
}

void  MEM_Stage(){
	//printf("MEM OK\n");
	//uint32_t EX_MEM_ALU_OUT;
	//uint32_t EX_MEM_W_VALUE;
	//uint32_t EX_MEM_BR_TARGET;
	//uint32_t EX_MEM_BR_TAKE;
	//EX_MEM_IMM

	//uint32_t MEM_WB_ALU_OUT;
	//uint32_t MEM_WB_MEM_OUT;
	CURRENT_STATE.PIPE[MEM_STAGE] = PRE_STATE.PIPE[EX_STAGE];
	CURRENT_STATE.BRANCH_PC = 0;
	if (PRE_STATE.PIPE_STALL[MEM_STAGE] == FALSE){
		CURRENT_STATE.MEM_WB_NPC = PRE_STATE.EX_MEM_NPC;
		CURRENT_STATE.MEM_WB_ins = PRE_STATE.EX_MEM_ins;
		CURRENT_STATE.MEM_WB_ALU_OUT = PRE_STATE.EX_MEM_ALU_OUT;
		uint32_t wdata = PRE_STATE.REGS[PRE_STATE.EX_MEM_ins.r_t.r_i.rt];

		//SW
		if (PRE_STATE.EX_MEM_ins.opcode == 0x2b){
			mem_write_32(PRE_STATE.EX_MEM_ALU_OUT, wdata);
		}

		//Branch handle
		if ((PRE_STATE.EX_MEM_ins.opcode == 4 || PRE_STATE.EX_MEM_ins.opcode == 5) && PRE_STATE.EX_MEM_BR_TAKE){
			CURRENT_STATE.BRANCH_PC = (uint32_t) (PRE_STATE.EX_MEM_IMM)*4 + PRE_STATE.EX_MEM_NPC;
			//printf("\nMEM %d\n",CURRENT_STATE.PIPE[MEM_STAGE]-MEM_TEXT_START);
			CURRENT_STATE.PIPE_STALL[ID_STAGE] = TRUE;
			CURRENT_STATE.PIPE_STALL[EX_STAGE] = TRUE;
			CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
		}

		if (PRE_STATE.LUHazard == TRUE){
			CURRENT_STATE.PIPE_STALL[MEM_STAGE] = TRUE;
		}

		//Forwarding
		if (PRE_STATE.EX_MEM_ins.opcode == 0x23){
			CURRENT_STATE.MEM_WB_MEM_OUT = mem_read_32(PRE_STATE.EX_MEM_ALU_OUT);
			if (END_BIT != TRUE) CURRENT_STATE.REGS[PRE_STATE.EX_MEM_ins.r_t.r_i.rt] = CURRENT_STATE.MEM_WB_MEM_OUT; 
		}

		if (INSTRUCTION_COUNT == MAX_INSTRUCTION_NUM - 1 ) END_BIT = TRUE;
	}
}

void WB_Stage(){
	//printf("WB OK\n");
	CURRENT_STATE.PIPE[WB_STAGE] = PRE_STATE.PIPE[MEM_STAGE];
	if (PRE_STATE.PIPE_STALL[WB_STAGE] == FALSE) {
		INSTRUCTION_COUNT++;
	}
	
}

void IF_Stage(){
	//printf("IF OK\n");
	//printf("\n%d %d\n",(PRE_STATE.PC - MEM_TEXT_START)>>2, NUM_INST);
	if (((PRE_STATE.PC - MEM_TEXT_START)>>2) >= NUM_INST || FETCH_BIT == FALSE){
			FETCH_BIT = FALSE;
			CURRENT_STATE.PIPE_STALL[ID_STAGE] = TRUE;
			return;
	}
	if (FETCH_BIT == TRUE){
		//uint32_t IF_ID_I
		//uint32_t IF_ID_NPC;
		//instruction* IF_ID_ins;
		//Load-use detect

		CURRENT_STATE.IF_ID_ins = get_inst_info(PRE_STATE.PC);
		CURRENT_STATE.IF_ID_NPC = PRE_STATE.PC + 4;
		CURRENT_STATE.PIPE[IF_STAGE] = PRE_STATE.PC;
		//if (CYCLE_COUNT == 13) printf("Br PC %d\n",CURRENT_STATE.BRANCH_PC - MEM_TEXT_START);
		//if (CURRENT_STATE.IF_ID_ins.opcode == 5) printf("\nIF %d\n",CURRENT_STATE.PIPE[IF_STAGE]-MEM_TEXT_START);

		if (CURRENT_STATE.BRANCH_PC !=0){
			CURRENT_STATE.PC = CURRENT_STATE.BRANCH_PC;
			//printf("PC branching\n");
		} else if (CURRENT_STATE.JUMP_PC != 0){
			CURRENT_STATE.PC = CURRENT_STATE.JUMP_PC;
			//printf("PC jump \n");
		} else{
			CURRENT_STATE.PC = PRE_STATE.PC + 4;
			//printf("PC normal \n");
		}
		//printf("\n%d %d\n",(PRE_STATE.PC - MEM_TEXT_START)>>2, NUM_INST);
	}
}

/***************************************************************/
/*                                                             */
/* Procedure: process_instruction                              */
/*                                                             */
/* Purpose: Process one instrction                             */
/*                                                             */
/***************************************************************/
void process_instruction(){	
	PRE_STATE = CURRENT_STATE;
	Init();
	if (CYCLE_COUNT>3) WB_Stage();
	if (CYCLE_COUNT>2) MEM_Stage();
	if (CYCLE_COUNT>1) EX_Stage();
	if (CYCLE_COUNT>0 && PRE_STATE.LUHazard == FALSE) ID_Stage();
	if (PRE_STATE.LUHazard == FALSE) IF_Stage();
	int k;
	int s;
	s=0;
	for(k = 0; k < 5; k++)
		if (CURRENT_STATE.PIPE_STALL[k]==TRUE) {
			CURRENT_STATE.PIPE[(k+4)%5] =FALSE;
			s++;
		}
	if (s>=4) RUN_BIT=FALSE;
}







