#ifndef RISCV_CPU_H
#define RISCV_CPU_H

#include <stdint.h>

#ifndef XLEN
#define XLEN 32
#endif

#if XLEN == 32
typedef uint32_t word_t;
#elif XLEN == 64
typedef uint64_t word_t;
#error "Not implemented yet!";
#else
#error "Invalid XLEN value";
#endif

struct Memory;

namespace riscv
{
struct Opcode
{
	enum Enum
	{
		Load = 0x03,
		MiscMem = 0x0F,
		OpImm = 0x13,
		AUIPC = 0x17,
		Store = 0x23,
		Op = 0x33,
		LUI = 0x37,
		Branch = 0x63,
		JALR = 0x67,
		JAL = 0x6F,
		System = 0x73
	};
};

struct ALUOp
{
	enum Enum
	{
		AddSub = 0,
		ShiftLeft = 1,
		SLT = 2,
		SLTU = 3,
		Xor = 4,
		ShiftRight = 5,
		Or = 6,
		And = 7
	};
};

struct BranchOp
{
	enum Enum
	{
		// 3 bits XYZ
		// Z = Polarity (0 = don't invert result, 1 = invert result)
		// Y = Sign (0 = signed comparison, 1 = unsigned comparison)
		// X = Inequality (0 = Equal, 1 = less than)
		// 01Z doesn't make sense since the sign doesn't make a difference when comparing 2 numbers for equality.
		Equal = 0,
		NotEqual = 1,
		LessThan = 4,
		GreaterEqual = 5,
		LessThanUnsigned = 6,
		GreaterEqualUnsigned = 7,
	};
};

struct CPUState
{
	// User-visible integer state
	word_t m_IRegs[32];
	word_t m_PC;
};

struct CPU
{
	CPUState m_State;
	CPUState m_NextState;
};

union Instruction
{
	uint32_t m_Word;

	struct _R
	{
		uint32_t opcode : 7;
		uint32_t rd : 5;
		uint32_t funct3 : 3;
		uint32_t rs1 : 5;
		uint32_t rs2 : 5;
		uint32_t funct7 : 7;
	} R;

	struct _I
	{
		uint32_t opcode : 7;
		uint32_t rd : 5;
		uint32_t funct3 : 3;
		uint32_t rs1 : 5;
		uint32_t imm : 12;
	} I;

	struct _S
	{
		uint32_t opcode : 7;
		uint32_t imm_0_4 : 5;
		uint32_t funct3 : 3;
		uint32_t rs1 : 5;
		uint32_t rs2 : 5;
		uint32_t imm_5_11 : 7;
	} S;

	struct _B
	{
		uint32_t opcode : 7;
		uint32_t imm_11 : 1;
		uint32_t imm_1_4 : 4;
		uint32_t funct3 : 3;
		uint32_t rs1 : 5;
		uint32_t rs2 : 5;
		uint32_t imm_5_10 : 6;
		uint32_t imm_12 : 1;
	} B;

	struct _U
	{
		uint32_t opcode : 7;
		uint32_t rd : 5;
		uint32_t imm_12_31 : 20;
	} U;

	struct _J
	{
		uint32_t opcode : 7;
		uint32_t rd : 5;
		uint32_t imm_12_19 : 8;
		uint32_t imm_11 : 1;
		uint32_t imm_1_10 : 10;
		uint32_t imm_20 : 1;
	} J;
};

inline word_t sext(uint32_t v, uint32_t signBitPos)
{
	const uint32_t shiftAmount = 31 - signBitPos;
	return (word_t)((((int32_t)v) << shiftAmount) >> shiftAmount);
}

inline word_t immI(Instruction instr)
{
	return sext(instr.I.imm, 11);
}

inline word_t immU(Instruction instr)
{
	return instr.U.imm_12_31 << 12;
}

inline word_t immS(Instruction instr)
{
	return sext(instr.S.imm_0_4 | (instr.S.imm_5_11 << 5), 11);
}

inline word_t immJ(Instruction instr)
{
	return sext((instr.J.imm_1_10 << 1) | (instr.J.imm_11 << 11) | (instr.J.imm_12_19 << 12) | (instr.J.imm_20 << 20), 20);
}

inline word_t immB(Instruction instr)
{
	return sext((instr.B.imm_1_4 << 1) | (instr.B.imm_5_10 << 5) | (instr.B.imm_11 << 11) | (instr.B.imm_12 << 12), 12);
}

word_t cpuGetPC(CPU* cpu);
void cpuSetPC(CPU* cpu, word_t val);
word_t cpuGetRegister(CPU* cpu, uint32_t reg);
void cpuSetRegister(CPU* cpu, uint32_t reg, word_t val);

// core_single_cycle.cpp
void cpuReset(CPU* cpu, word_t pc, word_t sp);
void cpuTick_SingleCycle(CPU* cpu, Memory* mem);

// disasm.cpp
void disasmInstruction(uint32_t instr, uint32_t addr, char* buf, uint32_t len);
}

#endif