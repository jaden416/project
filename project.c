#include "spimcore.h"
#define MEMSIZE (65536 >> 2)

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult,
         char *Zero) {
  unsigned Z;

  switch (ALUControl) {
  case '0':
    // Z = A + B
    Z = A + B;
    break;

  case '1':
    // Z = A – B
    Z = A - B;
    break;

  case '2':
    // if A < B, Z = 1; otherwise, Z = 0
    Z = (A < B) ? 1 : 0;
    break;

  case '3':
    // if A < B, Z = 1; otherwise, Z = 0 (A and B are unsigned integers)
    Z = (A < B) ? 1 : 0;
    break;

  case '4':
    // Z = A AND B
    Z = A && B;
    break;

  case '5':
    // Z = A OR B
    Z = A || B;
    break;

  case '6':
    // Z = Shift B left by 16 bits
    Z = B << 16;
    break;

  case '7':
    // Z = NOT A
    Z = !A;
    break;
  }

  *ALUresult = Z;

  *Zero = (!ALUresult) ? 0 : 1;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
  // check if PC is within bounds of memory
  if (PC < 0 || PC >= MEMSIZE * 4) {
    printf("Memory access violation: PC = %u\n", PC);
    return 1;
  }

  *instruction = Mem[PC >> 2];

  if (*instruction == 0X00000000) {
    printf("Halt instruction encountered: PC = %u\n", PC);
    return 1;
  }

  return 0;
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
  *op = (instruction >> 26) & 0x3F; // bits 31-26
  *r1 = (instruction >> 21) & 0x1F; // bits 25-21
  *r2 = (instruction >> 16) & 0x1F; // bits 20-16
  *r3 = (instruction >> 11) & 0x1F; // bits 15-11
  *funct = instruction & 0x3F;      // bits 5-0
  *offset = instruction & 0xFFFF;   // bits 15-0
  *jsec = instruction & 0x3FFFFFF;  // bits 25-0
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
  controls->RegDst = 2;
  controls->Jump = 2;
  controls->Branch = 2;
  controls->MemRead = 2;
  controls->MemtoReg = 2;
  controls->ALUOp = 0;
  controls->MemWrite = 2;
  controls->ALUSrc = 2;
  controls->RegWrite = 2;

  switch (op) {
  case 0x00: // R-type
    controls->RegDst = 1;
    controls->RegWrite = 1;
    controls->ALUOp = 7;
    break;
  case 0x08: // addi

  case 0x09: // addiu

  case 0x0C: // andi

  case 0x0D: // ori

  case 0x0E: // xori
    controls->RegWrite = 1;
    controls->ALUSrc = 1;
    controls->ALUOp = 0; // addition
  break;

  case 0x04: // beq

  case 0x05: // bne
    controls->Branch = 1;
    controls->ALUOp = 1; // subtraction
  break;

  case 0x23: // lw
    controls->ALUSrc = 1;
    controls->MemRead = 1;
    controls->RegWrite = 1;
    controls->MemtoReg = 1;
    controls->ALUOp = 0; // addition
  break;

  case 0x2B: // sw
    controls->ALUSrc = 1;
    controls->MemWrite = 1;
    controls->ALUOp = 0; // addition
  break;

  default:
    printf("Illegal instruction encountered: opcode = %x\n", op);
    return 1;
  }

  return 0;
}

/* Read Register */
/* 5 Points */
void read_register(unsigned r1, unsigned r2, unsigned *Reg, unsigned *data1, unsigned *data2) {
  *data1 = Reg[r1];
  *data2 = Reg[r2];
}

/* Sign Extend */
/* 10 Points */
void sign_extend(unsigned offset, unsigned *extended_value) {
  *extended_value = offset & 0x8000 ? offset | 0xFFFF0000 : offset;
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
  unsigned A = data1;
  unsigned B = ALUSrc ? extended_value : data2;
  char ALUControl = 0;

  switch (ALUOp) {
  case 0: // addition
    ALUControl = '0';
    break;
  case 1: // subtraction
    ALUControl = '1';
    break;
  case 2: // set less than
    ALUControl = '2';
    break;
  case 3: // set less than unsigned
    ALUControl = '3';
    break;
  case 4: // AND
    ALUControl = '4';
    break;
  case 5: // OR
    ALUControl = '5';
    break;
  case 6: // shift left
    ALUControl = '6';
    break;
  case 7: // R-type
    switch (funct) {
  case 0x20: // add
    ALUControl = '0';
    break;
  case 0x21: // addu
    ALUControl = '0';
    break;
  case 0x22: // sub
    ALUControl = '1';
    break;
  case 0x24: // and
    ALUControl = '4';
    break;
  case 0x25: // or
    ALUControl = '5';
    break;
  case 0x27: // nor
    ALUControl = '5';
    *ALUresult = ~(*ALUresult);
    break;
  case 0x2A: // slt
    ALUControl = '2';
    break;
  case 0x2B: // sltu
    ALUControl = '3';
    break;
  default:
    printf("Illegal instruction encountered: funct = %x\n", funct);
    return 1;
  }
    break;
  default:
    printf("Illegal ALUOp encountered: %d\n", ALUOp);
    return 1;
  }

  ALU(A, B, ALUControl, ALUresult, Zero);

  return 0;
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
  unsigned address = ALUresult >> 2;

  if (address >= MEMSIZE) {
    printf("Memory access violation: address = %x\n", ALUresult);
    return 1;
  }

  if (MemWrite) {
    Mem[address] = data2;
  } else if (MemRead) {
    *memdata = Mem[address];
  }

  return 0;
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg) {
  if (RegWrite) {
    unsigned regnum = RegDst ? r3 : r2;
    unsigned data = MemtoReg ? memdata : ALUresult;
    Reg[regnum] = data;
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
  unsigned target_address;

  if (Branch && Zero) {
    target_address = *PC + 4 + (extended_value << 2);
    if (target_address < 0x4000 || target_address >= MEMSIZE * 4) {
      printf("Illegal branch target address: 0x%08x\n", target_address);
      *PC = 0; // Set PC to 0 to halt the simulation
      return;
    }
    *PC = target_address;
  } else if (Jump) {
    target_address = ((*PC + 4) & 0xF0000000) | (jsec << 2);
    if (target_address < 0x4000 || target_address >= MEMSIZE * 4) {
      printf("Illegal jump target address: 0x%08x\n", target_address);
      *PC = 0; // Set PC to 0 to halt the simulation
      return;
    }
    *PC = target_address;
  } else {
    *PC += 4;
  }
}
