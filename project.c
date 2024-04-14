#include "spimcore.h"
#define MEMSIZE (16384)

/* ALU */
/* 10 Points */
void ALU(unsigned A, unsigned B, char ALUControl, unsigned *ALUresult, char *Zero) {
  switch (ALUControl) {
  case 0: // Z = A + B
    *ALUresult = A + B;
    break;
  case 1: // Z = A - B
    *ALUresult = A - B;
    break;
  case 2: // Z = if A < B then 1 else 0
    *ALUresult = (A < B) ? 1 : 0;
    break;
  case 3: // Z = if A < B (unsigned) then 1 else 0
    *ALUresult = ((unsigned)A < (unsigned)B) ? 1 : 0;
    break;
  case 4: // Z = A AND B
    *ALUresult = A & B;
    break;
  case 5: // Z = A OR B
    *ALUresult = A | B;
    break;
  case 6: // Z = B shifted left by 16 bits
    *ALUresult = B << 16;
    break;
  case 7: // Z = NOT A
    *ALUresult = ~A;
    break;
  default:
    // Handle unsupported ALU control codes
    break;
  }

  *Zero = (*ALUresult == 0) ? 1 : 0;
}

/* instruction fetch */
/* 10 Points */
int instruction_fetch(unsigned PC, unsigned *Mem, unsigned *instruction) {
  *instruction = Mem[PC >> 2];
  printf("Fetched instruction at address: %08x  Instruction: %08x\n", PC, *instruction);
  if (*instruction == 0x00000000) {
    printf("Halt condition reached\n");
    return 1; // Check for halt condition
  }
  return 0; // Successful fetch
}

/* instruction partition */
/* 10 Points */
void instruction_partition(unsigned instruction, unsigned *op, unsigned *r1, unsigned *r2, unsigned *r3, unsigned *funct, unsigned *offset, unsigned *jsec) {
  *op = (instruction >> 26) & 0x3F;        // bits 31-26
  printf("Extracted opcode: %02x\n", *op); // Debug statement
  *r1 = (instruction >> 21) & 0x1F;        // bits 25-21
  *r2 = (instruction >> 16) & 0x1F;        // bits 20-16
  *r3 = (instruction >> 11) & 0x1F;        // bits 15-11
  *funct = instruction & 0x3F;             // bits 5-0
  *offset = instruction & 0xFFFF;          // bits 15-0
  *jsec = instruction & 0x3FFFFFF;         // bits 25-0
}

/* instruction decode */
/* 15 Points */
int instruction_decode(unsigned op, struct_controls *controls) {
  printf("Decoding instruction with opcode: %02x\n", op);
  switch (op) {
  case 0x8: // addi instruction
    printf("Decoding addi instruction\n");
    controls->RegDst = 0;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 1;
    controls->RegWrite = 1;
    break;
  case 0x0: // add
    printf("Decoding add instruction\n");
    printf("Opcode matches: %02x\n", op);
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 0; // Set ALUOp for addition
    break;
  case 0x2a: // slt instruction
    printf("Decoding slt instruction\n");
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 2; // Set ALUOp for comparison
    break;
  case 0x2b: // sltu instruction
    printf("Decoding sltu instruction\n");
    controls->RegDst = 1;
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->MemWrite = 0;
    controls->RegWrite = 1;
    controls->ALUSrc = 0;
    controls->ALUOp = 3;
    break;
  case 0x23: // lw instruction
    printf("Decoding lw instruction\n");
    controls->RegDst = 0; // Assuming these instructions do not use Rd
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 1;
    controls->MemtoReg = 1;
    controls->ALUOp = 0; // ALUOp for calculating memory address
    controls->MemWrite = 0;
    controls->ALUSrc = 1; // Use immediate offset to calculate memory address
    controls->RegWrite = 1;
    break;
  case 0x02: // j instruction
    printf("Decoding j instruction\n");
    controls->RegDst = 0;
    controls->Jump = 1; // Jump instruction
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 0;
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;
    break;
  case 0x04: // beq instruction
             // Handle branch instruction
    printf("Decoding beq instruction\n");
    controls->RegDst = 2;
    controls->Jump = 0;
    controls->Branch = 1;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp = 1; // ALUOp for subtraction
    controls->MemWrite = 0;
    controls->ALUSrc = 0;
    controls->RegWrite = 0;
    break;
  case 0xf: // lui instruction
    printf("Decoding lui instruction\n");
    controls->RegDst =
        0; // The result of lui is stored in the destination register
    controls->Jump = 0;
    controls->Branch = 0;
    controls->MemRead = 0;
    controls->MemtoReg = 0;
    controls->ALUOp =
        6; // The ALU operation is shifting the immediate value left by 16 bits
    controls->MemWrite = 0;
    controls->ALUSrc =
        1; // The immediate value is the second operand for the ALU
    controls->RegWrite = 1; // Write to register
    break;
  default:
    // Handle unsupported instruction opcodes
    return 1; // Set halt condition
  }
  return 0; // No halt condition
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
  // Sign extend 16-bit offset to 32-bit
  if (offset & 0x8000) {
    *extended_value = offset | 0xFFFF0000;
  } else {
    *extended_value = offset;
  }
}

/* ALU operations */
/* 10 Points */
int ALU_operations(unsigned data1, unsigned data2, unsigned extended_value, unsigned funct, char ALUOp, char ALUSrc, unsigned *ALUresult, char *Zero) {
  printf("Performing ALU operations\n");
  printf("ALU input data1: %08x\n", data1);
  printf("ALU input data2: %08x\n", (ALUSrc == 1) ? extended_value : data2);

  if (ALUSrc == 1) {
    // Use data1 and extended_value
    ALU(data1, extended_value, ALUOp, ALUresult, Zero);
  } else {
    // Use data1 and data2
    ALU(data1, data2, ALUOp, ALUresult, Zero);
  }

  printf("ALU result: %08x\n", *ALUresult); // Print ALU result
  printf("Zero flag: %d\n", *Zero);         // Print Zero flag value

  return 0; // No halt condition
}

/* Read / Write Memory */
/* 10 Points */
int rw_memory(unsigned ALUresult, unsigned data2, char MemWrite, char MemRead, unsigned *memdata, unsigned *Mem) {
  // Print ALU result before accessing memory
  printf("ALU result: %08x\n", ALUresult);

  if (MemRead) {
    // Access memory using byte address provided by ALU
    *memdata = Mem[ALUresult >> 2];
    // Print memory access information
    printf("Accessing memory at byte address: %08x\n", ALUresult);
    printf("Memory data: %08x\n", *memdata);
  }

  if (MemWrite) {
    // Print memory write information
    printf("Writing data to memory at byte address: %08x\n", ALUresult);
    printf("Data to write: %08x\n", data2);
    Mem[ALUresult >> 2] = data2;
  }

  return 0; // No halt condition
}

/* Write Register */
/* 10 Points */
void write_register(unsigned r2, unsigned r3, unsigned memdata, unsigned ALUresult, char RegWrite, char RegDst, char MemtoReg, unsigned *Reg) {
  if (RegWrite) {
    if (MemtoReg) {
      Reg[r2] = memdata;
      printf("Wrote data to register $%d: %08x\n", r2, memdata);
    } else {
      Reg[(RegDst) ? r3 : r2] = ALUresult;
      printf("Wrote data to register $%d: %08x\n", (RegDst) ? r3 : r2, ALUresult);
    }
  }
}

/* PC update */
/* 10 Points */
void PC_update(unsigned jsec, unsigned extended_value, char Branch, char Jump, char Zero, unsigned *PC) {
  printf("Current PC: %08x\n", *PC);
  printf("Branching with Branch = %d, Zero = %d\n", Branch, Zero);
  if (Branch && Zero) {
    printf("Branching to target address: %08x\n", *PC);
    printf("Branching by extended value: %08x\n", (extended_value << 2));
    *PC += (extended_value << 2);
  } else if (Jump) {
    printf("Jumping to target address: %08x\n", (jsec << 2));
    *PC = (jsec << 2); // Set PC to jump target address directly
  } else {
    *PC += 4;
    printf("Incremented PC: %08x\n", *PC);
  }
}
