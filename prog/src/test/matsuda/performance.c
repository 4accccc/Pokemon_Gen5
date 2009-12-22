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
#include "test/performance.h"


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


//--------------------------------------------------------------
//	数値
//--------------------------------------------------------------
///数値の桁数(-%込み)
#define NUM_KETA			(4)
///数値のCGXサイズ(桁数分)
#define NUM_CGX_SIZE		(0x20 * NUM_KETA)
///数値の履歴表示数
#define NUM_HISTORY			(5)
///数値の表示座標X
#define NUM_X				(8)
///数値の表示座標Y
#define NUM_Y				(192-24)
///数値の表示座標X(ピーク値)
#define PEEK_NUM_X			(NUM_X + 40)
///数値CGXのキャラクタ毎の種類
enum{
	NUMBER_0,
	NUMBER_1,
	NUMBER_2,
	NUMBER_3,
	NUMBER_4,
	NUMBER_5,
	NUMBER_6,
	NUMBER_7,
	NUMBER_8,
	NUMBER_9,
	NUMBER_MINUS,
	NUMBER_PAR,
	
	NUMBER_MAX,
};

///ピーク計測時に維持するウェイト
#define PEEK_WAIT		(8)

//==============================================================================
//	構造体定義
//==============================================================================
///パフォーマンスアプリワーク構造体
typedef struct{
	u32 start_vblank_count;		//計測開始時のVブランクカウンタの値
	s16 start_vcount;			//計測開始時のVカウンタの値
	s32 peek_vcount;			//ピーク計測時のVカウンタ
	s32 peek_x;					//ピーク計測時のX座標
	s8 peek_wait;				//ピーク計測時に維持するウェイト
	s8 dummy;
	
	s32 peek_num;				//現在までの最大ピーク値
}PFM_APP_WORK;

///パフォーマンスシステム構造体
typedef struct{
	PFM_APP_WORK app[PERFORMANCE_ID_MAX];
	BOOL on_off;
	s16 num_oam_pos;				//数値表示ポジション
	s16 dummy;
}PERFORMANCE_SYSTEM;


//==============================================================================
//	変数定義
//==============================================================================
///パフォーマンスシステムワーク
static PERFORMANCE_SYSTEM pfm_sys;

///常時フィールドに負荷をかけるための擬似スキャンフラグ
///　※パフォーマンスと全然関係ないけど、このソースは常に常駐にあるので
BOOL DebugScanOnly = FALSE;

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
	0x0,0x40,0x4,0x0,0x0,0x14,0x4,0x0,
	0x40,0x11,0x4,0x0,0x14,0x11,0x4,0x0,
	0x40,0x11,0x4,0x0,0x0,0x14,0x4,0x0,
	0x0,0x40,0x4,0x0,0x0,0x0,0x0,0x0,
//0x1
	0x0,0x40,0x4,0x0,0x0,0x34,0x4,0x0,
	0x40,0x33,0x4,0x0,0x34,0x33,0x4,0x0,
	0x40,0x33,0x4,0x0,0x0,0x34,0x4,0x0,
	0x0,0x40,0x4,0x0,0x0,0x0,0x0,0x0,
};

//NcgOutCharTextコンバータ Version:1.01
static const u16 performance_meter_ncl[] = {
	0x5c1f,0x0667,0x1e7f,0x187b,0x0000,0x0000,0x0000,0x0000,	//0
	0x0000,0x0000,0x0000,0x5ad6,0x5ad6,0x0000,0x18c6,0x7fff,
};

//数字
//NcgOutCharTextコンバータ Version:1.01
//キャラクタ数 = 0xc(12)
static const u8 performance_num[] = {
//0x0
	0x0,0x0,0x0,0x0,0x0,0xee,0xce,0x0,
	0xe0,0xcc,0xee,0xc,0xee,0xc,0xe0,0xce,
	0xee,0xc,0xe0,0xce,0xec,0xce,0xe0,0xcc,
	0xc0,0xee,0xce,0xc,0x0,0xcc,0xcc,0x0,
//0x1
	0x0,0x0,0x0,0x0,0x0,0xe0,0xce,0x0,
	0x0,0xee,0xce,0x0,0x0,0xec,0xce,0x0,
	0x0,0xe0,0xce,0x0,0x0,0xe0,0xce,0x0,
	0xe0,0xee,0xee,0xce,0xc0,0xcc,0xcc,0xcc,
//0x2
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xc,
	0xee,0xcc,0xec,0xce,0xcc,0xc,0xee,0xce,
	0xe0,0xee,0xce,0xcc,0xee,0xce,0xcc,0x0,
	0xee,0xee,0xee,0xce,0xcc,0xcc,0xcc,0xcc,
//0x3
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xce,
	0xc0,0xcc,0xee,0xcc,0x0,0xee,0xce,0xc,
	0x0,0xcc,0xec,0xce,0xee,0x0,0xe0,0xce,
	0xec,0xee,0xee,0xcc,0xc0,0xcc,0xcc,0xc,
//0x4
	0x0,0x0,0x0,0x0,0x0,0xe0,0xee,0xc,
	0x0,0xee,0xee,0xc,0xe0,0xce,0xee,0xc,
	0xee,0xc,0xee,0xc,0xee,0xee,0xee,0xce,
	0xcc,0xcc,0xee,0xcc,0x0,0x0,0xcc,0xc,
//0x5
	0x0,0x0,0x0,0x0,0xee,0xee,0xee,0xc,
	0xee,0xcc,0xcc,0xc,0xee,0xee,0xee,0xc,
	0xcc,0xcc,0xec,0xce,0xee,0x0,0xe0,0xce,
	0xec,0xee,0xee,0xcc,0xc0,0xcc,0xcc,0xc,
//0x6
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xc,
	0xee,0xcc,0xcc,0xc,0xee,0xee,0xee,0x0,
	0xee,0xcc,0xec,0xce,0xee,0xc,0xe0,0xce,
	0xec,0xee,0xee,0xcc,0xc0,0xcc,0xcc,0xc,
//0x7
	0x0,0x0,0x0,0x0,0xee,0xee,0xee,0xce,
	0xee,0xcc,0xec,0xce,0xcc,0xc,0xee,0xcc,
	0x0,0xe0,0xce,0xc,0x0,0xee,0xcc,0x0,
	0x0,0xee,0xc,0x0,0x0,0xcc,0xc,0x0,
//0x8
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xc,
	0xee,0xcc,0xec,0xce,0xec,0xee,0xee,0xcc,
	0xee,0xcc,0xec,0xce,0xee,0xc,0xe0,0xce,
	0xec,0xee,0xee,0xcc,0xc0,0xcc,0xcc,0xc,
//0x9
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xc,
	0xee,0xcc,0xec,0xce,0xee,0xc,0xe0,0xce,
	0xec,0xee,0xee,0xce,0xc0,0xcc,0xec,0xce,
	0xe0,0xee,0xee,0xcc,0xc0,0xcc,0xcc,0xc,
//0xa
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0xe0,0xee,0xee,0xe,
	0xc0,0xcc,0xcc,0xc,0x0,0x0,0x0,0x0,
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
//0xb
	0x0,0x0,0x0,0x0,0x0,0x0,0x0,0xe,
	0xe0,0x0,0xe0,0xc,0xc0,0x0,0xce,0x0,
	0x0,0xe0,0xc,0x0,0x0,0xce,0x0,0xe,
	0xe0,0xc,0x0,0xc,0xc0,0x0,0x0,0x0,
};


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL Performance_Draw(int meter_type, PERFORMANCE_ID id, s32 v_count, u32 end_vblank_count);
static void Performance_CGXTrans(void);
static void MeterCGX_OffsetGet(u32 *offset, u32 *anm_offset);
static void Performance_Num(int meter_type, PERFORMANCE_ID id, s32 v_count, u32 start_v_blank_count, s32 end_v_blank_count);
static void Performance_NumTrans(s32 num, int peek_oam);
static void Performance_AllOffOam(void);



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
#if DEBUG_ONLY_FOR_none
  return;
#endif
  if( OS_GetConsoleType() & OS_CONSOLE_ISDEBUGGER ){
    pfm_sys.on_off = TRUE;	//デフォルトで表示ON
  }
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
	int debugButtonTrg = GFL_UI_KEY_GetTrg() & (PAD_BUTTON_START | PAD_BUTTON_SELECT);
	
#if DEBUG_ONLY_FOR_none
  return;
#endif
	if(debugButtonTrg){
		pfm_sys.on_off ^= TRUE;
	}
	if(GFL_UI_KEY_GetTrg() == (PAD_BUTTON_L | PAD_BUTTON_R)){
		int id;
		GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
		//ピーク値のリセットもかける
		for(id = 0; id < PERFORMANCE_ID_MAX; id++){
			pfm_sys.app[id].peek_num = -1000;
		}
		DEBUG_PerformanceSetActive(TRUE);
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
	
	if(pfm_sys.on_off == FALSE){
		return;
	}
	
	app = &pfm_sys.app[id];
	app->start_vblank_count = OS_GetVBlankCount();
	app->start_vcount = GX_GetVCount();
	
	Performance_Draw(METER_TYPE_START, id, app->start_vcount, app->start_vblank_count);
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
	BOOL peek_update;
	
	GF_ASSERT(id < PERFORMANCE_ID_MAX);

	if(pfm_sys.on_off == FALSE){
		return;
	}
	
	app = &pfm_sys.app[id];
	end_vblank_count = OS_GetVBlankCount();
	peek_update = Performance_Draw(METER_TYPE_END, id, GX_GetVCount(), end_vblank_count);
	
	if(id == PERFORMANCE_NUM_PRINT_ID && peek_update == TRUE){
		Performance_Num(METER_TYPE_END, id, GX_GetVCount(), app->start_vblank_count, end_vblank_count);
	}
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーターのON/OFF切り替え
 *
 * @param   isActive	ON/OFF
 */
//--------------------------------------------------------------
void DEBUG_PerformanceSetActive(BOOL isActive)
{
#if DEBUG_ONLY_FOR_none
  return;
#endif
  if( (OS_GetConsoleType() & OS_CONSOLE_NITRO) && isActive == TRUE ){
    return;
  }
  
	pfm_sys.on_off = isActive;
	if(isActive == FALSE){
		Performance_AllOffOam();
	}
}



//==============================================================================
//
//	
//
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   数値表示
 *
 * @param   meter_type				メータータイプ
 * @param   id						パフォーマンスメーターID
 * @param   v_count					Vカウント
 * @param   start_v_blank_count		開始VBlankカウント
 * @param   end_v_blank_count		終了VBlankカウント
 */
//--------------------------------------------------------------
static void Performance_Num(int meter_type, PERFORMANCE_ID id, s32 v_count, u32 start_v_blank_count, s32 end_v_blank_count)
{
	PFM_APP_WORK *app = &pfm_sys.app[id];
	s32 num, calc_vcount, v_blank_count_offset, end_vcount;
	u32 offset, anm_offset;
	
	if(meter_type != METER_TYPE_END){
		return;
	}
	
	if(app->start_vcount > 191){
		calc_vcount = -(262 - app->start_vcount);
	}
	else{
		calc_vcount = 0;
	}
	if(v_count > 191){
		calc_vcount += 262-v_count;
	}
	else{
		calc_vcount += v_count;
	}
	v_blank_count_offset = end_v_blank_count - start_v_blank_count;
	end_vcount = GX_GetVCount();
	if(id & 1){	//VBlank期間の処理負荷
		GF_ASSERT("まだ作成していない");
	}
	else{	//メインループの処理負荷
		if(start_v_blank_count < end_v_blank_count){
			calc_vcount += v_blank_count_offset * 191;
			if(v_blank_count_offset > 1){
				calc_vcount += (v_blank_count_offset - 1) * (262-192);
			}
			calc_vcount += end_vcount;
		}
		num = calc_vcount * 100 / 191;
	}
	
	//グラフィック転送
	Performance_NumTrans(num, FALSE);
	if(num > app->peek_num){
		app->peek_num = num;
		Performance_NumTrans(num, TRUE);
	}
}

//--------------------------------------------------------------
/**
 * @brief   数値転送
 *
 * @param   num		
 * @param   peek_oam		TRUE:ピーク計測用のOAMを使用。　FALSE:通常の履歴を流していくOAM
 */
//--------------------------------------------------------------
static void Performance_NumTrans(s32 num, int peek_oam)
{
	u32 offset, anm_offset, first_touch = 0;
	u32 vram_adrs;
	s32 k;
	int i, data, pos = 0, calc_num;
	int oam_pos;
	int pos_x;
	
	if(peek_oam == TRUE){
		oam_pos = NUM_HISTORY;
		pos_x = PEEK_NUM_X;
	}
	else{
		oam_pos = pfm_sys.num_oam_pos;
		pos_x = NUM_X;
	}
	
	MeterCGX_OffsetGet(&offset, &anm_offset);
	vram_adrs = HW_OBJ_VRAM + offset - (NUM_CGX_SIZE * (oam_pos + 1));
	
	calc_num = (num < 0) ? (-num) : num;
	k = 10;
	for(i = NUM_KETA-2; i >= 0; i--){
		data = calc_num % k;
		calc_num /= 10;
		if(data >= NUMBER_MAX){
			data = NUMBER_MINUS;	//保険
		}
		GFL_STD_MemCopy32(&performance_num[data*0x20], (void*)(vram_adrs + i*0x20), 0x20);
		
		if(calc_num == 0){
			GFL_STD_MemClear32((void*)vram_adrs, 0x20*i);
		}
	}
	GFL_STD_MemCopy32(&performance_num[NUMBER_PAR*0x20], (void*)(vram_adrs + (NUM_KETA-1)*0x20), 0x20);
	if(num < 0){
		GFL_STD_MemCopy32(&performance_num[NUMBER_MINUS*0x20], (void*)vram_adrs, 0x20);
	}

	//OAMセット
	{
		GXOamAttr *meter_oam = &(((GXOamAttr *)HW_OAM)[127 - PERFORMANCE_ID_MAX*2 - 1 - oam_pos]);
		u32 start_charno = (vram_adrs - HW_OBJ_VRAM) / anm_offset;
		u32 x, y;
		
		G2_SetOBJAttr(
			meter_oam,
			pos_x,						//x
			NUM_Y,						//y
			0,							//priority
			GX_OAM_MODE_NORMAL,			//mode
			0,							//mosaic
			GX_OAM_EFFECT_NONE,			//effect
			GX_OAM_SHAPE_32x8,			//shape
			GX_OAM_COLORMODE_16,		//color
			start_charno,				//charName
			METER_PALNO,				//cParam
			0							//rsParam
		);

		if(peek_oam == FALSE){
			for(i = 0; i < NUM_HISTORY; i++){
				if(i != oam_pos){
					meter_oam = &(((GXOamAttr *)HW_OAM)[127 - PERFORMANCE_ID_MAX*2 - 1 - i]);
					G2_GetOBJPosition(meter_oam, &x, &y);
					G2_SetOBJPosition(meter_oam, x, y-8);
				}
			}
			pfm_sys.num_oam_pos++;
			if(pfm_sys.num_oam_pos >= NUM_HISTORY){
				pfm_sys.num_oam_pos = 0;
			}
		}
	}
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーター描画
 *
 * @param   meter_type			メータータイプ
 * @param   id					パフォーマンスメーターID
 * @param   v_count				Vカウンタ
 * @param   end_vblank_count	End時のVブランクカウンタ
 *
 * @retval  TRUE:ピーク計測更新
 */
//--------------------------------------------------------------
static BOOL Performance_Draw(int meter_type, PERFORMANCE_ID id, s32 v_count, u32 end_vblank_count)
{
	GXOamAttr *meter_oam;
	GXOamEffect effect_type;
	s32 x, char_no;
	s32 calc_v_count;
	s32 check_peek = 0;
	BOOL delay;
	PFM_APP_WORK *app;
	BOOL peek_update = 0;
	
	app = &pfm_sys.app[id];
	delay = app->start_vblank_count != end_vblank_count;
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

	//ピーク更新確認
	if(meter_type == METER_TYPE_END){
		check_peek = (end_vblank_count - app->start_vblank_count) * 1000;
		if(end_vblank_count != app->start_vblank_count && v_count < 192){
			check_peek += v_count + 262;
		}
		else{
			check_peek += v_count;
		}
		
		if(check_peek > app->peek_vcount){
			app->peek_vcount = check_peek;
			app->peek_wait = 0;
			app->peek_x = x;
			peek_update = TRUE;
		}
		else{
			x = app->peek_x;
			app->peek_wait++;
			if(app->peek_wait > PEEK_WAIT){
				app->peek_wait = 0;
				app->peek_vcount = 0;
			}
		}
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

	return peek_update;
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
}

//--------------------------------------------------------------
/**
 * @brief   パフォーマンスメーターで使用しているOAMを全てOFFする
 */
//--------------------------------------------------------------
static void Performance_AllOffOam(void)
{
	int i;
	GXOamAttr *meter_oam = &(((GXOamAttr *)HW_OAM)[127 - PERFORMANCE_ID_MAX*2 - 1 - NUM_HISTORY]);
	
	for(i = 127 - PERFORMANCE_ID_MAX*2 - 1 - NUM_HISTORY; i <= 127; i++){
		G2_SetOBJAttr(
			meter_oam,
			0,						//x
			0,						//y
			0,							//priority
			GX_OAM_MODE_NORMAL,			//mode
			0,							//mosaic
			GX_OAM_EFFECT_NODISPLAY,	//effect
			GX_OAM_SHAPE_32x8,			//shape
			GX_OAM_COLORMODE_16,		//color
			0,				//charName
			0,				//cParam
			0							//rsParam
		);
		meter_oam++;
	}
}

#endif //PM_DEBUG
