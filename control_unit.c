/********************************************************************************
* control_unit.c: Contains static variables and function definitions for 
*                 implementation of an 8-bit control unit.
********************************************************************************/
#include "control_unit.h"

/* Static variables: */
static uint32_t ir; /* Instruction register, stores next instruction to execute. */
static uint8_t pc;  /* Program counter, stores address to next instruction to fetch. */
static uint8_t mar; /* Memory address register, stores address for current instruction. */
static uint8_t sr;  /* Status register, stores status bits ISNZVC. */

static uint8_t op_code; /* Stores OP-code, for example LDI, OUT, JMP etc. */
static uint8_t op1;     /* Stores first operand, most often a destination. */
static uint8_t op2;     /* Stores second operand, most often a value or read address. */

static enum cpu_state state;                    /* Stores current state. */
static uint8_t reg[CPU_REGISTER_ADDRESS_WIDTH]; /* CPU-registers R0 - R31. */

/********************************************************************************
* control_unit_reset: Resets control unit registers and corresponding program.
********************************************************************************/
void control_unit_reset(void)
{
   ir = 0x00;
   pc = 0x00;
   mar = 0x00;
   sr = 0x00;

   op_code = 0x00;
   op1 = 0x00;
   op2 = 0x00;

   state = CPU_STATE_FETCH;

   for (uint8_t i = 0; i < CPU_REGISTER_ADDRESS_WIDTH; ++i)
   {
      reg[i] = 0x00;
   }

   data_memory_reset();
   stack_reset();
   program_memory_write();
   return;
}

/********************************************************************************
* control_unit_run_next_state: Runs next state in the CPU instruction cycle:
********************************************************************************/
void control_unit_run_next_state(void)
{
   switch (state)
   {
      case CPU_STATE_FETCH:
      {
         ir = program_memory_read(pc); /* Fetches next instruction. */
         mar = pc;                     /* Stores address of current instruction. */
         pc++;                         /* Program counter points to next instruction. */
         state = CPU_STATE_DECODE;     /* Decodes the instruction during next clock cycle. */
         break;
      }
      case CPU_STATE_DECODE:
      {        
         op_code = ir >> 16;           /* Bit 23 downto 16 consists of the OP code. */
         op1 = ir >> 8;                /* Bit 15 downto 8 consists of the first operand. */
         op2 = ir;                     /* Bit 7 downto 0 consists of the second operand. */
         state = CPU_STATE_EXECUTE;    /* Executes the instruction during next clock cycle. */
         break;
      }
      case CPU_STATE_EXECUTE:
      {
         switch (op_code) /* Checks the OP code.*/
         {
         case NOP: /* No operation => do nothing. */
         {
            break; 
         }
         case LDI: /* Loads constant into specified CPU register. */
         {
            reg[op1] = op2; 
            break;
         }
         case MOV: /* Copies value to specified CPU register. */
         {
            reg[op1] = reg[op2]; 
            break;
         }
         case OUT: /* Writes value to I/O-location in data memory. */
         {
            data_memory_write(op1, reg[op2]); 
            break;
         }
         case IN: /* Reads value from I/O-location in data memory. */
         {
            reg[op1] = data_memory_read(op2); 
            break;
         }
         case STS: /* Stores value to data memory. */
         {
            data_memory_write(op1 + 256, reg[op2]);
            break;
         }
         case LDS: /* Loads value from data memory. */
         {
            reg[op1] = data_memory_read(op2 + 256); 
            break;
         }
         case CLR: /* Clears content of CPU register. */
         {
            reg[op1] = 0x00;
            break;
         }
         case ORI: /* Performs bitwise OR with a constant. */
         {
            reg[op1] = alu(OR, reg[op1], op2, &sr);
            break;
         }
         case ANDI: /* Performs bitwise AND with a constant. */
         {
            reg[op1] = alu(AND, reg[op1], op2, &sr);
            break;
         }
         case XORI: /* Performs bitwise XOR with a constant. */
         {
            reg[op1] = alu(XOR, reg[op1], op2, &sr);
            break;
         }
         case OR: /* Performs bitwise OR with content in CPU register. */
         {
            reg[op1] = alu(OR, reg[op1], reg[op2], &sr);
            break;
         }
         case AND: /* Performs bitwise AND with content in CPU register. */
         {
            reg[op1] = alu(AND, reg[op1], reg[op2], &sr);
            break;
         }
         case XOR: /* Performs bitwise AND with content in CPU register. */
         {
            reg[op1] = alu(XOR, reg[op1], reg[op2], &sr);
            break;
         }
         case ADDI: /* Performs addition with a constant. */
         {
            reg[op1] = alu(ADD, reg[op1], op2, &sr);
            break;
         }
         case SUBI: /* Performs subtraction with a constant. */
         {
            reg[op1] = alu(SUB, reg[op1], op2, &sr);
            break;
         }
         case ADD: /* Performs addition with a CPU register. */
         {
            reg[op1] = alu(ADD, reg[op1], reg[op2], &sr);
            break;
         }
         case SUB: /* Performs subtraction with a CPU register. */
         {
            reg[op1] = alu(SUB, reg[op1], reg[op2], &sr);
            break;
         }
         case INC: /* Increments content of a CPU register. */
         {
            reg[op1] = alu(ADD, reg[op1], 1, &sr);
            break;
         }
         case DEC: /* Decrements content of a CPU register. */
         {
            reg[op1] = alu(SUB, reg[op1], 1, &sr);
            break;
         }
         case CPI: /* Compares content between CPU register with a constant. */
         {
            (void)alu(SUB, reg[op1], op2, &sr); /* Return value is not stored. */
            break;
         }
         case CP: /* Compares content between two CPU registers. */
         {
            (void)alu(SUB, reg[op1], reg[op2], &sr); /* Return value is not stored. */
            break;
         }
         case JMP: /* Jumps to specified address. */
         {
            pc = op1; 
            break;
         }
         case BREQ: /* Branches to specified address i Z flag is set. */
         {
            if (read(sr, Z)) pc = op1;
            break;
         }
         case BRNE: /* Branches to specified address if Z flag is cleared. */
         {
            if (!read(sr, Z)) pc = op1;
            break;
         }
         case BRGE: /* Branches to specified address if S flag is cleared. */
         {
            if (!read(sr, S)) pc = op1;
            break;
         }
         case BRGT: /* Branches to specified address if both S and Z flags are cleared. */
         {
            if (!read(sr, S) && !read(sr, Z)) pc = op1;
            break;
         }
         case BRLE: /* Branches to specified address if S or Z flag is set. */
         {
            if (read(sr, S) || read(sr, Z)) pc = op1;
            break;
         }
         case BRLT: /* Branches to specified address if S flag is set. */
         {
            if (read(sr, S)) pc = op1;
            break;
         }
         case CALL: /* Stores the return address on the stack and jumps to specified address. */ 
         {
            stack_push(pc); 
            pc = op1;       
            break;
         }
         case RET: /* Jumps to return address stored on the stack. */
         {
            pc = stack_pop(); 
            break;
         }
         case PUSH: /* Stores content of specified CPU register on the stack. */
         {
            stack_push(reg[op1]); 
            break;
         }
         case POP: /* Loads value from the stack to a CPU-register. */
         {
            reg[op1] = stack_pop(); 
            break;
         }
         case LSL: /* Shifts content of CPU register on step to the left. */
         {
            reg[op1] = reg[op1] << 1;
            break;
         }
         case LSR: /* Shifts content of CPU register on step to the right. */
         {
            reg[op1] = reg[op1] >> 1;
            break;
         }
         default: /* System reset if error occurs. */
         {
            control_unit_reset(); 
            break;
         }
         }

         state = CPU_STATE_FETCH; /* Fetches next instruction during next clock cycle. */
         break;
      }
      default: /* System reset if error occurs. */
      {
         control_unit_reset();
         break;
      }
   }
   return;
}

/********************************************************************************
* control_unit_run_next_state: Runs next CPU instruction cycle, i.e. fetches
*                              a new instruction from program memory, decodes
*                              and executes it.
********************************************************************************/
void control_unit_run_next_instruction_cycle(void)
{
   do
   {
      control_unit_run_next_state();
   } while (state != CPU_STATE_EXECUTE);
   return;
}

/********************************************************************************
* control_unit_print: Prints information about the processor, for instance
*                     current subroutine, instruction, state, content in
*                     CPU-registers and I/O registers DDRB, PORTB and PINB.
********************************************************************************/
void control_unit_print(void)
{
   printf("--------------------------------------------------------------------------------\n");
   printf("Current subroutine:\t\t\t\t%s\n", program_memory_subroutine_name(mar));
   printf("Current instruction:\t\t\t\t%s\n", cpu_instruction_name(op_code));
   printf("Current state:\t\t\t\t\t%s\n", cpu_state_name(state));

   printf("Program counter:\t\t\t\t%hu\n", pc);
   printf("Stack pointer:\t\t\t\t\t%hu\n", stack_pointer());
   printf("Last element added to the stack:\t\t%hu\n\n", stack_last_added_element());

   printf("Instruction register:\t\t\t\t%s ", get_binary((ir >> 16) & 0xFF, 8));
   printf("%s ", get_binary((ir >> 8) & 0xFF, 8));
   printf("%s\n", get_binary(ir & 0xFF, 8));

   printf("Status register (ISNZVC):\t\t\t%s\n\n", get_binary(sr, 6));

   printf("Content in CPU register R16:\t\t\t%s\n", get_binary(reg[R16], 8));
   printf("Content in CPU register R17:\t\t\t%s\n", get_binary(reg[R17], 8));
   printf("Content in CPU register R18:\t\t\t%s\n", get_binary(reg[R18], 8));
   printf("Content in CPU register R24:\t\t\t%s\n\n", get_binary(reg[R24], 8));

   printf("Content in data direction register DDRB:\t%s\n", get_binary(data_memory_read(DDRB), 8));
   printf("Content in data register PORTB:\t\t\t%s\n", get_binary(data_memory_read(PORTB), 8));
   printf("Content in pin input register PINB:\t\t%s\n", get_binary(data_memory_read(PINB), 8));

   printf("--------------------------------------------------------------------------------\n\n");
   return;
}


