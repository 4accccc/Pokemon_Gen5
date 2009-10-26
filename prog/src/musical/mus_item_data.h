//======================================================================
/**
 * @file	mus_item_data.h
 * @brief	ミュージカル用 アイテムデータ
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================

#pragma once

#include "musical/musical_system.h"

//======================================================================
//	define
//======================================================================

//======================================================================
//	enum
//======================================================================
//ミュージカルポケモンの装備箇所識別用
//アイテムデータに設定される。装備できる場所の種類
typedef enum
{
	MUS_POKE_EQU_TYPE_EAR,		//耳(右耳・左耳)
	MUS_POKE_EQU_TYPE_HEAD,		//頭
	MUS_POKE_EQU_TYPE_EYE,		//目
	MUS_POKE_EQU_TYPE_FACE,		//顔
	MUS_POKE_EQU_TYPE_BODY,		//胴
	MUS_POKE_EQU_TYPE_WAIST,	//腰
	MUS_POKE_EQU_TYPE_HAND,		//手(右手・左手)

	MUS_POKE_EQU_IS_BACK,			//背面装備判定
	MUS_POKE_EQU_IS_FRONT,		//前面装備判定

	MUS_POKE_EQUIP_TYPE_MAX,
	
	MUS_POKE_EQU_TYPE_INVALID,	//無効値
}MUS_POKE_EQUIP_TYPE;

//ミュ－ジカルのアイテム使用タイプ
typedef enum
{
  MUT_SPIN,
  MUT_FLASH,
  MUT_FLYING,
  MUT_THROW,
  MUT_USE,
  
  MUT_MAX,
}MUSICAL_ITEM_USETYPE;

//======================================================================
//	typedef struct
//======================================================================
typedef struct _MUS_ITEM_DATA_SYS MUS_ITEM_DATA_SYS;
//１個分
typedef struct _MUS_ITEM_DATA_WORK MUS_ITEM_DATA_WORK;

//======================================================================
//	proto
//======================================================================

MUS_ITEM_DATA_SYS* MUS_ITEM_DATA_InitSystem( HEAPID heapId );
void MUS_ITEM_DATA_ExitSystem( MUS_ITEM_DATA_SYS *sysWork );

//MUS_ITEM_DRAW_WORKの作成で内部で作成されているので、描画を一緒に使う場合はそっちから取得してください
MUS_ITEM_DATA_WORK* MUS_ITEM_DATA_GetMusItemData( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo );

void	MUS_ITEM_DATA_GetDispOffset( MUS_ITEM_DATA_WORK*  dataWork , GFL_POINT* ofs );
GFL_BBD_TEXSIZ	MUS_ITEM_DATA_GetTexType( MUS_ITEM_DATA_WORK*  dataWork );
const BOOL	MUS_ITEM_DATA_CanEquipPos( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_POS pos );
const BOOL	MUS_ITEM_DATA_CanEquipPosUserData( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_USER pos );

//指定された箇所がメインとなる装備箇所か？
const BOOL	MUS_ITEM_DATA_CheckMainPosUserData( MUS_ITEM_DATA_WORK*  dataWork , const MUS_POKE_EQUIP_USER pos );

//装備が背面に来るアイテムか？
const BOOL	MUS_ITEM_DATA_IsBackItem( MUS_ITEM_DATA_WORK*  dataWork );
//装備が前面面に来るアイテムか？
const BOOL	MUS_ITEM_DATA_IsFrontItem( MUS_ITEM_DATA_WORK*  dataWork );

//装備箇所からユーザータイプ装備箇所へ変換
const MUS_POKE_EQUIP_USER MUS_ITEM_DATA_EquipPosToUserType( const MUS_POKE_EQUIP_USER pos );

//アイテムのコンディションタイプを取得
const MUSICAL_CONDITION_TYPE MUS_ITEM_DATA_GetItemConditionType( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo );

//アイテムの使用タイプを取得
const MUSICAL_ITEM_USETYPE MUS_ITEM_DATA_GetItemUseType( MUS_ITEM_DATA_SYS* sysWork , const u16 itemNo );
