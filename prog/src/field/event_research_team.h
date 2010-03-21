/////////////////////////////////////////////////////////////////////////////
/**
 * @breif すれ違い調査隊 関連イベント
 * @file  event_research_team.h
 * @auhor obata
 * @date  2010.03.21
 */
/////////////////////////////////////////////////////////////////////////////
#pragma once
#include <gflib.h>
#include "gamesystem/gamesystem.h"

//---------------------------------------------------------------------------
/**
 * @brief プレイヤーの隊員情報表示イベントを生成する
 *
 * @param gameSystem
 *
 * @return 生成したイベント
 */
//---------------------------------------------------------------------------
extern GMEVENT* EVENT_DispResearchTeamInfo( GAMESYS_WORK* gameSystem );
