
//============================================================================================
/**
 * @file  btlv_mcss.c
 * @brief 戦闘MCSS管理
 * @author  soga
 * @date  2008.11.14
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"
#include "system/mcss_tool.h"

#include "btlv_effect.h"

#include "battle/battgra_wb.naix"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

#define BTLV_MCSS_MAX ( BTLV_MCSS_POS_TOTAL ) //BTLV_MCSSで管理するMCSSのMAX値

#define BTLV_MCSS_DEFAULT_SHIFT ( FX32_SHIFT - 4 )          //ポリゴン１辺の基準の長さにするシフト値
#define BTLV_MCSS_DEFAULT_LINE  ( 1 << BTLV_MCSS_DEFAULT_SHIFT )  //ポリゴン１辺の基準の長さ

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct _BTLV_MCSS_WORK
{
  GFL_TCBSYS*     tcb_sys;
  MCSS_SYS_WORK*  mcss_sys;
  MCSS_WORK*      mcss[ BTLV_MCSS_MAX ];
  int             callback_count[ BTLV_MCSS_MAX ];    //コールバックが呼ばれた回数をカウント

  u8              poke_mcss_proj_mode   :1;
  u8                                    :7;
  u32             poke_mcss_tcb_move_execute;
  u32             poke_mcss_tcb_scale_execute;
  u32             poke_mcss_tcb_rotate_execute;

  u32             poke_mcss_tcb_blink_execute;
  u32             poke_mcss_tcb_alpha_execute;
  HEAPID          heapID;
};

typedef struct
{
  BTLV_MCSS_WORK*   bmw;
  int               position;
  EFFTOOL_MOVE_WORK emw;
}BTLV_MCSS_TCB_WORK;

typedef struct
{
  NNSG2dCharacterData*  pCharData;    //テクスチャキャラ
  NNSG2dPaletteData*    pPlttData;    //テクスチャパレット
  void*                 pBufChar;     //テクスチャキャラバッファ
  void*                 pBufPltt;     //テクスチャパレットバッファ
  int                   chr_ofs;
  int                   pal_ofs;
  BTLV_MCSS_WORK*       bmw;
}TCB_LOADRESOURCE_WORK;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================

static  void  BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK* maw, int position );
static  void  BTLV_MCSS_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK* maw, int position );
static  void  BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position );

static  void  BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end,
                                       int frame, int wait, int count, GFL_TCB_FUNC *func );
static  void  TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_Alpha( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_MCSS_MoveCircle( GFL_TCB *tcb, void *work );

static  void  BTLV_MCSS_CallBackFunctorFrame( u32 data, fx32 currentFrame );

#ifdef PM_DEBUG
void  BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position );
#endif

//============================================================================================
/**
 *  ポケモンの立ち位置テーブル
 */
//============================================================================================
/*
static  const VecFx32 poke_pos_table[]={
  { FX_F32_TO_FX32( -2.5f ),  FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.0f ) },   //POS_AA
  { FX_F32_TO_FX32(  4.5f ),  FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },   //POS_BB
  { FX_F32_TO_FX32( -3.5f ),  FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(   8.5f ) },   //POS_A
  { FX_F32_TO_FX32(  6.0f ),  FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  -9.0f ) },   //POS_B
  { FX_F32_TO_FX32( -0.5f ),  FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(   9.0f ) },   //POS_C
  { FX_F32_TO_FX32(  2.0f ),  FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -11.0f ) },   //POS_D
  { FX_F32_TO_FX32( -2.5f ),  FX_F32_TO_FX32( 1.2f ), FX_F32_TO_FX32(  10.0f ) },   //POS_E
  { FX_F32_TO_FX32(  4.5f ),  FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },   //POS_F
};
*/


static  const VecFx32 poke_pos_table[]={
  { FX_F32_TO_FX32( -2.5f + 3.000f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   7.5f - 0.5f ) },    //POS_AA
  { FX_F32_TO_FX32(  4.5f - 4.200f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },   //POS_BB
  { FX_F32_TO_FX32( -3.5f + 3.500f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   8.5f ) },   //POS_A
  { FX_F32_TO_FX32(  6.0f - 4.200f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  -9.0f ) },   //POS_B
  { FX_F32_TO_FX32( -0.5f + 3.845f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(   9.0f ) },   //POS_C
  { FX_F32_TO_FX32(  2.0f - 4.964f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -11.0f ) },   //POS_D
  { FX_F32_TO_FX32( -2.5f + 3.845f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32(  10.0f ) },   //POS_E
  { FX_F32_TO_FX32(  4.5f - 4.964f ), FX_F32_TO_FX32( 0.7f ), FX_F32_TO_FX32( -10.0f ) },   //POS_F
  { FX_F32_TO_FX32( -2.5f + 3.000f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   7.5f - 0.5f ) },    //POS_TR_AA
  { FX_F32_TO_FX32(  4.5f - 4.200f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( -25.0f ) },   //POS_TR_BB
  { FX_F32_TO_FX32( -3.5f + 3.500f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   8.5f ) },   //POS_TR_A
  { FX_F32_TO_FX32(  6.0f - 4.200f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(  -9.0f ) },   //POS_TR_B
  { FX_F32_TO_FX32( -0.5f + 3.845f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32(   9.0f ) },   //POS_TR_C
  { FX_F32_TO_FX32(  2.0f - 4.964f ), FX_F32_TO_FX32( 0.0f ), FX_F32_TO_FX32( -11.0f ) },   //POS_TR_D
};

//============================================================================================
/**
 *  ポケモンの立ち位置によるスケール補正テーブル
 */
//============================================================================================
static  const fx32  poke_scale_table[ 2 ][ BTLV_MCSS_POS_TOTAL ]={
  {
    0x1030,   //POS_AA
    0x119b,   //POS_BB
    0x0f00,   //POS_A
    0x10e0,   //POS_B
    0x0d00,   //POS_C
    0x1320,   //POS_D
    0x1000 * 2, //POS_E
    0x1000,   //POS_F
    0x1030,   //POS_TR_AA
    0x119b,   //POS_TR_BB
    0x0f00,   //POS_TR_A
    0x10e0,   //POS_TR_B
    0x0d00,   //POS_TR_C
    0x1320,   //POS_TR_D
  },
  {
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
    FX32_ONE * 16 * 2,
    FX32_ONE * 16,
  }
};

//============================================================================================
/**
 * @brief システム初期化
 *
 * @param[in] tcb_sys システム内で使用するTCBSYS構造体へのポインタ
 * @param[in] heapID  ヒープID
 */
//============================================================================================
BTLV_MCSS_WORK  *BTLV_MCSS_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID )
{
  BTLV_MCSS_WORK *bmw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_MCSS_WORK ) );

  bmw->mcss_sys = MCSS_Init( BTLV_MCSS_MAX, heapID );
  bmw->tcb_sys  = tcb_sys;

  return bmw;
}

//============================================================================================
/**
 * @brief システム終了
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Exit( BTLV_MCSS_WORK *bmw )
{
  MCSS_Exit( bmw->mcss_sys );
  GFL_HEAP_FreeMemory( bmw );
}

//============================================================================================
/**
 * @brief システムメイン
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Main( BTLV_MCSS_WORK *bmw )
{
  MCSS_Main( bmw->mcss_sys );
}

//============================================================================================
/**
 * @brief 描画
 *
 * @param[in] bmw BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_Draw( BTLV_MCSS_WORK *bmw )
{
  MCSS_Draw( bmw->mcss_sys );
}

//============================================================================================
/**
 * @brief BTLV_MCSS追加
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] pp      POKEMON_PARAM構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_Add( BTLV_MCSS_WORK *bmw, const POKEMON_PARAM *pp, int position )
{
  MCSS_ADD_WORK maw;

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT_MSG( bmw->mcss[ position ] == NULL, "pos=%d", position );

  BTLV_MCSS_MakeMAW( pp, &maw, position );
  bmw->mcss[ position ] = MCSS_Add( bmw->mcss_sys,
                    poke_pos_table[ position ].x,
                    poke_pos_table[ position ].y,
                    poke_pos_table[ position ].z,
                    &maw );

  BTLV_MCSS_SetDefaultScale( bmw, position );

  MCSS_SetAnimCtrlCallBack( bmw->mcss[ position ], position, BTLV_MCSS_CallBackFunctorFrame, 1 );
}

//============================================================================================
/**
 * @brief BTLV_MCSS追加
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] pp      POKEMON_PARAM構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_AddTrainer( BTLV_MCSS_WORK *bmw, int tr_type, int position )
{
  MCSS_ADD_WORK maw;

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT_MSG( bmw->mcss[ position ] == NULL, "pos=%d", position );

  BTLV_MCSS_MakeMAWTrainer( tr_type, &maw, position );
  bmw->mcss[ position ] = MCSS_Add( bmw->mcss_sys,
                    poke_pos_table[ position ].x,
                    poke_pos_table[ position ].y,
                    poke_pos_table[ position ].z,
                    &maw );

  BTLV_MCSS_SetDefaultScale( bmw, position );

  MCSS_SetAnimCtrlCallBack( bmw->mcss[ position ], position, BTLV_MCSS_CallBackFunctorFrame, 1 );
}

//============================================================================================
/**
 * @brief BTLV_MCSS削除
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_Del( BTLV_MCSS_WORK *bmw, int position )
{
  MCSS_Del( bmw->mcss_sys, bmw->mcss[ position ] );
  bmw->mcss[ position ] = NULL;
}

//============================================================================================
/**
 * @brief 座標セット
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  座標セットするMCSSの立ち位置
 * @param[in] pos_x     X座標
 * @param[in] pos_y     Y座標
 * @param[in] pos_z     Z座標
 */
//============================================================================================
void  BTLV_MCSS_SetPosition( BTLV_MCSS_WORK *bmw, int position, fx32 pos_x, fx32 pos_y, fx32 pos_z )
{ 
  VecFx32 pos;

  pos.x = pos_x;
  pos.y = pos_y;
  pos.z = pos_z;

  MCSS_SetPosition( bmw->mcss[ position ], &pos );
}

//============================================================================================
/**
 * @brief 正射影描画モードON
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_SetOrthoMode( BTLV_MCSS_WORK *bmw )
{
  int position;

  MCSS_SetOrthoMode( bmw->mcss_sys );

  bmw->poke_mcss_proj_mode = BTLV_MCSS_PROJ_ORTHO;

  for( position = 0 ; position < BTLV_MCSS_POS_TOTAL ; position++ ){
    if( bmw->mcss[ position ] ){
      BTLV_MCSS_SetDefaultScale( bmw, position );
    }
  }
}

//============================================================================================
/**
 * @brief 正射影描画モードOFF
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_ResetOrthoMode( BTLV_MCSS_WORK *bmw )
{
  int position;

  MCSS_ResetOrthoMode( bmw->mcss_sys );

  bmw->poke_mcss_proj_mode = BTLV_MCSS_PROJ_PERSPECTIVE;

  for( position = 0 ; position < BTLV_MCSS_POS_TOTAL ; position++ ){
    if( bmw->mcss[ position ] ){
      BTLV_MCSS_SetDefaultScale( bmw, position );
    }
  }
}

//============================================================================================
/**
 * @brief メパチ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  メパチさせたいポケモンの立ち位置
 * @param[in] flag    メパチフラグ（BTLV_MCSS_MEPACHI_ON、BTLV_MCSS_MEPACHI_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetMepachiFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  GF_ASSERT( bmw->mcss[ position ] != NULL );
  if( flag == BTLV_MCSS_MEPACHI_FLIP ){
    MCSS_FlipMepachiFlag( bmw->mcss[ position ] );
  }
  else if( flag == BTLV_MCSS_MEPACHI_ON ){
    MCSS_SetMepachiFlag( bmw->mcss[ position ] );
  }
  else{
    MCSS_ResetMepachiFlag( bmw->mcss[ position ] );
  }
}

//============================================================================================
/**
 * @brief アニメストップ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  アニメストップさせたいポケモンの立ち位置
 * @param[in] flag    アニメストップフラグ（BTLV_MCSS_ANM_STOP_ON、BTLV_MCSS_ANM_STOP_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetAnmStopFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  GF_ASSERT( bmw->mcss[ position ] != NULL );
  if( flag == BTLV_MCSS_ANM_STOP_ON ){
    MCSS_SetAnmStopFlag( bmw->mcss[ position ] );
  }
  else{
    MCSS_ResetAnmStopFlag( bmw->mcss[ position ] );
  }
}

//============================================================================================
/**
 * @brief バニッシュフラグ取得
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  フラグ操作させたいポケモンの立ち位置
 */
//============================================================================================
int   BTLV_MCSS_GetVanishFlag( BTLV_MCSS_WORK *bmw, int position )
{
  GF_ASSERT( bmw->mcss[ position ] != NULL );
  return MCSS_GetVanishFlag( bmw->mcss[ position ] );
}

//============================================================================================
/**
 * @brief バニッシュ処理
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  フラグ操作させたいポケモンの立ち位置
 * @param[in] flag    バニッシュフラグ（BTLV_MCSS_VANISH_ON、BTLV_MCSS_VANISH_OFF）
 */
//============================================================================================
void  BTLV_MCSS_SetVanishFlag( BTLV_MCSS_WORK *bmw, int position, int flag )
{
  GF_ASSERT( bmw->mcss[ position ] != NULL );
  if( flag == BTLV_MCSS_VANISH_FLIP ){
    MCSS_FlipVanishFlag( bmw->mcss[ position ] );
  }
  else if( flag == BTLV_MCSS_VANISH_ON ){
    MCSS_SetVanishFlag( bmw->mcss[ position ] );
  }
  else{
    MCSS_ResetVanishFlag( bmw->mcss[ position ] );
  }
}

//============================================================================================
/**
 * @brief ポケモンの初期立ち位置を取得
 *
 * @param[out]  pos     初期立ち位置を格納するワークへのポインタ
 * @param[in] position  取得するポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_GetPokeDefaultPos( VecFx32 *pos, int position )
{
  pos->x = poke_pos_table[ position ].x;
  pos->y = poke_pos_table[ position ].y;
  pos->z = poke_pos_table[ position ].z;
}

//============================================================================================
/**
 * @brief ポケモンの初期拡縮率を取得
 *
 * @param[in] position  取得するポケモンの立ち位置
 */
//============================================================================================
fx32  BTLV_MCSS_GetPokeDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
  return BTLV_MCSS_GetPokeDefaultScaleEx( bmw, position, bmw->poke_mcss_proj_mode );
}

//============================================================================================
/**
 * @brief ポケモンの初期拡縮率を取得
 *
 * @param[in] position  取得するポケモンの立ち位置
 * @param[in] proj      取得する射影方法
 */
//============================================================================================
fx32  BTLV_MCSS_GetPokeDefaultScaleEx( BTLV_MCSS_WORK *bmw, int position, BTLV_MCSS_PROJECTION proj )
{
  return poke_scale_table[ proj ][ position ];
}

//============================================================================================
/**
 * @brief ポケモンのスケール値を取得
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  取得するポケモンの立ち位置
 * @param[out]  scale   取得したスケール値を格納するワークへのポインタ
 */
//============================================================================================
void  BTLV_MCSS_GetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
  MCSS_GetScale( bmw->mcss[ position ], scale );
}

//============================================================================================
/**
 * @brief ポケモンのスケール値を格納
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  格納するポケモンの立ち位置
 * @param[in] scale   格納するスケール値
 */
//============================================================================================
void  BTLV_MCSS_SetScale( BTLV_MCSS_WORK *bmw, int position, VecFx32 *scale )
{
  VecFx32 shadow_scale;

  MCSS_SetScale( bmw->mcss[ position ], scale );
}

//============================================================================================
/**
 * @brief ポケモンの影バニッシュフラグを格納
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  格納するポケモンの立ち位置
 * @param[in] flag      格納するスケール値
 */
//============================================================================================
void  BTLV_MCSS_SetShadowVanishFlag( BTLV_MCSS_WORK *bmw, int position, u8 flag )
{
  MCSS_SetShadowVanishFlag( bmw->mcss[ position ], flag );
}

//============================================================================================
/**
 * @brief ポケモン移動
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  移動するポケモンの立ち位置
 * @param[in] type    移動タイプ
 * @param[in] pos     移動タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  移動先
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   移動フレーム数（目的地まで何フレームで到達するか）
 * @param[in] wait    移動ウエイト
 * @param[in] count   往復カウント（POKEMCSS_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MovePosition( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *pos, int frame, int wait, int count )
{
  VecFx32 start;

  MCSS_GetPosition( bmw->mcss[ position ], &start );
  //移動の補間は相対指定とする
  if( type == EFFTOOL_CALCTYPE_INTERPOLATION )
  { 
    //ポケモンの視線の方向で＋－を決定する
    if( position & 1 )
    { 
      pos->x = start.x - pos->x;
      pos->y += start.y;
      pos->z += start.z;
    }
    else
    {
      pos->x += start.x;
      pos->y += start.y;
      pos->z += start.z;
    }
  }
  //初期位置移動
  if( type == BTLEFF_POKEMON_MOVE_INIT )
  { 
    BTLV_MCSS_GetPokeDefaultPos( pos, position );
    type = EFFTOOL_CALCTYPE_INTERPOLATION;
  }
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, pos, frame, wait, count, TCB_BTLV_MCSS_Move );
  bmw->poke_mcss_tcb_move_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン拡縮
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  拡縮するポケモンの立ち位置
 * @param[in] type    拡縮タイプ
 * @param[in] scale   拡縮タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なスケール値
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param[in] wait    拡縮ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveScale( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *scale, int frame, int wait, int count )
{
  VecFx32 start;

  MCSS_GetOfsScale( bmw->mcss[ position ], &start );
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, scale, frame, wait, count, TCB_BTLV_MCSS_Scale );
  bmw->poke_mcss_tcb_scale_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン回転
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  回転するポケモンの立ち位置
 * @param[in] type    回転タイプ
 * @param[in] rotate    回転タイプにより意味が変化
 *              EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的な回転値
 *              EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   回転フレーム数（設定した回転値まで何フレームで到達するか）
 * @param[in] wait    回転ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveRotate( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *rotate, int frame, int wait, int count )
{
  VecFx32 start;

  MCSS_GetRotate( bmw->mcss[ position ], &start );
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, rotate, frame, wait, count, TCB_BTLV_MCSS_Rotate );
  bmw->poke_mcss_tcb_rotate_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモンまばたき
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  まばたきるポケモンの立ち位置
 * @param[in] type    まばたきイプ
 * @param[in] wait    まばたきウエイト
 * @param[in] count   まばたきカウント
 */
//============================================================================================
void  BTLV_MCSS_MoveBlink( BTLV_MCSS_WORK *bmw, int position, int type, int wait, int count )
{
  BTLV_MCSS_TCB_WORK  *pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

  pmtw->bmw       = bmw;
  pmtw->position      = position;
  pmtw->emw.move_type   = type;
  pmtw->emw.wait      = 0;
  pmtw->emw.wait_tmp    = wait;
  pmtw->emw.count     = count * 2;  //閉じて開くを1回とカウントするために倍しておく

  switch( type ){
  case BTLEFF_MEPACHI_ON:
    BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_ON );
    break;
  case BTLEFF_MEPACHI_OFF:
    BTLV_MCSS_SetMepachiFlag( bmw, position, BTLV_MCSS_MEPACHI_OFF );
    break;
  case BTLEFF_MEPACHI_MABATAKI:
    GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_Blink, pmtw, 0 );
    bmw->poke_mcss_tcb_blink_execute |= BTLV_EFFTOOL_Pos2Bit( position );
    break;
  }
}

//============================================================================================
/**
 * @brief ポケモンα値
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  α値セットするポケモンの立ち位置
 * @param[in] type      α値タイプ
 * @param[in] alpha     α値タイプにより意味が変化
 *                      EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なα値
 *                      EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame     フレーム数（設定したα値まで何フレームで到達するか）
 * @param[in] wait      ウエイト
 * @param[in] count     往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_MCSS_MoveAlpha( BTLV_MCSS_WORK *bmw, int position, int type, int alpha, int frame, int wait, int count )
{
  VecFx32 start;
  VecFx32 end;

  start.x = MCSS_GetAlpha( bmw->mcss[ position ] );
  start.y = 0;
  start.z = 0;
  end.x = alpha;
  end.y = 0;
  end.z = 0;
  BTLV_MCSS_TCBInitialize( bmw, position, type, &start, &end, frame, wait, count, TCB_BTLV_MCSS_Alpha );
  bmw->poke_mcss_tcb_alpha_execute |= BTLV_EFFTOOL_Pos2Bit( position );
}

//============================================================================================
/**
 * @brief ポケモン円運動
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] bmmcp     円運動パラメータ構造体
 */
//============================================================================================
void  BTLV_MCSS_MoveCircle( BTLV_MCSS_WORK *bmw, BTLV_MCSS_MOVE_CIRCLE_PARAM* bmmcp )
{ 
  BTLV_MCSS_MOVE_CIRCLE_PARAM* bmmcp_p = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_MOVE_CIRCLE_PARAM ) );

  bmmcp_p->bmw                      = bmw;
  bmmcp_p->position                 = bmmcp->position;
  bmmcp_p->axis                     = bmmcp->axis;
  bmmcp_p->shift                    = bmmcp->shift;
	bmmcp_p->radius_h                 = bmmcp->radius_h;
	bmmcp_p->radius_v                 = bmmcp->radius_v;
	bmmcp_p->frame                    = bmmcp->frame;
	bmmcp_p->rotate_wait              = bmmcp->rotate_wait;
	bmmcp_p->count                    = bmmcp->count;
	bmmcp_p->rotate_after_wait        = bmmcp->rotate_after_wait;
  bmmcp_p->rotate_wait_count        = 0;
  bmmcp_p->rotate_after_wait_count  = 0;

  //ポケモンの向きによって、＋－を決定
  if( bmmcp_p->position & 1 )
  { 
    bmmcp_p->shift ^= 1;
  }
  if( bmmcp_p->axis & 1 )
  { 
    bmmcp_p->angle  = 0x10000;
  }
  else
  { 
    bmmcp_p->angle  = 0;
  }
  bmmcp_p->speed  = 0x10000 / bmmcp->frame;

  GFL_TCB_AddTask( bmw->tcb_sys, TCB_BTLV_MCSS_MoveCircle, bmmcp_p, 0 );
  bmw->poke_mcss_tcb_move_execute |= BTLV_EFFTOOL_Pos2Bit( bmmcp_p->position );
}

//============================================================================================
/**
 * @brief タスクが起動中かチェック
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL  BTLV_MCSS_CheckTCBExecute( BTLV_MCSS_WORK *bmw, int position )
{
  BOOL  pal_fade_flag = FALSE;

  if( BTLV_MCSS_CheckExistPokemon( bmw, position ) )
  {
    pal_fade_flag = MCSS_CheckExecutePaletteFade( bmw->mcss[ position ] );
  }

  return ( ( bmw->poke_mcss_tcb_move_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->poke_mcss_tcb_scale_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->poke_mcss_tcb_rotate_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->poke_mcss_tcb_blink_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( bmw->poke_mcss_tcb_alpha_execute & BTLV_EFFTOOL_Pos2Bit( position ) ) ||
       ( pal_fade_flag ) );
}

//============================================================================================
/**
 * @brief 指定された立ち位置のポケモンが存在するかチェック
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 *
 * @retval  TRUE:存在する　FALSE:存在しない
 */
//============================================================================================
BOOL  BTLV_MCSS_CheckExistPokemon( BTLV_MCSS_WORK *bmw, int position )
{
  return (bmw->mcss[ position ] != NULL );
}

//============================================================================================
/**
 * @brief 指定された立ち位置のポケモンにパレットフェードをセットする
 *
 * @param[in] bmw       BTLV_MCSS管理ワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 * @param[in] start_evy セットするパラメータ（フェードさせる色に対する開始割合16段階）
 * @param[in] end_evy   セットするパラメータ（フェードさせる色に対する終了割合16段階）
 * @param[in] wait      セットするパラメータ（ウェイト）
 * @param[in] rgb       セットするパラメータ（フェードさせる色）
 *
 */
//============================================================================================
void  BTLV_MCSS_SetPaletteFade( BTLV_MCSS_WORK *bmw, int position, u8 start_evy, u8 end_evy, u8 wait, u32 rgb )
{
  MCSS_SetPaletteFade( bmw->mcss[ position ], start_evy, end_evy, wait, rgb );
}

//============================================================================================
/**
 * @brief POKEMON_PARAMからMCSS_ADD_WORKを生成する
 *
 * @param[in] pp      POKEMON_PARAM構造体へのポインタ
 * @param[out]  maw     MCSS_ADD_WORKワークへのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
static  void  BTLV_MCSS_MakeMAW( const POKEMON_PARAM *pp, MCSS_ADD_WORK *maw, int position )
{
  int dir = ( ( position & 1 ) ) ? MCSS_DIR_FRONT : MCSS_DIR_BACK;

  MCSS_TOOL_MakeMAWPP( pp, maw, dir );
}

//============================================================================================
/**
 * @brief tr_typeからMCSS_ADD_WORKを生成する
 *
 * @param[in]   pp      POKEMON_PARAM構造体へのポインタ
 * @param[out]  maw     MCSS_ADD_WORKワークへのポインタ
 * @param[in]   position  ポケモンの立ち位置
 */
//============================================================================================
static  void  BTLV_MCSS_MakeMAWTrainer( int tr_type, MCSS_ADD_WORK* maw, int position )
{ 
  int dir = ( ( position & 1 ) ) ? MCSS_DIR_FRONT : MCSS_DIR_BACK;

  MCSS_TOOL_MakeMAWTrainer( tr_type, maw, dir );
}

//============================================================================================
/**
 * @brief ポケモンデフォルトスケールセット
 */
//============================================================================================
static  void  BTLV_MCSS_SetDefaultScale( BTLV_MCSS_WORK *bmw, int position )
{
  VecFx32     scale;

  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  GF_ASSERT( bmw->mcss[ position ] );

  VEC_Set( &scale,
       poke_scale_table[ bmw->poke_mcss_proj_mode ][ position ],
       poke_scale_table[ bmw->poke_mcss_proj_mode ][ position ],
       FX32_ONE );

  MCSS_SetScale( bmw->mcss[ position ], &scale );

  VEC_Set( &scale,
       poke_scale_table[ 0 ][ position ],
       poke_scale_table[ 0 ][ position ] / 2,
       FX32_ONE );

  MCSS_SetShadowScale( bmw->mcss[ position ], &scale );
}

//============================================================================================
/**
 * @brief ポケモン操作系タスク初期化処理
 */
//============================================================================================
static  void  BTLV_MCSS_TCBInitialize( BTLV_MCSS_WORK *bmw, int position, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func )
{
  BTLV_MCSS_TCB_WORK  *pmtw = GFL_HEAP_AllocMemory( bmw->heapID, sizeof( BTLV_MCSS_TCB_WORK ) );

  pmtw->bmw               = bmw;
  pmtw->position          = position;
  pmtw->emw.move_type     = type;
  pmtw->emw.vec_time      = frame;
  pmtw->emw.vec_time_tmp  = frame;
  pmtw->emw.wait          = 0;
  pmtw->emw.wait_tmp      = wait;
  pmtw->emw.count         = count * 2;
  pmtw->emw.start_value.x = start->x;
  pmtw->emw.start_value.y = start->y;
  pmtw->emw.start_value.z = start->z;
  pmtw->emw.end_value.x   = end->x;
  pmtw->emw.end_value.y   = end->y;
  pmtw->emw.end_value.z   = end->z;

  switch( type ){
  case EFFTOOL_CALCTYPE_DIRECT:         //直接ポジションに移動
    break;
  case EFFTOOL_CALCTYPE_INTERPOLATION:  //移動先までを補間しながら移動
    BTLV_EFFTOOL_CalcMoveVector( &pmtw->emw.start_value, end, &pmtw->emw.vector, FX32_CONST( frame ) );
    break;
  case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG: //指定した区間を往復移動
    pmtw->emw.count         *= 2;
    //fall through
  case EFFTOOL_CALCTYPE_ROUNDTRIP:      //指定した区間を往復移動
    pmtw->emw.vector.x = FX_Div( end->x, FX32_CONST( frame ) );
    pmtw->emw.vector.y = FX_Div( end->y, FX32_CONST( frame ) );
    pmtw->emw.vector.z = FX_Div( end->z, FX32_CONST( frame ) );
    if( position & 1 )
    { 
      pmtw->emw.vector.x *= -1;
      pmtw->emw.vector.z *= -1;
    }
    break;
  }
  GFL_TCB_AddTask( bmw->tcb_sys, func, pmtw, 0 );
}

//============================================================================================
/**
 * @brief ポケモン移動タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Move( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_pos;
  BOOL  ret;

  MCSS_GetPosition( bmw->mcss[ pmtw->position ], &now_pos );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_pos );
  MCSS_SetPosition( bmw->mcss[ pmtw->position ], &now_pos );
  if( ret == TRUE ){
    bmw->poke_mcss_tcb_move_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン拡縮タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Scale( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_scale;
  BOOL  ret;

  MCSS_GetOfsScale( bmw->mcss[ pmtw->position ], &now_scale );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_scale );
  MCSS_SetOfsScale( bmw->mcss[ pmtw->position ], &now_scale );
  if( ret == TRUE ){
    bmw->poke_mcss_tcb_scale_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン回転タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Rotate( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_rotate;
  BOOL  ret;

  MCSS_GetRotate( bmw->mcss[ pmtw->position ], &now_rotate );
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_rotate );
  MCSS_SetRotate( bmw->mcss[ pmtw->position ], &now_rotate );
  if( ret == TRUE ){
    bmw->poke_mcss_tcb_rotate_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモンまばたきタスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Blink( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;

  if( pmtw->emw.wait == 0 ){
    pmtw->emw.wait = pmtw->emw.wait_tmp;
    BTLV_MCSS_SetMepachiFlag( pmtw->bmw, pmtw->position, BTLV_MCSS_MEPACHI_FLIP );
    if( --pmtw->emw.count == 0 ){
      bmw->poke_mcss_tcb_blink_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
      GFL_HEAP_FreeMemory( work );
      GFL_TCB_DeleteTask( tcb );
    }
  }
  else{
    pmtw->emw.wait--;
  }
}

//============================================================================================
/**
 * @brief ポケモンα値タスク
 */
//============================================================================================
static  void  TCB_BTLV_MCSS_Alpha( GFL_TCB *tcb, void *work )
{
  BTLV_MCSS_TCB_WORK  *pmtw = ( BTLV_MCSS_TCB_WORK * )work;
  BTLV_MCSS_WORK *bmw = pmtw->bmw;
  VecFx32 now_alpha;
  BOOL  ret;

  now_alpha.x = MCSS_GetAlpha( bmw->mcss[ pmtw->position ] );
  now_alpha.y = 0;
  now_alpha.z = 0;
  ret = BTLV_EFFTOOL_CalcParam( &pmtw->emw, &now_alpha );
  MCSS_SetAlpha( bmw->mcss[ pmtw->position ], now_alpha.x );
  if( ret == TRUE ){
    bmw->poke_mcss_tcb_alpha_execute &= ( BTLV_EFFTOOL_Pos2Bit( pmtw->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief ポケモン円運動タスク
 */
//============================================================================================
static  BTLV_MCSS_MOVE_CIRCLE_PARAM*  bmmcp_pp = NULL;
static  void  TCB_BTLV_MCSS_MoveCircle( GFL_TCB *tcb, void *work )
{ 
  BTLV_MCSS_MOVE_CIRCLE_PARAM*  bmmcp = ( BTLV_MCSS_MOVE_CIRCLE_PARAM * )work;
  BTLV_MCSS_WORK *bmw = bmmcp->bmw;
  VecFx32 ofs = { 0, 0, 0 };
  fx32  sin, cos;

  bmmcp_pp = bmmcp;

  if( bmmcp->rotate_after_wait_count ==0 )
  { 
    if( bmmcp->rotate_wait_count == bmmcp->rotate_wait )
    { 
      bmmcp->rotate_wait_count = 0;
      if( bmmcp->axis & 1 )
      { 
        bmmcp->angle -= bmmcp->speed;
      }
      else
      { 
        bmmcp->angle += bmmcp->speed;
      }
      if( bmmcp->angle & 0xffff0000 )
      { 
        bmmcp->angle &= 0x0000ffff;
        bmmcp->count--;
        bmmcp->rotate_after_wait_count = bmmcp->rotate_after_wait;
      }
      if( bmmcp->count )
      { 
        switch( bmmcp->shift ){ 
        case BTLEFF_SHIFT_H_P:    //シフトＨ＋
          sin = FX_Mul( FX_SinIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_h ) * -1;
          cos = FX_Mul( FX_CosIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_v ) * -1;
          sin += bmmcp->radius_h;
          break;
        case BTLEFF_SHIFT_H_M:    //シフトＨ－
          sin = FX_Mul( FX_SinIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_h ) - bmmcp->radius_h;
          cos = FX_Mul( FX_CosIdx( ( bmmcp->angle + 0x4000 ) & 0xffff ), bmmcp->radius_v );
          break;
        case BTLEFF_SHIFT_V_P:    //シフトＶ＋
          sin = FX_Mul( FX_SinIdx( bmmcp->angle ), bmmcp->radius_h ) * -1;
          cos = FX_Mul( FX_CosIdx( bmmcp->angle ), bmmcp->radius_v ) * -1;
          cos += bmmcp->radius_v;
          break;
        case BTLEFF_SHIFT_V_M:    //シフトＶ－
          sin = FX_Mul( FX_SinIdx( bmmcp->angle ), bmmcp->radius_h );
          cos = FX_Mul( FX_CosIdx( bmmcp->angle ), bmmcp->radius_v ) - bmmcp->radius_v;
          break;
        }
        switch( ( bmmcp->axis & 7 ) >> 1 ){ 
        default:
        case 0:   //X軸
          ofs.z = sin;
          ofs.y = cos;
          break;
        case 1:   //Y軸
          ofs.x = sin;
          ofs.z = cos;
          break;
        case 2:   //Z軸
          ofs.x = sin;
          ofs.y = cos;
          break;
        }
      }
      MCSS_SetOfsPosition( bmw->mcss[ bmmcp->position ], &ofs );
    }
    else
    {
      bmmcp->rotate_wait_count++;
    }
  }
  else
  { 
    bmmcp->rotate_after_wait_count--;
  }
  if( bmmcp->count == 0 ){
    bmw->poke_mcss_tcb_move_execute &= ( BTLV_EFFTOOL_Pos2Bit( bmmcp->position ) ^ BTLV_EFFTOOL_POS2BIT_XOR );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief 指定したフレームで呼ばれるコールバック関数
 */
//============================================================================================
static  void  BTLV_MCSS_CallBackFunctorFrame( u32 data, fx32 currentFrame )
{
  BTLV_MCSS_WORK *bmw = BTLV_EFFECT_GetMcssWork();

  bmw->callback_count[ data ]++;    //コールバックが呼ばれた回数をカウント
}

#ifdef PM_DEBUG
//============================================================================================
/**
 * @brief BTLV_MCSS追加（デバッグ用）
 *
 * @param[in] bmw     BTLV_MCSS管理ワークへのポインタ
 * @param[in] madw    MCSS_ADD_DEBUG_WORK構造体へのポインタ
 * @param[in] position  ポケモンの立ち位置
 */
//============================================================================================
void  BTLV_MCSS_AddDebug( BTLV_MCSS_WORK *bmw, const MCSS_ADD_DEBUG_WORK *madw, int position )
{
  GF_ASSERT( position < BTLV_MCSS_POS_TOTAL );
  if( bmw->mcss[ position ] ){
    BTLV_MCSS_Del( bmw, position );
  }

  bmw->mcss[ position ] = MCSS_AddDebug( bmw->mcss_sys,
                    poke_pos_table[ position ].x,
                    poke_pos_table[ position ].y,
                    poke_pos_table[ position ].z,
                    madw );

  BTLV_MCSS_SetDefaultScale( bmw, position );
}
#endif
