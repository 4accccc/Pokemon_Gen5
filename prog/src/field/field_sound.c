//======================================================================
/**
 * @file	field_sound.c
 * @brief	フィールドのサウンド関連まとめ
 * @author	kagaya
 * @data	05.07.13
 */
//======================================================================
#include <gflib.h>
#include "system/gfl_use.h"

#include "sound/pm_sndsys.h"
#include "sound/wb_sound_data.sadl"

#include "fieldmap.h"
#include "field/zonedata.h"

#include "field_sound.h"

//======================================================================
//  define
//======================================================================
#define TRACKBIT_ALL (0xffff) ///<全track ON
#define TRACKBIT_ACTION ((1<<9)|(1<<10)) ///<アクション用BGM Track
#define TRACKBIT_STILL (TRACKBIT_ALL^TRACKBIT_ACTION) ///<Action Track OFF

#define PUSH_MAX (1) ///<BGM退避回数最大

//======================================================================
//  struct  
//======================================================================
//--------------------------------------------------------------
/// FIELD_SOUND
//--------------------------------------------------------------
struct _TAG_FIELD_SOUND
{
  int push_count; //BGM Pushカウント
};

//======================================================================
//  proto
//======================================================================
static u32 fsnd_GetMapBGMIndex( FIELDMAP_WORK *fieldMap, u32 zone_id );
static void fsnd_PushCount( FIELD_SOUND *fsnd );
static void fsnd_PopCount( FIELD_SOUND *fsnd );

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
 * フィールドBGM フェードアウト → BGM再生
 * @param bgmNo 再生するBGMナンバー
 * @retval nothing
 */
//--------------------------------------------------------------
void FIELD_SOUND_PlayNextBGM( u32 bgmNo )
{ 
  u32 now;
  
  if( bgmNo == 0 ){
    return;
  }
  
//  now = PMSND_GetBGMsoundNo();
  now = PMSND_GetNextBGMsoundNo();
  
  if( now != bgmNo ){
    u8 fadeOutFrame = 60; //kari
    u8 fadeInFrame = 0; //kari
    PMSND_PlayNextBGM_EX(
        bgmNo, TRACKBIT_STILL, fadeOutFrame, fadeInFrame );
  }
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
void FIELD_SOUND_ChgangeBGMTrackAction( void )
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
u32 FIELD_SOUND_GetFieldBGMNo( FIELDMAP_WORK *fieldMap, u32 zone_id )
{
  PLAYER_MOVE_FORM form;
  FIELD_PLAYER *fld_player;
  
  fld_player = FIELDMAP_GetFieldPlayer( fieldMap );
  form = FIELD_PLAYER_GetMoveForm( fld_player );
//  zone_id = FIELDMAP_GetZoneID( fieldMap );
  
  if( form == PLAYER_MOVE_FORM_SWIM ){
    return( SEQ_NAMINORI );
  }

  if( form == PLAYER_MOVE_FORM_CYCLE ){
    return( SEQ_BICYCLE );
  }
  
  return( fsnd_GetMapBGMIndex(fieldMap,zone_id) );
}

//======================================================================
//  BGM退避、復帰
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
  PMSND_PauseBGM(TRUE);
  PMSND_PushBGM();
  fsnd_PushCount( fsnd );
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
  PMSND_PopBGM();
  PMSND_PauseBGM( FALSE );
  fsnd_PopCount( fsnd );
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
static u32 fsnd_GetMapBGMIndex( FIELDMAP_WORK *fieldMap, u32 zone_id )
{
  GAMESYS_WORK *gsys = FIELDMAP_GetGameSysWork( fieldMap );
  GAMEDATA *gdata = GAMESYSTEM_GetGameData( gsys );
  u16 idx = ZONEDATA_GetBGMID( zone_id, GAMEDATA_GetSeasonID(gdata) );
  return( idx );
}

//--------------------------------------------------------------
/**
 * 退避回数カウント
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PushCount( FIELD_SOUND *fsnd )
{
  fsnd->push_count++;
  GF_ASSERT(fsnd->push_count<=PUSH_MAX && "FIELD SOUND PUSH COUNT OVER");
}

//--------------------------------------------------------------
/**
 * 復帰回数カウント
 * @param fsnd FIELD_SOUND
 * @retval nothing
 */
//--------------------------------------------------------------
static void fsnd_PopCount( FIELD_SOUND *fsnd )
{
  fsnd->push_count--;
  GF_ASSERT(fsnd->push_count>0 && "FIELD SOUND POP COUNT ERROR" );
}

