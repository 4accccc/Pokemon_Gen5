//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_bbd_color.c
 *	@brief  ビルボード　カラー　データ
 *	@author	tomoya takahashi
 *	@date		2010.01.05
 *
 *	モジュール名：FLD_BBD_COLOR
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "field_bbd_color.h"

#include "arc/arc_def.h"
#include "arc/fieldmap/area_map_mmdl_color.naix"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------


//-------------------------------------
///	
//=====================================
typedef struct 
{
  fx32 norm_x;
  fx32 norm_y;
  fx32 norm_z;

  u16 def_r;
  u16 def_g;
  u16 def_b;

  u16 amb_r;
  u16 amb_g;
  u16 amb_b;

  u16 spq_r;
  u16 spq_g;
  u16 spq_b;

  u16 emi_r;
  u16 emi_g;
  u16 emi_b;

} FLD_BBD_COLOER_DATA;


//-------------------------------------
///	ビルボードカラー設定システム
//=====================================
struct _FLD_BBD_COLOR 
{
  ARCHANDLE* p_handle;
  FLD_BBD_COLOER_DATA buff;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------



//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラー　管理システム生成
 *
 *	@param	heapID    ヒープID
 *
 *	@return システム
 */
//-----------------------------------------------------------------------------
FLD_BBD_COLOR* FLD_BBD_COLOR_Create( HEAPID heapID )
{
  FLD_BBD_COLOR* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_BBD_COLOR) );

  p_wk->p_handle = GFL_ARC_OpenDataHandle( ARCID_FLD_BBD_COLOR, heapID );
  
  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラー　管理システム破棄
 *
 *	@param	p_wk    ワーク
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_Delete( FLD_BBD_COLOR* p_wk )
{
  GFL_ARC_CloseDataHandle( p_wk->p_handle );
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラー　管理システム  読み込み
 *
 *	@param	p_wk    ワーク
 *	@param	idx     インデックス
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_Load( FLD_BBD_COLOR* p_wk, u32 idx )
{
  GFL_ARC_LoadDataByHandle( p_wk->p_handle, idx, &p_wk->buff );

  // 読み込み情報の出力
  TOMOYA_Printf( "Normal x[0x%x] y[0x%x] z[0x%x]\n", p_wk->buff.norm_x, p_wk->buff.norm_y, p_wk->buff.norm_z );
  TOMOYA_Printf( "def r[%d] g[%d] b[%d]\n", p_wk->buff.def_r, p_wk->buff.def_g, p_wk->buff.def_b );
  TOMOYA_Printf( "amb r[%d] g[%d] b[%d]\n", p_wk->buff.amb_r, p_wk->buff.amb_g, p_wk->buff.amb_b );
  TOMOYA_Printf( "spc r[%d] g[%d] b[%d]\n", p_wk->buff.spq_r, p_wk->buff.spq_g, p_wk->buff.spq_b );
  TOMOYA_Printf( "emi r[%d] g[%d] b[%d]\n", p_wk->buff.emi_r, p_wk->buff.emi_g, p_wk->buff.emi_b );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラー　管理システム  データ設定
 *
 *	@param	cp_wk        ワーク
 *	@param	p_bbdsys    設定先ビルボードシステム
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_SetData( const FLD_BBD_COLOR* cp_wk, GFL_BBD_SYS* p_bbdsys )
{
  VecFx16 n;
  GXRgb def;
  GXRgb amb;
  GXRgb spq;
  GXRgb emi;
  
  // 各種データ設定
  {
    // 法線
    n.x = cp_wk->buff.norm_x; 
    n.y = cp_wk->buff.norm_y;
    n.z = cp_wk->buff.norm_z; 
    VEC_Fx16Normalize( &n, &n );
    GFL_BBD_SetUseCustomVecN( p_bbdsys, &n );

    // ディフューズ
    def = GX_RGB( cp_wk->buff.def_r, cp_wk->buff.def_g, cp_wk->buff.def_b );
    GFL_BBD_SetDiffuse( p_bbdsys, &def );

    // アンビエント 
    amb = GX_RGB( cp_wk->buff.amb_r, cp_wk->buff.amb_g, cp_wk->buff.amb_b );
    GFL_BBD_SetAmbient( p_bbdsys, &amb );

    // スペキュラー
    spq = GX_RGB( cp_wk->buff.spq_r, cp_wk->buff.spq_g, cp_wk->buff.spq_b );
    GFL_BBD_SetSpecular( p_bbdsys, &spq );

    // エミッション
    emi = GX_RGB( cp_wk->buff.emi_r, cp_wk->buff.emi_g, cp_wk->buff.emi_b );
    GFL_BBD_SetEmission( p_bbdsys, &emi );
  }
}



#ifdef DEBUG_BBD_COLOR_CONTROL

#include "message.naix"

#include "print/wordset.h"
#include "print/gf_font.h"
#include "print/printsys.h"

#include "msg/msg_d_tomoya.h"

#include "font/font.naix"


enum
{
  DEBUG_BBD_COLOR_CONT_SELECT_NORMAL,
  DEBUG_BBD_COLOR_CONT_SELECT_DEF,
  DEBUG_BBD_COLOR_CONT_SELECT_AMB,
  DEBUG_BBD_COLOR_CONT_SELECT_SPQ,
  DEBUG_BBD_COLOR_CONT_SELECT_EMI,

  DEBUG_BBD_COLOR_CONT_SELECT_MAX,
};

//-------------------------------------
///	デバック用データ
//=====================================
typedef struct 
{
  s16   cont_select;
  u16   print_req;

  VecFx16 normal;
  u8 def_r;
  u8 def_g;
  u8 def_b;
  u8 amb_r;
  u8 amb_g;
  u8 amb_b;
  u8 spq_r;
  u8 spq_g;
  u8 spq_b;
  u8 emi_r;
  u8 emi_g;
  u8 emi_b;

  u16   rotate_xz;
  u16   rotate_y;

  GFL_BBD_SYS* bbd_sys;

  WORDSET*    p_wordset;
  GFL_FONT*   p_font;
  GFL_MSGDATA*  p_msgdata;
  STRBUF*     p_strbuff;
  STRBUF*     p_strbuff_tmp;

} DEBUG_BBD_COLOR_CONTROL_WORK;

static DEBUG_BBD_COLOR_CONTROL_WORK* p_DEBUG_BBD_COLOR_CONTROL_WK = NULL;

// プロトタイプ
static void DEBUG_BBD_COLOR_InitContVec( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, const VecFx16* cp_vec );
static BOOL DEBUG_BBD_COLOR_ContVec( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, VecFx16* p_vec );
static BOOL DEBUG_BBD_COLOR_ContRgb( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, u8* r, u8* g, u8* b );


//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラーの調整　開始
 *
 *	@param	heapID  ヒープID
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_DEBUG_Init( GFL_BBD_SYS* bbd_sys, const FLD_BBD_COLOR* cp_data, HEAPID heapID )
{
  GF_ASSERT( p_DEBUG_BBD_COLOR_CONTROL_WK == NULL );

  // メモリ確保
  p_DEBUG_BBD_COLOR_CONTROL_WK = GFL_HEAP_AllocClearMemory( heapID, sizeof(DEBUG_BBD_COLOR_CONTROL_WORK) );


  // ワードセット作成
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset = WORDSET_Create( heapID );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_tomoya_dat, heapID );

  p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff    = GFL_STR_CreateBuffer( 256, heapID );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp  = GFL_STR_CreateBuffer( 256, heapID );

  // フォントデータ
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_font = GFL_FONT_Create(
    ARCID_FONT, NARC_font_large_gftr,
    GFL_FONT_LOADTYPE_FILE, FALSE, heapID );

  p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;

  // BBDSYS保存
  p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys = bbd_sys;

  // BBDSYSの情報を取得
  {
    // 法線
    p_DEBUG_BBD_COLOR_CONTROL_WK->normal.x = cp_data->buff.norm_x;
    p_DEBUG_BBD_COLOR_CONTROL_WK->normal.y = cp_data->buff.norm_y;
    p_DEBUG_BBD_COLOR_CONTROL_WK->normal.z = cp_data->buff.norm_z;
    VEC_Fx16Normalize( &p_DEBUG_BBD_COLOR_CONTROL_WK->normal, &p_DEBUG_BBD_COLOR_CONTROL_WK->normal );

    DEBUG_BBD_COLOR_InitContVec( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->normal );


    // 各種色
    p_DEBUG_BBD_COLOR_CONTROL_WK->def_r = cp_data->buff.def_r;
    p_DEBUG_BBD_COLOR_CONTROL_WK->def_g = cp_data->buff.def_g;
    p_DEBUG_BBD_COLOR_CONTROL_WK->def_b = cp_data->buff.def_b;
    p_DEBUG_BBD_COLOR_CONTROL_WK->amb_r = cp_data->buff.amb_r;
    p_DEBUG_BBD_COLOR_CONTROL_WK->amb_g = cp_data->buff.amb_g;
    p_DEBUG_BBD_COLOR_CONTROL_WK->amb_b = cp_data->buff.amb_b;
    p_DEBUG_BBD_COLOR_CONTROL_WK->spq_r = cp_data->buff.spq_r;
    p_DEBUG_BBD_COLOR_CONTROL_WK->spq_g = cp_data->buff.spq_g;
    p_DEBUG_BBD_COLOR_CONTROL_WK->spq_b = cp_data->buff.spq_b;
    p_DEBUG_BBD_COLOR_CONTROL_WK->emi_r = cp_data->buff.emi_r;
    p_DEBUG_BBD_COLOR_CONTROL_WK->emi_g = cp_data->buff.emi_g;
    p_DEBUG_BBD_COLOR_CONTROL_WK->emi_b = cp_data->buff.emi_b;
  }


  GFL_UI_KEY_SetRepeatSpeed( 4,8 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラーの調整　終了
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_DEBUG_Exit( void )
{
  GF_ASSERT( p_DEBUG_BBD_COLOR_CONTROL_WK );
  
  // フォントデータ
  GFL_FONT_Delete( p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_font = NULL;


  GFL_MSG_Delete( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata = NULL;

  WORDSET_Delete( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset = NULL;

  GFL_STR_DeleteBuffer( p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff = NULL;
  GFL_STR_DeleteBuffer( p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
  p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp = NULL;

  GFL_UI_KEY_SetRepeatSpeed( 8,15 );
}


//----------------------------------------------------------------------------
/**
 *	@brief  ビルボードカラーの調整　管理メイン
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_DEBUG_Control( void )
{
  GF_ASSERT( p_DEBUG_BBD_COLOR_CONTROL_WK );

  p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = FALSE;


  // 上下で、選択項目を変更
  if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ) 
  {
    p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select --;
    if( p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select < 0 ){
      p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select += DEBUG_BBD_COLOR_CONT_SELECT_MAX;
    }
    p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
  }
  else if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN )
  {
    p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select = (p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select + 1) % DEBUG_BBD_COLOR_CONT_SELECT_MAX;
    p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
  }

  // 選択項目ごとの処理
  switch( p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select )
  {
  case DEBUG_BBD_COLOR_CONT_SELECT_NORMAL:
    if( DEBUG_BBD_COLOR_ContVec( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->normal ) )
    {
      p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
    }
    break;

  case DEBUG_BBD_COLOR_CONT_SELECT_DEF:
    if( DEBUG_BBD_COLOR_ContRgb( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->def_r, &p_DEBUG_BBD_COLOR_CONTROL_WK->def_g, &p_DEBUG_BBD_COLOR_CONTROL_WK->def_b ) )
    {
      p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
    }
    break;

  case DEBUG_BBD_COLOR_CONT_SELECT_AMB:
    if( DEBUG_BBD_COLOR_ContRgb( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->amb_r, &p_DEBUG_BBD_COLOR_CONTROL_WK->amb_g, &p_DEBUG_BBD_COLOR_CONTROL_WK->amb_b ) )
    {
      p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
    }
    break;

  case DEBUG_BBD_COLOR_CONT_SELECT_SPQ:
    if( DEBUG_BBD_COLOR_ContRgb( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->spq_r, &p_DEBUG_BBD_COLOR_CONTROL_WK->spq_g, &p_DEBUG_BBD_COLOR_CONTROL_WK->spq_b ) )
    {
      p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
    }
    break;

  case DEBUG_BBD_COLOR_CONT_SELECT_EMI:
    if( DEBUG_BBD_COLOR_ContRgb( p_DEBUG_BBD_COLOR_CONTROL_WK, &p_DEBUG_BBD_COLOR_CONTROL_WK->emi_r, &p_DEBUG_BBD_COLOR_CONTROL_WK->emi_g, &p_DEBUG_BBD_COLOR_CONTROL_WK->emi_b ) )
    {
      p_DEBUG_BBD_COLOR_CONTROL_WK->print_req = TRUE;
    }
    break;


  default:
    break;
  }

  if( p_DEBUG_BBD_COLOR_CONTROL_WK->print_req )
  {
    GXRgb def;
    GXRgb amb;
    GXRgb spq;
    GXRgb emi;
    
    // 各種データ設定
    {
      // 法線
      GFL_BBD_SetUseCustomVecN( p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys, &p_DEBUG_BBD_COLOR_CONTROL_WK->normal );

      // ディフューズ
      def = GX_RGB( p_DEBUG_BBD_COLOR_CONTROL_WK->def_r, p_DEBUG_BBD_COLOR_CONTROL_WK->def_g, p_DEBUG_BBD_COLOR_CONTROL_WK->def_b );
      GFL_BBD_SetDiffuse( p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys, &def );

      // アンビエント 
      amb = GX_RGB( p_DEBUG_BBD_COLOR_CONTROL_WK->amb_r, p_DEBUG_BBD_COLOR_CONTROL_WK->amb_g, p_DEBUG_BBD_COLOR_CONTROL_WK->amb_b );
      GFL_BBD_SetAmbient( p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys, &amb );

      // スペキュラー
      spq = GX_RGB( p_DEBUG_BBD_COLOR_CONTROL_WK->spq_r, p_DEBUG_BBD_COLOR_CONTROL_WK->spq_g, p_DEBUG_BBD_COLOR_CONTROL_WK->spq_b );
      GFL_BBD_SetSpecular(p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys, &spq );

      // エミッション
      emi = GX_RGB( p_DEBUG_BBD_COLOR_CONTROL_WK->emi_r, p_DEBUG_BBD_COLOR_CONTROL_WK->emi_g, p_DEBUG_BBD_COLOR_CONTROL_WK->emi_b );
      GFL_BBD_SetEmission( p_DEBUG_BBD_COLOR_CONTROL_WK->bbd_sys, &emi );
    }
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  情報の描画処理
 *
 *	@param	p_win 
 */
//-----------------------------------------------------------------------------
void FLD_BBD_COLOR_DEBUG_PrintData( GFL_BMPWIN* p_win )
{
  static const u8 sc_CURSOR_Y[ DEBUG_BBD_COLOR_CONT_SELECT_MAX ] = 
  {
    0,
    48,
    80,
    112,
    144,
  };
  GF_ASSERT( p_DEBUG_BBD_COLOR_CONTROL_WK );
  GF_ASSERT( p_win );

  if( p_DEBUG_BBD_COLOR_CONTROL_WK->print_req )
  {
    // ビットマップクリア
    GFL_BMP_Clear( GFL_BMPWIN_GetBmp( p_win ), 15 );

    // 法線
    {
      WORDSET_RegisterHexNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 0, p_DEBUG_BBD_COLOR_CONTROL_WK->normal.x, 8, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterHexNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 1, p_DEBUG_BBD_COLOR_CONTROL_WK->normal.y, 8, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterHexNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 2, p_DEBUG_BBD_COLOR_CONTROL_WK->normal.z, 8, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      // 角度も出す
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 3, p_DEBUG_BBD_COLOR_CONTROL_WK->rotate_y/182, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 4, p_DEBUG_BBD_COLOR_CONTROL_WK->rotate_xz/182, 3, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

      GFL_MSG_GetString( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata, BBD_COLOR_00, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      WORDSET_ExpandStr( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 16, sc_CURSOR_Y[DEBUG_BBD_COLOR_CONT_SELECT_NORMAL], p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
    }

    // ディフューズ
    {
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 0, p_DEBUG_BBD_COLOR_CONTROL_WK->def_r, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 1, p_DEBUG_BBD_COLOR_CONTROL_WK->def_g, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 2, p_DEBUG_BBD_COLOR_CONTROL_WK->def_b, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

      GFL_MSG_GetString( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata, BBD_COLOR_01, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      WORDSET_ExpandStr( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 16, sc_CURSOR_Y[DEBUG_BBD_COLOR_CONT_SELECT_DEF], p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
    }

    // アンビエント
    {
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 0, p_DEBUG_BBD_COLOR_CONTROL_WK->amb_r, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 1, p_DEBUG_BBD_COLOR_CONTROL_WK->amb_g, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 2, p_DEBUG_BBD_COLOR_CONTROL_WK->amb_b, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

      GFL_MSG_GetString( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata, BBD_COLOR_02, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      WORDSET_ExpandStr( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 16, sc_CURSOR_Y[DEBUG_BBD_COLOR_CONT_SELECT_AMB], p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
    }

    // スペキュラー
    {
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 0, p_DEBUG_BBD_COLOR_CONTROL_WK->spq_r, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 1, p_DEBUG_BBD_COLOR_CONTROL_WK->spq_g, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 2, p_DEBUG_BBD_COLOR_CONTROL_WK->spq_b, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

      GFL_MSG_GetString( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata, BBD_COLOR_03, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      WORDSET_ExpandStr( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 16, sc_CURSOR_Y[DEBUG_BBD_COLOR_CONT_SELECT_SPQ], p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
    }

    // エミッション
    {
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 0, p_DEBUG_BBD_COLOR_CONTROL_WK->emi_r, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 1, p_DEBUG_BBD_COLOR_CONTROL_WK->emi_g, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );
      WORDSET_RegisterNumber( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, 2, p_DEBUG_BBD_COLOR_CONTROL_WK->emi_b, 2, STR_NUM_DISP_LEFT, STR_NUM_CODE_HANKAKU );

      GFL_MSG_GetString( p_DEBUG_BBD_COLOR_CONTROL_WK->p_msgdata, BBD_COLOR_04, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      WORDSET_ExpandStr( p_DEBUG_BBD_COLOR_CONTROL_WK->p_wordset, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff_tmp );
      PRINTSYS_Print( GFL_BMPWIN_GetBmp( p_win ), 16, sc_CURSOR_Y[DEBUG_BBD_COLOR_CONT_SELECT_EMI], p_DEBUG_BBD_COLOR_CONTROL_WK->p_strbuff, p_DEBUG_BBD_COLOR_CONTROL_WK->p_font );
    }


    // カーソルの描画
    {
      GFL_BMP_Fill( GFL_BMPWIN_GetBmp( p_win ), 1, sc_CURSOR_Y[p_DEBUG_BBD_COLOR_CONTROL_WK->cont_select], 8, 8, 1 );
    }

    GFL_BMPWIN_TransVramCharacter( p_win );
  }

}



//----------------------------------------------------------------------------
/**
 *	@brief  ベクトル管理の初期化
 *
 *	@param	p_wk      ワーク
 *	@param	cp_vec    操作ベクトル
 */
//-----------------------------------------------------------------------------
static void DEBUG_BBD_COLOR_InitContVec( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, const VecFx16* cp_vec )
{
  fx32 xz_dist;

  p_wk->rotate_xz = FX_Atan2Idx( cp_vec->x, cp_vec->z );
  xz_dist   = FX_Sqrt( FX_Mul( cp_vec->z, cp_vec->z ) + FX_Mul( cp_vec->x, cp_vec->x ) );
  p_wk->rotate_y  = FX_Atan2Idx( cp_vec->y, xz_dist );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ベクトル操作　メイン
 */
//-----------------------------------------------------------------------------
#define DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD ( 364 )
static BOOL DEBUG_BBD_COLOR_ContVec( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, VecFx16* p_vec )
{
  BOOL change = FALSE;
  MtxFx33 mtx_xz, mtx_y;

  // 上下左右回転
  // X 上
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_X ){
    // Y方向は、+９０度～-90度までしか動かさない
    if( ((p_wk->rotate_y+DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD) <= ((0xffff/4))) ||
        ((p_wk->rotate_y+DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD) > ((0xffff/4)*3)) ){
      p_wk->rotate_y += DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD;
    }else{
      p_wk->rotate_y = (0xffff/4);
    }

    change = TRUE;
  }
  // B 下
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_B ){

    // Y方向は、+９０度～-90度までしか動かさない
    if( (p_wk->rotate_y <= ((0xffff/4))) || (p_wk->rotate_y >= (((0xffff/4)*3) + DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD)) ){
      p_wk->rotate_y -= DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD;
    }else{
      p_wk->rotate_y = ((0xffff/4)*3);
    }
    change = TRUE;
  }
  // Y 左
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_Y ){

    p_wk->rotate_xz -= DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD;
    change = TRUE;
  }
  // A 右
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A ){

    p_wk->rotate_xz += DEBUG_BBD_COLOR_CONT_VEC_ROTATE_ADD;
    change = TRUE;
  }

  if( change ){

    p_vec->y = FX_Mul( FX_SinIdx( p_wk->rotate_y ), FX32_ONE );
    p_vec->x = FX_Mul( FX_CosIdx( p_wk->rotate_y ), FX32_ONE );
    p_vec->z = FX_Mul( FX_CosIdx( p_wk->rotate_xz ), p_vec->x );
    p_vec->x = FX_Mul( FX_SinIdx( p_wk->rotate_xz ), p_vec->x );

    VEC_Fx16Normalize( p_vec, p_vec );
  }

  return change;
}

//----------------------------------------------------------------------------
/**
 *	@brief  色の管理
 */
//-----------------------------------------------------------------------------
static BOOL DEBUG_BBD_COLOR_ContRgb( DEBUG_BBD_COLOR_CONTROL_WORK* p_wk, u8* r, u8* g, u8* b )
{
  BOOL change = FALSE;

  // R
  if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_Y ){
    (*r) = ((*r)+1) % 32;
    change = TRUE;
  }
  // G
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_X ){
    (*g) = ((*g)+1) % 32;
    change = TRUE;
  }
  // B
  else if( GFL_UI_KEY_GetRepeat() & PAD_BUTTON_A ){
    (*b) = ((*b)+1) % 32;
    change = TRUE;
  }
  return change;
}


#endif
