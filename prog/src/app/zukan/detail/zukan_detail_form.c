//============================================================================
/**
 *  @file   zukan_detail_form.c
 *  @brief  図鑑詳細画面の姿画面
 *  @author Koji Kawada
 *  @data   2010.02.02
 *  @note   
 *  モジュール名：ZUKAN_DETAIL_FORM
 */
//============================================================================

#define DEF_SCMD_CHANGE  // これが定義されているとき、CMDが発行されるまで待たずにSCMDが発行されたときに変更を開始する
#define DEF_MCSS_HANDLE  // これが定義されているとき、MCSSのハンドル読み込みを利用する
#define DEF_MCSS_TCBSYS  // これが定義されているとき、MCSSのTCBSYS外部指定を利用する
#define DEF_MINIMUM_LOAD  // これが定義されているとき、ポケモン変更やフォルム変更でそのとき見えている最小限必要なものしか読み込まない


#define DEF_POKE_ARRANGE_INDIVIDUAL  // これが定義されているとき、mons_no, form_no, sex, rare, egg, dir, personal_rnd
                                     // (rare, egg, personal_rndは現在未使用)ごとにポケモンの配置を設定できる


//#define DEBUG_KAWADA
//#define DEBUG_POKE_POS_SET


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "ui/ui_easy_clwk.h"
#include "pokeicon/pokeicon.h"
#include "gamesystem/gamedata_def.h"
#include "gamesystem/game_data.h"
#include "savedata/zukan_savedata.h"
#include "poke_tool/poke_personal.h"
#include "system/bmp_winframe.h"
#include "print/printsys.h"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/global_msg.h"
#include "sound/pm_sndsys.h"

#include "zukan_detail_def.h"
#include "zukan_detail_common.h"
#include "zukan_detail_graphic.h"
#include "zukan_detail_form.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "message.naix"
#include "msg/msg_zkn.h"
#include "msg/msg_zkn_form.h"
#include "zukan_gra.naix"
#include "zukan_data.naix"

// サウンド

// オーバーレイ


/*
表示ルール


1 フォルム違いがなくて、オスメスもないとき

ミュウのすがた
ミュウのすがた  (いろちがい)


2 フォルム違いがなくて、オスメス(オスとメスでグラフィックに
  違いがあろうとなかろうと、オスとメスを表示する)があるとき

オスのすがた
オスのすがた  (いろちがい)
メスのすがた
メスのすがた  (いろちがい)

注意！：オスメスどちらか片方したいないときもある。
例えば、オスしかいないときは

オスのすがた
オスのすがた  (いろちがい)

となる。


3 フォルム違いがあるとき(オスメスはあろうとなかろうと、無視する)

ノーマルフォルム
ノーマルフォルム  (いろちがい)
アタックフォルム
アタックフォルム  (いろちがい)
ディフェンスフォルム
ディフェンスフォルム  (いろちがい)
スピードフォルム
スピードフォルム  (いろちがい)


*/


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
// MCSSポケモンのフェード
#define POKE_MCSS_FADE_WAIT  (-2)//-1じゃ周りのフェードに比べて遅い(-1)  // 0のとき毎フレーム1進む; 1のときoxoxというふうに1フレームおきに1進む;
                                   // -1のとき毎フレーム2進む; -2のとき毎フレーム3進む;

// メインBGフレーム
#define BG_FRAME_M_POKE          (GFL_BG_FRAME0_M)
#define BG_FRAME_M_TEXT          (GFL_BG_FRAME2_M)
#define BG_FRAME_M_REAR          (GFL_BG_FRAME3_M)
// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_POKE      (0)
#define BG_FRAME_PRI_M_TEXT      (2)
#define BG_FRAME_PRI_M_REAR      (3)

#define BG_FRAME_PRI_M_TOUCHBAR  (2)  // この画面ではタッチバーのプライオリティを3D面のプライオリティより下げておく
                                      // 何故か1では、タッチバーBGより手前に3Dが来るが、タッチバー上のアイコンより後ろに3Dが来てしまう。

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_TEXT              = 1,
  BG_PAL_NUM_M_REAR              = ZKNDTL_COMMON_REAR_BG_PAL_NUM,
};
// 位置
enum
{
  BG_PAL_POS_M_TEXT              = 0,
  BG_PAL_POS_M_REAR              = 1,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_BUTTON_BAR       = 3,  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル
  OBJ_PAL_NUM_M_FIELD            = 1,  // フォルム名の背面フィールド
  OBJ_PAL_NUM_M_ARROW            = 2,  // トップ画面でフォルムを切り替えるための矢印
};
// 位置
enum
{
  OBJ_PAL_POS_M_BUTTON_BAR       = 0,
  OBJ_PAL_POS_M_FIELD            = 3,
  OBJ_PAL_POS_M_ARROW            = 4,
};


// サブBGフレーム
#define BG_FRAME_S_TEXT           (GFL_BG_FRAME2_S)
#define BG_FRAME_S_PANEL          (GFL_BG_FRAME3_S)
#define BG_FRAME_S_REAR           (GFL_BG_FRAME0_S)
// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_TEXT       (1)
#define BG_FRAME_PRI_S_PANEL      (2)
#define BG_FRAME_PRI_S_REAR       (3)

// サブBGパレット
enum
{
  BG_PAL_NUM_S_TEXT              = 1,
  BG_PAL_NUM_S_PANEL             = 1,
  BG_PAL_NUM_S_REAR              = ZKNDTL_COMMON_REAR_BG_PAL_NUM,
};
// 位置
enum
{
  BG_PAL_POS_S_TEXT              = 0,
  BG_PAL_POS_S_PANEL             = 1,
  BG_PAL_POS_S_REAR              = 2,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_POKEICON         = 3,
};
// 位置
enum
{
  OBJ_PAL_POS_S_POKEICON         = 0,
};


// ProcMainのシーケンス
enum
{
  SEQ_START      = 0,
  SEQ_FADE_CHANGE_BEFORE,
  SEQ_PREPARE_0,
  SEQ_PREPARE,
  SEQ_FADE_IN,
  SEQ_MAIN,
  SEQ_POST,
  SEQ_FADE_OUT,
  SEQ_FADE_CHANGE_AFTER,
  SEQ_END,
};

// 終了命令
typedef enum
{
  END_CMD_NONE,
  END_CMD_INSIDE,
  END_CMD_OUTSIDE,
}
END_CMD;

// 状態
typedef enum
{
  STATE_TOP,
  STATE_TOP_OSHIDASHI,
  STATE_TOP_TO_EXCHANGE,
  STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE,
  STATE_EXCHANGE,
  STATE_EXCHANGE_IREKAE,
  STATE_EXCHANGE_TO_TOP,
  STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE,
}
STATE;

// MCSSポケモン
typedef enum
{
  POKE_CURR_F,    // 今のフォルム(前向き)
  POKE_CURR_B,    // 今のフォルム(後向き)
  POKE_COMP_F,    // 比較フォルム(前向き)
  POKE_COMP_B,    // 比較フォルム(後向き)
  POKE_MAX,

#ifdef DEF_MINIMUM_LOAD 
  POKE_INDEX_NONE    = POKE_MAX,  // ないことを表すPOKE_INDEX
#endif
}
POKE_INDEX;

// ポケモンの大きさ
#define POKE_SCALE    (16.0f)
#define POKE_SIZE_MAX (96.0f)

// ポケモンの位置
typedef enum
{
  POKE_CURR_POS_CENTER,  // 今のフォルムのポケモンの位置
  POKE_CURR_POS_LEFT, 
  POKE_COMP_RPOS,        // 比較フォルムのポケモンの位置(POKE_CURR_POS_???を基準とした相対位置)
  POKE_POS_MAX,
}
POKE_POS_INDEX;

static VecFx32 poke_pos[POKE_POS_MAX] =
{
  { FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(-13.9f), FX_F32_TO_FX32(0.0f) },  // (-14.0f)だと絵が汚くなるものがいたので(-13.9f)にした。
  { FX_F32_TO_FX32(-16.0f), FX_F32_TO_FX32(-13.9f), FX_F32_TO_FX32(0.0f) },
  { FX_F32_TO_FX32( 32.0f), FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(0.0f) },
};

// ポケモンのX座標調整
#define POKE_X_ADJUST (0.25f)  // この値を掛ける


// 入れ替えの位置
#define POKE_IREKAE_POS_LEFT_X  (-16.0f)
#define POKE_IREKAE_POS_RIGHT_X ( 16.0f)
#define POKE_IREKAE_POS_BASE_Y  (-13.9f)
#define POKE_IREKAE_HEIGHT      ( 16.0f)  // 透視射影だとなるべく被らないようにしておかないとみっともないので、そこそこ高さのあるきれいな円を描くような高さにした。
#define POKE_IREKAE_POS_CURR_Z  (  0.0f)  // 後ろに描画されるように奥のほうへ配置
#define POKE_IREKAE_POS_COMP_Z  (  0.0f)  // 前に描画されるように手前のほうへ配置
                                          // としたかったが、透視射影だと奥に描画するとその分画面中心によった位置にポケモンが出てしまうのでダメ。

// 押し出しの位置
#define POKE_OSHIDASHI_POS_LEFT_X   (-64.0f)
#define POKE_OSHIDASHI_POS_CENTER_X (  0.0f)
#define POKE_OSHIDASHI_POS_RIGHT_X  ( 64.0f)
#define POKE_OSHIDASHI_POS_BASE_Y   (-13.9f)
#define POKE_OSHIDASHI_POS_BASE_Z   (  0.0f)

// 階層変更の位置
#define POKE_KAISOU_EXCHANGE_POS_LEFT_X  (-16.0f)
#define POKE_KAISOU_EXCHANGE_POS_RIGHT_X ( 16.0f)
#define POKE_KAISOU_TOP_POS_CENTER_X     (  0.0f)
#define POKE_KAISOU_TOP_POS_RIGHT_X      ( 64.0f)
#define POKE_KAISOU_POS_BASE_Y           (-13.9f)
#define POKE_KAISOU_POS_BASE_Z           (  0.0f)


// ポケモンのアニメーションのループの最大数
#ifndef DEBUG_POKE_POS_SET
#define POKE_MCSS_ANIME_LOOP_MAX  (2)
#else
#define POKE_MCSS_ANIME_LOOP_MAX  (60)
#endif


#ifdef DEF_MCSS_TCBSYS
  #define MCSS_TCBSYS_TASK_MAX (4)  // MCSS1つにつきTCB1つでいいらしい。最大4つのMCSSを同時に保持するので。
#endif


// ポケアイコンの位置
static const u8 pokeicon_pos[2] = { 128, 128 };

// 違う姿
typedef enum
{
  LOOKS_DIFF_ONE,   // 性別なし、フォルムなし
  LOOKS_DIFF_SEX,   // 性別あり、フォルムなし
  LOOKS_DIFF_FORM,  // 性別不問、フォルムあり
}
LOOKS_DIFF;

typedef enum
{
  COLOR_DIFF_NONE,     // 色違いなし
  COLOR_DIFF_NORMAL,   // 色違いありで通常色
  COLOR_DIFF_SPECIAL,  // 色違いありで特別色
}
COLOR_DIFF;

enum
{
  SEX_MALE,
  SEX_FEMALE,
};

// 違う姿の数
#define DIFF_MAX (64)  // これで足りるか？

#ifdef DEF_MINIMUM_LOAD 
  #define DIFF_NULL (DIFF_MAX)  // ないことを表す違う姿の番号
#endif


// テキスト
enum
{
  TEXT_POKENAME,
  TEXT_UP_NUM,
  TEXT_UP_LABEL,
  TEXT_POKE_CURR,
  TEXT_POKE_COMP,
  TEXT_MAX,
};
static const u16 text_up_label_num_y[2] = { 1, 17 };
// 文字数
#define STRBUF_NUM_LENGTH       (  8 )  // ??  // 例：見つけた姿の数  99  ←この例の99の文字数
#define STRBUF_POKENAME_LENGTH  ( 64 )  // [ポケモン種族名]のすがた  // buflen.hで確認する必要あり！

#define TEXT_POKE_POS_Y_1_LINE   (  9 )
#define TEXT_POKE_POS_Y_2_LINE_0 (  1 )
#define TEXT_POKE_POS_Y_2_LINE_1 ( 17 )

// よく使うテキスト
enum
{
  TEXT_FREQUENT_SUGATA_NUM,       // 姿の数
  TEXT_FREQUENT_IROCHIGAI_NUM,    // 色違いの数
  TEXT_FREQUENT_POKE_SUGATA,      // ポケモン名の姿
  TEXT_FREQUENT_MALE_SUGATA,      // オスの姿
  TEXT_FREQUENT_FEMALE_SUGATA,    // メスの姿
  TEXT_FREQUENT_NOTE_IROCHIGAI,   // (色違い)
  TEXT_FREQUENT_MAX,
};
static const u16 text_frequent_str_id[TEXT_FREQUENT_MAX] =
{
  ZKN_FORME_TEXT_03,
  ZKN_FORME_TEXT_04,
  ZKN_FORMNAME_10,
  ZNK_ZUKAN_GRAPH_00,
  ZNK_ZUKAN_GRAPH_01,
  ZKN_FORME_TEXT_05,
};

// OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド
enum
{
  OBJ_RES_BUTTON_BAR_NCG,  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル 
  OBJ_RES_BUTTON_BAR_NCL,
  OBJ_RES_BUTTON_BAR_NCE,
  OBJ_RES_FIELD_NCG,       // フォルム名の背面フィールド
  OBJ_RES_FIELD_NCL,
  OBJ_RES_FIELD_NCE,
  OBJ_RES_ARROW_NCG,       // トップ画面でフォルムを切り替えるための矢印
  OBJ_RES_ARROW_NCL,
  OBJ_RES_ARROW_NCE,
  OBJ_RES_MAX,
};
enum
{
  OBJ_BAR_RANGE,          // スクロールバー
  OBJ_BAR_CURSOR,         // バーカーソル
  OBJ_FIELD_CURR,         // フォルム名の背面フィールド
  OBJ_FIELD_COMP,         // フォルム名の背面フィールド
  OBJ_MAX,
};
#define BAR_Y_BASE (6)
static const u8 obj_setup_info[OBJ_MAX][8] =
{
  //  pos_x,         pos_y,  anmseq,  softpri,  bgpri,               ncg,                    ncl,                    nce
  {    18*8,  BAR_Y_BASE*8,      21,        1,  BG_FRAME_PRI_M_REAR, OBJ_RES_BUTTON_BAR_NCG, OBJ_RES_BUTTON_BAR_NCL, OBJ_RES_BUTTON_BAR_NCE },
  { 18*8+12,  BAR_Y_BASE*8,      22,        0,  BG_FRAME_PRI_M_REAR, OBJ_RES_BUTTON_BAR_NCG, OBJ_RES_BUTTON_BAR_NCL, OBJ_RES_BUTTON_BAR_NCE },
  {     0*8,           0*8,       0,        0,  BG_FRAME_PRI_M_REAR, OBJ_RES_FIELD_NCG,      OBJ_RES_FIELD_NCL,      OBJ_RES_FIELD_NCE      },
  {    16*8,           0*8,       0,        0,  BG_FRAME_PRI_M_REAR, OBJ_RES_FIELD_NCG,      OBJ_RES_FIELD_NCL,      OBJ_RES_FIELD_NCE      },
};

// ボタンの状態
typedef enum
{
  BUTTON_STATE_ACTIVE,       // 押すことができる
  BUTTON_STATE_PUSH_START,   // 押した瞬間(1フレーム)
  BUTTON_STATE_PUSH_ANIME,   // 押したアニメ中
  BUTTON_STATE_PUSH_END,     // 押したアニメが終了した瞬間(1フレーム)
}
BUTTON_STATE;
// ボタンOBJ
typedef enum
{
  BUTTON_OBJ_FRONT_BACK,  // 前後ボタン
  BUTTON_OBJ_PLAY,        // 再生ボタン
  BUTTON_OBJ_ARROW_L,     // トップ画面でフォルムを切り替えるための矢印
  BUTTON_OBJ_ARROW_R,     // トップ画面でフォルムを切り替えるための矢印
  BUTTON_OBJ_MAX,

  // タッチ用
  BUTTON_OBJ_NONE,        // ボタンを押していない
  BUTTON_OBJ_IGNORE,      // ボタンを押したが無視された(先に他のボタンを押していたり、押したボタンが既に押し中だったりしたとき)
}
BUTTON_OBJ;

// スクロールバー
// バーカーソル
#define  BAR_CURSOR_POS_Y      (BAR_Y_BASE*8)
#define  BAR_CURSOR_POS_X_MIN  (18*8+12)                 // BAR_CURSOR_POS_X_MIN<= <=BAR_CURSOR_POS_X_MAX
#define  BAR_CURSOR_POS_X_MAX  (18*8+12 +8*8)
// スクロールバー
#define  BAR_RANGE_TOUCH_X_MIN  (18*8)                   // BAR_RANGE_TOUCH_X_MIN<= <=BAR_RANGE_TOUCH_X_MAX
#define  BAR_RANGE_TOUCH_X_MAX  (18*8 +11*8-1)
#define  BAR_RANGE_TOUCH_Y_MIN  (BAR_Y_BASE*8)           // BAR_RANGE_TOUCH_Y_MIN<= <=BAR_RANGE_TOUCH_Y_MAX
#define  BAR_RANGE_TOUCH_Y_MAX  (BAR_Y_BASE*8 +1*8-1)


// 入れ替え
typedef enum
{
  IREKAE_STATE_NONE,    // 入れ替えしていない
  IREKAE_STATE_MOVE,    // 入れ替え中
}
IREKAE_STATE;

#define IREKAE_THETA_ADD    (0x400)  // 増加分  // FX_SinIdx
#define IREKAE_THETA_MAX   (0x8000)  // 半周    // FX_SinIdx


// 押し出し
typedef enum
{
  OSHIDASHI_STATE_NONE,    // 押し出ししていない
  OSHIDASHI_STATE_MOVE,    // 押し出し中
  OSHIDASHI_STATE_CHANGE_COMPARE_FORM,    // 押し出しが完了したので、比較フォルムを変更する
}
OSHIDASHI_STATE;
typedef enum
{
  OSHIDASHI_DIRECT_L_TO_R,  // 左から右へ
  OSHIDASHI_DIRECT_R_TO_L,  // 右から左へ
}
OSHIDASHI_DIRECT;

#define OSHIDASHI_SPEED  (2.0f)  // 押し出し速度(1フレームでこれだけ移動する)


// 階層変更  // TOP_TO_EXCHANGE or EXCHANGE_TO_TOP
#define KAISOU_CURR_SPEED  (0.5f)  // 階層変更の際のcurrの速度(1フレームでこれだけ移動する)
#define KAISOU_COMP_SPEED  (1.5f)//(2.0f)  // 階層変更の際のcompの速度(1フレームでこれだけ移動する)


// OBJを差し替える際に乱れないように、2つを交互に表示する
typedef enum
{
  OBJ_SWAP_0,
  OBJ_SWAP_1,
  OBJ_SWAP_MAX,
}
OBJ_SWAP;


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
// ボタン
typedef struct
{
  s16           set_pos_x;  // 配置座標
  s16           set_pos_y;
  u32           set_ncg;    // リソース
  u32           set_ncl;
  u32           set_nce;
  u8            pos_x;   // ピクセル  // 左上  // タッチ座標
  u8            pos_y; 
  u8            size_x;  // ピクセル           // タッチサイズ
  u8            size_y;
  u8            anmseq_active;  // 押すことができるときのアニメ番号
  u8            anmseq_push;    // 押したアニメ中のアニメ番号
  u32           key;
  u32           se;
  BUTTON_STATE  state;
  GFL_CLWK*     clwk;
}
BUTTON;

// NNS_G2dSetAnimCtrlCallBackFunctorに渡すユーザ定義情報param
typedef struct
{
  POKE_INDEX  poke_idx;
  void*       zukan_detail_form_work;  // (ZUKAN_DETAIL_FORM_WORK*)というふうにキャストして使って下さい
  u8          count;   // アニメーションが何回ループしたか
  BOOL        stop;    // TRUEのとき、アニメーションを止める必要あり
}
POKE_MCSS_CALL_BACK_DATA;

#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
typedef struct
{
  f32 x;
  f32 y;
  f32 z;
}
POKE_VEC_F32;
typedef enum
{
  POKE_ARRANGE_TOP_CENTER,      // TOP画面中央配置
  POKE_ARRANGE_TOP_LEFT,        // TOP画面左配置(画面外)
  POKE_ARRANGE_TOP_RIGHT,       // TOP画面右配置(画面外)
  POKE_ARRANGE_EXCHANGE_LEFT,   // EXCHANGE画面左配置
  POKE_ARRANGE_EXCHANGE_RIGHT,  // EXCHANGE画面右配置
  POKE_ARRANGE_MAX,
}
POKE_ARRANGE;
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

// ポケモンMCSS
typedef struct
{
  MCSS_WORK*                 poke_wk;              // NULLのときなし
  POKE_MCSS_CALL_BACK_DATA*  poke_call_back_data;  // poke_wkと対応しており、poke_wkがNULLのときこれもNULL

#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
  POKE_VEC_F32               pos[POKE_ARRANGE_MAX];  // ポケモンの配置
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
}
POKE_MCSS_WORK;

// 違う姿
typedef struct
{
  LOOKS_DIFF looks_diff;
  COLOR_DIFF color_diff;
  u16        other_diff;   // LOOKS_DIFF_ONEのとき、使用しない
                           // LOOKS_DIFF_SEXのとき、SEX_MALE or SEX_FEMALE
                           // LOOKS_DIFF_FORMのとき、formno

  // ZUKANSAVE_CheckPokeSeeFormに与えた引数
  u8         sex;          // PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN  // prog/include/poke_tool/poke_tool.h
  u8         rare;
  u16        form;
}
DIFF_INFO;


//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // 他のところから借用するもの
  GFL_CLUNIT*                 clunit;
  GFL_FONT*                   font;
  //PRINT_QUE*                  print_que;  // 専用のprint_queを用意することにした

  // ここで作成するもの
  MCSS_SYS_WORK*              mcss_sys_wk;
  POKE_MCSS_WORK              poke_mcss_wk[POKE_MAX];
#ifdef DEF_MCSS_TCBSYS
  void*                       mcss_tcbsys_wk;
  GFL_TCBSYS*                 mcss_tcbsys;
#endif
  // FRONT or BACK
  BOOL                        is_poke_front;  // TRUEのときFRONTを表示中、FALSEのときBACKを表示中

  ARCHANDLE*                  pokeicon_handle;
  u32                         pokeicon_ncg[OBJ_SWAP_MAX];    // pokeicon_clwk[i]がNULLでないときpokeicon_ncg[i]は有効
  u32                         pokeicon_ncl;
  u32                         pokeicon_nce;
  GFL_CLWK*                   pokeicon_clwk[OBJ_SWAP_MAX];   // 使っていないときはpokeicon_clwk[i]をNULLにしておく
  OBJ_SWAP                    pokeicon_swap_curr;            // 今表示しているpokeicon_clwkはpokeicon_clwk[pokeicon_swap_curr]

  DIFF_INFO                   diff_info_list[DIFF_MAX];  // ここには今のフォルムも含んでいる
  u16                         diff_num;  // 最大DIFF_MAX
  u16                         diff_sugata_num;  // diff_num = diff_sugata_num + diff_irochigai_num
  u16                         diff_irochigai_num;
  u16                         diff_curr_no;  // diff_info_list[diff_curr_no]が今のフォルム
  u16                         diff_comp_no;  // diff_info_list[diff_comp_no]が比較フォルム

  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_trans[TEXT_MAX];  // 転送する必要がある場合TRUE
  PRINT_QUE*                  print_que[TEXT_MAX];
  GFL_MSGDATA*                text_msgdata;

  u16*                        next_form_pos_list;  // 次のフォルムデータの位置を参照できるリスト
  GFL_MSGDATA*                form_msgdata;

  STRBUF*                     text_frequent_strbuf[TEXT_FREQUENT_MAX];

  GFL_ARCUTIL_TRANSINFO       panel_s_tinfo;

  ZKNDTL_COMMON_REAR_WORK*    rear_wk_m;
  ZKNDTL_COMMON_REAR_WORK*    rear_wk_s;

  // OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド
  u32                         obj_res[OBJ_RES_MAX];
  GFL_CLWK*                   obj_clwk[OBJ_MAX];
  BUTTON                      button[BUTTON_OBJ_MAX];
  BUTTON_OBJ                  push_button;  // 現在押されているボタンがあるときBUTTON_OBJ_NONE以外となる。
  BOOL                        bar_cursor_move_by_key;    // TRUEのとき、キーでバーカーソルを動かした
  BOOL                        bar_cursor_move_by_touch;  // TRUEのとき、タッチでバーカーソルを動かし中

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;

  // フェード
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_m;
  ZKNDTL_COMMON_FADE_WORK*    fade_wk_s;
  // パレットフェード
  ZKNDTL_COMMON_PF_WORK*      pf_wk;

  // 終了命令
  END_CMD                     end_cmd;

  // 入力可不可
  BOOL                        input_enable;  // 入力可のときTRUE

  // 状態
  STATE                       state;

  // 入れ替え
  IREKAE_STATE                irekae_state;
  int                         irekae_theta;  // FX_SinIdxの引数

  // 押し出し
  OSHIDASHI_STATE             oshidashi_state;
  OSHIDASHI_DIRECT            oshidashi_direct;

  // 階層変更  // TOP_TO_EXCHANGE or EXCHANGE_TO_TOP
  BOOL                        kaisou_curr_end;  // TRUEのとき、currは所定の位置に到着している
  BOOL                        kaisou_comp_end;  // TRUEのとき、compは所定の位置に到着している

#ifdef DEF_SCMD_CHANGE
  // タッチバーの左右アイコンでフォルムを変更するときだけ使用する変数
  BOOL                        change_form_by_cur_l_r_flag;  // TRUEのとき、タッチバーの左右アイコンでフォルムを変更する
                                                            // (bar_cursor_move_by_keyと同じタイミングで初期化される。1フレームだけTRUEになる。)

  // タッチバーの入れ替えアイコンでcurrとcompのフォルムを入れ替えるときだけ使用する変数
  BOOL                        irekae_by_exchange_flag;         // TRUEのとき、タッチバーの入れ替えアイコンでフォルムを入れ替えている
  BOOL                        irekae_by_exchange_poke_finish;  // ポケモンのフォルムの入れ替えが完了していたらTRUE
  BOOL                        irekae_by_exchange_icon_finish;  // タッチバーの入れ替えアイコンのアニメが完了していたらTRUE
#endif
}
ZUKAN_DETAIL_FORM_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk );

// テキスト
static void Zukan_Detail_Form_CreateTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_DeleteTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_InitPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ExitPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_MainPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_MainText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn, u8 i );
static void Zukan_Detail_Form_WriteUpText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_WritePokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u8 i, PRINT_QUE* print_que, GFL_BMPWIN* bmpwin, BOOL* trans_flag, u16 diff_no );
static void Zukan_Detail_Form_WritePokeCurrText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_WritePokeCompText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ScrollPokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド  // トップ画面でフォルムを切り替えるための矢印
static void Zukan_Detail_Form_ObjBaseCreate( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ObjBaseDelete( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

static BUTTON_OBJ Zukan_Detail_Form_ObjButtonCheckPush( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ObjButtonArrowSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ObjButtonMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

static void Zukan_Detail_Form_ObjFieldSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

static void Zukan_Detail_Form_ObjBarSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ObjBarMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static BOOL Zukan_Detail_Form_ObjBarMainTouch( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static BOOL Zukan_Detail_Form_ObjBarMainKey( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_ObjBarGetPosX( u16 num, u16 no, u8* a_pos_x_min, u8* a_pos_x_center, u8* a_pos_x_max );
static u16 Zukan_Detail_Form_ObjBarGetNo( u16 num, u8 x );
static u16 Zukan_Detail_Form_GetNoExceptOne( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                             u16 except_idx, u16 target_idx );
static u16 Zukan_Detail_Form_GetDiffInfoListIdx( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                                 u16 except_idx, u16 no );

// MCSSポケモン
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                      u32 personal_rnd );  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される

static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk );
// マクロ
#define BLOCK_POKE_EXIT(sys,wk)                     \
    {                                               \
      if( wk ) PokeExit( sys, wk );                 \
      wk = NULL;                                    \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk );
static void PokeAdjustOfsPosX( MCSS_WORK* poke_wk );
static void PokeGetCompareRelativePosition( MCSS_WORK* poke_wk, VecFx32* pos );

static void PokeMcssWorkInit( POKE_MCSS_WORK* poke_mcss_wk, HEAPID heap_id,
                              MCSS_SYS_WORK* mcss_sys_wk,
                              int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                              u32 personal_rnd );  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
static void PokeMcssWorkExit( POKE_MCSS_WORK* poke_mcss_wk, MCSS_SYS_WORK* mcss_sys_wk );
static void PokeMcssCallBackDataInit( POKE_MCSS_CALL_BACK_DATA* poke_mcss_call_back_data,
                                      POKE_INDEX poke_idx, ZUKAN_DETAIL_FORM_WORK* work );

static void PokeMcssAnimeStart( POKE_MCSS_WORK* poke_mcss_wk );
#ifdef DEF_MINIMUM_LOAD
static void PokeMcssAnimeStop( POKE_MCSS_WORK* poke_mcss_wk );
#endif
static void PokeMcssAnimeMain( POKE_MCSS_WORK* poke_mcss_wk );
static void Zukan_Detail_Form_PokeMcssCallBackFunctor( u32 data, fx32 currentFrame );

static void Zukan_Detail_Form_FlipFrontBack( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケアイコン
static void PokeiconBaseInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void PokeiconBaseExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static GFL_CLWK* PokeiconInit( ARCHANDLE* pokeicon_handle, u32* ncg, u32 ncl, u32 nce, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, u32 sex, BOOL egg );
static void PokeiconExit( u32 ncg, GFL_CLWK* clwk );
// マクロ
#define BLOCK_POKEICON_EXIT(ncg,clwk)                     \
    {                                                     \
      if( clwk ) PokeiconExit( ncg, clwk );               \
      clwk = NULL;                                        \
    }
// NULLを代入し忘れないようにマクロを使うようにしておく

// 違う姿情報を取得する
static void Zukan_Detail_Form_GetDiffInfoWithoutWork( ZUKAN_DETAIL_FORM_PARAM* param, ZKNDTL_COMMON_WORK* cmn,
                                                      DIFF_INFO*   a_diff_info_list,  // 要素数a_diff_info_list[DIFF_MAX]で
                                                      u16*         a_diff_num,
                                                      u16*         a_diff_sugata_num,
                                                      u16*         a_diff_irochigai_num,
                                                      u16*         a_diff_curr_no,
                                                      u16*         a_diff_comp_no );
static void Zukan_Detail_Form_GetDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 違う姿情報からMCSSポケモンを生成する
static void Zukan_Detail_Form_PokeInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                POKE_INDEX poke_f, POKE_INDEX poke_b, POKE_POS_INDEX pos, u16 diff_no );
// 違う姿情報からポケアイコンを生成する
static void Zukan_Detail_Form_PokeiconInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16 diff_no );
// 違う姿情報からポケモンの姿の説明STRBUFを得る
static void Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STRBUF** a_strbuf0, STRBUF** a_strbuf1, BOOL* a_can_delete_strbuf0, BOOL* a_can_delete_strbuf1, u16 diff_no );
                // *a_can_delete_strbuf0がTRUEのとき呼び出し元でa_strbuf0を削除すべき

// ポケモン変更
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

#ifdef DEF_MINIMUM_LOAD
// ポケモンの前後を変更した際に、変更後のMCSSが存在しなかったら作成する
static void Zukan_Detail_Form_MakePokeWhenFlipFrontBack( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
// ポケモンのアニメーションを再生する際に、今のフォルム(前向き)、今のフォルム(後向き)、比較フォルム(前向き)、比較フォルム(後向き)のどれかがなかったら作成する
static void Zukan_Detail_Form_MakePokeWhenAnimeStart( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
#endif

// 比較フォルム変更
static void Zukan_Detail_Form_ChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// ポケアイコン変更
static void Zukan_Detail_Form_ChangePokeicon( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 入力
static void Zukan_Detail_Form_Input( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 状態を遷移させる
static void Zukan_Detail_Form_ChangeState( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STATE state );

// 入れ替え
static void Zukan_Detail_Form_IrekaeMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 押し出し
static void Zukan_Detail_Form_OshidashiChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_OshidashiSetPosCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_OshidashiMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// 階層変更  // TOP_TO_EXCHANGE or EXCHANGE_TO_TOP
static void Zukan_Detail_Form_KaisouMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
#ifdef DEF_MINIMUM_LOAD
static void Zukan_Detail_Form_KaisouChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
#endif

// currとcompの番号、ポインタ入れ替え
static void Zukan_Detail_Form_SwapCurrComp( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );

// アルファ設定
static void Zukan_Detail_Form_AlphaInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_AlphaExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn );


#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
// ポケモンの配置をセットアップする
static void PokeSetupArrange( POKE_VEC_F32* pos,  // pos[POKE_ARRANGE_MAX]となっていること
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                      u32 personal_rnd );  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
// ポケモンの配置をVecFx32で得る
static void PokeMcssGetArrangePosFx32FromPokeArrange( POKE_MCSS_WORK* poke_mcss_wk, POKE_ARRANGE a, VecFx32* pos );
static void PokeMcssGetArrangePosFx32FromPokePosIndex( POKE_MCSS_WORK* poke_mcss_wk, POKE_POS_INDEX i, VecFx32* pos );
// ポケモンの配置をPOKE_VEC_F32で得る
static void PokeMcssGetArrangePosF32FromPokeArrange( POKE_MCSS_WORK* poke_mcss_wk, POKE_ARRANGE a, POKE_VEC_F32* pos );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd );
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn );

const ZKNDTL_PROC_DATA    ZUKAN_DETAIL_FORM_ProcData =
{
  Zukan_Detail_Form_ProcInit,
  Zukan_Detail_Form_ProcMain,
  Zukan_Detail_Form_ProcExit,
  Zukan_Detail_Form_CommandFunc,
  Zukan_Detail_Form_Draw3DFunc,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           
 *
 *  @param[in,out]   
 *
 *  @retval          
 */
//------------------------------------------------------------------
void             ZUKAN_DETAIL_FORM_InitParam(
                            ZUKAN_DETAIL_FORM_PARAM*  param,
                            HEAPID                    heap_id )
{
  param->heap_id = heap_id;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcInit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work;

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcInit\n" );
  }
#endif

  // ヒープ
  {
    work = ZKNDTL_PROC_AllocWork(proc, sizeof(ZUKAN_DETAIL_FORM_WORK), param->heap_id);
    GFL_STD_MemClear( work, sizeof(ZUKAN_DETAIL_FORM_WORK) ); 
  }

  // 他のところから借用するもの
  {
    ZUKAN_DETAIL_GRAPHIC_WORK* graphic = ZKNDTL_COMMON_GetGraphic(cmn);
    work->clunit      = ZUKAN_DETAIL_GRAPHIC_GetClunit(graphic);
    work->font        = ZKNDTL_COMMON_GetFont(cmn);
    //work->print_que   = ZKNDTL_COMMON_GetPrintQue(cmn);  // 専用のprint_queを用意することにした
  }

  // NULL初期化
  {
    u8 i;
    for( i=0; i<POKE_MAX; i++ )
    {
      work->poke_mcss_wk[i].poke_wk = NULL;
      work->poke_mcss_wk[i].poke_call_back_data = NULL;
    }

    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      work->pokeicon_clwk[i] = NULL;  // NULLで初期化
    }
    work->pokeicon_swap_curr = OBJ_SWAP_0;
  }
  // FRONT or BACKの初期化
  work->is_poke_front = TRUE;

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Zukan_Detail_Form_VBlankFunc, work, 1 );

  // テキスト
  Zukan_Detail_Form_InitPrintQue( param, work, cmn );

  // フェード
  {
    work->fade_wk_m = ZKNDTL_COMMON_FadeInit( param->heap_id );
    work->fade_wk_s = ZKNDTL_COMMON_FadeInit( param->heap_id );
   
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );

    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
    ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
  }
  // パレットフェード
  {
    work->pf_wk = ZKNDTL_COMMON_PfInit( param->heap_id );
  }

  // 終了情報
  work->end_cmd = END_CMD_NONE;

  // 入力可不可
  work->input_enable = TRUE;

  // 状態
  work->state = STATE_TOP;

#ifdef DEF_SCMD_CHANGE
  // タッチバーの左右アイコンでフォルムを変更するときだけ使用する変数
  work->change_form_by_cur_l_r_flag = FALSE;

  // タッチバーの入れ替えアイコンでcurrとcompのフォルムを入れ替えるときだけ使用する変数
  work->irekae_by_exchange_flag        = FALSE;
  work->irekae_by_exchange_poke_finish = TRUE;
  work->irekae_by_exchange_icon_finish = TRUE;
#endif

  return ZKNDTL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcExit( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;
  
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  // 最背面
  ZKNDTL_COMMON_RearExit( work->rear_wk_s );
  ZKNDTL_COMMON_RearExit( work->rear_wk_m );

  // BGパネル 
  ZKNDTL_COMMON_PanelDelete(
           BG_FRAME_S_PANEL,
           work->panel_s_tinfo );

  // OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド
  Zukan_Detail_Form_ObjBaseDelete( param, work, cmn );
  
  // ポケアイコン
  {
    u8 i;
    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      // NULLチェックはマクロ内で行っている
      BLOCK_POKEICON_EXIT( work->pokeicon_ncg[i], work->pokeicon_clwk[i] )
    }
  }
  
  // ポケアイコンの不変物
  PokeiconBaseExit( param, work, cmn );

  // テキスト
  Zukan_Detail_Form_DeleteTextBase( param, work, cmn );

  // この画面ではタッチバーのプライオリティを3D面のプライオリティより下げておいたので、元に戻しておく
  ZUKAN_DETAIL_TOUCHBAR_SetBgPriority( touchbar, ZKNDTL_BG_FRAME_PRI_M_TOUCHBAR );

  // パレットフェード
  {
    ZKNDTL_COMMON_PfExit( work->pf_wk );
  }
  // フェード
  {
    ZKNDTL_COMMON_FadeExit( work->fade_wk_s );
    ZKNDTL_COMMON_FadeExit( work->fade_wk_m );
  }

  // テキスト
  Zukan_Detail_Form_ExitPrintQue( param, work, cmn );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // ヒープ
  ZKNDTL_PROC_FreeWork( proc );

#ifdef DEBUG_KAWADA
  {
    OS_Printf( "Zukan_Detail_Form_ProcExit\n" );
  }
#endif

  return ZKNDTL_PROC_RES_FINISH;
}


//-------------------------------------
/// PROC 主処理
//=====================================
static ZKNDTL_PROC_RESULT Zukan_Detail_Form_ProcMain( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

  ZUKAN_DETAIL_GRAPHIC_WORK*   graphic  = ZKNDTL_COMMON_GetGraphic(cmn);

  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
 
  switch(*seq)
  {
  case SEQ_START:
    {
      *seq = SEQ_FADE_CHANGE_BEFORE;

/*
ここで3Dに切り替えるのはまだ早い。ここより前の画面の3Dがワイヤーフレームとして表示されてしまうので。
      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );
      }
*/

      // BG
      {
        // クリア
        u8 i;
        for( i=GFL_BG_FRAME0_M; i<=GFL_BG_FRAME3_S; i++ )
        {
          if(    i != ZKNDTL_BG_FRAME_M_TOUCHBAR
              && i != ZKNDTL_BG_FRAME_S_HEADBAR )
          {
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_X_SET, 0 );
            GFL_BG_SetScroll( i, GFL_BG_SCROLL_Y_SET, 0 );
            GFL_BG_ClearFrame(i);
          }
        }

        // メインBG
        GFL_BG_SetPriority( BG_FRAME_M_POKE,  BG_FRAME_PRI_M_POKE );
        GFL_BG_SetPriority( BG_FRAME_M_TEXT,  BG_FRAME_PRI_M_TEXT );
        GFL_BG_SetPriority( BG_FRAME_M_REAR,  BG_FRAME_PRI_M_REAR );
        
        // サブBG
        GFL_BG_SetPriority( BG_FRAME_S_TEXT,  BG_FRAME_PRI_S_TEXT  );
        GFL_BG_SetPriority( BG_FRAME_S_REAR,  BG_FRAME_PRI_S_REAR  );
        GFL_BG_SetPriority( BG_FRAME_S_PANEL, BG_FRAME_PRI_S_PANEL );

        // この画面ではタッチバーのプライオリティを3D面のプライオリティより下げておく
        ZUKAN_DETAIL_TOUCHBAR_SetBgPriority( touchbar, BG_FRAME_PRI_M_TOUCHBAR );
      }

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );
      }

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Init3D( graphic, param->heap_id );

      // MCSSポケモン
      Zukan_Detail_Form_McssSysInit( param, work, cmn );

      // テキスト
      Zukan_Detail_Form_CreateTextBase( param, work, cmn );

      // OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド
      Zukan_Detail_Form_ObjBaseCreate( param, work, cmn );
     
      // 違う姿情報を取得する
      Zukan_Detail_Form_GetDiffInfo( param, work, cmn );
    
      // ポケアイコンの不変物
      PokeiconBaseInit( param, work, cmn );

      // 最初のポケモンを表示する
      Zukan_Detail_Form_ChangePoke( param, work, cmn );
      // ポケアイコンの設定を変更しておく
      GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[work->pokeicon_swap_curr], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので

      // ポケモンを非表示にし黒にする
      MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
      MCSS_SetPaletteFade( work->poke_mcss_wk[POKE_CURR_F].poke_wk, 16, 16, 0, 0x0000 );
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
        MCSS_SetPaletteFade( work->poke_mcss_wk[POKE_COMP_F].poke_wk, 16, 16, 0, 0x0000 );
      }

      // BGパネル 
      work->panel_s_tinfo = ZKNDTL_COMMON_PanelCreate(
                              ZKNDTL_COMMON_PANEL_CREATE_TYPE_TRANS_PAL_CHARA,
                              param->heap_id,
                              BG_FRAME_S_PANEL,
                              BG_PAL_NUM_S_PANEL,
                              BG_PAL_POS_S_PANEL,
                              0,
                              ARCID_ZUKAN_GRA,
                              NARC_zukan_gra_info_info_bgu_NCLR,
                              NARC_zukan_gra_info_info_bgu_NCGR,
                              NARC_zukan_gra_info_formewin_bgu_NSCR,
                              0 );

      // 最背面
      work->rear_wk_m = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_FORM,
          BG_FRAME_M_REAR, BG_PAL_POS_M_REAR +0, BG_PAL_POS_M_REAR +1 );
      work->rear_wk_s = ZKNDTL_COMMON_RearInit( param->heap_id, ZKNDTL_COMMON_REAR_TYPE_FORM,
          BG_FRAME_S_REAR, BG_PAL_POS_S_REAR +0, BG_PAL_POS_S_REAR +1 );

      // パレットフェード
      {
        ZKNDTL_COMMON_PfSetPaletteDataFromVram( work->pf_wk );
        ZKNDTL_COMMON_PfSetBlackImmediately( work->pf_wk );
      }
    }
    break;
  case SEQ_FADE_CHANGE_BEFORE:
    {
      *seq = SEQ_PREPARE_0;

      // フェード
      ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetColorlessImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
     
      // ZKNDTL_COMMON_FadeSetColorlessImmediatelyでG2_SetBlendBrightnessExtやG2S_SetBlendBrightnessExtを設定しているので、
      // その後でZukan_Detail_Form_AlphaInitを呼ぶこと
      Zukan_Detail_Form_AlphaInit( param, work, cmn );
    }
    break;
  case SEQ_PREPARE_0:
    {
      // MCSSポケモン
      // ポケモンが黒になるのを待つために、1フレーム余計に待つ
      *seq = SEQ_PREPARE;
    }
    break;
  case SEQ_PREPARE:
    {
      // MCSSポケモン
      // ポケモンが黒になるのを待つ
      BOOL poke_fade = TRUE;
      if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[POKE_CURR_F].poke_wk ) )
      {
        poke_fade = FALSE;
      }
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[POKE_COMP_F].poke_wk ) )
        {
          poke_fade = FALSE;
        }
      }

      if( poke_fade )
      {
        *seq = SEQ_FADE_IN;

        //// フェード
        //ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_m );
        //ZKNDTL_COMMON_FadeSetBlackToColorless( work->fade_wk_s );

        // パレットフェード
        {
          ZKNDTL_COMMON_PfSetBlackToColorless( work->pf_wk );
        }

        // タッチバー
        if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) != ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR )
        {
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM,
              (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE );
          ZUKAN_DETAIL_TOUCHBAR_Appear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }
        else
        {
          ZUKAN_DETAIL_TOUCHBAR_SetDispOfGeneral(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
        }
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので
        {
          GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
          ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
              touchbar,
              GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_FORM ) );
        }
        // タイトルバー
        if( ZUKAN_DETAIL_HEADBAR_GetState( headbar ) != ZUKAN_DETAIL_HEADBAR_STATE_APPEAR )
        {
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM );
          ZUKAN_DETAIL_HEADBAR_Appear( headbar );
        }

        // MCSSポケモン
        // ポケモンを表示しフェードイン(黒→カラー)
        MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
        MCSS_SetPaletteFade( work->poke_mcss_wk[POKE_CURR_F].poke_wk, 16, 0, POKE_MCSS_FADE_WAIT, 0x0000 );
        if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        {
          MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
          MCSS_SetPaletteFade( work->poke_mcss_wk[POKE_COMP_F].poke_wk, 16, 0, POKE_MCSS_FADE_WAIT, 0x0000 );
        }
      }
    }
    break;
  case SEQ_FADE_IN:
    {
      // MCSSポケモン
      // ポケモンがカラーになるのを待つ
      BOOL poke_fade = TRUE;
      if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[POKE_CURR_F].poke_wk ) )
      {
        poke_fade = FALSE;
      }
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[POKE_COMP_F].poke_wk ) )
        {
          poke_fade = FALSE;
        }
      }

      if( //   (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          //&& (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
             (!ZKNDTL_COMMON_PfIsExecute( work->pf_wk ))
          && ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_APPEAR
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_APPEAR
          && poke_fade )
      {
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );  // ZUKAN_DETAIL_TOUCHBAR_SetTypeのときはUnlock状態なので

        //Zukan_Detail_Form_AlphaInit( param, work, cmn );

        *seq = SEQ_MAIN;
      }
    }
    break;
  case SEQ_MAIN:
    {
      if( work->end_cmd != END_CMD_NONE )
      {
        *seq = SEQ_POST;

        // ポケモンをカラーにする
        {
          u8 i;
          for( i=0; i<POKE_MAX; i++ )
          {
            if( work->poke_mcss_wk[i].poke_wk )
              MCSS_SetPaletteFade( work->poke_mcss_wk[i].poke_wk, 0, 0, 0, 0x0000 );
          }
        }
      }
      else
      {
        // 入力
        Zukan_Detail_Form_Input( param, work, cmn );
      }
    }
    break;
  case SEQ_POST:
    {
      // MCSSポケモン
      // ポケモンがカラーになるのを待つ
      BOOL poke_fade = TRUE;
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          if( work->poke_mcss_wk[i].poke_wk )
          {
            if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[i].poke_wk ) )
            {
              poke_fade = FALSE;
              break;
            }
          }
        }
      }

      if( poke_fade )
      {
        //Zukan_Detail_Form_AlphaExit( param, work, cmn );

        *seq = SEQ_FADE_OUT;

        //// フェード
        //ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_m );
        //ZKNDTL_COMMON_FadeSetColorlessToBlack( work->fade_wk_s );
       
        // パレットフェード
        {
          ZKNDTL_COMMON_PfSetColorlessToBlack( work->pf_wk );
        }

        // タイトルバー
        ZUKAN_DETAIL_HEADBAR_Disappear( headbar );
        
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_Disappear( touchbar, ZUKAN_DETAIL_TOUCHBAR_SPEED_OUTSIDE );
        }

        // MCSSポケモン
        // ポケモンを表示しフェードアウト(カラー→黒)
        {
          u8 i;
          for( i=0; i<POKE_MAX; i++ )
          {
            if( work->poke_mcss_wk[i].poke_wk )
            {
              MCSS_SetPaletteFade( work->poke_mcss_wk[i].poke_wk, 0, 16, POKE_MCSS_FADE_WAIT, 0x0000 );
            }
          }
        }
      }
    }
    break;
  case SEQ_FADE_OUT:
    {
      BOOL b_next_seq = FALSE;

      // MCSSポケモン
      // ポケモンが黒になるのを待つ
      BOOL poke_fade = TRUE;
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          if( work->poke_mcss_wk[i].poke_wk )
          {
            if( MCSS_CheckExecutePaletteFade( work->poke_mcss_wk[i].poke_wk ) )
            {
              poke_fade = FALSE;
              break;
            }
          }
        }
      }

      if( //   (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_m ))
          //&& (!ZKNDTL_COMMON_FadeIsExecute( work->fade_wk_s ))
             (!ZKNDTL_COMMON_PfIsExecute( work->pf_wk ))
          && ZUKAN_DETAIL_HEADBAR_GetState( headbar ) == ZUKAN_DETAIL_HEADBAR_STATE_DISAPPEAR
          && poke_fade )
      {
        if( work->end_cmd == END_CMD_OUTSIDE )
        {
          if( ZUKAN_DETAIL_TOUCHBAR_GetState( touchbar ) == ZUKAN_DETAIL_TOUCHBAR_STATE_DISAPPEAR )
          {
            b_next_seq = TRUE;
          }
        }
        else
        {
          b_next_seq = TRUE;
        }
      }

      if( b_next_seq )
      {
        *seq = SEQ_FADE_CHANGE_AFTER;
      }
    }
    break;
  case SEQ_FADE_CHANGE_AFTER:
    {
      *seq = SEQ_END;

      // ZKNDTL_COMMON_FadeSetColorlessImmediatelyでG2_SetBlendBrightnessExtやG2S_SetBlendBrightnessExtを設定しているので、
      // その前にZukan_Detail_Form_AlphaExitを呼ぶこと
      Zukan_Detail_Form_AlphaExit( param, work, cmn );
      
      // フェード
      ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_M, work->fade_wk_m );
      ZKNDTL_COMMON_FadeSetBlackImmediately( ZKNDTL_COMMON_FADE_DISP_S, work->fade_wk_s );
    }
    break;
  case SEQ_END:
    {
      // MCSSポケモン
      {
        u8 i;
        for( i=0; i<POKE_MAX; i++ )
        {
          //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[i].poke_wk )
          PokeMcssWorkExit( &work->poke_mcss_wk[i], work->mcss_sys_wk );
        }
      }
      Zukan_Detail_Form_McssSysExit( param, work, cmn );

      // 3D
      ZUKAN_DETAIL_GRAPHIC_Exit3D( graphic );

      // 切り替え
      {
        // グラフィックスモード設定
        GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
      }

      return ZKNDTL_PROC_RES_FINISH;
    }
    break;
  }

  // MCSSポケモン
  if( work->mcss_sys_wk )
  {
    u8 i;
    
#ifdef DEF_MCSS_TCBSYS
    GFL_TCB_Main( work->mcss_tcbsys );  // DEF_MCSS_TCBSYSが定義されておりDEF_MINIMUM_LOADが定義されていないとき、
                                        // GFL_TCB_Main, MCSS_Mainの順にしておかないと、メモリ解放忘れが出てしまう。
#endif

    MCSS_Main( work->mcss_sys_wk );
    
    for( i=0; i<POKE_MAX; i++ )  // MCSS_Main, PokeMcssAnimeMainの順じゃないと、アニメが1フレーム余計に進んでしまう。
    {
      PokeMcssAnimeMain( &work->poke_mcss_wk[i] );  // この関数内でNULLチェックはしてくれている
    }

//#ifdef DEF_MCSS_TCBSYS
//    GFL_TCB_Main( work->mcss_tcbsys );  // DEF_MCSS_TCBSYSが定義されておりDEF_MINIMUM_LOADが定義されていないとき、
//                                        // GFL_TCB_Main, MCSS_Mainの順にしておかないと、メモリ解放忘れが出てしまう。
//#endif
//    
//    MCSS_Main( work->mcss_sys_wk );

  } 

  if( *seq >= SEQ_PREPARE )
  {
    // 前後ボタン、再生ボタン
    Zukan_Detail_Form_ObjButtonMain( param, work, cmn );
 
    // 入れ替え 
    Zukan_Detail_Form_IrekaeMain( param, work, cmn );  // Zukan_Detail_Form_IrekaeMainでスクロールバーを更新する命令を出しているので、↓つづく

    // スクロールバー
    Zukan_Detail_Form_ObjBarMain( param, work, cmn );  // つづき↑必ずZukan_Detail_Form_IrekaeMain→Zukan_Detail_Form_ObjBarMainの順で。

    // 押し出し
    Zukan_Detail_Form_OshidashiMain( param, work, cmn );

    // 階層変更  // TOP_TO_EXCHANGE or EXCHANGE_TO_TOP
    Zukan_Detail_Form_KaisouMain( param, work, cmn );

    // 最背面
    ZKNDTL_COMMON_RearMain( work->rear_wk_m );
    ZKNDTL_COMMON_RearMain( work->rear_wk_s );
  }

  // フェード
  ZKNDTL_COMMON_FadeMain( work->fade_wk_m, work->fade_wk_s );
  // パレットフェード
  {
    ZKNDTL_COMMON_PfMain( work->pf_wk );
  }

  // テキスト
  Zukan_Detail_Form_MainPrintQue( param, work, cmn );




#ifdef DEBUG_POKE_POS_SET
  ////////////////////////////////////////////////////////////////
  {
    const f32 add = 0.01f;//0.01f;
    f32 add_x = 0.0f;
    f32 add_y = 0.0f;
    f32 add_z = 0.0f;
    u32 x, y;
    BOOL input = FALSE;
    u8 i;
    
    if(    ( GFL_UI_TP_GetPointCont(&x,&y) )
        && ( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ) )
    {
      if( y<48 )
      {
        add_y = add;
        //add_z = add;
        input = TRUE;
      }
      else if( y>=144 )
      {
        add_y = -add;
        //add_z = -add;
        input = TRUE;
      }
      else if( x<64 )
      {
        add_x = -add;
        input = TRUE;
      }
      else if( x>=192 )
      {
        add_x = add;
        input = TRUE;
      }
    }

    if( input )
    {
      for( i=0; i<POKE_MAX; i++ )
      {
        if( work->poke_mcss_wk[i].poke_wk )
        {
          VecFx32 pos;
          f32     pos_x, pos_y, pos_z;
          MCSS_GetPosition( work->poke_mcss_wk[i].poke_wk, &pos );
          //MCSS_GetScale( work->poke_mcss_wk[i].poke_wk, &pos );
          pos_x = FX_FX32_TO_F32(pos.x);
          pos_y = FX_FX32_TO_F32(pos.y);
          pos_z = FX_FX32_TO_F32(pos.z);
          pos_x += add_x;
          pos_y += add_y;
          pos_z += add_z;
          OS_Printf( "[%d] poke_wk (%f,%f,%f)\n", i, pos_x, pos_y, pos_z );
          pos.x = FX_F32_TO_FX32(pos_x);
          pos.y = FX_F32_TO_FX32(pos_y);
          pos.z = FX_F32_TO_FX32(pos_z);
          MCSS_SetPosition( work->poke_mcss_wk[i].poke_wk, &pos );
          //MCSS_SetScale( work->poke_mcss_wk[i].poke_wk, &pos );
        }
      }
    }
  }
  ////////////////////////////////////////////////////////////////
#endif




  return ZKNDTL_PROC_RES_CONTINUE;
}

//-------------------------------------
/// PROC 命令処理
//=====================================
static void Zukan_Detail_Form_CommandFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn, ZKNDTL_COMMAND cmd )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

    BOOL b_valid_cmd = FALSE;  // cmdがZKNDTL_CMD_NONE以外のZKNDTL_CMD_???のときTRUE。ZKNDTL_CMD_NONEやZKNDTL_SCMD_???のときFALSE。

    work->bar_cursor_move_by_key = FALSE;

#ifdef DEF_SCMD_CHANGE    
    work->change_form_by_cur_l_r_flag = FALSE;
#endif

    // 入力不可
    switch( cmd )
    {
    case ZKNDTL_SCMD_CLOSE:
    case ZKNDTL_SCMD_RETURN:
    case ZKNDTL_SCMD_INFO:
    case ZKNDTL_SCMD_MAP:
    case ZKNDTL_SCMD_VOICE:
    case ZKNDTL_SCMD_CUR_D:
    case ZKNDTL_SCMD_CUR_U:
    case ZKNDTL_SCMD_CHECK:
    case ZKNDTL_SCMD_FORM_CUR_D:
    case ZKNDTL_SCMD_FORM_CUR_U:
    case ZKNDTL_SCMD_FORM_CUR_R:
    case ZKNDTL_SCMD_FORM_CUR_L:
    case ZKNDTL_SCMD_FORM_EXCHANGE:
      {
        work->input_enable = FALSE;
      }
      break;
    }
    // 入力可
    switch( cmd )
    {
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
    case ZKNDTL_CMD_CUR_D:
    case ZKNDTL_CMD_CUR_U:
    case ZKNDTL_CMD_CHECK:
    case ZKNDTL_CMD_FORM_CUR_D:
    case ZKNDTL_CMD_FORM_CUR_U:
    case ZKNDTL_CMD_FORM_CUR_R:
    case ZKNDTL_CMD_FORM_CUR_L:
    case ZKNDTL_CMD_FORM_EXCHANGE:
      {
        work->input_enable = TRUE;
        b_valid_cmd = TRUE;
      }
      break;
    }


#ifdef DEF_SCMD_CHANGE
    switch( cmd )
    {
    case ZKNDTL_SCMD_CUR_D:
    case ZKNDTL_SCMD_FORM_CUR_D:
      {
        if( work->state == STATE_TOP )
        {
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToNextPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
          }
        }
        else if( work->state == STATE_EXCHANGE )
        {
          DIFF_INFO   diff_info_list[DIFF_MAX];
          u16         diff_num;
          u16         diff_sugata_num;
          u16         diff_irochigai_num;
          u16         diff_curr_no;
          u16         diff_comp_no;
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToNextPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          while( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_GetDiffInfoWithoutWork( param, cmn,
                                                      diff_info_list,
                                                      &diff_num,
                                                      &diff_sugata_num,
                                                      &diff_irochigai_num,
                                                      &diff_curr_no,
                                                      &diff_comp_no );
            if( diff_num >= 2 )
            {
              break;
            }
            else
            {
              ZKNDTL_COMMON_GoToNextPoke(cmn);
              monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
            }
          }
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
            if( diff_num >= 3 )
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
            }
            else
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
            }
          }
        }
      }
      break;
    case ZKNDTL_SCMD_CUR_U:
    case ZKNDTL_SCMD_FORM_CUR_U:
      {
        if( work->state == STATE_TOP )
        {
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToPrevPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
          }
        }
        else if( work->state == STATE_EXCHANGE )
        {
          DIFF_INFO   diff_info_list[DIFF_MAX];
          u16         diff_num;
          u16         diff_sugata_num;
          u16         diff_irochigai_num;
          u16         diff_curr_no;
          u16         diff_comp_no;
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToPrevPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          while( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_GetDiffInfoWithoutWork( param, cmn,
                                                      diff_info_list,
                                                      &diff_num,
                                                      &diff_sugata_num,
                                                      &diff_irochigai_num,
                                                      &diff_curr_no,
                                                      &diff_comp_no );
            if( diff_num >= 2 )
            {
              break;
            }
            else
            {
              ZKNDTL_COMMON_GoToPrevPoke(cmn);
              monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
            }
          }
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
            if( diff_num >= 3 )
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
            }
            else
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
            }
          }
        }
      }
      break;
    case ZKNDTL_SCMD_FORM_CUR_R:
      {
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no++;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no++;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          work->bar_cursor_move_by_key = TRUE;
          work->change_form_by_cur_l_r_flag = TRUE;
        }
      }
      break;
    case ZKNDTL_SCMD_FORM_CUR_L:
      {
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no += work->diff_num -1;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no += work->diff_num -1;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          work->bar_cursor_move_by_key = TRUE;
          work->change_form_by_cur_l_r_flag = TRUE;
        }
      }
      break;


    case ZKNDTL_SCMD_FORM_EXCHANGE:
      {
        if( work->diff_num >= 2 )
        {
          Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE_IREKAE );

          // タッチバーの入れ替えアイコンでcurrとcompのフォルムを入れ替えるときだけ使用する変数
          work->irekae_by_exchange_flag        = TRUE;
          work->irekae_by_exchange_poke_finish = FALSE;
          work->irekae_by_exchange_icon_finish = FALSE;
        }
        else
        {
          // タッチバーの入れ替えアイコンでcurrとcompのフォルムを入れ替えるときだけ使用する変数
          work->irekae_by_exchange_flag        = TRUE;
          work->irekae_by_exchange_poke_finish = TRUE;
          work->irekae_by_exchange_icon_finish = FALSE;
        }
      }
      break;


    }
#endif


    // コマンド
    switch( cmd )
    {
    case ZKNDTL_CMD_NONE:
      {
      }
      break;
    case ZKNDTL_CMD_CLOSE:
    case ZKNDTL_CMD_RETURN:
      {
        work->end_cmd = END_CMD_OUTSIDE;
      }
      break;
    case ZKNDTL_CMD_INFO:
    case ZKNDTL_CMD_MAP:
    case ZKNDTL_CMD_VOICE:
      {
        work->end_cmd = END_CMD_INSIDE;
      }
      break;
    case ZKNDTL_CMD_CUR_D:
    case ZKNDTL_CMD_FORM_CUR_D:
      {
#ifndef DEF_SCMD_CHANGE
        if( work->state == STATE_TOP )
        {
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToNextPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
          }
        }
        else if( work->state == STATE_EXCHANGE )
        {
          DIFF_INFO   diff_info_list[DIFF_MAX];
          u16         diff_num;
          u16         diff_sugata_num;
          u16         diff_irochigai_num;
          u16         diff_curr_no;
          u16         diff_comp_no;
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToNextPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          while( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_GetDiffInfoWithoutWork( param, cmn,
                                                      diff_info_list,
                                                      &diff_num,
                                                      &diff_sugata_num,
                                                      &diff_irochigai_num,
                                                      &diff_curr_no,
                                                      &diff_comp_no );
            if( diff_num >= 2 )
            {
              break;
            }
            else
            {
              ZKNDTL_COMMON_GoToNextPoke(cmn);
              monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
            }
          }
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
            if( diff_num >= 3 )
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
            }
            else
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
            }
          }
        }
#endif

        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CUR_U:
    case ZKNDTL_CMD_FORM_CUR_U:
      {
#ifndef DEF_SCMD_CHANGE
        if( work->state == STATE_TOP )
        {
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToPrevPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
          }
        }
        else if( work->state == STATE_EXCHANGE )
        {
          DIFF_INFO   diff_info_list[DIFF_MAX];
          u16         diff_num;
          u16         diff_sugata_num;
          u16         diff_irochigai_num;
          u16         diff_curr_no;
          u16         diff_comp_no;
          u16 monsno_curr;
          u16 monsno_go;
          monsno_curr = ZKNDTL_COMMON_GetCurrPoke(cmn);
          ZKNDTL_COMMON_GoToPrevPoke(cmn);
          monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
          while( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_GetDiffInfoWithoutWork( param, cmn,
                                                      diff_info_list,
                                                      &diff_num,
                                                      &diff_sugata_num,
                                                      &diff_irochigai_num,
                                                      &diff_curr_no,
                                                      &diff_comp_no );
            if( diff_num >= 2 )
            {
              break;
            }
            else
            {
              ZKNDTL_COMMON_GoToPrevPoke(cmn);
              monsno_go = ZKNDTL_COMMON_GetCurrPoke(cmn);
            }
          }
          if( monsno_curr != monsno_go )
          {
            Zukan_Detail_Form_ChangePoke(param, work, cmn);
            if( diff_num >= 3 )
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
            }
            else
            {
              ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
            }
          }
        }
#endif
        
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_CHECK:
      {
        GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
        GAMEDATA_SetShortCut(
            gamedata,
            SHORTCUT_ID_ZUKAN_FORM,
            ZUKAN_DETAIL_TOUCHBAR_GetCheckFlipOfGeneral( touchbar ) );
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_CUR_R:
      {
#ifndef DEF_SCMD_CHANGE
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no++;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no++;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          work->bar_cursor_move_by_key = TRUE;
        }
#endif
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_CUR_L:
      {
#ifndef DEF_SCMD_CHANGE
        if( work->diff_num >= 3 )
        {
          work->diff_comp_no += work->diff_num -1;
          work->diff_comp_no %= work->diff_num;
          if( work->diff_curr_no == work->diff_comp_no )
          {
            work->diff_comp_no += work->diff_num -1;
            work->diff_comp_no %= work->diff_num;
          }

          Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
          work->bar_cursor_move_by_key = TRUE;
        }
#endif
        ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
      }
      break;
    case ZKNDTL_CMD_FORM_RETURN:
      {
        //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );
        if( work->state == STATE_EXCHANGE )
        {
          Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE_TO_TOP );
        }
      }
      break;
    case ZKNDTL_CMD_FORM_EXCHANGE:
      {
#ifndef DEF_SCMD_CHANGE
        if( work->diff_num >= 2 )
        {
          Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE_IREKAE );
        }
        else
        {
          ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        }
#endif

#ifdef DEF_SCMD_CHANGE
        if( work->irekae_by_exchange_flag )
        {
          work->irekae_by_exchange_icon_finish = TRUE;
          if( work->irekae_by_exchange_poke_finish )
          {
            work->irekae_by_exchange_flag = FALSE;
            ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
          }
        }
#endif
      }
      break;
    default:
      {
        if( b_valid_cmd )
        {
          ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
        }
      }
      break;
    }
  }
}

//-------------------------------------
/// PROC 3D描画
//=====================================
static void Zukan_Detail_Form_Draw3DFunc( ZKNDTL_PROC* proc, int* seq, void* pwk, void* mywk, ZKNDTL_COMMON_WORK* cmn )
{
  if( mywk )
  {
    ZUKAN_DETAIL_FORM_PARAM*    param    = (ZUKAN_DETAIL_FORM_PARAM*)pwk;
    ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)mywk;

    if( work->mcss_sys_wk )
      MCSS_Draw( work->mcss_sys_wk );
  }
}


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Zukan_Detail_Form_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  ZUKAN_DETAIL_FORM_WORK*     work     = (ZUKAN_DETAIL_FORM_WORK*)wk;

  // パレットフェード
  {
    ZKNDTL_COMMON_PfTrans( work->pf_wk );
  }
}

//-------------------------------------
/// 違う姿情報からポケモンの姿の説明STRBUFを得る
    // 呼び出し元で解放すること(NULLのときはなし)
//=====================================
static void Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STRBUF** a_strbuf0, STRBUF** a_strbuf1, BOOL* a_can_delete_strbuf0, BOOL* a_can_delete_strbuf1, u16 diff_no )
                // *a_can_delete_strbuf0がTRUEのとき呼び出し元でa_strbuf0を削除すべき
{
  STRBUF* strbuf0 = NULL;
  STRBUF* strbuf1 = NULL;
  BOOL    can_delete_strbuf0 = FALSE;
  BOOL    can_delete_strbuf1 = FALSE;
  
  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  switch( work->diff_info_list[diff_no].looks_diff )
  {
  case LOOKS_DIFF_ONE:
    {
      WORDSET* wordset;
      STRBUF* src_strbuf;
      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = work->text_frequent_strbuf[TEXT_FREQUENT_POKE_SUGATA];//GFL_MSG_CreateString( work->text_msgdata, ZKN_FORMNAME_10 );
      strbuf0 = GFL_STR_CreateBuffer( STRBUF_POKENAME_LENGTH, param->heap_id );
      WORDSET_RegisterPokeMonsNameNo( wordset, 0, monsno );
      WORDSET_ExpandStr( wordset, strbuf0, src_strbuf );
      //GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );
      can_delete_strbuf0 = TRUE;
    }
    break;
  case LOOKS_DIFF_SEX:
    {
      switch( work->diff_info_list[diff_no].other_diff )
      {
      case SEX_MALE:
        {
          strbuf0 = work->text_frequent_strbuf[TEXT_FREQUENT_MALE_SUGATA];
          can_delete_strbuf0 = FALSE;
        }
        break;
      case SEX_FEMALE:
        {
          strbuf0 = work->text_frequent_strbuf[TEXT_FREQUENT_FEMALE_SUGATA];
          can_delete_strbuf0 = FALSE;
        }
        break;
      }
    }
    break;
  case LOOKS_DIFF_FORM:
    {
      u16 form_no = work->diff_info_list[diff_no].other_diff;

      // 次のフォルムデータの位置を参照できるリストを利用して、フォルムデータの位置を得る
      u16 pos = monsno;
      {
        u16 formno_count = 0;
        while( formno_count != form_no )
        {
          pos = work->next_form_pos_list[pos];
          if( pos == 0 ) break;
          formno_count++;
        }
        GF_ASSERT_MSG( pos > 0, "ZUKAN_DETAIL_FORM : フォルム番号が異常です。\n" );
      }

      strbuf0 = GFL_MSG_CreateString( work->form_msgdata, pos );
      can_delete_strbuf0 = TRUE;
    }
    break;
  }

  switch( work->diff_info_list[diff_no].color_diff )
  {
  case COLOR_DIFF_NONE:
    {
      // 何もしない
    }
    break;
  case COLOR_DIFF_NORMAL:
    {
      // 何もしない
    }
    break;
  case COLOR_DIFF_SPECIAL:
    {
      strbuf1 = work->text_frequent_strbuf[TEXT_FREQUENT_NOTE_IROCHIGAI];
      can_delete_strbuf1 = FALSE;
    }
    break;
  }

  *a_strbuf0 = strbuf0;
  *a_strbuf1 = strbuf1;
  *a_can_delete_strbuf0 = can_delete_strbuf0;
  *a_can_delete_strbuf1 = can_delete_strbuf1;
}

//-------------------------------------
/// テキスト
//=====================================
static void Zukan_Detail_Form_CreateTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 bmpwin_setup[TEXT_MAX][7] =
  {
    { BG_FRAME_S_TEXT, 19, 16,  8,  2, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_S_TEXT, 27, 19,  2,  4, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_S_TEXT,  3, 19, 23,  4, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_M_TEXT,  0,  1, 16,  4, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B },
    { BG_FRAME_M_TEXT, 16,  1, 16,  4, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B },
  };

  // パレット
  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_SUB_BG,
       0,
       BG_PAL_POS_S_TEXT * 0x20,
       BG_PAL_NUM_S_TEXT * 0x20,
       param->heap_id );

  GFL_ARC_UTIL_TransVramPaletteEx( ARCID_FONT, NARC_font_default_nclr,
       PALTYPE_MAIN_BG,
       0,
       BG_PAL_POS_S_TEXT * 0x20,
       BG_PAL_NUM_S_TEXT * 0x20,
       param->heap_id );

  // ビットマップウィンドウ
  {
    u8 i;
    for( i=0; i<TEXT_MAX; i++ )
    {
      work->text_bmpwin[i] = GFL_BMPWIN_Create( bmpwin_setup[i][0],
                                         bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                                         bmpwin_setup[i][5],
                                         bmpwin_setup[i][6] );

      // クリア
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[i] ), 0 );
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
    }
  }

  // メッセージ
  work->text_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_dat,
                           param->heap_id );

  work->form_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL,
                           ARCID_MESSAGE, NARC_message_zkn_form_dat,
                           param->heap_id );

  // 次のフォルムデータの位置を参照できるリスト
  {
    u32 size;
    u16 next_form_pos_num;

    work->next_form_pos_list = GFL_ARC_UTIL_LoadEx( ARCID_ZUKAN_DATA, NARC_zukan_data_zkn_next_form_pos_dat, FALSE, param->heap_id, &size );
    next_form_pos_num = (u16)( size / sizeof(u16) );

    GF_ASSERT_MSG( next_form_pos_num > MONSNO_END+1+1, "ZUKAN_DETAIL_FORM : 次のフォルムのデータの位置を参照できる列の個数が足りません。\n" );
  }

  // 変更されることのない固定テキスト TEXT_UP_LABEL
  {
    STRBUF* strbuf;
    GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_UP_LABEL] );
    
    strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_01 );
    PRINTSYS_PrintQueColor( work->print_que[TEXT_UP_LABEL], bmp_data,
                            0, text_up_label_num_y[0], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_STR_DeleteBuffer( strbuf );

    strbuf = GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_02 );
    PRINTSYS_PrintQueColor( work->print_que[TEXT_UP_LABEL], bmp_data,
                            0, text_up_label_num_y[1], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
    GFL_STR_DeleteBuffer( strbuf );

    work->text_trans[TEXT_UP_LABEL] = TRUE;

    // 既に済んでいるかもしれないので、Mainを1度呼んでおく
    Zukan_Detail_Form_MainText( param, work, cmn, TEXT_UP_LABEL );
  }

  // よく使うテキスト
  {
    u8 i;
    for( i=0; i<TEXT_FREQUENT_MAX; i++ )
    {
      work->text_frequent_strbuf[i] = GFL_MSG_CreateString( work->text_msgdata, text_frequent_str_id[i] );
    }
  }
}
static void Zukan_Detail_Form_DeleteTextBase( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_trans[i] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[i] );
    
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }

  // よく使うテキスト
  {
    u8 i;
    for( i=0; i<TEXT_FREQUENT_MAX; i++ )
    {
      GFL_STR_DeleteBuffer( work->text_frequent_strbuf[i] );
    }
  }

  GFL_MSG_Delete( work->text_msgdata );

  GFL_MSG_Delete( work->form_msgdata );

  // 次のフォルムデータの位置を参照できるリスト
  {
    GFL_HEAP_FreeMemory( work->next_form_pos_list );
  }
}
static void Zukan_Detail_Form_InitPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->print_que[i] = PRINTSYS_QUE_Create( param->heap_id );

    work->text_trans[i] = FALSE;
  }
}
static void Zukan_Detail_Form_ExitPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_trans[i] = FALSE;
    
    PRINTSYS_QUE_Clear( work->print_que[i] );
    PRINTSYS_QUE_Delete( work->print_que[i] );
  }
}
static void Zukan_Detail_Form_MainPrintQue( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;
  for( i=0; i<TEXT_MAX; i++ )
  {
    PRINTSYS_QUE_Main( work->print_que[i] );

    Zukan_Detail_Form_MainText( param, work, cmn, i );
  }
}
static void Zukan_Detail_Form_MainText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn, u8 i )
{
  if( work->text_trans[i] )
  {
    if( !PRINTSYS_QUE_IsExistTarget( work->print_que[i], GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
    {
      GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
      work->text_trans[i] = FALSE;
    }
  }
}
static void Zukan_Detail_Form_WriteUpText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u8 i;

  // 前回のテキストをクリア
  {
    for( i=TEXT_POKENAME; i<=TEXT_UP_NUM; i++ )
    {
      work->text_trans[i] = FALSE;
      PRINTSYS_QUE_Clear( work->print_que[i] );
      
      // クリア
      GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[i] ), 0 );
      //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );  // ちらつくのでコメントアウト
    }
  }

  // 今回のテキストを描画
  {
    u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);
    
    {
      GFL_BMP_DATA*	bmp_data  = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKENAME] );

      STRBUF* strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, monsno );

      u16 str_width = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
      u16 bmp_width = GFL_BMP_GetSizeX( bmp_data );
      u16 x = ( bmp_width - str_width ) / 2;

/*
      u16 str_height = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) );
      u16 bmp_height = GFL_BMP_GetSizeY( bmp_data );
      u16 y = ( bmp_height - str_height ) / 2;
*/
      u16 y = 1;  // 計算で出した位置は中心より上に見えるので

      PRINTSYS_PrintQueColor( work->print_que[TEXT_POKENAME], bmp_data,
                              x, y, strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      work->text_trans[TEXT_POKENAME] = TRUE;

      GFL_STR_DeleteBuffer( strbuf );
    }

    {
      WORDSET* wordset;
      STRBUF* src_strbuf;
      STRBUF* strbuf;
      GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_UP_NUM] );

      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = work->text_frequent_strbuf[TEXT_FREQUENT_SUGATA_NUM];//GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_03 );
      strbuf = GFL_STR_CreateBuffer( STRBUF_NUM_LENGTH, param->heap_id );
      WORDSET_RegisterNumber( wordset, 0, work->diff_sugata_num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
      PRINTSYS_PrintQueColor( work->print_que[TEXT_UP_NUM], bmp_data,
                              0, text_up_label_num_y[0], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      //GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );

      wordset = WORDSET_Create( param->heap_id );
      src_strbuf = work->text_frequent_strbuf[TEXT_FREQUENT_IROCHIGAI_NUM];//GFL_MSG_CreateString( work->text_msgdata, ZKN_FORME_TEXT_04 );
      strbuf = GFL_STR_CreateBuffer( STRBUF_NUM_LENGTH, param->heap_id );
      WORDSET_RegisterNumber( wordset, 0, work->diff_irochigai_num, 2, STR_NUM_DISP_SPACE, STR_NUM_CODE_DEFAULT );
      WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
      PRINTSYS_PrintQueColor( work->print_que[TEXT_UP_NUM], bmp_data,
                              0, text_up_label_num_y[1], strbuf, work->font, PRINTSYS_LSB_Make(15,2,0) );
      GFL_STR_DeleteBuffer( strbuf );
      //GFL_STR_DeleteBuffer( src_strbuf );
      WORDSET_Delete( wordset );

      work->text_trans[TEXT_UP_NUM] = TRUE;
    }

    for( i=TEXT_POKENAME; i<=TEXT_UP_NUM; i++ )
    {
      // 既に済んでいるかもしれないので、Mainを1度呼んでおく
      Zukan_Detail_Form_MainText( param, work, cmn, i );
    }
  }
}

static void Zukan_Detail_Form_WritePokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u8 i, PRINT_QUE* print_que, GFL_BMPWIN* bmpwin, BOOL* trans_flag, u16 diff_no )
{
  GFL_BMP_DATA*	bmp_data = GFL_BMPWIN_GetBmp( bmpwin );
  u16 bmp_width = GFL_BMP_GetSizeX( bmp_data );

  // 今回のテキストを描画
  {
    STRBUF* strbuf0;
    STRBUF* strbuf1;
    BOOL    can_delete_strbuf0;
    BOOL    can_delete_strbuf1;

    u16 str_width;
    u16 x;
    u16 y[2];

    Zukan_Detail_Form_GetPokeTextStrbufFromDiffInfo( param, work, cmn,
         &strbuf0, &strbuf1, &can_delete_strbuf0, &can_delete_strbuf1, diff_no );

    if( strbuf1 )
    {
      y[0] = TEXT_POKE_POS_Y_2_LINE_0;
      y[1] = TEXT_POKE_POS_Y_2_LINE_1;
    }
    else
    {
      y[0] = TEXT_POKE_POS_Y_1_LINE;
    }

    str_width = (u16)( PRINTSYS_GetStrWidth( strbuf0, work->font, 0 ) );
    x = ( bmp_width - str_width ) / 2;
    PRINTSYS_PrintQueColor( print_que, bmp_data,
                            x, y[0], strbuf0, work->font, PRINTSYS_LSB_Make(15,2,0) );

    if( strbuf1 )
    {
      str_width = (u16)( PRINTSYS_GetStrWidth( strbuf1, work->font, 0 ) );
      x = ( bmp_width - str_width ) / 2;
      PRINTSYS_PrintQueColor( print_que, bmp_data,
                              x, y[1], strbuf1, work->font, PRINTSYS_LSB_Make(15,2,0) );
    }

    if( strbuf0 && can_delete_strbuf0 ) GFL_STR_DeleteBuffer( strbuf0 );
    if( strbuf1 && can_delete_strbuf1 ) GFL_STR_DeleteBuffer( strbuf1 );

    *trans_flag = TRUE;

    // 既に済んでいるかもしれないので、Mainを1度呼んでおく
    Zukan_Detail_Form_MainText( param, work, cmn, i );
  }
}
static void Zukan_Detail_Form_WritePokeCurrText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前回のテキストをクリア
  {
    work->text_trans[TEXT_POKE_CURR] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[TEXT_POKE_CURR] );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKE_CURR] ), 0 );
    //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKE_CURR] );  // ちらつくのでコメントアウト
  }

  // 今回のテキストを描画
  Zukan_Detail_Form_WritePokeText( param, work, cmn,
               TEXT_POKE_CURR, work->print_que[TEXT_POKE_CURR], work->text_bmpwin[TEXT_POKE_CURR], &(work->text_trans[TEXT_POKE_CURR]), work->diff_curr_no );
 }
static void Zukan_Detail_Form_WritePokeCompText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前回のテキストをクリア
  {
    work->text_trans[TEXT_POKE_COMP] = FALSE;
    PRINTSYS_QUE_Clear( work->print_que[TEXT_POKE_COMP] );

    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( work->text_bmpwin[TEXT_POKE_COMP] ), 0 );
    //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKE_COMP] );  // ちらつくのでコメントアウト
  }

  // 今回のテキストを描画
  switch(work->state)
  {
  case STATE_TOP:
    {
      // 描画しない
      GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_POKE_COMP] );  // 上でちらつくのでコメントアウトしていたので、ここで転送
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        Zukan_Detail_Form_WritePokeText( param, work, cmn,
               TEXT_POKE_COMP, work->print_que[TEXT_POKE_COMP], work->text_bmpwin[TEXT_POKE_COMP], &(work->text_trans[TEXT_POKE_COMP]), work->diff_comp_no );
      }
      // 2姿以上なければ、描画しない
    }
    break;
  }
}
static void Zukan_Detail_Form_ScrollPokeText( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  switch(work->state)
  {
  case STATE_TOP:
    {
      //GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
      GFL_BG_SetScrollReq( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        //GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, 0 );
        GFL_BG_SetScrollReq( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, 0 );
      }
      else
      {
        //GFL_BG_SetScroll( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
        GFL_BG_SetScrollReq( BG_FRAME_M_TEXT, GFL_BG_SCROLL_X_SET, -64 );
      }
    }
    break;
  }
}

//-------------------------------------
/// MCSSポケモン
//=====================================
static void Zukan_Detail_Form_McssSysInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  work->mcss_sys_wk = MCSS_Init( POKE_MAX, param->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, 0 );  // 他に一緒に出す3Dはないので
  MCSS_SetOrthoMode( work->mcss_sys_wk );

#ifdef DEF_MCSS_HANDLE
  MCSS_OpenHandle( work->mcss_sys_wk, POKEGRA_GetArcID() );
#endif

#ifdef DEF_MCSS_TCBSYS
  work->mcss_tcbsys_wk = GFL_HEAP_AllocMemory( param->heap_id, GFL_TCB_CalcSystemWorkSize(MCSS_TCBSYS_TASK_MAX) );
  work->mcss_tcbsys = GFL_TCB_Init( MCSS_TCBSYS_TASK_MAX, work->mcss_tcbsys_wk );
  MCSS_SetTCBSys( work->mcss_sys_wk, work->mcss_tcbsys );
#endif
}
static void Zukan_Detail_Form_McssSysExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
#ifdef DEF_MCSS_HANDLE
  MCSS_CloseHandle( work->mcss_sys_wk );
#endif

  MCSS_Exit( work->mcss_sys_wk );

#ifdef DEF_MCSS_TCBSYS
  //MCSS_SetTCBSys( work->mcss_sys_wk, NULL );  // NULLを渡せばtcbをはずせる
  GFL_TCB_Exit( work->mcss_tcbsys );
  GFL_HEAP_FreeMemory( work->mcss_tcbsys_wk );
#endif
}

static MCSS_WORK* PokeInit( MCSS_SYS_WORK* mcss_sys_wk,
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                      u32 personal_rnd )  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
{
  VecFx32 scale =
  {
    FX_F32_TO_FX32(POKE_SCALE),
    FX_F32_TO_FX32(POKE_SCALE),
    FX32_ONE, 
  };

  MCSS_ADD_WORK add_wk;
  MCSS_WORK*    poke_wk;

  if( mons_no == MONSNO_PATTIIRU )  // MCSS_TOOL_AddPokeMcssの中身に合わせてある
  {
    MCSS_TOOL_SetMakeBuchiCallback( mcss_sys_wk, personal_rnd );
  }
  MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, egg, &add_wk, dir );
  poke_wk = MCSS_Add( mcss_sys_wk, 0, 0, 0, &add_wk );
  MCSS_SetShadowVanishFlag( poke_wk, TRUE );  // 影を消しておかないと、小さな点として影が表示されてしまう。 

  MCSS_SetAnmStopFlag( poke_wk );
  MCSS_SetScale( poke_wk, &scale );
  //PokeAdjustOfsPos( poke_wk );  // 地面原点で問題ないので、オフセットしない。
  PokeAdjustOfsPosX( poke_wk );

  return poke_wk;
}
static void PokeExit( MCSS_SYS_WORK* mcss_sys_wk, MCSS_WORK* poke_wk )
{
  MCSS_SetVanishFlag( poke_wk );
  MCSS_Del( mcss_sys_wk, poke_wk );
}
static void PokeAdjustOfsPos( MCSS_WORK* poke_wk )
{
  f32 size_y = (f32)MCSS_GetSizeY( poke_wk );
  f32 ofs_y;
  VecFx32 ofs;
  if( size_y > POKE_SIZE_MAX ) size_y = POKE_SIZE_MAX;
  ofs_y = ( POKE_SIZE_MAX - size_y ) / 2.0f;
  ofs.x = 0;  ofs.y = FX_F32_TO_FX32(ofs_y);  ofs.z = 0;
  MCSS_SetOfsPosition( poke_wk, &ofs );
}
static void PokeAdjustOfsPosX( MCSS_WORK* poke_wk )
{
  f32      offset_x = (f32)MCSS_GetOffsetX( poke_wk );  // 右にずれているとき+, 左にずれているとき-
  f32      ofs_position_x;
  VecFx32  ofs_position;

  ofs_position_x = - offset_x * POKE_X_ADJUST;
    
  ofs_position.x = FX_F32_TO_FX32(ofs_position_x);  ofs_position.y = 0;  ofs_position.z = 0;
  MCSS_SetOfsPosition( poke_wk, &ofs_position );
}

static void PokeGetCompareRelativePosition( MCSS_WORK* poke_wk, VecFx32* pos )
{
  VecFx32 origin;
  MCSS_GetPosition( poke_wk, &origin );

  pos->x = origin.x + poke_pos[POKE_COMP_RPOS].x;
  pos->y = origin.y + poke_pos[POKE_COMP_RPOS].y;
  pos->z = origin.z + poke_pos[POKE_COMP_RPOS].z;
}

static void PokeMcssWorkInit( POKE_MCSS_WORK* poke_mcss_wk, HEAPID heap_id,
                              MCSS_SYS_WORK* mcss_sys_wk,
                              int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                              u32 personal_rnd )  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
{
  poke_mcss_wk->poke_wk = PokeInit( mcss_sys_wk,
                                    mons_no, form_no, sex, rare, egg, dir,
                                    personal_rnd );
  poke_mcss_wk->poke_call_back_data = GFL_HEAP_AllocClearMemory( heap_id, sizeof(POKE_MCSS_CALL_BACK_DATA) );

  {
    NNSG2dMultiCellAnimation* mcss_anim_ctrl = MCSS_GetAnimCtrl( poke_mcss_wk->poke_wk );
    NNS_G2dSetAnimCtrlCallBackFunctor(
        NNS_G2dGetMCAnimAnimCtrl(mcss_anim_ctrl),
        NNS_G2D_ANMCALLBACKTYPE_LAST_FRM,
        (u32)poke_mcss_wk->poke_call_back_data,
        Zukan_Detail_Form_PokeMcssCallBackFunctor );
  }

#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
  PokeSetupArrange( poke_mcss_wk->pos,
                  mons_no, form_no, sex, rare, egg, dir,
                  personal_rnd );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
}
static void PokeMcssWorkExit( POKE_MCSS_WORK* poke_mcss_wk, MCSS_SYS_WORK* mcss_sys_wk )
{
  BLOCK_POKE_EXIT( mcss_sys_wk, poke_mcss_wk->poke_wk )
  if( poke_mcss_wk->poke_call_back_data ) GFL_HEAP_FreeMemory( poke_mcss_wk->poke_call_back_data );
  poke_mcss_wk->poke_call_back_data = NULL;
}
static void PokeMcssCallBackDataInit( POKE_MCSS_CALL_BACK_DATA* poke_mcss_call_back_data,
                                      POKE_INDEX poke_idx, ZUKAN_DETAIL_FORM_WORK* work )
{
  poke_mcss_call_back_data->poke_idx               = poke_idx;
  poke_mcss_call_back_data->zukan_detail_form_work = (void*)work;
  poke_mcss_call_back_data->count                  = 0;
  poke_mcss_call_back_data->stop                   = FALSE;
}

static void PokeMcssAnimeStart( POKE_MCSS_WORK* poke_mcss_wk )
{
  if( poke_mcss_wk->poke_wk && poke_mcss_wk->poke_call_back_data )
  {
    NNSG2dMultiCellAnimation* mcss_anim_ctrl = MCSS_GetAnimCtrl( poke_mcss_wk->poke_wk );
    poke_mcss_wk->poke_call_back_data->count = 0;
    poke_mcss_wk->poke_call_back_data->stop  = FALSE;
    NNS_G2dRestartMCAnimation( mcss_anim_ctrl );                             // ResetだけだとPOKE_MCSS_ANIME_LOOP_MAX回だけきちんとループしてくれなかったのでこれも残すことにした 100625
    NNS_G2dResetMCCellAnimationAll( &(mcss_anim_ctrl->multiCellInstance) );  // Restartだとセルとマルチセルがずれるそうなのでこちらにした 100624
    MCSS_ResetAnmStopFlag( poke_mcss_wk->poke_wk );
  }
}
#ifdef DEF_MINIMUM_LOAD
static void PokeMcssAnimeStop( POKE_MCSS_WORK* poke_mcss_wk )
{
  if( poke_mcss_wk->poke_wk && poke_mcss_wk->poke_call_back_data )
  {
    poke_mcss_wk->poke_call_back_data->stop = TRUE;
  }
}
#endif
static void PokeMcssAnimeMain( POKE_MCSS_WORK* poke_mcss_wk )
{
  if( poke_mcss_wk->poke_wk && poke_mcss_wk->poke_call_back_data )
  {
    NNSG2dMultiCellAnimation* mcss_anim_ctrl = MCSS_GetAnimCtrl( poke_mcss_wk->poke_wk );
    if( poke_mcss_wk->poke_call_back_data->stop )
    {
      MCSS_SetAnmStopFlag( poke_mcss_wk->poke_wk );
      NNS_G2dRestartMCAnimation( mcss_anim_ctrl );                             // ResetだけだとPOKE_MCSS_ANIME_LOOP_MAX回だけきちんとループしてくれなかったのでこれも残すことにした 100625
      NNS_G2dResetMCCellAnimationAll( &(mcss_anim_ctrl->multiCellInstance) );  // Restartだとセルとマルチセルがずれるそうなのでこちらにした 100624
      poke_mcss_wk->poke_call_back_data->stop = FALSE;
    }
  }
}
static void Zukan_Detail_Form_PokeMcssCallBackFunctor( u32 data, fx32 currentFrame )
{
  POKE_MCSS_CALL_BACK_DATA* poke_call_back_data = (POKE_MCSS_CALL_BACK_DATA*)data;
  ZUKAN_DETAIL_FORM_WORK*   work                = (ZUKAN_DETAIL_FORM_WORK*)poke_call_back_data->zukan_detail_form_work;

  poke_call_back_data->count++;
  if( poke_call_back_data->count >= POKE_MCSS_ANIME_LOOP_MAX )
  {
    poke_call_back_data->stop = TRUE;
  }

  MCSS_RestartAnime( work->poke_mcss_wk[poke_call_back_data->poke_idx].poke_wk );  // 1ループしたらアニメーションリセット
}

static void Zukan_Detail_Form_FlipFrontBack( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
#ifndef DEF_MINIMUM_LOAD
  // 今の状態を見て、前後を変更する
  if( work->is_poke_front )
  {
    MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
    MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_CURR_B].poke_wk );
    if( work->diff_num >= 2 )
    {
      MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
      MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_COMP_B].poke_wk );
    }
    // 変更後の状態
    work->is_poke_front = FALSE;
  }
  else
  {
    MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_CURR_B].poke_wk );
    MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
    if( work->diff_num >= 2 )
    {
      MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_COMP_B].poke_wk );
      MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
    }
    // 変更後の状態
    work->is_poke_front = TRUE;
  }
#else
  // 今の状態を見て、前後を変更する
  if( work->is_poke_front )
  {
    MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
    if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    {
      MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
    }
    // 変更後の状態
    work->is_poke_front = FALSE;
  }
  else
  {
    MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_CURR_B].poke_wk );
    if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    {
      MCSS_SetVanishFlag( work->poke_mcss_wk[POKE_COMP_B].poke_wk );
    }
    // 変更後の状態
    work->is_poke_front = TRUE;
  }

  Zukan_Detail_Form_MakePokeWhenFlipFrontBack( param, work, cmn );

  // 変更後の状態を見て、前後を変更する
  if( work->is_poke_front )
  {
    MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_CURR_F].poke_wk );
    if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    {
      MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_COMP_F].poke_wk );
    }
  }
  else
  {
    MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_CURR_B].poke_wk );
    if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    {
      MCSS_ResetVanishFlag( work->poke_mcss_wk[POKE_COMP_B].poke_wk );
    }
  }
#endif
}


//-------------------------------------
/// ポケアイコン
//=====================================
static void PokeiconBaseInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 不変物の生成
  work->pokeicon_handle = GFL_ARC_OpenDataHandle( ARCID_POKEICON, param->heap_id );

/*
  work->pokeicon_ncl = GFL_CLGRP_PLTT_RegisterEx(
                           work->pokeicon_handle,
                           POKEICON_GetPalArcIndex(),
                           CLSYS_DRAW_SUB,
                           OBJ_PAL_POS_S_POKEICON * 0x20,
                           0,
                           OBJ_PAL_NUM_S_POKEICON,
                           param->heap_id );
*/
  work->pokeicon_ncl = GFL_CLGRP_PLTT_RegisterComp(  // ポケアイコンのパレットは圧縮されている
                           work->pokeicon_handle,
                           POKEICON_GetPalArcIndex(),
                           CLSYS_DRAW_SUB,
                           OBJ_PAL_POS_S_POKEICON * 0x20,
                           param->heap_id );

  work->pokeicon_nce = GFL_CLGRP_CELLANIM_Register(
                           work->pokeicon_handle,
                           POKEICON_GetCellSubArcIndex(),
                           POKEICON_GetAnmSubArcIndex(),
                           param->heap_id );
}
static void PokeiconBaseExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 不変物の生成
  GFL_CLGRP_CELLANIM_Release( work->pokeicon_nce );
  GFL_CLGRP_PLTT_Release( work->pokeicon_ncl );
  
  GFL_ARC_CloseDataHandle( work->pokeicon_handle );
}

static GFL_CLWK* PokeiconInit( ARCHANDLE* pokeicon_handle, u32* ncg, u32 ncl, u32 nce, GFL_CLUNIT* clunit, HEAPID heap_id, u32 mons, u32 form_no, u32 sex, BOOL egg )
{
  GFL_CLWK* clwk;
  GFL_CLWK_DATA data;

  data.pos_x   = pokeicon_pos[0];
  data.pos_y   = pokeicon_pos[1];
  data.anmseq  = 1;  // アニメシーケンスで指定( 0=瀕死, 1=HP最大, 2=HP緑, 3=HP黄, 4=HP赤, 5=状態異常 )
  data.softpri = 1;  // 上にアイテムアイコンを描画するので優先度を下げておく
  data.bgpri   = 0;

  *ncg = GFL_CLGRP_CGR_Register(
             pokeicon_handle,
             POKEICON_GetCgxArcIndexByMonsNumber( mons, form_no, sex, egg ),
             FALSE,
             CLSYS_DRAW_SUB,
             heap_id );
    
  clwk = GFL_CLACT_WK_Create(
             clunit,
             *ncg,
             ncl,
             nce,
             &data,
             CLSYS_DEFREND_SUB,
             heap_id );
   
  // オートアニメ OFF
  GFL_CLACT_WK_SetAutoAnmFlag( clwk, FALSE );

  {
    u8 pal_num = POKEICON_GetPalNum( mons, form_no, sex, egg );
    GFL_CLACT_WK_SetPlttOffs( clwk, pal_num, CLWK_PLTTOFFS_MODE_OAM_COLOR );
  }

  //GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
  GFL_CLACT_WK_SetObjMode( clwk, GX_OAM_MODE_NORMAL );  // BGとともにこのOBJも暗くしたいのは、フェードインとフェードアウトのときだけであり、
                                                        // ポケモンを変更しただけのときは、暗くしたり半透明にしたりしたくない。
                                                        // BGとともにこのOBJも暗くしたいときは、この関数の後に設定すること。

  return clwk;
}
static void PokeiconExit( u32 ncg, GFL_CLWK* clwk )
{
  // CLWK破棄
  GFL_CLACT_WK_Remove( clwk );
  // リソース開放
  GFL_CLGRP_CGR_Release( ncg );
}

//-------------------------------------
/// 違う姿情報を取得する
//=====================================
static void Zukan_Detail_Form_GetDiffInfoWithoutWork( ZUKAN_DETAIL_FORM_PARAM* param, ZKNDTL_COMMON_WORK* cmn,
                                                      DIFF_INFO*   a_diff_info_list,  // 要素数a_diff_info_list[DIFF_MAX]で
                                                      u16*         a_diff_num,
                                                      u16*         a_diff_sugata_num,
                                                      u16*         a_diff_irochigai_num,
                                                      u16*         a_diff_curr_no,
                                                      u16*         a_diff_comp_no )
{
  const u8 sex_tbl_max = 3;
  const u32 sex_tbl[sex_tbl_max] =
  {
    PTL_SEX_MALE,
    PTL_SEX_FEMALE,
    PTL_SEX_UNKNOWN,
  };
  const u8 rare_tbl_max = 2;
  const BOOL rare_tbl[rare_tbl_max] =
  {
    FALSE,
    TRUE,
  };

  u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

  GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
  ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );

  // ポケモンパーソナルデータの情報
  u32 ppd_form_num;  // フォルム数
  u32 ppd_sex_vec;   // 性別ベクトル

  // 現在表示する姿
  u32  curr_sex;
  BOOL curr_rare;
  u32  curr_form;

  // ポケモンパーソナルデータの情報
  {
    POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle( monsno, 0, param->heap_id );
    ppd_form_num = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_form_max );  // 別フォルムなしのときは1
    ppd_sex_vec  = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );  // POKEPER_SEX_MALE, 1～253, POKEPER_SEX_FEMALE, POKEPER_SEX_UNKNOWN  // prog/include/poke_tool/poke_personal.h
    POKE_PERSONAL_CloseHandle( ppd );

#ifdef DEBUG_KAWADA
    {
      OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, ppd f=%d s=%d\n", monsno, ppd_form_num, ppd_sex_vec );
    }
#endif
  }

  // パーソナルのデータではなく、図鑑が持っているデータを使用する
  {
    ppd_form_num = ZUKANSAVE_GetFormMax( monsno );  // monsnoのポケモンが図鑑に登録することができるフォルム数を得られる。
        // プレイヤーが現在までに見たフォルム数を返してくるわけではない。
        // ポケモンWBでは、0番フォルムから連番で出現する。
        // 番号飛び飛びのフォルムが出現することはない。
        // よって、0<= <ZUKANSAVE_GetFormMax( monsno )のフォルムを見たかどうかチェックすればいい。
  }

  // 現在表示する姿
  {
    ZUKANSAVE_GetDrawData(  zkn_sv, monsno, &curr_sex, &curr_rare, &curr_form, param->heap_id );

#ifdef DEBUG_KAWADA
    {
      OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, curr f=%d, s=%d, r=%d\n", monsno, curr_form, curr_sex, curr_rare );
    }
#endif
  }

  // プレイヤーが入手した情報
  {
    u16 diff_num = 0;
    u16 diff_curr_no = 0;

    u32  sex;
    BOOL rare;
    u32  form;

    u8 sex_tbl_idx;
    u8 rare_tbl_idx;

    {
      // 性別でチェックするしないを分ける
      BOOL b_sex_check_tbl[sex_tbl_max] =
      {
        FALSE,
        FALSE,
        FALSE,
      };
      switch( ppd_sex_vec )
      {
        // 性別あり(オスのみ)
      case POKEPER_SEX_MALE:
        {
          b_sex_check_tbl[0] = TRUE;
        }
        break;
        // 性別あり(メスのみ)
      case POKEPER_SEX_FEMALE:
        {
          b_sex_check_tbl[1] = TRUE;
        }
        break;
        // 性別なし
      case POKEPER_SEX_UNKNOWN:
        {
          b_sex_check_tbl[2] = TRUE;
        }
        break;
        // 性別あり(オスメス)
      default:  // 1～253
        {
          b_sex_check_tbl[0] = TRUE;
          b_sex_check_tbl[1] = TRUE;
        }
        break;
      }

      for( form=0; form<ppd_form_num; form++ )
      {
        BOOL b_rare_finish_tbl[rare_tbl_max] =  // フォルム違いがあるときは、オスメスはあろうとなかろうと、無視する
        {                                       // ために、ノーマル/レアそれぞれがどれかの性別で登録されていたら、
          FALSE,                                // もう登録しないようにするためのフラグ
          FALSE,
        };
        for( sex_tbl_idx=0; sex_tbl_idx<sex_tbl_max; sex_tbl_idx++ )
        {
          if( !b_sex_check_tbl[sex_tbl_idx] ) continue;
          sex = sex_tbl[sex_tbl_idx];
          for( rare_tbl_idx=0; rare_tbl_idx<rare_tbl_max; rare_tbl_idx++ )
          {
            BOOL b_regist = FALSE;
            rare = rare_tbl[rare_tbl_idx];
            if( curr_form == form && curr_rare == rare && curr_sex == sex )  // 現在表示する姿
            {
              diff_curr_no = diff_num;
              b_regist = TRUE;
            }
            else if( ( ppd_form_num >= 2 ) && curr_form == form && curr_rare == rare )  // フォルム違いがあるときは、オスメスはあろうとなかろうと、無視する
            {
              // 何もしない
            }
            else
            {
              if( ZUKANSAVE_CheckPokeSeeForm( zkn_sv, monsno, (int)sex, (int)rare, (int)form ) )
              {
                b_regist = TRUE;
              }
            }

            if( b_regist )
            {
              if(    ( ( ppd_form_num >= 2 ) && ( !b_rare_finish_tbl[rare_tbl_idx] ) )    // フォルム違いがあるときは、まだこのノーマル/レアがどの性別でも登録されていないとき
                  || ( ( ppd_form_num <  2 )                                         ) )  // フォルム違いがないときは、いつでも真
              {
                GF_ASSERT_MSG( diff_num < DIFF_MAX,  "ZUKAN_DETAIL_FORM : 違う姿の数が多いです。\n" );
                
                b_rare_finish_tbl[rare_tbl_idx] = TRUE;

                if( ppd_form_num >= 2 )
                {
                  a_diff_info_list[diff_num].looks_diff = LOOKS_DIFF_FORM;
                  a_diff_info_list[diff_num].color_diff = rare?COLOR_DIFF_SPECIAL:COLOR_DIFF_NORMAL;
                  a_diff_info_list[diff_num].other_diff = (u16)form;
                }
                else
                {
                  a_diff_info_list[diff_num].looks_diff = (ppd_sex_vec==POKEPER_SEX_UNKNOWN)?LOOKS_DIFF_ONE:LOOKS_DIFF_SEX;
                  a_diff_info_list[diff_num].color_diff = rare?COLOR_DIFF_SPECIAL:COLOR_DIFF_NORMAL;
                  a_diff_info_list[diff_num].other_diff = (sex==PTL_SEX_FEMALE)?SEX_FEMALE:SEX_MALE;  // LOOKS_DIFF_ONEのときは使用されないので、何が入っていてもいい
                }
                a_diff_info_list[diff_num].sex        = (u8)sex;
                a_diff_info_list[diff_num].rare       = (u8)rare;
                a_diff_info_list[diff_num].form       = (u16)form;

#ifdef DEBUG_KAWADA
                {
                  OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, diff_no=%d, f=%d, s=%d, r=%d\n", monsno, diff_num, form, sex, rare );
                }
#endif

                diff_num++;
              }
            }
          }
        }
      }
    }

    GF_ASSERT_MSG( diff_num > 0,  "ZUKAN_DETAIL_FORM : 姿が何もありません。\n" );

    *a_diff_num = diff_num;
    *a_diff_curr_no = diff_curr_no;
    //*a_diff_comp_no = (diff_curr_no!=0)?(0):(1%diff_num);  // 0番目か1番目の姿にしておく
    *a_diff_comp_no = (diff_curr_no+1)%diff_num;  // currの次の姿にしておく
  }

  // 姿の数、色違いの数
  {
    u8 i;

    *a_diff_sugata_num = 0;
    *a_diff_irochigai_num = 0;

    for( i=0; i<(*a_diff_num); i++ )
    {
      if( a_diff_info_list[i].color_diff == COLOR_DIFF_SPECIAL )
      {
        (*a_diff_irochigai_num)++;
      }
    }

    (*a_diff_sugata_num) = (*a_diff_num) - (*a_diff_irochigai_num);
  }

/*
  パーソナルのデータではなく、図鑑が持っているデータを使用する
  ようにしたので、以下の特別処理は不要。
  // 特別処理
  {
    // フォルム0番しか見せないポケモン
    if(
           monsno == MONSNO_ARUSEUSU    // アルセウス
        || monsno == MONSNO_656         // インセクタ→ゲノセクト
    )
    {
      if( *a_diff_num > 0 )
      {
        // 「フォルム0番」と「フォルム0番の色違い」だけ残す
        DIFF_INFO   tmp_diff_info_list[2];
        u16         tmp_diff_num             = 0;
        s16         tmp_diff_curr_no         = -1;
        s16         tmp_diff_comp_no         = -1;
        u16 i;
        for( i=0; i<(*a_diff_num); i++ )
        {
          if( a_diff_info_list[i].form == 0 )
          {
            tmp_diff_info_list[tmp_diff_num] = a_diff_info_list[i];
            if( *a_diff_curr_no == i ) tmp_diff_curr_no = tmp_diff_num;
            if( *a_diff_comp_no == i ) tmp_diff_comp_no = tmp_diff_num;
            tmp_diff_num++;
            if( tmp_diff_num == 2 ) break;
          }
        }
        if( tmp_diff_num == 0 )  // 起こり得ないはずだが念のため
        {
          tmp_diff_info_list[tmp_diff_num] = a_diff_info_list[0];
          tmp_diff_num++;
        }
        if( tmp_diff_curr_no == -1 )
        {
          tmp_diff_curr_no = (tmp_diff_curr_no==0)?(1):(0);
        }
        if( tmp_diff_comp_no == -1 )
        {
          tmp_diff_comp_no = (tmp_diff_curr_no==0)?(1):(0); 
        }

        // 結果を持ち帰る
        for( i=0; i<tmp_diff_num; i++ )
        {
          a_diff_info_list[i] = tmp_diff_info_list[i];
        }
        *a_diff_num = tmp_diff_num;
        *a_diff_curr_no = (u16)tmp_diff_curr_no;
        *a_diff_comp_no = (u16)tmp_diff_comp_no;
        *a_diff_sugata_num = 0;
        *a_diff_irochigai_num = 0;
        for( i=0; i<(*a_diff_num); i++ )
        {
          if( a_diff_info_list[i].color_diff == COLOR_DIFF_SPECIAL )
          {
            (*a_diff_irochigai_num)++;
          }
        }
        (*a_diff_sugata_num) = (*a_diff_num) - (*a_diff_irochigai_num);
      }
    }
  }
*/
}
static void Zukan_Detail_Form_GetDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  Zukan_Detail_Form_GetDiffInfoWithoutWork( param, cmn,
                                            work->diff_info_list,
                                            &work->diff_num,
                                            &work->diff_sugata_num,
                                            &work->diff_irochigai_num,
                                            &work->diff_curr_no,
                                            &work->diff_comp_no );
}

//-------------------------------------
/// 違う姿情報からMCSSポケモンを生成する
//=====================================
static void Zukan_Detail_Form_PokeInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                POKE_INDEX poke_f, POKE_INDEX poke_b, POKE_POS_INDEX pos, u16 diff_no )
{
  int mons_no;
  int form_no;
  int sex;
  int rare;
  u32 personal_rnd = 0;

  mons_no = ZKNDTL_COMMON_GetCurrPoke(cmn);

  form_no = work->diff_info_list[diff_no].form;
  sex = work->diff_info_list[diff_no].sex;
  rare = work->diff_info_list[diff_no].rare;

  if( mons_no == MONSNO_PATTIIRU )  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
  {
    GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
    ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );
    personal_rnd = ZUKANSAVE_GetPokeRandomFlag( zkn_sv, ZUKANSAVE_RANDOM_PACHI );
  }

#ifndef DEF_MINIMUM_LOAD
  //work->poke_mcss_wk[poke_f].poke_wk = PokeInit( work->mcss_sys_wk,
  //                            mons_no, form_no, sex, rare, FALSE, MCSS_DIR_FRONT );
  //work->poke_mcss_wk[poke_b].poke_wk = PokeInit( work->mcss_sys_wk,
  //                            mons_no, form_no, sex, rare, FALSE, MCSS_DIR_BACK );
  PokeMcssWorkInit( &work->poke_mcss_wk[poke_f], param->heap_id,
                    work->mcss_sys_wk,
                    mons_no, form_no, sex, rare, FALSE, MCSS_DIR_FRONT,
                    personal_rnd );
  PokeMcssWorkInit( &work->poke_mcss_wk[poke_b], param->heap_id,
                    work->mcss_sys_wk,
                    mons_no, form_no, sex, rare, FALSE, MCSS_DIR_BACK,
                    personal_rnd );

  {
    VecFx32 p;
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
    if( pos == POKE_COMP_RPOS )
    {
      PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
    }
    else
    {
      p = poke_pos[pos];
    }
    MCSS_SetPosition( work->poke_mcss_wk[poke_f].poke_wk, &p );
    MCSS_SetPosition( work->poke_mcss_wk[poke_b].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[poke_f], pos, &p );
    MCSS_SetPosition( work->poke_mcss_wk[poke_f].poke_wk, &p );
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[poke_b], pos, &p );
    MCSS_SetPosition( work->poke_mcss_wk[poke_b].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
  }

  if( work->is_poke_front )
    MCSS_SetVanishFlag( work->poke_mcss_wk[poke_b].poke_wk );
  else
    MCSS_SetVanishFlag( work->poke_mcss_wk[poke_f].poke_wk );
#else
  {
    VecFx32 p;
    POKE_INDEX poke_relative = (work->is_poke_front)?(POKE_CURR_F):(POKE_CURR_B);  // 今の向きの位置の元

#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
    {
      if( pos == POKE_COMP_RPOS )
      {
        PokeGetCompareRelativePosition( work->poke_mcss_wk[poke_relative].poke_wk, &p );
      }
      else
      {
        p = poke_pos[pos];
      }
    }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 

    if( poke_f != POKE_INDEX_NONE )
    {
      PokeMcssWorkInit( &work->poke_mcss_wk[poke_f], param->heap_id,
                        work->mcss_sys_wk,
                        mons_no, form_no, sex, rare, FALSE, MCSS_DIR_FRONT,
                        personal_rnd );
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[poke_f], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
      MCSS_SetPosition( work->poke_mcss_wk[poke_f].poke_wk, &p );
      if( !(work->is_poke_front) )
      {
        MCSS_SetVanishFlag( work->poke_mcss_wk[poke_f].poke_wk );
      }
    }

    if( poke_b != POKE_INDEX_NONE )
    {
      PokeMcssWorkInit( &work->poke_mcss_wk[poke_b], param->heap_id,
                        work->mcss_sys_wk,
                        mons_no, form_no, sex, rare, FALSE, MCSS_DIR_BACK,
                        personal_rnd );
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[poke_b], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
      MCSS_SetPosition( work->poke_mcss_wk[poke_b].poke_wk, &p );
      if(  (work->is_poke_front) )
      {
        MCSS_SetVanishFlag( work->poke_mcss_wk[poke_b].poke_wk );
      }
    }
  }
#endif
}

//-------------------------------------
/// 違う姿情報からポケアイコンを生成する
//=====================================
static void Zukan_Detail_Form_PokeiconInitFromDiffInfo( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                u16 diff_no )
{
  u32 mons;
  u32 form_no;
  u32 sex;

  mons = ZKNDTL_COMMON_GetCurrPoke(cmn);

  form_no = work->diff_info_list[diff_no].form;
  sex = work->diff_info_list[diff_no].sex;

  work->pokeicon_clwk[work->pokeicon_swap_curr] = PokeiconInit(
                                                      work->pokeicon_handle,
                                                      &(work->pokeicon_ncg[work->pokeicon_swap_curr]),
                                                      work->pokeicon_ncl,
                                                      work->pokeicon_nce,
                                                      work->clunit,
                                                      param->heap_id,
                                                      mons,
                                                      form_no,
                                                      sex,
                                                      0 );
}

//-------------------------------------
/// ポケモン変更
//=====================================
static void Zukan_Detail_Form_ChangePoke( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前のを破棄
  u8 i;
  for( i=0; i<POKE_MAX; i++ )
  {
    //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[i].poke_wk )
    PokeMcssWorkExit( &work->poke_mcss_wk[i], work->mcss_sys_wk );
  }

  // 次のを生成
  Zukan_Detail_Form_GetDiffInfo( param, work, cmn );

  {
    POKE_POS_INDEX pos = POKE_CURR_POS_CENTER;
    if( work->state == STATE_EXCHANGE && work->diff_num >= 2 ) 
    {
      pos = POKE_CURR_POS_LEFT;
    }
#ifndef DEF_MINIMUM_LOAD
    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        POKE_CURR_F, POKE_CURR_B, pos, work->diff_curr_no );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data,
                              POKE_CURR_F, work );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data,
                              POKE_CURR_B, work );
#else
    {
      POKE_INDEX poke_f = (work->is_poke_front)?(POKE_CURR_F):(POKE_INDEX_NONE);
      POKE_INDEX poke_b = (work->is_poke_front)?(POKE_INDEX_NONE):(POKE_CURR_B);

      Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
          poke_f, poke_b, pos, work->diff_curr_no );
    
      if( poke_f != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data,
                                  POKE_CURR_F, work );
      }
      if( poke_b != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data,
                                  POKE_CURR_B, work );
      }
    }
#endif
  }

#ifndef DEF_MINIMUM_LOAD
  if( work->diff_num >= 2 )
  {
    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        POKE_COMP_F, POKE_COMP_B, POKE_COMP_RPOS, work->diff_comp_no );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                              POKE_COMP_F, work );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                              POKE_COMP_B, work );
  }
#else
  if( work->state == STATE_EXCHANGE && work->diff_num >= 2 )
  {
    {
      POKE_INDEX poke_f = (work->is_poke_front)?(POKE_COMP_F):(POKE_INDEX_NONE);
      POKE_INDEX poke_b = (work->is_poke_front)?(POKE_INDEX_NONE):(POKE_COMP_B);

      Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
          poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );
    
      if( poke_f != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                                  POKE_COMP_F, work );
      }
      if( poke_b != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                                  POKE_COMP_B, work );
      }
    }
  }
#endif

  // ポケアイコン変更
  Zukan_Detail_Form_ChangePokeicon( param, work, cmn );

  // テキスト
  // 変更されたポケモンで、上画面のテキストを書く
  Zukan_Detail_Form_WriteUpText( param, work, cmn );
  // 変更されたポケモンで、テキストをスクロールさせて、今のフォルムのテキストを書き、比較フォルムのテキストを書くor消す
  Zukan_Detail_Form_ScrollPokeText( param, work, cmn );
  Zukan_Detail_Form_WritePokeCurrText( param, work, cmn );
  Zukan_Detail_Form_WritePokeCompText( param, work, cmn );

  // 変更されたポケモン用に、フォルム名の背面フィールドを用意する
  Zukan_Detail_Form_ObjFieldSetup( param, work, cmn );

  // 変更されたポケモン用に、スクロールバーを用意する
  Zukan_Detail_Form_ObjBarSetup( param, work, cmn );

  // 変更されたポケモン用に、トップ画面でフォルムを切り替えるための矢印を用意する
  Zukan_Detail_Form_ObjButtonArrowSetup( param, work, cmn );

/*
  // 押し出し用関数を利用してcompをcurrの次のフォルムにしておく
  // 押し出し用関数を利用して位置設定
  if( work->state == STATE_TOP && work->diff_num >= 2 ) 
  {
    work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
    Zukan_Detail_Form_OshidashiChangeCompareForm( param, work, cmn );
  }
*/
  // Zukan_Detail_Form_GetDiffInfoでcompをcurrの次のフォルムにするようにしたので、
  // 押し出し用関数を利用して位置設定だけ行うことにする
#ifndef DEF_MINIMUM_LOAD
  if( work->state == STATE_TOP && work->diff_num >= 2 ) 
  {
    // 押し出し用関数を利用して位置設定
    Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
  }
#endif
}

#ifdef DEF_MINIMUM_LOAD
//-------------------------------------
/// ポケモンの前後を変更した際に、変更後のMCSSが存在しなかったら作成する
//=====================================
static void Zukan_Detail_Form_MakePokeWhenFlipFrontBack( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // work->is_poke_frontにはこれから作成する向きが既に設定されている
  // 存在しないものを作成(存在するものには正しい今のフォルム、比較フォルムが入っている)
  POKE_INDEX      poke_f, poke_b;
  POKE_POS_INDEX  pos;
  VecFx32         p;

  // 読み込み時に既に表示されているポケモンの絵が乱れるので、tcbを一時的にはずしておく
  MCSS_SetTCBSys( work->mcss_sys_wk, NULL );

  // CURR -----------------------------------------------
  if( !(work->poke_mcss_wk[POKE_CURR_F].poke_wk) ) poke_f = POKE_CURR_F;
  else                                             poke_f = POKE_INDEX_NONE;
  if( !(work->poke_mcss_wk[POKE_CURR_B].poke_wk) ) poke_b = POKE_CURR_B;
  else                                             poke_b = POKE_INDEX_NONE;

  if( work->is_poke_front )  poke_b = POKE_INDEX_NONE;
  else                       poke_f = POKE_INDEX_NONE;
    
  pos = POKE_CURR_POS_CENTER;
  if( work->state == STATE_EXCHANGE && work->diff_num >= 2 ) 
  {
    pos = POKE_CURR_POS_LEFT;
  }
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
  p = poke_pos[pos];
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

  Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
      poke_f, poke_b, pos, work->diff_curr_no );

  if( poke_f != POKE_INDEX_NONE ) 
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data,
                              POKE_CURR_F, work );
  }
  else
  {
    // 存在するものの位置設定
    if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
    {
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
    }
  }
  if( poke_b != POKE_INDEX_NONE ) 
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data,
                              POKE_CURR_B, work );
  }
  else
  {
    // 存在するものの位置設定
    if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
    {
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
    }
  }

  // COMP -----------------------------------------------
  if( work->state == STATE_EXCHANGE && work->diff_num >= 2 )
  {
    if( !(work->poke_mcss_wk[POKE_COMP_F].poke_wk) ) poke_f = POKE_COMP_F;
    else                                             poke_f = POKE_INDEX_NONE;
    if( !(work->poke_mcss_wk[POKE_COMP_B].poke_wk) ) poke_b = POKE_COMP_B;
    else                                             poke_b = POKE_INDEX_NONE;

    pos = POKE_COMP_RPOS;
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
    PokeGetCompareRelativePosition( work->poke_mcss_wk[(work->is_poke_front)?POKE_CURR_F:POKE_CURR_B].poke_wk, &p );  // この時点でPOKE_CURR_FかPOKE_CURR_Bは必ず存在する
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );

    if( poke_f != POKE_INDEX_NONE )
    {
      PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                                POKE_COMP_F, work );
    }
    else
    {
      // 存在するものの位置設定
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
        PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
      }
    }
    if( poke_b != POKE_INDEX_NONE )
    {
      PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                                POKE_COMP_B, work );
    }
    else
    {
      // 存在するものの位置設定
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      {
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
        PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
      }
    }
  }

  //      -----------------------------------------------
  // 一時的にはずしていたtcbを、再び使用するように戻しておく
  MCSS_SetTCBSys( work->mcss_sys_wk, work->mcss_tcbsys );

  if( work->state == STATE_TOP )
  {
    // 押し出し用関数を利用して位置設定
    work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
    Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
  }
}
//-------------------------------------
/// ポケモンのアニメーションを再生する際に、今のフォルム(前向き)、今のフォルム(後向き)、比較フォルム(前向き)、比較フォルム(後向き)のどれかがなかったら作成する
//=====================================
static void Zukan_Detail_Form_MakePokeWhenAnimeStart( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 存在しないものを作成(存在するものには正しい今のフォルム、比較フォルムが入っている)
  POKE_INDEX      poke_f, poke_b;
  POKE_POS_INDEX  pos;
  VecFx32         p;

  // 読み込み時に既に表示されているポケモンの絵が乱れるので、tcbを一時的にはずしておく
  MCSS_SetTCBSys( work->mcss_sys_wk, NULL );

  // CURR -----------------------------------------------
  if( !(work->poke_mcss_wk[POKE_CURR_F].poke_wk) ) poke_f = POKE_CURR_F;
  else                                             poke_f = POKE_INDEX_NONE;
  if( !(work->poke_mcss_wk[POKE_CURR_B].poke_wk) ) poke_b = POKE_CURR_B;
  else                                             poke_b = POKE_INDEX_NONE;

  pos = POKE_CURR_POS_CENTER;
  if( work->state == STATE_EXCHANGE && work->diff_num >= 2 ) 
  {
    pos = POKE_CURR_POS_LEFT;
  }
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
  p = poke_pos[pos];
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

  Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
      poke_f, poke_b, pos, work->diff_curr_no );

  if( poke_f != POKE_INDEX_NONE ) 
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data,
                              POKE_CURR_F, work );
  }
  else
  {
    // 存在するものの位置設定
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
  }
  if( poke_b != POKE_INDEX_NONE ) 
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data,
                              POKE_CURR_B, work );
  }
  else
  {
    // 存在するものの位置設定
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
  }

  // COMP -----------------------------------------------
  if( !(work->poke_mcss_wk[POKE_COMP_F].poke_wk) ) poke_f = POKE_COMP_F;
  else                                             poke_f = POKE_INDEX_NONE;
  if( !(work->poke_mcss_wk[POKE_COMP_B].poke_wk) ) poke_b = POKE_COMP_B;
  else                                             poke_b = POKE_INDEX_NONE;

  pos = POKE_COMP_RPOS;
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
  PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );  // この時点でPOKE_CURR_Fは必ず存在する
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

  Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
      poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );

  if( poke_f != POKE_INDEX_NONE )
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                              POKE_COMP_F, work );
  }
  else
  {
    // 存在するものの位置設定
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
  }
  if( poke_b != POKE_INDEX_NONE )
  {
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                              POKE_COMP_B, work );
  }
  else
  {
    // 存在するものの位置設定
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], pos, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
  }

  //      -----------------------------------------------
  // 一時的にはずしていたtcbを、再び使用するように戻しておく
  MCSS_SetTCBSys( work->mcss_sys_wk, work->mcss_tcbsys );

  if( work->state == STATE_TOP )
  {
    // 押し出し用関数を利用して位置設定
    work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
    Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
  }
}
#endif

//-------------------------------------
/// 比較フォルム変更
//=====================================
static void Zukan_Detail_Form_ChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 前のを破棄
  //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[POKE_COMP_F].poke_wk )
  //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[POKE_COMP_B].poke_wk )
  PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_F], work->mcss_sys_wk );
  PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_B], work->mcss_sys_wk );
  
  // 次のを生成
#ifndef DEF_MINIMUM_LOAD
  Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
      POKE_COMP_F, POKE_COMP_B, POKE_COMP_RPOS, work->diff_comp_no );
  PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                            POKE_COMP_F, work );
  PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                            POKE_COMP_B, work );
#else
  {
    {
      POKE_INDEX poke_f = (work->is_poke_front)?(POKE_COMP_F):(POKE_INDEX_NONE);
      POKE_INDEX poke_b = (work->is_poke_front)?(POKE_INDEX_NONE):(POKE_COMP_B);

      Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
          poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );
    
      if( poke_f != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                                  POKE_COMP_F, work );
      }
      if( poke_b != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                                  POKE_COMP_B, work );
      }
    }
  }
#endif

  // テキスト
  // 変更された比較フォルムでテキストを書く
  Zukan_Detail_Form_WritePokeCompText( param, work, cmn );

  // テキストに合わせて、フォルム名の背面フィールドを用意する
  Zukan_Detail_Form_ObjFieldSetup( param, work, cmn );
}

//-------------------------------------
/// ポケアイコン変更
//=====================================
static void Zukan_Detail_Form_ChangePokeicon( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 今表示しているものを非表示にし、次のものを表示するようスワップを入れ替える
  {
    // 今表示しているものを非表示にする
    if( work->pokeicon_clwk[work->pokeicon_swap_curr] )
    {
      GFL_CLACT_WK_SetDrawEnable( work->pokeicon_clwk[work->pokeicon_swap_curr], FALSE );
    }
    // 次のものを表示するようスワップを入れ替える
    work->pokeicon_swap_curr = ( work->pokeicon_swap_curr +1 ) %OBJ_SWAP_MAX;
  }

  // 前のを破棄
  if( work->pokeicon_clwk[work->pokeicon_swap_curr] )
  {
    // NULLチェックはマクロ内で行っている
    BLOCK_POKEICON_EXIT( work->pokeicon_ncg[work->pokeicon_swap_curr], work->pokeicon_clwk[work->pokeicon_swap_curr] )
  } 

  // 次のを生成
  Zukan_Detail_Form_PokeiconInitFromDiffInfo( param, work, cmn, work->diff_curr_no );
}

//-------------------------------------
/// 入力
//=====================================
static void Zukan_Detail_Form_Input( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
  switch(work->state)
  {
  case STATE_TOP:
    {
      if( work->input_enable )
      {
        BUTTON_OBJ curr_push_button;

        // 前後ボタン、再生ボタン  // トップ画面でフォルムを切り替えるための矢印
        curr_push_button = Zukan_Detail_Form_ObjButtonCheckPush( param, work, cmn );
        if( curr_push_button != BUTTON_OBJ_NONE )
        {
          switch( curr_push_button )
          {
          case BUTTON_OBJ_FRONT_BACK:
            {
              Zukan_Detail_Form_FlipFrontBack( param, work, cmn );
            }
            break;
          case BUTTON_OBJ_PLAY:
            {
              u8 i;
#ifdef DEF_MINIMUM_LOAD
              Zukan_Detail_Form_MakePokeWhenAnimeStart( param, work, cmn );
#endif
              for( i=0; i<POKE_MAX; i++ )
              {
                PokeMcssAnimeStart( &work->poke_mcss_wk[i] );  // この関数内でNULLチェックはしてくれている
              }
            }
            break;
          case BUTTON_OBJ_ARROW_L:
            {
              work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
              Zukan_Detail_Form_OshidashiChangeCompareForm( param, work, cmn );
              Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP_OSHIDASHI );
            }
            break;
          case BUTTON_OBJ_ARROW_R:
            {
              work->oshidashi_direct = OSHIDASHI_DIRECT_L_TO_R;
              Zukan_Detail_Form_OshidashiChangeCompareForm( param, work, cmn );
              Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP_OSHIDASHI );
            }
            break;
          }
        }
        else
        {
          u32 x, y;
          BOOL change_state = FALSE;
          int tk;  // Touch or Key
          
          if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
          {
            change_state = TRUE;
            tk = GFL_APP_END_KEY;
          }
          else
          {
	  	      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
            {
              if( 0<=y&&y<168)
              {
                change_state = TRUE;
                tk = GFL_APP_END_TOUCH;
              }
            }
          }
          if( change_state )
          {
            if( work->diff_num >= 2 )
            {
#ifdef DEF_MINIMUM_LOAD
              if(    ( ( (work->is_poke_front)) && (!(work->poke_mcss_wk[POKE_COMP_F].poke_wk)) )
                  || ( (!(work->is_poke_front)) && (!(work->poke_mcss_wk[POKE_COMP_B].poke_wk)) ) )
              {
                // (1) EXCHANGE画面からTOP画面に戻ってくる
                // (2) TOP画面で前後を入れ替える
                // (3) TOP画面からEXCHANGE画面に行く
                // という一連の操作を行ったときに、フォルムの入れ替えは行っていないので、
                // 最初のEXCHANGE画面と同じフォルムを、これから行くEXCHANGE画面でも表示したい。
                // だから、Zukan_Detail_Form_OshidashiChangeCompareFormではなく、
                // Zukan_Detail_Form_ChangeCompareFormとZukan_Detail_Form_OshidashiSetPosCompareFormを用いることにした。
                // 訂正↓
                // Zukan_Detail_Form_ChangeCompareFormと同様の機能を持つ関数Zukan_Detail_Form_KaisouChangeCompareFormを用意したので、
                // Zukan_Detail_Form_KaisouChangeCompareFormとZukan_Detail_Form_OshidashiSetPosCompareFormを用いることにした。
                
                //Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
                Zukan_Detail_Form_KaisouChangeCompareForm( param, work, cmn );

                // 押し出し用関数を利用して位置設定
                work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
                Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
              }
#endif

              //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );
              Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP_TO_EXCHANGE );
              PMSND_PlaySE( SEQ_SE_DECIDE1 );
              GFL_UI_SetTouchOrKey( tk );
              ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );
            }
          }
        }
      }
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->input_enable )
      {
        // タッチ継続中の場合は、それが最優先となる
        if( work->bar_cursor_move_by_touch )
        {
          // スクロールバーの操作チェックは他の関数で行っているので、ここでは行わない
        }
        else
        {
          BUTTON_OBJ curr_push_button;

          // 前後ボタン、再生ボタン
          curr_push_button = Zukan_Detail_Form_ObjButtonCheckPush( param, work, cmn );
          if( curr_push_button != BUTTON_OBJ_NONE )
          {
            switch( curr_push_button )
            {
            case BUTTON_OBJ_FRONT_BACK:
              {
                Zukan_Detail_Form_FlipFrontBack( param, work, cmn );
              }
              break;
            case BUTTON_OBJ_PLAY:
              {
                u8 i;
#ifdef DEF_MINIMUM_LOAD
                Zukan_Detail_Form_MakePokeWhenAnimeStart( param, work, cmn );
#endif
                for( i=0; i<POKE_MAX; i++ )
                {
                  PokeMcssAnimeStart( &work->poke_mcss_wk[i] );  // この関数内でNULLチェックはしてくれている
                }
              }
              break;
            }
          }
        }
      }
    }
    break;
  }
}

//-------------------------------------
/// 状態を遷移させる
//=====================================
static void Zukan_Detail_Form_ChangeState( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                STATE state )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  ZUKAN_DETAIL_HEADBAR_WORK*   headbar  = ZKNDTL_COMMON_GetHeadbar(cmn);
  BOOL b_top_exchange = FALSE;  // STATE_TOPからSTATE_EXCHANGEに変更するとき、もしくはその逆のときTRUE

  // 遷移前の状態 
  switch(work->state)
  {
  case STATE_TOP:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_TOP_OSHIDASHI:
        {
          if( work->oshidashi_state == OSHIDASHI_STATE_NONE )
          {
#ifdef DEF_MINIMUM_LOAD
            // 押し出すフォルムのアニメは止めておく
            PokeMcssAnimeStop( &work->poke_mcss_wk[POKE_COMP_F] );  // この関数内でNULLチェックはしてくれている
            PokeMcssAnimeStop( &work->poke_mcss_wk[POKE_COMP_B] );  // この関数内でNULLチェックはしてくれている
#endif

            work->oshidashi_state = OSHIDASHI_STATE_MOVE;
          }
          else
          {
            // 何もしない
          }
        }
        break;
      case STATE_TOP_TO_EXCHANGE:
        {
          work->kaisou_curr_end = FALSE;
          work->kaisou_comp_end = FALSE;

          // 押し出し用関数を利用して位置設定
          if( /*work->state == STATE_TOP &&*/ work->diff_num >= 2 ) 
          {
            work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
            Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
          }
        }
        break;
      }
    }
    break;
  case STATE_TOP_OSHIDASHI:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_TOP:
        {
          if( work->push_button == BUTTON_OBJ_NONE )  // ボタン押し中のときは、その押しがおさまってからアクティブに戻す
          {
            ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
          }
        }
        break;
      }
    }
    break;
  case STATE_TOP_TO_EXCHANGE:
    {
      // 遷移後の状態
      switch(state)
      {
/*
      case STATE_EXCHANGE:
        {
          b_top_exchange = TRUE;

          if( work->diff_num >= 2 )
          {
            VecFx32 p;

            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );

            PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
          }

          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
          if( work->diff_num >= 3 )
          {
            ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
          }
          else
          {
            ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
          }

          // タイトルバー
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM_COMPARE );  
        }
        break;
*/
      case STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE:
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll(
              touchbar,
              FALSE );
        }
        break;
      }
    }
    break;
  case STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_EXCHANGE:
        {
          b_top_exchange = TRUE;

          if( work->diff_num >= 2 )
          {
            VecFx32 p;

#ifndef DEF_MINIMUM_LOAD
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );

            PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_LEFT, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_LEFT, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );

            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
#else
            if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_LEFT, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
            }
            if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_LEFT, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
            }

#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
            PokeGetCompareRelativePosition( work->poke_mcss_wk[(work->is_poke_front)?POKE_CURR_F:POKE_CURR_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
            
            if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
            }
            if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL 
            }
#endif
          }

          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_FORM,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM,
              (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE );
          if( work->diff_num >= 3 )
          {
            ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, TRUE );
          }
          else
          {
            ZUKAN_DETAIL_TOUCHBAR_SetVisibleOfFormCurLR( touchbar, FALSE );
          }

          // タイトルバー
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM_COMPARE );  
        }
        break;
      }
    }
    break;
  case STATE_EXCHANGE:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_EXCHANGE_IREKAE:
        {
          if( work->irekae_state == IREKAE_STATE_NONE )
          {
            work->irekae_state = IREKAE_STATE_MOVE;
            work->irekae_theta = 0;
          }
          else
          {
            // 何もしない
          }
        }
        break;
      case STATE_EXCHANGE_TO_TOP:
        {
          work->kaisou_curr_end = FALSE;
          work->kaisou_comp_end = FALSE;
        }
        break;
      }
    }
    break;
  case STATE_EXCHANGE_IREKAE:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_EXCHANGE:
        {
#ifndef DEF_SCMD_CHANGE
          ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
#endif

#ifdef DEF_SCMD_CHANGE
          if( work->irekae_by_exchange_flag )
          {
            work->irekae_by_exchange_poke_finish = TRUE;
            if( work->irekae_by_exchange_icon_finish )
            {
              work->irekae_by_exchange_flag = FALSE;
              ZUKAN_DETAIL_TOUCHBAR_Unlock( touchbar );
            }
          }
#endif
        }
        break;
      }
    }
    break;
  case STATE_EXCHANGE_TO_TOP:
    {
      // 遷移後の状態
      switch(state)
      {
/*
      case STATE_TOP:
        {
          b_top_exchange = TRUE;

          if( work->diff_num >= 2 )
          {
            VecFx32 p;

            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
      
            PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
          }

          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM );
          {
            GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
            ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
                touchbar,
                GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_FORM ) );
          }
          // タイトルバー
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM );

          // 押し出し用関数を利用して位置設定
          if( work->diff_num >= 2 ) 
          {
            work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
            Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
          }
        }
        break;
*/
      case STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE:
        {
          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetVisibleAll(
              touchbar,
              FALSE );
        }
        break;
      }
    }
    break;
  case STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE:
    {
      // 遷移後の状態
      switch(state)
      {
      case STATE_TOP:
        {
          b_top_exchange = TRUE;

          if( work->diff_num >= 2 )
          {
            VecFx32 p;

#ifndef DEF_MINIMUM_LOAD
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
      
            PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_CENTER, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_CENTER, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
      
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
#else
            if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_CENTER, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
            if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_CENTER, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }

            PokeGetCompareRelativePosition( work->poke_mcss_wk[(work->is_poke_front)?POKE_CURR_F:POKE_CURR_B].poke_wk, &p );
            
            if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
            if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
#endif
          }

          // タッチバー
          ZUKAN_DETAIL_TOUCHBAR_SetType(
              touchbar,
              ZUKAN_DETAIL_TOUCHBAR_TYPE_GENERAL,
              ZUKAN_DETAIL_TOUCHBAR_DISP_FORM,
              (ZKNDTL_COMMON_GetPokeNum(cmn)>1)?TRUE:FALSE ); 
          {
            GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
            ZUKAN_DETAIL_TOUCHBAR_SetCheckFlipOfGeneral(
                touchbar,
                GAMEDATA_GetShortCut( gamedata, SHORTCUT_ID_ZUKAN_FORM ) );
          }
          // タイトルバー
          ZUKAN_DETAIL_HEADBAR_SetType(
              headbar,
              ZUKAN_DETAIL_HEADBAR_TYPE_FORM );

          // 押し出し用関数を利用して位置設定
          if( /*work->state == STATE_TOP &&*/ work->diff_num >= 2 ) 
          {
            work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
            Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
          }
        }
        break;
      }
    }
    break;
  }

  // 遷移後の状態
  work->state = state;

  if( b_top_exchange )
  {
    // テキスト
    // 遷移された状態で、テキストをスクロールさせて、比較フォルムのテキストを書くor消す
    Zukan_Detail_Form_ScrollPokeText( param, work, cmn );
    Zukan_Detail_Form_WritePokeCompText( param, work, cmn );

    // テキストに合わせて、フォルム名の背面フィールドを用意する
    Zukan_Detail_Form_ObjFieldSetup( param, work, cmn );

    // 変更された状態用に、スクロールバーを用意する
    Zukan_Detail_Form_ObjBarSetup( param, work, cmn );

    // 変更された状態用に、トップ画面でフォルムを切り替えるための矢印を用意する
    Zukan_Detail_Form_ObjButtonArrowSetup( param, work, cmn );

    // 入力可不可やボタン押し情報を元に戻す
    work->push_button = BUTTON_OBJ_NONE;
    ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
  }
}

//-------------------------------------
/// OBJ  // 前後ボタン、再生ボタン、スクロールバー、バーカーソル  // フォルム名の背面フィールド
//=====================================
static void Zukan_Detail_Form_ObjBaseCreate( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_ZUKAN_GRA, param->heap_id );

    work->obj_res[OBJ_RES_BUTTON_BAR_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_zukan_gra_info_info_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_BUTTON_BAR*0x20,
                                     0,
                                     OBJ_PAL_NUM_M_BUTTON_BAR,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_BUTTON_BAR_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_zukan_gra_info_info_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_BUTTON_BAR_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_zukan_gra_info_info_obj_NCER,
                                     NARC_zukan_gra_info_info_obj_NANR,
                                     param->heap_id );

    work->obj_res[OBJ_RES_FIELD_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_zukan_gra_info_formewin_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_FIELD*0x20,
                                     0,
                                     OBJ_PAL_NUM_M_FIELD,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_FIELD_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_zukan_gra_info_formewin_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_FIELD_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_zukan_gra_info_formewin_obj_NCER,
                                     NARC_zukan_gra_info_formewin_obj_NANR,
                                     param->heap_id );

    work->obj_res[OBJ_RES_ARROW_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCLR,
                                     CLSYS_DRAW_MAIN,
                                     OBJ_PAL_POS_M_ARROW*0x20,
                                     0,
                                     OBJ_PAL_NUM_M_ARROW,
                                     param->heap_id );	
    work->obj_res[OBJ_RES_ARROW_NCG] = GFL_CLGRP_CGR_Register(
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCGR,
                                     FALSE,
                                     CLSYS_DRAW_MAIN,
                                     param->heap_id );
    work->obj_res[OBJ_RES_ARROW_NCE] = GFL_CLGRP_CELLANIM_Register(
                                     handle,
                                     NARC_zukan_gra_info_mapwin_obj_NCER,
                                     NARC_zukan_gra_info_mapwin_obj_NANR,
                                     param->heap_id );

    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    u8 i;
    GFL_CLWK_DATA cldata;

    for( i=0; i<OBJ_MAX; i++ )
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x      = obj_setup_info[i][0];
      cldata.pos_y      = obj_setup_info[i][1];
      cldata.anmseq     = obj_setup_info[i][2];
      cldata.softpri    = obj_setup_info[i][3];
      cldata.bgpri      = obj_setup_info[i][4];

      work->obj_clwk[i] = GFL_CLACT_WK_Create(
                             work->clunit,
                             work->obj_res[obj_setup_info[i][5]], work->obj_res[obj_setup_info[i][6]], work->obj_res[obj_setup_info[i][7]],
                             &cldata,
                             CLSYS_DEFREND_MAIN,
                             param->heap_id );

      GFL_CLACT_WK_SetAutoAnmFlag( work->obj_clwk[i], TRUE );
      GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[i], FALSE );
      GFL_CLACT_WK_SetObjMode( work->obj_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }

  // ボタン
  {
    work->button[BUTTON_OBJ_FRONT_BACK].set_pos_x              = 8*8;
    work->button[BUTTON_OBJ_FRONT_BACK].set_pos_y              = 19*8;
    work->button[BUTTON_OBJ_FRONT_BACK].set_ncg                = OBJ_RES_BUTTON_BAR_NCG;
    work->button[BUTTON_OBJ_FRONT_BACK].set_ncl                = OBJ_RES_BUTTON_BAR_NCL;
    work->button[BUTTON_OBJ_FRONT_BACK].set_nce                = OBJ_RES_BUTTON_BAR_NCE;
    work->button[BUTTON_OBJ_FRONT_BACK].pos_x                  = 8*8;
    work->button[BUTTON_OBJ_FRONT_BACK].pos_y                  = 19*8;
    work->button[BUTTON_OBJ_FRONT_BACK].size_x                 = 8*8;
    work->button[BUTTON_OBJ_FRONT_BACK].size_y                 = 8*2;
    work->button[BUTTON_OBJ_FRONT_BACK].anmseq_active          = 24;
    work->button[BUTTON_OBJ_FRONT_BACK].anmseq_push            = 26;
    work->button[BUTTON_OBJ_FRONT_BACK].key                    = PAD_BUTTON_SELECT;
    work->button[BUTTON_OBJ_FRONT_BACK].se                     = SEQ_SE_DECIDE1;
    work->button[BUTTON_OBJ_FRONT_BACK].state                  = BUTTON_STATE_ACTIVE;
    work->button[BUTTON_OBJ_FRONT_BACK].clwk                   = NULL;

    work->button[BUTTON_OBJ_PLAY].set_pos_x              = 16*8;
    work->button[BUTTON_OBJ_PLAY].set_pos_y              = 19*8;
    work->button[BUTTON_OBJ_PLAY].set_ncg                = OBJ_RES_BUTTON_BAR_NCG;
    work->button[BUTTON_OBJ_PLAY].set_ncl                = OBJ_RES_BUTTON_BAR_NCL;
    work->button[BUTTON_OBJ_PLAY].set_nce                = OBJ_RES_BUTTON_BAR_NCE;
    work->button[BUTTON_OBJ_PLAY].pos_x                  = 16*8;
    work->button[BUTTON_OBJ_PLAY].pos_y                  = 19*8;
    work->button[BUTTON_OBJ_PLAY].size_x                 = 8*8;
    work->button[BUTTON_OBJ_PLAY].size_y                 = 8*2;
    work->button[BUTTON_OBJ_PLAY].anmseq_active          = 23;
    work->button[BUTTON_OBJ_PLAY].anmseq_push            = 25;
    work->button[BUTTON_OBJ_PLAY].key                    = PAD_BUTTON_START;
    work->button[BUTTON_OBJ_PLAY].se                     = SEQ_SE_DECIDE1;
    work->button[BUTTON_OBJ_PLAY].state                  = BUTTON_STATE_ACTIVE;
    work->button[BUTTON_OBJ_PLAY].clwk                   = NULL;

    work->button[BUTTON_OBJ_ARROW_L].set_pos_x              = 128;
    work->button[BUTTON_OBJ_ARROW_L].set_pos_y              = 96;
    work->button[BUTTON_OBJ_ARROW_L].set_ncg                = OBJ_RES_ARROW_NCG;
    work->button[BUTTON_OBJ_ARROW_L].set_ncl                = OBJ_RES_ARROW_NCL;
    work->button[BUTTON_OBJ_ARROW_L].set_nce                = OBJ_RES_ARROW_NCE;
    work->button[BUTTON_OBJ_ARROW_L].pos_x                  = 0;
    work->button[BUTTON_OBJ_ARROW_L].pos_y                  = 0;
    work->button[BUTTON_OBJ_ARROW_L].size_x                 = 24;
    work->button[BUTTON_OBJ_ARROW_L].size_y                 = 16;
    work->button[BUTTON_OBJ_ARROW_L].anmseq_active          = 4;
    work->button[BUTTON_OBJ_ARROW_L].anmseq_push            = 5;
    work->button[BUTTON_OBJ_ARROW_L].key                    = PAD_BUTTON_L;
    work->button[BUTTON_OBJ_ARROW_L].se                     = SEQ_SE_SELECT3;
    work->button[BUTTON_OBJ_ARROW_L].state                  = BUTTON_STATE_ACTIVE;
    work->button[BUTTON_OBJ_ARROW_L].clwk                   = NULL;

    work->button[BUTTON_OBJ_ARROW_R].set_pos_x              = 228;
    work->button[BUTTON_OBJ_ARROW_R].set_pos_y              = 96;
    work->button[BUTTON_OBJ_ARROW_R].set_ncg                = OBJ_RES_ARROW_NCG;
    work->button[BUTTON_OBJ_ARROW_R].set_ncl                = OBJ_RES_ARROW_NCL;
    work->button[BUTTON_OBJ_ARROW_R].set_nce                = OBJ_RES_ARROW_NCE;
    work->button[BUTTON_OBJ_ARROW_R].pos_x                  = 232;
    work->button[BUTTON_OBJ_ARROW_R].pos_y                  = 0;
    work->button[BUTTON_OBJ_ARROW_R].size_x                 = 24;
    work->button[BUTTON_OBJ_ARROW_R].size_y                 = 16;
    work->button[BUTTON_OBJ_ARROW_R].anmseq_active          = 2;
    work->button[BUTTON_OBJ_ARROW_R].anmseq_push            = 3;
    work->button[BUTTON_OBJ_ARROW_R].key                    = PAD_BUTTON_R;
    work->button[BUTTON_OBJ_ARROW_R].se                     = SEQ_SE_SELECT3;
    work->button[BUTTON_OBJ_ARROW_R].state                  = BUTTON_STATE_ACTIVE;
    work->button[BUTTON_OBJ_ARROW_R].clwk                   = NULL;
  }

  // ボタンCLWK作成
  {
    u8 i;
    GFL_CLWK_DATA cldata;

    for( i=0; i<BUTTON_OBJ_MAX; i++ )
    {
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
      cldata.pos_x      = work->button[i].set_pos_x;
      cldata.pos_y      = work->button[i].set_pos_y;
      cldata.anmseq     = work->button[i].anmseq_active;
      cldata.softpri    = 0;
      cldata.bgpri      = BG_FRAME_PRI_M_REAR;

      work->button[i].clwk = GFL_CLACT_WK_Create(
                             work->clunit,
                             work->obj_res[work->button[i].set_ncg], work->obj_res[work->button[i].set_ncl], work->obj_res[work->button[i].set_nce],
                             &cldata,
                             CLSYS_DEFREND_MAIN,
                             param->heap_id );

      GFL_CLACT_WK_SetAutoAnmFlag( work->button[i].clwk, TRUE );
      GFL_CLACT_WK_SetObjMode( work->button[i].clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }

  work->push_button = BUTTON_OBJ_NONE;
}
static void Zukan_Detail_Form_ObjBaseDelete( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // ボタンCLWK作成
  {
    u8 i;
    for( i=0; i<BUTTON_OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_Remove( work->button[i].clwk );
    }
  }

  // CLWK破棄
  {
    u8 i;
    for( i=0; i<OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_Remove( work->obj_clwk[i] );
    }
  }

  // リソース破棄
  {
    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_BUTTON_BAR_NCL] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_BUTTON_BAR_NCG] );
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_BUTTON_BAR_NCE] );

    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_FIELD_NCL] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_FIELD_NCG] );
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_FIELD_NCE] );

    GFL_CLGRP_PLTT_Release( work->obj_res[OBJ_RES_ARROW_NCL] );
    GFL_CLGRP_CGR_Release( work->obj_res[OBJ_RES_ARROW_NCG] );
    GFL_CLGRP_CELLANIM_Release( work->obj_res[OBJ_RES_ARROW_NCE] );
  }
}

static BUTTON_OBJ Zukan_Detail_Form_ObjButtonCheckPush( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 呼び出し元には、押した瞬間にはwork->push_buttonと同じものを返し、そうでないときはBUTTON_OBJ_NONEやBUTTON_OBJ_IGNOREを返す。
  
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);
  
  u8 i;
  u32 x, y;
  BUTTON_OBJ curr_push_button = BUTTON_OBJ_NONE;  // 今押したボタン  // 戻り値
  int tk;  // Touch or Key

  // キー
  if( curr_push_button == BUTTON_OBJ_NONE )
  {
    for( i=0; i<BUTTON_OBJ_MAX; i++ )
    {
      BOOL on_off = GFL_CLACT_WK_GetDrawEnable( work->button[i].clwk );
      if( on_off )
      {
        if( GFL_UI_KEY_GetTrg() & work->button[i].key )
        {
          tk = GFL_APP_END_KEY;
          curr_push_button = i;
          break;
        }
        else
        {
          // キーリピートに対応したボタン
          if( i == BUTTON_OBJ_ARROW_L || i == BUTTON_OBJ_ARROW_R )
          {
            if( GFL_UI_KEY_GetRepeat() & work->button[i].key )
            {
              tk = GFL_APP_END_KEY;
              curr_push_button = i;
              break;
            }
          }
        }
      }
    }
  }

  // タッチ
  if( curr_push_button == BUTTON_OBJ_NONE )
  {
    if( GFL_UI_TP_GetPointTrg( &x, &y ) )
    {
      for( i=0; i<BUTTON_OBJ_MAX; i++ )
      {
        BOOL on_off = GFL_CLACT_WK_GetDrawEnable( work->button[i].clwk );
        if( on_off )
        {
          if(    work->button[i].pos_x <= x && x < work->button[i].pos_x + work->button[i].size_x
              && work->button[i].pos_y <= y && y < work->button[i].pos_y + work->button[i].size_y )
          {
            tk = GFL_APP_END_TOUCH;
            curr_push_button = i;
            break;
          }
        }
      }
    }
  }

  if( curr_push_button != BUTTON_OBJ_NONE )
  {
    // 2つのボタンは同時に押せない、かつ、押し中のボタンは押せない
    if( work->push_button != BUTTON_OBJ_NONE )
    {
      curr_push_button = BUTTON_OBJ_IGNORE;
    }
    else
    {
      // ボタン押し確定
      GFL_UI_SetTouchOrKey( tk );
      ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );
      
      work->button[curr_push_button].state = BUTTON_STATE_PUSH_START;
      GFL_CLACT_WK_SetAnmSeq( work->button[curr_push_button].clwk, work->button[curr_push_button].anmseq_push );
      PMSND_PlaySE( work->button[curr_push_button].se );

      // アニメーションを合わせておく必要があるボタン
      {
        BUTTON_OBJ curr_pair_button = BUTTON_OBJ_NONE;
        if( curr_push_button == BUTTON_OBJ_ARROW_L )
        {
          curr_pair_button = BUTTON_OBJ_ARROW_R;
        }
        else if( curr_push_button == BUTTON_OBJ_ARROW_R )
        {
          curr_pair_button = BUTTON_OBJ_ARROW_L;
        }
        if( curr_pair_button != BUTTON_OBJ_NONE )
        {
          GFL_CLACT_WK_SetAutoAnmFlag( work->button[curr_pair_button].clwk, FALSE );
          GFL_CLACT_WK_SetAnmFrame( work->button[curr_pair_button].clwk, 0 );
        }
      }

      work->push_button = curr_push_button;
    }
  }

  return curr_push_button; 
}

static void Zukan_Detail_Form_ObjButtonArrowSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 現在の状態で、トップ画面でフォルムを切り替えるための矢印を準備する

  BOOL disp = FALSE;
  switch(work->state)
  {
  case STATE_TOP:
    {
      if( work->diff_num >= 2 )
      {
        disp = TRUE;  // 表示する
      }
    }
    break;
  case STATE_EXCHANGE:
    {
      // 表示しない
    }
    break;
  }

  if( disp )
  {
    GFL_CLACT_WK_SetDrawEnable( work->button[BUTTON_OBJ_ARROW_L].clwk, TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->button[BUTTON_OBJ_ARROW_R].clwk, TRUE );
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( work->button[BUTTON_OBJ_ARROW_L].clwk, FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->button[BUTTON_OBJ_ARROW_R].clwk, FALSE );
  }
}

static void Zukan_Detail_Form_ObjButtonMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  if( work->push_button == BUTTON_OBJ_NONE )
  {
    return;
  }

  {
    switch( work->button[work->push_button].state )
    {
    case BUTTON_STATE_ACTIVE:
      {
        // 何もしない
      }
      break;
    case BUTTON_STATE_PUSH_START:  // この関数の呼び出し位置によっては、この状態には1フレームもなっていないかも。
      {
        work->button[work->push_button].state = BUTTON_STATE_PUSH_ANIME;
      }
      break;
    case BUTTON_STATE_PUSH_ANIME:
      {
        if( !GFL_CLACT_WK_CheckAnmActive( work->button[work->push_button].clwk ) )
        {
          work->button[work->push_button].state = BUTTON_STATE_PUSH_END;
        } 
      }
      break;
    case BUTTON_STATE_PUSH_END:
      {
        GFL_CLACT_WK_SetAnmSeq( work->button[work->push_button].clwk, work->button[work->push_button].anmseq_active );
        work->button[work->push_button].state = BUTTON_STATE_ACTIVE;

        // アニメーションを合わせておく必要があるボタン
        {
          BUTTON_OBJ pair_button = BUTTON_OBJ_NONE;
          if( work->push_button == BUTTON_OBJ_ARROW_L )
          {
            pair_button = BUTTON_OBJ_ARROW_R;
          }
          else if( work->push_button == BUTTON_OBJ_ARROW_R )
          {
            pair_button = BUTTON_OBJ_ARROW_L;
          }
          if( pair_button != BUTTON_OBJ_NONE )
          {
            if( work->button[pair_button].state == BUTTON_STATE_ACTIVE )
            {
              GFL_CLACT_WK_SetAutoAnmFlag( work->button[pair_button].clwk, TRUE );
              GFL_CLACT_WK_SetAnmSeq( work->button[pair_button].clwk, work->button[pair_button].anmseq_active );
            }
          }
        }

        work->push_button = BUTTON_OBJ_NONE;

        if(    work->state == STATE_TOP
            || work->state == STATE_EXCHANGE )  // 変動中の状態のときは、その変動がおさまってからアクティブに戻す
        {
          ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
        }
      }
      break;
    }
  }
}

static void Zukan_Detail_Form_ObjFieldSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  GFL_CLACTPOS pos;
  u8           anmseq;
  BOOL         field_curr = TRUE;
  BOOL         field_comp = FALSE;

  switch(work->state)
  {
  case STATE_TOP:
    {
      field_curr = TRUE;
      field_comp = FALSE;
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 2 )
      {
        field_curr = TRUE;
        field_comp = TRUE;
      }
      else
      {
        field_curr = TRUE;
        field_comp = FALSE;
      }
    }
    break;
  }

  if( field_curr && field_comp )
  {
    pos.x = 0;  pos.y = 0; 
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_FIELD_CURR], &pos, CLSYS_DEFREND_MAIN );
    pos.x = 16*8;  pos.y = 0; 
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_FIELD_COMP], &pos, CLSYS_DEFREND_MAIN );
  }
  else if( field_curr )
  {
    pos.x = 8*8;  pos.y = 0; 
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_FIELD_CURR], &pos, CLSYS_DEFREND_MAIN );
  }

  // field_curr
  {
    anmseq = ( work->diff_info_list[work->diff_curr_no].color_diff == COLOR_DIFF_SPECIAL ) ? (1):(0);
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_FIELD_CURR], TRUE );
    GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_FIELD_CURR], anmseq );
  }

  if( field_comp )
  {
    anmseq = ( work->diff_info_list[work->diff_comp_no].color_diff == COLOR_DIFF_SPECIAL ) ? (1):(0);
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_FIELD_COMP], TRUE );
    GFL_CLACT_WK_SetAnmSeq( work->obj_clwk[OBJ_FIELD_COMP], anmseq );
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_FIELD_COMP], FALSE );
  }
}

static void Zukan_Detail_Form_ObjBarSetup( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 現在の状態でスクロールバーを準備する

  BOOL disp = FALSE;
  switch(work->state)
  {
  case STATE_TOP:
    {
      // 表示しない
    }
    break;
  case STATE_EXCHANGE:
    {
      if( work->diff_num >= 3 )
      {
        disp = TRUE;  // 表示する
      }
      else
      {
        // 表示しない
      }
    }
    break;
  }

  if( disp )
  {
    u8  pos_x_center;
    GFL_CLACTPOS pos;
    u16 no = Zukan_Detail_Form_GetNoExceptOne( param, work, cmn,
                 work->diff_curr_no, work->diff_comp_no );
    Zukan_Detail_Form_ObjBarGetPosX( work->diff_num -1, no, NULL, &pos_x_center, NULL );
    pos.x = pos_x_center;
    pos.y = BAR_CURSOR_POS_Y;
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_BAR_CURSOR], &pos, CLSYS_DEFREND_MAIN );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_BAR_RANGE], TRUE );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_BAR_CURSOR], TRUE );
    // 初期化 
    work->bar_cursor_move_by_key    = FALSE;
#ifdef DEF_SCMD_CHANGE
    work->change_form_by_cur_l_r_flag = FALSE;
#endif
    work->bar_cursor_move_by_touch  = FALSE;
  }
  else
  {
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_BAR_RANGE], FALSE );
    GFL_CLACT_WK_SetDrawEnable( work->obj_clwk[OBJ_BAR_CURSOR], FALSE );
  }
}
static void Zukan_Detail_Form_ObjBarMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL ret = FALSE;

#ifndef DEF_SCMD_CHANGE
  if(    work->state == STATE_EXCHANGE
      && work->input_enable
      && work->push_button == BUTTON_OBJ_NONE )
#else
   if(    (    work->state == STATE_EXCHANGE
            && work->input_enable
            && work->push_button == BUTTON_OBJ_NONE )
       || ( work->change_form_by_cur_l_r_flag       ) )
#endif
  {
    // タッチ継続中の場合は、それが最優先となる
    // だから、タッチ継続中の場合は、bar_cursor_move_by_keyはTRUEにならない。
    // bar_cursor_move_by_keyがTRUEになっているということは、タッチしていないときにキー入力があったということ。
    // だからそのときは、それを片付けてからタッチを調べるようにする。
    
    // キー
    if( !ret )
    {
      ret = Zukan_Detail_Form_ObjBarMainKey( param, work, cmn );
    }
    // タッチ
    if( !ret )
    {
      ret = Zukan_Detail_Form_ObjBarMainTouch( param, work, cmn );    
    }
  }
}

static BOOL Zukan_Detail_Form_ObjBarMainTouch( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  ZUKAN_DETAIL_TOUCHBAR_WORK*  touchbar = ZKNDTL_COMMON_GetTouchbar(cmn);

  BOOL b_se = FALSE;  // TRUEのとき、タッチ音を鳴らす
  u32 x, y;
  
  // タッチ継続中
  if( work->bar_cursor_move_by_touch )
  {
    if( GFL_UI_TP_GetPointCont( &x, &y ) )
    {
      // 状況変更なし
    }
    else
    {
      work->bar_cursor_move_by_touch = FALSE;
      ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, TRUE );
    }
  }
  // 新たにスクロールバーをタッチしたか検出する
  else
  {
    if( work->diff_num >= 3 )  // スクロールバーが表示されていなければタッチできない
    {
      if( GFL_UI_TP_GetPointTrg( &x, &y ) )
      {
        if(    BAR_RANGE_TOUCH_X_MIN<=x&&x<=BAR_RANGE_TOUCH_X_MAX
            && BAR_RANGE_TOUCH_Y_MIN<=y&&y<=BAR_RANGE_TOUCH_Y_MAX )
        {
          work->bar_cursor_move_by_touch = TRUE;
          ZUKAN_DETAIL_TOUCHBAR_SetUserActiveWhole( touchbar, FALSE );
          b_se = TRUE;
        }
      }
    }
  }

  // タッチしているので、バーカーソルを動かし、フォルムも変更する
  if( work->bar_cursor_move_by_touch )
  {
    u16 diff_comp_no_prev = work->diff_comp_no;
    u16 no_except_curr;
  
    GFL_CLACTPOS pos;
    x = MATH_CLAMP( x, BAR_CURSOR_POS_X_MIN, BAR_CURSOR_POS_X_MAX );
    pos.x = x;
    pos.y = BAR_CURSOR_POS_Y;
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_BAR_CURSOR], &pos, CLSYS_DEFREND_MAIN );

    no_except_curr = Zukan_Detail_Form_ObjBarGetNo( work->diff_num-1, (u8)x );

    work->diff_comp_no = Zukan_Detail_Form_GetDiffInfoListIdx( param, work, cmn,
                                                               work->diff_curr_no, no_except_curr );

    // 比較フォルム変更
    if( diff_comp_no_prev != work->diff_comp_no )
    {
      Zukan_Detail_Form_ChangeCompareForm( param, work, cmn );
      b_se = TRUE;
    }
  }

  if( b_se )
  {
    PMSND_PlaySE( SEQ_SE_SYS_06 );
  }
 
  if( work->bar_cursor_move_by_touch )
  {
    GFL_UI_SetTouchOrKey( GFL_APP_END_TOUCH );
  }

  return work->bar_cursor_move_by_touch;
}
static BOOL Zukan_Detail_Form_ObjBarMainKey( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  BOOL ret = FALSE;  // キーによる入力があったらTRUE
  if( work->bar_cursor_move_by_key )
  {
    u8  pos_x_center;
    GFL_CLACTPOS pos;
    u16 no = Zukan_Detail_Form_GetNoExceptOne( param, work, cmn,
                 work->diff_curr_no, work->diff_comp_no );
    Zukan_Detail_Form_ObjBarGetPosX( work->diff_num -1, no, NULL, &pos_x_center, NULL );
    pos.x = pos_x_center;
    pos.y = BAR_CURSOR_POS_Y;
    GFL_CLACT_WK_SetPos( work->obj_clwk[OBJ_BAR_CURSOR], &pos, CLSYS_DEFREND_MAIN );
    ret = TRUE;
  } 
  return ret;
}
static void Zukan_Detail_Form_ObjBarGetPosX( u16 num, u16 no, u8* a_pos_x_min, u8* a_pos_x_center, u8* a_pos_x_max )
{
  // num個のデータ中のno番目(0<=no<num)のデータを指すスクロールバーの位置を得る
  // その位置はpos_x_min<=  <pos_x_maxとなる
  // pos_x_min<=pos_x_center<pox_x_max
  // no==0のとき、pos_x_center=pos_x_min
  // no==num-1のとき、pos_x_center=pos_x_max

  u8 pos_x_min, pos_x_center, pos_x_max;

  pos_x_min = ( BAR_CURSOR_POS_X_MAX - BAR_CURSOR_POS_X_MIN +1 ) * no / num + BAR_CURSOR_POS_X_MIN;
  pos_x_max = ( BAR_CURSOR_POS_X_MAX - BAR_CURSOR_POS_X_MIN +1 ) * (no +1) / num + BAR_CURSOR_POS_X_MIN;

  if( no == 0 )                     pos_x_center = pos_x_min;
  else if( no == num -1 )           pos_x_center = pos_x_max -1;
  else if( pos_x_min >= pos_x_max ) pos_x_center = pos_x_min;
  else                              pos_x_center = ( pos_x_min + pos_x_max -1 ) / 2;

  if( a_pos_x_min )    *a_pos_x_min    = pos_x_min;
  if( a_pos_x_max )    *a_pos_x_max    = pos_x_max;
  if( a_pos_x_center ) *a_pos_x_center = pos_x_center;
}

static u16 Zukan_Detail_Form_ObjBarGetNo( u16 num, u8 x )
{
  // 座標xがnum個のデータ中のno番目(0<=no<num)のデータかを得る
  // xが範囲の最小より小さいときは0を返し、xが範囲の最大より大きいときはnum-1を返す
  
  u16 no;
  u8  pos_x_min, pos_x_center, pos_x_max;
  for( no=0; no<num; no++ )
  {
    Zukan_Detail_Form_ObjBarGetPosX( num, no, &pos_x_min, &pos_x_center, &pos_x_max );
    if( no == 0 )
    {
      if( x < pos_x_max )
      {
        break;
      }
    }
    else if( no == num-1 )
    {
      //if( pos_x_min <= x )  // もうここまでbreakせずに来たのなら、これしかないので無条件で
      {
        break;
      }
    }
    else
    {
      if( pos_x_min <= x && x < pos_x_max )
      {
        break;
      }
    }
  }

  return no;
}

static u16 Zukan_Detail_Form_GetNoExceptOne( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                             u16 except_idx, u16 target_idx )
{
  // diff_info_list[target_idx]となるtarget_idxがdiff_info_list中何番目かを求める
  // ただし、diff_info_list[except_idx]となるexcept_idxは除いて何番目かを数える
  // 即ち、target_idxのままか、target_idx-1となる

  u16 ret_no = 0;
  u16 i;
  for( i=0; i<work->diff_num; i++ )
  {
    if( i == except_idx )
    {
      // 何もしない
    }
    else if( i== target_idx )
    {
      break;
    }
    else
    {
      ret_no++;
    }
  }
  return ret_no;
}

static u16 Zukan_Detail_Form_GetDiffInfoListIdx( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn,
                                                 u16 except_idx, u16 no )
{
  // no番目のデータはdiff_info_listの何番目の添え字に当たるかを得る
  // ただし、diff_info_list[except_idx]となるexcept_idxは除いたno番目である
  // 即ち、戻り値ret_idxはret_idx=noかret_idx=no+1となるdiff_info_list[ret_idx]である

  u16 ret_idx = 0;
  u16 no_count = 0;
  u16 i;
  for( i=0; i<work->diff_num; i++ )
  {
    if( i == except_idx )
    {
      // 何もしない
    }
    else
    {
      if( no_count == no )
      {
        ret_idx = i;
        break;
      }
      no_count++;
    }
  }
  return ret_idx;
}

//-------------------------------------
/// 入れ替え
//=====================================
static void Zukan_Detail_Form_IrekaeMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->irekae_state == IREKAE_STATE_MOVE )
  {
    const f32  left_x  = POKE_IREKAE_POS_LEFT_X;
    const f32  right_x = POKE_IREKAE_POS_RIGHT_X;
    const f32  base_y  = POKE_IREKAE_POS_BASE_Y;
    const f32  height  = POKE_IREKAE_HEIGHT;
    const f32  curr_z  = POKE_IREKAE_POS_CURR_Z;  //-10.0f;  //-12800.0f;  // 後ろに描画されるように奥のほうへ配置
    const f32  comp_z  = POKE_IREKAE_POS_COMP_Z;             // 128.0f;    // 前に描画されるように手前のほうへ配置
    fx16 s_fx16;
    f32  s;

    work->irekae_theta += IREKAE_THETA_ADD;
    if( work->irekae_theta >= IREKAE_THETA_MAX )
    {
      work->irekae_theta = IREKAE_THETA_MAX;
      work->irekae_state = IREKAE_STATE_NONE;

      Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );


        if( work->diff_num >= 2 )
        {
#ifndef DEF_MINIMUM_LOAD
          // 位置入れ替え
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
          {
            VecFx32 p;
            PokeGetCompareRelativePosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
          }
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
          {
            VecFx32 p;

            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_CURR_POS_LEFT, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_CURR_POS_LEFT, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );

            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
            PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_COMP_RPOS, &p );
            MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
          }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
          
          // currとcompの番号、ポインタ入れ替え
          Zukan_Detail_Form_SwapCurrComp( param, work, cmn );
#else
          // 位置入れ替え
          {
            VecFx32 p;

            if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_F], POKE_CURR_POS_LEFT, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
            if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &poke_pos[POKE_CURR_POS_LEFT] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_COMP_B], POKE_CURR_POS_LEFT, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }

            PokeGetCompareRelativePosition( work->poke_mcss_wk[(work->is_poke_front)?POKE_COMP_F:POKE_COMP_B].poke_wk, &p );
            
            if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
            if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
            {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
              PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_COMP_RPOS, &p );
              MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
            }
          }

          // currとcompの番号、ポインタ入れ替え
          Zukan_Detail_Form_SwapCurrComp( param, work, cmn );
#endif

          // ポケアイコン変更
          Zukan_Detail_Form_ChangePokeicon( param, work, cmn );

          // テキスト
          // テキストはOBJではなくBGなので、入れ替えできないので丸々書き直し
          Zukan_Detail_Form_WritePokeCurrText( param, work, cmn );
          Zukan_Detail_Form_WritePokeCompText( param, work, cmn );

          // テキストに合わせて、フォルム名の背面フィールドを用意する
          Zukan_Detail_Form_ObjFieldSetup( param, work, cmn );

          // 図鑑セーブデータに閲覧中データ設定
          {
            u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

            GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
            ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );

            u32  sex  = work->diff_info_list[work->diff_curr_no].sex;
            BOOL rare = (work->diff_info_list[work->diff_curr_no].rare==0)?FALSE:TRUE;
            u32  form = work->diff_info_list[work->diff_curr_no].form;

            ZUKANSAVE_SetDrawData(  zkn_sv, monsno, sex, rare, form );

#ifdef DEBUG_KAWADA
            {
              OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, diff_no=%d, f=%d, s=%d, r=%d\n", monsno, work->diff_curr_no, form, sex, rare );
            }
#endif
#ifdef DEBUG_KAWADA
            {
              ZUKANSAVE_GetDrawData(  zkn_sv, monsno, &sex, &rare, &form, param->heap_id );
              OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, ZUKANSAVE_GetDrawData, f=%d, s=%d, r=%d\n", monsno, form, sex, rare );
            }
#endif
          }

          if( work->diff_num >= 3 )
          {
            work->bar_cursor_move_by_key = TRUE;
          }
        }
   

    }
    else
    {
      s_fx16 = FX_SinIdx(work->irekae_theta);
      s = FX_FX16_TO_F32(s_fx16); 

#ifndef DEF_MINIMUM_LOAD
      // POKE_CURR_
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        f32 x = ( right_x - left_x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_x;
        f32 y = height * s + base_y;
        VecFx32 p;
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(curr_z);
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
        POKE_VEC_F32 left_vec;
        POKE_VEC_F32 right_vec;
        f32 x;
        f32 y;
        VecFx32 p;

        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
        x = ( right_vec.x - left_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_vec.x;
        y = height * s + left_vec.y;  // 左右でyは同じだと信じる
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(left_vec.z);  // 左右でzは同じだと信じる
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );

        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
        x = ( right_vec.x - left_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_vec.x;
        y = height * s + left_vec.y;  // 左右でyは同じだと信じる
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(left_vec.z);  // 左右でzは同じだと信じる
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      }
     
      // POKE_COMP_
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        f32 x = ( left_x - right_x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_x;
        f32 y = - height * s + base_y;
        VecFx32 p;
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(comp_z);
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
        POKE_VEC_F32 left_vec;
        POKE_VEC_F32 right_vec;
        f32 x;
        f32 y;
        VecFx32 p;

        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
        x = ( left_vec.x - right_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_vec.x;
        y = - height * s + right_vec.y;  // 左右でyは同じだと信じる
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(right_vec.z);  // 左右でzは同じだと信じる
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );

        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
        x = ( left_vec.x - right_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_vec.x;
        y = - height * s + right_vec.y;  // 左右でyは同じだと信じる
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(right_vec.z);  // 左右でzは同じだと信じる
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      }
#else
      // POKE_CURR_
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        f32 x = ( right_x - left_x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_x;
        f32 y = height * s + base_y;
        VecFx32 p;
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(curr_z);
        if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        {
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
        }
        if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        {
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
        }
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
        POKE_VEC_F32 left_vec;
        POKE_VEC_F32 right_vec;
        f32 x;
        f32 y;
        VecFx32 p;

        if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        {
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
          x = ( right_vec.x - left_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_vec.x;
          y = height * s + left_vec.y;  // 左右でyは同じだと信じる
          p.x = FX_F32_TO_FX32(x);
          p.y = FX_F32_TO_FX32(y);
          p.z = FX_F32_TO_FX32(left_vec.z);  // 左右でzは同じだと信じる
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
        }
        if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        {
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
          x = ( right_vec.x - left_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + left_vec.x;
          y = height * s + left_vec.y;  // 左右でyは同じだと信じる
          p.x = FX_F32_TO_FX32(x);
          p.y = FX_F32_TO_FX32(y);
          p.z = FX_F32_TO_FX32(left_vec.z);  // 左右でzは同じだと信じる
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
        }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      }
     
      // POKE_COMP_
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        f32 x = ( left_x - right_x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_x;
        f32 y = - height * s + base_y;
        VecFx32 p;
        p.x = FX_F32_TO_FX32(x);
        p.y = FX_F32_TO_FX32(y);
        p.z = FX_F32_TO_FX32(comp_z);
        if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        {
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
        }
        if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        {
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
        }
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
        POKE_VEC_F32 left_vec;
        POKE_VEC_F32 right_vec;
        f32 x;
        f32 y;
        VecFx32 p;

        if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        {
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
          x = ( left_vec.x - right_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_vec.x;
          y = - height * s + right_vec.y;  // 左右でyは同じだと信じる
          p.x = FX_F32_TO_FX32(x);
          p.y = FX_F32_TO_FX32(y);
          p.z = FX_F32_TO_FX32(right_vec.z);  // 左右でzは同じだと信じる
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &p );
        }
        if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        {
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_EXCHANGE_LEFT, &left_vec );
          PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_EXCHANGE_RIGHT, &right_vec );
          x = ( left_vec.x - right_vec.x ) * work->irekae_theta / (f32)IREKAE_THETA_MAX + right_vec.x;
          y = - height * s + right_vec.y;  // 左右でyは同じだと信じる
          p.x = FX_F32_TO_FX32(x);
          p.y = FX_F32_TO_FX32(y);
          p.z = FX_F32_TO_FX32(right_vec.z);  // 左右でzは同じだと信じる
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &p );
        }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      }
#endif
    }
  }
}

//-------------------------------------
/// 押し出し
//=====================================
static void Zukan_Detail_Form_OshidashiChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16 diff_comp_no_prev = work->diff_comp_no;

  if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
  {
    if( work->diff_num >= 3 )
    {
      work->diff_comp_no = work->diff_curr_no + work->diff_num -1;
      work->diff_comp_no %= work->diff_num;
    }
  }
  else  // 右から左へ
  {
    if( work->diff_num >= 3 )
    {
      work->diff_comp_no = work->diff_curr_no +1;
      work->diff_comp_no %= work->diff_num;
    }
  }

#ifndef DEF_MINIMUM_LOAD
  if( work->diff_comp_no != diff_comp_no_prev )
#else
  if(    ( ( (work->is_poke_front)) && (!(work->poke_mcss_wk[POKE_COMP_F].poke_wk)) )
      || ( (!(work->is_poke_front)) && (!(work->poke_mcss_wk[POKE_COMP_B].poke_wk)) )
      || ( work->diff_comp_no != diff_comp_no_prev ) )
#endif
  {
    // 前のを破棄
    //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    //BLOCK_POKE_EXIT( work->mcss_sys_wk, work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_F], work->mcss_sys_wk );
    PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_B], work->mcss_sys_wk );
  
    // 次のを生成
#ifndef DEF_MINIMUM_LOAD
    Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
        POKE_COMP_F, POKE_COMP_B, POKE_COMP_RPOS, work->diff_comp_no );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                              POKE_COMP_F, work );
    PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                              POKE_COMP_B, work );
#else
    {
      POKE_INDEX poke_f = (work->is_poke_front)?(POKE_COMP_F):(POKE_INDEX_NONE);
      POKE_INDEX poke_b = (work->is_poke_front)?(POKE_INDEX_NONE):(POKE_COMP_B);

      Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
          poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );
    
      if( poke_f != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                                  POKE_COMP_F, work );
      }
      if( poke_b != POKE_INDEX_NONE )
      {
        PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                                  POKE_COMP_B, work );
      }
    }
#endif
  }
  
  // 押し出し用関数を利用して位置設定
  Zukan_Detail_Form_OshidashiSetPosCompareForm( param, work, cmn );
}
static void Zukan_Detail_Form_OshidashiSetPosCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  const f32  left_x    = POKE_OSHIDASHI_POS_LEFT_X;
  const f32  center_x  = POKE_OSHIDASHI_POS_CENTER_X;
  const f32  right_x   = POKE_OSHIDASHI_POS_RIGHT_X;
  const f32  base_y    = POKE_OSHIDASHI_POS_BASE_Y;
  const f32  base_z    = POKE_OSHIDASHI_POS_BASE_Z;

  VecFx32 pos_fx32;

  if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
  {
    // 左に置く
    pos_fx32.x = FX_F32_TO_FX32(left_x);
    pos_fx32.y = FX_F32_TO_FX32(base_y);
    pos_fx32.z = FX_F32_TO_FX32(base_z);
#ifndef DEF_MINIMUM_LOAD
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_TOP_LEFT, &pos_fx32 );
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
    PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_TOP_LEFT, &pos_fx32 );
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
#else
    if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_TOP_LEFT, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_TOP_LEFT, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
#endif
  }
  else  // 右から左へ
  {
    // 右に置く
    pos_fx32.x = FX_F32_TO_FX32(right_x);
    pos_fx32.y = FX_F32_TO_FX32(base_y);
    pos_fx32.z = FX_F32_TO_FX32(base_z);
#ifndef DEF_MINIMUM_LOAD
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
    PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_TOP_RIGHT, &pos_fx32 );
    MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
#else
    if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_TOP_RIGHT, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_TOP_RIGHT, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
#endif
  }
}
static void Zukan_Detail_Form_OshidashiMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->oshidashi_state == OSHIDASHI_STATE_MOVE )
  {
    const f32  left_x    = POKE_OSHIDASHI_POS_LEFT_X;
    const f32  center_x  = POKE_OSHIDASHI_POS_CENTER_X;
    const f32  right_x   = POKE_OSHIDASHI_POS_RIGHT_X;
    const f32  base_y    = POKE_OSHIDASHI_POS_BASE_Y;
    const f32  base_z    = POKE_OSHIDASHI_POS_BASE_Z;

    VecFx32 pos_fx32;
    f32     pos_x;
    BOOL    move_end = FALSE;
   
#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
    f32         comp_center_x;
    POKE_INDEX  poke_comp_see;
    POKE_INDEX  poke_comp_not;
    POKE_INDEX  poke_curr_see;
    POKE_INDEX  poke_curr_not;
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
    MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#else
    if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
    if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif
    pos_x = FX_FX32_TO_F32( pos_fx32.x );

    if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
    {
      pos_x += OSHIDASHI_SPEED;
      if( pos_x >= center_x )
      {
        move_end = TRUE;
      }
    }
    else  // 右から左へ
    {
      pos_x -= OSHIDASHI_SPEED;
      if( pos_x <= center_x )
      {
        move_end = TRUE;
      }
    }

    if( move_end )
    {
      pos_x = center_x;
      work->oshidashi_state = OSHIDASHI_STATE_CHANGE_COMPARE_FORM;
    }

    {
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif

#ifndef DEF_MINIMUM_LOAD
      MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#endif
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
      {
        pos_x += OSHIDASHI_SPEED;
      }
      else  // 右から左へ
      {
        pos_x -= OSHIDASHI_SPEED;
      }
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#endif
    }
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
    // POKE_COMP_
    {
      POKE_VEC_F32 comp_center_pos;
      poke_comp_see = (work->is_poke_front)?(POKE_COMP_F):(POKE_COMP_B);
      poke_comp_not = (work->is_poke_front)?(POKE_COMP_B):(POKE_COMP_F);
      PokeMcssGetArrangePosF32FromPokeArrange( work->poke_mcss_wk[poke_comp_see], POKE_ARRANGE_TOP_CENTER, &comp_center_pos );
      comp_center_x = comp_center_pos.x;
    }
    MCSS_GetPosition( work->poke_mcss_wk[poke_comp_see].poke_wk, &pos_fx32 );
    pos_x = FX_FX32_TO_F32( pos_fx32.x );
    if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
    {
      pos_x += OSHIDASHI_SPEED;
      if( pos_x >= comp_center_x )
      {
        move_end = TRUE;
      }
    }
    else  // 右から左へ
    {
      pos_x -= OSHIDASHI_SPEED;
      if( pos_x <= comp_center_x )
      {
        move_end = TRUE;
      }
    }

    // 終了判定
    if( move_end )
    {
      pos_x = comp_center_x;
      work->oshidashi_state = OSHIDASHI_STATE_CHANGE_COMPARE_FORM;
    }

    {
      // POKE_COMP_
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
      MCSS_SetPosition( work->poke_mcss_wk[poke_comp_see].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[poke_comp_not].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく

      // POKE_CURR_
      {
        poke_curr_see = (work->is_poke_front)?(POKE_CURR_F):(POKE_CURR_B);
        poke_curr_not = (work->is_poke_front)?(POKE_CURR_B):(POKE_CURR_F);
      }
      MCSS_GetPosition( work->poke_mcss_wk[poke_curr_see].poke_wk, &pos_fx32 );
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
      {
        pos_x += OSHIDASHI_SPEED;
      }
      else  // 右から左へ
      {
        pos_x -= OSHIDASHI_SPEED;
      }
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
      MCSS_SetPosition( work->poke_mcss_wk[poke_curr_see].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[poke_curr_not].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく
    }
#else
    // POKE_COMP_
    {
      POKE_VEC_F32 comp_center_pos;
      poke_comp_see = (work->is_poke_front)?(POKE_COMP_F):(POKE_COMP_B);
      poke_comp_not = (work->is_poke_front)?(POKE_COMP_B):(POKE_COMP_F);
      PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[poke_comp_see], POKE_ARRANGE_TOP_CENTER, &comp_center_pos );
      comp_center_x = comp_center_pos.x;
    }
    //if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
    //  MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
    //if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
    //  MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
    MCSS_GetPosition( work->poke_mcss_wk[poke_comp_see].poke_wk, &pos_fx32 );
    pos_x = FX_FX32_TO_F32( pos_fx32.x );
    if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
    {
      pos_x += OSHIDASHI_SPEED;
      if( pos_x >= comp_center_x )
      {
        move_end = TRUE;
      }
    }
    else  // 右から左へ
    {
      pos_x -= OSHIDASHI_SPEED;
      if( pos_x <= comp_center_x )
      {
        move_end = TRUE;
      }
    }

    // 終了判定
    if( move_end )
    {
      pos_x = comp_center_x;
      work->oshidashi_state = OSHIDASHI_STATE_CHANGE_COMPARE_FORM;
    }

    {
      // POKE_COMP_
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる

      // POKE_CURR_
      {
        poke_curr_see = (work->is_poke_front)?(POKE_CURR_F):(POKE_CURR_B);
        poke_curr_not = (work->is_poke_front)?(POKE_CURR_B):(POKE_CURR_F);
      }
      //if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      //  MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      //if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      //  MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
      MCSS_GetPosition( work->poke_mcss_wk[poke_curr_see].poke_wk, &pos_fx32 );
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      if( work->oshidashi_direct == OSHIDASHI_DIRECT_L_TO_R )  // 左から右へ
      {
        pos_x += OSHIDASHI_SPEED;
      }
      else  // 右から左へ
      {
        pos_x -= OSHIDASHI_SPEED;
      }
      pos_fx32.x = FX_F32_TO_FX32(pos_x);
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
    }
#endif
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
  }
  else if( work->oshidashi_state == OSHIDASHI_STATE_CHANGE_COMPARE_FORM )  // 押し出しが完了したので、比較フォルムを変更する
  {
    Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );

        {
          // currとcompの番号、ポインタ入れ替え
          Zukan_Detail_Form_SwapCurrComp( param, work, cmn );

#ifdef DEF_MINIMUM_LOAD
          // 押し出されたフォルムのアニメは止めておく
          PokeMcssAnimeStop( &work->poke_mcss_wk[POKE_COMP_F] );  // この関数内でNULLチェックはしてくれている
          PokeMcssAnimeStop( &work->poke_mcss_wk[POKE_COMP_B] );  // この関数内でNULLチェックはしてくれている
#endif

          // ポケアイコン変更
          Zukan_Detail_Form_ChangePokeicon( param, work, cmn );

          // テキスト
          // テキストはOBJではなくBGなので、入れ替えできないので丸々書き直し
          Zukan_Detail_Form_WritePokeCurrText( param, work, cmn );
          Zukan_Detail_Form_WritePokeCompText( param, work, cmn );

          // テキストに合わせて、フォルム名の背面フィールドを用意する
          Zukan_Detail_Form_ObjFieldSetup( param, work, cmn );

          // 図鑑セーブデータに閲覧中データ設定
          {
            u16 monsno = ZKNDTL_COMMON_GetCurrPoke(cmn);

            GAMEDATA* gamedata = ZKNDTL_COMMON_GetGamedata(cmn);
            ZUKAN_SAVEDATA* zkn_sv = GAMEDATA_GetZukanSave( gamedata );

            u32  sex  = work->diff_info_list[work->diff_curr_no].sex;
            BOOL rare = (work->diff_info_list[work->diff_curr_no].rare==0)?FALSE:TRUE;
            u32  form = work->diff_info_list[work->diff_curr_no].form;

            ZUKANSAVE_SetDrawData(  zkn_sv, monsno, sex, rare, form );

#ifdef DEBUG_KAWADA
            {
              OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, diff_no=%d, f=%d, s=%d, r=%d\n", monsno, work->diff_curr_no, form, sex, rare );
            }
#endif
#ifdef DEBUG_KAWADA
            {
              ZUKANSAVE_GetDrawData(  zkn_sv, monsno, &sex, &rare, &form, param->heap_id );
              OS_Printf( "ZUKAN_DETAIL_FORM : monsno=%d, ZUKANSAVE_GetDrawData, f=%d, s=%d, r=%d\n", monsno, form, sex, rare );
            }
#endif
          }
        }

      // 押し出し用関数を利用してcompをcurrの次のフォルムにしておく
      // 押し出し用関数を利用して位置設定
#ifndef DEF_MINIMUM_LOAD
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL 
      {
        VecFx32 p;
        PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_CENTER, &p );
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
        PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_CENTER, &p );
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
      }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL 
        {
          VecFx32 p;
          PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_F], POKE_CURR_POS_CENTER, &p );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &p );
        }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      } 
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &poke_pos[POKE_CURR_POS_CENTER] );
#else  // DEF_POKE_ARRANGE_INDIVIDUAL 
        {
          VecFx32 p;
          PokeMcssGetArrangePosFx32FromPokePosIndex( &work->poke_mcss_wk[POKE_CURR_B], POKE_CURR_POS_CENTER, &p );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &p );
        }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
      }
#endif
      work->oshidashi_direct = OSHIDASHI_DIRECT_R_TO_L;
      Zukan_Detail_Form_OshidashiChangeCompareForm( param, work, cmn );
      // STATE_TOPの状態のときに1度でもフォルム変更を行うと、compにはcurrの次のフォルムを設定して終わるようになる。
      // STATE_TOPの状態のときにフォルム変更を行わなかったら、compはSTATE_EXCHANGEの状態のときに設定したもののままになる。
      
      work->oshidashi_state = OSHIDASHI_STATE_NONE;
  }
}

//-------------------------------------
/// 階層変更  // TOP_TO_EXCHANGE or EXCHANGE_TO_TOP
//=====================================
static void Zukan_Detail_Form_KaisouMain( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  const f32  exchange_left_x  = POKE_KAISOU_EXCHANGE_POS_LEFT_X;
  const f32  exchange_right_x = POKE_KAISOU_EXCHANGE_POS_RIGHT_X;
  const f32  top_center_x     = POKE_KAISOU_TOP_POS_CENTER_X;
  const f32  top_right_x      = POKE_KAISOU_TOP_POS_RIGHT_X;
  const f32  base_y           = POKE_KAISOU_POS_BASE_Y;
  const f32  base_z           = POKE_KAISOU_POS_BASE_Z;

#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
  f32         curr_exchange_left_x;
  f32         comp_exchange_right_x;
  f32         curr_top_center_x;
  f32         comp_top_right_x;
  POKE_INDEX  poke_comp_see;
  POKE_INDEX  poke_comp_not;
  POKE_INDEX  poke_curr_see;
  POKE_INDEX  poke_curr_not;
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL

  if( work->diff_num < 2 )  // このif文は真になることはない気がする
  {
    if( work->state == STATE_TOP_TO_EXCHANGE )
    {
      //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );
      Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE );
    }
    else if( work->state == STATE_EXCHANGE_TO_TOP )
    {
      //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );
      Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE );
    }
    return;
  }

  if( work->state == STATE_TOP_TO_EXCHANGE )
  {
    VecFx32 pos_fx32;
    f32     pos_x;
    if( !work->kaisou_curr_end )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
      MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#endif
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x -= KAISOU_CURR_SPEED;
      if( pos_x <= exchange_left_x )
      {
        pos_x = exchange_left_x;
        work->kaisou_curr_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      }
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
      }
#endif
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      // DEF_MINIMUM_LOADが定義されていても、定義されていなくても、共通処理
      {
        POKE_VEC_F32 curr_exchange_left_pos;
        poke_curr_see = (work->is_poke_front)?(POKE_CURR_F):(POKE_CURR_B);
        poke_curr_not = (work->is_poke_front)?(POKE_CURR_B):(POKE_CURR_F);
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[poke_curr_see], POKE_ARRANGE_EXCHANGE_LEFT, &curr_exchange_left_pos );
        curr_exchange_left_x = curr_exchange_left_pos.x;
      }
      MCSS_GetPosition( work->poke_mcss_wk[poke_curr_see].poke_wk, &pos_fx32 );

      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x -= KAISOU_CURR_SPEED;
      if( pos_x <= curr_exchange_left_x )
      {
        pos_x = curr_exchange_left_x;
        work->kaisou_curr_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );

      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }

      // 目的地に着いたので、正しい位置に置く
      if( work->kaisou_curr_end )
      {
        VecFx32 curr_exchange_left_pos;
        if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_EXCHANGE_LEFT, &curr_exchange_left_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &curr_exchange_left_pos );
        }
        if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_EXCHANGE_LEFT, &curr_exchange_left_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &curr_exchange_left_pos );
        }
      }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( !work->kaisou_comp_end )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
      MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x -= KAISOU_COMP_SPEED;
      if( pos_x <= exchange_right_x )
      {
        pos_x = exchange_right_x;
        work->kaisou_comp_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      }
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
      }
#endif
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      // DEF_MINIMUM_LOADが定義されていても、定義されていなくても、共通処理
      {
        POKE_VEC_F32 comp_exchange_right_pos;
        poke_comp_see = (work->is_poke_front)?(POKE_COMP_F):(POKE_COMP_B);
        poke_comp_not = (work->is_poke_front)?(POKE_COMP_B):(POKE_COMP_F);
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[poke_comp_see], POKE_ARRANGE_EXCHANGE_RIGHT, &comp_exchange_right_pos );
        comp_exchange_right_x = comp_exchange_right_pos.x;
      }
      MCSS_GetPosition( work->poke_mcss_wk[poke_comp_see].poke_wk, &pos_fx32 );

      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x -= KAISOU_COMP_SPEED;
      if( pos_x <= comp_exchange_right_x )
      {
        pos_x = comp_exchange_right_x;
        work->kaisou_comp_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );

      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }

      // 目的地に着いたので、正しい位置に置く
      if( work->kaisou_comp_end )
      {
        VecFx32 comp_exchange_right_pos;
        if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_EXCHANGE_RIGHT, &comp_exchange_right_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &comp_exchange_right_pos );
        }
        if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_EXCHANGE_RIGHT, &comp_exchange_right_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &comp_exchange_right_pos );
        }
      }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( work->kaisou_curr_end && work->kaisou_comp_end )
    {
      //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );
      Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE );
    }
  }
  else if( work->state == STATE_TOP_TO_EXCHANGE_TOUCHBAR_CHANGE )
  {
    Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE );
  }
  else if( work->state == STATE_EXCHANGE_TO_TOP )
  {
    VecFx32 pos_fx32;
    f32     pos_x;
    if( !work->kaisou_curr_end )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
      MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#endif
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x += KAISOU_CURR_SPEED;
      if( pos_x >= top_center_x )
      {
        pos_x = top_center_x;
        work->kaisou_curr_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );
      }
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );
      }
#endif
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      // DEF_MINIMUM_LOADが定義されていても、定義されていなくても、共通処理
      {
        POKE_VEC_F32 curr_top_center_pos;
        poke_curr_see = (work->is_poke_front)?(POKE_CURR_F):(POKE_CURR_B);
        poke_curr_not = (work->is_poke_front)?(POKE_CURR_B):(POKE_CURR_F);
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[poke_curr_see], POKE_ARRANGE_TOP_CENTER, &curr_top_center_pos );
        curr_top_center_x = curr_top_center_pos.x;
      }
      MCSS_GetPosition( work->poke_mcss_wk[poke_curr_see].poke_wk, &pos_fx32 );

      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x += KAISOU_CURR_SPEED;
      if( pos_x >= curr_top_center_x )
      {
        pos_x = curr_top_center_x;
        work->kaisou_curr_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );

      if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }
      if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }

      // 目的地に着いたので、正しい位置に置く
      if( work->kaisou_curr_end )
      {
        VecFx32 curr_top_center_pos;
        if( work->poke_mcss_wk[POKE_CURR_F].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_F], POKE_ARRANGE_TOP_CENTER, &curr_top_center_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_F].poke_wk, &curr_top_center_pos );
        }
        if( work->poke_mcss_wk[POKE_CURR_B].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_CURR_B], POKE_ARRANGE_TOP_CENTER, &curr_top_center_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_CURR_B].poke_wk, &curr_top_center_pos );
        }
      }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( !work->kaisou_comp_end )
    {
#ifndef DEF_POKE_ARRANGE_INDIVIDUAL
#ifndef DEF_MINIMUM_LOAD
      MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        MCSS_GetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#endif
      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x += KAISOU_COMP_SPEED;
      if( pos_x >= top_right_x )
      {
        pos_x = top_right_x;
        work->kaisou_comp_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );
#ifndef DEF_MINIMUM_LOAD
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
#else
      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );
      }
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );
      }
#endif
#else  // DEF_POKE_ARRANGE_INDIVIDUAL
      // DEF_MINIMUM_LOADが定義されていても、定義されていなくても、共通処理
      {
        POKE_VEC_F32 comp_top_right_pos;
        poke_comp_see = (work->is_poke_front)?(POKE_COMP_F):(POKE_COMP_B);
        poke_comp_not = (work->is_poke_front)?(POKE_COMP_B):(POKE_COMP_F);
        PokeMcssGetArrangePosF32FromPokeArrange( &work->poke_mcss_wk[poke_comp_see], POKE_ARRANGE_TOP_RIGHT, &comp_top_right_pos );
        comp_top_right_x = comp_top_right_pos.x;
      }
      MCSS_GetPosition( work->poke_mcss_wk[poke_comp_see].poke_wk, &pos_fx32 );

      pos_x = FX_FX32_TO_F32( pos_fx32.x );
      pos_x += KAISOU_COMP_SPEED;
      if( pos_x >= comp_top_right_x )
      {
        pos_x = comp_top_right_x;
        work->kaisou_comp_end = TRUE;
      }
      pos_fx32.x = FX_F32_TO_FX32( pos_x );

      if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }
      if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
      {
        MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &pos_fx32 );  // 後で位置を直すので、見えてないものはてきとーな位置に置いておく、見えているときは正しい位置に置かれる
      }

      // 目的地に着いたので、見えないものを正しい位置に置く
      if( work->kaisou_comp_end )
      {
        VecFx32 comp_top_right_pos;
        if( work->poke_mcss_wk[POKE_COMP_F].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_F], POKE_ARRANGE_TOP_RIGHT, &comp_top_right_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_F].poke_wk, &comp_top_right_pos );
        }
        if( work->poke_mcss_wk[POKE_COMP_B].poke_wk )
        {
          PokeMcssGetArrangePosFx32FromPokeArrange( &work->poke_mcss_wk[POKE_COMP_B], POKE_ARRANGE_TOP_RIGHT, &comp_top_right_pos );
          MCSS_SetPosition( work->poke_mcss_wk[POKE_COMP_B].poke_wk, &comp_top_right_pos );
        }
      }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
    }
    if( work->kaisou_curr_end && work->kaisou_comp_end )
    {
      //Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );
      Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE );
    }
  }
  else if( work->state == STATE_EXCHANGE_TO_TOP_TOUCHBAR_CHANGE )
  {
    Zukan_Detail_Form_ChangeState( param, work, cmn, STATE_TOP );
  }
}

#ifdef DEF_MINIMUM_LOAD
static void Zukan_Detail_Form_KaisouChangeCompareForm( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  if( work->state == STATE_TOP )
  {
    // 前のを破棄
    PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_F], work->mcss_sys_wk );
    PokeMcssWorkExit( &work->poke_mcss_wk[POKE_COMP_B], work->mcss_sys_wk );
  
    // 次のを生成
    {
      // 読み込み時に既に表示されているポケモンの絵が乱れるので、tcbを一時的にはずしておく
      MCSS_SetTCBSys( work->mcss_sys_wk, NULL );

      {
        POKE_INDEX poke_f = (work->is_poke_front)?(POKE_COMP_F):(POKE_INDEX_NONE);
        POKE_INDEX poke_b = (work->is_poke_front)?(POKE_INDEX_NONE):(POKE_COMP_B);

        Zukan_Detail_Form_PokeInitFromDiffInfo( param, work, cmn,
            poke_f, poke_b, POKE_COMP_RPOS, work->diff_comp_no );
    
        if( poke_f != POKE_INDEX_NONE )
        {
          PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data,
                                    POKE_COMP_F, work );
        }
        if( poke_b != POKE_INDEX_NONE )
        {
          PokeMcssCallBackDataInit( work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data,
                                    POKE_COMP_B, work );
        }
      }

      // 一時的にはずしていたtcbを、再び使用するように戻しておく
      MCSS_SetTCBSys( work->mcss_sys_wk, work->mcss_tcbsys );
    }
  }
}
#endif


//-------------------------------------
/// currとcompの番号、ポインタ入れ替え
//=====================================
static void Zukan_Detail_Form_SwapCurrComp( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  u16 no;
  MCSS_WORK* mw;
  POKE_MCSS_CALL_BACK_DATA* pmcbd;

  // 番号、ポインタ入れ替え
  no = work->diff_curr_no;
  work->diff_curr_no = work->diff_comp_no;
  work->diff_comp_no = no;

  // NULLならNULLが入れ替わるだけなので、work->is_poke_frontを気にせず入れ替えていい
  mw = work->poke_mcss_wk[POKE_CURR_F].poke_wk;
  work->poke_mcss_wk[POKE_CURR_F].poke_wk = work->poke_mcss_wk[POKE_COMP_F].poke_wk;
  work->poke_mcss_wk[POKE_COMP_F].poke_wk = mw;

  pmcbd = work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data;
  work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data = work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data;
  work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data = pmcbd;
  if(work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data) work->poke_mcss_wk[POKE_CURR_F].poke_call_back_data->poke_idx = POKE_CURR_F;
  if(work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data) work->poke_mcss_wk[POKE_COMP_F].poke_call_back_data->poke_idx = POKE_COMP_F;

  mw = work->poke_mcss_wk[POKE_CURR_B].poke_wk;
  work->poke_mcss_wk[POKE_CURR_B].poke_wk = work->poke_mcss_wk[POKE_COMP_B].poke_wk;
  work->poke_mcss_wk[POKE_COMP_B].poke_wk = mw;

  pmcbd = work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data;
  work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data = work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data;
  work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data = pmcbd;
  if(work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data) work->poke_mcss_wk[POKE_CURR_B].poke_call_back_data->poke_idx = POKE_CURR_B;
  if(work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data) work->poke_mcss_wk[POKE_COMP_B].poke_call_back_data->poke_idx = POKE_COMP_B;

#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
  {
    u8            j;
    POKE_VEC_F32  p;
    for( j=0; j<POKE_ARRANGE_MAX; j++ )
    {
      p                                      = work->poke_mcss_wk[POKE_CURR_F].pos[j];
      work->poke_mcss_wk[POKE_CURR_F].pos[j] = work->poke_mcss_wk[POKE_COMP_F].pos[j];
      work->poke_mcss_wk[POKE_COMP_F].pos[j] = p;

      p                                      = work->poke_mcss_wk[POKE_CURR_B].pos[j];
      work->poke_mcss_wk[POKE_CURR_B].pos[j] = work->poke_mcss_wk[POKE_COMP_B].pos[j];
      work->poke_mcss_wk[POKE_COMP_B].pos[j] = p;
    }
  }
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL
}


//-------------------------------------
/// アルファ設定
//=====================================
static void Zukan_Detail_Form_AlphaInit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  int ev1 = 12;

  // 半透明にしないOBJの設定を変更する
  {
    u8 i;

    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      if( work->pokeicon_clwk[i] )
      {
        GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[i], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
      }
    }

    GFL_CLACT_WK_SetObjMode( work->obj_clwk[OBJ_BAR_RANGE], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
    GFL_CLACT_WK_SetObjMode( work->obj_clwk[OBJ_BAR_CURSOR], GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする

    for( i=0; i<BUTTON_OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_SetObjMode( work->button[i].clwk, GX_OAM_MODE_NORMAL );  // アルファアニメーションの影響を受けないようにする
    }
  }

  G2_SetBlendAlpha(
        GX_BLEND_PLANEMASK_NONE,
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        ev1,
        16 - ev1 );

  G2S_SetBlendAlpha(
        GX_BLEND_PLANEMASK_BG3,
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        ev1,
        16 - ev1 );
}
static void Zukan_Detail_Form_AlphaExit( ZUKAN_DETAIL_FORM_PARAM* param, ZUKAN_DETAIL_FORM_WORK* work, ZKNDTL_COMMON_WORK* cmn )
{
  // 半透明にしないOBJの設定を元に戻す
  {
    u8 i;

    for( i=0; i<OBJ_SWAP_MAX; i++ )
    {
      if( work->pokeicon_clwk[i] )
      {
        GFL_CLACT_WK_SetObjMode( work->pokeicon_clwk[i], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
      }
    }

    GFL_CLACT_WK_SetObjMode( work->obj_clwk[OBJ_BAR_RANGE], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    GFL_CLACT_WK_SetObjMode( work->obj_clwk[OBJ_BAR_CURSOR], GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので

    for( i=0; i<BUTTON_OBJ_MAX; i++ )
    {
      GFL_CLACT_WK_SetObjMode( work->button[i].clwk, GX_OAM_MODE_XLU );  // BGとともにこのOBJも暗くしたいので
    }
  }

  // 一部分フェードの設定を元に戻す
  ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_m );
  // 一部分フェードの設定を元に戻す
  ZKNDTL_COMMON_FadeSetPlaneDefault( work->fade_wk_s );
}


#ifdef DEF_POKE_ARRANGE_INDIVIDUAL
//-------------------------------------
/// ポケモンの配置をセットアップする
//=====================================
typedef struct
{
  int  mons_no;         // 1スタート  // MONSNO_ANNOON
  int  form_no;         // 0スタート  // FORMNO_ANNOON_UNR
  int  sex;             // PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN  // prog/include/poke_tool/poke_tool.h
  int  rare;            // TRUE, FALSE
  BOOL egg;             // TRUE, FALSE
  int  dir;             // MCSS_DIR_FRONT, MCSS_DIR_BACK
  u32  personal_rnd;    // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
  
  POKE_VEC_F32  pos[POKE_ARRANGE_MAX];  // ポケモンの配置
}
POKE_ARRANGE_INFO;
#define MALE_FEMALE_UNKNOWN (3)  // オスメス性別なしどれでも構わない  // PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWNと被らない値
#define POKE_ARRANGE_INFO_TBL_NUM (11)

static const POKE_VEC_F32 poke_arrange_default_pos[POKE_ARRANGE_MAX] =
{
  { POKE_KAISOU_TOP_POS_CENTER_X,     POKE_KAISOU_POS_BASE_Y, POKE_KAISOU_POS_BASE_Z },
  { POKE_OSHIDASHI_POS_LEFT_X,        POKE_KAISOU_POS_BASE_Y, POKE_KAISOU_POS_BASE_Z },
  { POKE_OSHIDASHI_POS_RIGHT_X,       POKE_KAISOU_POS_BASE_Y, POKE_KAISOU_POS_BASE_Z },
  { POKE_KAISOU_EXCHANGE_POS_LEFT_X,  POKE_KAISOU_POS_BASE_Y, POKE_KAISOU_POS_BASE_Z },
  { POKE_KAISOU_EXCHANGE_POS_RIGHT_X, POKE_KAISOU_POS_BASE_Y, POKE_KAISOU_POS_BASE_Z },
};

static const POKE_ARRANGE_INFO poke_arrange_info_tbl[POKE_ARRANGE_INFO_TBL_NUM] =
{
  // (rare, egg, personal_rndは現在未使用)  // 1列のデータのyは全て等しくしておくこと! IrekaeMain,OshidashiMain,KaisouMainでの移動はそれを前提としてつくられており、どれかのyしか使用していなかったり、yを変更していなかったりしている。
  { MONSNO_ANNOON,     FORMNO_ANNOON_UNR, MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // アンノーン R
    { {-0.2f,-13.9f,0.0f},{-64.2f,-13.9f,0.0f},{63.8f,-13.9f,0.0f},{-16.2f,-13.9f,0.0f},{15.8f,-13.9f,0.0f} } },

  { MONSNO_MIROKAROSU, 0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // ミロカロス 0
    { {-0.02f,-13.9f,0.0f},{-64.02f,-13.9f,0.0f},{63.98f,-13.9f,0.0f},{-16.11f,-13.9f,0.0f},{15.89f,-13.9f,0.0f} } },

  { MONSNO_571,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // カブルモ 0
    { {0.0f,-13.7f,0.0f},{-64.0f,-13.7f,0.0f},{64.0f,-13.7f,0.0f},{-16.4f,-13.7f,0.0f},{15.9f,-13.7f,0.0f} } },

  { MONSNO_576,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_FRONT, 0,  // ナゲキ 0
    { {0.1f,-13.9f,0.0f},{-63.9f,-13.9f,0.0f},{64.1f,-13.9f,0.0f},{-16.0f,-13.9f,0.0f},{16.0f,-13.9f,0.0f} } },

  { MONSNO_557,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // ミジュマル 0
    { {-0.3f,-13.9f,0.0f},{-64.3f,-13.9f,0.0f},{63.7f,-13.9f,0.0f},{-16.3f,-13.9f,0.0f},{15.7f,-13.9f,0.0f} } },

  { MONSNO_541,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_FRONT, 0,  // キバゴ 0
    { {-0.3f,-13.9f,0.0f},{-64.3f,-13.9f,0.0f},{63.7f,-13.9f,0.0f},{-15.7f,-13.9f,0.0f},{16.0f,-13.9f,0.0f} } },

  { MONSNO_541,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // キバゴ 0
    { {0.0f,-13.8f,0.0f},{-64.0f,-13.8f,0.0f},{64.0f,-13.8f,0.0f},{-16.0f,-13.8f,0.0f},{16.0f,-13.8f,0.0f} } },

  { MONSNO_636,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // クイタラン 0
    { {0.0f,-13.8f,0.0f},{-64.0f,-13.8f,0.0f},{64.0f,-13.8f,0.0f},{-16.0f,-13.8f,0.0f},{16.0f,-13.8f,0.0f} } },

  { MONSNO_PAUWAU,     0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // パウワウ 0
    { {-0.2f,-13.9f,0.0f},{-64.2f,-13.9f,0.0f},{63.8f,-13.9f,0.0f},{-16.4f,-13.9f,0.0f},{15.9f,-13.9f,0.0f} } },

  { MONSNO_YAZIRON,    0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_FRONT, 0,  // ヤジロン 0
    { {0.0f,-13.8f,0.0f},{-64.0f,-13.8f,0.0f},{64.0f,-13.8f,0.0f},{-16.0f,-13.8f,0.0f},{16.0f,-13.8f,0.0f} } },

  { MONSNO_KUTIITO,    0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_BACK, 0,  // クチート 0
    { {0.0f,-13.8f,0.0f},{-64.0f,-13.8f,0.0f},{64.0f,-13.8f,0.0f},{-16.0f,-13.8f,0.0f},{16.0f,-13.8f,0.0f} } },


//  { MONSNO_001,        0,                 MALE_FEMALE_UNKNOWN, 0, 0, MCSS_DIR_FRONT, 0,  // デフォルト
//    { {0.0f,-13.9f,0.0f},{-64.0f,-13.9f,0.0f},{64.0f,-13.9f,0.0f},{-16.0f,-13.9f,0.0f},{16.0f,-13.9f,0.0f} } },
};

static void PokeSetupArrange( POKE_VEC_F32* pos,  // pos[POKE_ARRANGE_MAX]となっていること
                      int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
                      u32 personal_rnd )  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
{
  // (rare, egg, personal_rndは現在未使用)
  u16 i;
  u8  j;

  // デフォルト値
  for( j=0; j<POKE_ARRANGE_MAX; j++ )
  {
    pos[j] = poke_arrange_default_pos[j];
  }

  // poke_arrange_info_tblにあれば、その値にする
  for( i=0; i<POKE_ARRANGE_INFO_TBL_NUM; i++ )
  {
    if(    poke_arrange_info_tbl[i].mons_no == mons_no
        && poke_arrange_info_tbl[i].form_no == form_no
        && poke_arrange_info_tbl[i].dir     == dir )
    {
      if(    poke_arrange_info_tbl[i].sex == MALE_FEMALE_UNKNOWN
          || poke_arrange_info_tbl[i].sex == sex )
      {
        for( j=0; j<POKE_ARRANGE_MAX; j++ )
        {
          pos[j] = poke_arrange_info_tbl[i].pos[j];
        }
        break;
      }
    }
  }
}

// ポケモンの配置をVecFx32で得る
static void PokeMcssGetArrangePosFx32FromPokeArrange( POKE_MCSS_WORK* poke_mcss_wk, POKE_ARRANGE a, VecFx32* pos )
{
  pos->x = FX_F32_TO_FX32(poke_mcss_wk->pos[a].x);
  pos->y = FX_F32_TO_FX32(poke_mcss_wk->pos[a].y);
  pos->z = FX_F32_TO_FX32(poke_mcss_wk->pos[a].z);
}
static void PokeMcssGetArrangePosFx32FromPokePosIndex( POKE_MCSS_WORK* poke_mcss_wk, POKE_POS_INDEX i, VecFx32* pos )
{
  POKE_ARRANGE a = POKE_ARRANGE_TOP_CENTER;
  switch(i)
  {
  case POKE_CURR_POS_CENTER: a = POKE_ARRANGE_TOP_CENTER;     break;
  case POKE_CURR_POS_LEFT:   a = POKE_ARRANGE_EXCHANGE_LEFT;  break;
  case POKE_COMP_RPOS:       a = POKE_ARRANGE_EXCHANGE_RIGHT; break;
  }
  PokeMcssGetArrangePosFx32FromPokeArrange( poke_mcss_wk, a, pos );
}

// ポケモンの配置をPOKE_VEC_F32で得る
static void PokeMcssGetArrangePosF32FromPokeArrange( POKE_MCSS_WORK* poke_mcss_wk, POKE_ARRANGE a, POKE_VEC_F32* pos )
{
  *pos = poke_mcss_wk->pos[a];
}
#endif  // DEF_POKE_ARRANGE_INDIVIDUAL


//=============================================================================
/**
 * 
 */
//=============================================================================
// マクロ
#undef BLOCK_POKE_EXIT
#undef BLOCK_POKEICON_EXIT

