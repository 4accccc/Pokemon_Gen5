//======================================================================
/**
 * @file	field_sound.c
 * @brief	フィールドのサウンド関連まとめ
 * @author	kagaya
 * @date	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "sound/pm_sndsys.h"

#include "fieldmap.h"
#include "field/zonedata.h"

#include "field_sound.h"

//======================================================================
//  define
//======================================================================
#define TRACKBIT_ALL (0xffff) ///<全track ON
#define TRACKBIT_ACTION ((1<<8)|(1<<9)) ///<アクション用BGM Track 9,10
#define TRACKBIT_STILL (TRACKBIT_ALL^TRACKBIT_ACTION) ///<Action Track OFF

#define PLAY_NEXTBGM_FADEOUT_FRAME (60)
#define PLAY_NEXTBGM_FADEIN_FRAME (0)

//--------------------------------------------------------------
/// フィールドBGM退避回数
//
// フィールドBGM階層
//                    ____ME(jingle
//          ____EVENT(event,trainer...
//  ____BASE(field
//--------------------------------------------------------------
enum
{
  FSND_PUSHCOUNT_NONE = 0, ///<退避なし
  FSND_PUSHCOUNT_BASEBGM = 1, ///<ベースBGM退避中
  FSND_PUSHCOUNT_EVENTBGM, ///<イベントBGM退避中
  FSND_PUSHCOUNT_OVER, ///<退避数オーバー
};

//======================================================================
//  struct  
//======================================================================
//--------------------------------------------------------------
/// FIELD_SOUND
//--------------------------------------------------------------
struct _TAG_FIELD_SOUND
{
  u16 play_event_bgm; //イベントBGM再生中
  s16 push_count; //BGM Pushカウント
};

//======================================================================
//  proto
//======================================================================
static u32 fsnd_GetMapBGMIndex( GAMEDATA *gdata, u32 zone_id );
static void fsnd_PushBGM( FIELD_SOUND *fsnd, int max );
static void fsnd_PopBGM( FIELD_SOUND *fsnd );

//======================================================================
//  FILED_SOUND
//======================================================================
//--------------------------------------------------------------
/**
 * FIELD_SOUND FIELD_SOUND作成
 * @param heapID HEAPID
 * @retval FIELD_SOUND*
 */
//--------------------------------------------------------------
FIELD_SOUND * FIELD_SOUND_Create( HEAPID heapID )
{
  FIELD_SOUND *fsnd;
  fsnd = GFL_HEAP_AllocClearMemory( heapID, sizeof(FIELD_SOUND) );
  return( fsnd );
}

//--------------------------------------------------------------
/**
 * FIELD_SOUND FIELDSOUND削除
 * @param heapID HEAPID
 * @retval FIELD_SOUND*
 */
//--------------------------------------------------------------
void FIELD_SOUND_Delete( FIELD_SOUND *fsnd )
{
  GFL_HEAP_FreeMemory( fsnd );
}

//======================================================================
//  フィールド BGM 再生
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドBGM  BGM再生
 * @param bgmNo 再生するBGMナンバー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PlayBGM( u32 bgmNo )
{ 
  u32 now;
  
  OS_Printf( "FIELD PLAY BGMNO %d\n", bgmNo );
  
  if( bgmNo == 0 ){
    OS_Printf( "WARNING: FIELD PLAY BGMNO ZERO\n" );
    return;
  }
  
  now = PMSND_GetNextBGMsoundNo();
  
  if( now != bgmNo ){
    PMSND_PlayBGM_EX( bgmNo, TRACKBIT_ALL );
  }
}

//--------------------------------------------------------------
/**
 * フィールドBGM フェードアウト → BGM再生
 * @param bgmNo 再生するBGMナンバー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PlayNextBGM( u32 bgmNo )
{ 
  u32 now;
  
  OS_Printf( "FIELD PLAY BGMNO %d\n", bgmNo );
  
  if( bgmNo == 0 ){
    OS_Printf( "WARNING: FIELD PLAY BGMNO ZERO\n" );
    return;
  }
  
  now = PMSND_GetNextBGMsoundNo();
  
  if( now != bgmNo ){
    PMSND_PlayNextBGM_EX( bgmNo, TRACKBIT_ALL,
        PLAY_NEXTBGM_FADEOUT_FRAME, PLAY_NEXTBGM_FADEIN_FRAME );
  }
}

//--------------------------------------------------------------
/**
 * ゾーン切り替え時のBGM変更
 * @param gdata GAMEDATA
 * @param form PLAYER_MOVE_FORM
 * @param zone_id 
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangePlayZoneBGM(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id )
{
  u32 no = FIELD_SOUND_GetFieldBGMNo( gdata, form, zone_id );
  FIELD_SOUND_PlayNextBGM( no );
}

//--------------------------------------------------------------
/**
 * 現在のBGMを退避し、イベント用のBGMを再生
 * @param fsnd FIELD_SOUND
 * @param bgmno BGM番号
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushPlayEventBGM( FIELD_SOUND *fsnd, u32 bgmno )
{
  if( fsnd->push_count <= FSND_PUSHCOUNT_BASEBGM ){ //EVENT BGMに移行済みか
    fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_BASEBGM );
  }
  FIELD_SOUND_PlayBGM( bgmno );
}

//--------------------------------------------------------------
/**
 * 現在のBGMを退避し、ME用のBGMを再生
 * @param fsnd FIELD_SOUND
 * @param bgmno BGM番号
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushPlayJingleBGM( FIELD_SOUND *fsnd, u32 bgmno )
{
  fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_EVENTBGM );
  FIELD_SOUND_PlayBGM( bgmno );
}

//======================================================================
//  フィールドBGM 退避、復帰
//======================================================================
//--------------------------------------------------------------
/**
 * フィールドBGM 退避
 * @param FIELD_SOUND *fsnd
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PushBGM( FIELD_SOUND *fsnd )
{
  fsnd_PushBGM( fsnd, FSND_PUSHCOUNT_BASEBGM );
}

//--------------------------------------------------------------
/**
 * フィールドBGM 復帰
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PopBGM( FIELD_SOUND *fsnd )
{
  fsnd_PopBGM( fsnd );
}

//--------------------------------------------------------------
/**
 * フィールドBGM 退避されていれば強制開放。
 * @param fsnd FIELD_SOUND
 * @retval nothing
 * @note エラー回避用なので基本は使用禁止。
 */
//--------------------------------------------------------------
void FIELD_SOUND_ForcePopBGM( FIELD_SOUND *fsnd )
{
  if( fsnd->push_count ){
    GF_ASSERT( 0 && "ERROR: FSOUND FORGET POP\n" );
    
    while( fsnd->push_count ){
      fsnd_PopBGM( fsnd );
      fsnd->push_count--;
    }
  }
}

//event_debug_menu.c
//event_ircbattle.c
//event_colosseum_battle.c
//scrcmd_trainer.c
//scrcmd_musical.c

//======================================================================
//  フィールド BGM フェード
//======================================================================
//--------------------------------------------------------------
/**
 * @brief BGMフェードイン
 * @param frames フェードインに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeInBGM( u16 frames )
{
  PMSND_FadeInBGM( frames );
}

//--------------------------------------------------------------
/**
 * @brief BGMフェードアウト
 * @param frames フェードアウトに要するフレーム数
 */
//--------------------------------------------------------------
void FIELD_SOUND_FadeOutBGM( u16 frames )
{
  PMSND_FadeOutBGM( frames );
}

//--------------------------------------------------------------
/**
 * @brief BGMフェード検出
 * @return BGMがフェード中かどうか(TRUE : フェード中)
 */
//--------------------------------------------------------------
BOOL FIELD_SOUND_IsBGMFade()
{
  return PMSND_CheckFadeOnBGM();
}

//======================================================================
//  フィールド BGM トラック関連
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド BGM 自機移動時のトラックに変更
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMTrackAction( void )
{
	u16 trackBit = TRACKBIT_ALL;
	PMSND_ChangeBGMtrack( trackBit );
}

//--------------------------------------------------------------
/**
 * フィールド BGM 自機停止時のトラックに変更
 * @param nothing
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMTrackStill( void )
{
	u16 trackBit = TRACKBIT_STILL;
	PMSND_ChangeBGMtrack( trackBit );
}

//--------------------------------------------------------------
/**
 * フィールド BGM ボリューム変更
 * @param vol BGMボリューム 0-127
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMVolume( int vol )
{
	PMSND_ChangeBGMVolume( TRACKBIT_ALL, vol );
}

//--------------------------------------------------------------
/**
 * フィールド BGM アクション用トラックボリューム変更
 * @param vol BGMボリューム 0-127
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_ChangeBGMActionVolume( int vol )
{
	PMSND_ChangeBGMVolume( TRACKBIT_ACTION, vol );
}

//======================================================================
//  フィールド BGM BGMナンバー
//======================================================================
//--------------------------------------------------------------
/**
 * フィールド BGM 再生するBGMナンバー取得
 * @param fieldMap FIELDMAP_WORK *
 * @param zone_id ゾーンID
 * @retval u16 フィールドBGMナンバー
 */
//--------------------------------------------------------------
u32 FIELD_SOUND_GetFieldBGMNo(
    GAMEDATA *gdata, PLAYER_MOVE_FORM form, u32 zone_id )
{
  if( form == PLAYER_MOVE_FORM_SWIM ){
    return( SEQ_BGM_NAMINORI );
  }
  
  if( form == PLAYER_MOVE_FORM_CYCLE ){
    return( SEQ_BGM_BICYCLE );
  }
  
  return( fsnd_GetMapBGMIndex(gdata,zone_id) );
}

//======================================================================
//  トレーナー視線BGM
//======================================================================
//--------------------------------------------------------------
/// 視線BGM対応データ
//--------------------------------------------------------------
#include "tr_tool/trtype_def.h"
#include "../../../resource/trtype_bgm/trtype_bgm.cdat"

//--------------------------------------------------------------
/**
 * トレーナー視線曲を再生します。
 * @param trtype トレーナータイプ
 * @retval u32 BGM番号
 */
//--------------------------------------------------------------
u32 FIELD_SOUND_GetTrainerEyeBgmNo( u32 trtype )
{
  int i = 0;
  while( DATA_TrainerTypeToEyeBGMNo[i][0] != TRTYPE_MAX ){
    if( DATA_TrainerTypeToEyeBGMNo[i][0] == trtype ){
      return( DATA_TrainerTypeToEyeBGMNo[i][1]  );
    }
    i++;
  }
  
  OS_Printf( "ERROR:UNKNOW TRAINER TYPE BGM\n" );
  GF_ASSERT( 0 );
  return( SEQ_BGM_EYE_01 );
}

//======================================================================
//  parts
//======================================================================
//--------------------------------------------------------------
/**
 * 現在のフィールドBGMインデックスを取得
 * @param fieldMap FIELDMAP_WORK
 * @retval u16 BGM Index
 */
//--------------------------------------------------------------
static u32 fsnd_GetMapBGMIndex( GAMEDATA *gdata, u32 zone_id )
{
  u16 idx = ZONEDATA_GetBGMID( zone_id, GAMEDATA_GetSeasonID(gdata) );
  return( idx );
}

//--------------------------------------------------------------
/**
 * BGM退避
 * @param fsnd FIELD_SOUND
 * @param max 超えるとエラーとなる退避数最大
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PushBGM( FIELD_SOUND *fsnd, int max )
{
  int count = fsnd->push_count + 1;
  
  if( count > max || count >= FSND_PUSHCOUNT_OVER ){
    GF_ASSERT( 0 && "ERROR:FSOUND PUSH COUNT OVER" );
    return;
  }
  
  PMSND_PauseBGM( TRUE );
  PMSND_PushBGM();
  fsnd->push_count++;
}

//--------------------------------------------------------------
/**
 * BGM復帰
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PopBGM( FIELD_SOUND *fsnd )
{
  int count = fsnd->push_count - 1;
  
  if( count < 0 ){
    GF_ASSERT( 0 && "ERROR:FSOUND POP COUNT ERROR" );
    return;
  }
  
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );
  fsnd->push_count--;
}
