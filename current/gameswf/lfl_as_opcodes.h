// lfl_as_opcodes.h - by Sergey 'iOrange' Kudlay <programmist.c@gmail.com> 2010

// action script bytecode opcodes definition

#ifndef	__lfl_as_opcodes_h__
#define	__lfl_as_opcodes_h__

enum E_ACTIONSCRIPT_OPCODES
{
    AS_OPCODE_ExeptionTrown         = 0x03,
    AS_OPCODE_Label                 = 0x09,
    
    AS_OPCODE_Jump                  = 0x10, //Unconditional branch

    /*
    The conditional branches instructions include iflt, ifle, ifnlt, ifnle, ifgt, ifge, ifngt, ifnge, ifeq,
    ifne, ifstricteq, ifstrictne, iftrue, and iffalse. These instructions perform any necessary type
    conversions in order to implement the compare; the conversion rules are outlined in ECMA-262.
    */
    AS_OPCODE_IfTrue                = 0x11, //iftrue. Branch if true.
    AS_OPCODE_IfFalse               = 0x12, //iffalse. Branch if false.
    AS_OPCODE_IfEqual               = 0x13, //ifeq. Branch if the first value is equal to the second value.
    AS_OPCODE_IfNotEqual            = 0x14, //ifne. Branch if the first value is not equal to the second value.
    AS_OPCODE_IfLessThan            = 0x15, //iflt. Branch if the first value is less than the second value.
    AS_OPCODE_IfLessThanOrEqual     = 0x16, //ifle. Branch if the first value is less than or equal to the second value.
    AS_OPCODE_IfGreaterThan         = 0x17, //ifgt. Branch if the first value is greater than the second value.
    AS_OPCODE_IfGreaterThanOrEqual  = 0x18, //ifge. Branch if the first value is greater than or equal to the second value.
    AS_OPCODE_IfStrictEqual         = 0x19, //ifstricteq. Compute value1 === value2 using the strict equality comparison algorithm in ECMA-262 section 11.9.6.
    AS_OPCODE_IfStrictnotEqual      = 0x1A, //ifstrictne. Compute value1 === value2 using the strict equality comparison algorithm in ECMA-262 section 11.9.6.
    AS_OPCODE_IfNotLessThan         = 0x0C, //ifnlt. Branch if the first value is not less than the second value.
    AS_OPCODE_IfNotLessThanOrEqual  = 0x0D, //ifnle. Branch if the first value is not less than or equal to the second value.
    AS_OPCODE_NotGreaterThan        = 0x0E, //ifngt. Branch if the first value is not greater than the second value.
    AS_OPCODE_IfNotGreaterThanOrEqual = 0x0F, //ifnge. Branch if the first value is not greater than or equal to the second value.

    AS_OPCODE_GreaterThan           = 0xAF, //greaterthan
    AS_OPCODE_GreaterThanOrEqual    = 0xB0, //greaterequals

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
    AS_OPCODE_Convert_b             = 0x76,
    AS_OPCODE_Coerce				= 0x80,
    AS_OPCODE_Coerce_s				= 0x85,
    AS_OPCODE_Not					= 0x96,
    AS_OPCODE_Add					= 0xA0,
    AS_OPCODE_Increment_i           = 0xC0,
    AS_OPCODE_Decrement_i           = 0xC1,
    AS_OPCODE_Increment             = 0x91,
    AS_OPCODE_Decrement             = 0x93,
    AS_OPCODE_Add_i                 = 0xC5,
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
    AS_OPCODE_Debug                 = 0xEF,
    AS_OPCODE_DebugLine             = 0xF0,
    AS_OPCODE_DebugFile             = 0xF1,

    Last_AS_Opcode
};

#endif	// __lfl_as_opcodes_h__
