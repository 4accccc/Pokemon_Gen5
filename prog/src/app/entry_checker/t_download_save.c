//=============================================================================
/**
 *
 *	@file		t_download_save.c
 *	@brief  	トーナメントダウンロード画面(save)
 *	@author
 *	@date
 *
 *  セーブ関連をまとめた([TdSave_*]と命名)
 *
 */
//=============================================================================
//=============================================================================
//必ず必要なインクルード
#include <gflib.h>
#include "system/gfl_use.h"

#include "strbuf.h"

#include "print/gf_font.h"
#include "print/printsys.h"
//
#include "ui/print_tool.h"
#include "ui/frame_list.h"
//
#include "app/app_menu_common.h"
#include "app/app_taskmenu.h"
#include "app/app_printsys_common.h"
#include "app/app_keycursor.h"
//
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"
#include "savedata/wbt_ex_save.h"
#include "savedata/wbt_download.h"
//
#include "app/t_download.h"
#include "t_download_graphic.h"
#include "t_download_local.h"

#include "t_download_save.h"

//=============================================================================
//=============================================================================
/**
 *								定義
 */
//=============================================================================
//=============================================================================
/**
 *								データ
 */
//=============================================================================
//=============================================================================
/**
 *								宣言
 */
//=============================================================================
//=============================================================================
/**
 *								関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連初期化(しばらくはテスト)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
s32 TdSave_init_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32					i;
	s32					cnt=0;
	GAMEDATA*	 		gamedata;
	SAVE_CONTROL_WORK*	save;
	WBTEXSAVE*			save_p;
	LOAD_RESULT			load_res;
	//
	// メモリ確保
	for(i=0;i<3;i++){
		wk->WbtSaveData[i] = GFL_HEAP_AllocMemoryLo( wk->heapID, sizeof(WBTDL_DATA) );
		GFL_STD_MemClear32( wk->WbtSaveData[i], sizeof(WBTDL_DATA) );
	}
	//
	// ロードしてセット(とりあえず1個のみチェック)
	gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	save = GAMEDATA_GetSaveControlWork( gamedata );
	load_res = SaveControl_Extra_Load( save, SAVE_EXTRA_ID_WBT_00, wk->heapID );
	OS_Printf("WBT00 Load Result = %d\n", load_res );
	//
	// 成功時のみセット
	if(load_res == LOAD_RESULT_OK){
		save_p = SaveControl_Extra_DataPtrGet( save, SAVE_EXTRA_ID_WBT_00, EXGMDATA_ID_WBT_00 );
		// save_p => WbtSaveData[]へ
		GFL_STD_MemCopy32( save_p, wk->WbtSaveData[0], sizeof(WBTDL_DATA) );
		GFL_STD_MemCopy32( save_p, wk->WbtSaveData[1], sizeof(WBTDL_DATA) );		// test
		GFL_STD_MemCopy32( save_p, wk->WbtSaveData[2], sizeof(WBTDL_DATA) );		// test
		// test
		cnt = 1;
	}
	// ここまでで、wbtdl_dataにデータが格納されているはず
	// 終了
	SaveControl_Extra_Unload( save, SAVE_EXTRA_ID_WBT_00 ); // 外部データを解放
	//
	return( cnt );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  データ関連破棄
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdSave_exit_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	
	// 破棄
	for(i=0;i<3;i++){
		if(wk->WbtSaveData[i]){
			GFL_HEAP_FreeMemory( wk->WbtSaveData[i] );
			wk->WbtSaveData[i] = 0;
		}
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  リストデータの初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
s32 TdSave_init_list_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32				i;
	s32				cnt;
	
	// 何個あるかをチェックする
	// =>連番にはなっているはず
	// (本当はそれなりのチェックがあるはず)
	cnt = 0;
	for(i=0;i<WBTDL_TOTAL_NUM;i++){
		cnt++;
	}
	return( cnt );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  文字列の取得(strcode => strbufへ)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const WBTDL_DATA*	データ
 *	@param	const s32			(種類)
 *	@param	STRBUF*				文字列バッファ
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdSave_get_str( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode, STRBUF* strbuf )
{
	const STRCODE *strcode;
	
	// トーナメント名
	if(mode == T_DOWNLOAD_SAVE_STR_NAME){
		strcode = WBTDL_DATA_GetTournamentName( wbtdl );
		GFL_STR_SetStringCodeOrderLength( strbuf, strcode, WBTDL_TOURNAMENT_NAME_LEN );
	}
	// 内容説明
	else if(mode == T_DOWNLOAD_SAVE_STR_EXPLAIN_00){
		strcode = WBTDL_DATA_GetContentsExplain( wbtdl );
		GFL_STR_SetStringCodeOrderLength( strbuf, strcode, WBTDL_TOURNAMENT_EXPLAIN_LEN );
	}
	// レギュレーション説明
	else if(mode == T_DOWNLOAD_SAVE_STR_EXPLAIN_01){
		strcode = WBTDL_DATA_GetRegulationExplain( wbtdl );
		GFL_STR_SetStringCodeOrderLength( strbuf, strcode, WBTDL_TOURNAMENT_REG_EXPLAIN_LEN );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  総アイテム数取得
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const WBTDL_DATA*	データ
 *	@param	const s32			(種類、0:ポケモン、1:道具、2:技)
 *
 *	@retval const s32			(個数)
 */
//-----------------------------------------------------------------------------
s32 TdSave_get_item_num( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode )
{
	s32		i;
	s32		j;
	s32		k=0;
	const u8	*cadr;
	//
	// ポケモン
	if(mode == T_DOWNLOAD_SAVE_ITEM_POKE){
		cadr = WBTDL_DATA_GetVetoPokemonBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKENUMBER_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					k++;
				}
			}
		}
	}
	// 道具
	else if(mode == T_DOWNLOAD_SAVE_ITEM_ITEM){
		cadr = WBTDL_DATA_GetVetoItemBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKEITEM_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					k++;
				}
			}
		}
	}
	// 技
	else if(mode == T_DOWNLOAD_SAVE_ITEM_SKILL){
		// 保留
	}
	//
	return( k );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  アイテムのid取得
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const WBTDL_DATA*	データ
 *	@param	const s32			(種類、0:ポケモン、1:道具、2:技)
 *	@param	const s32			(何番目の要素か:1はじまり)
 *
 *	@retval const s32			(番号)
 */
//-----------------------------------------------------------------------------
s32 TdSave_get_item_id( T_DOWNLOAD_MAIN_WORK* wk, const WBTDL_DATA* wbtdl, const s32 mode, const s32 inum )
{
	s32		i;
	s32		j;
	s32		k=0;
	const u8	*cadr;
	//
	// ポケモン
	if(mode == T_DOWNLOAD_SAVE_ITEM_POKE){
		cadr = WBTDL_DATA_GetVetoPokemonBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKENUMBER_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					k++;
					// 同じ数になった
					// =>通し番号を返す
					if(inum == k){
						return( i*8+j );
					}
				}
			}
		}
	}
	// 道具
	else if(mode == T_DOWNLOAD_SAVE_ITEM_ITEM){
		cadr = WBTDL_DATA_GetVetoItemBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKEITEM_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					k++;
					// 同じ数になった
					// =>通し番号を返す
					if(inum == k){
						return( i*8+j );
					}
				}
			}
		}
	}
	// 技
	else if(mode == T_DOWNLOAD_SAVE_ITEM_SKILL){
		// 保留
		return( 1 );
	}
	//
	return( 0 );
}

