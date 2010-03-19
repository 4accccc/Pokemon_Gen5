//=============================================================================
/**
 *
 *	@file		demo3d_cmd.c
 *	@brief  3Dデモコマンド
 *	@author	hosaka genya
 *	@data		2009.12.09
 *
 */
//=============================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/main.h"
#include "system/brightness.h"

// 文字列関連
#include "font/font.naix"
#include "print/gf_font.h"
#include "print/wordset.h"
#include "print/printsys.h" //PRINT_QUE
#include "gamesystem/msgspeed.h"

//アーカイブ
#include "arc_def.h"
#include "message.naix"
#include "demo3d_cmd.h"

#include "sound/pm_sndsys.h" // for SEQ_SE_XXX

//=============================================================================
/**
 *								定数定義
 */
//=============================================================================

//=============================================================================
/**
 *								構造体定義
 */
//=============================================================================

//=============================================================================
/**
 *							プロトタイプ宣言
 */
//=============================================================================
static void DEMOCMD_SePlay(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_SeStop(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_SeVolumeEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_SePanEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_SePitchEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_BrightnessReq(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_FlashReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_MotionBL_Start(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_MotionBL_End(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);

static int sub_ConvFrame( DEMO3D_CMD_WORK* wk, int frame );


// DEMO3D_CMD_TYPE と対応
//--------------------------------------------------------------
///	コマンドテーブル (関数ポインタテーブル)
//==============================================================
void (*DATA_Demo3D_CmdTable[ DEMO3D_CMD_TYPE_MAX ])() = 
{ 
  NULL, // null
  DEMOCMD_SePlay,
  DEMOCMD_SeStop,
  DEMOCMD_SeVolumeEffectReq,
  DEMOCMD_SePanEffectReq,
  DEMOCMD_SePitchEffectReq,
  DEMOCMD_BrightnessReq,
  DEMOCMD_FlashReq,
  DEMOCMD_MotionBL_Start,
  DEMOCMD_MotionBL_End,
  NULL, // end
};

//-----------------------------------------------------------------------------
/**
 *	@brief  SE再生コマンド
 *
 *	@param	param[0] SE_Label
 *	@param	param[1] volume 0～127 : DEMO3D_SE_PARAM_DEFAULTなら無効
 *	@param	param[2] pan -128～127 ：DEMO3D_SE_PARAM_DEFAULTなら無効
 *	@param	param[3] pitch -32768～32767(+/-64で半音)：DEMO3D_SE_PARAM_DEFAULTなら無効
 *	@param	param[4] playerNo 0～3 : DEMO3D_SE_PARAM_DEFAULTなら無効
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SePlay(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  SEPLAYER_ID player_no;

  //プレイヤーNo
  if(param[4] != DEMO3D_SE_PARAM_DEFAULT){
    PMSND_PlaySE_byPlayerID( param[0], param[4] );
    player_no = param[4];
  }else{
    PMSND_PlaySE(param[0]);
    player_no = PMSND_GetSE_DefaultPlayerID( param[0] );
  }

  // volume
  if( param[1] != DEMO3D_SE_PARAM_DEFAULT )
  {
    PMSND_PlayerSetVolume( player_no, param[1] );
  }

  // pan
  if( param[2] != DEMO3D_SE_PARAM_DEFAULT )
  {
		PMSND_SetStatusSE_byPlayerID( player_no, PMSND_NOEFFECT, PMSND_NOEFFECT, param[2] );
  }

  //pitch
  if( param[3] != DEMO3D_SE_PARAM_DEFAULT){
		PMSND_SetStatusSE_byPlayerID( player_no, PMSND_NOEFFECT, param[3], PMSND_NOEFFECT );
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SE:再生停止
 *
 *	@param	param[0] SE_Label
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SeStop(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  SEPLAYER_ID player_no;

  player_no = PMSND_GetSE_DefaultPlayerID( param[0] );

  PMSND_StopSE_byPlayerID( player_no );
}

//===================================================================================
///SEの動的パラメータ変化リクエスト
//===================================================================================
typedef struct _TASKWK_SE_EFFECT_REQ{
  DEMO3D_CMD_WORK* cmd;

  u8  seq;
  u8  type;
  u8  playerNo;
  int frame;
  int start,end;
  fx32  now,diff;
}TASKWK_SE_EFFECT_REQ;

static void taskAdd_SeEffectReq( DEMO3D_CMD_WORK* wk, u8 type, u16 seNo, int start, int end, int frame,int playerNo );
static void tcb_SeEffectReq( GFL_TCBL *tcb , void* work);
static void sub_SetSeEffect( DEMO3D_SE_EFFECT type, u8 playerNo, int value );

//-----------------------------------------------------------------------------
/**
 *	@brief  SEのボリューム動的変化リクエスト
 *
 *  @param  param[0]  SE_Label
 *  @param  param[1]  スタートの値(0～127) 
 *  @param  param[2]  エンドの値(0～127)
 *  @param  param[3]  スタート～エンドまでのフレーム数(1～)
 *  @param  param[4]  プレイヤーNo(0～3 デフォルトでいい時はDEMO3D_SE_PARAM_DEFAULT)
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SeVolumeEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  taskAdd_SeEffectReq( wk, DEMO3D_SE_EFF_VOLUME, param[0], param[1], param[2], param[3], param[4] );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  SEのパン動的変化リクエスト
 *
 *  @param  param[0]  SE_Label
 *  @param  param[1]  スタートの値(-128～127) 
 *  @param  param[2]  エンドの値(-128～127)
 *  @param  param[3]  スタート～エンドまでのフレーム数(1～)
 *  @param  param[4]  プレイヤーNo(0～3 デフォルトでいい時はDEMO3D_SE_PARAM_DEFAULT)
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SePanEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  taskAdd_SeEffectReq( wk, DEMO3D_SE_EFF_PAN, param[0], param[1], param[2], param[3], param[4] );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  SEのピッチ動的変化リクエスト
 *
 *  @param  param[0]  SE_Label
 *  @param  param[1]  スタートの値(-32768～32767 +/-64で半音) 
 *  @param  param[2]  エンドの値(-32768～32767 +/-64で半音) 
 *  @param  param[3]  スタート～エンドまでのフレーム数(1～)
 *  @param  param[4]  プレイヤーNo(0～3 デフォルトでいい時はDEMO3D_SE_PARAM_DEFAULT)
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SePitchEffectReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  taskAdd_SeEffectReq( wk, DEMO3D_SE_EFF_PITCH, param[0], param[1], param[2], param[3], param[4] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  SEの動的パラメータ変化タスク追加
 *
 *  @param  param[0]  SE_Label
 *  @param  param[0]  適用する効果 DEMO3D_SE_EFFECT型 
 *  @param  param[1]  スタートの値 
 *  @param  param[2]  エンドの値 
 *  @param  param[3]  スタート～エンドまでのフレーム数(1～)
 *  @param  param[4]  プレイヤーNo(0～3 デフォルトでいい時はDEMO3D_SE_PARAM_DEFAULT)
 *
 *  スタート/エンドの値のmin/maxはtypeによって変わります
 *	  volume 0～127
 *	  pan -128～127
 *	  pitch -32768～32767(+/-64で半音)
 */
//-----------------------------------------------------------------------------
static void taskAdd_SeEffectReq( DEMO3D_CMD_WORK* wk, u8 type, u16 seNo, int start, int end, int frame,int playerNo )
{
  GFL_TCBL* tcb;
  TASKWK_SE_EFFECT_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_SeEffectReq, sizeof(TASKWK_SE_EFFECT_REQ), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_SE_EFFECT_REQ ));

  twk->cmd = wk;
  twk->type = type;
  twk->start = start;
  twk->end = end;
  twk->frame =  sub_ConvFrame( wk, frame );
  
  if( playerNo == DEMO3D_SE_PARAM_DEFAULT ){
    twk->playerNo = playerNo;
  }else{
    twk->playerNo = PMSND_GetSE_DefaultPlayerID( seNo );
  }
  
  twk->now = FX32_CONST(start);
  twk->diff = FX_Div((FX32_CONST(end)-twk->now),FX32_CONST(frame));
}

static void tcb_SeEffectReq( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_SE_EFFECT_REQ* twk = (TASKWK_SE_EFFECT_REQ*)tcb_wk;

  if( twk->frame-- > 0 ){
    sub_SetSeEffect( twk->type, twk->playerNo, FX_Whole(twk->now) );
    twk->now += twk->diff;
    return;
  }
  sub_SetSeEffect( twk->type, twk->playerNo, twk->end );

  GFL_TCBL_Delete(tcb);
}

static void sub_SetSeEffect( DEMO3D_SE_EFFECT type, u8 playerNo, int value )
{
  switch(type){
  case DEMO3D_SE_EFF_VOLUME:
    PMSND_PlayerSetVolume( playerNo, value );
    break;
  case DEMO3D_SE_EFF_PAN:
		PMSND_SetStatusSE_byPlayerID( playerNo, PMSND_NOEFFECT, PMSND_NOEFFECT, value );
    break;
  case DEMO3D_SE_EFF_PITCH:
		PMSND_SetStatusSE_byPlayerID( playerNo, PMSND_NOEFFECT, value, PMSND_NOEFFECT );
    break;
  default:
    GF_ASSERT(0);
  }
}

//-----------------------------------------------------------------------------
/**
 *	@brief  マスター輝度を用いたブライトネス 
 *
 *  @param  param[0]  適用する画面(1:main,2:sub,3:double) 
 *  @param  param[1]  何frameでフェードするか？ (0は開始時輝度値を即時反映)
 *  @param  param[2]  フェード開始時の輝度 < -16～16(黒～白) >
 *  @param  param[3]  フェード終了時の輝度 < -16～16(黒～白) >
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_BrightnessReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  if(param[1] == 0){
    SetBrightness( param[2], PLANEMASK_ALL, param[0]);
  }else{
    ChangeBrightnessRequest( sub_ConvFrame( wk, param[1] ), param[3], param[2],PLANEMASK_ALL, param[0] );
  }
}

//===================================================================================
///マスター輝度を用いたフラッシュリクエスト
//===================================================================================
typedef struct _TASKWK_FLASH_REQ{
  DEMO3D_CMD_WORK* cmd;
  DEMO3D_ENGINE_WORK* core;

  s8  seq;
  s8  start,max,end;
  int frame_s,frame_e;
  u8  lcd_mask;
}TASKWK_FLASH_REQ;

static void taskAdd_FlashReq( DEMO3D_CMD_WORK* wk, u8 lcd_mask, s8 start, s8 max, s8 end, int frame_s, int frame_e );
static void tcb_FlashReq( GFL_TCBL *tcb , void* work);

//-----------------------------------------------------------------------------
/**
 *	@brief  マスター輝度を用いたフラッシュリクエスト
 *
 *  @param  param[0]  適用する画面 < "main"(1),"sub"(2),"double"(3) >
 *  @param  param[1]  startの輝度 < -16～16(黒～白) >
 *  @param  param[2]  max輝度     < -16～16(黒～白) >
 *  @param  param[3]  endの輝度   < -16～16(黒～白) >
 *  @param  param[4]  startからmaxまでのフレーム < 1～ >
 *  @param  param[5]  maxからendまでのフレーム < 1～ >
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_FlashReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  taskAdd_FlashReq( wk, param[0], param[1], param[2], param[3], sub_ConvFrame( wk, param[4] ), sub_ConvFrame( wk, param[5] ) );
}

static void taskAdd_FlashReq( DEMO3D_CMD_WORK* wk, u8 lcd_mask, s8 start, s8 max, s8 end, int frame_s, int frame_e )
{
  GFL_TCBL* tcb;
  TASKWK_FLASH_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_FlashReq, sizeof(TASKWK_FLASH_REQ), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_FLASH_REQ ));

  twk->cmd = wk;
  twk->start = start;
  twk->max = max;
  twk->end = end;
  twk->lcd_mask = lcd_mask;
  twk->frame_s = frame_s;
  twk->frame_e = frame_e;
   
  ChangeBrightnessRequest( twk->frame_s, twk->max, twk->start, PLANEMASK_ALL, twk->lcd_mask );
}

static void tcb_FlashReq( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_FLASH_REQ* twk = (TASKWK_FLASH_REQ*)tcb_wk;

  switch(twk->seq){
  case 0:
    if( IsFinishedBrightnessChg( twk->lcd_mask )){
      ChangeBrightnessRequest( twk->frame_e, twk->end, twk->max, PLANEMASK_ALL, twk->lcd_mask );
      twk->seq++;
    }
    return;
  case 1:
    if( IsFinishedBrightnessChg( twk->lcd_mask )){
      break;
    }
    return;
  }
  GFL_TCBL_Delete(tcb);
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ライトカラー変更 
 *
 *	@param	param[0]  light_no(0～3)
 *	@param  param[1]  light_col_r(0～31)
 *	@param  param[2]  light_col_g(0～31)
 *	@param  param[3]  light_col_b(0～31)
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_LightColorSet(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
#if 0
  GRAPHIC_G3D_WORK* g3d = &core->graphic->g3d;
//  GXRgb col = ;

  //ライト再設定
	GFL_G3D_LIGHT_SetColor( g3d->p_lightset, param[0], (u16*)&f_light->light);
  GFL_G3D_LIGHT_Switching( g3d->p_lightset );
#endif
}

//-----------------------------------------------------------------------------
/**
 *	@brief  モーションブラー 開始
 *
 *	@param	wk
 *	@param	param[0] モーションブラー係数 新しくブレンドする絵のα係数
 *	@param	param[1] モーションブラー係数 既にバッファされている絵のα係数
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_MotionBL_Start(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  GF_ASSERT( wk->mb == NULL );

	wk->mb = DEMO3D_MotionBlInit( wk->tcbsys, param[0], param[1] );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  モーションブラー 停止
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_MotionBL_End(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  DEMO3D_MotionBlExit( wk->mb );
  wk->mb = NULL;
}

//=============================================================================
/**
 *								その他サブルーチン
 */
//=============================================================================

//-----------------------------------------------------------------------------
/*
 *  @brief  指定のフレーム数を、フレームレートに応じて変換する
 *
 *  ※タスク型のコマンドは必ず60fpsで動くので、30fps設定のデモで指定されたフレーム数は
 *   60fps換算に計算しなおす
 */
//-----------------------------------------------------------------------------
static int sub_ConvFrame( DEMO3D_CMD_WORK* wk, int frame )
{
  if(wk->core->scene->frame_rate == DEMO3D_FRAME_RATE_30FPS ){
    frame *= 2;
  }
  return frame;
}


