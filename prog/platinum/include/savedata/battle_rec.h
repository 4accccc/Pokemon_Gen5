//==============================================================================
/**
 * @file	battle_rec.h
 * @brief	戦闘録画データ用ヘッダ
 * @author	matsuda
 * @date	2007.03.06(火)
 */
//==============================================================================
#ifndef	__BATTLE_REC_H__
#define	__BATTLE_REC_H__


#ifndef	__ASM_NO_DEF_	// ↓これ以降は、アセンブラでは無視 +++++++++++++++

#include "common.h"
#include "savedata/savedata_def.h"
#include "poketool/poke_tool.h"

#include "battle\battle_common.h"
#include "savedata/gds_profile.h"	//GDS_PROFILE_PTR定義

#include "application/battle_recorder/gds_battle_rec.h"

#endif	//__ASM_NO_DEF_		+++++++++++++++++++++++++++++++++++++++++++++++



#define	REC_BUFFER_SIZE	(0x0400)	///<対戦録画バッファサイズ
#define	REC_BUFFER_END	(0xff)		///<対戦録画バッファの終端定義

//ロードデータ用インデックス
#define LOADDATA_MYREC		(0)
#define LOADDATA_DOWNLOAD0	(1)
#define LOADDATA_DOWNLOAD1	(2)
#define LOADDATA_DOWNLOAD2	(3)

///録画データロード時の結果値
#define RECLOAD_RESULT_NULL		(0)	///<データなし
#define RECLOAD_RESULT_OK		(1)	///<正常読み込み
#define RECLOAD_RESULT_NG		(2)	///<データ不正
#define RECLOAD_RESULT_ERROR	(3)	///<ロードで読み出し失敗(セーブデータが破壊されている場合など)


//--------------------------------------------------------------
//	戦闘モード(施設一覧)
//--------------------------------------------------------------
///バトルタワー / シングル
#define RECMODE_TOWER_SINGLE		(GT_BATTLE_MODE_TOWER_SINGLE)
///バトルタワー / ダブル
#define RECMODE_TOWER_DOUBLE		(GT_BATTLE_MODE_TOWER_DOUBLE)
///バトルタワー / WIFIダウンロード
#define RECMODE_TOWER_WIFI_DL		(GT_BATTLE_MODE_TOWER_SINGLE)

///バトルファクトリー / シングル
#define RECMODE_FACTORY_SINGLE		(GT_BATTLE_MODE_FACTORY50_SINGLE)
///バトルファクトリー / ダブル
#define RECMODE_FACTORY_DOUBLE		(GT_BATTLE_MODE_FACTORY50_DOUBLE)

///バトルファクトリー / シングル100
#define RECMODE_FACTORY_SINGLE100	(GT_BATTLE_MODE_FACTORY100_SINGLE)
///バトルファクトリー / ダブル100
#define RECMODE_FACTORY_DOUBLE100	(GT_BATTLE_MODE_FACTORY100_DOUBLE)

///バトルステージ / シングル
#define RECMODE_STAGE_SINGLE		(GT_BATTLE_MODE_STAGE_SINGLE)
///バトルステージ / ダブル
#define RECMODE_STAGE_DOUBLE		(GT_BATTLE_MODE_STAGE_DOUBLE)

///バトルキャッスル / シングル
#define RECMODE_CASTLE_SINGLE		(GT_BATTLE_MODE_CASTLE_SINGLE)
///バトルキャッスル / ダブル
#define RECMODE_CASTLE_DOUBLE		(GT_BATTLE_MODE_CASTLE_DOUBLE)

///バトルルーレット / シングル
#define RECMODE_ROULETTE_SINGLE		(GT_BATTLE_MODE_ROULETTE_SINGLE)
///バトルルーレット / ダブル
#define RECMODE_ROULETTE_DOUBLE		(GT_BATTLE_MODE_ROULETTE_DOUBLE)

///コロシアム(通信対戦) / シングル
#define RECMODE_COLOSSEUM_SINGLE	(GT_BATTLE_MODE_COLOSSEUM_SINGLE)
///コロシアム(通信対戦) / ダブル
#define RECMODE_COLOSSEUM_DOUBLE	(GT_BATTLE_MODE_COLOSSEUM_DOUBLE)
///コロシアム(通信対戦) / ミックス
#define RECMODE_COLOSSEUM_MIX		(GT_BATTLE_MODE_COLOSSEUM_SINGLE)

//以下、マルチモード
#define RECMODE_TOWER_MULTI 		(GT_BATTLE_MODE_TOWER_MULTI)	///<バトルタワー / マルチ
#define RECMODE_FACTORY_MULTI		(GT_BATTLE_MODE_FACTORY50_MULTI)	///<バトルファクトリー / マルチ
#define RECMODE_FACTORY_MULTI100	(GT_BATTLE_MODE_FACTORY100_MULTI)///<バトルファクトリー / マルチ100
#define RECMODE_STAGE_MULTI			(GT_BATTLE_MODE_STAGE_MULTI)	///<バトルステージ / マルチ
#define RECMODE_CASTLE_MULTI		(GT_BATTLE_MODE_CASTLE_MULTI)	///<バトルキャッスル / マルチ
#define RECMODE_ROULETTE_MULTI		(GT_BATTLE_MODE_ROULETTE_MULTI)	///<バトルルーレット / マルチ
#define RECMODE_COLOSSEUM_MULTI		(GT_BATTLE_MODE_COLOSSEUM_MULTI)///<コロシアム(通信対戦) / マルチ

//終端
#define RECMODE_MAX						(22)


//--------------------------------------------------------------
//	録画ヘッダのパラメータ取得要INDEX
//--------------------------------------------------------------
#define RECHEAD_IDX_MONSNO			(0)		///<ポケモン番号
#define RECHEAD_IDX_FORM_NO			(1)		///<フォルム番号
#define RECHEAD_IDX_COUNTER			(2)		///<何戦目か
#define RECHEAD_IDX_MODE			(3)		///<録画施設
#define RECHEAD_IDX_DATA_NUMBER		(4)		///<データナンバー



#ifndef	__ASM_NO_DEF_	// ↓これ以降は、アセンブラでは無視 +++++++++++++++

//--------------------------------------------------------------
/**
 *	戦闘録画セーブデータの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_SAVEDATA		BATTLE_REC_SAVEDATA;

//--------------------------------------------------------------
/**
 *	戦闘録画本体の不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_WORK * 	BATTLE_REC_WORK_PTR;

//--------------------------------------------------------------
/**
 *	戦闘録画ヘッダの不完全型構造体宣言
 */
//--------------------------------------------------------------
typedef struct _BATTLE_REC_HEADER * 	BATTLE_REC_HEADER_PTR;

//--------------------------------------------------------------
/**
 *	戦闘録画のミラーリング参照先制御構造体
 */
//--------------------------------------------------------------
typedef struct{
	u32 rec_key;				///<認証キー
	u8 rec_flag:2;				///<参照先フラグ
	u8 			:6;		//余り
	
	u8 padding[3];		//パディング
}BATTLE_REC_SAVE_KEY;

//==============================================================================
//	
//==============================================================================
extern int BattleRec_GetWorkSize( void );
extern void BattleRec_WorkInit(BATTLE_REC_SAVEDATA *rec);

//------------------------------------------------------------------
/**
 * データ初期化
 *
 * @param   heapID	メモリ確保するためのヒープID		
 *
 */
//------------------------------------------------------------------
extern	void BattleRec_Init(SAVEDATA *sv,int heapID,LOAD_RESULT *result);

//------------------------------------------------------------------
/**
 * 対戦録画データの破棄
 */
//------------------------------------------------------------------
extern	void BattleRec_Exit(void);

//------------------------------------------------------------------
/**
 * 対戦録画データのメモリを確保しているかチェック
 *
 * @retval	TRUE:存在する　FALSE:存在しない
 */
//------------------------------------------------------------------
extern	BOOL BattleRec_DataExistCheck(void);

//--------------------------------------------------------------
/**
 * @brief   ワークのアドレスを取得
 *
 * @retval	brsに格納されているワークアドレス		
 */
//--------------------------------------------------------------
extern	BATTLE_REC_SAVEDATA *BattleRec_WorkAdrsGet( void );
extern void * BattleRec_RecWorkAdrsGet( void );

//------------------------------------------------------------------
/**
 * 対戦録画データのロード
 *
 * @param	sv		セーブデータ構造体へのポインタ
 * @param	heapID	データをロードするメモリを確保するためのヒープID
 * @param	result	ロード結果を格納するワーク
 * @param	bp		ロードしたデータから生成するBATTLE_PARAM構造体へのポインタ
 * @param	num		ロードするデータナンバー（LOADDATA_MYREC、LOADDATA_DOWNLOAD0、LOADDATA_DOWNLOAD1…）
 *
 * @retval	TRUE:正常読み込み。　FALSE:データ不正
 */
//------------------------------------------------------------------
BOOL BattleRec_Load(SAVEDATA *sv,int heapID,LOAD_RESULT *result,BATTLE_PARAM *bp,int num);

//------------------------------------------------------------------
/**
 * 対戦録画データのセーブ
 *
 * @param	sv		セーブデータ構造体へのポインタ
 *
 * @retval	セーブ結果
 */
//------------------------------------------------------------------
extern SAVE_RESULT BattleRec_Save(SAVEDATA *sv, int rec_mode, int fight_count, int num);

//--------------------------------------------------------------
/**
 * @brief   指定位置の録画データを削除(初期化)してセーブ実行
 *
 * @param   sv			
 * @param   heap_id		
 * @param   num			
 *
 * @retval  
 *
 *
 */
//--------------------------------------------------------------
extern SAVE_RESULT BattleRec_SaveDataErase(SAVEDATA *sv, int heap_id, int num);

//============================================================================================
/**
 *	戦闘録画処理
 *
 * @param[in]	client_no	録画を行うClientNo
 * @param[in]	pos			録画データの格納場所
 * @param[in]	data		録画データ
 */
//============================================================================================
extern	void BattleRec_DataRec(int client_no,int pos,REC_DATA data);

//============================================================================================
/**
 *	戦闘再生データ取得処理
 *
 * @param[in]	rp			戦闘システムワークの構造体ポインタ
 * @param[in]	client_no	録画を行うClientNo
 * @param[in]	pos			録画データの格納場所
 */
//============================================================================================
extern	REC_DATA BattleRec_DataPlay(int client_no,int pos);

//============================================================================================
/**
 *	BATTLE_PARAM構造体保存処理
 *
 * @param[in]	rec			録画再生データ保存ワークの構造体ポインタ
 * @param[in]	bp			BATTLE_PARAM構造体へのポインタ
 */
//============================================================================================
extern	void BattleRec_BattleParamRec(BATTLE_PARAM *bp);

//============================================================================================
/**
 *	BATTLE_PARAM構造体生成処理
 *
 * @param[in]	rec			録画再生データ保存ワークの構造体ポインタ
 * @param[in]	bp			BATTLE_PARAM構造体へのポインタ
 */
//============================================================================================
extern	void BattleRec_BattleParamCreate(BATTLE_PARAM *bp,SAVEDATA *sv);

//extern DENDOU_SAVEDATA* SaveData_GetDendouData( SAVEDATA* sv );
//録画データは通常セーブデータでないため、アクセス時には下記関数を使用してフラッシュから読み書きする
extern BATTLE_REC_SAVEDATA * SaveData_Extra_LoadBattleRecData(SAVEDATA * sv, int heap_id, 
	LOAD_RESULT * result, int num);
extern SAVE_RESULT SaveData_Extra_SaveBattleRecData(
	SAVEDATA * sv, BATTLE_REC_SAVEDATA * data, int num);

extern BATTLE_REC_HEADER_PTR BattleRec_Header_AllocMemory(int heap_id);
extern void BattleRec_Header_FreeMemory(BATTLE_REC_HEADER_PTR brhp);

extern SAVE_RESULT BattleRec_GDS_RecvData_Save(SAVEDATA *sv, int num);
extern void BattleRec_GDS_SendData_Conv(SAVEDATA *sv);
extern SAVE_RESULT BattleRec_GDS_MySendData_DataNumberSetSave(SAVEDATA *sv, u64 data_number);

//==============================================================================
//	データ取得
//==============================================================================
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderAllocCopy(int heap_id);
extern GDS_PROFILE_PTR BattleRec_GDSProfileAllocCopy(int heap_id);
extern GDS_PROFILE_PTR BattleRec_GDSProfilePtrGet(void);
extern BATTLE_REC_WORK_PTR BattleRec_WorkPtrGet(void);
extern BATTLE_REC_HEADER_PTR BattleRec_HeaderPtrGet(void);
extern u64 RecHeader_ParamGet(BATTLE_REC_HEADER_PTR head, int index, int param);
extern void BattleRec_DataSet(GDS_PROFILE_PTR gpp, BATTLE_REC_HEADER_PTR head, 
	BATTLE_REC_WORK_PTR rec, BATTLE_PARAM *bp, SAVEDATA *sv);


#endif	//__ASM_NO_DEF_

#endif	/* __BATTLE_REC_H__ */
