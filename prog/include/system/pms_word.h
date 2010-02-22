//============================================================================================
/**
 * @file	pms_word.h
 * @bfief	簡易会話用単語データ取り扱い
 * @author	taya
 * @date	06.01.20
 */
//============================================================================================
#ifndef __PMS_WORD_H__
#define __PMS_WORD_H__

#include "system\pms_data.h"

typedef struct _PMSW_MAN	PMSW_MAN;


extern PMSW_MAN* PMSW_MAN_Create(u32 heapID);
extern void PMSW_MAN_Delete( PMSW_MAN* man );
extern void PMSW_MAN_CopyStr( PMSW_MAN* man, PMS_WORD  pms_word, STRBUF* buf );
extern void PMSW_GetStr( PMS_WORD pms_word, STRBUF* dst , const HEAPID heapID);
extern PMS_WORD  PMSW_GetWordNumberByGmmID( u32 gmmID, u32 wordID );


extern int PMSW_GetDupWordCount( PMS_WORD word );
extern PMS_WORD PMSW_GetDupWord( PMS_WORD word, int idx );

extern BOOL GetWordSorceID( PMS_WORD pms_word, u32* fileID, u32* wordID );



//===================================================================================
// セーブデータ処理
//===================================================================================
#include "savedata\save_control.h"

#define PMS_DATA_ENTRY_MAX (20) ///< 20件保存可能
typedef struct _PMSW_SAVEDATA	PMSW_SAVEDATA;

//--------------------------------------------------
/**
 *	各国あいさつことばのID
 *	（gmmと連動しているため順番は固定）
 */
//--------------------------------------------------
typedef enum {
	PMSW_AISATSU_JP,	// 日本語
	PMSW_AISATSU_EN,	// 英語
	PMSW_AISATSU_FR,	// フランス語
	PMSW_AISATSU_IT,	// イタリア語
	PMSW_AISATSU_GE,	// ドイツ語
	PMSW_AISATSU_SP,	// スペイン語
	PMSW_AISATSU_KO,	// 韓国語
	PMSW_AISATSU_NO,	// 不明

	PMSW_AISATSU_HIDE_MAX,
}PMSW_AISATSU_ID;


//--------------------------------------------------
/**
 *	難解ことばの最大数
 */
//--------------------------------------------------
enum {
	PMSW_NANKAI_WORD_MAX = 32,
};


extern PMSW_SAVEDATA* SaveData_GetPMSW( SAVE_CONTROL_WORK* sv );

extern BOOL PMSW_GetNankaiFlag( const PMSW_SAVEDATA* saveData, u32 id );
extern u32 PMSW_SetNewNankaiWord( PMSW_SAVEDATA* saveData );
extern PMS_WORD  PMSW_NankaiWord_to_PMSWORD( u32 id );
extern BOOL PMSW_GetAisatsuFlag( const PMSW_SAVEDATA* saveData, PMSW_AISATSU_ID id );
extern BOOL PMSW_CheckNankaiWordComplete( PMSW_SAVEDATA* saveData );
extern void PMSW_SetAisatsuFlag( PMSW_SAVEDATA* saveData, PMSW_AISATSU_ID id );



// セーブデータ処理システム用
extern u32 PMSW_GetSaveDataSize(void);
extern void PMSW_InitSaveData( void* work );

//-----------------------------------------------------------------------------
/**
 * 登録データ
 */
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
/**
 *	@brief  登録済み簡易会話データを取得
 *
 *	@param	const PMSW_SAVEDATA* saveData セーブデータへのポインタ
 *	@param	id  登録ID
 *
 *	@retval const PMS_DATA 保存されている簡易会話データへのconstポインタ
 */
//-----------------------------------------------------------------------------
extern PMS_DATA* PMSW_GetDataEntry( PMSW_SAVEDATA* saveData, int id );

//-----------------------------------------------------------------------------
/**
 *	@brief  簡易会話データ登録
 *
 *	@param	const PMSW_SAVEDATA* saveData セーブデータへのポインタ
 *	@param	id  登録ID
 *	@param	PMS_DATA* data 簡易会話データ
 *
 *	@retval none
 */
//-----------------------------------------------------------------------------
extern void PMSW_SetDataEntry( PMSW_SAVEDATA* saveData, int id, PMS_DATA* data );

//-----------------------------------------------------------------------------
/**
 *		あいことば　生成用　重複なし単語テーブル
 *		0x5ACbyte使用
 */
//-----------------------------------------------------------------------------
typedef struct _PMSW_AIKOTOBA_TBL	PMSW_AIKOTOBA_TBL;

// PMSW_AIKOTOBATBL_GetWordIdxのエラーコード
#define PMSW_AIKOTOBA_WORD_NONE	( -1 )	

extern PMSW_AIKOTOBA_TBL* PMSW_AIKOTOBATBL_Init( u32 heapID );
extern void PMSW_AIKOTOBATBL_Delete( PMSW_AIKOTOBA_TBL* p_tbl );
extern u32 PMSW_AIKOTOBATBL_GetTblNum( const PMSW_AIKOTOBA_TBL* cp_tbl );
extern PMS_WORD PMSW_AIKOTOBATBL_GetTblData( const PMSW_AIKOTOBA_TBL* cp_tbl, u32 idx );
extern s16 PMSW_AIKOTOBATBL_GetWordIdx( const PMSW_AIKOTOBA_TBL* cp_tbl, PMS_WORD word );



#endif

