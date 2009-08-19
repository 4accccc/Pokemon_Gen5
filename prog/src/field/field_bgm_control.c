/////////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @file field_bgm_control.c
 * @brief フィールドマップにおけるBGMフェードの操作関数群
 * @author obata
 * @date   2009.08.19
 *
 */
/////////////////////////////////////////////////////////////////////////////////////////

#include "sound/pm_sndsys.h"
#include "field_sound.h"
#include "field_bgm_control.h"


//---------------------------------------------------------------------------------------
/**
 * @breif 指定ゾーンで再生すべきBGMのフェードインを開始する
 *
 * @param p_gdata     ゲームデータ
 * @param zone_id     BGMを開始するゾーン
 * @param fade_frames フェードインに要するフレーム数
 */
//---------------------------------------------------------------------------------------
void FIELD_BGM_CONTROL_FadeIn( GAMEDATA* p_gdata, u16 zone_id, u16 fade_frames )
{
  PLAYER_WORK*     p_player = GAMEDATA_GetPlayerWork( p_gdata, 0 );
  PLAYER_MOVE_FORM form     = PLAYERWORK_GetMoveForm( p_player );
  u32              bgm_no   = FIELD_SOUND_GetFieldBGMNo( p_gdata, form, zone_id );
  u32              now      = PMSND_GetBGMsoundNo();

  if( bgm_no != now )
  {
    FIELD_SOUND_PlayBGM( bgm_no );
    FIELD_SOUND_FadeInBGM( fade_frames );
  }
}

//---------------------------------------------------------------------------------------
/**
 * @breif 指定ゾーンのBGMが再生中のBGMと異なる場合, 再生中のBGMのフェードアウトを開始する
 *
 * @param p_gdata     ゲームデータ
 * @param zone_id     BGMを開始するゾーン
 * @param fade_frames フェードインに要するフレーム数
 */
//---------------------------------------------------------------------------------------
void FIELD_BGM_CONTROL_FadeOut( GAMEDATA* p_gdata, u16 zone_id, u16 fade_frames )
{
  PLAYER_WORK*     p_player = GAMEDATA_GetPlayerWork( p_gdata, 0 );
  PLAYER_MOVE_FORM form     = PLAYERWORK_GetMoveForm( p_player );
  u32              bgm_no   = FIELD_SOUND_GetFieldBGMNo( p_gdata, form, zone_id );
  u32              now      = PMSND_GetBGMsoundNo();

  if( bgm_no != now )
  {
    FIELD_SOUND_FadeOutBGM( fade_frames );
  }
} 

//---------------------------------------------------------------------------------------
/**
 * @brief フェード中かどうかを判定する
 *
 * @return フェード中ならTRUE
 */
//---------------------------------------------------------------------------------------
BOOL FIELD_BGM_CONTROL_IsFade()
{
  return FIELD_SOUND_IsBGMFade();
}
