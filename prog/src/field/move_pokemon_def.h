//============================================================================================
/**
 * @file	move_pokemon_def.h
 * @brief	移動ポケモン定義とか
 * @author	Nozomu Saito
 * @date	06.05.11
 */
//============================================================================================
#ifndef __MOVE_POKEMON_DEF_H__
#define	__MOVE_POKEMON_DEF_H__

#define MOVE_POKE_MAX		(4)	//ライカミ・カザカミ、予備2
#define MOVE_POKE_RAIKAMI	    (0)	//ライカミ
#define MOVE_POKE_KAZAKAMI		(1)	//カザカミ

#define	MVPOKE_LOCATION_MAX	(17)	//移動場所数
#define MVPOKE_ZONE_IDX_HIDE  (MVPOKE_LOCATION_MAX) //隠れている時のゾーンIdx
#define MVPOKE_ZONE_NULL  (0xFFFF)  //無効ゾーンID(隠れている時のID)

///移動ポケモン移動制御タイプ
#define MVPOKE_TYPE_NORMAL  (0) //通常
#define MVPOKE_TYPE_TIME    (1) //タイムゾーン依存

///移動ポケモン状態
#define MVPOKE_STATE_NONE (0)
#define MVPOKE_STATE_DOWN (1) //倒しちゃった
#define MVPOKE_STATE_GET  (2) //捕まえた

#endif	 //__MOVE_POKEMON_DEF_H__
