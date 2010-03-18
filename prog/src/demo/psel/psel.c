//============================================================================
/**
 *  @file   psel.c
 *  @brief  三匹選択
 *  @author Koji Kawada
 *  @data   2010.03.12
 *  @note   
 *  モジュール名：PSEL
 */
//============================================================================
//#define DEBUG_POS_SET_MODE  // これが定義されているとき、位置調整モードになる


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "system/ica_anime.h"
#include "system/ica_camera.h"
#include "system/pokegra.h"
#include "system/poke2dgra.h"
#include "gamesystem/msgspeed.h"
#include "app/app_keycursor.h"
#include "app/app_taskmenu.h"

#include "psel_graphic.h"
#include "demo/psel.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "script_message.naix"
#include "msg/script/msg_t01r0102.h"
#include "psel.naix"

// サウンド
#include "sound/pm_sndsys.h"

// オーバーレイ


//=============================================================================
/**
*  定数定義
*/
//=============================================================================
#define HEAP_SIZE              (0x100000)               ///< ヒープサイズ

// メインBGフレーム
#define BG_FRAME_M_THREE       (GFL_BG_FRAME0_M)     // 3D
#define BG_FRAME_M_REAR        (GFL_BG_FRAME1_M)
#define BG_FRAME_M_WIN         (GFL_BG_FRAME2_M)
#define BG_FRAME_M_TEXT        (GFL_BG_FRAME3_M)     // APP_TASKMENUもここに出す

// メインBGフレームのプライオリティ
#define BG_FRAME_PRI_M_THREE   (2)
#define BG_FRAME_PRI_M_REAR    (3)
#define BG_FRAME_PRI_M_WIN     (1)
#define BG_FRAME_PRI_M_TEXT    (0)

// メインBGパレット
// 本数
enum
{
  BG_PAL_NUM_M_REAR                   = 11,
  BG_PAL_NUM_M_APP_TASKMENU           =  2,
  BG_PAL_NUM_M_WIN                    =  1,
  BG_PAL_NUM_M_TEXT                   =  1,
};
// 位置
enum
{
  BG_PAL_POS_M_REAR            =   0,
  BG_PAL_POS_M_APP_TASKMENU    =  11,  // 2本
  BG_PAL_POS_M_WIN             =  13,
  BG_PAL_POS_M_TEXT            =  14,
};

// メインOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_M_FINGER          = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_M_FINGER          = 0,
};

// サブBGフレーム
#define BG_FRAME_S_INSIDE_BALL (GFL_BG_FRAME0_S)
#define BG_FRAME_S_WIN         (GFL_BG_FRAME1_S)  // BG_FRAME_S_WINとBG_FRAME_S_TEXTは共通のキャラ領域を使用
#define BG_FRAME_S_TEXT        (GFL_BG_FRAME2_S)  // BG_FRAME_S_WINとBG_FRAME_S_TEXTは共通のキャラ領域を使用

// サブBGフレームのプライオリティ
#define BG_FRAME_PRI_S_INSIDE_BALL    (2)
#define BG_FRAME_PRI_S_WIN            (1)
#define BG_FRAME_PRI_S_TEXT           (0)

// サブBGパレット
// 本数
enum
{
  BG_PAL_NUM_S_WIN                    =  1,
  BG_PAL_NUM_S_TEXT                   =  1,
  BG_PAL_NUM_S_INSIDE_BALL            =  8,
};
// 位置
enum
{
  BG_PAL_POS_S_WIN             =  0,
  BG_PAL_POS_S_TEXT            =  1,
  BG_PAL_POS_S_INSIDE_BALL     =  8,
};

// サブOBJパレット
// 本数
enum
{
  OBJ_PAL_NUM_S_MIZU      = 1,  // smallもbigもこのパレット列を使う
  OBJ_PAL_NUM_S_HONOO     = 1,
  OBJ_PAL_NUM_S_KUSA      = 1,
};
// 位置
enum
{
  OBJ_PAL_POS_S_MIZU      = 0,
  OBJ_PAL_POS_S_HONOO     = 1,
  OBJ_PAL_POS_S_KUSA      = 2,
};


// サウンド
#define PSELSND_DECIDE  ( SEQ_SE_DECIDE1 )
#define PSELSND_CANCEL  ( SEQ_SE_CANCEL1 )
#define PSELSND_MOVE    ( SEQ_SE_SELECT1 )
//#define PSELSND_OPEN    ( SEQ_SE_OPEN1 )


// ProcMainのシーケンス
enum
{
  SEQ_START            = 0,
  SEQ_LOAD,
  SEQ_S01_INIT,
  SEQ_S01_FADE_IN,
  SEQ_S01_MAIN,
  SEQ_S01_FADE_OUT,
  SEQ_S01_EXIT,
  SEQ_S02_INIT,
  SEQ_S02_FADE_IN,
  SEQ_S02_MAIN,
  SEQ_S02_FADE_OUT,
  SEQ_S02_EXIT,
  SEQ_UNLOAD,
  SEQ_END,
  SEQ_MAX,
};


// テキスト  // 後ろから確保すること
enum
{
  TEXT_LETTER,     // MAIN
  TEXT_EXPLAIN,    // MAIN
  TEXT_POKE_INFO,  // SUB
  TEXT_MAX,
};

#define TEXT_COLOR_L (15)  // MAINもSUBも同じ構成にしておく
#define TEXT_COLOR_S ( 2)
#define TEXT_COLOR_B ( 0)

static const u8 bmpwin_setup[TEXT_MAX][9] =
{
  // frmnum           posx  posy  sizx  sizy  palnum             dir                    x  y (x,yは無視してセンタリングすることもある)
  //{  BG_FRAME_M_TEXT,    4,    4,   24,   16, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B, 0, 0 },  // 上下1キャラずつ余裕がある
  {  BG_FRAME_M_TEXT,    4,    5,   24,   14, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B, 0, 0 },  // 上下に余裕がない
  {  BG_FRAME_M_TEXT,    1,   19,   30,    2, BG_PAL_POS_M_TEXT, GFL_BMP_CHRAREA_GET_B, 0, 0 },
  {  BG_FRAME_S_TEXT,    1,   18,   30,    4, BG_PAL_POS_S_TEXT, GFL_BMP_CHRAREA_GET_B, 0, 0 },
};

// 文字数
#define TEXT_POKE_INFO_LEN_MAX    (64)  // ポケモンのタイプと種族名(ピカチュウとか)の情報  // EOMを含めた文字数

// 送りカーソルデータ
static const u8 ALIGN4 sc_skip_cursor_character[128] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,
  0xF2,0xFF,0xFF,0xFF,0x20,0xFF,0xFF,0x2F, 0x00,0xF2,0xFF,0x22,0x00,0x20,0x2F,0x02,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x2F,0x00,0x00,0x00,
  0x22,0x00,0x00,0x00,0x02,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

  0x00,0x00,0x22,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};


// フェード
#define S01_FADE_IN_WAIT  (0)
#define S01_FADE_OUT_WAIT (-1)
#define S02_FADE_IN_WAIT  (2)
#define S02_FADE_OUT_WAIT (0)


// 2D OBJ
enum
{
  TWO_OBJ_RES_NCG,
  TWO_OBJ_RES_NCL,
  TWO_OBJ_RES_NCE,
  TWO_OBJ_RES_MAX,
  TWO_OBJ_RES_NAN        = TWO_OBJ_RES_MAX,
  TWO_OBJ_RES_DARK_NCL,
  TWO_OBJ_RES_FILE_MAX,
};

#define OBJ_BG_PRI_S_POKE     (BG_FRAME_PRI_S_INSIDE_BALL)
#define OBJ_BG_PRI_M_FINGER   (BG_FRAME_PRI_M_THREE)


// 選択対象
typedef enum
{
  TARGET_MIZU       = PSEL_RESULT_MIZU,   // 左     // 左から順番に並べておく  // PSEL_RESULTと同じ並び順にしておくこと  // psel.c内でPSEL_RESULTの並びを利用するのはここだけで、他はこのTARGETの並びを利用するようにしておく(これで、PSEL_RESULTを変更せざるを得ないときに最小限の変更で済むかな)
  TARGET_HONOO      = PSEL_RESULT_HONOO,  // 中央
  TARGET_KUSA       = PSEL_RESULT_KUSA,   // 右
  TARGET_POKE_MAX,                        // ポケモン数
  TARGET_BUTTON     = TARGET_POKE_MAX,    // 確認ボタン
  TARGET_MAX,
  TARGET_NONE       = TARGET_MAX,         // どれも選択していないとき
  TARGET_POKE_START = TARGET_MIZU,        // TARGET_POKE_START<= <=TARGET_POKE_END
  TARGET_POKE_END   = TARGET_KUSA,
}
TARGET;

// タッチの当たり判定
static const GFL_UI_TP_HITTBL target_tp_hittbl[TARGET_MAX +1] =
{
  { GFL_UI_TP_USE_CIRCLE,   83,  107,   23 },  // circle
  { GFL_UI_TP_USE_CIRCLE,  128,   82,   22 },  // circle
  { GFL_UI_TP_USE_CIRCLE,  172,  107,   23 },  // circle
  { 21*8, 24*8 -1, 9*8, 23*8 -1 },  // rect
  { GFL_UI_TP_HIT_END, 0, 0, 0 },  // テーブル終了
};

// 指指しカーソルの位置
static const u8 finger_pos[TARGET_POKE_MAX][2] =
{
  {  83,  58 },
  { 128,  38 },
  { 172,  58 },
};

// ポケモン大小セット
// monsno
static const u16 poke_monsno[TARGET_POKE_MAX] =
{
  MONSNO_557,  // MONSNO_???の???はグラフィックナンバー、定義されている値はmonsno
  MONSNO_554,
  MONSNO_551,
};
// gmm
static const u32 poke_str_id[TARGET_POKE_MAX] =
{
  msg_t01r0102_ball_water_01,
  msg_t01r0102_ball_fire_01,
  msg_t01r0102_ball_grass_01,
};

// リソース
static const u8 poke_res_big[TARGET_POKE_MAX][TWO_OBJ_RES_FILE_MAX] =
{
  { NARC_psel_psel_557_m_NCGR, NARC_psel_psel_557_n_NCLR, NARC_psel_psel_557_m_NCER, NARC_psel_psel_557_m_NANR, NARC_psel_psel_557_x_NCLR },
  { NARC_psel_psel_554_m_NCGR, NARC_psel_psel_554_n_NCLR, NARC_psel_psel_554_m_NCER, NARC_psel_psel_554_m_NANR, NARC_psel_psel_554_x_NCLR },
  { NARC_psel_psel_551_m_NCGR, NARC_psel_psel_551_n_NCLR, NARC_psel_psel_551_m_NCER, NARC_psel_psel_551_m_NANR, NARC_psel_psel_551_x_NCLR },
};
// パレット
static const u8 poke_obj_pal_pos_s[TARGET_POKE_MAX] =
{
  OBJ_PAL_POS_S_MIZU, 
  OBJ_PAL_POS_S_HONOO,
  OBJ_PAL_POS_S_KUSA,
};

// ポケモンの動き
typedef struct
{
  u8 p0_x;             // p0の位置
  u8 p0_y;             // p0は一番奥の開始場所
  u8 p1_x;             // p1の位置
  u8 p1_y;             // p1はジャンプで着地した中間地点
  u8 p2_x;             // p2の位置
  u8 p2_y;             // p2は一番手前の中央
  u8 p0_scale;         // p0におけるbigのスケール(p0_scale/16として使う)
  u8 p1_scale;         // p1におけるbigのスケール(p0_scale/16として使う)
  u8 p2_scale;         // p2におけるbigのスケール(p0_scale/16として使う)
  u8 p1_frame;         // p1で待つフレーム数
  u8 p0p1_frame;       // p0p1間の移動にかかるフレーム数
  u8 p1p2_frame;       // p1p2間の移動にかかるフレーム数
  u8 p0p1_jump;        // p0p1間の最大のジャンプの高さ
  u8 p1p2_jump;        // p1p2間の最大のジャンプの高さ
}
POKE_MOVE_DATA;

#if 0
  __ ...p0p1_jump
 /  \
+    \   __ ...p1p2_jump
p0    \ /  \
       +    \
       p1    \
              +
              p2
#endif

static const POKE_MOVE_DATA poke_move_data[TARGET_POKE_MAX] =
{
  {
     53,  99,  90, 109, 128, 120,
     8, 12, 16,
    16,
     4,  4,
    32, 32,
  },
  {
    128,  94, 128, 107, 128, 120,
     8, 12, 16,
    16,
     4,  4,
    32, 32,
  },
  {
    202,  99, 165, 109, 128, 120,
     8, 12, 16,
    16,
     4,  4,
    32, 32,
  },
};

typedef enum
{
  POKE_MOVE_STEP_P0_STOP,   // 1フレーム
  POKE_MOVE_STEP_P0_TO_P1,  // p0p1_frame
  POKE_MOVE_STEP_P1_WAIT,   // p1_frame
  POKE_MOVE_STEP_P1_TO_P2,  // p1p2_frame
  POKE_MOVE_STEP_P2_STOP,   // 1フレーム
}
POKE_MOVE_STEP;

typedef enum
{
  POKE_MOVE_REQ_NONE,        // 要求なし
  POKE_MOVE_REQ_P0_TO_P2,    // 奥から手前へ移動するよう要求
  POKE_MOVE_REQ_P2_TO_P0,    // 手前から奥へ移動するよう要求
}
POKE_MOVE_REQ;

// 描画優先度
#define POKE_SOFTPRI_BACK  (1)  // 奥
#define POKE_SOFTPRI_FRONT (0)  // 手前


// ボール内をイメージしたサブ画面BG
enum
{
  INSIDE_BALL_MIZU    = TARGET_MIZU,    // TARGETと同じ並び順にしておく
  INSIDE_BALL_HONOO   = TARGET_HONOO,
  INSIDE_BALL_KUSA    = TARGET_KUSA,
  INSIDE_BALL_NONE,
  INSIDE_BALL_MAX,
};

// ボール内をイメージしたサブ画面BGのパレットデータ
static const u8 inside_ball_res[INSIDE_BALL_MAX] =
{
  NARC_psel_psel_bg02_w_NCLR, 
  NARC_psel_psel_bg02_f_NCLR, 
  NARC_psel_psel_bg02_l_NCLR, 
  NARC_psel_psel_bg02_NCLR, 
};

// ボール内をイメージしたサブ画面BGのパレット
enum
{
  INSIDE_BALL_PAL_START,  // 開始色
  INSIDE_BALL_PAL_TRANS,  // 現在色
  INSIDE_BALL_PAL_MAX,
};

// パレットアニメの状態
typedef enum
{
  POKE_PAL_ANIME_STATE_COLOR,              // カラーにしたいときにPsel_PalPokeAnimeStartに引数として渡せる状態
  POKE_PAL_ANIME_STATE_COLOR_TO_DARK,
  POKE_PAL_ANIME_STATE_DARK,               // ダークにしたいときにPsel_PalPokeAnimeStartに引数として渡せる状態
  POKE_PAL_ANIME_STATE_DARK_TO_COLOR,
}
POKE_PAL_ANIME_STATE;
typedef enum
{
  INSIDE_BALL_PAL_ANIME_STATE_END,
  INSIDE_BALL_PAL_ANIME_STATE_START_TO_END,
}
INSIDE_BALL_PAL_ANIME_STATE;

// ポケモン大小
enum
{
  POKE_SMALL,
  POKE_BIG,
  POKE_MAX,
};
// ポケモン大小パレット
enum
{
  POKE_PAL_COLOR,  // 本来の色
  POKE_PAL_DARK,   // 非選択時の暗色
  POKE_PAL_TRANS,  // 現在色
  POKE_PAL_MAX,
};


// 3D個別
// 三匹選択
// s01
static const GFL_G3D_UTIL_RES s01_res_tbl[] = 
{
  { ARCID_PSEL,        NARC_psel_psel_s01_nsbmd,          GFL_G3D_UTIL_RESARC },
  { ARCID_PSEL,        NARC_psel_psel_s01_nsbca,          GFL_G3D_UTIL_RESARC },
  { ARCID_PSEL,        NARC_psel_psel_s01_nsbma,          GFL_G3D_UTIL_RESARC },
  { ARCID_PSEL,        NARC_psel_psel_s01_nsbta,          GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_ANM s01_anm_tbl[] = 
{
  { 1, 0 },
  { 2, 0 },
  { 3, 0 },
};
static const GFL_G3D_UTIL_OBJ s01_obj_tbl[] = 
{
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    s01_anm_tbl,               // アニメテーブル(複数指定のため)
    NELEMS(s01_anm_tbl),       // アニメリソース数
  },
};

// s02
static const GFL_G3D_UTIL_RES s02_res_tbl[] = 
{
  // s02
  { ARCID_PSEL,        NARC_psel_psel_s02_nsbmd,          GFL_G3D_UTIL_RESARC },
  { ARCID_PSEL,        NARC_psel_psel_s02_nsbca,          GFL_G3D_UTIL_RESARC },
  // mb
  { ARCID_PSEL,        NARC_psel_psel_mb_nsbmd,           GFL_G3D_UTIL_RESARC },
  { ARCID_PSEL,        NARC_psel_psel_mb_nsbma,           GFL_G3D_UTIL_RESARC },
};
static const GFL_G3D_UTIL_ANM s02_anm_tbl[] = 
{
  // s02
  { 1, 0 },
};
static const GFL_G3D_UTIL_ANM mb_anm_tbl[] = 
{
  // mb
  { 3, 0 },
};
static const GFL_G3D_UTIL_OBJ s02_obj_tbl[] = 
{
  // s02
  {
    0,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    0,                         // テクスチャリソースID
    s02_anm_tbl,               // アニメテーブル(複数指定のため)
    NELEMS(s02_anm_tbl),       // アニメリソース数
  },
  // mb
  {
    2,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    2,                         // テクスチャリソースID
    mb_anm_tbl,                // アニメテーブル(複数指定のため)
    NELEMS(mb_anm_tbl),        // アニメリソース数
  },
  {
    2,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    2,                         // テクスチャリソースID
    mb_anm_tbl,                // アニメテーブル(複数指定のため)
    NELEMS(mb_anm_tbl),        // アニメリソース数
  },
  {
    2,                         // モデルリソースID
    0,                         // モデルデータID(リソース内部INDEX)
    2,                         // テクスチャリソースID
    mb_anm_tbl,                // アニメテーブル(複数指定のため)
    NELEMS(mb_anm_tbl),        // アニメリソース数
  },
};
enum
{
  MB_L     = TARGET_MIZU,  // TARGETと同じ並び順にしておく
  MB_C     = TARGET_HONOO,
  MB_R     = TARGET_KUSA,
  MB_MAX
};
static const VecFx32 three_mb_trans[MB_MAX] =
{
  { FX_F32_TO_FX32(-12.5f), FX_F32_TO_FX32(29.0f), FX_F32_TO_FX32( 3.6f) },
  { FX_F32_TO_FX32(  0.0f), FX_F32_TO_FX32(29.0f), FX_F32_TO_FX32(-7.0f) },
  { FX_F32_TO_FX32( 12.5f), FX_F32_TO_FX32(29.0f), FX_F32_TO_FX32( 3.6f) },
};

// 3D全体
// 最大数
#define THREE_RES_MAX              (4)   // s02とmbを同時に読み込んだときの総数
#define THREE_OBJ_MAX              (4)   // s02とmbを同時に読み込んだときの総数
#define THREE_ICA_ANIME_INTERVAL   (10)
// セットアップ番号
enum
{
  THREE_SETUP_IDX_S01,
  THREE_SETUP_IDX_S02,
  THREE_SETUP_IDX_MAX
};
// セットアップデータ
static const GFL_G3D_UTIL_SETUP three_setup_tbl[THREE_SETUP_IDX_MAX] =
{
  { s01_res_tbl,   NELEMS(s01_res_tbl),   s01_obj_tbl,   NELEMS(s01_obj_tbl)   },
  { s02_res_tbl,   NELEMS(s02_res_tbl),   s02_obj_tbl,   NELEMS(s02_obj_tbl)   },
};
// ユーザ(このソースのプログラムを書いた人)が決めたオブジェクト番号
enum
{
  THREE_USER_OBJ_IDX_S01,
  THREE_USER_OBJ_IDX_S02,
  THREE_USER_OBJ_IDX_MB_L,  // MB_(MB_MAX)と同じ並び順にしておく
  THREE_USER_OBJ_IDX_MB_C,
  THREE_USER_OBJ_IDX_MB_R,
  THREE_USER_OBJ_IDX_MAX,
};


// 3Dフレーム数
/*
デザイナーさんの3Dオーサリングツール上では1フレームからスタートしているようだが、プログラムでは開始フレームを0として処理する
デザイナーさん指定のフレーム数
#define TIMETABLE_S01_CAMERA_MOVE      (  1)
#define TIMETABLE_S01_CARD_COME        ( 41)
#define TIMETABLE_S01_CARD_READ        ( 70)  // このフレームで止めておく
#define TIMETABLE_S01_CARD_VANISH      ( 71)
#define TIMETABLE_S01_RIBBON_LOOSE     ( 91)
#define TIMETABLE_S01_SCENE_END        (140)  // このフレームで止めておく

#define TIMETABLE_S02_BOX_OPEN         (  1)
#define TIMETABLE_S02_BOX_STOP         ( 40)  // このフレームで止めておく
*/
#define TIMETABLE_S01_CAMERA_MOVE      (  0)
#define TIMETABLE_S01_CARD_COME        ( 40)
#define TIMETABLE_S01_CARD_READ        ( 69)  // このフレームで止めておく
#define TIMETABLE_S01_CARD_VANISH      ( 70)
#define TIMETABLE_S01_RIBBON_LOOSE     ( 90)
#define TIMETABLE_S01_SCENE_END        (139)  // このフレームで止めておく

#define TIMETABLE_S02_BOX_OPEN         (  0)
#define TIMETABLE_S02_BOX_STOP         ( 39)  // このフレームで止めておく

enum
{
  TIMETABLE_MB_ZOOM_START,
  TIMETABLE_MB_ZOOM_END,        // このフレームで止めておく
  TIMETABLE_MB_DECIDE_START,
  TIMETABLE_MB_DECIDE_END,      // このフレームで止めておく
  TIMETABLE_MB_MAX,
};
/*
static const u8 timetable_mb[MB_MAX][TIMETABLE_MB_MAX] =
{
  {  41,  50,  71,  80 },
  {  51,  60,  81,  90 },
  {  61,  70,  91, 100 },
};
*/
static const u8 timetable_mb[MB_MAX][TIMETABLE_MB_MAX] =
{
  {  40,  49,  70,  79 },
  {  50,  59,  80,  89 },
  {  60,  69,  90,  99 },
};

// VBlank中のリクエスト
enum
{
  VBLANK_REQ_NONE                         = 0,
  VBLANK_REQ_S01_TEXT_M_VISIBLE_ON        = 1<< 0,
  VBLANK_REQ_S01_TEXT_M_VISIBLE_OFF       = 1<< 1,
  VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_ON    = 1<< 2,
  VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_OFF   = 1<< 3,
  VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_ON    = 1<< 4,
  VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_OFF   = 1<< 5,
  VBLANK_REQ_S02_TEXT_M_VISIBLE_ON        = 1<< 6,
  VBLANK_REQ_S02_TEXT_M_VISIBLE_OFF       = 1<< 7,
};


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// 3Dオブジェクトのプロパティ
//=====================================
typedef struct
{
  u16                         idx;        // GFL_G3D_UTILが割り振る番号
  GFL_G3D_OBJSTATUS           objstatus;
  BOOL                        draw;       // TRUEのとき描画する
}
THREE_OBJ_PROPERTY;
static THREE_OBJ_PROPERTY* ThreeObjPropertyCreateArray( HEAPID heap_id, u16 num );
static void ThreeObjPropertyDeleteArray( THREE_OBJ_PROPERTY* prop );
static THREE_OBJ_PROPERTY* ThreeObjPropertyCreateArray( HEAPID heap_id, u16 num )
{
  u16 i;
  THREE_OBJ_PROPERTY* prop_array = GFL_HEAP_AllocClearMemory( heap_id, sizeof(THREE_OBJ_PROPERTY) * num );
  // 0以外の値で初期化するものについて初期化を行う 
  for( i=0; i<num; i++ )
  {
    THREE_OBJ_PROPERTY* prop = &(prop_array[i]);
    VEC_Set( &(prop->objstatus.trans), 0, 0, 0 );
    VEC_Set( &(prop->objstatus.scale), FX32_ONE, FX32_ONE, FX32_ONE );
    MTX_Identity33( &(prop->objstatus.rotate) );
  }
  return prop_array;
}
static void ThreeObjPropertyDeleteArray( THREE_OBJ_PROPERTY* prop_array )
{
  GFL_HEAP_FreeMemory( prop_array );
}

//-------------------------------------
/// ポケモン大小セット
//=====================================
typedef struct
{
  u32        res[POKE_MAX][TWO_OBJ_RES_MAX];
  GFL_CLWK*  clwk[POKE_MAX];

  // オリジナルパレットデータ
  u16        pal[POKE_PAL_MAX][16];
  
  // パレットアニメの状態
  u16        pal_anime_state;
  u16        pal_anime_count;

  // 動き
  u16              move_step_count;   // POKE_MOVE_STEP各段階におけるカウント
  POKE_MOVE_STEP   move_step;
  POKE_MOVE_REQ    move_req;
}
POKE_SET;


//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                      heap_id;
  PSEL_PARAM*                 param;

  // タッチorキー
  int                         ktst;  // GFL_APP_END_KEY(GFL_APP_KTST_KEY) or GFL_APP_END_TOUCH(GFL_APP_KTST_TOUCH)

  // シーケンス
  int                         sub_seq;
  // シーケンスフレーム
  u32                         sub_seq_frame;  // sub_seqが切り替わったら0からスタートし直しsub_seqのフレーム数をカウントするなどの使い方をしていい、が、使わなくてもいい。使う場合は使う場所で自分で初期化して下さい。

  // グラフィック、フォントなど
  PSEL_GRAPHIC_WORK*          graphic;
  GFL_FONT*                   font;
  PRINT_QUE*                  print_que;

  // VBlank中TCB
  GFL_TCB*                    vblank_tcb;
  u32                         vblank_req;

  // 3D全体
  GFL_G3D_UTIL*               three_util;
  u16                         three_unit_idx[THREE_SETUP_IDX_MAX];             // GFL_G3D_UTILが割り振る番号        // 添え字はTHREE_SETUP_IDX_(THREE_SETUP_IDX_MAX)
  u16                         three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_MAX];  // three_obj_prop_tblのインデックス  // 添え字はTHREE_USER_OBJ_IDX_(THREE_USER_OBJ_IDX_MAX)
  u16                         three_obj_prop_num;
  THREE_OBJ_PROPERTY*         three_obj_prop_tbl;
  ICA_ANIME*                  three_ica_anime;

  // モンスターボールに対して、ズームしたり決定したりするアニメ
  TARGET                      three_mb_anime_target;
  BOOL                        three_mb_anime_reverse;
  u32                         three_mb_anime_frame;
  
  // フレーム
  u32                         three_frame;  // シーンが切り替わったら0からスタートし直し、3Dのフレーム数と一致するようにしておく

  // 2D OBJ
  // ポケモン大小セット
  POKE_SET                    poke_set[TARGET_POKE_MAX];
  // 指指しカーソル
  u32                         finger_res[TWO_OBJ_RES_MAX];
  GFL_CLWK*                   finger_clwk;
  
  // 選んでいるポケモン
  TARGET                      select_target_poke;  // TARGET_POKE_START<= <=TARGET_POKE_ENDおよびTARGET_NONEのどれか

  // POKE_INFOテキストを書くフラグ
  BOOL                        poke_info_print;  // これがTRUEのとき、ポケモンのタイプと種族名をサブ画面に書く(TRUEになったフレームで書いたらもしくは書けなかったらFALSEにするので、TRUEになっているのは1フレームだけ)

  // テキスト
  GFL_MSGDATA*                msgdata;
  GFL_BMPWIN*                 text_bmpwin[TEXT_MAX];
  BOOL                        text_finish[TEXT_MAX];  // bmpwinの転送が済んでいればTRUE
  PRINT_STREAM*               print_stream;
  GFL_TCBLSYS*                ps_tcblsys;
  STRBUF*                     ps_strbuf;
  APP_KEYCURSOR_WORK*         app_keycursor_wk;

  // APP_TASKMENU
  APP_TASKMENU_RES*           app_taskmenu_res;
  APP_TASKMENU_WIN_WORK*      app_taskmenu_win_wk;  // タッチのときに出る「これに決めた」ボタン
  APP_TASKMENU_WORK*          app_taskmenu_wk;      // タッチでもキーでも出る最終確認の「はい」「いいえ」ボタン

  // ブレンド
  u8                          ev1;  // G2S_SetBlendAlphaのev1  // 0<=ev1<=16  // ev2=16-ev1

  // ボール内をイメージしたサブ画面BGのパレット
  // オリジナルパレットデータ
  u16                         inside_ball_pal[INSIDE_BALL_MAX][16*BG_PAL_NUM_S_INSIDE_BALL];
  u16                         inside_ball_st_pal[INSIDE_BALL_PAL_MAX][16*BG_PAL_NUM_S_INSIDE_BALL];
  // パレットアニメの状態
  INSIDE_BALL_PAL_ANIME_STATE inside_ball_pal_anime_state;
  u8                          inside_ball_pal_anime_count;
  u8                          inside_ball_pal_anime_end;
}
PSEL_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void Psel_VBlankFunc( GFL_TCB* tcb, void* wk );

// 3D描画(GRAPHIC_3D_StartDrawとPSEL_GRAPHIC_3D_EndDrawの間で呼ぶ)
static void Psel_ThreeDraw( PSEL_WORK* work );

// 3D全体
static void Psel_ThreeInit( PSEL_WORK* work );
static void Psel_ThreeExit( PSEL_WORK* work );

// 3D個別
static void Psel_ThreeS01Init( PSEL_WORK* work );
static void Psel_ThreeS01Exit( PSEL_WORK* work );
static void Psel_ThreeS01Main( PSEL_WORK* work );

static void Psel_ThreeS02Init( PSEL_WORK* work );
static void Psel_ThreeS02Exit( PSEL_WORK* work );
static void Psel_ThreeS02Main( PSEL_WORK* work );

static void Psel_ThreeS02MbZoomAnimeStart( PSEL_WORK* work, TARGET target, BOOL reverse );
static BOOL Psel_ThreeS02MbZoomAnimeIsEnd( PSEL_WORK* work );
static void Psel_ThreeS02MbZoomAnimeMain( PSEL_WORK* work );
static void Psel_ThreeS02MbAnimeOnlyMbMain( PSEL_WORK* work );
static void Psel_ThreeS02MbDecideAnimeStart( PSEL_WORK* work, TARGET target );
static void Psel_ThreeS02MbDecideAnimeMain( PSEL_WORK* work );

// ポケモン大小セット
static void Psel_PokeSetInit( PSEL_WORK* work );
static void Psel_PokeSetExit( PSEL_WORK* work );
static void Psel_PokeSetMain( PSEL_WORK* work );
static void Psel_PokeSetSelectStart( PSEL_WORK* work, TARGET target_poke );

// 指指しカーソル
static void Psel_FingerInit( PSEL_WORK* work );
static void Psel_FingerExit( PSEL_WORK* work );
static void Psel_FingerDrawEnable( PSEL_WORK* work, BOOL on_off );
static void Psel_FingerUpdatePos( PSEL_WORK* work );

// BG全体
static void Psel_BgInit( PSEL_WORK* work );
static void Psel_BgExit( PSEL_WORK* work );

// BG個別
static void Psel_BgS01Init( PSEL_WORK* work );
static void Psel_BgS01Exit( PSEL_WORK* work );
static void Psel_BgS01Main( PSEL_WORK* work );

static void Psel_BgS02Init( PSEL_WORK* work );
static void Psel_BgS02Exit( PSEL_WORK* work );
static void Psel_BgS02Main( PSEL_WORK* work );

static void Psel_InsideBallInit( PSEL_WORK* work );
static void Psel_InsideBallExit( PSEL_WORK* work );

// テキスト全体
static void Psel_TextInit( PSEL_WORK* work );
static void Psel_TextExit( PSEL_WORK* work );
static void Psel_TextMain( PSEL_WORK* work );
static BOOL Psel_TextTransWait( PSEL_WORK* work );  // TRUEが返って来たら転送待ちなし
static void Psel_TextExplainStreamStart( PSEL_WORK* work, u32 str_id );
static BOOL Psel_TextExplainStreamWait( PSEL_WORK* work );  // TRUEが返って来たら終了
static void Psel_TextExplainPrintStart( PSEL_WORK* work, u32 str_id );
static void Psel_TextExplainClear( PSEL_WORK* work );
static void Psel_TextPokeInfoPrintStart( PSEL_WORK* work, TARGET target );
static void Psel_TextPokeInfoClear( PSEL_WORK* work );

// テキスト個別
static void Psel_TextS01Init( PSEL_WORK* work );
static void Psel_TextS01Exit( PSEL_WORK* work );
static void Psel_TextS01Main( PSEL_WORK* work );

// APP_TASKMENU
static void Psel_AppTaskmenuResInit( PSEL_WORK* work );
static void Psel_AppTaskmenuResExit( PSEL_WORK* work );

static void Psel_AppTaskmenuWinInit( PSEL_WORK* work );
static void Psel_AppTaskmenuWinExit( PSEL_WORK* work );
static void Psel_AppTaskmenuWinMain( PSEL_WORK* work );
static void Psel_AppTaskmenuWinDecideStart( PSEL_WORK* work );
static BOOL Psel_AppTaskmenuWinDecideIsEnd( PSEL_WORK* work );

static void Psel_AppTaskmenuInit( PSEL_WORK* work );
static void Psel_AppTaskmenuExit( PSEL_WORK* work );
static void Psel_AppTaskmenuMain( PSEL_WORK* work );
static BOOL Psel_AppTaskmenuIsEnd( PSEL_WORK* work );
static BOOL Psel_AppTaskmenuIsYes( PSEL_WORK* work );

// パレット
static void U16ToRGB( u16 rgb, u16* r, u16* g, u16* b );
static void Psel_PokeSetPalInit( PSEL_WORK* work );
static void Psel_PokeSetPalExit( PSEL_WORK* work );
static void Psel_PokeSetPalMain( PSEL_WORK* work );
static void Psel_InsideBallPalInit( PSEL_WORK* work );
static void Psel_InsideBallPalExit( PSEL_WORK* work );
static void Psel_InsideBallPalMain( PSEL_WORK* work );
static void Psel_PalInit( PSEL_WORK* work );
static void Psel_PalExit( PSEL_WORK* work );
static void Psel_PalMain( PSEL_WORK* work );
static void Psel_PokeSetPalAnimeStart( PSEL_WORK* work, TARGET target_poke, POKE_PAL_ANIME_STATE state );
static void Psel_InsideBallPalAnimeStart( PSEL_WORK* work, u8 end );


// シーケンス
typedef int (*PSEL_FUNC)( PSEL_WORK* work, int* seq );

static int Psel_Start      ( PSEL_WORK* work, int* seq );
static int Psel_Load       ( PSEL_WORK* work, int* seq );
static int Psel_S01Init    ( PSEL_WORK* work, int* seq );
static int Psel_S01FadeIn  ( PSEL_WORK* work, int* seq );
static int Psel_S01Main    ( PSEL_WORK* work, int* seq );
static int Psel_S01FadeOut ( PSEL_WORK* work, int* seq );
static int Psel_S01Exit    ( PSEL_WORK* work, int* seq );
static int Psel_S02Init    ( PSEL_WORK* work, int* seq );
static int Psel_S02FadeIn  ( PSEL_WORK* work, int* seq );
static int Psel_S02Main    ( PSEL_WORK* work, int* seq );
static int Psel_S02FadeOut ( PSEL_WORK* work, int* seq );
static int Psel_S02Exit    ( PSEL_WORK* work, int* seq );
static int Psel_Unload     ( PSEL_WORK* work, int* seq );
static int Psel_End        ( PSEL_WORK* work, int* seq );

static const PSEL_FUNC func_tbl[SEQ_MAX] =
{
  Psel_Start,
  Psel_Load,
  Psel_S01Init,
  Psel_S01FadeIn,
  Psel_S01Main,
  Psel_S01FadeOut,
  Psel_S01Exit,
  Psel_S02Init,
  Psel_S02FadeIn,
  Psel_S02Main,
  Psel_S02FadeOut,
  Psel_S02Exit,
  Psel_Unload,
  Psel_End,
};


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT Psel_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Psel_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT Psel_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    PSEL_ProcData =
{
  Psel_ProcInit,
  Psel_ProcMain,
  Psel_ProcExit,
};


//=============================================================================
/**
*  外部公開関数定義
*/
//=============================================================================
//------------------------------------------------------------------
/**
 *  @brief           パラメータ生成
 *
 *  @param[in]       heap_id       ヒープID
 *
 *  @retval          PSEL_PARAM
 */
//------------------------------------------------------------------
PSEL_PARAM*  PSEL_AllocParam(
                                HEAPID               heap_id
                           )
{
  PSEL_PARAM* param = GFL_HEAP_AllocMemory( heap_id, sizeof( PSEL_PARAM ) );
/*
  PSEL_InitParam(
      param
      );
*/
  return param;
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータ解放
 *
 *  @param[in,out]   param      パラメータ生成で生成したもの
 *
 *  @retval          
 */
//------------------------------------------------------------------
void            PSEL_FreeParam(
                            PSEL_PARAM*  param )
{
  GFL_HEAP_FreeMemory( param );
}

//------------------------------------------------------------------
/**
 *  @brief           パラメータを設定する
 *
 *  @param[in,out]   param         PSEL_PARAM
 *
 *  @retval          
 */
//------------------------------------------------------------------
/*
void  PSEL_InitParam(
                  PSEL_PARAM*      param
                         )
{
  param->result             = PSEL_RESULT_MIZU;
}
*/

//------------------------------------------------------------------
/**
 *  @brief           選択結果を得る
 *
 *  @param[in]       param         PSEL_PARAM
 *
 *  @retval          選択結果
 */
//------------------------------------------------------------------
PSEL_RESULT  PSEL_GetResult(
                  const PSEL_PARAM*      param )
{
  return param->result;
}


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT Psel_ProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PSEL_WORK*     work;

  // 黒
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PSEL, HEAP_SIZE );
    work = GFL_PROC_AllocWork( proc, sizeof(PSEL_WORK), HEAPID_PSEL );
    GFL_STD_MemClear( work, sizeof(PSEL_WORK) );
    
    work->heap_id       = HEAPID_PSEL;
    work->param         = (PSEL_PARAM*)pwk;
  }

  // タッチorキー
  work->ktst = GFL_UI_CheckTouchOrKey();

  // 選んでいるポケモン
  work->select_target_poke = TARGET_NONE;

  // POKE_INFOテキストを書くフラグ
  work->poke_info_print = FALSE;

  // シーケンス
  work->sub_seq = 0;

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT Psel_ProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PSEL_WORK*     work      = (PSEL_WORK*)mywk;
  
//  // 黒
//  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 16, 0 );
  // 白
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 16, 0 );

  // シーケンス
  // 何もしない

  // タッチorキー
  GFL_UI_SetTouchOrKey( work->ktst );

  // ヒープ、パラメータなど
  {
    // 選択結果をパラメータに設定して持ち帰る
    {
      work->param->result = work->select_target_poke;  // PSEL_RESULTとTARGETは同じ並び順
    }

    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_PSEL );
  }

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT Psel_ProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  PSEL_WORK*     work      = (PSEL_WORK*)mywk;
  
  int            seq_prev;
  
  seq_prev = *seq;
  *seq = (func_tbl[*seq])(work, &(work->sub_seq));
  if( *seq != SEQ_END )
  {
    if( *seq != seq_prev )
    {
      work->sub_seq = 0;
    }
  }
  else
  {
    return GFL_PROC_RES_FINISH;
  }

  if( SEQ_S01_INIT <= (*seq) && (*seq) <= SEQ_S02_EXIT )  // この時点で*seqは次のシーケンスになっている
  {
    // メイン
    PRINTSYS_QUE_Main( work->print_que );
    
    Psel_TextMain( work );

    // 2D描画
    PSEL_GRAPHIC_2D_Draw( work->graphic );

    // 3D描画
    PSEL_GRAPHIC_3D_StartDraw( work->graphic );
    Psel_ThreeDraw( work );
    PSEL_GRAPHIC_3D_EndDraw( work->graphic );
  }

  return GFL_PROC_RES_CONTINUE;
}

//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// VBlank関数
//=====================================
static void Psel_VBlankFunc( GFL_TCB* tcb, void* wk )
{
  PSEL_WORK* work = (PSEL_WORK*)wk;

  if( work->vblank_req & VBLANK_REQ_S01_TEXT_M_VISIBLE_ON )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_ON );
  }

  if( work->vblank_req & VBLANK_REQ_S01_TEXT_M_VISIBLE_OFF )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_OFF );
  }

  if( work->vblank_req & VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_ON )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_M_WIN, VISIBLE_ON );
  }

  if( work->vblank_req & VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_OFF )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_OFF );
    GFL_BG_SetVisible( BG_FRAME_M_WIN, VISIBLE_OFF );
  }

  if( work->vblank_req & VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_ON )
  {
    GFL_BG_SetVisible( BG_FRAME_S_TEXT, VISIBLE_ON );
    GFL_BG_SetVisible( BG_FRAME_S_WIN, VISIBLE_ON );
  }

  if( work->vblank_req & VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_OFF )
  {
    GFL_BG_SetVisible( BG_FRAME_S_TEXT, VISIBLE_OFF );
    GFL_BG_SetVisible( BG_FRAME_S_WIN, VISIBLE_OFF );
  }

  if( work->vblank_req & VBLANK_REQ_S02_TEXT_M_VISIBLE_ON )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_ON );
  }
  
  if( work->vblank_req & VBLANK_REQ_S02_TEXT_M_VISIBLE_OFF )
  {
    GFL_BG_SetVisible( BG_FRAME_M_TEXT, VISIBLE_OFF );
  }

  work->vblank_req = VBLANK_REQ_NONE;
}

//-------------------------------------
/// 3D描画(GRAPHIC_3D_StartDrawとPSEL_GRAPHIC_3D_EndDrawの間で呼ぶ)
//=====================================
static void Psel_ThreeDraw( PSEL_WORK* work )
{
  u16 i;
  for( i=0; i<work->three_obj_prop_num; i++ )
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
    if( prop->draw )
    {
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      GFL_G3D_DRAW_DrawObject( obj, &(prop->objstatus) );
    }
  }
}

//-------------------------------------
/// 3D全体
//=====================================
static void Psel_ThreeInit( PSEL_WORK* work )
{
  // 3D管理ユーティリティーのセットアップ
  work->three_util = GFL_G3D_UTIL_Create( THREE_RES_MAX, THREE_OBJ_MAX, work->heap_id );

  // NULL、ゼロ初期化
  work->three_obj_prop_num = 0;
}
static void Psel_ThreeExit( PSEL_WORK* work )
{
  // 3D管理ユーティリティーの破棄
  GFL_G3D_UTIL_Delete( work->three_util );
}

//-------------------------------------
/// 3D個別
//=====================================
static void Psel_ThreeS01Init( PSEL_WORK* work )
{
  // ユニット追加
  {
    u16 i = THREE_SETUP_IDX_S01;
    {
      work->three_unit_idx[i] = GFL_G3D_UTIL_AddUnit( work->three_util, &three_setup_tbl[i] );
    }
  }

  // オブジェクト全体
  {
    work->three_obj_prop_num = 1;
    work->three_obj_prop_tbl = ThreeObjPropertyCreateArray( work->heap_id, work->three_obj_prop_num );
  }

  // オブジェクト
  {
    u16 h = 0;

    u16 i = THREE_SETUP_IDX_S01;
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( work->three_util, work->three_unit_idx[i] );
      u16 head_user_obj_idx = THREE_USER_OBJ_IDX_S01;
      u16 j = 0;
      {
        THREE_OBJ_PROPERTY* prop;
        work->three_obj_prop_tbl_idx[head_user_obj_idx +j] = h;
        prop = &(work->three_obj_prop_tbl[h]);
        prop->idx  = head_obj_idx +j;
        prop->draw = TRUE;
        h++;
      }
    }
  }

  // アニメーション有効化
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      u16 j;
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }

  // カメラアニメ
  {
    work->three_ica_anime = ICA_ANIME_CreateStreamingAlloc(
        work->heap_id, ARCID_PSEL, NARC_psel_psel_s01_cam_bin, THREE_ICA_ANIME_INTERVAL );
    
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  // フレーム
/*
  デザイナーさんの3Dオーサリングツール上では1フレームからスタートしているようだが、プログラムでは開始フレームを0として処理する
  work->three_frame = 1;  // 0スタートではなく1スタート
*/
  work->three_frame = 0;
}
static void Psel_ThreeS01Exit( PSEL_WORK* work )
{
  // ユニット破棄
  {
    u16 i = THREE_SETUP_IDX_S01;
    {
      GFL_G3D_UTIL_DelUnit( work->three_util, work->three_unit_idx[i] );
    }
  }

  // オブジェクト全体
  {
    ThreeObjPropertyDeleteArray( work->three_obj_prop_tbl );
    work->three_obj_prop_num = 0;
  }

  // カメラアニメ
  {
    ICA_ANIME_Delete( work->three_ica_anime );
  }
}
static void Psel_ThreeS01Main( PSEL_WORK* work )
{
  const fx32 anime_speed = FX32_ONE;  // 増加分（FX32_ONEで１フレーム進める）
  
  // アニメーション更新
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      u16 j;
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
      }
    }
  }

  // カメラアニメ
  {
    ICA_ANIME_IncAnimeFrame( work->three_ica_anime, anime_speed );
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  // フレーム
  work->three_frame++;
}

static void Psel_ThreeS02Init( PSEL_WORK* work )
{
  // ユニット追加
  {
    u16 i = THREE_SETUP_IDX_S02;
    {
      work->three_unit_idx[i] = GFL_G3D_UTIL_AddUnit( work->three_util, &three_setup_tbl[i] );
    }
  }

  // オブジェクト全体
  {
    work->three_obj_prop_num = THREE_USER_OBJ_IDX_MB_R - THREE_USER_OBJ_IDX_S02 +1;
    work->three_obj_prop_tbl = ThreeObjPropertyCreateArray( work->heap_id, work->three_obj_prop_num );
  }

  // オブジェクト
  {
    u16 h = 0;
    
    u16 i = THREE_SETUP_IDX_S02;
    {
      u16 head_obj_idx = GFL_G3D_UTIL_GetUnitObjIdx( work->three_util, work->three_unit_idx[i] );
      u16 head_user_obj_idx = THREE_USER_OBJ_IDX_S02;
      u16 j;
      for( j=0; j<THREE_USER_OBJ_IDX_MB_R - THREE_USER_OBJ_IDX_S02 +1; j++ )
      {
        THREE_OBJ_PROPERTY* prop;
        work->three_obj_prop_tbl_idx[head_user_obj_idx +j] = h;
        prop = &(work->three_obj_prop_tbl[h]);
        prop->idx  = head_obj_idx +j;
        prop->draw = TRUE;
        if( THREE_USER_OBJ_IDX_MB_L <= (head_user_obj_idx +j) && (head_user_obj_idx +j) <= THREE_USER_OBJ_IDX_MB_R )
          prop->objstatus.trans = three_mb_trans[(head_user_obj_idx +j) - THREE_USER_OBJ_IDX_MB_L];
        h++;
      }
    }
  }

/*
  // アニメーション有効化
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      u16 j;
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_EnableAnime( obj, j );
      }
    }
  }
*/
  // ボックスのアニメーションだけ有効化
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_S02] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    u16 j;
    for( j=0; j<anime_count; j++ )
    {
      GFL_G3D_OBJECT_EnableAnime( obj, j );
    }
  }
  
  // カメラアニメ
  {
    work->three_ica_anime = ICA_ANIME_CreateStreamingAlloc(
        work->heap_id, ARCID_PSEL, NARC_psel_psel_s02_cam_bin, THREE_ICA_ANIME_INTERVAL );
    
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  // フレーム
/*
  デザイナーさんの3Dオーサリングツール上では1フレームからスタートしているようだが、プログラムでは開始フレームを0として処理する
  work->three_frame = 1;  // 0スタートではなく1スタート
*/
  work->three_frame = 0;
}
static void Psel_ThreeS02Exit( PSEL_WORK* work )
{
  // ユニット破棄
  {
    u16 i = THREE_SETUP_IDX_S02;
    {
      GFL_G3D_UTIL_DelUnit( work->three_util, work->three_unit_idx[i] );
    }
  }

  // オブジェクト全体
  {
    ThreeObjPropertyDeleteArray( work->three_obj_prop_tbl );
    work->three_obj_prop_num = 0;
  }

  // カメラアニメ
  {
    ICA_ANIME_Delete( work->three_ica_anime );
  }
}
static void Psel_ThreeS02Main( PSEL_WORK* work )
{
  const fx32 anime_speed = FX32_ONE;  // 増加分（FX32_ONEで１フレーム進める）

/*
  // アニメーション更新
  {
    u16 i;
    for( i=0; i<work->three_obj_prop_num; i++ )
    {
      THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[i]);
      GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
      u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
      u16 j;
      for( j=0; j<anime_count; j++ )
      {
        GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
      }
    }
  }
*/
  // ボックスのアニメーションだけ更新
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_S02] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    u16 j;
    for( j=0; j<anime_count; j++ )
    {
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
    }
  }

  // カメラアニメ
  {
    ICA_ANIME_IncAnimeFrame( work->three_ica_anime, anime_speed );
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  // フレーム
  work->three_frame++;
}

static void Psel_ThreeS02MbZoomAnimeStart( PSEL_WORK* work, TARGET target, BOOL reverse )
{
  u8 start_frame = timetable_mb[target][(reverse)?(TIMETABLE_MB_ZOOM_END):(TIMETABLE_MB_ZOOM_START)];

  // カメラアニメ
  {
    ICA_ANIME_SetAnimeFrame( work->three_ica_anime, FX32_CONST(start_frame) );  // これを呼ぶ前の状態のカメラと同じ位置のカメラのはず
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  // 対象のモンスターボールのアニメーションだけ無効化
  if( reverse )  // ズームアウトするときは、モンスターボールを対象からはずす
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_MB_L +target] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    u16 j;
    for( j=0; j<anime_count; j++ )
    {
      GFL_G3D_OBJECT_DisableAnime( obj, j );
    }
  }

  work->three_mb_anime_target   = target;
  work->three_mb_anime_reverse  = reverse;
  work->three_mb_anime_frame    = start_frame;
}
static BOOL Psel_ThreeS02MbZoomAnimeIsEnd( PSEL_WORK* work )
{
  u8 end_frame = timetable_mb[work->three_mb_anime_target][(work->three_mb_anime_reverse)?(TIMETABLE_MB_ZOOM_START):(TIMETABLE_MB_ZOOM_END)];

  if( work->three_mb_anime_frame == end_frame ) return TRUE;
  return FALSE;
}
static void Psel_ThreeS02MbZoomAnimeMain( PSEL_WORK* work )
{
  u8 end_frame = timetable_mb[work->three_mb_anime_target][(work->three_mb_anime_reverse)?(TIMETABLE_MB_ZOOM_START):(TIMETABLE_MB_ZOOM_END)];
  
  if( work->three_mb_anime_frame != end_frame )
  {
    fx32 anime_speed;
    anime_speed = (work->three_mb_anime_reverse)?(-FX32_ONE):(FX32_ONE);  // 増加分（FX32_ONEで１フレーム進める）

    // カメラアニメ
    {
      ICA_ANIME_IncAnimeFrame( work->three_ica_anime, anime_speed );
      ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
    }

    // フレーム
    (work->three_mb_anime_reverse)?(work->three_mb_anime_frame--):(work->three_mb_anime_frame++);

    if( work->three_mb_anime_frame == end_frame )  // フレームを進めたことで、終了フレームになったか
    {
      if( work->three_mb_anime_reverse )  // どのモンスターボールも対象としていないカメラに戻す
      {
        // カメラアニメ
        {
          ICA_ANIME_SetAnimeFrame( work->three_ica_anime, FX32_CONST(TIMETABLE_S02_BOX_STOP) );  // これを呼ぶ前の状態のカメラと同じ位置のカメラのはず
          ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
        }
      }
      else  // ズームインしたときは、対象のモンスターボールのアニメを有効にして目立たせる
      {
        // 対象のモンスターボールのアニメーションだけ有効化
        {
          THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_MB_L +work->three_mb_anime_target] ]);
          GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
          u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
          u16 j;
          for( j=0; j<anime_count; j++ )
          {
            GFL_G3D_OBJECT_EnableAnime( obj, j );
          }
        }
      }
    }
  }
  else
  {
    if( !(work->three_mb_anime_reverse) )  // 対象のモンスターボールのアニメだけ継続させる
    {
      Psel_ThreeS02MbAnimeOnlyMbMain( work );
    }
  }
}

static void Psel_ThreeS02MbAnimeOnlyMbMain( PSEL_WORK* work )  // 対象のモンスターボールがある場合は、アニメを途切れさせずに更新しなければならないので、Psel_ThreeS02MbZoomAnimeMainを呼ばないところではこの関数を呼ぶこと。
{
  fx32 anime_speed = FX32_ONE;  // 増加分（FX32_ONEで１フレーム進める）

  // 対象のモンスターボールのアニメーションだけ更新
  {
    THREE_OBJ_PROPERTY* prop = &(work->three_obj_prop_tbl[ work->three_obj_prop_tbl_idx[THREE_USER_OBJ_IDX_MB_L +work->three_mb_anime_target] ]);
    GFL_G3D_OBJ* obj = GFL_G3D_UTIL_GetObjHandle( work->three_util, prop->idx );
    u16 anime_count = GFL_G3D_OBJECT_GetAnimeCount( obj );
    u16 j;
    for( j=0; j<anime_count; j++ )
    {
      GFL_G3D_OBJECT_LoopAnimeFrame( obj, j, anime_speed );
    }
  }
}

static void Psel_ThreeS02MbDecideAnimeStart( PSEL_WORK* work, TARGET target )
{
  u8 start_frame = timetable_mb[target][TIMETABLE_MB_DECIDE_START];

  // カメラアニメ
  {
    ICA_ANIME_SetAnimeFrame( work->three_ica_anime, FX32_CONST(start_frame) );  // これを呼ぶ前の状態のカメラと同じ位置のカメラのはず
    ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
  }

  work->three_mb_anime_target   = target;
  work->three_mb_anime_reverse  = FALSE;
  work->three_mb_anime_frame    = start_frame;
}
static void Psel_ThreeS02MbDecideAnimeMain( PSEL_WORK* work )
{
  u8 end_frame = timetable_mb[work->three_mb_anime_target][TIMETABLE_MB_DECIDE_END];
  
  if( work->three_mb_anime_frame != end_frame )
  {
    fx32 anime_speed = FX32_ONE;  // 増加分（FX32_ONEで１フレーム進める）

    // カメラアニメ
    {
      ICA_ANIME_IncAnimeFrame( work->three_ica_anime, anime_speed );
      ICA_CAMERA_SetCameraStatus( PSEL_GRAPHIC_GetCamera( work->graphic ), work->three_ica_anime );
    }

    // フレーム
    work->three_mb_anime_frame++;
  }
}


//-------------------------------------
/// ポケモン大小セット
//=====================================
static void Psel_PokeSetInit( PSEL_WORK* work )
{
  u8 i;
  u8 j;

  int  formno       = 0;                  // 0<=formno<POKETOOL_GetPersonalParam( monsno, 0, POKEPER_ID_form_max )
  int  sex          = PTL_SEX_MALE;       // PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN
  int  rare         = 0;                  // 0(レアじゃない)か1(レア)  // PP_CheckRare( pp ) 
  int  dir          = POKEGRA_DIR_FRONT;  // POKEGRA_DIR_FRONT, POKEGRA_DIR_BACK
  BOOL egg          = FALSE;              // FALSE(タマゴじゃない)かTRUE(タマゴ)
  u32  personal_rnd = 0;                  // ?

  // small
  j = POKE_SMALL;
  // リソースの読み込み
  {
    ARCHANDLE* handle = POKE2DGRA_OpenHandle( work->heap_id );
    for( i=0; i<TARGET_POKE_MAX; i++ )
    {
      work->poke_set[i].res[j][TWO_OBJ_RES_NCG] = POKE2DGRA_OBJ_CGR_Register(
          handle,
          poke_monsno[i],
          formno, sex, rare, dir, egg,
          personal_rnd,
          CLSYS_DRAW_SUB,
          work->heap_id );

      work->poke_set[i].res[j][TWO_OBJ_RES_NCL] = POKE2DGRA_OBJ_PLTT_Register(
          handle,
          poke_monsno[i],
          formno, sex, rare, dir, egg,
          CLSYS_DRAW_SUB,
          poke_obj_pal_pos_s[i] * 0x20,
          work->heap_id );

      work->poke_set[i].res[j][TWO_OBJ_RES_NCE] = POKE2DGRA_OBJ_CELLANM_Register(
          poke_monsno[i],
          formno, sex, rare, dir, egg,
          APP_COMMON_MAPPING_32K,
          CLSYS_DRAW_SUB,
          work->heap_id );
    }
    GFL_ARC_CloseDataHandle( handle );
  }
  // CLWK作成
  {
    for( i=0; i<TARGET_POKE_MAX; i++ )
    {
      GFL_CLWK_DATA cldata;
      GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

      cldata.pos_x    = poke_move_data[i].p0_x;
      cldata.pos_y    = poke_move_data[i].p0_y;
      cldata.anmseq   = 0;
      cldata.softpri  = POKE_SOFTPRI_BACK;
      cldata.bgpri    = OBJ_BG_PRI_S_POKE;

      work->poke_set[i].clwk[j] = GFL_CLACT_WK_Create(
          PSEL_GRAPHIC_GetClunit( work->graphic ),
          work->poke_set[i].res[j][TWO_OBJ_RES_NCG],
          work->poke_set[i].res[j][TWO_OBJ_RES_NCL],
          work->poke_set[i].res[j][TWO_OBJ_RES_NCE],
          &cldata, CLSYS_DEFREND_SUB, work->heap_id );
      
      GFL_CLACT_WK_SetDrawEnable( work->poke_set[i].clwk[j], TRUE );
    }
  }

  // big
  j = POKE_BIG;
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_PSEL, work->heap_id );
    for( i=0; i<TARGET_POKE_MAX; i++ )
    {
/*
      smallとbigは同じパレットなので、samllのを使用する
      work->poke_set[i].res[j][TWO_OBJ_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
          handle,
          poke_res_big[i][TWO_OBJ_RES_NCL],
          CLSYS_DRAW_SUB,
          poke_obj_pal_pos_s[i] * 0x20,
          0,  // データ読み出し開始位置を0に固定しているが大丈夫かな、デザイナさんにお願いしておかないと
          1,  // 転送本数を1に固定しているが大丈夫かな、デザイナさんにお願いしておかないと
          work->heap_id );
*/
      work->poke_set[i].res[j][TWO_OBJ_RES_NCG] = GFL_CLGRP_CGR_Register(
          handle,
          poke_res_big[i][TWO_OBJ_RES_NCG],
          FALSE,
          CLSYS_DRAW_SUB,
          work->heap_id );
      work->poke_set[i].res[j][TWO_OBJ_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
          handle,
          poke_res_big[i][TWO_OBJ_RES_NCE],
          poke_res_big[i][TWO_OBJ_RES_NAN],
          work->heap_id );
    }
    GFL_ARC_CloseDataHandle( handle );
  }
  // CLWK作成
  {
    for( i=0; i<TARGET_POKE_MAX; i++ )
    {
      GFL_CLWK_AFFINEDATA claffinedata;
      GFL_STD_MemClear( &claffinedata, sizeof(GFL_CLWK_AFFINEDATA) );

      claffinedata.clwkdata.pos_x    = poke_move_data[i].p0_x;
      claffinedata.clwkdata.pos_y    = poke_move_data[i].p0_y;
      claffinedata.clwkdata.anmseq   = 0;
      claffinedata.clwkdata.softpri  = POKE_SOFTPRI_BACK;
      claffinedata.clwkdata.bgpri    = OBJ_BG_PRI_S_POKE;

      claffinedata.affinepos_x    = 0;
      claffinedata.affinepos_y    = 0;
      claffinedata.scale_x        = FX_F32_TO_FX32( ((f32)(poke_move_data[i].p0_scale))/16.0f );
      claffinedata.scale_y        = FX_F32_TO_FX32( ((f32)(poke_move_data[i].p0_scale))/16.0f );
      claffinedata.rotation       = 0;  // 回転角度(0～0xffff 0xffffが360度)
      claffinedata.affine_type    = CLSYS_AFFINETYPE_NORMAL;  // 縮小しかしないので、倍角でなくてよい

      work->poke_set[i].clwk[j] = GFL_CLACT_WK_CreateAffine(
          PSEL_GRAPHIC_GetClunit( work->graphic ),
          work->poke_set[i].res[j][TWO_OBJ_RES_NCG],
          //work->poke_set[i].res[j][TWO_OBJ_RES_NCL],
          work->poke_set[i].res[POKE_SMALL][TWO_OBJ_RES_NCL],
          work->poke_set[i].res[j][TWO_OBJ_RES_NCE],
          &claffinedata, CLSYS_DEFREND_SUB, work->heap_id );
      
      GFL_CLACT_WK_SetDrawEnable( work->poke_set[i].clwk[j], FALSE );
    }
  }

  // 動き
  {
    for( i=0; i<TARGET_POKE_MAX; i++ )
    {
      work->poke_set[i].move_step_count   = 0;
      work->poke_set[i].move_step         = POKE_MOVE_STEP_P0_STOP;
      work->poke_set[i].move_req          = POKE_MOVE_REQ_NONE;
    }
  }
}
static void Psel_PokeSetExit( PSEL_WORK* work )
{
  u8 i;
  u8 j;
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    for( j=0; j<POKE_MAX; j++ )
    {
      // CLWK破棄
      GFL_CLACT_WK_Remove( work->poke_set[i].clwk[j] );

      // リソース破棄
      GFL_CLGRP_CELLANIM_Release( work->poke_set[i].res[j][TWO_OBJ_RES_NCE] );
      GFL_CLGRP_CGR_Release( work->poke_set[i].res[j][TWO_OBJ_RES_NCG] );
      if( j != POKE_BIG ) GFL_CLGRP_PLTT_Release( work->poke_set[i].res[j][TWO_OBJ_RES_NCL] );  // smallとbigは同じパレットなので、smallのしか読み込んでいない
    }
  }
}

static void PokeSetCalcPosScale( GFL_CLACTPOS* pos, GFL_CLSCALE* scale,
                                 u8 pos_s_x, u8 pos_s_y, u8 pos_e_x, u8 pos_e_y,
                                 u8 scale_s, u8 scale_e,
                                 u8 total_frame, u8 jump_y,
                                 u16 count );
static void PokeSetCalcPosScale( GFL_CLACTPOS* pos, GFL_CLSCALE* scale,
                                 u8 pos_s_x, u8 pos_s_y, u8 pos_e_x, u8 pos_e_y,
                                 u8 scale_s, u8 scale_e,
                                 u8 total_frame, u8 jump_y,
                                 u16 count )
{
  f32           f_x       = ( (f32)pos_e_x - (f32)pos_s_x ) * (f32)count / (f32)total_frame + (f32)pos_s_x;
  f32           f_y       = ( (f32)pos_e_y - (f32)pos_s_y ) * (f32)count / (f32)total_frame + (f32)pos_s_y;
          
  // 求める高さ                                y
  // 最大のジャンプの高さ                      h
  // ジャンプする区間の移動にかかるフレーム数  f
  // 現在のフレーム数(0<=t<f)                  t
  // t=0のときy=0, t=fのときy=0, t=f/2のときy=hとなるようにする
  // y = -h * ( (t-(f/2))/(f/2) )*( (t-(f/2))/(f/2) ) + h
  f32           f_y_ofs   = - ((f32)jump_y) \
                            * ( (f32)count - (f32)total_frame/2.0f ) * ( (f32)count - (f32)total_frame/2.0f ) \
                            * 4.0f / (f32)total_frame / (f32)total_frame
                            + (f32)jump_y;
          
  f32           f_scale   = ( (f32)scale_e - (f32)scale_s ) * (f32)count / (f32)total_frame + (f32)scale_s;
  s16           s_x       = (s16)f_x;
  s16           s_y       = (s16)(f_y - f_y_ofs);  // 下にいくほど正なのでマイナスする
  fx32          fx_scale  = FX_F32_TO_FX32( f_scale / 16.0f );
  
  pos->x    = s_x;
  pos->y    = s_y;
  scale->x  = scale->y  = fx_scale;
} 

static void Psel_PokeSetMain( PSEL_WORK* work )
{
  // ポケモン大小セットの動きの更新(パレットアニメはPsel_PokeSetPalMainに任せておく)

  u8 i;

  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    POKE_SET*               p = &(work->poke_set[i]);
    const POKE_MOVE_DATA*   d = &(poke_move_data[i]);
    
    s8 count_add = 0;

    if( p->move_req == POKE_MOVE_REQ_NONE ) continue;

    switch( p->move_req )
    {
    case POKE_MOVE_REQ_NONE:     count_add =  0; break;
    case POKE_MOVE_REQ_P0_TO_P2: count_add = +1; break;
    case POKE_MOVE_REQ_P2_TO_P0: count_add = -1; break;
    }

    switch( p->move_step )
    {
    case POKE_MOVE_STEP_P0_STOP:
      {
        if( count_add > 0 )
        {
          GFL_CLACT_WK_SetDrawEnable( p->clwk[POKE_SMALL], FALSE );
          GFL_CLACT_WK_SetDrawEnable( p->clwk[POKE_BIG], TRUE );

          p->move_step_count = 0;
          p->move_step = POKE_MOVE_STEP_P0_TO_P1;
          
          // break;しない
        }
        else
        {
          if( p->move_req == POKE_MOVE_REQ_P2_TO_P0 ) p->move_req = POKE_MOVE_REQ_NONE;
          
          break;
        }
      }
    case POKE_MOVE_STEP_P0_TO_P1:
      {
        if( p->move_step_count == 0 )
        {
          GFL_CLACTPOS  pos;
          GFL_CLSCALE   scale;
          pos.x    = d->p0_x;
          pos.y    = d->p0_y;
          scale.x  = scale.y  = FX_F32_TO_FX32( (f32)d->p0_scale / 16.0f );
          GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
          GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );
        }
        else
        {
          GFL_CLACTPOS  pos;
          GFL_CLSCALE   scale;
          PokeSetCalcPosScale( &pos, &scale,
                               d->p0_x, d->p0_y, d->p1_x, d->p1_y,
                               d->p0_scale, d->p1_scale,
                               d->p0p1_frame, d->p0p1_jump,
                               p->move_step_count );
          GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
          GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );
        }

        if( p->move_step_count == 0 && count_add < 0 )
        {
          GFL_CLACT_WK_SetDrawEnable( p->clwk[POKE_SMALL], TRUE );
          GFL_CLACT_WK_SetDrawEnable( p->clwk[POKE_BIG], FALSE );
            
          p->move_step_count = 0;
          p->move_step = POKE_MOVE_STEP_P0_STOP;
        }
        else
        {
          p->move_step_count = p->move_step_count + count_add;
          if( p->move_step_count == d->p0p1_frame )
          {
            p->move_step_count = 0;
            p->move_step = POKE_MOVE_STEP_P1_WAIT;
          }
        }
      }
      break;
    case POKE_MOVE_STEP_P1_WAIT:
      {
        GFL_CLACTPOS  pos;
        GFL_CLSCALE   scale;
        pos.x    = d->p1_x;
        pos.y    = d->p1_y;
        scale.x  = scale.y  = FX_F32_TO_FX32( (f32)d->p1_scale / 16.0f );
        GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
        GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );

        if( p->move_step_count == 0 && count_add < 0 )
        {
          p->move_step_count = d->p0p1_frame -1;
          p->move_step = POKE_MOVE_STEP_P0_TO_P1;
        }
        else
        {
          p->move_step_count = p->move_step_count + count_add;
          if( p->move_step_count == d->p1_frame )
          {
            p->move_step_count = 0;
            p->move_step = POKE_MOVE_STEP_P1_TO_P2;
          }
        }
      }
      break;
    case POKE_MOVE_STEP_P1_TO_P2:
      {
        if( p->move_step_count == 0 )
        {
          GFL_CLACTPOS  pos;
          GFL_CLSCALE   scale;
          pos.x    = d->p1_x;
          pos.y    = d->p1_y;
          scale.x  = scale.y  = FX_F32_TO_FX32( (f32)d->p1_scale / 16.0f );
          GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
          GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );
        }
        else
        {
          GFL_CLACTPOS  pos;
          GFL_CLSCALE   scale;
          PokeSetCalcPosScale( &pos, &scale,
                               d->p1_x, d->p1_y, d->p2_x, d->p2_y,
                               d->p1_scale, d->p2_scale,
                               d->p1p2_frame, d->p1p2_jump,
                               p->move_step_count );
          GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
          GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );
        }

        if( p->move_step_count == 0 && count_add < 0 )
        {
          p->move_step_count = d->p1_frame -1;
          p->move_step = POKE_MOVE_STEP_P1_WAIT;
        }
        else
        {
          p->move_step_count = p->move_step_count + count_add;
          if( p->move_step_count == d->p1p2_frame )
          {
            p->move_step_count = 0;
            p->move_step = POKE_MOVE_STEP_P2_STOP;
          }
        }
      }
      break;
    case POKE_MOVE_STEP_P2_STOP:
      {
        GFL_CLACTPOS  pos;
        GFL_CLSCALE   scale;
        pos.x    = d->p2_x;
        pos.y    = d->p2_y;
        scale.x  = scale.y  = FX_F32_TO_FX32( (f32)d->p2_scale / 16.0f );
        GFL_CLACT_WK_SetPos( p->clwk[POKE_BIG], &pos, CLSYS_DEFREND_SUB );
        GFL_CLACT_WK_SetScale( p->clwk[POKE_BIG], &scale );
        
        if( count_add < 0 )
        {
          p->move_step_count = d->p1p2_frame -1;
          p->move_step = POKE_MOVE_STEP_P1_TO_P2;
        }
        else
        {
          if( p->move_req == POKE_MOVE_REQ_P0_TO_P2 )
          {
            p->move_req = POKE_MOVE_REQ_NONE;
            work->poke_info_print = TRUE;  // ポケモンのタイプと種族名を書く
          }
        }
      }
      break;
    }
  }
}

static void Psel_PokeSetSelectStart( PSEL_WORK* work, TARGET target_poke )  // 何回呼んでも、アニメの途中で呼んでも、大丈夫な頑強なつくりになっている
{
  u8 i;

  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    // パレットアニメ
    {
      if( i == target_poke )
      {
        Psel_PokeSetPalAnimeStart( work, i, POKE_PAL_ANIME_STATE_COLOR );
      }
      else
      {
        Psel_PokeSetPalAnimeStart( work, i, POKE_PAL_ANIME_STATE_DARK );
      }
    }

    // 動き
    {
      if( i == target_poke )
      {
        work->poke_set[i].move_req   = POKE_MOVE_REQ_P0_TO_P2;
        GFL_CLACT_WK_SetSoftPri( work->poke_set[i].clwk[POKE_BIG], POKE_SOFTPRI_FRONT );
      }
      else
      {
        work->poke_set[i].move_req   = POKE_MOVE_REQ_P2_TO_P0;
        GFL_CLACT_WK_SetSoftPri( work->poke_set[i].clwk[POKE_BIG], POKE_SOFTPRI_BACK );
      }
    }
  }
}


//-------------------------------------
/// 指指しカーソル
//=====================================
static void Psel_FingerInit( PSEL_WORK* work )
{
  // リソース読み込み
  {
    ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_PSEL, work->heap_id );
    work->finger_res[TWO_OBJ_RES_NCL] = GFL_CLGRP_PLTT_RegisterEx( 
        handle,
        NARC_psel_psel_cursol_NCLR,
        CLSYS_DRAW_MAIN,
        OBJ_PAL_POS_M_FINGER * 0x20,
        0,
        OBJ_PAL_NUM_M_FINGER,
        work->heap_id );
    work->finger_res[TWO_OBJ_RES_NCG] = GFL_CLGRP_CGR_Register(
        handle,
        NARC_psel_psel_cursol_NCGR,
        FALSE,
        CLSYS_DRAW_MAIN,
        work->heap_id );
    work->finger_res[TWO_OBJ_RES_NCE] = GFL_CLGRP_CELLANIM_Register(
        handle,
        NARC_psel_psel_cursol_NCER,
        NARC_psel_psel_cursol_NANR,
        work->heap_id );
    GFL_ARC_CloseDataHandle( handle );
  }

  // CLWK作成
  {
    GFL_CLWK_DATA cldata;
    GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );

    cldata.pos_x    = finger_pos[TARGET_POKE_START][0];
    cldata.pos_y    = finger_pos[TARGET_POKE_START][1];
    cldata.anmseq   = 0;
    cldata.softpri  = 0;
    cldata.bgpri    = OBJ_BG_PRI_M_FINGER;

    work->finger_clwk = GFL_CLACT_WK_Create(
        PSEL_GRAPHIC_GetClunit( work->graphic ),
        work->finger_res[TWO_OBJ_RES_NCG], work->finger_res[TWO_OBJ_RES_NCL], work->finger_res[TWO_OBJ_RES_NCE],
        &cldata, CLSYS_DEFREND_MAIN, work->heap_id );
    GFL_CLACT_WK_SetAutoAnmFlag( work->finger_clwk, TRUE );

    GFL_CLACT_WK_SetDrawEnable( work->finger_clwk, FALSE );
  }
}
static void Psel_FingerExit( PSEL_WORK* work )
{
  // CLWK破棄
  GFL_CLACT_WK_Remove( work->finger_clwk );
  
  // リソース破棄
  GFL_CLGRP_CELLANIM_Release( work->finger_res[TWO_OBJ_RES_NCE] );
  GFL_CLGRP_CGR_Release( work->finger_res[TWO_OBJ_RES_NCG] );
  GFL_CLGRP_PLTT_Release( work->finger_res[TWO_OBJ_RES_NCL] );
}
static void Psel_FingerDrawEnable( PSEL_WORK* work, BOOL on_off )
{
  GFL_CLACT_WK_SetDrawEnable( work->finger_clwk, on_off );
  if( on_off ) Psel_FingerUpdatePos( work );
}
static void Psel_FingerUpdatePos( PSEL_WORK* work )
{
  GFL_CLACTPOS pos;
  pos.x = finger_pos[work->select_target_poke][0];
  pos.y = finger_pos[work->select_target_poke][1];
  GFL_CLACT_WK_SetPos( work->finger_clwk, &pos, CLSYS_DEFREND_MAIN );
}

//-------------------------------------
/// BG
//=====================================
static void Psel_BgInit( PSEL_WORK* work )
{
  // ウィンドウ用のパレットとキャラを用意する

  // パレットの作成＆転送
  {
    u16* pal = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 0x10 );
    pal[0x00] = 0x0000;  // 透明
    pal[0x01] = 0x0000;  // 黒
    pal[0x02] = 0x7fff;  // 白

    GFL_BG_LoadPalette( BG_FRAME_M_WIN, pal, 0x20, BG_PAL_POS_M_WIN * 0x20 );
    GFL_BG_LoadPalette( BG_FRAME_S_WIN, pal, 0x20, BG_PAL_POS_S_WIN * 0x20 );
    
    GFL_HEAP_FreeMemory( pal );
  }

  // キャラの作成＆転送
  {
    GFL_BMP_DATA* bmp = GFL_BMP_Create( 2, 1, GFL_BMP_16_COLOR, work->heap_id );
    GFL_BMP_Fill( bmp, 0, 0, 8, 8, 0x00 );  // 0キャラ目
    GFL_BMP_Fill( bmp, 8, 0, 8, 8, 0x01 );  // 1キャラ目

    GFL_BG_LoadCharacter( BG_FRAME_M_WIN,
        GFL_BMP_GetCharacterAdrs(bmp), GFL_BMP_GetBmpDataSize(bmp),
        0 );
    GFL_BG_LoadCharacter( BG_FRAME_S_WIN,
        GFL_BMP_GetCharacterAdrs(bmp), GFL_BMP_GetBmpDataSize(bmp),
        0 );

    GFL_BMP_Delete( bmp );
  }
}
static void Psel_BgExit( PSEL_WORK* work )
{
  // 何もしない
}

static void Psel_BgS01Init( PSEL_WORK* work )
{
  // ウィンドウ用のスクリーンを用意する

  // スクリーンの作成＆転送
  {
    u16* scr = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 32*24 );
    u8 i, j;
    u16 h = 0;
    for(j=0; j<24; j++)       // TWL_ProgramingManual.pdf TWLプログラミングマニュアル
    {                         // 6.2.3.2.2 スクリーンデータのアドレスマッピング
      for(i=0; i<32; i++)     // スクリーンサイズが256×256 ドットのとき
      {                       // を参考にした。
        u16 chara_name = (4<=j&&j<20)?1:0;  // 1キャラ目:0キャラ目
        u16 flip_h     = 0;
        u16 flip_v     = 0;
        u16 pal        = BG_PAL_POS_M_WIN;
        scr[h] = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
        h++;
      }
    }

    GFL_BG_WriteScreen( BG_FRAME_M_WIN, scr, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( BG_FRAME_M_WIN );
    
    GFL_HEAP_FreeMemory( scr );
  }
}
static void Psel_BgS01Exit( PSEL_WORK* work )
{
  u16 chara_name = 0;  // 0キャラ目
  u16 flip_h     = 0;
  u16 flip_v     = 0;
  u16 pal        = BG_PAL_POS_M_WIN;
  u16 scr = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
  GFL_BG_ClearScreenCodeVReq( BG_FRAME_M_WIN, scr );
}

static void Psel_BgS02Init( PSEL_WORK* work )
{
  // ウィンドウ用のスクリーンを用意する

  // メイン画面
  // スクリーンの作成＆転送
  {
    u16* scr = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 32*24 );
    u8 i, j;
    u16 h = 0;
    for(j=0; j<24; j++)       // TWL_ProgramingManual.pdf TWLプログラミングマニュアル
    {                         // 6.2.3.2.2 スクリーンデータのアドレスマッピング
      for(i=0; i<32; i++)     // スクリーンサイズが256×256 ドットのとき
      {                       // を参考にした。
        u16 chara_name = (17<=j&&j<23)?1:0;  // 1キャラ目:0キャラ目
        u16 flip_h     = 0;
        u16 flip_v     = 0;
        u16 pal        = BG_PAL_POS_M_WIN;
        scr[h] = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
        h++;
      }
    }

    GFL_BG_WriteScreen( BG_FRAME_M_WIN, scr, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( BG_FRAME_M_WIN );
    
    GFL_HEAP_FreeMemory( scr );
  }

  // サブ画面
  // スクリーンの作成＆転送
  {
    u16* scr = GFL_HEAP_AllocClearMemory( work->heap_id, sizeof(u16) * 32*24 );
    u8 i, j;
    u16 h = 0;
    for(j=0; j<24; j++)       // TWL_ProgramingManual.pdf TWLプログラミングマニュアル
    {                         // 6.2.3.2.2 スクリーンデータのアドレスマッピング
      for(i=0; i<32; i++)     // スクリーンサイズが256×256 ドットのとき
      {                       // を参考にした。
        u16 chara_name = (17<=j&&j<23)?1:0;  // 1キャラ目:0キャラ目
        u16 flip_h     = 0;
        u16 flip_v     = 0;
        u16 pal        = BG_PAL_POS_S_WIN;
        scr[h] = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
        h++;
      }
    }

    GFL_BG_WriteScreen( BG_FRAME_S_WIN, scr, 0, 0, 32, 24 );
    GFL_BG_LoadScreenV_Req( BG_FRAME_S_WIN );
    
    GFL_HEAP_FreeMemory( scr );
  }
}
static void Psel_BgS02Exit( PSEL_WORK* work )
{
  u16 chara_name = 0;  // 0キャラ目
  u16 flip_h     = 0;
  u16 flip_v     = 0;
  u16 pal;
  u16 scr;

  // メイン画面
  {
    pal        = BG_PAL_POS_M_WIN;
    scr = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
    GFL_BG_ClearScreenCodeVReq( BG_FRAME_M_WIN, scr );
  }

  // サブ画面
  {
    pal        = BG_PAL_POS_S_WIN;
    scr = ( pal << 12 ) | ( flip_v << 11 ) | ( flip_h << 10 ) | ( chara_name << 0 );
    GFL_BG_ClearScreenCodeVReq( BG_FRAME_S_WIN, scr );
  }
}

static void Psel_InsideBallInit( PSEL_WORK* work )
{
  ARCHANDLE* handle = GFL_ARC_OpenDataHandle( ARCID_PSEL, work->heap_id );

  GFL_ARCHDL_UTIL_TransVramPaletteEx(
      handle,
      inside_ball_res[INSIDE_BALL_NONE],
      PALTYPE_SUB_BG,
      BG_PAL_POS_S_INSIDE_BALL * 0x20,
      BG_PAL_POS_S_INSIDE_BALL * 0x20,
      BG_PAL_NUM_S_INSIDE_BALL * 0x20,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramBgCharacter(
      handle,
      NARC_psel_psel_bg02_NCGR,
      BG_FRAME_S_INSIDE_BALL,
			0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARCHDL_UTIL_TransVramScreen(
      handle,
      NARC_psel_psel_bg02_NSCR,
      BG_FRAME_S_INSIDE_BALL,
      0,
      0,  // 全転送
      FALSE,
      work->heap_id );

  GFL_ARC_CloseDataHandle( handle );

  GFL_BG_LoadScreenReq( BG_FRAME_S_INSIDE_BALL );
}
static void Psel_InsideBallExit( PSEL_WORK* work )
{
  // 何もしない
}

//-------------------------------------
/// テキスト
//=====================================
static void Psel_TextInit( PSEL_WORK* work )
{
  u8 i;

  // パレット
  GFL_ARC_UTIL_TransVramPalette(
      ARCID_FONT,
      NARC_font_default_nclr,
      PALTYPE_MAIN_BG,
      BG_PAL_POS_M_TEXT * 0x20,
      BG_PAL_NUM_M_TEXT * 0x20,
      work->heap_id );

  GFL_ARC_UTIL_TransVramPalette(
      ARCID_FONT,
      NARC_font_default_nclr,
      PALTYPE_SUB_BG,
      BG_PAL_POS_S_TEXT * 0x20,
      BG_PAL_NUM_S_TEXT * 0x20,
      work->heap_id );

  // メッセージ
  work->msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_SCRIPT_MESSAGE, NARC_script_message_t01r0102_dat, work->heap_id );

  // TCBL
  work->ps_tcblsys = GFL_TCBL_Init( work->heap_id, work->heap_id, 1, 0 );

  // メッセージ送りキーカーソルアイコン
  work->app_keycursor_wk = APP_KEYCURSOR_CreateEx( TEXT_COLOR_B, TRUE, TRUE, work->heap_id, (u8*)sc_skip_cursor_character );

  // フォント
  GFL_FONTSYS_SetColor( TEXT_COLOR_L, TEXT_COLOR_S, TEXT_COLOR_B );

  // ビットマップウィンドウ
  for( i=0; i<TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = GFL_BMPWIN_Create(
                            bmpwin_setup[i][0],
                            bmpwin_setup[i][1], bmpwin_setup[i][2], bmpwin_setup[i][3], bmpwin_setup[i][4],
                            bmpwin_setup[i][5],
                            bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[i]), TEXT_COLOR_B );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );
    work->text_finish[i] = TRUE;
  }

  // NULL、ゼロ初期化
  work->print_stream = NULL;
  work->ps_strbuf = NULL;
}
static void Psel_TextExit( PSEL_WORK* work )
{
  u8 i;

  if( work->print_stream ) PRINTSYS_PrintStreamDelete( work->print_stream );
  if( work->ps_strbuf ) GFL_STR_DeleteBuffer( work->ps_strbuf );

  // ビットマップウィンドウ
  for( i=0; i<TEXT_MAX; i++ )
  {
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
  }

  // メッセージ送りキーカーソルアイコン
  APP_KEYCURSOR_Delete( work->app_keycursor_wk );

  // TCBL
  GFL_TCBL_Exit( work->ps_tcblsys );

  // メッセージ
  GFL_MSG_Delete( work->msgdata );
}
static void Psel_TextMain( PSEL_WORK* work )
{
  GFL_TCBL_Main( work->ps_tcblsys );
  Psel_TextTransWait( work );
}

static BOOL Psel_TextTransWait( PSEL_WORK* work )  // TRUEが返って来たら転送待ちなし
{
  // この関数は何度呼び出しても挙動は変わらないので、何度呼び出してもいい。
  // 何度も呼び出したからといってはやく処理が行われるわけではないし、何回も転送したりするわけではないので。

  BOOL ret = TRUE;
  u8 i;

  for( i=0; i<TEXT_MAX; i++ )
  {
    if( !(work->text_finish[i]) )
    {
      if( PRINTSYS_QUE_IsExistTarget( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
      {
        ret = FALSE;
      }
      else
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
        work->text_finish[i] = TRUE;
      }
    }
  }

  return ret;
}

static void Psel_TextExplainStreamStart( PSEL_WORK* work, u32 str_id )
{
  GFL_BMPWIN* bmpwin = work->text_bmpwin[TEXT_EXPLAIN];
  s16 x = 0;
  s16 y = 0;

  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(bmpwin), TEXT_COLOR_B );
  
  // 前のを消す
  if( work->print_stream ) PRINTSYS_PrintStreamDelete( work->print_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  if( work->ps_strbuf ) GFL_STR_DeleteBuffer( work->ps_strbuf );

  // 文字列作成
  {
    work->ps_strbuf  = GFL_MSG_CreateString( work->msgdata, str_id );
  }

  // センタリング
  {
    u16     str_width   = (u16)( PRINTSYS_GetStrWidth( work->ps_strbuf, work->font, 0 ) );
    u16     str_height  = (u16)( PRINTSYS_GetStrHeight( work->ps_strbuf, work->font ) ); 

    u16     bmp_width   = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(bmpwin) );
    u16     bmp_height  = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(bmpwin) );

    // センタリング
    if( bmp_width > str_width )
    {
      x = (s16)( ( bmp_width - str_width ) / 2 );
    }
    if( bmp_height > str_height )
    {
      y = (s16)( ( bmp_height - str_height ) / 2 );
    }
  }
  
  // ストリーム開始
  work->print_stream = PRINTSYS_PrintStream(
                           bmpwin,
                           x, y,
                           work->ps_strbuf,
                           work->font,
                           MSGSPEED_GetWait(),
                           work->ps_tcblsys, 2,
                           work->heap_id,
                           TEXT_COLOR_B );

  // スクリーンをつくっておかないと表示されない
  GFL_BMPWIN_MakeTransWindow_VBlank( bmpwin );
}
static BOOL Psel_TextExplainStreamWait( PSEL_WORK* work )  // TRUEが返って来たら終了
{
  BOOL ret = FALSE;

  switch( PRINTSYS_PrintStreamGetState( work->print_stream ) )
  {
  case PRINTSTREAM_STATE_RUNNING:
    if( ( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetTrg() )
    {
      PRINTSYS_PrintStreamShortWait( work->print_stream, 0 );
    }
    break;
  case PRINTSTREAM_STATE_PAUSE:
    if( ( GFL_UI_KEY_GetTrg() & ( PAD_BUTTON_A | PAD_BUTTON_B ) ) || GFL_UI_TP_GetTrg() )
    { 
      PRINTSYS_PrintStreamReleasePause( work->print_stream );
    }
    break;
  case PRINTSTREAM_STATE_DONE:
    {
      ret = TRUE;
    }
    break;
  }
  
  return ret;
}

static void Psel_TextExplainPrintStart( PSEL_WORK* work, u32 str_id )
{
  GFL_BMPWIN* bmpwin = work->text_bmpwin[TEXT_EXPLAIN];
  BOOL*       finish = &(work->text_finish[TEXT_EXPLAIN]);
  STRBUF* strbuf;
  u16     str_width;
  u16     str_height; 
  u16     bmp_width;
  u16     bmp_height;
  u16     x = 0;
  u16     y = 0;

  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(bmpwin), TEXT_COLOR_B );
  
  // 前のを消す
  if( work->print_stream ) PRINTSYS_PrintStreamDelete( work->print_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  work->print_stream = NULL;  // この関数でprint_streamは使わないのでNULLを入れておくこと。
  if( work->ps_strbuf ) GFL_STR_DeleteBuffer( work->ps_strbuf );
  work->ps_strbuf = NULL;  // この関数でps_strbufは使わないのでNULLを入れておくこと。

  // 文字列作成
  strbuf      = GFL_MSG_CreateString( work->msgdata, str_id );

  str_width   = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
  str_height  = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) ); 

  bmp_width   = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(bmpwin) );
  bmp_height  = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(bmpwin) );

  // センタリング
  if( bmp_width > str_width )
  {
    x = ( bmp_width - str_width ) / 2;
  }
  if( bmp_height > str_height )
  {
    y = ( bmp_height - str_height ) / 2;
  }
  
  PRINTSYS_PrintQueColor(
      work->print_que,
      GFL_BMPWIN_GetBmp(bmpwin),
      (s16)x, (s16)y,
      strbuf,
      work->font,
      PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

  GFL_STR_DeleteBuffer( strbuf );

  *finish = FALSE;

  // 直ちに転送できるかもしれないので、ここで1度呼んでおく
  Psel_TextTransWait( work );
}
static void Psel_TextExplainClear( PSEL_WORK* work )
{
  // 消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_EXPLAIN]), TEXT_COLOR_B );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_EXPLAIN] );

  // ストリームを表示しているときかもしれないので、ストリームも削除 
  if( work->print_stream ) PRINTSYS_PrintStreamDelete( work->print_stream );  // この関数でタスクも削除してくれるので、同時に動くタスクは1つで済む
  work->print_stream = NULL;  // この関数でprint_streamは使わないのでNULLを入れておくこと。
  if( work->ps_strbuf ) GFL_STR_DeleteBuffer( work->ps_strbuf );
  work->ps_strbuf = NULL;  // この関数でps_strbufは使わないのでNULLを入れておくこと。
}

static void Psel_TextPokeInfoPrintStart( PSEL_WORK* work, TARGET target )
{
  GFL_BMPWIN* bmpwin = work->text_bmpwin[TEXT_POKE_INFO];
  BOOL*       finish = &(work->text_finish[TEXT_POKE_INFO]);
  u16     str_width;
  u16     str_height; 
  u16     bmp_width;
  u16     bmp_height;
  u16     x = 0;
  u16     y = 0;

  WORDSET*  wordset       = WORDSET_Create( work->heap_id );
  STRBUF*   src_strbuf    = GFL_MSG_CreateString( work->msgdata, poke_str_id[target] );
  STRBUF*   strbuf        = GFL_STR_CreateBuffer( TEXT_POKE_INFO_LEN_MAX, work->heap_id );
  WORDSET_RegisterPokeMonsNameNo( wordset, 1, poke_monsno[target] );
  WORDSET_ExpandStr( wordset, strbuf, src_strbuf );
  GFL_STR_DeleteBuffer( src_strbuf );
  WORDSET_Delete( wordset );

  // 一旦消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(bmpwin), TEXT_COLOR_B );

  str_width   = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
  str_height  = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) ); 

  bmp_width   = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(bmpwin) );
  bmp_height  = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(bmpwin) );

  // センタリング
  //if( bmp_width > str_width )            // gmmにセンタリング指定を入れてあるので
  //{                                      // プログラムで改めてセンタリングする必要はないので
  //  x = ( bmp_width - str_width ) / 2;   // コメントアウトした
  //}
  if( bmp_height > str_height )
  {
    y = ( bmp_height - str_height ) / 2;
  }
  
  PRINTSYS_PrintQueColor(
      work->print_que,
      GFL_BMPWIN_GetBmp(bmpwin),
      (s16)x, (s16)y,
      strbuf,
      work->font,
      PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

  GFL_STR_DeleteBuffer( strbuf );

  *finish = FALSE;

  // 直ちに転送できるかもしれないので、ここで1度呼んでおく
  Psel_TextTransWait( work );
}
static void Psel_TextPokeInfoClear( PSEL_WORK* work )
{
  // 消去
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_POKE_INFO]), TEXT_COLOR_B );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_POKE_INFO] );
}

static void Psel_TextS01Init( PSEL_WORK* work )
{
  STRBUF* strbuf      = GFL_MSG_CreateString( work->msgdata, msg_t01r0102_letter_01 );

  u16     str_width   = (u16)( PRINTSYS_GetStrWidth( strbuf, work->font, 0 ) );
  u16     str_height  = (u16)( PRINTSYS_GetStrHeight( strbuf, work->font ) ); 

  u16     bmp_width   = GFL_BMP_GetSizeX( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]) );
  u16     bmp_height  = GFL_BMP_GetSizeY( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]) );
 
  u16     x = 0;
  u16     y = 0;

  // センタリング
  if( bmp_width > str_width )
  {
    x = ( bmp_width - str_width ) / 2;
  }
  if( bmp_height > str_height )
  {
    y = ( bmp_height - str_height ) / 2;
  }
  
  PRINTSYS_PrintQueColor(
      work->print_que,
      GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]),
      x, y,
      strbuf,
      work->font,
      PRINTSYS_LSB_Make(TEXT_COLOR_L,TEXT_COLOR_S,TEXT_COLOR_B) );

  GFL_STR_DeleteBuffer( strbuf );

  work->text_finish[TEXT_LETTER] = FALSE;

  // 直ちに転送できるかもしれないので、ここで1度呼んでおく
  Psel_TextTransWait( work );
}
static void Psel_TextS01Exit( PSEL_WORK* work )
{
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]), TEXT_COLOR_B );
  GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[TEXT_LETTER] );
}
static void Psel_TextS01Main( PSEL_WORK* work )
{
  if( Psel_TextTransWait( work ) )
  {
    APP_KEYCURSOR_Write( work->app_keycursor_wk, GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]), TEXT_COLOR_B );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_LETTER] );
  }

  //APP_KEYCURSOR_Clear( work->app_keycursor_wk, GFL_BMPWIN_GetBmp(work->text_bmpwin[TEXT_LETTER]), TEXT_COLOR_B );
  //GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[TEXT_LETTER] );
}

//-------------------------------------
/// APP_TASKMENU
//=====================================
static void Psel_AppTaskmenuResInit( PSEL_WORK* work )
{
  work->app_taskmenu_res = APP_TASKMENU_RES_Create( BG_FRAME_M_TEXT, BG_PAL_POS_M_APP_TASKMENU, work->font, work->print_que, work->heap_id );
}
static void Psel_AppTaskmenuResExit( PSEL_WORK* work )
{
  APP_TASKMENU_RES_Delete( work->app_taskmenu_res );	
}

static void Psel_AppTaskmenuWinInit( PSEL_WORK* work )
{
  // 窓の設定
  APP_TASKMENU_ITEMWORK item;
  item.str       = GFL_MSG_CreateString( work->msgdata, msg_t01r0102_ball_select_03 );
  item.msgColor  = APP_TASKMENU_ITEM_MSGCOLOR;
  item.type      = APP_TASKMENU_WIN_TYPE_NORMAL;

  work->app_taskmenu_win_wk = APP_TASKMENU_WIN_Create(
      work->app_taskmenu_res,
      &item,
      9, 21,
      14,
      work->heap_id );

  // 文字列解放
  GFL_STR_DeleteBuffer( item.str );
}
static void Psel_AppTaskmenuWinExit( PSEL_WORK* work )
{
  APP_TASKMENU_WIN_Delete( work->app_taskmenu_win_wk );
}
static void Psel_AppTaskmenuWinMain( PSEL_WORK* work )
{
  APP_TASKMENU_WIN_Update( work->app_taskmenu_win_wk );
}
static void Psel_AppTaskmenuWinDecideStart( PSEL_WORK* work )
{
  APP_TASKMENU_WIN_SetActive( work->app_taskmenu_win_wk, TRUE );
  APP_TASKMENU_WIN_SetDecide( work->app_taskmenu_win_wk, TRUE );
}
static BOOL Psel_AppTaskmenuWinDecideIsEnd( PSEL_WORK* work )
{
  return APP_TASKMENU_WIN_IsFinish( work->app_taskmenu_win_wk );
}

static void Psel_AppTaskmenuInit( PSEL_WORK* work )
{
  int i;

  APP_TASKMENU_INITWORK init;
  APP_TASKMENU_ITEMWORK item[2];

  // 窓の設定
  for( i=0; i<NELEMS(item); i++ )
  {
    item[i].str      = GFL_MSG_CreateString( work->msgdata, msg_t01r0102_ball_select_yes +i );
    item[i].msgColor = APP_TASKMENU_ITEM_MSGCOLOR;
    //item[i].type     = APP_TASKMENU_WIN_TYPE_NORMAL;  // NORMALでも最後尾のボタンはBボタンがショートカットボタンになっているようだが、Aボタンで最後尾のボタンを選ぶと決定音が鳴りBボタンで最後尾のボタンを選ぶとキャンセル音がなる。
    item[i].type     = (i==0) ? APP_TASKMENU_WIN_TYPE_NORMAL : APP_TASKMENU_WIN_TYPE_RETURN;
  }

  // 初期化
  init.heapId    = work->heap_id;
  init.itemNum   = NELEMS(item);
  init.itemWork  = item;
  init.posType   = ATPT_RIGHT_DOWN;
  init.charPosX  = 32;  // 31キャラまで描かれる
  init.charPosY  = 17;  // 16キャラまで描かれる
  //init.w         = 7;  // いいえがぴったりおさまる幅
  init.w         = 9;  // いいえとリターンマークがぴったりおさまる幅  //APP_TASKMENU_PLATE_WIDTH;
  init.h         = APP_TASKMENU_PLATE_HEIGHT;
	
  work->app_taskmenu_wk = APP_TASKMENU_OpenMenu( &init, work->app_taskmenu_res );  // 60fps用
  //work->app_taskmenu_wk = APP_TASKMENU_OpenMenuEx( &init, work->app_taskmenu_res );  // 30fps用

  // 文字列解放
  for( i=0; i<NELEMS(item); i++ )
  {
    GFL_STR_DeleteBuffer( item[i].str );
  }
}
static void Psel_AppTaskmenuExit( PSEL_WORK* work )
{
  APP_TASKMENU_CloseMenu( work->app_taskmenu_wk );
}
static void Psel_AppTaskmenuMain( PSEL_WORK* work )
{
  APP_TASKMENU_UpdateMenu( work->app_taskmenu_wk );
}
static BOOL Psel_AppTaskmenuIsEnd( PSEL_WORK* work )
{
  return APP_TASKMENU_IsFinish( work->app_taskmenu_wk );
}
static BOOL Psel_AppTaskmenuIsYes( PSEL_WORK* work )
{
  u8 pos = APP_TASKMENU_GetCursorPos( work->app_taskmenu_wk );
  if( pos == 0 ) return TRUE;
  return FALSE;
}

//-------------------------------------
/// パレット
//=====================================
static void U16ToRGB( u16 rgb, u16* r, u16* g, u16* b )
{
  *r = ( rgb & GX_RGB_R_MASK ) >> GX_RGB_R_SHIFT;
  *g = ( rgb & GX_RGB_G_MASK ) >> GX_RGB_G_SHIFT;
  *b = ( rgb & GX_RGB_B_MASK ) >> GX_RGB_B_SHIFT;
}

static void Psel_PokeSetPalInit( PSEL_WORK* work )
{
  ARCHANDLE* handle;
  NNSG2dPaletteData* pal_data;
  void* buff;
  u8 i;

  // オリジナルパレットデータ
  // 大小同じパレットなので、大のパレットを小にも割り当てることにする
  handle = GFL_ARC_OpenDataHandle( ARCID_PSEL, work->heap_id );
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    buff = GFL_ARCHDL_UTIL_LoadPalette( handle, poke_res_big[i][TWO_OBJ_RES_NCL], &pal_data, work->heap_id );
    GFL_STD_MemCopy( pal_data->pRawData, work->poke_set[i].pal[POKE_PAL_COLOR], 2*16 );  // 0番列から入っているのでこのままでいい
    GFL_HEAP_FreeMemory( buff );
    
    buff = GFL_ARCHDL_UTIL_LoadPalette( handle, poke_res_big[i][TWO_OBJ_RES_DARK_NCL], &pal_data, work->heap_id );
    GFL_STD_MemCopy( pal_data->pRawData, work->poke_set[i].pal[POKE_PAL_DARK], 2*16 );  // 0番列から入っているのでこのままでいい
    GFL_HEAP_FreeMemory( buff );
  }
  GFL_ARC_CloseDataHandle( handle );

  // パレットアニメの状態
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    // darkにする
    work->poke_set[i].pal_anime_state = POKE_PAL_ANIME_STATE_DARK;
    work->poke_set[i].pal_anime_count = 16;
  }

/*
  なくても大丈夫のようだ
  // 確実に読み込みを終える 
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    DC_FlushRange( work->poke_set[i].pal[POKE_PAL_COLOR], 2*16 );
    DC_FlushRange( work->poke_set[i].pal[POKE_PAL_DARK], 2*16 );
  }
*/
  
  // 初期状態 
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    // darkにする
    GFL_STD_MemCopy( work->poke_set[i].pal[POKE_PAL_DARK], work->poke_set[i].pal[POKE_PAL_TRANS], 2*16 );
    NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_OBJ_PLTT_SUB,
        poke_obj_pal_pos_s[i] * 2*16,
        work->poke_set[i].pal[POKE_PAL_TRANS],
        2*16 );
  }
}
static void Psel_PokeSetPalExit( PSEL_WORK* work )
{
  // 何もしない
}
static void Psel_PokeSetPalMain( PSEL_WORK* work )
{
  u8 i;
  u8 j;

  // 0=color, 16=dark
  
  for( i=0; i<TARGET_POKE_MAX; i++ )
  {
    BOOL need_trans = FALSE;

    switch( work->poke_set[i].pal_anime_state )
    {
    case POKE_PAL_ANIME_STATE_COLOR_TO_DARK:
      {
        if( work->poke_set[i].pal_anime_count == 16 )
        {
          work->poke_set[i].pal_anime_state = POKE_PAL_ANIME_STATE_DARK;
        }
        else
        {
          work->poke_set[i].pal_anime_count++;
          need_trans = TRUE;
        }
      }
      break;
    case POKE_PAL_ANIME_STATE_DARK_TO_COLOR:
      {
        if( work->poke_set[i].pal_anime_count == 0 )
        {
          work->poke_set[i].pal_anime_state = POKE_PAL_ANIME_STATE_COLOR;
        }
        else
        {
          work->poke_set[i].pal_anime_count--;
          need_trans = TRUE;
        }
      }
      break;
    }

    if( need_trans )
    {
      switch( work->poke_set[i].pal_anime_state )
      {
      case POKE_PAL_ANIME_STATE_COLOR_TO_DARK:
      case POKE_PAL_ANIME_STATE_DARK_TO_COLOR:
        {
          for( j=0; j<16; j++ )
          {
            u16 count = work->poke_set[i].pal_anime_count;
            u16 color = work->poke_set[i].pal[POKE_PAL_COLOR][j];
            u16 dark  = work->poke_set[i].pal[POKE_PAL_DARK][j];
            u16 color_r, color_g, color_b;
            u16 dark_r, dark_g, dark_b;
            u16 trans_r, trans_g, trans_b;
            U16ToRGB( color, &color_r, &color_g, &color_b );
            U16ToRGB( dark, &dark_r, &dark_g, &dark_b );
            trans_r = ( color_r*(16-count) + dark_r*(count) ) /16;
            trans_g = ( color_g*(16-count) + dark_g*(count) ) /16;
            trans_b = ( color_b*(16-count) + dark_b*(count) ) /16;
            work->poke_set[i].pal[POKE_PAL_TRANS][j] = GX_RGB( trans_r, trans_g, trans_b );
          }
        }
        break;
      }

      NNS_GfdRegisterNewVramTransferTask(
          NNS_GFD_DST_2D_OBJ_PLTT_SUB,
          poke_obj_pal_pos_s[i] * 2*16,
          work->poke_set[i].pal[POKE_PAL_TRANS],
          2*16 );
    }
  }
}

static void Psel_InsideBallPalInit( PSEL_WORK* work )
{
  ARCHANDLE* handle;
  NNSG2dPaletteData* pal_data;
  u16* raw_data;
  void* buff;
  u8 i;

  // オリジナルパレットデータ
  handle = GFL_ARC_OpenDataHandle( ARCID_PSEL, work->heap_id );
  for( i=0; i<INSIDE_BALL_MAX; i++ )
  {
    buff = GFL_ARCHDL_UTIL_LoadPalette( handle, inside_ball_res[i], &pal_data, work->heap_id );
    raw_data = pal_data->pRawData;  // BG_PAL_POS_S_INSIDE_BALL番列から入っているので、それだけずらした位置からコピーする
    GFL_STD_MemCopy( &(raw_data[BG_PAL_POS_S_INSIDE_BALL*16]), work->inside_ball_pal[i], 2*16*BG_PAL_NUM_S_INSIDE_BALL );
    GFL_HEAP_FreeMemory( buff );
  }
  GFL_ARC_CloseDataHandle( handle );

  // パレットアニメの状態
  // noneにする
  work->inside_ball_pal_anime_state = INSIDE_BALL_PAL_ANIME_STATE_END;
  work->inside_ball_pal_anime_count = 16;
  work->inside_ball_pal_anime_end   = INSIDE_BALL_NONE;

/*
  なくても大丈夫のようだ
  // 確実に読み込みを終える 
  for( i=0; i<INSIDE_BALL_MAX; i++ )
  {
    DC_FlushRange( work->inside_ball_pal[i], 2*16*BG_PAL_NUM_S_INSIDE_BALL );
  }
*/

  // 初期状態
  // noneにする
  GFL_STD_MemCopy( work->inside_ball_pal[INSIDE_BALL_NONE], work->inside_ball_st_pal[INSIDE_BALL_PAL_TRANS], 2*16*BG_PAL_NUM_S_INSIDE_BALL );
  NNS_GfdRegisterNewVramTransferTask(
      NNS_GFD_DST_2D_BG_PLTT_SUB,
      BG_PAL_POS_S_INSIDE_BALL * 2*16,
      work->inside_ball_st_pal[INSIDE_BALL_PAL_TRANS],
      2*16*BG_PAL_NUM_S_INSIDE_BALL );
}
static void Psel_InsideBallPalExit( PSEL_WORK* work )
{
  // 何もしない
}
static void Psel_InsideBallPalMain( PSEL_WORK* work )
{
  u8 j;
  BOOL need_trans = FALSE;
  
  // 0=start, 16=end
  
  if( work->inside_ball_pal_anime_state == INSIDE_BALL_PAL_ANIME_STATE_START_TO_END )
  {
    if( work->inside_ball_pal_anime_count == 16 )
    {
      work->inside_ball_pal_anime_state = INSIDE_BALL_PAL_ANIME_STATE_END;
    }
    else
    {
      work->inside_ball_pal_anime_count++;
      need_trans = TRUE;
    }

    if( need_trans )
    {
      for( j=0; j<16*BG_PAL_NUM_S_INSIDE_BALL; j++ )
      {
        u16 count = work->inside_ball_pal_anime_count;
        u16 color = work->inside_ball_st_pal[INSIDE_BALL_PAL_START][j];
        u16 dark  = work->inside_ball_pal[ work->inside_ball_pal_anime_end ][j];
        u16 color_r, color_g, color_b;
        u16 dark_r, dark_g, dark_b;
        u16 trans_r, trans_g, trans_b;
        U16ToRGB( color, &color_r, &color_g, &color_b );
        U16ToRGB( dark, &dark_r, &dark_g, &dark_b );
        trans_r = ( color_r*(16-count) + dark_r*(count) ) /16;
        trans_g = ( color_g*(16-count) + dark_g*(count) ) /16;
        trans_b = ( color_b*(16-count) + dark_b*(count) ) /16;
        work->inside_ball_st_pal[INSIDE_BALL_PAL_TRANS][j] = GX_RGB( trans_r, trans_g, trans_b );
      }
      NNS_GfdRegisterNewVramTransferTask(
        NNS_GFD_DST_2D_BG_PLTT_SUB,
        BG_PAL_POS_S_INSIDE_BALL * 2*16,
        work->inside_ball_st_pal[INSIDE_BALL_PAL_TRANS],
        2*16*BG_PAL_NUM_S_INSIDE_BALL );
    }
  }
}

static void Psel_PalInit( PSEL_WORK* work )
{
  Psel_PokeSetPalInit( work );
  Psel_InsideBallPalInit( work );
}
static void Psel_PalExit( PSEL_WORK* work )
{
  Psel_PokeSetPalExit( work );
  Psel_InsideBallPalExit( work );
}
static void Psel_PalMain( PSEL_WORK* work )
{
  Psel_PokeSetPalMain( work );
  Psel_InsideBallPalMain( work );
}

static void Psel_PokeSetPalAnimeStart( PSEL_WORK* work, TARGET target_poke, POKE_PAL_ANIME_STATE state )  // 何回呼んでも、アニメの途中で呼んでも、大丈夫な頑強なつくりになっている
{
  switch( state )
  {
  case POKE_PAL_ANIME_STATE_COLOR:
    {
      if( work->poke_set[target_poke].pal_anime_state != POKE_PAL_ANIME_STATE_COLOR )
      {
        work->poke_set[target_poke].pal_anime_state = POKE_PAL_ANIME_STATE_DARK_TO_COLOR;
      }
    }
    break;
  case POKE_PAL_ANIME_STATE_DARK:
    {
      if( work->poke_set[target_poke].pal_anime_state != POKE_PAL_ANIME_STATE_DARK )
      {
        work->poke_set[target_poke].pal_anime_state = POKE_PAL_ANIME_STATE_COLOR_TO_DARK;
      }
    }
    break;
  }
}

static void Psel_InsideBallPalAnimeStart( PSEL_WORK* work, u8 end )  // 何回呼んでも、アニメの途中で呼んでも、大丈夫な頑強なつくりになっている
{
  if( work->inside_ball_pal_anime_end != end )
  {
    work->inside_ball_pal_anime_state = INSIDE_BALL_PAL_ANIME_STATE_START_TO_END;
    work->inside_ball_pal_anime_count = 0;
    work->inside_ball_pal_anime_end = end;

    // 現在の色をスタート色として覚えておく 
    GFL_STD_MemCopy( work->inside_ball_st_pal[INSIDE_BALL_PAL_TRANS], work->inside_ball_st_pal[INSIDE_BALL_PAL_START], 2*16*BG_PAL_NUM_S_INSIDE_BALL );
  }
}


//-------------------------------------
/// シーケンス
//=====================================
static int Psel_Start( PSEL_WORK* work, int* seq )
{
  return SEQ_LOAD;
}
static int Psel_Load( PSEL_WORK* work, int* seq )
{
  // グラフィック、フォントなど
  {
    work->graphic       = PSEL_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );
  }

  // メインBG
  GFL_BG_SetPriority( BG_FRAME_M_THREE,    BG_FRAME_PRI_M_THREE );
  GFL_BG_SetPriority( BG_FRAME_M_REAR,     BG_FRAME_PRI_M_REAR );
  GFL_BG_SetPriority( BG_FRAME_M_WIN,      BG_FRAME_PRI_M_WIN );
  GFL_BG_SetPriority( BG_FRAME_M_TEXT,     BG_FRAME_PRI_M_TEXT );
  
  // サブBG
  GFL_BG_SetPriority( BG_FRAME_S_INSIDE_BALL,     BG_FRAME_PRI_S_INSIDE_BALL );
  GFL_BG_SetPriority( BG_FRAME_S_WIN,             BG_FRAME_PRI_S_WIN );
  GFL_BG_SetPriority( BG_FRAME_S_TEXT,            BG_FRAME_PRI_S_TEXT );

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( Psel_VBlankFunc, work, 1 );
  work->vblank_req = VBLANK_REQ_NONE;

  // 3D全体
  Psel_ThreeInit( work );
  // BG
  Psel_BgInit( work ); 
  // テキスト
  Psel_TextInit( work );
  // APP_TASKMENU
  Psel_AppTaskmenuResInit( work );

  return SEQ_S01_INIT;
}

static int Psel_S01Init    ( PSEL_WORK* work, int* seq )
{
  // バックグラウンドカラー
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x2f3b );
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x2f3b );

  // 生成
  Psel_ThreeS01Init( work );
  Psel_BgS01Init( work );
  Psel_TextS01Init( work );

  // テキストは非表示
  work->vblank_req |= VBLANK_REQ_S01_TEXT_M_VISIBLE_OFF;

  // ブレンド
  work->ev1 = 0; 
  G2_SetBlendAlpha(
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2,
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
      work->ev1, 16-work->ev1 );
  // 3Dのアルファをきれいに出すため、第1対象面にBG0を指定している
  // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考

  // シーケンスフレーム
  work->sub_seq_frame = 0;
  return SEQ_S01_FADE_IN;
}
static int Psel_S01FadeIn  ( PSEL_WORK* work, int* seq )
{
  if( work->sub_seq_frame >= 1 ) Psel_ThreeS01Main( work );  // 3Dのアニメ開始を少し遅らせる
  Psel_TextS01Main( work );

  switch(*seq)
  {
  case 0:
    {
      // フェードイン(黒→見える)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 16, 0, S01_FADE_IN_WAIT );
      (*seq)++;
    }
    break;
  case 1:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // シーケンスフレーム
        work->sub_seq_frame = 0;
        return SEQ_S01_MAIN;
      }
    }
    break;
  }

  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S01_FADE_IN;
}
static int Psel_S01Main    ( PSEL_WORK* work, int* seq )
{
  BOOL blend_m = FALSE;

  Psel_TextS01Main( work );

  switch(*seq)
  {
  case 0:  // ～3Dのアニメが手紙を読む段階の4フレーム前になるまで
    {
      Psel_ThreeS01Main( work );
      if( work->three_frame == TIMETABLE_S01_CARD_READ -4 )
      {
        (*seq)++;
      }
    }
    break;
  case 1:  // ～3Dのアニメが手紙を読む段階になるまで
    {
      // ブレンド
      if( work->ev1 < 8 )
      {
        work->ev1++;
        blend_m = TRUE;
      }

      Psel_ThreeS01Main( work );
      if( work->three_frame == TIMETABLE_S01_CARD_READ )
      {
        (*seq)++;
      }
    }
    break;
  case 2:  // ～手紙のテキストを表示
    {
      // ブレンド
      if( work->ev1 < 8 )
      {
        work->ev1++; 
        blend_m = TRUE;
      }
      else
      {
        // テキストは表示
        work->vblank_req |= VBLANK_REQ_S01_TEXT_M_VISIBLE_ON;
        (*seq)++;
      }
    }
    break;
  case 3:  // 手紙のテキストが確実に表示されるよう1フレーム待つ
    {
      (*seq)++;
    }
    break;
  case 4:  // プレイヤーの入力待ち
    {
      if(    ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
          || ( GFL_UI_TP_GetTrg() ) )
      {
        if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ) work->ktst = GFL_APP_KTST_KEY;
        else                                                    work->ktst = GFL_APP_KTST_TOUCH;
        
        PMSND_PlaySE( PSELSND_DECIDE );
        
        (*seq)++;
      }
    }
    break;
  case 5:  // 手紙のテキストを非表示
    {
      // テキストは非表示
      work->vblank_req |= VBLANK_REQ_S01_TEXT_M_VISIBLE_OFF;

      // ブレンド
      if( work->ev1 != 0 )
      {
        work->ev1--; 
        blend_m = TRUE;
      }
      (*seq)++;
    }
    break;
  case 6:  // ～手紙のテキストのウィンドウの非表示が4フレーム進むまで
    {
      // ブレンド
      if( work->ev1 != 0 )
      {
        work->ev1--; 
        blend_m = TRUE;
      }

      if( work->ev1 == 4 )
      {
        (*seq)++;
      }
    }
    break;
  case 7:  // ～3Dのアニメが終了の8フレーム前になるまで(ここのフレームは S01_FADE_OUT_WAIT と関係あり)
    {
      // ブレンド
      if( work->ev1 != 0 )
      {
        work->ev1--;
        blend_m = TRUE;
      }

      Psel_ThreeS01Main( work );
      if( work->three_frame == TIMETABLE_S01_SCENE_END -8 )
      {
        // シーケンスフレーム
        work->sub_seq_frame = 0;
        return SEQ_S01_FADE_OUT;
      }
    }
    break;
  }

  // ブレンド
  if( blend_m )
  {
    G2_SetBlendAlpha(
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2,
        GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
        work->ev1, 16-work->ev1 );
    // 3Dのアルファをきれいに出すため、第1対象面にBG0を指定している
    // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考
  }
 
  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S01_MAIN;
}
static int Psel_S01FadeOut ( PSEL_WORK* work, int* seq )
{
  if( work->three_frame != TIMETABLE_S01_SCENE_END ) Psel_ThreeS01Main( work );  // 3Dのアニメを止める
  Psel_TextS01Main( work );

  switch(*seq)
  {
  case 0:
    {
      // フェードアウト(見える→白)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, S01_FADE_OUT_WAIT );
      (*seq)++;
    }
    break;
  case 1:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // シーケンスフレーム
        work->sub_seq_frame = 0;
        return SEQ_S01_EXIT;
      }
    }
    break;
  }

  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S01_FADE_OUT;
}
static int Psel_S01Exit    ( PSEL_WORK* work, int* seq )
{
  // ブレンド
  G2_BlendNone();
  G2S_BlendNone();

  Psel_TextS01Exit( work );
  Psel_BgS01Exit( work );
  Psel_ThreeS01Exit( work );

  // シーケンスフレーム
  work->sub_seq_frame = 0;
  return SEQ_S02_INIT;
}

static int Psel_S02Init    ( PSEL_WORK* work, int* seq )
{
  // 画面の上下入れ替え
  GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

  // バックグラウンドカラー
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, 0x2f3b );
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x2f3b );
 
  Psel_ThreeS02Init( work );
  Psel_PokeSetInit( work );
  Psel_FingerInit( work );
  Psel_BgS02Init( work );
  Psel_InsideBallInit( work );
  Psel_PalInit( work );

  // テキストとウィンドウは非表示
  work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_OFF;
  work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_OFF;

  // ブレンド
  G2_SetBlendAlpha(
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG2/* | GX_BLEND_PLANEMASK_BG3*/,
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
      8, 8 );
  // 3Dのアルファをきれいに出すため、第1対象面にBG0を指定している
  // ev1とev2は使われない  // TWLプログラミングマニュアル「2D面とのαブレンディング」参考

  G2S_SetBlendAlpha(
      GX_BLEND_PLANEMASK_BG1/* | GX_BLEND_PLANEMASK_BG2*/,
      GX_BLEND_PLANEMASK_BG0 | GX_BLEND_PLANEMASK_BG1 | GX_BLEND_PLANEMASK_BG2 | GX_BLEND_PLANEMASK_BG3 | GX_BLEND_PLANEMASK_OBJ | GX_BLEND_PLANEMASK_BD,
      8, 8 );

  // シーケンスフレーム
  work->sub_seq_frame = 0;
  return SEQ_S02_FADE_IN;
}
static int Psel_S02FadeIn  ( PSEL_WORK* work, int* seq )
{
  if( work->sub_seq_frame >= 16 ) Psel_ThreeS02Main( work );  // 3Dのアニメ開始を少し遅らせる
  Psel_PokeSetMain( work );
  Psel_PalMain( work );

  switch(*seq)
  {
  case 0:
    {
      // フェードイン(白→見える)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 16, 0, S02_FADE_IN_WAIT );
      (*seq)++;
    }
    break;
  case 1:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // シーケンスフレーム
        work->sub_seq_frame = 0;
        return SEQ_S02_MAIN;
      }
    }
    break;
  }

  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S02_FADE_IN;
}
static int Psel_S02Main    ( PSEL_WORK* work, int* seq )
{
  enum
  {
    S02_MAIN_SEQ_BOX_OPEN_STOP_WAIT,
    S02_MAIN_SEQ_EXPLAIN_INIT,
    S02_MAIN_SEQ_EXPLAIN_WAIT,
    S02_MAIN_SEQ_SELECT_INIT,
    S02_MAIN_SEQ_SELECT_WAIT,
    S02_MAIN_SEQ_TOUCH_CONFIRM_INIT,
    S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT,
    S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT_DECIDE,
    S02_MAIN_SEQ_ZOOM_IN_ANIME_INIT,
    S02_MAIN_SEQ_ZOOM_IN_ANIME_WAIT,
    S02_MAIN_SEQ_ANSWER_INIT,
    S02_MAIN_SEQ_ANSWER_WAIT,
    S02_MAIN_SEQ_ZOOM_OUT_ANIME_INIT,
    S02_MAIN_SEQ_ZOOM_OUT_ANIME_WAIT,
    S02_MAIN_SEQ_DECIDE_ANIME_INIT,
    S02_MAIN_SEQ_DECIDE_ANIME_WAIT,
    S02_MAIN_SEQ_END,
  };

  int hit = GFL_UI_TP_HIT_NONE;
  BOOL select_change = FALSE;  // 選択しているものが変わったらTRUE  // ポケモンのタイプと種族名を消すのに使用する変数


#ifdef DEBUG_POS_SET_MODE
  if( GFL_UI_KEY_GetCont() & PAD_BUTTON_L )
  {
#if 0
    // サブ画面のOBJを動かして位置を調整する
    GFL_CLACTPOS finger_pos;
    GFL_CLACT_WK_GetPos( work->finger_clwk, &finger_pos, CLSYS_DEFREND_MAIN );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      OS_Printf( "finger_pos = ( %d, %d )\n", finger_pos.x, finger_pos.y );
    }
    else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )    finger_pos.y -= 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )  finger_pos.y += 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )  finger_pos.x -= 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ) finger_pos.x += 1;
    }
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )    finger_pos.y -= 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )  finger_pos.y += 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )  finger_pos.x -= 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ) finger_pos.x += 1;
    
    GFL_CLACT_WK_SetPos( work->finger_clwk, &finger_pos, CLSYS_DEFREND_MAIN );
#else
    // メイン画面のOBJを動かして位置を調整する
    GFL_CLWK* small_clwk;
    GFL_CLWK* big_clwk;
    GFL_CLACTPOS pos;

    small_clwk = work->poke_set[TARGET_MIZU].clwk[POKE_SMALL];
    big_clwk = work->poke_set[TARGET_MIZU].clwk[POKE_BIG];

    GFL_CLACT_WK_GetPos( big_clwk, &pos, CLSYS_DEFREND_SUB );

    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_X )
    {
      OS_Printf( "main pos = ( %d, %d )\n", pos.x, pos.y );
    }
    else if( GFL_UI_KEY_GetCont() & PAD_BUTTON_R )
    {
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP )    pos.y -= 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )  pos.y += 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )  pos.x -= 1;
      if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT ) pos.x += 1;
    }
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_UP )    pos.y -= 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_DOWN )  pos.y += 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_LEFT )  pos.x -= 1;
    else if( GFL_UI_KEY_GetCont() & PAD_KEY_RIGHT ) pos.x += 1;
    
    GFL_CLACT_WK_SetPos( small_clwk, &pos, CLSYS_DEFREND_SUB );
    GFL_CLACT_WK_SetPos( big_clwk, &pos, CLSYS_DEFREND_SUB );
#endif
    return SEQ_S02_MAIN;
  }
#endif


  Psel_PokeSetMain( work );
  Psel_PalMain( work );

  switch(*seq)
  {
  case S02_MAIN_SEQ_BOX_OPEN_STOP_WAIT:
    {
      Psel_ThreeS02Main( work );
      if( work->three_frame == TIMETABLE_S02_BOX_STOP )  // ～ボックスが開くまで
      {
        *seq = S02_MAIN_SEQ_EXPLAIN_INIT;
      }
    }
    break;
  case S02_MAIN_SEQ_EXPLAIN_INIT:
    {
      // テキストとウィンドウは表示
      work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_ON;
      Psel_TextExplainStreamStart( work, msg_t01r0102_ball_select_01 );  // 何故かストリームが表示されない
      //Psel_TextExplainPrintStart( work, msg_t01r0102_ball_select_01 );
      
      *seq = S02_MAIN_SEQ_EXPLAIN_WAIT;
    }
    break;
  case S02_MAIN_SEQ_EXPLAIN_WAIT:
    {
      if( Psel_TextExplainStreamWait( work ) )  // 何故かストリームが表示されない
      //if( Psel_TextTransWait( work ) )
      {
        if(    ( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) )
            || ( GFL_UI_TP_GetTrg() ) )
        {
          if( GFL_UI_KEY_GetTrg() & (PAD_BUTTON_A|PAD_BUTTON_B) ) work->ktst = GFL_APP_KTST_KEY;
          else                                                    work->ktst = GFL_APP_KTST_TOUCH;

          PMSND_PlaySE( PSELSND_DECIDE );

          // テキストとウィンドウは非表示
          work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_OFF;
          Psel_TextExplainClear( work );

          *seq = S02_MAIN_SEQ_SELECT_INIT;
        }
      }
    }
    break;
  case S02_MAIN_SEQ_SELECT_INIT:
    {
      // キー操作中
      if( work->ktst == GFL_APP_KTST_KEY )
      {
        if( work->select_target_poke == TARGET_NONE )
        {
          work->select_target_poke = TARGET_POKE_START;
          select_change = TRUE;  // どれも選ばれていない状態から最初のを選んでいる状態になったのでTRUEにしたが、前がどれも選ばれていない状態なので何も書かれていないはずだから、TRUEにしなくても問題はない
        }
        Psel_FingerDrawEnable( work, TRUE );
        Psel_FingerUpdatePos( work );
        Psel_PokeSetSelectStart( work, work->select_target_poke );
        Psel_InsideBallPalAnimeStart( work, (work->select_target_poke!=TARGET_NONE)?(work->select_target_poke):(INSIDE_BALL_NONE) );
      }
      // タッチ操作中
      else
      {
        Psel_FingerDrawEnable( work, FALSE );

        // select_target_pokeは指指しカーソル表示用に位置を持っているかもしれないが、タッチなのでどれも選ばれていない状態で開始することにする
        Psel_PokeSetSelectStart( work, TARGET_NONE );
        Psel_InsideBallPalAnimeStart( work, INSIDE_BALL_NONE );
        select_change = TRUE;  // どれも選ばれていない状態なので、ポケモンのタイプと種族名は消していい
      }

      *seq = S02_MAIN_SEQ_SELECT_WAIT;
    }
    break;
  case S02_MAIN_SEQ_SELECT_WAIT:
    {
      // キー操作中
      if( work->ktst == GFL_APP_KTST_KEY )
      {
        if( GFL_UI_KEY_GetTrg() )  // キー操作のまま
        {
          BOOL move = FALSE;
          if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
          {
            PMSND_PlaySE( PSELSND_DECIDE );
            
            *seq = S02_MAIN_SEQ_ZOOM_IN_ANIME_INIT;
          }
          else if( GFL_UI_KEY_GetTrg() & PAD_KEY_LEFT )
          {
            if( work->select_target_poke > TARGET_POKE_START )
            {
              work->select_target_poke--;
              move = TRUE;
            }
          }
          else if( GFL_UI_KEY_GetTrg() & PAD_KEY_RIGHT )
          {
            if( work->select_target_poke < TARGET_POKE_END )
            {
              work->select_target_poke++;
              move = TRUE;
            }
          }
          if( move )
          {
            PMSND_PlaySE( PSELSND_MOVE );
            Psel_FingerUpdatePos( work );
            Psel_PokeSetSelectStart( work, work->select_target_poke );
            Psel_InsideBallPalAnimeStart( work, (work->select_target_poke!=TARGET_NONE)?(work->select_target_poke):(INSIDE_BALL_NONE) );
            select_change = TRUE;
          }
        }
        else if( GFL_UI_TP_GetTrg() )  // タッチ操作に切り替え(る可能性あり)
        {
          hit = GFL_UI_TP_HitTrg(target_tp_hittbl);
          if( TARGET_POKE_START<=hit && hit<=TARGET_POKE_END )
          {
            if( work->select_target_poke != hit )  // 選択しているものが変更されたとき
            {
              select_change = TRUE;
            } 
            PMSND_PlaySystemSE( PSELSND_MOVE );  // 選択しているものが変更されようがいまいが、タッチ操作に切り替わり次のシーケンスへ進むので、音を鳴らす
            Psel_FingerDrawEnable( work, FALSE );
            work->select_target_poke = hit;
            Psel_PokeSetSelectStart( work, work->select_target_poke );
            Psel_InsideBallPalAnimeStart( work, (work->select_target_poke!=TARGET_NONE)?(work->select_target_poke):(INSIDE_BALL_NONE) );
            work->ktst = GFL_APP_KTST_TOUCH;

            *seq = S02_MAIN_SEQ_TOUCH_CONFIRM_INIT;
          }
        }
      }
      // タッチ操作中
      else
      {
        if( GFL_UI_KEY_GetTrg() )  // キー操作に切り替え
        {
          work->ktst = GFL_APP_KTST_KEY;
          
          *seq = S02_MAIN_SEQ_SELECT_INIT;
        }
        else if( GFL_UI_TP_GetTrg() )  // タッチ操作のまま
        {
          hit = GFL_UI_TP_HitTrg(target_tp_hittbl);
          if( TARGET_POKE_START<=hit && hit<=TARGET_POKE_END )
          {
            PMSND_PlaySystemSE( PSELSND_MOVE );
            work->select_target_poke = hit;
            Psel_PokeSetSelectStart( work, work->select_target_poke );
            Psel_InsideBallPalAnimeStart( work, (work->select_target_poke!=TARGET_NONE)?(work->select_target_poke):(INSIDE_BALL_NONE) );
            select_change = TRUE;

            *seq = S02_MAIN_SEQ_TOUCH_CONFIRM_INIT;
          }
        }
      }
    }
    break;
  case S02_MAIN_SEQ_TOUCH_CONFIRM_INIT:
    {
      Psel_AppTaskmenuWinInit( work );
      // テキストは表示
      work->vblank_req |= VBLANK_REQ_S02_TEXT_M_VISIBLE_ON;

      *seq = S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT;
    }
    break;
  case S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT:
    {
      Psel_AppTaskmenuWinMain( work );
      if( GFL_UI_KEY_GetTrg() )  // キー操作に切り替え
      {
        work->ktst = GFL_APP_KTST_KEY;
        
        if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A )
        {
          PMSND_PlaySystemSE( PSELSND_DECIDE );
          Psel_AppTaskmenuWinDecideStart( work );

          *seq = S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT_DECIDE;
        }
        else
        {
          Psel_AppTaskmenuWinExit( work );
          // テキストは非表示
          work->vblank_req |= VBLANK_REQ_S02_TEXT_M_VISIBLE_OFF;
          Psel_TextExplainClear( work );

          *seq = S02_MAIN_SEQ_SELECT_INIT;
        }
      }
      else if( GFL_UI_TP_GetTrg() )  // タッチ操作のまま
      {
        hit = GFL_UI_TP_HitTrg(target_tp_hittbl);
        if( hit == TARGET_BUTTON )
        {
          PMSND_PlaySystemSE( PSELSND_DECIDE );
          Psel_AppTaskmenuWinDecideStart( work );
          
          *seq = S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT_DECIDE;
        }
        else if( TARGET_POKE_START<=hit && hit<=TARGET_POKE_END )
        {
          if( work->select_target_poke != hit )  // 選んでいるものが変わったら
          {
            PMSND_PlaySystemSE( PSELSND_MOVE );
            work->select_target_poke = hit;
            Psel_PokeSetSelectStart( work, work->select_target_poke );
            Psel_InsideBallPalAnimeStart( work, (work->select_target_poke!=TARGET_NONE)?(work->select_target_poke):(INSIDE_BALL_NONE) );
            select_change = TRUE;
          }
        }
      }
    }
    break;
  case S02_MAIN_SEQ_TOUCH_CONFIRM_WAIT_DECIDE:
    {
      Psel_AppTaskmenuWinMain( work );
      if( Psel_AppTaskmenuWinDecideIsEnd( work ) )
      {
        Psel_AppTaskmenuWinExit( work );
        // テキストは非表示
        work->vblank_req |= VBLANK_REQ_S02_TEXT_M_VISIBLE_OFF;
        Psel_TextExplainClear( work );

        *seq = S02_MAIN_SEQ_ZOOM_IN_ANIME_INIT;
      }
    }
    break;
  case S02_MAIN_SEQ_ZOOM_IN_ANIME_INIT:
    {
      // テキストとウィンドウは表示
      work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_ON;
      Psel_TextExplainStreamStart( work, msg_t01r0102_ball_select_02 );  // 何故かストリームが表示されない
      //Psel_TextExplainPrintStart( work, msg_t01r0102_ball_select_02 );

      // ズームイン
      Psel_ThreeS02MbZoomAnimeStart( work, work->select_target_poke, FALSE );

      *seq = S02_MAIN_SEQ_ZOOM_IN_ANIME_WAIT;
    }
    break;
  case S02_MAIN_SEQ_ZOOM_IN_ANIME_WAIT:
    {
      // ズーム
      Psel_ThreeS02MbZoomAnimeMain( work );
      
      if( Psel_TextExplainStreamWait( work ) )  // 何故かストリームが表示されない
      //if( Psel_TextTransWait( work ) )
      {
        // ズーム
        if( Psel_ThreeS02MbZoomAnimeIsEnd( work ) )
        {
          *seq = S02_MAIN_SEQ_ANSWER_INIT;
        }
      }
    }
    break;
  case S02_MAIN_SEQ_ANSWER_INIT:
    {
      // ズーム中
      Psel_ThreeS02MbAnimeOnlyMbMain( work );
      
      // APP_TASKMENUを開く前に設定しておく
      // タッチorキー
      GFL_UI_SetTouchOrKey( work->ktst );

      Psel_AppTaskmenuInit( work );

      *seq = S02_MAIN_SEQ_ANSWER_WAIT;
    }
    break;
  case S02_MAIN_SEQ_ANSWER_WAIT:
    {
      // ズーム中
      Psel_ThreeS02MbAnimeOnlyMbMain( work );

      Psel_AppTaskmenuMain( work );
      if( Psel_AppTaskmenuIsEnd( work ) )
      {
        BOOL yes = Psel_AppTaskmenuIsYes( work );
        Psel_AppTaskmenuExit( work );

        // APP_TASKMENUを閉じた後に設定しておく
        // タッチorキー
        work->ktst = GFL_UI_CheckTouchOrKey();
     
        if( yes )
        {
          *seq = S02_MAIN_SEQ_DECIDE_ANIME_INIT;
        }
        else
        {
          *seq = S02_MAIN_SEQ_ZOOM_OUT_ANIME_INIT;
        }

        // テキストとウィンドウは非表示
        work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_M_VISIBLE_OFF;
        Psel_TextExplainClear( work );
      }
    }
    break;
  case S02_MAIN_SEQ_ZOOM_OUT_ANIME_INIT:
    {
      // ズームアウト
      Psel_ThreeS02MbZoomAnimeStart( work, work->select_target_poke, TRUE );
      
      *seq = S02_MAIN_SEQ_ZOOM_OUT_ANIME_WAIT;
    }
    break;
  case S02_MAIN_SEQ_ZOOM_OUT_ANIME_WAIT:
    {
      // ズーム
      Psel_ThreeS02MbZoomAnimeMain( work );
      if( Psel_ThreeS02MbZoomAnimeIsEnd( work ) )
      {
        *seq = S02_MAIN_SEQ_SELECT_INIT;
      } 
    }
    break;
  case S02_MAIN_SEQ_DECIDE_ANIME_INIT:
    {
      // 決定アニメ
      Psel_ThreeS02MbDecideAnimeStart( work, work->select_target_poke );
      
      *seq = S02_MAIN_SEQ_DECIDE_ANIME_WAIT;
    }
    break;
  case S02_MAIN_SEQ_DECIDE_ANIME_WAIT:
    {
      // 決定アニメ
      Psel_ThreeS02MbDecideAnimeMain( work );

      *seq = S02_MAIN_SEQ_END;
    }
    break;
  case S02_MAIN_SEQ_END:
    {
      // 決定アニメ
      Psel_ThreeS02MbDecideAnimeMain( work );

      // シーケンスフレーム
      work->sub_seq_frame = 0;
      return SEQ_S02_FADE_OUT;
    }
    break;
  }

  // ポケモンのタイプと種族名を書くor消す
  if( select_change )  // 消す
  {
    // テキストとウィンドウは非表示
    work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_OFF;
    Psel_TextPokeInfoClear( work );
  }
  else if( work->poke_info_print )  // 書く
  {
    // テキストとウィンドウは表示
    work->vblank_req |= VBLANK_REQ_S02_TEXT_WIN_S_VISIBLE_ON;
    Psel_TextPokeInfoPrintStart( work, work->select_target_poke );
  }
  work->poke_info_print = FALSE;

  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S02_MAIN;
}
static int Psel_S02FadeOut ( PSEL_WORK* work, int* seq )
{
  // 決定アニメ
  Psel_ThreeS02MbDecideAnimeMain( work );

  Psel_PokeSetMain( work );
  Psel_PalMain( work );

  switch(*seq)
  {
  case 0:
    {
      // フェードアウト(見える→白)
      GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_WHITEOUT, 0, 16, S02_FADE_OUT_WAIT );
      (*seq)++;
    }
    break;
  case 1:
    {
      if( !GFL_FADE_CheckFade() )
      {
        // シーケンスフレーム
        work->sub_seq_frame = 0;
        return SEQ_S02_EXIT;
      }
    }
    break;
  }

  // シーケンスフレーム
  work->sub_seq_frame++;
  return SEQ_S02_FADE_OUT;
}
static int Psel_S02Exit    ( PSEL_WORK* work, int* seq )
{
  // ブレンド
  G2_BlendNone();
  G2S_BlendNone();

  Psel_PalExit( work );
  Psel_InsideBallExit( work );
  Psel_BgS02Exit( work );
  Psel_FingerExit( work );
  Psel_PokeSetExit( work );
  Psel_ThreeS02Exit( work );

  // シーケンスフレーム
  work->sub_seq_frame = 0;
  return SEQ_UNLOAD;
}

static int Psel_Unload( PSEL_WORK* work, int* seq )
{
  // APP_TASKMENU
  Psel_AppTaskmenuResExit( work );
  // テキスト
  Psel_TextExit( work ); 
  // BG
  Psel_BgExit( work ); 
  // 3D全体
  Psel_ThreeExit( work );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィック、フォントなど
  {
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
    PSEL_GRAPHIC_Exit( work->graphic );
  }

  return SEQ_END;
}
static int Psel_End( PSEL_WORK* work, int* seq )
{
  return SEQ_END;
}

