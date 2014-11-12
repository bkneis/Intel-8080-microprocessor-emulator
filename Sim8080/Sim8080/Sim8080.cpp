/*
 * Author: Bryan Kneis
 * Created: 02/10/2013 
 * Revised: 26/11/2013 - Fixed the STA instruction
 * Description: Emulates the Intel 8080 Processor
 * User advice: Use the comments if any code is not understood and review the intel 8080 book for further help
*/

/* Lesson Schedule
  
 * Week 1 - PRACTICAL: MOV, MVI, LXI, LDA, LHLD
		    OWN TIME: STA, SHLD, LDAX, STAX, HCHG, SPHL
			IMPLEMENTED: ALL
 * Week 2 - PRACTICAL: Flags, ADI, ACI, CPI
			OWN TIME: SUI, SBI, ANI, ORI, XRI
			IMPLEMENTED: ALL EXECPT ORI AND XRI
 * Week 3 - PRACTICAL: PUSH, POP, CALL, RET
			OWN TIME: ORI, XRI, CNZ, CZ, CNC, CC, CPO, CPE, CP, CM, RNZ, RZ, RNC, RC, RPO, RPE, RP, RM
			IMPLEMENTED: ALL
 * Week 4 - PRACTICAL: INR, DCR, JMP
			OWN TIME: INX, DCX, JNZ, JZ, JNC, JC, JPO, JPE, JP, JM
			IMPLEMENTED: All EXECPT JP AND JM
 * Week 5 - PRACTICAL: ADD, SUB, ORA, ANA
			OWN TIME: JP, JM, ADC, SBB, XRA, CPI, DAD
			IMPLEMENTED: All EXECPT DAD
 * Week 6 - PRACTICAL: RLC, RAL
			OWN TIME: DAD, RRC, RAR, CMA, CMC, STC, RST, PCHL, XTH
			IMPLEMENTED: All
 */


// Sim8080.cpp : Defines the entry point for the console application.
//FINISH INDENTATIONS 

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "12345678"

#define IP_ADDRESS_SERVER "127.0.0.1" //server ip

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short // 16 bit variable
#define DWORD unsigned long // 32 bit variable
#define BYTE  unsigned char // 8 bit variable

#define MAX_FILENAME_SIZE 500

#define MAX_BUFFER_SIZE   500



SOCKADDR_IN server_addr;
SOCKADDR_IN client_addr;

SOCKET sock;  // This is our socket, it is the handle to the IO address to read/write packets

WSADATA data;




char InputBuffer [MAX_BUFFER_SIZE];

char hex_file [MAX_BUFFER_SIZE];
char trc_file [MAX_BUFFER_SIZE];




//////////////////////////
// Intel 8080 Registers //
//////////////////////////

#define REGISTER_B	0
#define REGISTER_C	1
#define REGISTER_D	2
#define REGISTER_E	3
#define REGISTER_H	4
#define REGISTER_L	5
#define REGISTER_M	6 // Memory
#define REGISTER_A	7 // The accumalator

#define FLAG_S	0x80
#define FLAG_Z	0x40
#define FLAG_A	0x10
#define FLAG_P	0x04
#define FLAG_C	0x01

BYTE Registers[8];
BYTE Flags;
WORD ProgramCounter;
WORD StackPointer;

////////////
// Memory //
////////////

#define K_1			1024
#define MEMORY_SIZE	K_1

BYTE Memory[MEMORY_SIZE];

///////////////////////
// Control variables //
///////////////////////

bool memory_in_range = true;
bool halt = false;

///////////////////////
// Disassembly table //
///////////////////////

char opcode_mneumonics[][12] =
{
	"NOP        ", 
	"LXI B,data ", 
	"STAX B     ", 
	"INX B      ", 
	"INR B      ", 
	"DCR B      ", 
	"MVI B,data ", 
	"RLC        ", 
	".BYTE 0x08 ", 
	"DAD B      ", 
	"LDAX B     ", 
	"DCX B      ", 
	"INR C      ", 
	"DCR C      ", 
	"MVI C,data ", 
	"RRC        ", 
	".BYTE 0x10 ", 
	"LXI D,data ", 
	"STAX D     ", 
	"INX D      ", 
	"INR D      ", 
	"DCR D      ", 
	"MVI D,data ", 
	"RAL        ", 
	".BYTE 0x18 ", 
	"DAD D      ", 
	"LDAX D     ", 
	"DCX D      ", 
	"INR E      ", 
	"DCR E      ", 
	"MVI E,data ", 
	"RAR        ", 
	"RIM        ", 
	"LXI H,data ", 
	"SHLD       ", 
	"INX H      ", 
	"INR H      ", 
	"DCR H      ", 
	"MVI H,data ", 
	"DAA        ", 
	".BYTE 0x28 ", 
	"DAD H      ", 
	"LHLD       ", 
	"DCX H      ", 
	"INR L      ", 
	"DCR L      ", 
	"MVI L,data ", 
	"CMA        ", 
	"SIM        ", 
	"LXI SP,data", 
	"STA        ", 
	"INX SP     ", 
	"INR M      ", 
	"DCR M      ", 
	"MVI M,data ", 
	"STC        ", 
	".BYTE 0x38 ", 
	"DAD SP     ", 
	"LDA        ", 
	"DCX SP     ", 
	"INR A      ", 
	"DCR A      ", 
	"MVI A,data ", 
	"CMC        ", 
	"MOV B,B    ", 
	"MOV B,C    ", 
	"MOV B,D    ", 
	"MOV B,E    ", 
	"MOV B,H    ", 
	"MOV B,L    ", 
	"MOV B,M    ", 
	"MOV B,A    ", 
	"MOV C,B    ", 
	"MOV C,C    ", 
	"MOV C,D    ", 
	"MOV C,E    ", 
	"MOV C,H    ", 
	"MOV C,L    ", 
	"MOV C,M    ", 
	"MOV C,A    ", 
	"MOV D,B    ", 
	"MOV D,C    ", 
	"MOV D,D    ", 
	"MOV D,E    ", 
	"MOV D,H    ", 
	"MOV D,L    ", 
	"MOV D,M    ", 
	"MOV D,A    ", 
	"MOV E,B    ", 
	"MOV E,C    ", 
	"MOV E,D    ", 
	"MOV E,E    ", 
	"MOV E,H    ", 
	"MOV E,L    ", 
	"MOV E,M    ", 
	"MOV E,A    ", 
	"MOV H,B    ", 
	"MOV H,C    ", 
	"MOV H,D    ", 
	"MOV H,E    ", 
	"MOV H,H    ", 
	"MOV H,L    ", 
	"MOV H,M    ", 
	"MOV H,A    ", 
	"MOV L,B    ", 
	"MOV L,C    ", 
	"MOV L,D    ", 
	"MOV L,E    ", 
	"MOV L,H    ", 
	"MOV L,L    ", 
	"MOV L,M    ", 
	"MOV L,A    ", 
	"MOV M,B    ", 
	"MOV M,C    ", 
	"MOV M,D    ", 
	"MOV M,E    ", 
	"MOV M,H    ", 
	"MOV M,L    ", 
	"HLT        ", 
	"MOV M,A    ", 
	"MOV A,B    ", 
	"MOV A,C    ", 
	"MOV A,D    ", 
	"MOV A,E    ", 
	"MOV A,H    ", 
	"MOV A,L    ", 
	"MOV A,M    ", 
	"MOV A,A    ", 
	"ADD B      ", 
	"ADD C      ", 
	"ADD D      ", 
	"ADD E      ", 
	"ADD H      ", 
	"ADD L      ", 
	"ADD M      ", 
	"ADD A      ", 
	"ADC B      ", 
	"ADC C      ", 
	"ADC D      ", 
	"ADC E      ", 
	"ADC H      ", 
	"ADC L      ", 
	"ADC M      ", 
	"ADC A      ", 
	"SUB B      ", 
	"SUB C      ", 
	"SUB D      ", 
	"SUB E      ", 
	"SUB H      ", 
	"SUB L      ", 
	"SUB M      ", 
	"SUB A      ", 
	"SBB B      ", 
	"SBB C      ", 
	"SBB D      ", 
	"SBB E      ", 
	"SBB H      ", 
	"SBB L      ", 
	"SBB M      ", 
	"SBB A      ", 
	"ANA B      ", 
	"ANA C      ", 
	"ANA D      ", 
	"ANA E      ", 
	"ANA H      ", 
	"ANA L      ", 
	"ANA M      ", 
	"ANA A      ", 
	"XRA B      ", 
	"XRA C      ", 
	"XRA D      ", 
	"XRA E      ", 
	"XRA H      ", 
	"XRA L      ", 
	"XRA M      ", 
	"XRA A      ", 
	"ORA B      ", 
	"ORA C      ", 
	"ORA D      ", 
	"ORA E      ", 
	"ORA H      ", 
	"ORA L      ", 
	"ORA M      ", 
	"ORA A      ", 
	"CMP B      ", 
	"CMP C      ", 
	"CMP D      ", 
	"CMP E      ", 
	"CMP H      ", 
	"CMP L      ", 
	"CMP M      ", 
	"CMP A      ", 
	"RNZ        ", 
	"POP B      ", 
	"JNZ        ", 
	"JMP        ", 
	"CNZ        ", 
	"PUSH B     ", 
	"ADI        ", 
	"RST 0      ", 
	"RZ         ", 
	"RET        ", 
	"JZ         ", 
	".BYTE 0xCB ", 
	"CZ         ", 
	"CALL       ", 
	"ACI        ", 
	"RST 1      ", 
	"RNC        ", 
	"POP D      ", 
	"JNC        ", 
	"OUT        ", 
	"CNC        ", 
	"PUSH D     ", 
	"SUI        ", 
	"RST 2      ", 
	"RC         ", 
	".BYTE 0xD9 ", 
	"JC         ", 
	"IN         ", 
	"CC         ", 
	".BYTE 0xDD ", 
	"SBI        ", 
	"RST 3      ", 
	"RPO        ", 
	"POP H      ", 
	"JPO        ", 
	"XTHL       ", 
	"CPO        ", 
	"PUSH H     ", 
	"ANI        ", 
	"RST 4      ", 
	"RPE        ", 
	"PCHL       ", 
	"JPE        ", 
	"XCHG       ", 
	"CPE        ", 
	".BYTE 0xED ", 
	"XRI        ", 
	"RST 5      ", 
	"RP         ", 
	"POP PSW    ", 
	"JP         ", 
	"DI         ", 
	"CP         ", 
	"PUSH PSW   ", 
	"ORI        ", 
	"RST 6      ", 
	"RM         ", 
	"SPHL       ", 
	"JM         ", 
	"EI         ", 
	"CM         ", 
	".BYTE 0xFD ", 
	"CPI        ", 
	"RST 7      "
};







////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (Start)                 //
////////////////////////////////////////////////////////////////////////////////


BYTE fetch() // The fetch() function is used to get the low-order and high-order byte
{
	BYTE byte = 0;

	if ((ProgramCounter >= 0) && (ProgramCounter <= MEMORY_SIZE))
	{
		memory_in_range = true;
		byte = Memory[ProgramCounter];
		ProgramCounter++;
	}
	else
	{
		memory_in_range = false;
	}
	return byte;
}

//                                                         set_flags

 //Function: set_flags
 //Description: sets flags dependant upon the value in a given register
 //Parameters: inReg(BYTE) - the register that will be used to set the flags
 //Return: none (void)
 //Warning: none

void set_flags(BYTE inReg) // set_flags function is called when a specific condition flag needs to be set
{
	BYTE regA; // Assign to a 8 bit variable
	int  bit_set_count;

	regA = inReg; // Set the variable to the functioons parameter

	//Zero flag sets the flag to one to indicate that the result in the accumulator contains all zeros.
	if (regA == 0) // Checks if the accumalator contains all 0			
	{
		Flags = Flags | FLAG_Z; // Set flag 
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z); // Otherwise set flag to 0
	}

	// Sign Flag - Instructions that affect the flag set it equal to bit 7. A 0 is a positive value, a 1 is a negative value.
	if ((regA & 0x80) != 0) // Checks if the flag is negative
	{
		Flags = Flags | FLAG_S; // Set sign flag
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_S); // Otherwise set sign flag to 0

	}


	// Parity Flag - Parity is determined by counting the number of one bits set in the result of the accumulator
	bit_set_count = 0;
	//Here I use a loop to increment the bit_set_count 
	int i = 0;
	int msk = 0x01; // Initial bit_set is 0x01
	for(i=0;i<8;i++) // Loop i 8 times for all 8 bit sets
	{
		if((regA & msk) != 0)
		{
			bit_set_count++;
		}
		msk = msk << 1; // Move to next bit_set_count
	}
	
	switch (bit_set_count)
	{
		case 0:
		case 2:
		case 4:
		case 6:
		case 8:
				Flags = Flags | FLAG_P;
				break;

		default:
			Flags = Flags & (0xFF - FLAG_P);
			break;
	}


}

//                                                      set_carry

//Function: set_carry
//Description: Sets the carry flag
//Parameters: opcode(BYTE)
//Returns: void
//Warnings: none
void set_carry(WORD temp_word) 
{
	Flags = Flags | FLAG_C;
}

//                                                      Block_00 Instructions

//Function: block_00_instructions
//Description: Performs any instruction with opcode 00. 
//Parameters: opcode(BYTE)
//Returns: void
//Warnings: none
void block_00_instructions(BYTE opcode) //These instruction move immediately to register
{
	BYTE ls3bits;
	BYTE destination;
	WORD address;
	BYTE lb;
	BYTE hb;
	long temp;
	long hl;
	WORD temp_word;
	BYTE temp_byte;
	WORD carry;
	
	ls3bits = opcode & 0x07;

	switch (ls3bits)
	{
	case 0x00:	// NOP
		break;

	case 0x01:	// LXI and DAD
	//LXI is a three-byte instruction; its second and third bytes contain the source data to be loaded into a register pair
		if ((opcode & 0x08) == 0)
		{
			lb = fetch();
			hb = fetch();
			switch(opcode) 
			{
			case 0x01: //LXI BC
				Registers[REGISTER_B] = hb; // Set register B to the high-order byte
				Registers[REGISTER_C] = lb; // Set register C to the low-order byte
				break;
			case 0x11: //LXI DE
				Registers[REGISTER_D] = hb;
				Registers[REGISTER_E] = lb;
				break;
			case 0x21: //LXI HL
				Registers[REGISTER_H] = hb;
				Registers[REGISTER_L] = lb;
				break;
			default: //LXI SP
				// Set the stack to the low-order and high-order byte, WORD is used as a 16 bit vairable is needed for 2 8 bits
				StackPointer = ((WORD)hb << 8) + (WORD)lb; 
				break;
			}
		} else //DAD
		//DAD adds the 16-bit value in the specified register pair to the contents of the H and L register pair
		{
			// Set temp_word to register H and L
			temp_word = ((WORD)Registers[REGISTER_H] << 8)  + ((WORD)Registers[REGISTER_L]);
			switch(opcode)
			{
			case 0x09:	//DAD BC
				address = ((WORD)Registers[REGISTER_B] << 8)  + (WORD)Registers[REGISTER_C];
				temp_word += address; // Add register B and C to contents of register H and L
				break;
			case 0x19:	//DAD D
				address = ((WORD)Registers[REGISTER_D] << 8)  + (WORD)Registers[REGISTER_E];
				temp_word += address;
				break;
			case 0x29:	//DAD H
				temp_word += temp_word;
				break;
			case 0x39:	//DAD SP
				temp_word += (WORD)StackPointer; // Add the stack to the contents of register H and L
				break;
			}

			Registers[REGISTER_H] = (BYTE)((temp_word >> 8) & 0xFF); // Add the first 8 bits to register H
			Registers[REGISTER_L] = (BYTE)(temp_word); // Then the next 8 bits to L

		}
	case 0x02:	// Load and Store instructions
		switch(opcode)
		{
			case 0x3A: //LDA loads the accumalator with a copy of the low-order and high-order byte
				lb = fetch();
				hb = fetch();
				address = ((WORD)hb << 8) + (WORD)lb; // Set address to the high and low order byte
				//Checks the address is valid
				if((address >= 0) && (address < MEMORY_SIZE)) // Checks if the address is valid
				{ 
					Registers[REGISTER_A] = Memory[address]; //Copy the contents of the accumalator to the specified address
				}
				else {
					Registers[REGISTER_A] = 0; 
				}
				break;

			case 0x2A: //LHLD copies the contents of the memory location to register L and the contents of the next to H
				lb = fetch();
				hb = fetch(); 
				address = ((WORD)hb << 8) + (WORD)lb; 
				//Checks if address is valid
				if ((address >= 0) && (address < (MEMORY_SIZE-1))) 
				{ 
					Registers[REGISTER_L] = Memory[address]; // Copies the first byte into reigster L
					Registers[REGISTER_H] = Memory[address+1]; // Then the address moves 1 bit to copy the next byte
				} 
				else { 
				Registers[REGISTER_L] = 0; 
				Registers[REGISTER_H] = 0; 
				} 
				break;

			case 0x22: //SHLD stores a coppy of register L to memeory location specified in the low-order and high-order byte

				lb = fetch();
				hb = fetch(); 
				// Set address to the low and high order byte
				address = ((WORD)hb << 8) + (WORD)lb; 
				//Checks if address is valid
				if ((address >= 0) && (address < (MEMORY_SIZE-1))) 
				{ 
					 Memory[address+1] = Registers[REGISTER_H]; 
					 Memory[address] = Registers[REGISTER_L]; 
				} 
				break;

			case 0x32: // STA stores a copy of the accumulator contents into the memory location specified by the low/high byte

				lb = fetch();
				hb = fetch();
				address = ((WORD)hb << 8) + (WORD)lb;
				//Checks if address is valid
				if((address >= 0) && (address < MEMORY_SIZE))
				{
					 Memory[address] = Registers[REGISTER_A]; // Copy address location to the accumalator
				}
				

		//STAX stores a copy of the accumalator's contents into the memory location addressed by register pair BC or DE.
		case 0x02: //STAX BC
			//Uses an Indirect Address
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			if ((address >= 0) && (address < (MEMORY_SIZE))) // Checks if address is valid
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;

		case 0x12: //STAX DE
			address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
			if ((address >= 0) && (address < (MEMORY_SIZE)))
			{
				Memory[address] = Registers[REGISTER_A];
			}
			break;


		//LDAX copies the contents of a memory location, specified by the register par BC or DE, to the accumalator
		case 0x0A: //LDAX BC
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			//Checks if address is valid
			if ((address >= 0) && (address < (MEMORY_SIZE)))
			{
				Registers[REGISTER_A] = Memory[address];
			}
			else {
				Registers[REGISTER_A] = 0;
			}
			break;

		case 0x1A: //LDAX DE
			address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
			if ((address >= 0) && (address < (MEMORY_SIZE)))
			{
				Registers[REGISTER_A] = Memory[address];
			}
			else
			{
				Registers[REGISTER_A] = 0;
			}
			break;
			default:

				break;
		}

		break;

	case 0x03:	// INX and DCX
	//INX adds one to the contents of the specified register pair.
		
		switch(opcode)
		{
			case 0x03:	//INX BC
				temp = (WORD)Registers[REGISTER_B] << 8 + (WORD)Registers[REGISTER_C]; // Set variable to register B and C
				Registers[REGISTER_B] = (BYTE)((temp >> 8) & 0xFF); // Shift temp 8 bits and add one (0xFF) to register B
				Registers[REGISTER_C] = (BYTE)(temp); // Add one to reigster C
				break;

			case 0x13:	//INX DE
				temp = (WORD)Registers[REGISTER_D] << 8 + (WORD)Registers[REGISTER_E];
				Registers[REGISTER_D] = (BYTE)((temp >> 8) & 0xFF);
				Registers[REGISTER_E] = (BYTE)(temp);
				break;

			case 0x23:	//INX HL
				temp = (WORD)Registers[REGISTER_H] << 8 + (WORD)Registers[REGISTER_L];
				Registers[REGISTER_H] = (BYTE)((temp >> 8) & 0xFF);
				Registers[REGISTER_L] = (BYTE)(temp);
				break;

			case 0x33:	//INX SP
				StackPointer++;
				break;

	//DCX decrements the contents of the specified register pair by one.

			case 0x0b:	//DCX BC
				temp = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C]; // Set variable to register B and C
				temp--;
				Registers[REGISTER_B] = (BYTE)((temp >> 8) & 0xFF);
				Registers[REGISTER_C] = (BYTE)(temp);
				break;

			case 0x1b:	//DCX DE
				temp = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];
				temp--;
				Registers[REGISTER_D] = (BYTE)((temp >> 8) & 0xFF);
				Registers[REGISTER_E] = (BYTE)(temp);
				break;

			case 0x2b:	//DCX HL
				temp = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
				temp--;
				Registers[REGISTER_H] = (BYTE)((temp >> 8) & 0xFF);
				Registers[REGISTER_L] = (BYTE)(temp);
				break;

			case 0x3b:	//DCX SP
				StackPointer--;
				break;

		}

		break;

	case 0x04:	// INR adds one to the contents of the specified byte. INR affects all of the condition flags except the carry

		destination = (opcode >> 3) & 0x07;
		// Checks if destination is in memory
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // Set address to register H and L
			if ((address >= 0) && (address < MEMORY_SIZE)) // Checks if that address is valid
			{
				Memory[address]++;
				set_flags(Memory[address]);
			}
		} else {
			Registers[destination]++;
			set_flags(Registers[destination]);
		}
		break;

	case 0x05:	// DCR subtracts one to the contents of the specified byte. It affects all the condition flags except the carry

		destination = (opcode >> 3) & 0x07;
		// Checks if destination is in memory
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // Set address to register H and L
			if ((address >= 0) && (address < MEMORY_SIZE)) // Checks if that address is valid
			{
				Memory[address]--;
				set_flags(Memory[address]);
			}
		} else
		{
			Registers[destination]--;
			set_flags(Registers[destination]);
		}

		break;

	case 0x06:	// MVI moves one byte of data by copying its second byte into the destination field

		destination = (opcode >> 3) & 0x07;

		Registers[destination] = fetch();
		//Checks if the move is to a register or memory
		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L]; // Set address to reigster H and L
			if ((address >= 0) && (address < MEMORY_SIZE)) // Checks is the address is valid
			{
				Memory[address] = Registers[REGISTER_M]; // Move the byte to the memory
			}
		}
		break;

	default:
		// Rotates, DAA and carry instructions
		switch (opcode)
		{
		case 0x07: //RLC shifts one bit from the accumalator to the left with the high-order byte transferring to low-order byte

			temp_word = (WORD)Registers[REGISTER_A]; // Set temp_word to the accumalator
			temp_word = temp_word << 1; // Shift it 1 bit the the left

			if ((temp_word & 0x100) == 0x100) // checks the value of the 9th bit
			{
				temp_word = temp_word | 0x01;
				set_carry(Flags); //set_carry(Flags);  // if its 1 then set the the carry flag
			} else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (BYTE)(temp_word); // Set the accumalator

			break;

		case 0x17: //RAL rotates the contents of the accumalator and the carry flag 1 bit to the left

			temp_word = (WORD)Registers[REGISTER_A]; // Set temp_word to the accumalator 
			temp_word = temp_word << 1; // Shift it 1 bit the the left

			if ((Flags & FLAG_C) == FLAG_C) // Checks if the carry flag is set
			{
				temp_word = temp_word | 0x01; 
			}
			if ((temp_word & 0x100) == 0x100) // Checks the value of the 9th bit
			{
				set_carry(Flags);  // Set carry flag
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}

			Registers[REGISTER_A] = (BYTE)(temp_word);

			break;

		case 0x0F: //RRC shifts one bit from the accumalator to the left with the hb transferring to the lb

			temp_word = (WORD)Registers[REGISTER_A]; // Set temp_word to the accumalator 
			if ((temp_word & 0x01) == 0x01)
			{
				set_carry(Flags); // Set the carry flag
			}
			else
			{
				Flags = Flags & (0xFF - FLAG_C);
			}

			temp_word = temp_word >> 1;

			if ((Flags & FLAG_C) != 0) //is set
			{
				temp_word = (WORD)temp_word | 0x80;
			}
			Registers[REGISTER_A] = (BYTE)(temp_word);

			break;

		case 0x1F: // RAR rotates the contents of the accumulator and the carry flag one bit position to the right

			temp_word = (WORD)Registers[REGISTER_A];

			if ((Flags & FLAG_C) != 0) // checks if flag C is set
			{
				temp_word = (WORD)Registers[REGISTER_A] | 0x100;
			}

			if ((temp_word & 0x01) == 0x01) // Checks the first bit of the accumalator
			{
				set_carry(Flags);  // set the carry			
			} 
			else {
				Flags = Flags & (0xFF - FLAG_C);
			}

			temp_word = temp_word >> 1; // Shift the accumalator 1 bit to the right
			Registers[REGISTER_A] = (BYTE)(temp_word);
			break;

		case 0x2F:	//CMA inverts each bit by using XOR
			temp_word = (WORD)Registers[REGISTER_A] ^ 0xFF; // Set variable inverted or using bitwsie XOR
			Registers[REGISTER_A] = temp_word; 
			break;

		case 0x3F: //CMC sets the carry flag to 1 if it is 0, and 0 if it is 1

			if ((Flags & FLAG_C) != 0) // check if carry flag is set
			{
				Flags = Flags & (0xFF - FLAG_C); //set carry flag to 0
			} else {
				set_carry(Flags);  // otherwise set the carry flag to 1	
			}
			break;

		case 0x37: // STC sets the Carry Flag.
			set_carry(Flags); 
			break;
		}
	}
}



void MOV_and_HLT_instructions(BYTE opcode)
{
	//Here we asign the variables to a data type
	BYTE source;
	BYTE destination;
	BYTE temp;
	WORD address;

	if (opcode == 0x76)
	{
		halt = true;  // HLT - halt microprocessor
	}
	else
	{
		source = opcode & 0x07;
		destination = (opcode >> 3) & 0x07;

		if (source == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];

			if ((address >= 0) && (address < MEMORY_SIZE))
			{
				Registers[REGISTER_M] = Memory[address];
			}
			else
			{
				Registers[REGISTER_M] = 0;
			}
		}

		Registers[destination] = Registers[source];

		if (destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];

			if ((address >= 0) && (address < MEMORY_SIZE))
			{
				Memory[address] = Registers[REGISTER_M];
			}
		}
	}
}



//                                                      Block_10 Instructions

//Function: block_10_instructions
//Description: Performs any instruction with opcode 10. 
//Parameters: opcode(BYTE)
//Returns: void
//Warnings: none

void block_10_instructions(BYTE opcode)
{
	//Here we asign the variables to a data type
	BYTE  source;
	BYTE  instruction_type;
	WORD temp_word = 0;
	WORD  address;

	instruction_type = (opcode >> 3) & 0x07;
	source = opcode & 0x07;

	if (source == REGISTER_M)
	{
		address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];

		if ((address >= 0) && (address < MEMORY_SIZE))
		{
			Registers[REGISTER_M] = Memory[address];
		}
		else
		{
			Registers[REGISTER_M] = 0;
		}
	}

	temp_word = (WORD)Registers[source];

	switch (instruction_type)
	{
	case 0x00: // ADD adds the contents of the register or memory to the accumalator
		temp_word = (WORD)Registers[REGISTER_A] + temp_word;
		break;

	case 0x01: // ADC adds one byte of data plus the carry flag to the accumalator
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		temp_word = (WORD)Registers[REGISTER_A] + temp_word;
		break;

	case 0x02: // SUB subtracts one byte of data from the contents of the accumulator	
		temp_word = (WORD)Registers[REGISTER_A] - temp_word;
		break;

	case 0x03: // SBB subtracts one byte of data plus the carry flag from the accumulator		
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		temp_word = (WORD)Registers[REGISTER_A] - temp_word;
		break;

	case 0x04: // ANA perforrms the logical AND operation using the contents of the specified byte and the accumulator	
		temp_word = (WORD)Registers[REGISTER_A] & temp_word;
		break;

	case 0x05: // XRA performs the logical exclusive OR operation using the contents of the specified byte and the accumulator
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		temp_word = (WORD)Registers[REGISTER_A] ^ temp_word;
		break;

	case 0x06: // ORA performs the logical inclusive OR operation using the contents of the specified byte and the accumulator
		temp_word = (WORD)Registers[REGISTER_A] | temp_word;
		break;

	default:
		break;
	}

	//Set the carrry flag

	if ((temp_word & 0x100) != 0)
	{
		set_carry(Flags); 
	}
	else
	{
		Flags = Flags & (FLAG_C - 0xFF);
	}

	set_flags((BYTE)(temp_word));

	Registers[REGISTER_A] = (BYTE)(temp_word);

}

//                                                      Block_11 Instructions

//Function: block_11_instructions
//Description: Performs any instruction with opcode 11. 
//Parameters: opcode(BYTE)
//Returns: void
//Warnings: none

void block_11_instructions(BYTE opcode)
{
	WORD address;
	BYTE lb;
	BYTE hb;
	BYTE temp;
	WORD temp_word;

	switch (opcode)
	{
	case 0xC0:	// RNZ tests to see if the zero flag is 0, if so it pops 2 bytes off the stack and puts them into the prgram counter
		if ((Flags & FLAG_Z) == 0)
		{
			//Check address held in stack pointer is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; // Take the low order byte from the top of the stack
				StackPointer++; // Push the stack back up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb; // Set the program counter to the low and high order byte
			}
		}
		break;

	case 0xC1:	// POP removes two bytes of data from the stack and copies them to a register pair

		//Check address held in stack pointer is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			//Puts the byte from the stack into the register
			Registers[REGISTER_C] = Memory[StackPointer];
			StackPointer++; // Moves the stack up

			Registers[REGISTER_B] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xC2:	// JNZ tests if the zero flag is 0, if so the execution jumps to the next instruction

		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) == 0) // Checks if the contents of the accumalator are not 0
		{
			address = ((WORD)hb << 8)  + (WORD)lb;
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address if valid
			{
				ProgramCounter = address; 
			}
		}

		break;


	case 0xC3:	// JMP loads the address in the low order anf high order bytes to the program counter
		lb = fetch();
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb; // Add the low and high byte to get the address
		ProgramCounter = address;
		break;

	case 0xC4:	// CNZ calls the function if the zero flag is off

		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) == 0) //Checks to see if the flag is set
		{
			
			address = ((WORD)hb << 8) + (WORD)lb;

			//Checks the address in stack pointer is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; // Push the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); // Add the high byte
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF); // Add the low byte
				ProgramCounter = address;
			}
		}
		break;

	case 0xC5:	// PUSH copies two bytes of data to the stack.
		// Checks if the address in stack is valid
		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
		{
			StackPointer--; 
			Memory[StackPointer] = Registers[REGISTER_B]; // Copy reigster B to the stack
			StackPointer--; //Moves the stack back up
			Memory[StackPointer] = Registers[REGISTER_C];

		}
		break;

	case 0xC6:	// ADI adds low-rder byte of the accumulator and stores the result in the accumulator.
		
		lb = fetch();
		
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		if (temp_word >= 0x100)
		{
			set_carry(Flags); 
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word); //Stores the result in the accumalator

		set_flags(Registers[REGISTER_A]);
		break;

	case 0xC7:	// RST 0 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address
		// Checks if address in stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		// Predtermined address
		ProgramCounter = 0x0000;
		break;

	case 0xC8:	// RZ checks if the zero flag is set, if so it pops 2 bytes of data off the stack and onto the program counter
		
		if ((Flags & FLAG_Z) != 0) // Checks if flag is set
		{
			//Checks the address in stack pointer is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; //Take the top byte of the stack
				StackPointer++; // Mopve the stack back up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb; // Add the 2 bytes to give the program counter the address
			}
		}
		break;

	case 0xC9:	// RET pops two bytes of data off the stack and places them In the program counter register.
		//Checks the address in stack pointer is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
		{
			lb = Memory[StackPointer];
			StackPointer++;
			hb = Memory[StackPointer];
			StackPointer++;

			ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
		}
		break;

	case 0xCA:	// JZ checks if the zero flag is 0, if so the execution jumps to the next instruction

		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_Z) != 0) // Checks if flag is set
		{
			address = ((WORD)hb << 8)  + (WORD)lb;
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address if valid
			{
				ProgramCounter = address; 
			}
		}

		break;

	case 0xCB:	// .BYTE 0xCB  
		break;

	case 0xCC:	// CZ checks if the zero flag is set,if so the program counter is pushed onto the stack then jumps to the address

		if ((Flags & FLAG_Z) != 0) //Checks to see if the flag is set
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			// Checks address in stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; // Move stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); // Push the program counter to the stack 
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}
		break;

	case 0xCD:	// CALL pushes the contents of the program counter onto the stack and then jumps to the address specified
		lb = fetch();
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		// Checks if address in stack is valid
		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			ProgramCounter = address;
		}

		break;

	case 0xCE:	// ACI adds the contents of the low order bit and the carry bit to the accumulator and stores the result in it

		lb = fetch(); 
		
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;

		//Check if carry flag is set
		if ((Flags & FLAG_C) !=0)
		{
			temp_word++;
		}

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word);

		set_flags(Registers[REGISTER_A]);


		break;

	case 0xCF:	// RST 1 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address

		address = (opcode >> 3) & 0x07;
		// Checks if address on stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predetermined address
		ProgramCounter = 0x0008;

		break;

	case 0xD0:	// RNC checks if the carry flag is set, if so RNC pops two bytes off the stack and puts them in the program counter
		if ((Flags & FLAG_C) == 0)
		{
			//Check address held in stack pointer is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer];
				StackPointer++;
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
			}
		}
		break;

	case 0xD1:	// POP removes two bytes of data from the stack and copies them to a register pair
		//Checks address held in stack pointer is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			//Puts the top byte of the stack into register E
			Registers[REGISTER_E] = Memory[StackPointer]; 
			StackPointer++; //Move the stack back up
			Registers[REGISTER_D] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xD2:	// JNC tests the carry, if carry is set to 0, the execution jumps to the next instruction

		lb = fetch();
		hb = fetch();
		//Checks if carry is set
		if ((Flags & FLAG_C) == 0)
		{
			address = ((WORD)hb << 8)  + (WORD)lb;
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address if valid
			{
				ProgramCounter = address; // Program counter jumps to next address
			}
		}

		break;

	case 0xD3:	// OUT         
		break;

	case 0xD4:	// CNC checks if the carry is 0, if so it pushes the program counter onto the stack and then jumps to the address

		if ((Flags & FLAG_C) == 0) //Check to see if the flag is set
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; //Push the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); //Copies program counter onto the top of the stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}
		break;

	case 0xD5:	// PUSH copies two bytes of data from a register pair to the stack

		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
		{
			StackPointer--; //Push the stack down
			Memory[StackPointer] = Registers[REGISTER_D]; //Copies the the byte from register d to the stack
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_E];

		}
		break;

	case 0xD6:	// SUI subtracts the low order byte from the accumulator and stores the result in the accumulator

		lb = fetch();
		
		//Subtract the low order byte from the accumalator
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set carry flag to indicate the outcome of the operation
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Otherwsie clear the carry flag
		}

		Registers[REGISTER_A] = (BYTE)(temp_word); // Stores the result in the accumalator

		set_flags(Registers[REGISTER_A]);
		break;

	case 0xD7:	// RST 2 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address
		// Checks if address in stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predetermined address
		ProgramCounter = 0x0010;
		break;

	case 0xD8:	// RC checks if the carry flag is set to 1, if so pops two bytes off the stack and placed in the program counter

		if ((Flags & FLAG_C) != 0) // Checks carry is set
		{
			// Checks address in stack is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; // Moves the low order byte to the stack
				StackPointer++; // Move the stack up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
			}
		}
		break;

	case 0xD9:	// .BYTE 0xD9  
		break;

	case 0xDA:	// JC tests if the carry is set to 1, if so it jumps to the specified address

		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_C) != 0) // Checks if carry is 1
		{
			address = ((WORD)hb << 8)  + (WORD)lb; // Set address to the low and high order byte
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address is valid
			{
				ProgramCounter = address; // Jumps to specified address
			}
		}
		break;

	case 0xDB:	// IN          
		break;

	case 0xDC:	// CC tets if the carry is 1, if so CC pushes the program counter onto the stack then jumps to that address

		if ((Flags & FLAG_C) != 0) //Checks to see if the flag is set
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb; // Sets address to high and low order byte
			// Checks if address on stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; // Move the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); // Push the program counter onto the stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address; // Jump to specidied address
			}
		}
		break;

	case 0xDD:	// .BYTE 0xDD  
		break;

	case 0xDE:	// SBI subtracts the contents of the low order byte and the setting of the carry flag from the accumulator

		lb = fetch(); 
		if ((Flags & FLAG_C) !=0) // Checks if carry flag is set
		{
			// Subract the low order byte and 1(carry) from accumalator
			temp_word = (WORD)Registers[REGISTER_A] - (((WORD)lb) + 1); 
		}
		else {
			// Subtract just the low order byte if flag is not set
			temp_word = (WORD)Registers[REGISTER_A] - (((WORD)lb) + 0);
		}
		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set the carry flag
		}
		else {
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word); // Store the result in the accumalator
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xDF:	// RST 3 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address
		//Check if address in stack pointer is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			//Push the stack down
			StackPointer--;
			//Add the program counter onto the stack
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predetermined address
		ProgramCounter = 0x0018;
		break;

	case 0xE0:	// RPO tests if the parity flag is 0. RPO pops two bytes off the stack and places them in the program counter

		//Checks if flag is odd parity
		if ((Flags & FLAG_P) == 0)
		{
			//Checks if address held in stack pointer is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; //Pops the low order byte of the stack to the program counter
				StackPointer++; //Move the stack back up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
			}
		}
		break;

	case 0xE1:	// POP removes two bytes of data from the stack and copies them to a register pair
		//Checks if address in stack is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			Registers[REGISTER_L] = Memory[StackPointer]; // Move the top byte of the stack to the register L
			StackPointer++; //moves the stack back up
			Registers[REGISTER_H] = Memory[StackPointer];
			StackPointer++;
		}
		break;

	case 0xE2:	// JPO test the parity flag, If the flag is set to one. execution continues with the next instruction.

		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_P) == 0) // Checks if parity is 0
		{
			address = ((WORD)hb << 8) + (WORD)lb;

			if((address >= 0) && (address < MEMORY_SIZE)) //Check address is valid
			{
				ProgramCounter = address; // Jump to next address
			}
		}

		break;

	case 0xE3:	// XTHL exchanges two bytes from the top of the stack with the two bytes stored in the H and L registers  
		//Check if address in stack is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			temp_word = Memory[StackPointer]; //Take the byte from the top of the stack and put it into temp_word
			StackPointer++; //Move the stack back up
			temp_word = ((WORD)temp_word << 8) + Memory[StackPointer];
			StackPointer++;

			Registers[REGISTER_L] = Memory[StackPointer]; //Move the byte from register L to the stack
			StackPointer--; //moves the stack back up
			Registers[REGISTER_H] = Memory[StackPointer];
			StackPointer--;

			Registers[REGISTER_H] = (WORD)((temp_word >> 8) & 0xFF); //Finally put temp_word into register H
			Registers[REGISTER_L] = (WORD)(temp_word & 0xFF);

		}
		break;

		break;

	case 0xE4:	// CPO tests if the parity flag is set to 0. CPO pushes the program counter onto the stack and then jumps to the address specified

		if ((Flags & FLAG_P) == 0) //Checks to see if the flag is set
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			//Checks if address in stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; //Move the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); //Put the program counter onto the stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}
		break;

	case 0xE5:	// PUSH copies two bytes of data to the stack. This data may be the contents of a register pair

		//Check if address in stack is valid
		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
		{
			StackPointer--; ///Move the stack down
			Memory[StackPointer] = Registers[REGISTER_H]; //Copy the top of the stack to the register H
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_L];
		}

		break;

	case 0xE6:	// ANI performs a logical AND operation with the low order byte and the accumulator and stored in the accumalator

		lb = fetch();
		
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag
		}

		Registers[REGISTER_A] = (BYTE)(temp_word); // Store in the accumalator

		set_flags(Registers[REGISTER_A]);
		break;

	case 0xE7:	// RST 4 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address
		//Checks if the address in the stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predetermined address
		ProgramCounter = 0x0020;
		break;

	case 0xE8:	// RPE tests if the parity flag is 1, if so it pops two bytes off the stack and places them in the program counter
		
		if ((Flags & FLAG_P) != 0) // Check if parity is 1
		{
			//If so check the address in the satck is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; //Move the top of the stack to the low order byte
				StackPointer++; //Move the stack back up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
			}
		}
		break;

	case 0xE9:	// PCHL loads tle contents of the Hand L registers Into the program counter register.
		ProgramCounter = ((WORD)Registers[REGISTER_H] << 8) + ((WORD)Registers[REGISTER_L]);
		break;

	case 0xEA:	// JPE tests if the parity flag is 1, if so the execution continues to the next instruction 

		lb = fetch();
		hb = fetch();
		if((Flags & FLAG_P) != 0) //Check if parity is 1
		{
			address = ((WORD)hb << 8) + (WORD)lb;

			if((address >= 0) && (address < MEMORY_SIZE)) //Check address is valid
			{
				ProgramCounter = address; // Jump to next instruction
			}
		}

		break;

	case 0xEB:	// XCHG exchanges the contents of the Hand L registers with the contents of the D and E registers.

		temp = Registers[REGISTER_H]; //Assign the contents of the register H to the variable temp
		Registers[REGISTER_H] = Registers[REGISTER_D]; //Move the contents of register D to reigster H
		Registers[REGISTER_D] = temp; // Now put the contents of the variable temp(holding register H contents) to register D

		temp = Registers[REGISTER_L];
		Registers[REGISTER_L] = Registers[REGISTER_E];
		Registers[REGISTER_E] = temp;

		break;

	case 0xEC:	// CPE tests if the parity flag is 1. CPE pushes the program counter onto the stack and then jumps to the address specified

		if ((Flags & FLAG_P) != 0) // Checks to see if the flag is set
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			//Check the address in the stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; //Move the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); //Put the contents of the program counter on stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}
		break;

	case 0xED:	// .BYTE 0xED  
		break;

	case 0xEE:	// XRI performs an exclusive OR operation using the low order byte and the contents of the accumulator

		lb = fetch();
		
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the bitwise &
		}

		// Store the contents in the accumalator
		Registers[REGISTER_A] = (BYTE)(temp_word);

		set_flags(Registers[REGISTER_A]);
		break;

	case 0xEF:	// RST 5 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address
		// Check if the address in the stack pointer is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predetermined address
		ProgramCounter = 0x0028;
		break;

	case 0xF0:	// RP tests if sign flag is 0, if so RP pops two bytes off the stack and places them in the program counter
		// Check if sign flag is 0
		if ((Flags & FLAG_S) == 0)
		{
			// Check if address in stack is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; // Pop the top byte of the stack to the low order byte
				StackPointer++; //Move the stack back up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
			}
		}
		break;

	case 0xF1:	// POP PSW uses the contents of the memory location specified by the stack pointer to restore the condition flags
		// Check if address in stack is valid
		if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE - 2)))
		{
			Flags = Memory[StackPointer];
			StackPointer++; // Move the stack back up
			Registers[REGISTER_A] = Memory[StackPointer]; //Restore the contents of the address to the accumalator
			StackPointer++;
		}
		break;

	case 0xF2:	// JP tests if the sign flag is 0, if so program execution resumes at the address specified

		lb = fetch();
		hb = fetch();
		// Check if the sugn flag is 0
		if ((Flags & FLAG_S) == 0)
		{
			address = ((WORD)hb << 8)  + (WORD)lb;
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address if valid
			{
				ProgramCounter = address; //Resume program execution at the address
			}
		}

		break;

	case 0xF3:	// DI          
		break;

	case 0xF4:	// CP tests if the sign flag is 0. If so, CP pushes the contents of the program counter onto the stack and then jumps to the address specified
		
		if ((Flags & FLAG_S) == 0) // Checks if contents of accumulator are positive
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			//Check if address in stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; // Move the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); //Push the contents of the program counter onto stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address; //Jumps the program counter to the address
			}
		}
		break;

	case 0xF5:	// PUSH PSW copies the contents of the accumalator and settings of the condition flags onto the stack
		//Check if address in stack is valid
		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
		{
			StackPointer--; //Move the stack down
			Memory[StackPointer] = Registers[REGISTER_A]; //Copy the contents of the accumalator to the stack
			StackPointer--; 
			Memory[StackPointer] = Flags; // Copy the settings of the condition flags to the stack

		}
		break;

	case 0xF6:	// ORI performs an inclusive OR using the contents of the low order byte and the the accumulator, The result is placed in the accumulator

		lb = fetch(); 
		
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word); // Result is stored in the accumalator

		set_flags(Registers[REGISTER_A]);
		break;

	case 0xF7:	// RST 6 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address  
		//Checks address in stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--; //Move the stack down
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); //Put program counter onto stack
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		//Predteremined address
		ProgramCounter = 0x0030;
		break;

	case 0xF8:	// RM tests if the sign flag is 1, if so RM pops 2 bytes off the stack and places then into program counter

		if ((Flags & FLAG_S) != 0) // Checks if the flag is set
		{
			//Checks if the address on the stack is valid
			if((StackPointer >= 0) && (StackPointer < (MEMORY_SIZE-2)))
			{
				lb = Memory[StackPointer]; // Pop top byte of the stack to the low-order byte
				StackPointer++; //Movge the stack up
				hb = Memory[StackPointer];
				StackPointer++;

				ProgramCounter = ((WORD)hb << 8) + (WORD)lb; // Sets the program counter to the lowe and high order byte
			}
		}	
		break;

	case 0xF9:	// SPHL loads the contents of the registers H and L into the stack pointer

		StackPointer = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];

		break;

	case 0xFA:	// JM tests if the sign flag is 0, if so execution continues(Junms) to the next instruction

		lb = fetch();
		hb = fetch();
		if ((Flags & FLAG_S) != 0)
		{
			address = ((WORD)hb << 8)  + (WORD)lb;
			if ((address >= 0) && (address < MEMORY_SIZE)) //checks if address if valid
			{
				ProgramCounter = address;
			}
		}

		break;

	case 0xFB:	// EI          
		break;

	case 0xFC:	// CM tests if the sign flag is 1, if so it pushes the program counter onto the stack then jumps to the address specified
		// Checks if sign flag is set
		if ((Flags & FLAG_S) != 0)
		{
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			// Checks if the address on the stack is valid
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--; //	Move the stack down
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF); // Push program counter onto stack
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address; // Jump the address to the stack
			}
		}
		break;

	case 0xFD:	// .BYTE 0xFD  
		break;

	case 0xFE:	// CPI compares the low-order byte with the accumulator and sets the zero and carry flag to show the result
		
		lb = fetch(); 
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb; // Set temp_word to the accumalator without the low-order byte

		if (temp_word >= 0x100)
		{
			set_carry(Flags);  // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clear the carry flag by setting it to 0, using the bitwise &
		}

		set_flags((BYTE)(temp_word));

		break;

	case 0xFF:	// RST 7 // RST 6 pushes the program counter onto the stack to provide a return address and then Jumps to a predetemined address  
		//Checks address in stack is valid
		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
		}
		// Predetermined address
		ProgramCounter = 0x0038;
		break;

	default:
		break;
	}
}



void execute(BYTE opcode)
{
	BYTE ms2bits;

	ms2bits = (opcode >> 6) & 0x03;

	switch (ms2bits)
	{
	case 0:
		block_00_instructions(opcode);
		break;

	case 1:		// MOV and HALT
		MOV_and_HLT_instructions(opcode);
		break;

	case 2:
		block_10_instructions(opcode);
		break;

	default:
		block_11_instructions(opcode);
		break;
	}
}


void emulate_8080()
{
	BYTE opcode;

	ProgramCounter = 0;
	halt = false;
	memory_in_range = true;

	printf("                  A  B  C  D  E  H  L   SP\n");

	while ((!halt) && (memory_in_range))
	{
		printf("%04X ", ProgramCounter);           // Print current address

		opcode = fetch();
		execute(opcode);		//Fetch-Execute cycle...

		printf("%s  ", opcode_mneumonics[opcode]);  // Print current opcode

		printf("%02X ", Registers[REGISTER_A]);     // Print Accumulator
		printf("%02X ", Registers[REGISTER_B]);     // Print Register B
		printf("%02X ", Registers[REGISTER_C]);     // Print Register C
		printf("%02X ", Registers[REGISTER_D]);     // Print Register D
		printf("%02X ", Registers[REGISTER_E]);     // Print Register E
		printf("%02X ", Registers[REGISTER_H]);     // Print Register H
		printf("%02X ", Registers[REGISTER_L]);     // Print Register L

		printf("%04X ", StackPointer);              // Print Stack Pointer

		if ((Flags & FLAG_S) == FLAG_S)	            // Print Sign Flag
		{
			printf("S=1 ");
		}
		else
		{
			printf("S=0 ");
		}

		if ((Flags & FLAG_Z) == FLAG_Z)	            // Print Zero Flag
		{
			printf("Z=1 ");
		}
		else
		{
			printf("Z=0 ");
		}

		if ((Flags & FLAG_A) == FLAG_A)	            // Print Auxillary Carry Flag
		{
			printf("A=1 ");
		}
		else
		{
			printf("A=0 ");
		}

		if ((Flags & FLAG_P) == FLAG_P)	            // Print Parity Flag
		{
			printf("P=1 ");
		}
		else
		{
			printf("P=0 ");
		}

		if ((Flags & FLAG_C) == FLAG_C)	            // Print Carry Flag
		{
			printf("C=1 ");
		}
		else
		{
			printf("C=0 ");
		}

		printf("\n");  // New line
	}

	printf("\n");  // New line
}




////////////////////////////////////////////////////////////////////////////////
//                      Intel 8080 Simulator/Emulator (End)                   //
////////////////////////////////////////////////////////////////////////////////













void initialise_filenames()
{
	int i;

	for (i=0; i<MAX_FILENAME_SIZE; i++)
	{
		hex_file [i] = '\0';
		trc_file [i] = '\0';
	}
}




int find_dot_position(char *filename)
{
	int  dot_position;
	int  i;
	char chr;

	dot_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		if (chr == '.') 
		{
			dot_position = i;
		}
		i++;
		chr = filename[i];
	}

	return (dot_position);
}


int find_end_position(char *filename)
{
	int  end_position;
	int  i;
	char chr;

	end_position = 0;
	i = 0;
	chr = filename[i];

	while (chr != '\0')
	{
		end_position = i;
		i++;
		chr = filename[i];
	}

	return (end_position);
}


bool file_exists(char *filename)
{
	bool exists;
	FILE *ifp;

	exists = false;

	if ( ( ifp = fopen( filename, "r" ) ) != NULL ) 
	{
		exists = true;

		fclose(ifp);
	}

	return (exists);
}



void create_file(char *filename)
{
	FILE *ofp;

	if ( ( ofp = fopen( filename, "w" ) ) != NULL ) 
	{
		fclose(ofp);
	}
}



bool getline(FILE *fp, char *buffer)
{
	bool rc;
	bool collect;
	char c;
	int  i;

	rc = false;
	collect = true;

	i = 0;
	while (collect)
	{
		c = getc(fp);
		
		switch (c)
		{
		case EOF:
			if (i > 0)
			{
				rc = true;
			}
			collect = false;
			break;

		case '\n':
			if (i > 0)
			{
				rc = true;
				collect = false;
				buffer[i] = '\0';
			}
			break;

		default:
			buffer[i] = c;
			i++;
			break;
		}
	}

	return (rc);
}





void load_and_run()
{
	char chr;
	int  ln;
	int  dot_position;
	int  end_position;
	long i;
	FILE *ifp;
	long address;
	long load_at;
	int  code;

	// Prompt for the .hex file

	printf("\n");
	printf("Enter the hex filename (.hex): ");

	ln = 0;
	chr = '\0';
	while (chr != '\n')
	{
		chr = getchar();

		switch(chr)
		{
		case '\n':
				break;
		default:
				if (ln < MAX_FILENAME_SIZE)
				{
					hex_file [ln] = chr;
					trc_file [ln] = chr;
					ln++;
				}
				break;
		}
	}

	// Tidy up the file names

	dot_position = find_dot_position(hex_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(hex_file);

		hex_file[end_position + 1] = '.';
		hex_file[end_position + 2] = 'h';
		hex_file[end_position + 3] = 'e';
		hex_file[end_position + 4] = 'x';
		hex_file[end_position + 5] = '\0';
	}
	else
	{
		hex_file[dot_position + 0] = '.';
		hex_file[dot_position + 1] = 'h';
		hex_file[dot_position + 2] = 'e';
		hex_file[dot_position + 3] = 'x';
		hex_file[dot_position + 4] = '\0';
	}

	dot_position = find_dot_position(trc_file);
	if (dot_position == 0)
	{
		end_position = find_end_position(trc_file);

		trc_file[end_position + 1] = '.';
		trc_file[end_position + 2] = 't';
		trc_file[end_position + 3] = 'r';
		trc_file[end_position + 4] = 'c';
		trc_file[end_position + 5] = '\0';
	}
	else
	{
		trc_file[dot_position + 0] = '.';
		trc_file[dot_position + 1] = 't';
		trc_file[dot_position + 2] = 'r';
		trc_file[dot_position + 3] = 'c';
		trc_file[dot_position + 4] = '\0';
	}

	if (file_exists(hex_file))
	{
		// Clear Registers and Memory

		Flags = 0;
		ProgramCounter = 0;
		StackPointer = 0;
		for (i=0; i<8; i++)
		{
			Registers[i] = 0;
		}
		for (i=0; i<MEMORY_SIZE; i++)
		{
			Memory[i] = 0;
		}

		// Load hex file

		if ( ( ifp = fopen( hex_file, "r" ) ) != NULL ) 
		{
			printf("Loading file...\n\n");

			load_at = 0;

			while (getline(ifp, InputBuffer))
			{
				if (sscanf(InputBuffer, "L=%x", &address) == 1)
				{
					load_at = address;
				}
				else if (sscanf(InputBuffer, "%x", &code) == 1)
				{
					if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
					{
						Memory[load_at] = (BYTE)code;
					}
					load_at++;
				}
				else
				{
					printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
				}
			}

			fclose(ifp);
		}

		// Emulate the 8080

		emulate_8080();
	}
	else
	{
		printf("\n");
		printf("ERROR> Input file %s does not exist!\n", hex_file);
		printf("\n");
	}
}




void test_and_mark()
{
	char buffer[1024];
	bool testing_complete;
	int  len = sizeof(SOCKADDR);
	char chr;
	int  i;
	int  j;
	bool end_of_program;
	long address;
	long load_at;
	int  code;
	int  mark;

	printf("\n");
	printf("Automatic Testing and Marking\n");
	printf("\n");

	testing_complete = false;

	sprintf(buffer, "Test Student %s", STUDENT_NUMBER);
	sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
	
	while (!testing_complete)
	{
		memset(buffer, '\0', sizeof(buffer));

		if (recvfrom(sock, buffer, sizeof(buffer)-1, 0, (SOCKADDR *)&client_addr, &len) != SOCKET_ERROR)
		{
			printf("Incoming Data: %s \n", buffer);

			//if (strcmp(buffer, "Testing complete") == 0)
			if (sscanf(buffer, "Testing complete %d", &mark) == 1)
			{
				testing_complete = true;
				printf("Current mark = %d\n", mark);
			}
			else if (strcmp(buffer, "Error") == 0)
			{
				printf("ERROR> Testing abnormally terminated\n");
				testing_complete = true;
			}
			else
			{
				// Clear Registers and Memory

				Flags = 0;
				ProgramCounter = 0;
				StackPointer = 0;
				for (i=0; i<8; i++)
				{
					Registers[i] = 0;
				}
				for (i=0; i<MEMORY_SIZE; i++)
				{
					Memory[i] = 0;
				}

				// Load hex file

				i = 0;
				j = 0;
				load_at = 0;
				end_of_program = false;
				while (!end_of_program)
				{
					chr = buffer[i];
					switch (chr)
					{
					case '\0':
						end_of_program = true;

					case ',':
						if (sscanf(InputBuffer, "L=%x", &address) == 1)
						{
							load_at = address;
						}
						else if (sscanf(InputBuffer, "%x", &code) == 1)
						{
							if ((load_at >= 0) && (load_at <= MEMORY_SIZE))
							{
								Memory[load_at] = (BYTE)code;
							}
							load_at++;
						}
						else
						{
							printf("ERROR> Failed to load instruction: %s \n", InputBuffer);
						}
						j = 0;
						break;

					default:
						InputBuffer[j] = chr;
						j++;
						break;
					}
					i++;
				}

				// Emulate the 8080

				if (load_at > 1)
				{
					emulate_8080();

					// Send results

					sprintf(buffer, "%02X%02X %02X%02X %02X%02X %02X%02X %04X", Registers[REGISTER_A], Flags, Registers[REGISTER_B], Registers[REGISTER_C], Registers[REGISTER_D], Registers[REGISTER_E], Registers[REGISTER_H], Registers[REGISTER_L], StackPointer);
					sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *)&server_addr, sizeof(SOCKADDR));
				}
			}
		}
	}
}



int _tmain(int argc, _TCHAR* argv[])
{
	char chr;
	char dummy;

	printf("\n");
	printf("Intel 8080 Microprocessor Emulator\n");
	printf("UWE Computer and Network Systems Assignment 1 (2013-14)\n");
	printf("\n");

	initialise_filenames();

	if (WSAStartup(MAKEWORD(2, 2), &data) != 0) return(0);

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);  // Here we create our socket, which will be a UDP socket (SOCK_DGRAM).
	if (!sock)
	{
	 // Creation failed!
	}

	memset(&server_addr, 0, sizeof(SOCKADDR_IN));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP_ADDRESS_SERVER);
	server_addr.sin_port = htons(PORT_SERVER);

	memset(&client_addr, 0, sizeof(SOCKADDR_IN));
	client_addr.sin_family = AF_INET;
	client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	client_addr.sin_port = htons(PORT_CLIENT);

	//int ret = bind(sock, (SOCKADDR *)&client_addr, sizeof(SOCKADDR));
	//if (ret)
	//{
	//   //printf("Bind failed! \n");  // Bind failed!
	//}



	chr = '\0';
	while ((chr != 'e') && (chr != 'E'))
	{
		printf("\n");
		printf("Please select option\n");
		printf("L - Load and run a hex file\n");
		printf("T - Have the server test and mark your emulator\n");
		printf("E - Exit\n");
		printf("Enter option: ");
		chr = getchar();
		if (chr != 0x0A)
		{
			dummy = getchar();  // read in the <CR>
		}
		printf("\n");

		switch (chr)
		{
		case 'L':
		case 'l':
			load_and_run();
			break;

		case 'T':
		case 't':
			test_and_mark();
			break;

		default:
			break;
		}
	}

	closesocket(sock);
	WSACleanup();


	return 0;
}

