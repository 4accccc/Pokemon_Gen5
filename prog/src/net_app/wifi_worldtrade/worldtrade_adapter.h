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
#include "system/net_err.h"

#include <dpw_common.h>

//savedata
#include "savedata/config.h"
#include "savedata/wifihistory.h"
#include "savedata/myitem_savedata.h"
#include "savedata/record.h"
#include "field/eventdata_system.h"

#include "gamesystem/game_data.h"

//tool
#include "poke_tool/poke_tool.h"
#include "pokeicon/pokeicon.h"

#include "net_app/net_bugfix.h"

//=============================================================================
/**
 *					デバッグマクロ
*/
//=============================================================================
#ifdef PM_DEBUG

#define CHANGE_POKE_RULE_IGNORE	//交換用ポケモンが結果と一致していなくてもえらべる
#define RESERVE_POKE_GS_BINARY	//サーバーに預けるポケモンはGSから作成したバイナリを使う
//#define DEBUG_SAVE_NONE					//デバッグ用にセーブしないで進みます


#ifdef DEBUG_ONLY_FOR_toru_nagihashi	//担当者が変わったら変えてください
#define DEBUG_AUTHER_ONLY				//現在ONになっているとadapter以外で行っていないところをワーニングで知らせます
#endif //PM_DEBUG

//担当者プリント
#ifdef DEBUG_AUTHER_ONLY
#define MORI_PRINT(...)				OS_Printf( __VA_ARGS__ )
#else
#define MORI_PRINT(...)				((void)0)
#endif //DEBUG_AUTHER_ONLY

//アルセウスイベントチェック
//#define ARUCEUSU_EVENT_CHECK

//PHC解放チェック
//#define PHC_EVENT_CHECK


#endif //PM_DEBUG
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
#define TEMOTI_POKEMAX				(6)
#define PARA_UNK							(2)
#define PARA_MALE							(0)
#define PARA_FEMALE						(1)
//=============================================================================
/**
 *					置き換える予定だが、まだ定義されていないマクロ
*/
//=============================================================================

//サウンド	worldtrade_localの中にもあります
#define SE_CANCEL							(SEQ_SE_SELECT1)	
#define SE_GTC_NG							(SEQ_SE_SELECT1)
#define SE_GTC_SEARCH					(SEQ_SE_SELECT1)
#define SE_GTC_PLAYER_IN			(SEQ_SE_SELECT1)
#define SE_GTC_ON							(SEQ_SE_SELECT1)
#define SE_GTC_OFF						(SEQ_SE_SELECT1)
#define SE_GTC_PLAYER_OUT			(SEQ_SE_SELECT1)
#define SE_GTC_APPEAR					(SEQ_SE_SELECT1)
#define SND_SCENE_WIFI_WORLD_TRADE	(SEQ_SE_SELECT1)
#define SND_SCENE_FIELD				(0)
#define SND_SCENE_P2P					(0)
#define SEQ_BLD_BLD_GTC				(0)
#define SEQ_GS_WIFI_ACCESS		(0)
#define SEQ_GS_BLD_GTC				(0)

//フォント置き換えようWT_PRINTの中で使ってください
#define FONT_TOUCH						(0)
#define FONT_SYSTEM						(0)
#define FONT_TALK							(0)

//メッセージ
#define MSG_TP_ON							(0)
#define MSG_TP_OFF						(0)

//進化
#define TUUSHIN_SHINKA				(0)
#define SHINKA_STATUS_FLAG_SIO_MODE	(0)

//NUMFONT
#define NUMFONT_MODE_LEFT			(0)
#define NUMFONT_MARK_SLASH		(0)

//PHC
#define PHC_WIFI_OPEN_COURSE_NO	(0)
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
//タイムウェイトアイコン
static inline void * TimeWaitIconAdd( GFL_BMPWIN *bmpwin, int a ) {	return NULL; }
static inline void TimeWaitIconDel( void *wk ){}

//サウンド
static inline void Snd_DataSetByScene( int a, int b, int c ){}

//UI
static inline void MsgPrintTouchPanelFlagSet( int a ){}

//NUMFONT
static inline NUMFONT * NUMFONT_Create( int a, int b, int c, HEAPID heapID ){return NULL;}
static inline void NUMFONT_Delete( NUMFONT *wk ){}
static inline void NUMFONT_WriteNumber( NUMFONT *wk, int a, int b, int c, GFL_BMPWIN *bmpwin, int d, int e ){}
static inline void NUMFONT_WriteMark( NUMFONT *wk, int a, GFL_BMPWIN *bmpwin, int c, int d ){}


//各種セーブデータ　PHC　ペラップ　イベント 図鑑登録
static inline PHC_SVDATA * SaveData_GetPhcSaveData( SAVE_CONTROL_WORK *sv ){	return NULL; }
static inline void PhcSvData_SetCourseOpenFlag( PHC_SVDATA *sv, int a ){}
static inline void SaveData_RequestTotalSave( void ){}
static inline EVENTWORK* SaveData_GetEventWork( SAVE_CONTROL_WORK *sv ){ return NULL; }

//アルセウスイベント
static inline int SysWork_AruseusuEventGet( EVENTWORK *ev ){	return 0;}
static inline void SysWork_AruseusuEventSet( EVENTWORK *ev, int a ){}

//カスタムボール
static inline void PokePara_CustomBallDataInit( POKEMON_PARAM *wk ){}


//図鑑
static inline BOOL ZukanWork_GetZenkokuZukanFlag( ZUKAN_WORK *wk ){return TRUE;}
static inline BOOL ZukanWork_GetPokeSeeFlag( ZUKAN_WORK *zukan, int a ){return TRUE;}
static inline void SaveData_GetPokeRegister( SAVE_CONTROL_WORK *sv, POKEMON_PARAM *pp ){}

//進化
static inline SHINKA_WORK* ShinkaInit( void *a, POKEMON_PARAM *pp, int no, CONFIG	*config, int contestflag, ZUKAN_WORK *zukanwork, MYITEM *myitem, RECORD *record, int cond, int mode, HEAPID heapID ){return NULL;}
static inline BOOL ShinkaEndCheck( SHINKA_WORK* wk ){	return TRUE;}
static inline void ShinkaEnd( SHINKA_WORK* wk ){}
static inline int PokeShinkaCheck( void *a, POKEMON_PARAM *pp, int tuushin, int item, int *cond ) {return 0;}
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
static inline int PokePersonalParaGet(int mons_no, int param )
{ 
	u32 ret;

	POKEMON_PERSONAL_DATA *ppd;
		
	ppd	= POKE_PERSONAL_OpenHandle( mons_no, 0, HEAPID_WORLDTRADE );
	ret	= POKE_PERSONAL_GetParam( ppd, param );
	POKE_PERSONAL_CloseHandle( ppd );
	
	return ret;
}
static inline int PokePasoLevelCalc( POKEMON_PASO_PARAM *paso )
{
	return PPP_CalcLevel( paso ); 
}
//通信アイコン
static inline void WirelessIconEasy( void )
{
	//GFL_NET_ReloadIcon();  // 接続中なのでアイコン表示
	GFL_NET_WirelessIconEasyXY(GFL_WICON_POSX, GFL_WICON_POSY, TRUE, HEAPID_WORLDTRADE);
	GFL_NET_WirelessIconEasy_HoldLCD( TRUE, HEAPID_WORLDTRADE );
}
static inline void WirelessIconEasy_SetLevel( int level )
{
	GFL_NET_WirelessIconEasy_SetLevel(level);
}
static inline void WirelessIconEasyEnd( void )
{
	GFL_NET_WirelessIconEasyEnd();
}


//通信エラー
//
//	090828現在、通信エラーの分岐処理は行っていません。
//	今は、エラーになると、エラー画面が出るのみの置き換えになっています
//
static inline void CommStateSetError( int error )
{	
	NetErr_ErrorSet();
}
static inline void CommFatalErrorFunc_NoNumber( void )
{
	NetErr_ErrorSet();
}
static inline void ComErrorWarningResetCall( int heapID, int type, int code )
{
	NetErr_ErrorSet();
}
static inline void Comm_WifiHistoryDataSet( WIFI_HISTORY *wifiHistory, int Nation, int Area, int langCode )
{
#if (PM_LANG == LANG_JAPAN)
	// 相手のカートリッジが日本なのに
	if(langCode==LANG_JAPAN){
		// 違う国の地球儀コードを入れてきたら登録しない
		if(Nation!=WIFI_NATION_JAPAN){
			return;
		}
	}
#endif
	
	// 国・地域を登録
//	OS_Printf("no=%d\n", WIFIHISTORY_GetStat(wifiHistory, Nation, Area ));
	if(WIFIHISTORY_GetStat( wifiHistory, Nation, Area )==WIFIHIST_STAT_NODATA){
		WIFIHISTORY_SetStat( wifiHistory, Nation, Area, WIFIHIST_STAT_NEW );
	}
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



//=============================================================================
/**
 *					BGWINFRM
 *
 *		GSにあったBGスクリーン加工処理は、なくなったので、
 *		WBのシステムを使い実装する
*/
//=============================================================================
#include "system/bgwinfrm.h"
#if 0
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
#endif

//=============================================================================
/**
 *					PRINT	及び FONT
 *					WBのPrintはPRINTSYSとFONTを使うようにされたので、
 *					必要な情報を纏めておくために作成したもの。
 *					追加部分です。
 *
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


