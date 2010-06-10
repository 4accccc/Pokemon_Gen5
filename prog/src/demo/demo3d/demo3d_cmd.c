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
#include "system/bmp_winframe.h"

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
static void DEMOCMD_BgmPlay(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_BgmStop(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_BgmFade(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_BgmChangeReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_BrightnessReq(DEMO3D_CMD_WORK* wk, DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_FlashReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_LightColorSet(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_LightVectorSet(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_TalkWinReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
static void DEMOCMD_ScrDrawReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param);
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
  DEMOCMD_BgmPlay,
  DEMOCMD_BgmStop,
  DEMOCMD_BgmFade,
  DEMOCMD_BgmChangeReq,
  DEMOCMD_BrightnessReq,
  DEMOCMD_FlashReq,
  DEMOCMD_LightColorSet,
  DEMOCMD_LightVectorSet,
  DEMOCMD_TalkWinReq,
  DEMOCMD_ScrDrawReq,
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
 *	@param	param[1] playerNo 0～3 : DEMO3D_SE_PARAM_DEFAULTなら無効
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_SeStop(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  SEPLAYER_ID player_no;

  if( param[1] == DEMO3D_SE_PARAM_DEFAULT){
    player_no = PMSND_GetSE_DefaultPlayerID( param[0] );
  }else{
    player_no = param[1];
  }
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

//===================================================================================
///BGM設定
//===================================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  BGM再生 
 *
 *	@param	param[0] BGM_Label
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_BgmPlay(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  PMSND_PlayBGM( param[0] );
}
//-----------------------------------------------------------------------------
/**
 *	@brief  BGMストップ
 *
 *	@param  none	
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_BgmStop(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  PMSND_StopBGM();
}

//-----------------------------------------------------------------------------
/**
 *	@brief  BGMフェード
 *
 *	@param  param[0]	フェードパターン(0:fadein,1:fadeout) 
 *  @param  param[1]  何frameでフェードするか？
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_BgmFade(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  int frame = sub_ConvFrame( wk, param[1] );

  if(param[0] == 0){
    PMSND_FadeInBGM( frame );
  }else{
    PMSND_FadeOutBGM( frame );
  }
}

//===================================================================================
///BGM変更リクエスト
//===================================================================================
typedef struct _TASKWK_BGM_CHANGE_REQ{
  DEMO3D_CMD_WORK* cmd;
  DEMO3D_ENGINE_WORK* core;

  u8  seq;
  u8  push_f;
  u8  pop_f;
  u8  type;
  int bgm_no;
  int fadeout_frame;
  int fadein_frame;

}TASKWK_BGM_CHANGE_REQ;

static void taskAdd_BgmChangeReq( DEMO3D_CMD_WORK* wk, int bgm_no, int fadeout_frame, int fadein_frame, int push_pop );
static void tcb_BgmChangeReq( GFL_TCBL *tcb , void* work);

//-----------------------------------------------------------------------------
/**
 *	@brief  BGM変更リクエスト
 *
 *  @param  param[0]  BGM-No(0～65535)
 *  @param  param[1]  fadeout_frame(0～65535) 
 *  @param  param[2]  fadein_frame(0～65535)
 *  @param  param[3]  type("play"=0,"stop"=1,"change"=2,"push"=3,"pop" =4)
 *
 *  typeはDEMO3D_BGM_CHG_TYPE型(demo3d_cmd_def.h)
 *
 *  @note 連続スリープにより予測不能な大規模遅延が発生した場合の挙動について問題あり
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_BgmChangeReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  int bgm_no = param[0];
  int fadeout_frame = sub_ConvFrame(wk,param[1]);
  int fadein_frame = sub_ConvFrame(wk,param[2]);
  int type = param[3];

  if( type == DEMO3D_BGM_PLAY ){ //play
    PMSND_PlayBGM( bgm_no );
    PMSND_FadeInBGM( fadein_frame );
  }else{
    taskAdd_BgmChangeReq( wk, bgm_no, fadeout_frame, fadein_frame, type );
  }
}

static void taskAdd_BgmChangeReq( DEMO3D_CMD_WORK* wk, int bgm_no, int fadeout_frame, int fadein_frame, int type )
{
  GFL_TCBL* tcb;
  TASKWK_BGM_CHANGE_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_BgmChangeReq, sizeof(TASKWK_BGM_CHANGE_REQ), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BGM_CHANGE_REQ ));

  twk->cmd = wk;
  twk->type = type;
  twk->push_f = ( type == DEMO3D_BGM_PUSH );
  twk->pop_f = ( type == DEMO3D_BGM_POP );
  
  twk->bgm_no = bgm_no;
  twk->fadeout_frame = fadeout_frame;
  twk->fadein_frame = fadein_frame;
  
  if( twk->fadeout_frame ){
    PMSND_FadeOutBGM( twk->fadeout_frame );
  }else{
    twk->seq = 1; //フェード待ち処理スキップ
  }
}

static void tcb_BgmChangeReq( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BGM_CHANGE_REQ* twk = (TASKWK_BGM_CHANGE_REQ*)tcb_wk;

  switch(twk->seq){
  case 0:
    if( PMSND_CheckFadeOnBGM()){
      return;
    }
    twk->seq++;
    //ブレイクスルー 
  case 1:
    if( twk->push_f ){
      if( !twk->cmd->bgm_push_f ){
        PMSND_PauseBGM(TRUE);
        PMSND_PushBGM();
        twk->cmd->bgm_push_f = TRUE;
      }else{
        GF_ASSERT_MSG(0,"BGMのpushは一回までです\nコマンド列を見直してください\n");
      }
    }else{
      PMSND_StopBGM();
      if( twk->type == DEMO3D_BGM_STOP ){
        break;
      }
    }
    if( twk->pop_f ){
      if( twk->cmd->bgm_push_f ) {
        PMSND_PopBGM();
        PMSND_PauseBGM(FALSE);
        twk->cmd->bgm_push_f = FALSE;
      }else{
        GF_ASSERT_MSG(0,"BGMがpushされずにpopされようとしています\nコマンド列を見直してください\n");
      }
    }else{
      PMSND_PlayBGM( twk->bgm_no );
    }
    if( twk->fadein_frame ){
      PMSND_FadeInBGM( twk->fadein_frame );
    }
    break;
  }
  GFL_TCBL_Delete(tcb);
}


//===================================================================================
///輝度変化エフェクトを用いたブライトネスリクエスト
//===================================================================================
typedef struct _TASKWK_BRIGHTNESS_REQ{
  DEMO3D_CMD_WORK* cmd;

  u8  seq;
  BRIGHT_DISPMASK disp_mask;
  BRIGHT_PLANEMASK plane_mask;
  int frame;
  int start,end;
  fx32  now,diff;
  
  int* task_ct;
}TASKWK_BRIGHTNESS_REQ;

static void taskAdd_BrightnessReq( DEMO3D_CMD_WORK* wk,
    BRIGHT_DISPMASK disp_mask, BRIGHT_PLANEMASK plane_mask,int start, int end, int frame, int* task_ct );
static void tcb_BrightnessReq( GFL_TCBL *tcb , void* tcb_wk);

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度変化エフェクトを用いたブライトネス 
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
    taskAdd_BrightnessReq( wk, param[0], PLANEMASK_ALL, param[2], param[3], sub_ConvFrame( wk, param[1]), NULL );
  }
}
//-----------------------------------------------------------------------------
/**
 *	@brief  輝度変化タスク追加
 *
 *  @param  disp_mask  適用する画面(1:main,2:sub,3:double) 
 *  @param  start   スタート輝度
 *  @param  end     エンド輝度
 *  @param  frame   スタート～エンドまでのフレーム数(1～)
 *  @param  task_ct タスクの監視(NULL可)
 */
//-----------------------------------------------------------------------------
static void taskAdd_BrightnessReq( DEMO3D_CMD_WORK* wk,
    BRIGHT_DISPMASK disp_mask, BRIGHT_PLANEMASK plane_mask,int start, int end, int frame, int* task_ct )
{
  GFL_TCBL* tcb;
  TASKWK_BRIGHTNESS_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_BrightnessReq, sizeof(TASKWK_BRIGHTNESS_REQ), 0 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_BRIGHTNESS_REQ ));

  twk->cmd = wk;
  twk->disp_mask = disp_mask;
  twk->plane_mask = plane_mask;
  twk->start = start;
  twk->end = end;
  twk->frame =  frame;
  twk->task_ct = task_ct;

  twk->now = FX32_CONST(start);
  twk->diff = FX_Div((FX32_CONST(end)-twk->now),FX32_CONST(frame));
  
  if(task_ct != NULL){
    (*task_ct)++;
  }
  SetBrightness( twk->start, twk->plane_mask, twk->disp_mask );
}

static void tcb_BrightnessReq( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_BRIGHTNESS_REQ* twk = (TASKWK_BRIGHTNESS_REQ*)tcb_wk;
  
  //システム終了で自殺
  if( twk->cmd->cmdsys_end_f) {
    SetBrightness( twk->end, twk->plane_mask, twk->disp_mask );
    if(twk->task_ct != NULL){
      (*twk->task_ct)--;
    }
    GFL_TCBL_Delete(tcb);
    return;
  }

  if( twk->frame-- > 0 ){
    SetBrightness( FX_Whole(twk->now), twk->plane_mask, twk->disp_mask );
    twk->now += twk->diff;
    return;
  }
  SetBrightness( twk->end, twk->plane_mask, twk->disp_mask );
  if(twk->task_ct != NULL){
    (*twk->task_ct)--;
  }
  GFL_TCBL_Delete(tcb);
}


//===================================================================================
///輝度変化エフェクトを用いたフラッシュリクエスト
//===================================================================================
typedef struct _TASKWK_FLASH_REQ{
  DEMO3D_CMD_WORK* cmd;
  DEMO3D_ENGINE_WORK* core;

  s8  seq;
  s8  start,max,end;
  int frame_s,frame_e;
  int task_ct;
  BRIGHT_DISPMASK   disp_mask;
  BRIGHT_PLANEMASK  plane_mask;
}TASKWK_FLASH_REQ;

static void taskAdd_FlashReq( DEMO3D_CMD_WORK* wk, u8 disp_mask, s8 start, s8 max, s8 end, int frame_s, int frame_e );
static void tcb_FlashReq( GFL_TCBL *tcb , void* work);

//-----------------------------------------------------------------------------
/**
 *	@brief  輝度変化エフェクトを用いたフラッシュリクエスト
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

static void taskAdd_FlashReq( DEMO3D_CMD_WORK* wk, u8 disp_mask, s8 start, s8 max, s8 end, int frame_s, int frame_e )
{
  GFL_TCBL* tcb;
  TASKWK_FLASH_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_FlashReq, sizeof(TASKWK_FLASH_REQ), 1 );

  twk = GFL_TCBL_GetWork(tcb);
  MI_CpuClear8( twk, sizeof( TASKWK_FLASH_REQ ));

  twk->cmd = wk;
  twk->start = start;
  twk->max = max;
  twk->end = end;
  twk->disp_mask = disp_mask;
  twk->plane_mask = PLANEMASK_ALL;
  twk->frame_s = frame_s;
  twk->frame_e = frame_e;
   
  taskAdd_BrightnessReq( wk, twk->disp_mask, twk->plane_mask, start, max, frame_s, &twk->task_ct );
}

static void tcb_FlashReq( GFL_TCBL *tcb , void* tcb_wk)
{
  TASKWK_FLASH_REQ* twk = (TASKWK_FLASH_REQ*)tcb_wk;

  //システム終了で自殺
  if( twk->cmd->cmdsys_end_f) {
    SetBrightness( twk->end, twk->plane_mask, twk->disp_mask );
    GFL_TCBL_Delete(tcb);
    return;
  }

  switch(twk->seq){
  case 0:
    if(twk->task_ct == 0){
      taskAdd_BrightnessReq( twk->cmd, twk->disp_mask, twk->plane_mask,twk->max, twk->end, twk->frame_e, &twk->task_ct );
      twk->seq++;
    }
    break;
  case 1:
    if(twk->task_ct){
      return;
    }
    break;
  }
  SetBrightness( twk->end, twk->plane_mask, twk->disp_mask );
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
  GXRgb col = GX_RGB(param[1],param[2],param[3]);

  DEMO3D_GRAPHIC_3DLightColorSet( core->graphic, param[0], col );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ライトベクトル変更 
 *
 *	@param	param[0]  light_no(0～3)
 *	@param  param[1]  light_vec_x(fx16 -7.9～7.9)
 *	@param  param[2]  light_vec_y(fx16 -7.9～7.9)
 *	@param  param[3]  light_vec_z(fx16 -7.9～7.9)
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_LightVectorSet(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  VecFx16 vec;
  
  vec.x = (fx16)param[1];
  vec.y = (fx16)param[2];
  vec.z = (fx16)param[3];

  DEMO3D_GRAPHIC_3DLightVectorSet( core->graphic, param[0], &vec );
}

//===================================================================================
///会話ウィンドウ表示
//===================================================================================
typedef struct _TASKWK_TALKWIN_REQ{
  DEMO3D_CMD_WORK* cmd;

  int seq;
  int ct;
  int sync;

  int msg_arc;
  int msg_id;
  GFL_ARCUTIL_TRANSINFO tInfo;

  GFL_BMPWIN *win;
  PRINT_STREAM* printStream;
  
}TASKWK_TALKWIN_REQ;

#define TALKWIN_BG_FRM    ( DEMO3D_CMD_FREE_BG_M1 )
#define TALKWIN_FRAME_CGX (1)
#define TALKWIN_FRAME_PAL (0)
#define TALKWIN_FONT_PAL  (1)

static void taskAdd_TalkWinReq( DEMO3D_CMD_WORK* wk, int msg_arc, int msg_id, int sync, u8 px, u8 py, u8 sx, u8 sy );
static void tcb_TalkWinReq( GFL_TCBL *tcb , void* work);
static void taskwk_TalkWinReqInit( DEMO3D_CMD_WORK* wk, TASKWK_TALKWIN_REQ* twk,
    int msg_arc, int msg_id, int sync, u8 px, u8 py, u8 sx, u8 sy );
static void taskwk_TalkWinReqFree( TASKWK_TALKWIN_REQ* wk );

//-----------------------------------------------------------------------------
/**
 *	@brief   会話ウィンドウ表示
 *
 *	@param	param[0]  メッセージアーカイブID 
 *	@param	param[1]  メッセージID
 *	@param	param[2]  メッセージセット数( 実際に表示されるメッセージID = param[2]*シーンID+param[1])
 *	@param	param[3]  ウィンドウを表示するフレーム数(1～)
 *	@param	param[4]  ウィンドウ左上X座標(1～30) 
 *	@param	param[5]  ウィンドウ左上Y座標(1～21)
 *	@param	param[6]  ウィンドウサイズX(2～28)
 *	@param	param[7]  ウィンドウサイズY(2～22) 
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_TalkWinReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  int msg_id = param[1]+param[2]*core->env.scene_id;

  taskAdd_TalkWinReq( wk, param[0], msg_id, sub_ConvFrame(wk,param[3]) , param[4], param[5], param[6], param[7] );
}

static void taskAdd_TalkWinReq( DEMO3D_CMD_WORK* wk, int msg_arc, int msg_id, int sync, u8 px, u8 py, u8 sx, u8 sy )
{
  GFL_TCBL* tcb;
  TASKWK_TALKWIN_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_TalkWinReq, sizeof(TASKWK_TALKWIN_REQ), 1 );

  twk = GFL_TCBL_GetWork(tcb);

  //BMPウィンドウ&ストリーム生成
  taskwk_TalkWinReqInit( wk, twk, msg_arc, msg_id, sync, px, py, sx, sy );
}

static void tcb_TalkWinReq( GFL_TCBL *tcb , void* tcb_wk )
{
  TASKWK_TALKWIN_REQ* twk = (TASKWK_TALKWIN_REQ*)tcb_wk;
  DEMO3D_CMD_WORK* cmd = twk->cmd;

  //システム終了で自殺
  if( twk->cmd->cmdsys_end_f ){
    taskwk_TalkWinReqFree( twk );
    GFL_TCBL_Delete(tcb);
    return;
  }
  switch(twk->seq){
  case 0:
    {
      GFL_MSGDATA *msg_man;
      STRBUF *str;

      //ストリーム開始
      msg_man = GFL_MSG_Create(
		  GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, twk->msg_arc, cmd->tmpHeapID );

      str = GFL_MSG_CreateString( msg_man, twk->msg_id );

      GFL_FONTSYS_SetColor( 1, 2, 15 );
      twk->printStream = PRINTSYS_PrintStream( twk->win, 0, 0,
        str, cmd->fontHandle, cmd->msg_spd, cmd->tcbsys, 0, cmd->heapID, 15 );

      GFL_BMPWIN_MakeTransWindow( twk->win );
      BmpWinFrame_WriteAreaMan( twk->win, WINDOW_TRANS_ON_V, twk->tInfo, TALKWIN_FRAME_PAL );

      GFL_STR_DeleteBuffer( str );
      GFL_MSG_Delete( msg_man );

      GFL_BG_SetPriority( TALKWIN_BG_FRM, 0 );
      GFL_BG_SetPriority( DEMO3D_3D_BG_M, 1 );
      GFL_BG_SetVisible( TALKWIN_BG_FRM, VISIBLE_ON );
    }
    twk->seq++;
    break;
  case 1:
    if(twk->ct++ < twk->sync) {
      break;
    }
    taskwk_TalkWinReqFree( twk );
    GFL_TCBL_Delete(tcb);
    break;
  }
}

static void taskwk_TalkWinReqInit( DEMO3D_CMD_WORK* wk, TASKWK_TALKWIN_REQ* twk,
    int msg_arc, int msg_id, int sync, u8 px, u8 py, u8 sx, u8 sy )
{
  
  MI_CpuClear8( twk, sizeof( TASKWK_TALKWIN_REQ ));
  
  twk->cmd = wk;
  twk->sync = sync;
  twk->msg_arc = msg_arc;
  twk->msg_id = msg_id;

  //Bmpウィンドウグラフィックセット
  twk->tInfo = BmpWinFrame_GraphicSetAreaMan(
	  TALKWIN_BG_FRM, TALKWIN_FRAME_PAL, MENU_TYPE_TALK, wk->tmpHeapID );

  //フォントパレットセット
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT,
			NARC_font_default_nclr, PALTYPE_MAIN_BG, TALKWIN_FONT_PAL*0x20, 0x20, wk->tmpHeapID );
  
  twk->win = GFL_BMPWIN_Create( TALKWIN_BG_FRM, px, py, sx, sy, TALKWIN_FONT_PAL, GFL_BMP_CHRAREA_GET_F );
  GFL_BMP_Clear( GFL_BMPWIN_GetBmp( twk->win ), 15 );
}

static void taskwk_TalkWinReqFree( TASKWK_TALKWIN_REQ* twk )
{
  if( twk->printStream != NULL ){
    PRINTSYS_PrintStreamDelete( twk->printStream );
  }
  GFL_FONTSYS_SetDefaultColor();
  
  BmpWinFrame_Clear( twk->win, WINDOW_TRANS_ON_V );
  GFL_BMPWIN_Delete( twk->win );
  BmpWinFrame_GraphicFreeAreaMan( TALKWIN_BG_FRM, twk->tInfo );
  
  MI_CpuClear8( twk, sizeof( TASKWK_TALKWIN_REQ ));
  
  GFL_BG_SetPriority( TALKWIN_BG_FRM, 1 );
  GFL_BG_SetPriority( DEMO3D_3D_BG_M, DEMO3D_3D_BG_PRI );
  GFL_BG_SetVisible( TALKWIN_BG_FRM, VISIBLE_OFF );
}

//===================================================================================
///BG一枚絵表示
//===================================================================================
typedef struct _TASKWK_SCR_DRAW_REQ{
  DEMO3D_CMD_WORK* cmd;

  int seq;
  int ct;
  int sync;

  int pltt_id;
  int char_id;
  int scr_id;

  int s_frm;
  int e_frm;

  fx32  alpha_n;
  fx32  alpha_d;
}TASKWK_SCR_DRAW_REQ;

#define SCR_DRAW_ALPHA_1ST  (GX_BLEND_PLANEMASK_BG1)
#define SCR_DRAW_ALPHA_2ND  (GX_BLEND_PLANEMASK_BG0)
#define ALPHA_EV2_NORMAL  (3)
#define ALPHA_EV1_NORMAL  (16-ALPHA_EV2_NORMAL)

static void taskAdd_ScrDrawReq( DEMO3D_CMD_WORK* wk,
    int pltt_id, int char_id, int scr_id, int sync, int s_frm, int e_frm );
static void tcb_ScrDrawReq( GFL_TCBL *tcb , void* work);
static void taskwk_ScrDrawReqInit( DEMO3D_CMD_WORK* wk, TASKWK_SCR_DRAW_REQ* twk,
    int pltt_id, int char_id, int scr_id, int sync, int s_frm, int e_frm );
static void taskwk_ScrDrawReqFree( TASKWK_SCR_DRAW_REQ* wk );

//-----------------------------------------------------------------------------
/**
 *	@brief   BG一枚絵表示
 *
 *  @param  param[0]  適用する画面 < "ver_all"(0xFF),"ver_black"(VERSION_BLACK),"ver_white"(VERSION_WHITE),"ver_gray"(VERSION_WHITE+1) >
 *	@param	param[1]  パレットアーカイブID 
 *	@param	param[2]  キャラクタアーカイブID
 *	@param	param[3]  スクリーンアーカイブID
 *	@param	param[4]  ウィンドウを表示するフレーム数(1～)
 *	@param	param[5]  αフェードインフレーム数 
 *	@param	param[6]  αフェードアウトフレーム数
 */
//-----------------------------------------------------------------------------
static void DEMOCMD_ScrDrawReq(DEMO3D_CMD_WORK* wk,DEMO3D_ENGINE_WORK* core, int* param)
{
  if( param[0] != 0xFF && param[0] != CasetteVersion ){
    return; //無視
  }
  taskAdd_ScrDrawReq( wk, param[1], param[2], param[3],
    sub_ConvFrame(wk,param[4]), sub_ConvFrame(wk,param[5]), sub_ConvFrame(wk,param[6]) );
}

static void taskAdd_ScrDrawReq( DEMO3D_CMD_WORK* wk,
    int pltt_id, int char_id, int scr_id, int sync, int s_frm, int e_frm )
{
  GFL_TCBL* tcb;
  TASKWK_SCR_DRAW_REQ* twk;

  tcb = GFL_TCBL_Create( wk->tcbsys, tcb_ScrDrawReq, sizeof(TASKWK_SCR_DRAW_REQ), 1 );

  twk = GFL_TCBL_GetWork(tcb);

  //BMPウィンドウ&ストリーム生成
  taskwk_ScrDrawReqInit( wk, twk, pltt_id, char_id, scr_id, sync, s_frm, e_frm );
}

static void tcb_ScrDrawReq( GFL_TCBL *tcb , void* tcb_wk )
{
  TASKWK_SCR_DRAW_REQ* twk = (TASKWK_SCR_DRAW_REQ*)tcb_wk;
  DEMO3D_CMD_WORK* cmd = twk->cmd;
 
  //システム終了で自殺
  if( twk->cmd->cmdsys_end_f) {
    taskwk_ScrDrawReqFree( twk );
    GFL_TCBL_Delete(tcb);
    return;
  }
  switch( twk->seq ){
  case 0:
    GFL_ARCHDL_UTIL_TransVramPalette( cmd->h_2dgra, twk->pltt_id, PALTYPE_MAIN_BG, 0, 0, cmd->tmpHeapID );
 	  GFL_ARCHDL_UTIL_TransVramScreenCharOfs(	cmd->h_2dgra, twk->scr_id,
						DEMO3D_CMD_FREE_BG_M1, 0, 0, 0, TRUE, cmd->tmpHeapID );	
    twk->seq++;
    break;
  case 1:
    twk->ct = 0;
    twk->alpha_n = 0;
    twk->alpha_d = FX_Div( FX32_CONST(16),FX32_CONST(twk->s_frm));

    GFL_BG_SetPriority( TALKWIN_BG_FRM, 0 );
    GFL_BG_SetPriority( DEMO3D_3D_BG_M, 1 );
    G2_SetBlendAlpha( SCR_DRAW_ALPHA_1ST,SCR_DRAW_ALPHA_2ND, 0, 16 );
    GFL_BG_SetVisible( TALKWIN_BG_FRM, VISIBLE_ON );
    twk->seq++;
    break;
  case 2:
    {
      int ev;
      
      if( twk->ct++ < twk->s_frm ){
        twk->alpha_n += twk->alpha_d;
        ev = FX_Whole( twk->alpha_n);
        G2_ChangeBlendAlpha( ev, 16-ev);
        break;
      }
      G2_ChangeBlendAlpha( 16, 0 );
      twk->ct = 0;
      twk->seq++;
    }
    break;
  case 3:
    if( twk->ct++ < twk->sync ){
      break;
    }
    twk->ct = 0;
    twk->alpha_n = 0;
    twk->alpha_d = FX_Div( FX32_CONST(16),FX32_CONST(twk->e_frm));
    twk->seq++;
    break;
  case 4:
    {
      int ev;
      
      if( twk->ct++ < twk->e_frm ){
        twk->alpha_n += twk->alpha_d;
        ev = FX_Whole( twk->alpha_n);
        G2_ChangeBlendAlpha( 16-ev, ev);
        break;
      }
      G2_ChangeBlendAlpha( 0, 16 );
      twk->ct = 0;
      twk->seq++;
    }
    break;
  default:
    taskwk_ScrDrawReqFree( twk );
    GFL_TCBL_Delete(tcb);
    break;
  }
}

static void taskwk_ScrDrawReqInit( DEMO3D_CMD_WORK* wk, TASKWK_SCR_DRAW_REQ* twk,
    int pltt_id, int char_id, int scr_id, int sync, int s_frm, int e_frm )
{
  GFL_MSGDATA *msg_man;
  STRBUF *str;
  
  MI_CpuClear8( twk, sizeof( TASKWK_SCR_DRAW_REQ ));
  
  twk->cmd = wk;
  twk->sync = sync;

  twk->pltt_id = pltt_id;
  twk->char_id = char_id;
  twk->scr_id = scr_id;

  twk->s_frm = s_frm;
  twk->e_frm = e_frm;
  GFL_ARCHDL_UTIL_TransVramBgCharacter(	wk->h_2dgra, twk->char_id,
						DEMO3D_CMD_FREE_BG_M1, 0, 0, TRUE, wk->tmpHeapID );
}

static void taskwk_ScrDrawReqFree( TASKWK_SCR_DRAW_REQ* twk )
{
  MI_CpuClear8( twk, sizeof( TASKWK_SCR_DRAW_REQ ));
  
  GFL_BG_SetVisible( TALKWIN_BG_FRM, VISIBLE_OFF );
  GFL_BG_SetPriority( DEMO3D_3D_BG_M, DEMO3D_3D_BG_PRI );
  GFL_BG_SetPriority( TALKWIN_BG_FRM, 1 );

  G2_BlendNone();
  GFL_BG_ClearFrame( DEMO3D_CMD_FREE_BG_M1 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  モーションブラー 開始
 *
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


