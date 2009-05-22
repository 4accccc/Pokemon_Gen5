//==============================================================================
/**
 * @file	battle_cursor.c
 * @brief	戦闘下画面用カーソル
 * @author	matsuda changed by soga
 * @date	2009.04.13(月)
 */
//==============================================================================
#include "gflib.h"
#include "system/palanm.h"
#include "battle_tcb_pri.h"
#include "battle_cursor.h"

#include "battle/pl_batt_obj_def.h"
#include "battle/pl_batt_bg_def.h"

#include "arc_def.h"

//==============================================================================
//	定数定義
//==============================================================================
///戦闘カーソルのアクターポインタ配列番号
enum{
	BCURSOR_ACT_LU,		///<左上カーソル
	BCURSOR_ACT_RU,		///<右上カーソル
	BCURSOR_ACT_LD,		///<左下カーソル
	BCURSOR_ACT_RD,		///<右下カーソル
	
	BCURSOR_ACT_EX,		///<特別カーソル
	
	BCURSOR_ACT_NUM,	///<戦闘カーソルのアクター使用数
};

//==============================================================================
//	構造体定義
//==============================================================================
///戦闘カーソルワーク
typedef struct _BCURSOR_WORK{
	GFL_CLUNIT *clunit;
	GFL_CLWK *clwk[ BCURSOR_ACT_NUM ];
}BCURSOR_WORK;

typedef struct _BCURSOR_RES_WORK{
	u32	charID;
	u32	plttID;
	u32	cellID;
}BCURSOR_RES_WORK;

//==============================================================================
//	データ
//==============================================================================
///AA用ゲージアクターヘッダ
static const GFL_CLWK_DATA BCursorObjParam = {
	0, 0,		//x, y
	0, 0, 0,	//アニメ番号、ソフト優先順位、	BG優先順位
};

//==============================================================================
//	プロトタイプ宣言
//==============================================================================


//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルのリソースをロードする
 *
 * @param   cursor		戦闘カーソル管理構造体へのポインタ		
 * @param   heap_id		ヒープID
 */
//--------------------------------------------------------------
BCURSOR_RES_PTR BCURSOR_ResourceLoad( PALETTE_FADE_PTR pfd, HEAPID heap_id )
{
	ARCHANDLE			*hdl;
	BCURSOR_RES_WORK	*bcrw;

	hdl = GFL_ARC_OpenDataHandle( ARCID_BATT_OBJ, heap_id );
	bcrw = GFL_HEAP_AllocMemory( heap_id, sizeof( BCURSOR_RES_WORK ) );

//	bcrw->plttID = GFL_CLGRP_PLTT_RegisterEx( hdl, BATTLE_CURSOR_OAM_SUB_NCLR, CLSYS_DRAW_SUB, 0, 0, 1, heap_id );
	bcrw->plttID = GFL_CLGRP_PLTT_RegisterComp( hdl, BATTLE_CURSOR_OAM_SUB_NCLR, CLSYS_DRAW_SUB, 0, heap_id );
	PaletteWorkSet_VramCopy( pfd, FADE_SUB_OBJ, 0, 0x20 );
	bcrw->charID = GFL_CLGRP_CGR_Register( hdl, BATTLE_CURSOR_OAM_SUB_NCGR_BIN, TRUE, CLSYS_DRAW_SUB, heap_id );
//	bcrw->cellID = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_CURSOR_OAM_SUB_NCER_BIN, BATTLE_CURSOR_OAM_SUB_NANR_BIN, heap_id );
	bcrw->cellID = GFL_CLGRP_CELLANIM_Register( hdl, BATTLE_CURSOR_OAM_SUB_NCER, BATTLE_CURSOR_OAM_SUB_NANR, heap_id );

	GFL_ARC_CloseDataHandle( hdl );

	return bcrw;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルのリソースを解放する
 *
 * @param   crp			
 * @param   char_id		キャラID
 * @param   pal_id		パレットID
 * @param   cell_id		セルID
 * @param   anm_id		アニメID
 */
//--------------------------------------------------------------
void BCURSOR_ResourceFree( BCURSOR_RES_PTR bcrp )
{
	GF_ASSERT( bcrp );
	GFL_CLGRP_CGR_Release( bcrp->charID );
	GFL_CLGRP_PLTT_Release( bcrp->plttID );
	GFL_CLGRP_CELLANIM_Release( bcrp->cellID );
	GFL_HEAP_FreeMemory( bcrp );
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルのアクターを生成します
 *
 * @param   bcrp		リソース管理構造体へのポインタ
 * @param   heap_id		ヒープID
 * @param   soft_pri	ソフトプライオリティ
 * @param   bg_pri		BGプライオリティ
 *
 * @retval  生成された戦闘カーソルワークのポインタ
 *
 * 戦闘カーソルワークの生成とUpdate用TCBの生成も同時に行います
 * 生成時は表示OFFになっています。
 */
//--------------------------------------------------------------
BCURSOR_PTR BCURSOR_ActorCreate( BCURSOR_RES_PTR bcrp, HEAPID heap_id, u32 soft_pri, u32 bg_pri)
{
	BCURSOR_WORK *cursor;
	GFL_CLWK_DATA obj_head;
	int i;
	
	obj_head = BCursorObjParam;
	obj_head.softpri = soft_pri;
	obj_head.bgpri = bg_pri;
	
	cursor = GFL_HEAP_AllocMemory( heap_id, sizeof( BCURSOR_WORK ) );
	MI_CpuClear8( cursor, sizeof( BCURSOR_WORK ) );
	
	cursor->clunit = GFL_CLACT_UNIT_Create( BCURSOR_ACT_NUM, 0, heap_id );
	
	for( i = 0 ; i < BCURSOR_ACT_NUM ; i++ ){
		cursor->clwk[ i ] = GFL_CLACT_WK_Create( cursor->clunit, bcrp->charID, bcrp->plttID, bcrp->cellID, &obj_head, CLSYS_DEFREND_SUB, heap_id );
		GFL_CLACT_WK_SetDrawEnable( cursor->clwk[ i ], FALSE );
		GFL_CLACT_WK_SetAutoAnmFlag( cursor->clwk[ i ], FALSE );
	}
	
	return cursor;
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルアクターを削除します
 *
 * @param   cursor		戦闘カーソルワークへのポインタ
 *
 * 戦闘カーソルワークの解放と、Update用TCBの削除も行います
 */
//--------------------------------------------------------------
void BCURSOR_ActorDelete( BCURSOR_PTR cursor )
{
	int	i;

	for( i = 0 ; i < BCURSOR_ACT_NUM ; i++ ){
		GFL_CLACT_WK_Remove( cursor->clwk[ i ] );
	}
	GFL_CLACT_UNIT_Delete( cursor->clunit );
	
	GFL_HEAP_FreeMemory( cursor );
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルの座標セットと表示のONを行います(全て個別に指定)
 *
 * @param   cursor		戦闘カーソルワークへのポインタ
 * @param   left		左座標
 * @param   right		右座標
 * @param   top			上座標
 * @param   bottom		下座標
 */
//--------------------------------------------------------------
void BCURSOR_IndividualPosSetON(BCURSOR_PTR cursor, int lu_x, int lu_y, int ru_x, int ru_y,
	int ld_x, int ld_y, int rd_x, int rd_y)
{
	int i;
	GFL_CLACTPOS	pos;
	
	pos.x = lu_x;
	pos.y = lu_y;
	GFL_CLACT_WK_SetPos( cursor->clwk[ BCURSOR_ACT_LU ], &pos, CLSYS_DEFREND_SUB );
	pos.x = ru_x;
	pos.y = ru_y;
	GFL_CLACT_WK_SetPos( cursor->clwk[ BCURSOR_ACT_RU ], &pos, CLSYS_DEFREND_SUB );
	pos.x = ld_x;
	pos.y = ld_y;
	GFL_CLACT_WK_SetPos( cursor->clwk[ BCURSOR_ACT_LD ], &pos, CLSYS_DEFREND_SUB );
	pos.x = rd_x;
	pos.y = rd_y;
	GFL_CLACT_WK_SetPos( cursor->clwk[ BCURSOR_ACT_RD ], &pos, CLSYS_DEFREND_SUB );

	GFL_CLACT_WK_SetAnmSeq( cursor->clwk[ BCURSOR_ACT_LU ], BCURSOR_ANMTYPE_LU );
	GFL_CLACT_WK_SetAnmSeq( cursor->clwk[ BCURSOR_ACT_RU ], BCURSOR_ANMTYPE_RU );
	GFL_CLACT_WK_SetAnmSeq( cursor->clwk[ BCURSOR_ACT_LD ], BCURSOR_ANMTYPE_LD );
	GFL_CLACT_WK_SetAnmSeq( cursor->clwk[ BCURSOR_ACT_RD ], BCURSOR_ANMTYPE_RD );

	for(i = 0; i < BCURSOR_ACT_EX; i++){
		GFL_CLACT_WK_SetDrawEnable( cursor->clwk[ i ], TRUE );
		GFL_CLACT_WK_SetAutoAnmFlag( cursor->clwk[ i ], TRUE );
	}
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルの座標セットと表示のONを行います(矩形で指定)
 *
 * @param   cursor		戦闘カーソルワークへのポインタ
 * @param   left		左座標
 * @param   right		右座標
 * @param   top			上座標
 * @param   bottom		下座標
 */
//--------------------------------------------------------------
void BCURSOR_PosSetON(BCURSOR_PTR cursor, int left, int right, int top, int bottom)
{
	BCURSOR_IndividualPosSetON(cursor, left, top, right, top, left, bottom, right, bottom);
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルの特別カーソルの座標セットと表示のONを行います
 *
 * @param   cursor			戦闘カーソルワークへのポインタ
 * @param   x				座標X
 * @param   y				座標Y
 * @param   anm_type		アニメタイプ(BCURSOR_ANMTYPE_???)
 */
//--------------------------------------------------------------
void BCURSOR_ExPosSetON( BCURSOR_PTR cursor, int x, int y, BCURSOR_ANMTYPE anm_type )
{
	GFL_CLACTPOS	pos;
	
	pos.x = x;
	pos.y = y;
	GFL_CLACT_WK_SetPos( cursor->clwk[ BCURSOR_ACT_EX ], &pos, CLSYS_DEFREND_SUB );
	GFL_CLACT_WK_SetAnmSeq( cursor->clwk[ BCURSOR_ACT_EX ], anm_type );
	GFL_CLACT_WK_SetDrawEnable( cursor->clwk[ BCURSOR_ACT_EX ], TRUE );
	GFL_CLACT_WK_SetAutoAnmFlag( cursor->clwk[ BCURSOR_ACT_EX ], TRUE );
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルを全て表示OFFする
 * @param   cursor		戦闘カーソルワークへのポインタ
 */
//--------------------------------------------------------------
void BCURSOR_OFF( BCURSOR_PTR cursor )
{
	int i;
	
	for(i = 0; i < BCURSOR_ACT_NUM; i++){
		GFL_CLACT_WK_SetDrawEnable( cursor->clwk[ i ], FALSE );
	}
}

//--------------------------------------------------------------
/**
 * @brief   戦闘カーソルの特別カーソルのみ表示OFFする
 * @param   cursor		戦闘カーソルワークへのポインタ
 */
//--------------------------------------------------------------
void BCURSOR_ExOFF( BCURSOR_PTR cursor )
{
	GFL_CLACT_WK_SetDrawEnable( cursor->clwk[ BCURSOR_ACT_EX ], FALSE );
}

