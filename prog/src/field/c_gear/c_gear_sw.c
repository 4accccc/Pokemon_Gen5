//=============================================================================
/**
 * @file	  c_gear_sw.c
 * @brief	  コミュニケーションギア
 * @author	iwasawa
 * @date	  11.07.19 
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "c_gear_sw.h"
#include "c_gear_pattern.h"
#include "savedata/save_tbl.h"
#include "savedata/c_gear_picture.h"
#include "system/main.h"  //HEAPID
#include "system/palanm.h"  //HEAPID

#include "sound/pm_sndsys.h"
#include "system/wipe.h"

#include "c_gear_sw.naix"
//#include "c_gear_obj_NANR_LBLDEFS.h"

#include "net/wih_dwc.h"

#include "field/field_status.h"
#include "field/field_status_local.h"

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
#include "../event_cg_power.h"      //EVENT_ChangeSubScreen


#ifdef PM_DEBUG

#ifdef DEUBG_ONLY_FOR_tomoya_takahashi

#define DEBUG_LOCAL

#endif


//#define DEBUG_KEY_CONTROL // Lボタン　＋　AですべてOFF状態 　Bでライブキャスター YでPALACE



#endif

#define OAM_USE_PLTT_NUM (14)
#define BG_USE_PLTT_NUM (14)

static const u16 _BLACK_COLOR[] = {
  0x0441, //男性用
  0x0423, //女性用
};

#if 0
static const u32 _bgpal[]=
{
  NARC_c_gear_c_gear_m_NCLR,NARC_c_gear_c_gear_f_NCLR,NARC_c_gear_c_gear_m_NCLR,
};
static const u32 _bgcgx[][3]=
{
  { NARC_c_gear_c_gear_m1_NCGR,NARC_c_gear_c_gear_f1_NCGR,NARC_c_gear_c_gear_m1_NCGR, },
  { NARC_c_gear_c_gear_m2_NCGR,NARC_c_gear_c_gear_f2_NCGR,NARC_c_gear_c_gear_m2_NCGR, },
  { NARC_c_gear_c_gear_m3_NCGR,NARC_c_gear_c_gear_f3_NCGR,NARC_c_gear_c_gear_m3_NCGR, },
  { NARC_c_gear_c_gear_m4_NCGR,NARC_c_gear_c_gear_f4_NCGR,NARC_c_gear_c_gear_m4_NCGR, },
  { NARC_c_gear_c_gear_m5_NCGR,NARC_c_gear_c_gear_f5_NCGR,NARC_c_gear_c_gear_m5_NCGR, },
};

static const u32 _cellpal[]=
{
  NARC_c_gear_c_gear_m_obj_NCLR,NARC_c_gear_c_gear_f_obj_NCLR,NARC_c_gear_c_gear_m_obj_NCLR,
};
#endif

#define _NET_DEBUG (1)  //デバッグ時は１
#define _BRIGHTNESS_SYNC (0)  // フェードのＳＹＮＣは要調整

// サウンド仮想ラベル
#define GEAR_SE_DECIDE_ (SEQ_SE_DECIDE3)
#define GEAR_SE_CANCEL_ (SEQ_SE_SYS_70)

// CGEAR キャラクタデータ
#define GEAR_CGX_SIZE_MAX ( 32*20*32 )
#define GEAR_BASE_CGX_SIZE  ( 32*8 )  //256char

//--------------------------------------------
// 画面構成定義
//--------------------------------------------

//-------------------------
#if 0
#define PANEL_Y1 (6)
#define PANEL_Y2 (4)
#define PANEL_X1 (-2)

#define PANEL_HEIDHT1 (C_GEAR_PANEL_HEIGHT-1)
#define PANEL_HEIGHT2 (C_GEAR_PANEL_HEIGHT)

#define PANEL_WIDTH (C_GEAR_PANEL_WIDTH)
#define PANEL_SIZEXY (4)   //

#endif
#define _CGEAR_TYPE_PATTERN_NUM (5)

//-------------------------------------
///	スリープ状態
//=====================================
typedef enum {
  _CGEAR_SLEEP_NONE,
  _CGEAR_SLEEP_START,
  _CGEAR_SLEEP_END,

} _CGEAR_SLEEP_STATE;

typedef enum{
  _CGEAR_NET_BIT_IR = (1 << CGEAR_PANELTYPE_IR),
  _CGEAR_NET_BIT_WIRELESS = (1 << CGEAR_PANELTYPE_WIRELESS),
  _CGEAR_NET_BIT_WIFI = (1 << CGEAR_PANELTYPE_WIFI),
} _CGEAR_NET_BIT;

#if 0
typedef enum{
  // パネル数
  CGEAR_PANEL_IR = 0,
  CGEAR_PANEL_WIFI,
  CGEAR_PANEL_WIRELESS,
  CGEAR_PANEL_MAX,
}CGEAR_PANEL_ID;
#endif

enum{
  // パレットタイプ
  _CGEAR_NET_PALTYPE_RED = 0,
  _CGEAR_NET_PALTYPE_BLUE,
  _CGEAR_NET_PALTYPE_YELLOW,
  _CGEAR_NET_PALTYPE_GREEN,
  _CGEAR_NET_PALTYPE_PINK,
  _CGEAR_NET_PALTYPE_GRAY,
  _CGEAR_NET_PALTYPE_DARK_RED,
  _CGEAR_NET_PALTYPE_DARK_BLUE,
  _CGEAR_NET_PALTYPE_DARK_YELLOW,

  _CGEAR_NET_PALTYPE_MAX,

  // ワイアレスカラーカウント
  _CGEAR_NET_WIRELES_TYPE_FUSHIGI = 0,
  _CGEAR_NET_WIRELES_TYPE_UNION,
  _CGEAR_NET_WIRELES_TYPE_OTHER,
  _CGEAR_NET_WIRELES_TYPE_MAX,
  
};

//----------------------------------------------------------
///パレット関連定義
//----------------------------------------------------------

//パネルのステータスパレットアニメ用
#define PANEL_COLOR_TRANS_POS_PALNO  (2)
#define PANEL_COLOR_TRANS_POS_OFS    (0x0d)

//すれ違いアイコンパレットアニメ用定義
#define CROSS_COLOR_MAX ( 17 )  //すれ違いアイコン用パレットソース数(お気に入りカラー16色+無効カラー1色)

#define CROSS_COLOR_SRC_POS_START   (( 9*16 ))  //コピーソースのパレット位置
#define CROSS_COLOR_TRANS_POS_PALNO (( 5 ))     //転送場所のパレットNo
#define CROSS_COLOR_TRANS_POS_START (( CROSS_COLOR_TRANS_POS_PALNO*16 )+1 ) //転送開始位置
#define CROSS_COLOR_ICON_MAX  (10)  //アイコン数

//リング色変え処理用
#define RING_NUM                ( C_GEAR_RING_NUM )
#define RING_COLOR_MAX          ( C_GEAR_RING_COLOR_MAX+1 )    //リングカラー数+BOOT時カラー

#define RING_COLOR_SET_NUM   (3)
enum{
  RING_COLOR_SET_BG = 0,    //BG用
  RING_COLOR_SET_OBJ = RING_COLOR_SET_NUM,   //OBJ用のノーマル色
  RING_COLOR_SET_OBJ_D = RING_COLOR_SET_OBJ+RING_COLOR_SET_NUM, //OBJ用の暗色
};

#define RING_COLOR_SET_NUM_ALL  ( RING_COLOR_SET_NUM*3 ) //1セット3色
#define RING_COLOR_DEF_NO 　    ( 0 )  //リングカラーのデフォルト
#define RING_COLOR_BASE_NO      ( RING_COLOR_MAX-2 )  //リングの地味カラーNo
#define RING_COLOR_BOOT_NO       ( RING_COLOR_MAX-1 ) //リングのBOOT時のカラーNo

#define RING_COLOR_TRANS_POS_BG_PALNO  (1) //転送場所のパレットNo
#define RING_COLOR_TRANS_POS_BG_START  ((RING_COLOR_TRANS_POS_BG_PALNO*16)+1) //転送場所のパレットNo
#define RING_COLOR_TRANS_POS_OBJ_R2_START1 (0*16+1+RING_COLOR_SET_NUM)
#define RING_COLOR_TRANS_POS_OBJ_R2_START2 (1*16+1+RING_COLOR_SET_NUM)
#define RING_COLOR_TRANS_POS_OBJ_R4_START1 (0*16+1)
#define RING_COLOR_TRANS_POS_OBJ_R4_START2 (1*16+1)

#define RING_COLOR_TRANS_POS_OBJ_PANEL_START (PANEL_COLOR_TRANS_POS_PALNO*16+1)

#if 0
// (u32)配列としてのインデックス
static const u32 sc_CROSS_COLOR_RES_IDX[CROSS_COLOR_MAX] = {
  1, 2, 3, 4, 5, 6, 7,
  9,10,11,12,13,14,15,
  17,18,19,
};
#endif


//-------------------------------------
///	OAMパレットフェード
//=====================================
//#define OAM_PFADE_NORMAL_MSK  ( (1<<0x1) | (1<<0x2) | (1<<0x3) | (1<<0x5) | (1<<0x6) | (1<<0x7) | (1<<0x8) | (1<<0x9) | (1<<0xB) | (1<<0xC) | (1<<0xD) | (1<<0xE) | (1<<0xF) )
#define OAM_PFADE_NORMAL_MSK  ( 0xFF )
#define OAM_PFADE_CROSS_MSK   ( 1<<CROSS_COLOR_TRANS_POS_PALNO )
#define OAM_PFADE_MSK_ALL    (0x3FFF)

//-------------------------------------
///	BGパレットフェード
//=====================================
#define BG_PFADE_NORMAL_MSK  ( (1<<0x1) | (1<<0x2) | (1<<0x3) | (1<<0x4) | (1<<0x5) )
#define BG_PFADE_MSK_ALL    (0x3FFF)

// パレット　通信受信アニメの数値
#if 0
static const u8 _CGEAR_NET_CHANGEPAL_POSX[ CGEAR_PANEL_MAX ] = {
  0xf, 0x7, 0xb, 
};
#endif

enum{
  _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST = 0,
  _CGEAR_NET_CHANGEPAL_ANM_TYPE_NORMAL,
  _CGEAR_NET_CHANGEPAL_ANM_TYPE_BEACON_CATCH,
  _CGEAR_NET_CHANGEPAL_ANM_TYPE_HIGH,
  _CGEAR_NET_CHANGEPAL_ANM_TYPE_OFF,

  _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX,

};
static const u16 _CGEAR_NET_CHANGEPAL_ANM_COUNT_MAX[ _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX ] = 
{
  23,
  160,  // 
  80,  // 
  40,
  16,
};

// アニメリセットあり
static const u8 _CGEAR_NET_CHANGEPAL_ANM_COUNT_RESET[ _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX ] = 
{
  TRUE,
  TRUE,  // 
  TRUE,  // 
  TRUE,  // 
  FALSE,
};

// アニメ差分
static const u8 _CGEAR_NET_CHANGEPAL_ANM_COUNT_DIF[ _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX ] = 
{
  0,
  12,  // 
  3,  // 
  0,  // 
  0,  // 
};

static const u8 _CGEAR_NET_CHANGEPAL_ANM_MOD_TBL[] = {
   0, 4, 8, 
  10,14,18,
  20,22,24,26,28,
  30,31,31,31,31,31,30,
  28,26,24,22,20,
  18,14,10,
   8, 4, 0,
};

static const u8 _CGEAR_NET_CHANGEPAL_ANM_MOD_CATCH_TBL[] = {
  0, 3, 7, 11, 18,
  23,27,31,31,31,31,31,
  29,27,26,25,24,26,27,
  29,31,31,31,31,31,29,28,27,
   23, 15, 7, 3, 0,
};

static const u8 _CGEAR_NET_CHANGEPAL_ANM_MOD_OFF_TBL[] = {
   0, 4, 8, 
  10,14,18,
  20,22,24,26,28,
  30,31,
};


#define _CGEAR_NET_CHANGEPAL_ANM_MOD_NUM_MAX  ( NELEMS(_CGEAR_NET_CHANGEPAL_ANM_MOD_TBL)-1 )
#define _CGEAR_NET_CHANGEPAL_ANM_MOD_CATCH_NUM_MAX  ( NELEMS(_CGEAR_NET_CHANGEPAL_ANM_MOD_CATCH_TBL)-1 )
#define _CGEAR_NET_CHANGEPAL_ANM_MOD_OFF_NUM_MAX  ( NELEMS(_CGEAR_NET_CHANGEPAL_ANM_MOD_OFF_TBL)-1 )
#define _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX  ( 31 )

#define _CGEAR_NET_CHANGEPAL_ANM_HIGH_TOPWAIT  ( 4 )
#define _CGEAR_NET_CHANGEPAL_ANM_HIGH_COUNT  ( 32)
#define _CGEAR_NET_CHANGEPAL_ANM_CHANGE_COUNT  ( 20)



#define _CGEAR_NET_CHANGEPAL_ANM_NORMAL_TOPWAIT  ( 12 )
#define _CGEAR_NET_CHANGEPAL_ANM_NORMAL_COUNT  ( 100 )

#define _CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_TOPWAIT  ( 8 )
#define _CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_COUNT  ( 70 )

#define _CGEAR_NET_CHANGEPAL_ANM_OFF_COUNT  ( 15 )



//--------------------------

typedef enum{
  _SELECTANIM_NONE,
  _SELECTANIM_WAIT,
  _SELECTANIM_STANDBY,
  _SELECTANIM_ANIMING,
  _SELECTANIM_RUN,
  _SELECTANIM_END,


} _SELECTANIM_ENUM;
#define _SELECT_ANIME_WAIT (16)

#define _SELECT_ANIME_OBJ_MAX ( 20 )

//-------------------------------------
///	パネルパレット情報
//=====================================
enum {

  PANEL_PLTT_NONE = 0x6,
  PANEL_PLTT_0 = 0x5,
  PANEL_PLTT_1 = 0x4,
  PANEL_PLTT_2 = 0x3,
  PANEL_PLTT_3 = 0x2,
  PANEL_PLTT_HIGH = 0x1,

  _CGEAR_NET_CHANGEPAL_HIGH = 0,
  _CGEAR_NET_CHANGEPAL_3,
  _CGEAR_NET_CHANGEPAL_2,
  _CGEAR_NET_CHANGEPAL_1,
  _CGEAR_NET_CHANGEPAL_0,
  _CGEAR_NET_CHANGEPAL_NUM, // 5
} ;




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
  PANEL_COLOR_PATTERN_NUM = _CGEAR_NET_CHANGEPAL_NUM,

  // アニメーションフレーム
  PANEL_ANIME_DEF_FRAME = 2,
  PANEL_ANIME_MIDDLE_FRAME = 3,
  PANEL_ANIME_SLOW_FRAME = 4,
  

};

typedef enum{
  PANEL_VIEW_NORMAL,  //通常
  PANEL_VIEW_CUSTOM,  //カスタムモードのアクティブ状態
  PANEL_VIEW_DRUG,    //ドラッグモード
  PANEL_VIEW_DRAW_OFF,     //
}PANEL_VIEW_MODE;

static const u8 sc_PANEL_COLOR_ANIME_END_INDEX[ PANEL_COLOR_TYPE_MAX ][ 9 ] = {
  // NONE
  { 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  // RED
  {  0, 1, 2, 3, 4, 3, 2, 1, 0 },
  // YELLOW
  {  0, 1, 2, 3, 4, 3, 2, 1, 0 },
  // BLUE
  {  0, 1, 2, 3, 4, 3, 2, 1, 0 },
  // BASE
  {  0, 1, 2, 3, 4, 3, 2, 1, 0 },
};
// カラーPATTERN情報
static const u8 sc_PANEL_COLOR_ANIME_TBL[ PANEL_COLOR_TYPE_MAX ][ PANEL_COLOR_PATTERN_NUM ] = 
{
  // NONE
  { PANEL_PLTT_NONE, PANEL_PLTT_0, PANEL_PLTT_1, PANEL_PLTT_2, PANEL_PLTT_3 },
  // RED
  { PANEL_PLTT_0, PANEL_PLTT_1, PANEL_PLTT_2, PANEL_PLTT_3, PANEL_PLTT_HIGH },
  // YELLOW
  { PANEL_PLTT_0, PANEL_PLTT_1, PANEL_PLTT_2, PANEL_PLTT_3, PANEL_PLTT_HIGH },
  // BLUE
  { PANEL_PLTT_0, PANEL_PLTT_1, PANEL_PLTT_2, PANEL_PLTT_3, PANEL_PLTT_HIGH },
  // BASE
  { PANEL_PLTT_0, PANEL_PLTT_1, PANEL_PLTT_2, PANEL_PLTT_3, PANEL_PLTT_HIGH },
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


//--------------------------
//セルアクター関連
//--------------------------

///ユーザーレンダラー
enum {
  CGEAR_REND_SUB,

  CGEAR_REND_MAX,
} ;
static const GFL_REND_SURFACE_INIT sc_REND_SURFACE_INIT[ CGEAR_REND_MAX ] = {
  {
    0, 0,
    256, 192,
    CLSYS_DRAW_SUB,
    CLSYS_REND_CULLING_TYPE_NONE,
  },
};

//セルアクターリソース種類定義
typedef enum{
  _CLACT_PLT, //パレット
  _CLACT_CHR, //コモンキャラクター
  _CLACT_CHR_PANEL, //デザインによって挿し替わるパネルキャラクター
  _CLACT_ANM, //コモンアニメ
  _CLACT_ANM_PANEL, //デザインによって挿し替わるパネルアニメ
  _CLACT_RES_MAX,
} _CLACT_RES_TYPE;

//セルアクターアニメーション種類定義
typedef enum{
  _CLACT_RES_COMMON,  //コモン
  _CLACT_RES_PANEL,   //パネルデザイン
}_CLACT_RES_IDX;

enum{
  ACT_T_HOUR1,
  ACT_T_HOUR2,
  ACT_T_COLON,
  ACT_T_MIN1,
  ACT_T_MIN2,

  ACT_AMPM_BASE,
  ACT_AMPM,
  ACT_BATTERY_BASE,
  ACT_BATTERY,
  ACT_LOGO,
  ACT_POWER,
  //↑電源Off時でも表示
  ACT_HELP,
  ACT_CUSTOM,
  ACT_S_RADAR,
  //↑Ring4に属してる(ACT_LOGO以外)
  ACT_CROSS,
  ACT_CONNECT,
  //↑Ring1に属してる
  ACT_IR,
  ACT_IR_RING,
  ACT_IR_STR,
  ACT_WIRELESS,
  ACT_WIRELESS_RING,
  ACT_WIRELESS_STR,
  ACT_WIFI,
  ACT_WIFI_RING,
  ACT_WIFI_STR,

  ACT_DEF_DRAW_MAX, //デフォルト描画するものMax

  ACT_PANEL_EFF = ACT_DEF_DRAW_MAX,
  ACT_MAX,
};

#define ACT_TIME_MAX    (ACT_T_MIN2-ACT_T_HOUR1+1)
#define ACT_NORMAL_MAX  (ACT_MAX-ACT_AMPM_BASE) //初期化時だけ使用してる
#define ACT_COMMON_MAX  ( ACT_POWER ) //電源Offでも表示するOBJIDのmax
#define ACT_PANEL_SET_NUM (3)
#define ACT_R4_OBJ_START  (ACT_T_HOUR1) //Ring4に属してる
#define ACT_R4_OBJ_END    (ACT_S_RADAR)

enum{
 ACTANM_N10,
 ACTANM_AMPM,
 ACTANM_N_COLON,
 ACTANM_CROSS,
 ACTANM_BATT3,
 ACTANM_BATT2,
 ACTANM_POWER,
 ACTANM_POWER_ANM,
 ACTANM_HELP,
 ACTANM_HELP_ANM,
 ACTANM_CUSTOM,
 ACTANM_CUSTOM_ANM,
 ACTANM_S_RADAR,
 ACTANM_S_RADAR_ANM,
 ACTANM_S_RADAR_OFF,
 ACTANM_S_RADAR_INFO,
 ACTANM_CONNECT,
 ACTANM_CONNECT_OFF,
 ACTANM_FMISSION,
 ACTANM_FMISSION_OFF,
 ACTANM_RIPPLE,
 ACTANM_WIN,
 ACTANM_LOGO_ALL,
 ACTANM_S_IR,
 ACTANM_S_WIRELESS,
 ACTANM_S_WIFI,
 ACTANM_RING_IR,
 ACTANM_RING_WIRELESS,
 ACTANM_RING_WIFI,
 ACTANM_LOOP_IR,
 ACTANM_LOOP_WIRELSS,
 ACTANM_LOOP_WIFI,
 ACTANM_E_IR,
 ACTANM_E_WIRELSS,
 ACTANM_E_WIFI,
};

enum{
 ACTANM_IR,
 ACTANM_WIRELESS,
 ACTANM_WIFI,
};

enum{
  ANM_OFS_PANEL_NORMAL = ACTANM_IR,
  ANM_OFS_PANEL_RING = ACTANM_RING_IR,
  ANM_OFS_PANEL_LOOP = ACTANM_LOOP_IR,
  ANM_OFS_PANEL_EFF = ACTANM_E_IR,
};

#if 0
// 表示OAMの時間とかの最大
enum{
  _CLACT_TIME_AMPM,
  _CLACT_TIME_NUMBER_2,
  _CLACT_TIME_NUMBER_10A,
  _CLACT_TIME_COLON,
  _CLACT_TIME_NUMBER_6,
  _CLACT_TIME_NUMBER_10B,
  _CLACT_BATT,
  _CLACT_LOGO,
  _CLACT_HELP,
  _CLACT_EDITMARKON,
  _CLACT_POWER,
  _CLACT_TIMEPARTS_MAX,
};
#endif
enum{
 ACT_SPRI_PANEL_DRUG,      //ドラッグ時のプライオリティ
 ACT_SPRI_PARTS,
 ACT_SPRI_BASE, 
 ACT_SPRI_EFFECT,
 ACT_SPRI_PANEL,
 ACT_SPRI_PANEL_BASE,
 ACT_SPRI_PANEL_RING,
 ACT_SPRI_CROSS,
};


#define POS_AMPM_BASE_X   (8*6)
#define POS_AMPM_BASE_Y   (22)
#define POS_AMPM_BASE_R   (16)
#define POS_T_NUM_X       (POS_AMPM_BASE_X-16)
#define POS_T_NUM_Y       (POS_AMPM_BASE_Y-4)
#define POS_AMPM_X        (POS_AMPM_BASE_X-8)
#define POS_AMPM_Y        (POS_T_NUM_Y+8)
#define POS_T_NUM_OX      (6)

#define POS_BATTERY_BASE_X  (26*8)
#define POS_BATTERY_BASE_Y  (POS_AMPM_BASE_Y)
#define POS_BATTERY_BASE_R  (16)
#define POS_COMM_ICON_X     (POS_BATTERY_BASE_X-8)
#define POS_COMM_ICON_Y     (POS_BATTERY_BASE_Y-12)
#define POS_BATTERY_X       (POS_BATTERY_BASE_X-8)
#define POS_BATTERY_Y       (POS_COMM_ICON_Y+16+1)

#define POS_LOGO_X     ( 16*8 )
#define POS_LOGO_Y     ( 1*8+2  )
#define POS_POWER_X    ( 25*8 )
#define POS_POWER_Y    ( 22*8+4 )
#define POS_POWER_R    ( 10 )
#define POS_HELP_X     ( 27*8-1 )
#define POS_HELP_Y     ( 20*8+3 )
#define POS_HELP_R     ( POS_POWER_R )
#define POS_CUSTOM_X   ( 29*8-4 )
#define POS_CUSTOM_Y   ( 18*8 )
#define POS_CUSTOM_R   ( POS_POWER_R )

#define POS_CROSS_X        ( 128 )
#define POS_CROSS_Y        ( 12*8 )
#define POS_CONNECT_X       ( POS_CROSS_X )
#define POS_CONNECT_Y       ( POS_CROSS_Y-10 )
#define POS_S_RADAR_X       ( 8*4+3 )
#define POS_S_RADAR_Y       ( 20*8 )
#define POS_S_RADAR_R       ( 15 )

#define POS_IR_DEF_X        ( 68 )
#define POS_IR_DEF_Y        ( 60 )
#define POS_WIRELESS_DEF_X  ( 128 )
#define POS_WIRELESS_DEF_Y  ( 164 )
#define POS_WIFI_DEF_X      ( 188 )
#define POS_WIFI_DEF_Y      ( 60 )
#define POS_PANEL_R         (16)


#define TP_AMPM_BASE_X     ( POS_AMPM_BASE_X )
#define TP_AMPM_BASE_Y     ( POS_AMPM_BASE_Y )
#define TP_AMPM_BASE_R     ( 16 )
#define TP_BATTERY_BASE_X  ( POS_BATTERY_BASE_X )
#define TP_BATTERY_BASE_Y  ( POS_BATTERY_BASE_Y )
#define TP_BATTERY_BASE_R  ( 16 )
#define TP_POWER_X         ( POS_POWER_X )
#define TP_POWER_Y         ( POS_POWER_Y )
#define TP_POWER_R         ( 10 )
#define TP_HELP_X          ( POS_HELP_X )
#define TP_HELP_Y          ( POS_HELP_Y )
#define TP_HELP_R          ( TP_POWER_R )
#define TP_CUSTOM_X        ( POS_CUSTOM_X )
#define TP_CUSTOM_Y        ( POS_CUSTOM_Y )
#define TP_CUSTOM_R        ( TP_POWER_R )
#define TP_S_RADAR_X       ( POS_S_RADAR_X )
#define TP_S_RADAR_Y       ( POS_S_RADAR_Y )
#define TP_S_RADAR_R       ( 15 )

#define TP_CROSS_T        ( 10*8+4 )
#define TP_CROSS_B        ( TP_CROSS_T+24 )
#define TP_CROSS_L        ( 10*8+4 )
#define TP_CROSS_R        ( TP_CROSS_L+11*8 )
#define TP_LOGO_T          ( POS_LOGO_Y )
#define TP_LOGO_B          ( TP_LOGO_T+16  )
#define TP_LOGO_L          ( POS_LOGO_X - 26 )
#define TP_LOGO_R          ( TP_LOGO_L + 6*8+4  )

#define TP_PANEL_R          (16+4)

#define TP_CIRCLE_CENTER_X  (16*8)
#define TP_CIRCLE_CENTER_Y  (12*8)
#define TP_CIRCLE0_OUT_R    (4*8+2)
#define TP_CIRCLE0_IN_R     (TP_CIRCLE0_OUT_R-5)
#define TP_CIRCLE1_OUT_R    (6*8+2)
#define TP_CIRCLE1_IN_R     (TP_CIRCLE1_OUT_R-5)
#define TP_CIRCLE2_OUT_R    (8*8+4+2)
#define TP_CIRCLE2_IN_R     (TP_CIRCLE2_OUT_R-5)
#define TP_CIRCLE3_OUT_R    (11*8+2)
#define TP_CIRCLE3_IN_R     (TP_CIRCLE3_OUT_R-5)
#define TP_CIRCLE4_OUT_R    (14*8+2)
#define TP_CIRCLE4_IN_R     (TP_CIRCLE4_OUT_R-5)

// タイプ
//#define _CLACT_TYPE_MAX (3)

#define _SLEEP_START_TIME_WAIT (16)
#define _SLEEP_COLOR_CHANGE_WAIT (8)


// スリープ復帰アニメ
enum{
  BOOT_ANIME_SEQ_ALL_ON,
  BOOT_ANIME_SEQ_ALL_WAIT,
  BOOT_ANIME_SEQ_SMOOTHIN_MAIN,
  BOOT_ANIME_SEQ_END,
};

// 機能選択時ボタンアニメーション種類
enum{
  SELECT_CURSOR_ANM_PANEL,  //IR,WIRELESS,WIFIボタンのアニメ
  SELECT_CURSOR_ANM_PAL,    //パレットアニメ再生
  SELECT_CURSOR_ANM_ACT,    //OBJアニメ再生
};


#if 0
typedef struct {
  int leftx;
  int lefty;
  int width;
  int height;
} _WINDOWPOS;
#endif
typedef struct {
  u8 time;
  u8 x;
  u8 y;
  u8 frame;
} _ANIM_DATA;

// スリープ復帰
const static _ANIM_DATA screenTable[]={
  { 0, 0, 0, PANEL_ANIME_DEF_FRAME},
  { 0, 0, 1, PANEL_ANIME_DEF_FRAME},
  { 0, 0, 2, PANEL_ANIME_DEF_FRAME},
  { 0, 8, 0, PANEL_ANIME_DEF_FRAME},
  { 0, 8, 1, PANEL_ANIME_DEF_FRAME},
  { 0, 8, 2, PANEL_ANIME_DEF_FRAME},

  { 1, 1, 0, PANEL_ANIME_DEF_FRAME},
  { 1, 1, 1, PANEL_ANIME_DEF_FRAME},
  { 1, 1, 2, PANEL_ANIME_DEF_FRAME},
  { 1, 1, 3, PANEL_ANIME_DEF_FRAME},
  { 1, 7, 0, PANEL_ANIME_DEF_FRAME},
  { 1, 7, 1, PANEL_ANIME_DEF_FRAME},
  { 1, 7, 2, PANEL_ANIME_DEF_FRAME},
  { 1, 7, 3, PANEL_ANIME_DEF_FRAME},

  { 2, 2, 0, PANEL_ANIME_DEF_FRAME},
  { 2, 2, 1, PANEL_ANIME_DEF_FRAME},
  { 2, 2, 2, PANEL_ANIME_DEF_FRAME},
  { 2, 6, 0, PANEL_ANIME_DEF_FRAME},
  { 2, 6, 1, PANEL_ANIME_DEF_FRAME},
  { 2, 6, 2, PANEL_ANIME_DEF_FRAME},

  { 3, 3, 0, PANEL_ANIME_DEF_FRAME},
  { 3, 3, 1, PANEL_ANIME_DEF_FRAME},
  { 3, 3, 2, PANEL_ANIME_DEF_FRAME},
  { 3, 3, 3, PANEL_ANIME_DEF_FRAME},
  { 3, 5, 0, PANEL_ANIME_DEF_FRAME},
  { 3, 5, 1, PANEL_ANIME_DEF_FRAME},
  { 3, 5, 2, PANEL_ANIME_DEF_FRAME},
  { 3, 5, 3, PANEL_ANIME_DEF_FRAME},

  { 4, 4, 0, PANEL_ANIME_DEF_FRAME},
  { 4, 4, 2, PANEL_ANIME_DEF_FRAME},
  { 4, 4, 1, PANEL_ANIME_DEF_FRAME},
};

#if 0
// 起動アニメ
#define START_UP_SCREEN_ANIME_FRAME_MAX ( 126 )
#define START_UP_SCREEN_ANIME_FRAME_START (0)
#define START_UP_SCREEN_ANIME_FRAME_NEXT (56)
#define START_UP_SCREEN_ANIME_FRAME_LAST (96)
const static _ANIM_DATA StartUpScreenTable[]={
  // 最初のいっこ
  { START_UP_SCREEN_ANIME_FRAME_START, 4, 1, PANEL_ANIME_SLOW_FRAME},

  // まわりの４こ
  { START_UP_SCREEN_ANIME_FRAME_NEXT+0, 2, 1, PANEL_ANIME_MIDDLE_FRAME},
  { START_UP_SCREEN_ANIME_FRAME_NEXT+2, 6, 1, PANEL_ANIME_MIDDLE_FRAME},
  { START_UP_SCREEN_ANIME_FRAME_NEXT+4, 4, 0, PANEL_ANIME_MIDDLE_FRAME},
  { START_UP_SCREEN_ANIME_FRAME_NEXT+6, 4, 2, PANEL_ANIME_MIDDLE_FRAME},


  // 最後のいっぱい出るところ 斜め対象的に点滅する。　こまおくりでみてみてください。
  { 0+START_UP_SCREEN_ANIME_FRAME_LAST, 2, 0, PANEL_ANIME_DEF_FRAME},
  { 3+START_UP_SCREEN_ANIME_FRAME_LAST, 6, 2, PANEL_ANIME_DEF_FRAME},
  { 5+START_UP_SCREEN_ANIME_FRAME_LAST, 3, 1, PANEL_ANIME_DEF_FRAME},
  { 7+START_UP_SCREEN_ANIME_FRAME_LAST, 5, 2, PANEL_ANIME_DEF_FRAME},
  { 8+START_UP_SCREEN_ANIME_FRAME_LAST, 1, 2, PANEL_ANIME_DEF_FRAME},
  { 8+START_UP_SCREEN_ANIME_FRAME_LAST, 7, 1, PANEL_ANIME_DEF_FRAME},
  { 9+START_UP_SCREEN_ANIME_FRAME_LAST, 1, 1, PANEL_ANIME_DEF_FRAME},
  { 9+START_UP_SCREEN_ANIME_FRAME_LAST, 7, 2, PANEL_ANIME_DEF_FRAME},
  {10+START_UP_SCREEN_ANIME_FRAME_LAST, 3, 2, PANEL_ANIME_DEF_FRAME},
  {10+START_UP_SCREEN_ANIME_FRAME_LAST, 5, 1, PANEL_ANIME_DEF_FRAME},
  {11+START_UP_SCREEN_ANIME_FRAME_LAST, 2, 2, PANEL_ANIME_DEF_FRAME},
  {11+START_UP_SCREEN_ANIME_FRAME_LAST, 6, 0, PANEL_ANIME_DEF_FRAME},
  {12+START_UP_SCREEN_ANIME_FRAME_LAST, 2, 1, PANEL_ANIME_DEF_FRAME},
  {12+START_UP_SCREEN_ANIME_FRAME_LAST, 6, 1, PANEL_ANIME_DEF_FRAME},

  {13+START_UP_SCREEN_ANIME_FRAME_LAST, 3, 0, PANEL_ANIME_DEF_FRAME},
  {13+START_UP_SCREEN_ANIME_FRAME_LAST, 5, 3, PANEL_ANIME_DEF_FRAME},
  {14+START_UP_SCREEN_ANIME_FRAME_LAST, 1, 0, PANEL_ANIME_DEF_FRAME},
  {14+START_UP_SCREEN_ANIME_FRAME_LAST, 0, 0, PANEL_ANIME_DEF_FRAME},
  {15+START_UP_SCREEN_ANIME_FRAME_LAST, 7, 3, PANEL_ANIME_DEF_FRAME},
  {15+START_UP_SCREEN_ANIME_FRAME_LAST, 8, 2, PANEL_ANIME_DEF_FRAME},
  {16+START_UP_SCREEN_ANIME_FRAME_LAST, 0, 1, PANEL_ANIME_DEF_FRAME},
  {16+START_UP_SCREEN_ANIME_FRAME_LAST, 0, 2, PANEL_ANIME_DEF_FRAME},
  {17+START_UP_SCREEN_ANIME_FRAME_LAST, 8, 2, PANEL_ANIME_DEF_FRAME},
  {17+START_UP_SCREEN_ANIME_FRAME_LAST, 8, 1, PANEL_ANIME_DEF_FRAME},
  {17+START_UP_SCREEN_ANIME_FRAME_LAST, 4, 0, PANEL_ANIME_DEF_FRAME},
  {18+START_UP_SCREEN_ANIME_FRAME_LAST, 1, 3, PANEL_ANIME_DEF_FRAME},
  {18+START_UP_SCREEN_ANIME_FRAME_LAST, 3, 3, PANEL_ANIME_DEF_FRAME},
  {18+START_UP_SCREEN_ANIME_FRAME_LAST, 4, 2, PANEL_ANIME_DEF_FRAME},
  {19+START_UP_SCREEN_ANIME_FRAME_LAST, 7, 0, PANEL_ANIME_DEF_FRAME},
  {19+START_UP_SCREEN_ANIME_FRAME_LAST, 5, 0, PANEL_ANIME_DEF_FRAME},
  {19+START_UP_SCREEN_ANIME_FRAME_LAST, 4, 1, PANEL_ANIME_DEF_FRAME},
};

#endif

enum{
  TOUCH_LABEL_EGNORE1,  //無視 時計パネルとバッテリーパネル(一応ヒットさせて、円の当たり判定に抜きを作っている)
  TOUCH_LABEL_EGNORE2,
  TOUCH_LABEL_POWER,
  TOUCH_LABEL_HELP,
  TOUCH_LABEL_CUSTOM,
  TOUCH_LABEL_S_RADAR,
  TOUCH_LABEL_CROSS,
  TOUCH_LABEL_LOGO,

  TOUCH_LABEL_IR,
  TOUCH_LABEL_WIRELESS,
  TOUCH_LABEL_WIFI,

  TOUCH_LABEL_RING0,
  TOUCH_LABEL_RING1,
  TOUCH_LABEL_RING2,
  TOUCH_LABEL_RING3,
  TOUCH_LABEL_RING4,
  TOUCH_LABEL_MAX,
};

static const GFL_UI_TP_HITTBL DATA_FixedButtonTbl[] = {  //上下左右
  { GFL_UI_TP_USE_CIRCLE, TP_AMPM_BASE_X,     TP_AMPM_BASE_Y,     TP_AMPM_BASE_R },
  { GFL_UI_TP_USE_CIRCLE, TP_BATTERY_BASE_X,  TP_BATTERY_BASE_Y,  TP_BATTERY_BASE_R },
  
  { GFL_UI_TP_USE_CIRCLE, TP_POWER_X,   TP_POWER_Y,   TP_POWER_R },
  { GFL_UI_TP_USE_CIRCLE, TP_HELP_X,    TP_HELP_Y,    TP_HELP_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CUSTOM_X,  TP_CUSTOM_Y,  TP_CUSTOM_R },
  { GFL_UI_TP_USE_CIRCLE, TP_S_RADAR_X, TP_S_RADAR_Y, TP_S_RADAR_R },
  
  { TP_CROSS_T,  TP_CROSS_B,  TP_CROSS_L,  TP_CROSS_R },
  { TP_LOGO_T,   TP_LOGO_B,   TP_LOGO_L,   TP_LOGO_R },
#if 0
//  {	PANEL_Y2 * 8,  (PANEL_Y2 * 8) + (PANEL_SIZEXY * 8 * PANEL_HEIGHT2), 0,255 },   //タッチパネル全部
  { (POS_CUSTOM_Y-8), (POS_CUSTOM_Y+8), (POS_CUSTOM_X-8), (POS_CUSTOM_X+8) },        //こうさく
  { (POS_HELP_Y-8), (POS_HELP_Y+8), (POS_HELP_X-8), (POS_HELP_X+8) },   //HELP
  { (POS_CROSS_Y_CENTER-8), (POS_CROSS_Y_CENTER+8), POS_CROSS_X_CENTER-40, POS_CROSS_X_CENTER+40 },                //SURECHIGAI
  { (POS_S_RADAR_Y-8), (POS_S_RADAR_Y+8), (POS_S_RADAR_X-8), (POS_S_RADAR_X+8) }, //RADAR
  { 22-8, (22+8), 128-32, 128+32 },    //CGEARLOGO
  { (POS_POWER_Y-8), (POS_POWER_Y+8), (POS_POWER_X-8), (POS_POWER_X+8) },        //電源
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
#endif
};

#define CGEAR_BUTTON_FIXED_TBL_MAX      ( NELEMS(DATA_FixedButtonTbl) )
#define CGEAR_BUTTON_ALL_TBL_MAX        ( CGEAR_BUTTON_FIXED_TBL_MAX+4 )  //fixed + (panel*3) + end_code
#define CGEAR_BUTTON_POWER_ON_ONLY_MAX  (3)

static const GFL_UI_TP_HITTBL DATA_FixedCircleTbl[] = {  //code, x, y, r
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE0_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE0_OUT_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE1_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE1_OUT_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE2_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE2_OUT_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE3_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE3_OUT_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE4_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE4_OUT_R },
  {GFL_UI_TP_HIT_END,0,0,0},		 //終了データ
};

//-------------------------------------
///	パネルポジションデータ
//=====================================
typedef struct _PANEL_POS_DATA{
  u8  x,y;
  u8  ring_id;
}PANEL_POS_DATA;

static const PANEL_POS_DATA DATA_PanelPosTbl[] = {
  {	16	,	96 	,	4	},  ///<	0
  {	40	,	96 	,	3	},  ///<	1
  {	60	,	96 	,	2	},  ///<	2
  {	196	,	96 	,	2	},  ///<	3
  {	216	,	96 	,	3	},  ///<	4
  {	240	,	96 	,	4	},  ///<	5
  {	128	,	64 	,	0	},  ///<	6
  {	128	,	128 	,	0	},  ///<	7
  {	128	,	144 	,	1	},  ///<	8
  {	128	,	164 	,	2	},  ///<	9
  {	128	,	184 	,	3	},  ///<	10
  {	52	,	52 	,	3	},  ///<	11
  {	68	,	60 	,	2	},  ///<	12
  {	88	,	72 	,	1	},  ///<	13
  {	168	,	120 	,	1	},  ///<	14
  {	188	,	132 	,	2	},  ///<	15
  {	204	,	140 	,	3	},  ///<	16
  {	52	,	140 	,	3	},  ///<	17
  {	68	,	132 	,	2	},  ///<	18
  {	88	,	120 	,	1	},  ///<	19
  {	168	,	72 	,	1	},  ///<	20
  {	188	,	60 	,	2	},  ///<	21
  {	204	,	52 	,	3	},  ///<	22
  {	84	,	20 	,	3	},  ///<	23
  {	96	,	36 	,	2	},  ///<	24
  {	104	,	56 	,	1	},  ///<	25
  {	112	,	68 	,	0	},  ///<	26
  {	144	,	124 	,	0	},  ///<	27
  {	152	,	136 	,	1	},  ///<	28
  {	160	,	156 	,	2	},  ///<	29
  {	172	,	172 	,	3	},  ///<	30
  {	84	,	172 	,	3	},  ///<	31
  {	96	,	156 	,	2	},  ///<	32
  {	104	,	136 	,	1	},  ///<	33
  {	112	,	124 	,	0	},  ///<	34
  {	144	,	68 	,	0	},  ///<	35
  {	152	,	56 	,	1	},  ///<	36
  {	160	,	36 	,	2	},  ///<	37
  {	172	,	20 	,	3	},  ///<	38
};
#define PANEL_POS_TBL_MAX (NELEMS(DATA_PanelPosTbl))
#define PANEL_POS_NULL    (0xFF)  //パネルポジションの無効値

#define PANEL_DISTANCE_ENABLE_MAX (FX32_CONST(32)) //この値以上に距離が離れてたら無視

//-------------------------------------
///	起動アニメーション
//=====================================
#if 0
#define STARTUP_ANIME_OBJ_MAX ( 18 )      // 起動アニメーションに必要なオブジェ数
#define STARTUP_ANIME_INDEX_START ( ACTANM_AMPM )  // 各オブジェに設定するアニメーション開始インデックス
#define STARTUP_ANIME_POS_X (128)
#define STARTUP_ANIME_POS_Y (96)
#define STARTUP_OAM_ALPHA_ANIME_FRAME_MAX ( 8 )
#define STARTUP_TBL_ALPHA_TIME_WAIT ( 8 )
#define STARTUP_END_TIME_WAIT ( 8 )
#endif
#define STARTUP_ANM_MODE_BOOT     (0) //初回起動
#define STARTUP_ANM_MODE_WAKEUP   (1) //スリープ復帰
#define STARTUP_ANM_RING_ANM_INTERVAL (5)
enum
{
  STARTUP_BOOT_SEQ_ANIME_START,
  STARTUP_BOOT_SEQ_WIPE_IN,
  STARTUP_BOOT_SEQ_ANIME_WAIT,
  STARTUP_BOOT_SEQ_END,
  STARTUP_BOOT_SEQ_SKIP,

  STARTUP_WAKEUP_SEQ_ANIME_START = 0,
  STARTUP_WAKEUP_SEQ_ANIME_WAIT,
};



//-------------------------------------------------------------------------
///	文字表示色定義(default)	-> gflib/fntsys.hへ移動
//------------------------------------------------------------------

#define WINCLR_COL(col)	(((col)<<4)|(col))



//--------------------------------------------
// 内部ワーク
//--------------------------------------------


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_DECAL_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_BUTTON_FRAME   (GFL_BG_FRAME0_S)
#define GEAR_FB_MESSAGE   (GFL_BG_FRAME3_S)

#define CGEAR_CLACT_BG_PRI  ( 2 )




typedef void (StateFunc)(C_GEAR_WORK* pState);
//typedef BOOL (TouchFunc)(int no, C_GEAR_WORK* pState);


//-------------------------------------
///	演出待ちイベントワーク
//=====================================
typedef struct {
  C_GEAR_WORK* pWork;
  u32 createEvent;
} EV_CGEAR_EFFECTWAIT;

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


//-------------------------------------
///	３カウンター
//=====================================
typedef struct {
  u16 plt_count[ CGEAR_PANEL_MAX ];
  u16 count_max;
  u8 count_reset;
  u8 play;
} PLT_COUNTER;


//-------------------------------------
///	パレット演出コントロール
//=====================================
typedef struct{
  PLT_COUNTER counter[ _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX ];
  u8  plt_anmtype[ CGEAR_PANEL_MAX ];
  
  u8  wireles_count;

  u8 ex_allreset;
} PANEL_PLT_ANIME;

//-------------------------------------
///	ボタンパレットフェード処理
//=====================================
typedef struct {
  u8  frame;
  u32 msk;
//  GFL_CLWK* dummy;
  PALETTE_FADE_PTR  p_fade_ptr;
  GFL_TCBSYS* p_fade_tcbsys;
} BUTTON_PAL_FADE;


//-------------------------------------
///	SEL選択エフェクトワーク
//=====================================
typedef struct {
  GFL_CLWK* cellSelect;
  int x;
  int y;
} SEL_SELECT_EFFECT;



struct _C_GEAR_WORK {
  StateFunc* state;      ///< ハンドルのプログラム状態
//  TouchFunc* touch;
  int selectType;   // 接続タイプ
  HEAPID heapID;
//  GFL_BUTTON_MAN* pButton;
//  GFL_ARCUTIL_TRANSINFO subchar;
  FIELD_SUBSCREEN_WORK* subscreen;
  GAMESYS_WORK* pGameSys;
  CGEAR_SAVEDATA* pCGSV;
  ARCHANDLE* handle;  // アーカイブハンドル
  u32 objRes[_CLACT_RES_MAX];  //CLACTリソース

  GFL_CLUNIT *cellUnit;
  GFL_CLSYS_REND* userRend;
  SEL_SELECT_EFFECT cellSelect[_SELECT_ANIME_OBJ_MAX];
//  GFL_CLWK  *cellTime[ACT_TIME_MAX];
  GFL_CLWK  *cellCursor[ACT_MAX];
#if 0
  GFL_CLWK  *cellType[_CLACT_TYPE_MAX];
  GFL_CLWK  *cellCross;
  GFL_CLWK  *cellCrossBase;
  GFL_CLWK  *cellRadar;
  GFL_CLWK  *cellStartUp[STARTUP_ANIME_OBJ_MAX];
#endif
  u16 crossColor[CROSS_COLOR_MAX]; //すれ違いカラー

  STARTUP_ENDCALLBACK* pCall;
  void* pWork;

  GFL_TCBLSYS* tcbsys;      //通常tcb動作用
  GFL_TCBSYS* pfade_tcbsys; //PaletteFadeReq専用
  GFL_TCB*                    vblank_tcb;
  void* pfade_tcbsys_wk;
  PALETTE_FADE_PTR            pfade_ptr;
 
  int eff_task_ct;
  int createEvent;
  int doEvent;

  BUTTON_PAL_FADE button_palfade;

  u16 palBase[CGEAR_PANEL_MAX]; //パネルのステータスパレットアニメ
  u16 palTrans[CGEAR_PANEL_MAX];
  u16 palChangeType[ CGEAR_PANEL_MAX ];
  u16 palChangeCol[_CGEAR_NET_PALTYPE_MAX][_CGEAR_NET_CHANGEPAL_NUM];

  u16 palRing[RING_COLOR_MAX][RING_COLOR_SET_NUM_ALL];  //リングカラー
#if 0
  u16 tpx;
  u16 tpy;
#endif
  // パネル
//  PANEL_MARK_ANIME panel_mark[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];

  u8 typeAnim[C_GEAR_PANEL_WIDTH][C_GEAR_PANEL_HEIGHT];
  u8 select_panel;
  u8 select_count;

  // 選択したボタンとボタンアニメの処理に必要なパラメータの保存場所
  u8 select_cursor;
  u8 select_cursor_anm; //選択後処理に引き渡す汎用アニメパラメータ
  u8 select_cursor_anm_type; //選択したボタンのアニメの種類

  // パレットアニメーション
  PANEL_PLT_ANIME plt_anime;

  // すれ違いカラーテーブル
  u8 cross_change;

  u8 GetOpenTrg;
  u8 touchx;    //タッチされた場所
  u8 touchy;    //タッチされた場所
  u8 bAction;
  u8 state_seq;
  u8 eff_seq;   //エフェクト制御用サブシーケンス
  u8 startCounter;
  u8 bPanelEdit;    //カスタムモード？
  u8 bPanelDrug;    //パネルドラッグモード？
  u8 design_no;     //パネルデザインNo
  u8 panel_pos[CGEAR_PANEL_MAX];   //パネルポジションID
  u8 ring_col[RING_NUM];    //リングカラーNo
  u8 sex;
  u8 power_flag;    // 電源がはいっているか？
  u8 connect_flag;  // 一人分でも通信バッファにデータがあるか？
  u8 fmission_flag; // フェスミッション通知が来ているか？
  u8 radar_flag;    // 調査隊依頼の達成通知が来ているか？
  u8 use_skip;    // スキップ許可

  u8 sleep_mode;  // イベント中のスリープモード
  u8 net_sleep_mode;  // ネットスリープモード
  u8 sleep_color_req;
  u8 sleep_color_chenage_wait;

  u8 ex_col_change;  // wireless強制カラー変更
  u8 ex_col_flag;  // wireless強制カラー変更
  u8 tvt_snd;    // 
  u8 intrude;    // 
  u8 ir_wifi_off;
  u8 ex_radar_cross_bttn_col_change;  // wireless強制カラー変更
  u8 ex_radar_cross_bttn_col_change_wait;
};



//-----------------------------------------------
//static 定義
//-----------------------------------------------
static void _changeState(C_GEAR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(C_GEAR_WORK* pWork,StateFunc* state, int line);
static void _modeSelectMenuInit(C_GEAR_WORK* pWork);
static void _modeSelectMenuWaitCore(C_GEAR_WORK* pWork);  // 通常の待機
static void _modeSelectMenuWait(C_GEAR_WORK* pWork);  // 通常の待機
static void _modeSelectMenuWait0(C_GEAR_WORK* pWork);  // 初期入手の演出
static void _modeSelectMenuWait1(C_GEAR_WORK* pWork);  // スリープ復帰の演出
static void _modeSelectMenuWait2(C_GEAR_WORK* pWork);  // 復帰・初期　共通の演出
//static void _gearXY2PanelScreen(int x,int y, int* px, int* py);
static BOOL _modeSelectWaitSubFadeEnd( const C_GEAR_WORK* cpWork );


static void _modeSetSavePanelType( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pSV,int x, int y, CGEAR_PANELTYPE_ENUM type );

// CGEARワーク初期化処理
// CGEARワーク破棄処理
static void cgear_InitWork( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect );
static void cgear_ExitWork( C_GEAR_WORK* pWork );

#if 0
// CGEARセーブ情報の取得
static CGEAR_PANELTYPE_ENUM _cgearSave_GetPanelType(const C_GEAR_WORK* cpWork,int x, int y);
static BOOL _cgearSave_IsPanelTypeIcon(const C_GEAR_WORK* cpWork,int x, int y);
static BOOL _cgearSave_IsPanelTypeLast(const C_GEAR_WORK* cpWork,int x, int y, CGEAR_PANELTYPE_ENUM type );
#endif

// スキップチェック
static BOOL _IsEffectSkip( const C_GEAR_WORK* cpWork );


// サブBGのセットアップ
static void _createSubBg(C_GEAR_WORK* pWork);
static void _setUpSubAlpha( C_GEAR_WORK* pWork );

// スクリーン操作
//static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type);
//static void _gearBootInitScreen(C_GEAR_WORK* pWork);
//static BOOL _gearStartUpMain(C_GEAR_WORK* pWork);

//static void _gearStartUpAllOff(C_GEAR_WORK* pWork);

static void _timeAnimation(C_GEAR_WORK* pWork);
//static void _typeAnimation(C_GEAR_WORK* pWork);
static void _editMarkONOFF(C_GEAR_WORK* pWork,BOOL bOn);
static void _gearArcCreate(C_GEAR_WORK* pWork, int sex, u32 design_no, BOOL pal_trans);
static void _arcGearRelease(C_GEAR_WORK* pWork);
static void _gearObjResCreate(C_GEAR_WORK* pWork, int sex);
static void _gearObjResChange(C_GEAR_WORK* pWork);
static GFL_CLWK* _gearObjAdd(C_GEAR_WORK* pWork, s16 x, s16 y, u8 anm, u8 bg_pri, u8 s_pri, u8 res_idx );
static void _gearObjSetPanelView(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id,  PANEL_VIEW_MODE mode );
static void _gearObjGetPanelPos( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, GFL_CLACTPOS* pos );
static void _gearObjSetPanelPos(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y );
static void _gearObjSetPanelVisible(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, BOOL visible_f );
static void _gearObjSetPos( GFL_CLWK* p_cell, s16 x, s16 y );
static void _gearObjSetAnmSeq( GFL_CLWK* p_cell, int anm_id );
static void _gearObjCreate(C_GEAR_WORK* pWork);
//static void _gearCrossObjCreate(C_GEAR_WORK* pWork);



// OAM操作
//static void _gearMarkObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearAllObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg);
//static void _gearStartUpObjDrawEnabel(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearStartUpAnimeStart(C_GEAR_WORK* pWork);
static void _gearStartUpAnimeEnd(C_GEAR_WORK* pWork);
static BOOL _gearStartUpMainBoot(C_GEAR_WORK* pWork);
static BOOL _gearStartUpMainWakeUp(C_GEAR_WORK* pWork);
static void _taskAdd_StartUpRingAnm( C_GEAR_WORK* pWork, u8 mode, u8 ring_id, u8 ring_col, u8 wait, int* task_ct );
//static BOOL _gearIsEndStartUpObjAnime(const C_GEAR_WORK* cpWork);

static void _gearRingColorChangeCore(C_GEAR_WORK* pWork, u8 ring_id, u8 col_no );
static void _gearRingColorPalAnmCore(C_GEAR_WORK* pWork, u8 ring_id, u8 evy, u16 color  );
static void _gearRingObjVisibleSet(C_GEAR_WORK* pWork, u8 ring_id, BOOL visible_f );
static void _gearRingColorChangePanel(C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, u8 col_no );
static void _gearRingColorChangeInit(C_GEAR_WORK* pWork );
static void _gearRingColorChangeBoot(C_GEAR_WORK* pWork );

static u8 _gearPanelRingIDGet( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id );
static void _gearPanelPosSet( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, u8 pos );
static void _gearPanelPosSetInit( C_GEAR_WORK* pWork );
static fx32 _gearGetDistance( s16 x1, s16 y1, s16 x2, s16 y2 );
static BOOL _gearPanelPosIsHit( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y );
static u8 _gearPanelPosSearchNearyPoint( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y );
static void _gearPanelSetDrugMode( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, BOOL on_f );

// パネルアニメーション処理
static void _PanelPaletteAnimeInit( C_GEAR_WORK* pWork );
static void _PanelPaletteAnimeInitSleep( C_GEAR_WORK* pWork );
static void _PanelPaletteAnimeColClear( C_GEAR_WORK* pWork );
static BOOL _PaletteSetColType( C_GEAR_WORK* pWork, int panel,int type, int anime_type, int setok_anime_type, int setok_panel_type );
static void _PaletteMake(C_GEAR_WORK* pWork,const u16* pltt,int type);
static void _PanelPaletteChange(C_GEAR_WORK* pWork);
static void _PanelPaletteUpdate( C_GEAR_WORK* pWork );
static void _PanelPalette_EXSet( C_GEAR_WORK* pWork, BOOL flag, BOOL ir_wifi_off );
static void _PanelPaletteColorSetUp( C_GEAR_WORK* pWork, int anime_type, int panel_type, BOOL set_first, BOOL ex_col_flag );
static BOOL _PanelPalette_IsExMode( const C_GEAR_WORK* cpWork );

static void _PanelPaletteChangeHigh(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count );
static void _PanelPaletteChangeNormal(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count);
static void _PanelPaletteChangeBeaconCatch(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count);
static void _PanelPaletteChangeChange(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count);
static void _PanelPaletteChangeOff(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count);
static void _PanelPaletteChangeCore( C_GEAR_WORK* pWork, int change_panel, int mod, int mod_max );

#if 0
static void _PanelMarkAnimeSysInit( C_GEAR_WORK* pWork );
static void _PanelMarkAnimeSysMain( C_GEAR_WORK* pWork );
static BOOL _PanelMarkAnimeSysIsAnime( const C_GEAR_WORK* cpWork );
static void _PanelMarkAnimeSysAllStop( C_GEAR_WORK* pWork );

static void _PanelMarkAnimeInit( PANEL_MARK_ANIME* p_mark, int x, int y );
static void _PanelMarkAnimeSetOff( PANEL_MARK_ANIME* p_mark, C_GEAR_WORK* pWork, CGEAR_PANELTYPE_ENUM panel_type );
static void _PanelMarkAnimeStart( PANEL_MARK_ANIME* p_mark, C_GEAR_WORK* pWork, u8 color_type, u8 anime_type, CGEAR_PANELTYPE_ENUM panel_type, u16 frame );
static void _PanelMarkAnimeStop( PANEL_MARK_ANIME* p_mark);
static void _PanelMarkAnimeMain( PANEL_MARK_ANIME* p_mark, const C_GEAR_WORK* cp_work );
static void _PanelMarkAnimeWriteScreen( const PANEL_MARK_ANIME* cp_mark, u32 anime_index );
static BOOL _PanelMarkAnimeIsAnime( const PANEL_MARK_ANIME* cp_mark );

static int _PanelMark_GetPatternIndex( const C_GEAR_WORK* cpWork, u32 col_type, int x, int y );
#endif


// パレットカウンター
static void _PLTCOUNTER_Init( PLT_COUNTER* p_wk, u16 count_max, u16 dif, BOOL count_reset );
static void _PLTCOUNTER_CountUp( PLT_COUNTER* p_wk );
static void _PLTCOUNTER_Reset( PLT_COUNTER* p_wk, u32 panel_type );
static void _PLTCOUNTER_ResetForce( PLT_COUNTER* p_wk, u32 panel_type ); // リセットなしでもリセット
static BOOL _PLTCOUNTER_IsEndAnime( const PLT_COUNTER* cp_wk, u32 panel_type );
static u16 _PLTCOUNTER_GetCount( const PLT_COUNTER* cp_wk, u32 panel_type );
static void _PLTCOUNTER_SetPlay( PLT_COUNTER* p_wk, BOOL flag );

// 選択アニメ
static void _modeSelectAnimInit(C_GEAR_WORK* pWork);
static BOOL _modeSelectAnimWait(C_GEAR_WORK* pWork);
static void _selectAnimInit(C_GEAR_WORK* pWork,int x,int y);

static SEL_SELECT_EFFECT* _selectAnimeGetCleanCLWK( C_GEAR_WORK* pWork );

static void _modeSelectNgAnimInit(C_GEAR_WORK* pWork);
static void _modeSelectNgAnimExit(C_GEAR_WORK* pWork);
static BOOL _modeSelectNgAnimWait(const C_GEAR_WORK* cpWork);
static void _modeStateSelectNgAnimeWait( C_GEAR_WORK* pWork );

// カーソル選択アニメ
static BOOL _CursorSelectAnimeWait( C_GEAR_WORK* pWork );
static BOOL _PalAnimeSelectAnimeWait( C_GEAR_WORK* pWork );

// イベント待ち
static void _modeEventWait( C_GEAR_WORK* pWork );

// スリープモードの管理
static void SleepMode_Start( C_GEAR_WORK* pWork );
static void SleepMode_NetSleepStart( C_GEAR_WORK* pWork );
static void SleepMode_NetSleepEnd( C_GEAR_WORK* pWork );
static void SleepMode_End( C_GEAR_WORK* pWork );
static void SleepMode_ColorUpdate( C_GEAR_WORK* pWork );
static void SleepMode_ColorUpdateEx( C_GEAR_WORK* pWork );
static BOOL SleepMode_IsSleep( const C_GEAR_WORK* cpWork );

// レーダー、すれ違いボタンの色のみ黒く
static void RadarCrossBttn_Off( C_GEAR_WORK* pWork );

// パレットフェード
static void _PFadeSetDefaultPal( C_GEAR_WORK* pWork, BOOL comm );
static void _PFadeSetBlack( C_GEAR_WORK* pWork );
static void _PFadeSetSleepBlack( C_GEAR_WORK* pWork, BOOL on_flag );
static void _PFadeSetExBttnBlack( C_GEAR_WORK* pWork, BOOL on_flag );
static void _PFadeToBlack( C_GEAR_WORK* pWork );
static void _PFadeFromBlack( C_GEAR_WORK* pWork );
static void _PFadeStop( C_GEAR_WORK* pWork );
static BOOL _PFadeIsFade( const C_GEAR_WORK* cpWork );

// パレットフェード　ボタンアニメ
static void _BUTTONPAL_Init( C_GEAR_WORK* pWork, BUTTON_PAL_FADE* p_fwk );
static void _BUTTONPAL_Exit( BUTTON_PAL_FADE* p_fwk );
static void _BUTTONPAL_Start( BUTTON_PAL_FADE* p_fwk, u32 msk );
static BOOL _BUTTONPAL_Update( BUTTON_PAL_FADE* p_fwk );


// CGEAR演出イベント
static GMEVENT* EVENT_ButtonEffectWaitCall( C_GEAR_WORK* pWork, u32 createEvent );
static GMEVENT_RESULT EVENT_ButtonEffectWait( GMEVENT *event, int *seq, void *wk );


//スリープチェック
static void _cgear_SleepCheck( C_GEAR_WORK* pWork, BOOL bAction );

//static void CGEAR_SW_PATTERN_SetUp( CGEAR_SAVEDATA* pCGSV, ARCHANDLE* handle, u32 pattern, HEAPID heapID );


#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG


#if 0
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
#endif


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


#if 0
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

#endif

static void _selectAnimInit(C_GEAR_WORK* pWork,int x,int y)
{
//  GFL_CLWK_DATA cellInitData;
//  int scrx,scry,i;
  SEL_SELECT_EFFECT* sel_select_wk;

#if 0
  _gearXY2PanelScreen(x, y, &scrx, &scry);
  cellInitData.pos_x = scrx * 8+16;
  cellInitData.pos_y = scry * 8+16;
  cellInitData.anmseq = ACTANM_E_IR;// + (pWork->select_panel - CGEAR_PANELTYPE_IR);
  cellInitData.softpri = 0;
  cellInitData.bgpri = CGEAR_CLACT_BG_PRI;
  i = x + y * C_GEAR_PANEL_WIDTH;

  sel_select_wk = _selectAnimeGetCleanCLWK( pWork );

  // もしもうあるなら再始動
  if(sel_select_wk->cellSelect){
    GFL_CLACT_WK_SetAnmSeq( sel_select_wk->cellSelect, cellInitData.anmseq );//BTS:6780
    GFL_CLACT_WK_ResetAnm( sel_select_wk->cellSelect );
    GFL_CLACT_WK_SetTypePos( sel_select_wk->cellSelect, cellInitData.pos_x, CGEAR_REND_SUB, CLSYS_MAT_X );
    GFL_CLACT_WK_SetTypePos( sel_select_wk->cellSelect, cellInitData.pos_y, CGEAR_REND_SUB, CLSYS_MAT_Y );
  }else{
  
    sel_select_wk->cellSelect = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                                pWork->objRes[_CLACT_CHR_PANEL],
                                                pWork->objRes[_CLACT_PLT],
                                                pWork->objRes[_CLACT_ANM_PANEL],
                                                &cellInitData ,
                                                CGEAR_REND_SUB,
                                                pWork->heapID );
    GFL_CLACT_WK_SetAutoAnmFlag( sel_select_wk->cellSelect, TRUE );
    GFL_CLACT_WK_SetDrawEnable( sel_select_wk->cellSelect, TRUE );
  }

  sel_select_wk->x = x;
  sel_select_wk->y = y;
#else
  sel_select_wk = _selectAnimeGetCleanCLWK( pWork );
  
  sel_select_wk->x = x;
  sel_select_wk->y = y;
  
  // もしもうあるならアニメを再始動
  if( sel_select_wk->cellSelect ){
    _gearObjSetAnmSeq( sel_select_wk->cellSelect, ACTANM_E_IR+pWork->select_panel );
    _gearObjSetPos( sel_select_wk->cellSelect, x, y );
    return;
  }
  //新規にエフェクトを追加
  sel_select_wk->cellSelect = 
    _gearObjAdd( pWork, x, y, ACTANM_E_IR+pWork->select_panel, CGEAR_CLACT_BG_PRI, ACT_SPRI_EFFECT, _CLACT_RES_PANEL );

  GFL_CLACT_WK_SetDrawEnable( sel_select_wk->cellSelect, TRUE );
  GFL_CLACT_WK_SetSRTAnimeFlag( sel_select_wk->cellSelect, TRUE ); //SRTアニメ反映
  
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  空いているCLWKを取得  もし空いていなかったらもっともフレームが進んだものを返す。
 *
 *	@param	cpWork  ワーク
 *
 *	@return CLWK
 */
//-----------------------------------------------------------------------------
static SEL_SELECT_EFFECT* _selectAnimeGetCleanCLWK( C_GEAR_WORK* pWork )
{
  int i;
  int max_index = -1;
  fx32 max_frame = -1;
  int max_index_idx = 0;
  int anm_index;
  fx32 anm_frame;
  
  for( i=0; i<_SELECT_ANIME_OBJ_MAX; i++ ){

    if( pWork->cellSelect[i].cellSelect == NULL ){
      return &pWork->cellSelect[i];
    }

    anm_index = GFL_CLACT_WK_GetAnmIndex( pWork->cellSelect[i].cellSelect );
    anm_frame = GFL_CLACT_WK_GetAnmFrame( pWork->cellSelect[i].cellSelect );
    if( (anm_index > max_index) || 
        ((anm_index == max_index) && (anm_frame > max_frame)) ){
      max_index = anm_index;
      max_frame = anm_frame;
      max_index_idx = i;
    }
  }

  return &pWork->cellSelect[max_index_idx];
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ完了待ち
 *
 *	@param	C_GEAR_WORK* pWork
 *	@param	x
 *	@param	y 
 *
 *	@retval TRUE  完了
 *	@retval FALSE 途中
 */
//-----------------------------------------------------------------------------
static BOOL _selectAnimeWait( const C_GEAR_WORK* cpWork,int x,int y )
{
  int i;

  for( i=0; i<_SELECT_ANIME_OBJ_MAX; i++ ){
    if( cpWork->cellSelect[i].cellSelect ){
      if( (cpWork->cellSelect[i].x == x) && (cpWork->cellSelect[i].y == y) ){

        if( GFL_CLACT_WK_CheckAnmActive( cpWork->cellSelect[i].cellSelect ) ){
          return FALSE;
        }
        
      }
    }
  }

  return TRUE;
}

static BOOL _modeSelectAnimWait(C_GEAR_WORK* pWork)
{
  switch( pWork->eff_seq ){
  case 0:
    // アニメーション表示開始
    _gearObjSetPos( pWork->cellCursor[ ACT_PANEL_EFF ], pWork->touchx, pWork->touchy );
    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_PANEL_EFF ], ACTANM_E_IR+pWork->select_panel );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ ACT_PANEL_EFF ], TRUE );
    pWork->eff_seq ++;
    break;

  case 1:
    if( GFL_CLACT_WK_CheckAnmActive( pWork->cellCursor[ ACT_PANEL_EFF ] ) == FALSE ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ ACT_PANEL_EFF ], FALSE );
      pWork->eff_seq = 0;
      return TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    return TRUE;
  }
  return FALSE;

#if 0
  int x;
  int y,i;
  int xbuff[6];
  int ybuff[6];

  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      // 終了チェック
      if( pWork->typeAnim[x][y] == _SELECTANIM_RUN ){
        if( _selectAnimeWait( pWork, x, y ) ){
          pWork->typeAnim[x][y] = _SELECTANIM_END;
        }
      }
      
      // 開始チェック
      if(pWork->typeAnim[x][y] == _SELECTANIM_STANDBY)
      {
        _selectAnimInit(pWork, x,y);
        pWork->typeAnim[x][y] = _SELECTANIM_ANIMING;
      }

    }
  }


  // イベントへジャンプ
  pWork->select_count ++;
  if( pWork->select_count >= _SELECT_ANIME_WAIT ){
    return TRUE;
  }


  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      // 
      // 広がっていく
      if(pWork->typeAnim[x][y] == _SELECTANIM_ANIMING)
      {
        pWork->typeAnim[x][y] = _SELECTANIM_RUN;

        
        // 自分の周り６つを探す。
        _getHexAround( x,y, xbuff, ybuff );
      
        for( i=0; i<6; i++ ){
          // 範囲内？チェック
          if( (xbuff[i] >= 0) && (xbuff[i] < C_GEAR_PANEL_WIDTH) &&
              (ybuff[i] >= 0) && (ybuff[i] < C_GEAR_PANEL_HEIGHT) ){
            // 設定
            if( pWork->typeAnim[ xbuff[i] ][ ybuff[i] ] == _SELECTANIM_WAIT ){
              pWork->typeAnim[ xbuff[i] ][ ybuff[i] ] = _SELECTANIM_STANDBY;
            }else if( pWork->typeAnim[ xbuff[i] ][ ybuff[i] ] == _SELECTANIM_NONE ){
              pWork->typeAnim[ xbuff[i] ][ ybuff[i] ] = _SELECTANIM_ANIMING;
            }
          }
        }
      }
    }
  }
#endif
  return FALSE;
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

//  type = _cgearSave_GetPanelType(pWork,pWork->touchx,pWork->touchy); // 2
//  pWork->select_panel = type;
  pWork->select_count = 0;

#if 0
  for(y = 0; y < C_GEAR_PANEL_HEIGHT; y++)
  {
    for(x = 0; x < C_GEAR_PANEL_WIDTH ; x++)
    {
      if(_cgearSave_GetPanelType(pWork,x,y) == type){
        pWork->typeAnim[x][y] = _SELECTANIM_WAIT;
      }
    }
  }
  pWork->typeAnim[pWork->touchx][pWork->touchy] = _SELECTANIM_STANDBY;
  _modeSelectAnimWait(pWork);
#endif
}

#if 0
//------------------------------------------------------------------------------
/**
 * @brief   タイプに変換
 * @retval  none
 */
//------------------------------------------------------------------------------
static int getTypeToTouchPos(C_GEAR_WORK* pWork,int touchx,int touchy,int *pxp, int* pyp)
{
  int xp,yp;
  int type = CGEAR_PANELTYPE_MAX;

  {  // ギアのタイプ分析
    int ypos[2] = {PANEL_Y1,PANEL_Y2};
    int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};
    touchx = touchx / 8;
    touchy = touchy / 8;
    xp = (touchx - PANEL_X1) / PANEL_SIZEXY;
    if(xp < 0){
      return CGEAR_PANELTYPE_MAX;
    }
    if(touchy - ypos[xp % 2] < 0){
      return CGEAR_PANELTYPE_MAX;
    }
    yp = (touchy - ypos[xp % 2]) / PANEL_SIZEXY;

    if((xp < C_GEAR_PANEL_WIDTH) && (yp < yloop[ xp % 2 ]))
    {
      type = _cgearSave_GetPanelType(pWork,xp,yp);
    }
  }
  *pxp=xp;
  *pyp=yp;
  return type;
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  パネルのパレットアニメ処理　初期化
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteAnimeInit( C_GEAR_WORK* pWork )
{
  int i, j;

  // カウント初期化
  for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
    _PLTCOUNTER_Init( &pWork->plt_anime.counter[ i ], _CGEAR_NET_CHANGEPAL_ANM_COUNT_MAX[i], 
        _CGEAR_NET_CHANGEPAL_ANM_COUNT_DIF[i], _CGEAR_NET_CHANGEPAL_ANM_COUNT_RESET[i] );

    // FIRST以外を再生
    if( i == _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST ){
      _PLTCOUNTER_SetPlay( &pWork->plt_anime.counter[ i ], FALSE );
    }
  }
  

  for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
    for( j=0; j<CGEAR_PANEL_MAX; j++ ){
      _PanelPaletteColorSetUp( pWork, i, j, FALSE, FALSE );
    }
  }

}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルのパレットアニメ処理　初期化  スリープ復帰用
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteAnimeInitSleep( C_GEAR_WORK* pWork )
{
  int i, j;

  // カウント初期化
  for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
    _PLTCOUNTER_Init( &pWork->plt_anime.counter[ i ], _CGEAR_NET_CHANGEPAL_ANM_COUNT_MAX[i], 
        _CGEAR_NET_CHANGEPAL_ANM_COUNT_DIF[i], _CGEAR_NET_CHANGEPAL_ANM_COUNT_RESET[i] );
    
    // FIRST以外は再生停止
    if( i != _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST ){
      _PLTCOUNTER_SetPlay( &pWork->plt_anime.counter[ i ], FALSE );
    }
  }

  for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
    for( j=0; j<CGEAR_PANEL_MAX; j++ ){
      _PanelPaletteColorSetUp( pWork, i, j, TRUE, pWork->ex_col_flag );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルパレットアニメ　カラー初期化
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteAnimeColClear( C_GEAR_WORK* pWork )
{
  int  i;
  for(i = 0 ; i < CGEAR_PANEL_MAX; i++)
  {
    _PanelPaletteChangeCore( pWork, i, 0, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードカラーのセットアップ
 *	 
 *	 anime_type状態で、アニメーションするところを拾っていく。
 *
 *	@param	pWork   ワーク
 *
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteColorSetUp( C_GEAR_WORK* pWork, int anime_type, int panel_type, BOOL set_first, BOOL ex_col_flag )
{
  GAME_COMM_SYS_PTR pGC = GAMESYSTEM_GetGameCommSysPtr(pWork->pGameSys);
  u32 bit = 0;
  u32 beacon_bit = 0;
  BOOL ret = TRUE;
  INTRUDE_CONNECT intrude_status;

  u32 anime_type_normal = _CGEAR_NET_CHANGEPAL_ANM_TYPE_NORMAL;
  u32 anime_type_off = _CGEAR_NET_CHANGEPAL_ANM_TYPE_OFF;
  u32 anime_type_high = _CGEAR_NET_CHANGEPAL_ANM_TYPE_HIGH;
  u32 anime_type_beacon_catch = _CGEAR_NET_CHANGEPAL_ANM_TYPE_BEACON_CATCH;

  // set_firstのときは、演出用のアニメタイプを設定
  if( set_first ){
    anime_type_high = _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST;
    anime_type_beacon_catch = _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST;
    anime_type_normal = _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST;
    anime_type_off = _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST;
  }

  bit = WIH_DWC_GetAllBeaconTypeBit( GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(pWork->pGameSys)) );

#ifdef DEBUG_KEY_CONTROL
  if( (GFL_UI_KEY_GetCont() & (PAD_BUTTON_L | PAD_BUTTON_A)) == (PAD_BUTTON_L | PAD_BUTTON_A) ){
    // すべてOFF
    _PaletteSetColType( pWork, CGEAR_PANEL_IR, _CGEAR_NET_PALTYPE_DARK_RED, 
        anime_type_normal, anime_type, panel_type );
    _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_DARK_BLUE, 
        anime_type_normal, anime_type, panel_type );
    _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_DARK_YELLOW, 
        anime_type_normal, anime_type, panel_type );
    return ;
  }
#endif

  intrude_status = Intrude_GetIntrudeStatus( pGC );

  if( ex_col_flag ){
    // 次のアニメーション確定処理
    if(intrude_status == INTRUDE_CONNECT_MISSION_PARTNER){
      //beacon_bit |= _CGEAR_NET_BIT_WIRELESS;ほかもアニメするようにコメントあうと・
      _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_GREEN, 
          anime_type_high, anime_type, panel_type );
    }else if(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR){ // ライブキャスター

      beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
      _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_YELLOW,
          anime_type_high, anime_type, panel_type );
    }
  }

#ifdef DEBUG_KEY_CONTROL
  if( (GFL_UI_KEY_GetCont() & (PAD_BUTTON_L | PAD_BUTTON_B)) == (PAD_BUTTON_L | PAD_BUTTON_B) ){
    beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
    _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_YELLOW,
        anime_type_high, anime_type, panel_type );
  }
  else if( (GFL_UI_KEY_GetCont() & (PAD_BUTTON_L | PAD_BUTTON_Y)) == (PAD_BUTTON_L | PAD_BUTTON_Y) ){
    beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
    _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_GREEN, 
        anime_type_high, anime_type, panel_type );
  }
#endif
  

  if(!beacon_bit){

    ret = FALSE;

    if(bit & GAME_COMM_SBIT_IRC_ALL){
      beacon_bit |= _CGEAR_NET_BIT_IR;
    }
    if(bit & GAME_COMM_SBIT_WIRELESS_ALL){
      beacon_bit |= _CGEAR_NET_BIT_WIRELESS;
    }
    if(bit & GAME_COMM_SBIT_WIFI_ALL){
      beacon_bit |= _CGEAR_NET_BIT_WIFI;
    }


    // WIFIカラー
    if(bit & GAME_COMM_STATUS_BIT_WIFI){      // 登録済み
      _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_GREEN, 
          anime_type_beacon_catch, anime_type, panel_type );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_FREE){  // かぎなし
      // Freeは赤表示に変更 2010.06.22 tomoya 
      /*_PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_YELLOW, 
          anime_type_beacon_catch, anime_type, panel_type );*/
      _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_RED, 
          anime_type_beacon_catch, anime_type, panel_type );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_ZONE){  // 任天堂ゾーン　など　公式
      _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_BLUE, 
          anime_type_beacon_catch, anime_type, panel_type );
    }
    else if(bit & GAME_COMM_STATUS_BIT_WIFI_LOCK){  // かぎあり
      _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_RED, 
          anime_type_beacon_catch, anime_type, panel_type );
    }

    // Wirelesカラー
    if( intrude_status != INTRUDE_CONNECT_NULL ){
      if( intrude_status != INTRUDE_CONNECT_MISSION_PARTNER ){
        _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_GREEN, 
            anime_type_beacon_catch, anime_type, panel_type );
      }
      
    }else{
      
      // トランシーバー以外は順番に
      static const u32 WIRELES_COUNT_MASK[ _CGEAR_NET_WIRELES_TYPE_MAX ] = 
      {
        GAME_COMM_STATUS_BIT_WIRELESS_FU, 
        GAME_COMM_STATUS_BIT_WIRELESS_UN,
        0,
      };
      static const u32 WIRELES_COUNT_COLOR[ _CGEAR_NET_WIRELES_TYPE_MAX ] = 
      {
        _CGEAR_NET_PALTYPE_RED, 
        _CGEAR_NET_PALTYPE_BLUE, 
        _CGEAR_NET_PALTYPE_PINK, 
      };
      int i;
      BOOL setup;

      for( i=0; i<_CGEAR_NET_WIRELES_TYPE_MAX; i++ ){
        pWork->plt_anime.wireles_count = (pWork->plt_anime.wireles_count + 1) % _CGEAR_NET_WIRELES_TYPE_MAX;
        if( (bit & WIRELES_COUNT_MASK[ pWork->plt_anime.wireles_count ]) || 
            ((pWork->plt_anime.wireles_count==_CGEAR_NET_WIRELES_TYPE_OTHER) && pWork->cross_change) ){  // すれ違いbeaconが変わった
          setup = _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, WIRELES_COUNT_COLOR[ pWork->plt_anime.wireles_count ], 
              anime_type_beacon_catch, anime_type, panel_type );
          if( setup ){
            
            if( (pWork->plt_anime.wireles_count==_CGEAR_NET_WIRELES_TYPE_OTHER) ){
              pWork->cross_change = FALSE; //すれ違いbeacon変更チェックリセット
            }
            break;
          }
        }
      }

      // ぜんぜん引っかからないなら、DARKイエロー
      if( i==_CGEAR_NET_WIRELES_TYPE_MAX ){
        _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_DARK_YELLOW, 
            anime_type_normal, anime_type, panel_type );
      }
    }
  }

  if( ret == FALSE ){
    // IRCカラー　つねに
    _PaletteSetColType( pWork, CGEAR_PANEL_IR, _CGEAR_NET_PALTYPE_DARK_RED, 
        anime_type_normal, anime_type, panel_type );
    
    // WirelessWifiのビットがたってないなら、通常演出をON
    if((beacon_bit & _CGEAR_NET_BIT_WIFI) == 0){
      _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_DARK_BLUE, 
          anime_type_normal, anime_type, panel_type );
    }
    if((beacon_bit & _CGEAR_NET_BIT_WIRELESS) == 0){
      _PaletteSetColType( pWork, CGEAR_PANEL_WIRELESS, _CGEAR_NET_PALTYPE_DARK_YELLOW, 
          anime_type_normal, anime_type, panel_type );
    }
  }else{

    // IR
    // WIFIを強制的にOFF状態に
    _PaletteSetColType( pWork, CGEAR_PANEL_IR, _CGEAR_NET_PALTYPE_GRAY, 
        anime_type_off, anime_type, panel_type );

    _PaletteSetColType( pWork, CGEAR_PANEL_WIFI, _CGEAR_NET_PALTYPE_GRAY, 
        anime_type_off, anime_type, panel_type );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  Wireless特殊状態　チェック
 *
 *	@param	cpWork 
 *
 *	@retval TRUE  Wireless特殊状態
 *	@retval FALSE 通常時
 */
//-----------------------------------------------------------------------------
static BOOL _PanelPalette_IsExMode( const C_GEAR_WORK* cpWork )
{
  if( (cpWork->ir_wifi_off == TRUE) && (cpWork->ex_col_flag == TRUE) ){
    return TRUE;
  }
  return FALSE;
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
  int i, j;
  BOOL sleep_end = FALSE;

  for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
    _PLTCOUNTER_CountUp( &pWork->plt_anime.counter[i] );
    
    for( j=0; j<CGEAR_PANEL_MAX; j++ ){
      if( _PLTCOUNTER_IsEndAnime( &pWork->plt_anime.counter[i], j ) || pWork->ex_col_change ){ // アニメ終了時かカラーチェンジ時
        _PanelPaletteColorSetUp( pWork, i, j, FALSE, pWork->ex_col_flag );
        if( i == _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST ){
          sleep_end = TRUE;
        }

        if( _PLTCOUNTER_IsEndAnime( &pWork->plt_anime.counter[i], j ) ){
          // リセット
          _PLTCOUNTER_Reset( &pWork->plt_anime.counter[i], j );
        }
      }
    }
  }

  if( sleep_end ){
    // 全解除
    for( i=0; i<_CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++ ){
      if( i != _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST ){
        _PLTCOUNTER_SetPlay( &pWork->plt_anime.counter[i], TRUE );
        //エフェクト開始
        for( j=0; j<CGEAR_PANEL_MAX; j++ ){
          _PanelPaletteColorSetUp( pWork, i, j, FALSE, pWork->ex_col_flag );
        }
      }else{
        _PLTCOUNTER_SetPlay( &pWork->plt_anime.counter[i], FALSE );
      }
    }
  }

  pWork->ex_col_change = FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  wireless通知
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PanelPalette_EXSet( C_GEAR_WORK* pWork, BOOL flag, BOOL ir_wifi_off )
{
  int  i, j;

  //カラーを一度初期化
  _PanelPaletteAnimeColClear( pWork );

  //全アニメーションタイプのカウンタ強制リセット
  for(i = 0 ; i < _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX; i++){
    for( j=0; j<CGEAR_PANEL_MAX; j++ ){
      _PLTCOUNTER_ResetForce( &pWork->plt_anime.counter[i], j );
    }
  }

  pWork->ex_col_flag = flag;
  pWork->ex_col_change = TRUE;
  pWork->ex_radar_cross_bttn_col_change = TRUE;
  pWork->ex_radar_cross_bttn_col_change_wait = 0;
  pWork->ir_wifi_off  = ir_wifi_off;
}


//------------------------------------------------------------------------------
/**
 * @brief   通信を拾ったら、パレットで点滅させる
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _PanelPaletteChange(C_GEAR_WORK* pWork)
{
  int i;

  for(i = 0 ; i < CGEAR_PANEL_MAX; i++)
  {
    switch( pWork->plt_anime.plt_anmtype[i] ){
    case _CGEAR_NET_CHANGEPAL_ANM_TYPE_HIGH:
      _PanelPaletteChangeHigh( pWork, i, &pWork->plt_anime.counter[ pWork->plt_anime.plt_anmtype[i] ] );
      break;
    case _CGEAR_NET_CHANGEPAL_ANM_TYPE_NORMAL:
      _PanelPaletteChangeNormal( pWork, i, &pWork->plt_anime.counter[ pWork->plt_anime.plt_anmtype[i] ] );
      break;
    case _CGEAR_NET_CHANGEPAL_ANM_TYPE_BEACON_CATCH:
      _PanelPaletteChangeBeaconCatch( pWork, i, &pWork->plt_anime.counter[ pWork->plt_anime.plt_anmtype[i] ] );
      break;
    case _CGEAR_NET_CHANGEPAL_ANM_TYPE_FIRST:
      _PanelPaletteChangeChange( pWork, i, &pWork->plt_anime.counter[ pWork->plt_anime.plt_anmtype[i] ] );
      break;
    case _CGEAR_NET_CHANGEPAL_ANM_TYPE_OFF:
      _PanelPaletteChangeOff( pWork, i, &pWork->plt_anime.counter[ pWork->plt_anime.plt_anmtype[i] ] );
      break;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルの色変え処理　HIGH
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteChangeHigh(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count)
{
  int mod;
  int index;
  u16 count = _PLTCOUNTER_GetCount( cp_count, change_panel );
  
  if( count > (_CGEAR_NET_CHANGEPAL_ANM_HIGH_COUNT+_CGEAR_NET_CHANGEPAL_ANM_HIGH_TOPWAIT) ){
    return ;
  }
  if( count < _CGEAR_NET_CHANGEPAL_ANM_HIGH_TOPWAIT  ){
    return ;
  }

  count -= _CGEAR_NET_CHANGEPAL_ANM_HIGH_TOPWAIT;

  mod = (count % (_CGEAR_NET_CHANGEPAL_ANM_HIGH_COUNT/4));

  index = (mod * _CGEAR_NET_CHANGEPAL_ANM_MOD_NUM_MAX) / (_CGEAR_NET_CHANGEPAL_ANM_HIGH_COUNT/4);
  
  mod = _CGEAR_NET_CHANGEPAL_ANM_MOD_TBL[ index ];
  _PanelPaletteChangeCore( pWork, change_panel, mod, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルの色変え処理　NORMAL
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteChangeNormal(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count)
{
  int mod;
  int index;
  u16 count = _PLTCOUNTER_GetCount( cp_count, change_panel );

  if( count > (_CGEAR_NET_CHANGEPAL_ANM_NORMAL_TOPWAIT+_CGEAR_NET_CHANGEPAL_ANM_NORMAL_COUNT) ){
    return ;
  }

  if( count < _CGEAR_NET_CHANGEPAL_ANM_NORMAL_TOPWAIT  ){
    return ;
  }

  count -= _CGEAR_NET_CHANGEPAL_ANM_NORMAL_TOPWAIT;

  mod = count;

  index = (mod * _CGEAR_NET_CHANGEPAL_ANM_MOD_NUM_MAX) / _CGEAR_NET_CHANGEPAL_ANM_NORMAL_COUNT;
  
  mod = _CGEAR_NET_CHANGEPAL_ANM_MOD_TBL[ index ];
  _PanelPaletteChangeCore( pWork, change_panel, mod, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  beaconキャッチアニメーション
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteChangeBeaconCatch(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count)
{
  int mod;
  int index;
  u16 count = _PLTCOUNTER_GetCount( cp_count, change_panel );

  if( count > (_CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_TOPWAIT+_CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_COUNT) ){
    return ;
  }

  if( count < _CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_TOPWAIT  ){
    return ;
  }

  count -= _CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_TOPWAIT;

  mod = count;

  index = (mod * _CGEAR_NET_CHANGEPAL_ANM_MOD_CATCH_NUM_MAX) / _CGEAR_NET_CHANGEPAL_ANM_BEACON_CATCH_COUNT;
  
  mod = _CGEAR_NET_CHANGEPAL_ANM_MOD_CATCH_TBL[ index ];
  _PanelPaletteChangeCore( pWork, change_panel, mod, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  変更通知今までどおり
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteChangeChange(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count)
{
  int mod;
  int index;
  u16 count = _PLTCOUNTER_GetCount( cp_count, change_panel );
  
  if( count > _CGEAR_NET_CHANGEPAL_ANM_CHANGE_COUNT ){
    return ;
  }

  mod = (count % (_CGEAR_NET_CHANGEPAL_ANM_CHANGE_COUNT/2));

  index = (mod * _CGEAR_NET_CHANGEPAL_ANM_MOD_NUM_MAX) / (_CGEAR_NET_CHANGEPAL_ANM_CHANGE_COUNT/2);
  
  mod = _CGEAR_NET_CHANGEPAL_ANM_MOD_TBL[ index ];
  _PanelPaletteChangeCore( pWork, change_panel, mod, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OFF
 */
//-----------------------------------------------------------------------------
static void _PanelPaletteChangeOff(C_GEAR_WORK* pWork, int change_panel, const PLT_COUNTER* cp_count)
{
  int mod;
  int index;
  u16 count = _PLTCOUNTER_GetCount( cp_count, change_panel );
  
  if( count > _CGEAR_NET_CHANGEPAL_ANM_OFF_COUNT  ){
    return ;
  }

  mod = count;

  index = (mod * _CGEAR_NET_CHANGEPAL_ANM_MOD_OFF_NUM_MAX) / _CGEAR_NET_CHANGEPAL_ANM_OFF_COUNT;
  
  mod = _CGEAR_NET_CHANGEPAL_ANM_MOD_OFF_TBL[ index ];
  _PanelPaletteChangeCore( pWork, change_panel, mod, _CGEAR_NET_CHANGEPAL_ANM_MOD_MAX );
}


static void _PanelPaletteChangeCore( C_GEAR_WORK* pWork, int change_panel, int mod, int mod_max )
{
  int add,rgb,base;
  int shift;

  pWork->palTrans[change_panel] = 0;
  for(rgb = 0; rgb < 3; rgb++){
    shift = rgb * 5;
    base = (pWork->palBase[change_panel] >> shift) & 0x1f;
    add = ((pWork->palChangeCol[ pWork->palChangeType[change_panel] ][0] >> shift) & 0x1f) - base;
    pWork->palTrans[change_panel] |= ((((add * mod) / mod_max) + base) & 0x1f)<<(shift);
  }

  // BGPalette
  PaletteWorkSet( pWork->pfade_ptr, &pWork->palTrans[change_panel],
      FADE_SUB_OBJ, 16*(change_panel+PANEL_COLOR_TRANS_POS_PALNO) + PANEL_COLOR_TRANS_POS_OFS, 2 );
}





static void _PaletteMake(C_GEAR_WORK* pWork,const u16* pltt,int type)
{
  int col;

  for( col = 0 ; col < _CGEAR_NET_CHANGEPAL_NUM; col++){
    pWork->palChangeCol[type][col]= pltt[ (type*16) + col ];
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  パネルのカラータイプを設定
 *
 */
//-----------------------------------------------------------------------------
static BOOL _PaletteSetColType( C_GEAR_WORK* pWork, int panel,int type, int anime_type, int setok_anime_type, int setok_panel_type )
{
  GF_ASSERT( type < _CGEAR_NET_PALTYPE_MAX );
  GF_ASSERT( panel < _CGEAR_NET_CHANGEPAL_NUM );
  GF_ASSERT( anime_type < _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX );
  GF_ASSERT( setok_anime_type < _CGEAR_NET_CHANGEPAL_ANM_TYPE_MAX );
  GF_ASSERT( setok_panel_type < _CGEAR_NET_CHANGEPAL_NUM );

  if( (anime_type == setok_anime_type) && (panel == setok_panel_type) ){
    pWork->palChangeType[ panel ] = type;
    pWork->plt_anime.plt_anmtype[ panel ] = anime_type;
    return TRUE;
  }

  return FALSE;
}

#if 0

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
 *	@brief  停止処理
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeSysAllStop( C_GEAR_WORK* pWork )
{
  int i, j;
  for( i=0; i<C_GEAR_PANEL_WIDTH; i++ ){
    for( j=0; j<C_GEAR_PANEL_HEIGHT; j++ ){
      _PanelMarkAnimeStop( &pWork->panel_mark[i][j] );
    }
  }
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
 *	@brief  OFF状態の設定
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeSetOff( PANEL_MARK_ANIME* p_mark, C_GEAR_WORK* pWork, CGEAR_PANELTYPE_ENUM panel_type )
{
  GF_ASSERT( p_mark->anime_on == FALSE );
  
  // color用のパネルを書き込み。
  p_mark->color = PANEL_COLOR_TYPE_NONE;
//  _gearPanelBgScreenMake(pWork, p_mark->x, p_mark->y, panel_type);
  _PanelMarkAnimeWriteScreen( p_mark, 0 );
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
  u32 set_index = 0;
  
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
    p_mark->end_anime_index  = _PanelMark_GetPatternIndex( pWork, color_type, p_mark->x, p_mark->y );  // アニメーションインデックスの終了値
    set_index                = PANEL_COLOR_PATTERN_NUM-1;
    p_mark->count       = (frame * PANEL_COLOR_PATTERN_NUM);
  }

  // color用のパネルを書き込み。
//  _gearPanelBgScreenMake(pWork, p_mark->x, p_mark->y, panel_type);
  _PanelMarkAnimeWriteScreen( p_mark, set_index );
}

//----------------------------------------------------------------------------
/**
 *	@brief  途中でも停止
 *
 *	@param	p_mark
 */
//-----------------------------------------------------------------------------
static void _PanelMarkAnimeStop( PANEL_MARK_ANIME* p_mark)
{
  p_mark->anime_on    = FALSE;
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
  int scrn_size_x;
  u32 pal_index;

  pal_index    = sc_PANEL_COLOR_ANIME_TBL[ cp_mark->color ][ anime_index ];

  _gearXY2PanelScreen(cp_mark->x, cp_mark->y, &scrn_x, &scrn_y );

  scrn_size_x = PANEL_SIZEXY;
  if( scrn_x < 0 ){
    scrn_size_x = PANEL_SIZEXY + scrn_x;
    scrn_x = 0;
  }else if( (scrn_x + PANEL_SIZEXY) > 32 ){
    scrn_size_x = 32 - scrn_x;
  }
  
  GFL_BG_ChangeScreenPalette( GEAR_BUTTON_FRAME, 
      scrn_x, scrn_y, scrn_size_x, PANEL_SIZEXY, pal_index );

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


//----------------------------------------------------------------------------
/**
 *	@brief  終点カラーを取得
 *
 *	@param	cpWork      ワーク
 *	@param	col_type    カラータイプ
 *	@param  x           xインデックス
 *	@param  y           yインデックス
 */
//-----------------------------------------------------------------------------
static int _PanelMark_GetPatternIndex( const C_GEAR_WORK* cpWork, u32 col_type, int x, int y )
{
  if( _cgearSave_IsPanelTypeIcon( cpWork, x, y ) ){
    return sc_PANEL_COLOR_ANIME_END_INDEX[ col_type ][ 4 ];
  }

  return sc_PANEL_COLOR_ANIME_END_INDEX[ col_type ][ x ];
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  カウントアップ　ワーク初期化
 *
 *	@param	p_wk
 *	@param	count_max 
 */
//-----------------------------------------------------------------------------
static void _PLTCOUNTER_Init( PLT_COUNTER* p_wk, u16 count_max, u16 dif, BOOL count_reset )
{
  p_wk->plt_count[ CGEAR_PANEL_WIRELESS ] = 0;
  p_wk->plt_count[ CGEAR_PANEL_WIFI ] = dif*1;
  p_wk->plt_count[ CGEAR_PANEL_IR ] = dif*2;
  p_wk->count_max = count_max;
  p_wk->count_reset = count_reset;
  p_wk->play = TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カウントアップ
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void _PLTCOUNTER_CountUp( PLT_COUNTER* p_wk )
{
  int i;
  if( p_wk->play == FALSE ){
    return ;
  }
  
  for( i=0; i<CGEAR_PANEL_MAX; i++ ){
    if( p_wk->plt_count[i] < p_wk->count_max ){
      p_wk->plt_count[i] ++;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメ終了チェック
 *
 *	@param	cp_wk       ワーク
 *	@param	panel_type  パネルタイプ
 *
 *	@retval TRUE    アニメ終了
 *	@retval FALSE   アニメ途中
 */
//-----------------------------------------------------------------------------
static BOOL _PLTCOUNTER_IsEndAnime( const PLT_COUNTER* cp_wk, u32 panel_type )
{
  GF_ASSERT( panel_type < CGEAR_PANEL_MAX );

  if( cp_wk->plt_count[ panel_type ] >= cp_wk->count_max ){
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメカウントリセット
 *
 *	@param	p_wk
 *	@param	panel_type 
 */
//-----------------------------------------------------------------------------
static void _PLTCOUNTER_Reset( PLT_COUNTER* p_wk, u32 panel_type )
{
  GF_ASSERT( panel_type < CGEAR_PANEL_MAX );
  if( p_wk->count_reset ){
    p_wk->plt_count[ panel_type ] = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  アニメカウント　強制リセット　リセットなしでもリセット
 *
 *	@param	p_wk
 *	@param	panel_type 
 */
//-----------------------------------------------------------------------------
static void _PLTCOUNTER_ResetForce( PLT_COUNTER* p_wk, u32 panel_type )
{
  GF_ASSERT( panel_type < CGEAR_PANEL_MAX );
  p_wk->plt_count[ panel_type ] = 0;
}

//----------------------------------------------------------------------------
/**
 *	@brief  カウント値を取得
 *
 *	@param	cp_wk 
 *	@param  panel_type    パネルタイプ
 *
 *	@return カウント値
 */
//-----------------------------------------------------------------------------
static u16 _PLTCOUNTER_GetCount( const PLT_COUNTER* cp_wk, u32 panel_type )
{
  GF_ASSERT( panel_type < CGEAR_PANEL_MAX );
  return cp_wk->plt_count[ panel_type ];
}

//----------------------------------------------------------------------------
/**
 *	@brief  カウントアップフラグを設定
 *
 *	@param	p_wk
 *	@param	flag 
 */
//-----------------------------------------------------------------------------
static void _PLTCOUNTER_SetPlay( PLT_COUNTER* p_wk, BOOL flag )
{
  p_wk->play = flag;
}

#if 0



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
#endif


//----------------------------------------------------------------------------
/**
 *	@brief  サブ画面フェード完了まち
 *
 *	@param	cpWork 
 *
 *	@retval TRUE  フェード完了
 *	@retval FALSE  フェード途中
 */
//-----------------------------------------------------------------------------
static BOOL _modeSelectWaitSubFadeEnd( const C_GEAR_WORK* cpWork )
{
  if( WIPE_SYS_EndCheck() == FALSE ){
    return FALSE;
  }

  // 輝度チェック
  if( GXS_GetMasterBrightness() != 0 ){
    return FALSE;
  }

  {
    FIELDMAP_WORK* p_fieldmap = GAMESYSTEM_GetFieldMapWork( cpWork->pGameSys );
    if( FIELDMAP_CheckSeasonDispFlag(p_fieldmap) ){
      return FALSE;
    }
  }

  return TRUE;
}

#if 0
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
      if(_cgearSave_GetPanelType(pWork,x,y)==type)
      {
        i++;
      }
    }
  }
  return i;
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェード基準となる　フェード終了値を設定
 *
 *	@param	pWork
 *	@param	comm 
 */
//-----------------------------------------------------------------------------
static void _PFadeSetDefaultPal( C_GEAR_WORK* pWork, BOOL comm )
{
  int i, j;
  u16* p_buff;
  u16* p_trans;
  if( !comm ){
    // 基準値を暗めにする。
    p_buff = PaletteWorkDefaultWorkGet( pWork->pfade_ptr, FADE_SUB_OBJ );
    p_trans = PaletteWorkTransWorkGet( pWork->pfade_ptr, FADE_SUB_OBJ );
    for( i=0; i<OAM_USE_PLTT_NUM; i++ ){
      if( (1<<i) & OAM_PFADE_NORMAL_MSK ){
        // 16こかえる
        SoftFade( &p_buff[ 16*i ], &p_buff[ 16*i ], 16, 8, _BLACK_COLOR[pWork->sex] );
        GFL_STD_MemCopy( &p_buff[ 16*i ], &p_trans[ 16*i ], 2*16 );
      }
    }

    p_buff = PaletteWorkDefaultWorkGet( pWork->pfade_ptr, FADE_SUB_BG );
    p_trans = PaletteWorkTransWorkGet( pWork->pfade_ptr, FADE_SUB_BG );
    for( i=0; i<OAM_USE_PLTT_NUM; i++ ){
      if( (1<<i) & BG_PFADE_NORMAL_MSK ){
        // 16こかえる
        SoftFade( &p_buff[ 16*i ], &p_buff[ 16*i ], 16, 8, _BLACK_COLOR[pWork->sex] );
        GFL_STD_MemCopy( &p_buff[ 16*i ], &p_trans[ 16*i ], 2*16 );
      }
    }
  }
}

static void _PFadeSetBlack( C_GEAR_WORK* pWork )
{
  // 黒く
/*
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,   -120, 0, 16, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );
*/
  ColorConceChangePfd( pWork->pfade_ptr, FADE_SUB_OBJ, OAM_PFADE_MSK_ALL, 16, _BLACK_COLOR[pWork->sex] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープブラック
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _PFadeSetSleepBlack( C_GEAR_WORK* pWork, BOOL on_flag )
{
  int evy, i;

  static const u8 anm_stop_tbl[] = {
    ACT_CUSTOM,
    ACT_IR_RING,
    ACT_WIRELESS_RING,
    ACT_WIFI_RING,
  };

  if(on_flag){
    evy = 8;
  }else{
    evy = 0;
  }
  //アニメーションの一時ストップ
  for( i = 0;i < NELEMS(anm_stop_tbl);i++){
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[anm_stop_tbl[i]], !on_flag );
  }

  /*
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,   -120, 0, evy, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );

  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_BG, BG_PFADE_NORMAL_MSK,   -120, 0, evy, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );
  */
  ColorConceChangePfd( pWork->pfade_ptr, FADE_SUB_OBJ, OAM_PFADE_MSK_ALL, evy, _BLACK_COLOR[pWork->sex] );
  ColorConceChangePfd( pWork->pfade_ptr, FADE_SUB_BG, BG_PFADE_NORMAL_MSK, evy, _BLACK_COLOR[pWork->sex] );
}

//----------------------------------------------------------------------------
/**
 *	@brief  特殊状態、　レーダー　すれ違いログボタン　OFF
 *
 *	@param	pWork
 *	@param	on_flag 
 */
//-----------------------------------------------------------------------------
static void _PFadeSetExBttnBlack( C_GEAR_WORK* pWork, BOOL off_flag )
{
  int evy;
  if(off_flag){
    evy = 8;
    GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR_OFF );
  }else{
    evy = 0;
    if( pWork->radar_flag ){
      GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR_INFO );
    }else{
      GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR );
    }
  }
  
  if( pWork->fmission_flag ){
    GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_CONNECT], ACTANM_FMISSION+off_flag );
  }else{
    GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_CONNECT], ACTANM_CONNECT+off_flag );
  }
  
  // ビーコンアイコンをパレットフェードで黒く
  /*
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, OAM_PFADE_CROSS_MSK,   -120, 0, evy, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );
  */
  ColorConceChangePfd( pWork->pfade_ptr, FADE_SUB_OBJ, OAM_PFADE_CROSS_MSK, evy, _BLACK_COLOR[pWork->sex] );
}


static void _PFadeToBlack( C_GEAR_WORK* pWork )
{
  // 黒く
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,  1, 0, 16, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );
}

static void _PFadeFromBlack( C_GEAR_WORK* pWork )
{
  // 黒く
  PaletteFadeReq(
    pWork->pfade_ptr, PF_BIT_SUB_OBJ, 0xffff,  1, 16, 0, _BLACK_COLOR[pWork->sex], pWork->pfade_tcbsys
    );
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレットフェード停止
 */
//-----------------------------------------------------------------------------
static void _PFadeStop( C_GEAR_WORK* pWork )
{
  PaletteFadeForceStop( pWork->pfade_ptr );
}

static BOOL _PFadeIsFade( const C_GEAR_WORK* cpWork )
{
  return PaletteFadeCheck( cpWork->pfade_ptr );
}


#if 0
static void _gearBootInitScreen(C_GEAR_WORK* pWork)
{
  int i;
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
      _gearPanelBgScreenMake(pWork, x, y, CGEAR_PANELTYPE_NONE);
    }
  }
  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
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


  if( START_UP_SCREEN_ANIME_FRAME_MAX >= pWork->startCounter ){
    // 徐々に枠が増えていく
    for(i=0;i < elementof(StartUpScreenTable);i++){
      if(StartUpScreenTable[i].time == pWork->startCounter) {
        _PanelMarkAnimeStart( &pWork->panel_mark[ StartUpScreenTable[i].x ][ StartUpScreenTable[i].y ], pWork, PANEL_COLOR_TYPE_BASE, PANEL_ANIME_TYPE_ON_OFF, CGEAR_PANELTYPE_BASE, StartUpScreenTable[i].frame );
        
        //_gearPanelBgScreenMake(pWork, StartUpScreenTable[i].x, StartUpScreenTable[i].y, 
        //    CGEAR_PANELTYPE_BOOT, FALSE);
      }
    }
    pWork->startCounter++;
  }else{
    if( _PanelMarkAnimeSysIsAnime( pWork ) == FALSE ){
      return TRUE;
    }
  }



  // パネルアニメのシステムメイン
//  _PanelMarkAnimeSysMain( pWork );

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  点灯させたテーブルを消していく。
 *
 *	@param	pWork
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static void _gearStartUpAllOff(C_GEAR_WORK* pWork)
{
  int x, y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};
  
  // まず全体を点等
  for(x = 0; x < C_GEAR_PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
      _PanelMarkAnimeSetOff( &pWork->panel_mark[ x ][ y ], pWork, CGEAR_PANELTYPE_BASE );
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   パネルタイプをスクリーンに書き込む
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _gearPanelBgScreenMake(C_GEAR_WORK* pWork,int xs,int ys, CGEAR_PANELTYPE_ENUM type)
{
  int ypos[2] = {PANEL_Y1,PANEL_Y2};
  int x,y,i,j;
  int typepos[] = {0x18, 0x0c,0x10,0x14,0x18,0x1c};
  u16* pScrAddr = GFL_BG_GetScreenBufferAdrs(GEAR_BUTTON_FRAME );
  int xscr;
  int yscr;
  int col_index;
  int col_type;

  _gearXY2PanelScreen(xs,ys,&xscr,&yscr);


  // パネルタイプとxインデックスからパレットナンバーを取得
  col_type = sc_PANEL_TYPE_TO_COLOR[ type ];
  col_index = _PanelMark_GetPatternIndex( pWork, col_type, xs, ys );

  for(y = yscr, i = 0; i < PANEL_SIZEXY; y++, i++){
    for(x = xscr, j = 0; j < PANEL_SIZEXY; x++, j++){
      if((x >= 0) && (x < 32)){
        int charpos = typepos[type] + i * 0x20 + j;
        int scr = x + (y * 32);

        
        pScrAddr[scr] = (sc_PANEL_COLOR_ANIME_TBL[col_type][ col_index ]<<12) | charpos;
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

static BOOL _gearGetTypeBestPosition(C_GEAR_WORK* pWork,CGEAR_PANELTYPE_ENUM type, int* px, int* py)
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
      if(_cgearSave_GetPanelType(pWork,x,y) == type)
      {
        // 最初にあった位置を覚えておく。
        if( first_data == FALSE ){
          first_x = x;
          first_y = y;
          first_data = TRUE;
        }

        // 
        if( _cgearSave_IsPanelTypeIcon(pWork,x,y) ){ // アイコンが出る場所
          _gearXY2PanelScreen(x,y,px,py);
          return TRUE;
        }

        // 
        if( _cgearSave_IsPanelTypeLast(pWork,x,y,type) ){ // アイコンが出る場所
          first_x = x;
          first_y = y;
          first_data = TRUE;
        }
      }
    }
  }
  
  if( first_data ){
    // もしなかったら、最初に見つかった位置にする。
    _gearXY2PanelScreen(first_x,first_y,px,py);
    CGEAR_SV_SetPanelType( pWork->pCGSV, first_x, first_y, type, TRUE, FALSE );
//    _gearPanelBgScreenMake(pWork, first_x, first_y, type );
    GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
    return TRUE;
  }

  return FALSE;
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
  int type;

  for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
    for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し

//      _gearPanelBgScreenMake(pWork, x, y, _cgearSave_GetPanelType(pWork,x,y));
    }
  }
  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
}
#endif


//------------------------------------------------------------------------------
/**
 * @brief   ギアのARCを読み込み
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearArcCreate(C_GEAR_WORK* pWork, int sex, u32 design_no, BOOL pal_trans)
{
  u32 scrno=0;

  if( pal_trans ){
    int pal, i;
    void* buff;
    NNSG2dPaletteData* pltt;
    u16* pltt_data;
	
    //BGパレットをVRAM転送
    GFL_ARCHDL_UTIL_TransVramPalette( pWork->handle,
        NARC_c_gear_sw_c_gear_m_nclr+sex, PALTYPE_SUB_BG, 0, 0x20*BG_USE_PLTT_NUM, GFL_HEAP_LOWID(pWork->heapID) );

    //パネルのパレットアニメ用ベースカラーを取得 
    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_sw_c_gear_m_obj_nclr+sex, &pltt, GFL_HEAP_LOWID(pWork->heapID) );

    pltt_data = pltt->pRawData;
    for( pal = 0 ; pal < CGEAR_PANEL_MAX; pal++){
      pWork->palBase[pal] = pltt_data[(16*(PANEL_COLOR_TRANS_POS_PALNO+pal)) + PANEL_COLOR_TRANS_POS_OFS ];
    }
    GFL_HEAP_FreeMemory( buff );

    //パネルとすれ違いアイコン用パレットアニメのソースカラーを取得
    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_sw_c_gear_obj_ani_nclr, &pltt, GFL_HEAP_LOWID(pWork->heapID) );

    for( i=0; i<_CGEAR_NET_PALTYPE_MAX; i++ ){
      _PaletteMake(pWork,pltt->pRawData,i);
    }
    GFL_HEAP_FreeMemory( buff );

    //リング色変更用のパレットソースを取得
    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_sw_c_gear_ring_nclr, &pltt, GFL_HEAP_LOWID(pWork->heapID) );

    pltt_data = pltt->pRawData;
    for( pal = 0 ; pal < RING_COLOR_MAX; pal++){
      MI_CpuCopy8( &pltt_data[(16*(pal+(sex*RING_COLOR_MAX)))], pWork->palRing[pal], RING_COLOR_SET_NUM_ALL*2 );
    }
    GFL_HEAP_FreeMemory( buff );

  }

  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    pWork->handle, NARC_c_gear_sw_c_gear_ncgr, GEAR_MAIN_FRAME, 0, 
    0, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );
  
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    pWork->handle, NARC_c_gear_sw_c_gear01_nscr, GEAR_MAIN_FRAME, 0, 
    0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );
  GFL_ARCHDL_UTIL_TransVramScreen(
    pWork->handle, NARC_c_gear_sw_off_gear_nscr+pWork->power_flag, GEAR_BUTTON_FRAME, 0, 
    0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );

#if 0
  // サブ画面BGキャラ転送
  {
    u32 pos;
    pos = GFL_BG_AllocCharacterArea( GEAR_MAIN_FRAME, GEAR_CGX_SIZE_MAX, GFL_BG_CHRAREA_GET_F );
    pWork->subchar = GFL_ARCUTIL_TRANSINFO_Make( pos, GEAR_CGX_SIZE_MAX );
    GFL_ARCHDL_UTIL_TransVramBgCharacter( pWork->handle, _bgcgx[design_no][sex],
                                          GEAR_MAIN_FRAME, pos, 0, FALSE, HEAPID_FIELD_SUBSCREEN);
  }

  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(pWork->handle,
                                         NARC_c_gear_c_gear01_NSCR,
                                         GEAR_MAIN_FRAME, 0,
                                         GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                         HEAPID_FIELD_SUBSCREEN);


#endif
}



static void _gearObjResCreate(C_GEAR_WORK* pWork, int sex)
{
  u32 scrno=0;

  pWork->objRes[_CLACT_PLT] = GFL_CLGRP_PLTT_Register( pWork->handle,
                                                       NARC_c_gear_sw_c_gear_m_obj_nclr+pWork->sex,
                                                       CLSYS_DRAW_SUB , 0 , pWork->heapID );

  pWork->objRes[_CLACT_CHR] = GFL_CLGRP_CGR_Register( pWork->handle,
                                                      NARC_c_gear_sw_c_gear_obj_ncgr ,
                                                      FALSE , CLSYS_DRAW_SUB , pWork->heapID );
  pWork->objRes[_CLACT_CHR_PANEL] = GFL_CLGRP_CGR_Register( pWork->handle,
                                                      NARC_c_gear_sw_c_gear_btn_m1_ncgr+(pWork->sex*_CGEAR_TYPE_PATTERN_NUM+pWork->design_no),
                                                      FALSE , CLSYS_DRAW_SUB , pWork->heapID );

  pWork->objRes[_CLACT_ANM] = GFL_CLGRP_CELLANIM_Register( pWork->handle,
                                                           NARC_c_gear_sw_c_gear_obj_ncer ,
                                                           NARC_c_gear_sw_c_gear_obj_nanr ,
                                                           pWork->heapID );
  pWork->objRes[_CLACT_ANM_PANEL] = GFL_CLGRP_CELLANIM_Register( pWork->handle,
                                                           NARC_c_gear_sw_c_gear_btn_m1_ncer ,
                                                           NARC_c_gear_sw_c_gear_btn_m1_nanr ,
                                                           pWork->heapID );

  // CrossObjカラー取得
  {
    void * p_buff;
    NNSG2dPaletteData* p_pltt;
    u16* pltt_data;
    p_buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_sw_c_gear_obj_ani_nclr, &p_pltt, pWork->heapID );

    // 色を保存
    pltt_data = (u16*)p_pltt->pRawData;
    MI_CpuCopy16( &pltt_data[CROSS_COLOR_SRC_POS_START], pWork->crossColor, sizeof(u16)*CROSS_COLOR_MAX);

    GFL_HEAP_FreeMemory( p_buff );
  }

  GFL_NET_SetIconForcePosition( POS_COMM_ICON_X, POS_COMM_ICON_Y );
  GFL_NET_ReloadIconTopOrBottom( FALSE, pWork->heapID );
}

static void _gearObjResChange(C_GEAR_WORK* pWork)
{
  void *buf;
  NNSG2dCharacterData* p_char;

  buf = GFL_ARC_LoadDataAllocByHandle( pWork->handle,
          NARC_c_gear_sw_c_gear_btn_m1_ncgr+(pWork->sex*_CGEAR_TYPE_PATTERN_NUM+pWork->design_no),
          GFL_HEAP_LOWID( pWork->heapID ) );
  NNS_G2dGetUnpackedCharacterData( buf, &p_char );

  //キャラクタ転送
  GFL_CLGRP_CGR_Replace(  pWork->objRes[ _CLACT_CHR_PANEL ], p_char );

  GFL_HEAP_FreeMemory( buf );
}


#if 0


static void _gearDecalScreenArcCreate(C_GEAR_WORK* pWork,BOOL bDecal)
{
  u32 scrno=0;

  if(bDecal == FALSE){
    scrno = NARC_c_gear_c_gear00_nodecal_NSCR;
    GFL_ARCHDL_UTIL_TransVramScreenCharOfs(pWork->handle,
                                           scrno,
                                           GEAR_DECAL_FRAME, 0,
                                           GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar), 0, 0,
                                           HEAPID_FIELD_SUBSCREEN);
  }

}
#endif

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
    int frame = GEAR_DECAL_FRAME;
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

  // 一応ObjWindowを消しておく
  GXS_SetVisibleWnd( GX_WNDMASK_NONE );
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
  G2S_SetBlendAlpha( GX_BLEND_PLANEMASK_BG1, GX_BLEND_PLANEMASK_BG2|GX_BLEND_PLANEMASK_BG0|GX_BLEND_PLANEMASK_OBJ, 9, 16 );
}

#if 0
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
  int type;
  int xp,yp;
  u32 touchx,touchy;
  BOOL result;

  // 起動してないなら、電源いがい触れない。
  if( pWork->power_flag == FALSE ){
    if( (bttnid != TOUCH_LABEL_POWER) &&
        (bttnid != TOUCH_LABEL_ALL) ){
      return ;
    }
    if( bttnid != TOUCH_LABEL_POWER ){
      return ;
    }
  }

  // すでに選択後であれば、何もしない。
  if( pWork->createEvent != FIELD_SUBSCREEN_ACTION_NONE ){
    return ;
  }

  GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);  //タッチした事になる

  switch(bttnid){
  case TOUCH_LABEL_ALL:
    result  = GFL_UI_TP_GetPointCont(&touchx,&touchy);
    GF_ASSERT( result );

    type = getTypeToTouchPos(pWork,touchx,touchy,&xp,&yp);

    if( pWork->bPanelEdit ){

      pWork->tpx = touchx;
      pWork->tpy = touchy;

      //タッチ場所のタイプをとっておく。
      if(((_gearPanelTypeNum(pWork,type) > 1 ) || (type == CGEAR_PANELTYPE_BASE)) && _isSetChip(xp,yp))
      {
        type = (type+1) % CGEAR_PANELTYPE_MAX;
        _modeSetSavePanelType(pWork, pWork->pCGSV,xp,yp,type);
        GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );
        PMSND_PlaySE( SEQ_SE_MSCL_07 );
      }

    }else{


      // ボタンプッシュ
      if( !SleepMode_IsSleep( pWork ) ){  //通信ONなら

        pWork->touchx =xp;
        pWork->touchy =yp;

        switch( type ){
        case CGEAR_PANELTYPE_IR:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_IRC;
          break;
        case CGEAR_PANELTYPE_WIFI:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_GSYNC;
          break;
        case CGEAR_PANELTYPE_WIRELESS:
          pWork->createEvent = FIELD_SUBSCREEN_ACTION_WIRELESS;
          break;
        }
      }
    }
    break;
  case TOUCH_LABEL_CUSTOM:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );
    pWork->bPanelEdit = pWork->bPanelEdit ^ 1;
    _editMarkONOFF(pWork, pWork->bPanelEdit);
    break;
  case TOUCH_LABEL_HELP:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CGEAR_HELP;
    break;
  case TOUCH_LABEL_CROSS:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW;
    break;
  case TOUCH_LABEL_S_RADAR:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_SCANRADAR;
    break;

  case TOUCH_LABEL_LOGO:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );

    GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                             GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));
    pWork->design_no = (++pWork->design_no) % _CGEAR_TYPE_PATTERN_NUM;

    CGEAR_SV_SetCGearType( pWork->pCGSV, pWork->design_no );

    _gearArcCreate(pWork, pWork->sex, pWork->design_no, FALSE);  //ARC読み込み BG&OBJ

//    _gearPanelBgCreate(pWork);	// パネル作成
//    _gearMarkObjDrawEnable(pWork,TRUE);

    break;

  case TOUCH_LABEL_POWER:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CGEAR_POWER;
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

  if( SleepMode_IsSleep( pWork ) ){
    return;
  }

  if( event == GFL_BMN_EVENT_TOUCH ){
    _touchFunction(pWork,bttnid);
  }
}
#endif

//----------------------------------------------------------------------------
/**
 *	@brief	タッチパネル入力取得
 *
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static int _TouchButtonCheck( C_GEAR_WORK* pWork )
{
  int i,ret;
  GFL_UI_TP_HITTBL  tbl[CGEAR_BUTTON_ALL_TBL_MAX];


  MI_CpuCopy8( DATA_FixedButtonTbl, tbl, sizeof(GFL_UI_TP_HITTBL) * CGEAR_BUTTON_FIXED_TBL_MAX); //固定テーブルコピー

  //可変テーブル値生成
  for( i = 0;i < CGEAR_PANEL_MAX; i++ ){
    GFL_CLACTPOS  pos;
    GFL_UI_TP_HITTBL* pt = &tbl[CGEAR_BUTTON_FIXED_TBL_MAX+i];
    _gearObjGetPanelPos( pWork, i, &pos );

    pt->circle.code = GFL_UI_TP_USE_CIRCLE;
    pt->circle.x = pos.x;
    pt->circle.y = pos.y;
    pt->circle.r = TP_PANEL_R;
  }
  tbl[CGEAR_BUTTON_ALL_TBL_MAX-1].rect.top = GFL_UI_TP_HIT_END; //終了コード

  ret = GFL_UI_TP_HitTrg( tbl );

  if( ( ret == GFL_UI_TP_HIT_NONE ) ||
      ( pWork->power_flag == FALSE && ret >= CGEAR_BUTTON_POWER_ON_ONLY_MAX )){
    return GFL_UI_TP_HIT_NONE;
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチパネル リング入力取得
 *
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static int _TouchRingCheck( C_GEAR_WORK* pWork )
{
  int ret;
 
  if( pWork->power_flag == FALSE ||
      pWork->bPanelEdit == FALSE ){  //電源OffかカスタムモードOffなら無反応
    return GFL_UI_TP_HIT_NONE;
  }
  ret = GFL_UI_TP_HitTrg( DATA_FixedCircleTbl );
  
  if( ret == GFL_UI_TP_HIT_NONE || ((ret%2) == 0 )){
    return GFL_UI_TP_HIT_NONE;
  }
  return (ret/2)+TOUCH_LABEL_RING0;
}

//----------------------------------------------------------------------------
/**
 *	@brief	タッチパネル パネルドラッグ操作
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _TouchPanelDrugCheck( C_GEAR_WORK* pWork )
{
  int ret;
  u32 x,y;

  if( GFL_UI_TP_GetPointCont( &x, &y ) == FALSE ){  //ドラッグ終了
    //最終座標にもっとも近いポイントをサーチ
    ret = _gearPanelPosSearchNearyPoint( pWork, pWork->select_panel, pWork->touchx, pWork->touchy );

    if( ret != PANEL_POS_NULL ){  //新しい位置にセット
      pWork->panel_pos[ pWork->select_panel ] = ret;
      CGEAR_SV_SetCGearPanelPos( pWork->pCGSV, pWork->select_panel, ret );
    }
    _gearPanelPosSet( pWork, pWork->select_panel, pWork->panel_pos[pWork->select_panel] );
    _gearPanelSetDrugMode( pWork, pWork->select_panel, FALSE );
    return;
  }
  pWork->touchx = x;
  pWork->touchy = y;
  _gearObjSetPanelPos( pWork, pWork->select_panel, x, y );
}

//----------------------------------------------------------------------------
/**
 *	@brief	入力取得
 *
 *	@param	bttnid		ボタンID
 *	@param	event		イベント種類
 *	@param	p_work		ワーク
 */
//-----------------------------------------------------------------------------
static void _InputCheck( C_GEAR_WORK* pWork )
{
  int ret;

  if( SleepMode_IsSleep( pWork ) ){
    return;
  }
  // すでに選択後であれば、何もしない。
  if( pWork->createEvent != FIELD_SUBSCREEN_ACTION_NONE ){
    return ;
  }
  if( pWork->bPanelDrug ){
    _TouchPanelDrugCheck( pWork );
    return;
  }
  if( !GFL_UI_TP_GetTrg() ){
    return;
  }
  ret = _TouchButtonCheck( pWork );

  if( ret == GFL_UI_TP_HIT_NONE ){
    ret = _TouchRingCheck( pWork );
  }
  if( ret == GFL_UI_TP_HIT_NONE ){
    return;
  }
  GFL_UI_SetTouchOrKey(GFL_APP_KTST_TOUCH);  //タッチした事になる

  switch( ret ){
  case TOUCH_LABEL_IR:
  case TOUCH_LABEL_WIRELESS:
  case TOUCH_LABEL_WIFI:
    {
      GFL_CLACTPOS  pos;
      CGEAR_PANEL_ID id = ret-TOUCH_LABEL_IR;

      _gearObjGetPanelPos( pWork, id, &pos );
      pWork->touchx = pos.x;
      pWork->touchy = pos.y;
      pWork->select_panel = id;

      if( pWork->bPanelEdit ){  //カスタムモード
        _gearPanelSetDrugMode( pWork, id, TRUE );  //ドラッグモードに移行
      }else{  //通常モード
        static const u8 action[CGEAR_PANEL_MAX] = {
          FIELD_SUBSCREEN_ACTION_IRC,
          FIELD_SUBSCREEN_ACTION_WIRELESS,
          FIELD_SUBSCREEN_ACTION_GSYNC,
        };
        pWork->createEvent = action[id];
      }
    }
    break;
  case TOUCH_LABEL_CUSTOM:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );
    pWork->bPanelEdit = pWork->bPanelEdit ^ 1;
    _editMarkONOFF(pWork, pWork->bPanelEdit);
    break;
  case TOUCH_LABEL_HELP:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CGEAR_HELP;
    break;
  case TOUCH_LABEL_CROSS:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW;
    break;
  case TOUCH_LABEL_S_RADAR:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_SCANRADAR;
    break;
  
  case TOUCH_LABEL_POWER:
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_CGEAR_POWER;
    break;

  case TOUCH_LABEL_LOGO:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );

    pWork->design_no = (++pWork->design_no) % _CGEAR_TYPE_PATTERN_NUM;

    CGEAR_SV_SetCGearType( pWork->pCGSV, pWork->design_no );

    _gearObjResChange( pWork );
    break;
  case TOUCH_LABEL_RING0:
  case TOUCH_LABEL_RING1:
  case TOUCH_LABEL_RING2:
  case TOUCH_LABEL_RING3:
  case TOUCH_LABEL_RING4:
    {
      u8 ring_id = ret-TOUCH_LABEL_RING0;

      PMSND_PlaySE( SEQ_SE_MSCL_07 );
      pWork->ring_col[ring_id] = (++pWork->ring_col[ring_id]) % C_GEAR_RING_COLOR_MAX;

      CGEAR_SV_SetCGearRingColor( pWork->pCGSV, ring_id, pWork->ring_col[ring_id] );

      _gearRingColorChangeCore( pWork, ring_id, pWork->ring_col[ring_id] );
    }
    break;
  }
 
  return;
}

//------------------------------------------------------------------------------
/**
 * @brief   エディットモードのONOFF
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _editMarkONOFF(C_GEAR_WORK* pWork,BOOL bOn)
{
  int i,view;

  if(bOn){
    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_CUSTOM ], ACTANM_CUSTOM_ANM );
    GFL_CLACT_WK_ResetAnm( pWork->cellCursor[ ACT_CUSTOM ] );
    view = PANEL_VIEW_CUSTOM;
  }
  else{
    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_CUSTOM ], ACTANM_CUSTOM );
    view = PANEL_VIEW_NORMAL;
  }
  for( i = 0;i < CGEAR_PANEL_MAX;i++){
    _gearObjSetPanelView( pWork, i, view );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   OBJの追加
 * @retval  none
 */
//------------------------------------------------------------------------------
static GFL_CLWK* _gearObjAdd(C_GEAR_WORK* pWork, s16 x, s16 y, u8 anm, u8 bg_pri, u8 s_pri, u8 res_idx )
{
  GFL_CLWK_DATA cellInitData;
  GFL_CLWK* obj;

  //セルの生成
  cellInitData.pos_x = x;
  cellInitData.pos_y = y;
  cellInitData.anmseq = anm;

  cellInitData.softpri = s_pri;
  cellInitData.bgpri = bg_pri;
  
  obj = GFL_CLACT_WK_Create(  pWork->cellUnit ,
                              pWork->objRes[_CLACT_CHR+res_idx],
                              pWork->objRes[_CLACT_PLT],
                              pWork->objRes[_CLACT_ANM+res_idx],
                              &cellInitData ,
                              CGEAR_REND_SUB,
                              pWork->heapID );
  GFL_CLACT_WK_SetDrawEnable( obj, FALSE );
  GFL_CLACT_WK_SetAutoAnmFlag( obj, TRUE );
  GFL_CLACT_WK_SetSRTAnimeFlag( obj, FALSE ); //SRTアニメ反映OFF

  return obj;
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルの表示モードセット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjSetPanelView(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id,  PANEL_VIEW_MODE mode )
{
  int i, act,anm;

  act = ACT_IR+(id*ACT_PANEL_SET_NUM);

  switch( mode ){
  case PANEL_VIEW_NORMAL:
    _gearObjSetAnmSeq( pWork->cellCursor[act+0], id+ANM_OFS_PANEL_NORMAL );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+0], TRUE );
    _gearObjSetAnmSeq( pWork->cellCursor[act+1], id+ANM_OFS_PANEL_RING );
    GFL_CLACT_WK_SetSoftPri( pWork->cellCursor[act+1], ACT_SPRI_PANEL_RING );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+2], TRUE );
    break;
  case PANEL_VIEW_CUSTOM:
    _gearObjSetAnmSeq( pWork->cellCursor[act+0], id+ANM_OFS_PANEL_NORMAL );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+0], TRUE );
    _gearObjSetAnmSeq( pWork->cellCursor[act+1], id+ANM_OFS_PANEL_LOOP );
    GFL_CLACT_WK_SetSoftPri( pWork->cellCursor[act+1], ACT_SPRI_PANEL_RING );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+2], TRUE );
    break;
  case PANEL_VIEW_DRUG:
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+0], FALSE );
    _gearObjSetAnmSeq( pWork->cellCursor[act+1], id+ANM_OFS_PANEL_LOOP );
    GFL_CLACT_WK_SetSoftPri( pWork->cellCursor[act+1], ACT_SPRI_PANEL_DRUG );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+2], FALSE );
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルOBJの座標取得
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjGetPanelPos( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, GFL_CLACTPOS* pos )
{
  GFL_CLACT_WK_GetPos( pWork->cellCursor[ ACT_IR + (id*ACT_PANEL_SET_NUM) ], pos, CGEAR_REND_SUB );
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルの座標セット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjSetPanelPos(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y )
{
  int i;

  for( i = 0;i < ACT_PANEL_SET_NUM; i++ ){
    _gearObjSetPos( pWork->cellCursor[ACT_IR+i+(id*ACT_PANEL_SET_NUM)], x, y );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルのVisibleセット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjSetPanelVisible(  C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, BOOL visible_f )
{
  int i;

  for( i = 0;i < ACT_PANEL_SET_NUM; i++ ){
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_IR+i+(id*ACT_PANEL_SET_NUM)], visible_f );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   OBJの座標セット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjSetPos( GFL_CLWK* p_cell, s16 x, s16 y )
{
  GFL_CLACTPOS pos;

  pos.x = x;
  pos.y = y;
  GFL_CLACT_WK_SetPos( p_cell, &pos, CGEAR_REND_SUB );
}


//------------------------------------------------------------------------------
/**
 * @brief   OBJのアニメSeqセット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearObjSetAnmSeq( GFL_CLWK* p_cell, int anm_id )
{
  GFL_CLACT_WK_SetAnmSeq( p_cell, anm_id );
  GFL_CLACT_WK_ResetAnm( p_cell );
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
  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
  
  for( i = 0; i < ACT_TIME_MAX; i++ )
  {
    pWork->cellCursor[ACT_T_HOUR1+i] = 
      _gearObjAdd( pWork, POS_T_NUM_X+(POS_T_NUM_OX*i), POS_T_NUM_Y, ACTANM_N10, CGEAR_CLACT_BG_PRI, ACT_SPRI_PARTS, _CLACT_RES_COMMON );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ACT_T_HOUR1+i], FALSE );
  }
  IWASAWA_Printf("ObjCreqte %d\n", ACTANM_RING_IR );
  GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ ACT_T_COLON ], ACTANM_N_COLON );

  for(i=0;i < ACT_NORMAL_MAX ;i++)
  {
    static const struct {
      u8  anm;
      u8  x,y;
      u8  s_pri:4;
      u8  res_idx:4;
    }prm[]=
    {
      { ACTANM_WIN,       POS_AMPM_BASE_X, POS_AMPM_BASE_Y, ACT_SPRI_BASE, _CLACT_RES_COMMON },
      { ACTANM_AMPM,      POS_AMPM_X, POS_AMPM_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_WIN,       POS_BATTERY_BASE_X, POS_BATTERY_BASE_Y, ACT_SPRI_BASE, _CLACT_RES_COMMON },
      { ACTANM_BATT2,     POS_BATTERY_X, POS_BATTERY_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_LOGO_ALL,  POS_LOGO_X, POS_LOGO_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_POWER,     POS_POWER_X, POS_POWER_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      
      { ACTANM_HELP,      POS_HELP_X, POS_HELP_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_CUSTOM,    POS_CUSTOM_X, POS_CUSTOM_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_S_RADAR,   POS_S_RADAR_X, POS_S_RADAR_Y, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_CROSS,     POS_CROSS_X, POS_CROSS_Y, ACT_SPRI_CROSS, _CLACT_RES_COMMON },
      { ACTANM_CONNECT,   POS_CONNECT_X, POS_CONNECT_Y, ACT_SPRI_CROSS, _CLACT_RES_COMMON },
      
      { ACTANM_IR,            POS_IR_DEF_X, POS_IR_DEF_Y, ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_IR,       POS_IR_DEF_X, POS_IR_DEF_Y, ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      { ACTANM_S_IR,          POS_IR_DEF_X, POS_IR_DEF_Y, ACT_SPRI_PANEL, _CLACT_RES_COMMON },
      { ACTANM_WIRELESS,      POS_WIRELESS_DEF_X, POS_WIRELESS_DEF_Y, ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_WIRELESS, POS_WIRELESS_DEF_X, POS_WIRELESS_DEF_Y, ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      { ACTANM_S_WIRELESS,    POS_WIRELESS_DEF_X, POS_WIRELESS_DEF_Y, ACT_SPRI_PANEL, _CLACT_RES_COMMON },
      { ACTANM_WIFI,          POS_WIFI_DEF_X, POS_WIFI_DEF_Y, ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_WIFI,     POS_WIFI_DEF_X, POS_WIFI_DEF_Y, ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      { ACTANM_S_WIFI,        POS_WIFI_DEF_X, POS_WIFI_DEF_Y, ACT_SPRI_PANEL, _CLACT_RES_COMMON },
      
      { ACTANM_E_IR,      POS_IR_DEF_X, POS_IR_DEF_Y, ACT_SPRI_EFFECT, _CLACT_RES_COMMON },
    };

    //セルの生成
    pWork->cellCursor[ACT_AMPM_BASE+i] = _gearObjAdd( pWork, prm[i].x, prm[i].y, prm[i].anm, CGEAR_CLACT_BG_PRI, prm[i].s_pri, prm[i].res_idx );
  }
  if( OS_IsRunOnTwl() ){  //DSIならバッテリー表示パターンを変更
    GFL_CLACT_WK_SetAnmSeq( pWork->cellCursor[ACT_BATTERY], ACTANM_BATT3 );
  }
  GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[ACT_PANEL_EFF], TRUE ); //SRTアニメ反映
  for(i=0;i < CGEAR_PANEL_MAX ;i++)
  {
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[ACT_IR_RING+(i*ACT_PANEL_SET_NUM)], TRUE ); //SRTアニメ反映
  }
//  _editMarkONOFF(pWork,FALSE);
  _timeAnimation(pWork);
//  _typeAnimation(pWork);
}

#if 0
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
#endif

//----------------------------------------------------------------------------
/**
 *	@brief  全OAMの表示許可
 *
 *	@param	pWork
 *	@param	bFlg
 */
//-----------------------------------------------------------------------------
static void _gearAllObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg)
{
  int i;

  // 選択時アニメ用
  for(i=0;i < _SELECT_ANIME_OBJ_MAX;i++)
  {
    if( pWork->cellSelect[i].cellSelect ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellSelect[i].cellSelect, bFlg );
    }
  }
#if 0 
  // タイプ表示用
  for(i=0;i < _CLACT_TYPE_MAX ;i++)
  {
    if( pWork->cellType[i] && pWork->power_flag ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellType[i], bFlg );
    }
  }
#endif
  // カーソル
  for(i=0;i < ACT_DEF_DRAW_MAX;i++)
  {
    if( pWork->cellCursor[i]  == NULL ){
      continue;
    }
    if( i > ACT_COMMON_MAX ){
      if( pWork->power_flag ){
        GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], bFlg );
      }
    }else{
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], bFlg );
    }
  }
  if( pWork->connect_flag == FALSE ){ //一人も通信バッファにいない時は、CONNECT表示は強制Off
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ ACT_CONNECT], FALSE ); 
  }
#if 0

  // すれ違いよう
  if( pWork->cellCross && pWork->power_flag ){
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCross, bFlg );
  }
  if( pWork->cellCrossBase && pWork->power_flag ){
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCrossBase, bFlg );
  }

  // レーダー
  if( pWork->cellRadar && pWork->power_flag ){
    GFL_CLACT_WK_SetDrawEnable( pWork->cellRadar, bFlg );
  }
#endif
  // 時計再設定
  if( bFlg == TRUE ){
    _timeAnimation( pWork );
  }
  
}

#if 0
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

#endif

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの開始
 */
//-----------------------------------------------------------------------------
static void _gearStartUpAnimeStart(C_GEAR_WORK* pWork)
{
#if 0
  int i;
  GFL_CLWK_DATA cellInitData;

  // オートアニメーション開始
  for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
  {

    //セルの生成
    cellInitData.pos_x = STARTUP_ANIME_POS_X;
    cellInitData.pos_y = STARTUP_ANIME_POS_Y;
    cellInitData.anmseq = STARTUP_ANIME_INDEX_START;// + i;
    cellInitData.softpri = 0;
    cellInitData.bgpri = CGEAR_CLACT_BG_PRI;
    //↑矢印
    pWork->cellStartUp[i] = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->objRes[_CLACT_CHR],
                                              pWork->objRes[_CLACT_PLT],
                                              pWork->objRes[_CLACT_ANM],
                                              &cellInitData ,
                                              CGEAR_REND_SUB,
                                              pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellStartUp[i], FALSE );
    GFL_CLACT_WK_SetAutoAnmSpeed( pWork->cellStartUp[i], FX32_ONE*2 );


    GFL_CLACT_WK_ResetAnm( pWork->cellStartUp[i] );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellStartUp[i], TRUE );
  }
#endif
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの終了
 */
//-----------------------------------------------------------------------------
static void _gearStartUpAnimeEnd(C_GEAR_WORK* pWork)
{
  //生きてるタスクがあれば全て殺す
  GFL_TCBL_DeleteAll( pWork->tcbsys );

  _gearAllObjDrawEnable( pWork, TRUE );
  _gearRingColorChangeInit( pWork );
#if 0
  int i;

  if( pWork->cellStartUp[0] ){
    // オートアニメーション開始
    for(i=0;i < STARTUP_ANIME_OBJ_MAX;i++)
    {
      GFL_CLACT_WK_Remove( pWork->cellStartUp[i] );
      pWork->cellStartUp[i] =NULL;
    }
  }
#endif
}

//========================================================================
//起動時のアニメーションのシーケンス
//========================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメ部分
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL _gearStartUpMainBoot(C_GEAR_WORK* pWork)
{
  switch(pWork->state_seq){
  case STARTUP_BOOT_SEQ_ANIME_START:
    //リング色をBOOT時カラーに
//    _gearRingColorChangeBoot( pWork );
    {
      int i;
      for( i = 0;i < RING_NUM;i++ ){
        _gearRingColorPalAnmCore( pWork, i, 16, _BLACK_COLOR[pWork->sex] ); //一度黒く
      }
    }
    //パネルのステータスカラーアニメ初期化
    _PanelPaletteAnimeColClear( pWork );

    // 全OAM　非表示
    _gearAllObjDrawEnable( pWork, FALSE );
    pWork->state_seq ++;
    break;
  case STARTUP_BOOT_SEQ_WIPE_IN:
    //サブ画面フェード待ち
    if( _modeSelectWaitSubFadeEnd( pWork ) == FALSE ){
      break;
    }
    //リングパレットアニメ登録
    pWork->eff_task_ct = 0;
    {
      int i;

      for( i = 0;i < RING_NUM;i++ ){
        _taskAdd_StartUpRingAnm( pWork, STARTUP_ANM_MODE_BOOT,
          i, pWork->ring_col[i], STARTUP_ANM_RING_ANM_INTERVAL*i, &pWork->eff_task_ct );
      }
    }
    pWork->state_seq ++;
    break;
  case STARTUP_BOOT_SEQ_ANIME_WAIT:
    if( pWork->eff_task_ct ){
      break;
    }
//    _gearStartUpAnimeEnd(  pWork );
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープ復帰アニメ部分
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL _gearStartUpMainWakeUp(C_GEAR_WORK* pWork)
{
  switch(pWork->state_seq){
  case STARTUP_WAKEUP_SEQ_ANIME_START:
    pWork->eff_task_ct = 0;
    {
      int i;

      for( i = RING_NUM-1;i >= 0; i-- ){
        _taskAdd_StartUpRingAnm( pWork, STARTUP_ANM_MODE_WAKEUP,
          i, pWork->ring_col[i], STARTUP_ANM_RING_ANM_INTERVAL*((RING_NUM-1)-i), &pWork->eff_task_ct );
      }
    }
    pWork->state_seq ++;
    break;
  case STARTUP_WAKEUP_SEQ_ANIME_WAIT:
    if( pWork->eff_task_ct ){
      break;
    }
    _gearStartUpAnimeEnd(  pWork );
    return TRUE;
  }
  return FALSE;

#if 0
  int i;
  int x , y;
  int yloop[2] = {PANEL_HEIDHT1,PANEL_HEIGHT2};

  switch( pWork->state_seq ){
  case BOOT_ANIME_SEQ_ALL_ON:
    // まず全体を点等
    for(x = 0; x < PANEL_WIDTH; x++){   // XはPANEL_WIDTH回
      for(y = 0; y < yloop[ x % 2]; y++){ //Yは xの％２でyloopの繰り返し
        _PanelMarkAnimeStart( &pWork->panel_mark[ x ][ y ], pWork, PANEL_COLOR_TYPE_BASE, PANEL_ANIME_TYPE_ON, CGEAR_PANELTYPE_BASE, PANEL_ANIME_DEF_FRAME );
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
          panel_type = _cgearSave_GetPanelType( pWork, screenTable[i].x, screenTable[i].y );
          _PanelMarkAnimeStart( &pWork->panel_mark[ screenTable[i].x ][ screenTable[i].y ], 
              pWork, sc_PANEL_TYPE_TO_COLOR[ panel_type ], PANEL_ANIME_TYPE_OFF, panel_type, screenTable[i].frame );
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
    if( (_PanelMarkAnimeSysIsAnime( pWork ) == FALSE) && (_PFadeIsFade( pWork ) == FALSE) ){
      return TRUE;
    }
    break;

  default:
    break;
  }


  // パネルアニメのシステムメイン
  _PanelMarkAnimeSysMain( pWork );

  return FALSE;
#endif
}

//========================================================================
//起動時のリングパレットアニメーション
//========================================================================
typedef struct{
  C_GEAR_WORK* cgw;

  u8  mode;
  u8  ring_id;
  u8  ring_col;
  u8  pal_pos;
  u8  ini_wait;
  s8  evy;
  s8  end_evy;
  s8  dif;
  u8  seq;

  int*  task_ct;
}TASKWK_STARTUP_RINGANM;

static void tcb_StartUpRingAnm( GFL_TCBL *tcb , void* tcb_wk);

/*
 *  @brief  起動リングアニメ登録
 */
static void _taskAdd_StartUpRingAnm( C_GEAR_WORK* pWork, u8 mode, u8 ring_id, u8 ring_col, u8 wait, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_STARTUP_RINGANM* twk;

  tcb = GFL_TCBL_Create( pWork->tcbsys, tcb_StartUpRingAnm, sizeof(TASKWK_STARTUP_RINGANM), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_STARTUP_RINGANM ));

  twk->cgw = pWork;
  twk->mode = mode;
  twk->ring_id = ring_id;
  twk->ring_col = ring_col;
  twk->ini_wait = wait;

  twk->evy = -16;
  twk->end_evy = 0;
  twk->dif = 2;
  _gearRingColorChangeCore( pWork, ring_id, ring_col ); //カラーをセット
  _gearRingColorPalAnmCore( pWork, ring_id, 16, _BLACK_COLOR[pWork->sex] ); //一度黒く

  if( task_ct != NULL ){
    twk->task_ct = task_ct;
    (*twk->task_ct)++;
  }
}

static void tcb_StartUpRingAnm( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_STARTUP_RINGANM* twk = (TASKWK_STARTUP_RINGANM*)tcb_wk;
  C_GEAR_WORK* pWork = twk->cgw;
  u16 color[2];

  color[0] = 0x7FFF;  //白
  color[1] = _BLACK_COLOR[pWork->sex];  //黒

  switch( twk->seq ){
  case 0: //初期ウェイト
    if( twk->ini_wait--  <= 0 ){
      twk->seq++;
    }
    return;
  case 1: //オリジナルカラー+8までフェード
    twk->evy += twk->dif;
    _gearRingColorPalAnmCore( pWork, twk->ring_id, MATH_ABS(twk->evy), color[(twk->evy<=0)] );
    if( twk->evy != twk->end_evy ){
      return;
    }

    if( twk->mode == STARTUP_ANM_MODE_BOOT ){
      twk->dif = -2;
      twk->end_evy = -16;
    }else{
      twk->dif = 0;
      twk->end_evy = 0;
      _gearRingObjVisibleSet( pWork, twk->ring_id, TRUE );
    }
    twk->seq++;
    return;
  case 2: //終端カラーまで戻す
    twk->evy += twk->dif;
    _gearRingColorPalAnmCore( pWork, twk->ring_id, MATH_ABS(twk->evy), color[(twk->evy<=0)] );
    if( twk->evy != twk->end_evy ){
      return;
    }
    break;
  }
  if( twk->task_ct != NULL ){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}

#if 0
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

  {

    //セルの生成

    cellInitData.pos_x = POS_CROSS_X_CENTER;
    cellInitData.pos_y = POS_CROSS_Y_CENTER;
    cellInitData.anmseq = NANR_c_gear_obj_CellAnime_surechigai;
    cellInitData.softpri = 0;
    cellInitData.bgpri = CGEAR_CLACT_BG_PRI;
    pWork->cellCross = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->objRes[_CLACT_CHR],
                                               pWork->objRes[_CLACT_PLT],
                                               pWork->objRes[_CLACT_ANM],
                                               &cellInitData ,
                                               CGEAR_REND_SUB,
                                               pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCross, FALSE );
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCross, FALSE );
  }
  {
    cellInitData.pos_x = POS_CROSS_X_CENTER;
    cellInitData.pos_y = POS_CROSS_Y_CENTER;
    cellInitData.anmseq = NANR_c_gear_obj_CellAnime_surechigai_waku;
    cellInitData.softpri = 20;
    cellInitData.bgpri = CGEAR_CLACT_BG_PRI;
    pWork->cellCrossBase = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                               pWork->objRes[_CLACT_CHR],
                                               pWork->objRes[_CLACT_PLT],
                                               pWork->objRes[_CLACT_ANM],
                                               &cellInitData ,
                                               CGEAR_REND_SUB,
                                               pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCrossBase, FALSE );
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCrossBase, FALSE );
    
  }

  {
    //セルの生成

    cellInitData.pos_x = POS_SCANRADAR_X;
    cellInitData.pos_y = POS_SCANRADAR_Y;
    cellInitData.anmseq = NANR_c_gear_obj_CellAnime_radar;
    cellInitData.softpri = 0;
    cellInitData.bgpri = CGEAR_CLACT_BG_PRI;
    pWork->cellRadar = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                            pWork->objRes[_CLACT_CHR],
                                            pWork->objRes[_CLACT_PLT],
                                            pWork->objRes[_CLACT_ANM],
                                            &cellInitData ,
                                            CGEAR_REND_SUB,
                                            pWork->heapID );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellRadar, FALSE );
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellRadar, FALSE );
  }
}

#endif

//------------------------------------------------------------------------------
/**
 * @brief   すれ違いOBJの実行
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearCrossObjMain(C_GEAR_WORK* pWork)
{
  const GAMEBEACON_INFO *beacon_info;
  u8 col;
  int i;
  int check_count;
  s32 log_count;
  int put_count = 0; 

  if( !pWork->power_flag ){
    return ;
  }

  if( _PanelPalette_IsExMode(pWork) ){
    return ;
  }

  log_count = GAMEBEACON_Get_LogCount();
  
  //Beacon変更チェック
  if( pWork->cross_change == FALSE ){
    check_count = 0;
    if( GAMEBEACON_Get_UpdateLogNo(&check_count) != GAMEBEACON_SYSTEM_LOG_MAX ){
      pWork->cross_change = TRUE; // 変更あり
    }
  }
  

  // 色設定
  log_count --; // 0オリジン
  for(i=0;i < GAMEBEACON_INFO_TBL_MAX;i++)
  {
    // 10個表示したら終わり
    if( put_count >= CROSS_COLOR_ICON_MAX ){
      break;
    }
    
    if( log_count < 0 ){
      // 何もなしの色を設定
      PaletteWorkSet( pWork->pfade_ptr, &pWork->crossColor[ 16 ], FADE_SUB_OBJ, CROSS_COLOR_TRANS_POS_START + put_count, 2 );
      put_count ++;
    }else{
    
      beacon_info = GAMEBEACON_Get_BeaconLog(log_count);
      if(beacon_info != NULL){
        col = GAMEBEACON_Get_FavoriteColorIndex(beacon_info);
        // colの色を設定
        PaletteWorkSet( pWork->pfade_ptr, &pWork->crossColor[ col ], FADE_SUB_OBJ, CROSS_COLOR_TRANS_POS_START + put_count, 2 );

        if( pWork->connect_flag == FALSE ){
          pWork->connect_flag  = TRUE;
          GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ ACT_CONNECT], TRUE );  //CONNECT表示開始 
        }
        put_count ++;
      }

      log_count --;
    }
  }
}
//------------------------------------------------------------------------------
/**
 * @brief   パネルリング色変更
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingColorChangePanel(C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, u8 col_no )
{
  u16*  pal = pWork->palRing[ col_no ];

  PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ],
    FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_PANEL_START+(id*16), RING_COLOR_SET_NUM*2 );
}

//------------------------------------------------------------------------------
/**
 * @brief   リング色変更コア
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingColorChangeCore(C_GEAR_WORK* pWork, u8 ring_id, u8 col_no )
{
  int i;
  int siz  = RING_COLOR_SET_NUM*2;
  u16*  pal = pWork->palRing[ col_no ];

  //リングベースの3色を転送
  PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_BG],
      FADE_SUB_BG, ring_id*RING_COLOR_SET_NUM+RING_COLOR_TRANS_POS_BG_START, siz );
  
  //パネルリングカラー転送
  for( i = 0;i < CGEAR_PANEL_MAX;i++) {
    if( ring_id != _gearPanelRingIDGet( pWork, i ) ){ //属しているリングが異なればスルー
      continue;
    } 
    _gearRingColorChangePanel( pWork, i, col_no );
  }

  //関連づいたOBJのリング色を転送
  switch( ring_id ){
  case 2: //内から3番目
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START1, siz );
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ_D],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START2, siz );
    break;
  case 4: //一番外側
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R4_START1, siz );
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ_D],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R4_START2, siz );
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   リング色パレットアニメコア
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingColorPalAnmCore(C_GEAR_WORK* pWork, u8 ring_id, u8 evy, u16 color  )
{
  int i;
  //リングベースの3色に適用
  SoftFadePfd( pWork->pfade_ptr, FADE_SUB_BG,
      ring_id*RING_COLOR_SET_NUM+RING_COLOR_TRANS_POS_BG_START, RING_COLOR_SET_NUM, evy, color );
//  PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_BG*RING_COLOR_SET_NUM],
//      FADE_SUB_BG, ring_id*RING_COLOR_SET_NUM+RING_COLOR_TRANS_POS_BG_START, siz );
  
  //パネルリングカラー転送
  for( i = 0;i < CGEAR_PANEL_MAX;i++) {
    if( ring_id != _gearPanelRingIDGet( pWork, i ) ){ //属しているリングが異なればスルー
      continue;
    } 
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_PANEL_START+(ring_id*16), RING_COLOR_SET_NUM, evy, color );
//    _gearRingColorChangePanel( pWork, i, col_no );
  }

  //関連づいたOBJのリング色を転送
  switch( ring_id ){
  case 1: //内から2番目

    break;
  case 2: //内から3番目
    /*
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ*RING_COLOR_SET_NUM],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START1, siz );
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ_D*RING_COLOR_SET_NUM],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START2, siz );
    */
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R2_START1, RING_COLOR_SET_NUM, evy, color );
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R2_START2, RING_COLOR_SET_NUM, evy, color );
    break;
  case 4: //一番外側
    /*
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ*RING_COLOR_SET_NUM],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R4_START1, siz );
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ_D*RING_COLOR_SET_NUM],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R4_START2, siz );
    */
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R4_START1, RING_COLOR_SET_NUM, evy, color );
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R4_START2, RING_COLOR_SET_NUM, evy, color );
    break;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   リングに属するOBJの表示非表示をセット
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingObjVisibleSet(C_GEAR_WORK* pWork, u8 ring_id, BOOL visible_f )
{
  int i;

  //パネル表示制御
  for( i = 0;i < CGEAR_PANEL_MAX;i++) {
    if( ring_id == _gearPanelRingIDGet( pWork, i ) ){ //属しているリングが異なればスルー
      _gearObjSetPanelVisible( pWork, i, visible_f );
    } 
  }

  //関連づいたOBJの表示制御
  switch( ring_id ){
  case 1: //内から2番目
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_CROSS], visible_f );
    if( pWork->connect_flag ){ //一人も通信バッファにいない時は、CONNECT表示は強制Off
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_CONNECT], visible_f );
    }
    break;
  case 2: //内から3番目
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_LOGO], visible_f );
    break;
  case 4: //一番外側
    for( i = ACT_R4_OBJ_START; i <= ACT_R4_OBJ_END; i++){
      if( i == ACT_LOGO ) continue; //ACT_LOGOのみr3 ちょっと仲間はずれ
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], visible_f );
    }
    break;
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   リング色変更 初期化時
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingColorChangeInit(C_GEAR_WORK* pWork )
{
  int i;

  for( i = 0; i < RING_NUM;i++){
    pWork->ring_col[i] = CGEAR_SV_GetCGearRingColor( pWork->pCGSV, i );
    _gearRingColorChangeCore( pWork, i, pWork->ring_col[i] );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   リング色変更 ブート時
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearRingColorChangeBoot(C_GEAR_WORK* pWork )
{
  int i;

  for( i = 0; i < RING_NUM;i++){
    _gearRingColorChangeCore( pWork, i, RING_COLOR_BOOT_NO );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルポジション 属しているリングIDを取得
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8 _gearPanelRingIDGet( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id )
{
  return DATA_PanelPosTbl[ pWork->panel_pos[id] ].ring_id;
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルポジション設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearPanelPosSet( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, u8 pos )
{
  const PANEL_POS_DATA*  pd;
  pWork->panel_pos[id] = pos;

  pd = &DATA_PanelPosTbl[pos];
  _gearObjSetPanelPos( pWork, id, pd->x, pd->y );
  _gearRingColorChangePanel( pWork, id, pWork->ring_col[ pd->ring_id ] );
  OS_Printf("panel pos %d p=%d -> ( %d, %d, %d )\n",id, pos, pd->x, pd->y, pd->ring_id );
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルポジション初期化設定
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearPanelPosSetInit( C_GEAR_WORK* pWork )
{
  int i;
  
  for( i = 0; i < CGEAR_PANEL_MAX; i++ ){
    pWork->panel_pos[i] = CGEAR_SV_GetCGearPanelPos( pWork->pCGSV, i );
    _gearPanelPosSet( pWork, i, pWork->panel_pos[i] );
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   2点間の距離取得 VEC_Distance()の内部処理を参考に2次元版
 * @retval  none
 */
//------------------------------------------------------------------------------
static fx32 _gearGetDistance( s16 x1, s16 y1, s16 x2, s16 y2 )
{
  VecFx32 v1,v2;

  VEC_Set( &v1, FX32_CONST(x1), FX32_CONST(y1), 0);
  VEC_Set( &v2, FX32_CONST(x2), FX32_CONST(y2), 0);
  return VEC_Distance( &v1, &v2 );
}

//------------------------------------------------------------------------------
/**
 * @brief   パネルポジション パネル同士あたり判定を取得
 * @retval  none
 */
//------------------------------------------------------------------------------
static BOOL _gearPanelPosIsHit( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y )
{
  int i;
  fx32  dis;

  for( i = 0;i < CGEAR_PANEL_MAX; i++ ){
    GFL_CLACTPOS  pos;
    if( i == id ){
      continue;
    }
    _gearObjGetPanelPos( pWork, i, &pos );
    dis = _gearGetDistance( pos.x, pos.y, x, y );

    if( dis <= FX32_CONST(TP_PANEL_R*2) ){
//      IWASAWA_Printf("Panel%dHit p%d (%d,%d) (%d,%d) ->%d\n",id, i, pos.x,pos.y, x, y,FX_Whole(dis)); 
      return TRUE;
    }
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   パネルポジション 指定座標にもっとも近いポイントをサーチ
 * @retval  none
 */
//------------------------------------------------------------------------------
static u8 _gearPanelPosSearchNearyPoint( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, s16 x, s16 y )
{
  int i,pos;
  fx32  min = FX32_MAX; //十分大きい値で初期化

//  IWASAWA_Printf("GetNearyPoint =%d\n", id );
  for( i = 0; i < PANEL_POS_TBL_MAX; i++ ){
    fx32  dis;
    const PANEL_POS_DATA* pd = &DATA_PanelPosTbl[i];

    //このポイントで他パネルと接触するかどうか？
    if( _gearPanelPosIsHit( pWork, id, pd->x, pd->y ) ){
      continue;
    }
    dis = _gearGetDistance( pd->x, pd->y, x, y );
//    IWASAWA_Printf("dis2=%08x, %d\n", dis, FX_Whole(dis)); 
    if( dis < min ){
      min = dis;
      pos = i;
    }
  }
  IWASAWA_Printf(" Neary = %d -> %d\n", pos, FX_Whole(min));
  if( min > PANEL_DISTANCE_ENABLE_MAX ){
    return PANEL_POS_NULL; 
  }
  return pos;
}

//------------------------------------------------------------------------------
/**
 * @brief   パネル ドラッグモードOn/Off
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearPanelSetDrugMode( C_GEAR_WORK* pWork, CGEAR_PANEL_ID id, BOOL on_f )
{
  int i;

  pWork->bPanelDrug = on_f;

  if( on_f ){
    for( i = 0; i < CGEAR_PANEL_MAX; i++ ){
      if( id == i ){
        _gearObjSetPanelView( pWork, i, PANEL_VIEW_DRUG );
      }else{
        _gearObjSetPanelView( pWork, i, PANEL_VIEW_NORMAL );
      }
    }
  }else{
    for( i = 0; i < CGEAR_PANEL_MAX; i++ ){
      _gearObjSetPanelView( pWork, i, PANEL_VIEW_CUSTOM );
    }
  }
}

#if 0
//------------------------------------------------------------------------------
/**
 * @brief   すれ違いOBJの開放
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearCrossObjDelete(C_GEAR_WORK* pWork)
{
  int i;

  if(pWork->cellCross){
    GFL_CLACT_WK_Remove(pWork->cellCross);
    pWork->cellCross = NULL;
  }
  if(pWork->cellCrossBase){
    GFL_CLACT_WK_Remove(pWork->cellCrossBase);
  }
  if(pWork->cellRadar){
    GFL_CLACT_WK_Remove(  pWork->cellRadar);
    pWork->cellRadar=NULL;
  }

}

#endif

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト全体の初期化
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeInit(C_GEAR_WORK* pWork,BOOL bBoot)
{
  MYSTATUS* pMy = GAMEDATA_GetMyStatus( GAMESYSTEM_GetGameData(pWork->pGameSys) );

  pWork->sex = MyStatus_GetMySex(pMy) % 2;  //PM_NEUTRALは男性扱い
  pWork->design_no = CGEAR_SV_GetCGearType(pWork->pCGSV);

  {
    int i;
    for( i = 0;i < C_GEAR_RING_NUM; i++ ){
      pWork->ring_col[i] = CGEAR_SV_GetCGearRingColor( pWork->pCGSV, i);
    }
  } 

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 64+ACT_MAX, 0 , pWork->heapID );

  // ユーザーレンダラー設定
  pWork->userRend = GFL_CLACT_USERREND_Create( sc_REND_SURFACE_INIT, CGEAR_REND_MAX, pWork->heapID );
  GFL_CLACT_UNIT_SetUserRend( pWork->cellUnit, pWork->userRend );
  
  _gearArcCreate(pWork, pWork->sex, pWork->design_no, TRUE);  //ARC読み込み BG&OBJ
  _gearObjResCreate(pWork, pWork->sex);
  _gearObjCreate(pWork); //CLACT設定
//  _gearCrossObjCreate(pWork);
//  pWork->pButton = GFL_BMN_Create( bttndata, _BttnCallBack, pWork,  pWork->heapID );

  if(bBoot){
//    _gearBootInitScreen(pWork);
    _gearAllObjDrawEnable(pWork, FALSE);
//    _gearAllObjDrawEnable(pWork, TRUE );
  }
  else{
//    _gearPanelBgCreate(pWork);	// パネル作成
    _gearAllObjDrawEnable(pWork, TRUE);
//    _gearMarkObjDrawEnable(pWork,TRUE);
  }

}






static void _arcGearRelease(C_GEAR_WORK* pWork)
{

///  _gearCrossObjDelete(pWork);
  _gearStartUpAnimeEnd( pWork );

  {
    int i;
    for(i = 0;i < ACT_MAX; i++){
      if(pWork->cellCursor[i]){
        GFL_CLACT_WK_Remove( pWork->cellCursor[i] );
        pWork->cellCursor[i] = NULL;
      }
    }
#if 0
    for(i = 0;i < _CLACT_TYPE_MAX; i++){
      if( pWork->cellType[i]){
        GFL_CLACT_WK_Remove( pWork->cellType[i] );
        pWork->cellType[i] = NULL;
      }
    }
#endif
    for(i = 0;i < _SELECT_ANIME_OBJ_MAX; i++){
      if(pWork->cellSelect[i].cellSelect){
        GFL_CLACT_WK_Remove( pWork->cellSelect[i].cellSelect );
        pWork->cellSelect[i].cellSelect = NULL;
      }
    }
  }


  GFL_CLGRP_CELLANIM_Release( pWork->objRes[_CLACT_ANM_PANEL] );
  GFL_CLGRP_CELLANIM_Release( pWork->objRes[_CLACT_ANM] );
  GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR_PANEL] );
  GFL_CLGRP_CGR_Release( pWork->objRes[_CLACT_CHR] );
  GFL_CLGRP_PLTT_Release( pWork->objRes[_CLACT_PLT] );

#if 0
  GFL_BG_FreeCharacterArea(GEAR_MAIN_FRAME,GFL_ARCUTIL_TRANSINFO_GetPos(pWork->subchar),
                           GFL_ARCUTIL_TRANSINFO_GetSize(pWork->subchar));

#endif
}


static void _workEnd(C_GEAR_WORK* pWork)
{
#if 0
  if(pWork->pButton){
    GFL_BMN_Delete(pWork->pButton);
  }
#endif
  _arcGearRelease(pWork);

  GFL_CLACT_USERREND_Delete( pWork->userRend );
  GFL_CLACT_UNIT_Delete( pWork->cellUnit );


  GFL_BG_FreeBGControl(GEAR_BUTTON_FRAME);
  GFL_BG_FreeBGControl(GEAR_DECAL_FRAME);
  GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);


  GFL_BG_SetVisible( GEAR_BUTTON_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_DECAL_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

}

//----------------------------------------------------------------------------
/**
 *	@brief  バッテリーに対応した、アニメーションINDEXを返す
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static u32 _batteryIconGetTwlIndex( void )
{
  static u8 sc_Index[] = {
    0,
    0,
    1,
    1,
    2,
    3,
  };
  return sc_Index[ GFL_UI_GetBattLevel() ];
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
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_AMPM ];
    int num = time.hour / 12;
    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {  //時10
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_HOUR1 ];
    int num = (time.hour % 12) / 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
#if 0
    if(num==0){
      GFL_CLACT_WK_SetDrawEnable(cp_wk,FALSE);
    }
    else{
      GFL_CLACT_WK_SetDrawEnable(cp_wk,TRUE);
    }
#endif
  }
  {  //時1
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_HOUR2 ];
    int num = (time.hour % 12) % 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }

  {  //コロン
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_COLON ];
    int num = time.second % 2;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }


  {//秒１０
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_MIN1 ];
    int num = time.minute / 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {//秒１
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_MIN2 ];
    int num = time.minute % 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_BATTERY ];
    if( OS_IsRunOnTwl() ){//DSIなら
      int index = _batteryIconGetTwlIndex();
      if(GFL_CLACT_WK_GetAnmIndex(cp_wk) != index ){
        GFL_CLACT_WK_SetAnmIndex(cp_wk, index );
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

#if 0
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
    if( _gearGetTypeBestPosition(pWork, CGEAR_PANELTYPE_IR+i, &x, &y) ){
      x *= 8;
      y *= 8;
      pos.x = x+24-6-2;  // OBJ表示の為の補正値
      pos.y = y+6+6+3;
      GFL_CLACT_WK_SetPos(pWork->cellType[i], &pos, CGEAR_REND_SUB);

      //BTS:5510対処  マイナスにならないように対処。
      x+=32;
      // X位置をプライオリティに。
      GFL_CLACT_WK_SetSoftPri( pWork->cellType[i], x/8 );

    }else{

      pos.x = -64;  // OBJ表示の為の補正値
      pos.y = 0;
      GFL_CLACT_WK_SetPos(pWork->cellType[i], &pos, CGEAR_REND_SUB);

    }
  }
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   モードセレクト画面待機　（通常ループ）　コア
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWaitCore(C_GEAR_WORK* pWork)
{
//  GFL_BMN_Main( pWork->pButton );
  _InputCheck( pWork );

  // スリープカラー反映
  SleepMode_ColorUpdate( pWork );

  // ボタン特殊状態のカラー反映
  RadarCrossBttn_Off( pWork );
  
  _timeAnimation(pWork);

  // スリープ中は停止
  if( !SleepMode_IsSleep( pWork ) ){
    if( pWork->power_flag ){
      _PanelPaletteUpdate( pWork ); 
      _PanelPaletteChange(pWork);

      _gearCrossObjMain(pWork);
    }
  }else{

    // CGEAR以外でイベントが発生
    // ボタンのリクエストクリア
    pWork->createEvent = FIELD_SUBSCREEN_ACTION_NONE;
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

  // BTS:7471
  // 条件抜けのケア
  // ここまで来て、演出完了コールバックがあったら、
  // 完了コールバックを呼ぶ
  {
    if(pWork->pCall){
      pWork->pCall(pWork->pWork);
    }
    pWork->pCall=NULL;
    pWork->pWork=NULL;
  }

  // 画面が出るまで待つ
  if( _modeSelectWaitSubFadeEnd( pWork ) == FALSE ){
    return ;
  }

  
  //コア 
  _modeSelectMenuWaitCore( pWork ); 


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
  if(_gearStartUpMainWakeUp(pWork) == FALSE){
    if(pWork->pCall){
      pWork->pCall(pWork->pWork);
    }
    pWork->pCall=NULL;
    pWork->pWork=NULL;
    return;
  }

  _PanelPaletteAnimeInitSleep( pWork );
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
  if( _gearStartUpMainBoot( pWork ) ){
    _CHANGE_STATE( pWork, _modeSelectMenuWait2 );
  }

#if 0
  switch(pWork->state_seq){
  case STARTUP_SEQ_ANIME_START:

    // 何もないテーブルを書き込み
//    _gearBootInitScreen( pWork ); // 1回表示するため。

    _PanelPaletteAnimeColClear( pWork );

    // 全OAM　非表示
    _gearAllObjDrawEnable( pWork, FALSE );

    pWork->state_seq ++;
    break;

  case STARTUP_SEQ_WIPE_IN:

    if( _modeSelectWaitSubFadeEnd( pWork ) == FALSE ){
      break;
    }


    // 起動アニメだけ表示
    _gearStartUpAnimeStart( pWork );
    _gearStartUpObjDrawEnabel( pWork, TRUE );
    pWork->state_seq ++;
    break;

  // アニメ待ち
  case STARTUP_SEQ_ANIME_WAIT:

    
    if( _gearIsEndStartUpObjAnime( pWork ) == FALSE ){
      break;
    }

    // ブラックアウト開始
    _PFadeToBlack( pWork );

    pWork->startCounter = 0;
    pWork->state_seq ++;
    break;

  // ALPHAアニメウエイト
  case STARTUP_SEQ_OAM_ALPHA_WAIT:

    // スキップ
    if( _IsEffectSkip(pWork) ){
      _PFadeStop( pWork );  // フェード停止
      pWork->state_seq = STARTUP_SEQ_SKIP;
      break;
    }

    if( pWork->startCounter < STARTUP_OAM_ALPHA_ANIME_FRAME_MAX ){
      pWork->startCounter ++;
      break;
    }
    
    _gearStartUpObjDrawEnabel( pWork, FALSE );
    _gearStartUpAnimeEnd( pWork );
    // OAMブラックアウト
    _PFadeSetBlack(pWork);

    pWork->state_seq++;
    pWork->startCounter = 0;
    break;

  // テーブルが順に出てくるアニメ
  case STARTUP_SEQ_TBL_IN_WAIT:

    // スキップ
    if( _IsEffectSkip(pWork) ){
      pWork->state_seq = STARTUP_SEQ_SKIP;
      break;
    }

    if( _gearStartUpMain( pWork ) == TRUE ){

      pWork->startCounter = 0;
      pWork->state_seq++;
    }
    break;

  case STARTUP_SEQ_TBL_ALPHA_WAIT:

    // スキップ
    if( _IsEffectSkip(pWork) ){
      pWork->state_seq = STARTUP_SEQ_SKIP;
      break;
    }
    
    pWork->startCounter ++;
    if( pWork->startCounter >= STARTUP_TBL_ALPHA_TIME_WAIT ){
      // 最終フレームで全消し
      _gearStartUpAllOff( pWork );
      pWork->startCounter = 0;
      pWork->state_seq++;
    }
    break;

  case STARTUP_SEQ_END:

    pWork->startCounter ++;
    if( pWork->startCounter < STARTUP_END_TIME_WAIT ){
      break;
    }
    
    _gearAllObjDrawEnable( pWork, TRUE );
//    _gearMarkObjDrawEnable(pWork,FALSE);
    _CHANGE_STATE(pWork, _modeSelectMenuWait2);
    break;



  case STARTUP_SEQ_SKIP:
    _gearStartUpAnimeEnd( pWork );
    // OAMブラックアウト
    _PFadeSetBlack(pWork);

    // アニメストップ
    _PanelMarkAnimeSysAllStop( pWork );
    // 全消し
    _gearStartUpAllOff( pWork );
    pWork->startCounter = STARTUP_END_TIME_WAIT;

    pWork->state_seq = STARTUP_SEQ_END;
    break;

  default:
    break;
  }
#endif 
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
    _PFadeSetBlack(pWork);
//    _gearAllObjDrawEnable( pWork, TRUE );
//    _gearMarkObjDrawEnable(pWork,FALSE);

    // 起動演出中のスリープにも対応。
    _gearStartUpAnimeEnd( pWork );

    // 枠部分を表示
//    _gearBootInitScreen( pWork ); // 1回表示するため。

    _PanelPaletteAnimeColClear( pWork );

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
    u16* screen;


    if(LOAD_RESULT_OK== SaveControl_Extra_LoadWork(pSave, SAVE_EXTRA_ID_CGEAR_PICUTRE, HEAPID_FIELD_SUBSCREEN,
                                                   pCGearWork,SAVESIZE_EXTRA_CGEAR_PICTURE)){
      CGEAR_PICTURE_SAVEDATA* pPic = (CGEAR_PICTURE_SAVEDATA*)pCGearWork;

      {
        u16 crc = GFL_STD_CrcCalc( pPic, CGEAR_PICTURTE_CHAR_SIZE+CGEAR_PICTURTE_PAL_SIZE+CGEAR_PICTURTE_SCR_SIZE );
        u16 crc2 = CGEAR_SV_GetCGearPictureCRC(CGEAR_SV_GetCGearSaveData(pSave));
        u32 char_ofs = GEAR_BASE_CGX_SIZE;//GFL_ARCUTIL_TRANSINFO_GetPos( pWork->subchar );
        if(crc == crc2){
          if(CGEAR_PICTURE_SAVE_IsPalette(pPic)){
            ret = TRUE;
            for(i=0;i<CGEAR_SW_DECAL_SIZEY;i++){
              GFL_BG_LoadCharacter(GEAR_MAIN_FRAME,&pCGearWork[CGEAR_SW_DECAL_SIZEX * 32 * i],CGEAR_SW_DECAL_SIZEX * 32, char_ofs + (5 + i)*32);
            }
            GFL_BG_LoadPalette(GEAR_MAIN_FRAME,pPic->palette,CGEAR_PICTURTE_PAL_SIZE, 32*0x0a );

            GFL_BG_LoadScreenBuffer(GEAR_DECAL_FRAME,pPic->scr,CGEAR_PICTURTE_SCR_SIZE );
            screen = GFL_BG_GetScreenBufferAdrs( GEAR_DECAL_FRAME );
            for( i=0; i<CGEAR_PICTURTE_SCR_SIZE/2; i++ ){
              screen[i] += char_ofs;
            }
            GFL_BG_LoadScreenReq(GEAR_DECAL_FRAME);
            TOMOYA_Printf( "Decal ON\n" );
          }
        }
#if PM_DEBUG
        else{
          OHNO_Printf("CRCDIFF chk%x pict%x\n",crc,crc2);
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
  
  //生きてるタスクがあれば全て殺す
  GFL_TCBL_DeleteAll( pWork->tcbsys );

  // パレットフェードを強制停止
  _PFadeStop(pWork);
  //停止分を一度パレット転送して最新にしてしまう
  if( pWork->pfade_tcbsys ) GFL_TCB_Main( pWork->pfade_tcbsys );

  // OAMをブラックアウトさせておく
  _PFadeSetBlack(pWork);
  // 枠部分を表示
//  _gearBootInitScreen( pWork ); // 1回表示するため。

  //パレット転送
//  if( pWork->pfade_tcbsys ) GFL_TCB_Main( pWork->pfade_tcbsys );

  pWork->GetOpenTrg=_CGEAR_SLEEP_START;
}

///< スリープ復帰時に呼ばれる関数
static void _SLEEPRELEASE_FUNC(void* pWk)
{
  C_GEAR_WORK* pWork = pWk;

  pWork->GetOpenTrg=_CGEAR_SLEEP_END;
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

//----------------------------------------------------------------------------
/**
 *	@brief  初期化処理
 */
//-----------------------------------------------------------------------------
static void cgear_InitWork( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect )
{
  BOOL ret = FALSE;

  pWork->heapID = HEAPID_FIELD_SUBSCREEN;
  pWork->pCGSV = pCGSV;
  pWork->subscreen = pSub;
  pWork->pGameSys = pGameSys;
  pWork->bAction = TRUE;
  pWork->power_flag = GAMESYSTEM_GetAlwaysNetFlag( pWork->pGameSys );
  pWork->use_skip = TRUE;
  pWork->createEvent = FIELD_SUBSCREEN_ACTION_NONE;
  pWork->doEvent = FIELD_SUBSCREEN_ACTION_NONE;

  pWork->handle = GFL_ARC_OpenDataHandle( ARCID_C_GEAR_SW, pWork->heapID );

  //エフェクト用tcbシステム追加
  pWork->tcbsys = GFL_TCBL_Init( pWork->heapID, pWork->heapID, 16, 128 );
  
  //PaletteFadeReq()がTCBLを受け付けないので専用に作る
  pWork->pfade_tcbsys_wk = GFL_HEAP_AllocClearMemory( pWork->heapID, GFL_TCB_CalcSystemWorkSize(4) );
  pWork->pfade_tcbsys = GFL_TCB_Init( 4, pWork->pfade_tcbsys_wk );

  if( power_effect ){
    _CHANGE_STATE(pWork,_modeSelectMenuWait0);
  }else{
    _PanelPaletteAnimeInit( pWork );
    _CHANGE_STATE(pWork,_modeSelectMenuWait);
  }

  _createSubBg(pWork);   //BGVRAM設定
  _modeInit(pWork, power_effect);
  if(CGEAR_SV_GetCGearPictureONOFF(pWork->pCGSV)){
    ret = _loadExData(pWork,pGameSys);  //デカール読み込み
  }
//  _gearDecalScreenArcCreate(pWork,ret);

  // パネルアニメのシステムクリア
//  _PanelMarkAnimeSysInit( pWork );

  pWork->pfade_ptr = PaletteFadeInit( HEAPID_FIELD_SUBSCREEN );
  PaletteTrans_AutoSet( pWork->pfade_ptr, TRUE );
  PaletteFadeWorkAllocSet( pWork->pfade_ptr, FADE_SUB_OBJ, 0x20*OAM_USE_PLTT_NUM, HEAPID_FIELD_SUBSCREEN );
  PaletteFadeWorkAllocSet( pWork->pfade_ptr, FADE_SUB_BG, 0x20*BG_USE_PLTT_NUM, HEAPID_FIELD_SUBSCREEN );
  // 現在VRAMにあるパレットを壊さないように、VRAMからパレット内容をコピーする
  PaletteWorkSet_VramCopy( pWork->pfade_ptr, FADE_SUB_OBJ, 0, 0x20*OAM_USE_PLTT_NUM );
  PaletteWorkSet_VramCopy( pWork->pfade_ptr, FADE_SUB_BG, 0, 0x20*BG_USE_PLTT_NUM );
//  _PFadeSetDefaultPal( pWork, pWork->power_flag );  //パネルステートアニメ初期化
  _gearPanelPosSetInit( pWork );  //パネル位置初期化
  _gearRingColorChangeInit( pWork );  //リング色初期化
  PaletteFadeTrans( pWork->pfade_ptr );

  {
    pWork->vblank_tcb = GFUser_VIntr_CreateTCB( _VBlankFunc, pWork, 1 );
  }


  GFL_UI_SleepGoSetFunc(&_SLEEPGO_FUNC,  pWork);
  GFL_UI_SleepReleaseSetFunc(&_SLEEPRELEASE_FUNC,  pWork);


  // ボタンパレットアニメシステム初期化
  _BUTTONPAL_Init( pWork, &pWork->button_palfade );

  
  // CrossObj初期化
  _gearCrossObjMain( pWork );


  //初期設定を行う。
  {
    
    // 時計を合わせる。
    _timeAnimation(pWork);
    
    if( power_effect == FALSE ){

      // スリープチェック
      _cgear_SleepCheck( pWork, !(GAMESYSTEM_IsEventExists( pGameSys )) );
      // スリープカラー反映
      SleepMode_ColorUpdateEx( pWork );
    }else{
      _modeSelectMenuWait0( pWork );
    }

  }

  // もしお誘いフラグが足っていたら切る
  {
    GAME_COMM_SYS_PTR p_gcommsys = GAMESYSTEM_GetGameCommSysPtr( pGameSys );
    if(p_gcommsys){

      if( Intrude_CheckLiveCommStatus( p_gcommsys ) == LIVE_COMM_INTRUDE_PARTNER ){
        Intrude_SetLiveCommStatus_IntrudeOut( p_gcommsys );
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  破棄処理
 */
//-----------------------------------------------------------------------------
static void cgear_ExitWork( C_GEAR_WORK* pWork )
{
  // もしお誘いフラグが足っていたら切る
  {
    GAME_COMM_SYS_PTR p_gcommsys = GAMESYSTEM_GetGameCommSysPtr( pWork->pGameSys );
    if(p_gcommsys){

      if( Intrude_CheckLiveCommStatus( p_gcommsys ) == LIVE_COMM_INTRUDE_PARTNER ){
        Intrude_SetLiveCommStatus_IntrudeOut( p_gcommsys );
      }
    }
  }
  
  _BUTTONPAL_Exit( &pWork->button_palfade );
  
  GFL_UI_SleepGoSetFunc(NULL,  NULL);
  GFL_UI_SleepReleaseSetFunc(NULL,  NULL);

  GFL_NET_ResetIconForcePosition();
  GFL_NET_ReloadIconTopOrBottom( FALSE, pWork->heapID );

  GFL_TCB_DeleteTask( pWork->vblank_tcb );
  
  // タスク
  GFL_TCB_Exit( pWork->pfade_tcbsys );
  GFL_HEAP_FreeMemory( pWork->pfade_tcbsys_wk );
  GFL_TCBL_Exit( pWork->tcbsys );

  // パレットフェード
  PaletteFadeWorkAllocFree( pWork->pfade_ptr, FADE_SUB_OBJ );
  PaletteFadeWorkAllocFree( pWork->pfade_ptr, FADE_SUB_BG );
  PaletteFadeFree( pWork->pfade_ptr );

  // Wireless以外の画面にいくならライブキャスターのSEをとめる
  if( pWork->doEvent != FIELD_SUBSCREEN_ACTION_WIRELESS ){
    FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );
    FSND_StopTVTRingTone( fsnd );
  }

  GFL_ARC_CloseDataHandle( pWork->handle );

  _workEnd(pWork);
  G2S_BlendNone();

  GFL_STD_MemClear( pWork, sizeof(C_GEAR_WORK) );
}


//------------------------------------------------------------------------------
/**
 * @brief   スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_SW_Init( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect )
{
  C_GEAR_WORK *pWork = NULL;

  pWork = GFL_HEAP_AllocClearMemory( HEAPID_FIELD_SUBSCREEN, sizeof( C_GEAR_WORK ) );

  cgear_InitWork( pWork, pCGSV, pSub, pGameSys, power_effect );
  
  return pWork;
}




//------------------------------------------------------------------------------
/**
 * @brief   起動画面スタート
 * @retval  none
 */
//------------------------------------------------------------------------------
C_GEAR_WORK* CGEAR_SW_FirstInit( CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, STARTUP_ENDCALLBACK* pCall,void* pWork2, BOOL power_effect )
{
  C_GEAR_WORK* pWork = CGEAR_SW_Init( pCGSV, pSub, pGameSys, power_effect);
  
  
  pWork->pCall = pCall;
  pWork->pWork = pWork2;

  if( power_effect ){
    _CHANGE_STATE(pWork,_modeSelectMenuWait0);
    // スキップ不可能
    pWork->use_skip = FALSE;
  }else{
    if(pWork->pCall){
      pWork->pCall(pWork->pWork);
    }
    pWork->pCall=NULL;
    pWork->pWork=NULL;
  }

#if 0
  // 初期配置情報を設定
  CGEAR_SW_PATTERN_SetUp( pCGSV, pWork->handle, GFUser_GetPublicRand(CGEAR_PATTERN_MAX), 
      HEAPID_FIELD_SUBSCREEN );
#endif
  // タイプの表示をリセット
//  _typeAnimation(pWork);


  return pWork;
}



//------------------------------------------------------------------------------
/**
 * @brief   PROCMain
 * @retval  none
 */
//------------------------------------------------------------------------------




void CGEAR_SW_Main( C_GEAR_WORK* pWork,BOOL bAction )
{
  if( pWork->GetOpenTrg == _CGEAR_SLEEP_START ){
    return ;
  }
  // スリープチェック
  _cgear_SleepCheck( pWork, bAction );

  if( !SleepMode_IsSleep( pWork ) )
  {
    INTRUDE_CONNECT intrude_status = Intrude_GetIntrudeStatus(GAMESYSTEM_GetGameCommSysPtr(pWork->pGameSys));

    
    // ミッション乱入のお誘いが来ているかチェック
    if( intrude_status == INTRUDE_CONNECT_MISSION_PARTNER ){

      if( pWork->intrude == FALSE ){
        // 強制的にアニメーションをリセットして、Wirelessパネルを乱入通知アニメに切り替える
        _PanelPalette_EXSet( pWork, TRUE, FALSE );
        pWork->intrude = TRUE;
      }
    }else{

      // ミッション乱入のお誘いは終わったので、パネル表示を元に戻す
      if(pWork->intrude == TRUE){
        _PanelPalette_EXSet( pWork, FALSE, FALSE );
        pWork->intrude = FALSE;
      }
    }

    
    // トランシーバー反応処理
    if( intrude_status == INTRUDE_CONNECT_NULL ){ 
      
      u32 bit = WIH_DWC_GetAllBeaconTypeBit(GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(pWork->pGameSys)));
      FIELD_SOUND* fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );

      // トランシーバー再生処理
      if(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR){ // トランシーバー
        if( pWork->tvt_snd == FALSE ){
          //OS_TPrintf("よびだし\n");
          pWork->tvt_snd = TRUE;
          // 強制的にアニメーションをリセット
          _PanelPalette_EXSet( pWork, TRUE, TRUE );
        }
        FSND_RequestTVTRingTone( fsnd);

      // トランシーバー停止処理
      }else if(!(bit & GAME_COMM_STATUS_BIT_WIRELESS_TR)){

        if( pWork->tvt_snd ){
          FSND_StopTVTRingTone( fsnd );
          pWork->tvt_snd = FALSE;
          _PanelPalette_EXSet( pWork, FALSE, FALSE );
        }
      }
    }

    
  }

  
  if( pWork->GetOpenTrg == _CGEAR_SLEEP_END ){
    pWork->GetOpenTrg=_CGEAR_SLEEP_NONE;
    if( SleepMode_IsSleep( pWork ) == FALSE ){
      _CHANGE_STATE(pWork,_modeSelectMenuWait1);
    }else{

      // 起動演出中のスリープにも対応してブートアニメを強制終了
      _gearStartUpAnimeEnd( pWork );
//      _gearAllObjDrawEnable( pWork, TRUE );

      //
//      _gearPanelBgCreate(pWork);	// パネル作成
      // スリープカラー設定
      _PFadeSetSleepBlack( pWork, SleepMode_IsSleep(pWork) );

      //アニメ開始。
      _PanelPaletteAnimeInit( pWork );

      _CHANGE_STATE(pWork,_modeSelectMenuWait);
    }
  }
  {
    StateFunc* state = pWork->state;
    if(state != NULL){
      state(pWork);
    }
  }

  if( pWork->tcbsys ) GFL_TCBL_Main( pWork->tcbsys );
  if( pWork->pfade_tcbsys ) GFL_TCB_Main( pWork->pfade_tcbsys );
}


//------------------------------------------------------------------------------
/**
 * @brief   CGEAR_ActionCallback
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_SW_ActionCallback( C_GEAR_WORK* pWork , FIELD_SUBSCREEN_ACTION actionno)
{
}



//------------------------------------------------------------------------------
/**
 * @brief   PROCEnd
 * @retval  none
 */
//------------------------------------------------------------------------------
void CGEAR_SW_Exit( C_GEAR_WORK* pWork )
{
  cgear_ExitWork( pWork );
  GFL_HEAP_FreeMemory(pWork);
}


//----------------------------------------------------------------------------
/**
 *	@brief  パワーOFF状態に遷移
 *
 *	@param	pWork 
 *
 *	@retval TRUE  OFFになった。
 *	@retval FALSE いまは出来ない。
 */
//-----------------------------------------------------------------------------
BOOL CGEAR_SW_ChangePowerOff( C_GEAR_WORK* pWork )
{
  CGEAR_SAVEDATA* pCGSV = pWork->pCGSV;
  FIELD_SUBSCREEN_WORK* pSub = pWork->subscreen;
  GAMESYS_WORK* pGameSys = pWork->pGameSys;
  
  if( (u32)pWork->state == (u32)_modeSelectMenuWait ){
    // マスター輝度ON
    WIPE_SetBrightness( WIPE_DISP_SUB, WIPE_FADE_BLACK );

    // 破棄ー＞初期化
    cgear_ExitWork( pWork );
    cgear_InitWork( pWork, pCGSV, pSub, pGameSys, FALSE );
  
    // マスター輝度OFF
    WIPE_ResetBrightness( WIPE_DISP_SUB );

    return TRUE;
  }
  return FALSE;
}


//------------------------------------------------------------------------------
/**
 * @brief   EventCheck
 * @retval  none
 */
//------------------------------------------------------------------------------

GMEVENT* CGEAR_SW_EventCheck(C_GEAR_WORK* pWork, BOOL bEvReqOK, FIELD_SUBSCREEN_WORK* pSub )
{
  GMEVENT* event=NULL;
  GAME_COMM_SYS_PTR game_comm = GAMESYSTEM_GetGameCommSysPtr(pWork->pGameSys);
  FIELDMAP_WORK *fieldWork = GAMESYSTEM_GetFieldMapWork(pWork->pGameSys);

  if(bEvReqOK == FALSE || fieldWork == NULL){
    return NULL;
  }

  switch(pWork->createEvent){
  case FIELD_SUBSCREEN_ACTION_GSYNC:
  case FIELD_SUBSCREEN_ACTION_IRC:
    if( _PanelPalette_IsExMode( pWork ) == FALSE ){
      PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
      event = EVENT_ButtonEffectWaitCall( pWork, pWork->createEvent );
    }else{
      PMSND_PlaySystemSE( SEQ_SE_SYS_100 ); // NGオン
      event = NULL;
      _modeSelectNgAnimInit( pWork );
      _CHANGE_STATE(pWork, _modeStateSelectNgAnimeWait);
    }
    break;

  case FIELD_SUBSCREEN_ACTION_WIRELESS:
    PMSND_PlaySystemSE( GEAR_SE_DECIDE_ );
    event = EVENT_ButtonEffectWaitCall( pWork, pWork->createEvent );
    break;
    
  case FIELD_SUBSCREEN_ACTION_CGEAR_HELP:
  case FIELD_SUBSCREEN_ACTION_CGEAR_POWER:
    PMSND_PlaySE( SEQ_SE_MSCL_07 );
    event = EVENT_ButtonEffectWaitCall( pWork, pWork->createEvent );
    break;

  case FIELD_SUBSCREEN_ACTION_SCANRADAR:
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW:
    if( _PanelPalette_IsExMode( pWork ) == FALSE ){
      PMSND_PlaySE( SEQ_SE_MSCL_07 );
      event = EVENT_ButtonEffectWaitCall( pWork, pWork->createEvent );
    }else{
      PMSND_PlaySystemSE( SEQ_SE_SYS_100 ); // NGオン
      event = NULL;
    }
    break;

  }
  if( event ){
    pWork->doEvent      = pWork->createEvent;
    // イベント待ちにする。
    _CHANGE_STATE(pWork,_modeEventWait);
  }
  pWork->createEvent  = FIELD_SUBSCREEN_ACTION_NONE;
  return event;
}

//----------------------------------------------------------------------------
/**
 *	@brief  CGEAR wireless遷移動作中チェック
 *
 *	@param	cpWork  ワーク
 *
 *	@retval TRUE    動作中
 *	@retval FALSE   ちがう
 */
//-----------------------------------------------------------------------------
BOOL CGEAR_SW_IsDoEventWireless( const C_GEAR_WORK* cpWork )
{
  if( cpWork == NULL ){
    return FALSE;
  }
  if( cpWork->doEvent == FIELD_SUBSCREEN_ACTION_WIRELESS ){
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  外部でTVTRingToneをとめたときに設定してもらう。
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
void CGEAR_SW_SetStopTVTRingTone( C_GEAR_WORK* pWork )
{
  if( pWork ){
    pWork->tvt_snd = FALSE;
    _PanelPalette_EXSet( pWork, FALSE, FALSE );
  }
}

#if 0
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
      if( _cgearSave_GetPanelType( pWork, j, i ) == type ){

        if( _cgearSave_IsPanelTypeIcon( pWork, j, i ) ){
          // OFFにする。
          CGEAR_SV_SetPanelType( pSV, j, i, type, FALSE, TRUE );
//          _gearPanelBgScreenMake(pWork, j, i, type);  // 色を普通の色に
        }else if( _cgearSave_IsPanelTypeLast( pWork, j, i, type ) ){
          // LastをOFFにする。
          CGEAR_SV_SetPanelType( pSV, j, i, type, FALSE, FALSE );
        }
        
      }
    }
  }

  // ONにする。
  CGEAR_SV_SetPanelType( pSV, x, y, type, TRUE, FALSE );
//  _gearPanelBgScreenMake(pWork, x, y, type);  // つよい色に


  GFL_BG_LoadScreenReq( GEAR_BUTTON_FRAME );

  // タイプの表示をリセット
//  _typeAnimation(pWork);
}



//----------------------------------------------------------------------------
/**
 *	@brief  Ｃｇｅａｒセーブ　パネルタイプの取得
 *
 *	@param	cpWork
 *	@param	x   
 *	@param	y
 *
 *	@return パネルタイプ
 */
//-----------------------------------------------------------------------------
static CGEAR_PANELTYPE_ENUM _cgearSave_GetPanelType(const C_GEAR_WORK* cpWork,int x, int y)
{
  if( cpWork->power_flag ){
    return CGEAR_SV_GetPanelType( cpWork->pCGSV, x, y );
  }

  return CGEAR_PANELTYPE_NONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アイコンの出るところかチェック
 *
 *	@param	cpWork
 *	@param	x
 *	@param	y
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL _cgearSave_IsPanelTypeIcon(const C_GEAR_WORK* cpWork,int x, int y)
{
  if( cpWork->power_flag ){
    return CGEAR_SV_IsPanelTypeIcon( cpWork->pCGSV, x, y );
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  1つ前にアイコンがあった場所か？チェック
 *
 *	@param	cpWork
 *	@param	x
 *	@param	y
 *	@param	type 
 *
 *	@return
 */
//-----------------------------------------------------------------------------
static BOOL _cgearSave_IsPanelTypeLast(const C_GEAR_WORK* cpWork,int x, int y, CGEAR_PANELTYPE_ENUM type )
{
  if( cpWork->power_flag ){
    return CGEAR_SV_IsPanelTypeLast( cpWork->pCGSV, x, y, type );
  }
  return FALSE;
}

#endif

//----------------------------------------------------------------------------
/**
 *	@brief  スキップチェック
 *
 *	@param	const C_GEAR_WORK* cpWork 
 *
 *	@retval TRUE  スキップ
 */
//-----------------------------------------------------------------------------
static BOOL _IsEffectSkip( const C_GEAR_WORK* cpWork )
{
  if( cpWork->use_skip ){
    if( GFL_UI_TP_GetTrg() ){
      return TRUE;
    }
  }
  return FALSE;
}




//----------------------------------------------------------------------------
/**
 *	@brief  カーソル選択　待ち　アニメーション
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static BOOL _CursorSelectAnimeWait( C_GEAR_WORK* pWork )
{
  switch( pWork->eff_seq ){
  case 0:
    // アニメーション開始
    _gearObjSetAnmSeq( pWork->cellCursor[ pWork->select_cursor ], pWork->select_cursor_anm+1 );
    pWork->eff_seq ++;
    break;

  case 1:
    if( GFL_CLACT_WK_CheckAnmActive( pWork->cellCursor[ pWork->select_cursor ] ) == FALSE ){
      _gearObjSetAnmSeq( pWork->cellCursor[ pWork->select_cursor ], pWork->select_cursor_anm ); //元のアニメに戻してる
      pWork->eff_seq = 0;
      return TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  パレットアニメでの　選択アニメーション
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static BOOL _PalAnimeSelectAnimeWait( C_GEAR_WORK* pWork )
{
  switch( pWork->eff_seq ){
  case 0:
    switch( pWork->select_cursor ){
    case TOUCH_LABEL_CROSS:
      _BUTTONPAL_Start( &pWork->button_palfade, OAM_PFADE_CROSS_MSK );
      break;
    default:
      // 選択カーソル不明
      GF_ASSERT(0);
      break;
    }
    pWork->eff_seq ++;
    break;

  case 1:
    if( _BUTTONPAL_Update( &pWork->button_palfade ) ){
      pWork->eff_seq = 0;
      return TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    return TRUE;
  }
  return FALSE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  イベント待ち
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void _modeEventWait( C_GEAR_WORK* pWork )
{
}


//----------------------------------------------------------------------------
/**
 *	@brief  スリープモード開始
 *
 *	@param	pWork   ワーク
 */
//-----------------------------------------------------------------------------
static void SleepMode_Start( C_GEAR_WORK* pWork )
{
  if( pWork->sleep_mode == FALSE ){
    pWork->sleep_mode = TRUE;
    pWork->sleep_color_req = TRUE;
    pWork->sleep_color_chenage_wait = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープモード開始
 *
 *	@param	pWork   ワーク
 */
//-----------------------------------------------------------------------------
static void SleepMode_NetSleepStart( C_GEAR_WORK* pWork )
{
  if( pWork->net_sleep_mode == FALSE ){
    pWork->net_sleep_mode = TRUE;
    pWork->sleep_color_req = TRUE;
    pWork->sleep_color_chenage_wait = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  ネットスリープモード　終了
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void SleepMode_NetSleepEnd( C_GEAR_WORK* pWork )
{
  if( pWork->net_sleep_mode ){
    pWork->net_sleep_mode = FALSE;
    pWork->sleep_color_req = TRUE;
    pWork->sleep_color_chenage_wait = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープモード停止
 *
 *	@param	pWork ワーク
 */
//-----------------------------------------------------------------------------
static void SleepMode_End( C_GEAR_WORK* pWork )
{
  if( pWork->sleep_mode == TRUE ){
    pWork->sleep_mode = FALSE;
    pWork->sleep_color_req = TRUE;
    pWork->sleep_color_chenage_wait = 0;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  色変換処理
 *
 *	@param	pWork 
 */
//-----------------------------------------------------------------------------
static void SleepMode_ColorUpdate( C_GEAR_WORK* pWork )
{
  if( pWork->sleep_color_req ){

    pWork->sleep_color_chenage_wait++;
    if( pWork->sleep_color_chenage_wait >= _SLEEP_COLOR_CHANGE_WAIT ){
      // 
      _PFadeSetSleepBlack( pWork, SleepMode_IsSleep(pWork) );
      pWork->sleep_color_req = FALSE;
    }
  }
}

// 強制色変換処理
static void SleepMode_ColorUpdateEx( C_GEAR_WORK* pWork )
{
  if( pWork->sleep_color_req ){
    _PFadeSetSleepBlack( pWork, SleepMode_IsSleep(pWork) );
    pWork->sleep_color_req = FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  スリープモードチェック
 *
 *	@param	cpWork 
 */
//-----------------------------------------------------------------------------
static BOOL SleepMode_IsSleep( const C_GEAR_WORK* cpWork )
{
  if( cpWork->sleep_mode ){
    return TRUE;
  }
  if( cpWork->net_sleep_mode ){
    return TRUE;
  }
  return FALSE;

}


//----------------------------------------------------------------------------
/**
 *	@brief  レーダーcrossボタン　OFF
 *
 *	@param	pWork   ワーク
 */
//-----------------------------------------------------------------------------
static void RadarCrossBttn_Off( C_GEAR_WORK* pWork )
{
  // もしスリープ状態ではなく、
  // リクエストがきたら、パレットを変える。
  if( SleepMode_IsSleep( pWork ) == FALSE ){
    
    if( pWork->ex_radar_cross_bttn_col_change ){

      pWork->ex_radar_cross_bttn_col_change_wait ++;
      
      if( pWork->ex_radar_cross_bttn_col_change_wait >= _SLEEP_COLOR_CHANGE_WAIT ){
        if( _PanelPalette_IsExMode(pWork) ){
          // レーダー　crossボタンの色を黒くする。
          _PFadeSetExBttnBlack( pWork, TRUE );
        }else{
          // 元に戻す。
          _PFadeSetExBttnBlack( pWork, FALSE );
        }
        pWork->ex_radar_cross_bttn_col_change = FALSE;
      }
    }
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  ボタン　パレット　アニメ  初期化
 *
 *	@param	pWork   ワーク
 *	@param	p_fwk   ワーク
 */
//-----------------------------------------------------------------------------
static void _BUTTONPAL_Init( C_GEAR_WORK* pWork, BUTTON_PAL_FADE* p_fwk )
{
#if 0
  // パレットアニメーションダミーのACTOR作成
  {
    GFL_CLWK_DATA cellInitData;

    cellInitData.pos_x = 0;
    cellInitData.pos_y = 0;
    cellInitData.anmseq = ACTANM_HELP;
    cellInitData.softpri = 0;
    cellInitData.bgpri = CGEAR_CLACT_BG_PRI;

    p_fwk->dummy = GFL_CLACT_WK_Create( pWork->cellUnit ,
                                              pWork->objRes[_CLACT_CHR],
                                              pWork->objRes[_CLACT_PLT],
                                              pWork->objRes[_CLACT_ANM],
                                              &cellInitData ,
                                              CGEAR_REND_SUB,
                                              pWork->heapID );

    // 表示OFF
    GFL_CLACT_WK_SetDrawEnable( p_fwk->dummy, FALSE );
  }
#endif
  // パレットフェード
  p_fwk->p_fade_ptr     = pWork->pfade_ptr;
  p_fwk->p_fade_tcbsys  = pWork->pfade_tcbsys;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン　パレット　アニメ  破棄処理
 *
 *	@param	p_fwk 
 */
//-----------------------------------------------------------------------------
static void _BUTTONPAL_Exit( BUTTON_PAL_FADE* p_fwk )
{
//  GFL_CLACT_WK_Remove( p_fwk->dummy );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ボタン　パレット　アニメ  開始
 *
 *	@param	p_fwk   ワーク
 *	@param	msk     パレットマスク
 */
//-----------------------------------------------------------------------------
static void _BUTTONPAL_Start( BUTTON_PAL_FADE* p_fwk, u32 bit )
{
#if 0
  GFL_CLACT_WK_ResetAnm( p_fwk->dummy );
  GFL_CLACT_WK_SetAutoAnmFlag( p_fwk->dummy, TRUE );
  GFL_CLACT_WK_SetAutoAnmSpeed( p_fwk->dummy, 2<<FX32_SHIFT );
  p_fwk->anime_on = TRUE;
#endif

  p_fwk->frame = 0;
  p_fwk->msk = bit;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ボタン　パレット　アニメ　更新
 *
 *	@param	p_fwk   ワーク
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
static BOOL _BUTTONPAL_Update( BUTTON_PAL_FADE* p_fwk )
{
  u32 frame_index;

#if 0
  static const u32 sc_INDEX_Color[] = {
    0,
    9,
    0,
    9,
    0,
    9,
  };
  
  if( p_fwk->anime_on == FALSE ){
    return TRUE;
  }


  if( GFL_CLACT_WK_CheckAnmActive( p_fwk->dummy ) == FALSE ){
    p_fwk->anime_on = FALSE;
    return TRUE;
  }

  frame_index = GFL_CLACT_WK_GetAnmIndex( p_fwk->dummy );
  // パレットカラーを調整
  PaletteFadeReq(
    p_fwk->p_fade_ptr, PF_BIT_SUB_OBJ, p_fwk->msk,   -120, 0, sc_INDEX_Color[frame_index], 0x0, p_fwk->p_fade_tcbsys
    );
#else
  //2フレームごとに明暗を切り替え、3ターン点滅させる
  if( p_fwk->frame >= 12 ){
    return TRUE;
  }
  frame_index = (p_fwk->frame++/2)%2;

  // パレットカラーを調整
/*
  PaletteFadeReq(
    p_fwk->p_fade_ptr, PF_BIT_SUB_OBJ, p_fwk->msk,   -120, 0, frame_index*9, 0x0, p_fwk->p_fade_tcbsys
    );
*/
  ColorConceChangePfd( p_fwk->p_fade_ptr, FADE_SUB_OBJ, p_fwk->msk, frame_index*9, 0x0000 );

#endif
  return FALSE;
}




// CGEAR演出イベント
//----------------------------------------------------------------------------
/**
 *	@brief  CGEAR演出待ちイベント生成
 */
//-----------------------------------------------------------------------------
static GMEVENT* EVENT_ButtonEffectWaitCall( C_GEAR_WORK* pWork, u32 createEvent )
{
  EV_CGEAR_EFFECTWAIT* p_wk;
  GMEVENT * event;
  
  event = GMEVENT_Create(pWork->pGameSys, NULL, EVENT_ButtonEffectWait, sizeof(EV_CGEAR_EFFECTWAIT));
  
  p_wk = GMEVENT_GetEventWork(event);
  
  p_wk->pWork = pWork;
  p_wk->pWork->eff_seq = 0;
  p_wk->createEvent = createEvent;

  // Effect初期化
  switch(p_wk->createEvent){
  case FIELD_SUBSCREEN_ACTION_GSYNC:
  case FIELD_SUBSCREEN_ACTION_IRC:
  case FIELD_SUBSCREEN_ACTION_WIRELESS:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_PANEL; //パネル専用アニメ 
    _modeSelectAnimInit( p_wk->pWork );
    break;

  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_PAL; //パレットアニメ 
    pWork->select_cursor = TOUCH_LABEL_CROSS;
    break;
  
  case FIELD_SUBSCREEN_ACTION_SCANRADAR:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_ACT; //OBJアニメ 
    pWork->select_cursor = ACT_S_RADAR;
    pWork->select_cursor_anm = ACTANM_S_RADAR; 
    break;

  case FIELD_SUBSCREEN_ACTION_CGEAR_HELP:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_ACT; //OBJアニメ 
    pWork->select_cursor = ACT_HELP;  //OBJアニメ
    pWork->select_cursor_anm = ACTANM_HELP; 
    break;
  case FIELD_SUBSCREEN_ACTION_CGEAR_POWER:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_ACT; //OBJアニメ 
    pWork->select_cursor = ACT_POWER; //OBJアニメ
    pWork->select_cursor_anm = ACTANM_POWER; 
    break;
  }
  
  return event;
}

static GMEVENT_RESULT EVENT_ButtonEffectWait( GMEVENT *event, int *seq, void *wk )
{
  EV_CGEAR_EFFECTWAIT* p_wk = wk;
  C_GEAR_WORK* pWork = p_wk->pWork;
  BOOL result = FALSE;
  GMEVENT* p_cevent;
  FIELDMAP_WORK* fieldWork = GAMESYSTEM_GetFieldMapWork( pWork->pGameSys );

  // Effect完了待ち
  switch(p_wk->pWork->select_cursor_anm_type){
  case SELECT_CURSOR_ANM_PANEL:
    result = _modeSelectAnimWait( p_wk->pWork );
    break;
  case SELECT_CURSOR_ANM_PAL:
    result = _PalAnimeSelectAnimeWait( p_wk->pWork );
    break;

  case SELECT_CURSOR_ANM_ACT:
    result = _CursorSelectAnimeWait( p_wk->pWork );
    break;
  default:
    GF_ASSERT(0);
    result = TRUE;
  }

  if( result == FALSE ){
    return GMEVENT_RES_CONTINUE;
  }

  // アニメ完了後別イベントへ
  switch(p_wk->createEvent){
  case FIELD_SUBSCREEN_ACTION_GSYNC:
    p_cevent = GMEVENT_CreateOverlayEventCall(pWork->pGameSys, 
        FS_OVERLAY_ID(event_gsync), EVENT_CallGSync, fieldWork );
    break;
  case FIELD_SUBSCREEN_ACTION_IRC:
    p_cevent = EVENT_IrcBattle(pWork->pGameSys, fieldWork, NULL, TRUE);
    break;
  case FIELD_SUBSCREEN_ACTION_WIRELESS:
    p_cevent = GMEVENT_CreateOverlayEventCall( pWork->pGameSys,
          FS_OVERLAY_ID( notwifi_event_cg_wireless ), EVENT_CG_Wireless, NULL );
    break;
  case FIELD_SUBSCREEN_ACTION_SCANRADAR:
    p_cevent = GMEVENT_CreateOverlayEventCall( pWork->pGameSys,
          FS_OVERLAY_ID( notwifi_event_research_radar ), EVENT_ResearchRadar, (void*)fieldWork );
    break;
  case FIELD_SUBSCREEN_ACTION_CHANGE_SCREEN_BEACON_VIEW:
    p_cevent= EVENT_ChangeSubScreen(pWork->pGameSys, fieldWork, FIELD_SUBSCREEN_BeaconViewCheck(pWork->subscreen));
    break;
  case FIELD_SUBSCREEN_ACTION_CGEAR_HELP:
    {
      CG_HELP_INIT_WORK *initWork = GFL_HEAP_AllocClearMemory( HEAPID_PROC,sizeof(CG_HELP_INIT_WORK) );
      initWork->myStatus = GAMEDATA_GetMyStatus(GAMESYSTEM_GetGameData(pWork->pGameSys));
      initWork->gameSys = pWork->pGameSys;
      p_cevent= EVENT_FieldSubProc_Callback(pWork->pGameSys, fieldWork, FS_OVERLAY_ID(cg_help),&CGearHelp_ProcData,initWork,NULL,initWork);
    }
    break;


  case FIELD_SUBSCREEN_ACTION_CGEAR_POWER:
    p_cevent= EVENT_CGearPower(pWork->pGameSys);
    break;
  }

  // イベント変更
  GMEVENT_ChangeEvent( event, p_cevent );
 

  return GMEVENT_RES_CONTINUE;
}



//----------------------------------------------------------------------------
/**
 *	@brief  NGアニメ
 */
//-----------------------------------------------------------------------------
static void _modeSelectNgAnimInit(C_GEAR_WORK* pWork)
{
#if 0
  int x,y;
  CGEAR_PANELTYPE_ENUM type;

  type = _cgearSave_GetPanelType(pWork,pWork->touchx,pWork->touchy); // 1
  pWork->select_type = type;
#endif
  pWork->select_count = 0;

  // タッチした場所にセルを作成
  _selectAnimInit( pWork, pWork->touchx,pWork->touchy );
}

//----------------------------------------------------------------------------
/**
 *	@brief  NGアニメ
 */
//-----------------------------------------------------------------------------
static void _modeSelectNgAnimExit(C_GEAR_WORK* pWork)
{
  int i;

  for( i=0; i<(_SELECT_ANIME_OBJ_MAX); i++ ){
    if( pWork->cellSelect[i].cellSelect ){
      GFL_CLACT_WK_Remove( pWork->cellSelect[i].cellSelect );
      pWork->cellSelect[i].cellSelect = NULL;
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  NGアニメ
 */
//-----------------------------------------------------------------------------
static BOOL _modeSelectNgAnimWait(const C_GEAR_WORK* cpWork)
{
  int i;
  BOOL ret = TRUE;

  for( i=0; i<(_SELECT_ANIME_OBJ_MAX); i++ ){
    if( cpWork->cellSelect[i].cellSelect ){
      if( GFL_CLACT_WK_CheckAnmActive( cpWork->cellSelect[i].cellSelect ) ){
        ret = FALSE;
        break;
      }
    }
  }
  return ret;
}

//----------------------------------------------------------------------------
/**
 *	@brief  選択NGアニメ　待ち　ステート
 */
//-----------------------------------------------------------------------------
static void _modeStateSelectNgAnimeWait( C_GEAR_WORK* pWork )
{
  //コア 
  _modeSelectMenuWaitCore( pWork ); 

  
  // NGアニメ
  if( _modeSelectNgAnimWait( pWork ) ){
    _modeSelectNgAnimExit( pWork );
    // 完了
    _CHANGE_STATE(pWork, _modeSelectMenuWait);
  }
}


//----------------------------------------------------------------------------
/**
 *	@brief  スリープチェック
 */
//-----------------------------------------------------------------------------
static void _cgear_SleepCheck( C_GEAR_WORK* pWork, BOOL bAction )
{
  // 通信OFF時は強制スリープ
  if( (GFL_NET_IsInit() ==FALSE) && pWork->power_flag ){
    // スリープ
    SleepMode_NetSleepStart( pWork );
  }else{
    // スリープOFF
    SleepMode_NetSleepEnd( pWork );
  }
  
  if(pWork->bAction != bAction){

    // イベント中は、画面を暗く
    if( bAction == FALSE ){
      SleepMode_Start( pWork );
    }else{
      SleepMode_End( pWork );
    }

    pWork->bAction = bAction;
  }
}

#if 0
//----------------------------------------------------------------------------
/**
 *	@brief  patternの状態に変更
 *
 *	@param	gamedata    ゲームデータ
 *	@param	pattern     pattern
 *	@param	heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void CGEAR_SW_PATTERN_SetUp( CGEAR_SAVEDATA* pCGSV, ARCHANDLE* handle, u32 pattern, HEAPID heapID )
{
  GF_ASSERT( pattern < CGEAR_PATTERN_MAX );

  GFL_ARC_LoadDataByHandle( handle, pattern+NARC_c_gear_pattern01_bin, CGEAR_SV_GetPanelTypeBuff( pCGSV ) );
}
#endif
