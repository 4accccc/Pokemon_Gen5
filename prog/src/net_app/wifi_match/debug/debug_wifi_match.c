//======================================================================
/**
 * @file    debug_wifi_match.c
 * @brief   サケ操作デバッグ
 * @author  Toru=Nagihashi    tayaさん、saitoさんのソースを参考に作成
 * @data    10/06/13
 */
//======================================================================

#ifdef PM_DEBUG

#include <gflib.h>
//#include <skb.h>
#include "system/main.h"
#include "print/printsys.h"
#include "print/str_tool.h"
#include "poke_tool/monsno_def.h"

#include "font/font.naix"
#include "arc_def.h"
#include "message.naix"
#include "debug_message.naix"
#include "msg/debug/msg_debug_wifi_match.h"

#include "../wifibattlematch_net.h"
#include "../wifibattlematch_util.h"
#include "net_app/debug_wifi_match.h"

FS_EXTERN_OVERLAY( wifibattlematch_view );
FS_EXTERN_OVERLAY(dpw_common);

#define PROC_HEAP_SIZE  (0x40000)
#define PRINT_FRAME (GFL_BG_FRAME1_M)
#define TEXT_FRAME (GFL_BG_FRAME0_S)
#define PRINT_PALIDX  (0)
#define LINE_HEIGHT (11)

#define LINE_MARGIN (1)
#define LINE_DIFFER (LINE_HEIGHT + LINE_MARGIN)
#define STRBUF_LEN  (64)
#define TOUCH_H   (8)
#define NUMINPUT_DISIT_MAX (10)
#define NUMBOX_MARGIN (3)
#define NUMBOX_CHAR_WIDTH (8)
#define SKB_PALIDX1 (1)
#define SKB_PALIDX2 (2)

#define INPUT_VALUE_MAX (0xffffffff)
#define CAPTION_NONE    (0xffffffff)

enum {
  STR_POS_X = 0,
  STR_LINE0_POS_Y  = LINE_DIFFER*0,
  STR_LINE1_POS_Y  = LINE_DIFFER*1,
  STR_LINE2_POS_Y  = LINE_DIFFER*2,
  STR_LINE3_POS_Y  = LINE_DIFFER*3,
  STR_LINE4_POS_Y  = LINE_DIFFER*4,
  STR_LINE5_POS_Y  = LINE_DIFFER*5,
  STR_LINE6_POS_Y  = LINE_DIFFER*6,
  STR_LINE7_POS_Y  = LINE_DIFFER*7,
  STR_LINE8_POS_Y  = LINE_DIFFER*8,
  STR_LINE9_POS_Y  = LINE_DIFFER*9,
  STR_LINE10_POS_Y  = LINE_DIFFER*10,
  STR_LINE11_POS_Y  = LINE_DIFFER*11,
  STR_LINE12_POS_Y  = LINE_DIFFER*12,
  STR_LINE13_POS_Y  = LINE_DIFFER*13,
  STR_LINE14_POS_Y  = LINE_DIFFER*14,
  STR_LINE15_POS_Y  = LINE_DIFFER*15,


};

typedef enum {
  SEQ_DRAW_CAPTION = 0,
  SEQ_DRAW_BOX,
  SEQ_WAIT_CTRL,
  SEQ_INPUT_STR,
  SEQ_INPUT_NUM,

}MAIN_CTRL_SEQ;

typedef enum {
  STR_TYPE_STR,      ///< 入力対応文字列（文字列から導かれるインデックスを保存する場合）
  STR_TYPE_NUM,      ///< 入力対応数値  arg =max arg2 =min
  STR_TYPE_SWITCH,   ///< タッチ対応トグルスイッチ
  STR_TYPE_EDITSTR,  ///< 入力対応文字列（文字列自体を保存する場合）
  STR_TYPE_FIXNUM,   ///< 表示するだけの数値
  STR_TYPE_BUTTON,  ///< アクションがあるもの arg=strID arg2=Func

  STR_TYPE_CAPTION,      ///< 文字列のみ
}STR_TYPE;

typedef enum {
  //ランダムマッチのデータ
  EDITBOX_ID_NONE = 0,
  EDITBOX_ID_SINGLE_WIN,
  EDITBOX_ID_SINGLE_LOSE,
  EDITBOX_ID_SINGLE_RATE,
  EDITBOX_ID_DOUBLE_WIN,
  EDITBOX_ID_DOUBLE_LOSE,
  EDITBOX_ID_DOUBLE_RATE,
  EDITBOX_ID_TRIPLE_WIN,
  EDITBOX_ID_TRIPLE_LOSE,
  EDITBOX_ID_TRIPLE_RATE,
  EDITBOX_ID_ROTATE_WIN,
  EDITBOX_ID_ROTATE_LOSE,
  EDITBOX_ID_ROTATE_RATE,
  EDITBOX_ID_SHOOTER_WIN,
  EDITBOX_ID_SHOOTER_LOSE,
  EDITBOX_ID_SHOOTER_RATE,
  EDITBOX_ID_RND_DISCONNECT,
  EDITBOX_ID_RND_CHEATS,

  //WIFIたいかいのデータ
  EDITBOX_ID_WIFI_WIN,
  EDITBOX_ID_WIFI_LOSE,
  EDITBOX_ID_WIFI_RATE,
  EDITBOX_ID_WIFI_DISCONNECT,
  EDITBOX_ID_WIFI_CHEATS,
  EDITBOX_ID_WIFI_POKE1,
  EDITBOX_ID_WIFI_POKE2,
  EDITBOX_ID_WIFI_POKE3,
  EDITBOX_ID_WIFI_POKE4,
  EDITBOX_ID_WIFI_POKE5,
  EDITBOX_ID_WIFI_POKE6,

  //プレイヤーの情報
  EDITBOX_ID_MY_RECORDID,
  EDITBOX_ID_MY_TRAINERID,
  EDITBOX_ID_MY_PID,
  EDITBOX_ID_MY_CONTRY,
  EDITBOX_ID_MY_AREA,
  EDITBOX_ID_MY_LANG,
  EDITBOX_ID_MY_VER,
  EDITBOX_ID_MY_VIEW,
  EDITBOX_ID_MY_SEX,
  EDITBOX_ID_MY_YEAR,
  EDITBOX_ID_MY_MONTH,
  EDITBOX_ID_MY_DAY,
  EDITBOX_ID_MY_HOUR,
  EDITBOX_ID_MY_SECOND,
  EDITBOX_ID_MY_MINITS,

  //せんせきのデータ(ポケモン)
  EDITBOX_ID_REC_BUTTON_L,
  EDITBOX_ID_REC_NUM,
  EDITBOX_ID_REC_BUTTON_R,
  EDITBOX_ID_REC_YOU_POKE1,
  EDITBOX_ID_REC_YOU_FORM1,
  EDITBOX_ID_REC_YOU_LV1,
  EDITBOX_ID_REC_YOU_SEX1,
  EDITBOX_ID_REC_YOU_POKE2,
  EDITBOX_ID_REC_YOU_FORM2,
  EDITBOX_ID_REC_YOU_LV2,
  EDITBOX_ID_REC_YOU_SEX2,
  EDITBOX_ID_REC_YOU_POKE3,
  EDITBOX_ID_REC_YOU_FORM3,
  EDITBOX_ID_REC_YOU_LV3,
  EDITBOX_ID_REC_YOU_SEX3,
  EDITBOX_ID_REC_YOU_POKE4,
  EDITBOX_ID_REC_YOU_FORM4,
  EDITBOX_ID_REC_YOU_LV4,
  EDITBOX_ID_REC_YOU_SEX4,
  EDITBOX_ID_REC_YOU_POKE5,
  EDITBOX_ID_REC_YOU_FORM5,
  EDITBOX_ID_REC_YOU_LV5,
  EDITBOX_ID_REC_YOU_SEX5,
  EDITBOX_ID_REC_YOU_POKE6,
  EDITBOX_ID_REC_YOU_FORM6,
  EDITBOX_ID_REC_YOU_LV6,
  EDITBOX_ID_REC_YOU_SEX6,
  EDITBOX_ID_REC_MY_POKE1,
  EDITBOX_ID_REC_MY_FORM1,
  EDITBOX_ID_REC_MY_LV1,
  EDITBOX_ID_REC_MY_SEX1,
  EDITBOX_ID_REC_MY_POKE2,
  EDITBOX_ID_REC_MY_FORM2,
  EDITBOX_ID_REC_MY_LV2,
  EDITBOX_ID_REC_MY_SEX2,
  EDITBOX_ID_REC_MY_POKE3,
  EDITBOX_ID_REC_MY_FORM3,
  EDITBOX_ID_REC_MY_LV3,
  EDITBOX_ID_REC_MY_SEX3,
  EDITBOX_ID_REC_MY_POKE4,
  EDITBOX_ID_REC_MY_FORM4,
  EDITBOX_ID_REC_MY_LV4,
  EDITBOX_ID_REC_MY_SEX4,
  EDITBOX_ID_REC_MY_POKE5,
  EDITBOX_ID_REC_MY_FORM5,
  EDITBOX_ID_REC_MY_LV5,
  EDITBOX_ID_REC_MY_SEX5,
  EDITBOX_ID_REC_MY_POKE6,
  EDITBOX_ID_REC_MY_FORM6,
  EDITBOX_ID_REC_MY_LV6,
  EDITBOX_ID_REC_MY_SEX6,

  //せんせきのデータ(たいせんじょうほう)
  EDITBOX_ID_REC_YOU_GSID,
  EDITBOX_ID_REC_YOU_PID,
  EDITBOX_ID_REC_CUPNO,
  EDITBOX_ID_REC_BTLTYPE,
  EDITBOX_ID_REC_RESULT,
  EDITBOX_ID_REC_MY_RESTPOKE,
  EDITBOX_ID_REC_YOU_RESTPOKE,
  EDITBOX_ID_BTL_YEAR,
  EDITBOX_ID_BTL_MONTH,
  EDITBOX_ID_BTL_DAY,
  EDITBOX_ID_BTL_HOUR,
  EDITBOX_ID_BTL_SECOND,
  EDITBOX_ID_BTL_MINITS,
  EDITBOX_ID_REC_NEXT,


  EDITBOX_ID_MAX,
  //文字列の場合は以下を使う

  EDITBOX_ID_MY_NAME = EDITBOX_ID_MAX,

  EDITBOX_ID_ALL_MAX,
  EDITBOX_ID_STR_MAX  = EDITBOX_ID_ALL_MAX - EDITBOX_ID_MAX,

}EDITBOX_ID;

enum {
  COLIDX_BLACK=1,
  COLIDX_GRAY,
  COLIDX_RED_D,
  COLIDX_RED_L,
  COLIDX_BLUE_D,
  COLIDX_BLUE_L,
  COLIDX_YELLOW_D,
  COLIDX_YELLOW_L,
  COLIDX_GREEN_D,
  COLIDX_GREEN_L,
  COLIDX_ORANGE_D,
  COLIDX_ORANGE_L,
  COLIDX_PINK_D,
  COLIDX_PINK_L,
  COLIDX_WHITE,
};

typedef struct {
  STR_TYPE  Type;
  int  StrID;
  u8   PosX;
  u8   PosY;

  u8   TouchX;
  u8   TouchY;
  u8   TouchW;
  u8   TouchH;

//  u16  paraID;
  u32  arg;
  u32  arg2;
  u32   editBoxID;

}STR_PARAM;

//--------------------------------------------------------------
/**
 *  数値入力ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_BMPWIN*   Win;
  GFL_BMP_DATA* Bmp;
  GFL_FONT*     Font;
  STRBUF*       Buf;
  PRINT_UTIL*   PrintUtil;
  PRINT_QUE*    PrintQue;
  const STR_PARAM* Param;

  u32  Num;
  u32  Disit;
  u32  NumMax;
  u32  NumMin;
  u8   Draw_ox;
  u8   ColorDefBg;
  u8   ColorCurBg;
  u8   ColorLetter;

  s8   NumAry[ NUMINPUT_DISIT_MAX ];
  u8   CurDisit;
  u32  DispNumWork;
}NUMINPUT_WORK;

//--------------------------------------------------------------
/**
 *  入力補完ワーク
 */
//--------------------------------------------------------------
typedef struct {
  GFL_SKB*      skb;
  GFL_MSGDATA*  msg;
  STRBUF*       buf;
  STRBUF*       subword;
  STRBUF*       fullword;
  int           index;
  int           search_first_index;
  int           searchingFlag;
}COMP_SKB_WORK;

//-------------------------------------
///	通信フロー
//=====================================
typedef struct _NETFLOW_WORK NETFLOW_WORK;
typedef BOOL (*NetFlow_StateFunction)( NETFLOW_WORK *p_wk, u32 *p_seq );
struct _NETFLOW_WORK
{
  GFL_FONT        *p_font;
  GFL_MSGDATA     *p_msg;
  WBM_TEXT_WORK   *p_text;  //上画面テキスト
  WIFIBATTLEMATCH_NET_WORK *p_net;  //ランダムマッチネットシステム
  WIFIBATTLEMATCH_NET_DATA  net_data;
  WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA sake_data;
  NetFlow_StateFunction   stateFunction;
  u32 seq;
} ;

//-------------------------------------
///	メインワーク
//=====================================
typedef struct {

  GFL_BMPWIN*     Win;
  GFL_BMP_DATA*   Bmp;
  GFL_FONT*       Font;
  GFL_MSGDATA*    MsgData;
  STRBUF*         StrBuf;
  PRINT_QUE*      PrintQue;
  PRINT_UTIL      PrintUtil;
  GFL_SKB*        skb;

  int             BoxIdx;
  u32             BoxValue[ EDITBOX_ID_MAX ];
  s32             page;

  HEAPID  HeapID;
  MAIN_CTRL_SEQ      Seq;

  COMP_SKB_WORK   comp;
  GFL_SKB_SETUP   skbSetup;
  NUMINPUT_WORK   NumInput;
  
  STRBUF*         BoxStrBuf[EDITBOX_ID_STR_MAX];

  BOOL            touch_prev_flag;
  
  NETFLOW_WORK    net_flow;

  BOOL            is_data_update;
}MAKE_WORK;

//STR_TYPE_BUTTONのarg2に指定するコールバック
typedef void (*ButtonAction_Callback)( MAKE_WORK *wk, const STR_PARAM *StrParam );
static void ButtonAction_LeftButton( MAKE_WORK *wk, const STR_PARAM *StrParam );
static void ButtonAction_RightButton( MAKE_WORK *wk, const STR_PARAM *StrParam );

//-------------------------------------
///	データ
//=====================================
static const STR_PARAM StrParamsRandomMatch[] = {
  //ランダムマッチ
  {STR_TYPE_CAPTION, SAKE_DATA_45,  STR_POS_X, STR_LINE0_POS_Y,
    0,0,0,0,
    0,0,0 },
  //シングル
  {STR_TYPE_CAPTION, SAKE_DATA_06,  STR_POS_X, STR_LINE1_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE2_POS_Y,
    3*8,STR_LINE2_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_SINGLE_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE2_POS_Y,
    12*8,STR_LINE2_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_SINGLE_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE2_POS_Y,
    21*8,STR_LINE2_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_SINGLE_RATE },
  //ダブル
  {STR_TYPE_CAPTION, SAKE_DATA_07,  STR_POS_X, STR_LINE3_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE4_POS_Y,
    3*8,STR_LINE4_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_DOUBLE_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE4_POS_Y,
    12*8,STR_LINE4_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_DOUBLE_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE4_POS_Y,
    21*8,STR_LINE4_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_DOUBLE_RATE },
  //トリプル
  {STR_TYPE_CAPTION, SAKE_DATA_08,  STR_POS_X, STR_LINE5_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE6_POS_Y,
    3*8,STR_LINE6_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_TRIPLE_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE6_POS_Y,
    12*8,STR_LINE6_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_TRIPLE_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE6_POS_Y,
    21*8,STR_LINE6_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_TRIPLE_RATE },
  //ローテーション
  {STR_TYPE_CAPTION, SAKE_DATA_47,  STR_POS_X, STR_LINE7_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE8_POS_Y,
    3*8,STR_LINE8_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_ROTATE_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE8_POS_Y,
    12*8,STR_LINE8_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_ROTATE_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE8_POS_Y,
    21*8,STR_LINE8_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_ROTATE_RATE },
  //シューター
  {STR_TYPE_CAPTION, SAKE_DATA_09,  STR_POS_X, STR_LINE9_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE10_POS_Y,
    3*8,STR_LINE10_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_SHOOTER_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE10_POS_Y,
    12*8,STR_LINE10_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_SHOOTER_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE10_POS_Y,
    21*8,STR_LINE10_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_SHOOTER_RATE },

  //せつだんかいすう
  {STR_TYPE_NUM, SAKE_DATA_02, STR_POS_X, STR_LINE11_POS_Y,
    10*8,STR_LINE11_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_RND_DISCONNECT },
  //ふせいかいすう
  {STR_TYPE_NUM, SAKE_DATA_10, STR_POS_X, STR_LINE12_POS_Y,
    10*8,STR_LINE12_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_RND_CHEATS },
};

static const STR_PARAM StrParamsWifMatch[] = {
  //WIFIたいかい
  {STR_TYPE_CAPTION, SAKE_DATA_46,  STR_POS_X, STR_LINE0_POS_Y,
    0,0,0,0,
    0,0,0 },
  //かち
  {STR_TYPE_NUM, SAKE_DATA_04,  STR_POS_X, STR_LINE1_POS_Y,
    3*8,STR_LINE1_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_WIFI_WIN },
  //まけ
  {STR_TYPE_NUM, SAKE_DATA_05,  9*8, STR_LINE1_POS_Y,
    12*8,STR_LINE1_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_WIFI_LOSE },
  //レート
  {STR_TYPE_NUM, SAKE_DATA_03,  18*8, STR_LINE1_POS_Y,
    21*8,STR_LINE1_POS_Y,5*8,TOUCH_H,
    9999,0,EDITBOX_ID_WIFI_RATE },
  //せつだんかいすう
  {STR_TYPE_NUM, SAKE_DATA_02, STR_POS_X, STR_LINE2_POS_Y,
    10*8,STR_LINE2_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_WIFI_DISCONNECT },
  //ふせいかいすう
  {STR_TYPE_NUM, SAKE_DATA_10, STR_POS_X, STR_LINE3_POS_Y,
    10*8,STR_LINE3_POS_Y,6*8,TOUCH_H,
    65535,0,EDITBOX_ID_WIFI_CHEATS },
  //とうろくポケモン
  {STR_TYPE_CAPTION, SAKE_DATA_48,  STR_POS_X, STR_LINE4_POS_Y,
    0,0,0,0,
    0,0,0 },
  //ポケモン１番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE5_POS_Y,
    1*8,STR_LINE5_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE1 },
  //ポケモン２番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE6_POS_Y,
    1*8,STR_LINE6_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE2 },
  //ポケモン３番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE7_POS_Y,
    1*8,STR_LINE7_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE3 },
  //ポケモン４番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE8_POS_Y,
    1*8,STR_LINE8_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE4 },
  //ポケモン５番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE9_POS_Y,
    1*8,STR_LINE9_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE5 },
  //ポケモン６番目
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE10_POS_Y,
    1*8,STR_LINE10_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_WIFI_POKE6 },
};

//せんせき１
static const STR_PARAM StrParamsRecord1[] = {
  //せんせきのりれき
  {STR_TYPE_CAPTION, SAKE_DATA_26,  STR_POS_X, STR_LINE0_POS_Y,
    0,0,0,0,
    0,0,0 },
  //←ボタン
  {STR_TYPE_BUTTON, CAPTION_NONE,  STR_POS_X, STR_LINE1_POS_Y,
    8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    SAKE_DATA_52,(u32)ButtonAction_LeftButton,EDITBOX_ID_REC_BUTTON_L },
  //りれきばんごう
  {STR_TYPE_FIXNUM, CAPTION_NONE,  0, STR_LINE1_POS_Y,
    4*8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    6,0,EDITBOX_ID_REC_NUM },
  //→ボタン
  {STR_TYPE_BUTTON, CAPTION_NONE,  0, STR_LINE1_POS_Y,
    7*8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    SAKE_DATA_53,(u32)ButtonAction_RightButton,EDITBOX_ID_REC_BUTTON_R },
  //たいせんあいてのポケモン
  {STR_TYPE_CAPTION, SAKE_DATA_34,  STR_POS_X, STR_LINE2_POS_Y,
    0,0,0,0,
    0,0,0 },
  //ポケモン１のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE3_POS_Y,
    1*8,STR_LINE3_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE1 },
  //ポケモン１のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE3_POS_Y,
    13*8,STR_LINE3_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM1 },
  //ポケモン１のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE3_POS_Y,
    20*8,STR_LINE3_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV1 },
  //ポケモン１の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE3_POS_Y,
    25*8,STR_LINE3_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX1 },
  //ポケモン２のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE4_POS_Y,
    1*8,STR_LINE4_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE2 },
  //ポケモン２のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE4_POS_Y,
    13*8,STR_LINE4_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM2 },
  //ポケモン２のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE4_POS_Y,
    20*8,STR_LINE4_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV2 },
  //ポケモン２の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE4_POS_Y,
    25*8,STR_LINE4_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX2 },
  //ポケモン３のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE5_POS_Y,
    1*8,STR_LINE5_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE3 },
  //ポケモン３のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE5_POS_Y,
    13*8,STR_LINE5_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM3 },
  //ポケモン３のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE5_POS_Y,
    20*8,STR_LINE5_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV3 },
  //ポケモン３の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE5_POS_Y,
    25*8,STR_LINE5_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX3 },
  //ポケモン４のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE6_POS_Y,
    1*8,STR_LINE6_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE4 },
  //ポケモン４のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE6_POS_Y,
    13*8,STR_LINE6_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM4 },
  //ポケモン４のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE6_POS_Y,
    20*8,STR_LINE6_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV4 },
  //ポケモン４の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE6_POS_Y,
    25*8,STR_LINE6_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX4 },
  //ポケモン５のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE7_POS_Y,
    1*8,STR_LINE7_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE5 },
  //ポケモン５のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE7_POS_Y,
    13*8,STR_LINE7_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM5 },
  //ポケモン５のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE7_POS_Y,
    20*8,STR_LINE7_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV5 },
  //ポケモン５の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE7_POS_Y,
    25*8,STR_LINE7_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX5 },
  //ポケモン６のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE8_POS_Y,
    1*8,STR_LINE8_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_YOU_POKE6 },
  //ポケモン６のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE8_POS_Y,
    13*8,STR_LINE8_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_YOU_FORM6 },
  //ポケモン６のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE8_POS_Y,
    20*8,STR_LINE8_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_YOU_LV6 },
  //ポケモン６の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE8_POS_Y,
    25*8,STR_LINE8_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_YOU_SEX6 },
  //じぶんのポケモン
  {STR_TYPE_CAPTION, SAKE_DATA_35,  STR_POS_X, STR_LINE9_POS_Y,
    0,0,0,0,
    0,0,0 },
  //ポケモン１のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE10_POS_Y,
    1*8,STR_LINE10_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE1 },
  //ポケモン１のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE10_POS_Y,
    13*8,STR_LINE10_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM1 },
  //ポケモン１のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE10_POS_Y,
    20*8,STR_LINE10_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV1 },
  //ポケモン１の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE10_POS_Y,
    25*8,STR_LINE10_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX1 },
  //ポケモン２のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE11_POS_Y,
    1*8,STR_LINE11_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE2 },
  //ポケモン２のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE11_POS_Y,
    13*8,STR_LINE11_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM2 },
  //ポケモン２のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE11_POS_Y,
    20*8,STR_LINE11_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV2 },
  //ポケモン２の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE11_POS_Y,
    25*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX2 },
  //ポケモン３のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE12_POS_Y,
    1*8,STR_LINE12_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE3 },
  //ポケモン３のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE12_POS_Y,
    13*8,STR_LINE12_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM3 },
  //ポケモン３のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE12_POS_Y,
    20*8,STR_LINE12_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV3 },
  //ポケモン３の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE12_POS_Y,
    25*8,STR_LINE12_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX3 },
  //ポケモン４のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE13_POS_Y,
    1*8,STR_LINE13_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE4 },
  //ポケモン４のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE13_POS_Y,
    13*8,STR_LINE13_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM4 },
  //ポケモン４のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE13_POS_Y,
    20*8,STR_LINE13_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV4 },
  //ポケモン４の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE13_POS_Y,
    25*8,STR_LINE13_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX4 },
  //ポケモン５のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE14_POS_Y,
    1*8,STR_LINE14_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE5 },
  //ポケモン５のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE14_POS_Y,
    13*8,STR_LINE14_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM5 },
  //ポケモン５のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE14_POS_Y,
    20*8,STR_LINE14_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV5 },
  //ポケモン５の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE14_POS_Y,
    25*8,STR_LINE14_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX5 },
  //ポケモン６のポケモンNO
  {STR_TYPE_STR, CAPTION_NONE, STR_POS_X, STR_LINE15_POS_Y,
    1*8,STR_LINE15_POS_Y,8*8,TOUCH_H,
    NARC_message_monsname_dat,0,EDITBOX_ID_REC_MY_POKE6 },
  //ポケモン６のフォルムNO
  {STR_TYPE_NUM, SAKE_DATA_31, 10*8, STR_LINE15_POS_Y,
    13*8,STR_LINE15_POS_Y,4*8,TOUCH_H,
    255,0,EDITBOX_ID_REC_MY_FORM6 },
  //ポケモン６のレベル
  {STR_TYPE_NUM, SAKE_DATA_32,  18*8, STR_LINE15_POS_Y,
    20*8,STR_LINE15_POS_Y,4*8,TOUCH_H,
    100,0,EDITBOX_ID_REC_MY_LV6 },
  //ポケモン６の性別
  {STR_TYPE_SWITCH, CAPTION_NONE,  STR_POS_X, STR_LINE15_POS_Y,
    25*8,STR_LINE15_POS_Y,3*8,TOUCH_H,
    SEX_00,3,EDITBOX_ID_REC_MY_SEX6 },
};

//せんせき２
static const STR_PARAM StrParamsRecord2[] = {
  //せんせきのりれき
  {STR_TYPE_CAPTION, SAKE_DATA_54,  STR_POS_X, STR_LINE0_POS_Y,
    0,0,0,0,
    0,0,0 },
  //←ボタン
  {STR_TYPE_BUTTON, CAPTION_NONE,  STR_POS_X, STR_LINE1_POS_Y,
    8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    SAKE_DATA_52,(u32)ButtonAction_LeftButton,EDITBOX_ID_REC_BUTTON_L },
  //りれきばんごう
  {STR_TYPE_FIXNUM, CAPTION_NONE,  0, STR_LINE1_POS_Y,
    4*8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    6,0,EDITBOX_ID_REC_NUM },
  //→ボタン
  {STR_TYPE_BUTTON, CAPTION_NONE,  0, STR_LINE1_POS_Y,
    7*8,STR_LINE1_POS_Y,2*8,TOUCH_H,
    SAKE_DATA_53,(u32)ButtonAction_RightButton,EDITBOX_ID_REC_BUTTON_R },
  //あいてのじょうほう
  {STR_TYPE_CAPTION, SAKE_DATA_27,  STR_POS_X, STR_LINE2_POS_Y,
    0,0,0,0,
    0,0,0 },
  //ゲームシンクID
  {STR_TYPE_NUM, SAKE_DATA_28,  STR_POS_X, STR_LINE3_POS_Y,
   9*8,STR_LINE3_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_REC_YOU_GSID },
  //プロファイルID
  {STR_TYPE_NUM, SAKE_DATA_29,  STR_POS_X, STR_LINE4_POS_Y,
   9*8,STR_LINE4_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_REC_YOU_PID },
  //たいせんじょうほう
  {STR_TYPE_CAPTION, SAKE_DATA_55,  STR_POS_X, STR_LINE5_POS_Y,
    0,0,0,0,
    0,0,0 },
  //たいかいNO
  {STR_TYPE_NUM, SAKE_DATA_39,  STR_POS_X, STR_LINE6_POS_Y,
   8*8,STR_LINE6_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_REC_CUPNO },
  //たいかいタイプ
  {STR_TYPE_SWITCH, SAKE_DATA_41,  STR_POS_X, STR_LINE7_POS_Y,
    8*8,STR_LINE7_POS_Y,9*8,TOUCH_H,
    BTLTYPE_00,6,EDITBOX_ID_REC_BTLTYPE },
  //けっか
  {STR_TYPE_SWITCH, SAKE_DATA_40,  STR_POS_X, STR_LINE8_POS_Y,
    8*8,STR_LINE8_POS_Y,9*8,TOUCH_H,
    BTLRESULT_00,7,EDITBOX_ID_REC_RESULT },
  //じぶんののこりポケ
  {STR_TYPE_NUM, SAKE_DATA_43,  STR_POS_X, STR_LINE9_POS_Y,
    9*8,STR_LINE9_POS_Y,3*8,TOUCH_H,
    6,0,EDITBOX_ID_REC_MY_RESTPOKE },
  //あいてののこりポケ
  {STR_TYPE_NUM, SAKE_DATA_42,  13*8, STR_LINE9_POS_Y,
    22*8,STR_LINE9_POS_Y,3*8,TOUCH_H,
    6,0,EDITBOX_ID_REC_YOU_RESTPOKE },
  //たいせんじかん
  {STR_TYPE_CAPTION, SAKE_DATA_36,  STR_POS_X, STR_LINE10_POS_Y,
    0,0,0,0,
    0,0,0 },
  //年
  {STR_TYPE_NUM, SAKE_DATA_12,  STR_POS_X, STR_LINE11_POS_Y,
    2*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    99,0,EDITBOX_ID_BTL_YEAR },
  //月
  {STR_TYPE_NUM, SAKE_DATA_13,  6*8, STR_LINE11_POS_Y,
    8*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    12,0,EDITBOX_ID_BTL_MONTH },
  //日
  {STR_TYPE_NUM, SAKE_DATA_14,  12*8, STR_LINE11_POS_Y,
    14*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    31,0,EDITBOX_ID_BTL_DAY },
  //時
  {STR_TYPE_NUM, SAKE_DATA_37,  STR_POS_X, STR_LINE12_POS_Y,
    2*8,STR_LINE12_POS_Y,3*8,TOUCH_H,
    24,0,EDITBOX_ID_BTL_HOUR },
  //分
  {STR_TYPE_NUM, SAKE_DATA_38,  6*8, STR_LINE12_POS_Y,
    8*8,STR_LINE12_POS_Y,3*8,TOUCH_H,
    59,0,EDITBOX_ID_BTL_MINITS },
  //秒
  {STR_TYPE_NUM, SAKE_DATA_49,  12*8, STR_LINE12_POS_Y,
    14*8,STR_LINE12_POS_Y,3*8,TOUCH_H,
    59,0,EDITBOX_ID_BTL_SECOND },

  //つぎのかきこみ
  {STR_TYPE_NUM, SAKE_DATA_51,  STR_POS_X, STR_LINE14_POS_Y,
    8*8,STR_LINE14_POS_Y,3*8,TOUCH_H,
    5,0,EDITBOX_ID_REC_NEXT },
};

//じぶんのじょうほう
static const STR_PARAM StrParamsMyStatus[] = {
  //じぶんの情報
  {STR_TYPE_CAPTION, SAKE_DATA_16,  STR_POS_X, STR_LINE0_POS_Y,
    0,0,0,0,
    0,0,0 },
  //なまえ
  {STR_TYPE_EDITSTR, SAKE_DATA_17,  STR_POS_X, STR_LINE1_POS_Y,
    4*8,STR_LINE1_POS_Y,8*8,TOUCH_H,
    0,0,EDITBOX_ID_MY_NAME },
  //レコードID
  {STR_TYPE_FIXNUM, SAKE_DATA_00,  STR_POS_X, STR_LINE2_POS_Y,
    8*8,STR_LINE2_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_MY_RECORDID },
  //トレーナーID
  {STR_TYPE_NUM, SAKE_DATA_18,  STR_POS_X, STR_LINE3_POS_Y,
    8*8,STR_LINE3_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_MY_TRAINERID },
  //PID
  {STR_TYPE_FIXNUM, SAKE_DATA_19,  STR_POS_X, STR_LINE4_POS_Y,
    8*8,STR_LINE4_POS_Y,12*8,TOUCH_H,
    0xFFFFFFFF,0,EDITBOX_ID_MY_PID },
  //国
  {STR_TYPE_STR, SAKE_DATA_20,  STR_POS_X, STR_LINE5_POS_Y,
    3*8,STR_LINE5_POS_Y,12*8,TOUCH_H,
    NARC_message_wifi_place_msg_world_dat,0,EDITBOX_ID_MY_CONTRY },
  //地域
  {STR_TYPE_NUM, SAKE_DATA_21,  15*8, STR_LINE5_POS_Y,
    19*8,STR_LINE5_POS_Y,12*8,TOUCH_H,
    255,0,EDITBOX_ID_MY_AREA },
  //言語
  {STR_TYPE_SWITCH, SAKE_DATA_22,  STR_POS_X, STR_LINE6_POS_Y,
    6*8,STR_LINE6_POS_Y,8*8,TOUCH_H,
    LANGCODE_00,7+1,EDITBOX_ID_MY_LANG },
  //ロムバージョン
  {STR_TYPE_SWITCH, SAKE_DATA_23,  15*8, STR_LINE6_POS_Y,
    23*8,STR_LINE6_POS_Y,8*8,TOUCH_H,
    CASETTE_00,13+1,EDITBOX_ID_MY_VER },
  //見た目
  {STR_TYPE_SWITCH, SAKE_DATA_24,  STR_POS_X, STR_LINE7_POS_Y,
    4*8,STR_LINE7_POS_Y,12*8,TOUCH_H,
    VIEW_00,16,EDITBOX_ID_MY_VIEW },
  //性別
  {STR_TYPE_SWITCH, SAKE_DATA_25,  18*8, STR_LINE7_POS_Y,
    23*8,STR_LINE7_POS_Y,3*8,TOUCH_H,
    SEX_00,2,EDITBOX_ID_MY_SEX },
  //最終ログイン
  {STR_TYPE_CAPTION, SAKE_DATA_15,  STR_POS_X, STR_LINE9_POS_Y,
    0,0,0,0,
    0,0,0 },
  //年
  {STR_TYPE_NUM, SAKE_DATA_12,  STR_POS_X, STR_LINE10_POS_Y,
    2*8,STR_LINE10_POS_Y,3*8,TOUCH_H,
    99,0,EDITBOX_ID_MY_YEAR },
  //月
  {STR_TYPE_NUM, SAKE_DATA_13,  6*8, STR_LINE10_POS_Y,
    8*8,STR_LINE10_POS_Y,3*8,TOUCH_H,
    12,0,EDITBOX_ID_MY_MONTH },
  //日
  {STR_TYPE_NUM, SAKE_DATA_14,  12*8, STR_LINE10_POS_Y,
    14*8,STR_LINE10_POS_Y,3*8,TOUCH_H,
    31,0,EDITBOX_ID_MY_DAY },
  //時
  {STR_TYPE_NUM, SAKE_DATA_37,  STR_POS_X, STR_LINE11_POS_Y,
    2*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    24,0,EDITBOX_ID_MY_HOUR },
  //分
  {STR_TYPE_NUM, SAKE_DATA_38,  6*8, STR_LINE11_POS_Y,
    8*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    59,0,EDITBOX_ID_MY_MINITS },
  //秒
  {STR_TYPE_NUM, SAKE_DATA_49,  12*8, STR_LINE11_POS_Y,
    14*8,STR_LINE11_POS_Y,3*8,TOUCH_H,
    59,0,EDITBOX_ID_MY_SECOND },
};

static const struct
{
  const STR_PARAM*  strParams;
  int len;
} layout[]  =
{
  //じぶんの情報
  {
    StrParamsMyStatus, NELEMS(StrParamsMyStatus)
  },
  //ランダムマッチ
  {
    StrParamsRandomMatch,NELEMS(StrParamsRandomMatch)
  },
  //WIFI大会
  {
    StrParamsWifMatch,NELEMS(StrParamsWifMatch)
  },
  //戦績１
  {
    StrParamsRecord1, NELEMS(StrParamsRecord1)
  },
  //戦績２
  {
    StrParamsRecord2, NELEMS(StrParamsRecord2)
  }
};

static const sc_version_tbl[] =
{
  VERSION_BLACK,VERSION_WHITE,
  VERSION_GOLD,VERSION_SILVER,
  VERSION_DIAMOND,VERSION_PEARL,VERSION_PLATINUM,
  VERSION_RUBY,VERSION_SAPPHIRE,VERSION_EMERALD,
  VERSION_RED,VERSION_GREEN,
  VERSION_COLOSSEUM,
};

static const sc_lang_tbl[]  =
{
  LANG_JAPAN	,
  LANG_ENGLISH,
  LANG_FRANCE	,
  LANG_ITALY	,
  LANG_GERMANY,
  LANG_SPAIN	,
  LANG_KOREA	,
};

static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static void SetupDisp( MAKE_WORK* wk );
static void CleanupDisp( MAKE_WORK* wk );
static BOOL MainCtrl( MAKE_WORK* wk );

static void PrintCaption( MAKE_WORK* wk, const STR_PARAM* p );
static int CheckBoxTouch( MAKE_WORK* wk );
static void UpdateBox( MAKE_WORK* wk, u32 inBoxID, u32 inValue );
static void UpdateStrBox( MAKE_WORK* wk, u32 inBoxID);
static PRINTSYS_LSB GetBoxSubBGColor( u8 inType );
static void GetBoxStr( MAKE_WORK* wk, u32 inBoxID, STRBUF* buf );
static u32 CalcDisit( u32 inValue );
static const STR_PARAM *GetBoxForEditBoxID( const MAKE_WORK* wk, u32 inBoxID );


static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const STR_PARAM* strParam, u32 value, const MAKE_WORK* make_wk );
static BOOL NumInput_Main( NUMINPUT_WORK* wk );
static u32 NumInput_GetNum( NUMINPUT_WORK* wk );
static void DrawNumInput( NUMINPUT_WORK* wk );
static void SetAryNumInput( NUMINPUT_WORK* wk, u32 value );
static u32 CalcInputNum( NUMINPUT_WORK* wk );


static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID );
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk );
static BOOL COMPSKB_Main( COMP_SKB_WORK* wk );
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk );
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len );
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx );
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx );
static BOOL is_hiragana( const STRBUF* buf );

static void NETFLOW_Init( NETFLOW_WORK *p_wk, GAMEDATA *p_gamedata, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, HEAPID heapID );
static void NETFLOW_Exit( NETFLOW_WORK *p_wk );
static void NETFLOW_Main( NETFLOW_WORK *p_wk );
static void NETFLOW_StartRequest( NETFLOW_WORK *p_wk, NetFlow_StateFunction state );
static BOOL NETFLOW_WaitRequest( const NETFLOW_WORK *cp_wk );
static WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *NETFLOW_GetSakeData( NETFLOW_WORK *p_wk );

static BOOL NETFLOW_STATE_Initialize( NETFLOW_WORK *p_wk, u32 *p_seq );
static BOOL NETFLOW_STATE_Send( NETFLOW_WORK *p_wk, u32 *p_seq );
static BOOL NETFLOW_STATE_Recv( NETFLOW_WORK *p_wk, u32 *p_seq );


static void StoreData( MAKE_WORK *p_wk, const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data );
static void StoreRecodeData( MAKE_WORK *p_wk, const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data, u32 idx );
static void ReStoreData( WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *p_data, const MAKE_WORK *cp_wk, u32 idx );

//--------------------------------------------------------------
/**
 *  Proc Data
 */
//--------------------------------------------------------------
const GFL_PROC_DATA DEBUG_WifiBattleMatch_ProcData = {
  PROC_MAKE_WifiBattleMatch_Init,
  PROC_MAKE_WifiBattleMatch_Main,
  PROC_MAKE_WifiBattleMatch_Quit,
};

static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Init( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  DEBUG_WIFIBATTLEMATCH_PARAM *p_param  = pwk;

  GFL_OVERLAY_Load( FS_OVERLAY_ID( wifibattlematch_view ) );
  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));

  //ヒープ作成
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_WIFIMATCH_DEBUG,   PROC_HEAP_SIZE );

  {
    MAKE_WORK* wk = GFL_PROC_AllocWork( proc, sizeof(MAKE_WORK), HEAPID_WIFIMATCH_DEBUG );
    GF_ASSERT(wk != NULL);
    GFL_STD_MemClear( wk, sizeof(MAKE_WORK));
    wk->Seq = 0;
    wk->HeapID = HEAPID_WIFIMATCH_DEBUG;
    wk->page  = 0;

    SetupDisp( wk );

    NETFLOW_Init( &wk->net_flow, p_param->p_gamedata, wk->MsgData, wk->PrintQue, HEAPID_WIFIMATCH_DEBUG );
    NETFLOW_StartRequest( &wk->net_flow, NETFLOW_STATE_Initialize );
    wk->is_data_update = TRUE;
  }

  return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Main( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAKE_WORK* wk = mywk;

  NETFLOW_Main( &wk->net_flow );

  if( MainCtrl(wk) ){
    return GFL_PROC_RES_FINISH;
  }

  return GFL_PROC_RES_CONTINUE;
}

static GFL_PROC_RESULT PROC_MAKE_WifiBattleMatch_Quit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  MAKE_WORK* wk = mywk;

  NETFLOW_Exit( &wk->net_flow );
  CleanupDisp( wk );

  GFL_PROC_FreeWork( proc );
  GFL_HEAP_DeleteHeap( HEAPID_WIFIMATCH_DEBUG );

	GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  GFL_OVERLAY_Unload( FS_OVERLAY_ID( wifibattlematch_view ) );

  return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------------
/**
 * 画面構築
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void SetupDisp( MAKE_WORK* wk )
{
  static const GFL_DISP_VRAM vramBank = {
    GX_VRAM_BG_128_A,           // メイン2DエンジンのBG
    GX_VRAM_BGEXTPLTT_NONE,     // メイン2DエンジンのBG拡張パレット
    GX_VRAM_SUB_BG_128_C,       // サブ2DエンジンのBG
    GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
    GX_VRAM_OBJ_128_B,          // メイン2DエンジンのOBJ
    GX_VRAM_OBJEXTPLTT_NONE,    // メイン2DエンジンのOBJ拡張パレット
    GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
    GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
    GX_VRAM_TEX_NONE,           // テクスチャイメージスロット
    GX_VRAM_TEXPLTT_NONE,       // テクスチャパレットスロット
    GX_OBJVRAMMODE_CHAR_1D_32K, // メインOBJマッピングモード
    GX_OBJVRAMMODE_CHAR_1D_32K, // サブOBJマッピングモード
  };

  GX_SetMasterBrightness(0);
  GXS_SetMasterBrightness(0);

  // BGsystem初期化
  GFL_BG_Init( wk->HeapID );
  {
    static const GFL_BG_SYS_HEADER sysHeader = {
      GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_3D,
    };

    static const GFL_BG_BGCNT_HEADER bgcnt_m = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    static const GFL_BG_BGCNT_HEADER bgcnt_s = {
      0, 0, 0x800, 0,
      GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
      GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x10000, 0x8000,
      GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
    };

    GFL_BG_SetBGMode( &sysHeader );
    GFL_BG_SetBGControl( PRINT_FRAME,   &bgcnt_m,   GFL_BG_MODE_TEXT );
    GFL_BG_SetBGControl( TEXT_FRAME,   &bgcnt_s,   GFL_BG_MODE_TEXT );

    GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_OFF );
    GFL_BG_SetVisible( PRINT_FRAME,       VISIBLE_ON  );
    GFL_BG_SetVisible( TEXT_FRAME,       VISIBLE_ON  );
  }

  GFL_BMPWIN_Init( wk->HeapID );
  GFL_FONTSYS_Init();

  // VRAMバンク設定
  GFL_DISP_SetBank( &vramBank );

  // 各種効果レジスタ初期化
  G2_BlendNone();
  G2S_BlendNone();

  // 上下画面設定
  GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  wk->Win = GFL_BMPWIN_Create( PRINT_FRAME, 0, 0, 32, 24, PRINT_PALIDX, GFL_BMP_CHRAREA_GET_F );
  wk->Bmp = GFL_BMPWIN_GetBmp( wk->Win );
  wk->Font = GFL_FONT_Create( ARCID_FONT, NARC_font_small_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, wk->HeapID );
  wk->MsgData = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_DEBUG_MESSAGE, NARC_debug_message_debug_wifi_match_dat, wk->HeapID );
  wk->StrBuf = GFL_STR_CreateBuffer( STRBUF_LEN, wk->HeapID );

  {
    int i;
    for( i = 0; i < EDITBOX_ID_STR_MAX; ++i )
    {
      wk->BoxStrBuf[i] = GFL_STR_CreateBuffer( 64, wk->HeapID );
    }
  }
  wk->PrintQue = PRINTSYS_QUE_Create( wk->HeapID );
  PRINT_UTIL_Setup( &wk->PrintUtil, wk->Win );


  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 0, 0, wk->HeapID );
  GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, 0, 0, wk->HeapID );
  GFL_BMP_Clear( wk->Bmp, 0x0f );
  GFL_BMPWIN_MakeScreen( wk->Win );
  GFL_BG_LoadScreenReq( PRINT_FRAME );

  GFL_BMPWIN_TransVramCharacter( wk->Win );


}
//----------------------------------------------------------------------------------
/**
 * 画面クリーンアップ
 *
 * @param   wk
 */
//----------------------------------------------------------------------------------
static void CleanupDisp( MAKE_WORK* wk )
{
  PRINTSYS_QUE_Delete( wk->PrintQue );


  {
    int i;
    for( i = 0; i < EDITBOX_ID_STR_MAX; ++i )
    {
      GFL_STR_DeleteBuffer( wk->BoxStrBuf[i] );
    }
  }
  GFL_STR_DeleteBuffer( wk->StrBuf );
  GFL_MSG_Delete( wk->MsgData );
  GFL_FONT_Delete( wk->Font );
  GFL_BMPWIN_Delete( wk->Win );
  GFL_BMPWIN_Exit();
  GFL_BG_Exit();
}

//----------------------------------------------------------------------------------
/**
 * 操作ルート
 *
 * @param   wk
 *
 * @retval  BOOL    終わったらTRUE
 */
//----------------------------------------------------------------------------------
static BOOL MainCtrl( MAKE_WORK* wk )
{
  if( PRINT_UTIL_Trans(&wk->PrintUtil, wk->PrintQue) )
  {
    switch( wk->Seq ){
    case SEQ_DRAW_CAPTION:
      {
        int i;
        for(i=0; i<layout[wk->page].len; ++i){
          PrintCaption( wk, &layout[wk->page].strParams[i] );
        }
      }
      wk->Seq = SEQ_DRAW_BOX;
      break;      
    case SEQ_DRAW_BOX:
      {
        int i;
        for(i=0; i<layout[wk->page].len; ++i){
          if( layout[wk->page].strParams[i].Type == STR_TYPE_EDITSTR )
          {
            UpdateStrBox( wk, layout[wk->page].strParams[i].editBoxID);
          }
          else
          {
            UpdateBox( wk, layout[wk->page].strParams[i].editBoxID, wk->BoxValue[ layout[wk->page].strParams[i].editBoxID ] );
          }
        }
        wk->Seq = SEQ_WAIT_CTRL;
      }
      break;      
    case SEQ_WAIT_CTRL:
      //通信中は操作させない
      if( !NETFLOW_WaitRequest(&wk->net_flow) )
      {
        break;
      }
      //データ反映フラグ
      if( wk->is_data_update == TRUE )
      {
        wk->is_data_update = FALSE;

        {
          const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data  = NETFLOW_GetSakeData( &wk->net_flow );
          StoreData( wk, cp_data );
        }

        wk->Seq = SEQ_DRAW_BOX;
        break;
      }

      //ページ更新
      {
        BOOL is_update  = FALSE;
        if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_L )
        {
          wk->page--;
          if( wk->page < 0 )
          {
            wk->page  = NELEMS(layout)-1;
          }
          is_update = TRUE;
        }
        if( GFL_UI_KEY_GetTrg() == PAD_BUTTON_R )
        {
          wk->page++;
          if( wk->page >= NELEMS(layout) )
          {
            wk->page  = 0;
          }
          is_update = TRUE;
        }
        if( is_update )
        {
          wk->Seq = SEQ_DRAW_CAPTION;
          GFL_BMP_Clear( wk->Bmp, 0x0f );
          break;
        }
      }
      //キー操作
      {
        const u16 key = GFL_UI_KEY_GetTrg();
        if ( key & PAD_BUTTON_X )
        {
          //受信
          NETFLOW_StartRequest(&wk->net_flow, NETFLOW_STATE_Recv );
          wk->is_data_update  = TRUE;
        }
        else if ( key & PAD_BUTTON_Y )
        {
          //送信
          {
            WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *p_data  = NETFLOW_GetSakeData( &wk->net_flow );
            ReStoreData( p_data, wk, wk->BoxValue[EDITBOX_ID_REC_NUM] );
          }
          NETFLOW_StartRequest(&wk->net_flow, NETFLOW_STATE_Send );
        }
        else if( key & PAD_BUTTON_B )
        {
          //アプリ終了
          return TRUE;
        }
      }

      //ボックスタッチ待ち
      wk->BoxIdx = CheckBoxTouch( wk );
      if( wk->BoxIdx >= 0 )
      {
        const STR_PARAM* p  = NULL;

        p = GetBoxForEditBoxID( wk, wk->BoxIdx );
        
        if( p == NULL ) break;

        switch( p->Type ){
        case STR_TYPE_STR:
        case STR_TYPE_EDITSTR:  
          {            
            static const GFL_SKB_SETUP setup = {
              STRBUF_LEN, GFL_SKB_STRTYPE_STRBUF,
              GFL_SKB_MODE_KATAKANA, TRUE, 0, PAD_BUTTON_START,
              PRINT_FRAME, SKB_PALIDX1, SKB_PALIDX2,
            };
            wk->skbSetup = setup;
            GetBoxStr( wk, wk->BoxIdx, wk->StrBuf );
            if( is_hiragana(wk->StrBuf) ){
              wk->skbSetup.mode = GFL_SKB_MODE_HIRAGANA;
            }
            wk->skb = GFL_SKB_Create( (void*)(wk->StrBuf), &wk->skbSetup, wk->HeapID );
            COMPSKB_Setup( &wk->comp, wk->skb, wk->StrBuf, p->arg, wk->HeapID );
            wk->Seq = SEQ_INPUT_STR;            
          }
          break;
        case STR_TYPE_NUM:
          NumInput_Setup( &wk->NumInput, wk->StrBuf, wk->Win, wk->Font,
              &wk->PrintUtil, wk->PrintQue, p, wk->BoxValue[wk->BoxIdx], wk );
          wk->Seq = SEQ_INPUT_NUM;
          break;          
        case STR_TYPE_SWITCH:
          {
            u32 val = wk->BoxValue[wk->BoxIdx];

            if( wk->touch_prev_flag ){
              if( val == 0 ){
                val = p->arg2-1;
              }
              else
              {
                --val;
              }
            }else{
              if( ++val >= p->arg2 ){
                val = 0;
              }
            }

            UpdateBox( wk, wk->BoxIdx, val );
          }
          break;
        case STR_TYPE_BUTTON:
          {
            ButtonAction_Callback action  = (ButtonAction_Callback)p->arg2;
            if( action != NULL )
            {
              action( wk, p );
            }
          }
          break;
        }       //end switch
        break;
      }
      break;
    case SEQ_INPUT_STR:      
      if( COMPSKB_Main(&wk->comp) )
      {
        int idx;
        const STR_PARAM* p;
        p = GetBoxForEditBoxID( wk, wk->BoxIdx );
        if( p == NULL ) break;

        if ( p->Type == STR_TYPE_EDITSTR)
        {

          const u32 strBoxID  = p->editBoxID - EDITBOX_ID_MAX;
          GF_ASSERT( p->editBoxID >= EDITBOX_ID_MAX);

          if( GFL_STR_GetBufferLength(wk->StrBuf) == 0 ){
            idx = 0;
          }
          GFL_STR_CopyBuffer( wk->BoxStrBuf[ strBoxID ], wk->StrBuf );
          idx = 0;
          UpdateStrBox( wk, wk->BoxIdx);
        }
        else
        {
          idx = COMPSKB_GetWordIndex( &wk->comp );
          if( idx < 0 ){
            if( GFL_STR_GetBufferLength(wk->StrBuf) == 0 ){
              idx = 0;
            }
          }
          if( idx >= 0 ){
            UpdateBox( wk, wk->BoxIdx, idx );
          }
        }
        COMPSKB_Cleanup( &wk->comp );
        GFL_SKB_Delete( wk->skb );
        wk->Seq = SEQ_WAIT_CTRL;
      }      
      break;
    case SEQ_INPUT_NUM:
      if( NumInput_Main(&wk->NumInput) )
      {
        u32 num = NumInput_GetNum( &wk->NumInput );
        UpdateBox( wk, wk->BoxIdx, num );
        wk->Seq = SEQ_WAIT_CTRL;
      }      
    }    //end switch
  } //end if
  return FALSE;
}

//----------------------------------------------------------------------------------
/**
 * 項目名描画
 *
 * @param   wk
 * @param   p
 */
//----------------------------------------------------------------------------------
static void PrintCaption( MAKE_WORK* wk, const STR_PARAM* p )
{
  if( p->StrID != CAPTION_NONE ){
    u32 ypos = p->PosY/* + ((LINE_HEIGHT - GFL_FONT_GetLineHeight(wk->Font)) / 2)*/;
    GFL_MSG_GetString( wk->MsgData, p->StrID, wk->StrBuf );
    PRINT_UTIL_Print( &wk->PrintUtil, wk->PrintQue, p->PosX, ypos, wk->StrBuf, wk->Font );
  }
}

static int CheckBoxTouch( MAKE_WORK* wk )
{
  u32 x, y;
  if( GFL_UI_TP_GetPointTrg( &x, &y ) )
  {
    u32 i;
    for(i=0; i<layout[wk->page].len; ++i)
    {
      const STR_PARAM* p = &layout[wk->page].strParams[i];
      if( p->Type != STR_TYPE_CAPTION && p->Type != STR_TYPE_FIXNUM )
      {
        if( (x >= p->TouchX) && (x <= (p->TouchX + p->TouchW))
            &&  (y >= p->TouchY) && (y <= (p->TouchY + p->TouchH))
          ){
          u32 x_center = p->TouchX + (p->TouchW / 2);
          wk->touch_prev_flag = ( x < x_center );
          return p->editBoxID;
        }
      }
    }
  }
  return -1;
}

static void UpdateBox( MAKE_WORK* wk, u32 inBoxID, u32 inValue )
{
  const STR_PARAM* p  = NULL;
  u32 str_width, str_height, xpos, ypos;
  PRINTSYS_LSB color;
  u8 color_bg;

  p = GetBoxForEditBoxID( wk, inBoxID );
  if( p == NULL ) return;
  if ( p->Type == STR_TYPE_EDITSTR ) return;
  if(  p->Type == STR_TYPE_CAPTION ) return;

  color = GetBoxSubBGColor( p->Type );
  color_bg = PRINTSYS_LSB_GetB( color );

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, color_bg );

  wk->BoxValue[ inBoxID ] = inValue;
  GetBoxStr( wk, inBoxID, wk->StrBuf );

  str_width = PRINTSYS_GetStrWidth( wk->StrBuf, wk->Font, 0 );
  xpos = p->TouchX;
  if( str_width < p->TouchW ){
    xpos += (p->TouchW - str_width) / 2;
  }
  str_height = GFL_FONT_GetLineHeight( wk->Font );
  ypos = p->TouchY;

  PRINT_UTIL_PrintColor( &wk->PrintUtil, wk->PrintQue, xpos, ypos, wk->StrBuf, wk->Font, color );
}

static void UpdateStrBox( MAKE_WORK* wk, u32 inBoxID)
{
  const STR_PARAM* p;
  u32 str_width, str_height, xpos, ypos;
  PRINTSYS_LSB color;
  u8 color_bg;
  const u32 strBoxID  = inBoxID - EDITBOX_ID_MAX;

  GF_ASSERT( inBoxID >= EDITBOX_ID_MAX);

  p = GetBoxForEditBoxID( wk, inBoxID );
  if( p == NULL ) return;
  if( p->Type != STR_TYPE_EDITSTR ) return;

  color = GetBoxSubBGColor( p->Type );
  color_bg = PRINTSYS_LSB_GetB( color );

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, color_bg );

  str_width = PRINTSYS_GetStrWidth( wk->BoxStrBuf[ strBoxID ], wk->Font, 0 );
  xpos = p->TouchX;
  if( str_width < p->TouchW ){
    xpos += (p->TouchW - str_width) / 2;
  }
  str_height = GFL_FONT_GetLineHeight( wk->Font );
  ypos = p->TouchY;

  PRINT_UTIL_PrintColor( &wk->PrintUtil, wk->PrintQue, xpos, ypos, wk->BoxStrBuf[ strBoxID ], wk->Font, color );
}


static PRINTSYS_LSB GetBoxSubBGColor( u8 inType )
{
  switch( inType ){
  case STR_TYPE_STR:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case STR_TYPE_NUM:     return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_BLUE_L );
  case STR_TYPE_SWITCH:  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_GREEN_L );
  case STR_TYPE_EDITSTR: return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_ORANGE_D );
  case STR_TYPE_FIXNUM:  return PRINTSYS_LSB_Make( COLIDX_WHITE, 0, COLIDX_GRAY );
  case STR_TYPE_BUTTON:  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_RED_L );
  }
  return PRINTSYS_LSB_Make( COLIDX_BLACK, 0, COLIDX_WHITE );
}

static void GetBoxStr( MAKE_WORK* wk, u32 inBoxID, STRBUF* buf )
{
  const STR_PARAM* p  = NULL;
  u32 value = wk->BoxValue[ inBoxID ];
  p = GetBoxForEditBoxID( wk, inBoxID );

  if( p == NULL ) return;


  switch( p->Type ){
  case STR_TYPE_STR:
    {
      GFL_MSGDATA* msgdat = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, p->arg, GFL_HEAP_LOWID(wk->HeapID) );
      GFL_MSG_GetString( msgdat, value, wk->StrBuf );
      GFL_MSG_Delete( msgdat );
    }
    break;
  case STR_TYPE_EDITSTR:
    {
      const u32 strBoxID  = inBoxID - EDITBOX_ID_MAX;
      GF_ASSERT( inBoxID >= EDITBOX_ID_MAX);
      GFL_STR_CopyBuffer( wk->StrBuf, wk->BoxStrBuf[ strBoxID ] );
    }
    break;
  case STR_TYPE_NUM:
    {
      u8 keta = 1;
      keta = CalcDisit(p->arg);
      STRTOOL_SetUnsignedNumber( wk->StrBuf, value, keta, STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    }
    break;
  case STR_TYPE_SWITCH:
    GFL_MSG_GetString( wk->MsgData, p->arg+value, wk->StrBuf );
    break;    
  case STR_TYPE_FIXNUM:
    STRTOOL_SetUnsignedNumber( wk->StrBuf, value, CalcDisit(p->arg), STR_NUM_DISP_SPACE, STR_NUM_CODE_ZENKAKU );
    break;
  case STR_TYPE_BUTTON:
    GFL_MSG_GetString( wk->MsgData, p->arg, wk->StrBuf );
    break;
  } //end switch  
}

static u32 CalcDisit( u32 inValue )
{
  u64 max;
  u32 disit;

  disit = 1;
  max = 10;
  while( 1 )
  {
    if( inValue < max ){ break; }
    max *= 10;
    ++disit;
  }
  return disit;
}
static const STR_PARAM *GetBoxForEditBoxID( const MAKE_WORK* wk, u32 inBoxID )
{
  const STR_PARAM* p  = NULL;
  int i;

  for(i=0; i<layout[wk->page].len; ++i)
  {
    if( layout[wk->page].strParams[i].editBoxID == inBoxID )
    {
      p = &layout[wk->page].strParams[i];
    }
  }

  return p;
}


//---------------------数値入力---------------------------------------------------------------------


//==============================================================================================
//  数値入力処理
//==============================================================================================
static void NumInput_Setup( NUMINPUT_WORK* wk, STRBUF* buf, GFL_BMPWIN* win, GFL_FONT* font,
            PRINT_UTIL* util, PRINT_QUE* que, const STR_PARAM* strParam, u32 value, const MAKE_WORK* make_wk )
{
  PRINTSYS_LSB color = GetBoxSubBGColor( strParam->Type );

  wk->Win = win;
  wk->Bmp = GFL_BMPWIN_GetBmp( win );
  wk->Font = font;
  wk->Buf = buf;
  wk->PrintUtil = util;
  wk->PrintQue = que;
  wk->Param = strParam;

  wk->Draw_ox = strParam->TouchX + NUMBOX_MARGIN;
  wk->ColorDefBg = PRINTSYS_LSB_GetB( color );
  wk->ColorCurBg = COLIDX_RED_L;
  wk->ColorLetter = PRINTSYS_LSB_GetL( color );

  wk->Num = value;

  wk->NumMax = strParam->arg;
  wk->NumMin = strParam->arg2;
  wk->DispNumWork = value;
  wk->Disit = CalcDisit( wk->NumMax );
  wk->CurDisit = 0;

  SetAryNumInput( wk, value );
  DrawNumInput( wk );
}
static BOOL NumInput_Main( NUMINPUT_WORK* wk )
{
  u32 x,y;
  u16 key = GFL_UI_KEY_GetTrg();

  if( (key & PAD_BUTTON_A)|| GFL_UI_TP_GetPointTrg( &x, &y ) ){
    wk->Num = CalcInputNum( wk );
    return TRUE;
  }
  if( key & PAD_BUTTON_B ){
    return TRUE;
  }

  {
    u8 update = TRUE;
    u8 up_key_flg = FALSE;
    do {
      if( key & PAD_KEY_LEFT ){
        if( wk->CurDisit < (wk->Disit-1) ){
          wk->CurDisit++;
          break;
        }
      }
      if( key & PAD_KEY_RIGHT ){
        if( wk->CurDisit > 0 ){
          wk->CurDisit--;
          break;
        }
      }
      if( key & PAD_KEY_UP )
      {
        int k = wk->CurDisit;
        while( k < (wk->Disit) ){
          wk->NumAry[k]++;
          if( wk->NumAry[k] <= 9 ){ break; }
          wk->NumAry[k] = 0;
          ++k;
        }
        // 桁あふれしたら最大値に
        if( k == (wk->Disit) ){
          OS_Printf("↑　ケタあふれ\n");
          SetAryNumInput( wk, wk->NumMax );
        }
        //上キー押下チェックON
        up_key_flg = TRUE;
        break;
      }
      if( key & PAD_KEY_DOWN )
      {
        int k = wk->CurDisit;
        while( k < (wk->Disit) )
        {
          wk->NumAry[k]--;
          if( wk->NumAry[k] >= 0 ){ break; }
          wk->NumAry[k] = 9;
          ++k;
        }
        // 桁あふれしたら最小値に
        if( k == (wk->Disit) ){
          OS_Printf("↓　ケタあふれ\n");
          SetAryNumInput( wk, wk->NumMin );
        }
        break;
      }
      if( key & PAD_BUTTON_L ){
        SetAryNumInput( wk, wk->NumMax );
        break;
      }
      if( key & PAD_BUTTON_R ){
        SetAryNumInput( wk, wk->NumMin );
        break;
      }

      update = FALSE;
    }while(0);

    if( update ){
      u32 num = CalcInputNum( wk );

      //数値加算したのに値が下回ったをチェック（値のオーバーフローチェック）
      if ( up_key_flg )
      {
        if (num < wk->DispNumWork)
        {
          num = wk->NumMax;
          SetAryNumInput( wk, num );
        }
      }

      if( num > wk->NumMax ){
        num = wk->NumMax;
        SetAryNumInput( wk, num );
      }
      if( num < wk->NumMin ){
        num = wk->NumMin;
        SetAryNumInput( wk, num );
      }
      DrawNumInput( wk );
      wk->DispNumWork = num;
    }
  }
  return FALSE;
}
static u32 NumInput_GetNum( NUMINPUT_WORK* wk )
{
  return wk->Num;
}
static void DrawNumInput( NUMINPUT_WORK* wk )
{
  const STR_PARAM* p = wk->Param;

  GFL_BMP_Fill( wk->Bmp, p->TouchX, p->TouchY, p->TouchW, p->TouchH, wk->ColorDefBg );
  {
    int i;
    u32 xpos = wk->Draw_ox;
    u32 ypos = p->TouchY;
    PRINTSYS_LSB color;
    u8 col_bg;

    for(i=wk->Disit-1; i>=0; --i)
    {
      col_bg = (i==wk->CurDisit)? wk->ColorCurBg : wk->ColorDefBg;
      color = PRINTSYS_LSB_Make( wk->ColorLetter, 0, col_bg );
      STRTOOL_SetUnsignedNumber( wk->Buf, wk->NumAry[i], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_ZENKAKU );
      GFL_BMP_Fill( wk->Bmp, xpos, p->TouchY, NUMBOX_CHAR_WIDTH, p->TouchH, col_bg );
      PRINT_UTIL_PrintColor( wk->PrintUtil, wk->PrintQue, xpos, ypos, wk->Buf, wk->Font, color );
      xpos += NUMBOX_CHAR_WIDTH;
      GFL_BMPWIN_TransVramCharacter( wk->Win );
    }
  }
}
static void SetAryNumInput( NUMINPUT_WORK* wk, u32 value )
{
  u32 k = 1000000000;
  u32 i = NUMINPUT_DISIT_MAX-1;

  GFL_STD_MemClear( wk->NumAry, sizeof(wk->NumAry) );
  while(i){
    wk->NumAry[i] = value / k;
    value -= (wk->NumAry[i] * k);
    k /= 10;
    i--;
  }
  wk->NumAry[i] = value;
}
static u32 CalcInputNum( NUMINPUT_WORK* wk )
{
  u32 i, k;
  u32 num;
  num = 0;
  for(i=0, k=1; i<wk->Disit; ++i, k*=10)
  {
    num += ( wk->NumAry[i] * k );
  }
  return num;
}




//==============================================================================================
//  入力補完処理
//==============================================================================================
static void COMPSKB_Setup( COMP_SKB_WORK* wk, GFL_SKB* skb, STRBUF* buf, u32 msgDataID, HEAPID heapID )
{
  wk->skb = skb;
  wk->msg = GFL_MSG_Create( GFL_MSG_LOAD_FAST, ARCID_MESSAGE, msgDataID, heapID );
  wk->buf = buf;
  wk->subword  = GFL_STR_CreateBuffer( 64, heapID );
  wk->fullword = GFL_STR_CreateBuffer( 64, heapID );
  wk->index = -1;
  wk->search_first_index = -1;
  wk->searchingFlag = FALSE;
}
static void COMPSKB_Cleanup( COMP_SKB_WORK* wk )
{
  GFL_STR_DeleteBuffer( wk->fullword );
  GFL_STR_DeleteBuffer( wk->subword );
  GFL_MSG_Delete( wk->msg );
}

static BOOL COMPSKB_Main( COMP_SKB_WORK* wk )
{
  GflSkbReaction reaction = GFL_SKB_Main( wk->skb );
  BOOL fSearchReq = FALSE;

  if( reaction != GFL_SKB_REACTION_NONE
  &&  reaction != GFL_SKB_REACTION_PAGE ){
    wk->searchingFlag = FALSE;
  }

  switch( reaction ){

  case GFL_SKB_REACTION_QUIT:
    if( wk->index == -1 )
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( comskb_is_match(wk, wk->buf, &idx) ){
        wk->index = idx;
      }
    }
    return TRUE;
  case GFL_SKB_REACTION_INPUT:
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( compskb_search(wk, wk->buf, -1, &idx) == 1 )
      {
        GFL_MSG_GetString( wk->msg, idx, wk->fullword );
        GFL_SKB_ReloadStr( wk->skb, wk->fullword );
        wk->index = idx;
      }
      else{
        wk->index = -1;
      }
    }
    break;
  case GFL_SKB_REACTION_BACKSPACE:
    {
      int idx;
      GFL_SKB_PickStr( wk->skb );
      if( compskb_search(wk, wk->buf, -1, &idx) == 1 ){
        wk->index = idx;
      }else{
        wk->index = -1;
      }
    }
    break;
  case GFL_SKB_REACTION_PAGE:
    fSearchReq = TRUE;
    break;
  case GFL_SKB_REACTION_NONE:
    {
      u16 key = GFL_UI_KEY_GetTrg();
      if( key & PAD_BUTTON_SELECT ){
        fSearchReq = TRUE;
      }
    }
    break;
  }

  if( fSearchReq )
  {
    if( wk->searchingFlag == FALSE ){
      GFL_SKB_PickStr( wk->skb );
      GFL_STR_CopyBuffer( wk->subword, wk->buf );
      wk->search_first_index = -1;
      wk->index = -1;
      wk->searchingFlag = TRUE;
    }
    {
      int idx;
      if( compskb_search(wk, wk->subword, wk->index, &idx) )
      {
        wk->index = idx;
        if( wk->search_first_index == -1 ){
          wk->search_first_index = idx;
        }
      }
      else
      {
        wk->index = wk->search_first_index;
      }

      if( wk->index != -1 )
      {
        GFL_MSG_GetString( wk->msg, wk->index, wk->fullword );
        GFL_SKB_ReloadStr( wk->skb, wk->fullword );
      }

    }
  }

  return FALSE;
}
static int COMPSKB_GetWordIndex( const COMP_SKB_WORK* wk )
{
  return wk->index;
}
static BOOL compskb_strncmp( const STRBUF* str1, const STRBUF* str2, u32 len )
{
  if( GFL_STR_GetBufferLength(str1) < len ){
    return FALSE;
  }
  if( GFL_STR_GetBufferLength(str2) < len ){
    return FALSE;
  }

  {
    const STRCODE *p1 = GFL_STR_GetStringCodePointer( str1 );
    const STRCODE *p2 = GFL_STR_GetStringCodePointer( str2 );
    u32 i;
    for(i=0; i<len; ++i){
      if( *p1++ != *p2++ ){ return FALSE; }
    }
    return TRUE;
  }
  return FALSE;
}
//----------------------------------------------------------------------------------
/**
 * 前方一致する文字列数をサーチ
 *
 * @param   wk
 * @param   word
 * @param   org_idx
 * @param   first_idx   [out] 最初に一致した文字列index
 *
 * @retval  u32   前方一致する文字列数
 */
//----------------------------------------------------------------------------------
static u32 compskb_search( COMP_SKB_WORK* wk, const STRBUF* word, int org_idx, int* first_idx )
{
  u32 word_len = GFL_STR_GetBufferLength( word );
  if( word_len )
  {
    u32 str_cnt, match_cnt, i;

    *first_idx = -1;
    match_cnt = 0;
    str_cnt = GFL_MSG_GetStrCount( wk->msg );
    i = (org_idx < 0)? 0 : org_idx+1;
    while( i < str_cnt )
    {
      GFL_MSG_GetString( wk->msg, i, wk->fullword );
      if( compskb_strncmp( word, wk->fullword, GFL_STR_GetBufferLength(word) ) ){
        if( *first_idx == -1 ){
          *first_idx = i;
        }
        ++match_cnt;
      }
      ++i;
    }
    return match_cnt;
  }
  return 0;
}
//----------------------------------------------------------------------------------
/**
 * 完全一致する文字列をサーチ
 *
 * @param   wk
 * @param   word
 * @param   match_idx   [OUT] 完全一致した文字列index
 *
 * @retval  BOOL    完全一致が見つかればTRUE
 */
//----------------------------------------------------------------------------------
static BOOL comskb_is_match( COMP_SKB_WORK* wk, const STRBUF* word, int* match_idx )
{
  u32 word_len = GFL_STR_GetBufferLength( word );
  if( word_len )
  {
    u32 str_cnt, i=0;

    str_cnt = GFL_MSG_GetStrCount( wk->msg );
    while( i < str_cnt )
    {
      GFL_MSG_GetString( wk->msg, i, wk->fullword );
      if( GFL_STR_CompareBuffer(word, wk->fullword) ){
        *match_idx = i;
        return TRUE;
      }
      ++i;
    }
  }
  return FALSE;
}

static BOOL is_hiragana( const STRBUF* buf )
{
  const STRCODE* ptr;
  ptr = GFL_STR_GetStringCodePointer( buf );
  if( (*ptr >= 0x3041) && (*ptr <= 0x3093) ){
    return TRUE;
  }
  return FALSE;
}

//=============================================================================
/**
 *  BUTTONのアクションコールバック
 */
//=============================================================================
static void ButtonAction_LeftButton( MAKE_WORK *wk, const STR_PARAM *StrParam )
{
  const STR_PARAM *p  = GetBoxForEditBoxID( wk, EDITBOX_ID_REC_NUM );

  if( wk->BoxValue[EDITBOX_ID_REC_NUM] == 0 )
  {
    wk->BoxValue[EDITBOX_ID_REC_NUM]  = p->arg-1;
  }
  else
  {
    wk->BoxValue[EDITBOX_ID_REC_NUM]--;
  }

  {
    const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data  = NETFLOW_GetSakeData( &wk->net_flow );
    StoreRecodeData( wk, cp_data, wk->BoxValue[EDITBOX_ID_REC_NUM] );
  }

  //読み込み直す
  wk->Seq = SEQ_DRAW_BOX;
}

static void ButtonAction_RightButton( MAKE_WORK *wk, const STR_PARAM *StrParam )
{
  const STR_PARAM *p  = GetBoxForEditBoxID( wk, EDITBOX_ID_REC_NUM );

  if( wk->BoxValue[EDITBOX_ID_REC_NUM] >= p->arg-1 )
  {
    wk->BoxValue[EDITBOX_ID_REC_NUM]  = 0;
  }
  else
  {
    wk->BoxValue[EDITBOX_ID_REC_NUM]++;
  }

  {
    const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data  = NETFLOW_GetSakeData( &wk->net_flow );
    StoreRecodeData( wk, cp_data, wk->BoxValue[EDITBOX_ID_REC_NUM] );
  }

  //読み込み直す
  wk->Seq = SEQ_DRAW_BOX;
}
//=============================================================================
/**
 *  通信フロー
 */
//=============================================================================

static void NETFLOW_Init( NETFLOW_WORK *p_wk, GAMEDATA *p_gamedata, GFL_MSGDATA *p_msg, PRINT_QUE *p_que, HEAPID heapID )
{
  GFL_STD_MemClear( p_wk, sizeof(NETFLOW_WORK) );
  p_wk->p_msg = p_msg;
  p_wk->p_font  = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE,  heapID );
  p_wk->p_text  = WBM_TEXT_Init( TEXT_FRAME, 0, 1, 1, p_que, p_wk->p_font, heapID );
  p_wk->p_net   = WIFIBATTLEMATCH_NET_Init( &p_wk->net_data, p_gamedata, NULL, heapID );

}
static void NETFLOW_Exit( NETFLOW_WORK *p_wk )
{
  WIFIBATTLEMATCH_NET_Exit( p_wk->p_net );
  WBM_TEXT_Exit( p_wk->p_text );
  GFL_FONT_Delete( p_wk->p_font );
  GFL_STD_MemClear( p_wk, sizeof(NETFLOW_WORK) );
}
static void NETFLOW_Main( NETFLOW_WORK *p_wk )
{
  if( p_wk->stateFunction )
  {
    if( p_wk->stateFunction( p_wk, &p_wk->seq ) )
    {
      p_wk->stateFunction = NULL;
    }
  }

  WIFIBATTLEMATCH_NET_Main( p_wk->p_net );
  WBM_TEXT_Main( p_wk->p_text );
}
static void NETFLOW_StartRequest( NETFLOW_WORK *p_wk, NetFlow_StateFunction state )
{
  if( NETFLOW_WaitRequest(p_wk) )
  {
    p_wk->stateFunction  = state;
    p_wk->seq = 0;
  }
}
static BOOL NETFLOW_WaitRequest( const NETFLOW_WORK *cp_wk )
{
  return cp_wk->stateFunction == NULL;
}

static WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *NETFLOW_GetSakeData( NETFLOW_WORK *p_wk )
{
  return &p_wk->sake_data;
}

static BOOL NETFLOW_STATE_Initialize( NETFLOW_WORK *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_INIT,
    SEQ_INITWAIT,
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_ERROR,
  };

  switch( *p_seq )
  {
  case SEQ_INIT:
    WIFIBATTLEMATCH_NET_StartInitialize( p_wk->p_net );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_00, WBM_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;

  case SEQ_INITWAIT:
    {
      WIFIBATTLEMATCH_GDB_RESULT  result;
      result  = WIFIBATTLEMATCH_NET_WaitInitialize( p_wk->p_net );

      switch( result )
      {
      case WIFIBATTLEMATCH_GDB_RESULT_SUCCESS:
        *p_seq  = SEQ_START;
        break;

      case WIFIBATTLEMATCH_GDB_RESULT_ERROR:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_04, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_ERROR;
        break;
      }
    }
    break;

  case SEQ_START:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_MYRECORD, WIFIBATTLEMATCH_GDB_GET_DEBUGALL, &p_wk->sake_data );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_02, WBM_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    {
      WIFIBATTLEMATCH_GDB_RESULT  result;
      result  = WIFIBATTLEMATCH_GDB_Process( p_wk->p_net );


      switch( result )
      {
      case WIFIBATTLEMATCH_GDB_RESULT_SUCCESS:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_03, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_END;
        break;

      case WIFIBATTLEMATCH_GDB_RESULT_ERROR:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_04, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_ERROR;
        break;
      }
    }
    break;
  case SEQ_END:
    if( WBM_TEXT_IsEnd( p_wk->p_text) )
    {
      return TRUE;
    }
    break;

  case SEQ_ERROR:
    /* でんげんせつだんをうながしている */
    break;
  }

  return FALSE;
}
static BOOL NETFLOW_STATE_Send( NETFLOW_WORK *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_ERROR,
  };

  switch( *p_seq )
  {
  case SEQ_START:
    WIFIBATTLEMATCH_GDB_StartWrite( p_wk->p_net, WIFIBATTLEMATCH_GDB_WRITE_DEBUGALL, &p_wk->sake_data );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_01, WBM_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    {
      WIFIBATTLEMATCH_GDB_RESULT  result;
      result  = WIFIBATTLEMATCH_GDB_ProcessWrite( p_wk->p_net );

      switch( result )
      {
      case WIFIBATTLEMATCH_GDB_RESULT_SUCCESS:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_03, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_END;
        break;

      case WIFIBATTLEMATCH_GDB_RESULT_ERROR:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_04, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_ERROR;
        break;
      }
    }
    break;
  case SEQ_END:
    if( WBM_TEXT_IsEnd( p_wk->p_text) )
    {
      return TRUE;
    }
    break;

  case SEQ_ERROR:
    /* でんげんせつだんをうながしている */
    break;
  }

  return FALSE;
}
static BOOL NETFLOW_STATE_Recv( NETFLOW_WORK *p_wk, u32 *p_seq )
{
  enum
  {
    SEQ_START,
    SEQ_WAIT,
    SEQ_END,
    SEQ_ERROR,
  };

  switch( *p_seq )
  {
  case SEQ_START:
    WIFIBATTLEMATCH_GDB_Start( p_wk->p_net, WIFIBATTLEMATCH_GDB_MYRECORD, WIFIBATTLEMATCH_GDB_GET_DEBUGALL, &p_wk->sake_data );
    WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_02, WBM_TEXT_TYPE_WAIT );
    (*p_seq)++;
    break;
  case SEQ_WAIT:
    {
      WIFIBATTLEMATCH_GDB_RESULT  result;
      result  = WIFIBATTLEMATCH_GDB_Process( p_wk->p_net );


      switch( result )
      {
      case WIFIBATTLEMATCH_GDB_RESULT_SUCCESS:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_03, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_END;
        break;

      case WIFIBATTLEMATCH_GDB_RESULT_ERROR:
        WBM_TEXT_Print( p_wk->p_text, p_wk->p_msg, D_WITIMATCH_MSG_04, WBM_TEXT_TYPE_QUE );
        *p_seq  = SEQ_ERROR;
        break;
      }
    }
    break;
  case SEQ_END:
    if( WBM_TEXT_IsEnd( p_wk->p_text) )
    {
      return TRUE;
    }
    break;

  case SEQ_ERROR:
    /* でんげんせつだんをうながしている */
    break;
  }

  return FALSE;
}


static void StoreData( MAKE_WORK *p_wk, const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data )
{
  const WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  *cp_rnd = &cp_data->rnd;
  const WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA *cp_wifi  = &cp_data->wifi;
  const MYSTATUS  *cp_mystatus  = (const MYSTATUS  *)cp_data->mystatus; 
  const WBM_NET_DATETIME  *cp_datetime  = &cp_data->datetime;

  p_wk->BoxValue[EDITBOX_ID_SINGLE_WIN] = cp_rnd->single_win;
  p_wk->BoxValue[EDITBOX_ID_SINGLE_LOSE] = cp_rnd->single_lose;
  p_wk->BoxValue[EDITBOX_ID_SINGLE_RATE] = cp_rnd->single_rate;
  p_wk->BoxValue[EDITBOX_ID_DOUBLE_WIN] = cp_rnd->double_win;
  p_wk->BoxValue[EDITBOX_ID_DOUBLE_LOSE] = cp_rnd->double_lose;
  p_wk->BoxValue[EDITBOX_ID_DOUBLE_RATE] = cp_rnd->double_rate;
  p_wk->BoxValue[EDITBOX_ID_TRIPLE_WIN] = cp_rnd->triple_win;
  p_wk->BoxValue[EDITBOX_ID_TRIPLE_LOSE] = cp_rnd->triple_lose;
  p_wk->BoxValue[EDITBOX_ID_TRIPLE_RATE] = cp_rnd->triple_rate;
  p_wk->BoxValue[EDITBOX_ID_ROTATE_WIN] = cp_rnd->rot_win;
  p_wk->BoxValue[EDITBOX_ID_ROTATE_LOSE] = cp_rnd->rot_lose;
  p_wk->BoxValue[EDITBOX_ID_ROTATE_RATE] = cp_rnd->rot_rate;
  p_wk->BoxValue[EDITBOX_ID_SHOOTER_WIN] = cp_rnd->shooter_win;
  p_wk->BoxValue[EDITBOX_ID_SHOOTER_LOSE] = cp_rnd->shooter_lose;
  p_wk->BoxValue[EDITBOX_ID_SHOOTER_RATE] = cp_rnd->shooter_rate;
  p_wk->BoxValue[EDITBOX_ID_RND_DISCONNECT] = cp_rnd->disconnect;
  p_wk->BoxValue[EDITBOX_ID_RND_CHEATS] = cp_rnd->cheat;

  //WIFIたいかいのデータ
  p_wk->BoxValue[EDITBOX_ID_WIFI_WIN] = cp_wifi->win;
  p_wk->BoxValue[EDITBOX_ID_WIFI_LOSE] = cp_wifi->lose;
  p_wk->BoxValue[EDITBOX_ID_WIFI_RATE] = cp_wifi->rate;
  p_wk->BoxValue[EDITBOX_ID_WIFI_DISCONNECT] = cp_wifi->disconnect;
  p_wk->BoxValue[EDITBOX_ID_WIFI_CHEATS] = cp_wifi->cheat;
  {
    const POKEPARTY *cp_party  = (const POKEPARTY *)cp_wifi->pokeparty;
    POKEMON_PARAM *p_pp;
    int i;

    for( i = 0; i < PokeParty_GetPokeCount(cp_party); i++ )
    {
      p_pp  = PokeParty_GetMemberPointer( cp_party, i );
      p_wk->BoxValue[EDITBOX_ID_WIFI_POKE1+i] = PP_Get( p_pp, ID_PARA_monsno, NULL);
    }
    for( ; i < PokeParty_GetPokeCountMax(cp_party); i++ )
    {
        p_wk->BoxValue[EDITBOX_ID_WIFI_POKE1+i] = 0;
    }
  }

  //プレイヤーの情報
  
  if( MyStatus_CheckNameClear( cp_mystatus ) )
  {
    GFL_STR_ClearBuffer(p_wk->BoxStrBuf[ EDITBOX_ID_MY_NAME - EDITBOX_ID_MAX ]);
  }
  else
  {
    MyStatus_CopyNameString( cp_mystatus, p_wk->BoxStrBuf[ EDITBOX_ID_MY_NAME - EDITBOX_ID_MAX ] );
  }
  p_wk->BoxValue[EDITBOX_ID_MY_RECORDID] = cp_rnd->recordID;
  p_wk->BoxValue[EDITBOX_ID_MY_TRAINERID] = MyStatus_GetID( cp_mystatus );
  p_wk->BoxValue[EDITBOX_ID_MY_PID] = MyStatus_GetProfileID( cp_mystatus );
  p_wk->BoxValue[EDITBOX_ID_MY_CONTRY] = MyStatus_GetMyNation( cp_mystatus );
  p_wk->BoxValue[EDITBOX_ID_MY_AREA] = MyStatus_GetMyArea( cp_mystatus );

  {
    int i;
    p_wk->BoxValue[EDITBOX_ID_MY_LANG]  = 0;
    for( i = 0; i < NELEMS( sc_lang_tbl ); i++ )
    {
      if( sc_lang_tbl[i]  == MyStatus_GetRegionCode( cp_mystatus ) )
      {
        p_wk->BoxValue[EDITBOX_ID_MY_LANG] = i;
      }
    }
  }
  {
    int i;
    p_wk->BoxValue[EDITBOX_ID_MY_VER] = 0;
    for( i = 0; i < NELEMS( sc_version_tbl ); i++ )
    {
      if( sc_version_tbl[i] == MyStatus_GetRomCode( cp_mystatus ) )
      {
        p_wk->BoxValue[EDITBOX_ID_MY_VER] = i;
      }
    }
  }
  p_wk->BoxValue[EDITBOX_ID_MY_VIEW] = MyStatus_GetTrainerView( cp_mystatus );
  p_wk->BoxValue[EDITBOX_ID_MY_SEX] = MyStatus_GetMySex( cp_mystatus );
  p_wk->BoxValue[EDITBOX_ID_MY_YEAR] = cp_datetime->year;
  p_wk->BoxValue[EDITBOX_ID_MY_MONTH] = cp_datetime->month;
  p_wk->BoxValue[EDITBOX_ID_MY_DAY] = cp_datetime->day;
  p_wk->BoxValue[EDITBOX_ID_MY_HOUR] = cp_datetime->hour;
  p_wk->BoxValue[EDITBOX_ID_MY_SECOND] = cp_datetime->minute;
  p_wk->BoxValue[EDITBOX_ID_MY_MINITS] = cp_datetime->second;

  StoreRecodeData( p_wk, cp_data, p_wk->BoxValue[EDITBOX_ID_REC_NUM] );
}
static void StoreRecodeData( MAKE_WORK *p_wk, const WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *cp_data, u32 idx )
{
  const WIFIBATTLEMATCH_RECORD_DATA *cp_record  = &cp_data->record_data[ idx ];

  //せんせきのデータ(ポケモン)
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE1]  = cp_record->your_monsno[ 0 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM1]  = cp_record->your_form[ 0 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV1]    = cp_record->your_lv[ 0 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX1]   = cp_record->your_sex[ 0 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE2]  = cp_record->your_monsno[ 1 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM2]  = cp_record->your_form[ 1 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV2]    = cp_record->your_lv[ 1 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX2]   = cp_record->your_sex[ 1 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE3]  = cp_record->your_monsno[ 2 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM3]  = cp_record->your_form[ 2 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV3]    = cp_record->your_lv[ 2 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX3]   = cp_record->your_sex[ 2 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE4]  = cp_record->your_monsno[ 3 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM4]  = cp_record->your_form[ 3 ];  
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV4]    = cp_record->your_lv[ 3 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX4]   = cp_record->your_sex[ 3 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE5]  = cp_record->your_monsno[ 4 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM5]  = cp_record->your_form[ 4 ];  
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV5]    = cp_record->your_lv[ 4 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX5]   = cp_record->your_sex[ 4 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE6]  = cp_record->your_monsno[ 5 ];
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM6]  = cp_record->your_form[ 5 ];  
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_LV6]    = cp_record->your_lv[ 5 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX6]   = cp_record->your_sex[ 5 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE1]   = cp_record->my_monsno[ 0 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM1]   = cp_record->my_form[ 0 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV1]     = cp_record->my_lv[ 0 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX1]    = cp_record->my_sex[ 0 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE2]   = cp_record->my_monsno[ 1 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM2]   = cp_record->my_form[ 1 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV2]     = cp_record->my_lv[ 1 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX2]    = cp_record->my_sex[ 1 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE3]   = cp_record->my_monsno[ 2 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM3]   = cp_record->my_form[ 2 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV3]     = cp_record->my_lv[ 2 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX3]    = cp_record->my_sex[ 2 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE4]   = cp_record->my_monsno[ 3 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM4]   = cp_record->my_form[ 3 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV4]     = cp_record->my_lv[ 3 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX4]    = cp_record->my_sex[ 3 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE5]   = cp_record->my_monsno[ 4 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM5]   = cp_record->my_form[ 4 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV5]     = cp_record->my_lv[ 4 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX5]    = cp_record->my_sex[ 4 ];    
  p_wk->BoxValue[EDITBOX_ID_REC_MY_POKE6]   = cp_record->my_monsno[ 5 ]; 
  p_wk->BoxValue[EDITBOX_ID_REC_MY_FORM6]   = cp_record->my_form[ 5 ];   
  p_wk->BoxValue[EDITBOX_ID_REC_MY_LV6]     = cp_record->my_lv[ 5 ];     
  p_wk->BoxValue[EDITBOX_ID_REC_MY_SEX6]    = cp_record->my_sex[ 5 ];    

  //せんせきのデータ(たいせんじょうほう)
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_GSID] = cp_record->your_gamesyncID;
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_PID] = cp_record->your_profileID;
  p_wk->BoxValue[EDITBOX_ID_REC_CUPNO] = cp_record->cupNO;
  p_wk->BoxValue[EDITBOX_ID_REC_BTLTYPE] = cp_record->btl_type;
  p_wk->BoxValue[EDITBOX_ID_REC_RESULT] = cp_record->result;
  p_wk->BoxValue[EDITBOX_ID_REC_MY_RESTPOKE] = cp_record->rest_my_poke;
  p_wk->BoxValue[EDITBOX_ID_REC_YOU_RESTPOKE] = cp_record->rest_you_poke;
  p_wk->BoxValue[EDITBOX_ID_BTL_YEAR] = cp_record->year;
  p_wk->BoxValue[EDITBOX_ID_BTL_MONTH] = cp_record->month;
  p_wk->BoxValue[EDITBOX_ID_BTL_DAY] = cp_record->day;
  p_wk->BoxValue[EDITBOX_ID_BTL_HOUR] = cp_record->hour;
  p_wk->BoxValue[EDITBOX_ID_BTL_SECOND] = cp_record->minute;
  p_wk->BoxValue[EDITBOX_ID_BTL_MINITS] = 0;
  p_wk->BoxValue[EDITBOX_ID_REC_NEXT] = cp_data->record_save_idx;
}
static void ReStoreData( WIFIBATTLEMATCH_GDB_SAKE_ALL_DATA *p_data, const MAKE_WORK *cp_wk, u32 idx )
{
  WIFIBATTLEMATCH_GDB_RND_SCORE_DATA  *p_rnd = &p_data->rnd;
  WIFIBATTLEMATCH_GDB_WIFI_SCORE_DATA *p_wifi  = &p_data->wifi;
  MYSTATUS  *p_mystatus  = (MYSTATUS  *)p_data->mystatus; 
  WBM_NET_DATETIME  *p_datetime  = &p_data->datetime;

  WIFIBATTLEMATCH_RECORD_DATA *p_record  = &p_data->record_data[ idx ];

  //ランダム大会
  p_rnd->single_win   = cp_wk->BoxValue[EDITBOX_ID_SINGLE_WIN];
  p_rnd->single_lose  = cp_wk->BoxValue[EDITBOX_ID_SINGLE_LOSE];
  p_rnd->single_rate  = cp_wk->BoxValue[EDITBOX_ID_SINGLE_RATE];
  p_rnd->double_win   = cp_wk->BoxValue[EDITBOX_ID_DOUBLE_WIN];
  p_rnd->double_lose  = cp_wk->BoxValue[EDITBOX_ID_DOUBLE_LOSE];
  p_rnd->double_rate  = cp_wk->BoxValue[EDITBOX_ID_DOUBLE_RATE];
  p_rnd->triple_win   = cp_wk->BoxValue[EDITBOX_ID_TRIPLE_WIN];
  p_rnd->triple_lose  = cp_wk->BoxValue[EDITBOX_ID_TRIPLE_LOSE];
  p_rnd->triple_rate  = cp_wk->BoxValue[EDITBOX_ID_TRIPLE_RATE];
  p_rnd->rot_win   = cp_wk->BoxValue[EDITBOX_ID_ROTATE_WIN];
  p_rnd->rot_lose  = cp_wk->BoxValue[EDITBOX_ID_ROTATE_LOSE];
  p_rnd->rot_rate  = cp_wk->BoxValue[EDITBOX_ID_ROTATE_RATE];
  p_rnd->shooter_win   = cp_wk->BoxValue[EDITBOX_ID_SHOOTER_WIN];
  p_rnd->shooter_lose  = cp_wk->BoxValue[EDITBOX_ID_SHOOTER_LOSE];
  p_rnd->shooter_rate  = cp_wk->BoxValue[EDITBOX_ID_SHOOTER_RATE];
  p_rnd->disconnect  = cp_wk->BoxValue[EDITBOX_ID_RND_DISCONNECT];
  p_rnd->cheat  = cp_wk->BoxValue[EDITBOX_ID_RND_CHEATS];

  //WIFI大会
  p_wifi->win = cp_wk->BoxValue[EDITBOX_ID_WIFI_WIN];
  p_wifi->lose = cp_wk->BoxValue[EDITBOX_ID_WIFI_LOSE];
  p_wifi->rate = cp_wk->BoxValue[EDITBOX_ID_WIFI_RATE];
  p_wifi->disconnect = cp_wk->BoxValue[EDITBOX_ID_WIFI_DISCONNECT];
  p_wifi->cheat = cp_wk->BoxValue[EDITBOX_ID_WIFI_CHEATS];
  {
    int i;

    POKEPARTY *p_party  = (POKEPARTY*)p_wifi->pokeparty;
    POKEMON_PARAM *p_pp;

    if( PokeParty_GetPokeCountMax(p_party) == 0 )
    {
      PokeParty_InitWork(p_party);
    }

    for( i = 0; i < TEMOTI_POKEMAX; i++ )
    {
      if( i < PokeParty_GetPokeCount(p_party) )
      {
        p_pp  = PokeParty_GetMemberPointer( p_party, i );
        PP_Put( p_pp, ID_PARA_monsno, cp_wk->BoxValue[EDITBOX_ID_WIFI_POKE1+i] );
      }
      else
      {
        if( cp_wk->BoxValue[EDITBOX_ID_WIFI_POKE1+i] != 0 )
        {
          p_pp  = PP_Create( cp_wk->BoxValue[EDITBOX_ID_WIFI_POKE1+i], 1, PTL_SETUP_ID_AUTO, HEAPID_WIFIMATCH_DEBUG );
          PokeParty_Add(p_party, p_pp);
          GFL_HEAP_FreeMemory( p_pp );
        }
      }
    }
  }

  //プレイヤーの情報
  MyStatus_SetMyNameFromString( p_mystatus, cp_wk->BoxStrBuf[  EDITBOX_ID_MY_NAME - EDITBOX_ID_MAX ] );
  MyStatus_SetID( p_mystatus, cp_wk->BoxValue[EDITBOX_ID_MY_TRAINERID] );
  MyStatus_SetProfileID( p_mystatus, cp_wk->BoxValue[EDITBOX_ID_MY_PID] );
  MyStatus_SetMyNationArea( p_mystatus, cp_wk->BoxValue[EDITBOX_ID_MY_CONTRY], cp_wk->BoxValue[EDITBOX_ID_MY_AREA] );
  MyStatus_SetRegionCode( p_mystatus, sc_lang_tbl[ cp_wk->BoxValue[EDITBOX_ID_MY_LANG] ] );
  MyStatus_SetRomCode( p_mystatus, sc_version_tbl[ cp_wk->BoxValue[EDITBOX_ID_MY_VER]] );
  MyStatus_SetTrainerView( p_mystatus, cp_wk->BoxValue[EDITBOX_ID_MY_VIEW] );
  MyStatus_SetMySex( p_mystatus, cp_wk->BoxValue[EDITBOX_ID_MY_SEX] );
  p_datetime->year  = cp_wk->BoxValue[EDITBOX_ID_MY_YEAR];
  p_datetime->month  = cp_wk->BoxValue[EDITBOX_ID_MY_MONTH];
  p_datetime->day  = cp_wk->BoxValue[EDITBOX_ID_MY_DAY];
  p_datetime->hour  = cp_wk->BoxValue[EDITBOX_ID_MY_HOUR];
  p_datetime->minute  = cp_wk->BoxValue[EDITBOX_ID_MY_SECOND];
  p_datetime->second  = cp_wk->BoxValue[EDITBOX_ID_MY_MINITS];

  //戦績
  {
    int i;
    for( i = 0; i < TEMOTI_POKEMAX; i++ )
    {
      p_record->your_monsno[i]  = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_POKE1+(i*4)];
      p_record->your_form[i]    = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_FORM1+(i*4)];
      p_record->your_lv[i]      = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_LV1+(i*4)];
      p_record->your_sex[i]     = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_SEX1+(i*4)];
      p_record->my_monsno[i]  = cp_wk->BoxValue[EDITBOX_ID_REC_MY_POKE1+(i*4)];
      p_record->my_form[i]    = cp_wk->BoxValue[EDITBOX_ID_REC_MY_FORM1+(i*4)];
      p_record->my_lv[i]      = cp_wk->BoxValue[EDITBOX_ID_REC_MY_LV1+(i*4)];
      p_record->my_sex[i]     = cp_wk->BoxValue[EDITBOX_ID_REC_MY_SEX1+(i*4)];
    }
  }

  p_record->your_gamesyncID = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_GSID];
  p_record->your_profileID = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_PID];
  p_record->cupNO = cp_wk->BoxValue[EDITBOX_ID_REC_CUPNO];
  p_record->btl_type = cp_wk->BoxValue[EDITBOX_ID_REC_BTLTYPE];
  p_record->result = cp_wk->BoxValue[EDITBOX_ID_REC_RESULT];
  p_record->rest_my_poke = cp_wk->BoxValue[EDITBOX_ID_REC_MY_RESTPOKE];
  p_record->rest_you_poke = cp_wk->BoxValue[EDITBOX_ID_REC_YOU_RESTPOKE];
  p_data->record_save_idx  = cp_wk->BoxValue[EDITBOX_ID_REC_NEXT];
  p_record->year  = cp_wk->BoxValue[EDITBOX_ID_BTL_YEAR];
  p_record->month  = cp_wk->BoxValue[EDITBOX_ID_BTL_MONTH];
  p_record->day  = cp_wk->BoxValue[EDITBOX_ID_BTL_DAY];
  p_record->hour  = cp_wk->BoxValue[EDITBOX_ID_BTL_HOUR];
  p_record->minute  = cp_wk->BoxValue[EDITBOX_ID_BTL_SECOND];

}

#endif
