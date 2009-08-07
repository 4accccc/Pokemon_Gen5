//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		worldtrade_adapter.h
 *	@brief	GTSを移植する際のパック関数群
 *	@author	Toru=Nagihashi
 *	@date		2009.08.05
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
#pragma once
//lib
#include <gflib.h>
#include <dwc.h>

//system
#include "print/wordset.h"
#include "print/printsys.h"

#include "libdpw/dpw_common.h"

//savedata
#include "savedata/config.h"
#include "savedata/wifihistory.h"
#include "savedata/myitem_savedata.h"
#include "savedata/record.h"
#include "field/eventdata_system.h"

//tool
#include "poke_tool/poke_tool.h"

#include "net_app/net_bugfix.h"


//=============================================================================
/**
 *					置き換える予定だが、まだ定義されていないマクロ
*/
//=============================================================================
#define PARA_UNK							(0)
#define PARA_MALE							(1)
#define PARA_FEMALE						(2)

#define SE_CANCEL							(0)	
#define SE_GTC_NG							(0)
#define SE_GTC_SEARCH					(0)
#define SE_GTC_PLAYER_IN			(0)
#define SE_GTC_ON							(0)
#define SE_GTC_OFF						(0)
#define SE_GTC_PLAYER_OUT			(0)
#define SE_GTC_APPEAR					(0)

#define FONT_TOUCH						(0)
#define FONT_SYSTEM						(0)
#define FONT_TALK							(0)
#define SND_SCENE_FIELD				(0)
#define SND_SCENE_P2P					(0)
#define SEQ_BLD_BLD_GTC				(0)
#define SEQ_GS_WIFI_ACCESS		(0)
#define MSG_TP_ON							(0)
#define MSG_TP_OFF						(0)

#define TOUCH_SW_RET_YES			(1)
#define TOUCH_SW_RET_NO				(2)

#define TEMOTI_POKEMAX				(6)

#define TUUSHIN_SHINKA				(0)

#define SHINKA_STATUS_FLAG_SIO_MODE	(0)

#define ID_PER_sex						(0)

#define MSG_NO_PUT						(0)
#define MSG_ALLPUT						(0)

#define COMM_DMA_NO						(_NETWORK_DMA_NO)
#define COMM_POWERMODE				(_NETWORK_POWERMODE)
#define COMM_SSL_PRIORITY			(_NETWORK_SSL_PRIORITY)

#define BGWINFRM_TRANS_VBLANK	(0)

#define NUMFONT_MODE_LEFT			(0)
#define NUMFONT_MARK_SLASH		(0)

#define MORI_PRINT(...)				OS_Printf( __VA_ARGS__ )

#define COMM_ERROR_RESET_GTS  (3)	//gflib/src/network/net_state.hに定義がある
#define COMM_ERROR_RESET_OTHER (4)
#define COMM_ERRORTYPE_POWEROFF (1)   // 電源を切らないといけない

#define SBOX_SELECT_CANCEL		(1)

#define PHC_WIFI_OPEN_COURSE_NO	(0)

#define SVBLK_ID_MAX						(0)

#define MAPNAME_D5ROOM					(0)

typedef int SBOX_CB_MODE;

typedef void PHC_SVDATA;


#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数

//=============================================================================
/**
 *					置き換える予定だが、まだ定義されていない関数
*/
//=============================================================================
typedef struct
{	
	int dummy;
}ZUKAN_WORK;

typedef struct
{	
	int dummy;
}SELBOX_WORK;

typedef struct
{	
	int dummy;
}SELBOX_SYS;

typedef struct
{	
	int dummy;
}TOUCH_SW_SYS;

typedef struct
{	
	int dummy;
}CLACT_ADD;

typedef struct
{	
	int dummy;
}CLACT_HEADER;

typedef struct
{	
	int dummy;
}DEMO_TRADE_PARAM;

typedef struct
{	
	int dummy;
}SHINKA_WORK;

typedef struct
{	
	int dummy;
}BGWINFRM_WORK;

typedef struct
{	
	int dummy;
}NUMFONT;

//=============================================================================
/**
 *					置き換える予定だが、まだなく、中身を定義していない関数
*/
//=============================================================================
static inline void * TimeWaitIconAdd( GFL_BMPWIN *bmpwin, int a ) {	return NULL; }
static inline void TimeWaitIconDel( void *wk ){}

static inline int FontProc_GetPrintStrWidth( int font, STRBUF *buf, int a ){	return 0;}
static inline void FontProc_LoadFont( int a, HEAPID heapID ){}
static inline void FontProc_UnloadFont( int a ){}

static inline void Snd_DataSetByScene( int a, int b, int c ){}
static inline void MsgPrintTouchPanelFlagSet( int a ){}

static inline void TOUCH_SW_FreeWork( TOUCH_SW_SYS *wk ){}

static inline	BOOL GF_MSG_PrintEndCheck( int i ){	return 0; }
static inline int GF_STR_PrintSimple( GFL_BMPWIN *bmpwin, int a, STRBUF *str, int b, int c, int d, void *e ){ return 0; }
static inline void GF_STR_PrintColor( GFL_BMPWIN *bmpwin, int a, STRBUF *str, int b, int c, int d, PRINTSYS_LSB e, void *f ){	}

static inline int PokeIconCgxArcIndexGetByMonsNumber( int pokeno, int tamago, int form ){ return 0; }
static inline int PokeIconPalNumGet( int pokeno, int form, int tamago ){ return 0; }
static inline int PokePasoLevelCalc( POKEMON_PASO_PARAM *paso ){ return 0; }
static inline int PokePersonalParaGet(int a, int b){	return 0;}
static inline int PokeShinkaCheck( void *a, POKEMON_PARAM *pp, int tuushin, int item, int *cond ) {return 0;}

static inline BOOL ItemMailCheck( int a ){	return 0; }

static inline SHINKA_WORK* ShinkaInit( void *a, POKEMON_PARAM *pp, int no, CONFIG	*config, int contestflag, ZUKAN_WORK *zukanwork, MYITEM *myitem, RECORD *record, int cond, int mode, HEAPID heapID ){return NULL;}
static inline BOOL ShinkaEndCheck( SHINKA_WORK* wk ){	return 0;}
static inline void ShinkaEnd( SHINKA_WORK* wk ){}

static inline void WirelessIconEasy( void ){}
static inline void WirelessIconEasy_SetLevel( int level ){}
static inline void WirelessIconEasyEnd( void ){}

static inline BOOL ZukanWork_GetZenkokuZukanFlag( ZUKAN_WORK *wk ){return 0;}
static inline BOOL ZukanWork_GetPokeSeeFlag( ZUKAN_WORK *zukan, int a ){return 0;}

static inline void EMAILSAVE_DCProfileCreate_Update( SAVE_CONTROL_WORK * savedata, Dpw_Common_Profile * profile ){	}

static inline void CommFatalErrorFunc_NoNumber( void ){	}
static inline void ComErrorWarningResetCall( int heapID, int type, int code ){	}
static inline void Comm_WifiHistoryDataSet( WIFI_HISTORY * wifiHistory, int a, int b, int c ){}

static inline BGWINFRM_WORK * BGWINFRM_Create( int a, int b, HEAPID heapID ){	return NULL; }
static inline void BGWINFRM_Add( BGWINFRM_WORK *wk, int a, int b, int c, int d ){}
static inline void BGWINFRM_BmpWinOn( BGWINFRM_WORK *wk, int a, GFL_BMPWIN *bmpwin ){}
static inline void BGWINFRM_PosGet( BGWINFRM_WORK *wk, int a, s8 *x, s8 *y  ){}
static inline void BGWINFRM_Exit( BGWINFRM_WORK *wk ){}
static inline void BGWINFRM_MoveMain( BGWINFRM_WORK *wk ){}
static inline void BGWINFRM_MoveInit( BGWINFRM_WORK *wk, int a, int b, int c, int d ){}
static inline BOOL BGWINFRM_MoveCheck( BGWINFRM_WORK *wk, int a ){ return FALSE; }
static inline void BGWINFRM_FrameSetArc( BGWINFRM_WORK *wk, int a, ARCID arcID, ARCDATID datID, int b ){}
static inline void BGWINFRM_FramePut( BGWINFRM_WORK *wk, int a, int b, int c ){}
static inline u16 * BGWINFRM_FrameBufGet( BGWINFRM_WORK *wk, int a ){	return NULL; }
static inline void BGWINFRM_FrameOn( BGWINFRM_WORK *wk, int a ){}
static inline void BGWINFRM_FrameOff( BGWINFRM_WORK *wk, int a ){}

static inline NUMFONT * NUMFONT_Create( int a, int b, int c, HEAPID heapID ){return NULL;}
static inline void NUMFONT_Delete( NUMFONT *wk ){}
static inline void NUMFONT_WriteNumber( NUMFONT *wk, int a, int b, int c, GFL_BMPWIN *bmpwin, int d, int e ){}
static inline void NUMFONT_WriteMark( NUMFONT *wk, int a, GFL_BMPWIN *bmpwin, int c, int d ){}

static inline u32 SelectBoxMain( SELBOX_WORK *wk ){ return 0;}

static inline void PokePara_CustomBallDataInit( POKEMON_PARAM *wk ){}

static inline PHC_SVDATA * SaveData_GetPhcSaveData( SAVE_CONTROL_WORK *sv ){	return NULL; }
static inline void PhcSvData_SetCourseOpenFlag( PHC_SVDATA *sv, int a ){}
static inline void SaveData_RequestTotalSave( void ){}
static inline void SaveData_DivSave_Init( SAVE_CONTROL_WORK *sv, int a ){}
static inline BOOL SaveData_DivSave_Main( SAVE_CONTROL_WORK *sv ){ return FALSE;	}
static inline void * SaveData_GetPerapVoice( SAVE_CONTROL_WORK *sv ){ return NULL;}
static inline void SaveData_GetPokeRegister( SAVE_CONTROL_WORK *sv, POKEMON_PARAM *pp ){}
static inline EVENTWORK* SaveData_GetEventWork( SAVE_CONTROL_WORK *sv ){ return NULL; }

static inline int SysWork_AruseusuEventGet( EVENTWORK *ev ){	return 0;}
static inline void SysWork_AruseusuEventSet( EVENTWORK *ev, int a ){}

static inline u32 gf_p_rand( int a ){	return 0;}

//=============================================================================
/**
 *					WBにはないが実装できるので、作成した関数
*/
//=============================================================================
extern void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp );
extern POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID );
extern void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest );
extern void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw );
extern STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID );
extern void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp );
extern void WT_PM_strncpy( STRCODE *p_dst, const STRCODE *cp_src, int len );

extern POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp );

//CommStateSetError->GFL_NET_ErrorFunc
//PP_Put ID_PARA_sex 再計算でNULL指定しているが
