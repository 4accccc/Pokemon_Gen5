//==============================================================================
/**
 * @file    fest_mission_nw.h
 * @brief   リンクフェスティバル ミッション管理(NotWifi)
 * @author  iwasawa
 * @date    2010.12.16
 */
//==============================================================================

#pragma once

//---------------------------------------------------------
/*
 * @brief ミッションパラメータ一括ワードセット
 */
//---------------------------------------------------------
extern void FEST_MISSION_WordsetParamAll( WORDSET* wset, FEST_MISSION* fmwk );

//---------------------------------------------------------
/*
 * @brief ミッションタイトル文字列取得
 *
 * ミッションパラメータからミッションタイトル文字列を生成
 */
//---------------------------------------------------------
extern void FEST_MISSION_GetTitleStr( GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* mission, HEAPID heapID );

//---------------------------------------------------------
/*
 * @brief ミッション説明文文字列取得
 *
 * ミッションパラメータからミッション説明文を生成
 */
//---------------------------------------------------------
extern void FEST_MISSION_GetExpStr( GFL_MSGDATA* man, STRBUF* str, const FEST_MISSION* mission, HEAPID heapID );

