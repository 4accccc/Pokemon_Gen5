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

//======================================================================
//  proto
//======================================================================
// SEの停止を監視
static void SoundSeFlag_CheckSeStop( SCRIPT_WORK * sc );
static void SoundSeFlag_SetPlay( SCRIPT_WORK * sc, u32 se_no );
static BOOL SoundSeFlag_IsSePlay( SCRIPT_WORK * sc );

//======================================================================
//  コマンド BGM
//======================================================================
//--------------------------------------------------------------
/**
 * BGM変更
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 現在再生中のBGMを退避し、指定BGMを再生している。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  u16 music = VMGetU16( core );
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
  FIELD_SOUND_PushPlayEventBGM( fsnd, music );
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * BGM停止
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmStop( VMHANDLE *core, void *wk )
{
  PMSND_StopBGM();
  return VMCMD_RESULT_CONTINUE;
}

//--------------------------------------------------------------
/**
 * シーケンスを一時停止または再開
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmPlayerPause( VMHANDLE *core, void *wk )
{
  BOOL flag = VMGetU8(core);
  PMSND_PauseBGM( flag );
  return VMCMD_RESULT_CONTINUE;
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

#if 0 //wb
//--------------------------------------------------------------
/**
 * マップ内限定のBGM指定がセットされる
 * 自転車BGMの制御などに使用
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmSpecialSet( VMHANDLE *core, void *wk )
{
  Snd_FieldBgmSetSpecial( core->fsys, VMGetU16(core) );
  return VMCMD_RESULT_CONTINUE;
}
#endif

//--------------------------------------------------------------
/**
 * BGMフェードアウト待ち　ウェイト部分
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval BOOL TRUE=終了
 */
//--------------------------------------------------------------
static BOOL EvWaitBgmFade( VMHANDLE *core, void *wk )
{
  if( !PMSND_CheckFadeOnBGM() ){
    return( TRUE );
  }
  return FALSE;
}

//--------------------------------------------------------------
/**
 * BGMフェードアウト待ち
 * @param  core    仮想マシン制御構造体へのポインタ
 * @return  VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmFadeOut( VMHANDLE *core, void *wk )
{
  u16 vol    = VMGetU16(core);
  u16 frame  = VMGetU16(core);
  PMSND_FadeOutBGM( frame );
  VMCMD_SetWait( core, EvWaitBgmFade );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * BGMフェードイン待ち(フェードアウトしたものが再開する)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmFadeIn( VMHANDLE *core, void *wk )
{
  u16 frame = VMGetU16(core);
  PMSND_FadeInBGM( frame );
  VMCMD_SetWait( core, EvWaitBgmFade );
  return VMCMD_RESULT_SUSPEND;
}

//--------------------------------------------------------------
/**
 * 現在のマップのBGMを再生
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 * @note 再生とあるが、実際には退避したBGMの復帰を行っている。
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmNowMapPlay( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
  FIELD_SOUND_PopBGM( fsnd );
  return VMCMD_RESULT_CONTINUE;
}

#if 0
//--------------------------------------------------------------
/**
 * BGMの復帰忘れがあれば強制復帰。エラー回避用。
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdBgmForcePop( VMHANDLE *core, void *wk )
{
  SCRCMD_WORK *work = wk;
  GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
  FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
  FIELD_SOUND_ForcePopBGM( fsnd );
  return VMCMD_RESULT_CONTINUE;
}
#endif

#if 0
VMCMD_RESULT EvCmdBgmNowMapPlay( VMHANDLE *core, void *wk )
{
#if 0
  int zone_id = core->fsys->location->zone_id;
  //u16 music  = Snd_PcBgmNoGet( core->fsys, Snd_FieldBgmNoGet(core->fsys,zone_id) );
  u16 music  = Snd_FieldBgmNoGetNonBasicBank( core->fsys,zone_id );
  Snd_BgmPlay( music );
#else
  {
	  u16 trackBit = 0xfcff;	// track 9,10 OFF
    SCRCMD_WORK *work = wk;
    SCRIPT_WORK *sc = SCRCMD_WORK_GetScriptWork( work );
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    SCRIPT_FLDPARAM *fparam = SCRIPT_GetMemberWork(
        sc, ID_EVSCR_WK_FLDPARAM );
    u16 zone_id = FIELDMAP_GetZoneID( fparam->fieldMap );
    u16 bgm = ZONEDATA_GetBGMID( zone_id, GAMEDATA_GetSeasonID(gdata) );
    
    if( bgm != 0 ){
      PMSND_PlayNextBGM_EX( bgm, trackBit, 60, 0 );
    }
  }
#endif
  return VMCMD_RESULT_CONTINUE;
}
#endif

#if 0 //wb
//--------------------------------------------------------------
/**
 * 演出BGM再生(ライバル、サポート、つれてけ)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdPlayerFieldDemoBgmPlay( VMHANDLE *core, void *wk )
{
  Snd_PlayerFieldDemoBgmPlay( SND_SCENE_FIELD, VMGetU16(core) );
  return VMCMD_RESULT_CONTINUE;
}
#endif

#if 0 //wb
//--------------------------------------------------------------
/**
 * フィールドBGMを固定にするフラグセット(セーブしない)
 * @param  core    仮想マシン制御構造体へのポインタ
 * @retval VMCMD_RESULT
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdCtrlBgmFlagSet( VMHANDLE *core, void *wk )
{
  Snd_CtrlBgmFlagSet( VMGetU8(core) );
  return VMCMD_RESULT_CONTINUE;
}
#endif

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
  u8* p_sound_se_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_SOUND_SE_FLAG );

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
  u8* p_sound_se_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_SOUND_SE_FLAG );
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
  u8* p_sound_se_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_SOUND_SE_FLAG );

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
  u8* p_sound_se_flag = SCRIPT_GetMemberWork( sc, ID_EVSCR_SOUND_SE_FLAG );

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
  u16 no = VMGetU16( core );
  
  {
    SCRCMD_WORK *work = wk;
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
    FIELD_SOUND_PushPlayJingleBGM( fsnd, no );
  }
  
  return VMCMD_RESULT_CONTINUE;
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
  if( PMSND_CheckPlayBGM() == FALSE ){
    SCRCMD_WORK *work = wk;
    GAMEDATA *gdata = SCRCMD_WORK_GetGameData( work );
    FIELD_SOUND *fsnd = GAMEDATA_GetFieldSound( gdata );
    FIELD_SOUND_PopBGM( fsnd );
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
 * @todo  色々懸念があった模様なので、サウンドとすり合わせる
 *
  //ぺラップ再生テスト
  //no = MONSNO_PERAPPU;
  //パターンを指定できる関数に置き換える予定
 * ★手持ちの先頭のポケモンの鳴き声を鳴らす時があったら、フォルムを見る必要がある！
  //フィールド上で、スカイフォルムが出現することはない。
  //育て屋に預けるとノーマルフォルムになるので、育て屋の鳴き声もOK
 */
//--------------------------------------------------------------
VMCMD_RESULT EvCmdVoicePlay( VMHANDLE *core, void *wk )
{
  u16 monsno  = SCRCMD_GetVMWorkValue(core, wk);
  u8 formno = VMGetU8(core);
  u8 ptn = VMGetU8(core);     //今は捨てている

  core->vm_register[0] = PMV_PlayVoice( monsno, formno );

  return VMCMD_RESULT_CONTINUE;
}

//return 1 = 終了
static BOOL EvWaitVoicePlay(VMHANDLE *core, void *wk)
{
  if (PMVOICE_CheckPlay( core->vm_register[0] ) == FALSE)
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
