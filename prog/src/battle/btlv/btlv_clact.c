
//============================================================================================
/**
 * @file  btlv_clact.c
 * @brief 戦闘エフェクト制御
 * @author  soga
 * @date  2008.11.21
 */
//============================================================================================

#include <gflib.h>

#include "system/gfl_use.h"

#include "btlv_effect.h"
#include "btlv_clact.h"

//============================================================================================
/**
 *  定数宣言
 */
//============================================================================================

//============================================================================================
/**
 *  構造体宣言
 */
//============================================================================================

struct _BTLV_CLACT_CLWK
{
  GFL_CLWK* clwk;
  u32       charID;
  u32       plttID;
  u32       cellID;
};

struct _BTLV_CLACT_WORK
{
  GFL_TCBSYS*     tcb_sys;
  GFL_CLUNIT*     clunit;
  BTLV_CLACT_CLWK bccl[ BTLV_CLACT_CLWK_MAX ];
  u32             clact_tcb_move_execute;
  u32             clact_tcb_scale_execute;
  HEAPID          heapID;
};

typedef struct
{
  BTLV_CLACT_WORK*  bclw;
  VecFx32           now_value;
  int               index;
  EFFTOOL_MOVE_WORK emw;
}BTLV_CLACT_TCB_WORK;

//============================================================================================
/**
 *  プロトタイプ宣言
 */
//============================================================================================
static  void  BTLV_CLACT_TCBInitialize( BTLV_CLACT_WORK *bclw, int index, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func );
static  void  TCB_BTLV_CLACT_Move( GFL_TCB *tcb, void *work );
static  void  TCB_BTLV_CLACT_Scale( GFL_TCB *tcb, void *work );

//============================================================================================
/**
 *  システム初期化
 *
 * @param[in] vramBank  初期化に必要なVRAMBANKマップ
 * @param[in] heapID    ヒープID
 *
 * @retval  bclw  BTLV_CLACT_WORK管理構造体へのポインタ
 */
//============================================================================================
BTLV_CLACT_WORK*  BTLV_CLACT_Init( GFL_TCBSYS *tcb_sys, HEAPID heapID )
{
  BTLV_CLACT_WORK *bclw = GFL_HEAP_AllocClearMemory( heapID, sizeof( BTLV_CLACT_WORK ) );

  bclw->tcb_sys = tcb_sys;
  bclw->heapID  = heapID;

  bclw->clunit = GFL_CLACT_UNIT_Create( BTLV_CLACT_CLWK_MAX, 0, heapID );

  return bclw;
}

//============================================================================================
/**
 *  システム終了
 *
 * @param[in] bclw  BTLV_CLACT_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_CLACT_Exit( BTLV_CLACT_WORK *bclw )
{
  int index;

  for( index = 0 ; index < BTLV_CLACT_CLWK_MAX ; index++ )
  {
    if( bclw->bccl[ index ].clwk )
    {
      BTLV_CLACT_Delete( bclw, index );
    }
  }
  GFL_CLACT_UNIT_Delete( bclw->clunit );
  GFL_HEAP_FreeMemory( bclw );
}

//============================================================================================
/**
 *  システムメイン
 *
 * @param[in] bclw  BTLV_CLACT_WORK管理構造体へのポインタ
 */
//============================================================================================
void  BTLV_CLACT_Main( BTLV_CLACT_WORK *bclw )
{
  GFL_CLACT_SYS_Main();
}

//============================================================================================
/**
 *  CLWK生成
 *
 *  ARCDATIDは、naixの並びが、NCGR、NCLR、NCER、NANRであることを期待しています
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] arcID   グラフィックリソースのARCID
 * @param[in] ncgrID  グラフィックリソース（ncgr）のARCDATID
 * @param[in] posx    初期X座標
 * @param[in] posy    初期Y座標
 *
 * @retval  登録したINDEXナンバー
 */
//============================================================================================
int BTLV_CLACT_Add( BTLV_CLACT_WORK *bclw, ARCID arcID, ARCDATID ncgrID, s16 posx, s16 posy )
{
  return BTLV_CLACT_AddEx( bclw, arcID, ncgrID, ncgrID + 1, ncgrID + 2, ncgrID + 3, posx, posy, 0, 0 );
}

//============================================================================================
/**
 *  CLWK生成（Affine値も設定可）
 *
 *  ARCDATIDは、naixの並びが、NCGR、NCLR、NCER、NANRであることを期待しています
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] arcID   グラフィックリソースのARCID
 * @param[in] ncgrID  グラフィックリソース（ncgr）のARCDATID
 * @param[in] posx    初期X座標
 * @param[in] posy    初期Y座標
 * @param[in] scalex  初期ScaleX値
 * @param[in] scaley  初期ScaleY値
 *
 * @retval  登録したINDEXナンバー
 */
//============================================================================================
int BTLV_CLACT_AddAffine( BTLV_CLACT_WORK *bclw, ARCID arcID, ARCDATID ncgrID, s16 posx, s16 posy, fx32 scalex, fx32 scaley )
{
  return BTLV_CLACT_AddEx( bclw, arcID, ncgrID, ncgrID + 1, ncgrID + 2, ncgrID + 3, posx, posy, scalex, scaley );
}

//============================================================================================
/**
 *  CLWK生成
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] arcID   グラフィックリソースのARCID
 * @param[in] ncgrID  グラフィックリソース（ncgr）のARCDATID
 * @param[in] nclrID  グラフィックリソース（nclr）のARCDATID
 * @param[in] ncerID  グラフィックリソース（ncer）のARCDATID
 * @param[in] nanrID  グラフィックリソース（nanr）のARCDATID
 * @param[in] posx    初期X座標
 * @param[in] posy    初期Y座標
 * @param[in] scalex  初期X座標
 * @param[in] scaley  初期Y座標
 *
 * @retval  index 登録したINDEXナンバー
 */
//============================================================================================
int BTLV_CLACT_AddEx( BTLV_CLACT_WORK *bclw, ARCID arcID,
                      ARCDATID ncgrID, ARCDATID nclrID, ARCDATID ncerID, ARCDATID nanrID,
                      s16 posx, s16 posy, fx32 scalex, fx32 scaley )
{
  ARCHANDLE     *hdl;
  int           index;
  GFL_CLWK_AFFINEDATA CLWKAffineParam = { 
    {
      0, 0,                   //x, y
      0, 0, 1,                //アニメ番号、優先順位、BGプライオリティ
    },
		0,		                    // アフィンｘ座標
	  0,		                    // アフィンｙ座標
	  FX32_ONE,			            // 拡大ｘ値
	  FX32_ONE,			            // 拡大ｙ値
	  0,                        // 回転角度(0～0xffff 0xffffが360度)
	  CLSYS_AFFINETYPE_NORMAL,  // 上書きアフィンタイプ（CLSYS_AFFINETYPE）
  };

  hdl = GFL_ARC_OpenDataHandle( arcID, bclw->heapID );

  for( index = 0 ; index < BTLV_CLACT_CLWK_MAX ; index++ )
  {
    if( bclw->bccl[ index ].clwk == NULL )
    {
      break;
    }
  }

  GF_ASSERT( index < BTLV_CLACT_CLWK_MAX );

  bclw->bccl[ index ].charID = GFL_CLGRP_CGR_Register( hdl, ncgrID, FALSE, CLSYS_DRAW_MAIN, bclw->heapID );
  bclw->bccl[ index ].plttID = GFL_CLGRP_PLTT_RegisterComp( hdl, nclrID, CLSYS_DRAW_MAIN,
                                                            BTLV_OBJ_PLTT_CLACT + 0x20 * index, bclw->heapID );
  bclw->bccl[ index ].cellID = GFL_CLGRP_CELLANIM_Register( hdl, ncerID, nanrID, bclw->heapID );

  //パレットをPaletteWorkにロード
  PaletteWorkSet_Arc( BTLV_EFFECT_GetPfd(), arcID, nclrID, bclw->heapID, FADE_MAIN_OBJ, 0x20,
    ( GFL_CLGRP_PLTT_GetAddr( bclw->bccl[ index ].plttID, CLSYS_DRAW_MAIN ) & 0x3ff ) / 2 );

  CLWKAffineParam.clwkdata.pos_x = posx;
  CLWKAffineParam.clwkdata.pos_y = posy;

  if( ( scalex == 0 ) && ( scaley == 0 ) )
  { 
    bclw->bccl[ index ].clwk = GFL_CLACT_WK_Create( bclw->clunit,
                                                    bclw->bccl[ index ].charID,
                                                    bclw->bccl[ index ].plttID,
                                                    bclw->bccl[ index ].cellID,
                                                    &CLWKAffineParam.clwkdata, CLSYS_DEFREND_MAIN, bclw->heapID );
  }
  else
  { 
    CLWKAffineParam.scale_x = scalex;
    CLWKAffineParam.scale_y = scaley;
    bclw->bccl[ index ].clwk = GFL_CLACT_WK_CreateAffine( bclw->clunit,
                                                          bclw->bccl[ index ].charID,
                                                          bclw->bccl[ index ].plttID,
                                                          bclw->bccl[ index ].cellID,
                                                          &CLWKAffineParam, CLSYS_DEFREND_MAIN, bclw->heapID );
  }

  GFL_CLACT_WK_SetAutoAnmFlag( bclw->bccl[ index ].clwk, TRUE );

  GFL_ARC_CloseDataHandle( hdl );

  return index;
}

//============================================================================================
/**
 *  CLWKの破棄
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] index   破棄するCLWKの管理インデックス
 */
//============================================================================================
void  BTLV_CLACT_Delete( BTLV_CLACT_WORK *bclw, int index )
{
  GF_ASSERT( bclw );
  GF_ASSERT( bclw->bccl[ index ].clwk != NULL );

  GFL_CLGRP_CGR_Release( bclw->bccl[ index ].charID );
  GFL_CLGRP_PLTT_Release( bclw->bccl[ index ].plttID );
  GFL_CLGRP_CELLANIM_Release( bclw->bccl[ index ].cellID );

  GFL_CLACT_WK_Remove( bclw->bccl[ index ].clwk );

  bclw->bccl[ index ].clwk = NULL;
}

//============================================================================================
/**
 *  座標移動
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理ワークへのポインタ
 * @param[in] index   移動するCLWKのインデックス
 * @param[in] type    移動タイプ
 * @param[in] pos     移動タイプにより意味が変化
 *                    EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  移動先
 *                    EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   移動フレーム数（目的地まで何フレームで到達するか）
 * @param[in] wait    移動ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_CLACT_MovePosition( BTLV_CLACT_WORK *bclw, int index, int type, GFL_CLACTPOS *pos, int frame, int wait, int count )
{
  GFL_CLACTPOS  start;
  VecFx32       start_fx32;
  VecFx32       pos_fx32;

  pos_fx32.x = pos->x << FX32_SHIFT;
  pos_fx32.y = pos->y << FX32_SHIFT;
  pos_fx32.z = 0;

  GFL_CLACT_WK_GetPos( bclw->bccl[ index ].clwk, &start, CLSYS_DEFREND_MAIN );
  start_fx32.x = start.x << FX32_SHIFT;
  start_fx32.y = start.y << FX32_SHIFT;
  start_fx32.z = 0;

  //移動の補間は相対指定とする
  if( type == EFFTOOL_CALCTYPE_INTERPOLATION )
  { 
    pos_fx32.x += start_fx32.x;
    pos_fx32.y += start_fx32.y;
    pos_fx32.z += start_fx32.z;
  }

  BTLV_CLACT_TCBInitialize( bclw, index, type, &start_fx32, &pos_fx32, frame, wait, count, TCB_BTLV_CLACT_Move );
  bclw->clact_tcb_move_execute |= BTLV_EFFTOOL_No2Bit( index );
}

//============================================================================================
/**
 * @brief OBJ拡縮
 *
 * @param[in] bclw    BTLV_CLACT管理ワークへのポインタ
 * @param[in] index   拡縮するCLWKのインデックス
 * @param[in] type    拡縮タイプ
 * @param[in] scale   拡縮タイプにより意味が変化
 *                    EFFTOOL_CALCTYPE_DIRECT EFFTOOL_CALCTYPE_INTERPOLATION  最終的なスケール値
 *                    EFFTOOL_CALCTYPE_ROUNDTRIP　往復の長さ
 * @param[in] frame   拡縮フレーム数（設定した拡縮値まで何フレームで到達するか）
 * @param[in] wait    拡縮ウエイト
 * @param[in] count   往復カウント（EFFTOOL_CALCTYPE_ROUNDTRIPでしか意味のないパラメータ）
 */
//============================================================================================
void  BTLV_CLACT_MoveScale( BTLV_CLACT_WORK *bclw, int index, int type, VecFx32 *scale, int frame, int wait, int count )
{
  VecFx32 start;

  start.x = FX32_ONE;
  start.y = FX32_ONE;
  start.z = FX32_ONE;

  BTLV_CLACT_TCBInitialize( bclw, index, type, &start, scale, frame, wait, count, TCB_BTLV_CLACT_Scale );
  bclw->clact_tcb_scale_execute |= BTLV_EFFTOOL_No2Bit( index );
}


//============================================================================================
/**
 *  アニメシーケンスセット
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] index   CLWKの管理インデックス
 * @param[in] anm_no  セットするアニメシーケンスナンバー
 */
//============================================================================================
void  BTLV_CLACT_SetAnime( BTLV_CLACT_WORK *bclw, int index, int anm_no )
{
  GFL_CLACT_WK_SetAnmSeq( bclw->bccl[ index ].clwk, anm_no );
}

//============================================================================================
/**
 *  パレットフェード
 *
 * @param[in] bclw      BTLV_CLACT_WORK管理構造体へのポインタ
 * @param[in] index     CLWKの管理インデックス
 * @param[in] start_evy
 * @param[in] end_evy 
 * @param[in] wait
 * @param[in] rgb 
 */
//============================================================================================
void  BTLV_CLACT_SetPaletteFade( BTLV_CLACT_WORK *bclw, int index, u8 start_evy, u8 end_evy, s8 wait, u16 rgb )
{ 
  u16 pltt_bit = 1 << ( ( GFL_CLGRP_PLTT_GetAddr( bclw->bccl[ index ].plttID, CLSYS_DRAW_MAIN ) & 0x3ff ) / 0x20 );

  PaletteFadeReq( BTLV_EFFECT_GetPfd(), PF_BIT_MAIN_OBJ, pltt_bit, wait, start_evy, end_evy, rgb, BTLV_EFFECT_GetTCBSYS() );
}

//============================================================================================
/**
 *  タスクが起動中かチェック
 *
 * @param[in] bclw    BTLV_CLACT_WORK管理ワークへのポインタ
 * @param[in] index   CLWKの管理インデックス
 *
 * @retval: TRUE:起動中　FALSE:終了
 */
//============================================================================================
BOOL  BTLV_CLACT_CheckTCBExecute( BTLV_CLACT_WORK *bclw, int index )
{
  if( bclw->bccl[ index ].clwk == NULL )
  {
    return FALSE;
  }
  return ( ( bclw->clact_tcb_move_execute & BTLV_EFFTOOL_No2Bit( index ) ) ||
           ( GFL_CLACT_WK_CheckAnmActive( bclw->bccl[ index ].clwk ) ) );
}


//============================================================================================
/**
 *  CLWK操作系タスク初期化処理
 */
//============================================================================================
static  void  BTLV_CLACT_TCBInitialize( BTLV_CLACT_WORK *bclw, int index, int type, VecFx32 *start, VecFx32 *end, int frame, int wait, int count, GFL_TCB_FUNC *func )
{
  BTLV_CLACT_TCB_WORK *bctw = GFL_HEAP_AllocMemory( bclw->heapID, sizeof( BTLV_CLACT_TCB_WORK ) );

  GF_ASSERT( index < BTLV_CLACT_CLWK_MAX );

  bctw->bclw              = bclw;
  bctw->index             = index;
  bctw->now_value.x       = start->x;
  bctw->now_value.y       = start->y;
  bctw->now_value.z       = start->z;
  bctw->emw.move_type     = type;
  bctw->emw.vec_time      = frame;
  bctw->emw.vec_time_tmp  = frame;
  bctw->emw.wait          = 0;
  bctw->emw.wait_tmp      = wait;
  bctw->emw.count         = count * 2;
  bctw->emw.start_value.x = start->x;
  bctw->emw.start_value.y = start->y;
  bctw->emw.start_value.z = start->z;
  bctw->emw.end_value.x   = end->x;
  bctw->emw.end_value.y   = end->y;
  bctw->emw.end_value.z   = end->z;

  switch( type ){
  case EFFTOOL_CALCTYPE_DIRECT:         //直接ポジションに移動
    break;
  case EFFTOOL_CALCTYPE_INTERPOLATION:  //移動先までを補間しながら移動
    BTLV_EFFTOOL_CalcMoveVector( &bctw->emw.start_value, end, &bctw->emw.vector, FX32_CONST( frame ) );
    break;
  case EFFTOOL_CALCTYPE_ROUNDTRIP_LONG: //指定した区間を往復移動
    bctw->emw.vec_time_tmp  *= 2;
    bctw->emw.count         *= 2;
    //fall through
  case EFFTOOL_CALCTYPE_ROUNDTRIP:      //指定した区間を往復移動
    bctw->emw.vector.x = FX_Div( end->x, FX32_CONST( frame ) );
    bctw->emw.vector.y = FX_Div( end->y, FX32_CONST( frame ) );
    bctw->emw.vector.z = FX_Div( end->z, FX32_CONST( frame ) );
    break;
  }
  GFL_TCB_AddTask( bclw->tcb_sys, func, bctw, 0 );
}

//============================================================================================
/**
 *  CLWK移動タスク
 */
//============================================================================================
static  void  TCB_BTLV_CLACT_Move( GFL_TCB *tcb, void *work )
{
  BTLV_CLACT_TCB_WORK *bctw = ( BTLV_CLACT_TCB_WORK * )work;
  BTLV_CLACT_WORK *bclw = bctw->bclw;
  GFL_CLACTPOS  now_pos;
  BOOL          ret;

  ret = BTLV_EFFTOOL_CalcParam( &bctw->emw, &bctw->now_value );
  now_pos.x = bctw->now_value.x >> FX32_SHIFT;
  now_pos.y = bctw->now_value.y >> FX32_SHIFT;
  GFL_CLACT_WK_SetPos( bclw->bccl[ bctw->index ].clwk, &now_pos, CLSYS_DEFREND_MAIN );
  if( ret == TRUE )
  {
    bclw->clact_tcb_move_execute &= ( BTLV_EFFTOOL_No2Bit( bctw->index ) ^ 0xffffffff );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

//============================================================================================
/**
 * @brief CLWK拡縮タスク
 */
//============================================================================================
static  void  TCB_BTLV_CLACT_Scale( GFL_TCB *tcb, void *work )
{
  BTLV_CLACT_TCB_WORK *bctw = ( BTLV_CLACT_TCB_WORK * )work;
  BTLV_CLACT_WORK *bclw = bctw->bclw;
  GFL_CLSCALE now_scale;
  BOOL  ret;

  ret = BTLV_EFFTOOL_CalcParam( &bctw->emw, &bctw->now_value );
  now_scale.x = bctw->now_value.x;
  now_scale.y = bctw->now_value.y;
  GFL_CLACT_WK_SetAffineParam( bclw->bccl[ bctw->index ].clwk, CLSYS_AFFINETYPE_NORMAL );
  GFL_CLACT_WK_SetScale( bclw->bccl[ bctw->index ].clwk, &now_scale );
  if( ret == TRUE ){
    bclw->clact_tcb_scale_execute &= ( BTLV_EFFTOOL_No2Bit( bctw->index ) ^ 0xffffffff );
    GFL_HEAP_FreeMemory( work );
    GFL_TCB_DeleteTask( tcb );
  }
}

