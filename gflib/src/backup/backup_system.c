//============================================================================================
/**
 * @file	backup_system.c
 * @brief	バックアップシステム
 * @author	tamada	GAME FREAK Inc.
 * @date	2007.06.14
 *
 */
//============================================================================================

#include <nitro.h>
#include <nnsys.h>

#include "backup_system.h"
#include "savedata_local.h"

#include "gflib.h"
#include "heapsys.h"

#include "flash_access.h"



//============================================================================================
//============================================================================================

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#define MIRROR1ST	(0)
#define	MIRROR2ND	(1)
#define	MIRRORERROR	(2)

typedef enum {
	SVLD_STAT_OK,
	SVLD_STAT_NG,
	SVLD_STAT_NULL,
	SVLD_STAT_ERROR,
}SVLD_STATUS;

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
	int save_seq;

}BACKUP_SYSTEM;

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータ構造体
 *
 * セーブデータ自体と、制御用ワークをまとめたもの
 */
//---------------------------------------------------------------------------
struct _GFL_SAVEDATA {
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


static SVLD_STATUS NewSVLD_Load(GFL_SAVEDATA * sv);
static LOAD_RESULT NewCheckLoadData(GFL_SAVEDATA * sv);

static void			SaveAsync_Init(GFL_SAVEDATA * sv);
static SAVE_RESULT	SaveAsync_Main(GFL_SAVEDATA * sv);
static void			SaveAsync_End(GFL_SAVEDATA * sv, SAVE_RESULT result);
static void			SaveAsync_Cancel(GFL_SAVEDATA * sv);

static BOOL EraseFooter(const GFL_SAVEDATA * sv);


//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
void GFL_BACKUP_Init(u32 heap_save_id, u32 heap_temp_id)
{
	svsys = GFL_HEAP_AllocMemory(heap_save_id, sizeof(BACKUP_SYSTEM));
	GFL_STD_MemClear32(svsys, sizeof(BACKUP_SYSTEM));
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
 * @brief	フラッシュ存在フラグのチェック
 * @param	sv			セーブデータ構造へのポインタ
 * @return	BOOL		TRUEのとき、フラッシュが存在する
 */
//---------------------------------------------------------------------------
BOOL GFL_BACKUP_IsEnableFlash(void)
{
	GF_ASSERT(svsys != NULL);	//システムが初期化されていなければならない
	return svsys->flash_exists;
}


//=============================================================================
//
//
//		セーブデータアクセス関連
//
//
//=============================================================================
//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
GFL_SAVEDATA * GFL_SAVEDATA_Create(const GFL_SVLD_PARAM * sv_param)
{
	GFL_SAVEDATA * sv;

	GF_ASSERT(svsys != NULL);	//システムが初期化されていなければならない

	sv = GFL_HEAP_AllocMemory(svsys->heap_save_id, sizeof(GFL_SAVEDATA));
	GFL_STD_MemClear32(sv, sizeof(GFL_SAVEDATA));

	//初期化用パラメータ設定
	sv->start_ofs = sv_param->savearea_top_address;
	sv->savearea_size = sv_param->savearea_size;
	sv->magic_number = sv_param->magic_number;

	//セーブデータ構造テーブル生成
	sv->svdt = SVDT_Create(svsys->heap_save_id, sv_param->table, sv_param->table_max,
			sv->savearea_size, sizeof(SAVE_FOOTER));

	//実データサイズを計算
	sv->svdt_size = SVDT_GetWorkSize(sv->svdt);

	//セーブデータ用メモリを確保
	sv->svwk = GFL_HEAP_AllocMemory(svsys->heap_save_id, sv->savearea_size);
	GFL_STD_MemClear32(sv->svwk, sv->savearea_size);
	sv->data_exists = FALSE;			//データは存在しない
	sv->global_counter = 0;
	sv->current_side = 0;

	sv->first_status = GFL_BACKUP_Load(sv);
	switch (sv->first_status) {
	case LOAD_RESULT_NULL:
	case LOAD_RESULT_BREAK:
		//新規 or データ破壊なのでクリアする
		GFL_SAVEDATA_Clear(sv);
	}

	return sv;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
void GFL_SAVEDATA_Delete(GFL_SAVEDATA * sv)
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
void * GFL_SAVEDATA_Get(GFL_SAVEDATA * sv, GFL_SVDT_ID gmdataID)
{
	return &(sv->svwk[SVDT_GetPageOffset(sv->svdt, gmdataID)]);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
const void * GFL_SAVEDATA_GetReadOnly(const GFL_SAVEDATA * sv, GFL_SVDT_ID gmdataID)
{
	return GFL_SAVEDATA_Get((GFL_SAVEDATA *)sv, gmdataID);
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータの初期化
 * @param	sv			セーブデータ構造へのポインタ
 *
 * GFL_BACKUP_Eraseと違い、フラッシュに書き込まない。
 * セーブデータがある状態で「さいしょから」遊ぶ場合などの初期化処理
 */
//---------------------------------------------------------------------------
void GFL_SAVEDATA_Clear(GFL_SAVEDATA * sv)
{
	SVDT_ClearWork(sv->svwk, sv->svdt);
}

//---------------------------------------------------------------------------
/**
 * @brief	最初の読み込み結果を返す
 * @param	sv			セーブデータ構造へのポインタ
 * @return	LOAD_RESULT	読み込み結果（savedata_def.h参照）
 */
//---------------------------------------------------------------------------
LOAD_RESULT GFL_SAVEDATA_GetLoadResult(const GFL_SAVEDATA * sv)
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
BOOL GFL_SAVEDATA_GetExistFlag(const GFL_SAVEDATA * sv)
{
	return sv->data_exists;
}



//============================================================================================
//
//
//		バックアップアクセス関連
//
//
//============================================================================================
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
BOOL GFL_BACKUP_Erase(GFL_SAVEDATA * sv)
{
	u32 adrs;
	u32 work_size = 0x1000;		//適当
	u8 * buf = GFL_HEAP_AllocMemory(- svsys->heap_temp_id, work_size);
    GFL_UI_SleepDisable(GFL_UI_SLEEP_SVLD);

	//ブロックのフッタ部分だけを先行して削除する
	EraseFooter(sv);

	GFL_STD_MemFill32(buf, 0xffffffff, work_size);
	for (adrs = 0; adrs < sv->savearea_size; adrs += work_size) {
		GFL_FLASH_Save(adrs + sv->start_ofs, buf, work_size);
	}
	GFL_HEAP_FreeMemory(buf);
	GFL_SAVEDATA_Clear(sv);
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
 *
 * ロード処理はフラッシュアクセス時間が十分早いため、分割関数は用意していない
 */
//---------------------------------------------------------------------------
LOAD_RESULT GFL_BACKUP_Load(GFL_SAVEDATA * sv)
{
	SVLD_STATUS result;
	if (!svsys->flash_exists) {
		return LOAD_RESULT_ERROR;
	}

	result = NewSVLD_Load(sv);

	switch ((SVLD_STATUS)result) {
	case SVLD_STAT_OK:
		sv->data_exists = TRUE;			//データは存在する
		return LOAD_RESULT_OK;
	case SVLD_STAT_NG:
		return LOAD_RESULT_BREAK;

	case SVLD_STAT_NULL:
		return LOAD_RESULT_NULL;
	case SVLD_STAT_ERROR:
		return LOAD_RESULT_ERROR;
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
SAVE_RESULT GFL_BACKUP_Save(GFL_SAVEDATA * sv)
{
	SAVE_RESULT result;

	if (!svsys->flash_exists) {
		return SAVE_RESULT_NG;
	}

	SaveAsync_Init(sv);

	do {
		result = SaveAsync_Main(sv);
	}while (result == SAVE_RESULT_CONTINUE || result == SAVE_RESULT_LAST);

	SaveAsync_End(sv, result);

	if (result == SAVE_RESULT_OK) {
		sv->data_exists = TRUE;			//データは存在する
	}
	return result;
}

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブ初期化
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
void GFL_BACKUP_SAVEASYNC_Init(GFL_SAVEDATA * sv)
{
	SaveAsync_Init(sv);
}

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブメイン処理
 * @param	sv			セーブデータ構造へのポインタ
 * @return	SAVE_RESULT
 */
//---------------------------------------------------------------------------
SAVE_RESULT GFL_BACKUP_SAVEASYNC_Main(GFL_SAVEDATA * sv)
{
	SAVE_RESULT result;
	result = SaveAsync_Main(sv);
	if (result != SAVE_RESULT_CONTINUE && result != SAVE_RESULT_LAST) {
		SaveAsync_End(sv, result);
	}
	return result;
}

//---------------------------------------------------------------------------
/**
 * @brief	分割セーブキャンセル処理
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
void GFL_BACKUP_SAVEASYNC_Cancel(GFL_SAVEDATA * sv)
{
	SaveAsync_Cancel(sv);
}

//============================================================================================
//
//
//			セーブデータ整合性チェック
//
//
//============================================================================================
typedef struct {
	SVLD_STATUS IsCorrect;
	u32 g_count;
}CHK_INFO;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void _setDummyInfo(CHK_INFO * chkinfo)
{
	chkinfo->IsCorrect = SVLD_STAT_ERROR;
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
static u16 _calcFooterCrc(const void * start, u32 size)
{
	return MATH_CalcCRC16CCITT(&svsys->crc_table, start, size - sizeof(SAVE_FOOTER));
}
//---------------------------------------------------------------------------
/**
 * @brief	バックアップ領域へのアドレスオフセット取得
 */
//---------------------------------------------------------------------------
static u32 _getBackupOffset(const GFL_SAVEDATA * sv)
{
	return sv->start_ofs;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static SAVE_FOOTER * _getFooterAddress(const GFL_SAVEDATA * sv, const u8 * svwk_adrs)
{
	const u8 * start_adr;

	start_adr = svwk_adrs + sv->svdt_size - sizeof(SAVE_FOOTER);
	return (SAVE_FOOTER *)start_adr;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static SVLD_STATUS _checkFooter(const GFL_SAVEDATA * sv, const u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);

	if (footer->size != sv->svdt_size) {
		//サイズが違う→データの存在チェックができない
		return SVLD_STAT_NULL;
	}
	if (footer->magic_number != sv->magic_number) {
		//マジックナンバーが違う→データの存在チェックができない
		return SVLD_STAT_NULL;
	}
	if (footer->crc != _calcFooterCrc(svwk_adrs, sv->svdt_size)) {
		//ＣＲＣは違う、サイズ・マジックナンバーは一緒→データがおかしい
		return SVLD_STAT_NG;
	}
	return SVLD_STAT_OK;
}

//---------------------------------------------------------------------------
/**
 */
//---------------------------------------------------------------------------
static void _setFooter(const GFL_SAVEDATA * sv, u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);

	footer->g_count = sv->global_counter;
	footer->size = sv->svdt_size;
	footer->magic_number = sv->magic_number;
	footer->crc = _calcFooterCrc(svwk_adrs, sv->svdt_size);

}

#if 0
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

	if (chk1->IsCorrect == SVLD_STAT_OK && chk2->IsCorrect == SVLD_STAT_OK) {
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
	else if (chk1->IsCorrect == SVLD_STAT_OK && chk2->IsCorrect != SVLD_STAT_OK) {
		//片方のみ正常の場合
		*res1 = MIRROR1ST;
		*res2 = MIRRORERROR;
		return 1;
	}
	else if (!chk1->IsCorrect != SVLD_STAT_OK && chk2->IsCorrect == SVLD_STAT_OK) {
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
static void _setCurrentInfo(GFL_SAVEDATA * sv, const CHK_INFO * chkinfo, u32 n_ofs)
{
	sv->global_counter = chkinfo[n_ofs].g_count;
	sv->current_side = n_ofs;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
static void _checkBlockInfo(CHK_INFO * chkinfo, GFL_SAVEDATA * sv, u8 * svwk_adrs)
{
	SAVE_FOOTER * footer = _getFooterAddress(sv, svwk_adrs);
	chkinfo->IsCorrect = _checkFooter(sv, svwk_adrs);
	chkinfo->g_count = footer->g_count;
}

//---------------------------------------------------------------------------
/**
 * @brief	セーブデータのチェック
 * @param	sv			セーブデータ構造へのポインタ
 * @retval	LOAD_RESULT		チェック結果
 */
//---------------------------------------------------------------------------
static LOAD_RESULT NewCheckLoadData(GFL_SAVEDATA * sv)
{
	CHK_INFO chkinfo[2];
	u32 nres;
	u32 newer, older;


	if(GFL_FLASH_Load(sv->start_ofs, sv->svwk, sv->savearea_size)) {
		_checkBlockInfo(&chkinfo[MIRROR1ST], sv, sv->svwk);
	} else {
		_setDummyInfo(&chkinfo[MIRROR1ST]);
	}

	if(GFL_FLASH_Load(sv->start_ofs, sv->svwk, sv->savearea_size)) {
		_checkBlockInfo(&chkinfo[MIRROR2ND], sv, sv->svwk);
	} else {
		_setDummyInfo(&chkinfo[MIRROR2ND]);
	}

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
#endif

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
SVLD_STATUS NewSVLD_Load(GFL_SAVEDATA * sv)
{
	u32 backup_pos;

	backup_pos = _getBackupOffset(sv);
	if (GFL_FLASH_Load(backup_pos, sv->svwk, sv->svdt_size) == FALSE) {
		return SVLD_STAT_ERROR;
	}

	return _checkFooter(sv, sv->svwk);
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
static u16 _saveAsyncStart_Body(GFL_SAVEDATA *sv)
{
	u32 backup_pos;
	u8 * svwk;

	_setFooter(sv, sv->svwk);
	backup_pos = _getBackupOffset(sv);
	svwk = sv->svwk;

	return GFL_FLASH_SAVEASYNC_Init(backup_pos, svwk, sv->svdt_size - LAST_DATA_SIZE);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブセット：フッタ部分途中
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static u16 _saveAsyncStart_Footer(GFL_SAVEDATA *sv)
{
	u32 backup_pos;
	u8 * svwk;

	backup_pos = _getBackupOffset(sv) + sv->svdt_size - LAST_DATA_SIZE;
	svwk = sv->svwk + sv->svdt_size - LAST_DATA_SIZE;

	return GFL_FLASH_SAVEASYNC_Init(backup_pos, svwk, LAST_DATA_SIZE);
}

//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブセット：フッタ部分最後
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static u16 _saveAsyncStart_Footer2(GFL_SAVEDATA *sv)
{
	u32 backup_pos;
	u8 * svwk;

	backup_pos = _getBackupOffset(sv) + sv->svdt_size - LAST_DATA_SIZE + LAST_DATA2_SIZE;
	svwk = sv->svwk + sv->svdt_size - LAST_DATA_SIZE + LAST_DATA2_SIZE;

	return GFL_FLASH_SAVEASYNC_Init(backup_pos, svwk, LAST_DATA2_SIZE);
}



//---------------------------------------------------------------------------
/**
 * @brief	非同期セーブ処理：初期化
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static void SaveAsync_Init(GFL_SAVEDATA * sv)
{
	svsys->save_seq = 0;

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
static SAVE_RESULT SaveAsync_Main(GFL_SAVEDATA * sv)
{
	BOOL result;

	switch (svsys->save_seq) {
	case 0:
		svsys->lock_id = _saveAsyncStart_Body(sv);
		svsys->save_seq ++;
		/* FALL THROUGH */
	case 1:
		//メインデータ部分セーブ
		if (GFL_FLASH_SAVEASYNC_Main(svsys->lock_id, &result) == FALSE) {
			break;
		}
		if (!result) {
			return SAVE_RESULT_NG;
		}
		svsys->save_seq ++;
		/* FALL THROUGH */

	case 2:
		svsys->lock_id = _saveAsyncStart_Footer2(sv);
		svsys->save_seq ++;
		/* FALL THROUGH */

	case 3:
		//フッタ部分セーブ
		if (GFL_FLASH_SAVEASYNC_Main(svsys->lock_id, &result) == FALSE) {
			break;
		}
		if (!result) {
			return SAVE_RESULT_NG;
		}
		svsys->save_seq ++;
		//最後のブロックのセーブの場合、それを外部に知らせるために
		//SAVE_RESULT_CONTINUEでなくSAVE_RESULT_LASTを返す
		return SAVE_RESULT_LAST;

	case 4:
		svsys->lock_id = _saveAsyncStart_Footer(sv);
		svsys->save_seq ++;
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
static void SaveAsync_End(GFL_SAVEDATA * sv, SAVE_RESULT result)
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
static void SaveAsync_Cancel(GFL_SAVEDATA * sv)
{
	sv->global_counter = svsys->counter_backup;
	//非同期処理キャンセルのリクエスト
	GFL_FLASH_SAVEASYNC_Cancel(svsys->lock_id);

    GFL_UI_SleepEnable(GFL_UI_SLEEP_SVLD);
}

//---------------------------------------------------------------------------
/**
 * @brief	指定ブロックのフッタ部分を消去する
 * @param	sv			セーブデータ構造へのポインタ
 */
//---------------------------------------------------------------------------
static BOOL EraseFooter(const GFL_SAVEDATA * sv)
{
	u32 backup_pos;
	SAVE_FOOTER dmy_footer;

	GFL_STD_MemFill(&dmy_footer, 0xff, sizeof(SAVE_FOOTER));
	backup_pos = _getBackupOffset(sv) + sv->svdt_size - LAST_DATA_SIZE;

	return GFL_FLASH_Save(backup_pos, &dmy_footer, LAST_DATA_SIZE);
}


