//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		townmap_grh.c
 *	@brief	タウンマップ用基本グラフィック
 *	@author	Toru=Nagihashi
 *	@data		2009.07.17
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]
//lib
#include <gflib.h>
#include "print/gf_font.h"
#include "system/gfl_use.h"

//	archive
#include "arc_def.h"
#include "townmap_gra.naix"
#include "font/font.naix"

//mine
#include "townmap_grh.h"

//=============================================================================
/**
 *					定数宣言
*/
//=============================================================================
//-------------------------------------
///	リソース
//=====================================
enum 
{	
	OBJ_RES_CHR	= 0,
	OBJ_RES_CEL,
	OBJ_RES_PLT,

	OBJ_RES_MAX
};

//=============================================================================
/**
 *					構造体宣言
*/
//=============================================================================
//-------------------------------------
///	BGワーク
//=====================================
typedef struct 
{
	int dummy;
} GRAPHIC_BG_WORK;
//-------------------------------------
///	OBJワーク
//=====================================
typedef struct 
{
	GFL_CLUNIT			*p_clunit[TOWNMAP_OBJ_CLUNIT_MAX];
	GFL_CLWK				*p_clwk[TOWNMAP_OBJ_CLWK_MAX];
	GFL_CLSYS_REND	*p_rend;
	u32							res[OBJ_RES_MAX];
} GRAPHIC_OBJ_WORK;
//-------------------------------------
///	タウンマップ用基本グラフィックワーク
//=====================================
struct _TOWNMAP_GRAPHIC_SYS
{	
	GRAPHIC_BG_WORK		bg;
	GRAPHIC_OBJ_WORK	obj;
	GFL_TCB	*p_vblank_task;
};

//=============================================================================
/**
 *					プロトタイプ宣言
*/
//=============================================================================
//-------------------------------------
///	GRAPHIC
//=====================================
static void TownMap_Graphic_Init( void );
static void TownMap_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs );
//-------------------------------------
///	BG
//=====================================
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID );
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk );
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk );
//-------------------------------------
///	OBJ
//=====================================
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID );
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk );
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk );
static GFL_CLWK		* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLWK id );
static GFL_CLUNIT * GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLUNIT id );
static GFL_CLSYS_REND* GRAPHIC_OBJ_GetRend( const GRAPHIC_OBJ_WORK *cp_wk );
static u32 GRAPHIC_OBJ_GetResource( const GRAPHIC_OBJ_WORK *cp_wk, u32 resID );
//=============================================================================
/**
 *					DATA
 */
//=============================================================================
//-------------------------------------
///	VRAM設定
//=====================================
static const GFL_DISP_VRAM sc_vramSetTable =
{
	GX_VRAM_BG_256_AD,					// メイン2DエンジンのBG
	GX_VRAM_BGEXTPLTT_0123_E,     // メイン2DエンジンのBG拡張パレット
	GX_VRAM_SUB_BG_128_C,				// サブ2DエンジンのBG
	GX_VRAM_SUB_BGEXTPLTT_NONE, // サブ2DエンジンのBG拡張パレット
	GX_VRAM_OBJ_128_B,					// メイン2DエンジンのOBJ
	GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
	GX_VRAM_SUB_OBJ_16_I,       // サブ2DエンジンのOBJ
	GX_VRAM_SUB_OBJEXTPLTT_NONE,// サブ2DエンジンのOBJ拡張パレット
	GX_VRAM_TEX_NONE,						// テクスチャイメージスロット
	GX_VRAM_TEXPLTT_NONE,				// テクスチャパレットスロット
	GX_OBJVRAMMODE_CHAR_1D_128K,		
	GX_OBJVRAMMODE_CHAR_1D_128K,		
};


//-------------------------------------
///	BG面設定
//=====================================
static const struct 
{
	u32									frame;
	GFL_BG_BGCNT_HEADER	bgcnt_header;
	u32									mode;
}	sc_bgsetup[TOWNMAP_BG_FRAME_MAX]	=
{	
	//BAR_M
	{	
		GFL_BG_FRAME0_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_BAR_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	//FONT_M
	{	
		GFL_BG_FRAME1_M,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x08000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_FONT_M, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	//ROAD_M
	{	
		GFL_BG_FRAME2_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_ROAD_M, 0, 0, FALSE
		},
		GFL_BG_MODE_256X16
	},
	//MAP_M
	{	
		GFL_BG_FRAME3_M,
		{
			0, 0, 0x1000, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_256,
			GX_BG_SCRBASE_0x2000, GX_BG_CHARBASE_0x18000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_MAP_M, 0, 0, FALSE	//slot3=0x6000
		},
		GFL_BG_MODE_256X16
	},

	//DEBUG_S
	{	
		GFL_BG_FRAME2_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x1000, GX_BG_CHARBASE_0x10000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	//FONT_S
	{	
		GFL_BG_FRAME0_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_FONT_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
	//WND_S
	{	
		GFL_BG_FRAME1_S,
		{
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x0800, GX_BG_CHARBASE_0x0c000, GFL_BG_CHRSIZ_256x256,
			GX_BG_EXTPLTT_01, TOWNMAP_BG_PRIORITY_WND_S, 0, 0, FALSE
		},
		GFL_BG_MODE_TEXT
	},
};
//フレーム取得用マクロ
#define GRAPHIC_BG_GetFrame( x )	(sc_bgsetup[ x ].frame)

//=============================================================================
/**
 *					GLOBAL
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	基本グラフィック作成
 *
 *	@param	HEAPID heapID		ヒープID
 *
 *	@return	ワーク
 */
//-----------------------------------------------------------------------------
TOWNMAP_GRAPHIC_SYS * TOWNMAP_GRAPHIC_Init( HEAPID heapID )
{	
	TOWNMAP_GRAPHIC_SYS *p_wk;

	//ワーク作成
	p_wk	= GFL_HEAP_AllocMemory( heapID, sizeof(TOWNMAP_GRAPHIC_SYS) );
	GFL_STD_MemClear( p_wk, sizeof(TOWNMAP_GRAPHIC_SYS) );

	//基本グラフィック設定
	TownMap_Graphic_Init();

	//モジュール作成
	GRAPHIC_BG_Init( &p_wk->bg, heapID );
	GRAPHIC_OBJ_Init( &p_wk->obj, &sc_vramSetTable, heapID );


	//VBlankTask登録
	p_wk->p_vblank_task	= GFUser_VIntr_CreateTCB(TownMap_Graphic_VBlankTask, p_wk, 0 );

	return p_wk;
}
//----------------------------------------------------------------------------
/**
 *	@brief	基本グラフィック破棄
 *
 *	@param	TOWNMAP_GRAPHIC_SYS *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
void TOWNMAP_GRAPHIC_Exit( TOWNMAP_GRAPHIC_SYS *p_wk )
{	
	//VBLANKTask消去
	GFL_TCB_DeleteTask( p_wk->p_vblank_task );

	//モジュール破棄
	GRAPHIC_OBJ_Exit( &p_wk->obj );
	GRAPHIC_BG_Exit( &p_wk->bg );

	//ワーク破棄
	GFL_HEAP_FreeMemory( p_wk );
}
//----------------------------------------------------------------------------
/**
 *	@brief	基本グラフィック描画
 *
 *	@param	TOWNMAP_GRAPHIC_SYS *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
void TOWNMAP_GRAPHIC_Draw( TOWNMAP_GRAPHIC_SYS *p_wk )
{
	GRAPHIC_OBJ_Main( &p_wk->obj );
}

//----------------------------------------------------------------------------
/**
 *	@brief	BG面取得
 *
 *	@param	const TOWNMAP_GRAPHIC_SYS *cp_wk	ワーク
 *	@param	frame		BGインデックス
 *
 *	@return	BG面
 */
//-----------------------------------------------------------------------------
u8 TOWNMAP_GRAPHIC_GetFrame( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_BG_FRAME frame )
{	
	return GRAPHIC_BG_GetFrame( frame );
}
//----------------------------------------------------------------------------
/**
 *	@brief	UNIT取得
 *
 *	@param	const TOWNMAP_GRAPHIC_SYS *cp_wk	ワーク
 *	@param	id	CLUNIT
 *
 *	@return	UNIT
 */
//-----------------------------------------------------------------------------
GFL_CLUNIT *TOWNMAP_GRAPHIC_GetUnit( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLUNIT id )
{	
	return GRAPHIC_OBJ_GetUnit( &cp_wk->obj, id );
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLWKを取得
 *
 *	@param	const TOWNMAP_GRAPHIC_SYS *cp_wk	ワーク
 *	@param	id																CLWKID
 *
 *	@return	GFL_CLWK
 */
//-----------------------------------------------------------------------------
GFL_CLWK	*TOWNMAP_GRAPHIC_GetClwk( const TOWNMAP_GRAPHIC_SYS *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLWK id )
{	
	return GRAPHIC_OBJ_GetClwk( &cp_wk->obj, id );
}
#if 0
//----------------------------------------------------------------------------
/**
 *	@brief	レンダラー取得
 *
 *	@param		ワーク
 *
 *	@return	レンダラー
 */
//-----------------------------------------------------------------------------
GFL_CLSYS_REND	*TOWNMAP_GRAPHIC_GetRend( const TOWNMAP_GRAPHIC_SYS *cp_wk )
{	
	return GRAPHIC_OBJ_GetRend( &cp_wk->obj );
}
#endif
//----------------------------------------------------------------------------
/**
 *	@brief	Resource取得
 *
 *	@param	const TOWNMAP_GRAPHIC_SYS *cp_wk	ワーク
 *	@param	*p_chr		キャラリソース
 *	@param	*p_cel		セルリソース
 *	@param	*p_plt		パレットリソース
 *
 */
//-----------------------------------------------------------------------------
void TOWNMAP_GRAPHIC_GetObjResource( const TOWNMAP_GRAPHIC_SYS *cp_wk, u32 *p_chr, u32 *p_cel, u32 *p_plt )
{	
	*p_chr	= GRAPHIC_OBJ_GetResource( &cp_wk->obj, OBJ_RES_CHR );
	*p_cel	= GRAPHIC_OBJ_GetResource( &cp_wk->obj, OBJ_RES_CEL );
	*p_plt	= GRAPHIC_OBJ_GetResource( &cp_wk->obj, OBJ_RES_PLT );
}
//=============================================================================
/**
 *						GRAPHIC
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィック初期化
 *
 */
//-----------------------------------------------------------------------------
static void TownMap_Graphic_Init( void )
{	
	//レジスタ初期化
	G2_BlendNone();
	G2S_BlendNone();	
	GX_SetVisibleWnd( GX_WNDMASK_NONE );
	GXS_SetVisibleWnd( GX_WNDMASK_NONE );

	//VRAMクリアー
	GFL_DISP_ClearVRAM( 0 );

	//VRAMバンク設定
	GFL_DISP_SetBank( &sc_vramSetTable );

	// ディスプレイON
	GFL_DISP_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );
	GFL_DISP_SetDispOn();

	//	表示
	GFL_DISP_GX_InitVisibleControl();

	//	フォント初期化
	GFL_FONTSYS_Init();
}
//----------------------------------------------------------------------------
/**
 *	@brief	グラフィックVBLANK関数
 *
 *	@param	p_tcb			タスクコントロールブロック
 *	@param	p_wk_adrs	ワークアドレス
 *
 */
//-----------------------------------------------------------------------------
static void TownMap_Graphic_VBlankTask( GFL_TCB *p_tcb, void *p_wk_adrs )
{	
	TOWNMAP_GRAPHIC_SYS *p_wk	= p_wk_adrs;
	GRAPHIC_OBJ_VBlankFunction( &p_wk->obj );
	GRAPHIC_BG_VBlankFunction( &p_wk->bg );
}
//=============================================================================
/**
 *						BG
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	BG	初期化
 *
 *	@param	GRAPHIC_BG_WORK *p_wk	ワーク
 *	@param	heapID								ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Init( GRAPHIC_BG_WORK *p_wk, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );

	//初期化
	GFL_BG_Init( heapID );
	GFL_BMPWIN_Init( heapID );

	//グラフィックモード設定
	{	
		static const GFL_BG_SYS_HEADER sc_bgsys_header	=
		{	
			GX_DISPMODE_GRAPHICS,GX_BGMODE_5,GX_BGMODE_0,GX_BG0_AS_2D
		};
		GFL_BG_SetBGMode( &sc_bgsys_header );
	}

	//BG面設定
	{	
		int i;
		for( i = 0; i < TOWNMAP_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_SetBGControl( sc_bgsetup[i].frame, &sc_bgsetup[i].bgcnt_header, sc_bgsetup[i].mode );
			GFL_BG_SetVisible( sc_bgsetup[i].frame, VISIBLE_ON );
		}
	}

	//読み込み
	{	
		ARCHANDLE	*p_handle;
		p_handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

		//下画面(メイン)
		//2面で使用拡張パレットスロットが違うので2回転送している
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_townmap_gra_tmap_bg_d_NCLR,
				PALTYPE_MAIN_BG_EX, 0x6000, 32*16*16, heapID );
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_townmap_gra_tmap_bg_d_NCLR,
				PALTYPE_MAIN_BG_EX, 0x4000, 32*16*16, heapID );

		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_townmap_gra_tmap_bg_d_NCGR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_ROAD_M), 0, 32*32*32*2, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_townmap_gra_tmap_map_d_NCGR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_MAP_M), 0, 32*32*32*2, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_townmap_gra_tmap_root_d_NSCR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_ROAD_M), 0, 0x800, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_townmap_gra_tmap_map_d_NSCR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_MAP_M), 0, 0x800, FALSE, heapID );

		//上画面（サブ）
		GFL_ARCHDL_UTIL_TransVramPalette( p_handle, NARC_townmap_gra_tmap_bg_u_NCLR,
				PALTYPE_SUB_BG, TOWNMAP_BG_PAL_S_00*0x20, 0, heapID );
		GFL_ARCHDL_UTIL_TransVramBgCharacter( p_handle, NARC_townmap_gra_tmap_bg_u_NCGR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_WND_S), 0, 0, FALSE, heapID );
		GFL_ARCHDL_UTIL_TransVramScreen( p_handle, NARC_townmap_gra_tmap_bg_u_NSCR,
				GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_WND_S), 0, 0, FALSE, heapID );

		//フォント用パレット
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, TOWNMAP_BG_PAL_M_14*0x20, 0x20, heapID );
		GFL_ARC_UTIL_TransVramPalette( ARCID_FONT, NARC_font_default_nclr, PALTYPE_SUB_BG, TOWNMAP_BG_PAL_S_14*0x20, 0x20, heapID );

		GFL_BG_FillCharacter( GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_FONT_M), 0x00, 1, 0 );

		GFL_ARC_CloseDataHandle( p_handle );
	}


}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	破棄
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_Exit( GRAPHIC_BG_WORK *p_wk )
{	
	//破棄
	{	
		GFL_BG_FillCharacterRelease( GRAPHIC_BG_GetFrame(TOWNMAP_BG_FRAME_FONT_M), 1,0 );
	}
	
	//BG面破棄
	{	
		int i;
		for( i = 0; i < TOWNMAP_BG_FRAME_MAX; i++ )
		{	
			GFL_BG_ClearFrame( sc_bgsetup[i].frame );
			GFL_BG_FreeBGControl( sc_bgsetup[i].frame );
		}
	}

	//終了
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();

	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_BG_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	BG	VBlank関数
 *
 *	@param	GRAPHIC_BG_WORK *p_wk		ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_BG_VBlankFunction( GRAPHIC_BG_WORK *p_wk )
{	
	GFL_BG_VBlankFunc();
}
//=============================================================================
/**
 *				OBJ
 */
//=============================================================================
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	初期化
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk			ワーク
 *	@param	GFL_DISP_VRAM* cp_vram_bank	バンクテーブル
 *	@param	heapID											ヒープID
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Init( GRAPHIC_OBJ_WORK *p_wk, const GFL_DISP_VRAM* cp_vram_bank, HEAPID heapID )
{	
	//クリア
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );

	//システム作成
  {
    const GFL_CLSYS_INIT sc_clsys_init = {
      0, 0,
      0, 512,
      4, 124,
      4, 124,
      0,
      32,32,32,32,
      16, 16,
    };
    GFL_CLACT_SYS_Create( &sc_clsys_init, cp_vram_bank, heapID );
  }

	//汎用UNIT
	{	
		p_wk->p_clunit[TOWNMAP_OBJ_CLUNIT_DEFAULT]	= GFL_CLACT_UNIT_Create( 128, 0, heapID );
		GFL_CLACT_UNIT_SetDefaultRend( p_wk->p_clunit[TOWNMAP_OBJ_CLUNIT_DEFAULT] );
	}

	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );


	//リソース読み込み
	{	
		ARCHANDLE	*	p_handle	= GFL_ARC_OpenDataHandle( ARCID_TOWNMAP_GRAPHIC, heapID );

		p_wk->res[OBJ_RES_PLT]	= GFL_CLGRP_PLTT_Register( p_handle,
				NARC_townmap_gra_tmap_obj_NCLR, CLSYS_DRAW_MAIN, TOWNMAP_OBJ_PAL_M_00*0x20, heapID );	

		p_wk->res[OBJ_RES_CHR]	= GFL_CLGRP_CGR_Register( p_handle,
				NARC_townmap_gra_tmap_obj_d_NCGR, FALSE, CLSYS_DRAW_MAIN, heapID );

		p_wk->res[OBJ_RES_CEL]	= GFL_CLGRP_CELLANIM_Register( p_handle,
				NARC_townmap_gra_tmap_obj_d_NCER,
				NARC_townmap_gra_tmap_obj_d_NANR, heapID );

		GFL_ARC_CloseDataHandle( p_handle );
	}

	//CLWK作成
	{	
		int i;
		GFL_CLWK_DATA cldata;
		GFL_STD_MemClear( &cldata, sizeof(GFL_CLWK_DATA) );
		for( i = 0; i < TOWNMAP_OBJ_CLWK_MAX; i++ )
		{	
			p_wk->p_clwk[i]	= GFL_CLACT_WK_Create( p_wk->p_clunit[TOWNMAP_OBJ_CLUNIT_DEFAULT],
					p_wk->res[OBJ_RES_CHR], p_wk->res[OBJ_RES_PLT], p_wk->res[OBJ_RES_CEL], &cldata, CLSYS_DEFREND_SUB, heapID );
		}
	}

	//CLWK設定
	{	
		int i;

		//アニメシーケンスのみ設定（座標は各管理側）
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[TOWNMAP_OBJ_CLWK_WINDOW], 0 );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[TOWNMAP_OBJ_CLWK_CURSOR], 4 );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[TOWNMAP_OBJ_CLWK_RING_CUR], 5 );
		GFL_CLACT_WK_SetAnmSeq( p_wk->p_clwk[TOWNMAP_OBJ_CLWK_HERO], 6 );


		for( i = 0; i < TOWNMAP_OBJ_CLWK_MAX; i++ )
		{	
			GFL_CLACT_WK_SetDrawEnable( p_wk->p_clwk[i], FALSE );
		}
	}

}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	破棄
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Exit( GRAPHIC_OBJ_WORK *p_wk )
{	
	int i;

	//CLWK取得
	{
		for( i = 0; i < TOWNMAP_OBJ_CLWK_MAX; i++ )
		{	
			if( p_wk->p_clwk[i] )
			{	
				GFL_CLACT_WK_Remove( p_wk->p_clwk[i] );
			}
		}
	}

	//リソース破棄
	{	
		GFL_CLGRP_CELLANIM_Release( p_wk->res[OBJ_RES_CEL] );
		GFL_CLGRP_CGR_Release( p_wk->res[OBJ_RES_CHR] );
		GFL_CLGRP_PLTT_Release( p_wk->res[OBJ_RES_PLT] );	
	}


	//システム破棄
	{
		for( i = 0; i < TOWNMAP_OBJ_CLUNIT_MAX; i++ )
		{	
			if( p_wk->p_clunit[i] )
			{	
				GFL_CLACT_UNIT_Delete( p_wk->p_clunit[i] );
			}
		}
	}

	GFL_CLACT_SYS_Delete();
	GFL_STD_MemClear( p_wk, sizeof(GRAPHIC_OBJ_WORK) );
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	メイン処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_Main( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_Main();
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJ描画	Vブランク処理
 *
 *	@param	GRAPHIC_OBJ_WORK *p_wk	ワーク
 *
 */
//-----------------------------------------------------------------------------
static void GRAPHIC_OBJ_VBlankFunction( GRAPHIC_OBJ_WORK *p_wk )
{	
	GFL_CLACT_SYS_VBlankFunc();
}

//----------------------------------------------------------------------------
/**
 *	@brief	CLWKの取得
 *
 *	@param	const GRAPHIC_OBJ_WORK *cp_wk	ワーク
 *	@param	id														CLWKID
 *
 *	@return	CLWK
 */
//-----------------------------------------------------------------------------
static GFL_CLWK		* GRAPHIC_OBJ_GetClwk( const GRAPHIC_OBJ_WORK *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLWK id )
{	
	GF_ASSERT( id < TOWNMAP_OBJ_CLWK_MAX );
	return cp_wk->p_clwk[id];
}
//----------------------------------------------------------------------------
/**
 *	@brief	CLユニットの取得
 *
 *	@param	const GRAPHIC__OBJ_WORK *cp_wk	ワーク
 *
 *	@return	GFL_CLUNIT
 */
//-----------------------------------------------------------------------------
static GFL_CLUNIT * GRAPHIC_OBJ_GetUnit( const GRAPHIC_OBJ_WORK *cp_wk, TOWNMAP_GRAPHIC_OBJ_CLUNIT id )
{	
	GF_ASSERT( id < TOWNMAP_OBJ_CLUNIT_MAX );
	return cp_wk->p_clunit[ id ];
}

//----------------------------------------------------------------------------
/**
 *	@brief	レンダラー取得
 *
 *	@param	const GRAPHIC_OBJ_WORK *cp_wk		ワーク
 *
 *	@return	レンダラー
 */
//-----------------------------------------------------------------------------
static GFL_CLSYS_REND* GRAPHIC_OBJ_GetRend( const GRAPHIC_OBJ_WORK *cp_wk )
{	
	return cp_wk->p_rend;
}
//----------------------------------------------------------------------------
/**
 *	@brief	OBJリソース
 *
 *	@param	const TOWNMAP_GRAPHIC_SYS *cp_wk	ワーク
 *	@param	resID		リソースID
 *
 *	@return	リソース
 */
//-----------------------------------------------------------------------------
static u32 GRAPHIC_OBJ_GetResource( const GRAPHIC_OBJ_WORK *cp_wk, u32 resID )
{	
	GF_ASSERT( resID < OBJ_RES_MAX );
	return cp_wk->res[resID];
}
