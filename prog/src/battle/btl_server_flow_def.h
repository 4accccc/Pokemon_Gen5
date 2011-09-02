//=============================================================================================
/**
 * @file  btl_server_flow_def.h
 * @brief ポケモンWB バトルシステム サーバコマンド生成処理 関連モジュール定義ヘッダ
 * @author  taya
 *
 * @date  2010.03.16  作成
 */
//=============================================================================================
#pragma once

typedef struct _BTL_SVFLOW_WORK   BTL_SVFLOW_WORK;
typedef struct _SVCL_WORK         SVCL_WORK;

//=================================================================================
// ポケウッド勝利条件・敗北条件
//=================================================================================
#define PW_SVSUB_WIN                      (  1 )  // クリア条件をこなした
#define PW_SVSUB_LOSE_TURN_OVER           ( -2 )  // ターン超過
#define PW_SVSUB_LOSE_ALL_DEAD_MINE       ( -3 )  // 自分が全滅
#define PW_SVSUB_LOSE_DEAD_ENEMY          ( -4 )  // 相手を１匹倒した
#define PW_SVSUB_LOSE_ALL_DEAD_ENEMY      ( -5 )  // 相手を全滅させた				
#define PW_SVSUB_LOSE_WRONG_DEAD_ENEMY    ( -6 )  // 指定とは違うポケモンを倒した。
#define PW_SVSUB_LOSE_DEAD_SP_ENEMY       ( -7 )  // 特定のポケモンを倒してしまった。
#define PW_SVSUB_LOSE_NO_DEAD_TURN        ( -8 )  // 敵を倒せないターンが存在
#define PW_SVSUB_LOSE_DEAD_ENEMY_NO_WAZA  ( -9 )  // 指定のポケモンを倒した時に指定の技を使っていなかった。
