// Sim8080.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <winsock2.h>

#pragma comment(lib, "wsock32.lib")


#define STUDENT_NUMBER    "123456789"

#define IP_ADDRESS_SERVER "127.0.0.1"

#define PORT_SERVER 0x1984 // We define a port that we are going to use.
#define PORT_CLIENT 0x1985 // We define a port that we are going to use.


#define WORD  unsigned short
#define DWORD unsigned long
#define BYTE  unsigned char

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
#define REGISTER_M	6
#define REGISTER_A	7

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


BYTE fetch()
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


// Add any instruction implementing routines here...



void set_flags(BYTE inReg)
{
	BYTE regA;
	int  bit_set_count;

	regA = inReg;

	if (regA == 0)			// set to zero
	{
		Flags = Flags | FLAG_Z;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_Z);
	}

	// Sign Flag
	if ((regA & 0x80) != 0)		//msbit set
	{
		Flags = Flags | FLAG_S;
	}
	else
	{
		Flags = Flags & (0xFF - FLAG_S);
	}

	//Parity Flag
	bit_set_count = 0;
	if ((regA & 0x01) != 0)	bit_set_count++;
	if ((regA & 0x02) != 0)	bit_set_count++;
	if ((regA & 0x04) != 0)	bit_set_count++;
	if ((regA & 0x08) != 0)	bit_set_count++;
	if ((regA & 0x10) != 0)	bit_set_count++;
	if ((regA & 0x20) != 0)	bit_set_count++;
	if ((regA & 0x40) != 0)	bit_set_count++;
	if ((regA & 0x80) != 0)	bit_set_count++;

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



void block_00_instructions(BYTE opcode)
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

	ls3bits = opcode & 0x07;

	switch (ls3bits)
	{
	case 0x00:	// NOP
		break;

	case 0x01:	// LXI and DAD

		if ((opcode & 0x08) == 0)
		{
			// LXI
			lb = fetch();
			hb = fetch();
			switch (opcode)
			{
			case 0x01: // LXI BC
				Registers[REGISTER_B] = hb;
				Registers[REGISTER_C] = lb;
				break;
			case 0x11: // LXI DE
				Registers[REGISTER_D] = hb;
				Registers[REGISTER_E] = lb;
				break;
			case 0x21: // LXI HL
				Registers[REGISTER_H] = hb;
				Registers[REGISTER_L] = lb;
				break;
			default: // LXI SP
				StackPointer = ((WORD)hb << 8) + (WORD)lb;
				break;
			}
	}
	else
	{
		}

		break;

	case 0x02:	// Load and Store instructions

		switch (opcode)
		{
		case 0x3A: // LDA
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;

			if ((address >= 0) && (address < MEMORY_SIZE))
			{
			Registers[REGISTER_A] = Memory[address];
			}
			else
			{
			Registers[REGISTER_A] = 0;
			}
			break;

			case 0x2A: // LHLD
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if ((address >= 0) && (address < (MEMORY_SIZE-1)))
			{
				Registers[REGISTER_L] = Memory[address];
				Registers[REGISTER_H] = Memory[address+1];
			}
			else
			{
				Registers[REGISTER_L] = 0;
				Registers[REGISTER_H] = 0;
			}
			break;

			case 0x22: //SHLD

				lb = fetch();
				hb = fetch();

				address = ((WORD)hb << 8) + (WORD)lb;

				if((address >=0) && (address < (MEMORY_SIZE-1)))
				{
					Memory[address+1] = Registers[REGISTER_H]; 
					Memory[address] = Registers[REGISTER_L]; 
				}

			break;

			case 0x32: // STA

				lb = fetch();
				hb = fetch();
				address = ((WORD)hb << 8) + (WORD)lb;

				if((address >= 0) && (address < MEMORY_SIZE))
				{
					 Memory[address] = Registers[REGISTER_A];
					
				}

			case 0x02: // STAX BC

				address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];

				if((address >=0) && (address < (MEMORY_SIZE) )) {
				Memory[address] = Registers[REGISTER_A];
					}

				break;

			case 0x12: // STAX DE

				address = ((WORD)Registers[REGISTER_D] << 8) + (WORD)Registers[REGISTER_E];

				if((address >=0) && (address < (MEMORY_SIZE) )) {
				Memory[address] = Registers[REGISTER_A];
					}

				break;

				case 0x0A: //LDAX B
			//load indirect through BC
			address = ((WORD)Registers[REGISTER_B] << 8) + (WORD)Registers[REGISTER_C];
			if ((address >= 0) && (address < (MEMORY_SIZE)))
			{
				Registers[REGISTER_A] = Memory[address];
			}
			else
			{
				Registers[REGISTER_A] = 0;
			}
			break;
		case 0x1A: //LDAX D
			//load indirect through DE
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

		destination = (opcode >> 3) & 0x07;

		switch(opcode)
	case 0x01: 

			break;

		break;

	case 0x04:	// INR D
		//Here is the code for incrementing (INR)
		//INR adds one to the contents of the specified byte. INR affects all of the condition flags except the carry flag.

		destination = (opcode >> 3) & 0x07;
		if(destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			//Here we check if the address is valid
			if((address >= 0) && (address < MEMORY_SIZE))
			{
				Memory[address]++;
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]++;
			set_flags(Registers[destination]);
		}

		break;

	case 0x05:	// DCR D
		//Here is the code for decrementing (DCR)
		//It is the same as the INR code, but uses a -- instead of a ++ within the if statement
		destination = (opcode >> 3) & 0x07;
		if(destination == REGISTER_M)
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			//Here we check if the address is valid
			if((address >= 0) && (address < MEMORY_SIZE))
			{
				Memory[address]--; //Here is the change between INR and DCR
				set_flags(Memory[address]);
			}
		}
		else
		{
			Registers[destination]--; //Here is the change between INR and DCR
			set_flags(Registers[destination]);
		}

		break;

	case 0x06:	// MVI

		destination = (opcode >> 3) & 0x07; Registers[destination] = fetch(); 
		if (destination == REGISTER_M) 
		{
			address = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];
			if ((address >= 0) && (address < MEMORY_SIZE)) 
			{ 
				Memory[address] = Registers[REGISTER_M];
			} 
		}

		break;

	default:	// Rotates,DAA and carry instructions
		break;
	}
}



void MOV_and_HLT_instructions(BYTE opcode)
{
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



void block_10_instructions(BYTE opcode)
{
	BYTE  source;
	BYTE  instruction_type;
	short temp = 0;
	WORD  address;

	instruction_type = (opcode >> 3) & 0x07;
	source = opcode & 0x07;

}



void block_11_instructions(BYTE opcode)
{
	WORD address;
	BYTE lb;
	BYTE hb;
	BYTE temp;
	WORD temp_word;

	switch (opcode)
	{
	case 0xC0:	// RNZ 

		if ((Flags & FLAG_Z) == 0)
		{
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

	case 0xC1:	// POP B 

		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE-2))
		{
			Registers[REGISTER_C] = Memory[StackPointer];
			StackPointer++;
			Registers[REGISTER_B] = Memory[StackPointer];
			StackPointer++;
		}

		break;

	case 0xC2:	// JNZ 



		break;

	case 0xC3:	// JMP 
		//This is the code for the jump function

		lb = fetch();
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;
		ProgramCounter = address;

		break;

	case 0xC4:	// CNZ 

		if ((Flags & FLAG_Z) == 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xC5:	// PUSH B 

		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE+1)))
		{
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_B];
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_C];
		}

		break;

	case 0xC6:	// ADI 
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;		//set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	//clear carry flag
		}
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xC7:	// RST 0   
		break;

	case 0xC8:	// RZ

		if ((Flags & FLAG_Z) != 0)
		{
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

	case 0xC9:	// RET 
		//This does the oposite of the CALL function, it pops 2 bytes of data and puts them in the program counter register

		if((StackPointer >=0) && (StackPointer < (MEMORY_SIZE-2)))
		{
			lb = Memory[StackPointer];
			StackPointer++;
			hb = Memory[StackPointer];
			StackPointer++;

			ProgramCounter = ((WORD)hb << 8) + (WORD)lb;
		}

		break;

	case 0xCA:	// JZ          
		break;

	case 0xCB:	// .BYTE 0xCB  
		break;

	case 0xCC:	// CZ   

		if ((Flags & FLAG_Z) != 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xCD:	// CALL  

		//Same for STA and LDA
		lb = fetch();
		hb = fetch();
		address = ((WORD)hb << 8) + (WORD)lb;

		if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE+1)))
		{
			StackPointer--;
			Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
			StackPointer--;
			Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
			ProgramCounter = address;
		}

		break;

	case 0xCE:	// ACI  
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] + (WORD)lb;
		if ((Flags & FLAG_C) != 0)
		{
			temp_word++;
		}
		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;		//set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	//clear carry flag
		}
		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xCF:	// RST 1       
		break;

	case 0xD0:	// RNC   

		if ((Flags & FLAG_C) == 0)
		{
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

	case 0xD1:	// POP D  

		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE-2))
		{
			Registers[REGISTER_E] = Memory[StackPointer];
			StackPointer++;
			Registers[REGISTER_D] = Memory[StackPointer];
			StackPointer++;
		}

		break;

	case 0xD2:	// JNC         
		break;

	case 0xD3:	// OUT         
		break;

	case 0xD4:	// CNC   

		if ((Flags & FLAG_C) == 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xD5:	// PUSH D 

		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE+1)))
		{
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_D];
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_E];
		}

		break;

	case 0xD6:	// SUI      

		lb = fetch(); 
		
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// Clears the carry flag by setting it to 0, This is done with a bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);

		set_flags(Registers[REGISTER_A]);

		break;

	case 0xD7:	// RST 2       
		break;

	case 0xD8:	// RC   

		if ((Flags & FLAG_C) != 0)
		{
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

	case 0xD9:	// .BYTE 0xD9  
		break;

	case 0xDA:	// JC          
		break;

	case 0xDB:	// IN          
		break;

	case 0xDC:	// CC    

		if ((Flags & FLAG_Z) != 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xDD:	// .BYTE 0xDD  
		break;

	case 0xDE:	// SBI 

		lb = fetch(); // fetches the Low byte
		if ((Flags & FLAG_C) !=0)
		{

			temp_word = (WORD)Registers[REGISTER_A] - (((WORD)lb) + 1);
		}
		else
		{
			temp_word = (WORD)Registers[REGISTER_A] - (((WORD)lb) + 0);
		}
		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// clear carry flag, this is done by setting it to 0. This is done with a bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);
		set_flags(Registers[REGISTER_A]);

		break;

	case 0xDF:	// RST 3       
		break;

	case 0xE0:	// RPO  

		if ((Flags & FLAG_P) == 0)
		{
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

	case 0xE1:	// POP H  

		if((StackPointer >= 0) && (StackPointer < MEMORY_SIZE-2))
		{
			Registers[REGISTER_L] = Memory[StackPointer];
			StackPointer++;
			Registers[REGISTER_H] = Memory[StackPointer];
			StackPointer++;
		}

		break;

	case 0xE2:	// JPO         
		break;

	case 0xE3:	// XTHL        
		break;

	case 0xE4:	// CPO  

		if ((Flags & FLAG_P) == 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xE5:	// PUSH H   

		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE+1)))
		{
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_H];
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_L];
		}

		break;

	case 0xE6:	// ANI  

		lb = fetch(); // fetches the Low byte
		
		temp_word = (WORD)Registers[REGISTER_A] & (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// clear carry flag, this is done by setting it to 0. This is done with a bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);

		set_flags(Registers[REGISTER_A]);

		break;

	case 0xE7:	// RST 4       
		break;

	case 0xE8:	// RPE   

		if ((Flags & FLAG_P) != 0)
		{
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

	case 0xE9:	// PCHL 
		break;

	case 0xEA:	// JPE         
		break;

	case 0xEB:	// XCHG

		temp = Registers[REGISTER_H];
		Registers[REGISTER_H] = Registers[REGISTER_D];
		Registers[REGISTER_D] = temp;

		temp = Registers[REGISTER_L];
		Registers[REGISTER_L] = Registers[REGISTER_E];
		Registers[REGISTER_E] = temp;

		break;

	case 0xEC:	// CPE  

		if ((Flags & FLAG_P) != 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xED:	// .BYTE 0xED  
		break;

	case 0xEE:	// XRI   

		lb = fetch(); // fetches the Low byte
		
		temp_word = (WORD)Registers[REGISTER_A] ^ (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// clear carry flag, this is done by setting it to 0. This is done with a bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);

		set_flags(Registers[REGISTER_A]);

		break;

	case 0xEF:	// RST 5       
		break;

	case 0xF0:	// RP 

		if ((Flags & FLAG_S) == 0)
		{
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

	case 0xF1:	// POP PSW 
		break;

	case 0xF2:	// JP          
		break;

	case 0xF3:	// DI          
		break;

	case 0xF4:	// CP 

		if ((Flags & FLAG_S) == 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xF5:	// PUSH PSW  

		if ((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE+1)))
		{
			if((Flags & FLAG_C) != 0) {
				StackPointer--;
			}
			StackPointer--;
			Memory[StackPointer] = Registers[REGISTER_A];
			StackPointer--;
			
		}

		break;

	case 0xF6:	// ORI  

		lb = fetch(); // fetches the Low byte
		
		temp_word = (WORD)Registers[REGISTER_A] | (WORD)lb;

		if (temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C; // set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	// clear carry flag, this is done by setting it to 0. This is done with a bitwise &
		}

		Registers[REGISTER_A] = (BYTE)(temp_word & 0xFF);

		set_flags(Registers[REGISTER_A]);

		break;

	case 0xF7:	// RST 6       
		break;

	case 0xF8:	// RM 

		if ((Flags & FLAG_S) != 0)
		{
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

	case 0xF9:	// SPHL     

		StackPointer = ((WORD)Registers[REGISTER_H] << 8) + (WORD)Registers[REGISTER_L];

		break;

	case 0xFA:	// JM          
		break;

	case 0xFB:	// EI          
		break;

	case 0xFC:	// CM   

		if ((Flags & FLAG_S) != 0)
		{
			//flag is not set
			lb = fetch();
			hb = fetch();
			address = ((WORD)hb << 8) + (WORD)lb;
			if((StackPointer >= 2) && (StackPointer < (MEMORY_SIZE + 1)))
			{
				StackPointer--;
				Memory[StackPointer] = (BYTE)((ProgramCounter >> 8) & 0xFF);
				StackPointer--;
				Memory[StackPointer] = (BYTE)(ProgramCounter & 0xFF);
				ProgramCounter = address;
			}
		}

		break;

	case 0xFD:	// .BYTE 0xFD  
		break;

	case 0xFE:	// CPI
		lb = fetch();
		temp_word = (WORD)Registers[REGISTER_A] - (WORD)lb;
		if(temp_word >= 0x100)
		{
			Flags = Flags | FLAG_C;		//set carry flag
		}
		else
		{
			Flags = Flags & (0xFF - FLAG_C);	//clear carry flag
		}
		set_flags(Registers[REGISTER_A]);
		break;

	case 0xFF:	// RST 7      
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
		execute(opcode);

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

