//======================================================================
/**
 * @file	dressup_system.h
 * @brief	ミュージカル用 ポケモン描画
 * @author	ariizumi
 * @data	09/02/10
 */
//======================================================================
#ifndef MUS_ITEM_DRAW_H__
#define MUS_ITEM_DRAW_H__

#include "musical/musical_system.h"
#include "musical/mus_item_data.h"

//======================================================================
//	define
//======================================================================

//エッジマーキングを消すためクリアカラーと一緒のID
#define MUS_ITEM_POLIGON_ID (63)

//======================================================================
//	enum
//======================================================================

//======================================================================
//	typedef struct
//======================================================================
//描画システム
typedef struct _MUS_ITEM_DRAW_SYSTEM MUS_ITEM_DRAW_SYSTEM;
//描画１匹分
typedef struct _MUS_ITEM_DRAW_WORK MUS_ITEM_DRAW_WORK;

//======================================================================
//	proto
//======================================================================

//システムの初期化と開放
MUS_ITEM_DRAW_SYSTEM*	MUS_ITEM_DRAW_InitSystem( GFL_BBD_SYS *bbdSys , u16 itemMax , HEAPID heapId );
void MUS_ITEM_DRAW_TermSystem( MUS_ITEM_DRAW_SYSTEM* work );

//更新
void MUS_ITEM_DRAW_UpdateSystem( MUS_ITEM_DRAW_SYSTEM* work );
void MUS_ITEM_DRAW_UpdateSystem_VBlank( MUS_ITEM_DRAW_SYSTEM* work );

//アイテムの追加・削除
//アイテム番号からリソースの読み込み
GFL_G3D_RES* MUS_ITEM_DRAW_LoadResource( u16 itemIdx );
void MUS_ITEM_DRAW_DeleteResource( GFL_G3D_RES *res );
//Vramへの転送
int MUS_ITEM_DRAW_TransResource( MUS_ITEM_DRAW_SYSTEM* work , GFL_G3D_RES* g3DresTex , u16 itemIdx );
void MUS_ITEM_DRAW_ReleaseTransResource( MUS_ITEM_DRAW_SYSTEM* work , int resIdx );

//番号から追加と、読み込み済みリソースから追加
MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddItemId( MUS_ITEM_DRAW_SYSTEM* work , u16 itemIdx , VecFx32 *pos );
MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddResource( MUS_ITEM_DRAW_SYSTEM* work ,  u16 itemIdx ,GFL_G3D_RES* g3DresTex, VecFx32 *pos );
MUS_ITEM_DRAW_WORK* MUS_ITEM_DRAW_AddResIdx( MUS_ITEM_DRAW_SYSTEM* work , u16 itemIdx , int resIdx , VecFx32 *pos );
void MUS_ITEM_DRAW_DelItem( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork );

//アイテム番号からARCの番号を調べる
u16 MUS_ITEM_DRAW_GetArcIdx( const u16 itemIdx );
void MUS_ITEM_DRAW_GetPicSize( MUS_ITEM_DRAW_WORK *itemWork , u8 *sizeXRate , u8 *sizeYRate );

const BOOL MUS_ITEM_DRAW_CanEquipPos( MUS_ITEM_DRAW_WORK *itemWork , const MUS_POKE_EQUIP_POS pos );
const BOOL MUS_ITEM_DRAW_CanEquipPosItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_POS pos );
const BOOL MUS_ITEM_DRAW_CanEquipPosUserDataItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_USER pos );
const BOOL MUS_ITEM_DRAW_CheckMainPosUserDataItemNo( MUS_ITEM_DRAW_SYSTEM* work , const u16 itemNo , const MUS_POKE_EQUIP_USER pos );
const BOOL MUS_ITEM_DRAW_IsBackItem( MUS_ITEM_DRAW_WORK *itemWork );
const BOOL MUS_ITEM_DRAW_IsFrontItem( MUS_ITEM_DRAW_WORK *itemWork );
const BOOL MUS_ITEM_DRAW_CanReverseItem( MUS_ITEM_DRAW_WORK *itemWork );

//絵の変更
void MUS_ITEM_DRAW_ChengeGraphic( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 newId , GFL_G3D_RES *newRes );
void MUS_ITEM_DRAW_ChengeGraphicResIdx( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 newId , u16 newResId );
void MUS_ITEM_DRAW_SetShadowPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes );
void MUS_ITEM_DRAW_SetDarkPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes , void **pltWork );
void MUS_ITEM_DRAW_ResetShadowPallet( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , GFL_G3D_RES *shadowRes );

//影用に各数値をコピーする
void MUS_ITEM_DRAW_CopyItemDataToShadow(  MUS_ITEM_DRAW_SYSTEM* work ,MUS_ITEM_DRAW_WORK *baseItem , MUS_ITEM_DRAW_WORK *shadowItem);

//値変換
void MUS_ITEM_DRAW_SetDrawEnable( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , BOOL flg );
void MUS_ITEM_DRAW_SetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos );
void MUS_ITEM_DRAW_GetPosition( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , VecFx32 *pos );
void MUS_ITEM_DRAW_SetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 sizeX , fx16 sizeY );
void MUS_ITEM_DRAW_GetSize( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , fx16 *sizeX , fx16 *sizeY );
void MUS_ITEM_DRAW_SetRotation( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 rotZ );
void MUS_ITEM_DRAW_GetRotation( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u16 *rotZ );
void MUS_ITEM_DRAW_SetAlpha( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , u8 alpha );
void MUS_ITEM_DRAW_SetFlipS( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , const BOOL );
void MUS_ITEM_DRAW_SetUseOffset( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork , const BOOL flg );
const BOOL MUS_ITEM_DRAW_GetUseOffset( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork );
void MUS_ITEM_DRAW_GetOffsetPos( MUS_ITEM_DRAW_WORK *itemWork , GFL_POINT *pos );

//座標チェック系
void MUS_ITEM_DRAW_CheckHit( MUS_ITEM_DRAW_WORK *itemWork , fx16 posX , fx16 posY );

//MUS_ITEM_DATA_SYSの取得
MUS_ITEM_DATA_SYS* MUS_ITEM_DRAW_GetItemDataSys( MUS_ITEM_DRAW_SYSTEM* work );

#if DEB_ARI
void MUS_ITEM_DRAW_Debug_DumpResData( MUS_ITEM_DRAW_SYSTEM* work , MUS_ITEM_DRAW_WORK *itemWork );
#endif

#if USE_MUSICAL_EDIT
void MUS_ITEM_DRAW_GetDispOffset( MUS_ITEM_DRAW_WORK* itemWork , GFL_POINT* ofs );
#endif	//USE_MUSICAL_EDIT

#endif MUS_ITEM_DRAW_H__