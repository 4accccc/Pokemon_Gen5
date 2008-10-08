/**********************************************************************

Copyright (c) 2005 Polygon Magic, Inc.

************************************************************************/

#ifndef NPBPC_H
#define NPBPC_H

/* 
	Model definition
*/
typedef signed char		char_t;
typedef signed char		int8_t;
typedef signed short	int16_t;
typedef signed int		int32_t;

typedef unsigned char	uchar_t;
typedef unsigned char	uint8_t;
typedef unsigned short	uint16_t;
typedef unsigned int	uint32_t;

#define	BPC_FILE_ID	0x584D4E41	// ANMX

/*
	BPC file structure object
*/
typedef struct __BPCFILEHEADER {
	char_t		id[ 4 ];	/* ANMX */
	uint32_t	version;	/* Version */
	uint32_t	filesize;	/* File size */
} BPCFILEHEADER, *LPBPCFILEHEADER;

enum {
	OBJ_NAME_LENGTH_MAX	= 32,		//!< The maximum length of the name of an object.
	CTRLTASK_MAX		= 512,		//!< The generation maximum number of a control task.
	PRIMTASK_MAX		= 1024,		//!< The generation maximum number of a primitive task
	PARAM_LOOP_MAX		= 1024,		//!< The number of times of the maximum repetition of a parameter
	PARAM_LOOP_IFN		= -1		//!< Repetition infinity of a parameter.
};

// Message.
enum BPC_MESSAGE { 
	OBJECT_CHANGE		= 1,
	FORM_CLOSE,						// Close to child form.
	EXIT_GENERATE_MODE				// Exit gnerate mode.
};

enum {
	TRANS_CHANGE	= 1,
	SIZE_CHANGE		= TRANS_CHANGE << 1,
	COLOR_CHANGE	= SIZE_CHANGE << 1
};

//! The kind of generation object
enum TASK_TYPE {
	CTRL_TASK_TYPE,					//!< TTaskProto, TObjTask, TObjRoot
	PRIM_TASK_TYPE					//!< TItemProto, TObjPrim
};

//! The list of a data change pattern to time.
enum ANM_PATTERN {
	NO_PATTERN		= 0,		//!< An unsuitable pattern (For un-mounting)
	NEWTON_PATTERN	= 1,		//!< A dynamic change pattern "speed and acceleration"
	FIXED_PATTERN,				//!< Initial value fixation.
	HISTGRAM_PATTERN,			//!< Histogram.
	SEQ_LINE_PATTERN,			//!< Polygonal line pattern.
	SEQ_RANDOM_PATTERN,			//!< Specification value random pattern.
	SCROLL_PATTERN,				//!< Scrolling.
	BURST_PATTERN,				//!< Diffusion pattern.
	NULL_PATTERN,				//!< NULL	Don't move from here.
	FCURVE_PATTERN				//!< F-Curve.
};

//! The list of a generation pattern to time.
enum GEN_PATTERN {
	GEN_CONSTANT	= 1,			//!< Regular-intervals pattern generation.
	GEN_SEQ						//!< Timetable generation.
};

//!	Src alpha type
enum SRC_ALPHA_TYPE {
	SAT_BLEND_ZERO,
	SAT_BLEND_ONE,
	SAT_DST_COLOR,
	SAT_M_DST_COLOR,
	SAT_SRC_ALPHA,
	SAT_M_SRC_ALPHA,
	SAT_DST_ALPHA,
	SAT_M_DST_ALPHA
};

//! Dst alpha type
enum DST_ALPHA_TYPE {
	DAT_BLEND_ZERO,
	DAT_BLEND_ONE,
	DAT_SRC_COLOR,
	DAT_M_SRC_COLOR,
	DAT_SRC_ALPHA,
	DAT_M_SRC_ALPHA,
	DAT_DST_ALPHA,
	DAT_M_DST_ALPHA
};

//! A simple setup in translucence mode
enum BLEND_MODE {
	BLEND_NONE,			//!< no blending
	ADDITION_01,		//!< Penetration (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)
	ADDITION_00,		//!< Addition (ONE, ONE)
	SUBTRACTION_01,		//!< Subtraction (ONE_MINUS_SRC_ALPHA, SRC_ALPHA)
	TRANSPARENT_00,		//!< Addition (SRC_ALPHA, ONE_MINUS_SRC_ALPHA)

	ZERO_ZERO,
	ZERO_ONE,
	ZERO_SCOLOR,
	ZERO_M_SCOLOR,
	ZERO_SALPHA,
	ZERO_M_SALPHA,
	ZERO_DALPHA,
	ZERO_M_DALPHA,

	ONE_ZERO,
    ONE_ONE,  
    ONE_SCOLOR,
    ONE_M_SCOLOR,
    ONE_SALPHA,
    ONE_M_SALPHA,
    ONE_DALHPA,
    ONE_M_DALPHA,

	DCOLOR_ZERO,
    DCOLOR_ONE,
    DCOLOR_SCOLOR,
    DCOLOR_M_SCOLOR,
    DCOLOR_SALPHA,
    DCOLOR_M_SALPHA,
    DCOLOR_DALHPA,
    DCOLOR_M_DALPHA,

	M_DCOLOR_ZERO,
    M_DCOLOR_ONE,
    M_DCOLOR_SCOLOR,
    M_DCOLOR_M_SCOLOR,
    M_DCOLOR_SALPHA,
    M_DCOLOR_M_SALPHA,
    M_DCOLOR_DALHPA,
    M_DCOLOR_M_DALPHA,

	SALPHA_ZERO,
    SALPHA_ONE,
    SALPHA_SCOLOR,
    SALPHA_M_SCOLOR,
    SALPHA_SALPHA,
    SALPHA_M_SALPHA,
    SALPHA_DALHPA,
    SALPHA_M_DALPHA,

	M_SALPHA_ZERO,
    M_SALPHA_ONE,
    M_SALPHA_SCOLOR,
    M_SALPHA_M_SCOLOR,
    M_SALPHA_SALPHA,
    M_SALPHA_M_SALPHA,
    M_SALPHA_DALHPA,
    M_SALPHA_M_DALPHA,

	DALPHA_ZERO,
    DALPHA_ONE,
    DALPHA_SCOLOR,
    DALPHA_M_SCOLOR,
    DALPHA_SALPHA,
    DALPHA_M_SALPHA,
    DALPHA_DALHPA,
	DALPHA_M_DALPHA,

	M_DALPHA_ZERO,
    M_DALPHA_ONE,
    M_DALPHA_SCOLOR,
    M_DALPHA_M_SCOLOR,
    M_DALPHA_SALPHA,
    M_DALPHA_M_SALPHA,
    M_DALPHA_DALHPA,
	M_DALPHA_M_DALPHA
};

/*!	@enum	Vertex_Mode
 	@brief	Vertex setting mode
 */
enum Vertex_Mode {
	VERTEX_MODE_NONE,		//!< With no setting mode.
	VERTEX_MODE_SIZE,		//!< Size specification.
	VERTEX_MODE_EVERY4,		//!< 4 vertex specification.
	VERTEX_MODE_FCURVE		//!< F-Curve size specification.
};

//! The list of the common attribute flags of a task
enum TASK_ATTRIBUTE {
	TYPE_BCB = 1,							//!< Billboardか
	TYPE_QCB = TYPE_BCB<<1,					//!< Square polygon.
	TYPE_SCB = TYPE_QCB<<1,
	TYPE_FSB = TYPE_SCB<<1,					//!< エミッター

	TYPE_BQSF_ALL	= TYPE_BCB|TYPE_QCB|TYPE_SCB|TYPE_FSB,
	
	TA_TEXTURE_ENABLE = TYPE_FSB<<1,		//!< Use of a texture.
	TA_SHADE_SMOOTH = TA_TEXTURE_ENABLE<<1,	//!< Smooth shading.
	TA_ALPHA_ENABLE = TA_SHADE_SMOOTH<<1,	//!< Use of alpha.
	TA_FOG_ENABLE	= TA_ALPHA_ENABLE<<1,	//!< Use of fog.

	ASPECT_LOCK		= TA_FOG_ENABLE << 1,	//!< The percentage of length and width is fixed.
	//RESERVE_08		= TA_FOG_ENABLE<<1,	// ---------------
	HAVE_LIGHT_BOTH	= ASPECT_LOCK<<1,	// 両面ライト計算
	HAVE_GRAVITY	= HAVE_LIGHT_BOTH<<1,	//!< Have a gravity point?
	HAVE_LIGHT		= HAVE_GRAVITY<<1,	//!< Have a light source?

	HAVE_PARENT		= HAVE_LIGHT<<1,	//!< A parent task existent?
	LIFE_DEPEND		= HAVE_PARENT<<1,	//!< Refer to parents' life.(Dependence)
	TIMER_KILL		= LIFE_DEPEND<<1,	//!< Does it end by Timer?
	TRANS_KILL		= TIMER_KILL<<1,	//!< Does it end with a move end?

	ROT_KILL		= TRANS_KILL<<1,	//!< Does it end with a rotation end?
	SCALE_KILL		= ROT_KILL<<1,		//!< Does it end with a magnification change end?
	ALPHA_KILL		= SCALE_KILL<<1,	//!< Does it end with a translucence change end?
	TEXTURE_KILL	= ALPHA_KILL<<1,	//!< Does it end with a Texture change end?

	CHILD_KILL		= TEXTURE_KILL<<1,	//!< Does it end at the time of the end of a child task?
	
	TYPE_AXIS_BCB	= CHILD_KILL<<1,	// 軸固定ビルボード
	HAVE_ZSORT		= TYPE_AXIS_BCB<<1,	// Zソートをする
	HAVE_INV_PRIORITY	= HAVE_ZSORT<<1,// 手動プリオリティをしない

	RESERVE_24		= HAVE_INV_PRIORITY<<1,	// ----------------
	UNDER_P_ROT_GEN	= RESERVE_24<<1,	//!< Is rotation of a parent task used only at the time of generation?
	HAVE_EXT_ATTR	= UNDER_P_ROT_GEN<<1,	//!< Does it have an extended attribute?
	HAVE_GEN_POINT	= HAVE_EXT_ATTR<<1,	//!< Is there any generation arrangement coordinates data?
	
	KILL_CHILD		= HAVE_GEN_POINT<<1,//!< //!< Is a child task destroy at the time of an end?
	UNDER_GRAVITY	= KILL_CHILD<<1,	//!< Is the gravitation of a parent task used?
	UNDER_P_TRANS	= UNDER_GRAVITY<<1,	//!< Is Trans of a parent task used?
	UNDER_P_ROT		= UNDER_P_TRANS<<1,		//!< Is rotation of a parent task used?(UNDER_P_ROT_GEN must be exclusion.)

	//! すべての終了条件フラグ
	HAVE_KILL_STATE	= TIMER_KILL|TRANS_KILL|ROT_KILL|SCALE_KILL|ALPHA_KILL|TEXTURE_KILL|CHILD_KILL
};

//! Classification of 3D element.
enum	BPC_ELEMENT_TYPE{
	MANAGE_ELEMENT		= 0,		//!< MngObj is generated.
	SQUARE_PRIM_ELEMNT	= 1		//!< SqrObj is generated.
};

/*!	@enum	Sqr_Attrib_Flag
 	@brief	The flag of a quadrangle attribute.
 */
enum Sqr_Attrib_Flag{
	SAF_EXT_SQUARE_ROTATE	= 1,		//!< Extented Square (The five vertexs)
	SAF_EXT_SQUARE_MIRROR	= SAF_EXT_SQUARE_ROTATE<<1,	//!< Mirror Square
	SAF_EXT_SQUARE_MIRROR2	= SAF_EXT_SQUARE_MIRROR<<1,	//!< Mirror Square2(未実装)

	SAF_EXT_SQUARE_RESERVE1	= SAF_EXT_SQUARE_MIRROR2<<1,	//!< Reservation bit 1.
	SAF_EXT_SQUARE_RESERVE2	= SAF_EXT_SQUARE_RESERVE1<<1,	//!< Reservation bit 2.

	SAF_BCB_PARENT_Z_ENABLE	= SAF_EXT_SQUARE_RESERVE2,	//!< The flag of whether Billboard uses a parent Z angle.

	//! The flag of an extended polygon setup.
	SAF_EXT_SQUARE_ALLFLAG	= SAF_EXT_SQUARE_ROTATE|SAF_EXT_SQUARE_MIRROR|SAF_EXT_SQUARE_MIRROR2
};
//! FCurve interporation type
enum BPC_FCURVE_INTERP_TYPE{
	BPC_FCURVE_INTERP_LINEAR = 0,	//!< Linear interporation.
	BPC_FCURVE_INTERP_HERMITE		//!< Hermite interporation.
};

#endif	/* NPBPC_H */
