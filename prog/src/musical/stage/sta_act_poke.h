//======================================================================
/**
 * @file	sta_act_poke.h
 * @brief	ステージ用　ポケモン
 * @author	ariizumi
 * @data	09/03/06
 */
//======================================================================
#ifndef STA_ACT_POKE_H__
#define STA_ACT_POKE_H__

#include "musical/mus_poke_draw.h"
#include "musical/mus_item_draw.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//ポケモンの向き(バトルベースなのでデフォは左向き
typedef enum
{
	SPD_LEFT,	//左向き
	SPD_RIGHT,	//右向き
}STA_POKE_DIR;


//======================================================================
//	typedef struct
//======================================================================

typedef struct _STA_POKE_SYS STA_POKE_SYS;
typedef struct _STA_POKE_WORK STA_POKE_WORK;

//======================================================================
//	proto
//======================================================================

STA_POKE_SYS* STA_POKE_InitSystem( HEAPID heapId , ACTING_WORK *actWork , MUS_POKE_DRAW_SYSTEM* drawSys , MUS_ITEM_DRAW_SYSTEM *itemDrawSys , GFL_BBD_SYS* bbdSys );
//ポケモンの描画の後にアイテムの更新をする必要があるので別関数化
void	STA_POKE_UpdateSystem( STA_POKE_SYS *work );
void	STA_POKE_UpdateSystem_Item( STA_POKE_SYS *work );
void	STA_POKE_DrawSystem( STA_POKE_SYS *work );
void	STA_POKE_ExitSystem( STA_POKE_SYS *work );

//システム系
void	STA_POKE_System_SetScrollOffset( STA_POKE_SYS *work , const u16 scrOfs );


//ポケ単体系
STA_POKE_WORK *STA_POKE_CreatePoke( STA_POKE_SYS *work , MUSICAL_POKE_PARAM *musPoke );
void STA_POKE_DeletePoke( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
void STA_POKE_GetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *pos );
void STA_POKE_SetPosition( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos );
void STA_POKE_GetRotate( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , u16 *rotate );
void STA_POKE_SetRotate( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const u16 rotate );
void STA_POKE_GetScale( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *scale );
void STA_POKE_SetScale( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *scale );
void STA_POKE_GetPositionOffset( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , VecFx32 *pos );
void STA_POKE_SetPositionOffset( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const VecFx32 *pos );
void STA_POKE_StartAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
void STA_POKE_StopAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
void STA_POKE_ChangeAnime( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const u8 anmIdx );
void STA_POKE_SetShowFlg( STA_POKE_SYS *work , STA_POKE_WORK *drawWork , const BOOL flg );
BOOL STA_POKE_GetShowFlg( STA_POKE_SYS *work , STA_POKE_WORK *drawWork );
STA_POKE_DIR STA_POKE_GetPokeDir( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );
void STA_POKE_SetPokeDir( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const STA_POKE_DIR dir );
void STA_POKE_SetFrontBack( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const BOOL isFront );
void STA_POKE_SetDrawItem( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , const BOOL isDrawItem );
const VecFx32* STA_POKE_GetRotOffset( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );

//アイテム使用
void STA_POKE_UseItemFunc( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork , MUS_POKE_EQUIP_POS ePos );
const BOOL STA_POKE_IsUsingItem( STA_POKE_SYS *work , STA_POKE_WORK *pokeWork );

#endif STA_ACT_POKE_H__

