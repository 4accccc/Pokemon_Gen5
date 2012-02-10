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
#include "system/time_icon.h"
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
// SaveFlag
enum{
	FLG_SAVE_START		= (1<<0),		// 開始
	//
	FLG_SAVE_SLOT_0		= (1<<4),		// slot 0
	FLG_SAVE_SLOT_1		= (1<<5),		// slot 1
	FLG_SAVE_SLOT_2		= (1<<6),		// slot 2
};

//=============================================================================
/**
 *								データ
 */
//=============================================================================
//--------------
// セーブ情報
//--------------
static const s32 td_save_id_tbl[] = {
	SAVE_EXTRA_ID_WBT_00, EXGMDATA_ID_WBT_00,		// slot 0
	SAVE_EXTRA_ID_WBT_01, EXGMDATA_ID_WBT_01,		// slot 1
	SAVE_EXTRA_ID_WBT_02, EXGMDATA_ID_WBT_02,		// slot 2
};
	
//=============================================================================
/**
 *								宣言
 */
//=============================================================================
static BOOL td_load_init_data( T_DOWNLOAD_MAIN_WORK* wk, const s32 num );
static void td_set_save_after( T_DOWNLOAD_MAIN_WORK * wk );
//
static BOOL td_chk_get_pokemon( T_DOWNLOAD_MAIN_WORK * wk, const s32 id );
static s32  td_chk_download_data( T_DOWNLOAD_MAIN_WORK* wk );

//=============================================================================
/**
 *								関数
 */
//=============================================================================
//-----------------------------------------------------------------------------
/**
 *	@brief  セーブデータ読み込み
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					個数
 */
//-----------------------------------------------------------------------------
s32 TdSave_init_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		cnt=0;
	BOOL	bflg;
	
	// 取得
	for(i=0;i<3;i++){
		bflg = td_load_init_data( wk, i );
		if(bflg == TRUE){
			wk->SaveFlag |= (1<<(4+i));
			// セットした最後のスロット番号
			wk->SlotLast = i;
			cnt++;
		}
	}
	//
	OS_Printf("save cnt :%d\n", cnt);
	return( cnt );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  データ読み込み
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(番号:0-2)
 *
 *	@retval BOOL				(TRUE:成功、FALSE:失敗)
 */
//-----------------------------------------------------------------------------
static BOOL td_load_init_data( T_DOWNLOAD_MAIN_WORK* wk, const s32 num )
{
	GAMEDATA*	 		gamedata;
	SAVE_CONTROL_WORK*	save;
	WBTEXSAVE*			save_p;
	LOAD_RESULT			load_res;
	BOOL				bflg=FALSE;
	//
	// メモリは確保してしまう
	wk->WbtSaveData[num] = GFL_HEAP_AllocMemoryLo( wk->heapID, sizeof(WBTDL_DATA) );
	GFL_STD_MemClear32( wk->WbtSaveData[num], sizeof(WBTDL_DATA) );
	//
	// ロードしてセット
	gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	save = GAMEDATA_GetSaveControlWork( gamedata );
	load_res = SaveControl_Extra_Load( save, td_save_id_tbl[2*num], wk->heapID );
//	OS_Printf("Result = %d:%d\n", num,load_res );
	//
	// 成功時のみセット
	if(load_res == LOAD_RESULT_OK){
		save_p = SaveControl_Extra_DataPtrGet( save, td_save_id_tbl[2*num], td_save_id_tbl[2*num+1] );
		// save_p => WbtSaveData[]へ
		GFL_STD_MemCopy32( save_p, wk->WbtSaveData[num], sizeof(WBTDL_DATA) );
		bflg = TRUE;
		OS_Printf("ok :%d\n",num);
	}
	// 外部データを開放
	SaveControl_Extra_Unload( save, td_save_id_tbl[2*num] );
	//
	return( bflg );
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
 *	@brief  リストデータの初期化(SELECTのみ通る処理)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					(個数)
 */
//-----------------------------------------------------------------------------
s32 TdSave_init_list_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32				cnt;
	//
	// データをチェックして必要なデータをセット
	cnt = td_chk_download_data( wk );
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
	BOOL	bflg;
	//
	// ポケモン
	if(mode == T_DOWNLOAD_SAVE_ITEM_POKE){
		cadr = WBTDL_DATA_GetVetoPokemonBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKENUMBER_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					// 持っているポケモンかどうかの判定
					bflg = td_chk_get_pokemon( wk, i*8+j );
					if(bflg == TRUE){
						k++;
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
				}
			}
		}
	}
	// 技(max 1つのみでよいらしい)
	else if(mode == T_DOWNLOAD_SAVE_ITEM_SKILL){
		//
		k=0;
		for(i=WAZANO_HATAKU;i<WAZANO_MAX;i++){
			bflg = WBTDL_DATA_IsVetoWazaNo( wbtdl, i );
			if(bflg == FALSE){
				k++;
			}
		}
		// 念のため
		if(k >= 1){
			k = 1;
		}
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
	BOOL	bflg;
	//
	// ポケモン
	if(mode == T_DOWNLOAD_SAVE_ITEM_POKE){
		cadr = WBTDL_DATA_GetVetoPokemonBit( wbtdl );
		k=0;
		for(i=0;i<WBTDL_POKENUMBER_MAX;i++){
			for(j=0;j<8;j++){
				// bitが立っていたらカウント
				if(cadr[i] & (1<<j)){
					// 持っているポケモンかどうかを調べる
					bflg = td_chk_get_pokemon( wk, i*8+j );
					if(bflg == TRUE){
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
	// 技(1つのみでよいらしい)
	else if(mode == T_DOWNLOAD_SAVE_ITEM_SKILL){
		for(i=WAZANO_HATAKU;i<WAZANO_MAX;i++){
			bflg = WBTDL_DATA_IsVetoWazaNo( wbtdl, i );
//			if(bflg == TRUE){
			if(bflg == FALSE){
				return( i );
			}
		}
		// 保留
		return( 0 );
	}
	//
	return( 0 );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  セーブ初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32			(番号:0-11)
 *	@param	const s32			(番号:0-2)
 *
 *	@retval BOOL				(TRUE:成功、FALSE:失敗)
 */
//-----------------------------------------------------------------------------
void TdSave_init_save( T_DOWNLOAD_MAIN_WORK* wk, const s32 src, const s32 dest )
{
	GAMEDATA*	 		gamedata;
	SAVE_CONTROL_WORK*	save;
	WBTEXSAVE*			save_p;
	LOAD_RESULT			load_res;
	const WBTDL_DATA*	wbtdl;
	//
	// 念のため
	if(wk->SaveFlag & FLG_SAVE_START){
		OS_Printf("save flag error(init)\n");
		return;
	}
	// アドレス指定
//	wbtdl = &wk->Param->data->data[src];
	wbtdl = wk->CheckBuffer[src];
	//
	// セーブ先を保存
	wk->SaveSrc		= src;
	wk->SaveDest	= dest;
	//
	// 領域をセット
	gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	save = GAMEDATA_GetSaveControlWork( gamedata );
	load_res = SaveControl_Extra_Load( save, td_save_id_tbl[2*dest], wk->heapID );
	save_p = SaveControl_Extra_DataPtrGet( save, td_save_id_tbl[2*dest], td_save_id_tbl[2*dest+1] );
	// データをセット
	// wbtdl => save_p
	GFL_STD_MemCopy32( wbtdl, save_p, sizeof(WBTDL_DATA) );
	//
	// 開始
	GAMEDATA_ExtraSaveAsyncStart( gamedata, td_save_id_tbl[2*dest] ); 
	wk->SaveFlag |= FLG_SAVE_START;
}

//-----------------------------------------------------------------------------
/**
 *	@brief  セーブ中
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval BOOL				(TRUE:セーブ中、FALSE:終了)
 */
//-----------------------------------------------------------------------------
BOOL TdSave_main_save( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32					num;
	GAMEDATA*	 		gamedata;
	SAVE_CONTROL_WORK*	save;
	SAVE_RESULT			save_ret;
	BOOL				bflg=TRUE;
	const WBTDL_DATA*	wbtdl;
	//
	// 念のため
	if(!(wk->SaveFlag & FLG_SAVE_START)){
		OS_Printf("save flag error(main)\n");
		return( FALSE);
	}
	// 先をセット
	num = wk->SaveDest;
	//
	gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	save = GAMEDATA_GetSaveControlWork( gamedata );
	//
	// セーブ実行
	save_ret = GAMEDATA_ExtraSaveAsyncMain( gamedata, td_save_id_tbl[2*num] );
	//
	// 終了
	if((save_ret == SAVE_RESULT_OK)||(save_ret == SAVE_RESULT_NG)){
		// 開放
		SaveControl_Extra_Unload( save, td_save_id_tbl[2*num] );
		wk->SaveFlag &= ~FLG_SAVE_START;
		//
		//--------------------------
		// 後処理
		// (実データをコピーしておく)
		// download -> local
		//--------------------------
		td_set_save_after( wk );
		//
		bflg = FALSE;
	}
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイムアイコン初期化
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdSave_init_timeicon( T_DOWNLOAD_MAIN_WORK* wk )
{
	// セーブのタイミングで作成するらしい
	// (時間アイコン表示)
	wk->TimeIconWork = TIMEICON_CreateTcbl( wk->MsgTcblSys, 
											wk->PrintUtil[T_DOWNLOAD_MSG_DOWN_MESWIN_01].win, 15,
											TIMEICON_DEFAULT_WAIT, wk->heapID );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイムアイコン処理
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdSave_main_timeicon( T_DOWNLOAD_MAIN_WORK* wk )
{
	if(wk->TimeIconWork){
		TIMEICON_Main( wk->TimeIconWork );
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  タイムアイコン破棄
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval
 */
//-----------------------------------------------------------------------------
void TdSave_exit_timeicon( T_DOWNLOAD_MAIN_WORK* wk )
{
	if(wk->TimeIconWork){
		TIMEICON_Exit( wk->TimeIconWork );
		wk->TimeIconWork = 0;
	}
}

//-----------------------------------------------------------------------------
/**
 *	@brief  空いているスロット番号を返す
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					(-1:空きなし、0-2:空きの場所)
 */
//-----------------------------------------------------------------------------
s32 TdSave_get_save_pos( T_DOWNLOAD_MAIN_WORK* wk )
{
#if 1
	s32		cpos;
	
	// 空き位置を探す
	cpos = TdSave_get_slotpos( wk, 1 );
	return( cpos );
#else
	s32		num;
	
	num = wk->SaveMaxNum;
	if(num > 2){
		return( -1 );
	}
	//
	return( num );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	リストの中にセーブデータがあるかどうか
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					(番号:0-9)
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
BOOL TdSave_chk_save_list( T_DOWNLOAD_MAIN_WORK * wk, const s32 num )
{
#if 1
	s16		i;
	u16		tid[2];
	BOOL	bflg=FALSE;
	//
	// リストの情報のtournamentidと
	// セーブの情報のtournamentidが一致しているかチェック
	// フレーム情報
//	tid[0] = WBTDL_DATA_GetTournamentId( &wk->Param->data->data[num] );
	tid[0] = WBTDL_DATA_GetTournamentId( wk->CheckBuffer[num] );
	// セーブ情報
	for(i=0;i<wk->SlotLast+1;i++){
		if(wk->SaveFlag & (1<<(4+i))){
			tid[1] = WBTDL_DATA_GetTournamentId( wk->WbtSaveData[i] );
			if(tid[0] == tid[1]){
				bflg = TRUE;
				break;
			}
		}
	}
	// 結果
	return( bflg );
#else
	s16		i;
	u16		tid[2];
	BOOL	bflg=FALSE;
	//
	// リストの情報のtournamentidと
	// セーブの情報のtournamentidが一致しているかチェック
	// フレーム情報
//	tid[0] = WBTDL_DATA_GetTournamentId( &wk->Param->data->data[num] );
	tid[0] = WBTDL_DATA_GetTournamentId( wk->CheckBuffer[num] );
	// セーブ情報
	for(i=0;i<wk->SaveMaxNum;i++){
		tid[1] = WBTDL_DATA_GetTournamentId( wk->WbtSaveData[i] );
		if(tid[0] == tid[1]){
			bflg = TRUE;
			break;
		}
	}
	// 結果
	return( bflg );
#endif
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	セーブ後処理
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 *
 * @return  none
 */
//--------------------------------------------------------------------------------------------
static void td_set_save_after( T_DOWNLOAD_MAIN_WORK * wk )
{
	s32		i;
	s32		cnt;
	const WBTDL_DATA*	wbtdl;
	
	//--------------------------
	// (実データをコピーしておく)
	// download -> local
	//--------------------------
//	wbtdl = &wk->Param->data->data[wk->SaveSrc];
	wbtdl = wk->CheckBuffer[wk->SaveSrc];
	GFL_STD_MemCopy32( wbtdl, wk->WbtSaveData[wk->SaveDest], sizeof(WBTDL_DATA) );
	wk->SaveFlag |= (1<<(4+wk->SaveDest));
	//
	cnt = 0;
	for(i=0;i<3;i++){
		if(wk->SaveFlag & (1<<(4+i))){
			// セットした最後のスロット番号
			wk->SlotLast = i;
			cnt++;
		}
	}
	// 念のため
	if(cnt > 3){
		OS_Printf("save max error\n");
		cnt = 3;
	}
	// 最大数をセット
	wk->SaveMaxNum = cnt;
}

//--------------------------------------------------------------------------------------------
/**
 * @brief	持っているポケモンかどうか
 *
 * @param	T_DOWNLOAD_MAIN_WORK*		wk
 * @param	const s32					id
 *
 * @return  BOOL						(TRUE:持っている、FALSE:なし)
 */
//--------------------------------------------------------------------------------------------
static BOOL td_chk_get_pokemon( T_DOWNLOAD_MAIN_WORK * wk, const s32 id )
{
	GAMEDATA*	gamedata = GAMESYSTEM_GetGameData( wk->Param->gamesys );
	ZUKAN_SAVEDATA *zsave = GAMEDATA_GetZukanSave( gamedata );
	BOOL	bflg;
	//
	bflg = ZUKANSAVE_GetPokeGetFlag( zsave, id );
	return( bflg );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  ダウンロードデータのチェック(正しいデータのみリストにセット)
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *
 *	@retval s32					(個数)
 */
//-----------------------------------------------------------------------------
static s32 td_chk_download_data( T_DOWNLOAD_MAIN_WORK* wk )
{
	s32		i;
	s32		j;
	BOOL	bflg;
	const WBTDL_DATA*	wbtdl;
	
	// 不正なデータかどうかをチェック
	j=0;
	for(i=0;i<WBTDL_TOTAL_NUM;i++){
		// ダウンロードしたデータをチェック(ここだけ参照)
		wbtdl = &wk->Param->data->data[i];
		bflg = WBTDL_DATA_CheckIllegal( wbtdl );
		if(bflg == TRUE){
			wk->CheckBuffer[j] = GFL_HEAP_AllocClearMemoryLo( wk->heapID, sizeof(WBTDL_DATA) );
			// wbtdl -> checkbuffer[j]
			GFL_STD_MemCopy32( wbtdl, wk->CheckBuffer[j], sizeof(WBTDL_DATA) );
			//
//			OS_Printf("data ok :%d\n",i);
			j++;
		}
	}
	OS_Printf("total data ok :%d\n",j);
	return( j );
}

//-----------------------------------------------------------------------------
/**
 *	@brief  実際のセーブデータ位置
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk
 *	@param	const s32	mode	(0:はじめて見つかった位置、1:空き位置)
 *
 *	@retval s32					正しく読み込まれた先頭の位置(なし:-1)
 */
//-----------------------------------------------------------------------------
s32 TdSave_get_slotpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode )
{
	s32		i;
	
	// はじめて見つかった位置
	if(mode == 0){
		for(i=0;i<3;i++){
			if(wk->SaveFlag & (1<<(4+i))){
				return(i);
			}
		}
	}
	// はじめからの空き位置
	else{
		for(i=0;i<3;i++){
			if(!(wk->SaveFlag & (1<<(4+i)))){
				return(i);
			}
		}
	}
	return(-1);
}


//-----------------------------------------------------------------------------
/**
 *	@brief  キー操作
 *
 *	@param	T_DOWNLOAD_MAIN_WORK* wk 
 *	@param	const s32			(0:上、1:下)
 *	@param	const s32			(src pos)
 *
 *	@retval s32					(dest pos)
 */
//-----------------------------------------------------------------------------
s32 TdSave_get_flag_curpos( T_DOWNLOAD_MAIN_WORK* wk, const s32 mode, const s32 curpos )
{
	s32		ret=curpos;
	
	//
	// 上に移動
	if(mode == 0){
		// 1の場合
		if(curpos == 1){
			// 0のデータを調べる
			if(wk->SaveFlag & (1<<4)){
				ret = 0;
			}
		}
		// 2の場合
		else if(curpos == 2){
			// 1をチェック
			if(wk->SaveFlag & (1<<(4+1))){
				ret = 1;
			}
			// 0をチェック
			else if(wk->SaveFlag & (1<<4)){
				ret = 0;
			}
		}
	}
	// 下に移動
	else{
		// 1の場合
		if(curpos == 1){
			// 2のデータを調べる
			if(wk->SaveFlag & (1<<(4+2))){
				ret = 2;
			}
		}
		// 0の場合
		else if(curpos == 0){
			// 1をチェック
			if(wk->SaveFlag & (1<<(4+1))){
				ret = 1;
			}
			// 2をチェック
			else if(wk->SaveFlag & (1<<(4+2))){
				ret = 2;
			}
		}
	}
	return( ret );
}

