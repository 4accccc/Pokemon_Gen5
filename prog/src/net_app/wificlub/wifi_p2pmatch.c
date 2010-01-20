//============================================================================================
/**
 * @file  wifi_p2pmatch.c
 * @bfief WIFIP2Pマッチング画面
 * @author  k.ohno
 * @date  06.04.07
 */
//============================================================================================

#include <gflib.h>
#include <dwc.h>
#include "arc_def.h"
#include "message.naix"
#include "msg/msg_wifi_lobby.h"

#include "net/network_define.h"
#include "net/dwc_rap.h"
#include "net/dwc_rapfriend.h"
#include "net_app/wificlub/wifi_p2pmatch.h"
#include "wifi_p2pmatch_local.h"
#include "wifi_p2pmatch_se.h"
#include "wifi_p2pmatchroom.h"


#include "poke_tool/regulation_def.h"

#include "savedata/wifihistory.h"
#include "savedata/config.h"
#include "savedata/battle_box_save.h"

#include "system/bmp_menuwork.h"
#include "system/bmp_winframe.h"
#include "system/bmp_menulist.h"
#include "system/bmp_menu.h"
#include "system/main.h"
#include "system/wipe.h"
#include "system/rtc_tool.h"

#include "gamesystem/msgspeed.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/pokeparty.h"
#include "item/itemsym.h"



#include "font/font.naix"
#include "print/str_tool.h"

#include "wifip2pmatch.naix"      // グラフィックアーカイブ定義
#include "comm_command_wfp2pmf.h"  //２Ｄフィールド
#include "comm_command_wfp2pmf_func.h"  //２Ｄフィールド
#include "msg/msg_wifi_lobby.h"
#include "msg/msg_wifi_system.h"

#include "field/fldmmdl_pl_code.h" //クリーチャー
#include "sound/pm_sndsys.h"  //SOUND関連


// 置き換える必要があるがまだない関数  @@OO
#define SND_HANDLE_FIELD (0)
#define BGM_POKECEN_VOL (0)
#define SND_SCENE_P2P (0)
#define SEQ_WIFILOBBY (0)
#define SND_SCENE_DUMMY (0)
#define BGM_WIFILOBBY_VOL (0)
#define SEQ_PC_01 (0)
#define SEQ_PC_02 (0)
#define FLAG_ARRIVE_D32R0101 (0)
#define GMDATA_ID_EMAIL (0)
#define ID_PARA_monsno (0)
#define ID_PARA_item (0)
#define NUMBER_DISPTYPE_ZERO (STR_NUM_DISP_ZERO)
#define NUMBER_CODETYPE_DEFAULT (0)
#define _SE_CURSOR (0)
#define _SE_INOUT  (0)
#define _SE_DESIDE (0)
#define _SE_OFFER (0)
#define _SE_TBLCHANGE (0)
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
#define BGM_FADE_VCHAT_TIME (0)
#define BGM_WIFILOBBY_VOL (0)
#define BGM_FADEIN_START_VOL_NOW (0)
#define _BATTLE_TOWER_REC (0)   // バトルタワーの記録表示を封印

#define _OAMBUTTON (0)  //下画面のボタンは要らない
#define MYSTATUS_GETTRAINERVIEW_COMPLETION (0)   //ユニオンの姿が数字で入らないとうまく出ない

typedef struct{
  int a;
} ZUKAN_WORK;


static void Snd_PlayerSetInitialVolume(int a,int b){}
static void* TimeWaitIconAdd(GFL_BMPWIN* a,int cgx){ return NULL; }
static void TimeWaitIconTaskDel(void* c){}
static void Snd_DataSetByScene( int a, int b, int c ){}
static void Snd_SceneSet( int a ){}
static int Snd_NowBgmNoGet(void){ return 0;}
static void Snd_PlayerSetInitialVolumeBySeqNo( int a,  int b){}
static void* SaveData_GetEventWork(void* a){ return NULL; }
static BOOL SysFlag_ArriveGet(void* a,int b){ return TRUE;}
static void* SaveData_Get(void* a, int b){ return NULL; }
static void* SaveData_GetFrontier(void* a){ return NULL; }
static void EMAILSAVE_Init(void* a){}
static ZUKAN_WORK* SaveData_GetZukanWork(SAVE_CONTROL_WORK* a){ return NULL; }
static int PokeParaGet( POKEMON_PARAM* poke, int no, void* c ){return 0;}
static BOOL ZukanWork_GetZenkokuZukanFlag(ZUKAN_WORK* pZukan){ return TRUE; }
static void CommInfoFinalize(void){}
static void Snd_SePlay(int a){}
static void Snd_BgmFadeOut( int a, int b){}
static void Snd_BgmFadeIn( int a, int b, int c){}
static void FONTOAM_OAMDATA_Delete( void* x){}



#define COMM_BRIGHTNESS_SYNC  ( 1 ) // 基本の輝度変更Sync数
#define _BMPMENULIST_FONTSIZE   (16)

// WIFI2DMAPシステムオーバーレイ
//FS_EXTERN_OVERLAY(wifi2dmap);

// WIFI　対戦AUTOﾓｰﾄﾞデバック
#ifdef _WIFI_DEBUG_TUUSHIN
extern WIFI_DEBUG_BATTLE_WK WIFI_DEBUG_BATTLE_Work;
#endif  //_WIFI_DEBUG_TUUSHIN


//-------------------------------------
/// マッチング用拡張ヒープサイズ
//=====================================
#define WIFI_P2PMATCH_EXHEAP_SIZE ( 0x10000 )




/// ＢＧパレット定義 2005/09/15
//
//  0 ～ 5  : フィールドマップＢＧ用
//  6       : 天候
//  7       : 地名ウインドウ
//  8       : 看板ウインドウ（マップ、標識など）
//  9       : 看板ウインドウ（枠、フォント）
//  10      : メッセージウインドウ
//  11      : メニューウインドウ
//  12      : メッセージフォント
//  13      : システムフォント
//  14    : 未使用（ローカライズ用）
//  15    : デバッグ用（製品版では未使用）
#define FLD_WEATHER_PAL      (  6 )     //  天候
#define FLD_PLACENAME_PAL    (  7 )         //  地名ウインドウ
#define FLD_BOARD1FRAME_PAL  (  8 )         //  看板ウインドウ（マップ、標識など）
#define FLD_BOARD2FRAME_PAL  (  9 )         //  看板ウインドウ（枠、フォント）
#define FLD_MESFRAME_PAL     ( 10 )         //  メッセージウインドウ
#define FLD_MENUFRAME_PAL    ( 11 )         //  メニューウインドウ
#define FLD_MESFONT_PAL      ( 12 )         //  メッセージフォント
#define FLD_SYSFONT_PAL      ( 13 )         //  システムフォント
#define FLD_LOCALIZE_PAL     ( 14 )         //  未使用（ローカライズ用）
#define FLD_DEBUG_PAL        ( 15 )         //  デバッグ用（製品版では未使用）

/*********************************************************************************************
  メイン画面のCGX割り振り   2006/01/12

    ウィンドウ枠  ：  409 - 511
      会話、メニュー、地名、看板

    BMPウィンドウ１ ：  297 - 408
      会話（最大）、看板、残りボール数

    BMPウィンドウ２ ：  55 - 296
      メニュー（最大）、はい/いいえ、地名

 *********************************************************************************************/

/*********************************************************************************************
  ウィンドウ枠
 *********************************************************************************************/
// 会話ウィンドウキャラ
#define TALK_WIN_CGX_SIZE ( 18+12 )
#define TALK_WIN_CGX_NUM  ( 512 - TALK_WIN_CGX_SIZE )
#define TALK_WIN_PAL    ( 10 )

// メニューウィンドウキャラ
#define MENU_WIN_CGX_SIZE ( 9 )
#define MENU_WIN_CGX_NUM  ( TALK_WIN_CGX_NUM - MENU_WIN_CGX_SIZE )
#define MENU_WIN_PAL    ( 11 )

// 地名ウィンドウキャラ
#define PLACE_WIN_CGX_SIZE  ( 10 )
#define PLACE_WIN_CGX_NUM ( MENU_WIN_CGX_NUM - PLACE_WIN_CGX_SIZE )
#define PLACE_WIN_PAL   ( 7 )

// 看板ウィンドウキャラ
#define BOARD_WIN_CGX_SIZE  ( 18+12 + 24 )
#define BOARD_WIN_CGX_NUM ( PLACE_WIN_CGX_NUM - BOARD_WIN_CGX_SIZE )
#define BOARD_WIN_PAL   ( FLD_BOARD2FRAME_PAL )

/*********************************************************************************************
  BMPウィンドウ
 *********************************************************************************************/
// 会話ウィンドウ（メイン）
#define FLD_MSG_WIN_PX    ( 2 )
#define FLD_MSG_WIN_PY    ( 19 )
#define FLD_MSG_WIN_SX    ( 27 )
#define FLD_MSG_WIN_SY    ( 4 )
#define FLD_MSG_WIN_PAL   ( FLD_MESFONT_PAL )
#define FLD_MSG_WIN_CGX   ( BOARD_WIN_CGX_NUM - ( FLD_MSG_WIN_SX * FLD_MSG_WIN_SY ) )

// 看板ウィンドウ（メイン）（会話と同じ位置（会話より小さい））
#define FLD_BOARD_WIN_PX  ( 9 )
#define FLD_BOARD_WIN_PY  ( 19 )
#define FLD_BOARD_WIN_SX  ( 20 )
#define FLD_BOARD_WIN_SY  ( 4 )
#define FLD_BOARD_WIN_PAL ( FLD_BOARD2FRAME_PAL )
#define FLD_BOARD_WIN_CGX ( FLD_MSG_WIN_CGX )

// メニューウィンドウ（メイン）
#define FLD_MENU_WIN_PX   ( 20 )
#define FLD_MENU_WIN_PY   ( 1 )
#define FLD_MENU_WIN_SX   ( 11 )
#define FLD_MENU_WIN_SY   ( 22 )
#define FLD_MENU_WIN_PAL  ( FLD_SYSFONT_PAL )
#define FLD_MENU_WIN_CGX  ( FLD_MSG_WIN_CGX - ( FLD_MENU_WIN_SX * FLD_MENU_WIN_SY ) )

// はい/いいえウィンドウ（メイン）（メニューと同じ位置（メニューより小さい））
#define FLD_YESNO_WIN_PX  ( 24 )
#define FLD_YESNO_WIN_PY  ( 13 )
#define FLD_YESNO_WIN_SX  ( 6 )
#define FLD_YESNO_WIN_SY  ( 4 )
#define FLD_YESNO_WIN_PAL ( FLD_SYSFONT_PAL )
#define FLD_YESNO_WIN_CGX ( FLD_MSG_WIN_CGX - ( FLD_YESNO_WIN_SX * FLD_YESNO_WIN_SY ) )

// 地名ウィンドウ（メニューと同じ位置（メニューより小さい））
#define FLD_PLACE_WIN_PX  ( 0 )
#define FLD_PLACE_WIN_PY  ( 0 )
#define FLD_PLACE_WIN_SX  ( 32 )
#define FLD_PLACE_WIN_SY  ( 3 )
#define FLD_PLACE_WIN_CGX ( FLD_MSG_WIN_CGX - ( FLD_PLACE_WIN_SX * FLD_PLACE_WIN_SY ) )



// メッセージウィンドウ（サブ）
#define FLD_MSG_WIN_S_PX  ( 2  )
#define FLD_MSG_WIN_S_PY  ( 19 )
#define FLD_MSG_WIN_S_PAL ( FLD_MESFONT_PAL )
#define FLD_MSG_WIN_S_CGX ( MENU_WIN_CGX_NUM - ( FLD_MSG_WIN_SX * FLD_MSG_WIN_SY ) )

// 看板ウィンドウ（サブ）
#define FLD_BOARD_WIN_S_PX  ( 9 )
#define FLD_BOARD_WIN_S_PY  ( 19 )
#define FLD_BOARD_WIN_S_SX  ( 21 )
#define FLD_BOARD_WIN_S_SY  ( 4 )
#define FLD_BOARD_WIN_S_PAL ( FLD_BOARD2FRAME_PAL )
#define FLD_BOARD_WIN_S_CGX ( FLD_MSG_WIN_S_CGX )

// メニューウィンドウ（サブ）
#define FLD_MENU_WIN_S_PX ( 25 )
#define FLD_MENU_WIN_S_PY ( 1 )
#define FLD_MENU_WIN_S_SX ( 6 )
#define FLD_MENU_WIN_S_SY ( 16 )
#define FLD_MENU_WIN_S_PAL  ( FLD_SYSFONT_PAL )
#define FLD_MENU_WIN_S_CGX  ( MENU_WIN_CGX_NUM - ( FLD_MENU_WIN_SX * FLD_MENU_WIN_SY ) )



/// ビットマップ転送関数用定義
//------------------------------------------------------------------
#define FBMP_TRANS_OFF  (0)
#define FBMP_TRANS_ON (1)
/// 文字表示転送関数用定義
//------------------------------------------------------------------
#define FBMPMSG_WAITON_SKIPOFF  (0)
#define FBMPMSG_WAITON_SKIPON (1)
#define FBMPMSG_WAITOFF     (2)

//-------------------------------------------------------------------------
/// 文字表示色定義(default) -> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col) (((col)<<4)|(col))

//-------------------------------------------------------------------------
/// 文字表示スピード定義(default)
//------------------------------------------------------------------
#define FBMP_MSG_SPEED_SLOW   (8)
#define FBMP_MSG_SPEED_NORMAL (4)
#define FBMP_MSG_SPEED_FAST   (1)



//============================================================================================
//  定数定義
//============================================================================================
enum {
  SEQ_IN = 0,
  SEQ_MAIN,
  SEQ_OUT,
};

enum {
  _MENU_LIST,
  _MENU_INPUT,
  _MENU_MYCODE,
  _MENU_EXIT,
};

typedef enum {
  _CLACT_UP_CUR,
  _CLACT_DOWN_CUR,
  _CLACT_LINE_CUR,
  _CLACT_VCT_STOP,
  _CLACT_VCT_MOVE,
} _OAM_ANIM_E;

typedef struct{  // スクリーン用RECT構造体
  u8 lt_x;
  u8 lt_y;
  u8 rb_x;
  u8 rb_y;
} _SCR_RECT;

#define WF_CLACT_WKNUM  ( 8 )
#define WF_CLACT_RESNUM ( 1 )
#define WF_FONTOAM_NUM  ( 1 )

#define _PRINTTASK_MAX (8)
#define _TIMING_GAME_CHECK  (13)// つながった直後
#define _TIMING_GAME_CHECK2  (14)// つながった直後
#define _TIMING_GAME_START  (15)// タイミングをそろえる
#define _TIMING_GAME_START2  (18)// タイミングをそろえる
#define _TIMING_BATTLE_END  (16)// タイミングをそろえる
#define _TIMING_POKEPARTY_END  (17)// タイミングをそろえる

#define _RECONECTING_WAIT_TIME (20)  //再接続時間


// ユーザー表示面の設定
#define WIFIP2PMATCH_PLAYER_DISP_X  ( 1 )
#define WIFIP2PMATCH_PLAYER_DISP_Y  ( 1 )
#define WIFIP2PMATCH_PLAYER_DISP_SIZX ( 28 )
#define WIFIP2PMATCH_PLAYER_DISP_SIZY ( 2 )
#define WIFIP2PMATCH_PLAYER_DISP_NAME_X ( 32 )
#define WIFIP2PMATCH_PLAYER_DISP_ST_X ( 102 )
#define WIFIP2PMATCH_PLAYER_DISP_WINSIZ (WIFIP2PMATCH_PLAYER_DISP_SIZX*WIFIP2PMATCH_PLAYER_DISP_SIZY)


//FRAME3
#define _CGX_BOTTOM ( 512 )             // 会話ウィンドウキャラ
#define _CGX_TITLE_BOTTOM (_CGX_BOTTOM - 18*3)  // タイトルウィンドウ
#define _CGX_USER_BOTTOM  (_CGX_TITLE_BOTTOM - WIFIP2PMATCH_PLAYER_DISP_WINSIZ) // ユーザーウィンドウ

// FRAME1に転送するユーザーデータ背景
#define _CGX_USET_BACK_BOTTOM ( MENU_WIN_CGX_NUM - WIFIP2PMATCH_PLAYER_DISP_WINSIZ )

// FRAME1に転送するICONデータ
#define PLAYER_DISP_ICON_PLTTOFS  (8)
#define PLAYER_DISP_ICON_PLTTOFS_SUB (13)
#define PLAYER_DISP_ICON_PLTTNUM  (2)
#define PLAYER_DISP_ICON_CG_SIZX  (12)
#define PLAYER_DISP_ICON_CG_SIZY  (4)
#define PLAYER_DISP_ICON_CGX    ( 0 )
#define PLAYER_DISP_ICON_SCRN_X ( 2 )
#define PLAYER_DISP_ICON_SCRN_Y ( 2 )
#define PLAYER_DISP_ICON_POS_X  ( 2 )
#define PLAYER_DISP_ICON_POS_Y  ( 1 )
#define PLAYER_DISP_VCTICON_POS_X ( 26 )
#define PLAYER_DISP_VCTICON_POS_Y ( 1 )
enum{
  PLAYER_DISP_ICON_IDX_NONE00,
  PLAYER_DISP_ICON_IDX_VCTNOT,
  PLAYER_DISP_ICON_IDX_VCTBIG,
  PLAYER_DISP_ICON_IDX_VCT,
  PLAYER_DISP_ICON_IDX_NORMAL,
  PLAYER_DISP_ICON_IDX_UNK,
  PLAYER_DISP_ICON_IDX_FIGHT,
  PLAYER_DISP_ICON_IDX_CHANGE,
  PLAYER_DISP_ICON_IDX_NONE,
  PLAYER_DISP_ICON_IDX_MINIGAME,
  PLAYER_DISP_ICON_IDX_FRONTIER,
  PLAYER_DISP_ICON_IDX_NUM,
};


// 会話ウィンドウ FRAME2
#define COMM_MESFRAME_PAL     ( 10 )         //  メッセージウインドウ
#define COMM_MENUFRAME_PAL    ( 11 )         //  メニューウインドウ
#define COMM_MESFONT_PAL      ( 12 )         //  メッセージフォント
#define COMM_SYSFONT_PAL    ( 13 )         //  システムフォント
#define COMM_TALK_WIN_CGX_SIZE  ( 18+12 )
//#define COMM_TALK_WIN_CGX_NUM ( 512 - COMM_TALK_WIN_CGX_SIZE)
#define COMM_TALK_WIN_CGX_NUM ( 48 )

#define COMM_MSG_WIN_PX   ( 2 )
#define COMM_MSG_WIN_PY   ( 19 )
#define COMM_MSG_WIN_SX   ( 27 )
#define COMM_MSG_WIN_SY   ( 4 )
#define COMM_MSG_WIN_PAL    ( COMM_MESFONT_PAL )
//#define COMM_MSG_WIN_CGX    ( (COMM_TALK_WIN_CGX_NUM - 73) - ( COMM_MSG_WIN_SX * COMM_MSG_WIN_SY ) )

#define COMM_SYS_WIN_PX   ( 4 )
#define COMM_SYS_WIN_PY   ( 4 )
#define COMM_SYS_WIN_SX   ( 23 )
#define COMM_SYS_WIN_SY   ( 16 )
#define COMM_SYS_WIN_PAL    ( COMM_MESFONT_PAL )
#define COMM_SYS_WIN_CGX    ( MENU_WIN_CGX_NUM - ( COMM_SYS_WIN_SX * COMM_SYS_WIN_SY ) )  // 通信システムウィンドウ転送先


#define REGWINDOW_WIN_PX   ( 3 )
#define REGWINDOW_WIN_PY   ( 5 )
#define REGWINDOW_WIN_SX   ( 25 )
#define REGWINDOW_WIN_SY   ( 16 )


// YesNoWin
#define YESNO_WIN_CGX ((COMM_SYS_WIN_CGX - (FLD_YESNO_WIN_SX*FLD_YESNO_WIN_SY)))
#define FRAME1_YESNO_WIN_CGX  ((_CGX_USET_BACK_BOTTOM - (FLD_YESNO_WIN_SX*FLD_YESNO_WIN_SY)))


// メインメッセージキャラ
#define _NUKI_FONT_PALNO  (13)
#define _COL_N_BLACK  ( GFL_FONTSYS_SetColor( 1, 2, 0 ) )   // フォントカラー：黒
#define _COL_N_WHITE  ( GFL_FONTSYS_SetColor( 15, 14, 0 ) )   // フォントカラー：白
#define _COL_N_RED      ( GFL_FONTSYS_SetColor( 3, 4, 0 ) )   // フォントカラー：青
#define _COL_N_BLUE     ( GFL_FONTSYS_SetColor( 5, 6, 0 ) )   // フォントカラー：赤
#define _COL_N_GRAY   ( GFL_FONTSYS_SetColor( 2, 14, 0 ) )    // フォントカラー：灰

// 名前表示BMP（上画面）
#define WIFIP2PMATCH_NAME_BMP_W  ( 16 )
#define WIFIP2PMATCH_NAME_BMP_H  (  2 )
#define WIFIP2PMATCH_NAME_BMP_SIZE (WIFIP2PMATCH_NAME_BMP_W * WIFIP2PMATCH_NAME_BMP_H)

// 会話ウインドウ表示位置定義
#define WIFIP2PMATCH_TALK_X   (  2 )
#define WIFIP2PMATCH_TALK_Y   (  19 )

#define WIFIP2PMATCH_TITLE_X    (   3  )
#define WIFIP2PMATCH_TITLE_Y    (   1  )
#define WIFIP2PMATCH_TITLE_W    (  26  )
#define WIFIP2PMATCH_TITLE_H    (   2  )


#define WIFIP2PMATCH_MSG_WIN_OFFSET     (1 + TALK_WIN_CGX_SIZ     + MENU_WIN_CGX_SIZ)
#define WIFIP2PMATCH_TITLE_WIN_OFFSET   ( WIFIP2PMATCH_MSG_WIN_OFFSET   + FLD_MSG_WIN_SX*FLD_MSG_WIN_SY )
#define WIFIP2PMATCH_NAME_WIN_OFFSET    ( WIFIP2PMATCH_TITLE_WIN_OFFSET + WIFIP2PMATCH_TITLE_W*WIFIP2PMATCH_TITLE_H )
#define WIFIP2PMATCH_YESNO_WIN_OFFSET   ( WIFIP2PMATCH_NAME_WIN_OFFSET  + WIFIP2PMATCH_NAME_BMP_SIZE*5 )


// VIEW面
enum{
  MCV_BTTN_FRIEND_TYPE_NONE,  // 設定していない
  MCV_BTTN_FRIEND_TYPE_RES, // 予約
  MCV_BTTN_FRIEND_TYPE_IN,    // 登録済み
  MCV_BTTN_FRIEND_TYPE_MAX, // タイプ最大値
};
#define MCV_PAL_BACK    ( 0 ) // 背景パレットの開始位置
#define MCV_PAL_FRMNO   ( 0 ) // 背景のバットパレット開始位置
#define MCV_PAL_BTTN    ( 4 ) // ボタンパレットの開始位置
enum{
  MCV_PAL_BACK_0 = 0,
  MCV_PAL_BACK_1,
  MCV_PAL_BACK_2,
  MCV_PAL_BACK_3,
  MCV_PAL_BACK_NUM,

  MCV_PAL_BTTN_GIRL = 0,
  MCV_PAL_BTTN_MAN,
  MCV_PAL_BTTN_NONE,
  MCV_PAL_BTTNST_GIRL,
  MCV_PAL_BTTNST_MAN,
  MCV_PAL_BTTN_NUM  // 今のところ余りは
    // MCV_PAL_BTTN+MCV_PAL_BTTN_NUM～(PLAYER_DISP_ICON_PLTTOFS_SUB-1まで
};
// アイコンの転送位置
#define MCV_ICON_CGX  (0)
#define MCV_ICON_CGSIZ  (48)
#define MCV_ICON_PAL    (PLAYER_DISP_ICON_PLTTOFS_SUB)

#define MCV_CGX_BTTN2 (MCV_ICON_CGX+MCV_ICON_CGSIZ) // FRAME2ユーザーデータ
#define MCV_CGX_BACK  (0)// FRAME0背景
#define MCV_SYSFONT_PAL ( 15 )  // システムフォント
// WIN設定
#define MCV_NAMEWIN_CGX   ( 1 ) // 名前ウィンドウ開始位置
#define MCV_NAMEWIN_DEFX  ( 4 ) // 基本位置
#define MCV_NAMEWIN_DEFY  ( 1 )
#define MCV_NAMEWIN_OFSX  ( 16 )  // 位置の移動値
#define MCV_NAMEWIN_OFSY  ( 6 )
#define MCV_NAMEWIN_SIZX  ( 9 ) // サイズ
#define MCV_NAMEWIN_SIZY  ( 3 )
#define MCV_NAMEWIN_CGSIZ ( MCV_NAMEWIN_SIZX*MCV_NAMEWIN_SIZY ) // CGサイズ
#define MCV_NAMEWIN_CGALLSIZ  ((MCV_NAMEWIN_CGSIZ*WCR_MAPDATA_1BLOCKOBJNUM)+MCV_NAMEWIN_CGX)// CGそうサイズ

#define MCV_STATUSWIN_CGX ( MCV_NAMEWIN_CGALLSIZ+1 )
#define MCV_STATUSWIN_DEFX  ( 1 ) // 基本位置
#define MCV_STATUSWIN_DEFY  ( 1 )
#define MCV_STATUSWIN_VCHATX ( 12 ) // VCHAT位置
#define MCV_STATUSWIN_OFSX  ( 16 )  // 位置の移動値
#define MCV_STATUSWIN_OFSY  ( 6 )
#define MCV_STATUSWIN_SIZX  ( 2 ) // サイズ
#define MCV_STATUSWIN_SIZY  ( 3 )
#define MCV_STATUSWIN_CGSIZ ( MCV_STATUSWIN_SIZX*MCV_STATUSWIN_SIZY ) // CGサイズ

#define MCV_USERWIN_CGX   ( 1 )
#define MCV_USERWIN_X   ( 1 )
#define MCV_USERWIN_Y   ( 1 )
#define MCV_USERWIN_SIZX  ( 30 )
#define MCV_USERWIN_SIZY  ( 21 )


#define MCV_BUTTON_SIZX ( 16 )
#define MCV_BUTTON_SIZY ( 6 )
#define MCV_BUTTON_DEFX (0)
#define MCV_BUTTON_DEFY (0)
#define MCV_BUTTON_OFSX (16)
#define MCV_BUTTON_OFSY (6)

#define MCV_BUTTON_ICON_OFS_X ( 1 )
#define MCV_BUTTON_VCTICON_OFS_X  ( 13 )
#define MCV_BUTTON_ICON_OFS_Y ( 1 )

#define MCV_BUTTON_FRAME_NUM  (4) // ボタンアニメフレーム数

enum{
  MCV_BUTTON_TYPE_GIRL,
  MCV_BUTTON_TYPE_MAN,
  MCV_BUTTON_TYPE_NONE,
};

enum{
  MCV_USERDISP_OFF, // ユーザー表示OFF
  MCV_USERDISP_INIT,  // ユーザー表示初期化
  MCV_USERDISP_ON,  // ユーザー表示ON
  MCV_USERDISP_ONEX,  // ユーザー表示強制ON
};

#if 0
// PAGE2
#define MCV_USERD_BTTW_TITLE_X    (8)
#define MCV_USERD_BTTW_TITLE_Y    (0)
#define MCV_USERD_BTTW_LAST_X   (8)
#define MCV_USERD_BTTW_LAST_Y   (24)
#define MCV_USERD_BTTW_LASTNUM_X  (64)
#define MCV_USERD_BTTW_LASTNUM_Y  (24)
#define MCV_USERD_BTTW_MAX_X    (8)
#define MCV_USERD_BTTW_MAX_Y    (48)
#define MCV_USERD_BTTW_MAXNUM_X   (64)
#define MCV_USERD_BTTW_MAXNUM_Y   (48)

// PAGE3
#define MCV_USERD_BTFC_TITLE_X      (8)
#define MCV_USERD_BTFC_TITLE_Y      (0)
#define MCV_USERD_BTFC_LV50_X     (8)
#define MCV_USERD_BTFC_LV50_Y     (24)
#define MCV_USERD_BTFC_LV50K_X      (136)
#define MCV_USERD_BTFC_LV50K_Y      (24)
#define MCV_USERD_BTFC_LV50LAST_X   (8)
#define MCV_USERD_BTFC_LV50LAST_Y   (48)
#define MCV_USERD_BTFC_LV50LASTNUM_X  (64)
#define MCV_USERD_BTFC_LV50LASTNUM_Y  (48)
#define MCV_USERD_BTFC_LV50LASTTRNUM_X  (184)
#define MCV_USERD_BTFC_LV50LASTTRNUM_Y  (48)
#define MCV_USERD_BTFC_LV50MAX_X    (8)
#define MCV_USERD_BTFC_LV50MAX_Y    (64)
#define MCV_USERD_BTFC_LV50MAXNUM_X   (64)
#define MCV_USERD_BTFC_LV50MAXNUM_Y   (64)
#define MCV_USERD_BTFC_LV50MAXTRNUM_X (184)
#define MCV_USERD_BTFC_LV50MAXTRNUM_Y (64)
#define MCV_USERD_BTFC_OPN_X      (8)
#define MCV_USERD_BTFC_OPN_Y      (88)
#define MCV_USERD_BTFC_OPNK_X     (136)
#define MCV_USERD_BTFC_OPNK_Y     (88)
#define MCV_USERD_BTFC_OPNLAST_X    (8)
#define MCV_USERD_BTFC_OPNLAST_Y    (112)
#define MCV_USERD_BTFC_OPNLASTNUM_X (64)
#define MCV_USERD_BTFC_OPNLASTNUM_Y (112)
#define MCV_USERD_BTFC_OPNLASTTRNUM_X (184)
#define MCV_USERD_BTFC_OPNLASTTRNUM_Y (112)
#define MCV_USERD_BTFC_OPNMAX_X   (8)
#define MCV_USERD_BTFC_OPNMAX_Y   (128)
#define MCV_USERD_BTFC_OPNMAXNUM_X    (64)
#define MCV_USERD_BTFC_OPNMAXNUM_Y    (128)
#define MCV_USERD_BTFC_OPNMAXTRNUM_X  (184)
#define MCV_USERD_BTFC_OPNMAXTRNUM_Y  (128)


// PAGE4
#define MCV_USERD_BTKS_TITLE_X    (8)
#define MCV_USERD_BTKS_TITLE_Y    (0)
#define MCV_USERD_BTKS_K_X      (136)
#define MCV_USERD_BTKS_K_Y      (24)
#define MCV_USERD_BTKS_LAST_X   (8)
#define MCV_USERD_BTKS_LAST_Y   (48)
#define MCV_USERD_BTKS_LASTNUM_X  (64)
#define MCV_USERD_BTKS_LASTNUM_Y  (48)
#define MCV_USERD_BTKS_LASTCPNUM_X  (184)
#define MCV_USERD_BTKS_LASTCPNUM_Y  (48)
#define MCV_USERD_BTKS_MAX_X    (8)
#define MCV_USERD_BTKS_MAX_Y    (80)
#define MCV_USERD_BTKS_MAXNUM_X   (64)
#define MCV_USERD_BTKS_MAXNUM_Y   (80)
#define MCV_USERD_BTKS_MAXCPNUM_X (184)
#define MCV_USERD_BTKS_MAXCPNUM_Y (80)

// PAGE5
#define MCV_USERD_BTST_TITLE_X    (8)
#define MCV_USERD_BTST_TITLE_Y    (0)
#define MCV_USERD_BTST_K_X      (8)
#define MCV_USERD_BTST_K_Y      (24)
#define MCV_USERD_BTST_LAST_X   (8)
#define MCV_USERD_BTST_LAST_Y   (48)
#define MCV_USERD_BTST_LASTNUM_X  (64)
#define MCV_USERD_BTST_LASTNUM_Y  (48)
#define MCV_USERD_BTST_MAX_X    (8)
#define MCV_USERD_BTST_MAX_Y    (80)
#define MCV_USERD_BTST_MAXNUM_X   (64)
#define MCV_USERD_BTST_MAXNUM_Y   (80)

// PAGE6
#define MCV_USERD_BTRT_TITLE_X    (8)
#define MCV_USERD_BTRT_TITLE_Y    (0)
#define MCV_USERD_BTRT_LAST_X   (8)
#define MCV_USERD_BTRT_LAST_Y   (24)
#define MCV_USERD_BTRT_LASTNUM_X  (64)
#define MCV_USERD_BTRT_LASTNUM_Y  (24)
#define MCV_USERD_BTRT_MAX_X    (8)
#define MCV_USERD_BTRT_MAX_Y    (48)
#define MCV_USERD_BTRT_MAXNUM_X   (64)
#define MCV_USERD_BTRT_MAXNUM_Y   (48)

// PAGE7
#define MCV_USERD_MINI_TITLE_X    (8)
#define MCV_USERD_MINI_TITLE_Y    (0)
#define MCV_USERD_MINI_BC_X     (8)   // たまいれ
#define MCV_USERD_MINI_BC_Y     (24)
#define MCV_USERD_MINI_BCNUM_X    (136) // たまいれ　かず
#define MCV_USERD_MINI_BCNUM_Y    (24)
#define MCV_USERD_MINI_BB_X     (8)   // たまのり
#define MCV_USERD_MINI_BB_Y     (48)
#define MCV_USERD_MINI_BBNUM_X    (136) // たまのり　かず
#define MCV_USERD_MINI_BBNUM_Y    (48)
#define MCV_USERD_MINI_BL_X     (8)   // ふうせん
#define MCV_USERD_MINI_BL_Y     (72)
#define MCV_USERD_MINI_BLNUM_X    (136) // ふうせん　かず
#define MCV_USERD_MINI_BLNUM_Y    (72)

// フロンティア非表示スクリーン描画
#define MCV_USERD_NOFR_SCRN_X   ( 0x1a )
#define MCV_USERD_NOFR_SCRN_Y   ( 0 )
#define MCV_USERD_NOFR_SCRN_SIZX  ( 0x1 )
#define MCV_USERD_NOFR_SCRN_SIZY  ( 0x1 )
#endif
/*
//  フロンティアタイプ
enum{
  MCV_FRONTIOR_TOWOR,
  MCV_FRONTIOR_FACTORY,
  MCV_FRONTIOR_FACTORY100,
  MCV_FRONTIOR_CASTLE,
  MCV_FRONTIOR_STAGE,
  MCV_FRONTIOR_ROULETTE,
  MCV_FRONTIOR_NUM,
} ;
*/
#define VRANTRANSFERMAN_NUM (32)  // VramTransferManagerタスク数


enum{
  MCV_USERD_BTTN_ANM_LEFT_PUSH = 0,
  MCV_USERD_BTTN_ANM_LEFT_RELEASE,
  MCV_USERD_BTTN_ANM_BACK_PUSH,
  MCV_USERD_BTTN_ANM_BACK_RELEASE,
  MCV_USERD_BTTN_ANM_RIGHT_PUSH,
  MCV_USERD_BTTN_ANM_RIGHT_RELEASE,
};

enum{
  MCV_USERD_BTTN_MODE_WAIT,
  MCV_USERD_BTTN_MODE_NML,
  MCV_USERD_BTTN_MODE_NOBACK,
};

#define MCV_USERD_BTTN_RESCONTID  ( 30 )  // リソースID
#define MCV_USERD_BTTN_BGPRI    (0)   // BG優先順位
#define MCV_USERD_BTTN_PRI      (128)   // BG優先順位
#define MCV_USERD_BTTN_Y      (172) // ボタンY位置
#define MCV_USERD_BTTN_LEFT_X   (40)  // ←
#define MCV_USERD_BTTN_BACK_X   (128) // もどる
#define MCV_USERD_BTTN_RIGHT_X    (224) // →
#define MCV_USERD_BTTN_FONT_X   ( -18 ) // もどる　文字位置
#define MCV_USERD_BTTN_FONT_Y   ( -8 )  // もどる　文字位置
#define MCV_USERD_BTTN_FONT_SIZX  ( 8 ) // もどる　文字描画範囲
#define MCV_USERD_BTTN_FONT_SIZY  ( 2 ) // もどる　文字描画範囲
#define MCV_USERD_BTTN_FONT_CGXOFS  ( 0 ) // CGXオフセット
#define MCV_USERD_BTTN_FONT_COL   ( 0 ) // カラーパレット
#define MCV_USERD_BTTN_FONT_OAMPRI  ( 0 )
#define MCV_USERD_BTTN_LEFT_SIZX  ( 56 )  // ←サイズ
#define MCV_USERD_BTTN_BACK_SIZX  ( 96 )  // もどるサイズ
#define MCV_USERD_BTTN_RIGHT_SIZX ( 56 )  // →さいず
#define MCV_USERD_BTTN_SIZY     ( 32 )  // 縦さいず
#define MCV_USERD_BTTN_LEFT_HIT_X ( 8 ) // ←あたり判定用X
#define MCV_USERD_BTTN_BACK_HIT_X ( 80 )  // ←あたり判定用X
#define MCV_USERD_BTTN_RIGHT_HIT_X  ( 192 ) // ←あたり判定用X
#define MCV_USERD_BTTN_RIGHT_HIT_Y  ( 160 ) // ←あたり判定用Y
#define MCV_USERD_BTTN_ANMPUSHOK  ( 3 ) // 押したと判断するボタンのアニメ
#define MCV_USERD_BTTN_ANMMAX   ( 5 ) // アニメ内フレーム数
#define MCV_USERD_BTTN_ANMMAX_0ORG  ( MCV_USERD_BTTN_ANMMAX-1 ) // アニメ内フレーム数

// FONTOAMY座標アニメデータ
static const s8 c_MCV_USER_BTTN_FONT_YANM[ MCV_USERD_BTTN_ANMMAX ] = {
  MCV_USERD_BTTN_FONT_Y,
  MCV_USERD_BTTN_FONT_Y - 1,
  MCV_USERD_BTTN_FONT_Y - 2,
  MCV_USERD_BTTN_FONT_Y - 2,
  MCV_USERD_BTTN_FONT_Y - 1,
};

// ボタンシーケンス　PUSHアニメ
static const u8 c_MCV_USER_BTTN_ANM_PUSH[ MCV_USERD_BTTN_NUM ] = {
  MCV_USERD_BTTN_ANM_LEFT_PUSH,
  MCV_USERD_BTTN_ANM_BACK_PUSH,
  MCV_USERD_BTTN_ANM_RIGHT_PUSH,
};

// ボタンシーケンス　RELEASEアニメ
static const u8 c_MCV_USER_BTTN_ANM_RELEASE[ MCV_USERD_BTTN_NUM ] = {
  MCV_USERD_BTTN_ANM_LEFT_RELEASE,
  MCV_USERD_BTTN_ANM_BACK_RELEASE,
  MCV_USERD_BTTN_ANM_RIGHT_RELEASE,
};

static const GFL_UI_TP_HITTBL c_MCV_USER_BTTN_OAM_Hit[ ] = {
  { // ←
    MCV_USERD_BTTN_RIGHT_HIT_Y, MCV_USERD_BTTN_RIGHT_HIT_Y+MCV_USERD_BTTN_SIZY,
    MCV_USERD_BTTN_LEFT_HIT_X, MCV_USERD_BTTN_LEFT_HIT_X+MCV_USERD_BTTN_LEFT_SIZX,
  },
  { // もどる
    MCV_USERD_BTTN_RIGHT_HIT_Y, MCV_USERD_BTTN_RIGHT_HIT_Y+MCV_USERD_BTTN_SIZY,
    MCV_USERD_BTTN_BACK_HIT_X, MCV_USERD_BTTN_BACK_HIT_X+MCV_USERD_BTTN_BACK_SIZX,
  },
  { // →
    MCV_USERD_BTTN_RIGHT_HIT_Y, MCV_USERD_BTTN_RIGHT_HIT_Y+MCV_USERD_BTTN_SIZY,
    MCV_USERD_BTTN_RIGHT_HIT_X, MCV_USERD_BTTN_RIGHT_HIT_X+MCV_USERD_BTTN_RIGHT_SIZX,
  },
  {GFL_UI_TP_HIT_END,0,0,0},     //終了データ
};
enum{
  MCV_USERD_BTTN_RET_NONE,  // 何の反応もなし
  MCV_USERD_BTTN_RET_LEFT,  // 左がおされた
  MCV_USERD_BTTN_RET_BACK,  // もどるがおされた
  MCV_USERD_BTTN_RET_RIGHT, // 右がおされた
};


static GFL_DISP_VRAM _defVBTbl = {
  GX_VRAM_BG_128_A,       // メイン2DエンジンのBG
  GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット

  GX_VRAM_SUB_BG_128_C,     // サブ2DエンジンのBG
  GX_VRAM_SUB_BGEXTPLTT_NONE,   // サブ2DエンジンのBG拡張パレット

  GX_VRAM_OBJ_128_B,        // メイン2DエンジンのOBJ
  GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット

  GX_VRAM_SUB_OBJ_128_D,     // サブ2DエンジンのOBJ
  GX_VRAM_SUB_OBJEXTPLTT_NONE,  // サブ2DエンジンのOBJ拡張パレット

  GX_VRAM_TEX_NONE,       // テクスチャイメージスロット
  GX_VRAM_TEXPLTT_NONE,     // テクスチャパレットスロット

  GX_OBJVRAMMODE_CHAR_1D_128K,
  GX_OBJVRAMMODE_CHAR_1D_32K,

};

#ifdef WFP2P_DEBUG  // 人をいっぱい出す
//#define WFP2PM_MANY_OBJ
#endif


static void _debugChangeState(WIFIP2PMATCH_WORK* wk, int state, int line)
{
  NET_PRINT("p2p: %d\n",line);
  wk->seq = state;
}

#if 1
#define   _CHANGESTATE(wk,state) _debugChangeState(wk,state,__LINE__)
#else  //GFL_NET_DEBUG
#define   _CHANGESTATE(wk,state)  wk->seq = state
#endif //GFL_NET_DEBUG

//static int _seqBackup;

//============================================================================================
//  プロトタイプ宣言
//============================================================================================
static WIFI_STATUS* WifiFriendMatchStatusGet( u32 idx );
static u8 WifiDwc_getFriendStatus( int idx );

static BOOL WifiP2PMatch_CommWifiBattleStart( WIFIP2PMATCH_WORK* wk, int friendno );

/*** 関数プロトタイプ ***/
static void VBlankFunc( GFL_TCB *tcb , void * work );
static void VramBankSet(void);
static void BgInit( HEAPID heapID );
static void InitMessageWork( WIFIP2PMATCH_WORK *wk );
static void FreeMessageWork( WIFIP2PMATCH_WORK *wk );
static void BgExit( WIFIP2PMATCH_WORK *wk );
static void BgGraphicSet( WIFIP2PMATCH_WORK * wk, ARCHANDLE* p_handle );
static void char_pltt_manager_init(void);
static void InitCellActor(WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle);
static void SetCellActor(WIFIP2PMATCH_WORK *wk);
static void BmpWinInit(WIFIP2PMATCH_WORK *wk, GFL_PROC* proc);
static void MainMenuMsgInit(WIFIP2PMATCH_WORK *wk);
static void SetCursor_Pos( GFL_CLWK* act, int x, int y );
static void WindowSet(void);

// マッチングルーム管理用関数
static void MCRSYS_SetMoveObjWork( WIFIP2PMATCH_WORK* wk, MCR_MOVEOBJ* p_obj );
static void MCRSYS_DelMoveObjWork( WIFIP2PMATCH_WORK* wk, MCR_MOVEOBJ* p_obj );
static void MCRSYS_SetFriendObj( WIFIP2PMATCH_WORK* wk, u32 heapID );
static int MCRSYS_ContFiendInOut( WIFIP2PMATCH_WORK* wk );
static void MCRSYS_ContFriendStatus( WIFIP2PMATCH_WORK* wk, u32 heapID );
static MCR_MOVEOBJ* MCRSYS_GetMoveObjWork( WIFIP2PMATCH_WORK* wk, u32 friendNo );

static void WifiP2PMatchFriendListIconLoad( WIFIP2PMATCH_ICON* p_data, ARCHANDLE* p_handle, u32 heapID );
static void WifiP2PMatchFriendListIconRelease( WIFIP2PMATCH_ICON* p_data );
static void WifiP2PMatchFriendListIconWrite(  WIFIP2PMATCH_ICON* p_data, u32 frm, u8 cx, u8 cy, u32 icon_type, u32 col );
static void WifiP2PMatchFriendListStIconWrite( WIFIP2PMATCH_ICON* p_data, u32 frm, u8 cx, u8 cy, u32 status,u32 gamemode );
static void WifiP2PMatchFriendListBmpIconWrite(  GFL_BMPWIN* p_bmp, WIFIP2PMATCH_ICON* p_data, u16 x, u16 y, u32 icon_type, u32 col,int pal );
static void WifiP2PMatchFriendListBmpStIconWrite( GFL_BMPWIN* p_bmp, WIFIP2PMATCH_ICON* p_data, u16 x, u16 y, u32 status,u32 gamemode,int pal );
static int WifiP2PMatchBglFrmIconPalGet( u32 frm );

static int WifiP2PMatchFriendListStart( WIFIP2PMATCH_WORK* wk );

// 友達データの強制表示処理
static void WifiP2PMatch_UserDispOn( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID );
static void WifiP2PMatch_UserDispOff( WIFIP2PMATCH_WORK *wk, u32 heapID );
static void WifiP2PMatch_UserDispOff_Target( WIFIP2PMATCH_WORK *wk, u32 target_friend, u32 heapID );

static void WifiP2PMatch_UserDispOn_MyAcces( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID );
static void WifiP2PMatch_UserDispOff_MyAcces( WIFIP2PMATCH_WORK *wk, u32 heapID );

// 友達データビューアー
static BOOL MCVSys_MoveCheck( const WIFIP2PMATCH_WORK *wk );
static void MCVSys_Init( WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle, u32 heapID );
static void MCVSys_Exit( WIFIP2PMATCH_WORK *wk );
static u32 MCVSys_Updata( WIFIP2PMATCH_WORK *wk, u32 heapID );
static void MCVSys_UpdataBttn( WIFIP2PMATCH_WORK *wk );
static BOOL MCVSys_UserDispEndCheck( WIFIP2PMATCH_WORK *wk );
static u32  MCVSys_UserDispGetFriend( const WIFIP2PMATCH_WORK* cp_wk );
static void MCVSys_UserDispOn( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID );
static void MCVSys_UserDispOff( WIFIP2PMATCH_WORK *wk );
static void MCVSys_UserDispPageChange( WIFIP2PMATCH_WORK *wk, u32 oambttn_ret );
static void MCVSys_BttnSet( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 type );
static void MCVSys_BttnDel( WIFIP2PMATCH_WORK *wk, u32 friendNo );
static u32 MCVSys_BttnTypeGet( const WIFIP2PMATCH_WORK *wk, u32 friendNo );
static void MCVSys_ReWrite( WIFIP2PMATCH_WORK *wk, u32 heapID );
static void MCVSys_BttnAllWriteReq( WIFIP2PMATCH_WORK *wk );

static void MCVSys_BttnCallBack( u32 bttnid, u32 event, void* p_work );
static void MCVSys_BttnAnmMan( WIFIP2PMATCH_WORK *wk );
static void MCVSys_GraphicSet( WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle, u32 heapID );
static void MCVSys_GraphicDel( WIFIP2PMATCH_WORK *wk );
static void MCVSys_GraphicScrnCGOfsChange( NNSG2dScreenData* p_scrn, u8 cgofs );
static void MCVSys_BackDraw( WIFIP2PMATCH_WORK *wk );
static void MCVSys_BttnDraw( WIFIP2PMATCH_WORK *wk );
static void MCVSys_UserDispDraw( WIFIP2PMATCH_WORK *wk, u32 heapID );
static void MCVSys_UserDispDrawType00( WIFIP2PMATCH_WORK *wk, u32 heapID );
static void MCVSys_UserDispDrawFrontierOffScrn( WIFIP2PMATCH_WORK *wk );
static void MCVSys_UserDispFrontiorNumDraw( WIFIP2PMATCH_WORK *wk, u32 strid, u32 factoryid, u32 friendno, u32 x, u32 y );
static void MCVSys_UserDispFrontiorTitleStrGet( WIFIP2PMATCH_WORK *wk, STRBUF* p_str, u32 factoryid, u32 friendno );
static void MCVSys_UserDispNumDraw( WIFIP2PMATCH_WORK *wk, u32 strid, u32 num, u32 x, u32 y );
static void MCVSys_BttnWrite( WIFIP2PMATCH_WORK *wk, u8 cx, u8 cy, u8 type, u8 frame );
static u32 MCVSys_StatusMsgIdGet( u32 status,u32 gamemode, int* col );
static void MCVSys_BttnWinDraw( WIFIP2PMATCH_WORK *wk, GFL_BMPWIN* p_bmp, u32 friendNo, u32 frame, u32 area_id );
static void MCVSys_BttnStatusWinDraw( WIFIP2PMATCH_WORK *wk, GFL_BMPWIN** p_stbmp, u32 friendNo, u32 frame, u32 area_id );
static void MCVSys_OamBttnOn( WIFIP2PMATCH_WORK *wk );
static void MCVSys_OamBttnOnNoBack( WIFIP2PMATCH_WORK *wk );
static void MCVSys_OamBttnOff( WIFIP2PMATCH_WORK *wk );
static void MCVSys_OamBttnObjAnmStart( WIFIP2PMATCH_VIEW*  p_view, u32 bttn_no );
static BOOL MCVSys_OamBttnObjAnmMain( WIFIP2PMATCH_VIEW*  p_view, u32 bttn_no, u32 push_bttn, u32 event );
static void MCVSys_FriendNameSet( WIFIP2PMATCH_WORK* p_wk, int friendno );

static void BmpWinDelete( WIFIP2PMATCH_WORK *wk );
static void _systemMessagePrint( WIFIP2PMATCH_WORK *wk, int msgno );
//static void WifiP2PMatchMessagePrint( WIFIP2PMATCH_WORK *wk, int msgno , BOOL bSystem);
static void EndMessageWindowOff( WIFIP2PMATCH_WORK *wk );
static void WifiP2PMatchDataSendRecv( WIFIP2PMATCH_WORK *wk );
static void SequenceChange_MesWait( WIFIP2PMATCH_WORK *wk, int next );
static void _myStatusChange(WIFIP2PMATCH_WORK *wk, int status,int gamemode);
static void _myStatusChange_not_send(WIFIP2PMATCH_WORK *wk, int status,int gamemode);
static u32 _WifiMyStatusGet( WIFIP2PMATCH_WORK * p_wk, WIFI_STATUS* p_status );
static u32 _WifiMyGameModeGet( WIFIP2PMATCH_WORK * p_wk, WIFI_STATUS* p_status );

//static void _timeWaitIconDel(WIFIP2PMATCH_WORK *wk);
//static void _friendNameExpand( WIFIP2PMATCH_WORK *wk, int friendNo);

//static BOOL _modeActive(int status);


static void WifiP2P_SetLobbyBgm( void );
static BOOL WifiP2P_CheckLobbyBgm( void );


//static void WifiP2P_Fnote_Set( WIFIP2PMATCH_WORK* wk, u32 idx );

static void _errorDisp(WIFIP2PMATCH_WORK* wk);

static void FriendRequestWaitOn( WIFIP2PMATCH_WORK* wk, BOOL msg_on );
static void FriendRequestWaitOff( WIFIP2PMATCH_WORK* wk );
static BOOL FriendRequestWaitFlagGet( const WIFIP2PMATCH_WORK* cp_wk );


// FuncTableからシーケンス遷移で呼ばれる関数
static int WifiP2PMatch_MainInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _normalConnectYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _normalConnectWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _differMachineInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _differMachineNext( WIFIP2PMATCH_WORK *wk, int seq );
static int _differMachineOneMore( WIFIP2PMATCH_WORK *wk, int seq );
static int _firstYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _retryInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _retryYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _retryWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _retry( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_ReConnectingWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_ConnectingInit( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_Connecting( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_FirstSaving( WIFIP2PMATCH_WORK *wk, int seq );
static int _firstConnectEndMsg( WIFIP2PMATCH_WORK *wk, int seq );
static int _firstConnectEndMsgWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_FriendListInit( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_FriendListInit2( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_FriendList( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnectInit2( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnectInit( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnectWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnect( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnectEndYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTConnectEndWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_VCTDisconnect(WIFIP2PMATCH_WORK *wk, int seq);
static int WifiP2PMatch_BattleDisconnect(WIFIP2PMATCH_WORK *wk, int seq);
static int WifiP2PMatch_Disconnect(WIFIP2PMATCH_WORK *wk, int seq);
static int WifiP2PMatch_BattleConnectInit( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BattleConnectWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BattleConnect( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_MainReturn( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSelectMenuInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSelectMenuWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentRestart( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSubSelectMenuWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSelectRelInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSelectRelYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeSelectRelWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _childModeMatchMenuInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _childModeMatchMenuInit2( WIFIP2PMATCH_WORK *wk, int seq );
static int _childModeMatchMenuWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _childModeMatchMenuLoop( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BCancelYesNoVCT( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BCancelWaitVCT( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BCancelYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_BCancelWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMenuInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMenuYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMenuSend( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMenuCheck( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMyStatusWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _parentModeCallMenuWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _personalDataInit( WIFIP2PMATCH_WORK *wk, int seq );
static int _personalDataWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _personalDataEnd( WIFIP2PMATCH_WORK *wk, int seq );
static int _exitYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _exitWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _exitExiting( WIFIP2PMATCH_WORK *wk, int seq );
static int _exitEnd( WIFIP2PMATCH_WORK *wk, int seq );
static int _nextBattleYesNo( WIFIP2PMATCH_WORK *wk, int seq );
static int _nextBattleWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _vchatNegoCheck( WIFIP2PMATCH_WORK *wk, int seq );
static int _vchatNegoWait( WIFIP2PMATCH_WORK *wk, int seq );
static int _playerDirectInit( WIFIP2PMATCH_WORK *wk, int seq );
static int MessageEndReturnList( WIFIP2PMATCH_WORK *wk, int seq );



static BOOL _myVChatStatusToggle(WIFIP2PMATCH_WORK *wk);
static BOOL _myVChatStatusToggleOrg(WIFIP2PMATCH_WORK *wk);
static BOOL _myVChatStatusOrgSet(WIFIP2PMATCH_WORK *wk);
static int _vchatToggleWait( WIFIP2PMATCH_WORK *wk, int seq );


static int WifiP2PMatch_EndWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_CheckAndEnd( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_CancelEnableWait( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_CancelEnableWaitDP( WIFIP2PMATCH_WORK *wk, int seq );
static int WifiP2PMatch_FirstSaving2( WIFIP2PMATCH_WORK *wk, int seq );
static int _callGameInit( WIFIP2PMATCH_WORK *wk, int seq );



#include "wifi_p2pmatch_message.c"
#include "wifi_p2pmatch_direct.c"
#include "wifi_p2pmatch_machine.c"



static void* _getMyUserData(void* pWork)  //DWCUserData
{
  WIFIP2PMATCH_WORK *wk = (WIFIP2PMATCH_WORK*)pWork;
  return WifiList_GetMyUserInfo(GAMEDATA_GetWiFiList(wk->pGameData));
}

static void* _getFriendData(void* pWork)  //DWCFriendData
{
  WIFIP2PMATCH_WORK *wk = (WIFIP2PMATCH_WORK*)pWork;
  return WifiList_GetDwcDataPtr(GAMEDATA_GetWiFiList(wk->pGameData),0);
}

static void _deleteFriendList(int deletedIndex,int srcIndex, void* pWork)
{
  WIFIP2PMATCH_WORK *wk = (WIFIP2PMATCH_WORK*)pWork;

  WifiList_DataMarge(GAMEDATA_GetWiFiList(wk->pGameData), deletedIndex, srcIndex);
}


// レコードコーナーメインシーケンス用関数配列定義
static int (*FuncTable[])(WIFIP2PMATCH_WORK *wk, int seq)={
  WifiP2PMatch_MainInit,  // WIFIP2PMATCH_MODE_INIT  = 0,
  _retryInit,        //WIFIP2PMATCH_RETRY_INIT
  _retryYesNo,        //WIFIP2PMATCH_RETRY_YESNO
  _retryWait,         //WIFIP2PMATCH_RETRY_WAIT
  _retry,             //WIFIP2PMATCH_RETRY
  WifiP2PMatch_ConnectingInit,        // WIFIP2PMATCH_CONNECTING_INIT,
  WifiP2PMatch_Connecting,            // WIFIP2PMATCH_CONNECTING,
  _firstConnectEndMsg,                 //WIFIP2PMATCH_FIRST_ENDMSG
  _firstConnectEndMsgWait,    //WIFIP2PMATCH_FIRST_ENDMSG_WAIT
  WifiP2PMatch_FriendListInit,        // WIFIP2PMATCH_FRIENDLIST_INIT,
  WifiP2PMatch_FriendListInit2, //WIFIP2PMATCH_FRIENDLIST_INIT2
  WifiP2PMatch_FriendList,            // WIFIP2PMATCH_MODE_FRIENDLIST
  _vchatToggleWait,                   // WIFIP2PMATCH_VCHATWIN_WAIT
  WifiP2PMatch_VCTConnectInit2,        // WIFIP2PMATCH_MODE_VCT_CONNECT_INIT2
  WifiP2PMatch_VCTConnectInit,        // WIFIP2PMATCH_MODE_VCT_CONNECT_INIT
  WifiP2PMatch_VCTConnectWait,        // WIFIP2PMATCH_MODE_VCT_CONNECT_WAIT
  WifiP2PMatch_VCTConnect,        // WIFIP2PMATCH_MODE_VCT_CONNECT
  WifiP2PMatch_VCTConnectEndYesNo,   // WIFIP2PMATCH_MODE_VCT_CONNECTEND_YESNO
  WifiP2PMatch_VCTConnectEndWait,   // WIFIP2PMATCH_MODE_VCT_CONNECTEND_WAIT
  WifiP2PMatch_VCTDisconnect,   // WIFIP2PMATCH_MODE_VCT_DISCONNECT
  WifiP2PMatch_BattleDisconnect,   //WIFIP2PMATCH_MODE_BATTLE_DISCONNECT
  WifiP2PMatch_Disconnect,  //WIFIP2PMATCH_MODE_DISCONNECT
  WifiP2PMatch_BattleConnectInit,     // WIFIP2PMATCH_MODE_BATTLE_CONNECT_INIT
  WifiP2PMatch_BattleConnectWait,     // WIFIP2PMATCH_MODE_BATTLE_CONNECT_WAIT
  WifiP2PMatch_BattleConnect,        // WIFIP2PMATCH_MODE_BATTLE_CONNECT
  WifiP2PMatch_MainReturn,            // WIFIP2PMATCH_MODE_MAIN_MENU
  WifiP2PMatch_EndWait,         // WIFIP2PMATCH_MODE_END_WAIT,
  WifiP2PMatch_CheckAndEnd,          //  WIFIP2PMATCH_MODE_CHECK_AND_END
  _parentModeSelectMenuInit,  // WIFIP2PMATCH_MODE_SELECT_INIT
  _parentModeSelectMenuWait,  // WIFIP2PMATCH_MODE_SELECT_WAIT
  _parentModeSubSelectMenuWait, // WIFIP2PMATCH_MODE_SUBBATTLE_WAIT
  _parentModeSelectRelInit,  //WIFIP2PMATCH_MODE_SELECT_REL_INIT
  _parentModeSelectRelYesNo, //WIFIP2PMATCH_MODE_SELECT_REL_YESNO
  _parentModeSelectRelWait,  //WIFIP2PMATCH_MODE_SELECT_REL_WAIT
  _childModeMatchMenuInit,   //WIFIP2PMATCH_MODE_MATCH_INIT
  _childModeMatchMenuInit2,   //WIFIP2PMATCH_MODE_MATCH_INIT2
  _childModeMatchMenuWait,   //WIFIP2PMATCH_MODE_MATCH_WAIT
  _childModeMatchMenuLoop, //WIFIP2PMATCH_MODE_MATCH_LOOP
  WifiP2PMatch_BCancelYesNo, //WIFIP2PMATCH_MODE_BCANCEL_YESNO
  WifiP2PMatch_BCancelWait,    //WIFIP2PMATCH_MODE_BCANCEL_WAIT
  _parentModeCallMenuInit,//WIFIP2PMATCH_MODE_CALL_INIT
  _parentModeCallMenuYesNo,//WIFIP2PMATCH_MODE_CALL_YESNO
  _parentModeCallMenuSend, //WIFIP2PMATCH_MODE_CALL_SEND
  _parentModeCallMenuCheck, //WIFIP2PMATCH_MODE_CALL_CHECK
  _parentModeCallMyStatusWait, //WIFIP2PMATCH_MODE_MYSTATUS_WAIT   //51
  _parentModeCallMenuWait,//WIFIP2PMATCH_MODE_CALL_WAIT
  _personalDataInit,    //WIFIP2PMATCH_MODE_PERSONAL_INIT
  _personalDataWait,    //WIFIP2PMATCH_MODE_PERSONAL_WAIT
  _personalDataEnd,    //WIFIP2PMATCH_MODE_PERSONAL_END
  _exitYesNo,       //WIFIP2PMATCH_MODE_EXIT_YESNO
  _exitWait,        //WIFIP2PMATCH_MODE_EXIT_WAIT
  _exitExiting,        //WIFIP2PMATCH_MODE_EXITING
  _exitEnd,        //WIFIP2PMATCH_MODE_EXIT_END
  _nextBattleYesNo,  //WIFIP2PMATCH_NEXTBATTLE_YESNO
  _nextBattleWait,  //WIFIP2PMATCH_NEXTBATTLE_WAIT
  _vchatNegoCheck, //WIFIP2PMATCH_MODE_VCHAT_NEGO
  _vchatNegoWait, //WIFIP2PMATCH_MODE_VCHAT_NEGO_WAIT
  WifiP2PMatch_ReConnectingWait, //WIFIP2PMATCH_RECONECTING_WAIT
  WifiP2PMatch_BCancelYesNoVCT, //WIFIP2PMATCH_MODE_BCANCEL_YESNO_VCT
  WifiP2PMatch_BCancelWaitVCT,    //WIFIP2PMATCH_MODE_BCANCEL_WAIT_VCT
  _parentRestart,  //WIFIP2PMATCH_PARENT_RESTART
  WifiP2PMatch_FirstSaving, //WIFIP2PMATCH_FIRST_SAVING
  WifiP2PMatch_CancelEnableWait, //WIFIP2PMATCH_MODE_CANCEL_ENABLE_WAIT
  WifiP2PMatch_FirstSaving2, //WIFIP2PMATCH_FIRST_SAVING2
  _callGameInit,  //WIFIP2PMATCH_MODE_CALLGAME_INIT
  _playerDirectInit, //WIFIP2PMATCH_MODE_PLAYERDIRECT_INIT
  _playerDirectInit1, //  WIFIP2PMATCH_PLAYERDIRECT_INIT1,
  _playerDirectInit2, //  WIFIP2PMATCH_PLAYERDIRECT_INIT2,
  _playerDirectInit3, //  WIFIP2PMATCH_PLAYERDIRECT_INIT3,
  _playerDirectInit5, //  WIFIP2PMATCH_PLAYERDIRECT_INIT5,
  _playerDirectInit6, //  WIFIP2PMATCH_PLAYERDIRECT_INIT6,
  _playerDirectInit7, //  WIFIP2PMATCH_PLAYERDIRECT_INIT7,
  _playerDirectWait, //  WIFIP2PMATCH_PLAYERDIRECT_WAIT,
  _playerDirectEnd, //  WIFIP2PMATCH_PLAYERDIRECT_END,    //80
  _playerDirectVCT, //  WIFIP2PMATCH_PLAYERDIRECT_VCT,
  _playerDirectTVT, //  WIFIP2PMATCH_PLAYERDIRECT_TVT,
  _playerDirectTrade, //  WIFIP2PMATCH_PLAYERDIRECT_TRADE,
  _playerDirectBattle1, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE1,
  _playerDirectSub1, // WIFIP2PMATCH_PLAYERDIRECT_SUB1
  _playerDirectSub2, // WIFIP2PMATCH_PLAYERDIRECT_SUB2
  _playerDirectSubStart, //WIFIP2PMATCH_PLAYERDIRECT_SUBSTART
  _playerDirectBattle2,//WIFIP2PMATCH_PLAYERDIRECT_BATTLE2
  _playerDirectBattleMode,  //WIFIP2PMATCH_PLAYERDIRECT_BATTLE_MODE,
  _playerDirectBattleMode2,  //WIFIP2PMATCH_PLAYERDIRECT_BATTLE_MODE2,
  _playerDirectBattleRule,  // WIFIP2PMATCH_PLAYERDIRECT_BATTLE_RULE,   //ルール
  _playerDirectBattleRule2,  // WIFIP2PMATCH_PLAYERDIRECT_BATTLE_RULE2,   //ルール
  _playerDirectBattleShooter,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_SHOOTER,  //シューター
  _playerDirectBattleShooter2,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_SHOOTER2,  //シューター
  _playerDirectBattleDecide,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_DECIDE,  //決定
  _playerDirectBattleWatch,  // WIFIP2PMATCH_PLAYERDIRECT_BATTLE_WATCH,  //ルールを見る 
  _playerDirectBattleWatch2,  // WIFIP2PMATCH_PLAYERDIRECT_BATTLE_WATCH2,  //ルールを見る 
  _playerDirectBattleGO,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO
  _playerDirectBattleGO1,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO1
  _playerDirectBattleGO2,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO2
  _playerDirectBattleGO3,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO3
  _playerDirectBattleGO4,  //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_GO4
  _playerDirectNoregParent, //WIFIP2PMATCH_PLAYERDIRECT_NOREG_PARENT
  _playerDirectBattleStart, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START
  _playerDirectBattleStart2, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START2
  _playerDirectBattleStart3, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START3
  _playerDirectBattleStart4, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START4
  _playerDirectBattleStart5, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START5
  _playerDirectBattleStart6, //  WIFIP2PMATCH_PLAYERDIRECT_BATTLE_START6
  _playerDirectFailed, //WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED
  _playerDirectFailed2, //WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED2
  _playerDirectFailed3, //WIFIP2PMATCH_PLAYERDIRECT_BATTLE_FAILED3
  MessageEndReturnList, //WIFIP2PMATCH_MESSAGEEND_RETURNLIST
  _playerMachineInit1,//WIFIP2PMATCH_PLAYERMACHINE_INIT1
  _playerMachineNoregParent, //WIFIP2PMATCH_PLAYERMACHINE_NOREG_PARENT
  _playerMachineBattleDecide, //WIFIP2PMATCH_PLAYERMACHINE_BATTLE_DECIDE
};

#define _MAXNUM   (2)         // 最大接続人数
#define _MAXSIZE  (80)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数


static GFLNetInitializeStruct aGFLNetInit = {
  NULL,  // 受信関数テーブル
  0, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  NULL,  // ビーコンデータ取得関数
  NULL,  // ビーコンデータサイズ取得関数
  NULL,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  _deleteFriendList,  ///< wifiフレンドリスト削除コールバック
  _getFriendData,   ///< DWC形式の友達リスト
  _getMyUserData,  ///< DWCのユーザデータ（自分のデータ）
  GFL_NET_DWC_HEAPSIZE,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  _MAXNUM,     // 最大接続人数
  _MAXSIZE,  //最大送信バイト数
  _BCON_GET_NUM,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_WIFICLUB,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD, // 赤外線タイムアウト時間
#endif
  0,//MP親最大サイズ 512まで
  0,//dummy
};



static void _GFL_NET_InitAndStruct(WIFIP2PMATCH_WORK *wk,BOOL bInit)
{

  if( OS_IsRunOnTwl() ){//DSIならVCT用のメモリが大きくとられる為、領域を多くしないといけない
    aGFLNetInit.heapSize = GFL_NET_DWCLOBBY_HEAPSIZE;
  }
  else{
    aGFLNetInit.heapSize = GFL_NET_DWC_HEAPSIZE;
  }
  if(bInit){
    GFL_NET_Init(&aGFLNetInit, NULL, wk);
  }
  else{
    GFL_NET_ChangeInitStruct(&aGFLNetInit);
  }
}

static const u8 ViewButtonFrame_y[ 4 ] = {
  8, 7, 5, 7
  };



// 自分ステータスを取得する
static u32 _WifiMyStatusGet( WIFIP2PMATCH_WORK * p_wk, WIFI_STATUS* p_status )
{
  return WIFI_STATUS_GetWifiStatus(p_status);
}
// 自分ゲームモードを取得する
static u32 _WifiMyGameModeGet( WIFIP2PMATCH_WORK * p_wk, WIFI_STATUS* p_status )
{
  return WIFI_STATUS_GetGameMode(p_status);
}

static BOOL _modeIsBattleStatus(int gamemode)
{
  switch(gamemode){
  case WIFI_GAME_BATTLE_SINGLE_ALL:   // シングルLv50対戦募集中
  case WIFI_GAME_BATTLE_SINGLE_FLAT:      // シングルバトル
  case WIFI_GAME_BATTLE_DOUBLE_ALL:      // ダブル
  case WIFI_GAME_BATTLE_DOUBLE_FLAT:      // ダブル
  case WIFI_GAME_BATTLE_TRIPLE_ALL:      // トリプル
  case WIFI_GAME_BATTLE_TRIPLE_FLAT:      // トリプル
  case WIFI_GAME_BATTLE_ROTATION_ALL:      // ローテーション
  case WIFI_GAME_BATTLE_ROTATION_FLAT:      // ローテーション
    return TRUE;
  }
  return FALSE;
}


// ステートがBATTLEWAITかどうか
static BOOL _modeBattleWait(int status,int gamemode)
{
  if(_modeIsBattleStatus(gamemode)){
    if(status == WIFI_STATUS_RECRUIT){
      return TRUE;
    }
  }
  return FALSE;
}

// ステートがWAITかどうか
static BOOL
_modeWait(int status)
{
  if(status==WIFI_STATUS_RECRUIT){
    return TRUE;
  }
  return FALSE;
  /*
  if(_modeBattleWait(status)){
    return TRUE;
  }
  switch(status){
  case WIFI_STATUS_TRADE_WAIT:    // 交換募集中
  case WIFI_STATUS_TVT_WAIT:    // 交換募集中
    return TRUE;
  }
  return FALSE;
   */
}

// ステートがBATTLEかどうか
static BOOL _modeBattle(int status,int gamemode)
{
  if(_modeIsBattleStatus(gamemode)){
    if(status == WIFI_STATUS_PLAYING){
      return TRUE;
    }
  }
  return FALSE;
}

#if 0
// ステートがACTIVEかどうか
static BOOL _modeActive(int status,int gamemode)
{
  if(status==WIFI_STATUS_PLAYING){
    return TRUE;
  }
  return FALSE;
  /*

  if(_modeBattle(status)){
    return TRUE;
  }
  switch(status){
  case WIFI_STATUS_TRADE:    // 交換中
  case WIFI_STATUS_TVT:    // 交換中
  case WIFI_STATUS_FRONTIER:    // フロンティア中
  case WIFI_STATUS_VCT:    // 交換中
    return TRUE;
  }
  return FALSE;
   */
}
#endif

static BOOL _is2pokeMode(int status,int gamemode)
{
  if(gamemode > WIFI_GAME_LOGIN_WAIT){
    if(gamemode < WIFI_GAME_UNKNOWN){
      return TRUE;
    }
  }
  return FALSE;
}


static int _convertState(int state)
{
  return state;
  /*
  int ret = WIFI_STATUS_UNKNOWN;

  switch(state){
  case WIFI_STATUS_TVT_WAIT:
    ret = WIFI_STATUS_TVT;
    break;
  case WIFI_STATUS_TRADE_WAIT:
    ret = WIFI_STATUS_TRADE;
    break;
  case WIFI_STATUS_SBATTLE50_WAIT:
    ret = WIFI_STATUS_SBATTLE50;
    break;
  case WIFI_STATUS_SBATTLE100_WAIT:
    ret = WIFI_STATUS_SBATTLE100;
    break;
  case WIFI_STATUS_SBATTLE_FREE_WAIT:
    ret = WIFI_STATUS_SBATTLE_FREE;
    break;
  case WIFI_STATUS_DBATTLE50_WAIT:
    ret = WIFI_STATUS_DBATTLE50;
    break;
  case WIFI_STATUS_DBATTLE100_WAIT:
    ret = WIFI_STATUS_DBATTLE100;
    break;
  case WIFI_STATUS_DBATTLE_FREE_WAIT:
    ret = WIFI_STATUS_DBATTLE_FREE;
    break;
  case WIFI_STATUS_LOGIN_WAIT:
    ret = WIFI_STATUS_VCT;
    break;
  case WIFI_STATUS_TVT:
    ret = WIFI_STATUS_TVT_WAIT;
    break;
  case WIFI_STATUS_TRADE:
    ret = WIFI_STATUS_TRADE_WAIT;
    break;
  case WIFI_STATUS_SBATTLE50:
    ret = WIFI_STATUS_SBATTLE50_WAIT;
    break;
  case WIFI_STATUS_SBATTLE100:
    ret = WIFI_STATUS_SBATTLE100_WAIT;
    break;
  case WIFI_STATUS_SBATTLE_FREE:
    ret = WIFI_STATUS_SBATTLE_FREE_WAIT;
    break;
  case WIFI_STATUS_DBATTLE50:
    ret = WIFI_STATUS_DBATTLE50_WAIT;
    break;
  case WIFI_STATUS_DBATTLE100:
    ret = WIFI_STATUS_DBATTLE100_WAIT;
    break;
  case WIFI_STATUS_DBATTLE_FREE:
    ret = WIFI_STATUS_DBATTLE_FREE_WAIT;
    break;
  case WIFI_STATUS_VCT:
    ret = WIFI_STATUS_LOGIN_WAIT;
    break;
  }
  return ret;
   */
}

//--------------------------------------------------------------------------------------------
/**
 * ステータスにあわせて通信ステートも変更する
 * @param status
 * @return  none
 */
//--------------------------------------------------------------------------------------------

static void _commStateChange(WIFIP2PMATCH_WORK * wk,int status,int gamemode)
{
  if(status == WIFI_STATUS_WAIT){
    GFL_NET_SetWifiBothNet(FALSE);
    _GFL_NET_InitAndStruct(wk,FALSE);
  }
  else if(_modeBattle(status, gamemode)){
    GFL_NET_SetWifiBothNet(TRUE); // wifiの通信を非同期から同期に
  }

  switch(status){
  case WIFI_STATUS_RECRUIT:    // 募集中  = 誰でも繋ぐ事が可能
  case WIFI_STATUS_PLAY_AND_RECRUIT:      // 他の人と接続中でさらに募集中
    GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),TRUE);  //接続許可
    break;
  case WIFI_STATUS_CALL:       // 他の人に接続しようとしている
  case WIFI_STATUS_WAIT:   // 待機中
  case WIFI_STATUS_NONE:   // 何も無い	NONEのときは出現もしません
  case WIFI_STATUS_PLAYING:      // 他の人と接続中
  case WIFI_STATUS_UNKNOWN:   // 新たに作ったらこの番号以上になる
  default:
    GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),TRUE);  //接続不可
    break;
  }
  NET_PRINT("status %d gamemode %d\n",status,gamemode);


}

//============================================================================================
//  プロセス関数
//============================================================================================

static void _graphicInit(WIFIP2PMATCH_WORK * wk)
{
  ARCHANDLE* p_handle;

  // sys_VBlankFuncChange( NULL, NULL );  // VBlankセット
  //    sys_HBlankIntrStop(); //HBlank割り込み停止
  // Vblank期間中のBG転送終了
  if(wk->vblankFunc){
    GFL_TCB_DeleteTask( wk->vblankFunc );
  }

  GFL_DISP_GX_SetVisibleControlDirect(0);   //全BG&OBJの表示OFF
  GFL_DISP_GXS_SetVisibleControlDirect(0);
  GX_SetVisiblePlane( 0 );
  GXS_SetVisiblePlane( 0 );
#if WB_FIX
  sys_KeyRepeatSpeedSet( SYS_KEYREPEAT_SPEED_DEF, SYS_KEYREPEAT_WAIT_DEF );
#endif

  p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIP2PMATCH, HEAPID_WIFIP2PMATCH );


  // 文字列マネージャー生成
  wk->WordSet    = WORDSET_Create( HEAPID_WIFIP2PMATCH );
  wk->MsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_lobby_dat, HEAPID_WIFIP2PMATCH );
  wk->SysMsgManager = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_wifi_system_dat, HEAPID_WIFIP2PMATCH );
  wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_gftr , GFL_FONT_LOADTYPE_FILE , FALSE , HEAPID_WIFIP2PMATCH );

  GFL_DISP_SetDispSelect( GX_DISP_SELECT_MAIN_SUB );

  // VRAM バンク設定
  VramBankSet();

  // BGLレジスタ設定
  BgInit(HEAPID_WIFIP2PMATCH );
  GFL_BMPWIN_Init( HEAPID_WIFIP2PMATCH );


  
  //BGグラフィックセット
  BgGraphicSet( wk, p_handle );

  //パレットアニメシステム作成
  ConnectBGPalAnm_Init(&wk->cbp, p_handle, NARC_wifip2pmatch_conect_anm_NCLR, HEAPID_WIFIP2PMATCH);

  // VBlank関数セット
  //    sys_VBlankFuncChange( VBlankFunc, wk );


  wk->vblankFunc = GFUser_VIntr_CreateTCB( VBlankFunc , wk , 1 );


  // OBJキャラ、パレットマネージャー初期化
  char_pltt_manager_init();

  // CellActorシステム初期化
  InitCellActor(wk, p_handle);

  // CellActro表示登録
  SetCellActor(wk);

  GFL_ARC_CloseDataHandle( p_handle );
}



#define DEFAULT_NAME_MAX    18

//--------------------------------------------------------------------------------------------
/**
 * グラフィックにかかわる部分の終了処理
 * @param wk  WIFIP2PMATCH_WORK
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void _graphicEnd(WIFIP2PMATCH_WORK* wk)
{
  int i;


  //    sys_VBlankFuncChange( NULL, NULL ); // VBlankReset
  // Vblank期間中のBG転送終了
  if(wk->vblankFunc){
    GFL_TCB_DeleteTask( wk->vblankFunc );
  }
  ConnectBGPalAnm_End(&wk->cbp);


  // マッチングルーム破棄
  if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == TRUE ){
    WIFI_MCR_Dest( &wk->matchroom );
  }
  // ビューアー破棄
  if( MCVSys_MoveCheck( wk ) == TRUE ){
    MCVSys_Exit( wk );
  }

  // アイコン破棄
  WifiP2PMatchFriendListIconRelease( &wk->icon );

#if 0
  // キャラ転送マネージャー破棄
  CLACT_U_CharManagerDelete(wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES]);

  // パレット転送マネージャー破棄
  CLACT_U_PlttManagerDelete(wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES]);
#endif

  // FontOAM破棄
  //FONTOAM_SysDelete( wk->fontoam );  @@OO

  // キャラ・パレット・セル・セルアニメのリソースマネージャー破棄
  //    for(i=0;i<CLACT_RESOURCE_NUM;i++){
  //        CLACT_U_ResManagerDelete(wk->resMan[i]);
  //    }
  // セルアクターセット破棄
  GFL_CLACT_UNIT_Delete(wk->clactSet);

  //OAMレンダラー破棄
  //    REND_OAM_Delete();
  GFL_CLACT_SYS_Delete();//

  // リソース解放
  //    DeleteCharManager();
  //    DeletePlttManager();

  // BMPウィンドウ開放
  BmpWinDelete( wk );
  BgExit(wk);

  GFL_BMPWIN_Exit();
  // BGL削除
  GFL_BG_Exit( );

  // 親のプロックの処理
  //    UnionRoomView_ObjInit( (COMM_UNIONROOM_VIEW*)GFL_PROC_GetParentWork( proc ) );

  // タッチパネルシステム終了
  //  StopTP();


  // メッセージマネージャー・ワードセットマネージャー解放
  GFL_MSG_Delete( wk->SysMsgManager );
  GFL_MSG_Delete( wk->MsgManager );
  WORDSET_Delete( wk->WordSet );

  GFL_FONT_Delete(wk->fontHandle);

  // オーバーレイ破棄
  //  GFL_OVERLAY_Unload( FS_OVERLAY_ID(wifi_2dmapsys) );
}


//--------------------------------------------------------------------------------------------
/**
 * VBlank関数
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
//#if 0
static void VBlankFunc( GFL_TCB *tcb , void * work )
{
  WIFIP2PMATCH_WORK* wk = work;

  // セルアクター
  // Vram転送マネージャー実行
  //  DoVramTransferManager();

  // レンダラ共有OAMマネージャVram転送
  //REND_OAMTrans();

  //ConnectBGPalAnm_VBlank(&wk->cbp);

  // _retry関数内でマスター輝度を設定して、
  // きれいに復帰できるようにするためここで復帰
  //  if( wk->brightness_reset == TRUE ){
  //    WIPE_ResetBrightness( WIPE_DISP_MAIN );
  //    WIPE_ResetBrightness( WIPE_DISP_SUB );
  //    wk->brightness_reset = FALSE;
  //  }

  // OS_SetIrqCheckFlag( OS_IE_V_BLANK );

  GFL_CLACT_SYS_VBlankFunc();

}
//#endif

//--------------------------------------------------------------------------------------------
/**
 * VRAM設定
 *
 * @param none
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void VramBankSet(void)
{
  GFL_DISP_SetBank( &_defVBTbl );
}

//--------------------------------------------------------------------------------------------
/**
 * BG設定
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgInit(HEAPID heapID )
{
  int i;

  GFL_BG_Init(heapID);
  // BG SYSTEM
  {
    GFL_BG_SYS_HEADER BGsys_data = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
    };
    GFL_BG_SetBGMode( &BGsys_data );
  }

  // 背景面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x1000, 0, GFL_BG_SCRSIZ_512x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000,
      0x10000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_M );
    GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );


  }

  // メイン画面1
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd000, GX_BG_CHARBASE_0x10000,
      0x8000,
      GX_BG_EXTPLTT_01,
      1, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
//    GFL_BG_FillCharacter( GFL_BG_FRAME1_M, 0x00, 1, 0 );
    GFL_BG_ClearFrame( GFL_BG_FRAME1_M);
    GFL_BG_LoadScreenReq( GFL_BG_FRAME1_M );
  }

  // リストなど
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xd800, GX_BG_CHARBASE_0x08000,
      0x8000,
      GX_BG_EXTPLTT_23,
      2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
   GFL_BG_FillCharacter( GFL_BG_FRAME2_M, 0x00, 1, 0 );
    GFL_BG_ClearFrame( GFL_BG_FRAME2_M);
    GFL_BG_LoadScreenReq( GFL_BG_FRAME2_M );
  }

#if 1
  // いろいろに使う
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x18000,
      0x8000,
      GX_BG_EXTPLTT_23,
      1, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME3_M);
    GFL_BG_LoadScreenReq( GFL_BG_FRAME3_M );
  }
#endif

  // サブ画面テキスト面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME0_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME0_S );
  }
  // ユーザー状態表示面
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME1_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME1_S );
  }
  // ボタンorユーザーデータ時の背景
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x10000,
      0x8000,
      GX_BG_EXTPLTT_01,
      2, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME2_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME2_S );
  }
  // ユーザーデータテキスト
  {
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x18000,
      0x8000,
      GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( GFL_BG_FRAME3_S, &TextBgCntDat, GFL_BG_MODE_TEXT );
    GFL_BG_ClearFrame( GFL_BG_FRAME3_S );
  }

  for(i=0;i <GFL_BG_FRAME3_S;i++){
    GFL_BG_ClearCharacter( i );
  }

  GFL_BG_SetPriority(GFL_BG_FRAME0_M , 3);  //はいけい
  GFL_BG_SetPriority(GFL_BG_FRAME1_M , 1);   // yesno win　ユーザーデータの背景
  GFL_BG_SetPriority(GFL_BG_FRAME3_M , 1);  // メッセージ
  GFL_BG_SetPriority(GFL_BG_FRAME2_M , 0);   // menuリスト

  G2_BlendNone();
  G2S_BlendNone();
}

static void WindowSet(void)
{
  GX_SetVisibleWnd( GX_WNDMASK_NONE );
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
  /*
    GX_SetVisibleWnd( GX_WNDMASK_W0 );
  G2_SetWnd0InsidePlane( GX_WND_PLANEMASK_NONE, 1 );
  G2_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, 1 );
  G2_SetWnd0Position( 0, 255, 0, 192 );

  GXS_SetVisibleWnd( GX_WNDMASK_W0 );
  G2S_SetWnd0InsidePlane( GX_WND_PLANEMASK_NONE, 1 );
  G2S_SetWndOutsidePlane( GX_WND_PLANEMASK_NONE, 1 );
  G2S_SetWnd0Position( 0, 255, 0, 192 );
   */
}

//----------------------------------------------------------------------------
/**
 *  @brief  生成した動作オブジェクトワークを設定する
 *
 *  @param  wk    ワーク
 *  @param  p_obj オブジェクトワーク
 */
//-----------------------------------------------------------------------------
static void MCRSYS_SetMoveObjWork( WIFIP2PMATCH_WORK* wk, MCR_MOVEOBJ* p_obj )
{
  int i;

  for( i=0; i<MATCHROOM_IN_OBJNUM; i++ ){
    if( wk->p_matchMoveObj[i] == NULL ){
      wk->p_matchMoveObj[i] = p_obj;
      return ;
    }
  }
  GF_ASSERT( 0 );
}
//----------------------------------------------------------------------------
/**
 *  @brief  引数のOBJが入っているワークをNULLにする
 *
 *  @param  wk    ワーク
 *  @param  p_obj NULLを入れてほしいワークに入っているポインタ
 */
//-----------------------------------------------------------------------------
static void MCRSYS_DelMoveObjWork( WIFIP2PMATCH_WORK* wk, MCR_MOVEOBJ* p_obj )
{
  int i;

  for( i=0; i<MATCHROOM_IN_OBJNUM; i++ ){
    if( wk->p_matchMoveObj[i] == p_obj ){
      wk->p_matchMoveObj[i] = NULL;
      return ;
    }
  }
  GF_ASSERT( 0 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  friendNOのオブジェクトワークを取得する
 *
 *  @param  wk      ワーク
 *  @param  friendNo  friendNO
 *
 *  @return オブジェクトワーク
 */
//-----------------------------------------------------------------------------
static MCR_MOVEOBJ* MCRSYS_GetMoveObjWork( WIFIP2PMATCH_WORK* wk, u32 friendNo )
{
  int i;

  for( i=0; i<MATCHROOM_IN_OBJNUM; i++ ){
    if( wk->p_matchMoveObj[i] ){
      if( WIFI_MCR_GetFriendNo( wk->p_matchMoveObj[i] ) == friendNo ){
        return wk->p_matchMoveObj[i];
      }
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *  @brief  アイコンデータ読込み
 *
 *  @param  p_data    データ格納先
 *  @param  p_handle  ハンドル
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatchFriendListIconLoad( WIFIP2PMATCH_ICON* p_data, ARCHANDLE* p_handle, u32 heapID )
{
  // パレット転送
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NCLR,
                                    PALTYPE_MAIN_BG, PLAYER_DISP_ICON_PLTTOFS*32,
                                    PLAYER_DISP_ICON_PLTTNUM*32, heapID );

  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NCLR,
                                    PALTYPE_SUB_BG, PLAYER_DISP_ICON_PLTTOFS_SUB*32,
                                    PLAYER_DISP_ICON_PLTTNUM*32, heapID );

  // キャラクタデータ転送 先頭からとらないといけない

  p_data->wf_match_all_iconcgx1m =
    GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle,
                                                 NARC_wifip2pmatch_wf_match_all_icon_NCGR,
                                                 GFL_BG_FRAME1_M, 0, 0, heapID);

  p_data->wf_match_all_iconcgx2s =
    GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle,
                                                 NARC_wifip2pmatch_wf_match_all_icon_NCGR,
                                                 GFL_BG_FRAME2_S, 0, 0, heapID);

/*
  if(p_data->wf_match_all_iconcgx1m){
    GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_M, PLAYER_DISP_ICON_CGX, p_data->wf_match_all_iconcgx1m);
  }
  if(p_data->wf_match_all_iconcgx2s){
    GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S, PLAYER_DISP_ICON_CGX,  p_data->wf_match_all_iconcgx2s);
  }
   */
/*  {
    u16 oneCharSize = GFL_BG_GetBaseCharacterSize(GFL_BG_FRAME1_M);
    p_data->wf_match_all_iconcgx1m = GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NCGR,
                                                     GFL_BG_FRAME1_M, PLAYER_DISP_ICON_CGX, 0, FALSE, heapID );
    GFL_BG_ReserveCharacterArea(GFL_BG_FRAME1_M, PLAYER_DISP_ICON_CGX, p_data->wf_match_all_iconcgx1m/oneCharSize*2);

    p_data->wf_match_all_iconcgx2s = GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NCGR,
                                                     GFL_BG_FRAME2_S, PLAYER_DISP_ICON_CGX, 0, FALSE, heapID );
    GFL_BG_ReserveCharacterArea(GFL_BG_FRAME2_S, PLAYER_DISP_ICON_CGX,  p_data->wf_match_all_iconcgx2s/oneCharSize);
  }
*/
  // さらにキャラクタデータを保存しておく
  if( p_data->p_charbuff == NULL ){
    p_data->p_charbuff = GFL_ARCHDL_UTIL_LoadOBJCharacter( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NCGR,
                                                           FALSE, &p_data->p_char, heapID);
  }

  // スクリーンデータ読込み
  // リトライ時にスクリーンデータがある場合は読み込まない
  if( p_data->p_buff == NULL ){
    p_data->p_buff = GFL_ARCHDL_UTIL_LoadScreen( p_handle, NARC_wifip2pmatch_wf_match_all_icon_NSCR,
                                                 FALSE, &p_data->p_scrn, heapID);
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  Iconデータはき
 *
 *  @param  p_data  ワーク
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatchFriendListIconRelease( WIFIP2PMATCH_ICON* p_data )
{

  if( p_data->p_buff != NULL ){
    GFL_HEAP_FreeMemory( p_data->p_buff );
    p_data->p_buff = NULL;
  }
  if( p_data->p_charbuff != NULL ){
    GFL_HEAP_FreeMemory( p_data->p_charbuff );
    p_data->p_charbuff = NULL;
  }

  //GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_M, PLAYER_DISP_ICON_CGX, p_data->wf_match_all_iconcgx1m);
  //GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S, PLAYER_DISP_ICON_CGX,  p_data->wf_match_all_iconcgx2s);

  GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_M,
                           GFL_ARCUTIL_TRANSINFO_GetPos(p_data->wf_match_all_iconcgx1m),
                           GFL_ARCUTIL_TRANSINFO_GetSize(p_data->wf_match_all_iconcgx1m));
  GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S,
                           GFL_ARCUTIL_TRANSINFO_GetPos(p_data->wf_match_all_iconcgx2s),
                           GFL_ARCUTIL_TRANSINFO_GetSize(p_data->wf_match_all_iconcgx2s));

  

}

//----------------------------------------------------------------------------
/**
 *  @brief  アイコンを書き込む
 *
 *  @param  p_data    データ
 *  @param  frm     フレーム
 *  @param  cx      キャラクタｘ座標
 *  @param  cy      キャラクタｙ座標
 *  @param  icon_type アイコンタイプ
 *  @param  col     0=灰 1=赤
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatchFriendListIconWrite(WIFIP2PMATCH_ICON* p_data, u32 frm, u8 cx, u8 cy, u32 icon_type, u32 col )
{
  int pal;
  // 書き込み
  GFL_BG_WriteScreenExpand(frm, cx, cy,
                           PLAYER_DISP_ICON_SCRN_X, PLAYER_DISP_ICON_SCRN_Y,
                           p_data->p_scrn->rawData,
                           PLAYER_DISP_ICON_SCRN_X*icon_type, 0,
                           p_data->p_scrn->screenWidth/8, p_data->p_scrn->screenHeight/8 );


  // パレット
  pal = WifiP2PMatchBglFrmIconPalGet( frm );

  // パレットナンバーをあわせる
  GFL_BG_ChangeScreenPalette( frm, cx, cy,
                              PLAYER_DISP_ICON_SCRN_X, PLAYER_DISP_ICON_SCRN_Y, pal+col );

  // 転送
  GFL_BG_LoadScreenV_Req( frm );
}



static u8 _gamemode2icon(  u32 gamemode )
{
  u8 scrn_idx=0;

  if( gamemode == WIFI_GAME_VCT){      // VCT中
    scrn_idx = PLAYER_DISP_ICON_IDX_VCT;
  }
  else if(_modeIsBattleStatus(gamemode)){
    scrn_idx = PLAYER_DISP_ICON_IDX_FIGHT;
  }
  else if( gamemode == WIFI_GAME_TRADE){          // 交換中
    scrn_idx = PLAYER_DISP_ICON_IDX_CHANGE;
  }
  else if(WIFI_GAME_LOGIN_WAIT == gamemode){    // 待機中　ログイン直後はこれ
    scrn_idx = PLAYER_DISP_ICON_IDX_NORMAL;
  }
  else if(WIFI_GAME_NONE == gamemode){    // 何も無い
    scrn_idx = PLAYER_DISP_ICON_IDX_NONE;
  }
  else if(gamemode >= WIFI_GAME_UNKNOWN){    // 新たに作ったらこの番号以上になる
    scrn_idx = PLAYER_DISP_ICON_IDX_UNK;
  }
  return scrn_idx;
}


//----------------------------------------------------------------------------
/**
 *  @brief  Icon書き込み
 *
 *  @param  p_data    データ
 *  @param  frm     フレーム
 *  @param  cx      キャラクタｘ座標
 *  @param  cy      キャラクタｙ座標
 *  @param  status    状態
 *  @param  vctIcon   ボイスチャットONOFFフラグ
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatchFriendListStIconWrite(  WIFIP2PMATCH_ICON* p_data, u32 frm, u8 cx, u8 cy, u32 status,u32 gamemode )
{
  u8 col=0;
  u8 scrn_idx=0;

  scrn_idx = _gamemode2icon(gamemode);
  if(status == WIFI_STATUS_RECRUIT){
    col = 1;
  }

  // 書き込み
  WifiP2PMatchFriendListIconWrite( p_data, frm, cx, cy, scrn_idx, col );
}



//----------------------------------------------------------------------------
/**
 *  @brief  ビットマップにアイコンデータを書き込む
 *
 *  @param  p_bmp   ビットマップ
 *  @param  p_data    アイコンデータ
 *  @param  x     書き込みｘ座標
 *  @param  y     書き込みｙ座標
 *  @param  status    状態
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatchFriendListBmpStIconWrite( GFL_BMPWIN* p_bmp, WIFIP2PMATCH_ICON* p_data, u16 x, u16 y, u32 status,u32 gamemode,int pal )
{
  u8 col=0;
  u8 scrn_idx=0;

  if(status == WIFI_STATUS_RECRUIT){
    col = 1;
  }
  scrn_idx = _gamemode2icon(gamemode);

  WifiP2PMatchFriendListBmpIconWrite( p_bmp, p_data, x, y, scrn_idx, col,pal );
}

//----------------------------------------------------------------------------
/**
 *  @brief  frmのアイコンパレットナンバーを返す
 *
 *  @param  frm   BGL　FRAME
 *
 *  @return パレットNO
 */
//-----------------------------------------------------------------------------
static int WifiP2PMatchBglFrmIconPalGet( u32 frm )
{
  if( frm < GFL_BG_FRAME0_S ){
    return PLAYER_DISP_ICON_PLTTOFS;
  }
  return PLAYER_DISP_ICON_PLTTOFS_SUB;
}

//----------------------------------------------------------------------------
/**
 *  @brief  フレンドリスト開始宣言
 */
//-----------------------------------------------------------------------------
static int WifiP2PMatchFriendListStart( WIFIP2PMATCH_WORK *wk )
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
  //  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK,
  //          COMM_BRIGHTNESS_SYNC, 1, HEAPID_WIFIP2PMATCH);
//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT,
  //                            0,16,COMM_BRIGHTNESS_SYNC);

  // 部屋のBGM設定
  WIFI_STATUS_SetVChatStatus(wk->pMatch, wk->pParentWork->vchatMain);
  WifiP2P_SetLobbyBgm();

  return WIFIP2PMATCH_FRIENDLIST_INIT;
}

//--------------------------------------------------------------------------------------------
/**
 * BG解放
 *
 * @param ini   BGLデータ
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void BgExit( WIFIP2PMATCH_WORK *wk )
{
  int i;

  // for(i=0;i<GFL_BG_FRAME3_S;i++){
  //   GFL_BG_FreeBGControl( i );
  // }
  return;
  if(wk->talkwin_m2){
    GFL_BG_FreeCharacterArea(GFL_BG_FRAME1_S,GFL_ARCUTIL_TRANSINFO_GetPos(wk->talkwin_m2),
                             GFL_ARCUTIL_TRANSINFO_GetSize(wk->talkwin_m2));
    GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S,GFL_ARCUTIL_TRANSINFO_GetPos(wk->menuwin_m2),
                             GFL_ARCUTIL_TRANSINFO_GetSize(wk->menuwin_m2));
    GFL_BG_FreeCharacterArea(GFL_BG_FRAME2_S,GFL_ARCUTIL_TRANSINFO_GetPos(wk->menuwin_m1),
                             GFL_ARCUTIL_TRANSINFO_GetSize(wk->menuwin_m1));
    wk->talkwin_m2=0;
    wk->menuwin_m2=0;
    wk->menuwin_m1=0;
  }

}


//--------------------------------------------------------------------------------------------
/**
 * グラフィックデータセット
 *
 * @param wk    ポケモンリスト画面のワーク
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------


static void BgGraphicSet( WIFIP2PMATCH_WORK * wk, ARCHANDLE* p_handle )
{
  GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0 | GX_PLANEMASK_BG1 | GX_PLANEMASK_BG2 |
                                 GX_PLANEMASK_BG3, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0|GX_PLANEMASK_BG2, VISIBLE_ON );

  // アイコンデータ読込み
  WifiP2PMatchFriendListIconLoad(  &wk->icon, p_handle, HEAPID_WIFIP2PMATCH );

  
  // 上下画面ＢＧパレット転送
 // GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_wifip2pmatch_conect_NCLR, PALTYPE_MAIN_BG, 0, 0,  HEAPID_WIFIP2PMATCH);
 // GFL_ARCHDL_UTIL_TransVramPalette(    p_handle, NARC_wifip2pmatch_conect_NCLR, PALTYPE_SUB_BG,  0, 0,  HEAPID_WIFIP2PMATCH);

  // 会話フォントパレット転送
  //    TalkFontPaletteLoad( PALTYPE_MAIN_BG, _NUKI_FONT_PALNO*0x20, HEAPID_WIFIP2PMATCH );
  //    TalkFontPaletteLoad( PALTYPE_MAIN_BG, COMM_MESFONT_PAL*0x20, HEAPID_WIFIP2PMATCH );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , 13*0x20, 16*2, HEAPID_WIFIP2PMATCH );

/*
  // メイン画面BG2キャラ転送
  {
    u32 scr = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle, NARC_wifip2pmatch_conect_NCGR,
                                                           GFL_BG_FRAME0_M, 0, 0, HEAPID_WIFIP2PMATCH);

    // メイン画面BG2スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifip2pmatch_conect_01_NSCR,
                                              GFL_BG_FRAME0_M, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(scr), 0, 0, HEAPID_WIFIP2PMATCH);
  }

  // サブ画面BG0キャラ転送
  {
    u32 scr = GFL_ARCHDL_UTIL_TransVramBgCharacterAreaMan( p_handle,
                                                           NARC_wifip2pmatch_conect_sub_NCGR,
                                                           GFL_BG_FRAME0_S, 0, 0, HEAPID_WIFIP2PMATCH);

    // サブ画面BG0スクリーン転送
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(   p_handle, NARC_wifip2pmatch_conect_sub_NSCR,
                                              GFL_BG_FRAME0_S, 0,
                                              GFL_ARCUTIL_TRANSINFO_GetPos(scr),0, 0, HEAPID_WIFIP2PMATCH);
  }
   */
  {
    wk->talkwin_m2 = BmpWinFrame_GraphicSetAreaMan(
      GFL_BG_FRAME2_M,  COMM_MESFRAME_PAL,  MENU_TYPE_SYSTEM, HEAPID_WIFIP2PMATCH );
    wk->menuwin_m2 = BmpWinFrame_GraphicSetAreaMan(
      GFL_BG_FRAME2_M,  MENU_WIN_PAL, MENU_TYPE_SYSTEM, HEAPID_WIFIP2PMATCH );
    wk->menuwin_m1 = BmpWinFrame_GraphicSetAreaMan(
      GFL_BG_FRAME1_M,  MENU_WIN_PAL, MENU_TYPE_SYSTEM, HEAPID_WIFIP2PMATCH );
  }

}


//** CharManager PlttManager用 **//
#define WIFIP2PMATCH_CHAR_CONT_NUM        (20)
#define WIFIP2PMATCH_CHAR_VRAMTRANS_MAIN_SIZE   (128*1024)
#define WIFIP2PMATCH_CHAR_VRAMTRANS_SUB_SIZE    (16*1024)
#define WIFIP2PMATCH_PLTT_CONT_NUM        (20)

//-------------------------------------
//
//  キャラクタマネージャー
//  パレットマネージャーの初期化
//
//=====================================
static void char_pltt_manager_init(void)
{
#if 0
  // キャラクタマネージャー初期化
  {
    CHAR_MANAGER_MAKE cm = {
      WIFIP2PMATCH_CHAR_CONT_NUM,
      WIFIP2PMATCH_CHAR_VRAMTRANS_MAIN_SIZE,
      WIFIP2PMATCH_CHAR_VRAMTRANS_SUB_SIZE,
      HEAPID_WIFIP2PMATCH
      };
    InitCharManagerReg(&cm, GX_OBJVRAMMODE_CHAR_1D_128K, GX_OBJVRAMMODE_CHAR_1D_32K );
  }
  // パレットマネージャー初期化
  InitPlttManager(WIFIP2PMATCH_PLTT_CONT_NUM, HEAPID_WIFIP2PMATCH);

  // 読み込み開始位置を初期化
  CharLoadStartAll();
  PlttLoadStartAll();
  //通信アイコン用にキャラ＆パレット制限
  CLACT_U_WmIcon_SetReserveAreaCharManager(NNS_G2D_VRAM_TYPE_2DMAIN, GX_OBJVRAMMODE_CHAR_1D_128K);
  CLACT_U_WmIcon_SetReserveAreaPlttManager(NNS_G2D_VRAM_TYPE_2DMAIN);
#endif
}


//------------------------------------------------------------------
/**
 * セルアクター初期化
 *
 * @param   wk    レーダー構造体のポインタ
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void InitCellActor(WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle)
{
  int i;

#if 0
  // OAMマネージャーの初期化
  NNS_G2dInitOamManagerModule();

  // 共有OAMマネージャ作成
  // レンダラ用OAMマネージャ作成
  // ここで作成したOAMマネージャをみんなで共有する
  REND_OAMInit(
    0, 126,   // メイン画面OAM管理領域
    0, 32,    // メイン画面アフィン管理領域
    0, 126,   // サブ画面OAM管理領域
    0, 32,    // サブ画面アフィン管理領域
    HEAPID_WIFIP2PMATCH);


  // セルアクター初期化
  wk->clactSet = CLACT_U_SetEasyInit( WF_CLACT_WKNUM, &wk->renddata, HEAPID_WIFIP2PMATCH );

  CLACT_U_SetSubSurfaceMatrix( &wk->renddata, 0, NAMEIN_SUB_ACTOR_DISTANCE );


  //リソースマネージャー初期化
  for(i=0;i<CLACT_RESOURCE_NUM;i++){    //リソースマネージャー作成
    wk->resMan[i] = CLACT_U_ResManagerInit(WF_CLACT_RESNUM, i, HEAPID_WIFIP2PMATCH);
  }
#endif
  const u8 CELL_MAX = 16;
  //  GFL_CLSYS_INIT cellSysInitData = GFL_CLSYSINIT_DEF_DIVSCREEN;
  //  cellSysInitData.oamst_main = GFL_CLSYS_OAMMAN_INTERVAL; //通信アイコンの分
  //  cellSysInitData.oamnum_main = 64-GFL_CLSYS_OAMMAN_INTERVAL;
  //  cellSysInitData.oamst_sub = 16;
  //  cellSysInitData.oamnum_sub = 128-16;

  GFL_CLACT_SYS_Create( &GFL_CLSYSINIT_DEF_DIVSCREEN , &_defVBTbl, HEAPID_WIFIP2PMATCH );
  wk->clactSet  = GFL_CLACT_UNIT_Create( CELL_MAX , 0, HEAPID_WIFIP2PMATCH );
  GFL_CLACT_UNIT_SetDefaultRend( wk->clactSet );


  // FontOAMシステム作成
  //  wk->fontoam = FONTOAM_SysInit( WF_FONTOAM_NUM, HEAPID_WIFIP2PMATCH );  //@@OO


#if 0
  //---------上画面用-------------------

  //chara読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES] =
    CLACT_U_ResManagerResAddArcChar_ArcHandle(wk->resMan[CLACT_U_CHAR_RES],
                                              p_handle,
                                              NARC_wifip2pmatch_conect_obj_NCGR, FALSE, 0, NNS_G2D_VRAM_TYPE_2DMAIN, HEAPID_WIFIP2PMATCH);

  //pal読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES] =
    CLACT_U_ResManagerResAddArcPltt_ArcHandle(wk->resMan[CLACT_U_PLTT_RES],
                                              p_handle,
                                              NARC_wifip2pmatch_conect_obj_NCLR, 0, 0, NNS_G2D_VRAM_TYPE_2DMAIN, 7, HEAPID_WIFIP2PMATCH);

  //cell読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_CELL_RES] =
    CLACT_U_ResManagerResAddArcKindCell_ArcHandle(wk->resMan[CLACT_U_CELL_RES],
                                                  p_handle,
                                                  NARC_wifip2pmatch_conect_NCER, FALSE, 0, CLACT_U_CELL_RES,HEAPID_WIFIP2PMATCH);

  //同じ関数でanim読み込み
  wk->resObjTbl[MAIN_LCD][CLACT_U_CELLANM_RES] =
    CLACT_U_ResManagerResAddArcKindCell_ArcHandle(wk->resMan[CLACT_U_CELLANM_RES],
                                                  p_handle,
                                                  NARC_wifip2pmatch_conect_NANR, FALSE, 0, CLACT_U_CELLANM_RES,HEAPID_WIFIP2PMATCH);


  // リソースマネージャーから転送

  // Chara転送
  CLACT_U_CharManagerSetAreaCont( wk->resObjTbl[MAIN_LCD][CLACT_U_CHAR_RES] );
  //  CLACT_U_CharManagerSet( wk->resObjTbl[SUB_LCD][CLACT_U_CHAR_RES] );

  // パレット転送
  CLACT_U_PlttManagerSetCleanArea( wk->resObjTbl[MAIN_LCD][CLACT_U_PLTT_RES] );
  //  CLACT_U_PlttManagerSet( wk->resObjTbl[SUB_LCD][CLACT_U_PLTT_RES] );

#endif

}

#define TRAINER_NAME_POS_X    ( 24 )
#define TRAINER_NAME_POS_Y    ( 32 )
#define TRAINER_NAME_POS_SPAN ( 32 )

#define TRAINER_NAME_WIN_X    (  3 )
#define TRAINER1_NAME_WIN_Y   (  6 )
#define TRAINER2_NAME_WIN_Y   (  7 )

//------------------------------------------------------------------
/**
 * セルアクター登録
 *
 * @param   wk      WIFIP2PMATCH_WORK*
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void SetCellActor(WIFIP2PMATCH_WORK *wk)
{
#if 0
  int i;
  // セルアクターヘッダ作成
  CLACT_U_MakeHeader(&wk->clActHeader_m, 0, 0, 0, 0, CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
                     0, 0,
                     wk->resMan[CLACT_U_CHAR_RES],
                     wk->resMan[CLACT_U_PLTT_RES],
                     wk->resMan[CLACT_U_CELL_RES],
                     wk->resMan[CLACT_U_CELLANM_RES],
                     NULL,NULL);

  /*
  CLACT_U_MakeHeader(&wk->clActHeader_s, 1, 1, 1, 1, CLACT_U_HEADER_DATA_NONE, CLACT_U_HEADER_DATA_NONE,
  0, 0,
  wk->resMan[CLACT_U_CHAR_RES],
  wk->resMan[CLACT_U_PLTT_RES],
  wk->resMan[CLACT_U_CELL_RES],
  wk->resMan[CLACT_U_CELLANM_RES],
  NULL,NULL);
   */
  {
    //登録情報格納
    CLACT_ADD add;

    add.ClActSet  = wk->clactSet;
    add.ClActHeader = &wk->clActHeader_m;

    add.mat.z   = 0;
    add.sca.x   = FX32_ONE;
    add.sca.y   = FX32_ONE;
    add.sca.z   = FX32_ONE;
    add.rot     = 0;
    add.pri     = 1;
    add.DrawArea  = NNS_G2D_VRAM_TYPE_2DMAIN;
    add.heap    = HEAPID_WIFIP2PMATCH;

    //セルアクター表示開始
    // サブ画面用(矢印の登録）
    for(i=0;i < _OAM_NUM;i++){
      add.mat.x = FX32_ONE *   TRAINER_NAME_POS_X;
      add.mat.y = FX32_ONE * ( TRAINER_NAME_POS_Y + TRAINER_NAME_POS_SPAN*i ) + NAMEIN_SUB_ACTOR_DISTANCE;
      wk->MainActWork[i] = CLACT_Add(&add);
      CLACT_SetAnmFlag(wk->MainActWork[i],1);
      CLACT_AnmChg( wk->MainActWork[i], i );
      CLACT_BGPriorityChg(wk->MainActWork[i], 2);
      CLACT_SetDrawFlag( wk->MainActWork[i], 0 );
    }

  }
#endif
  GFL_DISP_GX_SetVisibleControl(  GX_PLANEMASK_OBJ, VISIBLE_ON ); //メイン画面OBJ面ＯＮ
  //  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON ); //サブ画面OBJ面ＯＮ
}


// はい・いいえBMP（下画面）
#define YESNO_WIN_FRAME_CHAR  ( 1 + TALK_WIN_CGX_SIZ )
#define YESNO_CHARA_OFFSET    ( 1 + TALK_WIN_CGX_SIZ + MENU_WIN_CGX_SIZ + FLD_MSG_WIN_SX*FLD_MSG_WIN_SY )
#define YESNO_WINDOW_X      ( 22 )
#define YESNO_WINDOW_Y1     (  7 )
#define YESNO_WINDOW_Y2     ( 13 )
#define YESNO_CHARA_W     (  8 )
#define YESNO_CHARA_H     (  4 )

/* static const NET_BMPWIN_DAT TouchYesNoBmpDat[2]={
 *     {
 *         GFL_BG_FRAME0_M, YESNO_WINDOW_X, YESNO_WINDOW_Y1,
 *         YESNO_CHARA_W, YESNO_CHARA_H, 13, YESNO_CHARA_OFFSET
 *         },
 *     {
 *         GFL_BG_FRAME0_M, YESNO_WINDOW_X, YESNO_WINDOW_Y2,
 *         YESNO_CHARA_W, YESNO_CHARA_H, 13, YESNO_CHARA_OFFSET+YESNO_CHARA_W*YESNO_CHARA_H
 *
 *         }
 * };
 */


//------------------------------------------------------------------
/**
 * $brief   開始時のメッセージ   WIFIP2PMATCH_MODE_INIT
 * @param   wk
 * @param   seq
 * @retval  none
 */
//------------------------------------------------------------------

static int WifiP2PMatch_MainInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 status,gamemode;

  if(wk->pParentWork->seq != WIFI_GAME_NONE){

    if(wk->pParentWork->btalk){  //話しかけ接続時
      _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_UNIONMATCH);
       WifiP2PMatch_FriendListInit2( wk, seq );
      _CHANGESTATE(wk,WIFIP2PMATCH_PLAYERDIRECT_INIT1);
    }
    else{  //相手と切断
      GFL_NET_StateWifiMatchEnd(TRUE);  // マッチングを切る
      _myStatusChange(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
      wk->timer = _RECONECTING_WAIT_TIME;
      _CHANGESTATE(wk, WIFIP2PMATCH_RECONECTING_WAIT);
      //      _CHANGESTATE(wk,WIFIP2PMATCH_CONNECTING_INIT);
    }
  }
  else
  {        // 接続がまだ
    _CHANGESTATE(wk, WIFIP2PMATCH_FRIENDLIST_INIT);
    PMSND_PlayNextBGM(SEQ_BGM_WIFI_ACCESS, 60, 0);
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   ログイン接続中  WIFIP2PMATCH_CONNECTING_INIT
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static int WifiP2PMatch_ConnectingInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  GFL_BG_SetPriority(GFL_BG_FRAME0_M , 3);  //はいけい
  GFL_BG_SetPriority(GFL_BG_FRAME1_M , 1);   // yesno win
  GFL_BG_SetPriority(GFL_BG_FRAME3_M , 1);  // メッセージ
  GFL_BG_SetPriority(GFL_BG_FRAME2_M , 0);   //
  _CHANGESTATE(wk,WIFIP2PMATCH_CONNECTING);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   _sendMatchStatus ステータスを送る
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static void _sendMatchStatus(WIFIP2PMATCH_WORK *wk)
{
  GFL_NET_DWC_SetMyInfo((const char *)wk->pMatch, sizeof(WIFI_STATUS));
}

//------------------------------------------------------------------
/**
 * $brief   MyMatchStatus作成
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static void _makeMyMatchStatus(WIFIP2PMATCH_WORK* wk, u32 status, u32 gamemode)
{
  MYSTATUS* pMyStatus = GAMEDATA_GetMyStatus(wk->pGameData);
  WIFI_HISTORY* pHistry = SaveData_GetWifiHistory(wk->pSaveData);

  WIFI_STATUS_SetPMVersion(wk->pMatch, PM_VERSION);
  WIFI_STATUS_SetPMLang(wk->pMatch, PM_LANG);
  WIFI_STATUS_SetSex(wk->pMatch, MyStatus_GetMySex(pMyStatus));
  WIFI_STATUS_SetTrainerView(wk->pMatch,MyStatus_GetTrainerView(pMyStatus));

#if PM_DEBUG
  if(MyStatus_GetMySex(pMyStatus) == PM_MALE){
    WIFI_STATUS_SetTrainerView(wk->pMatch,PLBOY1);
  }
  else{
    WIFI_STATUS_SetTrainerView(wk->pMatch,PLGORGGEOUSW);
  }
#endif

  _myStatusChange_not_send(wk, status, gamemode); // BGM状態などを調整
  WIFI_STATUS_SetMyNation(wk->pMatch, WIFIHISTORY_GetMyNation(pHistry));
  WIFI_STATUS_SetMyArea(wk->pMatch, WIFIHISTORY_GetMyArea(pHistry));
  WIFI_STATUS_SetVChatStatus(wk->pMatch, TRUE);

  _sendMatchStatus(wk);

}

//------------------------------------------------------------------
/**
 * $brief   フレンドのstatus読み込み
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _readFriendMatchStatus(WIFIP2PMATCH_WORK* wk)
{
  int i,num = 0;
  WIFI_STATUS* p_status;
  u32 status;

  //    MI_CpuFill8(wk->index2No, 0, sizeof(WIFIP2PMATCH_MEMBER_MAX));
  for(i = 0;i < WIFIP2PMATCH_MEMBER_MAX;i++){
    wk->index2No[i] = 0;

    // その人は自分の友達か？
    if( wk->friend_num > i ){
      // このチェックを少し進化させる
      // 最新情報を受信して初めて登録される
      if(DWC_STATUS_OFFLINE != WifiDwc_getFriendStatus(i)){
        p_status = WifiFriendMatchStatusGet( i );
        status = _WifiMyStatusGet( wk, p_status );

        if( (status != WIFI_STATUS_NONE) &&
            (status < WIFI_STATUS_UNKNOWN) ){
          //          GF_ASSERT(p_status->trainer_view != PLHERO);
          wk->index2No[num] = i+1;
          num++;
        }else{
          if( MCVSys_BttnTypeGet( wk, i+1 ) != MCV_BTTN_FRIEND_TYPE_NONE ){
            MCVSys_BttnDel( wk, i+1 );
          }
        }
      }
    }
  }
  return num;
}

//------------------------------------------------------------------
/**
 * $brief   フレンドのstatus検査　こちらの状態と変わったら変わった数を返す
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _checkUserDataMatchStatus(WIFIP2PMATCH_WORK* wk)
{
  int i,num = 0;
  BOOL back_up_wait;
  BOOL now_wait;
  MCR_MOVEOBJ* p_obj;
  WIFI_STATUS* p_status;
  u32 status;

  for(i = 0;i < WIFIP2PMATCH_MEMBER_MAX;i++){

    p_status = WifiFriendMatchStatusGet( i );
    status = _WifiMyStatusGet( wk, p_status );
    if((wk->matchStatusBackup[i]  != status) ||
       (wk->matchVchatBackup[i]  != WIFI_STATUS_GetVChatStatus(p_status)) ){

      // オブジェクトワーク
      p_obj = MCRSYS_GetMoveObjWork( wk, i+1 );

      // そのオブジェが登録され、新しい状態が描画オブジェに反映されるまでは
      // バックアップステータスを更新しない
      if( p_obj ){

        // ステータスが待ち状態になったらジャンプさせる
        // 待ち状態から普通に戻ったら通常に戻す
        back_up_wait = _modeWait( wk->matchStatusBackup[i] );
        now_wait = _modeWait( status );
        if( (back_up_wait == TRUE) && (now_wait == FALSE) ){
          //NPCの通常動作に戻す
          WIFI_MCR_NpcMoveSet( &wk->matchroom, p_obj,  MCR_NPC_MOVE_NORMAL );
        }else if( (back_up_wait == FALSE) && (now_wait == TRUE) ){

          //NPCをジャンプ動作にする
          WIFI_MCR_NpcMoveSet( &wk->matchroom, p_obj,  MCR_NPC_MOVE_JUMP );
        }

        wk->matchStatusBackup[i] = status;
        wk->matchVchatBackup[i] = WIFI_STATUS_GetVChatStatus(p_status);
#if 0 //@todo 通信に移動しなければいけない
        {
          const MYSTATUS* pMy = WIFI_STATUS_GetMyStatus(p_status);
          // 通信取得分を入れる
          NET_PRINT("通信取得分を入れる  %d %d\n", i, MyStatus_GetTrainerView(pMy));
          WifiList_SetFriendInfo(wk->pList, i,
                                 WIFILIST_FRIEND_UNION_GRA,
                                 MyStatus_GetTrainerView(pMy));
          WifiList_SetFriendInfo(wk->pList, i,
                                 WIFILIST_FRIEND_SEX,
                                 MyStatus_GetMySex(pMy));
        }
#endif
        num++;
      }
    }
  }
  return num;
}

//------------------------------------------------------------------
/**
 * $brief   エラー表示
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static void _errorDisp(WIFIP2PMATCH_WORK* wk)
{
  GFL_NETSTATE_DWCERROR* pErr = GFL_NET_StateGetWifiError();
  int msgno,no = pErr->errorCode;
  int type =  GFL_NET_DWC_ErrorType(pErr->errorCode, pErr->errorType);

  if((type == 11) || (no == ERRORCODE_0)){
    msgno = dwc_error_0015;
    type = 11;
  }
  else if(no == ERRORCODE_HEAP){
    msgno = dwc_error_0014;
    type = 12;
  }
  else{
    if( type >= 0 ){
      msgno = dwc_error_0001 + type;
    }else{
      msgno = dwc_error_0012;
    }
  }
  NET_PRINT("エラーメッセージ %d \n",msgno);
  EndMessageWindowOff(wk);
  WORDSET_RegisterNumber(wk->WordSet, 0, no,
                         5, NUMBER_DISPTYPE_ZERO, NUMBER_CODETYPE_DEFAULT);

  // エラーメッセージを表示しておくシンク数
  wk->timer = 30;

  // 上画面下の自分情報文字列をクリアする
  _systemMessagePrint(wk, msgno);

  switch(type){
  case 1:
  case 4:
  case 5:
  case 11:
    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY_INIT);
    break;
  case 6:
  case 7:
  case 8:
  case 9:
    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY_INIT);
    break;
  case 10:
    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY_INIT);
    break;
  case 0:
  case 2:
  case 3:
  default:
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CHECK_AND_END);
    break;
  }
}

//------------------------------------------------------------------
/**
 * $brief   接続をリトライする  WIFIP2PMATCH_RETRY_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _retryInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( wk->timer > 0 ){
    wk->timer --;
    return seq;
  }

  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
    wk->SysMsgWin = _BmpWinDel(wk->SysMsgWin);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_052, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY_YESNO);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   接続をリトライする  WIFIP2PMATCH_RETRY_YESNO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _retryYesNo( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( WifiP2PMatchMessageEndCheck(wk) ){

    _yenowinCreateM2( wk );

    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   接続をリトライする  WIFIP2PMATCH_RETRY_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _retryWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);

  if(ret == BMPMENU_NULL)
  {  // まだ選択中
    return seq;
  }
  else if(ret == 0)
  { // はいを選択した場合
    //GFL_NET_StateWifiLogout();
    GFL_NET_Exit(NULL);
    _myStatusChange(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
    _CHANGESTATE(wk,WIFIP2PMATCH_RETRY);
  }
  else
  {  // いいえを選択した場合
    _myStatusChange(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT );  // VCHATの状態を元に戻すため
    //GFL_NET_StateWifiLogout();
    GFL_NET_Exit(NULL);
    //@@OO     CommInfoFinalize();   //Info初期化
    wk->endSeq = WIFI_GAME_NONE;
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_END_WAIT);
  }
  EndMessageWindowOff(wk);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   接続をリトライ  WIFIP2PMATCH_RETRAY
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _retry( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;
  ARCHANDLE* p_handle;

  if(!GFL_NET_IsResetEnable())
  {

    if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == TRUE )
    {

      // ブラックアウト
      //      WIPE_SetBrightness( WIPE_DISP_MAIN, WIPE_FADE_BLACK );
      //      WIPE_SetBrightness( WIPE_DISP_SUB, WIPE_FADE_BLACK );
//      GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT,16,16,0);

    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
      
      WIFI_MCR_Dest( &wk->matchroom );
      for( i=0; i<MATCHROOM_IN_OBJNUM; i++ )
      {
        wk->p_matchMoveObj[i] = NULL;
      }

      // OAMバッファクリーン
      GFL_CLACT_SYS_ClearAllOamBuffer();
    }
    // ビューアー破棄
    if( MCVSys_MoveCheck( wk ) == TRUE ){
      MCVSys_Exit( wk );

    }

    // ビットマップOFF
    // ユーザーデータ
    wk->MyInfoWinBack = _BmpWinDel(wk->MyInfoWinBack);

    //接続開始
    GFL_BG_ClearFrame( GFL_BG_FRAME3_M);

    // グラフィックリセット
    p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIP2PMATCH, HEAPID_WIFIP2PMATCH );
    BgGraphicSet( wk, p_handle );
    GFL_ARC_CloseDataHandle( p_handle );


    // パレットフェード再開
    ConnectBGPalAnm_OccSet(&wk->cbp, TRUE);

    // ブライトネス状態リセっト
    // MCR画面破棄時に暗くした物を解除
    wk->brightness_reset = TRUE;
    //    WIPE_ResetBrightness( WIPE_DISP_MAIN );
    //    WIPE_ResetBrightness( WIPE_DISP_SUB );

    if( wk->menulist){
      GFL_BMPWIN_Delete(wk->ListWin);
      BmpMenuList_Exit(wk->lw, NULL, NULL);
      wk->lw = NULL;
      BmpMenuWork_ListDelete( wk->menulist );
      wk->menulist=NULL;
    }

    //        wk->friendMatchReadCount;
    MI_CpuClear8(wk->index2No, WIFIP2PMATCH_MEMBER_MAX);
    MI_CpuClear8(wk->index2NoBackUp, WIFIP2PMATCH_MEMBER_MAX);
    MI_CpuClear8(wk->matchStatusBackup, WIFIP2PMATCH_MEMBER_MAX*sizeof(int));
    MI_CpuClear8(wk->matchVchatBackup, WIFIP2PMATCH_MEMBER_MAX*sizeof(int));

#if 0
    for(i=0;i < _OAM_NUM;i++){
      CLACT_SetDrawFlag( wk->MainActWork[i], 0 );
    }
#endif
    MainMenuMsgInit(wk);
    _GFL_NET_InitAndStruct(wk,FALSE);
    //    GFL_NET_Init(&aGFLNetInit, NULL, wk);
    GFL_NET_StateWifiEnterLogin();
    //        wk->pMatch = GFL_NET_StateWifiEnterLogin(wk->pSaveData,sizeof(TEST_MATCH_WORK));
    WifiP2PMatchMessagePrint(wk, dwc_message_0008, TRUE);
    GF_ASSERT( wk->timeWaitWork == NULL );
    wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
    _CHANGESTATE(wk,WIFIP2PMATCH_CONNECTING_INIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   ログイン接続中  WIFIP2PMATCH_CONNECTING
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static int WifiP2PMatch_Connecting( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if( GFL_NET_DWC_GetSaving()) {
    _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_SAVING);
    WifiP2PMatchMessagePrint(wk, dwc_message_0015, TRUE);
    GF_ASSERT( wk->timeWaitWork == NULL );
    wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
  }

  if(GFL_NET_StateIsWifiLoginState()){
    NET_PRINT("WIFI接続完了\n");
    if( wk->bInitMessage ){  // 初回接続時にはセーブシーケンスへ
      //            SaveData_SaveParts(wk->pSaveData, SVBLK_ID_NORMAL);  //セーブ中
      _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_ENDMSG);
    }
    else{
      _makeMyMatchStatus(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
      _readFriendMatchStatus( wk );
      _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
    }
  }
  else if(GFL_NET_StateIsWifiError() || (GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT)){
    _errorDisp(wk);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   初回セーブ処理
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static int WifiP2PMatch_FirstSaving( WIFIP2PMATCH_WORK *wk, int seq )

{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if( GFL_NET_DWC_GetSaving()) {
    GAMEDATA_SaveAsyncStart(wk->pGameData);
    _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_SAVING2);
    return seq;
  }
  if(GFL_NET_StateIsWifiLoginState()){
    NET_PRINT("WIFI接続完了\n");
    if( wk->bInitMessage ){
      _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_ENDMSG);
    }
    else{
      _makeMyMatchStatus(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
      _readFriendMatchStatus( wk );
      _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
    }
  }
  else if(GFL_NET_StateIsWifiError() || (GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT)){
    _errorDisp(wk);
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   初回セーブ処理  WIFIP2PMATCH_FIRST_SAVING2
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static int WifiP2PMatch_FirstSaving2( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_DWC_GetSaving()){
    SAVE_RESULT result = GAMEDATA_SaveAsyncMain(wk->pGameData);
    if (result != SAVE_RESULT_CONTINUE && result != SAVE_RESULT_LAST) {
      GFL_NET_DWC_ResetSaving();
    }
    else{
      return seq;
    }
  }
  if(GFL_NET_StateIsWifiLoginState()){
    NET_PRINT("WIFI接続完了\n");
    if( wk->bInitMessage ){
      _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_ENDMSG);
    }
    else{
      _makeMyMatchStatus(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
      _readFriendMatchStatus( wk );
      _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
    }
  }
  else if(GFL_NET_StateIsWifiError() || (GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT)){
    _errorDisp(wk);
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   切断＋VCT待ちに移行するまで待つ  WIFIP2PMATCH_RECONECTING_WAIT
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------
static int WifiP2PMatch_ReConnectingWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;

  if(wk->timer > 0){
    wk->timer--;
    return seq;
  }

  NET_PRINT("GFL_NET_StateIsWifiLoginState %d  \n",GFL_NET_StateIsWifiLoginState());
  NET_PRINT("GFL_NET_StateGetWifiStatus %d  \n",GFL_NET_StateGetWifiStatus());
  NET_PRINT("GFL_NET_StateIsWifiDisconnect %d  \n",GFL_NET_StateIsWifiDisconnect());
  NET_PRINT("GFL_NET_IsConnectMember %d  \n",GFL_NET_IsConnectMember(GFL_NET_NETID_SERVER));

  if(GFL_NET_StateIsWifiLoginMatchState()){
    NET_PRINT("WIFI接続完了\n");
    if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == TRUE ){
      WIFI_MCR_Dest( &wk->matchroom );
    }
    _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
  }
  else if(GFL_NET_StateIsWifiError() || (GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT)){
    _errorDisp(wk);
  }
  else if((GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_DISCONNECTING) || GFL_NET_StateIsWifiDisconnect() || !GFL_NET_IsConnectMember(GFL_NET_NETID_SERVER)){
    GFL_NET_StateWifiMatchEnd(TRUE);
    if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == TRUE ){
      WIFI_MCR_Dest( &wk->matchroom );
    }
    _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
  }
  return seq;
}




//------------------------------------------------------------------
/**
 * $brief   最初接続完了  WIFIP2PMATCH_FIRST_ENDMSG
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _firstConnectEndMsg( WIFIP2PMATCH_WORK *wk, int seq )
{
  EndMessageWindowOff(wk);
  _systemMessagePrint(wk, dwc_message_0004);
  _CHANGESTATE(wk,WIFIP2PMATCH_FIRST_ENDMSG_WAIT);
  //#if AFTER_MASTER_070410_WIFIAPP_N22_EUR_FIX
  wk->bInitMessage = FALSE;
  //#endif //AFTER_MASTER_070410_WIFIAPP_N22_EUR_FIX
  return seq;

}

//WIFIP2PMATCH_FIRST_ENDMSG_WAIT
static int _firstConnectEndMsgWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
    if( 0==WifiList_GetFriendDataNum(wk->pList) ){  //フレンドがいない
      WifiP2PMatchMessagePrint(wk, dwc_message_0011, TRUE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXITING);
      wk->timer = 1;
    }
    else{
      _makeMyMatchStatus(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
      _readFriendMatchStatus( wk );
      {
        _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
      }
      wk->SysMsgWin = _BmpWinDel(wk->SysMsgWin);
    }
  }
  return seq;

}

//------------------------------------------------------------------
/**
 * $brief   フレンドリスト表示初期化
 *
 * @param   wk
 *
 * @retval  none
 */
//------------------------------------------------------------------

static void _userDataDisp(WIFIP2PMATCH_WORK* wk)
{
  if(wk->MyInfoWin){
    GFL_BMPWIN_Delete(wk->MyInfoWin);
  }

  GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG,
                                0x20*COMM_MESFONT_PAL, 0x20, HEAPID_WIFIP2PMATCH);

  wk->MyInfoWinBack = _BmpWinDel(wk->MyInfoWinBack);

  wk->MyInfoWin=GFL_BMPWIN_Create(
    GFL_BG_FRAME3_M,
    WIFIP2PMATCH_PLAYER_DISP_X, WIFIP2PMATCH_PLAYER_DISP_Y,
    WIFIP2PMATCH_PLAYER_DISP_SIZX, WIFIP2PMATCH_PLAYER_DISP_SIZY,
    COMM_MESFONT_PAL, GFL_BMP_CHRAREA_GET_B );


  wk->MyInfoWinBack=GFL_BMPWIN_Create(
    GFL_BG_FRAME2_M,
    WIFIP2PMATCH_PLAYER_DISP_X, WIFIP2PMATCH_PLAYER_DISP_Y,
    WIFIP2PMATCH_PLAYER_DISP_SIZX, WIFIP2PMATCH_PLAYER_DISP_SIZY,
    COMM_MESFONT_PAL, GFL_BMP_CHRAREA_GET_B );

  // 初期化
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MyInfoWin), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_TransVramCharacter(wk->MyInfoWin);
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->MyInfoWinBack), WINCLR_COL(FBMP_COL_WHITE) );
  GFL_BMPWIN_TransVramCharacter(wk->MyInfoWinBack);


  // システムウィンドウ設定
  //  BmpMenuWinWrite( wk->MyInfoWinBack, WINDOW_TRANS_ON, MENU_WIN_CGX_NUM, MENU_WIN_PAL );
  //    GFL_BMPWIN_MakeFrameScreen(wk->MyInfoWinBack, MENU_WIN_CGX_NUM, MENU_WIN_PAL);
  //BmpWinFrame_CgxSet(GFL_BG_FRAME1_M,COMM_TALK_WIN_CGX_NUM, MENU_TYPE_SYSTEM, HEAPID_WIFIP2PMATCH);
  BmpWinFrame_Write( wk->MyInfoWinBack, WINDOW_TRANS_ON_V, GFL_ARCUTIL_TRANSINFO_GetPos(wk->menuwin_m2), COMM_MESFRAME_PAL );
}

// ダミーオブジェクトを登録しまくる
static void DEBUG_DummyObjIn( WIFIP2PMATCH_WORK *wk, int num )
{
  int i;
  MCR_MOVEOBJ* p_moveobj;
  static const u8 view_list[ 32 ][2] = {
    { PLBOY1, PM_MALE },
    { PLBOY3, PM_MALE },
    { PLMAN3, PM_MALE },
    { PLBADMAN, PM_MALE },
    { PLEXPLORE, PM_MALE },
    { PLFIGHTER, PM_MALE },
    { PLGORGGEOUSM, PM_MALE },
    { PLMYSTERY, PM_MALE },
    { PLGIRL1, PM_FEMALE },
    { PLGIRL2, PM_FEMALE },
    { PLWOMAN2, PM_FEMALE },
    { PLWOMAN3, PM_FEMALE },
    { PLIDOL, PM_FEMALE },
    { PLLADY, PM_FEMALE },
    { PLCOWGIRL, PM_FEMALE },
    { PLGORGGEOUSW, PM_FEMALE },
    { PLBOY1, PM_MALE },
    { PLBOY3, PM_MALE },
    { PLMAN3, PM_MALE },
    { PLBADMAN, PM_MALE },
    { PLEXPLORE, PM_MALE },
    { PLFIGHTER, PM_MALE },
    { PLGORGGEOUSM, PM_MALE },
    { PLMYSTERY, PM_MALE },
    { PLGIRL1, PM_FEMALE },
    { PLGIRL2, PM_FEMALE },
    { PLWOMAN2, PM_FEMALE },
    { PLWOMAN3, PM_FEMALE },
    { PLIDOL, PM_FEMALE },
    { PLLADY, PM_FEMALE },
    { PLCOWGIRL, PM_FEMALE },
    { PLGORGGEOUSW, PM_FEMALE },
  };

  for( i=0; i<num; i++ ){
    // 自分を出す
    p_moveobj = WIFI_MCR_SetNpc( &wk->matchroom, view_list[i][0], i+1 );

    // みんな飛び跳ねる
    WIFI_MCR_NpcMoveSet( &wk->matchroom, p_moveobj, MCR_NPC_MOVE_JUMP );
    MCRSYS_SetMoveObjWork( wk, p_moveobj ); // 登録したワークを保存しておく

  }
}

//------------------------------------------------------------------
/**
 * $brief   マッチングしてよいかどうかの判定
 * @param   friendIndex判定する友人のindex
 * @retval  マッチングokならTRUE
 */
//------------------------------------------------------------------

static BOOL WIFIP2PModeCheck( int friendIndex ,void* pWork)
{
  u32 mySt,myGamemode;
  u32 targetSt,targetGamemode;
  WIFIP2PMATCH_WORK *wk = pWork;
  WIFI_STATUS* pMatch = wk->pMatch;
  WIFI_STATUS* p_status = WifiFriendMatchStatusGet( friendIndex);

  mySt = WIFI_STATUS_GetWifiStatus(pMatch);
  targetSt = WIFI_STATUS_GetWifiStatus(p_status);

  myGamemode = WIFI_STATUS_GetGameMode(pMatch);
  targetGamemode = WIFI_STATUS_GetGameMode(p_status);

  OS_TPrintf("%d %d\n",mySt, targetSt);

  if(myGamemode == targetGamemode){
    return TRUE;
  }
  return FALSE;
}



//------------------------------------------------------------------
/**
 * $brief   フレンドリスト表示初期化   WIFIP2PMATCH_FRIENDLIST_INIT
 *
 * @param   wk
 *
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_FriendListInit( WIFIP2PMATCH_WORK *wk, int seq )
{

  // エラーチェック
  if( GFL_NET_StateIsWifiError() == FALSE ){

    // 切断から再VCHAT募集状態に変わったのかをチェック
    if(!GFL_NET_StateIsWifiLoginMatchState()){
      return seq;
    }
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_FRIENDLIST_INIT2);
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   フレンドリスト表示初期化   WIFIP2PMATCH_FRIENDLIST_INIT2
 *
 * @param   wk
 *
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_FriendListInit2( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i,x;
  int num = 0;
  ARCHANDLE* p_handle;
  MCR_MOVEOBJ* p_moveobj;
  int obj_code;

  // コールバックを設定
  GFL_NET_DWC_SetConnectModeCheckCallback( WIFIP2PModeCheck );

  // ここから先は通信エラーをシステムウィンドウで出す

  ConnectBGPalAnm_OccSet(&wk->cbp, FALSE);

  p_handle = GFL_ARC_OpenDataHandle( ARCID_WIFIP2PMATCH, HEAPID_WIFIP2PMATCH );



  _myVChatStatusOrgSet(wk);

  //    _timeWaitIconDel(wk);   timeWait内でMsgWinを破棄しているということはメッセージ終了でもOK↓
  EndMessageWindowOff(wk);

  GFL_CLACT_SYS_Delete();//


  GFL_BG_ClearFrame(  GFL_BG_FRAME3_M);

  if(wk->menulist){
    GFL_BMPWIN_Delete(wk->ListWin);
    BmpMenuList_Exit(wk->lw, NULL, NULL);
    wk->lw = NULL;
    BmpMenuWork_ListDelete( wk->menulist );
    wk->menulist = NULL;
  }
  if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == FALSE ){

    if( MyStatus_GetMySex( GAMEDATA_GetMyStatus(wk->pGameData) ) == PM_MALE ){
      obj_code = PLHERO;
    }else{
      obj_code = PLHEROINE;
    }

    // マッチングルーム初期化
    wk->friend_num = WifiList_GetFriendDataLastIdx( wk->pList );
#ifdef WFP2PM_MANY_OBJ
    wk->friend_num = 32;
#endif
    WIFI_MCR_Init( &wk->matchroom, HEAPID_WIFIP2PMATCH, p_handle,
                   obj_code, wk->friend_num, ARCID_WIFIP2PMATCH );

    OS_TPrintf("自分を出す--\n");
    p_moveobj = WIFI_MCR_SetPlayer( &wk->matchroom, obj_code );
    MCRSYS_SetMoveObjWork( wk, p_moveobj ); // 登録したワークを保存しておく

#ifdef WFP2PM_MANY_OBJ
    DEBUG_DummyObjIn( wk, wk->friend_num );
#endif
  }
  // ビューアー初期化
  if( MCVSys_MoveCheck( wk ) == FALSE ){
    MCVSys_Init( wk, p_handle, HEAPID_WIFIP2PMATCH );
  }


  
  // 今の状態を書き込む
  MCVSys_ReWrite( wk, HEAPID_WIFIP2PMATCH );


  // ユーザーデータ表示
  EndMessageWindowOff(wk);
  _readFriendMatchStatus(wk);

  // ユーザーデータのBMP作成
  _userDataDisp(wk);

  _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);

  _commStateChange(wk,WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
  _GFL_NET_InitAndStruct(wk,FALSE);

  wk->preConnect = -1;

  // 080605 tomoya BTS:249の対処のため
  // 親の通信リセット処理後直ぐに接続してしまうのを回避
  // （接続されてもVCHATの状態のまま、FriendListシーケンスに移行させる）
  //  mydwc_ResetNewPlayer(); // ちなみにmydwc_returnLobby内でも初期化してる
#if 0
  CLACT_SetDrawFlag( wk->MainActWork[_CLACT_LINE_CUR], 1 );
#endif

  GFL_ARC_CloseDataHandle( p_handle );

  // ワイプイン
  //  WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEIN, WIPE_TYPE_FADEIN, WIPE_FADE_BLACK,
  //          COMM_BRIGHTNESS_SYNC, 1, HEAPID_WIFIP2PMATCH);
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
//  GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT,
  //                            16,0,COMM_BRIGHTNESS_SYNC);

  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(FALSE);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   誰かが接続してきた場合
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _checkParentConnect(WIFIP2PMATCH_WORK *wk)
{
 if( GFL_NET_StateGetWifiStatus()==GFL_NET_STATE_MATCHED ){
    return TRUE;
  }
  return FALSE;
}

static BOOL _checkParentNewPlayer( WIFIP2PMATCH_WORK *wk )
{
  if( (GFL_NET_DWC_IsNewPlayer() != -1) ){
    // ボイスチャットとBGM音量の関係を整理 tomoya takahashi
    // ここでVCHATをONにしないと
    // DWC_RAP内でVCHATを開始しなし
    GFL_NET_DWC_SetVChat( WIFI_STATUS_GetVChatStatus( wk->pMatch ));
    NET_PRINT( "Connect VCHAT set\n" );
    return TRUE;
  }
  return FALSE;
}


//----------------------------------------------------------------------------
/**
 *  @brief  友達の出入りを管理する
 *
 *  @param  wk  ワーク
 *
 *  @retval 今の友達の数
 */
//-----------------------------------------------------------------------------
static int MCRSYS_ContFiendInOut( WIFIP2PMATCH_WORK* wk )
{
  int friend_num;
  int i, j;
  MCR_MOVEOBJ* p_obj;
  BOOL match;
  WIFI_STATUS* p_status;
  BOOL in_flag;
  BOOL out_flag;

  // 最新版の友達数と、友達ナンバー配列を作成
  friend_num = _readFriendMatchStatus( wk );
  //  wk->friendMatchReadCount = friend_num;

  in_flag = FALSE;
  out_flag = FALSE;

  // バックアップと比較する
  if( GFL_STD_MemComp( wk->index2No, wk->index2NoBackUp, sizeof(u8)*WIFIP2PMATCH_MEMBER_MAX ) != 0 ){

    // 人が変わった
    // バックアップ側にいて、最新版にいないやつはOUT
    // 最新版にいて、バックアップ版にいないやつはIN
    for( i=0; i<WIFIP2PMATCH_MEMBER_MAX; i++ ){

      // OUTチェック
      match = FALSE;
      for( j=0; j<WIFIP2PMATCH_MEMBER_MAX; j++ ){
        if( wk->index2No[j] == wk->index2NoBackUp[i] ){
          match = TRUE;
          break;
        }
      }
      // いなかったので削除
      // 一気に32人入ってきたとき0がいないという風になってしまうので
      // ０じゃないかもチェック
      if( (match == FALSE) && (wk->index2NoBackUp[i] != 0) ){
        p_obj = MCRSYS_GetMoveObjWork( wk, wk->index2NoBackUp[i] );
        if( p_obj != NULL ){
          WIFI_MCR_DelPeopleReq( &wk->matchroom, p_obj );
          MCRSYS_DelMoveObjWork( wk, p_obj );
          MCVSys_BttnDel( wk, wk->index2NoBackUp[i] );
          WifiP2PMatch_UserDispOff_Target( wk, wk->index2NoBackUp[i], HEAPID_WIFIP2PMATCH );  // その人がした画面に詳細表示されているなら消す
          out_flag = TRUE;
        }
      }

      // INチェック
      match = FALSE;
      for( j=0; j<WIFIP2PMATCH_MEMBER_MAX; j++ ){
        if( wk->index2No[i] == wk->index2NoBackUp[j] ){
          match = TRUE;
          break;
        }
      }
      // 前はいなかったので追加
      if( (match == FALSE) && (wk->index2No[i] != 0) ){
        p_status = WifiFriendMatchStatusGet( wk->index2No[i]-1 );

        p_obj = WIFI_MCR_SetNpc( &wk->matchroom,
                                 WIFI_STATUS_GetTrainerView(p_status), wk->index2No[i] );


        // 登録できたかチェック
        if( p_obj ){

          MCVSys_BttnSet( wk, wk->index2No[i], MCV_BTTN_FRIEND_TYPE_IN );
          MCRSYS_SetMoveObjWork( wk, p_obj );

          in_flag = TRUE;
        }else{

          // オブジェが登録できなかったので予約登録
          // （これだとボタンを押すことが出来ない）
          MCVSys_BttnSet( wk, wk->index2No[i], MCV_BTTN_FRIEND_TYPE_RES );
          wk->index2No[i] = 0;  // 入ってきたのはなかったことにする
        }
      }
    }

    // 最新版に変更
    // memcpy( wk->index2NoBackUp, wk->index2No, sizeof(u8)*WIFIP2PMATCH_MEMBER_MAX );
    GFL_STD_MemCopy(  wk->index2No, wk->index2NoBackUp,sizeof(u8)*WIFIP2PMATCH_MEMBER_MAX );

    // ボタン全描画リクエスト
    MCVSys_BttnAllWriteReq( wk );

    if( in_flag == TRUE ){
      Snd_SePlay( _SE_INOUT );
    }else if( out_flag == TRUE ){
      Snd_SePlay( _SE_INOUT );
    }
  }


  return friend_num;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達の状態変化を表示
 *
 *  @param  wk  ワーク
 */
//-----------------------------------------------------------------------------
static void MCRSYS_ContFriendStatus( WIFIP2PMATCH_WORK* wk, u32 heapID )
{
  int i;
  u32 friendID;
  u32 type;
  BOOL vchat;
  int change_num;

  // 状態変更数を取得（今までと動きを一緒にするためにこうしておく）
  change_num = _checkUserDataMatchStatus( wk );

  // 皆の状態変化を登録する
  if( change_num > 0 ){
    MCVSys_ReWrite( wk, heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達オブジェクトの登録破棄管理
 *
 *  @param  wk  ワーク
 */
//-----------------------------------------------------------------------------
static void MCRSYS_SetFriendObj( WIFIP2PMATCH_WORK* wk, u32 heapID )
{
  // 友達ナンバーが変わっていたらいなくなった友達を消し、増えた友達を登録
  MCRSYS_ContFiendInOut( wk );

  // 状態が変わったら何かを変える
  MCRSYS_ContFriendStatus( wk, heapID );
}


//------------------------------------------------------------------
/**
 * $brief   フレンドリスト表示中処理 WIFIP2PMATCH_MODE_FRIENDLIST
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int WifiP2PMatch_FriendList( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 ret = MCR_RET_NONE;
  int checkMatch;
  u32 check_friend;
  MCR_MOVEOBJ* p_obj;
  u32 status,gamemode;


  // ワイプ終了待ち
  if( !WIPE_SYS_EndCheck() ){
    return seq;
  }

  // エラーチェック
  if( GFL_NET_StateIsWifiError()){
    //        wk->localTime=0;
    _errorDisp(wk);
    return seq;
  }

  // ボイスチャットONOFF繰り返しでここの画面ではまる為
  // えら状態になったらDISCONNECT処理に遷移
  if( GFL_NET_StateGetWifiStatus()==GFL_NET_STATE_DISCONNECTING ){
    // この処理は、_parentModeCallMenuInitからもってきました
    // 適合しない=>切断処理へ
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
    _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
    GFL_NET_StateWifiMatchEnd(TRUE);
    return seq;
  }

  // 友達が現れたら出す
  MCRSYS_SetFriendObj( wk, HEAPID_WIFIP2PMATCH );
  MCVSys_UpdataBttn( wk );


  // パソコンアニメが動いていたら終了
  WIFI_MCR_PCAnmOff( &wk->matchroom );

  // ボイスチャットのONOFFはプレイヤーが動けるときにだけ設定できる
  if( WIFI_MCR_PlayerMovePauseGet( &wk->matchroom ) == FALSE ){
    // ボイスチャット
    if(GFL_NET_DWC_IsNewPlayer() == -1){
      if(PAD_BUTTON_X & GFL_UI_KEY_GetTrg()){
        if(_myVChatStatusToggleOrg(wk)){
          WifiP2PMatchMessagePrint(wk, msg_wifilobby_054, FALSE);
        }
        else{
          WifiP2PMatchMessagePrint(wk, msg_wifilobby_055, FALSE);
        }
        Snd_SePlay(_SE_DESIDE);
        _userDataInfoDisp(wk);
        _CHANGESTATE(wk,WIFIP2PMATCH_VCHATWIN_WAIT);
        WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );  // した画面初期化
        return seq;
      }
    }


    // した画面も動かない
    check_friend = MCVSys_Updata( wk, HEAPID_WIFIP2PMATCH );  // した画面も動かす
    p_obj = MCRSYS_GetMoveObjWork( wk, check_friend );
    if( check_friend != 0 ){
      if( p_obj ){
        WIFI_MCR_CursorOn( &wk->matchroom, p_obj );
      }else{
        WIFI_MCR_CursorOff( &wk->matchroom );
      }
    }else{
      WIFI_MCR_CursorOff( &wk->matchroom );
    }

  }

  {
    int j;
    for(j = 0; j < WIFIP2PMATCH_MEMBER_MAX ; j++){
      int n = GFUser_GetPublicRand0(WIFIP2PMATCH_MEMBER_MAX);  //いつも若い順になら無いように乱数検査
      if(WIFI_STATUS_IsVChatMac(wk->pMatch, WifiFriendMatchStatusGet( n ))){
        //自分が呼び出されているので、接続開始 状態を取得
        status = _WifiMyStatusGet( wk, WifiFriendMatchStatusGet( n ) );
        gamemode = _WifiMyGameModeGet( wk, WifiFriendMatchStatusGet( n ) );
        wk->friendNo = n + 1;

        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALLGAME_INIT);
        return seq;

      }
    }
  }


  // 友達からこちらに接続してきたときの処理
  checkMatch = _checkParentConnect(wk);
  if( (0 !=  checkMatch) && (wk->preConnect != -1) ){ // 接続してきた
    NET_PRINT("接続\n");
    Snd_SePlay(_SE_OFFER);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_INIT);
    return seq;
  }
  if((wk->preConnect == -1) && (GFL_NET_DWC_IsNewPlayer() != -1)){  // 通常のコネクト開始
    wk->preConnect = GFL_NET_DWC_IsNewPlayer();
    _friendNameExpand(wk, wk->preConnect);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_043, FALSE);

    // ボイスチャット設定
    GFL_NET_DWC_SetVChat(WIFI_STATUS_GetVChatStatus( wk->pMatch ));// ボイスチャットとBGM音量の関係を整理 tomoya takahashi
    NET_PRINT( "Connect VCHAT set\n" );

    // つながった人のデータ表示
    //    WifiP2PMatch_UserDispOn( wk, wk->preConnect+1, HEAPID_WIFIP2PMATCH );

    wk->localTime = 0;
  }
  else if((wk->preConnect != -1) && (GFL_NET_DWC_IsNewPlayer() == -1)){ // 自分的には新しくコネクトしてきたのに、実際はコネクトしてきていなかったとき？

    // ボイスチャット設定
    GFL_NET_DWC_SetVChat(FALSE);// ボイスチャットとBGM音量の関係を整理 tomoya takahashi
    _friendNameExpand(wk, wk->preConnect);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    GFL_NET_StateWifiMatchEnd(TRUE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
    return seq;
  }

  // VCHAT　ON　状態なのに、新しいコネクトが-1ならVCHATをOFFにする
  // 080615 tomoya
  if( (GFL_NET_DWC_IsVChat() == TRUE) && (GFL_NET_DWC_IsNewPlayer() == -1) ){
    GFL_NET_DWC_SetVChat(FALSE);// ボイスチャットとBGM音量の関係を整理 tomoya takahashi
  }


  // 状態を取得
  status = _WifiMyStatusGet( wk, wk->pMatch );

  // 誰も自分に接続してきていないときだけリストを動かせる
  if(wk->preConnect == -1){

    // CANCELボタンでも待機状態をクリア
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_CANCEL ){
      if(_modeWait(status)){  // 待ち状態のとき
        Snd_SePlay(_SE_DESIDE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_INIT);
        WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );  // した画面初期化
        return seq;
      }
    }

    ret = WIFI_MCR_Main( &wk->matchroom );
    WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
    //        ret = BmpListMain(wk->lw);
  }
  switch(ret){
  case MCR_RET_NONE:
    return seq;
  case MCR_RET_CANCEL:
    Snd_SePlay(_SE_DESIDE);
    if(_modeWait(status)){  // 待ち状態のとき
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_INIT);
    }
    else{ // それか終了チェックへ
      wk->endSeq = WIFI_GAME_NONE;
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXIT_YESNO);
      WifiP2PMatchMessagePrint(wk, dwc_message_0010, TRUE);
      //        wk->localTime=0;
      ret = BMPMENULIST_CANCEL;
    }
    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );  // した画面初期化
    return seq;

  case MCR_RET_MYSELECT:
    //        wk->localTime=0;
    Snd_SePlay(_SE_DESIDE);
    if(_modeWait(status)){
      WIFI_MCR_PCAnmStart( &wk->matchroom );  // pcアニメ開始
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_INIT);
    }
    else{  // 募集の行で選択したとき
      if(status == WIFI_STATUS_WAIT){
        WIFI_MCR_PCAnmStart( &wk->matchroom );  // pcアニメ開始
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_INIT);
      }
    }
    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );  // した画面初期化
    return seq;
    break;

  case MCR_RET_SELECT:
    Snd_SePlay(_SE_DESIDE);
    if(_modeWait(status)){
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_INIT);
    }
    else{  // 人の名前ー＞マッチングへ
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_MATCH_INIT);
    }
    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );  // した画面初期化
    return seq;
    break;

  default:
    GF_ASSERT(0);
    break;
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   話しかけられた場合の確認画面に   WIFIP2PMATCH_MODE_CALLGAME_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _callGameInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  // だれかが話しかけてきた
  int n = wk->friendNo -1;

  if(n>=0 && n < WIFIP2PMATCH_MEMBER_MAX){
    if( WifiP2PMatch_CommWifiBattleStart( wk, n ) ){
      wk->cancelEnableTimer = _CANCELENABLE_TIMER;

      _commStateChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_UNIONMATCH);
      _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_UNIONMATCH);  // 接続中になる

      _friendNameExpand(wk, n);
      WifiP2PMatchMessagePrint(wk,msg_wifilobby_082, FALSE);
      wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
      wk->cancelEnableTimer = _CANCELENABLE_TIMER;
      _CHANGESTATE(wk, WIFIP2PMATCH_MODE_MATCH_LOOP);
      return seq;
    }
  }

  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST); //もとに戻る

  return seq;

}

//------------------------------------------------------------------
/**
 * $brief   VCTONOFFウインドウを閉じる  WIFIP2PMATCH_VCHATWIN_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _vchatToggleWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_UI_KEY_GetTrg()){
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    // 主人公の動作を許可
    FriendRequestWaitOff( wk );
    EndMessageWindowOff(wk);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCT接続開始
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnectInit2( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_VCT_CONNECT_INIT2
{
  WIFI_STATUS* p_status;
  int vchat;
  int status;

  // VChatフラグを取得
  p_status = WifiFriendMatchStatusGet( wk->friendNo - 1 );
  vchat = WIFI_STATUS_GetVChatStatus(p_status);
  status  = _WifiMyStatusGet( wk, p_status );

  wk->cancelEnableTimer--;
  if(wk->cancelEnableTimer < 0  ){
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CANCEL_ENABLE_WAIT);
  }
  else if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_FAIL){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_012, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT ){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if((GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_DISCONNECTING) || (GFL_NET_StateIsWifiDisconnect())){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_016, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
  }
#if PLFIX_T869
  // VCHATフラグがOFFになっていたら通信切断
  else if(vchat == FALSE){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
#endif
  // 相手の状態が通常状態でない場合切断
  else if( (status != WIFI_STATUS_WAIT) && (status != WIFI_STATUS_RECRUIT) ){
    OS_TPrintf("status not loginwait %d\n",status);
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateGetWifiStatus()==GFL_NET_STATE_MATCHED){  // つながった
    //    _timeWaitIconDel(wk);   timeWait内でMsgWinを破棄しているということはメッセージ終了でもOK↓
    EndMessageWindowOff(wk);
    // VCT接続開始 + 接続MACは消去
    WIFI_STATUS_ResetVChatMac(wk->pMatch);
    _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_VCT);  // VCT中になる
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT);
    WifiList_SetLastPlayDate( wk->pList, wk->friendNo - 1); // 最後に遊んだ日付は、VCTがつながったときに設定する
  }

  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCT接続開始
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnectInit( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_VCT_CONNECT_INIT
{

  // 080703 tomoya takahashi
  // やはりはまるときはあるようなので、
  //　すぐに次にいくように修正する
  _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
  WifiP2PMatchMessagePrint(wk, msg_wifilobby_011, FALSE);
  _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_VCT);  // VCT中になる
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT_WAIT);

  WifiList_SetLastPlayDate( wk->pList, GFL_NET_DWC_GetFriendIndex()); // 最後に遊んだ日付は、VCTがつながったときに設定する

#if 0
  if(GFL_NET_StateGetWifiStatus()==GFL_NET_STATE_MATCHED){  // つながった
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_011, FALSE);
    _myStatusChange(wk, WIFI_STATUS_PLAYING, WIFI_GAME_VCT);  // VCT中になる
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT_WAIT);
  }
#endif

  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCT接続まち
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnectWait( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_VCT_CONNECT_WAIT
{

  WIFI_STATUS* p_status;
  int vchat;
  int status;

  if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
    return seq;
  }

  // 080624　追加
  // 通信リセットされるとindexが0より小さくなる
  // そしたら切断
  if( GFL_NET_DWC_GetFriendIndex() < 0 ){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_016, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
    return seq;
  }

  if( WifiP2PMatchMessageEndCheck(wk) ){
    if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
      EndMessageWindowOff(wk);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT);
    }
    else{
      WifiP2PMatch_VCTConnect(wk,seq);
    }
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCT接続中  WIFIP2PMATCH_MODE_VCT_CONNECT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnect( WIFIP2PMATCH_WORK *wk, int seq )
{
  WIFI_STATUS* p_status;
  int status,gamemode;


  // 通信リセットされるとindexが0より小さくなる
  // そしたら切断
  if( GFL_NET_DWC_GetFriendIndex() < 0 ){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_016, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
    return seq;
  }

  // VChatフラグを取得
  p_status = WifiFriendMatchStatusGet( GFL_NET_DWC_GetFriendIndex() );
  status  = _WifiMyStatusGet( wk, p_status );
  gamemode  = _WifiMyGameModeGet( wk, p_status );

  // 友達が現れたら出す
  MCRSYS_SetFriendObj( wk, HEAPID_WIFIP2PMATCH );
  MCVSys_UpdataBttn( wk );

#if 0
  CLACT_SetDrawFlag( wk->MainActWork[_CLACT_VCT_MOVE], 0 );
  CLACT_SetDrawFlag( wk->MainActWork[_CLACT_VCT_STOP], 0 );
#endif
  if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_FAIL){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_012, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT ){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if((GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_DISCONNECTING) || (GFL_NET_StateIsWifiDisconnect())){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_016, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
  }
  // 相手の状態がVCT状態でない場合切断
  //  else if( gamemode != WIFI_GAME_VCT ){
  //    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
  //    WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
  //    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  //  }
  else if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_017, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECTEND_YESNO);
  }
  else{
    // アイコン書き換え
    if(GFL_NET_DWC_IsSendVoiceAndInc()){
      WifiP2PMatchFriendListIconWrite(   &wk->icon, GFL_BG_FRAME1_M,
                                         PLAYER_DISP_ICON_POS_X, PLAYER_DISP_ICON_POS_Y, PLAYER_DISP_ICON_IDX_VCTBIG, 0 );
    }
    else{
      WifiP2PMatchFriendListIconWrite(   &wk->icon, GFL_BG_FRAME1_M,
                                         PLAYER_DISP_ICON_POS_X, PLAYER_DISP_ICON_POS_Y, PLAYER_DISP_ICON_IDX_VCT, 0 );
    }
  }

  return seq;
}

//------------------------------------------------------------------
/**
 * @brief   VCT接続終了待ち  WIFIP2PMATCH_MODE_VCT_CONNECTEND_YESNO
 * @param   wk
 * @retval  seq
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnectEndYesNo( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
    return seq;
  }

  //Snd_PlayerSetInitialVolume( SND_HANDLE_BGM, PV_VOL_DEFAULT/3 );
  if( WifiP2PMatchMessageEndCheck(wk)){
    // 最後に遊んだ日付は、VCTがつながったときに設定する
    WifiList_SetLastPlayDate( wk->pList, GFL_NET_DWC_GetFriendIndex());
    // はいいいえウインドウを出す

    _yenowinCreateM2(wk);

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECTEND_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCT接続終了YESNO待ち  WIFIP2PMATCH_MODE_VCT_CONNECTEND_WAIT
 * @param   wk
 * @retval  seq
 */
//------------------------------------------------------------------
static int WifiP2PMatch_VCTConnectEndWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);

  if(ret == BMPMENU_NULL){  // まだ選択中
    if((GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_DISCONNECTING)
       || GFL_NET_StateIsWifiDisconnect()
       || !GFL_NET_IsConnectMember(GFL_NET_NETID_SERVER)){

      // ユーザーデータOFF
      //      WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );
      //
      EndMessageWindowOff(wk);

      _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
      BmpMenu_YesNoMenuExit( wk->pYesNoWork );
      wk->preConnect = -1;
      wk->timer = _RECONECTING_WAIT_TIME;
      _CHANGESTATE(wk,WIFIP2PMATCH_RECONECTING_WAIT);
    }
    else if( GFL_NET_StateIsWifiError() ){
      _errorDisp(wk);
    }
    return seq;
  }
  else if(ret == 0)
  { // はいを選択した場合

    if(!GFL_NET_StateIsWifiError()){
      _myStatusChange(wk, WIFI_STATUS_WAIT,WIFI_GAME_LOGIN_WAIT);
      //GFL_NET_StateWifiBattleMatchEnd();
      GFL_NET_StateWifiMatchEnd(TRUE);
      wk->preConnect = -1;
      wk->timer = _RECONECTING_WAIT_TIME;
      _CHANGESTATE(wk,WIFIP2PMATCH_RECONECTING_WAIT);
    }
    else
    {
      _errorDisp(wk);
    }
  }
  else
  {  // いいえを選択した場合
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT);
  }
  EndMessageWindowOff(wk);
  return seq;
}



//WIFIP2PMATCH_MODE_VCT_DISCONNECT
static int WifiP2PMatch_VCTDisconnect(WIFIP2PMATCH_WORK *wk, int seq)
{
  u32 status;

  if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
    return seq;
  }


  // ユーザーデータOFF
  //  WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );

  status = _WifiMyStatusGet( wk, wk->pMatch );

  if(status != WIFI_STATUS_WAIT){
    _myStatusChange(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
  }
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    wk->timer = 30;
    return seq;
  }
  wk->timer--;
  if((GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)) || (wk->timer==0)){
    EndMessageWindowOff(wk);
    GFL_NET_StateWifiMatchEnd(TRUE);

    // 通信状態を元に戻す
    //GFL_NET_StateChangeWiFiLogin();
    //    GFL_NET_ChangeInitStruct(&aGFLNetInit);
    _GFL_NET_InitAndStruct(wk,FALSE);

    //        if(wk->menulist==NULL){
    if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == FALSE ){
      wk->timer = _RECONECTING_WAIT_TIME;
      _CHANGESTATE(wk,WIFIP2PMATCH_RECONECTING_WAIT);
    }
    else{

      // VCHATを元に戻す
      _myVChatStatusOrgSet( wk );
      _userDataInfoDisp(wk);

      wk->preConnect = -1;
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

      // 主人公の動作を許可
      FriendRequestWaitOff( wk );
    }
  }
  return seq;
}

//WIFIP2PMATCH_MODE_BATTLE_DISCONNECT
static int WifiP2PMatch_BattleDisconnect(WIFIP2PMATCH_WORK *wk, int seq)
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
    EndMessageWindowOff(wk);
    _myStatusChange(wk,  WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
    GFL_NET_StateWifiMatchEnd(TRUE);
    wk->timer = _RECONECTING_WAIT_TIME;
    _CHANGESTATE(wk,WIFIP2PMATCH_RECONECTING_WAIT);
  }
  return seq;
}

//WIFIP2PMATCH_MODE_DISCONNECT
static int WifiP2PMatch_Disconnect(WIFIP2PMATCH_WORK *wk, int seq)
{
  // ユーザーデータOFF
  //  WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );
  //
  // エラーチェック
  if( GFL_NET_StateIsWifiError() ){
    _errorDisp(wk);
    return seq;
  }


  if( !WifiP2PMatchMessageEndCheck(wk) ){
    wk->timer = 30;
    return seq;
  }
  wk->timer--;
  if((GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)) || (wk->timer==0)){
    EndMessageWindowOff(wk);
    _myStatusChange(wk,  WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);

    GFL_NET_StateWifiMatchEnd(TRUE);
    // 通信状態を元に戻す
    //GFL_NET_StateChangeWiFiLogin();
    //    GFL_NET_ChangeInitStruct(&aGFLNetInit);
    _GFL_NET_InitAndStruct(wk,FALSE);

    //        if(wk->menulist==NULL){
    if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == FALSE ){
      _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
    }
    else{

      // VCHATをオリジナルに戻す
      _myVChatStatusOrgSet( wk );
      _userDataInfoDisp(wk);

      wk->preConnect = -1;
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
      // 主人公の動作を許可
      FriendRequestWaitOff( wk );
    }
  }
  return seq;
}

static int WifiP2PMatch_BattleConnectInit( WIFIP2PMATCH_WORK *wk, int seq )    // WIFIP2PMATCH_MODE_BATTLE_CONNECT_INIT
{
  return seq;
}

static int WifiP2PMatch_BattleConnectWait( WIFIP2PMATCH_WORK *wk, int seq )     // WIFIP2PMATCH_MODE_BATTLE_CONNECT_WAIT
{
  return seq;
}

static int WifiP2PMatch_BattleConnect( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_BATTLE_CONNECT
{
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   カーソル位置を変更する
 *
 * @param   act   アクターのポインタ
 * @param   x
 * @param   y
 *
 * @retval  none
 */
//------------------------------------------------------------------
static void SetCursor_Pos( GFL_CLWK* act, int x, int y )
{
  GFL_CLACTPOS clp;

  clp.x = x;
  clp.y = y;
  GFL_CLACT_WK_SetWldPos(act ,&clp);

}

//------------------------------------------------------------------
/**
 * $brief   ボタンを押すと終了  WIFIP2PMATCH_MODE_CHECK_AND_END
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int WifiP2PMatch_CheckAndEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( wk->timer > 0 ){
    wk->timer --;
    return seq;
  }

  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){


    WifiP2PMatchMessagePrint(wk, dwc_message_0011, TRUE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXITING);
    wk->timer = 1;

    /*
        wk->endSeq = WIFI_P2PMATCH_END;
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_END_WAIT);
        GFL_NET_StateWifiLogout();
//*/
    }
    return seq;
}



//------------------------------------------------------------------
/**
 * $brief   募集の取り消し WIFIP2PMATCH_MODE_SELECT_REL_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _parentModeSelectRelInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  WifiP2PMatchMessagePrint(wk, msg_wifilobby_007, FALSE);
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_YESNO);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   募集の取り消し  WIFIP2PMATCH_MODE_SELECT_REL_YESNO
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _parentModeSelectRelYesNo( WIFIP2PMATCH_WORK* wk, int seq )
{
  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  if( WifiP2PMatchMessageEndCheck(wk) ){
    // はいいいえウインドウを出す
    _yenowinCreateM2(wk);

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_REL_WAIT);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   募集の取り消し  WIFIP2PMATCH_MODE_SELECT_REL_WAIT
 * @param   wk
 * @param   seq
 * @retval  int
 */
//------------------------------------------------------------------

static int _parentModeSelectRelWait( WIFIP2PMATCH_WORK* wk, int seq )
{
  int i;
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン

  if((wk->preConnect == -1) && (GFL_NET_DWC_IsNewPlayer() != -1)){  // 接続があった

    // ボイスチャットとBGM音量の関係を整理 tomoya takahashi
    GFL_NET_DWC_SetVChat(WIFI_STATUS_GetVChatStatus( wk->pMatch ));
    NET_PRINT( "Connect VCHAT set\n" );

    // すでにYesNoSelectMainで解放されてなければ
    if( ret == BMPMENU_NULL ){
      BmpMenu_YesNoMenuExit( wk->pYesNoWork );
    }
    ret = 1;  // いいえに変更
  }
  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }else if(ret == 0){ // はいを選択した場合
    //書き込み
    _myStatusChange(wk,  WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
    GFL_NET_StateWifiMatchEnd(TRUE);
    // 通信状態を元に戻す
    _GFL_NET_InitAndStruct(wk,FALSE);

    // 主人公の動作を許可
    FriendRequestWaitOff( wk );
  }
  else{  // いいえを選択した場合

    // 主人公動作停止を再度表示
    FriendRequestWaitOff( wk );
    FriendRequestWaitOn( wk, TRUE );
  }
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   待機状態になる為の選択メニュー WIFIP2PMATCH_MODE_SELECT_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeSelectMenuInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  _ParentModeSelectMenu(wk);
  WifiP2PMatchMessagePrint(wk, msg_wifilobby_006, FALSE);
  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_WAIT);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   待機状態になる為の選択メニュー WIFIP2PMATCH_MODE_SELECT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeSelectMenuWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 ret;
  int num = PokeParty_GetBattlePokeNum(wk->pMyPoke);

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  ret = BmpMenuList_Main(wk->sublw);

  if( 0 !=  _checkParentNewPlayer(wk)){ // 接続してきた場合はメニューを閉じた扱いにして次
    ret = BMPMENULIST_CANCEL;
  }

  if(ret != BMPMENULIST_NULL ){
    wk->SubListWin = _BmpWinDel(wk->SubListWin);
    BmpMenuList_Exit(wk->sublw, NULL,  &wk->singleCur[_MENUTYPE_GAME]);
    BmpMenuWork_ListDelete( wk->submenulist );
    EndMessageWindowOff(wk);
  }

  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    Snd_SePlay(_SE_DESIDE);
    FriendRequestWaitOff(wk);
    return seq;
    break;
  case WIFI_GAME_BATTLE_SINGLE_ALL:
    Snd_SePlay(_SE_DESIDE);
    _battleCustomSelectMenu(wk);
    _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_BATTLE2);
    return seq;
  case WIFI_GAME_VCT:
  case WIFI_GAME_TRADE:
    Snd_SePlay(_SE_DESIDE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    break;
  case WIFI_GAME_TVT:
    Snd_SePlay(_SE_DESIDE);
#if defined(SDK_TWL)
    if(OS_IsRunOnTwl() && OS_IsRestrictPhotoExchange()){   // ペアレンタルコントロール
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_1012, FALSE);
      _CHANGESTATE(wk, WIFIP2PMATCH_MESSAGEEND_RETURNLIST);
    }
#else
    if(0){
    }
#endif
    else{
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    }
    break;
  }
  WifiP2PMatch_CommWifiBattleStart( wk, -1 );
  _myStatusChange(wk, WIFI_STATUS_RECRUIT, ret );
//  _commStateChange(wk, WIFI_STATUS_RECRUIT, ret );
  if( wk->seq == WIFIP2PMATCH_MODE_FRIENDLIST ){
    FriendRequestWaitOn( wk, TRUE );       // 動作停止させる
  }
  return seq;

}




//------------------------------------------------------------------
/**
 * $brief   リストに戻る  WIFIP2PMATCH_MESSAGEEND_RETURNLIST
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int MessageEndReturnList( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
    EndMessageWindowOff(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
}


//------------------------------------------------------------------
/**
 * $brief   ゲームの内容が決まったので、親として再初期化  WIFIP2PMATCH_PARENT_RESTART
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentRestart( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 ret;

  // エラーチェック
  if( GFL_NET_StateIsWifiError() ){
    _errorDisp(wk);
    return seq;
  }

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン
  if( 0 !=  _checkParentNewPlayer(wk)){ // 接続してきた

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
  }
  if(GFL_NET_StateIsWifiLoginMatchState()){
    GFL_NET_SetAutoErrorCheck(FALSE);
    GFL_NET_SetNoChildErrorCheck(TRUE);

    GFL_NET_SetWifiBothNet(FALSE);
    //        WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK,
    //                      COMM_BRIGHTNESS_SYNC, 1, HEAPID_WIFIP2PMATCH);
    WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                    WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
//    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT,
  //                              0,16,COMM_BRIGHTNESS_SYNC);

    wk->bRetryBattle = FALSE;

    //        CommInfoInitialize( wk->pSaveData, NULL );   //@@OO
    CommCommandWFP2PMF_MatchStartInitialize(wk);

    // 自分を教える
    //        CommInfoSendPokeData();  //@@OO

    // 自分はエントリー
    //        CommInfoSetEntry( GFL_NET_SystemGetCurrentID() );    //@@OO
    seq = SEQ_OUT;            //終了シーケンスへ
  }
  return seq;

}





//------------------------------------------------------------------
/**
 * $brief   待機状態になる為の選択メニュー WIFIP2PMATCH_MODE_SUBBATTLE_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeSubSelectMenuWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 ret;

  WIFI_MCR_PCAnmMain( &wk->matchroom ); // パソコンアニメメイン

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  ret = BmpMenuList_Main(wk->sublw);
  switch(ret){
  case BMPMENULIST_NULL:
    return seq;
  case BMPMENULIST_CANCEL:
    Snd_SePlay(_SE_DESIDE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_SELECT_INIT);
    break;
  default:
    Snd_SePlay(_SE_DESIDE);
    _myStatusChange(wk, WIFI_STATUS_RECRUIT ,ret);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    EndMessageWindowOff(wk);
    break;
  }


  //  {
  if( wk->seq == WIFIP2PMATCH_MODE_FRIENDLIST ){  // 何か選択したときのみ、CANCELはだめ
    BOOL msg_on = TRUE;
    WifiP2PMatch_CommWifiBattleStart( wk, -1 );
    // 動作停止させる
    FriendRequestWaitOn( wk, msg_on );
  }



  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, &wk->singleCur[wk->bSingle]);
  BmpMenuWork_ListDelete( wk->submenulist );
  return seq;

}

//------------------------------------------------------------------
/**
 * $brief   マッチング申し込み WIFIP2PMATCH_MODE_MATCH_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

#if 1
static int _childModeMatchMenuInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  int gmmno,gmmidx;
  u16 friendNo,status,gamemode;
  WIFI_STATUS* p_status;
  MCR_MOVEOBJ* p_player;
  MCR_MOVEOBJ* p_npc;
  u32 way;

  GFL_NET_SetClientConnect(GFL_NET_HANDLE_GetCurrentHandle(),FALSE);  //接続禁止

  friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
  NET_PRINT("_childModeMatchMenuInit %d\n",friendNo);

  //NPCを自分の方向に向ける
  p_player = MCRSYS_GetMoveObjWork( wk, 0 );
  GF_ASSERT( p_player );

  p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );
  // 相手がいなくなったり
  // 相手のステータスが違うものに変わったら、
  // 表示を消して元に戻る
  if( p_npc == NULL ){
    _friendNameExpand(wk, friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
    return seq;
  }

  way = WIFI_MCR_GetRetWay( p_player );
  WIFI_MCR_NpcPauseOn( &wk->matchroom, p_npc, way );


  // 相手の状態を得る
  p_status = WifiFriendMatchStatusGet( friendNo - 1 );
  GFL_STD_MemCopy(p_status, &wk->targetStatus, sizeof(WIFI_STATUS));

  status = _WifiMyStatusGet( wk, p_status );
  gamemode = _WifiMyGameModeGet( wk, p_status );
  //  wk->keepStatus = status;


  _friendNameExpand(wk, friendNo - 1);

  if(WIFI_GAME_TRADE == gamemode){
    gmmidx = 0;
  }
  else if(WIFI_GAME_TVT == gamemode){
    gmmidx = 1;
  }
  else if(WIFI_GAME_VCT == gamemode){
    gmmidx = 2;
  }
  else if(_modeIsBattleStatus(gamemode)){
    gmmidx = 3;
  }

  switch(status){
  case WIFI_STATUS_NONE:
    gmmno = msg_wifilobby_077;
    break;
  case WIFI_STATUS_WAIT:   // 待機中
    gmmno = msg_wifilobby_005;
    break;
  case WIFI_STATUS_RECRUIT:    // 募集中
    {
      u32 msgbuff[]={msg_wifilobby_004,msg_wifilobby_080,msg_wifilobby_078,msg_wifilobby_003};
      gmmno = msgbuff[gmmidx];
    }
    break;
  case WIFI_STATUS_PLAYING:      // 他の人と接続中
    {
      u32 msgbuff[]={msg_wifilobby_049,msg_wifilobby_079,msg_wifilobby_050,msg_wifilobby_048};
      gmmno = msgbuff[gmmidx];
    }
    break;
  case WIFI_STATUS_PLAY_AND_RECRUIT:      // 他の人と接続中でさらに募集中
    gmmno = msg_wifilobby_077;
    break;
  default://WIFI_STATUS_UNKNOWN
    gmmno = msg_wifilobby_077;
    break;
  }

  WifiP2PMatchMessagePrint(wk, gmmno, FALSE);
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_MATCH_INIT2);
  return seq;
}

#else
#endif

//----------------------------------------------------------------------------
/**
 *  @brief  メッセージの終了を見て、リストを出します WIFIP2PMATCH_MODE_MATCH_INIT2
 */
//-----------------------------------------------------------------------------
static int _childModeMatchMenuInit2( WIFIP2PMATCH_WORK *wk, int seq )
{
  MCR_MOVEOBJ* p_npc;
  u32 checkMatch;
  int friendNo;
  u32 status,gamemode;
  WIFI_STATUS* p_status;


  // エラーチェック
  if( GFL_NET_StateIsWifiError() ){
    _errorDisp(wk);
    return seq;
  }

  // 話しかけている友達ナンバー取得
  friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
  p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );

  // 相手がいなくなったり
  // 相手のステータスが違うものに変わったら、
  // 表示を消して元に戻る
  if( p_npc == NULL ){

    // NPCを元に戻す
    WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );

    BmpMenuList_Exit( wk->sublw, NULL, NULL );
    _friendNameExpand(wk, friendNo - 1);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
    return seq;
  }
  else{
    p_status = WifiFriendMatchStatusGet( friendNo - 1 );
    status = _WifiMyStatusGet( wk, p_status );
    gamemode = _WifiMyGameModeGet( wk, p_status );

    // 状態がかわったり
    if((_WifiMyGameModeGet( wk, &wk->targetStatus ) != gamemode)){

      // NPCを元に戻す
      WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );

      _friendNameExpand(wk, friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
      return seq;
    }
  }


  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  {
    p_status = WifiFriendMatchStatusGet( friendNo - 1 );
    status = _WifiMyStatusGet( wk, p_status );

#if 1
    switch(status){
    case WIFI_STATUS_RECRUIT:
    case WIFI_STATUS_PLAY_AND_RECRUIT:
      _ChildModeMatchMenuDisp(wk);
      _CHANGESTATE(wk, WIFIP2PMATCH_MODE_MATCH_WAIT);
      break;
    case WIFI_STATUS_PLAYING:
      //メッセージ出して終了
      break;
    case WIFI_STATUS_WAIT:   //相手指定で呼びかけるモードにいく
      _ChildModeMatchMenuDisp(wk);
      _CHANGESTATE(wk, WIFIP2PMATCH_MODE_PLAYERDIRECT_INIT);
    }
#endif

  }

  return seq;
}



//----------------------------------------------------------------------------
/**
 *  @brief  相手を指定して接続 WIFIP2PMATCH_MODE_PLAYERDIRECT_INIT
 */
//-----------------------------------------------------------------------------
static int _playerDirectInit( WIFIP2PMATCH_WORK *wk, int seq )
{


  u32 ret;
  int status,gamemode;
  int friendNo,message = 0,vchat,fst;
  u16 mainCursor;
  int checkMatch;
  MCR_MOVEOBJ* p_npc;
  WIFI_STATUS* p_status;

  ret = BmpMenuList_Main(wk->sublw);

  // 話しかけている友達ナンバー取得
  friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
  p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );

  // エーラーチェック
  if(GFL_NET_StateIsWifiError()){
    wk->SubListWin = _BmpWinDel(wk->SubListWin);
    BmpMenuList_Exit(wk->sublw, NULL, NULL);
    BmpMenuWork_ListDelete( wk->submenulist );

    if( p_npc != NULL ){
      WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );
    }

    _errorDisp(wk);
    return seq;
  }


  switch(ret){
  case BMPMENULIST_NULL:

    // 相手がいなくなったり
    // 相手のステータスが違うものに変わったら、
    // 表示を消して元に戻る
    if( p_npc == NULL ){
      BmpMenuList_Exit( wk->sublw, NULL, NULL );
      _friendNameExpand(wk, friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
      message = 1;
      break;
    }else{
      p_status = WifiFriendMatchStatusGet( friendNo - 1 );
      status = _WifiMyStatusGet( wk, p_status );
      vchat = WIFI_STATUS_GetVChatStatus(p_status);
      gamemode = _WifiMyGameModeGet( wk, p_status );

      // 状態がかわったり
      // 4人募集のゲームじゃないのに、VCHATフラグが変わったら切断
      if((_WifiMyGameModeGet( wk, &wk->targetStatus ) != gamemode) ||
         ( (WIFI_STATUS_GetVChatStatus( &wk->targetStatus ) != vchat)) ){

        _friendNameExpand(wk, friendNo - 1);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
        message = 1;
        break;
      }
    }
    return seq;

  case BMPMENULIST_CANCEL:
    Snd_SePlay(_SE_DESIDE);

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    break;
  default:
    Snd_SePlay(_SE_DESIDE);
    if(ret == _CONNECTING){  // MACをビーコンで流し、親になる

      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

      wk->friendNo = friendNo;
      if(  friendNo != 0 ){
        int num = PokeParty_GetBattlePokeNum(wk->pMyPoke);

        p_status = WifiFriendMatchStatusGet( friendNo - 1 );
        status = _WifiMyStatusGet( wk, p_status );
        gamemode = WIFI_GAME_UNIONMATCH;
        vchat = WIFI_STATUS_GetVChatStatus(p_status);

        if(WIFI_STATUS_UNKNOWN == status){
          break;
        }


        {
          NET_PRINT( "wifi接続先 %d %d\n", friendNo - 1,status );
          if( WifiP2PMatch_CommWifiBattleStart( wk, - 1 ) ){
            WIFI_STATUS_SetVChatMac(wk->pMatch, WifiFriendMatchStatusGet( friendNo - 1 ));

            wk->cancelEnableTimer = _CANCELENABLE_TIMER;
            _commStateChange(wk,WIFI_STATUS_CALL, gamemode);
            _myStatusChange(wk, WIFI_STATUS_CALL, gamemode);  // 呼びかけ待機中になる
            _friendNameExpand(wk, friendNo - 1);
            WifiP2PMatchMessagePrint(wk,msg_wifilobby_014, FALSE);
            GF_ASSERT( wk->timeWaitWork == NULL );
            wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );

            _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

            message = 1;
          }else{
            _friendNameExpand(wk, friendNo - 1);
            WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
            _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
            message = 1;
          }
        }
      }
    }
    else if(ret == _INFOVIEW){
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_PERSONAL_INIT);
    }
    break;
  }


  // 後始末

  if(message==0){
    EndMessageWindowOff(wk);
  }
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, NULL);
  BmpMenuWork_ListDelete( wk->submenulist );

  // NPCを元に戻す
  if( p_npc != NULL ){
    WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );
  }

  // 戻るだけなら人の情報を消す
  if( wk->seq == WIFIP2PMATCH_MODE_FRIENDLIST ){

    // その人の情報を消す
    //    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );

  }else{

    // 繋がりにいくとき

  }

  return seq;

}




//----------------------------------------------------------------------------
/**
 *  @brief  ロビー用のBGMを設定する
 */
//-----------------------------------------------------------------------------
static void WifiP2P_SetLobbyBgm( void )
{
  u32 bgm;

  // ボイスチャットなしにする
  GFL_NET_DWC_SetVChat(FALSE);    // ボイスチャットとBGM音量の関係を整理 tomoya takahashi

  if( WifiP2P_CheckLobbyBgm() == FALSE ){

    if( GFL_RTC_IsNightTime() == FALSE ){     //FALSE=昼、TRUE=夜
      bgm = SEQ_PC_01;
    }else{
      bgm = SEQ_PC_02;
    }
    Snd_SceneSet( SND_SCENE_DUMMY );
    Snd_DataSetByScene( SND_SCENE_P2P, bgm, 1 );  //wifiロビー再生
  }else{
    // もうＢＧＭ流れていても、音量だけ元に戻す
    Snd_PlayerSetInitialVolumeBySeqNo( Snd_NowBgmNoGet(), BGM_WIFILOBBY_VOL );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ロビー用のBGMが再生中か取得する
 *
 *  @retval TRUE  再生中
 *  @retval FALSE 他のBGMが再生中
 */
//-----------------------------------------------------------------------------
static BOOL WifiP2P_CheckLobbyBgm( void )
{
  u32 now_bgm;

  now_bgm = Snd_NowBgmNoGet();

  if( (now_bgm != SEQ_PC_01) &&
      (now_bgm != SEQ_PC_02) ){
    return FALSE;
  }
  return TRUE;
}

//------------------------------------------------------------------
/**
 * $brief   つなぎに行く選択メニュー WIFIP2PMATCH_MODE_MATCH_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _childModeMatchMenuWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 ret;
  int status,gamemode;
  int friendNo,message = 0,vchat,fst;
  u16 mainCursor;
  int checkMatch;
  MCR_MOVEOBJ* p_npc;
  WIFI_STATUS* p_status;

  ret = BmpMenuList_Main(wk->sublw);

  checkMatch = _checkParentNewPlayer(wk);
  if( 0 !=  checkMatch ){ // 接続してきた
    ret = BMPMENULIST_CANCEL;
  }

  // 話しかけている友達ナンバー取得
  friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
  p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );


  // エーラーチェック
  if(GFL_NET_StateIsWifiError()){
    wk->SubListWin = _BmpWinDel(wk->SubListWin);
    BmpMenuList_Exit(wk->sublw, NULL, NULL);
    BmpMenuWork_ListDelete( wk->submenulist );

    if( p_npc != NULL ){
      WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );
    }

    _errorDisp(wk);
    return seq;
  }


  switch(ret){
  case BMPMENULIST_NULL:

    // 相手がいなくなったり
    // 相手のステータスが違うものに変わったら、
    // 表示を消して元に戻る
    if( p_npc == NULL ){
      BmpMenuList_Exit( wk->sublw, NULL, NULL );
      _friendNameExpand(wk, friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
      message = 1;
      break;
    }else{
      p_status = WifiFriendMatchStatusGet( friendNo - 1 );
      status = _WifiMyStatusGet( wk, p_status );
      vchat = WIFI_STATUS_GetVChatStatus(p_status);
      gamemode = _WifiMyGameModeGet( wk, p_status );

      // 状態がかわったり
      // 4人募集のゲームじゃないのに、VCHATフラグが変わったら切断
      if((_WifiMyGameModeGet( wk, &wk->targetStatus ) != gamemode) ||
         ( (WIFI_STATUS_GetVChatStatus( &wk->targetStatus ) != vchat)) ){

        // NPCを元に戻す
        // ここでやってしまうとPauseOffを２重でよんでしまう
        //        WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );

        _friendNameExpand(wk, friendNo - 1);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
        message = 1;
        break;
      }
    }
    return seq;

  case BMPMENULIST_CANCEL:
    Snd_SePlay(_SE_DESIDE);

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    break;
  default:
    Snd_SePlay(_SE_DESIDE);
    if(ret == _CONNECTING){  // 子機が親機に接続

      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

      wk->friendNo = friendNo;
      if(  friendNo != 0 ){
        int num = PokeParty_GetBattlePokeNum(wk->pMyPoke);

        p_status = WifiFriendMatchStatusGet( friendNo - 1 );
        status = _WifiMyStatusGet( wk, p_status );
        gamemode = _WifiMyGameModeGet( wk, p_status );
        vchat = WIFI_STATUS_GetVChatStatus(p_status);

        status = _convertState(status);
        if(WIFI_STATUS_UNKNOWN == status){
          break;
        }

        //自分が子機になって相手に接続
        fst = WifiDwc_getFriendStatus(friendNo - 1);

        {
          NET_PRINT( "wifi接続先 %d %d\n", friendNo - 1,status );
          if( WifiP2PMatch_CommWifiBattleStart( wk, friendNo - 1 ) ){
            //ともだちに繋ぎに行く時はMACセットしない
            //     WIFI_STATUS_SetVChatMac(wk->pMatch, WifiFriendMatchStatusGet( friendNo - 1 ));

            wk->cancelEnableTimer = _CANCELENABLE_TIMER;
            _commStateChange(wk,WIFI_STATUS_RECRUIT, gamemode);
            _myStatusChange(wk, WIFI_STATUS_RECRUIT, gamemode);  // 接続待機中になる
            _friendNameExpand(wk, friendNo - 1);
            WifiP2PMatchMessagePrint(wk,msg_wifilobby_014, FALSE);
            GF_ASSERT( wk->timeWaitWork == NULL );
            wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
            if(gamemode != WIFI_GAME_VCT){
              _CHANGESTATE(wk,WIFIP2PMATCH_MODE_MATCH_LOOP);
            }
            else
            {
              wk->cancelEnableTimer = _CANCELENABLE_TIMER;
              _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT_INIT2);
            }
            message = 1;
          }else{
            _friendNameExpand(wk, friendNo - 1);
            WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
            _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
            message = 1;
          }
        }
      }
    }
    else if(ret == _INFOVIEW){
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_PERSONAL_INIT);
    }
    break;
  }


  // 後始末

  if(message==0){
    EndMessageWindowOff(wk);
  }
  wk->SubListWin = _BmpWinDel(wk->SubListWin);
  BmpMenuList_Exit(wk->sublw, NULL, NULL);
  BmpMenuWork_ListDelete( wk->submenulist );

  // NPCを元に戻す
  if( p_npc != NULL ){
    WIFI_MCR_NpcPauseOff( &wk->matchroom, p_npc );
  }

  // 戻るだけなら人の情報を消す
  if( wk->seq == WIFIP2PMATCH_MODE_FRIENDLIST ){

    // その人の情報を消す
    //    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );

  }else{

    // 繋がりにいくとき

  }

  return seq;
}


//WIFIP2PMATCH_MODE_MATCH_LOOP
static int _childModeMatchMenuLoop( WIFIP2PMATCH_WORK *wk, int seq )
{
  int status;


  wk->cancelEnableTimer--;
  if(wk->cancelEnableTimer < 0  ){
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CANCEL_ENABLE_WAIT);
  }
  else if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_TIMEOUT ){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_FAIL){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_012, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if((GFL_NET_StateGetWifiStatus() == GFL_NET_STATE_DISCONNECTING) || (GFL_NET_StateIsWifiDisconnect())){
    _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
  }
  else if(GFL_NET_StateGetWifiStatus()==GFL_NET_STATE_MATCHED){  // 相手に接続した

    // ２～４人募集でないとき
    status = _WifiMyStatusGet( wk, wk->pMatch );
    {

      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_YESNO);
      //    _timeWaitIconDel(wk);   timeWait内でMsgWinを破棄しているということはメッセージ終了でもOK↓
      EndMessageWindowOff(wk);
      //    CommInfoInitialize(wk->pSaveData,NULL);   //Info初期化   //@@OO
      CommCommandWFP2PMF_MatchStartInitialize(wk);
      wk->timer = 30;

    }
  }
  return seq;
}

//-------------------------------------VCTのキャンセル機能を急遽実装
//-------------------------------------VCTのキャンセル機能を急遽取り外し

//------------------------------------------------------------------
/**
 * $brief   Bキャンセル  WIFIP2PMATCH_MODE_BCANCEL_YESNO_VCT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_BCancelYesNoVCT( WIFIP2PMATCH_WORK *wk, int seq )
{
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   Bキャンセル  WIFIP2PMATCH_MODE_BCANCEL_WAIT_VCT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_BCancelWaitVCT( WIFIP2PMATCH_WORK *wk, int seq )
{
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   DWC切断 WIFIP2PMATCH_MODE_CANCEL_ENABLE_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_CancelEnableWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  _myVChatStatusOrgSet(wk);
  _myStatusChange(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);
  GFL_NET_StateWifiMatchEnd(TRUE);
  wk->preConnect = -1;
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
  // 主人公の動作を許可
  FriendRequestWaitOff( wk );
  EndMessageWindowOff(wk);
  return seq;
}


//------------------------------------------

//------------------------------------------------------------------
/**
 * $brief   Bキャンセル  WIFIP2PMATCH_MODE_BCANCEL_YESNO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_BCancelYesNo( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_VCT_CONNECT
{
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   Bキャンセル  WIFIP2PMATCH_MODE_BCANCEL_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_BCancelWait( WIFIP2PMATCH_WORK *wk, int seq )        // WIFIP2PMATCH_MODE_VCT_CONNECT
{
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   親機受付まち。子機から応募があったことを知らせる WIFIP2PMATCH_MODE_CALL_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _parentModeCallMenuInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  int mySt;
  int targetSt;
  WIFI_STATUS* p_status;
  int myvchat;

  p_status = WifiFriendMatchStatusGet( GFL_NET_DWC_GetFriendIndex() );
  mySt = _WifiMyStatusGet( wk, wk->pMatch );
  targetSt = _WifiMyStatusGet( wk,p_status );
  myvchat = WIFI_STATUS_GetVChatStatus( wk->pMatch );

  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_YESNO);

  _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
  WifiP2PMatchMessagePrint(wk, msg_wifilobby_008, FALSE);
  //   CommInfoInitialize(wk->pSaveData,NULL);   //Info初期化  //@@OO
  CommCommandWFP2PMF_MatchStartInitialize(wk);
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_YESNO);
  wk->timer = 30;
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   コネクションはろうとしている期間中のエラー表示
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static BOOL _connectingErrFunc(WIFIP2PMATCH_WORK *wk)
{
  if(GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_TIMEOUT){
    NET_PRINT("_connectingErrFunc%d \n",GFL_NET_StateGetWifiStatus());
    _friendNameExpand(wk, wk->preConnect);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(GFL_NET_StateIsWifiError()){
    _errorDisp(wk);
  }
  else{

#if 1//PL_T0857_080711_FIX
    // 友達のSTATUSとVCHATをチェック  違っていたら切断
    {
      u32 mySt;
      u32 targetSt;
      u32 MyGamemode;
      u32 TargetGamemode;
      WIFI_STATUS* p_status;
      int myvchat, targetvchat;

      p_status = WifiFriendMatchStatusGet( GFL_NET_DWC_GetFriendIndex() );
      mySt    = _WifiMyStatusGet( wk, wk->pMatch );
      targetSt= _WifiMyStatusGet( wk,p_status );

      MyGamemode    = _WifiMyGameModeGet( wk, wk->pMatch );
      TargetGamemode= _WifiMyGameModeGet( wk,p_status );
      myvchat   = WIFI_STATUS_GetVChatStatus( wk->pMatch );
      targetvchat = WIFI_STATUS_GetVChatStatus(p_status);

      //      NET_PRINT( "check mystart=%d tastatus=%d tastatus_org=%d myvchat=%d tavchat=%d \n",
      //          mySt, targetSt, targetSt_org, myvchat, targetvchat );

      if( (MyGamemode != TargetGamemode) ||   (myvchat != targetvchat) ){
        _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);

      }else{

        return FALSE;
      }
    }
#else
    return FALSE;
#endif
  }
  wk->bRetryBattle = FALSE;
  return TRUE;
}

//------------------------------------------------------------------
/**
 * $brief   つながるべきステート確認  WIFIP2PMATCH_MODE_CALL_YESNO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeCallMenuYesNo( WIFIP2PMATCH_WORK *wk, int seq )
{
  if((GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_DISCONNECTING) || GFL_NET_StateIsWifiDisconnect() ){
    NET_PRINT("DISCONNECT %d %d\n",GFL_NET_StateGetWifiStatus(),GFL_NET_StateIsWifiDisconnect());

    if(wk->bRetryBattle){
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_065, FALSE);
    }
    else{
      _friendNameExpand(wk, wk->preConnect);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE);
    }
    wk->bRetryBattle = FALSE;
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
  }
  else if(_connectingErrFunc(wk)){
  }
  else{

    if(wk->timer > 1){
      wk->timer--;
    }

    if(wk->timer==0){
      if(GFL_NET_HANDLE_IsNegotiation(GFL_NET_HANDLE_GetCurrentHandle())){
        //@@OO                CommToolTempDataReset();


        GFL_STD_MemClear(&wk->matchGameMode,sizeof(wk->matchGameMode));
        GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,_TIMING_GAME_CHECK2);
        wk->bRetryBattle = FALSE;
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_SEND);
      }
    }
    else{
      if(wk->timer == 1){
        if(GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle())!=0){
          NET_PRINT("送った %d\n",GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()));
          if( GFL_NET_HANDLE_RequestNegotiation() == TRUE ){
            wk->timer = 0;
          }
        }
        else if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
          NET_PRINT("送った %d\n",GFL_NET_GetNetID(GFL_NET_HANDLE_GetCurrentHandle()));
          if( GFL_NET_HANDLE_RequestNegotiation() == TRUE ){
            wk->timer = 0;
          }
        }
      }
    }
  }
  return seq;
}




//------------------------------------------------------------------
/**
 * $brief   MySTATUSを受信 CNM_WFP2PMF_MYSTATUS
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

void WifiP2PMatchRecvMyStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  WIFIP2PMATCH_WORK *wk = pWork;
  MYSTATUS* pTargetSt;
  const u16 *pRecvData = pData;


  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
  //０，１，２，３が通信用に入れても良い
  pTargetSt = GAMEDATA_GetMyStatusPlayer(wk->pGameData,netID);  //netIDで代入
  MyStatus_Copy(pData,pTargetSt);

  //  wk->matchGameMode[netID] = pRecvData[0];
}



//------------------------------------------------------------------
/**
 * $brief   PokePartyを受信 CNM_WFP2PMF_POPEPARTY
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

void WifiP2PMatchRecvPokeParty(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{

  //すでに受信は完了
  
}

//------------------------------------------------------------------
/**
 * $brief   PokePartyの受信ワークを返す CNM_WFP2PMF_POPEPARTY
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

u8* WifiP2PMatchGetPokePartyWork(int netID, void* pWk, int size)
{
  WIFIP2PMATCH_WORK *wk = pWk;
  return (void*)wk->pParentWork->pPokeParty[netID];
}

//------------------------------------------------------------------
/**
 * $brief   直接接続時のフローコマンドを受信 CNM_WFP2PMF_DIRECT_COMMAND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

void WifiP2PMatchRecvDirectMode(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  WIFIP2PMATCH_WORK *wk = pWork;
  MYSTATUS* pTargetSt;
  const u8 *pChangeStateNo = pData;


  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }

  if(wk->seq == WIFIP2PMATCH_PLAYERDIRECT_WAIT){
    _CHANGESTATE(wk, pChangeStateNo[0]);
  }
  else{
    GF_ASSERT(0);  // 不正コマンド受信は切断
    _CHANGESTATE(wk, WIFIP2PMATCH_MODE_DISCONNECT);
  }
  //  wk->matchGameMode[netID] = pRecvData[0];
}

//------------------------------------------------------------------
/**
 * $brief   レギュレーションを受信 CNM_WFP2PMF_DIRECT_COMMAND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

void WifiP2PMatchRecvBattleRegulation(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  WIFIP2PMATCH_WORK *wk = pWork;
  MYSTATUS* pTargetSt;
  const u32 *pRegNo = pData;


  if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
    return; //自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
  }
//  wk->matchRegulation = pRegNo[0];
  _convertRegulation(wk,pRegNo[0]);


}


//------------------------------------------------------------------
/**
 * $brief   つながるべきステートを受信 CNM_WFP2PMF_STATUS
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

void WifiP2PMatchRecvGameStatus(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
  WIFIP2PMATCH_WORK *wk = pWork;
  const u16 *pRecvData = pData;


  wk->matchGameMode[netID] = pRecvData[0];
  NET_PRINT("WifiP2PMatchRecvGameStatus %d %d\n",wk->matchGameMode[netID], netID);

}

//
//------------------------------------------------------------------
/**
 * $brief   つながるべきステートを送信  WIFIP2PMATCH_MODE_CALL_SEND
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _parentModeCallMenuSend( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(_connectingErrFunc(wk)){
  }
  else if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_GAME_CHECK2)){
    //    else if(CommIsTimingSync(_TIMING_GAME_CHECK2)){
    u16 status = _WifiMyStatusGet( wk, wk->pMatch );
    u16 gamemode = _WifiMyGameModeGet( wk, wk->pMatch );
    BOOL result = TRUE;

    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_MYSTATUS, MyStatus_GetWorkSize(), GAMEDATA_GetMyStatus(wk->pGameData));
    GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), CNM_WFP2PMF_STATUS, sizeof(u16), &gamemode);
    //@@OO        result = CommToolSetTempData(GFL_NET_SystemGetCurrentID() ,&status);
    if( result ){
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_CHECK);
    }
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   つながるべきステート解析  WIFIP2PMATCH_MODE_CALL_CHECK
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeCallMenuCheck( WIFIP2PMATCH_WORK *wk, int seq )
{
  int id=0;

  if(GFL_NET_IsParentMachine()){  //相手側のIDをみる
    id = 1;
  }
  if(_connectingErrFunc(wk)){
  }
  else if(WIFI_GAME_NONE != wk->matchGameMode[id]){  // 送られてきた
    //    u16 org_status = _WifiMyStatusGet( wk, wk->pMatch );
    //    u16 status = _convertState(org_status);
    u16 gamemode = _WifiMyGameModeGet( wk, wk->pMatch );

    NET_PRINT("_parentModeCallMenuCheck %d %d\n",wk->matchGameMode[id], gamemode);

    if(wk->matchGameMode[id] == gamemode){
      GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,_TIMING_GAME_START);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_MYSTATUS_WAIT);
    }
    else{  // 異なるステートを選択した場合
      _friendNameExpand(wk, GFL_NET_DWC_GetFriendIndex());
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_DISCONNECT);
    }
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   タイミングコマンド待ち WIFIP2PMATCH_MODE_MYSTATUS_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeCallMyStatusWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int status;

  if(_connectingErrFunc(wk)){
  }
  else if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_GAME_START)){
    //   CommInfoSendPokeData();    //@@OO
    GFL_NET_SetAutoErrorCheck(TRUE);
    GFL_NET_SetNoChildErrorCheck(TRUE);

    GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,_TIMING_GAME_START2);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_WAIT);
  }
  return seq;
}


static BOOL _parent_MsgEndCheck( WIFIP2PMATCH_WORK *wk )
{
  if(WifiP2PMatchMessageEndCheck(wk)){
    return TRUE;
  }

  if( !wk->MsgWin ){
    return TRUE;
  }

  return FALSE;
}

//------------------------------------------------------------------
/**
 * $brief   タイミングコマンド待ち WIFIP2PMATCH_MODE_CALL_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static int _parentModeCallMenuWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  u32 status,gamemode;
  int checkFriend[GFL_NET_MACHINE_MAX];
  WIFI_STATUS* p_status;

  if(_connectingErrFunc(wk)){
  }
  else if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_GAME_START2)
          && (_parent_MsgEndCheck( wk ) == TRUE) ){   // メッセージの終了も待つように変更 08.06.01  tomoya
    GFL_NET_DWC_FriendAutoInputCheck( WifiList_GetDwcDataPtr(GAMEDATA_GetWiFiList(wk->pGameData), 0));

    EndMessageWindowOff(wk);

    p_status = WifiFriendMatchStatusGet( GFL_NET_DWC_GetFriendIndex() );
    status = _WifiMyStatusGet( wk, p_status );
    gamemode = _WifiMyGameModeGet( wk, p_status );

    if(gamemode == WIFI_GAME_UNIONMATCH){ //繋ぐだけ。繋いだ後に各ゲーム選択
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERDIRECT_INIT1);
    }
    else{
      /// 検査後にゲームに飛ぶ
      status = WIFI_STATUS_PLAYING;
      _myStatusChange(wk, status,gamemode);  // 接続中になる
      _CHANGESTATE(wk, WIFIP2PMATCH_PLAYERMACHINE_INIT1);
    }
  }
  return seq;
}



//------------------------------------------------------------------
/**
 * $brief   メインメニューの戻る際の初期化
 *
 * @param   wk
 *
 * @retval  none
 */
//------------------------------------------------------------------
static int WifiP2PMatch_MainReturn( WIFIP2PMATCH_WORK *wk, int seq )
{
  GFL_BG_ClearFrame(  GFL_BG_FRAME3_M);
  wk->mainCur = 0;
  _CHANGESTATE(wk,WifiP2PMatchFriendListStart(wk));
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   個人データ表示初期化 WIFIP2PMATCH_MODE_PERSONAL_INIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _personalDataInit( WIFIP2PMATCH_WORK *wk, int seq )
{
  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_PERSONAL_WAIT);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   個人データ表示 WIFIP2PMATCH_MODE_PERSONAL_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _personalDataWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if( 0 !=  _checkParentConnect(wk)){ // 接続してきた
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_PERSONAL_END);
  }
  if(GFL_UI_KEY_GetTrg() & (PAD_BUTTON_CANCEL|PAD_BUTTON_DECIDE)){
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_PERSONAL_END);
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   個人データ表示おわり WIFIP2PMATCH_MODE_PERSONAL_END
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _personalDataEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i,x;
  int num = 0;

  EndMessageWindowOff(wk);


  _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   終了確認メッセージ  WIFIP2PMATCH_MODE_EXIT_YESNO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _exitYesNo( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( WifiP2PMatchMessageEndCheck(wk) ){
    // はいいいえウインドウを出す

    //@todo 読み込みが必要かどうか10.01.16
    //    BmpWinFrame_GraphicSet(
    //      GFL_BG_FRAME2_M, MENU_WIN_CGX_NUM, MENU_WIN_PAL, 0, HEAPID_WIFIP2PMATCH );

    _yenowinCreateM2(wk);

    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXIT_WAIT);
  }


  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   終了確認メッセージ  WIFIP2PMATCH_MODE_EXIT_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _exitWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;
  int ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  //  接続チェック
  if( 0 !=  _checkParentNewPlayer(wk)){ // 接続してきた
    if(ret == BMPMENU_NULL){
      BmpMenu_YesNoMenuExit( wk->pYesNoWork );
    }
    ret = BMPMENU_CANCEL; // CANCEL
  }

  if(ret == BMPMENU_NULL){  // まだ選択中
    return seq;
  }else if(ret == 0){ // はいを選択した場合
    WifiP2PMatchMessagePrint(wk, dwc_message_0011, TRUE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXITING);
    wk->timer = 1;
  }
  else{  // いいえを選択した場合
    EndMessageWindowOff(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);

    // 主人公の動作を許可
    FriendRequestWaitOff( wk );
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   終了確認メッセージ  WIFIP2PMATCH_MODE_EXITING
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _exitExiting( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  if(wk->timer == 1){
    wk->timer = 0;
    //GFL_NET_StateWifiLogout();
    GFL_NET_Exit(NULL);
  }
  if(!GFL_NET_IsInit()){
    NET_PRINT("切断した時にフレンドコードを詰める\n");
    WifiList_FormUpData(wk->pList);  // 切断した時にフレンドコードを詰める
    //SaveData_SaveParts(wk->pSaveData, SVBLK_ID_NORMAL);  //セーブ中
    WifiP2PMatchMessagePrint(wk, dwc_message_0012, TRUE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_EXIT_END);
    wk->timer = 30;
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   終了完了  WIFIP2PMATCH_MODE_EXIT_END
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------


static int _exitEnd( WIFIP2PMATCH_WORK *wk, int seq )
{
  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }
  wk->timer--;
  if(wk->timer==0){
    wk->endSeq = WIFI_GAME_NONE;
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_END_WAIT);
    EndMessageWindowOff(wk);
  }

  // WIFI　対戦AUTOﾓｰﾄﾞデバック
#ifdef _WIFI_DEBUG_TUUSHIN
  WIFI_DEBUG_BATTLE_Work.DEBUG_WIFI_A_REQ = FALSE;
  WIFI_DEBUG_BATTLE_Work.DEBUG_WIFI_B_REQ = FALSE;
  WIFI_DEBUG_BATTLE_Work.DEBUG_WIFI_TOUCH_REQ = FALSE;
#endif  //_WIFI_DEBUG_TUUSHIN
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   もう一度対戦するか聞く WIFIP2PMATCH_NEXTBATTLE_YESNO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _nextBattleYesNo( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),_TIMING_BATTLE_END)==FALSE){
    return seq;
  }
  // 通信同期中に電源を切られたら、ずっと同期待ちしてしまうので、通信同期後にオートエラーチェックを
  // はずす
  GFL_NET_SetAutoErrorCheck(FALSE);
  GFL_NET_SetNoChildErrorCheck(TRUE);

  if( WifiP2PMatchMessageEndCheck(wk) ){
    _yenowinCreateM2(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_NEXTBATTLE_WAIT);
  }

  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   もう一度対戦するか聞く WIFIP2PMATCH_NEXTBATTLE_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _nextBattleWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  int i;
  int ret;

  if( !WifiP2PMatchMessageEndCheck(wk) ){
    return seq;
  }

  if(GFL_NET_StateIsWifiLoginState() || GFL_NET_StateIsWifiDisconnect() || (GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_TIMEOUT)){  // 切断した扱いとな
    BmpMenu_YesNoMenuExit(wk->pYesNoWork);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_065, FALSE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_BATTLE_DISCONNECT);
  }
  else if(GFL_NET_StateIsWifiError()){
    BmpMenu_YesNoMenuExit(wk->pYesNoWork);
    _errorDisp(wk);
  }
  else{
    ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);
    if(ret == BMPMENU_NULL){  // まだ選択中
      return seq;
    }else if(ret == 0){ // はいを選択した場合
      //EndMessageWindowOff(wk);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_073, FALSE);
      GF_ASSERT( wk->timeWaitWork == NULL );
      wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
      wk->bRetryBattle = TRUE;
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_CALL_YESNO);
      wk->timer = 30;
    }
    else{  // いいえを選択した場合
      EndMessageWindowOff(wk);
      //@@OO            CommInfoFinalize();
      //GFL_NET_StateWifiBattleMatchEnd();
      GFL_NET_StateWifiMatchEnd(TRUE);
      wk->timer = _RECONECTING_WAIT_TIME;
      _CHANGESTATE(wk,WIFIP2PMATCH_RECONECTING_WAIT);
    }
    //        wk->pMatch = GFL_NET_StateGetMatchWork();  //@@
    //      wk->pMatch->myMatchStatus.vchat = wk->pMatch->myMatchStatus.vchat_org;
  }
  return seq;
}


//------------------------------------------------------------------
/**
 * $brief   VCAHTを変更してもいいかどうか聞く WIFIP2PMATCH_MODE_VCHAT_NEGO
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _vchatNegoCheck( WIFIP2PMATCH_WORK *wk, int seq )
{
#if 0
  // 相手の状態がかわったりしたら終わる
  {
    int status,friendNo,vchat;
    MCR_MOVEOBJ* p_npc;
    WIFI_STATUS* p_status;

    // 話しかけている友達ナンバー取得
    friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
    p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );

    // 相手がいなくなったり
    // 相手のステータスが違うものに変わったら、
    // 表示を消して元に戻る
    if( p_npc == NULL ){
      _friendNameExpand(wk, friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
      return seq;
    }else{
      p_status = WifiFriendMatchStatusGet( friendNo - 1 );
      status = _WifiMyStatusGet( wk, p_status );
      vchat = WIFI_STATUS_GetVChatStatus(p_status);
      gamemode = _WifiMyGameModeGet( wk, p_status );

      // 状態がかわったり
      if((_WifiMyGameModeGet( wk, &wk->targetStatus ) != gamemode) || (wk->keepVChat != vchat)){
        _friendNameExpand(wk, friendNo - 1);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
        return seq;
      }
    }
  }

  if( WifiP2PMatchMessageEndCheck(wk) ){

    _yenowinCreateM2(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCHAT_NEGO_WAIT);
  }
#endif
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief   VCAHTを変更してもいいかどうか聞く WIFIP2PMATCH_MODE_VCHAT_NEGO_WAIT
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static int _vchatNegoWait( WIFIP2PMATCH_WORK *wk, int seq )
{
#if 0
  int i;
  int ret,status;

  // 相手の状態がかわったりしたら終わる
  {
    int friendNo,vchat;
    MCR_MOVEOBJ* p_npc;
    WIFI_STATUS* p_status;

    // 話しかけている友達ナンバー取得
    friendNo = WIFI_MCR_PlayerSelect( &wk->matchroom );
    p_npc = MCRSYS_GetMoveObjWork( wk, friendNo );

    // 相手がいなくなったり
    // 相手のステータスが違うものに変わったら、
    // 表示を消して元に戻る
    if( p_npc == NULL ){
      BmpMenu_YesNoMenuExit(wk->pYesNoWork);
      _friendNameExpand(wk, friendNo - 1);
      WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
      _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
      return seq;
    }else{
      p_status = WifiFriendMatchStatusGet( friendNo - 1 );
      status = _WifiMyStatusGet( wk, p_status );
      vchat = WIFI_STATUS_GetVChatStatus(p_status);
      gamemode = _WifiMyGameModeGet( wk, p_status );

      // 状態がかわったり
      if((_WifiMyGameModeGet( wk, &wk->targetStatus ) != gamemode) || (wk->keepVChat != vchat)){
        BmpMenu_YesNoMenuExit(wk->pYesNoWork);
        _friendNameExpand(wk, friendNo - 1);
        WifiP2PMatchMessagePrint(wk, msg_wifilobby_013, FALSE);
        _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
        return seq;
      }
    }
  }


  if(GFL_NET_StateGetWifiStatus() >= GFL_NET_STATE_TIMEOUT){
    BmpMenu_YesNoMenuExit(wk->pYesNoWork);
    WifiP2PMatchMessagePrint(wk, msg_wifilobby_015, FALSE); // 反応がない・・・
    GFL_NET_StateWifiMatchEnd(TRUE);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_DISCONNECT);
    return seq;
  }
  else if(GFL_NET_StateIsWifiError()){
    BmpMenu_YesNoMenuExit(wk->pYesNoWork);
    _errorDisp(wk);
  }
  else{
    ret = BmpMenu_YesNoSelectMain(wk->pYesNoWork);
    if(ret == BMPMENU_NULL){  // まだ選択中
      return seq;
    }else if(ret == 0){ // はいを選択した場合
      // 接続開始
      status = _convertState(wk->keepStatus);


      if(WIFI_STATUS_UNKNOWN == status){   // 未知のステートの場合何も無いように戻る
      }
      else if( WifiDwc_getFriendStatus(wk->friendNo - 1) == DWC_STATUS_MATCH_SC_SV ){
        _myVChatStatusToggle(wk); // 自分のVCHATを反転
        if( WifiP2PMatch_CommWifiBattleStart( wk, -1 ) ){ //親になる
          //接続先MACアドレス設定
          WIFI_STATUS_SetVChatMac(wk->pMatch, WifiFriendMatchStatusGet( friendNo - 1 ));

          wk->cancelEnableTimer = _CANCELENABLE_TIMER;
          _commStateChange(wk, WIFI_STATUS_RECRUIT, WIFI_GAME_VCT);
          GFL_NET_SetWifiBothNet(FALSE);  // VCT中は同期送信の必要ない
          _myStatusChange(wk, WIFI_STATUS_RECRUIT, WIFI_GAME_VCT);  // 接続中になる
          _friendNameExpand(wk, wk->friendNo - 1);
          WifiP2PMatchMessagePrint(wk,msg_wifilobby_014, FALSE);
          GF_ASSERT( wk->timeWaitWork == NULL );
          wk->timeWaitWork = TimeWaitIconAdd( wk->MsgWin, COMM_TALK_WIN_CGX_NUM );
          if(status != WIFI_STATUS_VCT){
            _CHANGESTATE(wk,WIFIP2PMATCH_MODE_MATCH_LOOP);
            return seq;
          }
          else{
            wk->cancelEnableTimer = _CANCELENABLE_TIMER;
            _CHANGESTATE(wk,WIFIP2PMATCH_MODE_VCT_CONNECT_INIT2);
            return seq;
          }
        }
      }
    }
    // いいえを選択した場合  もしくは適合しない場合
    EndMessageWindowOff(wk);


    // その人の情報を消す
    //    WifiP2PMatch_UserDispOff( wk, HEAPID_WIFIP2PMATCH );

    // ステータスをもどす
    // 080703 tomoya takahashi
    _myStatusChange(wk, WIFI_STATUS_LOGIN_WAIT);

    // VCHAT元に戻す
    _myVChatStatusOrgSet( wk );
    _userDataInfoDisp(wk);
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_FRIENDLIST);
    return seq;

  }
#endif
  return seq;
}


// WIFIP2PMATCH_MODE_END_WAIT
//------------------------------------------------------------------
/**
 * $brief   終了
 *
 * @param   wk
 * @param   seq
 *
 * @retval  int
 */
//------------------------------------------------------------------
static int  WifiP2PMatch_EndWait( WIFIP2PMATCH_WORK *wk, int seq )
{
  if(!GFL_NET_IsInit()){
    //        WIPE_SYS_Start( WIPE_PATTERN_WMS, WIPE_TYPE_FADEOUT, WIPE_TYPE_FADEOUT, WIPE_FADE_BLACK,
    //                      COMM_BRIGHTNESS_SYNC, 1, HEAPID_WIFIP2PMATCH);
  //  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
    //                WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
//    GFL_FADE_SetMasterBrightReq(GFL_FADE_MASTER_BRIGHT_BLACKOUT,
  //                              0,16,COMM_BRIGHTNESS_SYNC);
    seq = SEQ_OUT;            //終了シーケンスへ
  }
  return seq;
}

//------------------------------------------------------------------
/**
 * $brief
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------
static void _myStatusChange(WIFIP2PMATCH_WORK *wk, int status,int gamemode)
{
  if(wk->pMatch==NULL){  //0707
    return;
  }
  _myStatusChange_not_send( wk, status, gamemode );
  _sendMatchStatus(wk);
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分のステータス設定    ただ送信はしない
 *
 *  @param  wk
 *  @param  status
 */
//-----------------------------------------------------------------------------
static void _myStatusChange_not_send(WIFIP2PMATCH_WORK *wk, int status,int gamemode)
{
  int org_status;

  if(wk->pMatch==NULL){  //0707
    return;
  }

  org_status = _WifiMyStatusGet( wk, wk->pMatch );

  if(org_status != status){
    _commStateChange(wk,status,gamemode);

    WIFI_STATUS_SetWifiStatus(wk->pMatch,status);
    WIFI_STATUS_SetGameMode(wk->pMatch,gamemode);
    if(gamemode == WIFI_GAME_TVT && status == WIFI_STATUS_PLAYING){
      Snd_BgmFadeOut( 0, BGM_FADE_VCHAT_TIME); // VCT状態へ
    }
    else if(status == WIFI_STATUS_PLAYING){
      //@todo ボリュームを落す
    }
    else{
      // @todo ボリュームを戻す
    }
  }
  _userDataInfoDisp(wk);
}

//------------------------------------------------------------------
/**
 * $brief   VCHATフラグの切り替え
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static BOOL _myVChatStatusToggle(WIFIP2PMATCH_WORK *wk)
{
  WIFI_STATUS_SetVChatStatus(wk->pMatch, 1 - WIFI_STATUS_GetVChatStatus( wk->pMatch ));
  _sendMatchStatus(wk);
  return WIFI_STATUS_GetVChatStatus( wk->pMatch );
}

//------------------------------------------------------------------
/**
 * $brief   VCHATフラグの切り替え
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static BOOL _myVChatStatusToggleOrg(WIFIP2PMATCH_WORK *wk)
{
  //本物をひっくり返す
  wk->pParentWork->vchatMain = 1 - wk->pParentWork->vchatMain;
  WIFI_STATUS_SetVChatStatus(wk->pMatch, wk->pParentWork->vchatMain);
  _sendMatchStatus(wk);
  return WIFI_STATUS_GetVChatStatus( wk->pMatch );
}

//------------------------------------------------------------------
/**
 * $brief   VCHATフラグをオリジナルにもどす
 * @param   wk
 * @retval  none
 */
//------------------------------------------------------------------

static BOOL _myVChatStatusOrgSet(WIFIP2PMATCH_WORK *wk)
{
  //  @@oo
  //  NET_PRINT( "change org %d\n", wk->pMatch->myMatchStatus.vchat_org );
  //  wk->pMatch->myMatchStatus.vchat = wk->pMatch->myMatchStatus.vchat_org;
  //  _sendMatchStatus(wk);
  return WIFI_STATUS_GetVChatStatus( wk->pMatch );//wk->pMatch->myMatchStatus.vchat_org;
}


//----------------------------------------------------------------------------
/**
 *  @brief  ユーザーデータ強制表示処理
 *
 *  @param  wk      システムワーク
 *  @param  friendNo  友達番号
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatch_UserDispOn( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID )
{
  MCR_MOVEOBJ* p_obj;

  MCVSys_UserDispOn( wk, friendNo, heapID );
  p_obj = MCRSYS_GetMoveObjWork( wk, friendNo );
  if( p_obj ){
    WIFI_MCR_CursorOn( &wk->matchroom, p_obj );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ユーザーデータ強制表示処理終了
 *
 *  @param  wk      システムワーク
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatch_UserDispOff( WIFIP2PMATCH_WORK *wk, u32 heapID )
{
  MCVSys_UserDispOff( wk );
  WIFI_MCR_CursorOff( &wk->matchroom );

  // 再描画
  MCVSys_ReWrite( wk, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  表示中の人なら強制的に表示終了
 *
 *  @param  wk        システムワーク
 *  @param  target_friend ターゲットの人物のインデックス
 *  @param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatch_UserDispOff_Target( WIFIP2PMATCH_WORK *wk, u32 target_friend, u32 heapID )
{
  if( MCVSys_UserDispGetFriend( wk ) == target_friend ){
    WifiP2PMatch_UserDispOff( wk, heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分でアクセスしたときのユーザーデータ強制表示処理
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatch_UserDispOn_MyAcces( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID )
{
  MCVSys_UserDispOn( wk, friendNo, heapID );
}

//----------------------------------------------------------------------------
/**
 *  @brief  自分でアクセスしたときのユーザーデータ強制表示処理終了
 */
//-----------------------------------------------------------------------------
static void WifiP2PMatch_UserDispOff_MyAcces( WIFIP2PMATCH_WORK *wk, u32 heapID )
{
  MCVSys_UserDispOff( wk );

  // 再描画
  MCVSys_ReWrite( wk, HEAPID_WIFIP2PMATCH );
}


//----------------------------------------------------------------------------
/**
 *  @brief  動作しているかチェック
 *
 *  @param  wk  システムワーク
 *
 *  @retval TRUE  動作中
 *  @retval FALSE 非動作中
 */
//-----------------------------------------------------------------------------
static BOOL MCVSys_MoveCheck( const WIFIP2PMATCH_WORK *wk )
{
  if( wk->view.p_bttnman == NULL ){
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データビューアー初期化
 *
 *  @param  wk      システムワーク
 *  @param  p_handle  ハンドル
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void MCVSys_Init( WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle, u32 heapID )
{
  static const GFL_UI_TP_HITTBL bttndata[ WCR_MAPDATA_1BLOCKOBJNUM+1 ] = {
    { 0,    47,   0,    119 },
    { 48,   95,   0,    119 },
    { 96,   143,  0,    119 },
    { 144,  191,  0,    119 },

    { 0,    47,   128,  255 },
    { 48,   95,   128,  255 },
    { 96,   143,  128,  255 },
    { 144,  191,  128,  255 },
    {GFL_UI_TP_HIT_END,0,0,0},     //終了データ
  };

  GFL_STD_MemFill( &wk->view, 0, sizeof(WIFIP2PMATCH_VIEW) );

  // 表示設定
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );

  // ボタンデータ初期化
  wk->view.p_bttnman = GFL_BMN_Create( bttndata, MCVSys_BttnCallBack, wk, heapID );

  // ワードセット初期化
  wk->view.p_wordset = WORDSET_Create( heapID );

  // グラフィックデータ読込み
  MCVSys_GraphicSet( wk, p_handle, heapID );


  // とりあえず更新
  wk->view.bttn_allchg = TRUE;
  MCVSys_BackDraw( wk );
  MCVSys_BttnDraw( wk );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データビューアー破棄
 *
 *  @param  wk      システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_Exit( WIFIP2PMATCH_WORK *wk )
{
  if( MCVSys_MoveCheck( wk ) == FALSE ){
    return;
  }

  // グラフィックはき
  MCVSys_GraphicDel( wk );

  // ボタンマネージャ破棄
  GFL_BMN_Delete( wk->view.p_bttnman );
  wk->view.p_bttnman = NULL;

  // ワードセット破棄
  WORDSET_Delete( wk->view.p_wordset );
  wk->view.p_wordset = NULL;

  // BG２、３は非表示にしておく
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3, VISIBLE_OFF );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データビューアーアップデート
 *
 *  @param  wk      システムワーク
 *
 *  @retval 押されている友達番号  + 1
 *  @retval 0 押されていない
 */
//-----------------------------------------------------------------------------
static u32 MCVSys_Updata( WIFIP2PMATCH_WORK *wk, u32 heapID )
{
  u32 map_param, map_param2;
  u32 oambttn_ret;
  BOOL userd_end;

  // 現在フレーム設定
  map_param = WIFI_MCR_GetPlayerOnMapParam( &wk->matchroom );

  // オブジェクトの位置だと下を見ないとちゃんとしたフレームの位置を取得できない
  map_param2 = WIFI_MCR_GetPlayerOnUnderMapParam( &wk->matchroom );
  if( (map_param2 >= MCR_MAPPM_MAP00) &&
      (map_param2 <= MCR_MAPPM_MAP03) ){
    map_param = map_param2;
  }

  if( (map_param >= MCR_MAPPM_MAP00) &&
      (map_param <= MCR_MAPPM_MAP03) ){
    if( wk->view.frame_no != (map_param - MCR_MAPPM_MAP00) ){
      wk->view.frame_no = (map_param - MCR_MAPPM_MAP00);


      Snd_SePlay( _SE_TBLCHANGE );

      // 背景カラー変更
      MCVSys_BackDraw( wk );

      // ボタンも書き換える
      wk->view.button_on = TRUE;
      wk->view.bttn_allchg = TRUE;  // ボタン強制変更
    }
  }


  // ユーザーデータ表示処理
  if( wk->view.user_disp == MCV_USERDISP_INIT ){
    wk->view.user_disp = MCV_USERDISP_ON;
    //    wk->view.user_dispno = WF_USERDISPTYPE_NRML;
    MCVSys_UserDispDraw( wk, heapID );
  }


  if( wk->view.user_disp == MCV_USERDISP_OFF ){

    // ボタンメイン
    GFL_BMN_Main( wk->view.p_bttnman );

    // 表示メイン
    if( wk->view.button_on == TRUE ){
      MCVSys_BttnDraw( wk );
      wk->view.button_on = FALSE;
    }
  }

  // 歩いたり、タッチパネルに触れていたらユーザー表示をOFFする
  if( (wk->view.user_disp == MCV_USERDISP_ON) ||
      (wk->view.user_disp == MCV_USERDISP_ONEX) ){

    // した画面OAMボタンメイン
    //oambttn_ret = MCVSys_OamBttnMain( wk );

    //  USERD終了チェック
    userd_end = MCVSys_UserDispEndCheck( wk );

    // なにかキーを押すか、「もどる」を押したら終了する
    if( userd_end == TRUE ){
      wk->view.bttn_chg_friendNo = wk->view.touch_friendNo; // この友達のボタンを更新してもらう
      wk->view.touch_friendNo = 0;
      wk->view.touch_frame = 0;
      wk->view.button_on = TRUE;
      wk->view.bttn_allchg = TRUE;
      wk->view.user_disp = MCV_USERDISP_OFF;
     // MCVSys_OamBttnOff( wk );  // ボタンOFF
      Snd_SePlay( _SE_DESIDE ); // キャンセル音
    }
  }

  return wk->view.touch_friendNo;
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達データビューアーアップデート  ボタンの表示のみ
 *
 *  @param  wk  システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_UpdataBttn( WIFIP2PMATCH_WORK *wk )
{
  if( wk->view.user_disp == MCV_USERDISP_OFF ){

    // 表示メイン
    if( wk->view.button_on == TRUE ){
      MCVSys_BttnDraw( wk );
      wk->view.button_on = FALSE;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ユーザーディスプレイを終了するかチェック
 *
 *  @param  wk      ワーク
 *  @param  oambttn_ret OAMボタンメイン戻り値
 *
 *  @retval TRUE  終了
 *  @retval FALSE しない
 */
//-----------------------------------------------------------------------------
static BOOL MCVSys_UserDispEndCheck( WIFIP2PMATCH_WORK *wk  )
{
  // 強制表示しているので終了しない
  if( (wk->view.user_disp == MCV_USERDISP_ONEX) ){
    return FALSE;
  }

  //  移動はcontボタンはトリガー
  if( (GFL_UI_KEY_GetCont() & (PAD_KEY_LEFT|PAD_KEY_RIGHT|PAD_KEY_UP|PAD_KEY_DOWN)) ||
      (GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B|PAD_BUTTON_X)) 
      ){
    return TRUE;
  }

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  今詳細表示している人の友達IDXを取得
 *
 *  @param  cp_wk ワーク
 *
 *  @return 友達インデックス  (0なら表示してない)
 */
//-----------------------------------------------------------------------------
static u32  MCVSys_UserDispGetFriend( const WIFIP2PMATCH_WORK* cp_wk )
{
  return cp_wk->view.touch_friendNo;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ユーザーデータ表示
 *
 *  @param  wk      ワーク
 *  @param  friendNo  友達番号
 */
//-----------------------------------------------------------------------------
static void MCVSys_UserDispOn( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 heapID )
{
  if( MCVSys_MoveCheck(wk) == TRUE ){
    wk->view.touch_friendNo = friendNo;
    wk->view.touch_frame  = 2;
    wk->view.user_disp    = MCV_USERDISP_ONEX;
    //    wk->view.user_dispno = WF_USERDISPTYPE_NRML;
    MCVSys_UserDispDraw( wk, heapID );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ユーザーデータOFF
 *
 *  @param  wk      ワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_UserDispOff( WIFIP2PMATCH_WORK *wk )
{
  if( MCVSys_MoveCheck(wk) == TRUE ){
    wk->view.touch_friendNo = 0;
    wk->view.touch_frame = 0;
    wk->view.user_disp    = MCV_USERDISP_OFF;
    wk->view.button_on    = TRUE;
    wk->view.bttn_allchg = TRUE;
//    MCVSys_OamBttnOff( wk );  // ボタンOFF
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ページ変更処理
 *
 *  @param  wk        ワーク
 *  @param  oambttn_ret   ボタン戻り値
 */
//-----------------------------------------------------------------------------
#if 0
static void MCVSys_UserDispPageChange( WIFIP2PMATCH_WORK *wk, u32 oambttn_ret )
{
  u32 typenum;

  // フロンティア有無で、表示数を変える
  if( _frontierInCheck(wk) == TRUE ){
    typenum = WF_USERDISPTYPE_NUM;
  }else{
    typenum = WF_USERDISPTYPE_MINI+1; // ミニゲームまで表示
  }

  if( oambttn_ret == MCV_USERD_BTTN_RET_RIGHT ){
    wk->view.user_dispno = (wk->view.user_dispno+1) % typenum;
  }else{
    wk->view.user_dispno --;
    if( wk->view.user_dispno < 0 ){
      wk->view.user_dispno += typenum;
    }
  }
}
#endif
//----------------------------------------------------------------------------
/**
 *  @brief  ボタンイベントコールバック
 *
 *  @param  bttnid    ボタンID
 *  @param  event   イベント種類
 *  @param  p_work    ワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnCallBack( u32 bttnid, u32 event, void* p_work )
{
  WIFIP2PMATCH_WORK *wk = p_work;
  u32 friendNo;

  // 友達番号取得
  friendNo = (wk->view.frame_no * WCR_MAPDATA_1BLOCKOBJNUM) + bttnid;
  friendNo ++;  // 自分が０だから

  // 動作中かチェック
  if( wk->view.bttnfriendNo[ friendNo-1 ] != MCV_BTTN_FRIEND_TYPE_IN ){
    // 動作していないので何もしない
    return ;
  }

  switch( event ){
  case GFL_BMN_EVENT_TOUCH:   ///< 触れた瞬間
    wk->view.touch_friendNo = friendNo;
    wk->view.user_disp = MCV_USERDISP_INIT;
    Snd_SePlay( _SE_DESIDE );
    break;

  default:
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  グラフィックデータ設定
 *
 *  @param  wk      システムワーク
 *  @param  p_handle  アーカイブハンドル
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void MCVSys_GraphicSet( WIFIP2PMATCH_WORK *wk, ARCHANDLE* p_handle, u32 heapID )
{
  int i, j;
  int x, y;

  // BG設定
  // FRAME0_Sスクリーンデータクリーン
  GFL_BG_ClearFrame(   GFL_BG_FRAME0_S );
  // パレット転送
//  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifip2pmatch_wf_match_btm_NCLR,  // 背景用
  //                                  PALTYPE_SUB_BG, MCV_PAL_BACK*32, MCV_PAL_BACK_NUM*32, heapID );
  GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_wifip2pmatch_wf_match_btm_button_NCLR, // ボタン用
                                    PALTYPE_SUB_BG, MCV_PAL_BTTN*32, MCV_PAL_BTTN_NUM*32, heapID );

  // キャラクタ転送
//  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_wf_match_btm_NCGR,
  //                                      GFL_BG_FRAME0_S, MCV_CGX_BACK, 0, FALSE, heapID );
  GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_wifip2pmatch_wf_match_btm_button_NCGR,
                                        GFL_BG_FRAME2_S, MCV_CGX_BTTN2, 0, FALSE, heapID );

  // スクリーン読込みor転送
  // 背景はキャラクタ位置がずれているのでスクリーンデータを書き換える
//  GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_wifip2pmatch_wf_match_btm_NSCR,   GFL_BG_FRAME0_S, 0, 0, FALSE, heapID );

  // ボタンスクリーン読込み
  wk->view.p_bttnbuff = GFL_ARCHDL_UTIL_LoadScreen( p_handle, NARC_wifip2pmatch_wf_match_btm_button_NSCR, FALSE, &wk->view.p_bttnscrn, heapID );
  MCVSys_GraphicScrnCGOfsChange( wk->view.p_bttnscrn, MCV_CGX_BTTN2 );

  // ユーザースクリーン読込み
    for( i=0; i < 1; i++ ){
    wk->view.p_userbuff[i] = GFL_ARCHDL_UTIL_LoadScreen( p_handle, NARC_wifip2pmatch_wf_match_btm_result00_NSCR+i, FALSE, &wk->view.p_userscrn[i], heapID );
    MCVSys_GraphicScrnCGOfsChange( wk->view.p_userscrn[i], MCV_CGX_BTTN2 );
  }
  
  // ダミースクリーン読み込み
  wk->view.p_useretcbuff = GFL_ARCHDL_UTIL_LoadScreen( p_handle, NARC_wifip2pmatch_wf_match_btm_etc_NSCR, FALSE, &wk->view.p_useretcscrn, heapID );
  MCVSys_GraphicScrnCGOfsChange( wk->view.p_useretcscrn, MCV_CGX_BTTN2 );



  // フォントパレット読込み
	GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 
		MCV_SYSFONT_PAL*32, 0x20, heapID);
//  TalkFontPaletteLoad( PALTYPE_SUB_BG, MCV_SYSFONT_PAL*32, heapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_SUB_BG , 13*0x20, 16*2, heapID );

  // ビットマップ作成
  for( i=0; i<WCR_MAPDATA_1BLOCKOBJNUM; i++ ){
    x = i/4;  // 配置は2*4
    y = i%4;
    //    GFL_BG_BmpWinInit( wk->view.nameWin[i] );
    wk->view.nameWin[i] = GFL_BMPWIN_Create(
      GFL_BG_FRAME0_S,
      MCV_NAMEWIN_DEFX+(MCV_NAMEWIN_OFSX*x),
      MCV_NAMEWIN_DEFY+(MCV_NAMEWIN_OFSY*y),
      MCV_NAMEWIN_SIZX, MCV_NAMEWIN_SIZY,
      MCV_SYSFONT_PAL, GFL_BMP_CHRAREA_GET_B );
    // 透明にして展開
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->view.nameWin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( wk->view.nameWin[i] );

    // 状態面書き込み先
    for( j=0; j<WF_VIEW_STATUS_NUM; j++ ){
      //  GFL_BG_BmpWinInit( wk->view.statusWin[i][j] );
      wk->view.statusWin[i][j] = GFL_BMPWIN_Create(
        GFL_BG_FRAME1_S,
        MCV_STATUSWIN_DEFX+(MCV_STATUSWIN_OFSX*x)+(j*MCV_STATUSWIN_VCHATX),
        MCV_STATUSWIN_DEFY+(MCV_STATUSWIN_OFSY*y),
        MCV_STATUSWIN_SIZX, MCV_STATUSWIN_SIZY,
        PLAYER_DISP_ICON_PLTTOFS_SUB,
        GFL_BMP_CHRAREA_GET_B );
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->view.statusWin[i][j]), 0 );
      GFL_BMPWIN_TransVramCharacter( wk->view.statusWin[i][j] );
    }

  }
  wk->view.userWin = GFL_BMPWIN_Create(
    GFL_BG_FRAME3_S,
    MCV_USERWIN_X, MCV_USERWIN_Y,
    MCV_USERWIN_SIZX, MCV_USERWIN_SIZY,
    MCV_SYSFONT_PAL, GFL_BMP_CHRAREA_GET_B );
  // 透明にして展開
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->view.userWin), 0 );
  GFL_BMPWIN_MakeScreen(wk->view.userWin);
  GFL_BMPWIN_TransVramCharacter( wk->view.userWin );
  
  GFL_BG_LoadScreenReq(GFL_BG_FRAME3_S);

}

//----------------------------------------------------------------------------
/**
 *  @brief  グラフィックデータ破棄
 *
 *  @param  wk  システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_GraphicDel( WIFIP2PMATCH_WORK *wk )
{
  int i, j;


  // ビットマップ破棄
  for( i=0; i<WCR_MAPDATA_1BLOCKOBJNUM; i++ ){
    GFL_BMPWIN_Delete( wk->view.nameWin[i] );
    for( j=0; j<WF_VIEW_STATUS_NUM; j++ ){
      GFL_BMPWIN_Delete( wk->view.statusWin[i][j] );
    }
  }
  GFL_BMPWIN_Delete( wk->view.userWin );

  // ボタンスクリーン破棄
  GFL_HEAP_FreeMemory( wk->view.p_bttnbuff );

  // ユーザーウインドウ
    for( i=0; i<1; i++ ){
    GFL_HEAP_FreeMemory( wk->view.p_userbuff[i] );
  }
  
  // ダミースクリーン破棄
  GFL_HEAP_FreeMemory( wk->view.p_useretcbuff );
}

//----------------------------------------------------------------------------
/**
 *  @brief  SCRNのキャラクタNoを転送先のアドレス分足す
 *
 *  @param  p_scrn  スクリーンデータ
 *  @param  cgofs キャラクタオフセット（キャラクタ単位）
 */
//-----------------------------------------------------------------------------
static void MCVSys_GraphicScrnCGOfsChange( NNSG2dScreenData* p_scrn, u8 cgofs )
{
  u16* p_scrndata;
  int i, j;
  int siz_x, siz_y;

  p_scrndata = (u16*)p_scrn->rawData;
  siz_x = p_scrn->screenWidth/8;
  siz_y = p_scrn->screenHeight/8;

  for( i=0; i<siz_y; i++ ){
    for( j=0; j<siz_x; j++ ){
      p_scrndata[ (i*siz_x)+j ] += cgofs;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタン設定
 *
 *  @param  wk      システムワーク
 *  @param  friendNo  友達番号
 *  @param  type    設定タイプ
 *
 *  type
 *    MCV_BTTN_FRIEND_TYPE_RES, // 予約
 *    MCV_BTTN_FRIEND_TYPE_IN,  // 登録済み
 *
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnSet( WIFIP2PMATCH_WORK *wk, u32 friendNo, u32 type )
{
  GF_ASSERT( friendNo > 0 );
  GF_ASSERT( type > MCV_BTTN_FRIEND_TYPE_NONE );
  GF_ASSERT( type < MCV_BTTN_FRIEND_TYPE_MAX );
  wk->view.bttnfriendNo[ friendNo-1 ] = type;
  wk->view.button_on = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタンからはずす
 *
 *  @param  wk      システムワーク
 *  @param  friendNo  友達番号
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnDel( WIFIP2PMATCH_WORK *wk, u32 friendNo )
{
  GF_ASSERT( friendNo > 0 );
  wk->view.bttnfriendNo[ friendNo-1 ] = MCV_BTTN_FRIEND_TYPE_NONE;
  wk->view.button_on = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタンの表示タイプを取得する
 *
 *  @param  wk      ワーク
 *  @param  friendNo  友達number
 *
 *  @return 表示タイプ
 *  type
 *    MCV_BTTN_FRIEND_TYPE_RES, // 予約
 *    MCV_BTTN_FRIEND_TYPE_IN,  // 登録済み
 *    MCV_BTTN_FRIEND_TYPE_NONE,  // なし
 */
//-----------------------------------------------------------------------------
static u32 MCVSys_BttnTypeGet( const WIFIP2PMATCH_WORK *wk, u32 friendNo )
{
  GF_ASSERT( friendNo > 0 );
  return wk->view.bttnfriendNo[ friendNo-1 ];
}

//----------------------------------------------------------------------------
/**
 *  @brief  強制書き直し
 *
 *  @param  wk      システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_ReWrite( WIFIP2PMATCH_WORK *wk, u32 heapID )
{
  if( wk->view.user_disp != MCV_USERDISP_OFF ){
    MCVSys_UserDispDraw( wk, heapID );
  }else{
    wk->view.bttn_allchg = TRUE;
    MCVSys_BttnDraw( wk );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタンをすべて書くリクエストを出す
 *
 *  @param  wk
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnAllWriteReq( WIFIP2PMATCH_WORK *wk )
{
  wk->view.bttn_allchg = TRUE;
  wk->view.button_on = TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  背景描画
 *
 *  @param  wk  システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_BackDraw( WIFIP2PMATCH_WORK *wk )
{
  // 背景のカラーを変える
//  GFL_BG_ChangeScreenPalette(   GFL_BG_FRAME0_S, 0, 0,
  //                              32, 24, wk->view.frame_no+MCV_PAL_FRMNO );

//  GFL_BG_LoadScreenV_Req(   GFL_BG_FRAME0_S );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタン表示
 *
 *  @param  wk  システムワーク
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnDraw( WIFIP2PMATCH_WORK *wk )
{
  int i, j;
  int sex;
  int bttn_type;
  int x, y;
  int friend_no;
  int frame;
  BOOL write_change_masterflag; // 強制的にすべてのボタンを書き換える
  BOOL write_change_localflag;  // ここのボタンごとの書き換えチェック

  // 全書き換えチェック
  if( wk->view.bttn_allchg == TRUE ){
    wk->view.bttn_allchg = FALSE;
    write_change_masterflag = TRUE;

    // スクリーンクリア
    //    GFL_BG_ScrFill( GFL_BG_FRAME2_S, 0, 0, 0, 32, 24, 0 );
    GFL_BG_ClearScreen(GFL_BG_FRAME2_S);
  }else{
    write_change_masterflag = FALSE;
  }


  for( i=0; i<WCR_MAPDATA_1BLOCKOBJNUM; i++ ){

    friend_no = (wk->view.frame_no*WCR_MAPDATA_1BLOCKOBJNUM) + i + 1;
    // 表示タイプ設定
    if( wk->view.bttnfriendNo[friend_no - 1] != MCV_BTTN_FRIEND_TYPE_NONE ){
      sex = WifiList_GetFriendInfo( wk->pList, friend_no - 1, WIFILIST_FRIEND_SEX );
      if(sex == PM_MALE){
        bttn_type = MCV_BUTTON_TYPE_MAN;
      }else{
        bttn_type = MCV_BUTTON_TYPE_GIRL;
      }
    }else{
      bttn_type = MCV_BUTTON_TYPE_NONE;
    }

    // ボタン表示座標
    x = i/4;
    y = i%4;

    // 押されているボタンかチェック
    if( friend_no == wk->view.touch_friendNo ){
      frame = wk->view.touch_frame;
    }else{
      frame = 0;
    }


    // ボタンの形が変わっているかチェック
    if( (friend_no == wk->view.touch_friendNo) ||
        (friend_no == wk->view.bttn_chg_friendNo) ){
      write_change_localflag = TRUE;
    }else{
      write_change_localflag = FALSE;
    }

    // 書き換えるかチェック
    if( (write_change_masterflag == TRUE) ||
        (write_change_localflag == TRUE) ){

      // ボタン
      MCVSys_BttnWrite( wk,
                        MCV_BUTTON_DEFX+(MCV_BUTTON_OFSX*x), MCV_BUTTON_DEFY+(MCV_BUTTON_OFSY*y),
                        bttn_type, frame );

      // 名前表示
      if( bttn_type != MCV_BUTTON_TYPE_NONE ){

        // 名前の表示
        NET_PRINT("friendno %d\n",friend_no);
        MCVSys_BttnWinDraw( wk, wk->view.nameWin[i], friend_no, frame, i );
        GFL_BMPWIN_MakeScreen(wk->view.nameWin[i]);
        GFL_BG_LoadScreenV_Req(GFL_BG_FRAME0_S);

        //MCVSys_BttnStatusWinDraw( wk, wk->view.statusWin[i], friend_no, frame, i );
      }else{

        // 透明にする
        GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->view.nameWin[i]), 0 );
        GFL_BMPWIN_TransVramCharacter( wk->view.nameWin[i] );
        for( j=0; j<WF_VIEW_STATUS_NUM; j++ ){
          GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->view.statusWin[i][j]), 0 );
          GFL_BMPWIN_TransVramCharacter( wk->view.statusWin[i][j] );
        }
      }
    }
  }

  GFL_BG_LoadScreenV_Req(  GFL_BG_FRAME2_S );

  // メッセージ面の表示設定
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG1, VISIBLE_ON );
  GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG3, VISIBLE_OFF );
}


//----------------------------------------------------------------------------
/**
 *  @brief  ファクトリーデータ表示
 *
 *  @param  wk      ワーク
 *  @param  strid   文字ID
 *  @param  factoryid ファクトリーデータID
 *  @param  friendno  友達番号
 *  @param  x     表示X
 *  @param  y     表示Y
 */
//-----------------------------------------------------------------------------
static void MCVSys_UserDispFrontiorNumDraw( WIFIP2PMATCH_WORK *wk, u32 strid, u32 factoryid, u32 friendno, u32 x, u32 y )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  フロンティア　タイトルメッセージ取得
 *
 *  @param  wk      ワーク
 *  @param  p_str   文字列格納先
 *  @param  factoryid ファクトリー　セーブデータID
 *  @param  friendno  友達ナンバー
 *
 *  factoryid
 *    MCV_FRONTIOR_TOWOR,
 *    MCV_FRONTIOR_FACTORY,
 *    MCV_FRONTIOR_FACTORY100,
 *    MCV_FRONTIOR_CASTLE,
 *    MCV_FRONTIOR_STAGE,
 *    MCV_FRONTIOR_ROULETTE,
 *
 */
//-----------------------------------------------------------------------------
static void MCVSys_UserDispFrontiorTitleStrGet( WIFIP2PMATCH_WORK *wk, STRBUF* p_str, u32 factoryid, u32 friendno )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  数字を表示する処理
 *
 *  @param  wk    ワーク
 *  @param  strid メッセージID
 *  @param  num   数字
 *  @param  x   ｘドット座標
 *  @param  y   ｙドット座標
 */
//-----------------------------------------------------------------------------
static void MCVSys_UserDispNumDraw( WIFIP2PMATCH_WORK *wk, u32 strid, u32 num, u32 x, u32 y )
{
  WORDSET_RegisterNumber(wk->view.p_wordset, 0, num, 4, STR_NUM_DISP_SPACE, NUMBER_CODETYPE_DEFAULT);
  GFL_MSG_GetString(  wk->MsgManager, strid, wk->pExpStrBuf );
  WORDSET_ExpandStr( wk->view.p_wordset, wk->TitleString, wk->pExpStrBuf );

  PRINTSYS_Print( GFL_BMPWIN_GetBmp(wk->view.userWin),
                  x,  y,
                  wk->TitleString, wk->fontHandle);
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタン描画
 *
 *  @param  wk    システムワーク
 *  @param  cx    ｘキャラ座標
 *  @param  cy    ｙキャラ座標
 *  @param  type  ボタンタイプ
 *  @param  frame フレーム
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnWrite( WIFIP2PMATCH_WORK *wk, u8 cx, u8 cy, u8 type, u8 frame )
{
  u16 reed_x, reed_y;
  u16* p_buff;

  GF_ASSERT( frame < 4 );

  // frame 3のときは1を表示する
  if( frame == 3 ){
    frame = 1;
  }

  if( (frame < 2) ){
    reed_x = MCV_BUTTON_SIZX * frame;
    reed_y = MCV_BUTTON_SIZY * type;

    GFL_BG_WriteScreenExpand( GFL_BG_FRAME2_S,
                              cx, cy, MCV_BUTTON_SIZX, MCV_BUTTON_SIZY,
                              wk->view.p_bttnscrn->rawData,
                              reed_x, reed_y,
                              32, 18 );
  }else{
    reed_x = 0;
    reed_y = MCV_BUTTON_SIZY * type;
    p_buff = (u16*)wk->view.p_bttnscrn->rawData;

    GFL_BG_WriteScreenExpand( GFL_BG_FRAME2_S,
                              cx, cy, MCV_BUTTON_SIZX, MCV_BUTTON_SIZY,
                              &p_buff[ 32*18 ],
                              reed_x, reed_y,
                              16, 18 );
  }

  // パレットカラーを合わせる
  GFL_BG_ChangeScreenPalette( GFL_BG_FRAME2_S, cx, cy,
                              MCV_BUTTON_SIZX, MCV_BUTTON_SIZY, type+MCV_PAL_BTTN );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ボタン上のトレーナー名の表示
 *
 *  @param  wk      システムワーク
 *  @param  p_bmp   ビットマップ
 *  @param  friendNo  友達ナンバー
 *  @param  frame   ボタンフレーム
 *  @param  area_id   配置ID
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnWinDraw( WIFIP2PMATCH_WORK *wk, GFL_BMPWIN* p_bmp, u32 friendNo, u32 frame, u32 area_id )
{
  int sex;
  int col;
  int y;

  // 画面クリーン
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_bmp), 0 );

  //y座標を取得
  y = ViewButtonFrame_y[ frame ];

  sex = WifiList_GetFriendInfo( wk->pList, friendNo-1, WIFILIST_FRIEND_SEX );
  // トレーナー名
  if( sex == PM_MALE ){
    _COL_N_BLUE;
  }else{
    _COL_N_RED;
  }
  MCVSys_FriendNameSet(wk, friendNo-1);
  GFL_MSG_GetString(  wk->MsgManager, msg_wifilobby_033, wk->pExpStrBuf );
  WORDSET_ExpandStr( wk->view.p_wordset, wk->TitleString, wk->pExpStrBuf );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp(p_bmp),
                  0,  y,
                  wk->TitleString, wk->fontHandle);
  GFL_BMPWIN_TransVramCharacter( p_bmp );
}

//----------------------------------------------------------------------------
/**
 *  @brief  状態ウィンドウの描画
 *
 *  @param  wk      しすてむワーク
 *  @param  p_stbmp   状態ビットマップ
 *  @param  friendNo  友達番号
 *  @param  frame   ボタンフレーム
 *  @param  area_id   配置ID
 */
//-----------------------------------------------------------------------------
static void MCVSys_BttnStatusWinDraw( WIFIP2PMATCH_WORK *wk, GFL_BMPWIN** p_stbmp, u32 friendNo, u32 frame, u32 area_id )
{
  int y;
  int i;
  int vct_icon;
  WIFI_STATUS* p_status;
  u32 status,gamemode;

  p_status = WifiFriendMatchStatusGet( friendNo - 1 );
  status = _WifiMyStatusGet( wk, p_status );
  gamemode = _WifiMyGameModeGet( wk, p_status );

  // アイコン表示
  y = ViewButtonFrame_y[ frame ];

  for( i=0; i<WF_VIEW_STATUS_NUM; i++ ){
    // 画面クリーン
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(p_stbmp[i]), WINCLR_COL(FBMP_COL_WHITE) );
    if( i==0 ){
      int pal = WifiP2PMatchBglFrmIconPalGet( GFL_BMPWIN_GetFrame(p_stbmp[i]) );
      WifiP2PMatchFriendListBmpStIconWrite( p_stbmp[i], &wk->icon,
                                            0, y,
                                            status,gamemode,pal );
    }else{
      // パレットNo取得
      int pal = WifiP2PMatchBglFrmIconPalGet( frame );

      if( WIFI_STATUS_GetVChatStatus(p_status) ){
        vct_icon = PLAYER_DISP_ICON_IDX_NONE;
      }else{
        vct_icon = PLAYER_DISP_ICON_IDX_VCTNOT;
      }
      WifiP2PMatchFriendListBmpIconWrite( p_stbmp[i], &wk->icon,
                                          0, y,
                                          vct_icon, 0, pal );
    }
    GFL_BMPWIN_TransVramCharacter( p_stbmp[i] );
  }

}


//----------------------------------------------------------------------------
/**
 *  @brief  Vierのほうのワードセットに人の名前を設定
 *
 *  @param  p_wk      ワーク
 *  @param  friendno    友達ナンバー
 */
//-----------------------------------------------------------------------------
static void MCVSys_FriendNameSet( WIFIP2PMATCH_WORK* p_wk, int friendno )
{
  if(friendno != -1){
    int sex = WifiList_GetFriendInfo(p_wk->pList, friendno, WIFILIST_FRIEND_SEX);
    WifiList_GetFriendName(p_wk->pList, friendno, p_wk->pExpStrBuf);
    WORDSET_RegisterWord( p_wk->view.p_wordset, 0, p_wk->pExpStrBuf, sex, TRUE, PM_LANG);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達マッチデータのステータス取得
 *
 *  @param  wk      システムワーク
 *  @param  idx     インデックス
 */
//-----------------------------------------------------------------------------
static WIFI_STATUS* WifiFriendMatchStatusGet( u32 idx )
{
  GF_ASSERT( idx < WIFIP2PMATCH_MEMBER_MAX );

#ifdef WFP2PM_MANY_OBJ
  idx = 0;
#endif
  return (WIFI_STATUS*)GFL_NET_DWC_GetFriendInfo( idx );
  //  return &wk->pMatch->friendMatchStatus[ idx ];
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達の状態を取得する
 *
 *  @param  idx   インデックス
 */
//-----------------------------------------------------------------------------
static u8 WifiDwc_getFriendStatus( int idx )
{
#ifdef WFP2PM_MANY_OBJ
  idx = 0;
#endif
  return GFL_NET_DWC_getFriendStatus( idx );
}

//----------------------------------------------------------------------------
/**
 *  @brief  P2P通信ゲームを開始する。
 *
 *  @param  friendno  ターゲット番号  （-1なら親）
 *  @param  status    ステータス
 *
 *  @return
 */
//-----------------------------------------------------------------------------
static BOOL WifiP2PMatch_CommWifiBattleStart( WIFIP2PMATCH_WORK* wk, int friendno )
{

  // ボイスチャット設定
  // ボイスチャットとBGM音量の関係を整理 tomoya takahashi
  GFL_NET_DWC_SetVChat(WIFI_STATUS_GetVChatStatus( wk->pMatch ));

  // 接続する前に４人募集で送られてくる可能性のある
  // コマンドを設定する
  CommCommandWFP2PMF_MatchStartInitialize(wk);

  return GFL_NET_StateStartWifiPeerMatch( friendno );
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達募集中　設定ON
 *
 *  @param  wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FriendRequestWaitOn( WIFIP2PMATCH_WORK* wk, BOOL msg_on )
{
  if( wk->friend_request_wait == FALSE ){
    // メッセージをだして、プレイヤー動作を停止する
    wk->friend_request_wait = TRUE;
    WIFI_MCR_PlayerMovePause( &wk->matchroom, TRUE );

    if( msg_on == TRUE ){
      WifiP2PMatchMessagePrint( wk, msg_wifilobby_142, FALSE );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達募集中　設定OFF
 *
 *  @param  wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FriendRequestWaitOff( WIFIP2PMATCH_WORK* wk )
{
  if( wk->friend_request_wait == TRUE ){
    // メッセージを消して、プレイヤー動作を開始する
    wk->friend_request_wait = FALSE;
    EndMessageWindowOff( wk );
    WIFI_MCR_PlayerMovePause( &wk->matchroom, FALSE );
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  友達募集中フラグ取得
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  募集中
 *  @retval FALSE ぼしゅうしていない
 */
//-----------------------------------------------------------------------------
static BOOL FriendRequestWaitFlagGet( const WIFIP2PMATCH_WORK* cp_wk )
{
  return cp_wk->friend_request_wait;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：初期化
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT WifiP2PMatchProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFIP2PMATCH_WORK * wk = mywk;
  WIFIP2PMATCH_PROC_PARAM* pParentWork = pwk;
  u32 result;


  GFL_HEAP_CreateHeapLo( GFL_HEAPID_APP, HEAPID_WIFIP2PMATCH, 0xa0000 );

  wk = GFL_PROC_AllocWork( proc, sizeof(WIFIP2PMATCH_WORK), HEAPID_WIFIP2PMATCH );
  MI_CpuFill8( wk, 0, sizeof(WIFIP2PMATCH_WORK) );
  GFL_NET_ChangeWork(wk);

    // Vram転送マネージャ作成
#if WB_FIX
    initVramTransferManagerHeap( VRANTRANSFERMAN_NUM, HEAPID_WIFIP2PMATCH );
#endif

    //        wk->MsgIndex = _PRINTTASK_MAX;
    wk->pMatch = pParentWork->pMatch;

    wk->pSaveData = pParentWork->pSaveData;
    wk->pGameData = pParentWork->pGameData;

    wk->pMyPoke = GAMEDATA_GetMyPokemon(wk->pGameData);
    wk->pList = GAMEDATA_GetWiFiList(wk->pGameData);
    wk->pConfig = SaveData_GetConfig(pParentWork->pSaveData);
    wk->initSeq = pParentWork->seq;    // P2PかDPWか
    wk->endSeq = WIFI_GAME_NONE;
    wk->preConnect = -1;
    wk->pParentWork = pParentWork;

    GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3, VISIBLE_OFF );
    GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_BG0|GX_PLANEMASK_BG1|GX_PLANEMASK_BG2|GX_PLANEMASK_BG3, VISIBLE_OFF );

    // ワーク初期化
    InitMessageWork( wk );
    _CHANGESTATE(wk,WIFIP2PMATCH_MODE_INIT);

   // グラフィック初期化
    _graphicInit(wk);

  _makeMyMatchStatus(wk, WIFI_STATUS_WAIT, WIFI_GAME_LOGIN_WAIT);

  
    if(GFL_NET_IsInit()){
      GFL_NET_ReloadIcon();  // 接続中なのでアイコン表示
    }

    return GFL_PROC_RES_FINISH;
}




//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：メイン
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------

static GFL_PROC_RESULT WifiP2PMatchProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFIP2PMATCH_WORK * wk  = mywk;

  if(FuncTable[wk->seq]!=NULL){
    *seq = (*FuncTable[wk->seq])( wk, *seq );
    if(*seq == SEQ_OUT){
      WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                      WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , HEAPID_WIFIP2PMATCH );
      return GFL_PROC_RES_FINISH;
    }
  }
  if(wk->clactSet){
    GFL_CLACT_SYS_Main();
  }
  if( WIFI_MCR_GetInitFlag( &wk->matchroom ) == TRUE ){
    WIFI_MCR_Draw( &wk->matchroom );
  }
  ConnectBGPalAnm_Main(&wk->cbp);
  if(wk->SysMsgQue){
    PRINTSYS_QUE_Main(wk->SysMsgQue);
    GFL_TCBL_Main( wk->pMsgTcblSys );
  }
  return GFL_PROC_RES_CONTINUE;
}

//--------------------------------------------------------------------------------------------
/**
 * プロセス関数：終了
 *
 * @param proc  プロセスデータ
 * @param seq   シーケンス
 *
 * @return  処理状況
 */
//--------------------------------------------------------------------------------------------
static GFL_PROC_RESULT WifiP2PMatchProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  WIFIP2PMATCH_WORK  *wk    = mywk;
  WIFIP2PMATCH_PROC_PARAM* pParentWork = pwk;


  if( !WIPE_SYS_EndCheck() ){
    return GFL_PROC_RES_CONTINUE;
  }

  if(wk->pRegulation){
    Regulation_Copy(wk->pRegulation, pParentWork->pRegulation);
    GFL_HEAP_FreeMemory(wk->pRegulation);
    wk->pRegulation=NULL;
  }
  
  _graphicEnd(wk);

  pParentWork->seq = wk->endSeq;

  if(GFL_NET_IsInit()){
    pParentWork->targetID = GFL_NET_DWC_GetFriendIndex();
  }

  // ワーク解放
  FreeMessageWork( wk );

  GFL_PROC_FreeWork( proc );        // GFL_PROCワーク開放


#if WB_TEMP_FIX
  // VramTransferマネージャ破棄
  DellVramTransferManager();
#endif

  GFL_HEAP_DeleteHeap( HEAPID_WIFIP2PMATCH );

  // BGMがポケセンのままならBGM音量を変更する
  if( WifiP2P_CheckLobbyBgm() == TRUE ){
    Snd_PlayerSetInitialVolume( SND_HANDLE_FIELD, BGM_POKECEN_VOL );
  }

  return GFL_PROC_RES_FINISH;
}



// プロセス定義データ
const GFL_PROC_DATA WifiP2PMatchProcData = {
  WifiP2PMatchProc_Init,
  WifiP2PMatchProc_Main,
  WifiP2PMatchProc_End,
};

