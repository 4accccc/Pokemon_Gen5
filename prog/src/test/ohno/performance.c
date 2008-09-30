//===================================================================
/**
 * @file	performance
 * @brief	パフォーマンスメーター　デバッグ用
 * @author	GAME FREAK Inc.
 * @date	08.08.01
 */
//===================================================================

#ifdef PM_DEBUG

#include <nitro.h>
#include <nnsys.h>
#include "gflib.h"
#include "performance.h"


//==============================================================================
//	定数定義
//==============================================================================
///メータータイプ
enum{
	METER_TYPE_START,	///<処理負荷開始メーター
	METER_TYPE_END,		///<処理負荷終了メーター
};

///メーターのアニメパターン数
#define METER_ANM_PATERN		(2)
///メーターのCGX展開位置(キャラクタ番号)
#define METER_CHARNO			(1024 - METER_ANM_PATERN)
///メーターの処理落ち時のCGX展開位置(キャラクタ番号)
#define METER_DELAY_CHARNO		(METER_CHARNO + 1)
///メーターのパレット番号
#define METER_PALNO				(15)


//OBJ16Kモードの時
#define METER_ICON_CHAR_OFFSET16  ( (512-METER_ANM_PATERN)*32 )	
//OBJ32Kモードの時
#define METER_ICON_CHAR_OFFSET32  ( (1024-METER_ANM_PATERN)*32 )	
//OBJ64Kモードの時
#define METER_ICON_CHAR_OFFSET64 ( (1024-METER_ANM_PATERN)*32 )	
//OBJ80Kモードの時
#define METER_ICON_CHAR_OFFSET80 ( (2560-METER_ANM_PATERN)*32 )	
//OBJ128Kモードの時
#define METER_ICON_CHAR_OFFSET128 ( (4096-METER_ANM_PATERN)*32 )	
//OBJ256Kモードの時
#define METER_ICON_CHAR_OFFSET256 ( (8192-METER_ANM_PATERN)*32 )	


//==============================================================================
//	構造体定義
//==============================================================================
///パフォーマンスアプリワーク構造体
typedef struct{
	u32 start_vblank_count;		//計測開始時のVブランクカウンタの値
	s16 start_vcount;			//計測開始時のVカウンタの値
}PFM_APP_WORK;

///パフォーマンスシステム構造体
typedef struct{
	PFM_APP_WORK app[PERFORMANCE_ID_MAX];
	BOOL on_off;
}PERFORMANCE_SYSTEM;


//==============================================================================
//	変数定義
//==============================================================================
///パフォーマンスシステムワーク
static PERFORMANCE_SYSTEM pfm_sys;


//==============================================================================
//	データ
//==============================================================================
///メーター表示座標Y
static const s32 MeterPosY[] = {
	192-16,			//PERFORMANCE_ID_MAIN
	192-8,			//PERFORMANCE_ID_VBLANK
	0,				//PERFORMANCE_ID_USER_A
	8,				//PERFORMANCE_ID_USER_VBLANK_A
	24,				//PERFORMANCE_ID_USER_B
	32,				//PERFORMANCE_ID_USER_VBLANK_B
};
SDK_COMPILER_ASSERT(NELEMS(MeterPosY) == PERFORMANCE_ID_MAX);


//NcgOutCharTextコンバータ Version:1.01
//キャラクタ数 = 0x2(2)
static const u8 performance_meter[] = {
//0x0
	0x0,0x0,0x0,0x44,0x0,0x40,0x44,0x41,
	0x40,0x14,0x11,0x41,0x14,0x11,0x11,0x41,
	0x40,0x14,0x11,0x41,0x0,0x40,0x44,0x41,
	0x0,0x0,0x0,0x44,0x0,0x0,0x0,0x0,
//0x1
	0x0,0x0,0x0,0x44,0x0,0x40,0x44,0x43,
	0x40,0x34,0x33,0x43,0x34,0x33,0x33,0x43,
	0x40,0x34,0x33,0x43,0x0,0x40,0x44,0x43,
	0x0,0x0,0x0,0x44,0x0,0x0,0x0,0x0,
};

//NcgOutCharTextコンバータ Version:1.01
static const u16 performance_meter_ncl[] = {
	0x5c1f,0x0667,0x1e7f,0x187b,0x0000,0x0000,0x0000,0x0000,	//0
	0x0000,0x0000,0x0000,0x5ad6,0x5ad6,0x0000,0x18c6,0x7fff,
};


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static void Performance_Draw(int meter_type, PERFORMANCE_ID id, s32 v_count, BOOL delay);
static void Performance_CGXTrans(void);
static void MeterCGX_OffsetGet(u32 *offset, u32 *anm_offset);



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーターシステムの初期化
 */
//--------------------------------------------------------------
void DEBUG_PerformanceInit(void)
{
	GFL_STD_MemClear(&pfm_sys, sizeof(PERFORMANCE_SYSTEM));
	pfm_sys.on_off = TRUE;	//デフォルトで表示ON
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメータシステムのメイン処理
 *
 * 今の所パフォーマンスメーターのON,OFF切り替えの監視だけ
 */
//--------------------------------------------------------------
void DEBUG_PerformanceMain(void)
{
	int debugButtonTrg = GFL_UI_KEY_GetTrg()&PAD_BUTTON_DEBUG;
	
	if(debugButtonTrg){
		pfm_sys.on_off ^= TRUE;
	}
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーターのスタートライン描画
 *
 * @param   id		パフォーマンスメーターID
 */
//--------------------------------------------------------------
void DEBUG_PerformanceStartLine(PERFORMANCE_ID id)
{
	PFM_APP_WORK *app;
	
	GF_ASSERT(id < PERFORMANCE_ID_MAX);
	
	app = &pfm_sys.app[id];
	app->start_vblank_count = OS_GetVBlankCount();
	
	Performance_Draw(METER_TYPE_START, id, GX_GetVCount(), FALSE);
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーターの終端ライン描画
 *
 * @param   id		パフォーマンスメーターID
 */
//--------------------------------------------------------------
void DEBUG_PerformanceEndLine(PERFORMANCE_ID id)
{
	PFM_APP_WORK *app;
	u32 end_vblank_count;
	
	GF_ASSERT(id < PERFORMANCE_ID_MAX);
	
	app = &pfm_sys.app[id];
	end_vblank_count = OS_GetVBlankCount();
	Performance_Draw(METER_TYPE_END, id, GX_GetVCount(), app->start_vblank_count != end_vblank_count);
}




//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーター描画
 *
 * @param   meter_type			メータータイプ
 * @param   id					パフォーマンスメーターID
 * @param   v_count				Vカウンタ
 * @param   delay				遅延フラグ(TRUE:遅延している)
 */
//--------------------------------------------------------------
static void Performance_Draw(int meter_type, PERFORMANCE_ID id, s32 v_count, BOOL delay)
{
	GXOamAttr *meter_oam;
	GXOamEffect effect_type;
	s32 x, char_no;
	s32 calc_v_count;
	
	meter_oam = &(((GXOamAttr *)HW_OAM)[127 - id*2 - meter_type]);
	
	calc_v_count = v_count;
	if(calc_v_count < 192){	//VBlank期間を画面上左端にするので期間内と期間外の数値を反転する
		calc_v_count += 262-192;
	}
	else{
		calc_v_count -= 192;
	}
	x = 255 * calc_v_count / 262;	//Vカウンタを画面横幅サイズの割合に変換
	if(id & 1){	//VBlank期間の処理負荷を調べるID
		if(v_count < 192){
			delay = TRUE;	//VBlank期間外
		}
	}
	else{	//メインループ内の処理負荷を調べるID
		if(meter_type == METER_TYPE_START){
			if(v_count > 191){
			//	x = 0;	//StartがVBlank期間内の場合は処理負荷の見易さを考慮してX位置は0にする
						//※VBlank割り込みの処理は、完了後、VBlank期間の終了を見ていないので、
						//  処理が終わればすぐにメインループの処理が走る
			}
		}
	}
	
	if(meter_type == METER_TYPE_START){
		effect_type = GX_OAM_EFFECT_NONE;
	}
	else{
		effect_type = GX_OAM_EFFECT_FLIP_H;
		x -= 8;
	}
	
	if(x > 259){	//ちょっと画面端に食い込んでる方が処理負荷が見やすいので。
		x %= 256;
	}
	char_no = (delay == TRUE) ? METER_DELAY_CHARNO : METER_CHARNO;
	switch(GX_GetBankForOBJ()){
	case GX_VRAM_OBJ_16_F:
	case GX_VRAM_OBJ_16_G:
		char_no -= 512;
		break;
	}
	
	//キャラデータ転送
	Performance_CGXTrans();
	
	//OAMセット
	G2_SetOBJAttr(
		meter_oam,
		x,							//x
		MeterPosY[id],				//y
		0,							//priority
		GX_OAM_MODE_NORMAL,			//mode
		0,							//mosaic
		effect_type,				//effect
		GX_OAM_SHAPE_8x8,			//shape
		GX_OAM_COLORMODE_16,		//color
		char_no,					//charName
		METER_PALNO,				//cParam
		0							//rsParam
	);
	
#if 0
	{
		PFM_APP_WORK *app;
		app = &pfm_sys.app[id];
		if(id==0){
			if(meter_type==0){
				OS_TPrintf("開始ライン=%d, v_count=%d, start_v=%d, end_v=%d\n", x, v_count, app->start_vblank_count, OS_GetVBlankCount());
			}
			else{
				OS_TPrintf("終了ライン=%d, v_count=%d, start_v=%d, end_v=%d\n", x, v_count, app->start_vblank_count, OS_GetVBlankCount());
			}
		}
		else if(id==1){
			if(meter_type==0){
				OS_TPrintf("VBlank開始ライン=%d, v_count=%d, start_v=%d, end_v=%d\n", x, v_count, app->start_vblank_count, OS_GetVBlankCount());
			}
			else{
				OS_TPrintf("VBlank終了ライン=%d, v_count=%d, start_v=%d, end_v=%d\n", x, v_count, app->start_vblank_count, OS_GetVBlankCount());
			}
		}
	}
#endif
}

//--------------------------------------------------------------
/**
 * @brief   メーターCGX転送
 */
//--------------------------------------------------------------
static void Performance_CGXTrans(void)
{
	u32 offset, anm_offset;
	
	MeterCGX_OffsetGet(&offset, &anm_offset);
	GFL_STD_MemCopy32(performance_meter, (void*)(HW_OBJ_VRAM + offset), 0x20);
	GFL_STD_MemCopy32(&performance_meter[0x20], (void*)(HW_OBJ_VRAM + offset + anm_offset), 0x20);
	//パレット
	GFL_STD_MemCopy16(&performance_meter_ncl, (void*)(HW_OBJ_PLTT + 0x20*METER_PALNO), 0x20);
}

//--------------------------------------------------------------
/**
 * @brief   メーターCGXの転送位置とアニメ毎のオフセットアドレスを返す
 *
 * @param   offset			メーターCGX転送位置
 * @param   anm_offset		アニメ毎のオフセットCGX転送位置
 */
//--------------------------------------------------------------
static void MeterCGX_OffsetGet(u32 *offset, u32 *anm_offset)
{
#if 0
	int objBank;
	
	objBank = GX_GetBankForOBJ();
	switch( objBank ){
	case GX_VRAM_OBJ_16_F:
	case GX_VRAM_OBJ_16_G:
		*offset = METER_ICON_CHAR_OFFSET16;
		*anm_offset = 0x20;
		break;
	case GX_VRAM_OBJ_32_FG:
		*offset = METER_ICON_CHAR_OFFSET32;
		*anm_offset = 0x20;
		break;
	case GX_VRAM_OBJ_64_E:
		*offset = METER_ICON_CHAR_OFFSET64;
		*anm_offset = 0x40;
		break;
	case GX_VRAM_OBJ_80_EF:
	case GX_VRAM_OBJ_80_EG:
		*offset = METER_ICON_CHAR_OFFSET80;
		*anm_offset = 0x80;
		break;
	case GX_VRAM_OBJ_96_EFG:
	case GX_VRAM_OBJ_128_A:
	case GX_VRAM_OBJ_128_B:
		*offset = METER_ICON_CHAR_OFFSET128;
		*anm_offset = 0x80;
		break;
	case GX_VRAM_OBJ_256_AB:
	default:
		*offset = METER_ICON_CHAR_OFFSET256;
		*anm_offset = 0x100;
		break;
	}
#else
	int objBank, obj_vram_mode;
	
	objBank = GX_GetBankForOBJ();
	switch( objBank ){
	case GX_VRAM_OBJ_16_F:
	case GX_VRAM_OBJ_16_G:
		*offset = 512*0x20;
		break;
	case GX_VRAM_OBJ_32_FG:
		*offset = 1024*0x20;
		break;
	case GX_VRAM_OBJ_64_E:
		*offset = 2048*0x20;
		break;
	case GX_VRAM_OBJ_80_EF:
	case GX_VRAM_OBJ_80_EG:
		*offset = 2560*0x20;
		break;
	case GX_VRAM_OBJ_96_EFG:
		*offset = 3072*0x20;
		break;
	case GX_VRAM_OBJ_128_A:
	case GX_VRAM_OBJ_128_B:
		*offset = 4096*0x20;
		break;
	case GX_VRAM_OBJ_256_AB:
	default:
		*offset = 8192*0x20;
		break;
	}

	obj_vram_mode = GX_GetOBJVRamModeChar();
	switch( obj_vram_mode ){
	case GX_OBJVRAMMODE_CHAR_1D_32K:
	default:
		*anm_offset = 0x20;
		if(*offset > 1024*0x20){
			*offset = 1024*0x20;
		}
		break;
	case GX_OBJVRAMMODE_CHAR_1D_64K:
		*anm_offset = 0x40;
		if(*offset > 2048*0x20){
			*offset = 2048*0x20;
		}
		break;
	case GX_OBJVRAMMODE_CHAR_1D_128K:
		*anm_offset = 0x80;
		if(*offset > 4096*0x20){
			*offset = 4096*0x20;
		}
		break;
	case GX_OBJVRAMMODE_CHAR_1D_256K:
		*anm_offset = 0x100;
		break;
	}
	*offset -= (*anm_offset) * METER_ANM_PATERN;
#endif
}





#if 0
//------------------------------------------------------------------
/**
 * @brief		パフォーマンスメーターのスタート 
 */
//------------------------------------------------------------------

void DEBUG_PerformanceStart(void)
{
    int debugButtonTrg = GFL_UI_KEY_GetTrg()&PAD_BUTTON_DEBUG;
    
    if(debugButtonTrg){
        Onoff = Onoff ? FALSE
            : TRUE;
        if(Onoff){
            G2_SetWnd0InsidePlane(0,FALSE);
            G2_SetWnd1InsidePlane(0,FALSE);
            G2_SetWndOutsidePlane(0xff,FALSE);
            G2_SetWnd0Position(0, 0, 3, 192);
            G2_SetWnd1Position(263/2-1, 178, 263/2+1, 180);
            GX_SetVisibleWnd(3);
        }
    }
    if(Onoff){
        CountKeep = OS_GetVBlankCount();
    }
}

//------------------------------------------------------------------
/**
 * @brief		パフォーマンスメーターの表示
 */
//------------------------------------------------------------------

void DEBUG_PerformanceDisp(void)
{
    if(Onoff){
        u32 vn,cnt;

        cnt = OS_GetVBlankCount() - CountKeep;
        vn = GX_GetVCount() + (cnt * 263);
        vn = vn / 2;
        if(vn > 263){
            vn = 263;
        }
        G2_SetWnd0Position(0, 180, vn, 182);
//        OS_TPrintf("%d %d\n",vn,CountKeep);
    }
}
#endif

#endif //PM_DEBUG
