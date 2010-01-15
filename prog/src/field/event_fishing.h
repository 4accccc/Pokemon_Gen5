/**
 *  @file   event_fishing.h
 *  @brief  釣りイベント
 *  @author Miyuki Iwasawa
 *  @datge  09.12.04
 */


/*
 *  @brief  釣りができるポジションかチェック
 *
 *  @param  outPos  座標が要らなければNULLで良い
 */
extern BOOL FieldFishingCheckPos( GAMEDATA* gdata, FIELDMAP_WORK* fieldmap, VecFx32* outPos );

//------------------------------------------------------------------
/*
 *  @brief  釣りイベント起動
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldFishing( FIELDMAP_WORK* fieldmap, GAMESYS_WORK* gsys );



