//==============================================================================
/**
 * @file	gra_tool.c
 * @brief	グラフィックツール
 * @author	goto
 * @date	2005.11.16
 *
 * ここに色々な解説等を書いてもよい
 *
 */
//==============================================================================

#include <gflib.h>
#include "net_app/union/union_gra_tool.h"
#include "poke_tool/monsno_def.h"
#include "poke_tool/poke_tool.h"
#include "arc_def.h"

//	GSのsystem\softsprite.cから移植
void			SoftSpriteChrMask(u8* src);
void			SoftSpriteChrMask_DP(u8 *src);
void			SoftSpriteChrMask_ArcID(u8 *src, int arc_id);
// =============================================================================
//
//
//	■画像変換
//
//
// =============================================================================
// 1Charを2D→1D変換
static void CharCopy1_1D_from_2D(const u8* src, u8* dst, int* ofs, int* bofs, int cs, int len);

//--------------------------------------------------------------
/**
 * @brief	2D→1Dに1キャラ変換
 *
 * @param	src			元データ
 * @param	dst			展開先
 * @param	ofs			元データのコピー開始位置
 * @param	bofs		展開先バッファの書き込み位置
 * @param	cs			コピーサイズ
 * @param	len			2Dデータの幅
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
static void CharCopy1_1D_from_2D(const u8* src, u8* dst, int* ofs, int* bofs, int cs, int len)
{
	int i;
	u32 dat;
	
	for (i = 0; i < 8; i++){
		GFL_STD_MemCopy(&src[ *ofs ], &dst[ *bofs ],  cs);
		*ofs  += len;
		*bofs += cs;
	}
}

//--------------------------------------------------------------
/**
 * @brief	2D→1Dに変換する
 *
 * @param	arc_id				アーカイブインデックス
 * @param	index_no			ファイルインデックス
 * @param	heap_id				ヒープID
 * @param	x					変換対象矩形:x
 * @param	y					変換対象矩形:y
 * @param	w					変換対象矩形:w
 * @param	h					変換対象矩形:h
 * @param	buff				展開先バッファ
 *
 * @retval	none	
 *
 */
//--------------------------------------------------------------
void ChangesInto_1D_from_2D(int arc_id, int index_no, int heap_id, int x, int y, int w, int h, void* buff)
{
	void* aw = NULL;
	u8* p_data = NULL;
	u8* p_buff = NULL;
	NNSG2dCharacterData* ch_data = NULL;

	int size;
	int cs;
	int ofs;
	int bofs;
	int len;
	BOOL res;
	
	///< データの展開
	aw = GFL_ARC_LoadDataAlloc(arc_id, index_no, heap_id);
	
	GF_ASSERT(aw != NULL);
	
	///< キャラデータの取得
	res = NNS_G2dGetUnpackedCharacterData(aw, &ch_data);
	
	GF_ASSERT(res != FALSE);

	GF_ASSERT(ch_data->W >= (x + w));		///< サイズチェック
	GF_ASSERT(ch_data->H >= (y + h));
	
	///< 準備
	cs     = (sizeof(u8) * 4);				///< コピーサイズ 8dot = 4Byte
	p_data = ch_data->pRawData;				///< キャラデータ
	
	SoftSpriteChrMask_ArcID(p_data, arc_id);				///< 復号
	
	p_buff = (u8*)buff;						///< 展開バッファ
	
	len = (ch_data->W * cs);				///< 長さ(元画像の幅)
	ofs = (x * cs) + (y * len);				///< 開始位置
	bofs = 0;								///< バッファの書き込み位置
	
	//全体ｺﾋﾟｰ
	{
		int i;
		int j;

		for (j = y; j < (y + h); j++){
			for (i = x; i < (x + w); i++){
				ofs = (i * cs) + (j * len * CHAR_DOT_Y);			///< 開始位置
				CharCopy1_1D_from_2D(p_data, p_buff, &ofs, &bofs, cs, len);	///< 1キャラコピー
			}
		}
	}
	
	GFL_HEAP_FreeMemory(aw);
}

//	GSのgflib\calctool.cから移植
//============================================================================================
/**
 *	乱数暗号キー生成ルーチン
 *
 * @param[in,out]	code	暗号キー格納ワークのポインタ
 *
 * @return	暗号キー格納ワークの上位2バイトを暗号キーとして返す
 */
//============================================================================================
static	u16 CodeRand_UniGraTool(u32 *code)
{
    code[0] = code[0] *1103515245L + 24691;
    return (u16)(code[0] / 65536L) ;
}

//	GSのsystem\softsprite.cから移植
//============================================================================================
/**
 *	ポケモングラフィックにかけられたマスクを解除
 *
 * @param[in/out]	src		ポケモングラフィックが展開された領域のポインタ
 */
//============================================================================================
void	SoftSpriteChrMask(u8 *src)
{

	int	i;
	u16	*buf;
	u32	code;

	buf=(u16 *)src;

	code=buf[0];

	//プラチナは前方マスク
	for(i=0;i<(20*10*0x20)/2;i++){
		buf[i]^=code;
		CodeRand_UniGraTool(&code);
	}

//DPでは、後方マスク
#if 0
	code=buf[(20*10*0x20)/2-1];

	for(i=(20*10*0x20)/2-1;i>-1;i--){
		buf[i]^=code;
		CodeRand_UniGraTool(&code);
	}
#endif
}

//============================================================================================
/**
 *	ポケモングラフィックにかけられたマスクを解除(DPグラフィック用)
 *
 * @param[in/out]	src		ポケモングラフィックが展開された領域のポインタ
 */
//============================================================================================
void	SoftSpriteChrMask_DP(u8 *src)
{
	int	i;
	u16	*buf;
	u32	code;

	buf=(u16 *)src;

	code=buf[(20*10*0x20)/2-1];

	for(i=(20*10*0x20)/2-1;i>-1;i--){
		buf[i]^=code;
		CodeRand_UniGraTool(&code);
	}
}

//--------------------------------------------------------------
/**
 * @brief   ポケモングラフィックにかけられたマスクを解除
 * 			アーカイブIDによってプラチナ用、DP用の復号処理に分岐
 *
 * @param   src			ポケモングラフィックが展開された領域のポインタ
 * @param   arc_id		アーカイブID
 */
//--------------------------------------------------------------
void	SoftSpriteChrMask_ArcID(u8 *src, int arc_id)
{
#if 1	//PLATINUM_MERGE_UNFIX
//	if(arc_id == ARC_DP_POKE_GRA || arc_id == ARC_DP_OTHER_POKE || arc_id == ARC_TRF_GRA){
	if( arc_id == ARCID_TRF_GRA){
		SoftSpriteChrMask_DP(src);
	}
	else{
		SoftSpriteChrMask(src);
	}
#else
	//金銀ではまだDpの復号処理を呼んでおく	08.07.07
	SoftSpriteChrMask_DP(src);
#endif
}

