//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *  GAME FREAK inc.
 *
 *  @file   field_light.c
 *  @brief    フィールドライトシステム
 *  @author   tomoya takahashi
 *  @date   2009.03.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "arc_def.h"
#include "message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "msg/msg_d_tomoya.h"

#include "font/font.naix"


#include  "field_light.h"

//-----------------------------------------------------------------------------
/**
 *          コーディング規約
 *    ●関数名
 *        １文字目は大文字それ以降は小文字にする
 *    ●変数名
 *        ・変数共通
 *            constには c_ を付ける
 *            staticには s_ を付ける
 *            ポインタには p_ を付ける
 *            全て合わさると csp_ となる
 *        ・グローバル変数
 *            １文字目は大文字
 *        ・関数内変数
 *            小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
#ifdef PM_DEBUG
//#define DEBUG_LIGHT_AUTO  // ライトを昼多めに
#endif

#ifdef DEBUG_LIGHT_AUTO
static const u32 sc_DEBUG_LIGHT_AUTO_END_TIME[] = {
  0, 7200, 7700, 8100,
  9000, 14070, 21600, 30000, 34000,
  36200, 37200, 38200, 39000, 42000, 43200,
};
#endif


//-----------------------------------------------------------------------------
/**
 *          定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
/// シーケンス
//=====================================
enum {
  FIELD_LIGHT_SEQ_NORMAL,   // 通常
  FIELD_LIGHT_SEQ_COLORFADE,  // カラーフェード（外部指定データ）
} ;

//-------------------------------------
/// ライトテーブル最大数
//=====================================
#define LIGHT_TABLE_MAX_SIZE  ( 16 )

//-------------------------------------
/// カラーフェード
//=====================================
enum {
  COLOR_FADE_SEQ_INIT,
  COLOR_FADE_SEQ_IN,
  COLOR_FADE_SEQ_OUT,
  COLOR_FADE_SEQ_END,
} ;


//-------------------------------------
/// アーカイブデータ
//=====================================
#define LIGHT_ARC_ID      ( ARCID_FIELD_LIGHT )
#define LIGHT_ARC_SEASON_NUM  ( PMSEASON_TOTAL )



#ifdef DEBUG_FIELD_LIGHT
// デバック管理シーケンス
enum{
  DEBUG_LIGHT_SEQ_LIGHT,
  DEBUG_LIGHT_SEQ_MATERIAL,
  DEBUG_LIGHT_SEQ_OTHER,

  DEBUG_LIGHT_SEQ_NUM,
};

// ライト管理項目数
enum {
  DEBUG_CONT_LIGHT00_FLAG,
  DEBUG_CONT_LIGHT00_RGB,
  DEBUG_CONT_LIGHT00_VEC,

  DEBUG_CONT_LIGHT01_FLAG,
  DEBUG_CONT_LIGHT01_RGB,
  DEBUG_CONT_LIGHT01_VEC,

  DEBUG_CONT_LIGHT02_FLAG,
  DEBUG_CONT_LIGHT02_RGB,
  DEBUG_CONT_LIGHT02_VEC,

  DEBUG_CONT_LIGHT03_FLAG,
  DEBUG_CONT_LIGHT03_RGB,
  DEBUG_CONT_LIGHT03_VEC,

  DEBUG_CONT_LIGHT_NUM,
} ;

// マテリアル管理
enum {
  DEBUG_CONT_MATERIAL_DEFFUSE,
  DEBUG_CONT_MATERIAL_AMBIENT,
  DEBUG_CONT_MATERIAL_SPECULAR,
  DEBUG_CONT_MATERIAL_EMISSION,

  DEBUG_CONT_MATERIAL_NUM,
};

// そのた管理
enum {
  DEBUG_CONT_OTHER_FOG,
  DEBUG_CONT_OTHER_BG,

  DEBUG_CONT_OTHER_NUM,
};

#define DEBUG_PRINT_X ( 10 )


#endif  // DEBUG_FIELD_LIGHT

//-----------------------------------------------------------------------------
/**
 *          構造体宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
/// ライト１データ
//=====================================
typedef struct {
  u32     endtime;
  u8      light_flag[4];
  GXRgb   light_color[4];
  VecFx16   light_vec[4];

  GXRgb   diffuse;
  GXRgb   ambient;
  GXRgb   specular;
  GXRgb   emission;
  GXRgb   fog_color;
  GXRgb   bg_color;

} LIGHT_DATA;

//-------------------------------------
/// RGBフェード
//=====================================
typedef struct {
  u16 r_start;
  s16 r_dist;
  u16 g_start;
  s16 g_dist;
  u16 b_start;
  s16 b_dist;
} RGB_FADE;


//-------------------------------------
/// 方向ベクトルフェード
//=====================================
typedef struct {
  VecFx32 start;  // 開始ベクトル
  VecFx32 normal; // 回転軸
  u16 way_dist; // 回転量
} VEC_FADE;


//-------------------------------------
/// ライトフェード
//=====================================
typedef struct {

  // フェードしない情報
  u32     endtime;
  u8      light_flag[4];

  RGB_FADE    light_color[4];
  VEC_FADE    light_vec[4];

  RGB_FADE    diffuse;
  RGB_FADE    ambient;
  RGB_FADE    specular;
  RGB_FADE    emission;
  RGB_FADE    fog_color;
  RGB_FADE    bg_color;

  u16       count;
  u16       count_max;
} LIGHT_FADE;


//-------------------------------------
/// カラーフェード
//=====================================
typedef struct {
  u16   seq;
  u16   insync;
  u16   outsync;
  GXRgb color;
} COLOR_FADE;


//-------------------------------------
/// フィールドライトシステム
//=====================================
struct _FIELD_LIGHT {
  u32   seq;  // シーケンス

  // GFLIBライトシステム
  GFL_G3D_LIGHTSET* p_liblight;

  // FOGシステム
  FIELD_FOG_WORK* p_fog;

  // データバッファ
  u32     data_num;     // データ数
  LIGHT_DATA data[LIGHT_TABLE_MAX_SIZE];        // データ
  u32     now_index;      // 今の反映インデックス
  u16     default_lightno;


  // 反映情報
  LIGHT_DATA  reflect_data;   // 反映データ
  u16     reflect_flag;   // 反映フラグ
  BOOL    change;       // データ設定フラグ

  // 反映フェード情報
  LIGHT_FADE  fade;

  // カラーフェード
  COLOR_FADE  color_fade;

  // 反映時間
  s32     time_second;

#ifdef DEBUG_FIELD_LIGHT
  BOOL  debug_flag;
  s16   debug_cont_seq;
  s16   debug_cont_select;
  s32   debug_print_req;

  u16   debug_rotate_xz;
  u16   debug_rotate_y;

  WORDSET*    p_debug_wordset;
  GFL_FONT*   p_debug_font;
  GFL_MSGDATA*  p_debug_msgdata;
  STRBUF*     p_debug_strbuff;
  STRBUF*     p_debug_strbuff_tmp;
#endif  // DEBUG_FIELD_LIGHT

};


//-----------------------------------------------------------------------------
/**
 *          プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
/// システム
//=====================================
static void FIELD_LIGHT_ReflectSub( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight );
static void FIELD_LIGHT_ForceReflect( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight );
static void FIELD_LIGHT_LoadData( FIELD_LIGHT* p_sys, u32 light_no, u32 heapID );
static void FIELD_LIGHT_LoadDataEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID );
static void FIELD_LIGHT_ReleaseData( FIELD_LIGHT* p_sys );
static s32  FIELD_LIGHT_SearchNowIndex( const FIELD_LIGHT* cp_sys, int rtc_second );


//-------------------------------------
/// RGBフェード
//=====================================
static void RGB_FADE_Init( RGB_FADE* p_wk, GXRgb start, GXRgb end );
static GXRgb RGB_FADE_Calc( const RGB_FADE* cp_wk, u16 count, u16 count_max );

//-------------------------------------
/// 方向ベクトルフェード
//=====================================
static void VEC_FADE_Init( VEC_FADE* p_wk, const VecFx16* cp_start, const VecFx16* cp_end );
static void VEC_FADE_Calc( const VEC_FADE* cp_wk, u16 count, u16 count_max, VecFx16* p_ans );

//-------------------------------------
/// ライトフェード
//=====================================
static void LIGHT_FADE_Init( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end );
static void LIGHT_FADE_InitEx( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end, u32 sync );
static void LIGHT_FADE_InitColor( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, GXRgb end_color, u32 sync );
static void LIGHT_FADE_Main( LIGHT_FADE* p_wk );
static BOOL LIGHT_FADE_IsFade( const LIGHT_FADE* cp_wk );
static void LIGHT_FADE_GetData( const LIGHT_FADE* cp_wk, LIGHT_DATA* p_data );


//-------------------------------------
/// カラーフェード
//=====================================
static void COLOR_FADE_Init( COLOR_FADE* p_wk, u16 insync, u16 outsync, GXRgb color );
static BOOL COLOR_FADE_Main( COLOR_FADE* p_wk, LIGHT_FADE* p_fade, const LIGHT_DATA* cp_refdata, const LIGHT_DATA* cp_nowdata );
static BOOL COLOR_FADE_IsFade( const COLOR_FADE* cp_wk );


#ifdef DEBUG_FIELD_LIGHT
//-------------------------------------
/// ライトデバック機能
//=====================================
static void DEBUG_LIGHT_ContLight( FIELD_LIGHT* p_wk );
static void DEBUG_LIGHT_ContMaterial( FIELD_LIGHT* p_wk );
static void DEBUG_LIGHT_ContOther( FIELD_LIGHT* p_wk );

static void DEBUG_LIGHT_PrintLight( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win );
static void DEBUG_LIGHT_PrintMaterial( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win );
static void DEBUG_LIGHT_PrintOther( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win );

static GXRgb DEBUG_LIGHT_ContRgb( GXRgb rgb );
static void DEBUG_LIGHT_InitContVec( FIELD_LIGHT* p_wk, const VecFx16* cp_vec );
static BOOL DEBUG_LIGHT_ContVec( FIELD_LIGHT* p_wk, VecFx16* p_vec );

static void DEBUG_LIGHT_SetWordsetRgb( FIELD_LIGHT* p_wk, u32 bufstart, GXRgb rgb );
static void DEBUG_LIGHT_SetWordsetVec( FIELD_LIGHT* p_wk, u32 bufstart, const VecFx16* cp_vec );
#endif // DEBUG_FIELD_LIGHT





//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  システム作成
 *
 *  @param  light_no    ライトナンバー
 *  @param  rtc_second    秒数
 *  @param  p_fog     フォグシステム
 *  @param  p_liblight    ライト管理システム
 *  @param  heapID      ヒープ
 *
 *  @return システムワーク
 */
//-----------------------------------------------------------------------------
FIELD_LIGHT* FIELD_LIGHT_Create( u32 light_no, int rtc_second, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight, u32 heapID )
{
  FIELD_LIGHT* p_sys;

  p_sys = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_LIGHT) );

  //  1/2で考える
  rtc_second /= 2;

  // ライト情報読み込み
  FIELD_LIGHT_LoadData( p_sys, light_no, heapID );

  // データ反映
  p_sys->reflect_flag = TRUE;

  // 初期情報を設定
  p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, rtc_second );

  // データ反映
  GFL_STD_MemCopy( &p_sys->data[p_sys->now_index], &p_sys->reflect_data, sizeof(LIGHT_DATA) );
  p_sys->change = TRUE;

  // フォグシステムを保存
  p_sys->p_fog = p_fog;

  // gflib
  p_sys->p_liblight = p_liblight;

  return p_sys;
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  システム破棄
 *
 *  @param  p_sys     システムワーク
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Delete( FIELD_LIGHT* p_sys )
{
  // データ破棄
  FIELD_LIGHT_ReleaseData( p_sys );

  GFL_HEAP_FreeMemory( p_sys );
}


//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  システムメイン
 *
 *  @param  p_sys     システムワーク
 *  @param  rtc_second    今のRTC時間（秒単位）
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Main( FIELD_LIGHT* p_sys, int rtc_second )
{
  int starttime;

#ifdef DEBUG_FIELD_LIGHT
  if( p_sys->debug_flag ){
    return;
  }
#endif

  //  1/2で考える
  rtc_second /= 2;

  // ライトデータの有無チェック
  if( p_sys->data_num == 0 ){
    return ;
  }

  // ライトデータ変更チェック
  switch( p_sys->seq ){
  // 通常
  case FIELD_LIGHT_SEQ_NORMAL:
    if( (p_sys->now_index - 1) < 0 ){
      starttime = 0;
    }else{
      starttime = p_sys->data[ p_sys->now_index-1 ].endtime;
    }

    //OS_TPrintf( "starttime %d endtime %d now %d\n", starttime, p_sys->data[ p_sys->now_index ].endtime, rtc_second );
    // 今のテーブルの範囲内じゃないかチェック
    if( (starttime > rtc_second) ||
      (p_sys->data[ p_sys->now_index ].endtime <= rtc_second) ){

      // 変更
      p_sys->now_index  = (p_sys->now_index + 1) % p_sys->data_num;

      // フェード設定
      LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->data[ p_sys->now_index ] );
    }
    break;

  // ライトフェード（外部指定データ）
  case FIELD_LIGHT_SEQ_COLORFADE: // カラーフェード（外部指定データ）
    // 色にする
    if( COLOR_FADE_Main( &p_sys->color_fade, &p_sys->fade, &p_sys->reflect_data, &p_sys->data[ p_sys->now_index ] ) ){
      p_sys->seq = FIELD_LIGHT_SEQ_NORMAL;
    }
    break;

  }

  // ライトフェード
  if( LIGHT_FADE_IsFade( &p_sys->fade ) ){
    LIGHT_FADE_Main( &p_sys->fade );
    LIGHT_FADE_GetData( &p_sys->fade, &p_sys->reflect_data );
    p_sys->change = TRUE;
  }

  // rtc時間を保存
  p_sys->time_second = rtc_second;

}

//----------------------------------------------------------------------------
/**
 *  @brief  データ反映関数
 *
 *  @param  p_sys
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Reflect( FIELD_LIGHT* p_sys, BOOL force )
{
  // データ設定処理へ
  if( p_sys->change || force ){
    FIELD_LIGHT_ReflectSub( p_sys, p_sys->p_fog, p_sys->p_liblight );
    p_sys->change = FALSE;
  }
}


//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  ライト情報の変更
 *
 *  @param  light_no    ライトナンバー
 *  @param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_Change( FIELD_LIGHT* p_sys, u32 light_no, u32 heapID )
{
  // ライト情報を再読み込み
  FIELD_LIGHT_ReleaseData( p_sys );
  FIELD_LIGHT_LoadData( p_sys, light_no, heapID );

  // 初期情報を設定
  p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, p_sys->time_second );

  // フェード開始
  LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->data[ p_sys->now_index ] );
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  ライト情報の変更　外部情報
 *
 *  @param  arcid     アークID
 *  @param  dataid    データID
 *  @param  sync      フェードシンク数（１以上）
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_ChangeEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, s32 sync, u32 heapID )
{
  // ライト情報を再読み込み
  FIELD_LIGHT_ReleaseData( p_sys );
  FIELD_LIGHT_LoadDataEx( p_sys, arcid, dataid, heapID );

  // 初期情報を設定
  p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, p_sys->time_second );

  // フェード開始
  LIGHT_FADE_InitEx( &p_sys->fade, &p_sys->reflect_data, &p_sys->data[ p_sys->now_index ], sync );
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  基本ライトに戻る（）
 *
 *  @param  p_sys
 *  @param  heapID
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_ReLoadDefault( FIELD_LIGHT* p_sys, u32 heapID )
{
  // ライト情報を再読み込み
  FIELD_LIGHT_ReleaseData( p_sys );
  FIELD_LIGHT_LoadData( p_sys, p_sys->default_lightno, heapID );

  // 初期情報を設定
  p_sys->now_index = FIELD_LIGHT_SearchNowIndex( p_sys, p_sys->time_second );

  // フェード開始
  LIGHT_FADE_Init( &p_sys->fade, &p_sys->reflect_data, &p_sys->data[ p_sys->now_index ] );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェード開始
 *
 *  @param  p_sys   システムワーク
 *  @param  color   色
 *  @param  insync    フェードインシンク数
 *  @param  outsync   フェードアウトシンク数
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_COLORFADE_Start( FIELD_LIGHT* p_sys, GXRgb color, u32 insync, u32 outsync )
{
  COLOR_FADE_Init( &p_sys->color_fade, insync, outsync, color );
  p_sys->seq = FIELD_LIGHT_SEQ_COLORFADE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェード完了まち
 *
 *  @param  cp_sys    システムワーク
 */
//-----------------------------------------------------------------------------
BOOL FIELD_LIGHT_COLORFADE_IsFade( const FIELD_LIGHT* cp_sys )
{
  return COLOR_FADE_IsFade( &cp_sys->color_fade );
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  反映フラグ設定
 *
 *  @param  p_sys   ワーク
 *  @param  flag    フラグ
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_SetReflect( FIELD_LIGHT* p_sys, BOOL flag )
{
  p_sys->reflect_flag = flag;
  if( p_sys->reflect_flag ){
    // データ反映
    p_sys->change = TRUE;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト  反映フラグを取得
 *
 *  @param  cp_sys    ワーク
 *
 *  @retval TRUE  反映
 *  @retval FALSE 反映しない
 */
//-----------------------------------------------------------------------------
BOOL FIELD_LIGHT_GetReflect( const FIELD_LIGHT* cp_sys )
{
  return cp_sys->reflect_flag;
}



//----------------------------------------------------------------------------
/**
 *  @brief  今が夜か取得
 *
 *  @param  cp_sys    システムワーク
 *
 *  @retval TRUE  夜
 *  @retval FALSE それ以外
 */
//-----------------------------------------------------------------------------
BOOL FIELD_LIGHT_GetNight( const FIELD_LIGHT* cp_sys )
{
  if( (cp_sys->time_second >= 34200) || (cp_sys->time_second < 7200) ){
    return TRUE;
  }
  return FALSE;
}


#ifdef DEBUG_FIELD_LIGHT
//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライトデバック  初期化
 *
 *  @param  p_sys   システムワーク
 *  @param  heapID    ヒープＩＤ
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_DEBUG_Init( FIELD_LIGHT* p_sys, HEAPID heapID )
{
  GF_ASSERT( !p_sys->p_debug_wordset );
  GF_ASSERT( !p_sys->p_debug_msgdata );

  // ワードセット作成
  p_sys->p_debug_wordset = WORDSET_Create( heapID );
  p_sys->p_debug_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_tomoya_dat, heapID );

  p_sys->p_debug_strbuff    = GFL_STR_CreateBuffer( 256, heapID );
  p_sys->p_debug_strbuff_tmp  = GFL_STR_CreateBuffer( 256, heapID );

  // フォントデータ
  p_sys->p_debug_font = GFL_FONT_Create(
    ARCID_FONT, NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  p_sys->debug_flag = TRUE;

  p_sys->debug_print_req = TRUE;

  GFL_UI_KEY_SetRepeatSpeed( 4,8 );
}

//----------------------------------------------------------------------------
/**
 *  @brief  フィールドライト　ワーク破棄
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_DEBUG_Exit( FIELD_LIGHT* p_sys )
{
  GF_ASSERT( p_sys->p_debug_wordset );
  GF_ASSERT( p_sys->p_debug_msgdata );


  // フォントデータ
  GFL_FONT_Delete( p_sys->p_debug_font );
  p_sys->p_debug_font = NULL;


  GFL_MSG_Delete( p_sys->p_debug_msgdata );
  p_sys->p_debug_msgdata = NULL;

  WORDSET_Delete( p_sys->p_debug_wordset );
  p_sys->p_debug_wordset = NULL;

  GFL_STR_DeleteBuffer( p_sys->p_debug_strbuff );
  p_sys->p_debug_strbuff = NULL;
  GFL_STR_DeleteBuffer( p_sys->p_debug_strbuff_tmp );
  p_sys->p_debug_strbuff_tmp = NULL;

  p_sys->debug_flag = FALSE;

  GFL_UI_KEY_SetRepeatSpeed( 8,15 );
}


//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報デバックコントロール
 *
 *  @param  p_sys システム
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_DEBUG_Control( FIELD_LIGHT* p_sys )
{
  p_sys->debug_print_req = FALSE;

  // LRでライト情報を交換
  if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_L ){

    if( p_sys->now_index > 0 ){
      p_sys->now_index --;
    }else{
      p_sys->now_index = p_sys->data_num-1;
    }
    p_sys->debug_print_req = TRUE;

    // データ反映
    GFL_STD_MemCopy( &p_sys->data[p_sys->now_index], &p_sys->reflect_data, sizeof(LIGHT_DATA) );
    p_sys->change = TRUE;

  }else if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_R ){

    p_sys->now_index = (p_sys->now_index + 1) % p_sys->data_num;
    p_sys->debug_print_req = TRUE;

    // データ反映
    GFL_STD_MemCopy( &p_sys->data[p_sys->now_index], &p_sys->reflect_data, sizeof(LIGHT_DATA) );
    p_sys->change = TRUE;
  }

  // タッチで、項目を変更
  {
    u32 x, y;
    if( GFL_UI_TP_GetPointTrg( &x, &y ) ){

      if( x<128 ){
        // 項目モドル
        if( p_sys->debug_cont_seq > 0 ){
          p_sys->debug_cont_seq --;
        }else{
          p_sys->debug_cont_seq  = DEBUG_LIGHT_SEQ_NUM-1;
        }
        p_sys->debug_print_req    = TRUE;
        p_sys->debug_cont_select  = 0;
      }else{
        // 項目ススム
        p_sys->debug_cont_seq   = (p_sys->debug_cont_seq+1) % DEBUG_LIGHT_SEQ_NUM;
        p_sys->debug_print_req    = TRUE;
        p_sys->debug_cont_select  = 0;
      }
    }
  }

  // ライトコントロール
  switch( p_sys->debug_cont_seq ){
  case DEBUG_LIGHT_SEQ_LIGHT:
    DEBUG_LIGHT_ContLight( p_sys );
    break;
  case DEBUG_LIGHT_SEQ_MATERIAL:
    DEBUG_LIGHT_ContMaterial( p_sys );
    break;
  case DEBUG_LIGHT_SEQ_OTHER:
    DEBUG_LIGHT_ContOther( p_sys );
    break;

  default:
    GF_ASSERT(0);
    break;
  }

  if( p_sys->change ){
    FIELD_LIGHT_ForceReflect( p_sys, p_sys->p_fog, p_sys->p_liblight );
    p_sys->change = FALSE;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  情報をビットマップウィンドウに表示
 *
 *  @param  cp_sys  システム
 *  @param  p_win ウィンドウ
 */
//-----------------------------------------------------------------------------
void FIELD_LIGHT_DEBUG_PrintData( FIELD_LIGHT* p_sys, GFL_BMPWIN* p_win )
{
  if( p_sys->debug_print_req ){
    // ビットマップクリア
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_win ), 15 );

    //描画
    switch( p_sys->debug_cont_seq ){
    case DEBUG_LIGHT_SEQ_LIGHT:
      DEBUG_LIGHT_PrintLight( p_sys, p_win );
      break;
    case DEBUG_LIGHT_SEQ_MATERIAL:
      DEBUG_LIGHT_PrintMaterial( p_sys, p_win );
      break;
    case DEBUG_LIGHT_SEQ_OTHER:
      DEBUG_LIGHT_PrintOther( p_sys, p_win );
      break;

    default:
      GF_ASSERT(0);
      break;
    }

  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライト管理処理
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_ContLight( FIELD_LIGHT* p_wk )
{
  u32 cont_light = 0;
  GXRgb change_rgb;
  BOOL vec_cont_init = FALSE;

  // 上下、選択項目を変える
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){

    if( p_wk->debug_cont_select > 0 ){
      p_wk->debug_cont_select --;
    }else{
      p_wk->debug_cont_select = DEBUG_CONT_LIGHT_NUM-1;
    }
    p_wk->debug_print_req = TRUE;

    // ベクトル管理方法の更新要請
    vec_cont_init = TRUE;

  }else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){

    p_wk->debug_cont_select = (p_wk->debug_cont_select + 1) % DEBUG_CONT_LIGHT_NUM;
    p_wk->debug_print_req = TRUE;

    // ベクトル管理方法の更新要請
    vec_cont_init = TRUE;
  }

  // 管理ライト判定
  cont_light = p_wk->debug_cont_select / 3;

  //  管理
  switch( p_wk->debug_cont_select ){
  case DEBUG_CONT_LIGHT00_FLAG:
  case DEBUG_CONT_LIGHT01_FLAG:
  case DEBUG_CONT_LIGHT02_FLAG:
  case DEBUG_CONT_LIGHT03_FLAG:
    if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_Y ){
      p_wk->reflect_data.light_flag[ cont_light ] ^= 1;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_LIGHT00_RGB:
  case DEBUG_CONT_LIGHT01_RGB:
  case DEBUG_CONT_LIGHT02_RGB:
  case DEBUG_CONT_LIGHT03_RGB:

    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.light_color[ cont_light ] );
    if( change_rgb != p_wk->reflect_data.light_color[ cont_light ] ){
      p_wk->reflect_data.light_color[ cont_light ] = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_LIGHT00_VEC:
  case DEBUG_CONT_LIGHT01_VEC:
  case DEBUG_CONT_LIGHT02_VEC:
  case DEBUG_CONT_LIGHT03_VEC:
    if( vec_cont_init ){
      DEBUG_LIGHT_InitContVec( p_wk, &p_wk->reflect_data.light_vec[ cont_light ] );
    }

    if( DEBUG_LIGHT_ContVec( p_wk, &p_wk->reflect_data.light_vec[ cont_light ] ) ){
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  マテリアル管理処理
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_ContMaterial( FIELD_LIGHT* p_wk )
{
  GXRgb change_rgb;

  // 上下、選択項目を変える
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){

    if( p_wk->debug_cont_select > 0 ){
      p_wk->debug_cont_select --;
    }else{
      p_wk->debug_cont_select = DEBUG_CONT_MATERIAL_NUM-1;
    }
    p_wk->debug_print_req = TRUE;

  }else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){

    p_wk->debug_cont_select = (p_wk->debug_cont_select + 1) % DEBUG_CONT_MATERIAL_NUM;
    p_wk->debug_print_req = TRUE;
  }

  //  管理
  switch( p_wk->debug_cont_select ){
  case DEBUG_CONT_MATERIAL_DEFFUSE:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.diffuse );
    if( change_rgb != p_wk->reflect_data.diffuse ){
      p_wk->reflect_data.diffuse = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_MATERIAL_AMBIENT:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.ambient );
    if( change_rgb != p_wk->reflect_data.ambient ){
      p_wk->reflect_data.ambient = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_MATERIAL_SPECULAR:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.specular );
    if( change_rgb != p_wk->reflect_data.specular ){
      p_wk->reflect_data.specular = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_MATERIAL_EMISSION:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.emission );
    if( change_rgb != p_wk->reflect_data.emission ){
      p_wk->reflect_data.emission = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  その他管理処理
 *
 *  @param  p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_ContOther( FIELD_LIGHT* p_wk )
{
  GXRgb change_rgb;

  // 上下、選択項目を変える
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){

    if( p_wk->debug_cont_select > 0 ){
      p_wk->debug_cont_select --;
    }else{
      p_wk->debug_cont_select = DEBUG_CONT_OTHER_NUM-1;
    }
    p_wk->debug_print_req = TRUE;

  }else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){

    p_wk->debug_cont_select = (p_wk->debug_cont_select + 1) % DEBUG_CONT_OTHER_NUM;
    p_wk->debug_print_req = TRUE;
  }

  //  管理
  switch( p_wk->debug_cont_select ){
  case DEBUG_CONT_OTHER_FOG:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.fog_color );
    if( change_rgb != p_wk->reflect_data.fog_color ){
      p_wk->reflect_data.fog_color = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  case DEBUG_CONT_OTHER_BG:
    change_rgb = DEBUG_LIGHT_ContRgb( p_wk->reflect_data.bg_color );
    if( change_rgb != p_wk->reflect_data.bg_color ){
      p_wk->reflect_data.bg_color = change_rgb;
      p_wk->change = TRUE;
      p_wk->debug_print_req = TRUE;
    }
    break;

  default:
    GF_ASSERT(0);
    break;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報の描画
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_PrintLight( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win )
{
  int i;

  //  毎回フレーム数を表示
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, 0, p_wk->reflect_data.endtime, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  // プリント
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_LIGHT_FRAME, p_wk->p_debug_strbuff_tmp );

  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 160, 0, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  // ライト情報を書き込む
  for( i=0; i<4; i++ ){

    // ライトナンバー
    WORDSET_RegisterNumber( p_wk->p_debug_wordset, 0, i, 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

    // ライトＯＮ・ＯＦＦ
    WORDSET_RegisterNumber( p_wk->p_debug_wordset, 1, p_wk->reflect_data.light_flag[i], 1, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );

    // RGB
    DEBUG_LIGHT_SetWordsetRgb( p_wk, 2, p_wk->reflect_data.light_color[i] );

    // 方向
    DEBUG_LIGHT_SetWordsetVec( p_wk, 5, &p_wk->reflect_data.light_vec[i] );

    // プリント
    GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_LIGHT_FLAG, p_wk->p_debug_strbuff_tmp );

    WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
    PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, i*(3*16), p_wk->p_debug_strbuff, p_wk->p_debug_font );

  }

  // カーソルの描画
  GFL_BMP_Fill( GFL_BMPWIN_GetBmp( p_win ), 1, p_wk->debug_cont_select*16+2, 8, 8, 1 );

  GFL_BMPWIN_TransVramCharacter( p_win );
}

//----------------------------------------------------------------------------
/**
 *  @brief  マテリアル情報の描画
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_PrintMaterial( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win )
{
  //  毎回フレーム数を表示
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, 0, p_wk->reflect_data.endtime, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_LIGHT_FRAME, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 72, p_wk->p_debug_strbuff, p_wk->p_debug_font );



  //  ディフューズ
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.diffuse );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_DIFFUSE, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 0, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  //  アンビエント
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.ambient );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_AMBIENT, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 16, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  //  スペキュラー
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.specular );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_SPECULAR, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 32, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  //  エミッション
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.emission );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_EMISSION, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 48, p_wk->p_debug_strbuff, p_wk->p_debug_font );


  // カーソル描画
  GFL_BMP_Fill( GFL_BMPWIN_GetBmp( p_win ), 1, p_wk->debug_cont_select*16+2, 8, 8, 1 );

  GFL_BMPWIN_TransVramCharacter( p_win );
}

//----------------------------------------------------------------------------
/**
 *  @brief  その他情報の描画
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_PrintOther( FIELD_LIGHT* p_wk, GFL_BMPWIN* p_win )
{
  //  毎回フレーム数を表示
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, 0, p_wk->reflect_data.endtime, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_LIGHT_FRAME, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 72, p_wk->p_debug_strbuff, p_wk->p_debug_font );



  //  フォグ
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.fog_color );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_FOG, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 0, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  // 背面
  DEBUG_LIGHT_SetWordsetRgb( p_wk, 0, p_wk->reflect_data.bg_color );
  GFL_MSG_GetString( p_wk->p_debug_msgdata, D_TOMOYA_BG, p_wk->p_debug_strbuff_tmp );
  WORDSET_ExpandStr( p_wk->p_debug_wordset, p_wk->p_debug_strbuff, p_wk->p_debug_strbuff_tmp );
  PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), DEBUG_PRINT_X, 16, p_wk->p_debug_strbuff, p_wk->p_debug_font );

  // カーソル描画
  GFL_BMP_Fill( GFL_BMPWIN_GetBmp( p_win ), 1, p_wk->debug_cont_select*16+2, 8, 8, 1 );

  GFL_BMPWIN_TransVramCharacter( p_win );
}


//----------------------------------------------------------------------------
/**
 *  @brief  色管理
 *
 *  @param  rgb   色
 *
 *  @return 色情報
 */
//-----------------------------------------------------------------------------
static GXRgb DEBUG_LIGHT_ContRgb( GXRgb rgb )
{
  u8 r,g,b;

  r = (rgb & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT;
  g = (rgb & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT;
  b = (rgb & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT;

  // R
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_Y ){
    r = (r+1) % 32;
  }
  // G
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_X ){
    g = (g+1) % 32;
  }
  // B
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A ){
    b = (b+1) % 32;
  }

  return GX_RGB( r, g, b );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ベクトル管理開始処理
 *
 *  @param  p_wk  ワーク
 *  @param  cp_vec  ベクトル
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_InitContVec( FIELD_LIGHT* p_wk, const VecFx16* cp_vec )
{
  fx32 xz_dist;

  p_wk->debug_rotate_xz = FX_Atan2Idx( cp_vec->x, cp_vec->z );
  xz_dist   = FX_Sqrt( FX_Mul( cp_vec->z, cp_vec->z ) + FX_Mul( cp_vec->x, cp_vec->x ) );
  p_wk->debug_rotate_y  = FX_Atan2Idx( cp_vec->y, xz_dist );

}

//----------------------------------------------------------------------------
/**
 *  @brief  ベクトル管理
 *
 *  @param  p_vec ベクトル
 */
//-----------------------------------------------------------------------------
#define DEBUG_LIGHT_CONT_VEC_ROTATE_ADD ( 182 )
static BOOL DEBUG_LIGHT_ContVec( FIELD_LIGHT* p_wk, VecFx16* p_vec )
{
  BOOL change = FALSE;
  MtxFx33 mtx_xz, mtx_y;

  // 上下左右回転
  // X 上
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_X ){
    // Y方向は、９０度～２７０度までしか動かさない
    if( p_wk->debug_rotate_y >= ((0xffff/4) + DEBUG_LIGHT_CONT_VEC_ROTATE_ADD) ){
      p_wk->debug_rotate_y -= DEBUG_LIGHT_CONT_VEC_ROTATE_ADD;
    }else{
      p_wk->debug_rotate_y = (0xffff/4);
    }
    change = TRUE;
  }
  // B 下
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B ){

    // Y方向は、１８０度～３６０度までしか動かさない
    if( (p_wk->debug_rotate_y+DEBUG_LIGHT_CONT_VEC_ROTATE_ADD) < ((0xffff/4)*3)  ){
      p_wk->debug_rotate_y += DEBUG_LIGHT_CONT_VEC_ROTATE_ADD;
    }else{
      p_wk->debug_rotate_y = ((0xffff/4)*3);
    }

    change = TRUE;
  }
  // Y 左
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_Y ){

    p_wk->debug_rotate_xz -= DEBUG_LIGHT_CONT_VEC_ROTATE_ADD;
    change = TRUE;
  }
  // A 右
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A ){

    p_wk->debug_rotate_xz += DEBUG_LIGHT_CONT_VEC_ROTATE_ADD;
    change = TRUE;
  }

  if( change ){

    p_vec->y = FX_Mul( FX_SinIdx( p_wk->debug_rotate_y ), FX32_ONE );
    p_vec->x = FX_Mul( FX_CosIdx( p_wk->debug_rotate_y ), FX32_ONE );
    p_vec->z = FX_Mul( FX_CosIdx( p_wk->debug_rotate_xz ), p_vec->x );
    p_vec->x = FX_Mul( FX_SinIdx( p_wk->debug_rotate_xz ), p_vec->x );
  }

  return change;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ワードセットにＲＧＢを入れる
 *
 *  @param  p_wk    ワーク
 *  @param  bufstart  バッファ開始インデックス
 *  @param  rgb     ＲＧＢ
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_SetWordsetRgb( FIELD_LIGHT* p_wk, u32 bufstart, GXRgb rgb )
{
  u8 r,g,b;

  r = (rgb & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT;
  g = (rgb & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT;
  b = (rgb & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT;

  // RGBを設定
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+0, r, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+1, g, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+2, b, 2, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ワードセットにベクトルを入れる
 *
 *  @param  p_wk    ワーク
 *  @param  bufstart  バッファ開始インデックス
 *  @param  cp_vec    ベクトル
 */
//-----------------------------------------------------------------------------
static void DEBUG_LIGHT_SetWordsetVec( FIELD_LIGHT* p_wk, u32 bufstart, const VecFx16* cp_vec )
{
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+0, cp_vec->x, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+1, cp_vec->y, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
  WORDSET_RegisterNumber( p_wk->p_debug_wordset, bufstart+2, cp_vec->z, 5, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
}



#endif // DEBUG_FIELD_LIGHT






//-----------------------------------------------------------------------------
/**
 *      プライベート関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報の設定
 *
 *  @param  cp_sys  システムワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_ReflectSub( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight )
{
  int i;
  VecFx16 dummy_vec = {0};
  u16   dummy_col = 0;

  if( cp_sys->reflect_flag ){

    for( i=0; i<4; i++ ){
      if( cp_sys->reflect_data.light_flag[i] ){
        GFL_G3D_LIGHT_SetVec( p_liblight, i, (VecFx16*)&cp_sys->reflect_data.light_vec[i] );
        GFL_G3D_LIGHT_SetColor( p_liblight, i, (u16*)&cp_sys->reflect_data.light_color[i] );
      }else{

        GFL_G3D_LIGHT_SetVec( p_liblight, i, &dummy_vec );
        GFL_G3D_LIGHT_SetColor( p_liblight, i, &dummy_col );
      }
    }

    NNS_G3dGlbMaterialColorDiffAmb( cp_sys->reflect_data.diffuse,
        cp_sys->reflect_data.ambient, TRUE );

    NNS_G3dGlbMaterialColorSpecEmi( cp_sys->reflect_data.specular,
        cp_sys->reflect_data.emission, FALSE );

    FIELD_FOG_SetColorRgb( p_fog, cp_sys->reflect_data.fog_color );

    G3X_SetClearColor(cp_sys->reflect_data.bg_color,31,0x7fff,0,FALSE);
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  強制ライト反映
 *
 *  @param  cp_sys      システムワーク
 *  @param  p_fog     フォグワーク
 *  @param  p_liblight    ライトワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_ForceReflect( const FIELD_LIGHT* cp_sys, FIELD_FOG_WORK* p_fog, GFL_G3D_LIGHTSET* p_liblight )
{
  int i;
  VecFx16 dummy_vec = {0};
  u16   dummy_col = 0;

  for( i=0; i<4; i++ ){
    if( cp_sys->reflect_data.light_flag[i] ){
      GFL_G3D_LIGHT_SetVec( p_liblight, i, (VecFx16*)&cp_sys->reflect_data.light_vec[i] );
      GFL_G3D_LIGHT_SetColor( p_liblight, i, (u16*)&cp_sys->reflect_data.light_color[i] );
    }else{

      GFL_G3D_LIGHT_SetVec( p_liblight, i, &dummy_vec );
      GFL_G3D_LIGHT_SetColor( p_liblight, i, &dummy_col );
    }
  }

  NNS_G3dGlbMaterialColorDiffAmb( cp_sys->reflect_data.diffuse,
      cp_sys->reflect_data.ambient, TRUE );

  NNS_G3dGlbMaterialColorSpecEmi( cp_sys->reflect_data.specular,
      cp_sys->reflect_data.emission, FALSE );

  FIELD_FOG_SetColorRgb( p_fog, cp_sys->reflect_data.fog_color );

  G3X_SetClearColor(cp_sys->reflect_data.bg_color,31,0x7fff,0,FALSE);
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報の読み込み処理
 *
 *  @param  p_sys   システムワーク
 *  @param  light_no  ライトナンバー
 *  @param  heapID    ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_LoadData( FIELD_LIGHT* p_sys, u32 light_no, u32 heapID )
{
  // 基本季節・ライトナンバー設定
  p_sys->default_lightno  = light_no;

  FIELD_LIGHT_LoadDataEx( p_sys, LIGHT_ARC_ID, light_no, heapID );


}

//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報の読み込み処理
 *
 *  @param  p_sys     システムワーク
 *  @param  arcid     アーカイブID
 *  @param  dataid      データID
 *  @param  heapID      ヒープID
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_LoadDataEx( FIELD_LIGHT* p_sys, u32 arcid, u32 dataid, u32 heapID )
{
  u32 size;
  int i, j;

  size = GFL_ARC_GetDataSize( arcid, dataid );
  p_sys->data_num = size / sizeof(LIGHT_DATA);
  GF_ASSERT( p_sys->data_num < LIGHT_TABLE_MAX_SIZE );

  GFL_ARC_LoadData( p_sys->data, arcid, dataid );


  // 方向ベクトルの値を一応単位ベクトルにする。
  for( i=0; i<p_sys->data_num; i++ ){

    for( j=0; j<4; j++ ){
      VEC_Fx16Normalize( &p_sys->data[i].light_vec[j], &p_sys->data[i].light_vec[j] );
    }

#ifdef DEBUG_LIGHT_AUTO
    // 時間の部分を上書き
    p_sys->data[i].endtime = sc_DEBUG_LIGHT_AUTO_END_TIME[i];
#endif

#if 0
    // データのデバック表示
    OS_TPrintf( "data number %d\n", i );
    OS_TPrintf( "endtime  %d\n", p_sys->data[i].endtime );
    for( j=0; j<4; j++ ){
      OS_TPrintf( "light_flag %d\n", p_sys->data[i].light_flag[i] );
      OS_TPrintf( "light_color r=%d g=%d b=%d\n",
          (p_sys->data[i].light_color[i] & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
          (p_sys->data[i].light_color[i] & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
          (p_sys->data[i].light_color[i] & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );
      OS_TPrintf( "light_vec x=0x%x y=0x%x z=0x%x\n",
          p_sys->data[i].light_vec[i].x,
          p_sys->data[i].light_vec[i].y,
          p_sys->data[i].light_vec[i].z );
    }

    OS_TPrintf( "diffuse r=%d g=%d b=%d\n",
        (p_sys->data[i].diffuse & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
        (p_sys->data[i].diffuse & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
        (p_sys->data[i].diffuse & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );

    OS_TPrintf( "ambient r=%d g=%d b=%d\n",
        (p_sys->data[i].ambient & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
        (p_sys->data[i].ambient & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
        (p_sys->data[i].ambient & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );

    OS_TPrintf( "specular r=%d g=%d b=%d\n",
        (p_sys->data[i].specular & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
        (p_sys->data[i].specular & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
        (p_sys->data[i].specular & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );

    OS_TPrintf( "emission r=%d g=%d b=%d\n",
        (p_sys->data[i].emission & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
        (p_sys->data[i].emission & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
        (p_sys->data[i].emission & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );

    OS_TPrintf( "fog_color r=%d g=%d b=%d\n",
        (p_sys->data[i].fog_color & GX_RGB_R_MASK)>>GX_RGB_R_SHIFT,
        (p_sys->data[i].fog_color & GX_RGB_G_MASK)>>GX_RGB_G_SHIFT,
        (p_sys->data[i].fog_color & GX_RGB_B_MASK)>>GX_RGB_B_SHIFT );
#endif
  }

}

//----------------------------------------------------------------------------
/**
 *  @brief  ライト情報の破棄　処理
 *
 *  @param  p_sys   システムワーク
 */
//-----------------------------------------------------------------------------
static void FIELD_LIGHT_ReleaseData( FIELD_LIGHT* p_sys )
{
}

//----------------------------------------------------------------------------
/**
 *  @brief  rtc秒のときのデータインデックスを取得
 *
 *  @param  cp_sys      ワーク
 *  @param  rtc_second    rtc秒
 *
 *  @return データインデックス
 */
//-----------------------------------------------------------------------------
static s32  FIELD_LIGHT_SearchNowIndex( const FIELD_LIGHT* cp_sys, int rtc_second )
{
  int i;

  for( i=0; i<cp_sys->data_num; i++ ){

    if( cp_sys->data[i].endtime > rtc_second ){
      return i;
    }
  }

  // ありえない
  GF_ASSERT( 0 );
  return 0;
}


//----------------------------------------------------------------------------
/**
 *  @brief  RGBフェード 初期化
 *
 *  @param  p_wk    ワーク
 *  @param  start   開始色
 *  @param  end     終了色
 */
//-----------------------------------------------------------------------------
static void RGB_FADE_Init( RGB_FADE* p_wk, GXRgb start, GXRgb end )
{
  p_wk->r_start   = (start & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT;
  p_wk->r_dist    = ((end & GX_RGB_R_MASK )>> GX_RGB_R_SHIFT) - p_wk->r_start;
  p_wk->g_start   = (start & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT;
  p_wk->g_dist    = ((end & GX_RGB_G_MASK )>> GX_RGB_G_SHIFT) - p_wk->g_start;
  p_wk->b_start   = (start & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT;
  p_wk->b_dist    = ((end & GX_RGB_B_MASK )>> GX_RGB_B_SHIFT) - p_wk->b_start;
}

//----------------------------------------------------------------------------
/**
 *  @brief  RGBフェード 計算処理
 *
 *  @param  cp_wk     ワーク
 *  @param  count     カウント値
 *  @param  count_max   カウント最大値
 *
 *  @return RGBカラー
 */
//-----------------------------------------------------------------------------
static GXRgb RGB_FADE_Calc( const RGB_FADE* cp_wk, u16 count, u16 count_max )
{
  u8 r, g, b;

  r = (cp_wk->r_dist * count) / count_max;
  r += cp_wk->r_start;
  g = (cp_wk->g_dist * count) / count_max;
  g += cp_wk->g_start;
  b = (cp_wk->b_dist * count) / count_max;
  b += cp_wk->b_start;

  return GX_RGB( r, g, b );
}

//----------------------------------------------------------------------------
/**
 *  @brief  方向ベクトルフェード  初期化
 *
 *  @param  p_wk      ワーク
 *  @param  cp_start    開始
 *  @param  cp_end      終了
 */
//-----------------------------------------------------------------------------
static void VEC_FADE_Init( VEC_FADE* p_wk, const VecFx16* cp_start, const VecFx16* cp_end )
{
  fx32 cos;
  VecFx16 normal;

  VEC_Set( &p_wk->start, cp_start->x, cp_start->y, cp_start->z );
  VEC_Fx16CrossProduct( cp_start, cp_end, &normal );
  VEC_Set( &p_wk->normal, normal.x, normal.y, normal.z );
  cos       = VEC_Fx16DotProduct( cp_start, cp_end );
  p_wk->way_dist  = FX_AcosIdx( cos );
}

//----------------------------------------------------------------------------
/**
 *  @brief  方向ベクトルのフェードを計算
 *
 *  @param  cp_wk     ワーク
 *  @param  count     カウント値
 *  @param  count_max   カウント最大値
 *  @param  p_ans     計算結果
 */
//-----------------------------------------------------------------------------
static void VEC_FADE_Calc( const VEC_FADE* cp_wk, u16 count, u16 count_max, VecFx16* p_ans )
{
  u16 rotate;
  MtxFx43 mtx;
  VecFx32 way;

  rotate = (cp_wk->way_dist * count) / count_max;
  MTX_RotAxis43( &mtx, &cp_wk->normal, FX_SinIdx( rotate ), FX_CosIdx( rotate ) );

  MTX_MultVec43( &cp_wk->start, &mtx, &way );
  VEC_Normalize( &way, &way );

  VEC_Fx16Set( p_ans, way.x, way.y, way.z );
}


//----------------------------------------------------------------------------
/**
 *  @brief  ライトフェード処理  初期化
 *
 *  @param  p_wk      ワーク
 *  @param  cp_start    開始ライト情報
 *  @param  cp_end      終了ライト情報
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_Init( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end )
{
  LIGHT_FADE_InitEx( p_wk, cp_start, cp_end, LIGHT_FADE_COUNT_MAX );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライトフェード処理  初期化  シンク指定
 *
 *  @param  p_wk      ワーク
 *  @param  cp_start    開始ライト情報
 *  @param  cp_end      終了ライト情報
 *  @param  sync      使用シンク数
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_InitEx( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, const LIGHT_DATA* cp_end, u32 sync )
{
  int i;

  p_wk->count     = 0;
  p_wk->count_max   = sync;


  // フェードしない情報も保存
  p_wk->endtime = cp_end->endtime;

  for( i=0; i<4; i++ ){

    RGB_FADE_Init( &p_wk->light_color[i], cp_start->light_color[i], cp_end->light_color[i] );
    VEC_FADE_Init( &p_wk->light_vec[i], &cp_start->light_vec[i], &cp_end->light_vec[i] );

    p_wk->light_flag[i] = cp_end->light_flag[i];
  }

  RGB_FADE_Init( &p_wk->diffuse, cp_start->diffuse, cp_end->diffuse );
  RGB_FADE_Init( &p_wk->ambient, cp_start->ambient, cp_end->ambient );
  RGB_FADE_Init( &p_wk->specular, cp_start->specular, cp_end->specular );
  RGB_FADE_Init( &p_wk->emission, cp_start->emission, cp_end->emission );
  RGB_FADE_Init( &p_wk->fog_color, cp_start->fog_color, cp_end->fog_color );
  RGB_FADE_Init( &p_wk->bg_color, cp_start->bg_color, cp_end->bg_color );
}

//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェード
 *
 *  @param  p_wk    ワーク
 *  @param  cp_start  開始ライトデータ
 *  @param  end_color 終了色情報
 *  @param  sync    かけるシンク数
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_InitColor( LIGHT_FADE* p_wk, const LIGHT_DATA* cp_start, GXRgb end_color, u32 sync )
{
  int i;
  p_wk->count     = 0;
  p_wk->count_max   = sync;


  // フェードしない情報も保存
  p_wk->endtime = cp_start->endtime;

  for( i=0; i<4; i++ ){

    RGB_FADE_Init( &p_wk->light_color[i], cp_start->light_color[i], end_color );
    VEC_FADE_Init( &p_wk->light_vec[i], &cp_start->light_vec[i], &cp_start->light_vec[i] );

    p_wk->light_flag[i] = cp_start->light_flag[i];
  }

  RGB_FADE_Init( &p_wk->diffuse, cp_start->diffuse, end_color );
  RGB_FADE_Init( &p_wk->ambient, cp_start->ambient, end_color );
  RGB_FADE_Init( &p_wk->specular, cp_start->specular, end_color );
  RGB_FADE_Init( &p_wk->emission, cp_start->emission, end_color );
  RGB_FADE_Init( &p_wk->fog_color, cp_start->fog_color, cp_start->fog_color );
  RGB_FADE_Init( &p_wk->bg_color, cp_start->bg_color, cp_start->bg_color );
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライトフェード処理  メイン
 *
 *  @param  p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_Main( LIGHT_FADE* p_wk )
{
  if( p_wk->count < p_wk->count_max ){
    p_wk->count ++;
  }
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライトフェード処理  フェード中チェック
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  フェード中
 *  @retval FALSE フェードしてない
 */
//-----------------------------------------------------------------------------
static BOOL LIGHT_FADE_IsFade( const LIGHT_FADE* cp_wk )
{
  if( cp_wk->count >= cp_wk->count_max ){
    return FALSE;
  }
  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  ライトフェード処理  ライトデータ取得
 *
 *  @param  cp_wk   ワーク
 *  @param  p_data    ライトデータ格納先
 */
//-----------------------------------------------------------------------------
static void LIGHT_FADE_GetData( const LIGHT_FADE* cp_wk, LIGHT_DATA* p_data )
{
  int i;

  p_data->endtime   = cp_wk->endtime;

  for( i=0; i<4; i++ ){
    p_data->light_color[i]  = RGB_FADE_Calc( &cp_wk->light_color[i], cp_wk->count, cp_wk->count_max );
    VEC_FADE_Calc( &cp_wk->light_vec[i], cp_wk->count, cp_wk->count_max, &p_data->light_vec[i] );

    p_data->light_flag[i] = cp_wk->light_flag[i];
  }

  p_data->diffuse   = RGB_FADE_Calc( &cp_wk->diffuse, cp_wk->count, cp_wk->count_max );
  p_data->ambient   = RGB_FADE_Calc( &cp_wk->ambient, cp_wk->count, cp_wk->count_max );
  p_data->specular  = RGB_FADE_Calc( &cp_wk->specular, cp_wk->count, cp_wk->count_max );
  p_data->emission  = RGB_FADE_Calc( &cp_wk->emission, cp_wk->count, cp_wk->count_max );
  p_data->fog_color = RGB_FADE_Calc( &cp_wk->fog_color, cp_wk->count, cp_wk->count_max );
  p_data->bg_color  = RGB_FADE_Calc( &cp_wk->bg_color, cp_wk->count, cp_wk->count_max );

}


//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェード開始
 *
 *  @param  p_wk    ワーク
 *  @param  insync    インシンク
 *  @param  outsync   アウトシンク
 *  @param  color   色
 */
//-----------------------------------------------------------------------------
static void COLOR_FADE_Init( COLOR_FADE* p_wk, u16 insync, u16 outsync, GXRgb color )
{
  p_wk->seq   = COLOR_FADE_SEQ_INIT;
  p_wk->insync  = insync;
  p_wk->outsync = outsync;
  p_wk->color   = color;
}

//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェードメイン
 *
 *  @param  p_wk    ワーク
 *  @param  p_fade    フェードシステム
 *  @param  cp_refdata  反映してるのライトデータ
 *  @param  cp_nowdata  今のライトデータ
 *
 *  @retval TRUE  フェード完了
 *  @retval FALSE フェード中
 */
//-----------------------------------------------------------------------------
static BOOL COLOR_FADE_Main( COLOR_FADE* p_wk, LIGHT_FADE* p_fade, const LIGHT_DATA* cp_refdata, const LIGHT_DATA* cp_nowdata )
{
  switch( p_wk->seq ){
  case COLOR_FADE_SEQ_INIT:
    LIGHT_FADE_InitColor( p_fade, cp_refdata, p_wk->color, p_wk->insync );
    p_wk->seq = COLOR_FADE_SEQ_IN;
    break;

  case COLOR_FADE_SEQ_IN:
    if( !LIGHT_FADE_IsFade( p_fade ) ){
      LIGHT_FADE_InitEx( p_fade, cp_refdata, cp_nowdata, p_wk->outsync );
      p_wk->seq = COLOR_FADE_SEQ_OUT;
    }
    break;

  case COLOR_FADE_SEQ_OUT:
    if( !LIGHT_FADE_IsFade( p_fade ) ){
      p_wk->seq = COLOR_FADE_SEQ_END;
    }
    break;

  case COLOR_FADE_SEQ_END:
    return TRUE;

  default:
    GF_ASSERT(0);
    break;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *  @brief  カラーフェード完了待ち
 *
 *  @param  cp_wk ワーク
 *
 *  @retval TRUE  フェード中
 *  @retval FALSE フェード完了
 */
//-----------------------------------------------------------------------------
static BOOL COLOR_FADE_IsFade( const COLOR_FADE* cp_wk )
{
  if( cp_wk->seq == COLOR_FADE_SEQ_END ){
    return FALSE;
  }
  return TRUE;
}




