//============================================================================================
/**
 * @file	saveload_system.c
 * @brief	セーブロードシステム
 * @author	tamada	GAME FREAK Inc.
 * @date	2005.10.12
 *
 * @li		2006.04.17	savedata.cから分離
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "savedata.h"
#include "savedata_local.h"

#include "gflib.h"
#include "heapsys.h"

#include "flash_access.h"



//=============================================================================
//=============================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MIRROR1ST	(0)
#define	MIRROR2ND	(1)
#define	MIRRORERROR	(2)

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
typedef struct {
	u32 g_count;		///<グローバルカウンタ
	u32 size;			///<データサイズ（フッタサイズ含む）
	u32 magic_number;	///<マジックナンバー
	u16 crc;			///<データ全体のCRC値
}SAVE_FOOTER;

//---------------------------------------------------------------------------
/**
 * @brief	バックアップ制御構造体
 */
//---------------------------------------------------------------------------
typedef struct {
	BOOL flash_exists;			///<バックアップFLASHが存在するかどうか
	u32 heap_save_id;			///<セーブデータ用に使用するヒープの指定
	u32 heap_temp_id;			///<テンポラリに使用するヒープの指定
	MATHCRC16Table crc_table;	///<CRC算出用テーブル

	///分割転送制御用ワーク
	u32 counter_backup;
	u16 lock_id;
	int div_seq;

}BACKUP_SYSTEM;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造体
 *
 * セーブデータ自体と、制御用ワークをまとめたもの
 */
//---------------------------------------------------------------------------
struct _SAVEDATA {
	u32 start_ofs;
	u32 savearea_size;
	u32 magic_number;

	BOOL data_exists;			///<データが存在するかどうか
	LOAD_RESULT first_status;	///<一番最初のセーブデータチェック結果
	u32 global_counter;
	u32 current_side;

	///セーブデータ構造定義へのポインタ
	SVDT * svdt;
	u32 svdt_size;

	u8 * svwk;	///<実際のデータ保持領域
};

//============================================================================================
//
//
//			変数
//
//
//============================================================================================
//---------------------------------------------------------------------------
/**
 * @brief	バックアップシステムの制御構造体
 */
//---------------------------------------------------------------------------
static BACKUP_SYSTEM * svsys;


//============================================================================================
//
//
//			公開関数
//
//
//============================================================================================


static SAVE_RESULT NewSVLD_Save(SAVEDATA * sv);
static BOOL NewSVLD_Load(SAVEDATA * sv);
static LOAD_RESULT NewCheckLoadData(SAVEDATA * sv);

static void NEWSVLD_DivSaveInit(SAVEDATA * sv);
static SAVE_RESULT NEWSVLD_DivSaveMain(SAVEDATA * sv);
static void NEWSVLD_DivSaveEnd(SAVEDATA * sv, SAVE_RESULT result);
static void NEWSVLD_DivSaveCancel(SAVEDATA * sv);

static BOOL EraseFlashFooter(const SAVEDATA * sv);


//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
void GFL_BACKUP_Init(u32 heap_save_id, u32 heap_temp_id)
{
	svsys = GFL_HEAP_AllocMemory(heap_save_id, sizeof(BACKUP_SYSTEM));
	GFL_STD_MemClear32(svsys, sizeof(SAVEDATA));
	svsys->heap_save_id = heap_save_id;
	svsys->heap_temp_id = heap_temp_id;
	MATH_CRC16CCITTInitTable(&svsys->crc_table);
	//フラッシュアクセス関連初期化
	svsys->flash_exists = GFL_FLASH_Init();
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
void GFL_BACKUP_Exit(void)
{
	GFL_HEAP_FreeMemory(svsys);
	svsys = NULL;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
SAVEDATA * GFL_SVLD_Create(const GFL_SVLD_PARAM * sv_param)
{
	SAVEDATA * sv;

	sv = GFL_HEAP_AllocMemory(svsys->heap_save_id, sizeof(SAVEDATA));
	GFL_STD_MemClear32(sv, sizeof(SAVEDATA));

	//初期化用パラメータ設定
	sv->start_ofs = sv_param->savearea_top_address;
	sv->savearea_size = sv_param->savearea_size;
	sv->magic_number = sv_param->magic_number;

	//セーブデータ構造テーブル生成
	sv->svdt = SVDT_Create(svsys->heap_save_id, sv_param->table, sv_param->table_max,
			sv->savearea_size, sizeof(SAVE_FOOTER));

	//セーブデータ用メモリを確保
	sv->svwk = GFL_HEAP_AllocMemory(svsys->heap_save_id, sv->savearea_size);
	GFL_STD_MemClear32(sv->svwk, sv->savearea_size);
	sv->data_exists = FALSE;			//データは存在しない

	//実データサイズを計算
	sv->svdt_size = SVDT_GetWorkSize(sv->svdt);


	//データ存在チェックを行っている
	sv->first_status = NewCheckLoadData(sv);
	switch (sv->first_status) {
	case LOAD_RESULT_OK:
	case LOAD_RESULT_NG:
		//まともなデータがあるようなので読み込む
		(void)NewSVLD_Load(sv);
		sv->data_exists = TRUE;			//データは存在する
		break;
	case LOAD_RESULT_NULL:
	case LOAD_RESULT_BREAK:
		//新規 or データ破壊なのでクリアする
		SaveData_ClearData(sv);
		break;
	}

	return sv;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
void GFL_SVLD_Delete(SAVEDATA * sv)
{
	SVDT_Delete(sv->svdt);
	GFL_HEAP_FreeMemory(sv->svwk);
	GFL_HEAP_FreeMemory(sv);
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ（部分）のポインタを取得する
 * @param	sv			セーブデータ構造へのポインタ
 * @param	gmdataID	取得したいセーブデータのID
 * @return	必要なセーブ領域へのポインタ
 */
//---------------------------------------------------------------------------
void * SaveData_Get(SAVEDATA * sv, DATA_ID gmdataID)
{
	return &(sv->svwk[SVDT_GetPageOffset(sv->svdt, gmdataID)]);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const void * SaveData_GetReadOnlyData(const SAVEDATA * sv, DATA_ID gmdataID)
{
	return SaveData_Get((SAVEDATA *)sv, gmdataID);
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータの消去処理
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	TRUE		書き込み成功
 * @retval	FALSE		書き込み失敗
 *
 * セーブデータをクリアした上でフラッシュに書き込む。
 */
//---------------------------------------------------------------------------
BOOL SaveData_Erase(SAVEDATA * sv)
{
	u32 adrs;
	u32 work_size = 0x1000;		//適当
	u8 * buf = GFL_HEAP_AllocMemory(- svsys->heap_temp_id, work_size);
    GFL_UI_SleepDisable(GFL_UI_SLEEP_SVLD);

	//各ブロックのフッタ部分だけを先行して削除する
	EraseFlashFooter(sv);

	GFL_STD_MemFill32(buf, 0xffffffff, work_size);
	for (adrs = 0; adrs < sv->savearea_size; adrs += work_size) {
		GFL_FLASH_Save(adrs + sv->start_ofs, buf, work_size);
		//GFL_FLASH_Save(adrs + sv->start_ofs + sv->savearea_size, buf, work_size);
	}
	GFL_HEAP_FreeMemory(buf);
	SaveData_ClearData(sv);
	sv->data_exists = FALSE;
    GFL_UI_SleepEnable(GFL_UI_SLEEP_SVLD);

	return TRUE;
}

//---------------------------------------------------------------------------
/**
 * @brief	ロード処理
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	TRUE		読み込み成功
 * @retval	FALSE		読み込み失敗
 */
//---------------------------------------------------------------------------
BOOL SaveData_Load(SAVEDATA * sv)
{
	BOOL result;
	if (!svsys->flash_exists) {
		return FALSE;
	}

	result = NewSVLD_Load(sv);

	if (result) {
		sv->data_exists = TRUE;			//データは存在する
		return TRUE;
	} else {
		return FALSE;
	}
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブ処理
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	TRUE		書き込み成功
 * @retval	FALSE		書き込み失敗
 */
//---------------------------------------------------------------------------
SAVE_RESULT SaveData_Save(SAVEDATA * sv)
{
	SAVE_RESULT result;

	if (!svsys->flash_exists) {
		return SAVE_RESULT_NG;
	}

	result = NewSVLD_Save(sv);

	if (result == SAVE_RESULT_OK) {
		sv->data_exists = TRUE;			//データは存在する
	}
	return result;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータの初期化
 * @param	sv			セーブデータ構造へのポインタ
 *
 * SaveData_Eraseと違い、フラッシュに書き込まない。
 * セーブデータがある状態で「さいしょから」遊ぶ場合などの初期化処理
 */
//---------------------------------------------------------------------------
void SaveData_ClearData(SAVEDATA * sv)
{
	SVDT_ClearWork(sv->svwk, sv->svdt);
}

//---------------------------------------------------------------------------
/**
 * @brief	フラッシュ存在フラグのチェック
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOOL		TRUEのとき、フラッシュが存在する
 */
//---------------------------------------------------------------------------
BOOL GFL_BACKUP_IsEnableFlash(const SAVEDATA * sv)
{
	return svsys->flash_exists;
}

//---------------------------------------------------------------------------
/**
 * @brief	最初の読み込み結果を返す
 * @param	sv			セーブデータ構造へのポインタ
 * @return	LOAD_RESULT	読み込み結果（savedata_def.h参照）
 */
//---------------------------------------------------------------------------
LOAD_RESULT SaveData_GetLoadResult(const SAVEDATA * sv)
{
	return sv->first_status;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ存在フラグを取得
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOOL		TRUEのとき、セーブデータが存在する
 */
//---------------------------------------------------------------------------
BOOL SaveData_GetExistFlag(const SAVEDATA * sv)
{
	return sv->data_exists;
}

//============================================================================================
//
//
//		メインデータセーブ処理
//
//
//============================================================================================

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブ初期化
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
void SaveData_DivSave_Init(SAVEDATA * sv)
{
	NEWSVLD_DivSaveInit(sv);
}

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブメイン処理
 * @param	sv			セーブデータ構造へのポインタ
 * @return	SAVE_RESULT
 */
//---------------------------------------------------------------------------
SAVE_RESULT SaveData_DivSave_Main(SAVEDATA * sv)
{
	SAVE_RESULT result;
	result = NEWSVLD_DivSaveMain(sv);
	if (result != SAVE_RESULT_CONTINUE && result != SAVE_RESULT_LAST) {
		NEWSVLD_DivSaveEnd(sv, result);
	}
	return result;
}

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブキャンセル処理
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
void SaveData_DivSave_Cancel(SAVEDATA * sv)
{
	NEWSVLD_DivSaveCancel(sv);
}

//============================================================================================
//
//
//			セーブデータ整合性チェック
//
//
//============================================================================================
typedef struct {
	BOOL IsCorrect;
	u32 g_count;
}CHK_INFO;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void _setDummyInfo(CHK_INFO * chkinfo)
{
	chkinfo->IsCorrect = FALSE;
	chkinfo->g_count = 0;
}
//---------------------------------------------------------------------------
/**
 * @brief	CRCの計算
 * @param	sv		
 * @param	start	セーブデータの開始アドレス
 * @param	size	セーブデータのサイズ（フッタ部分含む）
 */
//---------------------------------------------------------------------------
static u16 _calcFooterCrc(void * start, u32 size)
{
	return MATH_CalcCRC16CCITT(&svsys->crc_table, start, size - sizeof(SAVE_FOOTER));
}
//---------------------------------------------------------------------------
/**
 * @brief	セーブフラッシュへのアドレスオフセット取得
 */
//---------------------------------------------------------------------------
static u32 _getFlashOffset(const SAVEDATA * sv)
{
	return sv->start_ofs;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static SAVE_FOOTER * _getFooterAddress(SAVEDATA * sv, u8 * svwk_adrs)
{
	u8 * start_adr;

	start_adr = svwk_adrs + sv->svdt_size - sizeof(SAVE_FOOTER);
	return (SAVE_FOOTER *)start_adr;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static BOOL _checkSaveFooter(SAVEDATA * sv, u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);

	if (footer->size != sv->svdt_size) {
		return FALSE;
	}
	if (footer->magic_number != sv->magic_number) {
		return FALSE;
	}
	if (footer->crc != _calcFooterCrc(svwk_adrs, sv->svdt_size)) {
		return FALSE;
	}
	return TRUE;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static void _setSaveFooter(SAVEDATA * sv, u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);

	footer->g_count = sv->global_counter;
	footer->size = sv->svdt_size;
	footer->magic_number = sv->magic_number;
	footer->crc = _calcFooterCrc(svwk_adrs, sv->svdt_size);

}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static int _diffCounter(u32 counter1, u32 counter2)
{
	if (counter1 == 0xffffffff && counter2 == 0) {
		return -1;
	}else if (counter1 == 0 && counter2 == 0xffffffff) {
		return 1;
	} else if (counter1 > counter2) {
		return 1;
	} else if (counter1 < counter2) {
		return -1;
	}
	return 0;
}

//---------------------------------------------------------------------------
/**
 * @brief	データのカウンタを調べる
 * @param	chk1	調べるデータその１
 * @param	chk2	調べるデータその２
 * @param	res1	最新データへのオフセット
 * @param	res2	古いデータへのオフセット
 * @return	int		正常なデータの数
 */
//---------------------------------------------------------------------------
static u32 _getNewerData(const CHK_INFO * chk1, const CHK_INFO * chk2, u32 *res1, u32 *res2)
{
	int global;
	global = _diffCounter(chk1->g_count, chk2->g_count);

	if (chk1->IsCorrect && chk2->IsCorrect) {
		//両方とも正常の場合
		if (global > 0) {
			*res1 = MIRROR1ST;
			*res2 = MIRROR2ND;
		}
		else if (global < 0) {
			*res1 = MIRROR2ND;
			*res2 = MIRROR1ST;
		}
		return 2;
	}
	else if (chk1->IsCorrect && !chk2->IsCorrect) {
		//片方のみ正常の場合
		*res1 = MIRROR1ST;
		*res2 = MIRRORERROR;
		return 1;
	}
	else if (!chk1->IsCorrect && chk2->IsCorrect) {
		//片方のみ正常の場合
		*res1 = MIRROR2ND;
		*res2 = MIRRORERROR;
		return 1;
	}
	else {
		//両方とも異常の場合
		*res1 = MIRRORERROR;
		*res2 = MIRRORERROR;
		return 0;
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void _setCurrentInfo(SAVEDATA * sv, const CHK_INFO * chkinfo, u32 n_ofs)
{
	sv->global_counter = chkinfo[n_ofs].g_count;
	sv->current_side = n_ofs;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void _checkBlockInfo(CHK_INFO * chkinfo, SAVEDATA * sv, u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);
	chkinfo->IsCorrect = _checkSaveFooter(sv, svwk_adrs);
	chkinfo->g_count = footer->g_count;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータのチェック
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	LOAD_RESULT		チェック結果
 */
//---------------------------------------------------------------------------
static LOAD_RESULT NewCheckLoadData(SAVEDATA * sv)
{
	CHK_INFO chkinfo[2];
	u8 * buffer1;
	u8 * buffer2;
	u32 nres;
	u32 newer, older;

	buffer1 = GFL_HEAP_AllocMemory(- svsys->heap_temp_id, sv->savearea_size);
	buffer2 = GFL_HEAP_AllocMemory(- svsys->heap_temp_id, sv->savearea_size);

	if(GFL_FLASH_Load(sv->start_ofs, buffer1, sv->savearea_size)) {
		_checkBlockInfo(&chkinfo[MIRROR1ST], sv, buffer1);
	} else {
		_setDummyInfo(&chkinfo[MIRROR1ST]);
	}

	if(GFL_FLASH_Load(sv->start_ofs, buffer2, sv->savearea_size)) {
		_checkBlockInfo(&chkinfo[MIRROR2ND], sv, buffer2);
	} else {
		_setDummyInfo(&chkinfo[MIRROR2ND]);
	}

	GFL_HEAP_FreeMemory(buffer1);
	GFL_HEAP_FreeMemory(buffer2);

	nres = _getNewerData(&chkinfo[MIRROR1ST], &chkinfo[MIRROR2ND], &newer, &older);

	if (nres == 2) {
		// 両方正常データ
		_setCurrentInfo(sv, chkinfo, newer);
		return LOAD_RESULT_OK;
	}
	if (nres == 1) {
		//どちらか壊れている
		_setCurrentInfo(sv, chkinfo, newer);
		return LOAD_RESULT_NG;
	}
	//if (nres == 0) 
	//データ壊れている→単にデータがないとみなす
	return LOAD_RESULT_NULL;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
BOOL NewSVLD_Load(SAVEDATA * sv)
{
	u32 flash_pos;

	flash_pos = _getFlashOffset(sv);
	if (GFL_FLASH_Load(flash_pos, sv->svwk, sv->svdt_size) == FALSE) {
		return FALSE;
	}
	if (_checkSaveFooter(sv, sv->svwk) == FALSE) {
		return FALSE;
	}
	return TRUE;
}

//============================================================================================
//
//
//			分割セーブ
//
//
//============================================================================================
#define	LAST_DATA_SIZE	sizeof(SAVE_FOOTER)
#define	LAST_DATA2_SIZE	(8)

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブセット：データメイン部分
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static u16 _saveDivStart_Body(SAVEDATA *sv)
{
	u32 flash_pos;
	u8 * svwk;

	_setSaveFooter(sv, sv->svwk);
	flash_pos = _getFlashOffset(sv);
	svwk = sv->svwk;

	return GFL_FLASH_SAVEASYNC_Init(flash_pos, svwk, sv->svdt_size - LAST_DATA_SIZE);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブセット：フッタ部分途中
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static u16 _saveDivStart_Footer(SAVEDATA *sv)
{
	u32 flash_pos;
	u8 * svwk;

	flash_pos = _getFlashOffset(sv) + sv->svdt_size - LAST_DATA_SIZE;
	svwk = sv->svwk + sv->svdt_size - LAST_DATA_SIZE;

	return GFL_FLASH_SAVEASYNC_Init(flash_pos, svwk, LAST_DATA_SIZE);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブセット：フッタ部分最後
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static u16 _saveDivStart_Footer2(SAVEDATA *sv)
{
	u32 flash_pos;
	u8 * svwk;

	flash_pos = _getFlashOffset(sv) + sv->svdt_size - LAST_DATA_SIZE + LAST_DATA2_SIZE;
	svwk = sv->svwk + sv->svdt_size - LAST_DATA_SIZE + LAST_DATA2_SIZE;

	return GFL_FLASH_SAVEASYNC_Init(flash_pos, svwk, LAST_DATA2_SIZE);
}



//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブ処理：初期化
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static void NEWSVLD_DivSaveInit(SAVEDATA * sv)
{
	svsys->div_seq = 0;

	svsys->counter_backup = sv->global_counter;
	sv->global_counter ++;

    GFL_UI_SleepDisable(GFL_UI_SLEEP_SVLD);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブ処理：メイン
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	SAVE_RESULT_CONTINUE	セーブ継続中
 * @retval	SAVE_RESULT_LAST		セーブ継続中、最後の部分
 * @retval	SAVE_RESULT_OK			セーブ終了、成功
 * @retval	SAVE_RESULT_NG			セーブ終了、失敗
 */
//---------------------------------------------------------------------------
static SAVE_RESULT NEWSVLD_DivSaveMain(SAVEDATA * sv)
{
	BOOL result;

	switch (svsys->div_seq) {
	case 0:
		svsys->lock_id = _saveDivStart_Body(sv);
		svsys->div_seq ++;
		/* FALL THROUGH */
	case 1:
		//メインデータ部分セーブ
		if (GFL_FLASH_SAVEASYNC_Main(svsys->lock_id, &result) == FALSE) {
			break;
		}
		if (!result) {
			return SAVE_RESULT_NG;
		}
		svsys->div_seq ++;
		/* FALL THROUGH */

	case 2:
		svsys->lock_id = _saveDivStart_Footer2(sv);
		svsys->div_seq ++;
		/* FALL THROUGH */

	case 3:
		//フッタ部分セーブ
		if (GFL_FLASH_SAVEASYNC_Main(svsys->lock_id, &result) == FALSE) {
			break;
		}
		if (!result) {
			return SAVE_RESULT_NG;
		}
		svsys->div_seq ++;
		//最後のブロックのセーブの場合、それを外部に知らせるために
		//SAVE_RESULT_CONTINUEでなくSAVE_RESULT_LASTを返す
		return SAVE_RESULT_LAST;

	case 4:
		svsys->lock_id = _saveDivStart_Footer(sv);
		svsys->div_seq ++;
		/* FALL THROUGH */

	case 5:
		//フッタ部分セーブ
		if (GFL_FLASH_SAVEASYNC_Main(svsys->lock_id, &result) == FALSE) {
			break;
		}
		if (!result) {
			return SAVE_RESULT_NG;
		}
		return SAVE_RESULT_OK;
		break;
	}
	return SAVE_RESULT_CONTINUE;
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブ処理：終了処理
 * @param	sv			セーブデータ構造へのポインタ
 * @param	result		セーブ結果
 */
//---------------------------------------------------------------------------
static void NEWSVLD_DivSaveEnd(SAVEDATA * sv, SAVE_RESULT result)
{
	if (result == SAVE_RESULT_NG) {
		//セーブ失敗の場合
		sv->global_counter = svsys->counter_backup;
	} else {
		//今回セーブしたブロックの参照ミラーを逆転しておく
		sv->current_side = !sv->current_side;
		sv->data_exists = TRUE;			//データは存在する
	}
    GFL_UI_SleepEnable(GFL_UI_SLEEP_SVLD);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブ処理：キャンセル処理
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static void NEWSVLD_DivSaveCancel(SAVEDATA * sv)
{
	sv->global_counter = svsys->counter_backup;
	//非同期処理キャンセルのリクエスト
	GFL_FLASH_SAVEASYNC_Cancel(svsys->lock_id);

    GFL_UI_SleepEnable(GFL_UI_SLEEP_SVLD);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SAVE_RESULT NewSVLD_Save(SAVEDATA * sv)
{
	SAVE_RESULT result;

	NEWSVLD_DivSaveInit(sv);

	do {
		result = NEWSVLD_DivSaveMain(sv);
	}while (result == SAVE_RESULT_CONTINUE || result == SAVE_RESULT_LAST);
	NEWSVLD_DivSaveEnd(sv, result);
	return result;
}

//---------------------------------------------------------------------------
/**
 * @brief	指定ブロックのフッタ部分を消去する
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static BOOL EraseFlashFooter(const SAVEDATA * sv)
{
	u32 flash_pos;
	SAVE_FOOTER dmy_footer;

	GFL_STD_MemFill(&dmy_footer, 0xff, sizeof(SAVE_FOOTER));
	flash_pos = _getFlashOffset(sv) + sv->svdt_size - LAST_DATA_SIZE;

	return GFL_FLASH_Save(flash_pos, &dmy_footer, LAST_DATA_SIZE);
}


