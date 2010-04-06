//=============================================================================
/**
 * @file	  c_gear.c
 * @brief	  コミュニケーションギア
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "c_gear.h"
#include "c_gear_pattern.h"
#include "savedata/save_tbl.h"
#include "savedata/c_gear_picture.h"
#include "system/main.h"  //HEAPID
#include "system/palanm.h"  //HEAPID

#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "c_gear.naix"
#include "c_gear_obj_NANR_LBLDEFS.h"

#include "net/wih_dwc.h"

#include "gamesystem/game_beacon_accessor.h"
#include "../../field/field_comm/intrude_work.h"
#include "../../field/field_comm/intrude_main.h"

#include "../field_sound.h"

#include "../event_ircbattle.h"      //EVENT_IrcBattle
#include "../event_gsync.h"         //EVENT_GSync
#include "../event_cg_wireless.h"         //EVENT_CG_Wireless
#include "net_app/cg_help.h"  //CGHELP呼び出し
#include "../event_fieldmap_control.h"  //CGHELP呼び出し
#include "../event_research_radar.h"  //EVENT_ResearchRadar
#include "../event_subscreen.h"      //EVENT_ChangeSubScreen


#ifdef PM_DEBUG

#ifdef DEUBG_ONLY_FOR_tomoya_takahashi

#define DEBUG_LOCAL

#endif

#endif


#define _NET_DEBUG (1)  //デバッグ時は１
#define _BRIGHTNESS_SYNC (0)  // フェードのＳＹＮＣは要調整

// サウンド仮想ラベル
#define GEAR_SE_DECIDE_ (SEQ_SE_DECIDE3)
#define GEAR_SE_CANCEL_ (SEQ_SE_SYS_70)

#define MSG_COUNTDOWN_FRAMENUM (30*3)

//--------------------------------------------
// 画面構成定義
//--------------------------------------------
#define _WINDOW_MAXNUM (4)   //ウインドウのパターン数

#define _MESSAGE_BUF_NUM	( 100*2 )

#define _BUTTON_WIN_CENTERX (16)   // 真ん中
#define _BUTTON_WIN_CENTERY (13)   //
#define _BUTTON_WIN_WIDTH (22)    // ウインドウ幅
#define _BUTTON_WIN_HEIGHT (3)    // ウインドウ高さ
#define _BUTTON_WIN_PAL   (12)  // ウインドウ
#define _BUTTON_MSG_PAL   (13)  // メッセージフォント

#define	_BUTTON_WIN_CGX_SIZE   ( 18+12 )
#define	_BUTTON_WIN_CGX	( 2 )


#define	_BUTTON_FRAME_CGX		( _BUTTON_WIN_CGX + ( 23 * 16 ) )	// 通信システムウィンドウ転送先

#define	FBMP_COL_WHITE		(15)


#define POS_SCANRADAR_X  (40)
#define POS_SCANRADAR_Y  (170)



//-------------------------

#define PANEL_Y1 (6)
#define PANEL_Y2 (4)
#define PANEL_X1 (-2)

#define PANEL_HEIDHT1 (C_GEAR_PANEL_HEIGHT-1)
#define PANEL_HEIGHT2 (C_GEAR_PANEL_HEIGHT)

#define PANEL_WIDTH (C_GEAR_PANEL_WIDTH)
#define PANEL_SIZEXY (4)   //

#define _CGEAR_TYPE_PATTERN_NUM (2)


typedef enum{
  _CLACT_PLT,
  _CLACT_CHR,
  _CLACT_ANM,
} _CGEAR_CLACT_TYPE;


typedef enum{
  _CGEAR_NET_BIT_IR = (1 << CGEAR_PANELTYPE_IR),
  _CGEAR_NET_BIT_WIRELESS = (1 << CGEAR_PANELTYPE_WIRELESS),
  _CGEAR_NET_BIT_WIFI = (1 << CGEAR_PANELTYPE_WIFI),
} _CGEAR_NET_BIT;



#define _CGEAR_NET_CHANGEPAL_NUM (3)
#define _CGEAR_NET_CHANGEPAL_POSX (0xd)
#define _CGEAR_NET_CHANGEPAL_POSY (1)
enum{
  // パネル数
  _CGEAR_NET_CHANGEPAL_IRC = 0,
  _CGEAR_NET_CHANGEPAL_WIFI,
  _CGEAR_NET_CHANGEPAL_WIRELES,
  _CGEAR_NET_CHANGEPAL_MAX,

  
  // パレットタイプ
  _CGEAR_NET_PALTYPE_RED = 0,
  _CGEAR_NET_PALTYPE_BLUE,
  _CGEAR_NET_PALTYPE_YELLOW,
  _CGEAR_NET_PALTYPE_GREEN,

  _CGEAR_NET_PALTYPE_MAX,

  // ワイアレスカラーカウント
  _CGEAR_NET_WIRELES_TYPE_PALACE = 0,
  _CGEAR_NET_WIRELES_TYPE_UNION,
  _CGEAR_NET_WIRELES_TYPE_FUSHIGI,
  _CGEAR_NET_WIRELES_TYPE_MAX,
  
};
#define _CGEAR_NET_CHANGEPAL_ANM_COUNT_MAX  ( 74 )
#define _CGEAR_NET_CHANGEPAL_ANM_COUNT  ( 64 )


//--------------------------

typedef enum{
  _SELECTANIM_NONE,
  _SELECTANIM_STANDBY,
  _SELECTANIM_ANIMING,

} _SELECTANIM_ENUM;



//--------------------------

typedef struct {
  int leftx;
  int lefty;
  int width;
  int height;
} _WINDOWPOS;

typedef struct {
  u8 time;
  u8 x;
  u8 y;
  u8 downtime;
} _ANIM_DATA;




// 表示OAMの時間とかの最大
#define _CLACT_TIMEPARTS_MAX (10)

#define _CLACT_EDITMARKOFF (9)
#define _CLACT_EDITMARKON (9)

//すれ違いよう
#define _CLACT_CROSS_MAX  (12)

// タイプ
#define _CLACT_TYPE_MAX (3)

#define POS_HELPMARK_X    (198)
#define POS_HELPMARK_Y    (POS_SCANRADAR_Y)
#define POS_EDITMARK_X    (222)
#define POS_EDITMARK_Y    (POS_SCANRADAR_Y)

#define _SLEEP_START_TIME_WAIT (16)


// スリープ復帰アニメ
enum{
  BOOT_ANIME_SEQ_ALL_ON,
  BOOT_ANIME_SEQ_ALL_WAIT,
  BOOT_ANIME_SEQ_SMOOTHIN_MAIN,
  BOOT_ANIME_SEQ_END,
};

// スリープ復帰
const static _ANIM_DATA screenTable[]={
  { 0, 0, 0, 11},
  { 0, 0, 1, 11},
  { 0, 0, 2, 11},
  { 0, 8, 0, 11},
  { 0, 8, 1, 11},
  { 0, 8, 2, 11},

  { 1, 1, 0, 10},
  { 1, 1, 1, 10},
  { 1, 1, 2, 10},
  { 1, 1, 3, 10},
  { 1, 7, 0, 10},
  { 1, 7, 1, 10},
  { 1, 7, 2, 10},
  { 1, 7, 3, 10},

  { 2, 2, 0, 9},
  { 2, 2, 1, 9},
  { 2, 2, 2, 9},
  { 2, 6, 0, 9},
  { 2, 6, 1, 9},
  { 2, 6, 2, 9},

  { 3, 3, 0, 8},
  { 3, 3, 1, 8},
  { 3, 3, 2, 8},
  { 3, 3, 3, 8},
  { 3, 5, 0, 8},
  { 3, 5, 1, 8},
  { 3, 5, 2, 8},
  { 3, 5, 3, 8},

  { 4, 4, 0, 7},
  { 4, 4, 2, 7},

  { 5, 4, 1, 6},
};

// 起動アニメ
#define START_UP_SCREEN_ANIME_FRAME_MAX ( 24 )
const static _ANIM_DATA StartUpScreenTable[]={
  { 0, 2, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 3, 6, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 5, 3, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 7, 5, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 8, 1, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 8, 7, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 9, 1, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  { 9, 7, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {10, 3, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {10, 5, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  {11, 2, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {11, 6, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {12, 2, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  {12, 6, 1, START_UP_SCREEN_ANIME_FRAME_MAX},

  {13, 3, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {13, 5, 3, START_UP_SCREEN_ANIME_FRAME_MAX},
  {14, 1, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {14, 0, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {15, 7, 3, START_UP_SCREEN_ANIME_FRAME_MAX},
  {15, 8, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {16, 0, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  {16, 0, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {17, 8, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {17, 8, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
  {17, 4, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {18, 1, 3, START_UP_SCREEN_ANIME_FRAME_MAX},
  {18, 3, 3, START_UP_SCREEN_ANIME_FRAME_MAX},
  {18, 4, 2, START_UP_SCREEN_ANIME_FRAME_MAX},
  {19, 7, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {19, 5, 0, START_UP_SCREEN_ANIME_FRAME_MAX},
  {19, 4, 1, START_UP_SCREEN_ANIME_FRAME_MAX},
};




static const GFL_UI_TP_HITTBL bttndata[] = {  //上下左右
 
  {	PANEL_Y2 * 8,  PANEL_Y2 * 8 + (PANEL_SIZEXY * 8 * PANEL_HEIGHT2), 0,32*8-1 },   //タッチパネル全部
  { (POS_EDITMARK_Y-8), (POS_EDITMARK_Y+8), (POS_EDITMARK_X-8), (POS_EDITMARK_X+8) },        //こうさく
  { (POS_HELPMARK_Y-8), (POS_HELPMARK_Y+8), (POS_HELPMARK_X-8), (POS_HELPMARK_X+8) },   //HELP
  { 160, 160+24, 128-32, 128+32 },                //SURECHIGAI
  { (POS_SCANRADAR_Y-12), (POS_SCANRADAR_Y+12), (POS_SCANRADAR_X-16), (POS_SCANRADAR_X+16) }, //RADAR
  { 16, (16+8), 96, (96+(7*8)) },    //CGEARLOGO
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};


//-------------------------------------
///	起動アニメーション
//=====================================
#define STARTUP_ANIME_OBJ_MAX ( 18 )      // 起動アニメーションに必要なオブジェ数
#define STARTUP_ANIME_INDEX_START ( NANR_c_gear_obj_CellAnime_logo1 )  // 各オブジェに設定するアニメーション開始インデックス
#define STARTUP_ANIME_POS_X (128)
#define STARTUP_ANIME_POS_Y (96)
#define STARTUP_OAM_ALPHA_ANIME_FRAME_MAX ( 8 )
#define STARTUP_TBL_ALPHA_TIME_WAIT ( 8 )
#define STARTUP_END_TIME_WAIT ( 8 )
enum
{
  STARTUP_SEQ_ANIME_START,
  STARTUP_SEQ_WIPE_IN,
  STARTUP_SEQ_ANIME_WAIT,
  STARTUP_SEQ_OAM_ALPHA_WAIT,
  STARTUP_SEQ_TBL_IN_WAIT,
  STARTUP_SEQ_TBL_ALPHA_WAIT,
  STARTUP_SEQ_END,
};


//-------------------------------------
///	パネルのカラーアニメ
//=====================================
enum{

  // アニメタイプ
  PANEL_ANIME_TYPE_ON_OFF = 0,
  PANEL_ANIME_TYPE_ON,
  PANEL_ANIME_TYPE_OFF,
  PANEL_ANIME_TYPE_MAX,

  // カラータイプ
  PANEL_COLOR_TYPE_NONE = 0,
  PANEL_COLOR_TYPE_RED,
  PANEL_COLOR_TYPE_YELLOW,
  PANEL_COLOR_TYPE_BLUE,
  PANEL_COLOR_TYPE_BASE,
  PANEL_COLOR_TYPE_MAX,

  // アニメーション段階
  PANEL_COLOR_PATTERN_NUM = 5,

  // アニメーションフレーム
  PANEL_ANIME_DEF_FRAME = 2,
  

};
static const u8 sc_PANEL_COLOR_ANIME_END_INDEX[] = { 0, 1, 2, 3, 4, 3, 2, 1, 0 };
// カラーPATTERN情報
static const u8 sc_PANEL_COLOR_ANIME_TBL[ PANEL_COLOR_TYPE_MAX ][ PANEL_COLOR_PATTERN_NUM ] = 
{
  // NONE
  { 0x9, 0x9, 0x9, 0x9, 0x9 },
  // RED
  { 0x1, 0x1, 0x1, 0x1, 0x1 },
  // YELLOW
  { 0x2, 0x2, 0x2, 0x2, 0x2 },
  // BLUE
  { 0x3, 0x3, 0x3, 0x3, 0x3 },
  // BASE
  { 0x7, 0x6, 0x5, 0x4, 0xb },
};
// カラータイプのパネルタイプ
static const u16 sc_PANEL_TYPE_TO_COLOR[] = {
  PANEL_COLOR_TYPE_NONE,
  PANEL_COLOR_TYPE_RED,
  PANEL_COLOR_TYPE_YELLOW,
  PANEL_COLOR_TYPE_BLUE,
  PANEL_COLOR_TYPE_BASE,
  PANEL_COLOR_TYPE_BASE,
};

//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))



//--------------------------------------------
// 内部ワーク
//--------------------------------------------


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BMPWIN_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)
#define GEAR_FB_MESSAGE   (GFL_BG_FRAME3_S)


typedef void (StateFunc)(C_GEAR_WORK* pState);
typedef BOOL (TouchFunc)(int no, C_GEAR_WORK* pState);

//-------------------------------------
///	パネル　マーク　アニメ
//=====================================
typedef struct {
  u16 anime_on:1;
  u16 count:15;
  u16 one_frame;
  u8  color;
  u8  end_anime_index;
  u8  x;
  u8  y;
} PANEL_MARK_ANIME;



struct _C_GEAR_WORK {
  StateFunc* state;      ///< ハンドルのプログラム状態
  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
  GFL_BUTTON_MAN* pButton;
  GFL_ARCUTIL_TRANSINFO subchar;
  FIELD_SUBSCREEN_WORK* subscreen;
  GAMESYS_WORK* pGameSys;
  CGEAR_SAVEDATA* pCGSV;
  ARCHANDLE* handle;  // アーカイブハンドル
  u32 objRes[3];  //CLACTリソース

  GFL_CLUNIT *cellUnit;
  GFL_CLWK  *cellSelect[C_GEAR_PANEL_WIDTH * C_GEAR_PANEL_HEIGHT];
  GFL_CLWK  *cellCursor[_CLACT_TIMEPARTS_MAX];
  GFL_CLWK  *cellType[_CLACT_TYPE_MAX];
  GFL_CLWK  *cellCross[_CLACT_CROSS_MAX];
  GFL_CLWK  *cellRadar;
  GFL_CLWK  *cellStartUp[STARTUP_ANIME_OBJ_MAX];
  u8 crossColor[_CLACT_CROSS_MAX]; //すれ違いカラー -1してつかう

  GFL_CLWK  *cellMove;

  STARTUP_ENDCALLBACK* pCall;
  void* pWork;

  GFL_TCBSYS* pfade_tcbsys;
  GFL_TCB*                    vblank_tcb;
  void* pfade_tcbsys_wk;
  PALETTE_FADE_PTR            pfade_ptr;
  int createEvent;

  u16 palBase[_CGEAR_NET_CHANGEPAL_MAX][_CGEAR_NET_CHANGEPAL_NUM];
  u16 palTrans[_CGEAR_NET_CHANGEPAL_MAX][_CGEAR_NET_CHANGEPAL_NUM];
  u16 palChangeType[ _CGEAR_NET_CHANGEPAL_MAX ];
  u16 palChangeCol[_CGEAR_NET_PALTYPE_MAX][_CGEAR_NET_CHANGEPAL_NUM];
  u16 tpx;
  u16 tpy;

  // パネル
  PANEL_MARK_ANIME panel_mark[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];

  u8 typeAnim[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];

  u8 GetOpenTrg;
  u8 plt_counter;  //パレットアニメカウンタ
  u8 beacon_bit;   //ビーコンbit
  u8 touchx;    //タッチされた場所
  u8 touchy;    //タッチされた場所
  u8 select_counter;  //選択した時のアニメカウンタ
  u8 bAction;
  u8 cellMoveCreateCount;
  u8 cellMoveType;
  u8 state_seq;
  u8 startCounter;
  u8 bPanelEdit;
  u8 bgno;
  u8 wireles_count;
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(C_GEAR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc* state, int line);
static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork);
static void _modeSelectMenuInit(C_GEAR_WORK* pWork);
static void _modeSelectMenuWait(C_GEAR_WORK* pWork);  // 通常の待機
static void _modeSelectMenuWait0(C_GEAR_WORK* pWork);  // 初期入手の演出
static void _modeSelectMenuWait1(C_GEAR_WORK* pWork);  // スリープ復帰の演出
static void _modeSelectMenuWait2(C_GEAR_WORK* pWork);  // 復帰・初期　共通の演出
static void _gearXY2PanelScreen(int x,int y, int* px, int* py);


static void _modeSetSavePanelType( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type );


// サブBGのセットアップ
static void _createSubBg(C_GEAR_WORK* pWork);
static void _setUpSubAlpha( C_GEAR_WORK* pWork );

// スクリーン操作
static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type,BOOL bNoneWrite);
static void _gearBootInitScreen(C_GEAR_WORK* pWork);
static BOOL _gearBootMain(C_GEAR_WORK* pWork);
static BOOL _gearStartUpMain(C_GEAR_WORK* pWork);

static void _timeAnimation(C_GEAR_WORK* pWork);
static void _typeAnimation(C_GEAR_WORK* pWork);
static void _editMarkONOFF(C_GEAR_WORK* pWork,BOOL bOn);
static void _gearArcCreate(C_GEAR_WORK* pWork, u32 bgno);
static void _arcGearRelease(C_GEAR_WORK* pWork);
static void _gearObjResCreate(C_GEAR_WORK* pWork);
static void _gearObjCreate(C_GEAR_WORK* pWork);
static void _gearCrossObjCreate(C_GEAR_WORK* pWork);

// OAM操作
static void _gearMarkObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearAllObjDrawEnabel(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearStartUpObjDrawEnabel(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearStartStartUpObjAnime(C_GEAR_WORK* pWork);
static BOOL _gearIsEndStartUpObjAnime(const C_GEAR_WORK* cpWork);




// パネルアニメーション処理
static void _PaletteSetColType( C_GEAR_WORK* pWork, int panel,int type );
static void _PaletteMake(C_GEAR_WORK* pWork,const u16* pltt,int type);
static void _PanelPaletteChange(C_GEAR_WORK* pWork);
static void _PanelPaletteUpdate( C_GEAR_WORK* pWork );
static void _PanelPaletteColorSetUp( C_GEAR_WORK* pWork );


static void _PanelMarkAnimeSysInit( C_GEAR_WORK* pWork );
static void _PanelMarkAnimeSysMain( C_GEAR_WORK* pWork );
static BOOL _PanelMarkAnimeSysIsAnime( const C_GEAR_WORK* cpWork );

static void _PanelMarkAnimeInit( PANEL_MARK_ANIME* p_mark, int x, int y );
static void _PanelMarkAnimeStart( PANEL_MARK_ANIME* p_mark, C_GEAR_WORK* pWork, u8 color_type, u8 anime_type, CGEAR_PANELTYPE_ENUM panel_type, u16 frame );
static void _PanelMarkAnimeMain( PANEL_MARK_ANIME* p_mark, const C_GEAR_WORK* cp_work );
static void _PanelMarkAnimeWriteScreen( const PANEL_MARK_ANIME* cp_mark, u32 anime_index );
static BOOL _PanelMarkAnimeIsAnime( const PANEL_MARK_ANIME* cp_mark );

// 選択アニメ
static void _modeSelectAnimInit(C_GEAR_WORK* pWork);
static void _modeSelectAnimWait(C_GEAR_WORK* pWork);
static void _selectAnimInit(C_GEAR_WORK* pWork,int x,int y);

// パレットフェード
static void _PFadeToBlack( C_GEAR_WORK* pWork );
static void _PFadeFromBlack( C_GEAR_WORK* pWork );

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG


//------------------------------------------------------------------------------
/**
 * @brief   チップを置いて良いかどうか
 * @retval  置いていいときTRUE
 */
//------------------------------------------------------------------------------

static BOOL _isSetChip(int x,int y)
{
  if((x % 2) == 0){  // 偶数なら短い列の方   yは=かy-1が隣になる
    if(y < (C_GEAR_PANEL_HEIGHT-1)){
      return TRUE;
    }
  }
  else{
    if(y < (C_GEAR_PANEL_HEIGHT)){
      return TRUE;
    }
  }
  return FALSE;
}



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(C_GEAR_WORK* pWork,StateFunc state)
{
  pWork->state = state;
  pWork->startCounter = 0;
  pWork->state_seq = 0;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc state, int line)
{
  NET_PRINT("cg: %d\n",line);
  _changeState(pWork, state);
}
#endif


//------------------------------------------------------------------------------
/**
 * @brief   与えられたxyから周囲の座標を返す
 * @param   x,y          探す場所
 * @param   xbuff,ybuff  探した場所をバッファする
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _getHexAround(int x,int y, int* xbuff,int* ybuff)
{
  if((x % 2) == 0){  // 偶数なら短い列の方   yは=かy-1が隣になる
    xbuff[0]=x;   ybuff[0]=y-1;
    xbuff[1]=x;   ybuff[1]=y+1;
    xbuff[2]=x+1; ybuff[2]=y;
    xbuff[3]=x+1; ybuff[3]=y-1;
    xbuff[4]=x-1; ybuff[4]=y;
    xbuff[5]=x-1; ybuff[5]=y-1;
  }
  else{   // 奇数なら長い列の方   yは=かy+1が隣になる
    xbuff[0]=x;   ybuff[0]=y-1;
    xbuff[1]=x;   ybuff[1]=y+1;
    xbuff[2]=x+1; ybuff[2]=y;
    xbuff[3]=x+1; ybuff[3]=y+1;
    xbuff[4]=x-1; ybuff[4]=y;
    xbuff[5]=x-1; ybuff[5]=y+1;
  }
}



static void _selectAnimInit(C_GEAR_WORK* pWork,int x,int y)
{
  GFL_CLWK_DATA cellInitData;
  int scrx,scry,i;

  _gearXY2PanelScreen(x, y, &scrx, &scry);
  cellInitData.pos_x = scrx * 8+16;
  cellInitData.pos_y = scry * 8+16;
  cellInitData.anmseq = NANR_c_gear_obj_CellAnime01;
  cellInitData.softpri = 0;
  cellInitData.bgpri = 0;
  i = x + y * C_GEAR_PANEL_WIDTH;
  pWork->cellSelect[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->objRes[_CLACT_CHR],
                                              pWork->objRes[_CLACT_PLT],
                                              pWork->objRes[_CLACT_ANM],
                                              &cellInitData ,
                                              CLSYS_DEFREND_SUB ,
                                              pWork->heapID );
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellSelect[i], TRUE );
  GFL_CLACT_WK_SetDrawEnable( pWork->cellSelect[i], TRUE );
}


static void _modeSelectAnimWait(C_GEAR_WORK* pWork)
{
  int x;
  int y,i;
  int xbuff[6];
  int ybuff[6];

  pWork->select_counter++;

  if(	pWork->select_counter % 2 != 1){
    return;
  }

  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      if(pWork->typeAnim[x][y] == _SELECTANIM_STANDBY)
      {
        _selectAnimInit(pWork, x,y);
        pWork->typeAnim[x][y] = _SELECTANIM_ANIMING;
      }
    }
  }

  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      if(pWork->typeAnim[x][y] == _SELECTANIM_ANIMING)
      {
        _getHexAround(x,y,xbuff,ybuff);
        for(i=0;i<6;i++)
        {
          if((xbuff[i] >= 0) && (xbuff[i] < C_GEAR_PANEL_WIDTH))
          {
            int subnum = 1 - xbuff[i] % 2;
            if((ybuff[i] >= 0) && (ybuff[i] < (C_GEAR_PANEL_HEIGHT-subnum)))
            {
              if(pWork->typeAnim[xbuff[i]][ybuff[i]] == _SELECTANIM_NONE){
                pWork->typeAnim[xbuff[i]][ybuff[i]] = _SELECTANIM_STANDBY;
              }
            }
          }
        }
      }
    }
  }

}



//------------------------------------------------------------------------------
/**
 * @brief   選択されたのでアニメを行う
 * @param   x,y          探す場所
 * @param   xbuff,ybuff  探した場所をバッファする
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeSelectAnimInit(C_GEAR_WORK* pWork)
{
  int x,y;
  CGEAR_PANELTYPE_ENUM type;

  pWork->typeAnim[pWork->touchx][pWork->touchy] = _SELECTANIM_STANDBY;
  type = CGEAR_SV_GetPanelType(pWork->pCGSV,pWork->touchx,pWork->touchy);

  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      if(CGEAR_SV_GetPanelType(pWork->pCGSV,x,y) == type){
        pWork->typeAnim[x][y] = _SELECTANIM_STANDBY;
      }
    }
  }
  _modeSelectAnimWait(pWork);
  _CHANGE_STATE(pWork, _modeSelectAnimWait);
}

//------------------------------------------------------------------------------
/**
 * @brief   タイプに変換
 * @retval  none
 */
//------------------------------------------------------------------------------
static int getTypeToTouchPos(C_GEAR_WORK* pWork,int touchx,int touchy,int *pxp, int* pyp)
{
  int xp,yp;
  int type = CGEAR_PANELTYPE_NONE;

  {  // ギアのタイプ分析
    int ypos[2] = {PANEL_Y1,PANEL_Y2};
    int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};
    touchx = touchx / 8;
    touchy = touchy / 8;
    xp = (touchx - PANEL_X1) / PANEL_SIZEXY;
    if(touchy - ypos[xp % 2] < 0){
      return CGEAR_PANELTYPE_MAX;
    }
    yp = (touchy - ypos[xp % 2]) / PANEL_SIZEXY;

    if((xp < C_GEAR_PANEL_WIDTH) && (yp < yloop[ xp % 2 ]))
    {
      type = CGEAR_SV_GetPanelType(pWork->pCGSV,xp,yp);
    }
  }
  *pxp=xp;
  *pyp=yp;
  return type;
}


//------------------------------------------------------------------------------
/**
 * @brief   パレスにジャンプする
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeSelectJumpPalace(C_GEAR_WORK* pWork)
{

  if(pWork->select_counter==3){
  }



}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードカラーのセットアップ
 *
 *	@param	pWork   ワーク
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteColorSetUp( C_GEAR_WORK* pWork )
{
  GAME_COMM_SYS_PTR pGC = GAMESYSTEM_GetGameCommSysPtr(pWork->pGameSys);

  pWork->beacon_bit=0;
  pWork->plt_counter=0;

  // 次のアニメーション確定処理
  if(Intrude_CheckPalaceConnect(pGC)){
    // pWork->beacon_bit |= GAME_COMM_STATUS_BIT_WIRELESS;  これは使い方がおかしい？
    pWork->beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
  }

  if(!pWork->beacon_bit){
    u32 bit = WIH_DWC_GetAllBeaconTypeBit();
    if(bit & GAME_COMM_SBIT_IRC_ALL){
      pWork->beacon_bit |= _CGEAR_NET_BIT_IR;
    }
    if(bit & GAME_COMM_SBIT_WIRELESS_ALL){
      pWork->beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
    }
    if(bit & GAME_COMM_SBIT_WIFI_ALL){
      pWork->beacon_bit |= _CGEAR_NET_BIT_WIFI;
    }

    // IRCカラー
    if(bit & GAME_COMM_STATUS_BIT_IRC){
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_IRC, _CGEAR_NET_PALTYPE_RED );
    }
    
    // WIFIカラー
    if(bit & GAME_COMM_STATUS_BIT_WIFI){      // 登録済み
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIFI, _CGEAR_NET_PALTYPE_GREEN );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_FREE){  // かぎなし
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIFI, _CGEAR_NET_PALTYPE_YELLOW );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_ZONE){  // 任天堂ゾーン　など　公式
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIFI, _CGEAR_NET_PALTYPE_BLUE );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_LOCK){  // かぎあり
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIFI, _CGEAR_NET_PALTYPE_RED );
    }

    // Wirelesカラー
    if(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR){ // トランシーバーは最優先
      _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIRELES, _CGEAR_NET_PALTYPE_YELLOW );
    }
    else 
    {
      // トランシーバー以外は順番に
      static const u32 WIRELES_COUNT_MASK[ _CGEAR_NET_WIRELES_TYPE_MAX ] = 
      {
        GAME_COMM_STATUS_BIT_WIRELESS, 
        GAME_COMM_STATUS_BIT_WIRELESS_UN, 
        GAME_COMM_STATUS_BIT_WIRELESS_FU, 
      };
      static const u32 WIRELES_COUNT_COLOR[ _CGEAR_NET_WIRELES_TYPE_MAX ] = 
      {
        _CGEAR_NET_PALTYPE_GREEN, 
        _CGEAR_NET_PALTYPE_BLUE, 
        _CGEAR_NET_PALTYPE_RED, 
      };
      int i;

      for( i=0; i<_CGEAR_NET_WIRELES_TYPE_MAX; i++ ){
        pWork->wireles_count = (pWork->wireles_count + 1) % _CGEAR_NET_WIRELES_TYPE_MAX;
        if( bit & WIRELES_COUNT_MASK[ pWork->wireles_count ] ){
          _PaletteSetColType( pWork, _CGEAR_NET_CHANGEPAL_WIRELES, WIRELES_COUNT_COLOR[ pWork->wireles_count ] );
          break;
        }
      }
    }
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  パネルのパレット変更　アップデート処理
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteUpdate( C_GEAR_WORK* pWork )
{
  {
    pWork->plt_counter++;
    if(pWork->plt_counter>=_CGEAR_NET_CHANGEPAL_ANM_COUNT_MAX)
    {
      _PanelPaletteColorSetUp( pWork );
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   通信を拾ったら、パレットで点滅させる
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _PanelPaletteChange(C_GEAR_WORK* pWork)
{
  u8 bittype[_CGEAR_NET_CHANGEPAL_MAX]={_CGEAR_NET_BIT_IR,_CGEAR_NET_BIT_WIRELESS,_CGEAR_NET_BIT_WIFI};
  CGEAR_PANELTYPE_ENUM type[_CGEAR_NET_CHANGEPAL_MAX] = {CGEAR_PANELTYPE_IR,CGEAR_PANELTYPE_WIRELESS,CGEAR_PANELTYPE_WIFI};
  //u16 rgbmask[3] = {0x1f,0x3e,0x7c0};

  int i,x,y,pal;

  for(i = 0 ; i < _CGEAR_NET_CHANGEPAL_MAX ; i++)
  {
    if((pWork->plt_counter <= _CGEAR_NET_CHANGEPAL_ANM_COUNT) && (pWork->beacon_bit & bittype[ i ]))
    {
      for(pal = 0; pal < _CGEAR_NET_CHANGEPAL_NUM; pal++)
      {
        int add,rgb,base;
        int shift;
        int mod;
        pWork->palTrans[i][pal] = 0;
        for(rgb = 0; rgb < 3; rgb++){
          shift = rgb * 5;
          base = (pWork->palBase[i][pal] >> shift) & 0x1f;
          add = ((pWork->palChangeCol[ pWork->palChangeType[i] ][pal] >> shift) & 0x1f) - base;
          mod = (pWork->plt_counter % _CGEAR_NET_CHANGEPAL_ANM_COUNT);
          if(mod >= (_CGEAR_NET_CHANGEPAL_ANM_COUNT/2)){
            mod = _CGEAR_NET_CHANGEPAL_ANM_COUNT - mod;
          }
          pWork->palTrans[i][pal] |= ((((add * mod) / (_CGEAR_NET_CHANGEPAL_ANM_COUNT/2)) + base) & 0x1f)<<(shift);
        }
      }
      PaletteWorkSet(pWork->pfade_ptr, pWork->palTrans[i], FADE_SUB_OBJ, (16*(i+_CGEAR_NET_CHANGEPAL_POSY) + _CGEAR_NET_CHANGEPAL_POSX), _CGEAR_NET_CHANGEPAL_NUM*2);
    }
  }
}


static void _PaletteMake(C_GEAR_WORK* pWork,const u16* pltt,int type)
{
  int y=type,x;

  for(x = 0 ; x < _CGEAR_NET_CHANGEPAL_NUM; x++){
    pWork->palChangeCol[y][x]= pltt[ (type*16) + _CGEAR_NET_CHANGEPAL_POSX + x ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルのカラータイプを設定
 *
 */
//-----------------------------------------------------------------------------
static void _PaletteSetColType( C_GEAR_WORK* pWork, int panel,int type )
{
  GF_ASSERT( type < _CGEAR_NET_PALTYPE_MAX );
  GF_ASSERT( panel < _CGEAR_NET_CHANGEPAL_NUM );
  pWork->palChangeType[ panel ] = type;
}



//----------------------------------------------------------------------------
/**
 *	@brief  パネルマークアニメシステム　初期化
 *
 *	@param	pWork   
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeSysInit( C_GEAR_WORK* pWork )
{
  int i, j;
  for( i=0; i<C_GEAR_PANEL_WIDTH; i++ ){
    for( j=0; j<C_GEAR_PANEL_HEIGHT; j++ ){
      _PanelMarkAnimeInit( &pWork->panel_mark[i][j], i, j );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルマーク　アニメシステム　メイン
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeSysMain( C_GEAR_WORK* pWork )
{
  int i, j;
  for( i=0; i<C_GEAR_PANEL_WIDTH; i++ ){
    for( j=0; j<C_GEAR_PANEL_HEIGHT; j++ ){
      _PanelMarkAnimeMain( &pWork->panel_mark[i][j], pWork );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルマーク　アニメシステム　メイン
 *
 *	@retval TRUE  アニメ中
 *	@retval FALSE  アニメ完了
 */
//-----------------------------------------------------------------------------
static BOOL _PanelMarkAnimeSysIsAnime( const C_GEAR_WORK* cpWork )
{
  int i, j;
  BOOL result = FALSE;
  for( i=0; i<C_GEAR_PANEL_WIDTH; i++ ){
    for( j=0; j<C_GEAR_PANEL_HEIGHT; j++ ){
      if( _PanelMarkAnimeIsAnime( &cpWork->panel_mark[i][j] ) ){
        result = TRUE;
        break;
      }
    }
  }

  return result;
}


//----------------------------------------------------------------------------
/**
 *	@brief  パネルマークのスクリーンパレットアニメーション  初期化
 *
 *	@param	p_mark      マークワーク
 *	@param  x           インデックスｘ
 *	@param  y           インデックスｙ
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeInit( PANEL_MARK_ANIME* p_mark, int x, int y )
{
  GFL_STD_MemClear( p_mark, sizeof(PANEL_MARK_ANIME) );

  p_mark->x = x;
  p_mark->y = y;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルマークのスクリーンパレットアニメーション　開始
 *
 *	@param	p_mark        マークワーク
 *	@param	color_type    カラータイプ
 *	@param  panel_type    パネルタイプ
 *	@param	anime_type    アニメタイプ
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeStart( PANEL_MARK_ANIME* p_mark, C_GEAR_WORK* pWork, u8 color_type, u8 anime_type, CGEAR_PANELTYPE_ENUM panel_type, u16 frame )
{
  GF_ASSERT( color_type < PANEL_COLOR_TYPE_MAX );
  GF_ASSERT( anime_type < PANEL_ANIME_TYPE_MAX );
  
  p_mark->count       = 0;
  p_mark->color       = color_type;
  p_mark->anime_on    = TRUE;
  p_mark->one_frame   = frame;

  if( anime_type == PANEL_ANIME_TYPE_ON_OFF ){
    p_mark->end_anime_index  = 0;  // アニメーションインデックスの終了値
  }else if( anime_type == PANEL_ANIME_TYPE_ON ){
    p_mark->end_anime_index  = PANEL_COLOR_PATTERN_NUM-1;  // アニメーションインデックスの終了値
  }else if( anime_type == PANEL_ANIME_TYPE_OFF ){
    // 0, 1, 2, 3, 4, 3, 2, 1, 0
    p_mark->end_anime_index  = sc_PANEL_COLOR_ANIME_END_INDEX[ p_mark->x ];  // アニメーションインデックスの終了値
    p_mark->count       = frame * PANEL_COLOR_PATTERN_NUM;
  }

  // color用のパネルを書き込み。
  _gearPanelBgScreenMake(pWork, p_mark->x, p_mark->y, panel_type, FALSE);
  _PanelMarkAnimeWriteScreen( p_mark, p_mark->count/frame );
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルマークのスクリーンパレットアニメーション　メイン
 *
 *	@param	p_mark      マークワーク
 *	@param	cp_work     ワーク
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeMain( PANEL_MARK_ANIME* p_mark, const C_GEAR_WORK* cp_work )
{
  u32 anime_index;
  BOOL screen_write;
  
  if( p_mark->anime_on ){

    p_mark->count ++;

    anime_index = p_mark->count / p_mark->one_frame;
    screen_write = FALSE;

    
    // アニメインデックスがPANEL_COLOR_PATTERN_NUMまでは、同じ
    if( (anime_index >= PANEL_COLOR_PATTERN_NUM) ){
      anime_index = (PANEL_COLOR_PATTERN_NUM-1) - (anime_index % PANEL_COLOR_PATTERN_NUM);

      // 終了チェック
      if( anime_index == p_mark->end_anime_index ){
        p_mark->anime_on = FALSE;
      }
    }

    if( (anime_index < PANEL_COLOR_PATTERN_NUM) && 
        ((p_mark->count % p_mark->one_frame) == 0) ){
      //スクリーン書き換え
      screen_write = TRUE;
    }

    if( screen_write ){
      _PanelMarkAnimeWriteScreen( p_mark, anime_index );
    }
  }
  
  // 
}

//----------------------------------------------------------------------------
/**
 *	@brief  スクリーンのパレット切り替え
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeWriteScreen( const PANEL_MARK_ANIME* cp_mark, u32 anime_index )
{
  int scrn_x, scrn_y;
  u32 pal_index;

  pal_index    = sc_PANEL_COLOR_ANIME_TBL[ cp_mark->color ][ anime_index ];

  _gearXY2PanelScreen(cp_mark->x, cp_mark->y, &scrn_x, &scrn_y );
  
  GFL_BG_ChangeScreenPalette( GEAR_BUTTON_FRAME, 
      scrn_x, scrn_y, PANEL_SIZEXY, PANEL_SIZEXY, pal_index );

  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}


//----------------------------------------------------------------------------
/**
 *	@brief  アニメーション中チェック
 *
 *	@param	cp_mark 
 */
//-----------------------------------------------------------------------------
static BOOL _PanelMarkAnimeIsAnime( const PANEL_MARK_ANIME* cp_mark )
{
  return cp_mark->anime_on;
}





//------------------------------------------------------------------------------
/**
 * @brief   ギアｘｙをスクリーンに変換
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearXY2PanelScreen(int x,int y, int* px, int* py)
{
  int ypos[2] = {PANEL_Y1,PANEL_Y2};
  *px = PANEL_X1 + x * PANEL_SIZEXY;
  *py = ypos[ x % 2 ] + y * PANEL_SIZEXY;
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルの数を取得
 * @retval  none
 */
//------------------------------------------------------------------------------

static int _gearPanelTypeNum(C_GEAR_WORK* pWork, CGEAR_PANELTYPE_ENUM type)
{
  int x,y,i;

  if(type == CGEAR_PANELTYPE_BASE){
    return 1;
  }
  i=0;
  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH; x++)
    {
      if(CGEAR_SV_GetPanelType(pWork->pCGSV,x,y)==type)
      {
        i++;
      }
    }
  }
  return i;
}


static void _PFadeToBlack( C_GEAR_WORK* pWork )
{
  // 黒く
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,   -120, 0, 16, 0x0, pWork->pfade_tcbsys
    );
}

static void _PFadeFromBlack( C_GEAR_WORK* pWork )
{
  // 黒から戻る
  static const int time = 1;
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,  time,  16, 0, 0x0, pWork->pfade_tcbsys
    );
}



static void _gearBootInitScreen(C_GEAR_WORK* pWork)
{
  int i;
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
      _gearPanelBgScreenMake(pWork, x, y, CGEAR_PANELTYPE_NONE, FALSE);
    }
  }
  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}



//----------------------------------------------------------------------------
/**
 *	@brief  スリープ復帰アニメ
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL _gearBootMain(C_GEAR_WORK* pWork)
{
  int i;
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  switch( pWork->state_seq ){
  case BOOT_ANIME_SEQ_ALL_ON:
    // まず全体を点等
    for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
      for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
        _PanelMarkAnimeStart( &pWork->panel_mark[ x ][ y ], pWork, PANEL_COLOR_TYPE_BASE, PANEL_ANIME_TYPE_ON, CGEAR_PANELTYPE_BOOT, PANEL_ANIME_DEF_FRAME );
      }
    }
    pWork->state_seq ++;
    break;

  case BOOT_ANIME_SEQ_ALL_WAIT:
    if( _PanelMarkAnimeSysIsAnime( pWork ) == FALSE ){

      pWork->state_seq ++;
    }
    break;

  case BOOT_ANIME_SEQ_SMOOTHIN_MAIN:
    {
      u32 panel_type;
      for(i=0;i < elementof(screenTable);i++){
        if(screenTable[i].time == pWork->startCounter){
          panel_type = CGEAR_SV_GetPanelType( pWork->pCGSV, screenTable[i].x, screenTable[i].y );
          _PanelMarkAnimeStart( &pWork->panel_mark[ screenTable[i].x ][ screenTable[i].y ], 
              pWork, sc_PANEL_TYPE_TO_COLOR[ panel_type ], PANEL_ANIME_TYPE_OFF, panel_type, PANEL_ANIME_DEF_FRAME );
        }
      }

      if( pWork->startCounter >= screenTable[ NELEMS(screenTable)-1 ].time ){
        _PFadeFromBlack(pWork);
        _gearMarkObjDrawEnable(pWork,TRUE);
        pWork->state_seq ++;
      }

      // ずっとカウントアップ
      pWork->startCounter++;
    }
    break;
    
  case BOOT_ANIME_SEQ_END:
    if( _PanelMarkAnimeSysIsAnime( pWork ) == FALSE ){
      return TRUE;
    }
    break;

  default:
    break;
  }



  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スタートアップ　スクリーンアニメ　メイン
 *
 *	@param	pWork
 */
//-----------------------------------------------------------------------------
static BOOL _gearStartUpMain(C_GEAR_WORK* pWork)
{
  int i;


  if( StartUpScreenTable[0].downtime <= pWork->startCounter ){
    if( _PanelMarkAnimeSysIsAnime( pWork ) == FALSE ){
      return TRUE;
    }
    return FALSE;
  }


  // 徐々に枠が増えていく
  for(i=0;i < elementof(StartUpScreenTable);i++){
    if(StartUpScreenTable[i].time == pWork->startCounter) {
      _PanelMarkAnimeStart( &pWork->panel_mark[ StartUpScreenTable[i].x ][ StartUpScreenTable[i].y ], pWork, PANEL_COLOR_TYPE_BASE, PANEL_ANIME_TYPE_ON_OFF, CGEAR_PANELTYPE_BASE, PANEL_ANIME_DEF_FRAME );
      
      //_gearPanelBgScreenMake(pWork, StartUpScreenTable[i].x, StartUpScreenTable[i].y, 
      //    CGEAR_PANELTYPE_BOOT, FALSE);
    }
  }

  pWork->startCounter++;
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   パネルタイプをスクリーンに書き込む
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type,BOOL bNoneWrite)
{
  int ypos[2] = {PANEL_Y1,PANEL_Y2};
  int x,y,i,j;
  int typepos[] = {0,  0x0c,0x10,0x14,0x18,0x1c};
  int palpos[] =  {0,0x1000,0x2000,0x3000};
  int palpos2[] =  {0x7000,0x6000,0x5000,0x4000,0x4000,0x4000,0x5000,0x6000,0x7000};
  int palpos3[] =  {0x7000,0x6000,0x5000,0x4000,0xB000,0x4000,0x5000,0x6000,0x7000};
  u16* pScrAddr = GFL_BG_GetScreenBufferAdrs(GEAR_BUTTON_FRAME );
  int xscr;
  int yscr;

  _gearXY2PanelScreen(xs,ys,&xscr,&yscr);
  for(y = yscr, i = 0; i < PANEL_SIZEXY; y++, i++){
    for(x = xscr, j = 0; j < PANEL_SIZEXY; x++, j++){
      if((x >= 0) && (x < 32)){
        int charpos = typepos[type] + i * 0x20 + j;
        int scr = x + (y * 32);
        if(type == CGEAR_PANELTYPE_NONE){
          charpos = 0;
          if(bNoneWrite){
            pScrAddr[scr]=0;
          }
        }
        if(type==CGEAR_PANELTYPE_BOOT){
          pScrAddr[scr] = palpos3[xs] + charpos;
        }
        else if(type==CGEAR_PANELTYPE_BASE){
          pScrAddr[scr] = palpos2[xs] + charpos;
        }
        else{
          pScrAddr[scr] = palpos[type] + charpos;
        }
        //		NET_PRINT("x%d y%d  %d \n",x,y,palpos[type] + charpos);
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアセーブデータをみて、文字を出す良い場所を返す
 * @retval  見つからなかったらFALSE
 */
//------------------------------------------------------------------------------

static void _gearGetTypeBestPosition(C_GEAR_WORK* pWork,CGEAR_PANELTYPE_ENUM type, int* px, int* py)
{
  int x,y;
  int first_x, first_y;
  BOOL first_data;

  first_x = 0;
  first_y = 0;
  first_data = FALSE;

  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH; x++)
    {
      if(CGEAR_SV_GetPanelType(pWork->pCGSV,x,y) == type)
      {
        // 最初にあった位置を覚えておく。
        if( first_data == FALSE ){
          first_x = x;
          first_y = y;
          first_data = TRUE;
        }

        // 
        if( CGEAR_SV_IsPanelTypeIcon(pWork->pCGSV,x,y) ){ // アイコンが出る場所
          _gearXY2PanelScreen(x,y,px,py);
          return ;
        }
      }
    }
  }
  
  // もしなかったら、最初に見つかった位置にする。
  _gearXY2PanelScreen(first_x,first_y,px,py);
  CGEAR_SV_SetPanelType( pWork->pCGSV, first_x, first_y, CGEAR_PANELTYPE_GET_ICON_TYPE(type) );
}

//------------------------------------------------------------------------------
/**
 * @brief   ギアのパネルをセーブデータにしたがって作る
 * @retval  none
 */
//------------------------------------------------------------------------------


static void _gearPanelBgCreate(C_GEAR_WORK* pWork)
{
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
      _gearPanelBgScreenMake(pWork, x, y, CGEAR_SV_GetPanelType(pWork->pCGSV,x,y),FALSE);
    }
  }
  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}


//------------------------------------------------------------------------------
/**
 * @brief   ギアのARCを読み込み
 * @retval  none
 */
//------------------------------------------------------------------------------

static u32 _bgpal[]={NARC_c_gear_c_gear_NCLR,NARC_c_gear_c_gear2_NCLR,NARC_c_gear_c_gear_NCLR};
static u32 _bgcgx[]={NARC_c_gear_c_gear_NCGR,NARC_c_gear_c_gear2_NCGR,NARC_c_gear_c_gear_NCGR};


//static u32 _objpal[]={NARC_c_gear_c_gear_obj_NCLR,NARC_c_gear_c_gear2_obj_NCLR,NARC_c_gear_c_gear_obj_NCLR};
//static u32 _objcgx[]={NARC_c_gear_c_gear_obj_NCGR,NARC_c_gear_c_gear2_obj_NCGR,NARC_c_gear_c_gear_obj_NCGR};

static void _gearArcCreate(C_GEAR_WORK* pWork, u32 bgno)
{
  u32 scrno=0;


  GFL_ARCHDL_UTIL_TransVramPalette( pWork->handle, _bgpal[ bgno ],
                                    PALTYPE_SUB_BG, 0, 0,  HEAPID_FIELD_SUBSCREEN);



  // サブ画面BGキャラ転送
  pWork->subchar = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( pWork->handle, _bgcgx[bgno],
                                                                GEAR_MAIN_FRAME, 0, 0, HEAPID_FIELD_SUBSCREEN);

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(pWork->handle,
                                         NARC_c_gear_c_gear01_NSCR,
                                         GEAR_MAIN_FRAME, 0,
                                         GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                         HEAPID_FIELD_SUBSCREEN);



}



static void _gearObjResCreate(C_GEAR_WORK* pWork)
{
  u32 scrno=0;

  {
    int x,y, i;
    void* buff;
    NNSG2dPaletteData* pltt;
    u16* pltt_data;

    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_c_gear_obj_NCLR, &pltt, GFL_HEAP_LOWID(HEAPID_FIELD_SUBSCREEN) );

    pltt_data = pltt->pRawData;
    for(y = 0 ; y < _CGEAR_NET_CHANGEPAL_MAX; y++){
      for(x = 0 ; x < _CGEAR_NET_CHANGEPAL_NUM; x++){
        pWork->palBase[y][x] = pltt_data[(16*(_CGEAR_NET_CHANGEPAL_POSY+y)) + _CGEAR_NET_CHANGEPAL_POSX + x];
      }
    }
    GFL_HEAP_FreeMemory( buff );

    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_c_gear_obj_ani_NCLR, &pltt, GFL_HEAP_LOWID(HEAPID_FIELD_SUBSCREEN) );

    for( i=0; i<_CGEAR_NET_PALTYPE_MAX; i++ ){
      _PaletteMake(pWork,pltt->pRawData,i);
    }

    GFL_HEAP_FreeMemory( buff );
 
  }



  pWork->objRes[_CLACT_PLT] = GFL_CLGRP_PLTT_Register( pWork->handle,
                                                       NARC_c_gear_c_gear_obj_NCLR,
                                                       CLSYS_DRAW_SUB , 0 , HEAPID_FIELD_SUBSCREEN );
  pWork->objRes[_CLACT_CHR] = GFL_CLGRP_CGR_Register( pWork->handle,
                                                      NARC_c_gear_c_gear_obj_NCGR ,
                                                      FALSE , CLSYS_DRAW_SUB , HEAPID_FIELD_SUBSCREEN );

  pWork->objRes[_CLACT_ANM] = GFL_CLGRP_CELLANIM_Register( pWork->handle,
                                                           NARC_c_gear_c_gear_obj_NCER ,
                                                           NARC_c_gear_c_gear_obj_NANR ,
                                                           pWork->heapID );


  GFL_NET_WirelessIconEasy_HoldLCD(FALSE, pWork->heapID);
  GFL_NET_ChangeIconPosition(240-22,14);
  GFL_NET_ReloadIcon();
}





static void _gearDecalScreenArcCreate(C_GEAR_WORK* pWork,BOOL bDecal)
{
  u32 scrno=0;

  if(bDecal){
    scrno = NARC_c_gear_c_gear00_NSCR;
  }
  else{
    scrno = NARC_c_gear_c_gear00_nodecal_NSCR;
  }

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(pWork->handle,
                                         scrno,
                                         GEAR_BMPWIN_FRAME, 0,
                                         GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                         HEAPID_FIELD_SUBSCREEN);
}


//------------------------------------------------------------------------------
/**
 * @brief   サブ画面の設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _createSubBg(C_GEAR_WORK* pWork)
{
  int i = 0;
  for(i = GFL_BG_FRAME0_S;i <= GFL_BG_FRAME3_S ; i++)
  {
    GFL_BG_SetVisible( i, VISIBLE_OFF );
  }
  {
    int frame = GEAR_MAIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 3 );
    //  GFL_BG_FillCharacter( frame, 0x00, 1, 0 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_BMPWIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 1 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }

  {
    int frame = GEAR_BUTTON_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x6000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, 2 );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }

  // アルファ値設定
  _setUpSubAlpha( pWork );
}

//----------------------------------------------------------------------------
/**
 *	@brief  サブ面ALPHA設定
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _setUpSubAlpha( C_GEAR_WORK* pWork )
{
  G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG0, GX_BLEND_PLANEMASK_BG1 , 9, 15 );
}



//------------------------------------------------------------------------------
/**
 * @brief   受け取った数のウインドウを等間隔に作る 幅は3char
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _buttonWindowCreate(int num,int* pMsgBuff,C_GEAR_WORK* pWork)
{
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック  GFL_BMN_EVENT_TOUCHのみ
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------

static void _touchFunction(C_GEAR_WORK *pWork, int bttnid)
{
  u32 touchx,touchy;

  switch(bttnid){
  case 0:
    if(GFL_UI_TP_GetPointCont(&touchx,&touchy)){
      pWork->tpx = touchx;
      pWork->tpy = touchy;
    }
    pWork->cellMoveCreateCount = 0;
    break;
  case 1:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );
    pWork->bPanelEdit = pWork->bPanelEdit ^ 1;
    _editMarkONOFF(pWork, pWork->bPanelEdit);
    break;
  case 2:
//    PMSND_PlaySE( SEQ_SE_MSCL_07 );
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CGEAR_HELP;
//    FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_CGEAR_HELP);
    //help
    break;
  case 3:
  //  PMSND_PlaySE( SEQ_SE_MSCL_07 );
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW;
//    FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW);
    break;
  case 4:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_SCANRADAR;
//    FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_SCANRADAR);
    break;
  case 5:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );

    GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                             GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));

//    _arcGearRelease(pWork);
    _gearArcCreate(pWork, pWork->bgno);
    _gearPanelBgCreate(pWork);	// パネル作成
  //  _gearObjCreate(pWork); //CLACT設定
    //_gearCrossObjCreate(pWork);
    _gearMarkObjDrawEnable(pWork,TRUE);

    pWork->bgno++;
    pWork->bgno = pWork->bgno % _CGEAR_TYPE_PATTERN_NUM;

    break;
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief	ボタンイベントコールバック
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  C_GEAR_WORK *pWork = p_work;
  u32 friendNo;
  u32 touchx,touchy;
  int xp,yp;
  int type = CGEAR_PANELTYPE_NONE;

  if(!pWork->bAction){
    return;
  }

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:
    _touchFunction(pWork,bttnid);
    break;
  case GFL_BMN_EVENT_HOLD:
    if(pWork->bPanelEdit){
      if(GFL_UI_TP_GetPointCont(&touchx,&touchy)){
        pWork->tpx=touchx;
        pWork->tpy=touchy;
      }
      if(pWork->cellMove){
        GFL_CLACTPOS pos;
        pos.x = pWork->tpx;  // OBJ表示の為の補正値
        pos.y = pWork->tpy;
        GFL_CLACT_WK_SetPos(pWork->cellMove, &pos, CLSYS_DEFREND_SUB);
      }
      else if(pWork->cellMoveCreateCount > 20){
        GFL_CLWK_DATA cellInitData;
        int type = getTypeToTouchPos(pWork,touchx,touchy,&xp,&yp);
        if(type != CGEAR_PANELTYPE_MAX){
          pWork->cellMoveType = type;
        }
        if(pWork->cellMoveType != CGEAR_PANELTYPE_NONE)
        {

          //セルの生成
          cellInitData.pos_x = pWork->tpx;
          cellInitData.pos_y = pWork->tpy;
          cellInitData.anmseq = NANR_c_gear_obj_CellAnime01 + pWork->cellMoveType - 1;
          cellInitData.softpri = 0;
          cellInitData.bgpri = 0;
          pWork->cellMove = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                 pWork->objRes[_CLACT_CHR],
                                                 pWork->objRes[_CLACT_PLT],
                                                 pWork->objRes[_CLACT_ANM],
                                                 &cellInitData ,
                                                 CLSYS_DEFREND_SUB ,
                                                 pWork->heapID );
          GFL_CLACT_WK_SetDrawEnable( pWork->cellMove, TRUE );
          GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellMove, TRUE );
        }
      }
      else{
        pWork->cellMoveCreateCount++;
      }
    }
    break;

  case GFL_BMN_EVENT_RELEASE:		///< 離された瞬間

    touchx=pWork->tpx;
    touchy=pWork->tpy;
    pWork->tpx=0;
    pWork->tpy=0;
    type = getTypeToTouchPos(pWork,touchx,touchy,&xp,&yp);  // ギアのタイプ分析
    if(type == CGEAR_PANELTYPE_MAX){
      return;
    }

    if(pWork->cellMove){
      GFL_CLACT_WK_Remove( pWork->cellMove );
      pWork->cellMove=NULL;

      if((type == CGEAR_PANELTYPE_BASE) ||(  _gearPanelTypeNum(pWork,type) > 1 && _isSetChip(xp,yp)))  //下にあるタイプが一個以上ある場合
      {
        type = pWork->cellMoveType;
        _modeSetSavePanelType(pWork, pWork->pCGSV,xp,yp,type);
        _gearPanelBgScreenMake(pWork, xp, yp,type, FALSE);
        GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
      }
      return;
    }


    if(pWork->bPanelEdit)  ///< パネルタイプを変更
    {
      if(((_gearPanelTypeNum(pWork,type) > 1 ) || (type == CGEAR_PANELTYPE_BASE))&& _isSetChip(xp,yp))
      {
        type = (type+1) % CGEAR_PANELTYPE_MAX;
        _modeSetSavePanelType(pWork, pWork->pCGSV,xp,yp,type);
        _gearPanelBgScreenMake(pWork, xp, yp,type, FALSE);
        GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
        PMSND_PlaySE( SEQ_SE_MSCL_07 );
      }
    }
    else{    ///< ギアメニューを変更
      pWork->touchx = xp;
      pWork->touchy = yp;

      if(GFL_NET_IsInit()){  //通信ONなら
        switch(type){
        case CGEAR_PANELTYPE_IR:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_IRC;
//          FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_IRC);
          break;
        case CGEAR_PANELTYPE_WIRELESS:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_WIRELESS;
//          FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_WIRELESS);
          break;
        case CGEAR_PANELTYPE_WIFI:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_GSYNC;
//          FIELD_SUBSCREEN_SetAction(pWork->subscreen, FIELD_SUBSCREEN_ACTION_GSYNC);
          break;
        }
      }
    }
    break;

  default:
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   エディットモードのONOFF
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _editMarkONOFF(C_GEAR_WORK* pWork,BOOL bOn)
{
  if(bOn){
    GFL_CLACT_WK_SetAnmIndex( pWork->cellCursor[_CLACT_EDITMARKON], 0 );
  }
  else{
    GFL_CLACT_WK_SetAnmIndex( pWork->cellCursor[_CLACT_EDITMARKON], 1 );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   OBJの初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjCreate(C_GEAR_WORK* pWork)
{
  int i;
  GFL_CLWK_DATA cellInitData;
  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);


  for(i=0;i < _CLACT_TIMEPARTS_MAX ;i++)
  {
    static u8 anmbuff[]=
    {
      NANR_c_gear_obj_CellAnime_ampm,
      NANR_c_gear_obj_CellAnime_NO2,NANR_c_gear_obj_CellAnime_NO10a,
      NANR_c_gear_obj_CellAnime_colon,
      NANR_c_gear_obj_CellAnime_NO6,NANR_c_gear_obj_CellAnime_NO10b,
      NANR_c_gear_obj_CellAnime_batt1,
      NANR_c_gear_obj_CellAnime_loro_all,
      NANR_c_gear_obj_CellAnime_help,
      NANR_c_gear_obj_CellAnime_cus_on,
      };
    static u8 xbuff[]=
    {
      63,      42-10,      48-10,      52-10,      57-10,      63-10,
      178,      128,
      POS_HELPMARK_X,      POS_EDITMARK_X,    };
    static u8 ybuff[]=
    {
      22,   22,
      22,   22,
      22,   22,
      22,   22,
      POS_HELPMARK_Y,
      POS_EDITMARK_Y,
    };

    //セルの生成
    cellInitData.pos_x = xbuff[i];
    cellInitData.pos_y = ybuff[i];
    cellInitData.anmseq = anmbuff[i];
    if(NANR_c_gear_obj_CellAnime_batt1==cellInitData.anmseq){
      if( OS_IsRunOnTwl() ){//DSIなら
        cellInitData.anmseq = NANR_c_gear_obj_CellAnime_batt2;
      }
    }

    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    pWork->cellCursor[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                pWork->objRes[_CLACT_CHR],
                                                pWork->objRes[_CLACT_PLT],
                                                pWork->objRes[_CLACT_ANM],
                                                &cellInitData ,
                                                CLSYS_DEFREND_SUB ,
                                                pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], TRUE );
  }
  _editMarkONOFF(pWork,FALSE);

  for(i=0;i < _CLACT_TYPE_MAX ;i++)
  {
    static int anmbuff[]=
    {
      NANR_c_gear_obj_CellAnime_IR,
      NANR_c_gear_obj_CellAnime_WIRELESS,
      NANR_c_gear_obj_CellAnime_WIFI,
    };

    //セルの生成

    cellInitData.pos_x = 8;
    cellInitData.pos_y = 8;
    cellInitData.anmseq = anmbuff[i];
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    pWork->cellType[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->objRes[_CLACT_CHR],
                                              pWork->objRes[_CLACT_PLT],
                                              pWork->objRes[_CLACT_ANM],
                                              &cellInitData ,
                                              CLSYS_DEFREND_SUB,
                                              pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellType[i], FALSE );
  }
  _timeAnimation(pWork);
  _typeAnimation(pWork);

  // 起動セルアニメ
  for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
  {
    //セルの生成

    cellInitData.pos_x = STARTUP_ANIME_POS_X;
    cellInitData.pos_y = STARTUP_ANIME_POS_Y;
    cellInitData.anmseq = STARTUP_ANIME_INDEX_START + i;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    //↑矢印
    pWork->cellStartUp[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->objRes[_CLACT_CHR],
                                              pWork->objRes[_CLACT_PLT],
                                              pWork->objRes[_CLACT_ANM],
                                              &cellInitData ,
                                              CLSYS_DEFREND_SUB,
                                              pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellStartUp[i], FALSE );
    GFL_CLACT_WK_SetAutoAnmSpeed( pWork->cellStartUp[i], FX32_ONE*2 );
    GFL_CLACT_WK_SetObjMode( pWork->cellStartUp[i], GX_OAM_MODE_XLU );
  }
  
}


//------------------------------------------------------------------------------
/**
 * @brief   マーカーの表示許可
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearMarkObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg)
{
  int i;
  for(i=0;i < _CLACT_TYPE_MAX ;i++)
  {
    GFL_CLACT_WK_SetDrawEnable( pWork->cellType[i], bFlg );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  全OAMの表示許可
 *
 *	@param	pWork
 *	@param	bFlg
 */
//-----------------------------------------------------------------------------
static void _gearAllObjDrawEnabel(C_GEAR_WORK* pWork,BOOL bFlg)
{
  int i;

  // 選択時アニメ用
  for(i=0;i < C_GEAR_PANEL_WIDTH * C_GEAR_PANEL_HEIGHT;i++)
  {
    if( pWork->cellSelect[i] ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellSelect[i], bFlg );
    }
  }
  
  // タイプ表示用
  for(i=0;i < _CLACT_TYPE_MAX ;i++)
  {
    if( pWork->cellType[i] ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellType[i], bFlg );
    }
  }

  // カーソル
  for(i=0;i < _CLACT_TIMEPARTS_MAX;i++)
  {
    if( pWork->cellCursor[i] ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], bFlg );
    }
  }


  // すれ違いよう
  for(i=0;i < _CLACT_CROSS_MAX;i++)
  {
    if( pWork->cellCross[i] ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCross[i], bFlg );
    }
  }

  // レーダー
  if( pWork->cellRadar ){
    GFL_CLACT_WK_SetDrawEnable( pWork->cellRadar, bFlg );
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの表示　ON・OFF
 */
//-----------------------------------------------------------------------------
static void _gearStartUpObjDrawEnabel(C_GEAR_WORK* pWork,BOOL bFlg)
{
  int i;

  // 起動時アニメ用
  for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
  {
    GFL_CLACT_WK_SetDrawEnable( pWork->cellStartUp[i], bFlg );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの開始
 */
//-----------------------------------------------------------------------------
static void _gearStartStartUpObjAnime(C_GEAR_WORK* pWork)
{
  int i;

  // オートアニメーション開始
  for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
  {
    GFL_CLACT_WK_ResetAnm( pWork->cellStartUp[i] );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellStartUp[i], TRUE );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの終了待ち
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL _gearIsEndStartUpObjAnime(const C_GEAR_WORK* cpWork)
{
  int i;

  // すべてのアニメーションの完了を待つ
  for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
  {
    if( GFL_CLACT_WK_CheckAnmActive( cpWork->cellStartUp[i] ) == TRUE ){
      return FALSE;
    }
  }
  return TRUE;
}



//------------------------------------------------------------------------------
/**
 * @brief   すれ違いOBJの初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearCrossObjCreate(C_GEAR_WORK* pWork)
{
  int i;
  GFL_CLWK_DATA cellInitData;

  for(i=0;i < _CLACT_CROSS_MAX ;i++)
  {

    //セルの生成

    cellInitData.pos_x = 128-(8*_CLACT_CROSS_MAX/2) + 8*i + 8;
    cellInitData.pos_y = 180;
    cellInitData.anmseq = NANR_c_gear_obj_CellAnime_sure01;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->cellCross[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->objRes[_CLACT_CHR],
                                               pWork->objRes[_CLACT_PLT],
                                               pWork->objRes[_CLACT_ANM],
                                               &cellInitData ,
                                               CLSYS_DEFREND_SUB,
                                               pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCross[i], TRUE );
    GFL_CLACT_WK_SetAnmIndex( pWork->cellCross[i], 16);
    //pWork->crossColor[i]=i+1;
  }

  {
    //セルの生成

    cellInitData.pos_x = POS_SCANRADAR_X;
    cellInitData.pos_y = POS_SCANRADAR_Y;
    cellInitData.anmseq = NANR_c_gear_obj_CellAnime_radar;
    cellInitData.softpri = 0;
    cellInitData.bgpri = 0;
    pWork->cellRadar = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                            pWork->objRes[_CLACT_CHR],
                                            pWork->objRes[_CLACT_PLT],
                                            pWork->objRes[_CLACT_ANM],
                                            &cellInitData ,
                                            CLSYS_DEFREND_SUB,
                                            pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellRadar, TRUE );
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   すれ違いOBJの実行
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearCrossObjMain(C_GEAR_WORK* pWork)
{
  const GAMEBEACON_INFO *beacon_info;
  int i;
  if(!pWork->cellCross[0]){
    return ;
  }

  for(i=0;i < _CLACT_CROSS_MAX ;i++)
  {
    GFL_CLWK* cp_wk =  pWork->cellCross[i];
    GXRgb dest_buf;
    beacon_info = GAMEBEACON_Get_BeaconLog(i);
    if(beacon_info != NULL){
      //      if(pWork->crossColor[i] != 0){
      u8 col;
      GAMEBEACON_Get_FavoriteColor(&dest_buf, beacon_info);
      col = dest_buf; //pWork->crossColor[i] - 1;
//      OS_TPrintf("色\n");
      GFL_CLACT_WK_SetDrawEnable( cp_wk, TRUE );
      if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  col){
        GFL_CLACT_WK_SetDrawEnable( cp_wk, FALSE );
        GFL_CLACT_WK_SetAnmIndex( cp_wk,col);
        GFL_CLACT_WK_SetDrawEnable( cp_wk, TRUE );
      }
      //    }
    }
    else{
      GFL_CLACT_WK_SetAnmIndex( cp_wk, 16);
//      GFL_CLACT_WK_SetDrawEnable( cp_wk, FALSE );
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   すれ違い用のカラーセット
 * @param   color お気に入りの色
 * @param   index 光らせる場所index
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_SetCrossColor(C_GEAR_WORK* pWork,int color,int index)
{
  GF_ASSERT(index < _CLACT_CROSS_MAX);
  pWork->crossColor[index] = color + 1;
}

//------------------------------------------------------------------------------
/**
 * @brief   すれ違い用のカラーリセット
 * @param   index 光らせる場所index
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_ResetCrossColor(C_GEAR_WORK* pWork,int index)
{
  GF_ASSERT(index < _CLACT_CROSS_MAX);
  pWork->crossColor[index] = 01;
}


//------------------------------------------------------------------------------
/**
 * @brief   すれ違いOBJの開放
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearCrossObjDelete(C_GEAR_WORK* pWork)
{
  int i;

  for(i=0;i < _CLACT_CROSS_MAX ;i++) {
    if(pWork->cellCross[i]){
      GFL_CLACT_WK_Remove(pWork->cellCross[i]);
      pWork->cellCross[i] = NULL;
    }
  }
  if(pWork->cellRadar){
    GFL_CLACT_WK_Remove(  pWork->cellRadar);
    pWork->cellRadar=NULL;
  }

}



//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
//#define _NUKI_FONT_PALNO  (13)

static void _modeInit(C_GEAR_WORK* pWork,BOOL bBoot)
{
  MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );
  u32 bgno = MyStatus_GetMySex(pMy);

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 56+_CLACT_TIMEPARTS_MAX+STARTUP_ANIME_OBJ_MAX, 0 , pWork->heapID );
  _gearArcCreate(pWork, bgno);  //ARC読み込み BG&OBJ
  _gearObjResCreate(pWork);
  if(bBoot){
    _gearBootInitScreen(pWork);
  }
  else{
    _gearPanelBgCreate(pWork);	// パネル作成
  }
  _gearObjCreate(pWork); //CLACT設定
  _gearCrossObjCreate(pWork);
  _gearMarkObjDrawEnable(pWork,TRUE);
  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );

}






static void _arcGearRelease(C_GEAR_WORK* pWork)
{

  _gearCrossObjDelete(pWork);

  if(pWork->cellMove){
    GFL_CLACT_WK_Remove( pWork->cellMove );
    pWork->cellMove=NULL;
  }
  {
    int i;
    for(i = 0;i < _CLACT_TIMEPARTS_MAX; i++){
      if(pWork->cellCursor[i]){
        GFL_CLACT_WK_Remove( pWork->cellCursor[i] );
        pWork->cellCursor[i] = NULL;
      }
    }
    for(i = 0;i < _CLACT_TYPE_MAX; i++){
      if( pWork->cellType[i]){
        GFL_CLACT_WK_Remove( pWork->cellType[i] );
        pWork->cellType[i] = NULL;
      }
    }
    for(i = 0;i < C_GEAR_PANEL_WIDTH * C_GEAR_PANEL_HEIGHT; i++){
      if(pWork->cellSelect[i]){
        GFL_CLACT_WK_Remove( pWork->cellSelect[i] );
        pWork->cellSelect[i] = NULL;
      }
    }
    for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++){
      GFL_CLACT_WK_Remove( pWork->cellStartUp[i] );
      pWork->cellStartUp[i] =NULL;
    }
  }
  GFL_CLGRP_CELLANIM_Release( pWork->objRes[_CLACT_ANM] );
  GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR] );
  GFL_CLGRP_PLTT_Release( pWork->objRes[_CLACT_PLT] );


  GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));


}


static void _workEnd(C_GEAR_WORK* pWork)
{
  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
  }
  _arcGearRelease(pWork);

  GFL_CLACT_UNIT_Delete( pWork->cellUnit );


  GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
  GFL_BG_FreeBGControl(GEAR_BMPWIN_FRAME);
  GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);


  GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_BMPWIN_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

}


//------------------------------------------------------------------------------
/**
 * @brief   時間アニメーション
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _timeAnimation(C_GEAR_WORK* pWork)
{
  RTCTime time;

  GFL_RTC_GetTime( &time );

  {  //AMPM
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_ampm];
    int num = time.hour / 12;
    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {  //時10
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO2];
    int num = (time.hour % 12) / 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
    if(num==0){
      GFL_CLACT_WK_SetDrawEnable(cp_wk,FALSE);
    }
    else{
      GFL_CLACT_WK_SetDrawEnable(cp_wk,TRUE);
    }
  }
  {  //時1
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO10a];
    int num = (time.hour % 12) % 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }

  {  //ころん
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_colon];
    int num = time.second % 2;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }


  {//秒１０
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO6];
    int num = time.minute / 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {//秒１
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_NO10b];
    int num = time.minute % 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {
    GFL_CLWK* cp_wk = pWork->cellCursor[NANR_c_gear_obj_CellAnime_batt1];
    if( OS_IsRunOnTwl() ){//DSIなら
      if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  GFL_UI_GetBattLevel()){
        GFL_CLACT_WK_SetAnmIndex(cp_wk, GFL_UI_GetBattLevel());
      }
    }
    else{
      int num = (GFL_UI_GetBattLevel() == GFL_UI_BATTLEVEL_HI ? 1 : 0);
      if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
        GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
      }
    }
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   タイプのアニメーション 位置調整
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _typeAnimation(C_GEAR_WORK* pWork)
{
  int i;

  for(i=0;i < _CLACT_TYPE_MAX ;i++)
  {
    int x,y;
    GFL_CLACTPOS pos;
    _gearGetTypeBestPosition(pWork, CGEAR_PANELTYPE_IR+i, &x, &y);
    x *= 8;
    y *= 8;
    //		GFL_CLACT_WK_GetPos( pWork->cellType[i], &pos , CLSYS_DEFREND_SUB);
    //		if((pos.x != x) || (pos.y != y)){
    pos.x = x+24-6-2;  // OBJ表示の為の補正値
    pos.y = y+6+6+3;
    GFL_CLACT_WK_SetPos(pWork->cellType[i], &pos, CLSYS_DEFREND_SUB);
    //		}
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機　（通常ループ）
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait(C_GEAR_WORK* pWork)
{
  if( pWork->state_seq == 0 ){
    pWork->state_seq = 1;
    _PanelPaletteColorSetUp( pWork );
  }else{
    _PanelPaletteUpdate( pWork ); 
  }
  
  GFL_BMN_Main( pWork->pButton );
  _timeAnimation(pWork);
  _PanelPaletteChange(pWork);

  


#if 0
  ///インフォメーションメッセージ構造体(キューを取得する時、この構造体に変換して取得する)
  typedef struct{
    STRBUF *name[INFO_WORDSET_MAX];     ///<未使用の場合はNULLが入っています
    u8 wordset_no[INFO_WORDSET_MAX];
    u16 message_id;
    u8 padding[2];
  }GAME_COMM_INFO_MESSAGE;
#endif

}
//------------------------------------------------------------------------------
/**
 * @brief   スリープ復帰　画面　待機１
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait2(C_GEAR_WORK* pWork)
{
  if(_gearBootMain(pWork) == FALSE){
    if(pWork->pCall){
      pWork->pCall(pWork->pWork);
    }
    pWork->pCall=NULL;
    pWork->pWork=NULL;
    return;
  }

  // 通常待機へ遷移
  _CHANGE_STATE(pWork, _modeSelectMenuWait);
}


//------------------------------------------------------------------------------
/**
 * @brief   初期起動　画面　待機
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait0(C_GEAR_WORK* pWork)
{

  switch(pWork->state_seq){
  case STARTUP_SEQ_ANIME_START:
    
    // 何もないテーブルを書き込み
    _gearBootInitScreen( pWork ); // 1回表示するため。

    // 全OAM　非表示
    _gearAllObjDrawEnabel( pWork, FALSE );

    pWork->state_seq ++;
    break;

  case STARTUP_SEQ_WIPE_IN:
    // ワイプ復帰
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

    // 起動アニメだけ表示
    _gearStartUpObjDrawEnabel( pWork, TRUE );
    _gearStartStartUpObjAnime( pWork );
    pWork->state_seq ++;
    break;

  // アニメ待ち
  case STARTUP_SEQ_ANIME_WAIT:
    if( _gearIsEndStartUpObjAnime( pWork ) == FALSE ){
      break;
    }

    pWork->startCounter = 0;
    pWork->state_seq ++;
    break;

  // ALPHAアニメウエイト
  case STARTUP_SEQ_OAM_ALPHA_WAIT:

    if( pWork->startCounter < STARTUP_OAM_ALPHA_ANIME_FRAME_MAX ){
      s8 alpha;

      pWork->startCounter ++;
      alpha = (pWork->startCounter * 16) / STARTUP_OAM_ALPHA_ANIME_FRAME_MAX;
      G2S_SetBlendAlpha( 0, 
          GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3,
          16 - alpha, 16 );
      break;
    }
    
    _gearStartUpObjDrawEnabel( pWork, FALSE );
    // OAMブラックアウト
    _PFadeToBlack(pWork);

    pWork->state_seq++;
    pWork->startCounter = 0;
    break;

  // テーブルが順に出てくるアニメ
  case STARTUP_SEQ_TBL_IN_WAIT:
    if( _gearStartUpMain( pWork ) == TRUE ){

      pWork->startCounter = 0;
      pWork->state_seq++;
    }
    break;

  // ボタンテーブルのALPHA消し
  case STARTUP_SEQ_TBL_ALPHA_WAIT:
    pWork->startCounter++;

    // ALPHAで、消していく。
    {
      s8 alpha;

      pWork->startCounter ++;
      alpha = (pWork->startCounter * 16) / STARTUP_TBL_ALPHA_TIME_WAIT;
      G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG0, 
          GX_BLEND_PLANEMASK_BG1|GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG3,
          16 - alpha, 16 );
    }
    
    if( pWork->startCounter == STARTUP_TBL_ALPHA_TIME_WAIT ){
      // 最終フレームで全消し
      _gearBootInitScreen( pWork );
      pWork->startCounter = 0;
      pWork->state_seq++;
    }
    break;

  case STARTUP_SEQ_END:

    pWork->startCounter ++;
    if( pWork->startCounter < STARTUP_END_TIME_WAIT ){
      break;
    }
    
    //ALPHAリセット
    _setUpSubAlpha( pWork );

    _gearAllObjDrawEnabel( pWork, TRUE );
    _gearStartUpObjDrawEnabel( pWork, FALSE );
    _gearMarkObjDrawEnable(pWork,FALSE);
    _CHANGE_STATE(pWork, _modeSelectMenuWait2);
    break;

  default:
    break;
  }
  
}

//------------------------------------------------------------------------------
/**
 * @brief   スリープ復帰　タイムウエイト
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait1(C_GEAR_WORK* pWork)
{
  if(pWork->startCounter==0){
    _PFadeToBlack(pWork);
    _gearMarkObjDrawEnable(pWork,FALSE);

    // 枠部分を表示
    _gearBootInitScreen( pWork ); // 1回表示するため。

  }else if(pWork->startCounter==1){
    WIPE_SYS_Start( WIPE_PATTERN_S , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  }

  pWork->startCounter++;
  if( pWork->startCounter >= _SLEEP_START_TIME_WAIT ){
    pWork->startCounter = 0;
    _CHANGE_STATE(pWork, _modeSelectMenuWait2);
  }
}




static BOOL _loadExData(C_GEAR_WORK* pWork,GAMESYS_WORK* pGameSys)
{
  BOOL ret=FALSE;

  {
    int i;
    u8* pCGearWork = GFL_HEAP_AllocMemory(HEAPID_FIELD_SUBSCREEN,SAVESIZE_EXTRA_CGEAR_PICTURE);
    SAVE_CONTROL_WORK* pSave = GAMEDATA_GetSaveControlWork(GAMESYSTEM_GetGameData(pGameSys));


    if(LOAD_RESULT_OK== SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE, HEAPID_FIELD_SUBSCREEN,
                                                   pCGearWork,SAVESIZE_EXTRA_CGEAR_PICTURE)){
      CGEAR_PICTURE_SAVEDATA* pPic = (CGEAR_PICTURE_SAVEDATA*)pCGearWork;

      {
        u16 crc = GFL_STD_CrcCalc( pPic, CGEAR_PICTURTE_CHAR_SIZE+CGEAR_PICTURTE_PAL_SIZE+CGEAR_PICTURTE_SCR_SIZE );
        u16 crc2 = CGEAR_SV_GetCGearPictureCRC(CGEAR_SV_GetCGearSaveData(pSave));
        if(crc == crc2){
          if(CGEAR_PICTURE_SAVE_IsPalette(pPic)){
            ret = TRUE;
            for(i=0;i<CGEAR_DECAL_SIZEY;i++){
              GFL_BG_LoadCharacter(GEAR_MAIN_FRAME,&pCGearWork[CGEAR_DECAL_SIZEX * 32 * i],CGEAR_DECAL_SIZEX * 32, (5 + i)*32);
            }
            GFL_BG_LoadPalette(GEAR_MAIN_FRAME,pPic->palette,CGEAR_PICTURTE_PAL_SIZE, 32*0x0a );
          }
          GFL_BG_LoadScreen(GEAR_MAIN_FRAME,pPic->scr,CGEAR_PICTURTE_SCR_SIZE, 0 );
          GFL_BG_LoadScreenReq(GEAR_MAIN_FRAME);
        }
#if PM_DEBUG
        else{
          OS_TPrintf("CRCDIFF chk%x pict%x\n",crc,crc2);
        }
#endif
      }
    }
    SaveControl_Extra_UnloadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE);
    GFL_HEAP_FreeMemory(pCGearWork);
  }
  return ret;
}



///< スリープ起動時に呼ばれる関数
static void _SLEEPGO_FUNC(void* pWk)
{
  C_GEAR_WORK* pWork = pWk;

  WIPE_SetBrightness(WIPE_DISP_SUB,WIPE_FADE_BLACK);
}

///< スリープ復帰時に呼ばれる関数
static void _SLEEPRELEASE_FUNC(void* pWk)
{
  C_GEAR_WORK* pWork = pWk;

  pWork->GetOpenTrg=TRUE;
}

//-------------------------------------
/// VBlank関数
//=====================================
static void _VBlankFunc( GFL_TCB* tcb, void* wk )
{
  C_GEAR_WORK* pWork = (C_GEAR_WORK*)wk;

  // 白く飛ばす演出のためのパレットフェード
  if( pWork->pfade_ptr ) PaletteFadeTrans( pWork->pfade_ptr );
}

//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys )
{
  C_GEAR_WORK *pWork = NULL;
  BOOL ret = FALSE;

  //GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_CGEAR, 0x8000 );

  // OS_TPrintf("zzzz start field_heap = %x\n", GFL_HEAP_GetHeapFreeSize(HEAPID_FIELD_SUBSCREEN));

  pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELD_SUBSCREEN, sizeof( C_GEAR_WORK ) );
  pWork->heapID = HEAPID_FIELD_SUBSCREEN;
  pWork->pCGSV = pCGSV;
  pWork->subscreen = pSub;
  pWork->pGameSys = pGameSys;
  pWork->bAction = TRUE;

  pWork->handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR, HEAPID_FIELD_SUBSCREEN );

  pWork->pfade_tcbsys_wk = GFL_HEAP_AllocClearMemory( HEAPID_FIELD_SUBSCREEN, GFL_TCB_CalcSystemWorkSize(1) );
  pWork->pfade_tcbsys = GFL_TCB_Init( 1, pWork->pfade_tcbsys_wk );

  //	GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT_SUB, 16, 0, _BRIGHTNESS_SYNC);
  _CHANGE_STATE(pWork,_modeSelectMenuWait);

  _createSubBg(pWork);   //BGVRAM設定
  _modeInit(pWork, FALSE);
  if(CGEAR_SV_GetCGearPictureONOFF(pWork->pCGSV)){
    ret = _loadExData(pWork,pGameSys);  //デカール読み込み
  }
  _gearDecalScreenArcCreate(pWork,ret);

  pWork->pfade_ptr = PaletteFadeInit( HEAPID_FIELD_SUBSCREEN );
  PaletteTrans_AutoSet( pWork->pfade_ptr, TRUE );
  PaletteFadeWorkAllocSet( pWork->pfade_ptr, FADE_SUB_OBJ, 0x20*10, HEAPID_FIELD_SUBSCREEN );
  // 現在VRAMにあるパレットを壊さないように、VRAMからパレット内容をコピーする
  PaletteWorkSet_VramCopy( pWork->pfade_ptr, FADE_SUB_OBJ, 0, 0x20*10 );

  {
    pWork->vblank_tcb = GFUser_VIntr_CreateTCB( _VBlankFunc, pWork, 1 );
  }


  GFL_UI_SleepGoSetFunc(&_SLEEPGO_FUNC,  pWork);
  GFL_UI_SleepReleaseSetFunc(&_SLEEPRELEASE_FUNC,  pWork);

  // パネルアニメのシステムクリア
  _PanelMarkAnimeSysInit( pWork );


  //  _PFadeToBlack(pWork);
  //  OS_TPrintf("zzzz start field_heap = %x\n", GFL_HEAP_GetHeapFreeSize(HEAPID_FIELD_SUBSCREEN));

  return pWork;
}




//------------------------------------------------------------------------------
/**
 * @brief   起動画面スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_FirstInit( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, STARTUP_ENDCALLBACK* pCall,void* pWork2 )
{
  C_GEAR_WORK* pWork = CGEAR_Init( pCGSV, pSub, pGameSys);
  
  _CHANGE_STATE(pWork,_modeSelectMenuWait0);
  
  pWork->pCall = pCall;
  pWork->pWork = pWork2;

  // 初期配置情報を設定
  CGEAR_PATTERN_SetUp( pCGSV, pWork->handle, GFUser_GetPublicRand(CGEAR_PATTERN_MAX), 
      HEAPID_FIELD_SUBSCREEN );

  // タイプの表示をリセット
  _typeAnimation(pWork);
    
  return pWork;
}



//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------




void CGEAR_Main( C_GEAR_WORK* pWork,BOOL bAction )
{
  if(pWork->bAction != bAction){
    pWork->bAction = bAction;
  }
  pWork->bAction = bAction;


  if(!pWork->bAction){
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );
    FSND_StopTVTRingTone( fsnd );
  }

  if(GFL_NET_IsInit())
  {
    // トランシーバー反応処理
    if( !pWork->beacon_bit ){
      u32 bit = WIH_DWC_GetAllBeaconTypeBit();

      // トランシーバー再生処理
      if(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR){ // トランシーバー
        if(pWork->bAction){
          FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );
          OS_TPrintf("よびだし\n");
          FSND_RequestTVTRingTone( fsnd);
        }
      }
      // トランシーバー停止処理
      if(!(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR)){
        FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );
        FSND_StopTVTRingTone( fsnd );
      }
    }
  }
  if( pWork->GetOpenTrg ){
    pWork->GetOpenTrg=FALSE;
    _CHANGE_STATE(pWork,_modeSelectMenuWait1);
  }
  {
    StateFunc* state = pWork->state;
    if(state != NULL){
      state(pWork);
    }
  }
  _gearCrossObjMain(pWork);
  if( pWork->pfade_tcbsys ) GFL_TCB_Main( pWork->pfade_tcbsys );


  // パネルアニメのシステムメイン
  _PanelMarkAnimeSysMain( pWork );
}


//------------------------------------------------------------------------------
/**
 * @brief   CGEAR_ActionCallback
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_ActionCallback( C_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno)
{
}



//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_Exit( C_GEAR_WORK* pWork )
{
  GFL_UI_SleepGoSetFunc(NULL,  NULL);
  GFL_UI_SleepReleaseSetFunc(NULL,  NULL);

  GFL_NET_ChangeIconPosition(GFL_WICON_POSX,GFL_WICON_POSY);
  GFL_NET_ReloadIcon();

  // パレットフェード
  PaletteFadeWorkAllocFree( pWork->pfade_ptr, FADE_SUB_OBJ );
  PaletteFadeFree( pWork->pfade_ptr );
  // タスク
  GFL_TCB_Exit( pWork->pfade_tcbsys );
  GFL_HEAP_FreeMemory( pWork->pfade_tcbsys_wk );
  GFL_TCB_DeleteTask( pWork->vblank_tcb );
  {
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );
    FSND_StopTVTRingTone( fsnd );
  }

  GFL_ARC_CloseDataHandle( pWork->handle );

  _workEnd(pWork);
  G2S_BlendNone();
  GFL_HEAP_FreeMemory(pWork);

}


//------------------------------------------------------------------------------
/**
 * @brief   EventCheck
 * @retval  none
 */
//------------------------------------------------------------------------------

GMEVENT* CGEAR_EventCheck(C_GEAR_WORK* pWork, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* pSub )
{
  GMEVENT* event=NULL;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(pWork->pGameSys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(pWork->pGameSys);

  if(bEvReqOK == FALSE || fieldWork == NULL){
    pWork->createEvent=FIELD_SUBSCREEN_ACTION_NONE;
    return NULL;
  }
  switch(pWork->createEvent){
  case FIELD_SUBSCREEN_ACTION_GSYNC:
    PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
    _CHANGE_STATE(pWork,_modeSelectAnimInit);
    event = EVENT_GSync(pWork->pGameSys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_IRC:
    PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
    _CHANGE_STATE(pWork,_modeSelectAnimInit);
    event = EVENT_IrcBattle(pWork->pGameSys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_WIRELESS:
    PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
    _CHANGE_STATE(pWork,_modeSelectAnimInit);
    event = EVENT_CG_Wireless(pWork->pGameSys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_SCANRADAR:
    PMSND_PlaySystemSE( SEQ_SE_MSCL_07 );
    event = EVENT_ResearchRadar( pWork->pGameSys, fieldWork );
    break;
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW:
    PMSND_PlaySystemSE( SEQ_SE_MSCL_07 );
    event = EVENT_ChangeSubScreen(pWork->pGameSys, fieldWork, FIELD_SUBSCREEN_BEACON_VIEW);
    break;
  case FIELD_SUBSCREEN_ACTION_CGEAR_HELP:
    PMSND_PlaySystemSE( SEQ_SE_MSCL_07 );
    {
      CG_HELP_INIT_WORK *initWork = GFL_HEAP_AllocClearMemory( HEAPID_PROC,sizeof(CG_HELP_INIT_WORK) );
      initWork->myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(pWork->pGameSys));
      event = EVENT_FieldSubProc_Callback(pWork->pGameSys, fieldWork, FS_OVERLAY_ID(cg_help),&CGearHelp_ProcData,initWork,NULL,initWork);
    }
    break;
  }
  return event;
}


//----------------------------------------------------------------------------
/**
 *	@brief  パネルタイプを設定  （設定した場所をICONの場所にする。）
 *
 *	@param	pSV
 *	@param	x
 *	@param	y
 *	@param	type 
 */
//-----------------------------------------------------------------------------
static void _modeSetSavePanelType( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type )
{
  int i, j;

  for( i=0; i<C_GEAR_PANEL_HEIGHT; i++ ){
    for( j=0; j<C_GEAR_PANEL_WIDTH; j++ ){
      if( CGEAR_SV_GetPanelType( pSV, j, i ) == type ){
        if( CGEAR_SV_IsPanelTypeIcon( pSV, j, i ) ){
          // OFFにする。
          CGEAR_SV_SetPanelType( pSV, j, i, type );
        }
      }
    }
  }

  // ONにする。
  CGEAR_SV_SetPanelType( pSV, x, y, CGEAR_PANELTYPE_GET_ICON_TYPE(type) );

  // タイプの表示をリセット
  _typeAnimation(pWork);
}



