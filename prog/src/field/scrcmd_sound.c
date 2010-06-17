//======================================================================
/**
 * @file  scr_sound.c
 * @brief  スクリプトコマンド：サウンド関連
 * @author  Satoshi Nohara
 * @date  06.06.26
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"
#include "system/vm_cmd.h"

#include "gamesystem/gamesystem.h"
#include "gamesystem/game_event.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field/zonedata.h"
#include "field_sound.h"

#include "sound/pm_voice.h"
#include "sound/pm_wb_voice.h"

#include "script.h"
#include "script_local.h"
#include "script_def.h"
#include "scrcmd.h"
#include "scrcmd_work.h"

#include "scrcmd_sound.h"

#include "gamesystem/iss_sys.h"
#include "gamesystem/iss_switch_sys.h"

//======================================================================
//  define
//======================================================================

//======================================================================
//  struct
//======================================================================
typedef struct VOICE_PLAY_WORK_tag
{
  u16 MonsNo;
  u16 FormNo;
  BOOL Mine;
  SCRCMD_WORK* scr_work;
}VOICE_PLAY_WORK;

//======================================================================
//  proto
//======================================================================
// SEの停止を監視
static void SoundSeFlag_CheckSeStop( SCRIPT_WORK * sc );
static void SoundSeFlag_SetPlay( SCRIPT_WORK * sc, u32 se_no );
static BOOL SoundSeFlag_IsSePlay( SCRIPT_WORK * sc );

static GMEVENT_RESULT PlayVoiceEvent( GMEVENT* event, int* seq, void* wk );

//======================================================================
//  コマンド BGM
//======================================================================

//--------------------------------------------------------------
/**
 * BGM 変更
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 現在再生中のBGMを破棄し、指定BGMを再生している。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  u16       soundIdx = VMGetU16( core );

  {
    GMEVENT* event;
    event = EVENT_FSND_PlayEventBGM( gsys, soundIdx );
    SCRIPT_CallEvent( sc, event );
  }

  // フィールドサウンド。
  // 環境SE停止。
  {
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND* fsound = GAMEDATA_GetFieldSound( gdata );
    FSND_PauseEnvSE( fsound, FSND_ENVSE_PAUSE_BGM_CHANGE );
  }
  
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGM変更 ( FOフレーム数指定ver. )
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 現在再生中のBGMを破棄し、指定BGMを再生している。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlayEx( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work      = wk;
  GAMESYS_WORK* gsys      = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*  script    = SCRCMD_WORK_GetScriptWork( work );
  u16           sound_idx = VMGetU16( core ); // コマンド第一引数: BGM
  u16           frame     = VMGetU16( core ); // コマンド第二引数: FOフレーム数

  {
    GMEVENT* event;
    event = EVENT_FSND_PlayEventBGMEx( gsys, sound_idx, frame );
    SCRIPT_CallEvent( script, event );
  }

  // フィールドサウンド。
  // 環境SE停止。
  {
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND* fsound = GAMEDATA_GetFieldSound( gdata );
    FSND_PauseEnvSE( fsound, FSND_ENVSE_PAUSE_BGM_CHANGE );
  }
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGM 変更 ( 無音曲再生 )
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 現在再生中のBGMを破棄し、無音BGMを再生している。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlaySilent( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work   = wk;
  GAMESYS_WORK* gsys   = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*  script = SCRCMD_WORK_GetScriptWork( work );
  u16           frame  = VMGetU16( core ); // コマンド第一引数: FOフレーム数

  {
    GMEVENT* event;
    event = EVENT_FSND_PlayEventBGMSilent( gsys, frame );
    SCRIPT_CallEvent( script, event );
  }

  // フィールドサウンド。
  // 環境SE停止。
  {
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND* fsound = GAMEDATA_GetFieldSound( gdata );
    FSND_PauseEnvSE( fsound, FSND_ENVSE_PAUSE_BGM_CHANGE );
  }
  
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGM終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlayCheck( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 music  = VMGetU16( core );
  u16 *ret_wk  = SCRCMD_GetVMWork( core, work );
  *ret_wk = PMSND_CheckPlayBGM();
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BGM終了待ち ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitBgm( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );

  if( PMSND_CheckPlayBGM() == FALSE )
  {
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * BGM終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmWait( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitBgm );
  return VMCMD_RESULT_SUSPEND; 
}

//--------------------------------------------------------------
/**
 * BGM のボリュームを下げる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmVolumeDown( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work = wk;
  GAMESYS_WORK*  gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gameData   = GAMESYSTEM_GetGameData( gameSystem );
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( gameData );
  u16 volume, frame;

  volume = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: ボリューム
  frame  = SCRCMD_GetVMWorkValue( core, work ); // 第二引数: フレーム数

  FIELD_SOUND_ChangePlayerVolume( fieldSound, volume, frame );  
  SCREND_CHK_SetBitOn( SCREND_CHK_BGM_VOLUME_DOWN );

  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BGM のボリュームを復帰させる
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmVolumeRecover( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work = wk;
  GAMESYS_WORK*  gameSystem = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gameData   = GAMESYSTEM_GetGameData( gameSystem );
  FIELDMAP_WORK* fieldmap   = GAMESYSTEM_GetFieldMapWork( gameSystem );
  FIELD_SOUND*   fieldSound = GAMEDATA_GetFieldSound( gameData );
  u16 frame;

  frame  = SCRCMD_GetVMWorkValue( core, work ); // 第一引数: フレーム数

  FIELD_SOUND_ChangePlayerVolume( fieldSound, 127, frame );  
  SCREND_CHK_SetBitOff( SCREND_CHK_BGM_VOLUME_DOWN );

  return VMCMD_RESULT_CONTINUE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGM ボリューム操作処理の対チェック
 *
 *	@param	end_check   ワーク
 *	@param	seq         シーケンス
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
BOOL SCREND_CheckEndBGMVolumeDown( SCREND_CHECK *end_check, int *seq )
{
  GAMEDATA*    gameData   = GAMESYSTEM_GetGameData( end_check->gsys );
  FIELD_SOUND* fieldSound = GAMEDATA_GetFieldSound( gameData );

  switch( *seq ){
  case 0:
    FIELD_SOUND_ChangePlayerVolume( fieldSound, 127, 0 );  
    (*seq) ++;
    break;
  case 1:
    return TRUE;
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * 現在のマップのBGMを再生
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 再生中のBGMを破棄し, フィールドのBGMを再生する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmNowMapPlay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work = wk;
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gdata    = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_SOUND*   fsound   = GAMEDATA_GetFieldSound( gdata );
  SCRIPT_WORK*   sc       = SCRCMD_WORK_GetScriptWork( work );

  {
    GMEVENT* event;
    u8 seasonID;
    u16 zoneID;
    u32 soundIdx;

    seasonID = GAMEDATA_GetSeasonID( gdata );
    zoneID   = FIELDMAP_GetZoneID( fieldmap );
    soundIdx = FSND_GetFieldBGM( gdata, zoneID, seasonID );

    event = EVENT_FSND_ChangeBGM( gsys, soundIdx, FSND_FADE_LONG, FSND_FADE_NORMAL );

    SCRIPT_CallEvent( sc, event );
    FSND_RePlayEnvSE( fsound, FSND_ENVSE_PAUSE_BGM_CHANGE ); // 環境音復帰
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 現在のマップのBGMを再生 ( FOフレーム数指定ver. )
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 再生中のBGMを破棄し, フィールドのBGMを再生する
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmNowMapPlayEx( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work     = wk;
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gdata    = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  FIELD_SOUND*   fsnd     = GAMEDATA_GetFieldSound( gdata );
  SCRIPT_WORK*   script   = SCRCMD_WORK_GetScriptWork( work );
  u16            frame    = VMGetU16( core ); // コマンド第一引数: FOフレーム数

  {
    GMEVENT* event;
    u8 seasonID;
    u16 zoneID;
    u32 soundIdx;

    seasonID = GAMEDATA_GetSeasonID( gdata );
    zoneID   = FIELDMAP_GetZoneID( fieldmap );
    soundIdx = FSND_GetFieldBGM( gdata, zoneID, seasonID );
    event    = EVENT_FSND_ChangeBGM( gsys, soundIdx, frame, FSND_FADE_NORMAL );
    SCRIPT_CallEvent( script, event );
    FSND_RePlayEnvSE( fsnd, FSND_ENVSE_PAUSE_BGM_CHANGE ); // 環境音復帰
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 現在のBGMを退避し, イベントBGMを再生する
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 再生中のBGMを破棄し, フィールドのBGMを再生する
 *
 * ※通常のイベントBGMはフィールドBGMの退避を行わないで再生する。
 * 　イベントBGM ⇒ バトル ⇒ フィールドBGM の流れが必要な場面で使用する。
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdPlayTempEventBGM( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*         gdata = GAMESYSTEM_GetGameData( gsys );
  FIELDMAP_WORK* fieldmap = GAMESYSTEM_GetFieldMapWork( gsys );
  SCRIPT_WORK*         sc = SCRCMD_WORK_GetScriptWork( work );
  u16            soundIdx = SCRCMD_GetVMWorkValue( core, work );
  GMEVENT* event;

  //event = EVENT_FSND_PushPlayNextBGM( gsys, soundIdx, FSND_FADE_FAST, FSND_FADE_NONE );
  event = EVENT_FSND_PushPlayEventBGM( gsys, soundIdx ); // 100413 即時再生に変更
  SCRIPT_CallEvent( sc, event );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 現在のBGMをPushする
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 *
 * 使用箇所はできるだけ限定したいので、コマンドの利用箇所は許可制
 * (10.04.7現在 3Dデモ呼び出しのみ)
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdBgmPush( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*       sc   = SCRCMD_WORK_GetScriptWork( work );

  u16 fade_frame = SCRCMD_GetVMWorkValue( core, work );
  
  SCRIPT_CallEvent( sc, EVENT_FSND_PushBGM( gsys, fade_frame ));

  SCREND_CHK_SetBitOn( SCREND_CHK_BGM_PUSH_POP );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 現在のBGMをPopする
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 *
 * 使用箇所はできるだけ限定したいので、コマンドの利用箇所は許可制
 * (10.04.7現在 3Dデモ呼び出しのみ)
 */
//-------------------------------------------------------------- 
VMCMD_RESULT EvCmdBgmPop( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*       work = wk;
  GAMESYS_WORK*      gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*       sc   = SCRCMD_WORK_GetScriptWork( work );

  u16 fadeout_frame = SCRCMD_GetVMWorkValue( core, work );
  u16 fadein_frame = SCRCMD_GetVMWorkValue( core, work );
  
  SCRIPT_CallEvent( sc, EVENT_FSND_PopBGM( gsys, fadeout_frame, fadein_frame ));
  
  SCREND_CHK_SetBitOff( SCREND_CHK_BGM_PUSH_POP );
  return VMCMD_RESULT_SUSPEND;
}

//----------------------------------------------------------------------------
/**
 *	@brief  BGM Push/Pop処理の対チェック
 *
 *	@param	end_check   ワーク
 *	@param	seq         シーケンス
 *
 *	@retval TRUE    完了
 *	@retval FALSE   途中
 */
//-----------------------------------------------------------------------------
BOOL SCREND_CheckEndBGMPushPop( SCREND_CHECK *end_check, int *seq )
{
  SCRIPT_WORK *sc = end_check->ScrWk;
  SCRIPT_FLDPARAM *fparam = SCRIPT_GetFieldParam( sc );

  switch( *seq ){
  case 0:
    SCRIPT_CallEvent( sc, EVENT_FSND_PopBGM( end_check->gsys, 0, 0 ));
    (*seq) ++;
    break;
  case 1:
    return TRUE;
  }
  return FALSE;
}




//======================================================================
//  コマンド　EnvSE
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  環境SEの停止状況クリア
 */
//-----------------------------------------------------------------------------
VMCMD_RESULT EvCmdEnvSeBGMPlayClear( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*   work     = wk;
  GAMESYS_WORK*  gsys     = SCRCMD_WORK_GetGameSysWork( work );
  GAMEDATA*      gdata    = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND*   fsnd     = GAMEDATA_GetFieldSound( gdata );

  FSND_RePlayEnvSE( fsnd, FSND_ENVSE_PAUSE_BGM_CHANGE ); // 環境音復帰

  return VMCMD_RESULT_CONTINUE;
}




//======================================================================
//  コマンド　SE
//======================================================================
//----------------------------------------------------------------------------
/**
 *	@brief  監視しているSEがとまっていないかチェック　とまっていたら、フラグを落とす
 */
//-----------------------------------------------------------------------------
static void SoundSeFlag_CheckSeStop( SCRIPT_WORK * sc )
{
  int i;
  u8* p_sound_se_flag = SCRIPT_GetSoundSeFlag( sc );

  // SE停止チェック
  for( i=0; i<SEPLAYER_MAX; i++ )
  {
    if( (*p_sound_se_flag) & (1<<i) )
    {
      // 停止チェック
      if( PMSND_CheckPlaySE_byPlayerID(i) == FALSE )
      {
        // 停止しているのでフラグを落とす
        (*p_sound_se_flag) &= ~(1<<i);
      }
    }
  }
}

//----------------------------------------------------------------------------
/**
 *	@brief  SEの再生設定
 *	
 *	@param	sc
 *	@param	se_no 
 */
//-----------------------------------------------------------------------------
static void SoundSeFlag_SetPlay( SCRIPT_WORK * sc, u32 se_no )
{
  u8* p_sound_se_flag = SCRIPT_GetSoundSeFlag( sc );
  SEPLAYER_ID player_id = PMSND_GetSE_DefaultPlayerID( se_no );

  (*p_sound_se_flag) |= (1<<player_id);
}

//----------------------------------------------------------------------------
/**
 *	@brief  監視SEの再生チェック
 *
 *	@param	sc    ワーク
 *
 *	@retval TRUE  再生中
 *	@retval FLASE 再生なし
 */
//-----------------------------------------------------------------------------
static BOOL SoundSeFlag_IsSePlay( SCRIPT_WORK * sc )
{
  int i;
  u8* p_sound_se_flag = SCRIPT_GetSoundSeFlag( sc );

  // SE停止チェック
  for( i=0; i<SEPLAYER_MAX; i++ )
  {
    if( (*p_sound_se_flag) & (1<<i) )
    {
      // 停止チェック
      if( PMSND_CheckPlaySE_byPlayerID(i) == TRUE )
      {
        return TRUE;
      }
    }
  }

  // フラグのクリア
  (*p_sound_se_flag) = 0;

  return FALSE;
}

//----------------------------------------------------------------------------
/**
 *	@brief  SEの停止
 *	
 *	@param	sc
 *	@param	se_no 
 */
//-----------------------------------------------------------------------------
static void SoundSeFlag_StopSe( SCRIPT_WORK * sc )
{
  int i;
  u8* p_sound_se_flag = SCRIPT_GetSoundSeFlag( sc );

  // SE停止チェック
  for( i=0; i<SEPLAYER_MAX; i++ )
  {
    if( (*p_sound_se_flag) & (1<<i) )
    {
      // 停止チェック
      PMSND_StopSE_byPlayerID( i );
    }
  }

  // フラグのクリア
  (*p_sound_se_flag) = 0;
}

//--------------------------------------------------------------
/**
 * ＳＥを鳴らす
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSePlay( VMHANDLE *core, void *wk )
{
  u16 se_no = SCRCMD_GetVMWorkValue(core,wk);
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *scriptwk = SCRCMD_WORK_GetScriptWork(work);

#if 0
  Snd_SePlay( SCRCMD_VMGetWorkValue(core,wk) );
#else //wb

  // 再生したSEがとまっていたら、監視フラグを落とす
  SoundSeFlag_CheckSeStop( scriptwk );
  
  // 再生
  PMSND_PlaySE( se_no );

  // 監視開始
  SoundSeFlag_SetPlay( scriptwk, se_no );  
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ＳＥを止める
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSeStop( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *scriptwk = SCRCMD_WORK_GetScriptWork(work);
#if 0
  Snd_SeStopBySeqNo( VMGetWorkValue(core), 0 );
#else //wb
  SoundSeFlag_StopSe( scriptwk );
#endif
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * SE終了待ち ウェイト部分
 * @param 
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitSe(VMHANDLE *core, void *wk)
{
  SCRCMD_WORK *work = wk;
  SCRIPT_WORK *scriptwk = SCRCMD_WORK_GetScriptWork(work);
#if 0
  //if( Snd_SePlayCheckAll() == 0 ){
  if( Snd_SePlayCheck(core->reg[0]) == 0 ){
    return TRUE;
  }
#else //wb
  if( SoundSeFlag_IsSePlay(scriptwk) == FALSE ){
    return TRUE;
  }
#endif
  return FALSE;
}

//--------------------------------------------------------------
/**
 * SE終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 * @li    雨などのループ音で、無限ループになってしまうので、SEナンバー指定して、それをチェック！
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSeWait(VMHANDLE *core, void *wk)
{
#if 0
  //仮想マシンの汎用レジスタにBGMナンバーを格納
  core->reg[0] = VMGetWorkValue(core);
  VMCMD_SetWait( core, EvWaitSe );
#else //wb
  VMCMD_SetWait( core, EvWaitSe );
#endif
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  ME
//======================================================================
//--------------------------------------------------------------
/**
 * ME再生
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMePlay(VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work = wk;
  GAMESYS_WORK* gsys = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  u16      sound_idx = VMGetU16( core );

  {
    GMEVENT* event;
    event = EVENT_FSND_PushPlayJingleBGM( gsys, sound_idx );
    SCRIPT_CallEvent( sc, event );
  }

  /* BTS:5951 対処時に、BGMPushしているので、ここはいらないことが発覚
     * tomoya
  // フィールドサウンド。
  // 環境SE停止。
  {
    GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
    FIELD_SOUND* fsound = GAMEDATA_GetFieldSound( gdata );
    FSND_PauseEnvSE( fsound );
  }
  */
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * ME終了待ち ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitMe( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK*  work  = wk;
  GAMESYS_WORK* gsys  = SCRCMD_WORK_GetGameSysWork( work );
  SCRIPT_WORK*  sc    = SCRCMD_WORK_GetScriptWork( work );
  GAMEDATA*     gdata = GAMESYSTEM_GetGameData( gsys );
  FIELD_SOUND*  fsnd  = GAMEDATA_GetFieldSound( gdata );

  if( (PMSND_CheckPlayBGM() == FALSE) && 
      (FIELD_SOUND_HaveRequest(fsnd) == FALSE) )
  {
    GMEVENT* event;
    event = EVENT_FSND_PopBGM( gsys, FSND_FADE_NONE, FSND_FADE_FAST );
    SCRIPT_CallEvent( sc, event );

    /* BTS:5951 対処時に、BGMPushしているので、ここはいらないことが発覚
     * tomoya
    // フィールドサウンド。
    // 環境SE再始動。
    {
      GAMEDATA* gdata = GAMESYSTEM_GetGameData( gsys );
      FIELD_SOUND* fsound = GAMEDATA_GetFieldSound( gdata );
      FSND_RePlayEnvSE( fsound );
    }
    */

    return TRUE;
  }
  
  return FALSE;
}

//--------------------------------------------------------------
/**
 * ME終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdMeWait(VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitMe );
  return VMCMD_RESULT_SUSPEND;
}

//======================================================================
//  鳴き声
//======================================================================
//--------------------------------------------------------------
/**
 * 鳴き声を鳴らす
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdVoicePlay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK* work = wk;
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys   = SCRCMD_WORK_GetGameSysWork( work );
  u16 monsno = SCRCMD_GetVMWorkValue(core, wk);
  u16 formno = SCRCMD_GetVMWorkValue(core, wk);
  {
    VOICE_PLAY_WORK* vpw;
    GMEVENT* event;
    event = GMEVENT_Create( gsys, NULL, PlayVoiceEvent, sizeof(VOICE_PLAY_WORK) );
    vpw = GMEVENT_GetEventWork( event );
    vpw->MonsNo = monsno;
    vpw->FormNo = formno;
    vpw->Mine = FALSE;
    vpw->scr_work = work;
    SCRIPT_CallEvent( sc, event );
  }

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 鳴き声を鳴らす ( ペラップ録音データ再生ver. )
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 *
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdVoicePlay_forMine( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK* work = wk;
  SCRIPT_WORK*    sc = SCRCMD_WORK_GetScriptWork( work );
  GAMESYS_WORK* gsys   = SCRCMD_WORK_GetGameSysWork( work );
  u16 monsno = SCRCMD_GetVMWorkValue(core, wk);
  u16 formno = SCRCMD_GetVMWorkValue(core, wk);
  {
    VOICE_PLAY_WORK* vpw;
    GMEVENT* event;
    event = GMEVENT_Create( gsys, NULL, PlayVoiceEvent, sizeof(VOICE_PLAY_WORK) );
    vpw = GMEVENT_GetEventWork( event );
    vpw->MonsNo = monsno;
    vpw->FormNo = formno;
    vpw->Mine = TRUE;
    vpw->scr_work = work;
    SCRIPT_CallEvent( sc, event );
  }

  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
///@return TRUE = 終了
//--------------------------------------------------------------
static BOOL EvWaitVoicePlay(VMHANDLE *core, void *wk)
{
  SCRCMD_WORK*  work = wk;
  u32 player = SCRCMD_WORK_GetPMVoiceIndex( work );
  if (PMVOICE_CheckPlay( player ) == FALSE)
  {
    return TRUE;
  }
  return FALSE;
}
//--------------------------------------------------------------
/**
 * 鳴き声終了待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdVoiceWait( VMHANDLE *core, void *wk )
{
  VMCMD_SetWait( core, EvWaitVoicePlay );
  return VMCMD_RESULT_SUSPEND;
}


//======================================================================
// ISS
//======================================================================

//--------------------------------------------------------------
/**
 * @brief ISS-S の指定スイッチを ON にする
 * @param core 仮想マシン制御構造体へのポインタ
 * @return VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdIssSwitchOn( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*    work  = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*   gsys  = SCRCMD_WORK_GetGameSysWork( work );
  ISS_SYS*        iss   = GAMESYSTEM_GetIssSystem( gsys );
  ISS_SWITCH_SYS* iss_s = ISS_SYS_GetIssSwitchSystem( iss );
  u16             idx   = SCRCMD_GetVMWorkValue( core, work );  // コマンド第一引数

  ISS_SWITCH_SYS_SwitchOn( iss_s, idx );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief ISS-S の指定スイッチを OFF にする
 * @param core 仮想マシン制御構造体へのポインタ
 * @return VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdIssSwitchOff( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*    work  = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*   gsys  = SCRCMD_WORK_GetGameSysWork( work );
  ISS_SYS*        iss   = GAMESYSTEM_GetIssSystem( gsys );
  ISS_SWITCH_SYS* iss_s = ISS_SYS_GetIssSwitchSystem( iss );
  u16             idx   = SCRCMD_GetVMWorkValue( core, work );  // コマンド第一引数

  ISS_SWITCH_SYS_SwitchOff( iss_s, idx );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * @brief ISS-S の指定スイッチの状態をチェックする
 * @param core 仮想マシン制御構造体へのポインタ
 * @return VMCMD_RESULT
 */
//--------------------------------------------------------------
extern VMCMD_RESULT EvCmdIssSwitchCheck( VMHANDLE* core, void* wk )
{
  SCRCMD_WORK*     work = (SCRCMD_WORK*)wk;
  GAMESYS_WORK*    gsys = SCRCMD_WORK_GetGameSysWork( work );
  ISS_SYS*          iss = GAMESYSTEM_GetIssSystem( gsys );
  ISS_SWITCH_SYS* iss_s = ISS_SYS_GetIssSwitchSystem( iss );
  u16*           ret_wk = SCRCMD_GetVMWork( core, work );       // コマンド第一引数
  u16               idx = SCRCMD_GetVMWorkValue( core, work );  // コマンド第二引数

  *ret_wk = ISS_SWITCH_SYS_IsSwitchOn( iss_s, idx );
  return VMCMD_RESULT_CONTINUE;
}

//---------------------------------------------------------------------------------
/**
 * @brief 鳴き声再生イベント
 */
//---------------------------------------------------------------------------------
static GMEVENT_RESULT PlayVoiceEvent( GMEVENT* event, int* seq, void* wk )
{
  u32 player;
  VOICE_PLAY_WORK* vpw = wk;
  //ＢＧＭロード中ならロードし終わるのを待つ
  if ( PMSND_IsLoading() ) return GMEVENT_RES_CONTINUE;

  if (vpw->Mine) player = PMV_PlayVoice_forMine( vpw->MonsNo, vpw->FormNo );
  else player = PMV_PlayVoice( vpw->MonsNo, vpw->FormNo );
  
  SCRCMD_WORK_SetPMVoiceIndex( vpw->scr_work, player );

  return GMEVENT_RES_FINISH;
}

//======================================================================
//  ぺラップ
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
/**
 * ペラップデータがあるかチェック
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPerapDataCheck( VMHANDLE *core, void *wk )
{
  u16* ret_wk  = SCRCMD_GetVMWork( core, work );

  if( Snd_PerapVoiceCheck(SaveData_GetPerapVoice(core->fsys->savedata)) == TRUE ){
    *ret_wk = TRUE;
    return VMCMD_RESULT_CONTINUE;
  }

  *ret_wk = FALSE;
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * ペラップ録音開始
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPerapRecStart( VMHANDLE *core, void *wk )
{
  u16* ret_wk      = SCRCMD_GetVMWork( core, work );

  if( Snd_PerapVoiceRecStart() == MIC_RESULT_SUCCESS ){
    *ret_wk = TRUE;  //成功
    return VMCMD_RESULT_CONTINUE;
  }

  *ret_wk = FALSE;  //失敗
  return VMCMD_RESULT_CONTINUE;

}

//--------------------------------------------------------------
/**
 * ペラップ録音停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPerapRecStop( VMHANDLE *core, void *wk )
{
  Snd_PerapVoiceRecStop();
  return VMCMD_RESULT_SUSPEND;

}

//--------------------------------------------------------------
/**
 * ペラップ録音したデータをセーブ
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPerapSave( VMHANDLE *core, void *wk )
{
  Snd_PerapVoiceDataSave( SaveData_GetPerapVoice(core->fsys->savedata) );
  return VMCMD_RESULT_SUSPEND;

}
#endif

//======================================================================
//  その他
//======================================================================
#if 0 //wb
//--------------------------------------------------------------
/**
 * クライマックス演出サウンドデータ追加ロード
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSndClimaxDataLoad( VMHANDLE *core, void *wk )
{
  Snd_DataSetByScene( SND_SCENE_SUB_CLIMAX, 0, 0 );  //サウンドデータロード(BGM引継ぎ)
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 初期ボリュームセット
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSndInitialVolSet( VMHANDLE *core, void *wk )
{
  u16 no  = VMGetWorkValue(core);
  u16 vol = VMGetWorkValue(core);

  Snd_PlayerSetInitialVolumeBySeqNo( no, vol );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * デモ演出サウンドデータロード
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSndDemo01DataLoad( VMHANDLE *core, void *wk )
{
  if( sys.cont & PAD_KEY_UP ){
    //Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_CHO01_2_SIM, 1 );
    //Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_TOWN01, 1 );
    //Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_TOWN01, 1 );
    Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_TOWN02, 1 );    //ディレイTrあり
  }else{
    //Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_TOWN01, 1 );
    Snd_DataSetByScene( SND_SCENE_DEMO01, SEQ_PL_TOWN02, 1 );    //ディレイTrあり
  }
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * フィールドサウンドデータロード
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSndFieldDataLoad( VMHANDLE *core, void *wk )
{
  u16 bgm = SCRCMD_GetVMWorkValue( core, work );
  Snd_DataSetByScene( SND_SCENE_FIELD, bgm, 1 );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * @brief  今のBGMナンバー取得
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdSndNowBgmNoGet( VMHANDLE *core, void *wk )
{
  u16* ret_wk  = SCRCMD_GetVMWork( core, work );
  *ret_wk    = Snd_NowBgmNoGet();
  return VMCMD_RESULT_CONTINUE;
}
#endif
