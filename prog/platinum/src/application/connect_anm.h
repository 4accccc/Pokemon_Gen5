//==============================================================================
/**
 * @file	connect_anm.h
 * @brief	Wifi接続画面のBGアニメのinline関数
 * @author	matsuda
 * @date	2007.12.26(水)
 *
 * Wifi接続BG画面が各アプリケーション個々で作成されているので、アニメーションさせる為の関数を
 * ここに全てinline関数として用意。(複数のオーバーレイにまたがっている為)
 */
//==============================================================================
#ifndef __CONNECT_ANM_H__
#define __CONNECT_ANM_H__

#include "application/connect_anm_types.h"


//==============================================================================
//
//	inline関数
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   Wifi接続BGアニメ：初期化
 *
 * @param   cbp				Wifi接続BGのパレットアニメ制御構造体へのポインタ
 * @param   p_hdl			BGパレットが入っているアーカイブのハンドル
 * @param   pal_index		BGパレットのIndex
 * @param   heap_id			ヒープID(この場限りのテンポラリで使用)
 *
 * ワークの確保やTCBの生成等は行っていない為、終了関数はありません。
 */
//--------------------------------------------------------------
static inline void ConnectBGPalAnm_Init(CONNECT_BG_PALANM *cbp, ARCHANDLE *p_hdl, int pal_index, int heap_id)
{
	NNSG2dPaletteData *palData;
	void *p_work;
	
	MI_CpuClear8(cbp, sizeof(CONNECT_BG_PALANM));

	p_work = ArcUtil_HDL_PalDataGet(p_hdl, pal_index, &palData, heap_id);
	
	//バッファにパレットデータをコピー
	MI_CpuCopy16(&((u16*)(palData->pRawData))[CBP_PAL_START_NUMBER * 16], 
		cbp->src_color, CBP_PAL_NUM * 0x20);
	MI_CpuCopy16(&((u16*)(palData->pRawData))[CBP_PAL_START_NUMBER * 16], 
		cbp->dest_color, CBP_PAL_NUM * 0x20);
	
	sys_FreeMemoryEz(p_work);
	
	//有効フラグON
	cbp->occ = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   Wifi接続BGアニメ：メイン処理
 *
 * @param   cbp		Wifi接続BGのパレットアニメ制御構造体へのポインタ
 */
//--------------------------------------------------------------
static inline void ConnectBGPalAnm_Main(CONNECT_BG_PALANM *cbp)
{
	int i, s;
	int anm_pos, evy, next_color_pos;
	
	if(cbp->occ == FALSE){
		return;
	}

	anm_pos = cbp->anm_pos;
	cbp->evy += CBP_ADD_EVY;
	
	if(cbp->evy >= (16 << 8)){
		evy = 16;
		cbp->evy = 0;
		if(cbp->evy_dir == 0){
			cbp->anm_pos++;
			if(cbp->anm_pos >= CBP_PAL_NUM){
				cbp->anm_pos = CBP_PAL_NUM - 2;
				cbp->evy_dir ^= 1;
			}
		}
		else{
			cbp->anm_pos--;
			if(cbp->anm_pos < 0){
				cbp->anm_pos = 1;
				cbp->evy_dir ^= 1;
			}
		}
	}
	else{
		evy = cbp->evy >> 8;
	}

	if(cbp->evy_dir == 0){
		next_color_pos = anm_pos + 1;
		if(next_color_pos >= CBP_PAL_NUM){
			next_color_pos = anm_pos - 1;
		}
	}
	else{
		next_color_pos = anm_pos - 1;
		if(next_color_pos < 0){
			next_color_pos = 1;
		}
	}
	
	for(s = CBP_PAL_COLOR_START; s < CBP_PAL_COLOR_START + CBP_PAL_COLOR_NUM; s++){
		SoftFade(&cbp->src_color[anm_pos][s], &cbp->dest_color[0][s], 
			1, evy, cbp->src_color[next_color_pos][s]);
	}
}

//--------------------------------------------------------------
/**
 * @brief   Wifi接続BGアニメ：転送
 *
 * @param   cbp		Wifi接続BGのパレットアニメ制御構造体へのポインタ
 *
 * VBlank中に呼び出すようにしてください
 */
//--------------------------------------------------------------
static inline void ConnectBGPalAnm_VBlank(CONNECT_BG_PALANM *cbp)
{
	if(cbp->occ == FALSE){
		return;
	}
	
	DC_FlushRange(cbp->dest_color, CBP_TRANS_PAL_NUM * 0x20);
	GX_LoadBGPltt((const void *)cbp->dest_color, 
		CBP_PAL_START_NUMBER * 0x20, CBP_TRANS_PAL_NUM * 0x20);
	GXS_LoadBGPltt((const void *)cbp->dest_color, 
		CBP_PAL_START_NUMBER * 0x20, CBP_TRANS_PAL_NUM * 0x20);
}

//--------------------------------------------------------------
/**
 * @brief   Wifi接続BGアニメ：有効、無効フラグをセット
 *
 * @param   cbp		Wifi接続BGのパレットアニメ制御構造体へのポインタ
 * @param   occ		TRUE:有効化。　FALSE：無効化
 */
//--------------------------------------------------------------
static inline void ConnectBGPalAnm_OccSet(CONNECT_BG_PALANM *cbp, BOOL occ)
{
	cbp->occ = occ;
}


#endif	//__CONNECT_ANM_H__
