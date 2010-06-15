//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *  @file   worldtrade_adapter.h
 *  @brief  GTSを移植する際のパック関数群
 *  @author Toru=Nagihashi
 *  @date   2009.08.05
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
#include "system/net_err.h"
#include "pm_define.h"

#include <dpw_common.h>

//savedata
#include "savedata/config.h"
#include "savedata/wifihistory.h"
#include "savedata/myitem_savedata.h"
#include "savedata/record.h"
#include "field/eventdata_system.h"
#include "system/gfl_use.h"

#include "gamesystem/game_data.h"
#include "system/time_icon.h"
#include "net_app/comm_wifihistory.h"

//tool
#include "poke_tool/poke_tool.h"
#include "pokeicon/pokeicon.h"

#include "arc_def.h"
#include "font/font.naix"



//=============================================================================
/**
 *          定義
*/
//=============================================================================
#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数
#define COMM_DMA_NO           (_NETWORK_DMA_NO)
#define COMM_POWERMODE        (_NETWORK_POWERMODE)
#define COMM_SSL_PRIORITY     (_NETWORK_SSL_PRIORITY)
#define COMM_ERROR_RESET_GTS  (3) //gflib/src/network/net_state.hに定義がある
#define COMM_ERROR_RESET_OTHER (4)
#define COMM_ERRORTYPE_POWEROFF (1)   // 電源を切らないといけない
#define PARA_UNK              (2)
#define PARA_MALE             (0)
#define PARA_FEMALE           (1)

// 通常のフォントカラー
#define FBMP_COL_NULL   (0)
#define FBMP_COL_BLACK    (1)
#define FBMP_COL_BLK_SDW  (2)
#define FBMP_COL_RED    (3)
#define FBMP_COL_RED_SDW  (4)
#define FBMP_COL_GREEN    (5)
#define FBMP_COL_GRN_SDW  (6)
#define FBMP_COL_BLUE   (7)
#define FBMP_COL_BLU_SDW  (8)
#define FBMP_COL_PINK   (9)
#define FBMP_COL_PNK_SDW  (10)
#define FBMP_COL_WHITE    (15)

//=============================================================================
/**
 *          使用していないが、置換が大変なので定義でだましているもの
*/
//=============================================================================

#define MSG_NO_PUT            (0)
#define MSG_ALLPUT            (0)
#define FONT_TOUCH            (0)
#define FONT_SYSTEM           (0)
#define FONT_TALK             (0)

//メッセージ
#define MSG_TP_ON             (0)
#define MSG_TP_OFF            (0)

//PHC
#define PHC_WIFI_OPEN_COURSE_NO (0)
typedef void PHC_SVDATA;

//各種セーブデータ　PHC
static inline PHC_SVDATA * SaveData_GetPhcSaveData( SAVE_CONTROL_WORK *sv ){  return NULL; }
static inline void PhcSvData_SetCourseOpenFlag( PHC_SVDATA *sv, int a ){}
static inline EVENTWORK* SaveData_GetEventWork( SAVE_CONTROL_WORK *sv ){ return NULL; }

//アルセウスイベント
static inline int SysWork_AruseusuEventGet( EVENTWORK *ev ){  return 0;}
static inline void SysWork_AruseusuEventSet( EVENTWORK *ev, int a ){}

//UI
static inline void MsgPrintTouchPanelFlagSet( int a ){}

//=============================================================================
/**
 *          以下、単純に置き換えた関数
*/
//=============================================================================

//タイムウェイトアイコン
static inline void * TimeWaitIconAdd( GFL_BMPWIN *bmpwin, int a ) 
{ 
  return  TIMEICON_Create( GFUser_VIntr_GetTCBSYS(), bmpwin, 0xF, TIMEICON_DEFAULT_WAIT, HEAPID_WORLDTRADE ); 
}
static inline void TimeWaitIconDel( void *wk )
{
  TIMEICON_Exit( (TIMEICON_WORK *)wk );
}

static inline int PokePersonalParaGet(int mons_no, int param )
{ 
  u32 ret;

  POKEMON_PERSONAL_DATA *ppd;
    
  ppd = POKE_PERSONAL_OpenHandle( mons_no, 0, HEAPID_WORLDTRADE );
  ret = POKE_PERSONAL_GetParam( ppd, param );
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
  GFL_NET_ReloadIconTopOrBottom( FALSE, HEAPID_WORLDTRADE );
}
static inline void WirelessIconEasy_SetLevel( int level )
{
  GFL_NET_WirelessIconEasy_SetLevel(level);
}
static inline void WirelessIconEasyEnd( void )
{
  GFL_NET_WirelessIconEasyEnd();
}

static inline void TalkFontPaletteLoad(PALTYPE type, u32 offs, u32 heap)
{
  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, type, 
    offs, 0x20, heap);
}

//=============================================================================
/**
 *          以下、WBにはないが実装できるので、作成した関数
*/
//=============================================================================
extern void WT_WORDSET_RegisterPokeNickNamePPP( WORDSET* wordset, u32 bufID, const POKEMON_PASO_PARAM * ppp );
extern POKEMON_PARAM *PokemonParam_AllocWork( HEAPID heapID );
extern void WT_PokeCopyPPtoPP( const POKEMON_PARAM *pp_src, POKEMON_PARAM *pp_dest );
extern void BmpMenuWinClear( GFL_BMPWIN * win, u8 trans_sw );
extern STRBUF * MSGDAT_UTIL_AllocExpandString( const WORDSET *wordset, GFL_MSGDATA *MsgManager, u32 strID, HEAPID heapID );
extern void PokeReplace( const POKEMON_PASO_PARAM *ppp, POKEMON_PARAM *pp );

extern POKEMON_PASO_PARAM* PPPPointerGet( POKEMON_PARAM *pp );


//=============================================================================
/**
 *          BGWINFRM
 *
 *    GSにあったBGスクリーン加工処理は、なくなったので、
 *    WBのシステムを使い実装する->移植したので置き換え
*/
//=============================================================================
#include "system/bgwinfrm.h"


//=============================================================================
/**
 *          PRINT 及び FONT
 *          WBのPrintはPRINTSYSとFONTを使うようにされたので、
 *          必要な情報を纏めておくために作成したもの。
 *          追加部分です。
 *
*/
//=============================================================================
#define WT_PRINT_BUFF_MAX (24)
#define WT_PRINT_STREAM_MAX (1)

typedef struct 
{
  PRINT_UTIL    util;
  BOOL          use;
} WT_PRINT_QUE;

typedef struct
{ 
  GFL_FONT      *font;
  GFL_TCBLSYS   *tcbsys;
  const CONFIG  *cfg;
  PRINT_QUE     *que;
  PRINT_STREAM  *stream[WT_PRINT_STREAM_MAX];
  WT_PRINT_QUE  one[WT_PRINT_BUFF_MAX];
}WT_PRINT;

extern int FontProc_GetPrintStrWidth( WT_PRINT *wk, u8 font_idx, STRBUF *buf, int magin );

extern void WT_PRINT_Init( WT_PRINT *wk, const CONFIG *cfg );
extern void WT_PRINT_Exit( WT_PRINT *wk );
extern void WT_PRINT_Main( WT_PRINT *wk );

extern BOOL GF_MSG_PrintEndCheck( WT_PRINT *setup );

extern void GF_STR_PrintSimple( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, WT_PRINT *setup );

extern void GF_STR_PrintColor( GFL_BMPWIN *bmpwin, u8 font_idx, STRBUF *str, int x, int y, int put_type, PRINTSYS_LSB color, WT_PRINT *setup );


extern void WT_PRINT_ClearBuffer( WT_PRINT *wk );

//=============================================================================
/**
 *      NUMFONT
 *        WBにNUMFONTはないので、smallfontを使い実装する
*/
//=============================================================================

//NUMFONT
#define NUMFONT_MODE_LEFT     (STR_NUM_DISP_LEFT)
#define NUMFONT_MODE_RIGHT    (STR_NUM_DISP_SPACE)
#define NUMFONT_MARK_SLASH    (0)

typedef struct _NUMFONT NUMFONT;

extern NUMFONT * NUMFONT_Create( int a, int b, int c, HEAPID heapID );
extern void NUMFONT_Delete( NUMFONT *wk );
extern void NUMFONT_Main( NUMFONT *wk );
extern void NUMFONT_WriteNumber( NUMFONT *wk, int num, int keta, int mode, GFL_BMPWIN *bmpwin, int x, int y );
extern void NUMFONT_WriteMark( NUMFONT *wk, int mark, GFL_BMPWIN *bmpwin, int x, int y );

//=============================================================================
/**
 *          その他
*/
//=============================================================================
extern void WorldTrade_DispCallFatal( void );
