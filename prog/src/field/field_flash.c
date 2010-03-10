//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		field_flash.c
 *	@brief  フラッシュワーク
 *	@author	tomoya takahashi
 *	@date		2009.11.18
 *
 *	モジュール名：FIELD_FLASH
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>

#include "fld_wipe_3dobj.h"

#include "field_flash.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------

// NEAR scale alpha
#define FLASH_NEAR_SCALE  ( 0x3000 )
#define FLASH_NEAR_ALPHA  ( 31 )

// FAR scale alpha
#define FLASH_FAR_SCALE  ( 0xAA00 )
#define FLASH_FAR_ALPHA  ( 31 )

// OFF scale
#define FLASH_OFF_SCALE  ( 0 )

// フェード時間
#define FLASH_FADE_TIME ( 15 )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	スケール値管理
//=====================================
typedef struct 
{
  
  fx32 scale;
  u8   alpha;

  // フェード
  u16 count;
  u16 count_max;
  fx32 start_scale;
  fx32 move_scale;

  s16 start_alpha;
  s16 move_alpha;
  
} FLASH_SCALE;


//-------------------------------------
///	フラッシュ管理ワーク
//=====================================
struct _FIELD_FLASH 
{
  FLD_WIPEOBJ*  p_wipe;
  FLASH_SCALE   scale;
  u16           status;
  u16           req;
};

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

//-------------------------------------
///	リクエストの反映
//=====================================
static void FLASH_REQ_OnNear( FIELD_FLASH* p_wk );
static void FLASH_REQ_FadeOut( FIELD_FLASH* p_wk );
static void FLASH_REQ_OnFar( FIELD_FLASH* p_wk );
static void FLASH_REQ_Off( FIELD_FLASH* p_wk );
 
//-------------------------------------
///	スケール管理
//=====================================
static void FLASH_SCALE_Init( FLASH_SCALE* p_wk );
static void FLASH_SCALE_Exit( FLASH_SCALE* p_wk );
static void FLASH_SCALE_StartFade( FLASH_SCALE* p_wk, fx32 start, fx32 end, u8 start_alpha, u8 end_alpha, u32 time );
static BOOL FLASH_SCALE_MainFade( FLASH_SCALE* p_wk );
static BOOL FLASH_SCALE_IsFade( const FLASH_SCALE* cp_wk );
static fx32 FLASH_SCALE_GetScale( const FLASH_SCALE* cp_wk );
static void FLASH_SCALE_SetScale( FLASH_SCALE* p_wk, fx32 scale );
static u8 FLASH_SCALE_GetAlpha( const FLASH_SCALE* cp_wk );
static void FLASH_SCALE_SetAlpha( FLASH_SCALE* p_wk, u8 alpha );



//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュシステム生成
 *
 *	@param	heapID  ヒープID
 *
 *	@return
 */
//-----------------------------------------------------------------------------
FIELD_FLASH* FIELD_FLASH_Create( HEAPID heapID )
{
  FIELD_FLASH* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_FLASH) );

  p_wk->p_wipe  = FLD_WIPEOBJ_Create( heapID );
  p_wk->status  = FIELD_FLASH_STATUS_NONE;
  p_wk->req     = FIELD_FLASH_REQ_MAX;

  FLASH_SCALE_Init( &p_wk->scale );

  return p_wk;
}


//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュワーク　破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FLASH_Delete( FIELD_FLASH* p_wk )
{
  // NULLならムシ
  if(p_wk==NULL){ return; };
  
  FLASH_SCALE_Exit( &p_wk->scale );
  FLD_WIPEOBJ_Delete( p_wk->p_wipe );
  GFL_HEAP_FreeMemory( p_wk );
}


//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ  更新
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FLASH_Update( FIELD_FLASH* p_wk )
{
  // NULLならムシ
  if(p_wk==NULL){ return; };

  // リクエスト処理
  if( p_wk->req != FIELD_FLASH_REQ_MAX )
  {
    static void (*const cpFunc[])( FIELD_FLASH* ) =
    {
      FLASH_REQ_OnNear,
      FLASH_REQ_FadeOut,
      FLASH_REQ_OnFar,
      FLASH_REQ_Off,
    };
    cpFunc[ p_wk->req ]( p_wk );
    p_wk->req = FIELD_FLASH_REQ_MAX;
  }

  if( p_wk->status == FIELD_FLASH_STATUS_FADEOUT )
  {
    BOOL result;
    // フェード処理
    result = FLASH_SCALE_MainFade( &p_wk->scale );
    
    if( result )
    {
      p_wk->status = FIELD_FLASH_STATUS_FAR;
    }
  }
  
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ　描画
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
void FIELD_FLASH_Draw( FIELD_FLASH* p_wk )
{
  // NULLならムシ
  if(p_wk==NULL){ return; };
  
  // 描画
  FLD_WIPEOBJ_Main( p_wk->p_wipe, FLASH_SCALE_GetScale( &p_wk->scale ), FLASH_SCALE_GetAlpha( &p_wk->scale ) );
}


//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ　管理
 *
 *	@param	p_wk  ワーク
 *	@param	req   リクエスト
 */
//-----------------------------------------------------------------------------
void FIELD_FLASH_Control( FIELD_FLASH* p_wk, FIELD_FLASH_REQ req )
{
  GF_ASSERT( p_wk );

  p_wk->req = req;
}

//----------------------------------------------------------------------------
/**
 *	@brief  フラッシュ状態の取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return 状態
 */
//-----------------------------------------------------------------------------
FIELD_FLASH_STATUS FIELD_FLASH_GetStatus( const FIELD_FLASH* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->status;
}





//-----------------------------------------------------------------------------
/**
 *      private関数
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  初期化
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FLASH_SCALE_Init( FLASH_SCALE* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(FLASH_SCALE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  破棄
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FLASH_SCALE_Exit( FLASH_SCALE* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(FLASH_SCALE) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  フェード開始
 *
 *	@param	p_wk      ワーク
 *	@param	start     開始スケール
 *	@param	end       終了スケール
 *	@param	time      フェードシンク
 */
//-----------------------------------------------------------------------------
static void FLASH_SCALE_StartFade( FLASH_SCALE* p_wk, fx32 start, fx32 end, u8 start_alpha, u8 end_alpha, u32 time )
{
  GF_ASSERT( p_wk );
  GF_ASSERT( time > 0 );
  
  p_wk->scale       = start;
  p_wk->start_scale = start;
  p_wk->move_scale  = end - start;
  p_wk->alpha       = start_alpha;
  p_wk->start_alpha = start_alpha;
  p_wk->move_alpha  = end_alpha - start_alpha;
  p_wk->count       = 0;
  p_wk->count_max   = time;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  フェードメイン
 *
 *	@param	p_wk  ワーク
 *
 *	@retval TRUE  フェード完了
 *	@retval FALSE フェード途中
 */
//-----------------------------------------------------------------------------
static BOOL FLASH_SCALE_MainFade( FLASH_SCALE* p_wk )
{
  if( p_wk->count < p_wk->count_max )
  {
    p_wk->count++;

    // スケール値を求める
    p_wk->scale = p_wk->start_scale + FX_Div( FX_Mul(p_wk->move_scale, p_wk->count<<FX32_SHIFT), p_wk->count_max << FX32_SHIFT );

    // α値を求める
    p_wk->alpha = p_wk->start_alpha + ((p_wk->move_alpha * p_wk->count) / p_wk->count_max);
    return FALSE; 
  }

  return TRUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  フェード中チェック
 *
 *	@param	cp_wk   ワーク
 *  
 *	@retval TRUE    フェード中
 *	@retval FALSE   フェードしてない
 */
//-----------------------------------------------------------------------------
static BOOL FLASH_SCALE_IsFade( const FLASH_SCALE* cp_wk )
{
  if( cp_wk->count < cp_wk->count_max )
  {
    return TRUE;
  }
  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  スケール値取得
 *
 *	@param	cp_wk   ワーク
 *
 *	@return スケール値
 */
//-----------------------------------------------------------------------------
static fx32 FLASH_SCALE_GetScale( const FLASH_SCALE* cp_wk )
{
  GF_ASSERT( cp_wk );
  return cp_wk->scale;
}

//----------------------------------------------------------------------------
/**
 *	@brief  スケールワーク  スケール値設定
 *	@param	p_wk    ワーク
 *	@param	scale   スケール値
 */
//-----------------------------------------------------------------------------
static void FLASH_SCALE_SetScale( FLASH_SCALE* p_wk, fx32 scale )
{
  GF_ASSERT( p_wk );
  GF_ASSERT( FLASH_SCALE_IsFade(p_wk) == FALSE );

  p_wk->scale = scale;
}

//----------------------------------------------------------------------------
/**
 *	@brief  アルファ値の取得
 *
 *	@param	cp_wk   ワーク
 */
//-----------------------------------------------------------------------------
static u8 FLASH_SCALE_GetAlpha( const FLASH_SCALE* cp_wk )
{
  GF_ASSERT( cp_wk );

  return cp_wk->alpha;
}

//----------------------------------------------------------------------------
/**
 *	@brief  α値の設定
 *
 *	@param	p_wk      ワーク
 *	@param	alpha     α
 */
//-----------------------------------------------------------------------------
static void FLASH_SCALE_SetAlpha( FLASH_SCALE* p_wk, u8 alpha )
{
  GF_ASSERT( p_wk );
  GF_ASSERT( FLASH_SCALE_IsFade(p_wk) == FALSE );

  p_wk->alpha = alpha;
}





//----------------------------------------------------------------------------
/**
 *	@brief  ONリクエストの反映
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FLASH_REQ_OnNear( FIELD_FLASH* p_wk )
{
  // ON
  p_wk->status = FIELD_FLASH_STATUS_NEAR;
  FLASH_SCALE_SetScale( &p_wk->scale, FLASH_NEAR_SCALE );
  FLASH_SCALE_SetAlpha( &p_wk->scale, FLASH_NEAR_ALPHA );
}

//----------------------------------------------------------------------------
/**
 *	@brief  フェードアウトの反映
 *
 *	@param	p_wk    ワーク
 */
//-----------------------------------------------------------------------------
static void FLASH_REQ_FadeOut( FIELD_FLASH* p_wk )
{
  // 
  p_wk->status = FIELD_FLASH_STATUS_FADEOUT;
  FLASH_SCALE_StartFade( &p_wk->scale, FLASH_NEAR_SCALE, FLASH_FAR_SCALE, FLASH_NEAR_ALPHA, FLASH_FAR_ALPHA, FLASH_FADE_TIME );

  // フラッシュ使用音
  PMSND_PlaySE( SEQ_SE_FLD_127 );
}

//----------------------------------------------------------------------------
/**
 *	@brief  ファーのフラッシュワイプを設定
 *
 *	@param	p_wk 
 */
//-----------------------------------------------------------------------------
static void FLASH_REQ_OnFar( FIELD_FLASH* p_wk )
{
  // ON
  p_wk->status = FIELD_FLASH_STATUS_FAR;
  FLASH_SCALE_SetScale( &p_wk->scale, FLASH_FAR_SCALE );
  FLASH_SCALE_SetAlpha( &p_wk->scale, FLASH_FAR_ALPHA );
}

//----------------------------------------------------------------------------
/**
 *	@brief  OFFリクエストの反映
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void FLASH_REQ_Off( FIELD_FLASH* p_wk )
{
  // OFF
  p_wk->status = FIELD_FLASH_STATUS_NONE;
  FLASH_SCALE_SetScale( &p_wk->scale, FLASH_OFF_SCALE );
}

