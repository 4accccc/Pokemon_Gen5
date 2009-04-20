//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカルのエディタ
 * @author	ariizumi
 * @data	09/03/02
 */
//======================================================================
#include <gflib.h>
#include "system/main.h"
#include "system/gfl_use.h"
#include "arc_def.h"
#include "font/font.naix"
#include "mus_debug.naix"
#include "message.naix"
#include "msg/msg_mus_item_name.h"

#include "poke_tool/poke_tool.h"
#include "poke_tool/monsno_def.h"

#include "print/printsys.h"
#include "print/str_tool.h"
#include "print/global_msg.h"

#include "test/ariizumi/musical_editor.h"
#include "musical/musical_system.h"
#include "musical/musical_stage_sys.h"
#include "musical/mus_item_draw.h"

#include "musical/stage/script/sta_act_script_def.h"
#include "musical/stage/sta_acting.h"
#include "musical/stage/sta_act_poke.h"

#include "test/soga/gf_mcs.h"
#include "test/ariizumi/ari_debug.h"

#if USE_MUSICAL_EDIT

//======================================================================
//	define
//======================================================================
#pragma mark [>define
#define	HEAD_SIZE			( 4 )

#define EDIT_FRAME_SUB_BG		GFL_BG_FRAME1_S

#define EDIT_PAL_FONT		(0xF)

#define	MCS_READ_CH			( 0 )
#define	MCS_WRITE_CH		( 0 )

#define EDIT_STATE_FIRST	(0xFE)
#define EDIT_STATE_NONE	(0xFF)
#define EDIT_STATE_ON	(0x01)

//======================================================================
//	enum
//======================================================================
#pragma mark [>enum
enum	//メインシーケンス
{
	STA_SEQ_INIT_ACTING,	//ステージ演技部分
	STA_SEQ_LOOP_ACTING,
	STA_SEQ_TERM_ACTING,
};

//MCS用定義
enum
{
	SEND_SEQUENCE = 1,
	SEND_SEQUENCE_OK,
	SEND_RESOURCE,
	SEND_RESOURCE_OK,
	SEND_RECEIVE,
	SEND_RECEIVE_OK,
	SEND_DECIDE,
	SEND_CANCEL,
	SEND_IDLE,
};

//MCS用Seq
typedef enum
{
	MSEQ_WAIT,
	MSEQ_LOAD_SEQ,
	MSEQ_LOAD_RES,
	MSEQ_SEND,
}MSC_SEQ;

//ポケ画面Win
enum
{
	EDIT_POKEWIN_POKENAME,
	EDIT_POKEWIN_POS,
	EDIT_POKEWIN_EQUIP,
//	EDIT_POKEWIN_POSITION,

	EDIT_POKEWIN_MAX,
};
//サブWin
enum
{
	EDIT_SUBWIN_OBJNAME,
	EDIT_SUBWIN_LIGHT,

	EDIT_SUBWIN_MAX,
};
//======================================================================
//	typedef struct
//======================================================================
#pragma mark [>struct
typedef struct
{
	HEAPID				heapId;
	ACTING_INIT_WORK	*actInitWork;
	ACTING_WORK			*actWork;
	
	MSC_SEQ				mcsSeq;
	
	GFL_BMPWIN			**winArr;
	GFL_BMPWIN			**winArrSub;
	GFL_FONT			*fontHandle;
	GFL_MSGDATA			*itemMsgHandle;
	
	u8		dispEquipPos[4];
	u8		objNum;
	u8		befObjId;
	u8		lightNum;
	u8		befLightState;
}MUS_EDIT_LOCAL_WORK;

//======================================================================
//	proto
//======================================================================
#pragma mark [>prot
static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk );
static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk );

static void MusicalEdit_InitGraphic( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_TermGraphic( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_InitBgPokeEquip( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_TermBgPokeEquip( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_PrintFunc( MUS_EDIT_LOCAL_WORK *work , GFL_BMPWIN *win , STRBUF *str );
static void MusicalEdit_DrawPokeData( MUS_EDIT_LOCAL_WORK *work , const u8 pokeNo );
static void MusicalEdit_DrawObjData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_DrawLightData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_UpdatePokeData( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y );
static void MusicalEdit_UpdateTouch( MUS_EDIT_LOCAL_WORK *work );

static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work );
static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work );

GFL_PROC_DATA MusicalEdit_ProcData =
{
	MusicalEditProc_Init,
	MusicalEditProc_Main,
	MusicalEditProc_Term
};

FS_EXTERN_OVERLAY(sogabe_debug);

//--------------------------------------------------------------
//	
//--------------------------------------------------------------

static GFL_PROC_RESULT MusicalEditProc_Init( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	int ePos;
	MUS_EDIT_LOCAL_WORK *work;
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MUSICAL_STAGE, 0x100000 );

	work = GFL_PROC_AllocWork( proc, sizeof(MUS_EDIT_LOCAL_WORK), HEAPID_MUSICAL_STAGE );
	work->actInitWork = GFL_HEAP_AllocMemory( HEAPID_MUSICAL_STAGE , sizeof(ACTING_INIT_WORK) );
	work->actInitWork->heapId = HEAPID_MUSICAL_STAGE;
	work->actInitWork->musPoke[0].pokePara = PP_Create( MONSNO_PIKUSII , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[1].pokePara = PP_Create( MONSNO_RAITYUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[2].pokePara = PP_Create( MONSNO_EREBUU , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	work->actInitWork->musPoke[3].pokePara = PP_Create( 495 , 20 , PTL_SETUP_POW_AUTO , HEAPID_MUSICAL_STAGE );
	for( ePos=0;ePos<MUS_POKE_EQUIP_MAX;ePos++ )
	{
		work->actInitWork->musPoke[0].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[1].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[2].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
		work->actInitWork->musPoke[3].equip[ePos].itemNo = MUSICAL_ITEM_INVALID;
	}
	
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HAND_R].itemNo = 13;
	work->actInitWork->musPoke[0].equip[MUS_POKE_EQU_HEAD].itemNo = 16;
	
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_EAR_L].itemNo = 7;
	work->actInitWork->musPoke[1].equip[MUS_POKE_EQU_BODY].itemNo = 9;
	
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HAND_R].itemNo = 31;
	work->actInitWork->musPoke[2].equip[MUS_POKE_EQU_HEAD].itemNo = 15;
	
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HAND_R].itemNo = 30;
	work->actInitWork->musPoke[3].equip[MUS_POKE_EQU_HEAD].itemNo = 21;

/*
	for( ePos=0;ePos<4;ePos++ )
	{
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HAND_R].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HAND_L].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_HEAD].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_EAR_L].type = 30;
		work->actInitWork->musPoke[ePos].equip[MUS_POKE_EQU_BODY].type = 30;
	}
*/	

	//mcs用初期化
	work->heapId = HEAPID_MUSICAL_STAGE;
	work->mcsSeq = MSEQ_WAIT;
	GFL_OVERLAY_Load(FS_OVERLAY_ID(sogabe_debug));
	
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Term( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	MUS_EDIT_LOCAL_WORK *work = mywk;
	

	GFL_OVERLAY_Unload(FS_OVERLAY_ID(sogabe_debug));
	
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[0].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[1].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[2].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork->musPoke[3].pokePara );
	GFL_HEAP_FreeMemory( work->actInitWork );
	GFL_PROC_FreeWork( proc );
	GFL_HEAP_DeleteHeap( HEAPID_MUSICAL_STAGE );

	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT MusicalEditProc_Main( GFL_PROC * proc, int * seq , void *pwk, void *mywk )
{
	MUS_EDIT_LOCAL_WORK *work = mywk;
	switch( *seq )
	{
	case STA_SEQ_INIT_ACTING:
		work->actWork = STA_ACT_InitActing( work->actInitWork );
		MCS_Init( work->heapId );
		MusicalEdit_InitGraphic( work );
		MusicalEdit_InitBgPokeEquip( work );
		*seq = STA_SEQ_LOOP_ACTING;
		break;
		
	case STA_SEQ_LOOP_ACTING:
		{
			ACTING_RETURN ret;
			ret = STA_ACT_LoopActing( work->actWork );
			MusicalEdit_McsMain( work );
			if( ret == ACT_RET_GO_END )
			{
				*seq = STA_SEQ_TERM_ACTING;
			}
			MusicalEdit_UpdateTouch( work );
			MusicalEdit_UpdatePokeData( work );
		}
		break;

	case STA_SEQ_TERM_ACTING:
		MusicalEdit_TermBgPokeEquip( work );
		MusicalEdit_TermGraphic( work );
		STA_ACT_TermActing( work->actWork );
		MCS_Exit();
		return GFL_PROC_RES_FINISH;
		break;
	}
	
	return GFL_PROC_RES_CONTINUE;
}

#pragma mark [>bg func
static void MusicalEdit_InitGraphic( MUS_EDIT_LOCAL_WORK *work )
{
	GFL_ARC_UTIL_TransVramPalette( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCLR , 
										PALTYPE_SUB_BG , 0 , sizeof(u16)*16 , work->heapId );
	GFL_ARC_UTIL_TransVramBgCharacter( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_NCGR ,
										EDIT_FRAME_SUB_BG , 0 , 0, FALSE , work->heapId );
	
	//フォント読み込み
	work->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_small_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , work->heapId );
	GFL_FONTSYS_SetColor( 1,0,0 );
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , EDIT_PAL_FONT*0x20, 16*sizeof(u16), work->heapId );

	//メッセージ
	work->itemMsgHandle = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL , ARCID_MESSAGE , NARC_message_mus_item_name_dat , work->heapId );

}

static void MusicalEdit_TermGraphic( MUS_EDIT_LOCAL_WORK *work )
{
	GFL_MSG_Delete( work->itemMsgHandle );
	GFL_FONTSYS_SetDefaultColor();
	GFL_FONT_Delete( work->fontHandle );
}

static void MusicalEdit_InitBgPokeEquip( MUS_EDIT_LOCAL_WORK *work )
{
	u8 i;
	GFL_ARC_UTIL_TransVramScreen( ARCID_MUSICAL_DEBUG , NARC_mus_debug_musical_editor_poke_NSCR , 
										EDIT_FRAME_SUB_BG ,  0 , 0, FALSE , work->heapId );

	//PokeWin
	work->winArr = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*)*EDIT_POKEWIN_MAX*4 );
	for(i=0;i<4;i++)
	{
		u8 baseNo = EDIT_POKEWIN_MAX*i;
		work->winArr[ baseNo+EDIT_POKEWIN_POKENAME ] = 
					GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
							1+i*8 , 3 , 6 , 1 , 
							EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
		work->winArr[ baseNo+EDIT_POKEWIN_POS ] = 
					GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
							1+i*8 , 5 , 4 , 1 , 
							EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
		work->winArr[ baseNo+EDIT_POKEWIN_EQUIP ] = 
					GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 
							1+i*8 , 6 , 6 , 1 , 
							EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
	}
	
	for(i=0;i<4*EDIT_POKEWIN_MAX;i++)
	{
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArr[ i ] ) , 0xF );
		GFL_BMPWIN_MakeScreen( work->winArr[ i ] );
	}

	for(i=0;i<4;i++)
	{
		work->dispEquipPos[i] = 0;
		MusicalEdit_DrawPokeData( work , i );
	}

	//SubWin
	work->winArrSub = GFL_HEAP_AllocMemory( work->heapId , sizeof(GFL_BMPWIN*)*EDIT_SUBWIN_MAX );
	work->winArrSub[ EDIT_SUBWIN_OBJNAME ] = 
				GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG , 9 , 16 , 6 , 1 , 
				EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
	work->winArrSub[ EDIT_SUBWIN_LIGHT ] = 
				GFL_BMPWIN_Create( EDIT_FRAME_SUB_BG ,17 , 16 , 6 , 1 , 
				EDIT_PAL_FONT , GFL_BMP_CHRAREA_GET_B );
	
	for(i=0;i<EDIT_SUBWIN_MAX;i++)
	{
		GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->winArrSub[ i ] ) , 0xF );
		GFL_BMPWIN_MakeScreen( work->winArrSub[ i ] );
	}

	work->objNum = 0;
	work->lightNum = 0;
	work->befObjId = EDIT_STATE_FIRST;
	work->befLightState = EDIT_STATE_FIRST;
	MusicalEdit_DrawObjData( work );
	MusicalEdit_DrawLightData( work );
	GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
	
}

static void MusicalEdit_TermBgPokeEquip( MUS_EDIT_LOCAL_WORK *work )
{
	u8 i;
	for(i=0;i<4*EDIT_SUBWIN_MAX;i++)
	{
		GFL_BMPWIN_Delete( work->winArrSub[ i ] );
	}
	GFL_HEAP_FreeMemory( work->winArrSub );
	
	for(i=0;i<4*EDIT_POKEWIN_MAX;i++)
	{
		GFL_BMPWIN_Delete( work->winArr[ i ] );
	}
	GFL_HEAP_FreeMemory( work->winArr );
}

static void MusicalEdit_PrintFunc( MUS_EDIT_LOCAL_WORK *work , GFL_BMPWIN *win , STRBUF *str )
{
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp( win ) , 0xF );
	PRINTSYS_Print( GFL_BMPWIN_GetBmp(win) , 1 , 1 , str , work->fontHandle );
	GFL_BMPWIN_TransVramCharacter( win );
}

static void MusicalEdit_DrawPokeData( MUS_EDIT_LOCAL_WORK *work , const u8 pokeNo )
{
	u8 i;
	GFL_BMPWIN **baseWin = &work->winArr[pokeNo*EDIT_POKEWIN_MAX];
	STRBUF	*str = GFL_STR_CreateBuffer( 32 , work->heapId );
	STRCODE posName[MUS_POKE_EQUIP_MAX][20] =
	{
		{ L'H',L'E',L'A',L'D',0xFFFF },
		{ L'H',L'A',L'R',L'_',L'R',0xFFFF },
		{ L'H',L'A',L'R',L'_',L'L',0xFFFF },
		{ L'B',L'O',L'D',L'Y',0xFFFF },
		{ L'W',L'A',L'I',L'S',L'T',0xFFFF },
		{ L'H',L'A',L'N',L'D',L'_',L'R',0xFFFF },
		{ L'H',L'A',L'N',L'D',L'_',L'L',0xFFFF },
		{ L'T',L'A',L'I',L'L',0xFFFF },
	};
	MUSICAL_POKE_PARAM *musPoke = &work->actInitWork->musPoke[pokeNo];

	u8 Col[3];
	GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
	GFL_FONTSYS_SetColor( 1 , 0 , 0 );

	//名前
	GFL_MSG_GetString( GlobalMsg_PokeName , PP_Get( musPoke->pokePara , ID_PARA_monsno , 0 ) , str );
	MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_POKENAME] , str );
	GFL_STR_ClearBuffer( str );
	
	//装備箇所
	GFL_STR_SetStringCode(str,posName[work->dispEquipPos[pokeNo]]);
	MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_POS] , str );
	GFL_STR_ClearBuffer( str );
	
	//装備品
	{
		const u8 itemNo = musPoke->equip[ work->dispEquipPos[pokeNo] ].itemNo;
		if( itemNo == MUSICAL_ITEM_INVALID )
		{
			GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_NONE , str );
		}
		else
		{
			GFL_MSG_GetString( work->itemMsgHandle , MUS_ITEM_DRAW_GetArcIdx( itemNo )+ITEM_NAME_000 , str );
		}
		
		MusicalEdit_PrintFunc( work , baseWin[EDIT_POKEWIN_EQUIP] , str );
		GFL_STR_ClearBuffer( str );
	}

	GFL_STR_DeleteBuffer( str );
	GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
}

static void MusicalEdit_DrawObjData( MUS_EDIT_LOCAL_WORK *work )
{
	STRBUF	*str = GFL_STR_CreateBuffer( 32 , work->heapId );
	STA_OBJ_SYS  *objSys  = STA_ACT_GetObjectSys( work->actWork );
	STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , work->objNum );
	
	u8 Col[3];
	GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
	GFL_FONTSYS_SetColor( 1 , 0 , 0 );

	if( objWork == NULL )
	{
		GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_NONE , str );
		work->befObjId = EDIT_STATE_NONE;
	}
	else
	{
		GFL_MSG_GetString( work->itemMsgHandle , STA_OBJ_GetObjNo(objSys,objWork)+OBJ_NAME_000 , str );
		work->befObjId = STA_OBJ_GetObjNo(objSys,objWork);
	}
	
	MusicalEdit_PrintFunc( work , work->winArrSub[EDIT_SUBWIN_OBJNAME] , str );

	GFL_STR_DeleteBuffer( str );
	
	MusicalEdit_UpdateNumberFunc( work->objNum , 1 , 14 , 15 );
	GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
	
}

static void MusicalEdit_DrawLightData( MUS_EDIT_LOCAL_WORK *work )
{
	STRBUF	*str = GFL_STR_CreateBuffer( 32 , work->heapId );
	STA_LIGHT_SYS  *lightSys  = STA_ACT_GetLightSys( work->actWork );
	STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , work->lightNum );

	u8 Col[3];
	GFL_FONTSYS_GetColor( &Col[0] , &Col[1] , &Col[2] );
	GFL_FONTSYS_SetColor( 1 , 0 , 0 );

	if( lightWork == NULL )
	{
		GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_OFF , str );
		work->befLightState = EDIT_STATE_NONE;
	}
	else
	{
		GFL_MSG_GetString( work->itemMsgHandle , ITEM_NAME_ON  , str );
		work->befLightState = EDIT_STATE_ON;
	}
	
	MusicalEdit_PrintFunc( work , work->winArrSub[EDIT_SUBWIN_LIGHT] , str );

	GFL_STR_DeleteBuffer( str );

	MusicalEdit_UpdateNumberFunc( work->lightNum , 1 , 22 , 15 );
	GFL_FONTSYS_SetColor( Col[0] , Col[1] , Col[2] );
}

static void MusicalEdit_UpdatePokeData( MUS_EDIT_LOCAL_WORK *work )
{
	u8 i;
	STA_POKE_SYS *pokeSys = STA_ACT_GetPokeSys( work->actWork );
	//ポケ座標
	for( i=0;i<4;i++ )
	{
//		GFL_BMPWIN *win = work->winArr[i*EDIT_POKEWIN_WINMAX+EDIT_POKEWIN_POSITION];
		STA_POKE_WORK *pokeWork = STA_ACT_GetPokeWork( work->actWork , i );
		//座標
		{
			VecFx32 pos;
			STA_POKE_GetPosition( pokeSys , pokeWork , &pos );
			MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 6+i*8 , 10 );
			MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 6+i*8 , 11 );
			MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 6+i*8 , 12 );
		}
	}
	
	//ステージ・カーテン
	MusicalEdit_UpdateNumberFunc( STA_ACT_GetStageScroll(work->actWork) , 6 , 6 , 16 );
	MusicalEdit_UpdateNumberFunc( STA_ACT_GetCurtainHeight(work->actWork) , 6 , 6 , 18 );

	//オブジェ
	{
		STA_OBJ_SYS  *objSys  = STA_ACT_GetObjectSys( work->actWork );
		STA_OBJ_WORK *objWork = STA_ACT_GetObjectWork( work->actWork , work->objNum );
		VecFx32 pos;
		if( objWork == NULL )
		{
			VEC_Set( &pos,0,0,0 );
		}
		else
		{
			STA_OBJ_GetPosition( objSys , objWork , &pos );
		}
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 14 , 20 );
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 14 , 21 );
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 14 , 22 );
		
	}

	//ライト
	{
		STA_LIGHT_SYS  *lightSys  = STA_ACT_GetLightSys( work->actWork );
		STA_LIGHT_WORK *lightWork = STA_ACT_GetLightWork( work->actWork , work->lightNum );
		VecFx32 pos;
		if( lightWork == NULL )
		{
			VEC_Set( &pos,0,0,0 );
		}
		else
		{
			STA_LIGHT_GetPosition( lightSys , lightWork , &pos );
		}
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.x) , 6 , 22 , 20 );
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.y) , 6 , 22 , 21 );
		MusicalEdit_UpdateNumberFunc( (s32)F32_CONST(pos.z) , 6 , 22 , 22 );
	}

	GFL_BG_LoadScreenV_Req( EDIT_FRAME_SUB_BG );
}
static void MusicalEdit_UpdateNumberFunc( const s32 num , const u8 digit , const u8 x , const u8 y )
{
	const BOOL isMinus = (num<0?TRUE:FALSE);
	u32	tempNum = MATH_ABS(num);
	u16 nowDigit = 0;
	
	while( tempNum != 0 || nowDigit == 0 )
	{
		u16 chrNo = tempNum%10 + 0x02;
		GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
		nowDigit++;
		tempNum/=10;
	}
	if( isMinus == TRUE )
	{
		u16 chrNo = 0x0C;
		GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
		nowDigit++;
	}
	while( nowDigit < digit )
	{
		u16 chrNo = 0x01;
		GFL_BG_WriteScreen( EDIT_FRAME_SUB_BG , &chrNo , x-nowDigit,y,1,1);
		nowDigit++;
	}
	
}

static void MusicalEdit_UpdateTouch( MUS_EDIT_LOCAL_WORK *work )
{
	static const GFL_UI_TP_HITTBL hitTbl[24+1] =
	{
		{ 56 , 68 ,  8 , 20 },
		{ 56 , 68 , 20 , 32 },
		{ 56 , 68 , 32 , 44 },
		{ 56 , 68 , 44 , 56 },

		{ 56 , 68 , 72 , 84 },
		{ 56 , 68 , 84 , 96 },
		{ 56 , 68 , 96 ,108 },
		{ 56 , 68 ,108 ,120 },

		{ 56 , 68 ,136 ,148 },
		{ 56 , 68 ,148 ,150 },
		{ 56 , 68 ,150 ,162 },
		{ 56 , 68 ,162 ,174 },

		{ 56 , 68 ,200 ,212 },
		{ 56 , 68 ,212 ,224 },
		{ 56 , 68 ,224 ,236 },
		{ 56 , 68 ,236 ,248 },

		{136 ,148 , 72 , 84 },
		{136 ,148 , 84 , 96 },
		{136 ,148 , 96 ,108 },
		{136 ,148 ,108 ,120 },

		{136 ,148 ,136 ,148 },
		{136 ,148 ,148 ,150 },
		{136 ,148 ,150 ,162 },
		{136 ,148 ,162 ,174 },
		
		{ GFL_UI_TP_HIT_END },
	};
	
	const int ret = GFL_UI_TP_HitTrg( hitTbl );
	
	if( ret != GFL_UI_TP_HIT_NONE )
	{
		const u8 type = ret/4;
		const s8 valueArr[4] = {-5,-1,1,5};
		s8 value = valueArr[ ret%4 ];
		
		if( type < 4 )
		{
			if( work->dispEquipPos[type] + value < 0 )
			{
				work->dispEquipPos[type] = (work->dispEquipPos[type]+MUS_POKE_EQUIP_MAX+value)%MUS_POKE_EQUIP_MAX;
			}
			else
			{
				work->dispEquipPos[type] = (work->dispEquipPos[type]+value)%MUS_POKE_EQUIP_MAX;
			}
			MusicalEdit_DrawPokeData( work , type );
		}
		if( type == 4 )
		{
			OS_TPrintf("[%d]+[%d]->",work->objNum,value);
			if( work->objNum + value < 0 )
			{
				work->objNum = (work->objNum+ACT_OBJECT_MAX+value)%ACT_OBJECT_MAX;
			}
			else
			{
				work->objNum = (work->objNum+value)%ACT_OBJECT_MAX;
			}
			OS_TPrintf("[%d]\n",work->objNum);
			MusicalEdit_DrawObjData( work );
		}
		if( type == 5 )
		{
			if( work->lightNum + value < 0 )
			{
				work->lightNum = (work->lightNum+ACT_LIGHT_MAX+value)%ACT_LIGHT_MAX;
			}
			else
			{
				work->lightNum = (work->lightNum+value)%ACT_LIGHT_MAX;
			}
			MusicalEdit_DrawLightData( work );
		}
	}
}

#pragma mark [>mcs func

static void MusicalEdit_McsMain( MUS_EDIT_LOCAL_WORK *work )
{
	u32 size;
	MCS_Main();

	switch(work->mcsSeq)
	{
	case MSEQ_WAIT:
		MusicalEdit_McsWait( work );
		break;
	case MSEQ_LOAD_SEQ:
		MusicalEdit_McsLoadSeq( work );
		break;
	case MSEQ_LOAD_RES:
	case MSEQ_SEND:
		MusicalEdit_McsLoadRes( work );
		break;
	}

	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B )
	{
		STA_ACT_EDITOR_StartScript( work->actWork );
	}
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y )
	{
		
	}
}

static void MusicalEdit_McsWait( MUS_EDIT_LOCAL_WORK *work )
{
	const u32 size = MCS_CheckRead();
	if( size > 0 )
	{
		u32	head;
		MCS_Read( &head, HEAD_SIZE );
		if( head == SEND_SEQUENCE )
		{
			work->mcsSeq = MSEQ_LOAD_SEQ;
			head = SEND_SEQUENCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("sequence_data head:%d\n",size);
		}
		else if( head == SEND_RESOURCE )
		{
			work->mcsSeq = MSEQ_LOAD_RES;
			head = SEND_RESOURCE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("resource_data head:%d\n",size);
		}
		else if( head == SEND_RECEIVE )
		{
			work->mcsSeq = MSEQ_SEND;
//				evw->sub_seq_no = SUB_SEQ_INIT;
//				evw->answer = 0;
			head = SEND_RECEIVE_OK;
			MCS_Write( MCS_WRITE_CH, &head, 4 );
			OS_TPrintf("receive head:%d\n",size);
		}
	}
}
static void MusicalEdit_McsLoadSeq( MUS_EDIT_LOCAL_WORK *work )
{
	const u32 size = MCS_CheckRead();
	if( size )
	{
		u32	head;
		void *scriptData = GFL_HEAP_AllocClearMemory( work->heapId, size );
		MCS_Read( scriptData, size );
		STA_ACT_EDITOR_SetScript( work->actWork , scriptData );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		work->mcsSeq = MSEQ_WAIT;
		OS_TPrintf("sequence_data load:%d\n",size);
		
	}
	
}

static void MusicalEdit_McsLoadRes( MUS_EDIT_LOCAL_WORK *work )
{
	//ダミー読み込み
	const u32	size = MCS_CheckRead();

	if( size )
	{
		u32	head;
		void* data;
		data = GFL_HEAP_AllocClearMemory( work->heapId, size );
		MCS_Read( data, size );
		head = SEND_IDLE;
		MCS_Write( MCS_WRITE_CH, &head, 4 );
		work->mcsSeq = MSEQ_WAIT;

		GFL_HEAP_FreeMemory( data );
		OS_TPrintf("dummy load:%d\n",size);
	}
}

#endif //USE_MUSICAL_EDIT
