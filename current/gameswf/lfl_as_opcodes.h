// lfl_as_opcodes.h - by Sergey 'iOrange' Kudlay <programmist.c@gmail.com> 2010

// action script bytecode opcodes definition

#ifndef	__lfl_as_opcodes_h__
#define	__lfl_as_opcodes_h__

enum E_ACTIONSCRIPT_OPCODES
{
	AS_OPCODE_IfTrue				= 0x11,
	AS_OPCODE_IfFalse				= 0x12,
	AS_OPCODE_IfNotEqual			= 0x14,
	AS_OPCODE_PopScope				= 0x1D,
	AS_OPCODE_PushNull				= 0x20,
	AS_OPCODE_PushByte				= 0x24,
	AS_OPCODE_PushShort				= 0x25,
	AS_OPCODE_PushTrue				= 0x26,
	AS_OPCODE_PushFalse				= 0x27,
	AS_OPCODE_PopAndDiscard			= 0x29,
	AS_OPCODE_Dup					= 0x2A,
	AS_OPCODE_PushInt				= 0x2D,
	AS_OPCODE_PushString			= 0x2C,
	AS_OPCODE_PushDouble			= 0x2F,
	AS_OPCODE_PushScope				= 0x30,
	AS_OPCODE_CallProperty			= 0x46,
	AS_OPCODE_ReturnVoid			= 0x47,
	AS_OPCODE_ReturnValue			= 0x48,
	AS_OPCODE_ConstructSuper		= 0x49,
	AS_OPCODE_ConstructProp			= 0x4A,
	AS_OPCODE_ConstructPropVoid		= 0x4F,
	AS_OPCODE_NewArray				= 0x56,
	AS_OPCODE_NewClass				= 0x58,
	AS_OPCODE_FindPropStrict		= 0x5D,
	AS_OPCODE_FindProperty			= 0x5E,
	AS_OPCODE_GetLex				= 0x60,
	AS_OPCODE_SetProperty			= 0x61,
	AS_OPCODE_GetLocal				= 0x62,
	AS_OPCODE_SetLocal				= 0x63,
	AS_OPCODE_GetScopeObject		= 0x65,
	AS_OPCODE_GetProperty			= 0x66,
	AS_OPCODE_InitProperty			= 0x68,
	AS_OPCODE_Convert_i				= 0x73,
	AS_OPCODE_Coerce				= 0x80,
	AS_OPCODE_Coerce_s				= 0x85,
	AS_OPCODE_Not					= 0x96,
	AS_OPCODE_Add					= 0xA0,
	AS_OPCODE_Multiply				= 0xA2,
	AS_OPCODE_Equals				= 0xAB,
	AS_OPCODE_LessThen				= 0xAD,
	AS_OPCODE_IncLocal_i			= 0xC2,
	AS_OPCODE_GetLocal_0			= 0xD0,
	AS_OPCODE_GetLocal_1			= 0xD1,
	AS_OPCODE_GetLocal_2			= 0xD2,
	AS_OPCODE_GetLocal_3			= 0xD3,
	AS_OPCODE_SetLocal_0			= 0xD4,
	AS_OPCODE_SetLocal_1			= 0xD5,
	AS_OPCODE_SetLocal_2			= 0xD6,
	AS_OPCODE_SetLocal_3			= 0xD7,


	Last_AS_Opcode
};

#endif	// __lfl_as_opcodes_h__
