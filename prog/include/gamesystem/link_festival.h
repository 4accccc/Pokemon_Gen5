//==============================================================================
/**
 * @file    link_festival.h
 * @brief   リンクフェスティバル
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#ifndef __H_LINK_FESTIVAL_H__
#define __H_LINK_FESTIVAL_H__

#include "fest_mission.h"

typedef struct _LINK_FESTIVAL LINK_FESTIVAL;

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域生成
 */
//---------------------------------------------------------
extern LINK_FESTIVAL* LINK_FESTIVAL_Create( HEAPID heapID, GAMESYS_WORK* gsys );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル管理領域解放
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_Delete( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル状態アップデート
 */
//---------------------------------------------------------
extern void LINK_FESTIVAL_Update( LINK_FESTIVAL* wk );

//---------------------------------------------------------
/*
 * @brief リンクフェスティバル ミッションワークポインタ取得
 */
//---------------------------------------------------------
extern FEST_MISSION* LINK_FESTIVAL_GetMissionWork( LINK_FESTIVAL* wk );


#endif //__H_LINK_FESTIVAL_H__

