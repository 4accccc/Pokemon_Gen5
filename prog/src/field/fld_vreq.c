//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fld_vreq.h
 *	@brief  Vblank期間　レジスタ反映
 *	@author	tomoya takahashi
 *	@date		2010.05.07
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include "fld_vreq.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///設定タイプ	
//=====================================
enum {
  FLD_VREQ_SET_NONE,
  FLD_VREQ_SET_BLENDNONE,
  FLD_VREQ_SET_BLENDALPHA,
  FLD_VREQ_SET_BLENDBRIGHTNESS,
  FLD_VREQ_SET_BLENDBRIGHTNESS_EX,
  FLD_VREQ_CHANGE_BLENDALPHA,
  FLD_VREQ_CHANGE_BLENDBRIGHTNESS,

  FLD_VREQ_TYPE_MAX,
} ;

#define FLD_VREQ_BGFRAME_MAX  ( GFL_BG_FRAME3_S+1 ) 
#define FLD_VREQ_BGFRAME_VISIBLE_NONE  ( 0xff ) 

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------

typedef union {
  // 
  struct{
    int plane1;
    int plane2;
    int ev1;
    int ev2;
  } set_alpha;

  // 
  struct{
    int plane;
    int brightness;
  } set_brightness;

  // 
  struct{
    int plane1;
    int plane2;
    int ev1;
    int ev2;
    int brightness;
  } set_brightness_ex;

  // 
  struct{
    int ev1;
    int ev2;
  } change_alpha;

  // 
  struct{
    int brightness;
  } change_brightness;

} ALPHA_DATA;

//-------------------------------------
///	システムワーク
//=====================================
struct _FLD_VREQ {
  // Alpha
  u16 main_type;
  u16 sub_type;
  ALPHA_DATA main;
  ALPHA_DATA sub;

  // BG
  u8 bg_frame_visible[ FLD_VREQ_BGFRAME_MAX ];

  // MasterBrightness
  u8 master_brightness_main_req;  // リクエスト
  u8 master_brightness_sub_req;
  s8 master_brightness_main;      // 値
  s8 master_brightness_sub;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
static void FldAlpha_SetBlendNone( const ALPHA_DATA* cp_data, BOOL main );
static void FldAlpha_SetBlendAlpha( const ALPHA_DATA* cp_data, BOOL main );
static void FldAlpha_SetBlendBrightness( const ALPHA_DATA* cp_data, BOOL main );
static void FldAlpha_SetBlendBrightnessExt( const ALPHA_DATA* cp_data, BOOL main );
static void FldAlpha_ChangeBlendAlpha( const ALPHA_DATA* cp_data, BOOL main );
static void FldAlpha_ChangeBlendBrightness( const ALPHA_DATA* cp_data, BOOL main );


//----------------------------------------------------------------------------
/**
 *	@brief  フィールドALPHA管理 生成
 *
 *	@param	heapID  ヒープID
 *
 *	@return ALPHA管理ワーク
 */
//-----------------------------------------------------------------------------
FLD_VREQ* FLD_VREQ_Create( HEAPID heapID )
{
  FLD_VREQ* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLD_VREQ) );

  // Visibleテーブル初期化
  GFL_STD_MemFill( p_wk->bg_frame_visible, FLD_VREQ_BGFRAME_VISIBLE_NONE, FLD_VREQ_BGFRAME_MAX );

  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドALPHA管理　破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_Delete( FLD_VREQ* p_wk )
{
  GFL_HEAP_FreeMemory( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フィールドALPHA管理　反映
 *
 *	@param	p_wk  ワーク 
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_VBlank( FLD_VREQ* p_wk )
{
  static void (*pFunc[])( const ALPHA_DATA* cp_data, BOOL main ) = 
  {
    NULL,
    FldAlpha_SetBlendNone,
    FldAlpha_SetBlendAlpha,
    FldAlpha_SetBlendBrightness,
    FldAlpha_SetBlendBrightnessExt,
    FldAlpha_ChangeBlendAlpha,
    FldAlpha_ChangeBlendBrightness,
  };
  int i;

  // メイン
  if( pFunc[ p_wk->main_type ] ){
    pFunc[ p_wk->main_type ]( &p_wk->main, TRUE );
    p_wk->main_type = FLD_VREQ_SET_NONE;
  }

  // サブ
  if( pFunc[ p_wk->sub_type ] ){
    pFunc[ p_wk->sub_type ]( &p_wk->sub, FALSE );
    p_wk->sub_type = FLD_VREQ_SET_NONE;
  }

  // メインフレーム　表示非表示
  for( i=0; i<FLD_VREQ_BGFRAME_MAX; i++ ){
    if( p_wk->bg_frame_visible[i] != FLD_VREQ_BGFRAME_VISIBLE_NONE ){
      GFL_BG_SetVisible( i, p_wk->bg_frame_visible[i] );
      p_wk->bg_frame_visible[i] = FLD_VREQ_BGFRAME_VISIBLE_NONE;
    }
  }

  // マスター輝度
  if( p_wk->master_brightness_main_req ){
    GX_SetMasterBrightness( p_wk->master_brightness_main );
    p_wk->master_brightness_main_req = FALSE;
  }
  if( p_wk->master_brightness_sub_req ){
    GXS_SetMasterBrightness( p_wk->master_brightness_sub );
    p_wk->master_brightness_sub_req = FALSE;
  }
  
}




//-------------------------------------
///	BG面ON・OFF
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  BG面 ON・OFF
 *
 *	@param	p_wk
 *	@param	frmnum
 *	@param	visible 
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_GFL_BG_SetVisible( FLD_VREQ* p_wk, u8 frmnum, u8 visible )
{
  p_wk->bg_frame_visible[ frmnum ] = visible;
}


//-------------------------------------
///	マスター輝度
//=====================================
//----------------------------------------------------------------------------
/**
 *	@brief  マスター輝度　メイン設定
 *
 *	@param	p_wk
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_GX_SetMasterBrightness( FLD_VREQ* p_wk, int brightness )
{
  p_wk->master_brightness_main_req  = TRUE;
  p_wk->master_brightness_main      = brightness;
}
void FLD_VREQ_GXS_SetMasterBrightness( FLD_VREQ* p_wk, int brightness )
{
  p_wk->master_brightness_sub_req  = TRUE;
  p_wk->master_brightness_sub      = brightness;
}



//----------------------------------------------------------------------------
/**
 *	@brief  BlendNoneリクエスト
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_BlendNone( FLD_VREQ* p_wk )
{
  p_wk->main_type = FLD_VREQ_SET_BLENDNONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlphaリクエスト
 *
 *	@param	p_wk
 *  @param  plane1  ブレンドの第1対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  plane2 	ブレンドの第2対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_SetBlendAlpha( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2 )
{
  p_wk->main_type = FLD_VREQ_SET_BLENDALPHA;
  p_wk->main.set_alpha.plane1 = plane1;
  p_wk->main.set_alpha.plane2 = plane2;
  p_wk->main.set_alpha.ev1    = ev1;
  p_wk->main.set_alpha.ev2    = ev2;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessリクエスト
 *
 *	@param	p_wk          ワーク
    @param  plane  	      輝度変更対象の平面を選択（GXBlendPlaneMask型の値でorをとったもの）
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_SetBlendBrightness( FLD_VREQ* p_wk, int plane, int brightness )
{
  p_wk->main_type = FLD_VREQ_SET_BLENDBRIGHTNESS;
  p_wk->main.set_brightness.plane       = plane;
  p_wk->main.set_brightness.brightness  = brightness;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessExtリクエスト
 *
 *	@param	p_wk          ワーク
 *  @param  plane1  ブレンドの第1対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  plane2 	ブレンドの第2対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_SetBlendBrightnessExt( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2, int brightness )
{
  p_wk->main_type = FLD_VREQ_SET_BLENDBRIGHTNESS_EX;
  p_wk->main.set_brightness_ex.plane1 = plane1;
  p_wk->main.set_brightness_ex.plane2 = plane2;
  p_wk->main.set_brightness_ex.ev1    = ev1;
  p_wk->main.set_brightness_ex.ev2    = ev2;
  p_wk->main.set_brightness_ex.brightness  = brightness;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlphaチェンジ　リクエスト
 *
 *	@param	p_wk    ワーク
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_ChangeBlendAlpha( FLD_VREQ* p_wk, int ev1, int ev2 )
{
  p_wk->main_type = FLD_VREQ_CHANGE_BLENDALPHA;
  p_wk->main.change_alpha.ev1    = ev1;
  p_wk->main.change_alpha.ev2    = ev2;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessチェンジ　リクエスト
 *
 *	@param	p_wk          ワーク
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2_ChangeBlendBrightness( FLD_VREQ* p_wk, int brightness )
{
  p_wk->main_type = FLD_VREQ_CHANGE_BLENDBRIGHTNESS;
  p_wk->main.change_brightness.brightness    = brightness;
}


//----------------------------------------------------------------------------
/**
 *	@brief  BlendNoneリクエスト サブ面
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_BlendNone( FLD_VREQ* p_wk )
{
  p_wk->sub_type = FLD_VREQ_SET_BLENDNONE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlphaリクエスト  サブ面
 *
 *	@param	p_wk
 *  @param  plane1  ブレンドの第1対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  plane2 	ブレンドの第2対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_SetBlendAlpha( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2 )
{
  p_wk->sub_type = FLD_VREQ_SET_BLENDALPHA;
  p_wk->sub.set_alpha.plane1 = plane1;
  p_wk->sub.set_alpha.plane2 = plane2;
  p_wk->sub.set_alpha.ev1    = ev1;
  p_wk->sub.set_alpha.ev2    = ev2;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessリクエスト   サブ面
 *
 *	@param	p_wk          ワーク
    @param  plane  	      輝度変更対象の平面を選択（GXBlendPlaneMask型の値でorをとったもの）
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_SetBlendBrightness( FLD_VREQ* p_wk, int plane, int brightness )
{
  p_wk->sub_type = FLD_VREQ_SET_BLENDBRIGHTNESS;
  p_wk->sub.set_brightness.plane       = plane;
  p_wk->sub.set_brightness.brightness  = brightness;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessExtリクエスト  サブ面
 *
 *	@param	p_wk          ワーク
 *  @param  plane1  ブレンドの第1対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  plane2 	ブレンドの第2対象面を選択（GXBlendPlaneMask型の値の論理和をとったもの）
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_SetBlendBrightnessExt( FLD_VREQ* p_wk, int plane1, int plane2, int ev1, int ev2, int brightness )
{
  p_wk->sub_type = FLD_VREQ_SET_BLENDBRIGHTNESS_EX;
  p_wk->sub.set_brightness_ex.plane1 = plane1;
  p_wk->sub.set_brightness_ex.plane2 = plane2;
  p_wk->sub.set_brightness_ex.ev1    = ev1;
  p_wk->sub.set_brightness_ex.ev2    = ev2;
  p_wk->sub.set_brightness_ex.brightness  = brightness;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlphaチェンジ　リクエスト  サブ面
 *
 *	@param	p_wk    ワーク
 *  @param  ev1 	  第1対象面に対するαブレンディング係数(0 <= ev1 <= 31)
 *  @param  ev2 	  第2対象面に対するαブレンディング係数(0 <= ev2 <= 31)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_ChangeBlendAlpha( FLD_VREQ* p_wk, int ev1, int ev2 )
{
  p_wk->sub_type = FLD_VREQ_CHANGE_BLENDALPHA;
  p_wk->sub.change_alpha.ev1    = ev1;
  p_wk->sub.change_alpha.ev2    = ev2;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessチェンジ　リクエスト サブ面
 *
 *	@param	p_wk          ワーク
    @param  brightness 	  輝度変更のための係数を指定します(-16 <= brightness <= 16)
 */
//-----------------------------------------------------------------------------
void FLD_VREQ_G2S_ChangeBlendBrightness( FLD_VREQ* p_wk, int brightness )
{
  p_wk->sub_type = FLD_VREQ_CHANGE_BLENDBRIGHTNESS;
  p_wk->sub.change_brightness.brightness    = brightness;
}





//-----------------------------------------------------------------------------
/**
 *    private関数
 */
//-----------------------------------------------------------------------------

//----------------------------------------------------------------------------
/**
 *	@brief  BlendNone設定
 *
 *	@param	cp_data
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_SetBlendNone( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_BlendNone();
  }else{
    G2S_BlendNone();
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlpha設定
 *
 *	@param	cp_data
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_SetBlendAlpha( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_SetBlendAlpha( cp_data->set_alpha.plane1, cp_data->set_alpha.plane2, 
        cp_data->set_alpha.ev1, cp_data->set_alpha.ev2 );
  }else{
    G2S_SetBlendAlpha( cp_data->set_alpha.plane1, cp_data->set_alpha.plane2, 
        cp_data->set_alpha.ev1, cp_data->set_alpha.ev2 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightness設定
 *
 *	@param	cp_data 
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_SetBlendBrightness( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_SetBlendBrightness( cp_data->set_brightness.plane, cp_data->set_brightness.brightness );
  }else{
    G2S_SetBlendBrightness( cp_data->set_brightness.plane, cp_data->set_brightness.brightness );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightnessExt設定
 *
 *	@param	cp_data
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_SetBlendBrightnessExt( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_SetBlendBrightnessExt( cp_data->set_brightness_ex.plane1, cp_data->set_brightness_ex.plane2, 
        cp_data->set_brightness_ex.ev1, cp_data->set_brightness_ex.ev2, 
        cp_data->set_brightness_ex.brightness );
  }else{
    G2S_SetBlendBrightnessExt( cp_data->set_brightness_ex.plane1, cp_data->set_brightness_ex.plane2, 
        cp_data->set_brightness_ex.ev1, cp_data->set_brightness_ex.ev2, 
        cp_data->set_brightness_ex.brightness );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendAlpha変更
 *
 *	@param	cp_data 
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_ChangeBlendAlpha( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_ChangeBlendAlpha( cp_data->change_alpha.ev1, cp_data->change_alpha.ev2 );
  }else{
    G2S_ChangeBlendAlpha( cp_data->change_alpha.ev1, cp_data->change_alpha.ev2 );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  BlendBrightness 変更
 *
 *	@param	cp_data
 *	@param	main 
 */
//-----------------------------------------------------------------------------
static void FldAlpha_ChangeBlendBrightness( const ALPHA_DATA* cp_data, BOOL main )
{
  if( main ){
    G2_ChangeBlendBrightness( cp_data->change_brightness.brightness );
  }else{
    G2S_ChangeBlendBrightness( cp_data->change_brightness.brightness );
  }
}
