/*
 *  @file   fskill_amaikaori.h
 *  @brief  フィールド技：「あまいかおり」
 *  @author Miyuki Iwasawa
 *  @date   09.10.27
 */

#pragma once

//------------------------------------------------------------------
/*
 *  @brief  あまいみつイベント起動
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldAmaimitu( FIELDMAP_WORK* fieldmap, GAMESYS_WORK* gsys );

//------------------------------------------------------------------
/*
 *  @brief  あまいかおりイベント起動
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_FieldSkillAmaikaori( GAMESYS_WORK* gsys, FIELDMAP_WORK* fieldmap, u8 poke_pos );


