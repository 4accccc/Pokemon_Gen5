//======================================================================
/**
 * @file	plist_plate.c
 * @brief	ポケモンリスト プレート
 * @author	ariizumi
 * @data	09/06/10
 *
 * モジュール名：PLIST_PLATE
 */
//======================================================================
#pragma once

#include "plist_local_def.h"
#include "poke_tool\poke_tool.h"

//プレートサイズ
#define PLIST_PLATE_WIDTH  (16)
#define PLIST_PLATE_HEIGHT (6)
//各表示物の位置
//プレートの左上からの相対座標
#define PLIST_PLATE_POKE_POS_X (24)
#define PLIST_PLATE_POKE_POS_Y (16)

//プレートの色の種類(そのままパレット番号に対応
typedef enum
{
  PPC_NORMAL        = 3,  //通常
  PPC_CHANGE        = 5,  //入れ替え
  PPC_NORMAL_SELECT = 6,  //通常(選択
  PPC_CHANGE_SELECT = 8,  //入れ替え

  //以下は自動設定
  PPC_DEATH        = 4,  //死亡
  PPC_DEATH_SELECT = 7,  //死亡(選択
}PLIST_PLATE_COLTYPE;

//戦闘参加時の状態
typedef enum
{
  PPBO_JOIN_1 = 0,  //いちばんめ(以下略
  PPBO_JOIN_2,
  PPBO_JOIN_3,
  PPBO_JOIN_4,
  PPBO_JOIN_5,
  PPBO_JOIN_6,
  
  PPBO_JOIN_OK, //参加できる
  PPBO_JOIN_NG, //参加できない
  
  PPBO_INVALLID,  //バトル以外

}PLIST_PLATE_BATTLE_ORDER;

typedef enum
{
  PPCB_OK,
  PPCB_NG,
  PPCB_NG_SAME_MONSNO,  //同じ手持ちが要る
  PPCB_NG_SAME_ITEM,    //同じアイテムがある
  PPCB_NG_OVER_NUM,     //人数オーバー
}PLIST_PLATE_CAN_BATTLE;

extern PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate( PLIST_WORK *work , const u8 idx , POKEMON_PARAM *pp );
extern PLIST_PLATE_WORK* PLIST_PLATE_CreatePlate_Blank( PLIST_WORK *work , const u8 idx );

extern void PLIST_PLATE_DeletePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
extern void PLIST_PLATE_UpdatePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

extern void PLIST_PLATE_SetActivePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const BOOL isActive );

extern void PLIST_PLATE_ChangeColor( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , PLIST_PLATE_COLTYPE col );

//入れ替えアニメでプレートを移動させる(キャラ単位
extern void PLIST_PLATE_MovePlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const u8 moveValue );
extern void PLIST_PLATE_MovePlateXY( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const s8 subX , const s8 subY );
extern void PLIST_PLATE_ClearPlate( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const u8 moveValue );
extern void PLIST_PLATE_ResetParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , POKEMON_PARAM *pp , const u8 moveValue );
extern void PLIST_PLATE_ReDrawParam( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

//HPバーアニメ関係
extern void PLIST_PALTE_InitHpAnime( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
extern const BOOL PLIST_PALTE_UpdateHpAnime( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

extern const BOOL PLIST_PLATE_CanSelect( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
//プレートの位置取得(clact用
extern void PLIST_PLATE_GetPlatePosition( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , GFL_CLACTPOS *pos );
//プレートの位置取得(タッチ判定用
extern void PLIST_PLATE_GetPlateRect( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , GFL_UI_TP_HITTBL *hitTbl );
//バトル参加用状態取得・設定
extern const PLIST_PLATE_BATTLE_ORDER PLIST_PLATE_GetBattleOrder( const PLIST_PLATE_WORK *plateWork );
extern void PLIST_PLATE_SetBattleOrder( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork , const PLIST_PLATE_BATTLE_ORDER order );
//選択時、参加可能か？(同じ道具・ポケチェック)
extern const PLIST_PLATE_CAN_BATTLE PLIST_PLATE_CanJoinBattle( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );

//表示HPの取得
extern u16 PLIST_PLATE_GetDispHp( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
//タマゴチェック
const BOOL PLIST_PLATE_IsEgg( PLIST_WORK *work , PLIST_PLATE_WORK *plateWork );
