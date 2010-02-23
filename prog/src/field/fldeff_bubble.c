//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *	GAME FREAK inc.
 *
 *	@file		fldeff_bubble.c
 *	@brief  深海　水泡エフェクト
 *	@author	tomoya takahashi
 *	@date		2010.02.23
 *
 *	モジュール名：
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]

#include <gflib.h>
#include "system/gfl_use.h"

#include "field/field_const.h"

#include "fieldmap.h"
#include "field_effect.h"
#include "fldmmdl.h"

#include "fldeff_bubble.h"

//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	Resourceインデックス
//=====================================
enum {
  // Resource
  BUBBLE_RES_MDL_BUBBLE = 0,
  BUBBLE_RES_ANM_ITA,
  BUBBLE_RES_ANM_IMA,
  BUBBLE_RES_MAX,

  // レンダラー
  BUBBLE_RND_BUBBLE = 0,
  BUBBLE_RND_MAX,

  // アニメ
  BUBBLE_ANM_ITA = 0,
  BUBBLE_ANM_IMA,
  BUBBLE_ANM_MAX,
} ;

// 読み込みリソース
static const u32 sc_RES_INDEX[ BUBBLE_RES_MAX ] = {
  NARC_fldeff_awa_nsbmd,
  NARC_fldeff_awa_nsbta,
  NARC_fldeff_awa_nsbma,
};

// レンダラー定義
static const u32 sc_RND_RES_INDEX[ BUBBLE_RND_MAX ] = {
  BUBBLE_RES_MDL_BUBBLE,
};

// アニメーション定義
static const u32 sc_ANM_RES_INDEX[ BUBBLE_ANM_MAX ] = {
  BUBBLE_RES_ANM_ITA,
  BUBBLE_RES_ANM_IMA,
};


//-------------------------------------
///	泡ワーク最大数
//=====================================
#define BUBBLE_WK_MAX   (6)


//-------------------------------------
///	泡出力　タイミング
//=====================================
#define BUBBLE_POP_TIMING ( 120 )
#define BUBBLE_POP_ACTION_TIMING ( 5 )

//-------------------------------------
///	出現座標調整
//=====================================
#define BUBBLE_POP_POS_Y  ( 7*FX32_ONE )
#define BUBBLE_POP_POS_FRONT_UP  ( 14*FX32_ONE )  //泡が自機の向こうから出るように
#define BUBBLE_POP_POS_FRONT  ( 4*FX32_ONE )

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//--------------------------------------------------------------
///  FLDEFF_BUBBLE型
//--------------------------------------------------------------
typedef struct _TAG_FLDEFF_BUBBLE FLDEFF_BUBBLE;

//--------------------------------------------------------------
///  FLDEFF_BUBBLE構造体
//--------------------------------------------------------------
struct _TAG_FLDEFF_BUBBLE
{
  FLDEFF_CTRL*  p_fectrl;
  
  GFL_G3D_RES*  p_res[BUBBLE_RES_MAX];
  GFL_G3D_RND*  p_rnd[BUBBLE_RND_MAX];
  GFL_G3D_ANM*  p_anm[BUBBLE_ANM_MAX];
  GFL_G3D_OBJ*  p_obj[BUBBLE_RND_MAX];
};

//--------------------------------------------------------------
/// TASKHEADER_BUBBLE
//--------------------------------------------------------------
typedef struct
{
  FLDEFF_BUBBLE* p_fldeff_bubble;
  MMDL* p_mmdl;
}TASKHEADER_BUBBLE;

//-------------------------------------
///	泡ワーク
//=====================================
typedef struct {
  BOOL flag;
  int frame;
  VecFx32 pos;
} BUBBLE_WK;


//--------------------------------------------------------------
/// TASKWORK_FOOTMARK
//--------------------------------------------------------------
typedef struct
{
  TASKHEADER_BUBBLE head;
  u8 last_action;
  s8 action_count;
  s8 frame;
  BUBBLE_WK obj[BUBBLE_WK_MAX];
}TASKWORK_BUBBLE;

//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------

static void bubble_InitResource( FLDEFF_BUBBLE* p_wk );
static void bubble_ExitResource( FLDEFF_BUBBLE* p_wk );


static void bubbleTask_Init( FLDEFF_TASK* p_task, void* p_work );
static void bubbleTask_Exit( FLDEFF_TASK* p_task, void* p_work );
static void bubbleTask_Update( FLDEFF_TASK* p_task, void* p_work );
static void bubbleTask_Draw( FLDEFF_TASK* p_task, void* p_work );



// 泡管理
static BUBBLE_WK* bubbleTask_GetClearWk( TASKWORK_BUBBLE* p_wk );
static void bubbleTask_SetUpObj( BUBBLE_WK* p_wk, const MMDL* cp_mmdl );
static void bubbleTask_ClearObj( BUBBLE_WK* p_wk );
static void bubbleTask_UpdateObj( BUBBLE_WK* p_wk, int frame_max );
static void bubbleTask_DrawObj( BUBBLE_WK* p_wk, GFL_G3D_OBJ* p_mdl );

// 目の前の方向を取得
static u16 bubbleTask_GetFrontWay( const MMDL* cp_mmdl, VecFx16* p_way );

// ヘッダー
static const FLDEFF_TASK_HEADER data_bubbleTaskHeader;

//----------------------------------------------------------------------------
/**
 *	@brief  モデルに水泡　エフェクトタスクを関連付ける
 *
 *	@param	p_mmdl      モデル
 *	@param	p_fectrl    エフェクトワーク
 */
//-----------------------------------------------------------------------------
void FLDEFF_BUBBLE_SetMMdl( MMDL* p_mmdl, FLDEFF_CTRL* p_fectrl )
{
  FLDEFF_BUBBLE* p_bubble;
  TASKHEADER_BUBBLE head;
  VecFx32 pos;

  p_bubble = FLDEFF_CTRL_GetEffectWork( p_fectrl, FLDEFF_PROCID_BUBBLE );
  head.p_fldeff_bubble  = p_bubble;
  head.p_mmdl           = p_mmdl;

  // 位置
  MMDL_GetVectorPos( p_mmdl, &pos );

  FLDEFF_CTRL_AddTask(
      p_fectrl, &data_bubbleTaskHeader, &pos, 0, &head, 0 );
}



//----------------------------------------------------------------------------
/**
 *	@brief  水泡エフェクトを初期化
 *
 *	@param	p_fectrl  エフェクトワーク
 *	@param	heapID    ヒープID
 *
 *	@return エフェクト使用ワーク
 */
//-----------------------------------------------------------------------------
void * FLDEFF_BUBBLE_Init( FLDEFF_CTRL* p_fectrl, HEAPID heapID )
{
  FLDEFF_BUBBLE* p_wk;

  p_wk = GFL_HEAP_AllocClearMemory( heapID, sizeof(FLDEFF_BUBBLE) );

  p_wk->p_fectrl = p_fectrl;
  bubble_InitResource( p_wk );
  return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief  水泡エフェクトの破棄
 *
 *	@param	Pfectrl エフェクトワーク
 *	@param	p_work  ワーク
 */
//-----------------------------------------------------------------------------
void FLDEFF_BUBBLE_Delete( FLDEFF_CTRL* p_fectrl, void* p_work )
{
  FLDEFF_BUBBLE* p_wk = p_work;
  bubble_ExitResource( p_wk );
  GFL_HEAP_FreeMemory( p_wk );
}





//-----------------------------------------------------------------------------
/**
 *      private 
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief  リソース初期化
 */
//-----------------------------------------------------------------------------
static void bubble_InitResource( FLDEFF_BUBBLE* p_wk )
{
  int i, j;
  ARCHANDLE* p_handle = FLDEFF_CTRL_GetArcHandleEffect( p_wk->p_fectrl );
  BOOL result;

  // リソース読み込み
  for( i=0; i<BUBBLE_RES_MAX; i++ ){
    p_wk->p_res[i] = GFL_G3D_CreateResourceHandle( p_handle, sc_RES_INDEX[i] );
    if( GFL_G3D_CheckResourceType( p_wk->p_res[i], GFL_G3D_RES_CHKTYPE_TEX ) ){
      // テクスチャ転送
      result = GFL_G3D_TransVramTexture( p_wk->p_res[i] );
      GF_ASSERT( result );
    }
  }

  // レンダラー生成
  for( i=0; i<BUBBLE_RND_MAX; i++ ){
    p_wk->p_rnd[i] = GFL_G3D_RENDER_Create( p_wk->p_res[ sc_RND_RES_INDEX[i] ], 
        0, p_wk->p_res[ sc_RND_RES_INDEX[i] ] );
  }

  // アニメーションオブジェ
  for( i=0; i<BUBBLE_ANM_MAX; i++ ){
    p_wk->p_anm[i] = GFL_G3D_ANIME_Create( p_wk->p_rnd[BUBBLE_RND_BUBBLE], 
        p_wk->p_res[ sc_ANM_RES_INDEX[i] ], 0 );
  }

  // OBJ生成
  for( i=0; i<BUBBLE_RND_MAX; i++ ){
    p_wk->p_obj[i] = GFL_G3D_OBJECT_Create( p_wk->p_rnd[i], p_wk->p_anm, BUBBLE_ANM_MAX );

    for( j=0; j<BUBBLE_ANM_MAX; j++ ){
      GFL_G3D_OBJECT_EnableAnime( p_wk->p_obj[i], j );
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  リソース破棄
 */
//-----------------------------------------------------------------------------
static void bubble_ExitResource( FLDEFF_BUBBLE* p_wk )
{
  int i;
  BOOL result;
  
  for( i=0; i<BUBBLE_RND_MAX; i++ ){
    GFL_G3D_OBJECT_Delete( p_wk->p_obj[i] );
  }

  for( i=0; i<BUBBLE_ANM_MAX; i++ ){
    GFL_G3D_ANIME_Delete( p_wk->p_anm[i] );
  }

  for( i=0; i<BUBBLE_RND_MAX; i++ ){
    GFL_G3D_RENDER_Delete( p_wk->p_rnd[i] );
  }

  for( i=0; i<BUBBLE_RES_MAX; i++ ){
    if( GFL_G3D_CheckResourceType( p_wk->p_res[i], GFL_G3D_RES_CHKTYPE_TEX ) ){
      // テクスチャ破棄
      result = GFL_G3D_FreeVramTexture( p_wk->p_res[i] );
      GF_ASSERT( result );
    }
    GFL_G3D_DeleteResource( p_wk->p_res[i] );
  }
}



//----------------------------------------------------------------------------
/**
 *	@brief  水泡　タスク　初期化
 */
//-----------------------------------------------------------------------------
static void bubbleTask_Init( FLDEFF_TASK* p_task, void* p_work )
{
  TASKWORK_BUBBLE* p_wk = p_work;
  const TASKHEADER_BUBBLE * cp_head;
  cp_head = FLDEFF_TASK_GetAddPointer( p_task );
  p_wk->head = *cp_head;
  p_wk->frame = BUBBLE_POP_TIMING;
  p_wk->last_action = MMDL_CheckPossibleAcmd( p_wk->head.p_mmdl );
  p_wk->action_count = BUBBLE_POP_ACTION_TIMING;
}

//----------------------------------------------------------------------------
/**
 *	@brief  水泡　タスク　破棄
 */
//-----------------------------------------------------------------------------
static void bubbleTask_Exit( FLDEFF_TASK* p_task, void* p_work )
{
}

//----------------------------------------------------------------------------
/**
 *	@brief  水泡　タスク　更新
 */
//-----------------------------------------------------------------------------
static void bubbleTask_Update( FLDEFF_TASK* p_task, void* p_work )
{
  TASKWORK_BUBBLE* p_wk = p_work;
  int i;
  int frame_max;
  BUBBLE_WK* p_obj;

  
  // 発動管理
  p_wk->frame ++;
  if( p_wk->frame >= BUBBLE_POP_TIMING ){
    p_obj = bubbleTask_GetClearWk( p_wk );
    if( p_obj ){
      bubbleTask_SetUpObj( p_obj, p_wk->head.p_mmdl );
    }
    p_wk->frame = 0;
  }

  // 動きはじめ、とまったとき泡
  // はじめー＞とまりをBUBBLE_POP_ACTION_TIMING回繰り返したら出す
  if( p_wk->last_action != MMDL_CheckPossibleAcmd( p_wk->head.p_mmdl ) ){

    p_wk->action_count ++;
    
    if( p_wk->action_count >= BUBBLE_POP_ACTION_TIMING ){
      p_obj = bubbleTask_GetClearWk( p_wk );
      if( p_obj ){
        bubbleTask_SetUpObj( p_obj, p_wk->head.p_mmdl );
      }
      p_wk->action_count = 0;
    }
    
    p_wk->last_action = MMDL_CheckPossibleAcmd( p_wk->head.p_mmdl );
  }

  // ワークメイン
  GFL_G3D_OBJECT_GetAnimeFrameMax( p_wk->head.p_fldeff_bubble->p_obj[ BUBBLE_RND_BUBBLE ], BUBBLE_ANM_ITA, &frame_max );

  for( i=0; i<BUBBLE_WK_MAX; i++ ){
    bubbleTask_UpdateObj( &p_wk->obj[i], frame_max );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  水泡　タスク　描画
 */
//-----------------------------------------------------------------------------
static void bubbleTask_Draw( FLDEFF_TASK* p_task, void* p_work )
{
  TASKWORK_BUBBLE* p_wk = p_work;
  int i;

  for( i=0; i<BUBBLE_WK_MAX; i++ ){
    bubbleTask_DrawObj( &p_wk->obj[i], p_wk->head.p_fldeff_bubble->p_obj[ BUBBLE_RND_BUBBLE ] );
  }
}



// 泡管理
//----------------------------------------------------------------------------
/**
 *	@brief  空いているワークを取得
 *
 *	@param	p_wk    ワーク
 *
 *	@return ワーク
 */
//-----------------------------------------------------------------------------
static BUBBLE_WK* bubbleTask_GetClearWk( TASKWORK_BUBBLE* p_wk )
{
  int i;
  for( i=0; i<BUBBLE_WK_MAX; i++ ){
    if( p_wk->obj[i].flag == FALSE ){
      return &p_wk->obj[i];
    }
  }
  return NULL;
}

//----------------------------------------------------------------------------
/**
 *	@brief  オブジェのセットアップ
 *
 *	@param	p_wk    ワーク
 *	@param	cp_pos  位置
 */
//-----------------------------------------------------------------------------
static void bubbleTask_SetUpObj( BUBBLE_WK* p_wk, const MMDL* cp_mmdl )
{
  VecFx16 way;
  u16 dir;
  fx32 mul;
  
  p_wk->flag  = TRUE;
  p_wk->frame = 0;
  MMDL_GetVectorPos( cp_mmdl, &p_wk->pos );
  dir = bubbleTask_GetFrontWay( cp_mmdl, &way );
  if( dir == DIR_UP ){
    mul = BUBBLE_POP_POS_FRONT_UP;
  }else{
    mul = BUBBLE_POP_POS_FRONT;
  }

  p_wk->pos.y += BUBBLE_POP_POS_Y;
  p_wk->pos.x += FX_Mul( way.x, mul );
  p_wk->pos.z += FX_Mul( way.z, mul );
}

//----------------------------------------------------------------------------
/**
 *	@brief  オブジェ情報のクリア
 *
 *	@param	p_wk  ワーク
 */
//-----------------------------------------------------------------------------
static void bubbleTask_ClearObj( BUBBLE_WK* p_wk )
{
  GFL_STD_MemClear( p_wk, sizeof(BUBBLE_WK) );
}

//----------------------------------------------------------------------------
/**
 *	@brief  オブジェ情報の更新
 */
//-----------------------------------------------------------------------------
static void bubbleTask_UpdateObj( BUBBLE_WK* p_wk, int frame_max )
{
  if( p_wk->flag == FALSE ){
    return;
  }
  
  p_wk->frame += FX32_ONE;
  if( p_wk->frame > frame_max ){
    bubbleTask_ClearObj( p_wk );
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  オブジェ情報の描画
 */
//-----------------------------------------------------------------------------
static void bubbleTask_DrawObj( BUBBLE_WK* p_wk, GFL_G3D_OBJ* p_mdl )
{
  static GFL_G3D_OBJSTATUS status = {
    {0},
    {FX32_ONE,FX32_ONE,FX32_ONE},
    {
      FX32_ONE,0,0,
      0,FX32_ONE,0,
      0,0,FX32_ONE,
    },
  };
  int i;

  if( p_wk->flag == FALSE ){
    return;
  }

  // 位置とフレームで描画
  status.trans = p_wk->pos;

  for( i=0; i<BUBBLE_ANM_MAX; i++ ){
    GFL_G3D_OBJECT_SetAnimeFrame( p_mdl,
        i, &p_wk->frame );
  }
  GFL_G3D_DRAW_DrawObjectCullingON( p_mdl, &status );
}


//----------------------------------------------------------------------------
/**
 *	@brief  目の前の方向を返す
 */
//-----------------------------------------------------------------------------
static u16 bubbleTask_GetFrontWay( const MMDL* cp_mmdl, VecFx16* p_way )
{
  u16 dir = MMDL_GetDirDisp( cp_mmdl );
  
  if( !MMDL_CheckStatusBit( cp_mmdl, MMDL_STABIT_RAIL_MOVE ) ){
    p_way->y = 0;
    p_way->x = MMDL_TOOL_GetDirAddValueGridX( dir );
    p_way->z = MMDL_TOOL_GetDirAddValueGridZ( dir );

    p_way->x = GRID_TO_FX32( p_way->x );
    p_way->z = GRID_TO_FX32( p_way->z );
    VEC_Fx16Normalize( p_way, p_way );
  }else{
    MMDL_Rail_GetDirLineWay( cp_mmdl, dir, p_way );
    p_way->y = 0;
    VEC_Fx16Normalize( p_way, p_way );
  }

  return dir;
}



//--------------------------------------------------------------
//  水泡タスク　ヘッダー
//--------------------------------------------------------------
static const FLDEFF_TASK_HEADER data_bubbleTaskHeader =
{
  sizeof(TASKWORK_BUBBLE),
  bubbleTask_Init,
  bubbleTask_Exit,
  bubbleTask_Update,
  bubbleTask_Draw,
};



