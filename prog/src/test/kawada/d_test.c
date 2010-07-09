//============================================================================
/**
 *  @file   d_test.c
 *  @brief  テスト
 *  @author Koji Kawada
 *  @data   2010.03.31
 *  @note   
 *  モジュール名：D_KAWADA_TEST, D_TEST
 */
//============================================================================
#define DEBUG_KAWADA  // これが定義されているとき、デバッグ出力があったりする


// インクルード
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"

#include "system/mcss.h"
#include "system/mcss_tool.h"
#include "poke_tool/monsno_def.h"

#include "print/gf_font.h"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_msg.h"

#include "../../demo/shinka_demo/shinka_demo_graphic.h"
#include "../../demo/egg_demo/egg_demo_graphic.h"
#include "../../app/zukan/detail/zukan_detail_graphic.h"
#include "d_test.h"

// アーカイブ
#include "arc_def.h"
#include "font/font.naix"
#include "debug_message.naix"
#include "msg/debug/msg_d_kawada.h"

// サウンド

// オーバーレイ
FS_EXTERN_OVERLAY(shinka_demo);
FS_EXTERN_OVERLAY(egg_demo);
FS_EXTERN_OVERLAY(zukan_detail);

#ifdef PM_DEBUG

//=============================================================================
/**
*  定数定義
*/
//=============================================================================

// メインBGフレーム
#define BG_FRAME_M_3D      (GFL_BG_FRAME0_M)

// サブBGフレーム
#define BG_FRAME_S_TEXT    (GFL_BG_FRAME0_S)

// サブBGパレット
// 本数
enum
{
  BG_PAL_NUM_S_FONT_DEFAULT  = 1,
};
// 位置
enum
{
  BG_PAL_POS_S_FONT_DEFAULT  = 0,
};




//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
/**
*  進化デモカメラとタマゴ孵化デモカメラで共通使用できる事柄
*/
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================


//=============================================================================
/**
*  定数定義
*/
//=============================================================================

// mons_noからgra_noを得るためのテーブル
static const u16 d_test_cmn_mons_no_to_gra_no_tbl[MONSNO_END +1] =
{
  #include "d_test_mons_no_to_gra_no.h" 
};
/*
d_test_mons_no_to_gra_no.h
は
pokemon_wb/prog/include/poke_tool/monsnum_def.h
の
MONSNO_???
の
???
を取り出して手づくりしたファイル。

gra_no = d_test_cmn_mons_no_to_gra_no_tbl[mons_no];
というふうに使用できる。

0<=mons_no<=MONSNO_END

0
MONSNO_HUSIGIDANE
  .
  .
  .
MONSNO_INSEKUTA
*/

// gra_no順にmons_noを並べたテーブル
static u16 d_test_cmn_mons_no_tbl_gra_no_order[MONSNO_END +1];
/*
一番若いgra_noのmons_noは
mons_no = d_test_cmn_mons_no_tbl_gra_no_order[1];
というふうに使用する。

d_test_cmn_mons_no_tbl_gra_no_order[0]は使用しないこと。
*/


// バックグラウンドカラー
typedef enum
{
  D_TEST_CMN_BG_COLOR_ENUM_BLACK,
  D_TEST_CMN_BG_COLOR_ENUM_LBLACK,
  D_TEST_CMN_BG_COLOR_ENUM_GRAY,
  D_TEST_CMN_BG_COLOR_ENUM_DWHITE,
  D_TEST_CMN_BG_COLOR_ENUM_WHITE,
  D_TEST_CMN_BG_COLOR_ENUM_RED,
  D_TEST_CMN_BG_COLOR_ENUM_GREEN,
  D_TEST_CMN_BG_COLOR_ENUM_BLUE,
  D_TEST_CMN_BG_COLOR_ENUM_MAX,
}
D_TEST_CMN_BG_COLOR_ENUM;

static const u16 d_test_cmn_bg_color_enum[D_TEST_CMN_BG_COLOR_ENUM_MAX] =
{
  0x0000,
  0x2108,
  0x4210,
  0x6318,
  0x7fff,
  0x0f3e,
  0x2790,
  0x7b10,
};

// テキスト
enum
{
  D_TEST_CMN_TEXT_DUMMY,
  
  D_TEST_CMN_TEXT_LABEL_POKE_NAME,
  D_TEST_CMN_TEXT_LABEL_POKE_MONS_NO,
  D_TEST_CMN_TEXT_LABEL_POKE_GRA_NO,
  D_TEST_CMN_TEXT_LABEL_POKE_FORM_NO,
  D_TEST_CMN_TEXT_LABEL_POKE_SEX,
  D_TEST_CMN_TEXT_LABEL_POKE_COLOR,
  
  D_TEST_CMN_TEXT_POKE_NAME,
  D_TEST_CMN_TEXT_POKE_MONS_NO,
  D_TEST_CMN_TEXT_POKE_GRA_NO,
  D_TEST_CMN_TEXT_POKE_FORM_NO,
  D_TEST_CMN_TEXT_POKE_SEX,
  D_TEST_CMN_TEXT_POKE_COLOR,

  D_TEST_CMN_TEXT_MAX,
};

static const u8 d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_MAX][9] =
{
  // frmnum           posx  posy  sizx  sizy  palnum                     dir                    x  y (x,yは無視してセンタリングすることもある)
  {  BG_FRAME_S_TEXT,    4,    2,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  
  {  BG_FRAME_S_TEXT,    4,    2,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,    4,    4,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,    4,    6,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,    4,    8,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,    4,   10,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,    4,   12,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  
  {  BG_FRAME_S_TEXT,   18,    2,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,   18,    4,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,   18,    6,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,   18,    8,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,   18,   10,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
  {  BG_FRAME_S_TEXT,   18,   12,   12,    2, BG_PAL_POS_S_FONT_DEFAULT, GFL_BMP_CHRAREA_GET_F, 0, 0 },
};

#define D_TEST_CMN_STRBUF_LEN (64)  // 文字数(これだけあれば足りるでしょう)

// オフセットを設定する関数
typedef void (D_Test_CmnMcssSetOfsPositionFunc)(void* d_test_cmn_wk);  // d_test_cmn_wkは(D_TEST_COMMON_WORK*)にキャストして使用する


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                             heap_id;

  // フォントなど
  GFL_FONT*                          font;
  PRINT_QUE*                         print_que;

  // バックグラウンドカラー
  D_TEST_CMN_BG_COLOR_ENUM           bg_color_enum;

  // テキスト
  GFL_MSGDATA*                       msgdata;
  GFL_BMPWIN*                        text_bmpwin[D_TEST_CMN_TEXT_MAX];
  BOOL                               text_trans[D_TEST_CMN_TEXT_MAX];  // TRUEのとき転送する必要あり

  // MCSS
  MCSS_SYS_WORK*                     mcss_sys_wk;
  MCSS_WORK*                         mcss_wk;

  VecFx32                            mcss_pos;
  VecFx32                            mcss_scale;
  D_Test_CmnMcssSetOfsPositionFunc*  mcss_set_ofs_position_func;

  // 状態
  BOOL                               order_mons_no;   // mons_no順のときTRUE、gra_no順のときFALSE
  BOOL                               mcss_anm_play;   // TRUEのときアニメ再生する、FALSEのときアニメ再生しない
  BOOL                               mcss_dir_front;  // TRUEのとき前向き、FALSEのとき後向き

  // ポケモン
  int                                mons_no;
  int                                form_no;
  int                                sex;
  int                                rare;
  BOOL                               egg;
  int                                dir;
  u32                                personal_rnd;  // personal_rndはmons_no==MONSNO_PATTIIRUのときのみ使用される
}
D_TEST_COMMON_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// 共通使用できる事柄
static D_TEST_COMMON_WORK* D_Test_CmnInit( HEAPID heap_id );
static void D_Test_CmnExit( D_TEST_COMMON_WORK* work );
static void D_Test_CmnMain( D_TEST_COMMON_WORK* work );
static void D_Test_CmnDraw3D( D_TEST_COMMON_WORK* work );
static BOOL D_Test_CmnInput( D_TEST_COMMON_WORK* work );  // 終了入力があったときTRUE、継続するときFALSEを返す

static void D_Test_CmnTextInit( D_TEST_COMMON_WORK* work );
static void D_Test_CmnTextExit( D_TEST_COMMON_WORK* work );
static void D_Test_CmnTextMain( D_TEST_COMMON_WORK* work );


// gra_no順にmons_noを並べたテーブルをつくる
static void D_Test_CmnMakeMonsNoTblGraNoOrder(void);

// 今のmons_noを渡し次のmons_noを得る。その順番はorder_mons_noによるがgra_no順だとしてもgra_noを返すことはない。
static u16 D_Test_CmnGetNextMonsNo( u16 mons_no_curr, BOOL order_mons_no );  // mons_no順のときTRUE、gra_no順のときFALSE  // (MONSNO_END+1)を返したとき次なし
static u16 D_Test_CmnGetPrevMonsNo( u16 mons_no_curr, BOOL order_mons_no );  // mons_no順のときTRUE、gra_no順のときFALSE  // (0)を返したとき前なし

static u16 D_Test_CmnGetFirstMonsNo( BOOL order_mons_no );  // mons_no順のときTRUE、gra_no順のときFALSE
static u16 D_Test_CmnGetLastMonsNo( BOOL order_mons_no );  // mons_no順のときTRUE、gra_no順のときFALSE


static void D_Test_CmnBgColorEnumSetNext(
    D_TEST_COMMON_WORK*     work );

static void D_Test_CmnMcssSet(
    D_TEST_COMMON_WORK*                work,
    const VecFx32*                     mcss_pos,
    const VecFx32*                     mcss_scale,
    D_Test_CmnMcssSetOfsPositionFunc*  mcss_set_ofs_position_func );

// ポケモン切り替え
static void D_Test_CmnPokeChange(
    D_TEST_COMMON_WORK* work,
    int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
    u32 personal_rnd );
static void D_Test_CmnPokeChangeOnlyData(
    D_TEST_COMMON_WORK* work,
    int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
    u32 personal_rnd );
static void D_Test_CmnPokeUpdateText( D_TEST_COMMON_WORK* work );
static void D_Test_CmnPokeAnmFlip( D_TEST_COMMON_WORK* work );
static void D_Test_CmnPokeDirFlip( D_TEST_COMMON_WORK* work );

// 次のフォルム情報を得る(今のポケモンの次のフォルムがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
static BOOL D_Test_CmnPokeGetNextForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値
// 前のフォルム情報を得る(今のポケモンの前のフォルムがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
static BOOL D_Test_CmnPokeGetPrevForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値

// 次のポケモン情報を得る(次のポケモンがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
static BOOL D_Test_CmnPokeGetNextPoke(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値
// 前のポケモン情報を得る(前のポケモンがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
static BOOL D_Test_CmnPokeGetPrevPoke(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値
// 前のポケモンの最後のフォルム情報を得る(前のポケモンないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
static BOOL D_Test_CmnPokeGetPrevPokeLastForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値

// 最初のポケモンの最初のフォルム情報を得る
static void D_Test_CmnPokeGetFirstPokeFirstForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値
// 最後のポケモンの最後のフォルム情報を得る
static void D_Test_CmnPokeGetLastPokeLastForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値
// 最後のポケモンの最初のフォルム情報を得る
static void D_Test_CmnPokeGetLastPokeFirstForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd );                                                   // 引き数戻り値

// ポケモンパーソナルデータの情報から、フォルム数と性別ベクトルを得る
static void D_Test_CmnPokeGetPersonalData(
    D_TEST_COMMON_WORK* work,
    u16 mons_no,
    u32* ppd_form_max, u32* ppd_sex );  // 引き数戻り値


//=============================================================================
/**
*  ローカル関数定義
*/
//=============================================================================
//-------------------------------------
/// 共通使用できる事柄
//=====================================
static D_TEST_COMMON_WORK* D_Test_CmnInit( HEAPID heap_id )
{
  // ヒープ、パラメータなど
  D_TEST_COMMON_WORK* work = GFL_HEAP_AllocClearMemory( heap_id, sizeof(D_TEST_COMMON_WORK) );
  work->heap_id = heap_id;

  // フォントなど
  {
    work->font          = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr, GFL_FONT_LOADTYPE_FILE, FALSE, work->heap_id );
    work->print_que     = PRINTSYS_QUE_Create( work->heap_id );

    // パレット
    GFL_ARC_UTIL_TransVramPaletteEx(
        ARCID_FONT,
        NARC_font_default_nclr,
        PALTYPE_SUB_BG,
        0,
        BG_PAL_POS_S_FONT_DEFAULT * 0x20,
        BG_PAL_NUM_S_FONT_DEFAULT * 0x20,
        work->heap_id );
  }

  // バックグラウンドカラー
  work->bg_color_enum = D_TEST_CMN_BG_COLOR_ENUM_BLACK;
  D_Test_CmnBgColorEnumSetNext( work );

  // MCSS
  work->mcss_sys_wk = MCSS_Init( 1, work->heap_id );
  MCSS_SetTextureTransAdrs( work->mcss_sys_wk, 0 );  // 他に一緒に出す3Dはないので
  MCSS_SetOrthoMode( work->mcss_sys_wk );
  MCSS_OpenHandle( work->mcss_sys_wk, POKEGRA_GetArcID() );

  // 0, NULL初期化
  {
    VecFx32  mcss_pos    = { 0, 0, 0 };
    VecFx32  mcss_scale  = { 0, 0, 0 };

    work->mcss_wk = NULL;

    D_Test_CmnMcssSet(
      work,
      &mcss_pos,
      &mcss_scale,
      NULL );
  }

  // 状態
  work->order_mons_no  = FALSE;//TRUE;
  work->mcss_anm_play  = TRUE;
  work->mcss_dir_front = TRUE;

  // テキスト
  D_Test_CmnTextInit( work );

  // gra_no順にmons_noを並べたテーブルをつくる
  D_Test_CmnMakeMonsNoTblGraNoOrder();

  return work;
}
static void D_Test_CmnExit( D_TEST_COMMON_WORK* work )
{
  // テキスト
  D_Test_CmnTextExit( work );

  // MCSS
  if( work->mcss_wk )
  {
    MCSS_SetVanishFlag( work->mcss_wk );
    MCSS_Del( work->mcss_sys_wk, work->mcss_wk );
  }
 
  MCSS_CloseHandle( work->mcss_sys_wk );
  MCSS_Exit( work->mcss_sys_wk );

  // フォントなど
  {
    PRINTSYS_QUE_Clear( work->print_que );
    PRINTSYS_QUE_Delete( work->print_que );
    GFL_FONT_Delete( work->font );
  }

  // ヒープ、パラメータなど
  GFL_HEAP_FreeMemory( work );
}
static void D_Test_CmnMain( D_TEST_COMMON_WORK* work )
{
  D_Test_CmnTextMain( work );

  PRINTSYS_QUE_Main( work->print_que );

  // MCSS
  MCSS_Main( work->mcss_sys_wk );
}
static void D_Test_CmnDraw3D( D_TEST_COMMON_WORK* work )
{
  // MCSS
  MCSS_Draw( work->mcss_sys_wk );
}
static BOOL D_Test_CmnInput( D_TEST_COMMON_WORK* work )  // 終了入力があったときTRUE、継続するときFALSEを返す
{
  int trg    = GFL_UI_KEY_GetTrg();
  int repeat = GFL_UI_KEY_GetRepeat();

  int  mons_no;
  int  form_no;
  int  sex;
  int  rare;
  BOOL egg;
  int  dir;
  u32  personal_rnd;

  BOOL ret = FALSE;

  u8 i;

  // 次のフォルム
  if( repeat & PAD_KEY_DOWN )
  {
    ret = D_Test_CmnPokeGetNextForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    if( !ret )
    {
      ret = D_Test_CmnPokeGetNextPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( !ret ) D_Test_CmnPokeGetFirstPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    }
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }
  else if( repeat & PAD_KEY_UP )
  {
    ret = D_Test_CmnPokeGetPrevForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    if( !ret )
    {
      ret = D_Test_CmnPokeGetPrevPokeLastForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( !ret ) D_Test_CmnPokeGetLastPokeLastForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    }
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // 次のポケモン
  else if( repeat & PAD_KEY_RIGHT )
  {
    ret = D_Test_CmnPokeGetNextPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    if( !ret ) D_Test_CmnPokeGetFirstPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }
  else if( repeat & PAD_KEY_LEFT )
  {
    ret = D_Test_CmnPokeGetPrevPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    if( !ret ) D_Test_CmnPokeGetLastPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // 10個先のポケモン
  else if( repeat & PAD_BUTTON_R )
  {
    for( i=0; i<10; i++ )
    {
      ret = D_Test_CmnPokeGetNextPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( ret )
      {
        D_Test_CmnPokeChangeOnlyData( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
      }
      else
      {
        D_Test_CmnPokeGetFirstPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
        break;
      }
    } 
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }
  else if( repeat & PAD_BUTTON_L )
  {
    for( i=0; i<10; i++ )
    {
      ret = D_Test_CmnPokeGetPrevPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( ret )
      {
        D_Test_CmnPokeChangeOnlyData( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
      }
      else
      {
        D_Test_CmnPokeGetLastPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
        break;
      }
    }
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // 50個先のポケモン
  else if( repeat & PAD_BUTTON_A )
  {
    for( i=0; i<50; i++ )
    {
      ret = D_Test_CmnPokeGetNextPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( ret )
      {
        D_Test_CmnPokeChangeOnlyData( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
      }
      else
      {
        D_Test_CmnPokeGetFirstPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
        break;
      }
    } 
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }
  else if( repeat & PAD_BUTTON_Y )
  {
    for( i=0; i<50; i++ )
    {
      ret = D_Test_CmnPokeGetPrevPoke( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
      if( ret )
      {
        D_Test_CmnPokeChangeOnlyData( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
      }
      else
      {
        D_Test_CmnPokeGetLastPokeFirstForm( work, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
        break;
      }
    }
    D_Test_CmnPokeChange( work, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // アニメーションの再生、停止
  else if( trg & PAD_BUTTON_X )
  {
    D_Test_CmnPokeAnmFlip( work );
  }

  // 前後の変更
  else if( trg & PAD_BUTTON_START )
  {
    D_Test_CmnPokeDirFlip( work );
  }

  // バックグラウンドカラーの変更
  else if( trg & PAD_BUTTON_SELECT )
  {
    D_Test_CmnBgColorEnumSetNext( work );
  }

  // 終了
  else if( trg & PAD_BUTTON_B )
  {
    return TRUE;
  }

  return FALSE;
}

static void D_Test_CmnTextInit( D_TEST_COMMON_WORK* work )
{
  u8 i;

  // バックグラウンドカラー
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_S, 0x7fff );

  // メッセージ
  work->msgdata = GFL_MSG_Create(
                      GFL_MSG_LOAD_NORMAL,
                      ARCID_DEBUG_MESSAGE,
                      NARC_debug_message_d_kawada_dat,
                      work->heap_id );

  // ビットマップウィンドウ
  for( i=0; i<D_TEST_CMN_TEXT_MAX; i++ )
  {
    work->text_bmpwin[i] = GFL_BMPWIN_Create(
                               d_test_cmn_bmpwin_setup[i][0],
                               d_test_cmn_bmpwin_setup[i][1], d_test_cmn_bmpwin_setup[i][2], d_test_cmn_bmpwin_setup[i][3], d_test_cmn_bmpwin_setup[i][4],
                               d_test_cmn_bmpwin_setup[i][5], d_test_cmn_bmpwin_setup[i][6] );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[i]), 0 );
    GFL_BMPWIN_TransVramCharacter( work->text_bmpwin[i] );

    work->text_trans[i] = FALSE;
  }

  // 固定テキスト
  {
    u8 i;
    for( i=D_TEST_CMN_TEXT_LABEL_POKE_NAME; i<=D_TEST_CMN_TEXT_LABEL_POKE_COLOR; i++ )
    {
      STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata, label_poke_name +i -D_TEST_CMN_TEXT_LABEL_POKE_NAME );
      PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[i] ),
                              d_test_cmn_bmpwin_setup[i][7], d_test_cmn_bmpwin_setup[i][8],
                              src_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
      GFL_STR_DeleteBuffer( src_strbuf );
      work->text_trans[i] = TRUE;
    }
    D_Test_CmnTextMain( work );
  }
}
static void D_Test_CmnTextExit( D_TEST_COMMON_WORK* work )
{
  u8 i;
 
  // print_que
  PRINTSYS_QUE_Clear( work->print_que );

  // ビットマップウィンドウ
  for( i=0; i<D_TEST_CMN_TEXT_MAX; i++ )
  {
    GFL_BMPWIN_Delete( work->text_bmpwin[i] );
    work->text_trans[i] = FALSE;
  }

  // メッセージ
  GFL_MSG_Delete( work->msgdata );
}
static void D_Test_CmnTextMain( D_TEST_COMMON_WORK* work )
{
  u8 i;

  for( i=0; i<D_TEST_CMN_TEXT_MAX; i++ )
  {
    if( work->text_trans[i] )
    {
      if( !PRINTSYS_QUE_IsExistTarget( work->print_que, GFL_BMPWIN_GetBmp(work->text_bmpwin[i]) ) )
      {
        GFL_BMPWIN_MakeTransWindow_VBlank( work->text_bmpwin[i] );
        work->text_trans[i] = FALSE;
      }
    }
  }
}

// gra_no順にmons_noを並べたテーブルをつくる
static void D_Test_CmnMakeMonsNoTblGraNoOrder(void)
{
  // バブルソート
  u16 mons_no_tbl[MONSNO_END +1];
  s16 i, j;

  // 初期並び
  for( i=0; i<=MONSNO_END; i++ )
  {
    mons_no_tbl[i] = i;
  }

  // バブルソート実行
  for( i=MONSNO_END; i>=1; i-- )
  {
    for( j=i-1; j>=0; j-- )
    {
      if( d_test_cmn_mons_no_to_gra_no_tbl[ mons_no_tbl[i] ] < d_test_cmn_mons_no_to_gra_no_tbl[ mons_no_tbl[j] ] )
      {
        u16 no = mons_no_tbl[i];
        mons_no_tbl[i] = mons_no_tbl[j];
        mons_no_tbl[j] = no;
      }
    }
  }

  // 結果
  for( i=0; i<=MONSNO_END; i++ )
  {
    d_test_cmn_mons_no_tbl_gra_no_order[i] = mons_no_tbl[i];
    //OS_Printf( "[%3d] mons_no=%3d, gra_no=%3d\n",
    //    i,
    //    d_test_cmn_mons_no_tbl_gra_no_order[i],
    //    d_test_cmn_mons_no_to_gra_no_tbl[ d_test_cmn_mons_no_tbl_gra_no_order[i] ] );
  }
}

// 今のmons_noを渡し次のmons_noを得る。その順番はorder_mons_noによるがgra_no順だとしてもgra_noを返すことはない。
static u16 D_Test_CmnGetNextMonsNo( u16 mons_no_curr, BOOL order_mons_no )  // mons_no順のときTRUE、gra_no順のときFALSE  // (MONSNO_END+1)を返したとき次なし
{
  if( mons_no_curr <= 0 || MONSNO_END < mons_no_curr )
  {
    return (MONSNO_END+1);
  }
  
  if( order_mons_no )
  {
    return (mons_no_curr+1);
  }
  else
  {
    s16 i;

    for( i=0; i<=MONSNO_END; i++ )
    {
      if( d_test_cmn_mons_no_tbl_gra_no_order[i] == mons_no_curr )
      {
        break;
      }
    }

    if( i<=0 || MONSNO_END<=i )  // i==MONSNO_ENDのときは次はMONSNO_END+1なのでなし
    {
      return (MONSNO_END+1);
    }
    else
    {
      return d_test_cmn_mons_no_tbl_gra_no_order[i+1];
    }
  }
}
static u16 D_Test_CmnGetPrevMonsNo( u16 mons_no_curr, BOOL order_mons_no )  // mons_no順のときTRUE、gra_no順のときFALSE  // (0)を返したとき前なし
{ 
  if( mons_no_curr <= 0 || MONSNO_END < mons_no_curr )
  {
    return (0);
  }

  if( order_mons_no )
  {
    return (mons_no_curr-1);
  }
  else
  {
    s16 i;
    for( i=0; i<=MONSNO_END; i++ )
    {
      if( d_test_cmn_mons_no_tbl_gra_no_order[i] == mons_no_curr )
      {
        break;
      }
    }

    if( i<=1 || MONSNO_END<i )  // i==1のときは前は0なのでなし
    {
      return (0);
    }
    else
    {
      return d_test_cmn_mons_no_tbl_gra_no_order[i-1];
    }
  }
}

static u16 D_Test_CmnGetFirstMonsNo( BOOL order_mons_no )  // mons_no順のときTRUE、gra_no順のときFALSE
{
  if( order_mons_no )
  {
    return MONSNO_HUSIGIDANE;
  }
  else
  {
    return d_test_cmn_mons_no_tbl_gra_no_order[1];
  }
}
static u16 D_Test_CmnGetLastMonsNo( BOOL order_mons_no )  // mons_no順のときTRUE、gra_no順のときFALSE
{
  if( order_mons_no )
  {
    return MONSNO_END;
  }
  else
  {
    return d_test_cmn_mons_no_tbl_gra_no_order[MONSNO_END];
  }
}




static void D_Test_CmnBgColorEnumSetNext(
    D_TEST_COMMON_WORK*     work )
{
  work->bg_color_enum++;
  if( work->bg_color_enum >= D_TEST_CMN_BG_COLOR_ENUM_MAX )
  {
    work->bg_color_enum = D_TEST_CMN_BG_COLOR_ENUM_BLACK;
  }
  GFL_BG_SetBackGroundColor( GFL_BG_FRAME0_M, d_test_cmn_bg_color_enum[work->bg_color_enum] );
}

static void D_Test_CmnMcssSet(
    D_TEST_COMMON_WORK*                work,
    const VecFx32*                     mcss_pos,
    const VecFx32*                     mcss_scale,
    D_Test_CmnMcssSetOfsPositionFunc*  mcss_set_ofs_position_func )
{
  work->mcss_pos                     = *mcss_pos;
  work->mcss_scale                   = *mcss_scale;
  work->mcss_set_ofs_position_func   = mcss_set_ofs_position_func;
}


//-------------------------------------
/// ポケモン切り替え
//=====================================
static void D_Test_CmnPokeChange(
    D_TEST_COMMON_WORK* work,
    int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
    u32 personal_rnd )
{
  // 前のを削除
  {
    // MCSS
    if( work->mcss_wk )
    {
      MCSS_SetVanishFlag( work->mcss_wk );
      MCSS_Del( work->mcss_sys_wk, work->mcss_wk );
    }
  }

  // 次のを生成
  {
    // MCSS
    MCSS_ADD_WORK add_wk;

    if( mons_no == MONSNO_PATTIIRU )  // MCSS_TOOL_AddPokeMcssの中身に合わせてある
    {
      MCSS_TOOL_SetMakeBuchiCallback( work->mcss_sys_wk, personal_rnd );
    }
    MCSS_TOOL_MakeMAWParam( mons_no, form_no, sex, rare, egg, &add_wk, dir );
    work->mcss_wk = MCSS_Add( work->mcss_sys_wk, work->mcss_pos.x, work->mcss_pos.y, work->mcss_pos.z, &add_wk );
    MCSS_SetShadowVanishFlag( work->mcss_wk, TRUE );  // 影を消しておかないと、小さな点として影が表示されてしまう。
    MCSS_SetScale( work->mcss_wk, &work->mcss_scale );
    if( work->mcss_set_ofs_position_func )
    {
      work->mcss_set_ofs_position_func( work );
    }

    if( work->mcss_anm_play )
    {
      MCSS_SetAnimeIndex( work->mcss_wk, 0 );
      MCSS_ResetAnmStopFlag( work->mcss_wk );
      MCSS_TOOL_SetAnmRestartCallback( work->mcss_wk );  // 1ループしたらアニメーションリセットを呼ぶためのコールバックセット
    }
    else
    {
      MCSS_SetAnmStopFlag( work->mcss_wk );
    }
  }

  // 表示を変更したので、データも変更しておく
  {
    D_Test_CmnPokeChangeOnlyData(
      work,
      mons_no, form_no, sex, rare, egg, dir,
      personal_rnd );
  }

  // テキストも変更しておく
  D_Test_CmnPokeUpdateText( work );
}

static void D_Test_CmnPokeChangeOnlyData(
    D_TEST_COMMON_WORK* work,
    int mons_no, int form_no, int sex, int rare, BOOL egg, int dir,
    u32 personal_rnd )
{
  // 表示は変更せずデータだけ変更する
  {
    work->mons_no        = mons_no;
    work->form_no        = form_no;
    work->sex            = sex;
    work->rare           = rare;
    work->egg            = egg;
    work->dir            = dir;
    work->personal_rnd   = personal_rnd;
  }
}

static void D_Test_CmnPokeUpdateText( D_TEST_COMMON_WORK* work )
{
  // 現在の状態でテキストを更新する

  // ポケモン名 ピカチュウ
  {
    STRBUF* src_strbuf = GFL_MSG_CreateString( GlobalMsg_PokeName, work->mons_no );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_NAME]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_NAME] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_NAME][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_NAME][8],
                            src_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_NAME] = TRUE;
  }

  // 全国図鑑No 025
  {
    STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_mons_no );
    STRBUF* dst_strbuf = GFL_STR_CreateBuffer( D_TEST_CMN_STRBUF_LEN, work->heap_id );
    WORDSET* wordset = WORDSET_Create( work->heap_id );
    WORDSET_RegisterNumber( wordset, 0, work->mons_no, 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, dst_strbuf, src_strbuf );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_MONS_NO]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_MONS_NO] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_MONS_NO][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_MONS_NO][8],
                            dst_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    WORDSET_Delete( wordset );
    GFL_STR_DeleteBuffer( dst_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_MONS_NO] = TRUE;
  }

  // グラフィックNo 025
  {
    STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_gra_no );
    STRBUF* dst_strbuf = GFL_STR_CreateBuffer( D_TEST_CMN_STRBUF_LEN, work->heap_id );
    WORDSET* wordset = WORDSET_Create( work->heap_id );
    WORDSET_RegisterNumber( wordset, 0, d_test_cmn_mons_no_to_gra_no_tbl[work->mons_no], 3, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, dst_strbuf, src_strbuf );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_GRA_NO]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_GRA_NO] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_GRA_NO][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_GRA_NO][8],
                            dst_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    WORDSET_Delete( wordset );
    GFL_STR_DeleteBuffer( dst_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_GRA_NO] = TRUE;
  }

  // フォルムNo 00
  {
    STRBUF* src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_form_no );
    STRBUF* dst_strbuf = GFL_STR_CreateBuffer( D_TEST_CMN_STRBUF_LEN, work->heap_id );
    WORDSET* wordset = WORDSET_Create( work->heap_id );
    WORDSET_RegisterNumber( wordset, 0, work->form_no, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
    WORDSET_ExpandStr( wordset, dst_strbuf, src_strbuf );
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_FORM_NO]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_FORM_NO] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_FORM_NO][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_FORM_NO][8],
                            dst_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    WORDSET_Delete( wordset );
    GFL_STR_DeleteBuffer( dst_strbuf );
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_FORM_NO] = TRUE;
  }

  // 性別
  {
    STRBUF* src_strbuf;
    switch( work->sex )
    {
    case PTL_SEX_MALE:
      {
        src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_sex_male );
      }
      break;
    case PTL_SEX_FEMALE:
      {
        src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_sex_female );
      }
      break;
    case PTL_SEX_UNKNOWN:
      {
        src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_sex_none );
      }
      break;
    }
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_SEX]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_SEX] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_SEX][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_SEX][8],
                            src_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_SEX] = TRUE;
  }

  // 色
  {
    STRBUF* src_strbuf;
    if( work->rare == 0 )  // ノーマル
    {
      src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_color_normal );
    }
    else                   // レア
    {
      src_strbuf = GFL_MSG_CreateString( work->msgdata, poke_color_rare );
    }
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp(work->text_bmpwin[D_TEST_CMN_TEXT_POKE_COLOR]), 0 );
    PRINTSYS_PrintQueColor( work->print_que, GFL_BMPWIN_GetBmp( work->text_bmpwin[D_TEST_CMN_TEXT_POKE_COLOR] ),
                            d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_COLOR][7], d_test_cmn_bmpwin_setup[D_TEST_CMN_TEXT_POKE_COLOR][8],
                            src_strbuf, work->font, PRINTSYS_LSB_Make(1,2,0) );  
    GFL_STR_DeleteBuffer( src_strbuf );
    work->text_trans[D_TEST_CMN_TEXT_POKE_COLOR] = TRUE;
  }

  D_Test_CmnTextMain( work );
}

static void D_Test_CmnPokeAnmFlip( D_TEST_COMMON_WORK* work )
{
  if( work->mcss_anm_play )
  {
    work->mcss_anm_play = FALSE;
  }
  else
  {
    work->mcss_anm_play = TRUE;
  }

  if( work->mcss_wk )
  {
    if( work->mcss_anm_play )
    {
      MCSS_SetAnimeIndex( work->mcss_wk, 0 );
      MCSS_ResetAnmStopFlag( work->mcss_wk );
      MCSS_TOOL_SetAnmRestartCallback( work->mcss_wk );  // 1ループしたらアニメーションリセットを呼ぶためのコールバックセット
    }
    else
    {
      MCSS_SetAnmStopFlag( work->mcss_wk );
    }
  }
}

static void D_Test_CmnPokeDirFlip( D_TEST_COMMON_WORK* work )
{
  if( work->mcss_dir_front )
  {
    work->mcss_dir_front = FALSE;
    work->dir = MCSS_DIR_BACK;
  }
  else
  {
    work->mcss_dir_front = TRUE;
    work->dir = MCSS_DIR_FRONT;
  }

  D_Test_CmnPokeChange(
    work,
    work->mons_no, work->form_no, work->sex, work->rare, work->egg, work->dir,
    work->personal_rnd );
}


//-------------------------------------
/// 次のフォルム情報を得る(今のポケモンの次のフォルムがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
//=====================================
static BOOL D_Test_CmnPokeGetNextForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,
    u32* personal_rnd )
{
  u32 ppd_form_max;
  u32 ppd_sex;

  D_Test_CmnPokeGetPersonalData(
      work, 
      work->mons_no,
      &ppd_form_max,
      &ppd_sex );

  if( ppd_form_max <= 1 )  // フォルム違いなし
  {
    if( work->rare == 0 )  // 今ノーマルカラーなら次はレアカラーにする
    {
      *mons_no       = work->mons_no;
      *form_no       = work->form_no;
      *sex           = work->sex;
      *rare          = 1;
      *egg           = work->egg;
      *dir           = work->dir;
      *personal_rnd  = work->personal_rnd;
      return TRUE;
    }
    else  // 今レアカラーなら次は次の性別のノーマルカラーにする
    {
      switch( ppd_sex )
      {
      case POKEPER_SEX_MALE:      // 性別固定
      case POKEPER_SEX_FEMALE:
      case POKEPER_SEX_UNKNOWN:
        {
          return FALSE;
        }
        break;
      default:  // 1～253        // オスとメス、どちらも存在する
        {
          if( work->sex == PTL_SEX_FEMALE )  // 今メスなので次の性別なし
          {
            return FALSE;
          }
          else  // 今オスなので次はメスにする
          {
            *mons_no       = work->mons_no;
            *form_no       = work->form_no;
            *sex           = PTL_SEX_FEMALE;
            *rare          = 0;
            *egg           = work->egg;
            *dir           = work->dir;
            *personal_rnd  = work->personal_rnd;
            return TRUE;
          }
        }
        break;
      }
    }
  }
  else  // フォルム違いあり
  {
    if( work->rare == 0 )  // 今ノーマルカラーなら次はレアカラーにする
    {
      *mons_no       = work->mons_no;
      *form_no       = work->form_no;
      *sex           = work->sex;
      *rare          = 1;
      *egg           = work->egg;
      *dir           = work->dir;
      *personal_rnd  = work->personal_rnd;
      return TRUE;
    }
    else  // 今レアカラーなら次は次のフォルムのノーマルカラーにする
    {
      if( work->form_no +1 >= ppd_form_max )  // 次のフォルムなし
      {
        return FALSE;
      }
      else
      {
        *mons_no       = work->mons_no;
        *form_no       = work->form_no +1;
        *sex           = work->sex;
        *rare          = 0;
        *egg           = work->egg;
        *dir           = work->dir;
        *personal_rnd  = work->personal_rnd;
        return TRUE;
      }
    }
  }
}
//-------------------------------------
/// 前のフォルム情報を得る(今のポケモンの前のフォルムがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
//=====================================
static BOOL D_Test_CmnPokeGetPrevForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  u32 ppd_form_max;
  u32 ppd_sex;

  D_Test_CmnPokeGetPersonalData(
      work, 
      work->mons_no,
      &ppd_form_max,
      &ppd_sex );

  if( ppd_form_max <= 1 )  // フォルム違いなし
  {
    if( work->rare == 1 )  // 今レアカラーなら次はノーマルカラーにする
    {
      *mons_no       = work->mons_no;
      *form_no       = work->form_no;
      *sex           = work->sex;
      *rare          = 0;
      *egg           = work->egg;
      *dir           = work->dir;
      *personal_rnd  = work->personal_rnd;
      return TRUE;
    }
    else  // 今ノーマルカラーなら次は前の性別のレアカラーにする
    {
      switch( ppd_sex )
      {
      case POKEPER_SEX_MALE:      // 性別固定
      case POKEPER_SEX_FEMALE:
      case POKEPER_SEX_UNKNOWN:
        {
          return FALSE;
        }
        break;
      default:  // 1～253        // オスとメス、どちらも存在する
        {
          if( work->sex == PTL_SEX_MALE )  // 今オスなので前の性別なし
          {
            return FALSE;
          }
          else  // 今メスなので次はオスにする
          {
            *mons_no       = work->mons_no;
            *form_no       = work->form_no;
            *sex           = PTL_SEX_MALE;
            *rare          = 1;
            *egg           = work->egg;
            *dir           = work->dir;
            *personal_rnd  = work->personal_rnd;
            return TRUE;
          }
        }
        break;
      }
    }
  }
  else  // フォルム違いあり
  {
    if( work->rare == 1 )  // 今レアカラーなら次はノーマルカラーにする
    {
      *mons_no       = work->mons_no;
      *form_no       = work->form_no;
      *sex           = work->sex;
      *rare          = 0;
      *egg           = work->egg;
      *dir           = work->dir;
      *personal_rnd  = work->personal_rnd;
      return TRUE;
    }
    else  // 今ノーマルカラーなら次は前のフォルムのレアカラーにする
    {
      if( work->form_no <= 0 )  // 前のフォルムなし
      {
        return FALSE;
      }
      else
      {
        *mons_no       = work->mons_no;
        *form_no       = work->form_no -1;
        *sex           = work->sex;
        *rare          = 1;
        *egg           = work->egg;
        *dir           = work->dir;
        *personal_rnd  = work->personal_rnd;
        return TRUE;
      }
    }
  }
}

//-------------------------------------
/// 次のポケモン情報を得る(次のポケモンがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
//=====================================
static BOOL D_Test_CmnPokeGetNextPoke(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  *mons_no = D_Test_CmnGetNextMonsNo( work->mons_no, work->order_mons_no );

  if( *mons_no > MONSNO_END )
  {
    return FALSE;
  }
  else
  {
    u32 ppd_form_max;
    u32 ppd_sex;

    D_Test_CmnPokeGetPersonalData(
        work, 
        *mons_no,
        &ppd_form_max,
        &ppd_sex );

    *form_no = 0;

    switch( ppd_sex )
    {
    case POKEPER_SEX_MALE:
      {
        *sex = PTL_SEX_MALE;
      }
      break;
    case POKEPER_SEX_FEMALE:
      {
        *sex = PTL_SEX_FEMALE;
      }
      break;
    case POKEPER_SEX_UNKNOWN:
      {
        *sex = PTL_SEX_UNKNOWN;
      }
      break;
    default:  // 1～253
      {
        *sex = PTL_SEX_MALE;
      }
      break;
    }

    *rare         = 0;
    *egg          = FALSE;
    *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
    *personal_rnd = 0;

    return TRUE;
  }
}
//-------------------------------------
/// 前のポケモン情報を得る(前のポケモンがないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
//=====================================
static BOOL D_Test_CmnPokeGetPrevPoke(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  *mons_no = D_Test_CmnGetPrevMonsNo( work->mons_no, work->order_mons_no );

  if( *mons_no <= 0 )
  {
    return FALSE;
  }
  else
  {
    u32 ppd_form_max;
    u32 ppd_sex;

    D_Test_CmnPokeGetPersonalData(
        work, 
        *mons_no,
        &ppd_form_max,
        &ppd_sex );

    *form_no = 0;

    switch( ppd_sex )
    {
    case POKEPER_SEX_MALE:
      {
        *sex = PTL_SEX_MALE;
      }
      break;
    case POKEPER_SEX_FEMALE:
      {
        *sex = PTL_SEX_FEMALE;
      }
      break;
    case POKEPER_SEX_UNKNOWN:
      {
        *sex = PTL_SEX_UNKNOWN;
      }
      break;
    default:  // 1～253
      {
        *sex = PTL_SEX_MALE;
      }
      break;
    }

    *rare         = 0;
    *egg          = FALSE;
    *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
    *personal_rnd = 0;

    return TRUE;
  }
}
//-------------------------------------
/// 前のポケモンの最後のフォルム情報を得る(前のポケモンないときFALSEを返す、そのときの引き数戻り値にはゴミが入っている)
//=====================================
static BOOL D_Test_CmnPokeGetPrevPokeLastForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  *mons_no = D_Test_CmnGetPrevMonsNo( work->mons_no, work->order_mons_no );

  if( *mons_no <= 0 )
  {
    return FALSE;
  }
  else
  {
    u32 ppd_form_max;
    u32 ppd_sex;

    D_Test_CmnPokeGetPersonalData(
        work, 
        *mons_no,
        &ppd_form_max,
        &ppd_sex );

    *form_no = ppd_form_max -1;
    
    switch( ppd_sex )
    {
    case POKEPER_SEX_MALE:      // 性別固定
      {
        *sex = PTL_SEX_MALE;
      }
      break;
    case POKEPER_SEX_FEMALE:
      {
        *sex = PTL_SEX_FEMALE;
      }
      break;
    case POKEPER_SEX_UNKNOWN:
      {
        *sex = PTL_SEX_UNKNOWN;
      }
      break;
    default:  // 1～253        // オスとメス、どちらも存在する
      {
        *sex = PTL_SEX_FEMALE;
      }
      break;
    }

    *rare         = 1;
    *egg          = FALSE;
    *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
    *personal_rnd = 0;
    
    return TRUE;
  }
}

//-------------------------------------
/// 最初のポケモンの最初のフォルム情報を得る
//=====================================
static void D_Test_CmnPokeGetFirstPokeFirstForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  u32 ppd_form_max;
  u32 ppd_sex;
  
  *mons_no = D_Test_CmnGetFirstMonsNo( work->order_mons_no );  // MONSNO_HUSIGIDANE<= <=MONSNO_END  // prog/include/poke_tool/monsno_def.h
  
  D_Test_CmnPokeGetPersonalData(
      work,
      *mons_no,
      &ppd_form_max,
      &ppd_sex );

  *form_no      = 0;

  switch( ppd_sex )
  {
  case POKEPER_SEX_MALE:
    {
      *sex = PTL_SEX_MALE;  // PTL_SEX_MALE, PTL_SEX_FEMALE, PTL_SEX_UNKNOWN  // prog/include/poke_tool/poke_tool.h
    }
    break;
  case POKEPER_SEX_FEMALE:
    {
      *sex = PTL_SEX_FEMALE;
    }
    break;
  case POKEPER_SEX_UNKNOWN:
    {
      *sex = PTL_SEX_UNKNOWN;
    }
    break;
  default:  // 1～253
    {
      *sex = PTL_SEX_MALE;
    }
    break;
  }

  *rare         = 0;
  *egg          = FALSE;
  *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
  *personal_rnd = 0;
}
//-------------------------------------
/// 最後のポケモンの最後のフォルム情報を得る
//=====================================
static void D_Test_CmnPokeGetLastPokeLastForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  u32 ppd_form_max;
  u32 ppd_sex;
 
  *mons_no = D_Test_CmnGetLastMonsNo( work->order_mons_no );

  D_Test_CmnPokeGetPersonalData(
      work,
      *mons_no,
      &ppd_form_max,
      &ppd_sex );

  *form_no      = ppd_form_max -1;

  switch( ppd_sex )
  {
  case POKEPER_SEX_MALE:
    {
      *sex = PTL_SEX_MALE;
    }
    break;
  case POKEPER_SEX_FEMALE:
    {
      *sex = PTL_SEX_FEMALE;
    }
    break;
  case POKEPER_SEX_UNKNOWN:
    {
      *sex = PTL_SEX_UNKNOWN;
    }
    break;
  default:  // 1～253
    {
      *sex = PTL_SEX_FEMALE;
    }
    break;
  }

  *rare         = 1;
  *egg          = FALSE;
  *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
  *personal_rnd = 0;
}
//-------------------------------------
/// 最後のポケモンの最初のフォルム情報を得る
//=====================================
static void D_Test_CmnPokeGetLastPokeFirstForm(
    D_TEST_COMMON_WORK* work,
    int* mons_no, int* form_no, int* sex, int* rare, BOOL* egg, int* dir,  // 引き数戻り値
    u32* personal_rnd )                                                    // 引き数戻り値
{
  u32 ppd_form_max;
  u32 ppd_sex;
  
  *mons_no = D_Test_CmnGetLastMonsNo( work->order_mons_no );
  
  D_Test_CmnPokeGetPersonalData(
      work,
      *mons_no,
      &ppd_form_max,
      &ppd_sex );

  *form_no      = 0;

  switch( ppd_sex )
  {
  case POKEPER_SEX_MALE:
    {
      *sex = PTL_SEX_MALE;
    }
    break;
  case POKEPER_SEX_FEMALE:
    {
      *sex = PTL_SEX_FEMALE;
    }
    break;
  case POKEPER_SEX_UNKNOWN:
    {
      *sex = PTL_SEX_UNKNOWN;
    }
    break;
  default:  // 1～253
    {
      *sex = PTL_SEX_MALE;
    }
    break;
  }

  *rare         = 0;
  *egg          = FALSE;
  *dir          = (work->mcss_dir_front)?(MCSS_DIR_FRONT):(MCSS_DIR_BACK);
  *personal_rnd = 0;
} 

//-------------------------------------
/// ポケモンパーソナルデータの情報から、フォルム数と性別ベクトルを得る
//=====================================
static void D_Test_CmnPokeGetPersonalData(
    D_TEST_COMMON_WORK* work,
    u16 mons_no,
    u32* ppd_form_max, u32* ppd_sex )  // 引き数戻り値
{
  POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle( mons_no, 0, work->heap_id );

  *ppd_form_max = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_form_max );  // 別フォルムなしのときは1
  *ppd_sex      = POKE_PERSONAL_GetParam( ppd, POKEPER_ID_sex );       // POKEPER_SEX_MALE, 1～253, POKEPER_SEX_FEMALE, POKEPER_SEX_UNKNOWN  // prog/include/poke_tool/poke_personal.h

  POKE_PERSONAL_CloseHandle( ppd );
} 




//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
/**
*  進化デモカメラ
*/
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

#define HEAPID_D_TEST_SHINKA (HEAPID_SHINKA_DEMO)
#define HEAP_SIZE_D_TEST_SHINKA  (0x70000)


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                             heap_id;
  D_KAWADA_TEST_SHINKA_PARAM*        param;

  // グラフィックなど
  SHINKADEMO_GRAPHIC_WORK*           graphic;

  // VBlank中TCB
  GFL_TCB*                           vblank_tcb;

  // 共通使用できる事柄
  D_TEST_COMMON_WORK*                cmn_wk;
}
D_TEST_SHINKA_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void D_Test_ShinkaVBlankFunc( GFL_TCB* tcb, void* wk );

// オフセットを設定する関数
static void D_Test_ShinkaMcssSetOfsPositionFunc(void* d_test_cmn_wk);


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT D_Test_ShinkaProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ShinkaProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ShinkaProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    D_KAWADA_TEST_ShinkaProcData =
{
  D_Test_ShinkaProcInit,
  D_Test_ShinkaProcMain,
  D_Test_ShinkaProcExit,
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
 *  @param[in]       
 *
 *  @retval          
 */
//------------------------------------------------------------------


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT D_Test_ShinkaProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_SHINKA_WORK*  work;

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(shinka_demo) );

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_D_TEST_SHINKA, HEAP_SIZE_D_TEST_SHINKA );
    work = GFL_PROC_AllocWork( proc, sizeof(D_TEST_SHINKA_WORK), HEAPID_D_TEST_SHINKA );
    GFL_STD_MemClear( work, sizeof(D_TEST_SHINKA_WORK) );
    
    work->heap_id  = HEAPID_D_TEST_SHINKA;
    work->param    = (D_KAWADA_TEST_SHINKA_PARAM*)pwk;
  }

  // グラフィックなど
  {
    work->graphic    = SHINKADEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
    SHINKADEMO_GRAPHIC_InitBGSub( work->graphic );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( D_Test_ShinkaVBlankFunc, work, 1 );

  // 共通使用できる事柄
  {
    VecFx32  mcss_pos    = { FX_F32_TO_FX32(0.0f), FX_F32_TO_FX32(-18.0f), 0 };
    VecFx32  mcss_scale  = { FX_F32_TO_FX32(16.0f), FX_F32_TO_FX32(16.0f), FX32_ONE };

    work->cmn_wk = D_Test_CmnInit( work->heap_id );
    D_Test_CmnMcssSet(
      work->cmn_wk,
      &mcss_pos,
      &mcss_scale,
      D_Test_ShinkaMcssSetOfsPositionFunc );
  }

  // 最初のポケモン
  {
    int  mons_no;
    int  form_no;
    int  sex;
    int  rare;
    BOOL egg;
    int  dir;
    u32  personal_rnd;
    D_Test_CmnPokeGetFirstPokeFirstForm( work->cmn_wk, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    D_Test_CmnPokeChange( work->cmn_wk, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // フェードイン(ただちに見えるように)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 0, 0 );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT D_Test_ShinkaProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_SHINKA_WORK*  work  = (D_TEST_SHINKA_WORK*)mywk;

  // 共通使用できる事柄
  D_Test_CmnExit( work->cmn_wk );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィックなど
  {
    SHINKADEMO_GRAPHIC_ExitBGSub( work->graphic );
    SHINKADEMO_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_D_TEST_SHINKA );
  }

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(shinka_demo) );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT D_Test_ShinkaProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_SHINKA_WORK*     work     = (D_TEST_SHINKA_WORK*)mywk;

  BOOL res = D_Test_CmnInput( work->cmn_wk );

  if( res )
  {
    return GFL_PROC_RES_FINISH;
  }
  
  // メイン
  D_Test_CmnMain( work->cmn_wk );
 
  // 3D描画
  SHINKADEMO_GRAPHIC_3D_StartDraw( work->graphic );
  D_Test_CmnDraw3D( work->cmn_wk );  // 共通使用できる事柄
  SHINKADEMO_GRAPHIC_3D_EndDraw( work->graphic );

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
static void D_Test_ShinkaVBlankFunc( GFL_TCB* tcb, void* wk )
{
  D_TEST_SHINKA_WORK* work = (D_TEST_SHINKA_WORK*)wk;
}

//-------------------------------------
/// オフセットを設定する関数
//=====================================
static void D_Test_ShinkaMcssSetOfsPositionFunc(void* d_test_cmn_wk)
{
  D_TEST_COMMON_WORK* cmn_wk = (D_TEST_COMMON_WORK*)d_test_cmn_wk;

  {
    f32      size_y   = (f32)MCSS_GetSizeY( cmn_wk->mcss_wk );
    f32      offset_y = (f32)MCSS_GetOffsetY( cmn_wk->mcss_wk );  // 浮いているとき-, 沈んでいるとき+
    f32      offset_x = (f32)MCSS_GetOffsetX( cmn_wk->mcss_wk );  // 右にずれているとき+, 左にずれているとき-
    f32      ofs_position_y;
    f32      ofs_position_x;
    VecFx32  ofs_position;

    //size_y   *= 16.0f / 16.0f;  // 意味ないのでコメントアウトした
    if( size_y > 96.0f ) size_y = 96.0f;

    //offset_y *= 16.0f / 16.0f;  // 意味ないのでコメントアウトした
    //offset_x *= 16.0f / 16.0f;  // 意味ないのでコメントアウトした

#if 1  // もしかして何倍不要か？
    ofs_position_y = ( ( 96.0f - size_y ) / 2.0f + offset_y ) * (0.33f);
    ofs_position_x = - offset_x * (0.33f);
#else
    ofs_position_y = ( ( 96.0f - size_y ) / 2.0f + offset_y );
    ofs_position_x = - offset_x;
#endif

    ofs_position.x = FX_F32_TO_FX32(ofs_position_x);  ofs_position.y = FX_F32_TO_FX32(ofs_position_y);  ofs_position.z = 0;
    MCSS_SetOfsPosition( cmn_wk->mcss_wk, &ofs_position );
  }
}




//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
/**
*  タマゴ孵化デモカメラ
*/
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

#define HEAPID_D_TEST_EGG     (HEAPID_EGG_DEMO)
#define HEAP_SIZE_D_TEST_EGG  (0x50000)


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                             heap_id;
  D_KAWADA_TEST_EGG_PARAM*           param;

  // グラフィックなど
  EGG_DEMO_GRAPHIC_WORK*             graphic;

  // VBlank中TCB
  GFL_TCB*                           vblank_tcb;

  // 共通使用できる事柄
  D_TEST_COMMON_WORK*                cmn_wk;
}
D_TEST_EGG_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void D_Test_EggVBlankFunc( GFL_TCB* tcb, void* wk );

// オフセットを設定する関数
static void D_Test_EggMcssSetOfsPositionFunc(void* d_test_cmn_wk);


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT D_Test_EggProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_EggProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_EggProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    D_KAWADA_TEST_EggProcData =
{
  D_Test_EggProcInit,
  D_Test_EggProcMain,
  D_Test_EggProcExit,
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
 *  @param[in]       
 *
 *  @retval          
 */
//------------------------------------------------------------------


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT D_Test_EggProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_EGG_WORK*  work;

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(egg_demo) );

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_D_TEST_EGG, HEAP_SIZE_D_TEST_EGG );
    work = GFL_PROC_AllocWork( proc, sizeof(D_TEST_EGG_WORK), HEAPID_D_TEST_EGG );
    GFL_STD_MemClear( work, sizeof(D_TEST_EGG_WORK) );
    
    work->heap_id  = HEAPID_D_TEST_EGG;
    work->param    = (D_KAWADA_TEST_EGG_PARAM*)pwk;
  }

  // グラフィックなど
  {
    work->graphic    = EGG_DEMO_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id );
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( D_Test_EggVBlankFunc, work, 1 );

  // 共通使用できる事柄
  {
    VecFx32  mcss_pos    = { 0, FX_F32_TO_FX32(-190.0f), FX_F32_TO_FX32(-800.0f) };
    VecFx32  mcss_scale  = { FX_F32_TO_FX32(16.0f), FX_F32_TO_FX32(16.0f), FX32_ONE };

    work->cmn_wk = D_Test_CmnInit( work->heap_id );
    D_Test_CmnMcssSet(
      work->cmn_wk,
      &mcss_pos,
      &mcss_scale,
      D_Test_EggMcssSetOfsPositionFunc );
  }

  // 最初のポケモン
  {
    int  mons_no;
    int  form_no;
    int  sex;
    int  rare;
    BOOL egg;
    int  dir;
    u32  personal_rnd;
    D_Test_CmnPokeGetFirstPokeFirstForm( work->cmn_wk, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    D_Test_CmnPokeChange( work->cmn_wk, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // フェードイン(ただちに見えるように)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 0, 0 );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT D_Test_EggProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_EGG_WORK*  work  = (D_TEST_EGG_WORK*)mywk;

  // 共通使用できる事柄
  D_Test_CmnExit( work->cmn_wk );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィックなど
  {
    EGG_DEMO_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_D_TEST_EGG );
  }

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(egg_demo) );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT D_Test_EggProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_EGG_WORK*     work     = (D_TEST_EGG_WORK*)mywk;

  BOOL res = D_Test_CmnInput( work->cmn_wk );

  if( res )
  {
    return GFL_PROC_RES_FINISH;
  }
  
  // メイン
  D_Test_CmnMain( work->cmn_wk );
 
  // 3D描画
  EGG_DEMO_GRAPHIC_3D_StartDraw( work->graphic );
  D_Test_CmnDraw3D( work->cmn_wk );  // 共通使用できる事柄
  EGG_DEMO_GRAPHIC_3D_EndDraw( work->graphic );

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
static void D_Test_EggVBlankFunc( GFL_TCB* tcb, void* wk )
{
  D_TEST_EGG_WORK* work = (D_TEST_EGG_WORK*)wk;
}

//-------------------------------------
/// オフセットを設定する関数
//=====================================
static void D_Test_EggMcssSetOfsPositionFunc(void* d_test_cmn_wk)
{
  D_TEST_COMMON_WORK* cmn_wk = (D_TEST_COMMON_WORK*)d_test_cmn_wk;

  {
    f32      size_y   = (f32)MCSS_GetSizeY( cmn_wk->mcss_wk );
    f32      offset_y = (f32)MCSS_GetOffsetY( cmn_wk->mcss_wk );  // 浮いているとき-, 沈んでいるとき+
    f32      offset_x = (f32)MCSS_GetOffsetX( cmn_wk->mcss_wk );  // 右にずれているとき+, 左にずれているとき-
    f32      ofs_position_y;
    f32      ofs_position_x;
    VecFx32  ofs_position;

    OS_Printf( "MCSS_GetOffsetX=%d\n", MCSS_GetOffsetX(cmn_wk->mcss_wk) );

    if( size_y > 96.0f ) size_y = 96.0f;

    ofs_position_y = ( 96.0f - size_y ) / 2.0f + offset_y;
    ofs_position_x = - offset_x;
      
    ofs_position.x = FX_F32_TO_FX32(ofs_position_x);  ofs_position.y = FX_F32_TO_FX32(ofs_position_y);  ofs_position.z = 0;
    MCSS_SetOfsPosition( cmn_wk->mcss_wk, &ofs_position );
  }
}








//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================
/**
*  図鑑フォルムカメラ
*/
//=============================================================================
//=============================================================================
//=============================================================================
//=============================================================================

#define HEAPID_D_TEST_ZUKAN_FORM     (HEAPID_SHINKA_DEMO)
#define HEAP_SIZE_D_TEST_ZUKAN_FORM  (0x90000)


//=============================================================================
/**
*  定数定義
*/
//=============================================================================


//=============================================================================
/**
*  構造体宣言
*/
//=============================================================================
//-------------------------------------
/// PROC ワーク
//=====================================
typedef struct
{
  // ヒープ、パラメータなど
  HEAPID                             heap_id;
  D_KAWADA_TEST_ZUKAN_FORM_PARAM*    param;

  // グラフィックなど
  ZUKAN_DETAIL_GRAPHIC_WORK*         graphic;

  // VBlank中TCB
  GFL_TCB*                           vblank_tcb;

  // 共通使用できる事柄
  D_TEST_COMMON_WORK*                cmn_wk;
}
D_TEST_ZUKAN_FORM_WORK;


//=============================================================================
/**
*  ローカル関数のプロトタイプ宣言
*/
//=============================================================================
// VBlank関数
static void D_Test_ZukanFormVBlankFunc( GFL_TCB* tcb, void* wk );

// オフセットを設定する関数
static void D_Test_ZukanFormMcssSetOfsPositionFunc(void* d_test_cmn_wk);


//=============================================================================
/**
*  PROC
*/
//=============================================================================
static GFL_PROC_RESULT D_Test_ZukanFormProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ZukanFormProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk );
static GFL_PROC_RESULT D_Test_ZukanFormProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk );

const GFL_PROC_DATA    D_KAWADA_TEST_ZukanFormProcData =
{
  D_Test_ZukanFormProcInit,
  D_Test_ZukanFormProcMain,
  D_Test_ZukanFormProcExit,
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
 *  @param[in]       
 *
 *  @retval          
 */
//------------------------------------------------------------------


//=============================================================================
/**
*  ローカル関数定義(PROC)
*/
//=============================================================================
//-------------------------------------
/// PROC 初期化処理
//=====================================
static GFL_PROC_RESULT D_Test_ZukanFormProcInit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_ZUKAN_FORM_WORK*  work;

  // オーバーレイ
  GFL_OVERLAY_Load( FS_OVERLAY_ID(zukan_detail) );

  // ヒープ、パラメータなど
  {
    GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_D_TEST_ZUKAN_FORM, HEAP_SIZE_D_TEST_ZUKAN_FORM );
    work = GFL_PROC_AllocWork( proc, sizeof(D_TEST_ZUKAN_FORM_WORK), HEAPID_D_TEST_ZUKAN_FORM );
    GFL_STD_MemClear( work, sizeof(D_TEST_ZUKAN_FORM_WORK) );
    
    work->heap_id  = HEAPID_D_TEST_ZUKAN_FORM;
    work->param    = (D_KAWADA_TEST_ZUKAN_FORM_PARAM*)pwk;
  }

  // グラフィックなど
  {
    work->graphic    = ZUKAN_DETAIL_GRAPHIC_Init( GX_DISP_SELECT_MAIN_SUB, work->heap_id, TRUE );

    // 切り替え
    {
      // グラフィックスモード設定
      GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_3D );
    }
  }

  // VBlank中TCB
  work->vblank_tcb = GFUser_VIntr_CreateTCB( D_Test_ZukanFormVBlankFunc, work, 1 );

  // 共通使用できる事柄
  {
    VecFx32  mcss_pos;
    VecFx32  mcss_scale;

    switch( work->param->mode )
    {
    case 0:
      {
        mcss_pos.x   = FX_F32_TO_FX32(0.0f);   mcss_pos.y   = FX_F32_TO_FX32(-13.9f);  mcss_pos.z   = FX_F32_TO_FX32(0.0f);
        mcss_scale.x = FX_F32_TO_FX32(16.0f);  mcss_scale.y = FX_F32_TO_FX32(16.0f);   mcss_scale.z = FX32_ONE;
      }
      break;
    case 1:
      {
        mcss_pos.x   = FX_F32_TO_FX32(-16.0f);  mcss_pos.y   = FX_F32_TO_FX32(-13.9f);  mcss_pos.z   = FX_F32_TO_FX32(0.0f);
        mcss_scale.x = FX_F32_TO_FX32(16.0f);   mcss_scale.y = FX_F32_TO_FX32(16.0f);   mcss_scale.z = FX32_ONE;
      }
      break;
    case 2:
      {
        mcss_pos.x   = FX_F32_TO_FX32(16.0f);  mcss_pos.y   = FX_F32_TO_FX32(-13.9f);  mcss_pos.z   = FX_F32_TO_FX32(0.0f);
        mcss_scale.x = FX_F32_TO_FX32(16.0f);  mcss_scale.y = FX_F32_TO_FX32(16.0f);   mcss_scale.z = FX32_ONE;
      }
      break;
    }

    work->cmn_wk = D_Test_CmnInit( work->heap_id );
    D_Test_CmnMcssSet(
      work->cmn_wk,
      &mcss_pos,
      &mcss_scale,
      D_Test_ZukanFormMcssSetOfsPositionFunc );
  }

  // 最初のポケモン
  {
    int  mons_no;
    int  form_no;
    int  sex;
    int  rare;
    BOOL egg;
    int  dir;
    u32  personal_rnd;
    D_Test_CmnPokeGetFirstPokeFirstForm( work->cmn_wk, &mons_no, &form_no, &sex, &rare, &egg, &dir, &personal_rnd );
    D_Test_CmnPokeChange( work->cmn_wk, mons_no, form_no, sex, rare, egg, dir, personal_rnd );
  }

  // フェードイン(ただちに見えるように)
  GFL_FADE_SetMasterBrightReq( GFL_FADE_MASTER_BRIGHT_BLACKOUT, 0, 0, 0 );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 終了処理
//=====================================
static GFL_PROC_RESULT D_Test_ZukanFormProcExit( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_ZUKAN_FORM_WORK*  work  = (D_TEST_ZUKAN_FORM_WORK*)mywk;

  // 共通使用できる事柄
  D_Test_CmnExit( work->cmn_wk );

  // VBlank中TCB
  GFL_TCB_DeleteTask( work->vblank_tcb );

  // グラフィックなど
  {
    // 切り替え
    {
      // グラフィックスモード設定
      GX_SetGraphicsMode( GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BG0_AS_2D );
    }

    ZUKAN_DETAIL_GRAPHIC_Exit( work->graphic );
  }

  // ヒープ、パラメータなど
  {
    GFL_PROC_FreeWork( proc );
    GFL_HEAP_DeleteHeap( HEAPID_D_TEST_ZUKAN_FORM );
  }

  // オーバーレイ
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(zukan_detail) );

  return GFL_PROC_RES_FINISH;
}

//-------------------------------------
/// PROC 主処理
//=====================================
static GFL_PROC_RESULT D_Test_ZukanFormProcMain( GFL_PROC* proc, int* seq, void* pwk, void* mywk )
{
  D_TEST_ZUKAN_FORM_WORK*     work     = (D_TEST_ZUKAN_FORM_WORK*)mywk;

  BOOL res = D_Test_CmnInput( work->cmn_wk );

  if( res )
  {
    return GFL_PROC_RES_FINISH;
  }
  
  // メイン
  D_Test_CmnMain( work->cmn_wk );
 
  // 3D描画
  ZUKAN_DETAIL_GRAPHIC_3D_StartDraw( work->graphic );
  D_Test_CmnDraw3D( work->cmn_wk );  // 共通使用できる事柄
  ZUKAN_DETAIL_GRAPHIC_3D_EndDraw( work->graphic );

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
static void D_Test_ZukanFormVBlankFunc( GFL_TCB* tcb, void* wk )
{
  D_TEST_ZUKAN_FORM_WORK* work = (D_TEST_ZUKAN_FORM_WORK*)wk;
}

//-------------------------------------
/// オフセットを設定する関数
//=====================================
static void D_Test_ZukanFormMcssSetOfsPositionFunc(void* d_test_cmn_wk)
{
  D_TEST_COMMON_WORK* cmn_wk = (D_TEST_COMMON_WORK*)d_test_cmn_wk;

  {
    f32      offset_x = (f32)MCSS_GetOffsetX( cmn_wk->mcss_wk );  // 右にずれているとき+, 左にずれているとき-
    f32      ofs_position_x;
    VecFx32  ofs_position;

#if 1  // もしかして何倍不要か？
    ofs_position_x = - offset_x * (0.25f);
#else 
    ofs_position_x = - offset_x * (0.25f);
#endif

    ofs_position.x = FX_F32_TO_FX32(ofs_position_x);  ofs_position.y = 0;  ofs_position.z = 0;
    MCSS_SetOfsPosition( cmn_wk->mcss_wk, &ofs_position );
  }
}


#endif
