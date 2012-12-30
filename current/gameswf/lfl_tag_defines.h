// lfl_tag_defines.h - by Timur Losev 2010

// SWF file tags definition

#ifndef	__lfl_tag_defines_h__
#define	__lfl_tag_defines_h__



/*
	0 End
	1 ShowFrame
	2 DefineShape

	4 PlaceObject
	5 RemoveObject
	6 DefineBits
	7 DefineButton
	8 JPEGTables
	9 SetBackgroundColor
	10 DefineFont
	11 DefineText
	12 DoAction
	13 DefineFontInfo
	14 DefineSound
	15 StartSound

	17 DefineButtonSound
	18 SoundStreamHead
	19 SoundStreamBlock
	20 DefineBitsLossless
	21 DefineBitsJPEG2
	22 DefineShape2
	23 DefineButtonCxform
	24 Protect

	26 PlaceObject2

	28 RemoveObject2

	32 DefineShape3
	33 DefineText2
	34 DefineButton2
	35 DefineBitsJPEG3
	36 DefineBitsLossless2
	37 DefineEditText

	39 DefineSprite

	43 FrameLabel

	45 SoundStreamHead2
	46 DefineMorphShape

	48 DefineFont2

	56 ExportAssets
	57 ImportAssets
	58 EnableDebugger
	59 DoInitAction
	60 DefineVideoStream
	61 VideoFrame
	62 DefineFontInfo2

	64 EnableDebugger2
	65 ScriptLimits
	66 SetTabIndex

	69 FileAttributes
	70 PlaceObject3
	71 ImportAssets2
	73 DefineFontAlignZones
	74 CSMTextSettings
	75 DefineFont3
	76 SymbolClass
	77 Metadata
	78 DefineScalingGrid

	82 DoABC
	83 DefineShape4
	84 DefineMorphShape2

	86 DefineSceneAndFrameLabelData
	87 DefineBinaryData
	88 DefineFontName
	89 StartSound2
	90 DefineBitsJPEG4
	91 DefineFont4
*/

//----------------------------------------------------------------
/*tag name/tag value*/
//----------------------------------------------------------------
enum E_TAG_DEFINES
{
	TAG_End								= 0,
	TAG_ShowFrame						= 1,
	TAG_DefineShape						= 2,

	TAG_PlaceObject						= 4,
	TAG_RemoveObject					= 5,
	TAG_DefineBits						= 6,
	TAG_DefineButton					= 7,
	TAG_JPEGTables						= 8,
	TAG_SetBackgroundColor				= 9,
	TAG_DefineFont						= 10,
	TAG_DefineText						= 11,
	TAG_DoAction						= 12,
	TAG_DefineFontInfo					= 13,
	TAG_DefineSound						= 14,
	TAG_StartSound						= 15,

	TAG_DefineButtonSound				= 17,
	TAG_SoundStreamHead					= 18,
	TAG_SoundStreamBlock				= 19,
	TAG_DefineBitsLossless				= 20,
	TAG_DefineBitsJPEG2					= 21,
	TAG_DefineShape2					= 22,
	TAG_DefineButtonCxform				= 23,
	TAG_Protect							= 24,

	TAG_PlaceObject2					= 26,

	TAG_RemoveObject2					= 28,

	TAG_DefineShape3					= 32,
	TAG_DefineText2						= 33,
	TAG_DefineButton2					= 34,
	TAG_DefineBitsJPEG3					= 35,
	TAG_DefineBitsLossless2				= 36,
	TAG_DefineEditText					= 37,

	TAG_DefineSprite					= 39,

	TAG_FrameLabel						= 43,

	TAG_SoundStreamHead2				= 45,
	TAG_DefineMorphShape				= 46,

	TAG_DefineFont2						= 48,

	TAG_ExportAssets					= 56,
	TAG_ImportAssets					= 57,
	TAG_EnableDebugger					= 58,
	TAG_DoInitAction					= 59,
	TAG_DefineVideoStream				= 60,
	TAG_VideoFrame						= 61,
	TAG_DefineFontInfo2					= 62,

	TAG_EnableDebugger2					= 64,
	TAG_ScriptLimits					= 65,
	TAG_SetTabIndex						= 66,

	TAG_FileAttributes					= 69,
	TAG_PlaceObject3					= 70,
	TAG_ImportAssets2					= 71,

	TAG_DefineFontAlignZones			= 73,
	TAG_CSMTextSettings					= 74,
	TAG_DefineFont3						= 75,
	TAG_SymbolClass						= 76,
	TAG_Metadata						= 77,
	TAG_DefineScalingGrid				= 78,

	TAG_DoABC							= 82,
	TAG_DefineShape4					= 83,
	TAG_DefineMorphShape2				= 84,

	TAG_DefineSceneAndFrameLabelData	= 86,
	TAG_DefineBinaryData				= 87,
	TAG_DefineFontName					= 88,
	TAG_StartSound2						= 89,
	TAG_DefineBitsJPEG4					= 90,
	TAG_DefineFont4						= 91,

	Last_Defined_TAG	/* iOrange - reservation */
};

#endif	// __lfl_tag_defines_h__
