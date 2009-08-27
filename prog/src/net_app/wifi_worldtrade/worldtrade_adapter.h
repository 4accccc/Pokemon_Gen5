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
#include "system/selbox.h"

#include "libdpw/dpw_common.h"

//savedata
#include "savedata/config.h"
#include "savedata/wifihistory.h"
#include "savedata/myitem_savedata.h"
#include "savedata/record.h"
#include "field/eventdata_system.h"

//tool
#include "poke_tool/poke_tool.h"
#include "pokeicon/pokeicon.h"

#include "net_app/net_bugfix.h"


//=============================================================================
/**
 *					定義
*/
//=============================================================================
#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数
#define COMM_DMA_NO						(_NETWORK_DMA_NO)
#define COMM_POWERMODE				(_NETWORK_POWERMODE)
#define COMM_SSL_PRIORITY			(_NETWORK_SSL_PRIORITY)
#define COMM_ERROR_RESET_GTS  (3)	//gflib/src/network/net_state.hに定義がある
#define COMM_ERROR_RESET_OTHER (4)
#define COMM_ERRORTYPE_POWEROFF (1)   // 電源を切らないといけない


//=============================================================================
/**
 *					置き換える予定だが、まだ定義されていないマクロ
*/
//=============================================================================
#define PARA_UNK							(0)
#define PARA_MALE							(1)
#define PARA_FEMALE						(2)

#define SE_CANCEL							(SEQ_SE_000)	
#define SE_GTC_NG							(SEQ_SE_000)
#define SE_GTC_SEARCH					(SEQ_SE_000)
#define SE_GTC_PLAYER_IN			(SEQ_SE_000)
#define SE_GTC_ON							(SEQ_SE_000)
#define SE_GTC_OFF						(SEQ_SE_000)
#define SE_GTC_PLAYER_OUT			(SEQ_SE_000)
#define SE_GTC_APPEAR					(SEQ_SE_000)

#define FONT_TOUCH						(0)
#define FONT_SYSTEM						(0)
#define FONT_TALK							(0)

#define SND_SCENE_FIELD				(0)
#define SND_SCENE_P2P					(0)
#define SEQ_BLD_BLD_GTC				(0)
#define SEQ_GS_WIFI_ACCESS		(0)
#define MSG_TP_ON							(0)
#define MSG_TP_OFF						(0)

#define TEMOTI_POKEMAX				(6)

#define TUUSHIN_SHINKA				(0)

#define SHINKA_STATUS_FLAG_SIO_MODE	(0)

#define ID_PER_sex						(0)

#define NUMFONT_MODE_LEFT			(0)
#define NUMFONT_MARK_SLASH		(0)

#define MORI_PRINT(...)				OS_Printf( __VA_ARGS__ )




#define PHC_WIFI_OPEN_COURSE_NO	(0)

#define SVBLK_ID_MAX						(0)

#define MAPNAME_D5ROOM					(0)


typedef void PHC_SVDATA;



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
}DEMO_TRADE_PARAM;

typedef struct
{	
	int dummy;
}SHINKA_WORK;

typedef struct
{	
	int dummy;
}NUMFONT;

//=============================================================================
/**
 *					使用していないが、置換が大変なので定義でだましているもの
*/
//=============================================================================

#define MSG_NO_PUT						(0)
#define MSG_ALLPUT						(0)

typedef struct
{	
	int dummy;
}CLACT_ADD;

typedef struct
{	
	int dummy;
}CLACT_HEADER;

//=============================================================================
/**
 *					置き換える予定だが、まだなく、中身を定義していない関数
*/
//=============================================================================
static inline void * TimeWaitIconAdd( GFL_BMPWIN *bmpwin, int a ) {	return NULL; }
static inline void TimeWaitIconDel( void *wk ){}

static inline void Snd_DataSetByScene( int a, int b, int c ){}
static inline void MsgPrintTouchPanelFlagSet( int a ){}


static inline int PokePasoLevelCalc( POKEMON_PASO_PARAM *paso ){ return 0; }
static inline int PokePersonalParaGet(int a, int b){ return 0;}
static inline int PokeShinkaCheck( void *a, POKEMON_PARAM *pp, int tuushin, int item, int *cond ) {return 0;}

static inline BOOL ItemMailCheck( int a ){	return 0; }



static inline void WirelessIconEasy( void )
{
	GFL_NET_ReloadIcon();  // 接続中なのでアイコン表示
}
static inline void WirelessIconEasy_SetLevel( int level ){}
static inline void WirelessIconEasyEnd( void ){}




static inline void CommFatalErrorFunc_NoNumber( void ){	}
static inline void ComErrorWarningResetCall( int heapID, int type, int code ){	}
static inline void Comm_WifiHistoryDataSet( WIFI_HISTORY * wifiHistory, int a, int b, int c ){}


static inline NUMFONT * NUMFONT_Create( int a, int b, int c, HEAPID heapID ){return NULL;}
static inline void NUMFONT_Delete( NUMFONT *wk ){}
static inline void NUMFONT_WriteNumber( NUMFONT *wk, int a, int b, int c, GFL_BMPWIN *bmpwin, int d, int e ){}
static inline void NUMFONT_WriteMark( NUMFONT *wk, int a, GFL_BMPWIN *bmpwin, int c, int d ){}

static inline void PokePara_CustomBallDataInit( POKEMON_PARAM *wk ){}

static inline PHC_SVDATA * SaveData_GetPhcSaveData( SAVE_CONTROL_WORK *sv ){	return NULL; }
static inline void PhcSvData_SetCourseOpenFlag( PHC_SVDATA *sv, int a ){}
static inline void SaveData_RequestTotalSave( void ){}
static inline void * SaveData_GetPerapVoice( SAVE_CONTROL_WORK *sv ){ return NULL;}
static inline void SaveData_GetPokeRegister( SAVE_CONTROL_WORK *sv, POKEMON_PARAM *pp ){}
static inline EVENTWORK* SaveData_GetEventWork( SAVE_CONTROL_WORK *sv ){ return NULL; }

static inline int SysWork_AruseusuEventGet( EVENTWORK *ev ){	return 0;}
static inline void SysWork_AruseusuEventSet( EVENTWORK *ev, int a ){}

static inline u32 gf_p_rand( int a ){	return 0; }

//=============================================================================
/**
 *					まだ置き換えられない暫定措置のもの
*/
//=============================================================================
static inline BOOL ZukanWork_GetZenkokuZukanFlag( ZUKAN_WORK *wk ){return TRUE;}
static inline BOOL ZukanWork_GetPokeSeeFlag( ZUKAN_WORK *zukan, int a ){return TRUE;}

static inline SHINKA_WORK* ShinkaInit( void *a, POKEMON_PARAM *pp, int no, CONFIG	*config, int contestflag, ZUKAN_WORK *zukanwork, MYITEM *myitem, RECORD *record, int cond, int mode, HEAPID heapID ){return NULL;}
static inline BOOL ShinkaEndCheck( SHINKA_WORK* wk ){	return TRUE;}
static inline void ShinkaEnd( SHINKA_WORK* wk ){}
//=============================================================================
/**
 *					以下、単純に置き換えた関数
*/
//=============================================================================
static inline int PokeIconCgxArcIndexGetByMonsNumber( int pokeno, int tamago, int form )
{
	return POKEICON_GetCgxArcIndexByMonsNumber( pokeno, form, tamago ); 
}
static inline int PokeIconPalNumGet( int pokeno, int form, int tamago )
{
	return POKEICON_GetPalNum( pokeno, form, tamago );
}
static inline void SaveData_DivSave_Init( SAVE_CONTROL_WORK *sv, int a )
{
	SaveControl_SaveAsyncInit( sv );
}
static inline SAVE_RESULT SaveData_DivSave_Main( SAVE_CONTROL_WORK *sv )
{
	return SaveControl_SaveAsyncMain( sv );
}
//=============================================================================
/**
 *					以下、WBにはないが実装できるので、作成した関数
*/
//=============================================================================
extern void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp );
extern POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID );
extern void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest );
extern void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw );
extern STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID );
extern void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp );

extern POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp );


extern void EMAILSAVE_DCProfileCreate_Update( SAVE_CONTROL_WORK * savedata, Dpw_Common_Profile * profile );

//CommStateSetError->GFL_NET_ErrorFunc
//PP_Put ID_PARA_sex 再計算でNULL指定しているが


//=============================================================================
/**
 *					PRINT	及び FONT
*/
//=============================================================================
#define WT_PRINT_BUFF_MAX	(24)

typedef struct 
{
	PRINT_QUE			*que;
	PRINT_UTIL		util;
	BOOL					use;
} WT_PRINT_QUE;


typedef struct
{	
	GFL_FONT			*font;
	GFL_TCBLSYS		*tcbsys;
	const CONFIG	*cfg;
	PRINT_STREAM	*stream[WT_PRINT_BUFF_MAX];
	WT_PRINT_QUE	one[WT_PRINT_BUFF_MAX];
}WT_PRINT;


extern int FontProc_GetPrintStrWidth( WT_PRINT *wk, u8 font_idx, STRBUF *buf, int magin );

extern void WT_PRINT_Init( WT_PRINT *wk, const CONFIG *cfg );
extern void WT_PRINT_Exit( WT_PRINT *wk );
extern void WT_PRINT_Main( WT_PRINT *wk );

extern BOOL GF_MSG_PrintEndCheck( WT_PRINT *setup );

extern void GF_STR_PrintSimple( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, WT_PRINT *setup );
extern void GF_STR_PrintColor( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, int put_type, PRINTSYS_LSB color, WT_PRINT *setup );

//=============================================================================
/**
 *					BGWINFRM
 *
 *		GSにあったBGスクリーン加工処理は、なくなったので、
 *		WBのシステムを使い実装する
*/
//=============================================================================
#define BGWINFRM_TRANS_VBLANK	(0)

typedef struct _BGWINFRM_WORK BGWINFRM_WORK;

extern BGWINFRM_WORK * BGWINFRM_Create( int mode, int max, HEAPID heapID );
extern void BGWINFRM_Add( BGWINFRM_WORK *wk, int index, int frm, int sx, int sy );
extern void BGWINFRM_BmpWinOn( BGWINFRM_WORK *wk, int index, GFL_BMPWIN *win );
extern void BGWINFRM_PosGet( BGWINFRM_WORK *wk, int index, s8 *px, s8 *py  );
extern void BGWINFRM_Exit( BGWINFRM_WORK *wk );
extern void BGWINFRM_MoveMain( BGWINFRM_WORK *wk );
extern u32 BGWINFRM_MoveOne( BGWINFRM_WORK * wk, u32 index );
extern void BGWINFRM_MoveInit( BGWINFRM_WORK *wk, int index, int add_x, int add_y, int cnt );
extern void BGWINFRM_FrameSet( BGWINFRM_WORK * wk, u32 index, u16 * scr );
extern u32 BGWINFRM_MoveCheck( BGWINFRM_WORK *wk, int index );
extern void BGWINFRM_FrameSetArc( BGWINFRM_WORK *wk, int index, ARCID arcID, ARCDATID datID, int comp );
extern void BGWINFRM_FramePut( BGWINFRM_WORK *wk, int index, int px, int py );
extern u16 * BGWINFRM_FrameBufGet( BGWINFRM_WORK *wk, int index );
extern void BGWINFRM_FrameOn( BGWINFRM_WORK *wk, int index );
extern void BGWINFRM_FrameOff( BGWINFRM_WORK *wk, int index );
