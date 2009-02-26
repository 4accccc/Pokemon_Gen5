//==============================================================================
/**
 * @file	bmp_oam.c
 * @brief	BMPをOAMに描画
 * @author	matsuda
 * @date	2009.02.25(水)
 */
//==============================================================================
#include <gflib.h>
#include "system/bmp_oam.h"
#include "bmpoam.naix"
#include "arc_def.h"


//==============================================================================
//	定数定義
//==============================================================================
///ダミーセルのサイズX(キャラクタ単位)
#define DUMMY_CELL_SIZE_CHAR_X		(32/8)
///ダミーセルのサイズY(キャラクタ単位)
#define DUMMY_CELL_SIZE_CHAR_Y		(16/8)
///ダミーセルの合計データサイズ(byte単位)
#define DUMMY_CELL_SIZE_TOTAL		(DUMMY_CELL_SIZE_CHAR_X * DUMMY_CELL_SIZE_CHAR_Y * 0x20)

///1つのフォントOAMで管理出来る最大アクター数
#define FACT_ONE_ACT_NUM			(4)


//==============================================================================
//	構造体定義
//==============================================================================
///BMPOAMアクター構造体
typedef struct _BMPOAM_ACT{
	GFL_BMP_DATA *bmp;
	GFL_CLWK *cap[FACT_ONE_ACT_NUM];
	CLSYS_DRAW_TYPE draw_type;
	u32 cgr_id[FACT_ONE_ACT_NUM];
	u8 bmp_size_x;
	u8 bmp_size_y;
	u16 padding;
}BMPOAM_ACT;

///BMPOAM管理構造体
typedef struct _BMPOAM_SYS{
	GFL_CLUNIT *clunit;
	HEAPID heap_id;
	u32 cell_id;			///<共通セルの登録index
}BMPOAM_SYS;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void _FOLocal_CommonResourceSet(BMPOAM_SYS_PTR bsp);
static void _FOLocal_CommonResourceFree(BMPOAM_SYS_PTR bsp);


//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   BMPOAMシステム作成
 *
 * @param   heap_id		BMPOAMで常に使用するヒープID
 * @param   p_unit		CLUNITへのポインタ
 *
 * @retval  作成したBMPOAMシステムへのポインタ
 */
//--------------------------------------------------------------
BMPOAM_SYS_PTR BmpOam_Init(HEAPID heap_id, GFL_CLUNIT* p_unit)
{
	BMPOAM_SYS_PTR bsp;
	
	bsp = GFL_HEAP_AllocClearMemory(heap_id, sizeof(BMPOAM_SYS));
	bsp->heap_id = heap_id;
	bsp->clunit = p_unit;
	
	//共通リソース登録
	_FOLocal_CommonResourceSet(bsp);
	
	return bsp;
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMシステム破棄
 *
 * @param   bsp		BMPOAMシステムへのポインタ
 */
//--------------------------------------------------------------
void BmpOam_Exit(BMPOAM_SYS_PTR bsp)
{
	_FOLocal_CommonResourceFree(bsp);
	GFL_HEAP_FreeMemory(bsp);
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクター作成
 *
 * @param   bsp			BMPOAMシステムへのポインタ
 * @param   head		BMPOAMアクターヘッダへのポインタ
 *
 * @retval  作成したBMPOAMアクターへのポインタ
 */
//--------------------------------------------------------------
BMPOAM_ACT_PTR BmpOam_ActorAdd(BMPOAM_SYS_PTR bsp, const BMPOAM_ACT_DATA *head)
{
	BMPOAM_ACT_PTR bact;
	u8 size_x, size_y, x, y;
	ARCHANDLE *hdl;
	GFL_CLWK_DATA clwkdata = {
		0, 0,		//pos_x, pos_y
		0,			//anmseq
		0, 0,		//softpri, bgpri
	};
	
	bact = GFL_HEAP_AllocClearMemory(bsp->heap_id, sizeof(BMPOAM_ACT));
	bact->bmp = head->bmp;
	
	size_x = GFL_BMP_GetSizeX(head->bmp) / DUMMY_CELL_SIZE_CHAR_X;
	size_y = GFL_BMP_GetSizeY(head->bmp) / DUMMY_CELL_SIZE_CHAR_Y;
	if(GFL_BMP_GetSizeX(head->bmp) % DUMMY_CELL_SIZE_CHAR_X != 0){
		size_x++;
	}
	if(GFL_BMP_GetSizeY(head->bmp) % DUMMY_CELL_SIZE_CHAR_Y != 0){
		size_y++;
	}
	//このASSERTに引っかかったらFACT_ONE_ACT_NUMの数字を大きくして管理出来る
	//アクター数を増やす必要がある
	GF_ASSERT(size_y*size_x <= FACT_ONE_ACT_NUM);

	bact->bmp_size_x = size_x;
	bact->bmp_size_y = size_y;
	
	//キャラ登録
	hdl = GFL_ARC_OpenDataHandle(ARCID_BMPOAM, bsp->heap_id);
	for(y = 0; y < size_y; y++){
		for(x = 0; x < size_x; x++){
			//キャラ登録(領域確保)
			bact->cgr_id[x + y*size_x] = GFL_CLGRP_CGR_Register(hdl, 
				NARC_bmpoam_bmpoam_dummy32x16_NCGR, FALSE, head->draw_type, bsp->heap_id);
			//アクター生成
			clwkdata.pos_x = head->x + x * DUMMY_CELL_SIZE_CHAR_X*8;
			clwkdata.pos_y = head->y + y * DUMMY_CELL_SIZE_CHAR_Y*8;
			clwkdata.softpri = head->soft_pri;
			clwkdata.bgpri = head->bg_pri;
			bact->cap[x] = GFL_CLACT_WK_Create(bsp->clunit, 
				bact->cgr_id[x], head->pltt_index, bsp->cell_id, &clwkdata, 
				head->setSerface, bsp->heap_id);
			GFL_CLACT_WK_SetPlttOffs(bact->cap[x], head->pal_offset, CLWK_PLTTOFFS_MODE_OAM_COLOR);
		}
	}
	GFL_ARC_CloseDataHandle(hdl);
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターの削除
 *
 * @param   bact		BMPOAMアクターへのポインタ
 */
//--------------------------------------------------------------
void BmpOam_ActorDel(BMPOAM_ACT_PTR bact)
{
	u8 x, y;
	
	for(y = 0; y < bact->bmp_size_y; y++){
		for(x = 0; x < bact->bmp_size_x; x++){
			GFL_CLGRP_CGR_Release(bact->cgr_id[x + y*bact->bmp_size_x]);
			GFL_CLACT_WK_Remove(bact->cap[x + y*bact->bmp_size_x]);
		}
	}
	GFL_HEAP_FreeMemory(bact);
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターに関連付けされているBMPをVRAM転送
 *
 * @param   bact		BMPOAMアクターへのポインタ
 */
//--------------------------------------------------------------
void BmpOam_ActorBmpTrans(BMPOAM_ACT_PTR bact)
{
	u8 *bmp_adrs;
	u32 total_size;
	void (*load_func)(const void *pSrc, u32 offset, u32 szByte);
	u32 dest_adrs;
	int dest_count = 0;
	int x, y;
	
	bmp_adrs = GFL_BMP_GetCharacterAdrs(bact->bmp);
	total_size = GFL_BMP_GetBmpDataSize(bact->bmp);
	
	DC_FlushRange(bmp_adrs, total_size);
	if(bact->draw_type == CLSYS_DRAW_MAIN){
		load_func = GX_LoadOBJ;
	}
	else{
		load_func = GXS_LoadOBJ;
	}

	for(y = 0; y < bact->bmp_size_y; y++){
		for(x = 0; x < bact->bmp_size_x; x++){
			dest_adrs = GFL_CLGRP_CGR_GetAddr(
				bact->cgr_id[x + y*bact->bmp_size_x], bact->draw_type);
			load_func(&bmp_adrs[dest_count * DUMMY_CELL_SIZE_TOTAL], 
				dest_adrs, DUMMY_CELL_SIZE_TOTAL);
			dest_count++;
		}
	}
}


//==============================================================================
//	ローカル関数
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   共通リソース読み込み
 * @param   bsp		フォントOAMシステムへのポインタ
 */
//--------------------------------------------------------------
static void _FOLocal_CommonResourceSet(BMPOAM_SYS_PTR bsp)
{
	ARCHANDLE *hdl;
	
	hdl = GFL_ARC_OpenDataHandle(ARCID_BMPOAM, bsp->heap_id);
	
	//セル＆セルアニメ
	bsp->cell_id = GFL_CLGRP_CELLANIM_Register(hdl, 
		NARC_bmpoam_bmpoam_dummy32x16_NCER, NARC_bmpoam_bmpoam_dummy32x16_NANR, bsp->heap_id);

	GFL_ARC_CloseDataHandle(hdl);
}

//--------------------------------------------------------------
/**
 * @brief   共通リソース解放
 * @param   bsp		フォントOAMシステムへのポインタ
 */
//--------------------------------------------------------------
static void _FOLocal_CommonResourceFree(BMPOAM_SYS_PTR bsp)
{
	GFL_CLGRP_CELLANIM_Release(bsp->cell_id);
}
