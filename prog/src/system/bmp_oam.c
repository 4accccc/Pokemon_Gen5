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
///ダミーセルのサイズX(dot単位)
#define DUMMY_CELL_SIZE_X		(32)
///ダミーセルのサイズY(dot単位)
#define DUMMY_CELL_SIZE_Y		(16)
///ダミーセルの合計データサイズ(byte単位)
#define DUMMY_CELL_SIZE_TOTAL		((DUMMY_CELL_SIZE_X/8) * (DUMMY_CELL_SIZE_Y/8) * 0x20)

///1つのフォントOAMで管理出来る最大アクター数
#define FACT_ONE_ACT_NUM			(8)


///データのマッピングモード毎のアクセスindex
enum{
	DATAID_128K,
	DATAID_64K,
	DATAID_32K,
	
	DATAID_MAX,
	DATAID_NULL = 0xffffffff,	//未使用
};

//==============================================================================
//	構造体定義
//==============================================================================
///BMPOAMアクター構造体
typedef struct _BMPOAM_ACT{
	GFL_BMP_DATA *bmp;
	GFL_CLWK *cap[FACT_ONE_ACT_NUM];
	CLSYS_DRAW_TYPE draw_type;
	u32 cgr_id[FACT_ONE_ACT_NUM];
	u16 setSerface;		///<CLSYS_DEFREND_MAIN,SUB or CLWK_SETSF_NONE
	u8 act_num_x;
	u8 act_num_y;
}BMPOAM_ACT;

///BMPOAM管理構造体
typedef struct _BMPOAM_SYS{
	GFL_CLUNIT *clunit;
	HEAPID heap_id;
	u32 cell_id[DATAID_MAX];			///<共通セルの登録index
}BMPOAM_SYS;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static u32 _FOLocal_CellResourceSet(BMPOAM_SYS_PTR bsp, CLSYS_DRAW_TYPE draw_type, ARCHANDLE *hdl);
static void _FOLocal_CommonResourceFree(BMPOAM_SYS_PTR bsp);


//==============================================================================
//	データ
//==============================================================================
///リソースデータのテーブル　　※DATAID_???と並びを同じにしておくこと！
static const struct{
	u32 ncgr;
	u32 ncer;
	u32 nanr;
}OamResourceTbl[] = {
	{
		NARC_bmpoam_bmpoam_dummy32x16_NCGR,
		NARC_bmpoam_bmpoam_dummy32x16_NCER,
		NARC_bmpoam_bmpoam_dummy32x16_NANR,
	},
	{
		NARC_bmpoam_bmpoam_dummy32x16_64k_NCGR,
		NARC_bmpoam_bmpoam_dummy32x16_64k_NCER,
		NARC_bmpoam_bmpoam_dummy32x16_64k_NANR,
	},
	{
		NARC_bmpoam_bmpoam_dummy32x16_32k_NCGR,
		NARC_bmpoam_bmpoam_dummy32x16_32k_NCER,
		NARC_bmpoam_bmpoam_dummy32x16_32k_NANR,
	},
};

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
	int i;
	
	bsp = GFL_HEAP_AllocClearMemory(heap_id, sizeof(BMPOAM_SYS));
	bsp->heap_id = heap_id;
	bsp->clunit = p_unit;
	for(i = 0; i < DATAID_MAX; i++){
		bsp->cell_id[i] = DATAID_NULL;
	}
	
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
	u8 num_x, num_y, x, y;
	ARCHANDLE *hdl;
	u32 data_index;
	GFL_CLWK_DATA clwkdata = {
		0, 0,		//pos_x, pos_y
		0,			//anmseq
		0, 0,		//softpri, bgpri
	};
	
	bact = GFL_HEAP_AllocClearMemory(bsp->heap_id, sizeof(BMPOAM_ACT));
	bact->bmp = head->bmp;
	
	num_x = GFL_BMP_GetSizeX(head->bmp) / DUMMY_CELL_SIZE_X;
	num_y = GFL_BMP_GetSizeY(head->bmp) / DUMMY_CELL_SIZE_Y;
	if(GFL_BMP_GetSizeX(head->bmp) % DUMMY_CELL_SIZE_X != 0){
		num_x++;
	}
	if(GFL_BMP_GetSizeY(head->bmp) % DUMMY_CELL_SIZE_Y != 0){
		num_y++;
	}
	//このASSERTに引っかかったらFACT_ONE_ACT_NUMの数字を大きくして管理出来る
	//アクター数を増やす必要がある
	GF_ASSERT(num_y*num_x <= FACT_ONE_ACT_NUM);

	bact->act_num_x = num_x;
	bact->act_num_y = num_y;
	bact->setSerface = head->setSerface;
	bact->draw_type = head->draw_type;
	
	//キャラ登録
	hdl = GFL_ARC_OpenDataHandle(ARCID_BMPOAM, GetHeapLowID(bsp->heap_id));
	for(y = 0; y < num_y; y++){
		for(x = 0; x < num_x; x++){
			//共通セルリソース登録
			data_index = _FOLocal_CellResourceSet(bsp, head->draw_type, hdl);
			//キャラ登録(領域確保)
			bact->cgr_id[x + y*num_x] = GFL_CLGRP_CGR_Register(hdl, 
				OamResourceTbl[data_index].ncgr, FALSE, head->draw_type, bsp->heap_id);
			//アクター生成
			clwkdata.pos_x = head->x + x * DUMMY_CELL_SIZE_X;
			clwkdata.pos_y = head->y + y * DUMMY_CELL_SIZE_Y;
			clwkdata.softpri = head->soft_pri;
			clwkdata.bgpri = head->bg_pri;
			bact->cap[x + y*num_x] = GFL_CLACT_WK_Create(bsp->clunit, 
				bact->cgr_id[x + y*num_x], head->pltt_index, bsp->cell_id[data_index], &clwkdata, 
				head->setSerface, bsp->heap_id);
			GFL_CLACT_WK_SetPlttOffs(bact->cap[x], head->pal_offset, CLWK_PLTTOFFS_MODE_OAM_COLOR);
		}
	}
	GFL_ARC_CloseDataHandle(hdl);
	
	return bact;
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
	
	for(y = 0; y < bact->act_num_y; y++){
		for(x = 0; x < bact->act_num_x; x++){
			GFL_CLGRP_CGR_Release(bact->cgr_id[x + y*bact->act_num_x]);
			GFL_CLACT_WK_Remove(bact->cap[x + y*bact->act_num_x]);
		}
	}
	GFL_HEAP_FreeMemory(bact);
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターの表示・非表示フラグ設定
 *
 * @param   bact		BMPOAMアクターへのポインタ
 * @param   on_off		TRUE:表示　FALSE:非表示
 */
//--------------------------------------------------------------
void BmpOam_ActorSetDrawEnable(BMPOAM_ACT_PTR bact, BOOL on_off)
{
	u8 x, y;
	
	for(y = 0; y < bact->act_num_y; y++){
		for(x = 0; x < bact->act_num_x; x++){
			GFL_CLACT_WK_SetDrawEnable(bact->cap[x + y*bact->act_num_x], on_off);
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターの表示・非表示フラグ取得
 *
 * @param   bact		BMPOAMアクターへのポインタ
 * 
 * @retval   TRUE:表示　FALSE:非表示
 */
//--------------------------------------------------------------
BOOL BmpOam_ActorGetDrawEnable(BMPOAM_ACT_PTR bact)
{
	return GFL_CLACT_WK_GetDrawEnable( bact->cap[0] );
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
	int x, y, bmp_char_x, bmp_char_y;
	u32 src_u, src_d, trans_size, last_trans_size;
	
	bmp_adrs = GFL_BMP_GetCharacterAdrs(bact->bmp);
	total_size = GFL_BMP_GetBmpDataSize(bact->bmp);
	
	DC_FlushRange(bmp_adrs, total_size);
	if(bact->draw_type == CLSYS_DRAW_MAIN){
		load_func = GX_LoadOBJ;
	}
	else{
		load_func = GXS_LoadOBJ;
	}

	bmp_char_x = GFL_BMP_GetSizeX(bact->bmp) / 8;
	bmp_char_y = GFL_BMP_GetSizeY(bact->bmp) / 8;
	last_trans_size = (GFL_BMP_GetSizeX(bact->bmp) % DUMMY_CELL_SIZE_X) / 8 * 0x20;
	if(last_trans_size == 0){
		last_trans_size = DUMMY_CELL_SIZE_X / 8 * 0x20;
	}
	
	src_u = 0;
	src_d = bmp_char_x * 0x20;
	
	for(y = 0; y < bact->act_num_y; y++){
		for(x = 0; x < bact->act_num_x; x++){
			if(x == bact->act_num_x-1){
				trans_size = last_trans_size;
			}
			else{
				trans_size = DUMMY_CELL_SIZE_X / 8 * 0x20;
			}
			
			//上段
			dest_adrs = GFL_CLGRP_CGR_GetAddr(
				bact->cgr_id[x + y*bact->act_num_x], bact->draw_type);
			load_func(&bmp_adrs[src_u], dest_adrs, trans_size);
			
			//下段
			if(y == bact->act_num_y-1 && (bmp_char_y & 1)){
				;	//BMP_Yが奇数キャラの場合は最後の下段は無し
			}
			else{
				dest_adrs += DUMMY_CELL_SIZE_X / 8 * 0x20;
				load_func(&bmp_adrs[src_d], dest_adrs, trans_size);
			}

			src_u += trans_size;
			src_d += trans_size;
		}
		src_u += bmp_char_x * 0x20;
		src_d += bmp_char_x * 0x20;
	}
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターの座標取得(取得座標は左上)
 *
 * @param   bact		
 * @param   x			X座標代入先
 * @param   y			Y座標代入先
 */
//--------------------------------------------------------------
void BmpOam_ActorGetPos(BMPOAM_ACT_PTR bact, s16 *x, s16 *y)
{
	GFL_CLACTPOS pos;
	
	GFL_CLACT_WK_GetPos(bact->cap[0], &pos, bact->setSerface);
	*x = pos.x;
	*y = pos.y;
}

//--------------------------------------------------------------
/**
 * @brief   BMPOAMアクターの座標をセット(セット座標は左上)
 *
 * @param   bact		BMPOAMアクターへのポインタ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
void BmpOam_ActorSetPos(BMPOAM_ACT_PTR bact, s16 x, s16 y)
{
	int num_y, num_x;
	GFL_CLACTPOS pos;
	
	for(num_y = 0; num_y < bact->act_num_y; num_y++){
		for(num_x = 0; num_x < bact->act_num_x; num_x++){
			pos.x = x + num_x * DUMMY_CELL_SIZE_X;
			pos.y = y + num_y * DUMMY_CELL_SIZE_Y;
			GFL_CLACT_WK_SetPos(bact->cap[num_x + num_y*bact->act_num_x], &pos, bact->setSerface);
		}
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief  BMPOAMアクターのOBJモードをセット
 *
 *	@param	BMPOAM_ACT_PTR bact BMPOAMアクターへのポインタ
 *	@param	mode                モード
 */
//-----------------------------------------------------------------------------
void BmpOam_ActorSetObjMode( BMPOAM_ACT_PTR bact, GXOamMode mode )
{ 
	int num_y, num_x;
	for(num_y = 0; num_y < bact->act_num_y; num_y++){
		for(num_x = 0; num_x < bact->act_num_x; num_x++){
      GFL_CLACT_WK_SetObjMode( bact->cap[num_x + num_y*bact->act_num_x], mode );
		}
	}
}
//----------------------------------------------------------------------------
/**
 *	@brief  BMPOAMアクターのOBJプライオリティをセット
 *
 *	@param	BMPOAM_ACT_PTR bact
 *	@param	soft_pri 
 */
//-----------------------------------------------------------------------------
void BmpOam_ActorSetSoftPriprity( BMPOAM_ACT_PTR bact, u8 soft_pri )
{ 
	int num_y, num_x;
	for(num_y = 0; num_y < bact->act_num_y; num_y++){
		for(num_x = 0; num_x < bact->act_num_x; num_x++){
      GFL_CLACT_WK_SetSoftPri( bact->cap[num_x + num_y*bact->act_num_x], soft_pri );
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
 * @param   draw_type
 * @retval	データIndex
 *
 * 既に同じマッピングモードのセルが登録されている場合は、そのIndexを返します
 */
//--------------------------------------------------------------
static u32 _FOLocal_CellResourceSet(BMPOAM_SYS_PTR bsp, CLSYS_DRAW_TYPE draw_type, ARCHANDLE *hdl)
{
	GXOBJVRamModeChar obj_vram_mode;
	int index;
	
	//マッピングモードを取得し、登録済みかチェック
	if(draw_type == CLSYS_DRAW_MAIN){
		obj_vram_mode = GX_GetOBJVRamModeChar();
	}
	else{
		obj_vram_mode = GXS_GetOBJVRamModeChar();
	}
	switch(obj_vram_mode){
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		index = DATAID_128K;
		break;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		index = DATAID_64K;
		break;
	case GX_OBJVRAMMODE_CHAR_1D_32K:
		index = DATAID_32K;
		break;
	default:
		GF_ASSERT(0);	//対応していないマッピングモードです
		index = DATAID_128K;
		break;
	}
	if(bsp->cell_id[index] != DATAID_NULL){
		return index;		//このマッピングモードのセルは既に登録済み
	}

	//セル＆セルアニメ
	bsp->cell_id[index] = GFL_CLGRP_CELLANIM_Register(hdl, 
		OamResourceTbl[index].ncer, OamResourceTbl[index].nanr, bsp->heap_id);

	return index;
}

//--------------------------------------------------------------
/**
 * @brief   共通リソース解放
 * @param   bsp		フォントOAMシステムへのポインタ
 */
//--------------------------------------------------------------
static void _FOLocal_CommonResourceFree(BMPOAM_SYS_PTR bsp)
{
	int i;
	
	for(i = 0; i < DATAID_MAX; i++){
		if(bsp->cell_id[i] != DATAID_NULL){
			GFL_CLGRP_CELLANIM_Release(bsp->cell_id[i]);
			bsp->cell_id[i] = DATAID_NULL;
		}
	}
}
