//============================================================================================
/**
 * @file	pm_voice.h
 * @brief	ポケモン鳴き声波形再生基本システム	
 * @author	
 * @date	
 */
//============================================================================================
#ifndef __PMVOICE_H__
#define __PMVOICE_H__

//------------------------------------------------------------------
/**
 * @brief	鳴き声オプション
 */
//------------------------------------------------------------------
enum {
	PMVOICE_MODE_NORMAL = 0,
	PMVOICE_MODE_CHORUS = 1,
	PMVOICE_MODE_REVERSE = 2,
};

//------------------------------------------------------------------
/**
 * @brief	カスタマイズコールバック用定義
 */
//------------------------------------------------------------------
// 波形IDX取得用コールバック
typedef void (PMVOICE_CB_GET_WVIDX)(	u32 pokeNo, 		// [in]ポケモンナンバー
										u32 pokeFormNo,		// [in]ポケモンフォームナンバー
										u32* waveIdx );		// [out]波形IDX
// 波形カスタマイズコールバック(TRUE: コールバック内で生成)　
typedef BOOL (PMVOICE_CB_GET_WVDAT)(	u32 pokeNo,			// [in]ポケモンナンバー
										u32 pokeFormNo,		// [in]ポケモンフォームナンバー
										u32 userParam,		// [in]ユーザーパラメーター
										void** wave,		// [out]波形データ
										u32* size,			// [out]波形サイズ
										int* rate,			// [out]波形再生レート
										int* speed );		// [out]波形再生スピード

//============================================================================================
/**
 *
 * @brief	システム（main.c等から呼び出される）
 *
 */
//============================================================================================
extern void	PMVOICE_Init
			( HEAPID heapID,							// 使用heapID 
			  PMVOICE_CB_GET_WVIDX* CallBackGetWaveIdx,	// 波形IDX取得用コールバック
			  PMVOICE_CB_GET_WVDAT* CallBackCustomWave 	// 波形カスタマイズコールバック
			);
extern void	PMVOICE_Reset( void );
extern void	PMVOICE_Main( void );
extern void	PMVOICE_Exit( void );
extern BOOL	PMVOICE_CheckBusy( void );

//============================================================================================
/**
 *
 * @brief	システム設定
 *
 */
//============================================================================================
extern void	PMVOICE_PlayerHeapReserve( int num, HEAPID heapID );// プレーヤー用waveバッファ事前確保
extern void	PMVOICE_PlayerHeapRelease( void );		// プレーヤー用waveバッファ開放

//============================================================================================
/**
 *
 * @brief	鳴き声サウンド関数
 *
 */
//============================================================================================
//------------------------------------------------------------------
/**
 * @brief	鳴き声再生関数(return: voicePlayerIdx)
 */
//------------------------------------------------------------------
extern u32	PMVOICE_Play
			(	u32		pokeNo,			// ポケモンナンバー
				u32		pokeFormNo,		// ポケモンフォームナンバー
				u8		pan,			// 定位(L:0 - 64 - 127:R)
				BOOL	chorus,			// コーラス使用フラグ
				int		chorusVolOfs,	// コーラスボリューム差
				int		chorusSpOfs,	// 再生速度差
				BOOL	reverse,		// 逆再生フラグ
				u32		userParam		// ユーザーパラメーター	
			);		
//------------------------------------------------------------------
/**
 * @brief	鳴き声ステータス変更関数
 */
//------------------------------------------------------------------
extern void	PMVOICE_SetPan( u32 voicePlayerIdx, u8 pan);
extern void	PMVOICE_SetVolume( u32 voicePlayerIdx, int volOfs);
extern void	PMVOICE_SetSpeed( u32 voicePlayerIdx, int spdOfs);
extern void	PMVOICE_SetStatus( u32 voicePlayerIdx, u8 pan, int volOfs, int spdOfs );
//------------------------------------------------------------------
/**
 * @brief	鳴き声強制停止関数
 */
//------------------------------------------------------------------
extern void	PMVOICE_Stop( u32 voicePlayerIdx );			//鳴き声を停止
//------------------------------------------------------------------
/**
 * @brief	鳴き声終了検出関数(TRUE: 再生中)
 */
//------------------------------------------------------------------
extern BOOL	PMVOICE_CheckPlay( u32 voicePlayerIdx );	//鳴き声終了検出(TRUE:再生中)





#endif

