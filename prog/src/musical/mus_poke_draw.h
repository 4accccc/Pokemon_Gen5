//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカル用 ポケモン描画
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUS_POKE_DRAW_H__
#define MUS_POKE_DRAW_H__

#include "musical/musical_system.h"
#include "mus_poke_data.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//描画システム
typedef struct _MUS_POKE_DRAW_SYSTEM MUS_POKE_DRAW_SYSTEM;
//描画１匹分
typedef struct _MUS_POKE_DRAW_WORK MUS_POKE_DRAW_WORK;

//装備箇所の位置情報
typedef struct 
{
	BOOL	isEnable;
	VecFx32 pos;
	VecFx32 ofs;
	VecFx32 scale;
	VecFx32 rotOfs;
	u16		rot;
	u16		itemRot;
}MUS_POKE_EQUIP_DATA;

//======================================================================
//	proto
//======================================================================

//システムの初期化と開放
MUS_POKE_DRAW_SYSTEM*	MUS_POKE_DRAW_InitSystem( HEAPID heapId );
void MUS_POKE_DRAW_TermSystem( MUS_POKE_DRAW_SYSTEM* work );

//更新
void MUS_POKE_DRAW_UpdateSystem( MUS_POKE_DRAW_SYSTEM* work );
void MUS_POKE_DRAW_DrawSystem( MUS_POKE_DRAW_SYSTEM* work );

//ポケモンの追加・削除
MUS_POKE_DRAW_WORK* MUS_POKE_DRAW_Add( MUS_POKE_DRAW_SYSTEM* work , MUSICAL_POKE_PARAM *musPoke , const BOOL useBack );
void MUS_POKE_DRAW_Del( MUS_POKE_DRAW_SYSTEM* work , MUS_POKE_DRAW_WORK *drawWork );

//各パラメータの調整
void MUS_POKE_DRAW_SetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos );
void MUS_POKE_DRAW_GetPosition( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *pos );
void MUS_POKE_DRAW_SetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale );
void MUS_POKE_DRAW_GetScale( MUS_POKE_DRAW_WORK *drawWork , VecFx32 *scale );
void MUS_POKE_DRAW_SetRotation( MUS_POKE_DRAW_WORK *drawWork , u16 rot );
void MUS_POKE_DRAW_GetRotation( MUS_POKE_DRAW_WORK *drawWork , u16 *rot );
void MUS_POKE_DRAW_SetShowFlg( MUS_POKE_DRAW_WORK *drawWork , const BOOL flg );
BOOL MUS_POKE_DRAW_GetShowFlg( MUS_POKE_DRAW_WORK *drawWork );
void MUS_POKE_DRAW_StartAnime( MUS_POKE_DRAW_WORK *drawWork );
void MUS_POKE_DRAW_StopAnime( MUS_POKE_DRAW_WORK *drawWork );
void MUS_POKE_DRAW_ChangeAnime( MUS_POKE_DRAW_WORK *drawWork , const u8 anmIdx );

void MUS_POKE_DRAW_FlipFrontBack( MUS_POKE_DRAW_WORK *drawWork );
void MUS_POKE_DRAW_SetFrontBack( MUS_POKE_DRAW_WORK *drawWork , const BOOL isFront );

//MCSSシステムのテクスチャ読み込みアドレスの変更(読み込み前に！
void MUS_POKE_DRAW_SetTexAddres( MUS_POKE_DRAW_SYSTEM* work , u32 adr );
void MUS_POKE_DRAW_SetPltAddres( MUS_POKE_DRAW_SYSTEM* work , u32 adr );

MUS_POKE_EQUIP_DATA *MUS_POKE_DRAW_GetEquipData( MUS_POKE_DRAW_WORK *drawWork , const MUS_POKE_EQUIP_POS pos );
VecFx32 *MUS_POKE_DRAW_GetShadowOfs( MUS_POKE_DRAW_WORK *drawWork );
VecFx32 *MUS_POKE_DRAW_GetRotateOfs( MUS_POKE_DRAW_WORK *drawWork );

#endif MUS_POKE_DRAW_H__