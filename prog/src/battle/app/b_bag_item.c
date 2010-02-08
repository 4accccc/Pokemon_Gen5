//============================================================================================
/**
 * @file	b_bag_item.c
 * @brief	戦闘用バッグ画面 アイテム制御関連
 * @author	Hiroyuki Nakamura
 * @date	09.08.26
 */
//============================================================================================
#include <gflib.h>
/*↑[GS_CONVERT_TAG]*/
/*
#include "system/msgdata.h"
#include "system/numfont.h"
#include "system/wordset.h"
*/
//#include "system/clact_tool.h"
/*↑[GS_CONVERT_TAG]*/
/*
#include "battle/battle_common.h"
#include "itemtool/myitem.h"
*/
#include "b_app_tool.h"

#include "b_bag.h"
#include "b_bag_main.h"
#include "b_bag_item.h"


typedef struct {
	u16	item;
	u16	cost;
}SHOOTER_ITEM;

// ポケットIDテーブル
static const u8 PocketNum[] = {
	2, 3, 0, 1, 0
};

// シューター用アイテムテーブル
static const SHOOTER_ITEM ShooterItemTable[] =
{
	{ ITEM_KURITHIKATTAA, 1 },		// クリティカッター
	{ ITEM_SUPIIDAA, 1 },					// スピーダー
	{ ITEM_SUPESYARUAPPU, 1 },		// スペシャルアップ
	{ ITEM_SUPESYARUGAADO, 1 },		// スペシャルガード
	{ ITEM_DHIFENDAA, 1 },				// ディフェンダー
	{ ITEM_PURASUPAWAA, 1 },			// プラスパワー
	{ ITEM_YOKUATAARU, 1 },				// ヨクアタール
	{ ITEM_EFEKUTOGAADO, 1 },			// エフェクトガード

	{ ITEM_KIZUGUSURI, 2 },				// キズぐすり

	{ ITEM_PIIPIIEIDO, 3 },				// ピーピーエイド
	{ ITEM_AITEMUKOORU, 3 },			// アイテムコール
	{ ITEM_KURITHIKATTO2, 3 },		// クリティカット２
	{ ITEM_SUPIIDAA2, 3 },				// スピーダー２
	{ ITEM_spAPPU2, 3 },					// ＳＰアップ２
	{ ITEM_spGAADO2, 3 },					// ＳＰガード２
	{ ITEM_DHIFENDAA2, 3 },				// ディフェンダー２
	{ ITEM_PURASUPAWAA2, 3 },			// プラスパワー２
	{ ITEM_YOKUATAARU2, 3 },			// ヨクアタール２

	{ ITEM_IIKIZUGUSURI, 4 },			// いいキズぐすり
	{ ITEM_KOORINAOSI, 4 },				// こおりなおし
	{ ITEM_DOKUKESI, 4 },					// どくけし
	{ ITEM_NEMUKEZAMASI, 4 },			// ねむけざまし
	{ ITEM_MAHINAOSI, 4 },				// まひなおし
	{ ITEM_YAKEDONAOSI, 4 },			// やけどなおし

	{ ITEM_SUKIRUKOORU, 5 },			// スキルコール
	{ ITEM_KURITHIKATTO3, 5 },		// クリティカット３
	{ ITEM_SUPIIDAA3, 5 },				// スピーダー３
	{ ITEM_spAPPU3, 5 },					// ＳＰアップ３
	{ ITEM_spGAADO3, 5 },					// ＳＰガード３
	{ ITEM_DHIFENDAA3, 5 },				// ディフェンダー３
	{ ITEM_PURASUPAWAA3, 5 },			// プラスパワー３
	{ ITEM_YOKUATAARU3, 5 },			// ヨクアタール３

	{ ITEM_SUGOIKIZUGUSURI, 6 },	// すごいキズぐすり
	{ ITEM_NANDEMONAOSI, 6 },			// なんでもなおし
	{ ITEM_PIIPIIEIDAA, 6 },			// ピーピーエイダー

	{ ITEM_PIIPIIRIKABAA, 7 },		// ピーピーリカバー
	{ ITEM_AITEMUDOROPPU, 7 },		// アイテムドロップ

	{ ITEM_MANTANNOKUSURI, 8 },		// まんたんのくすり

	{ ITEM_HURATTOKOORU, 9 },			// フラットコール

	{ ITEM_PIIPIIMAKKUSU, 10 },		// ピーピーマックス

	{ ITEM_GENKINOKAKERA, 11 },		// げんきのかけら

	{ ITEM_SUPIIDAA6, 12 },				// スピーダー６
	{ ITEM_spAPPU6, 12 },					// ＳＰアップ６
	{ ITEM_spGAADO6, 12 },				// ＳＰガード６
	{ ITEM_DHIFENDAA6, 12 },			// ディフェンダー６
	{ ITEM_PURASUPAWAA6, 12 },		// プラスパワー６
	{ ITEM_YOKUATAARU6, 12 },			// ヨクアタール６

	{ ITEM_KAIHUKUNOKUSURI, 13 },	// かいふくのくすり

	{ ITEM_GENKINOKATAMARI, 14 },	// げんきのかたまり

	{ ITEM_DUMMY_DATA, 0 },				// 終了
};



//--------------------------------------------------------------------------------------------
/**
 * 前回使用したアイテムをチェック
 *
 * @param	wk		ワーク
 *
 * @retval	"TRUE = あり"
 * @retval	"FALSE = なし"
 */
//--------------------------------------------------------------------------------------------
BOOL BattleBag_UsedItemChack( BBAG_WORK * wk )
{
	if( wk->used_item == ITEM_DUMMY_DATA ){ return FALSE; }

	if( MYITEM_CheckItem( wk->dat->myitem, wk->used_item, 1, wk->dat->heap ) == FALSE ){
		wk->used_item = ITEM_DUMMY_DATA;
		wk->used_poke = 0;
		return FALSE;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------------
/**
 * 最後に使った道具のカーソル位置再設定
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_CorsorReset( BBAG_WORK * wk )
{
	u32	i;

	for( i=0; i<BBAG_POCKET_IN_MAX; i++ ){
		if( wk->used_item == wk->pocket[wk->poke_id][i].id ){
			wk->dat->item_pos[wk->poke_id] = i%6;
			wk->dat->item_scr[wk->poke_id] = i/6;
			break;
		}
	}
}


//--------------------------------------------------------------------------------------------
/**
 * アイテムを戦闘ポケットに振り分ける
 *
 * @param	wk		ワーク
 *
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void BattleBag_PocketInit( BBAG_WORK * wk )
{
	ITEM_ST * item;
	u32	i, j, k;
	s32	prm;

	for( i=0; i<BAG_POKE_MAX; i++ ){
		j = 0;
		while(1){
//			item = MyItem_PosItemGet( wk->dat->myitem, i, j );
			item = MYITEM_PosItemGet( wk->dat->myitem, i, j );
			if( item == NULL ){ break; }
			if( !( item->id == 0 || item->no == 0 ) ){
				prm = ITEM_GetParam( item->id, ITEM_PRM_BTL_POCKET, wk->dat->heap );
				for( k=0; k<5; k++ ){
					if( ( prm & (1<<k) ) == 0 ){ continue; }
					wk->pocket[ PocketNum[k] ][wk->item_max[ PocketNum[k] ]] = *item;
					wk->item_max[ PocketNum[k] ]++;
				}
			}
			j++;
		}
	}

	for( i=0; i<5; i++ ){
		if( wk->item_max[i] == 0 ){
			wk->scr_max[i] = 0;
		}else{
			wk->scr_max[i] = (wk->item_max[i]-1) / 6;
		}
		if( wk->scr_max[i] < wk->dat->item_scr[i] ){
			wk->dat->item_scr[i] = wk->scr_max[i];
		}
	}
}

void BattleBag_ShooterPocketInit( BBAG_WORK * wk )
{
	ITEM_ST	item;
	u32	i, j, k;
	s32	prm;

	i = 0;
	while(1){
		if( ShooterItemTable[i].item == ITEM_DUMMY_DATA || ShooterItemTable[i].cost == 0 ){
			break;
		}
		wk->pocket[0][wk->item_max[0]].id = ShooterItemTable[i].item;
		wk->pocket[0][wk->item_max[0]].no = 1;
		wk->item_max[0]++;
		i++;
	}

	wk->scr_max[0] = (wk->item_max[0]-1) / 6;
}

void BattleBag_GetDemoPocketInit( BBAG_WORK * wk )
{
	wk->pocket[2][0].id = ITEM_MONSUTAABOORU;
	wk->pocket[2][0].no = 30;
	wk->item_max[2] = 1;
	wk->scr_max[2] = 0;
}

//--------------------------------------------------------------------------------------------
/**
 * 指定位置にアイテムがあるか
 *
 * @param	wk		ワーク
 * @param	pos		位置（０～５）
 *
 * @retval	"あり = アイテム番号"
 * @retval	"なし = 0"
 */
//--------------------------------------------------------------------------------------------
u16 BattleBag_PosItemCheck( BBAG_WORK * wk, u32 pos )
{
	if( wk->pocket[wk->poke_id][wk->dat->item_scr[wk->poke_id]*6+pos].id != 0 &&
		wk->pocket[wk->poke_id][wk->dat->item_scr[wk->poke_id]*6+pos].no != 0 ){
		return wk->pocket[wk->poke_id][wk->dat->item_scr[wk->poke_id]*6+pos].id;
	}
	return 0;
}

//--------------------------------------------------------------------------------------------
/**
 * アイテム機能を取得
 *
 * @param	wk		ワーク
 *
 * @return	アイテム機能
 */
//--------------------------------------------------------------------------------------------
u8 BattleBag_ItemUseCheck( BBAG_WORK * wk )
{
//	return (u8)ItemParamGet( wk->dat->ret_item, ITEM_PRM_BATTLE, wk->dat->heap );
	return 0;
}


//--------------------------------------------------------------------------------------------
/**
 * シューター時のコスト取得
 *
 * @param		item		アイテム番号
 *
 * @return	アイテムのコスト
 */
//--------------------------------------------------------------------------------------------
u16 BBAGITEM_GetCost( u16 item )
{
	u32	i = 0;

	while(1){
		if( ShooterItemTable[i].item == 0 || ShooterItemTable[i].cost == 0 ){
			break;
		}
		if( ShooterItemTable[i].item == item ){
			break;
		}
		i++;
	}
	return ShooterItemTable[i].cost;
}
