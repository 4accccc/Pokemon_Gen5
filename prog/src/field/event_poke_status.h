///////////////////////////////////////////////////////////////////////////////////////
/**
 *
 * @brief ポケモン系の画面呼び出しイベント
 * @file  event_poke_status.c
 * @author ariizumi
 * @date   2009.10.22
 *
 */
///////////////////////////////////////////////////////////////////////////////////////

#pragma once

FS_EXTERN_OVERLAY( notwifi_event_poke_status );

typedef struct _EVENT_POKESEL_PARAM{
  u16*  retDecide;  ///<選択結果の格納先ワーク
  u16*  retPos;     ///<選択位置の格納先ワーク
}EVENT_POKESEL_PARAM;

typedef struct _EVENT_POKESEL_PARAM_WOBOE{
  u16*  retDecide;  ///<選択結果の格納先ワーク
  u16*  retPos;     ///<選択位置の格納先ワーク
  u8    learnBit;   ///<覚えられるポケモンを示すビット
  u16   wazano;     ///<覚える対象の技ナンバー
}EVENT_POKESEL_PARAM_WOBOE;

typedef struct _EVENT_POKESEL_PARAM_WSELECT{
 u16* retDecide;    ///< 選択結果の格納先ワーク
 u16* retPos;       ///< 選択位置の格納先ワーク
 u8   pokePos;      ///< ポケモンの位置
 u16  wazano;       ///< 覚える技NO
 BOOL canSelHiden;  ///< 秘伝選択可能
}EVENT_POKESEL_PARAM_WSELECT;

//-------------------------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント
 *
 * @param gsys      ゲームシステム
 * @param parent    EVENT_POKESEL_PARAM*  選択結果の格納パラメータ
 *
 * @return イベント
 */
//-------------------------------------------------------------------------------------
extern GMEVENT * EVENT_CreatePokeSelect( 
    GAMESYS_WORK * gsys, void* parent );

//------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント：わざ覚え
 *
 * @param gsys      ゲームシステム
 * @param parent    EVENT_POKESEL_PARAM_WOBOE*  選択結果の格納パラメータ他
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_CreatePokeSelectWazaOboe( 
    GAMESYS_WORK * gsys, void* parent );

//-------------------------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント：ミュージカル
 *
 * @param gsys      ゲームシステム
 * @param parent    EVENT_POKESEL_PARAM*  選択結果の格納パラメータ
 *
 * @return イベント
 */
//-------------------------------------------------------------------------------------
extern GMEVENT * EVENT_CreatePokeSelectMusical( 
    GAMESYS_WORK * gsys, void* parent );

//------------------------------------------------------------------
/**
 * @brief ポケモン選択イベント
 *
 * @param gsys      ゲームシステム
 * @param parent    EVENT_POKESEL_PARAM_WSELECT*  選択結果の格納パラメータ他
 */
//------------------------------------------------------------------
extern GMEVENT * EVENT_CreateWazaSelect( GAMESYS_WORK * gsys, void* parent );
