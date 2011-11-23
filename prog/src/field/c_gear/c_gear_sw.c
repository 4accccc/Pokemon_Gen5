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

#include "net/wih_dwc.h"

#include "field/field_status.h"
#include "field/field_status_local.h"

#include "gamesystem/game_beacon.h"
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
  _CGEAR_NET_BIT_IR = (1 << CGEAR_PANEL_IR),
  _CGEAR_NET_BIT_WIRELESS = (1 << CGEAR_PANEL_WIRELESS),
  _CGEAR_NET_BIT_WIFI = (1 << CGEAR_PANEL_WIFI),
} _CGEAR_NET_BIT;

//ビーコンからの通知フラグID各種
enum{
  INFO_FLAG_RADAR = GAMEBEACON_CGEAR_INFO_RADAR,
  INFO_FLAG_HOT = GAMEBEACON_CGEAR_INFO_HOT,
  INFO_FLAG_FMISSION = GAMEBEACON_CGEAR_INFO_FMISSION,
  INFO_FLAG_MAX = GAMEBEACON_CGEAR_INFO_MAX,
};

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
#define DECAL_COLOR_TRANS_POS_PALNO (0x0d)

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
enum{
  RING_0, //一番内側
  RING_1,
  RING_2,
  RING_3,
  RING_4, //一番外側
  RING_NUM,
}RING_ID;
#define RING_COLOR_MAX          ( CGEAR_RING_COLOR_MAX )    //リングカラー数

#define RING_COLOR_SET_NUM   (3)
enum{
  RING_COLOR_SET_BG = 0,    //BG用
  RING_COLOR_SET_OBJ = RING_COLOR_SET_NUM,   //OBJ用のノーマル色
  RING_COLOR_SET_OBJ_D = RING_COLOR_SET_OBJ+RING_COLOR_SET_NUM, //OBJ用の暗色
};

#define FRAME_COLOR_SET_NUM_ALL (16)  //大外のフレームは16色丸ごと変える
#define RING_COLOR_SET_NUM_ALL  ( RING_COLOR_SET_NUM*3 ) //1セット3色
#define RING_COLOR_DEF_NO 　    ( 0 )  //リングカラーのデフォルト
#define RING_COLOR_BASE_NO      ( RING_COLOR_MAX-1 )  //リングの地味カラーNo

#define FRAME_COLOR_TRANS_POS_BG_PALNO  (0) //フレームカラー転送場所のパレットNo
#define RING_COLOR_TRANS_POS_BG_PALNO   (1) //リングカラー転送場所のパレットNo
#define RING_COLOR_TRANS_POS_BG_START  ((RING_COLOR_TRANS_POS_BG_PALNO*16)+1) //転送場所のパレットNo
#define RING_COLOR_TRANS_POS_OBJ_R2_START1 (0*16+1+RING_COLOR_SET_NUM)
#define RING_COLOR_TRANS_POS_OBJ_R2_START2 (1*16+1+RING_COLOR_SET_NUM)
#define RING_COLOR_TRANS_POS_OBJ_R4_START1 (0*16+1)
#define RING_COLOR_TRANS_POS_OBJ_R4_START2 (1*16+1)

#define RING_COLOR_TRANS_POS_OBJ_PANEL_START (PANEL_COLOR_TRANS_POS_PALNO*16+1)

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


#define _SELECT_ANIME_OBJ_MAX ( 4 )

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
  ACT_T_MIN1,
  ACT_T_MIN2,

  ACT_AMPM,
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
  //↑Ring0に属してる
  ACT_IR,
  ACT_IR_RING,
  ACT_WIRELESS,
  ACT_WIRELESS_RING,
  ACT_WIFI,
  ACT_WIFI_RING,

  ACT_DEF_DRAW_MAX, //デフォルト描画するものMax

  ACT_PANEL_EFF = ACT_DEF_DRAW_MAX,
  ACT_MAX,
};

#define ACT_TIME_MAX    (ACT_T_MIN2-ACT_T_HOUR1+1)
#define ACT_NORMAL_MAX  (ACT_MAX-ACT_AMPM) //初期化時だけ使用してる
#define ACT_COMMON_MAX  ( ACT_POWER ) //電源Offでも表示するOBJIDのmax
#define ACT_PANEL_SET_NUM (2)
#define ACT_R4_OBJ_START  (ACT_T_HOUR1) //Ring4に属してる
#define ACT_R4_OBJ_END    (ACT_S_RADAR)

enum{
 ACTANM_N10,
 ACTANM_AM,
 ACTANM_PM,
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
 ACTANM_CONNECT_INFO,
 ACTANM_LOGO_ALL,
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
#define POS_T_NUM_X       (POS_AMPM_BASE_X-15)
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
#define TP_CIRCLE0_OUT_R    (4*8+4)
#define TP_CIRCLE0_IN_R     (TP_CIRCLE0_OUT_R-8)
#define TP_CIRCLE1_OUT_R    (6*8+4)
#define TP_CIRCLE1_IN_R     (TP_CIRCLE1_OUT_R-8)
#define TP_CIRCLE2_OUT_R    (8*8+4+4)
#define TP_CIRCLE2_IN_R     (TP_CIRCLE2_OUT_R-8)
#define TP_CIRCLE3_OUT_R    (11*8+4)
#define TP_CIRCLE3_IN_R     (TP_CIRCLE3_OUT_R-8)
#define TP_CIRCLE4_OUT_R    (14*8+4)
#define TP_CIRCLE4_IN_R     (TP_CIRCLE4_OUT_R-8)
//フレーム
#define TP_CIRCLE5_IN_R     (17*8)
#define TP_CIRCLE5_OUT_R    (TP_CIRCLE5_IN_R+24)

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
#define SELECT_CURSOR_ANM_ACT_OBJ_NUM (2) //OBJアニメは2パーツから構成されている

enum{
  TOUCH_LABEL_IR,
  TOUCH_LABEL_WIRELESS,
  TOUCH_LABEL_WIFI,
  
  TOUCH_LABEL_EGNORE1,  //無視 時計パネルとバッテリーパネル(一応ヒットさせて、円の当たり判定に抜きを作っている)
  TOUCH_LABEL_EGNORE2,
  TOUCH_LABEL_POWER,
  TOUCH_LABEL_HELP,
  TOUCH_LABEL_CUSTOM,
  TOUCH_LABEL_S_RADAR,
  TOUCH_LABEL_CROSS,
  TOUCH_LABEL_LOGO,

  TOUCH_LABEL_RING0,
  TOUCH_LABEL_RING1,
  TOUCH_LABEL_RING2,
  TOUCH_LABEL_RING3,
  TOUCH_LABEL_RING4,
  TOUCH_LABEL_FRAME,
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
};

#define CGEAR_BUTTON_FIXED_TBL_MAX      ( NELEMS(DATA_FixedButtonTbl) )
#define CGEAR_BUTTON_ALL_TBL_MAX        ( CGEAR_BUTTON_FIXED_TBL_MAX+CGEAR_PANEL_MAX+1 )  //fixed + (panel*3) + end_code
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
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE5_IN_R },
  { GFL_UI_TP_USE_CIRCLE, TP_CIRCLE_CENTER_X, TP_CIRCLE_CENTER_Y, TP_CIRCLE5_OUT_R },
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
  {	128	,	128 ,	0	},  ///<	7
  {	128	,	144 ,	1	},  ///<	8
  {	128	,	164 ,	2	},  ///<	9
  {	128	,	184 ,	3	},  ///<	10
  {	52	,	52 	,	3	},  ///<	11
  {	68	,	60 	,	2	},  ///<	12
  {	88	,	72 	,	1	},  ///<	13
  {	168	,	120 ,	1	},  ///<	14
  {	188	,	132 ,	2	},  ///<	15
  {	204	,	140 ,	3	},  ///<	16
  {	52	,	140 ,	3	},  ///<	17
  {	68	,	132 ,	2	},  ///<	18
  {	88	,	120 ,	1	},  ///<	19
  {	168	,	72 	,	1	},  ///<	20
  {	188	,	60 	,	2	},  ///<	21
  {	204	,	52 	,	3	},  ///<	22
  {	84	,	20 	,	3	},  ///<	23
  {	96	,	36 	,	2	},  ///<	24
  {	104	,	56 	,	1	},  ///<	25
  {	112	,	68 	,	0	},  ///<	26
  {	144	,	124 ,	0	},  ///<	27
  {	152	,	136 ,	1	},  ///<	28
  {	160	,	156 ,	2	},  ///<	29
  {	172	,	172 ,	3	},  ///<	30
  {	84	,	172 ,	3	},  ///<	31
  {	96	,	156 ,	2	},  ///<	32
  {	104	,	136 ,	1	},  ///<	33
  {	112	,	124 ,	0	},  ///<	34
  {	144	,	68 	,	0	},  ///<	35
  {	152	,	56 	,	1	},  ///<	36
  {	160	,	36 	,	2	},  ///<	37
  {	172	,	20 	,	3	},  ///<	38
  {	128	,	48	,	1	},  ///<	39
  {	32	,	40	,	4	},  ///<	40
  {	224	,	40	,	4	},  ///<	41
};
#define PANEL_POS_TBL_MAX (NELEMS(DATA_PanelPosTbl))
#define PANEL_POS_NULL    (0xFF)  //パネルポジションの無効値

#define PANEL_DISTANCE_ENABLE_MAX (FX32_CONST(32)) //この値以上に距離が離れてたら無視

//-------------------------------------
///	起動アニメーション
//=====================================
#define STARTUP_ANM_MODE_BOOT     (0) //初回起動
#define STARTUP_ANM_MODE_WAKEUP   (1) //スリープ復帰
#define STARTUP_ANM_RING_ANM_INTERVAL (3)
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


#define GEAR_MAIN_FRAME   (GFL_BG_FRAME0_S)
#define GEAR_DECAL_FRAME   (GFL_BG_FRAME1_S)
#define GEAR_RING_FRAME   (GFL_BG_FRAME2_S)
#define GEAR_BASE_FRAME   (GFL_BG_FRAME3_S)

#define GEAR_MAIN_FRAME_MSK (GX_BLEND_PLANEMASK_BG0)
#define GEAR_DECAL_FRAME_MSK (GX_BLEND_PLANEMASK_BG1)
#define GEAR_RING_FRAME_MSK (GX_BLEND_PLANEMASK_BG2)
#define GEAR_BASE_FRAME_MSK (GX_BLEND_PLANEMASK_BG3)

#define GEAR_MAIN_FRAME_PRI (0)
#define GEAR_DECAL_FRAME_PRI (1)
#define GEAR_RING_FRAME_PRI (2)
#define GEAR_BASE_FRAME_PRI (3)

#define CGEAR_CLACT_BG_PRI_NA  ( GEAR_DECAL_FRAME_PRI )  //アルファ無し
#define CGEAR_CLACT_BG_PRI     ( GEAR_RING_FRAME_PRI )

#define ALPHA_TARGET_1ST  (GEAR_DECAL_FRAME_MSK)
#define ALPHA_TARGET_2ND  (GEAR_BASE_FRAME_MSK|GEAR_RING_FRAME_MSK|GEAR_DECAL_FRAME_MSK|GX_BLEND_PLANEMASK_OBJ)
#define ALPHA_1ST (21)//(10)
#define ALPHA_2ND (31-ALPHA_1ST)


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
  int selectType;   // 接続タイプ
  HEAPID heapID;
  FIELD_SUBSCREEN_WORK* subscreen;
  GAMESYS_WORK* pGameSys;
  CGEAR_SAVEDATA* pCGSV;
  ARCHANDLE* handle;  // アーカイブハンドル
  u32 objRes[_CLACT_RES_MAX];  //CLACTリソース

  GFL_CLUNIT *cellUnit;
  GFL_CLSYS_REND* userRend;
  SEL_SELECT_EFFECT cellSelect[_SELECT_ANIME_OBJ_MAX];
  GFL_CLWK  *cellCursor[ACT_MAX];
  u16 crossColor[CROSS_COLOR_MAX]; //すれ違いカラー
  u8  info_flag[INFO_FLAG_MAX];  //ビーコンから通知されるフラグ

  STARTUP_ENDCALLBACK* pCall;
  void* pWork;

  GFL_TCBLSYS* tcbsys;      //通常tcb動作用
  GFL_TCBSYS* pfade_tcbsys; //PaletteFadeReq専用
  GFL_TCB*                    vblank_tcb;
  GFL_TCBL*                   p_tcbl_fesinfo; //フェスミッション通知用TCB
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

  u16 palRing[RING_COLOR_MAX][RING_COLOR_SET_NUM_ALL];   //リングカラーパレットを保存しておくバッファ
  u16 palFrame[RING_COLOR_MAX][FRAME_COLOR_SET_NUM_ALL]; //フレームカラーパレットを保存しておくバッファ

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
  u8 custom_col[CGEAR_CUSTOM_COLOR_NUM];    //カスタムカラーNo
  u8 sex;
  u8 power_flag;    // 電源がはいっているか？
  u8 connect_flag;  // 一人分でも通信バッファにデータがあるか？
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

#ifdef PM_DEBUG
  u8  deb_alpha1;
  u8  deb_alpha2;
#endif
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
static BOOL _modeSelectWaitSubFadeEnd( const C_GEAR_WORK* cpWork );

// CGEARワーク初期化処理
// CGEARワーク破棄処理
static void cgear_InitWork( C_GEAR_WORK* pWork, CGEAR_SAVEDATA* pCGSV,FIELD_SUBSCREEN_WORK* pSub,GAMESYS_WORK* pGameSys, BOOL power_effect );
static void cgear_ExitWork( C_GEAR_WORK* pWork );

// スキップチェック
static BOOL _IsEffectSkip( const C_GEAR_WORK* cpWork );


// サブBGのセットアップ
static void _createSubBg(C_GEAR_WORK* pWork);
static void _setUpSubAlpha( C_GEAR_WORK* pWork );

static void _timeAnimation(C_GEAR_WORK* pWork);
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


// OAM操作
static void _gearAllObjDrawEnable(C_GEAR_WORK* pWork,BOOL bFlg);
static void _gearStartUpAnimeStart(C_GEAR_WORK* pWork);
static void _gearStartUpAnimeEnd(C_GEAR_WORK* pWork);
static BOOL _gearStartUpMainBoot(C_GEAR_WORK* pWork);
static BOOL _gearStartUpMainWakeUp(C_GEAR_WORK* pWork);
static void _taskAdd_StartUpRingAnm( C_GEAR_WORK* pWork, u8 mode, u8 ring_id, u8 ring_col, u8 wait, int* task_ct );

static void _gearFrameColorChange(C_GEAR_WORK* pWork, u8 col_no );
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
static BOOL SleepMode_IsSleepView( const C_GEAR_WORK* cpWork );

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
static void _cgear_CheckMissionPartner( C_GEAR_WORK* pWork, INTRUDE_CONNECT intrude_status );
static void _cgear_CheckTVTStatus( C_GEAR_WORK* pWork, INTRUDE_CONNECT intrude_status );
static void _cgear_CheckBeaconInfo( C_GEAR_WORK* pWork );

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(pWork, state)  _changeState(pWork ,state)
#endif //_NET_DEBUG


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

//ステートが通常ループかどうか？(起動演出ステートなどでないかチェック)
static BOOL _checkStateIsNormal(C_GEAR_WORK* pWork)
{
  return ( pWork->state == _modeSelectMenuWait );
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


static void _selectAnimInit(C_GEAR_WORK* pWork,int x,int y)
{
  SEL_SELECT_EFFECT* sel_select_wk;

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
    _gearObjAdd( pWork, x, y, ACTANM_E_IR+pWork->select_panel, CGEAR_CLACT_BG_PRI, ACT_SPRI_EFFECT, _CLACT_RES_COMMON );

  GFL_CLACT_WK_SetDrawEnable( sel_select_wk->cellSelect, TRUE );
  GFL_CLACT_WK_SetSRTAnimeFlag( sel_select_wk->cellSelect, TRUE ); //SRTアニメ反映
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
  pWork->select_count = 0;
}

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
    ACT_CONNECT,
    ACT_S_RADAR,
//    ACT_S_RADAR_I,
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
    _gearObjSetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR_OFF );
    GFL_CLACT_WK_SetPlttOffs( pWork->cellCursor[ACT_CONNECT], 1, CLWK_PLTTOFFS_MODE_OAM_COLOR );  //暗色パレットに変更
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ACT_CONNECT], FALSE );
  }else{
    evy = 0;
    if( pWork->info_flag[INFO_FLAG_RADAR] ){
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR_INFO );
    }else{
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR );
    }
    GFL_CLACT_WK_SetPlttOffs( pWork->cellCursor[ACT_CONNECT], 0, CLWK_PLTTOFFS_MODE_OAM_COLOR );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ACT_CONNECT], TRUE );
  }
  // ビーコンアイコンをパレットフェードで黒く
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
    
    //フレーム色変更用のパレットソースを取得
    buff = GFL_ARCHDL_UTIL_LoadPalette( pWork->handle, NARC_c_gear_sw_c_gear_frame_nclr, &pltt, GFL_HEAP_LOWID(pWork->heapID) );

    pltt_data = pltt->pRawData;
    for( pal = 0 ; pal < RING_COLOR_MAX; pal++){
      MI_CpuCopy8( &pltt_data[(16*(pal+(sex*RING_COLOR_MAX)))], pWork->palFrame[pal], FRAME_COLOR_SET_NUM_ALL*2 );
    }
    GFL_HEAP_FreeMemory( buff );

  }

  //キャラ転送
  GFL_ARCHDL_UTIL_TransVramBgCharacter(
    pWork->handle, NARC_c_gear_sw_c_gear_ncgr, GEAR_MAIN_FRAME, 0, 
    0, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );

  GFL_BG_ClearCharacter( GEAR_DECAL_FRAME );
 
  //スクリーン転送
  GFL_ARCHDL_UTIL_TransVramScreen(
    pWork->handle, NARC_c_gear_sw_c_gear01_nscr, GEAR_MAIN_FRAME, 0, 
    0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );
  if( pWork->power_flag ){
    GFL_ARCHDL_UTIL_TransVramScreen(
      pWork->handle, NARC_c_gear_sw_c_gear02_nscr, GEAR_RING_FRAME, 0, 
      0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );
  }else{
    GFL_ARCHDL_UTIL_TransVramScreen(
      pWork->handle, NARC_c_gear_sw_off_gear_nscr, GEAR_RING_FRAME, 0, 
      0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );
  }
  GFL_ARCHDL_UTIL_TransVramScreen(
    pWork->handle, NARC_c_gear_sw_c_gear03_nscr, GEAR_BASE_FRAME, 0, 
    0x800, FALSE, GFL_HEAP_LOWID( HEAPID_FIELD_SUBSCREEN ) );

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
    int frame = GEAR_BASE_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6000, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, GEAR_BASE_FRAME_PRI );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_RING_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x6800, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, GEAR_RING_FRAME_PRI );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_MAIN_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7000, GX_BG_CHARBASE_0x00000, 0x4000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, GEAR_MAIN_FRAME_PRI );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  {
    int frame = GEAR_DECAL_FRAME;
    GFL_BG_BGCNT_HEADER TextBgCntDat = {
      0, 0, 0x800, 0, GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x7800, GX_BG_CHARBASE_0x04000, 0x2000,GX_BG_EXTPLTT_01,
      0, 0, 0, FALSE
      };
    GFL_BG_SetBGControl( frame, &TextBgCntDat, GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( frame, VISIBLE_ON );
    GFL_BG_SetPriority( frame, GEAR_DECAL_FRAME_PRI );

    GFL_BG_FillScreen( frame,	0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
    GFL_BG_LoadScreenReq( frame );
  }
  // アルファ値設定
//  _setUpSubAlpha( pWork );

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
  G2S_SetBlendAlpha( ALPHA_TARGET_1ST, ALPHA_TARGET_2ND, ALPHA_1ST, ALPHA_2ND );
}

#ifdef PM_DEBUG
static void _debSetAlpha( C_GEAR_WORK* pWork )
{
  int cont = GFL_UI_KEY_GetCont();
  int trg  = GFL_UI_KEY_GetTrg();
  int flag = FALSE;
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L ){
    if( trg & PAD_BUTTON_A ){
      pWork->deb_alpha1 = (pWork->deb_alpha1+1)%32;
      flag = TRUE;
    }
  }else{
    if( trg & PAD_BUTTON_A ){
      pWork->deb_alpha1 = (pWork->deb_alpha1+31)%32;
      flag = TRUE;
    }
  }
  if( flag == 0 ){
    return;
  }
  OS_Printf("SetAlpha %d \n", pWork->deb_alpha1 );
  G2S_SetBlendAlpha( ALPHA_TARGET_1ST, ALPHA_TARGET_2ND, pWork->deb_alpha1, (31-pWork->deb_alpha1) );
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



  //可変テーブル値生成
  for( i = 0;i < CGEAR_PANEL_MAX; i++ ){
    GFL_CLACTPOS  pos;
    GFL_UI_TP_HITTBL* pt = &tbl[i];
    _gearObjGetPanelPos( pWork, i, &pos );

    pt->circle.code = GFL_UI_TP_USE_CIRCLE;
    pt->circle.x = pos.x;
    pt->circle.y = pos.y;
    pt->circle.r = TP_PANEL_R;
  }
  //固定テーブルコピー
  MI_CpuCopy8( DATA_FixedButtonTbl, &tbl[CGEAR_PANEL_MAX], sizeof(GFL_UI_TP_HITTBL) * CGEAR_BUTTON_FIXED_TBL_MAX);

  tbl[CGEAR_BUTTON_ALL_TBL_MAX-1].rect.top = GFL_UI_TP_HIT_END; //終了コード

  ret = GFL_UI_TP_HitTrg( tbl );

  if( ( ret == GFL_UI_TP_HIT_NONE ) ||
      ( pWork->power_flag == FALSE && ret != TOUCH_LABEL_POWER )){
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
  case TOUCH_LABEL_FRAME:
    {
      u8 custom_id = ret-TOUCH_LABEL_RING0;

      PMSND_PlaySE( SEQ_SE_MSCL_07 );
      pWork->custom_col[custom_id] = (++pWork->custom_col[custom_id]) % CGEAR_RING_COLOR_MAX;

      CGEAR_SV_SetCGearCustomColor( pWork->pCGSV, custom_id, pWork->custom_col[custom_id] );

      if( custom_id == CGEAR_CUSTOM_COLOR_FRAME ){
        _gearFrameColorChange( pWork, pWork->custom_col[custom_id] );
      }else{
        _gearRingColorChangeCore( pWork, custom_id, pWork->custom_col[custom_id] );
      }
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
//    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_CUSTOM_I ], ACTANM_CUSTOM_I_ANM );
    view = PANEL_VIEW_CUSTOM;
  }
  else{
    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_CUSTOM ], ACTANM_CUSTOM );
//    _gearObjSetAnmSeq( pWork->cellCursor[ ACT_CUSTOM_I ], ACTANM_CUSTOM_I );
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
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[act+1], FALSE ); //SRTアニメ反映
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
    break;
  case PANEL_VIEW_CUSTOM:
    _gearObjSetAnmSeq( pWork->cellCursor[act+0], id+ANM_OFS_PANEL_NORMAL );
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+0], TRUE );
    _gearObjSetAnmSeq( pWork->cellCursor[act+1], id+ANM_OFS_PANEL_LOOP );
    GFL_CLACT_WK_SetSoftPri( pWork->cellCursor[act+1], ACT_SPRI_PANEL_RING );
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[act+1], TRUE ); //SRTアニメ反映
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
    break;
  case PANEL_VIEW_DRUG:
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+0], FALSE );
    _gearObjSetAnmSeq( pWork->cellCursor[act+1], id+ANM_OFS_PANEL_LOOP );
    GFL_CLACT_WK_SetSoftPri( pWork->cellCursor[act+1], ACT_SPRI_PANEL_DRUG );
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[act+1], TRUE ); //SRTアニメ反映
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[act+1], TRUE );
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
  int i,pos;
  static const u8 num_ofs[] = { 0, POS_T_NUM_OX, (POS_T_NUM_OX*2-3), POS_T_NUM_OX };

  GFL_DISP_GXS_SetVisibleControl(GX_PLANEMASK_OBJ,VISIBLE_ON);
  
  pos = POS_T_NUM_X;
  for( i = 0; i < ACT_TIME_MAX; i++ )
  {
    pos += num_ofs[i];
    pWork->cellCursor[ACT_T_HOUR1+i] = 
#if 1
      _gearObjAdd( pWork, pos, POS_T_NUM_Y, ACTANM_N10, CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PARTS, _CLACT_RES_COMMON );
#else
      _gearObjAdd( pWork, pos, POS_T_NUM_Y, ACTANM_N10, CGEAR_CLACT_BG_PRI, ACT_SPRI_PARTS, _CLACT_RES_COMMON );
#endif
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ACT_T_HOUR1+i], FALSE );

  }
  for(i=0;i < ACT_NORMAL_MAX ;i++)
  {
    static const struct {
      u8  anm;
      u8  x,y;
      u8  bg_pri;
      u8  s_pri:4;
      u8  res_idx:4;
    }prm[]=
    {
      { ACTANM_AM,       POS_AMPM_BASE_X, POS_AMPM_BASE_Y, CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_BASE, _CLACT_RES_COMMON },
      { ACTANM_BATT2,       POS_BATTERY_BASE_X, POS_BATTERY_BASE_Y, CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_BASE, _CLACT_RES_COMMON },
      { ACTANM_LOGO_ALL,  POS_LOGO_X, POS_LOGO_Y, CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_POWER,     POS_POWER_X, POS_POWER_Y, CGEAR_CLACT_BG_PRI_NA,    ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      
      { ACTANM_HELP,      POS_HELP_X, POS_HELP_Y, CGEAR_CLACT_BG_PRI_NA,     ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_CUSTOM,    POS_CUSTOM_X, POS_CUSTOM_Y, CGEAR_CLACT_BG_PRI_NA,     ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_S_RADAR,   POS_S_RADAR_X, POS_S_RADAR_Y, CGEAR_CLACT_BG_PRI_NA,     ACT_SPRI_PARTS, _CLACT_RES_COMMON },
      { ACTANM_CROSS,     POS_CROSS_X, POS_CROSS_Y, CGEAR_CLACT_BG_PRI_NA,  ACT_SPRI_CROSS, _CLACT_RES_COMMON },
      { ACTANM_CONNECT,   POS_CONNECT_X, POS_CONNECT_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_CROSS, _CLACT_RES_COMMON },
      
      { ACTANM_IR,            POS_IR_DEF_X, POS_IR_DEF_Y, CGEAR_CLACT_BG_PRI_NA,  ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_IR,       POS_IR_DEF_X, POS_IR_DEF_Y, CGEAR_CLACT_BG_PRI_NA,  ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      { ACTANM_WIRELESS,      POS_WIRELESS_DEF_X, POS_WIRELESS_DEF_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_WIRELESS, POS_WIRELESS_DEF_X, POS_WIRELESS_DEF_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      { ACTANM_WIFI,          POS_WIFI_DEF_X, POS_WIFI_DEF_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PANEL_BASE, _CLACT_RES_PANEL },
      { ACTANM_RING_WIFI,     POS_WIFI_DEF_X, POS_WIFI_DEF_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_PANEL_RING, _CLACT_RES_COMMON },
      
      { ACTANM_E_IR,      POS_IR_DEF_X, POS_IR_DEF_Y,  CGEAR_CLACT_BG_PRI_NA, ACT_SPRI_EFFECT, _CLACT_RES_COMMON },
    };

    //セルの生成
    pWork->cellCursor[ACT_AMPM+i] = _gearObjAdd( pWork, prm[i].x, prm[i].y, prm[i].anm, prm[i].bg_pri, prm[i].s_pri, prm[i].res_idx );
  }
  GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ACT_AMPM], FALSE );

  if( OS_IsRunOnTwl() ){  //DSIならバッテリー表示パターンを変更
    _gearObjSetAnmSeq( pWork->cellCursor[ACT_BATTERY], ACTANM_BATT3 );
  }
  GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[ACT_PANEL_EFF], TRUE ); //SRTアニメ反映

#if 0
  for(i=0;i < CGEAR_PANEL_MAX ;i++)
  {
    GFL_CLACT_WK_SetSRTAnimeFlag( pWork->cellCursor[ACT_IR_RING+(i*ACT_PANEL_SET_NUM)], TRUE ); //SRTアニメ反映
  }
#endif
  _timeAnimation(pWork);
}

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
  // カーソル
  for(i=0;i < ACT_DEF_DRAW_MAX;i++)
  {
    if( pWork->cellCursor[i]  == NULL ){
      continue;
    }
    if( i > ACT_COMMON_MAX ){
      if( pWork->power_flag || !bFlg ){
        GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], bFlg );
        IWASAWA_Printf("NotCommon %d->%d",i,bFlg);
      }
    }else{
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[i], bFlg );
    }
  }
  if( pWork->connect_flag == FALSE ){ //一人も通信バッファにいない時は、CONNECT表示は強制Off
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ ACT_CONNECT], FALSE ); 
  }
  // 時計再設定
  if( bFlg == TRUE ){
    _timeAnimation( pWork );
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  起動アニメーションの開始
 */
//-----------------------------------------------------------------------------
static void _gearStartUpAnimeStart(C_GEAR_WORK* pWork)
{
  //リング色をBOOT時カラーに
  _gearRingColorChangeBoot( pWork );

  //パネルのステータスカラーアニメ初期化
  _PanelPaletteAnimeColClear( pWork );

  // 全OAM　非表示
  _gearAllObjDrawEnable( pWork, FALSE );
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
    //起動アップアニメの初期描画状態にする
    _gearStartUpAnimeStart( pWork );
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
          i, pWork->custom_col[i], STARTUP_ANM_RING_ANM_INTERVAL*i, &pWork->eff_task_ct );
      }
    }
    pWork->state_seq ++;
    break;
  case STARTUP_BOOT_SEQ_ANIME_WAIT:
    if( pWork->eff_task_ct ){
      break;
    }
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
    { //電源On時のみリング演出
      int i;

      for( i = RING_NUM-1;i >= 0; i-- ){
        _taskAdd_StartUpRingAnm( pWork, STARTUP_ANM_MODE_WAKEUP,
          i, pWork->custom_col[i], STARTUP_ANM_RING_ANM_INTERVAL*((RING_NUM-1)-i), &pWork->eff_task_ct );
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

//========================================================================
//フェスミッション通知時のリングパレットアニメーション
//========================================================================
typedef struct{
  C_GEAR_WORK* cgw;

  u8  pal_pos;
  u8  ini_wait;
  s8  evy;
  s8  end_evy;
  s8  dif;
  u8  seq;

  int*  task_ct;
}TASKWK_FESINFO_RINGANM;

static void tcb_FesInfoRingAnm( GFL_TCBL *tcb , void* tcb_wk);

static void ring2_FesInfoAnmCore( C_GEAR_WORK* pWork, u8 evy )
{
  //リングベースの3色に適用
  SoftFadePfd( pWork->pfade_ptr, FADE_SUB_BG,
     RING_0*RING_COLOR_SET_NUM+RING_COLOR_TRANS_POS_BG_START, RING_COLOR_SET_NUM, evy, 0x7FFF );
}

/*
 *  @brief  フェスミッション情報表示アニメ登録
 */
static void _taskAdd_FesInfoRingAnm( C_GEAR_WORK* pWork )
{
  GFL_TCBL* tcb;
  TASKWK_FESINFO_RINGANM* twk;

  if( pWork->p_tcbl_fesinfo != NULL ){
    GF_ASSERT(0);
    return;
  }
  tcb = GFL_TCBL_Create( pWork->tcbsys, tcb_FesInfoRingAnm, sizeof(TASKWK_FESINFO_RINGANM), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_FESINFO_RINGANM ));

  twk->cgw = pWork;
  twk->ini_wait = 2;

  twk->evy = 0;
  twk->end_evy = 10;
  twk->dif = 1;
//  ring2_FesInfoAnmCore( pWork, twk->evy );

  pWork->p_tcbl_fesinfo = tcb;  //TCBのポインタを憶えておく  
}

static void _taskDel_FesInfoRingAnm( C_GEAR_WORK* pWork )
{
  if( pWork->p_tcbl_fesinfo == NULL ){
    return;
  }
  //元の色に戻す
  ring2_FesInfoAnmCore( pWork, 0 );

  GFL_TCBL_Delete( pWork->p_tcbl_fesinfo );
  pWork->p_tcbl_fesinfo = NULL;
}

static void tcb_FesInfoRingAnm( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_FESINFO_RINGANM* twk = (TASKWK_FESINFO_RINGANM*)tcb_wk;
  C_GEAR_WORK* pWork = twk->cgw;
  
  if( SleepMode_IsSleepView( pWork ) ) return;  //スリープ中/スリープ表示状態遷移中は停止

  twk->evy += twk->dif;
  ring2_FesInfoAnmCore( pWork, twk->evy );
  
  if( twk->evy == 0 || twk->evy == twk->end_evy ){
    twk->dif *= -1; //方向逆
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
 * @brief   フレーム色変更
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _gearFrameColorChange(C_GEAR_WORK* pWork, u8 col_no )
{
  u16*  pal = pWork->palFrame[ col_no ];

  PaletteWorkSet( pWork->pfade_ptr, pal,
    FADE_SUB_BG, FRAME_COLOR_TRANS_POS_BG_PALNO, FRAME_COLOR_SET_NUM_ALL*2 );
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
  case RING_2: //内から3番目
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START1, siz );
    PaletteWorkSet( pWork->pfade_ptr, &pal[RING_COLOR_SET_OBJ_D],
      FADE_SUB_OBJ, RING_COLOR_TRANS_POS_OBJ_R2_START2, siz );
    break;
  case RING_4: //一番外側
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
      ring_id*RING_COLOR_SET_NUM+(RING_COLOR_TRANS_POS_BG_START+(1-pWork->power_flag)*16), RING_COLOR_SET_NUM, evy, color );
  
  //パネルリングカラー転送
  for( i = 0;i < CGEAR_PANEL_MAX;i++) {
    if( ring_id != _gearPanelRingIDGet( pWork, i ) ){ //属しているリングが異なればスルー
      continue;
    } 
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_PANEL_START+(ring_id*16), RING_COLOR_SET_NUM, evy, color );
  }

  //関連づいたOBJのリング色を転送
  switch( ring_id ){
  case RING_2: //内から3番目
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R2_START1, RING_COLOR_SET_NUM, evy, color );
    SoftFadePfd( pWork->pfade_ptr, FADE_SUB_OBJ,
      RING_COLOR_TRANS_POS_OBJ_R2_START2, RING_COLOR_SET_NUM, evy, color );
    break;
  case RING_4: //一番外側
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
  if( pWork->power_flag ){
    for( i = 0;i < CGEAR_PANEL_MAX;i++) {
      if( ring_id == _gearPanelRingIDGet( pWork, i ) ){ //属しているリングが異なればスルー
        _gearObjSetPanelVisible( pWork, i, visible_f );
      } 
    }
  }

  //関連づいたOBJの表示制御
  switch( ring_id ){
  case RING_0: //内から1番目
    if( pWork->power_flag ){
      GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_CROSS], visible_f );
      if( pWork->connect_flag ){ //一人も通信バッファにいない時は、CONNECT表示は強制Off
        GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_CONNECT], visible_f );
      }
    }
    break;
  case RING_2: //内から3番目
    GFL_CLACT_WK_SetDrawEnable( pWork->cellCursor[ACT_LOGO], visible_f );
    break;
  case RING_4: //一番外側
    for( i = ACT_R4_OBJ_START; i <= ACT_R4_OBJ_END; i++){
      if( i == ACT_LOGO ) continue; //ACT_LOGOのみr3 ちょっと仲間はずれ
      if( !pWork->power_flag && i>ACT_COMMON_MAX ){
        continue;
      }
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

  for( i = 0; i < CGEAR_CUSTOM_COLOR_NUM;i++){
    pWork->custom_col[i] = CGEAR_SV_GetCGearCustomColor( pWork->pCGSV, i );
    if( i < CGEAR_CUSTOM_COLOR_FRAME ){
      _gearRingColorChangeCore( pWork, i, pWork->custom_col[i] );
    }else{
      _gearFrameColorChange( pWork, pWork->custom_col[i] );
    }
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

  _gearRingColorChangeInit( pWork );  //初期色を転送した上で一度黒くしておく
  for( i = 0;i < RING_NUM;i++ ){
    _gearRingColorPalAnmCore( pWork, i, 16, _BLACK_COLOR[pWork->sex] );
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
  _gearRingColorChangePanel( pWork, id, pWork->custom_col[ pd->ring_id ] );
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
    for( i = 0;i < CGEAR_CUSTOM_COLOR_NUM; i++ ){
      pWork->custom_col[i] = CGEAR_SV_GetCGearCustomColor( pWork->pCGSV, i );
    }
  } 

  //セル系システムの作成
  pWork->cellUnit = GFL_CLACT_UNIT_Create( 64+ACT_MAX, 0 , pWork->heapID );

  // ユーザーレンダラー設定
  pWork->userRend = GFL_CLACT_USERREND_Create( sc_REND_SURFACE_INIT, CGEAR_REND_MAX, pWork->heapID );
  GFL_CLACT_UNIT_SetUserRend( pWork->cellUnit, pWork->userRend );
  GFL_CLACT_UNIT_SetAutoAnmOptimizeFlag( pWork->cellUnit, TRUE );
  
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

}


static void _workEnd(C_GEAR_WORK* pWork)
{
  _arcGearRelease(pWork);

  GFL_CLACT_USERREND_Delete( pWork->userRend );
  GFL_CLACT_UNIT_Delete( pWork->cellUnit );
  
  GFL_BG_SetVisible( GEAR_BASE_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_RING_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_DECAL_FRAME, VISIBLE_OFF );
  GFL_BG_SetVisible( GEAR_MAIN_FRAME, VISIBLE_OFF );

  GFL_BG_FreeBGControl(GEAR_DECAL_FRAME);
  GFL_BG_FreeBGControl(GEAR_MAIN_FRAME);
  GFL_BG_FreeBGControl(GEAR_RING_FRAME);
  GFL_BG_FreeBGControl(GEAR_BASE_FRAME);
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
    int anm = (time.hour / 12)+ACTANM_AM;
    if( GFL_CLACT_WK_GetAnmSeq( pWork->cellCursor[ ACT_AMPM ] ) != anm ){
      _gearObjSetAnmSeq( pWork->cellCursor[ ACT_AMPM ], anm );
    }
  }
  {  //時10
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_HOUR1 ];
    int num = (time.hour % 12) / 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }
  {  //時1
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_T_HOUR2 ];
    int num = (time.hour % 12) % 10;

    if(GFL_CLACT_WK_GetAnmIndex(cp_wk) !=  num){
      GFL_CLACT_WK_SetAnmIndex(cp_wk,num);
    }
  }

  {  //コロン
    GFL_CLWK* cp_wk = pWork->cellCursor[ ACT_AMPM ];
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
}

//------------------------------------------------------------------------------
/**
 * @brief   スリープ復帰　タイムウエイト
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _modeSelectMenuWait1(C_GEAR_WORK* pWork)
{
  pWork->startCounter++;
  if( pWork->startCounter >= _SLEEP_START_TIME_WAIT ){
    pWork->startCounter = 0;
    _CHANGE_STATE(pWork, _modeSelectMenuWait2);
  }
}




static BOOL _loadExData(C_GEAR_WORK* pWork,GAMESYS_WORK* pGameSys)
{
  BOOL ret=FALSE;

#ifdef PM_DEBUG
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_START ){
    return FALSE;
  }
#endif
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
        if(crc == crc2){
          if(CGEAR_PICTURE_SAVE_IsPalette(pPic)){
            ret = TRUE;
            GFL_BG_LoadCharacter(GEAR_DECAL_FRAME,pCGearWork,CGEAR_SW_DECAL_SIZE_MAX, 0 );
            GFL_BG_LoadPalette(GEAR_DECAL_FRAME,pPic->palette,CGEAR_PICTURTE_PAL_SIZE, 32*DECAL_COLOR_TRANS_POS_PALNO );

            GFL_BG_LoadScreenBuffer(GEAR_DECAL_FRAME,pPic->scr,CGEAR_PICTURTE_SCR_SIZE );
            //スクリーンのパレットナンバーを書き換え
            GFL_BG_ChangeScreenPalette( GEAR_DECAL_FRAME, 0, 0, 32, 24, DECAL_COLOR_TRANS_POS_PALNO );
            GFL_BG_LoadScreenReq(GEAR_DECAL_FRAME);
            // アルファ値設定
            _setUpSubAlpha( pWork );
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

  //スリープ復帰時の初期描画状態を作っておく
  _gearStartUpAnimeStart(pWork);

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
  
  //インフォメーション通知初期化
  //_cgear_CheckBeaconInfo( pWork );

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
  
  _taskDel_FesInfoRingAnm( pWork );
  _BUTTONPAL_Exit( &pWork->button_palfade );
  
  GFL_UI_SleepGoSetFunc(NULL,  NULL);
  GFL_UI_SleepReleaseSetFunc(NULL,  NULL);

  GFL_NET_ResetIconForcePosition();
  GFL_NET_ReloadIconTopOrBottom( FALSE, pWork->heapID );

  GFL_TCB_DeleteTask( pWork->vblank_tcb );
  
  // タスク
  GFL_TCB_Exit( pWork->pfade_tcbsys );
  GFL_HEAP_FreeMemory( pWork->pfade_tcbsys_wk );
  GFL_TCBL_DeleteAll( pWork->tcbsys );
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
    _cgear_CheckMissionPartner( pWork, intrude_status );
    
    // トランシーバー反応処理
    _cgear_CheckTVTStatus( pWork, intrude_status );

    //ビーコンからの通知チェック
    _cgear_CheckBeaconInfo( pWork );

#ifdef PM_DEBUG
    _debSetAlpha( pWork );
#endif
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
    // アニメーション開始(ベースとフォントのパーツが対)
    _gearObjSetAnmSeq( pWork->cellCursor[ pWork->select_cursor ], pWork->select_cursor_anm );
//    _gearObjSetAnmSeq( pWork->cellCursor[ pWork->select_cursor+1 ], pWork->select_cursor_anm+1 );
    //Xメニューから復帰時のスリープブラック_PFadeSetSleepBlack()時対策でオートアニメをセットしなおす
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ pWork->select_cursor ], TRUE );
    GFL_CLACT_WK_SetAutoAnmFlag( pWork->cellCursor[ pWork->select_cursor + 1 ], TRUE );
    pWork->eff_seq ++;
    break;

  case 1:
    //対のパーツは同じアニメ設定なので、アニメ終了は最初の一個を待てばいいことにする
    if( GFL_CLACT_WK_CheckAnmActive( pWork->cellCursor[ pWork->select_cursor ] ) == FALSE ){
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
 *	@brief  スリープモードチェック
 *
 *	@param	cpWork 
 *
 *	スリープ表示状態遷移中もTRUEと返す
 */
//-----------------------------------------------------------------------------
static BOOL SleepMode_IsSleepView( const C_GEAR_WORK* cpWork )
{
  if( SleepMode_IsSleep( cpWork ) ) return TRUE;
  return cpWork->sleep_color_req;
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

  //2フレームごとに明暗を切り替え、3ターン点滅させる
  if( p_fwk->frame >= 12 ){
    return TRUE;
  }
  frame_index = (p_fwk->frame++/2)%2;

  // パレットカラーを調整
  ColorConceChangePfd( p_fwk->p_fade_ptr, FADE_SUB_OBJ, p_fwk->msk, frame_index*9, 0x0000 );
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
    pWork->select_cursor_anm = ACTANM_S_RADAR_ANM; 
    break;

  case FIELD_SUBSCREEN_ACTION_CGEAR_HELP:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_ACT; //OBJアニメ 
    pWork->select_cursor = ACT_HELP;  //OBJアニメ
    pWork->select_cursor_anm = ACTANM_HELP_ANM; 
    break;
  case FIELD_SUBSCREEN_ACTION_CGEAR_POWER:
    pWork->select_cursor_anm_type = SELECT_CURSOR_ANM_ACT; //OBJアニメ 
    pWork->select_cursor = ACT_POWER; //OBJアニメ
    pWork->select_cursor_anm = ACTANM_POWER_ANM; 
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
    GAMEBEACON_ResetCGearInfoFlag( GAMEBEACON_CGEAR_INFO_HOT );

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

//------------------------------------------------------------------------------
/**
 * @brief   ミッション乱入のお誘いチェック 
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _cgear_CheckMissionPartner( C_GEAR_WORK* pWork, INTRUDE_CONNECT intrude_status )
{
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
}

//------------------------------------------------------------------------------
/**
 * @brief   トランシーバーチェック 
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _cgear_CheckTVTStatus( C_GEAR_WORK* pWork, INTRUDE_CONNECT intrude_status )
{
  u32 bit;
  FIELD_SOUND* fsnd;

  if( intrude_status != INTRUDE_CONNECT_NULL ){ 
    return;
  }
      
  bit = WIH_DWC_GetAllBeaconTypeBit(GAMEDATA_GetWiFiList(GAMESYSTEM_GetGameData(pWork->pGameSys)));
  fsnd = GAMEDATA_GetFieldSound( GAMESYSTEM_GetGameData(pWork->pGameSys) );

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

//------------------------------------------------------------------------------
/**
 * @brief   ビーコンからの通知チェック 
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _cgear_CheckBeaconInfo( C_GEAR_WORK* pWork )
{
  int i;

  if( !_checkStateIsNormal( pWork ) ){
    return; //通常ステートでない場合は更新しない
  }
#ifdef PM_DEBUG
  {
    int cont = GFL_UI_KEY_GetCont();
    int trg = GFL_UI_KEY_GetTrg();

    if( trg & PAD_BUTTON_A ){
      if( cont & PAD_BUTTON_START ){
        GAMEBEACON_SetCGearInfoFlag( INFO_FLAG_RADAR );
      }
      if( cont & PAD_BUTTON_SELECT ){
        GAMEBEACON_ResetCGearInfoFlag( INFO_FLAG_RADAR );
      }
    }
    if( trg & PAD_BUTTON_B ){
      if( cont & PAD_BUTTON_START ){
        GAMEBEACON_SetCGearInfoFlag( INFO_FLAG_HOT );
      }
      if( cont & PAD_BUTTON_SELECT ){
        GAMEBEACON_ResetCGearInfoFlag( INFO_FLAG_HOT );
      }
    }
    if( trg & PAD_BUTTON_Y ){
      if( cont & PAD_BUTTON_START ){
        GAMEBEACON_SetCGearInfoFlag( INFO_FLAG_FMISSION );
      }
      if( cont & PAD_BUTTON_SELECT ){
        GAMEBEACON_ResetCGearInfoFlag( INFO_FLAG_FMISSION );
      }
    }
  }
#endif  //PM_DEBUG

  //調査レーダー
  if( GAMEBEACON_GetCGearInfoFlag( INFO_FLAG_RADAR ) ){
    if( !pWork->info_flag[INFO_FLAG_RADAR] ){
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR_INFO );
      pWork->info_flag[INFO_FLAG_RADAR] = TRUE;
    }
  }else{
    if( pWork->info_flag[INFO_FLAG_RADAR] ){
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_S_RADAR], ACTANM_S_RADAR );
      pWork->info_flag[INFO_FLAG_RADAR] = FALSE;
    }
  }
  
  //フェスミッション
  if( GAMEBEACON_GetCGearInfoFlag( INFO_FLAG_FMISSION ) ){
    if( !pWork->info_flag[INFO_FLAG_FMISSION] ){
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_CONNECT], ACTANM_FMISSION );
      pWork->info_flag[INFO_FLAG_FMISSION] = TRUE;
      pWork->info_flag[INFO_FLAG_HOT] = FALSE;
      _taskAdd_FesInfoRingAnm( pWork );
    }
    return;
  }
  //フェスミッションフラグがOff
  
  //新規ビーコン＆デルパワー
  if( GAMEBEACON_GetCGearInfoFlag( INFO_FLAG_HOT ) ){
    if( !pWork->info_flag[INFO_FLAG_HOT] ){
      _gearObjSetAnmSeq( pWork->cellCursor[ACT_CONNECT], ACTANM_CONNECT_INFO );
      pWork->info_flag[INFO_FLAG_HOT] = TRUE;
      pWork->info_flag[INFO_FLAG_FMISSION] = FALSE;
      _taskDel_FesInfoRingAnm( pWork );
    }
    return;
  }
  //ホットビーコンフラグOffでまだInfo表示だったら元に戻す
  if( pWork->info_flag[INFO_FLAG_HOT] || pWork->info_flag[INFO_FLAG_FMISSION]){
    _gearObjSetAnmSeq( pWork->cellCursor[ACT_CONNECT], ACTANM_CONNECT );
    pWork->info_flag[INFO_FLAG_HOT] = FALSE;
    pWork->info_flag[INFO_FLAG_FMISSION] = FALSE;
    _taskDel_FesInfoRingAnm( pWork );
  }
}

