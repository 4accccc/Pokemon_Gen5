#include <gflib.h>
#include "arc_def.h"
#include "net/network_define.h"
#include "savedata/save_tbl.h"

#include "system/main.h"
#include "system/bmp_menu.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "system/wipe.h"
#include "print/global_font.h"
#include "print/wordset.h"

#include "mystery.naix"
#include "message.naix"
#include "font/font.naix"
#include "msg/msg_mystery.h"
#include "msg/msg_wifi_system.h"

#include "savedata/mystery_data.h"
#include "savedata/save_control.h"
#include "savedata/config.h"
#include "poke_tool/poke_tool.h"
#include "poke_tool/pokeparty.h"
#include "test/ariizumi/ari_debug.h"
#include "comm_mystery_func.h"



//--------------------------------------------------------------------------------------------
/**
 * VRAM初期化
 *
 * @param	none
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CommMysteryFunc_VramBankSet(void)
{
	//CLACT の初期化も変えること
	const GFL_DISP_VRAM vramSetTable = {
		GX_VRAM_BG_128_C,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_32_H,			/* サブ2DエンジンのBG */
		GX_VRAM_SUB_BGEXTPLTT_NONE,		/* サブ2DエンジンのBG拡張パレット */
		GX_VRAM_OBJ_64_E,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_128_D,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_0_B,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_01_FG,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,		// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
	};
	GFL_DISP_SetBank( &vramSetTable );
}


//--------------------------------------------------------------------------------------------
/**
 * @brief	BG初期化
 * @param	none
 * @return	none
 */
//--------------------------------------------------------------------------------------------
void CommMysteryFunc_BgInit( GFL_BG_INI * ini )
{
	{	/* BG SYSTEM */
		GFL_BG_SYS_HEADER BGsys_data = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_0, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &BGsys_data );
	}

	{	/* MAIN DISP BG0 */
		GFL_BG_BGCNT_HEADER MBg0_Data = {
			0, 0, 0x0800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe000, GX_BG_CHARBASE_0x00000,0x8000,
			GX_BG_EXTPLTT_01, 1, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME0_M, &MBg0_Data, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME0_M );
	}

	{	/* MAIN DISP BG1 */
		GFL_BG_BGCNT_HEADER MBg1_Data = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xe800, GX_BG_CHARBASE_0x08000,0x4000,
			GX_BG_EXTPLTT_01, 2, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME1_M, &MBg1_Data, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME1_M );
	}

	{	/* MAIN DISP BG2 */
		GFL_BG_BGCNT_HEADER MBg2_Data = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf000, GX_BG_CHARBASE_0x00000,0,
			GX_BG_EXTPLTT_23, 0, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME2_M, &MBg2_Data, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME2_M );
	}

	{	/* MAIN DISP BG3 */
		GFL_BG_BGCNT_HEADER MBg3_Data = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0xf800, GX_BG_CHARBASE_0x0c000,0x4000,
			GX_BG_EXTPLTT_23, 3, 0, 0, FALSE
		};
		GFL_BG_SetBGControl( GFL_BG_FRAME3_M, &MBg3_Data, GFL_BG_MODE_TEXT );
		GFL_BG_ClearScreen( GFL_BG_FRAME3_M );
	}
}


//------------------------------------------------------------------
/**
 * @brief	おくりものをもらえるかチェック
 * @param	NONE
 * @return	NONE
 */
//------------------------------------------------------------------
int CommMysteryFunc_CheckGetGift(SAVE_CONTROL_WORK *sv, DOWNLOAD_GIFT_DATA *gcp)
{
	u32 version;
	MYSTERY_DATA *fdata = SaveControl_DataPtrGet(sv,GMDATA_ID_MYSTERYDATA);

	version = gcp->version;
	
	// ふしぎなおくりもの消去処理
	if(version == 0xFFFFFFFF && gcp->data.gift_type == 0xFFFF){
		MYSTERYDATA_Init(fdata);
    SaveControl_Save(sv);
		OS_ResetSystem(0); 
	}

	// 対象バージョンが０ならば全てのバージョンで受け取れる
	if(version == 0)
		version = ~0;

	// 対象バージョンに含まれているかチェック
	if((version & (1 << PM_VERSION)) == FALSE)
		return COMMMYSTERYFUNC_ERROR_VERSION;

	// 一度しか受け取れないおくりものは、
	if(gcp->data.only_one_flag == TRUE &&
		 // すでに受け取っているか調べる
		 MYSTERYDATA_IsEventRecvFlag(fdata, gcp->data.event_id) == TRUE)
		return COMMMYSTERYFUNC_ERROR_HAVE;

  //カードいっぱい検査
	if( MYSTERYDATA_CheckCardDataSpace(fdata) == FALSE)
	{
    return COMMMYSTERYFUNC_ERROR_FULLCARD;
	}
	return COMMMYSTERYFUNC_ERROR_NONE;
}


/*  */
