//[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[
/**
 *
 *	@file		debug_tomoya.c
 *	@brief		僕の作成したシステムのサンプルソース
 *	@author		tomoya takahashi
 *	@data		2007.01.24
 *
 */
//]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]


#include "gflib.h"
#include "procsys.h"
#include "ui.h"
#include "clact.h"
#include "main.h"		//HEAPID参照のため

#define __DEBUG_TOMOYA_H_GLOBAL
#include "debug_tomoya.h"


//-----------------------------------------------------------------------------
/**
 *					コーディング規約
 *		●関数名
 *				１文字目は大文字それ以降は小文字にする
 *		●変数名
 *				・変数共通
 *						constには c_ を付ける
 *						staticには s_ を付ける
 *						ポインタには p_ を付ける
 *						全て合わさると csp_ となる
 *				・グローバル変数
 *						１文字目は大文字
 *				・関数内変数
 *						小文字と”＿”と数字を使用する 関数の引数もこれと同じ
*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
/**
 *					定数宣言
*/
//-----------------------------------------------------------------------------
// セルアクターサンプル
#define CLACT_WKNUM	(8)		// CLACT_UNIT内に作成するワークの数

//-----------------------------------------------------------------------------
/**
 *					構造体宣言
*/
//-----------------------------------------------------------------------------
//-------------------------------------
///	セルアクターサンプルプロセスワーク
//=====================================
typedef struct {
	void* p_cellbuff;
	void* p_cellanmbuff;
	NNSG2dCellDataBank*		p_cell;
	NNSG2dCellAnimBankData*	p_cellanm;
	NNSG2dImageProxy		imageproxy;
	NNSG2dImagePaletteProxy	plttproxy;
} DEBUG_CLACT_RES;

typedef struct {
	GFL_CLUNIT* p_unit;
	DEBUG_CLACT_RES res[ 1 ];
	GFL_CLWK*	p_wk;
	GFL_CLWK*	p_subwk[ CLACT_WKNUM ];
} DEBUG_CLACT;


//-----------------------------------------------------------------------------
/**
 *					プロトタイプ宣言
*/
//-----------------------------------------------------------------------------
// 基本的なところ
static void DEBUG_CommonDispInit( void );
static void DEBUG_CommonDispExit( void );
static void* DEBUG_CommonFileLoad( char* path, u32 heapID );


// セルアクターサンプル
static GFL_PROC_RESULT DEBUG_ClactProcInit( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work );
static GFL_PROC_RESULT DEBUG_ClactProcMain( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work );
static GFL_PROC_RESULT DEBUG_ClactProcEnd( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work );

extern const GFL_PROC_DATA DebugClactProcData = {
	DEBUG_ClactProcInit,
	DEBUG_ClactProcMain,
	DEBUG_ClactProcEnd,
};

static void DEBUG_ClactWorkResLoad( DEBUG_CLACT_RES* p_wk, u32 heapID );
static void DEBUG_ClactWorkResRelease( DEBUG_CLACT_RES* p_wk );
static GFL_CLWK* DEBUG_ClactWorkAdd( GFL_CLUNIT* p_unit, DEBUG_CLACT_RES* p_res, const GFL_CLWK_DATA* cp_data, u32 heapID );
static void DEBUG_ClactWorkDel( GFL_CLWK* p_wk );
static void DEBUG_ClactWorkKeyMove( GFL_CLWK* p_wk, int trg, int cont );



//----------------------------------------------------------------------------
/**
 *	@brief	基本的な画面設定
 *
 *	@param	heapID	ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUG_CommonDispInit( void )
{
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
	GFL_DISP_SetDispOn();
	{
		static const GFL_BG_DISPVRAM param = {
			GX_VRAM_BG_128_B,				// メイン2DエンジンのBG
			GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
			GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
			GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
			GX_VRAM_OBJ_80_EF,				// メイン2DエンジンのOBJ
			GX_VRAM_OBJEXTPLTT_NONE,		// メイン2DエンジンのOBJ拡張パレット
			GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
			GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
			GX_VRAM_TEX_0_A,				// テクスチャイメージスロット
			GX_VRAM_TEXPLTT_0_G				// テクスチャパレットスロット
		};
		GFL_DISP_SetBank( &param );
	}

	// OBJマッピングモード
	GX_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );
	GXS_SetOBJVRamModeChar( GX_OBJVRAMMODE_CHAR_1D_32K );


	GFL_DISP_GX_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
	GFL_DISP_GXS_SetVisibleControl( GX_PLANEMASK_OBJ, VISIBLE_ON );
}

//----------------------------------------------------------------------------
/**
 *	@brief	基本的な画面設定の破棄
 */
//-----------------------------------------------------------------------------
static void DEBUG_CommonDispExit( void )
{
	GFL_DISP_SetDispOff();
	GFL_DISP_GX_InitVisibleControl();
	GFL_DISP_GXS_InitVisibleControl();
}

//----------------------------------------------------------------------------
/**
 *	@brief	ファイル読み込み
 *
 *	@param	path		パス
 *	@param	heapID		ヒープID
 *
 *	@return	データポインタ
 */
//-----------------------------------------------------------------------------
static void* DEBUG_CommonFileLoad( char* path, u32 heapID )
{
	int size, result;
	void* buff;

	// サイズ取得
	size = GF_GetFileSize( path );

	// バッファ作成
	buff = GFL_HEAP_AllocMemory( heapID, size );

	// 読み込み
	result = GF_ReadFile( path, buff );
	GF_ASSERT( size == result );
	return buff;
}

//-----------------------------------------------------------------------------
/**
 *			セルアクターサンプルソース
 */
//-----------------------------------------------------------------------------
//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターサンプルソース
 *
 *	@param	p_proc		プロセスワーク
 *	@param	p_seq		シーケンス
 *	@param	p_param		パラメータ
 *	@param	p_work		プロセス内ワーク
 *
 *	@retval	GFL_PROC_RES_CONTINUE	動作継続中
 *	@retval	GFL_PROC_RES_FINISH		動作終了
 *
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_ClactProcInit( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work )
{
	DEBUG_CLACT*		p_clactw;

	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_TOMOYA_DEBUG, 0x10000 );
	// 基本的な画面設定
	DEBUG_CommonDispInit();
	
	// clactサンプルワーク作成
	p_clactw = (DEBUG_CLACT*)GFL_PROC_AllocWork( p_proc, sizeof(DEBUG_CLACT), HEAPID_TOMOYA_DEBUG );
	GFL_STD_MemFill( p_clactw, 0, sizeof(DEBUG_CLACT) );
	

	// セルアクターシステム初期化
	// まずこの処理を行う必要があります。
	{
		static const GFL_CLSYS_INIT	param = {
			// メインとサブのサーフェース左上座標を設定します。
			// サーフェースのサイズは（256,192）にするのが普通なので、
			// メンバには入れませんでした。
			// 上下の画面をつなげて使用するときは、
			// サブサーフェースの左上座標を(0, 192)などにする必要があると思います。
			0, 0,		// メインサーフェースの左上座標（x,y）
			0, 256,		// サブサーフェースの左上座標（x,y）
			
			// 今はフルにOAMAttrを使用する場合の設定
			// 通信アイコンなどで0～3のOam領域を使えないときなどは、
			// OAMAttr管理数設定を変更する必要があります。
			0, 128,		// メインOAMマネージャのOamAttr管理数(開始No,管理数)
			0, 128,		// サブOAMマネージャのOamAttr管理数(開始No,管理数)
		};
		GFL_CLACT_Init( &GFL_CLSYSINIT_DEF_DIVSCREEN, HEAPID_TOMOYA_DEBUG );
	}
	
	// セルアクターユニット作成
	// 各アプリケーション単位で作成する。
	p_clactw->p_unit = GFL_CLACT_UNIT_Create( CLACT_WKNUM, HEAPID_TOMOYA_DEBUG );

	
	// セルアクターワーク生成
	// セルアクターワークを生成する手順
	// １：キャラ、パレット、セル、セルアニメ（マルチセル、マルチセルアニメ）を読み込む
	// ２：キャラ、パレットをVRAMに転送する
	// ３：登録データを作成する
	// ４：登録
	// 1.2読み込みと転送
	{
		static const GFL_CLWK_DATA data = {
			64, 64,	//座標(x,y)
			0,		//アニメーションシーケンス
			4,		//優先順位
			0,		//bg優先順位
		};
		DEBUG_ClactWorkResLoad( &p_clactw->res[0], HEAPID_TOMOYA_DEBUG );
		p_clactw->p_wk = DEBUG_ClactWorkAdd( p_clactw->p_unit, &p_clactw->res[0], &data, HEAPID_TOMOYA_DEBUG );

		{
			static GFL_CLWK_DATA sub_data = {
				0, 64,	//座標(x,y)
				0,		//アニメーションシーケンス
				0,		//優先順位
				0,		//bg優先順位
			};
			int i;

			for( i=0; i<4; i++ ){
				sub_data.pos_x += i*16;
				sub_data.softpri = i*4;
				p_clactw->p_subwk[i] = DEBUG_ClactWorkAdd( p_clactw->p_unit, &p_clactw->res[0], &sub_data, HEAPID_TOMOYA_DEBUG );
			}
		}
	}


	// 操作説明
	{
		OS_TPrintf( "********キー操作********\n" );
		OS_TPrintf( "十字キー	自機移動\n" );
		OS_TPrintf( "Aボタン	アニメーションスイッチ\n" );
		OS_TPrintf( "Bボタン	アニメーションチェンジ\n" );
		OS_TPrintf( "Yボタン	メイン画面からサブ画面に移動\n" );
		OS_TPrintf( "Xボタン	サブ画面からメイン画面に移動\n" );
		OS_TPrintf( "STARTボタン	メニューに戻る\n" );
		
	}
	
	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターサンプルソース
 *
 *	@param	p_proc		プロセスワーク
 *	@param	p_seq		シーケンス
 *	@param	p_param		パラメータ
 *	@param	p_work		プロセス内ワーク
 *
 *	@retval	GFL_PROC_RES_CONTINUE	動作継続中
 *	@retval	GFL_PROC_RES_FINISH		動作終了
 *
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_ClactProcMain( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work )
{
	DEBUG_CLACT*		p_clactw = (DEBUG_CLACT*)p_work;
	int trg, cont;


	trg = GFL_UI_KEY_GetTrg();
	cont = GFL_UI_KEY_GetCont();
	// キー動作
	DEBUG_ClactWorkKeyMove( p_clactw->p_wk, trg, cont );

	// セルアクターユニット描画処理
	GFL_CLACT_ClearOamBuff();
	GFL_CLACT_UNIT_Draw( p_clactw->p_unit );

	// セルアクターシステムメイン処理
	// 全ユニット描画が完了してから行う必要があります。
	GFL_CLACT_Main();

/*		GFLUser_VIntr関数内でVBlank関数が呼ばれるのでコメントアウト
	// Vintr待ち
	OS_WaitIrq(TRUE,OS_IE_V_BLANK);
	// Vブランク期間で実行します。
	// ただし、この関数は割り込みないで使用しないでください。
	GFL_CLACT_VblankFunc();
	// 割り込み内で使用するときは、GFL_CLACT_VBlankFuncTransOnly()と
	// GFL_CLACT_ClearOamBuff()を使用してください。
//*/
	if( trg & PAD_BUTTON_START ){
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターサンプルソース
 *
 *	@param	p_proc		プロセスワーク
 *	@param	p_seq		シーケンス
 *	@param	p_param		パラメータ
 *	@param	p_work		プロセス内ワーク
 *
 *	@retval	GFL_PROC_RES_CONTINUE	動作継続中
 *	@retval	GFL_PROC_RES_FINISH		動作終了
 *
 */
//-----------------------------------------------------------------------------
static GFL_PROC_RESULT DEBUG_ClactProcEnd( GFL_PROC* p_proc, int* p_seq, void* p_param, void* p_work )
{
	DEBUG_CLACT*		p_clactw = (DEBUG_CLACT*)p_work;

	// DISPLAYOFF
	DEBUG_CommonDispExit();

	// セルアクターワーク破棄
	DEBUG_ClactWorkDel( p_clactw->p_wk );

	// セルアクターリソース破棄
	DEBUG_ClactWorkResRelease( &p_clactw->res[0] );

	// セルアクターユニット破棄
	GFL_CLACT_UNIT_Delete( p_clactw->p_unit );

	// セルアクターシステム破棄
	// 全てのセルアクターユニットを破棄してから呼んでください。
	GFL_CLACT_Exit();

	// ワーク破棄
	GFL_PROC_FreeWork( p_proc );
	//GFL_HEAP_FreeMemory( p_param );

	GFL_HEAP_DeleteHeap( HEAPID_TOMOYA_DEBUG );

	return GFL_PROC_RES_FINISH;
}

//----------------------------------------------------------------------------
/**
 *	@brief	セルアクターリソース読み込み
 *
 *	@param	p_wk		ワーク
 *	@param	heapID		ヒープID
 */
//-----------------------------------------------------------------------------
static void DEBUG_ClactWorkResLoad( DEBUG_CLACT_RES* p_wk, u32 heapID )
{
	BOOL result;
	void* p_buff;
	NNSG2dCharacterData* p_char;
	NNSG2dPaletteData* p_pltt;
	
	// キャラクタデータ読み込み＆転送
	{
		p_buff = DEBUG_CommonFileLoad( "src/sample_graphic/hero.NCGR", heapID );
		result = NNS_G2dGetUnpackedCharacterData( p_buff, &p_char );
		GF_ASSERT( result );
		NNS_G2dInitImageProxy( &p_wk->imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&p_wk->imageproxy );
		NNS_G2dLoadImage1DMapping( 
				p_char, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&p_wk->imageproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// パレットデータ読み込み＆転送
	{
		p_buff = DEBUG_CommonFileLoad( "src/sample_graphic/hero.NCLR", heapID );
		result = NNS_G2dGetUnpackedPaletteData( p_buff, &p_pltt );
		GF_ASSERT( result );
		NNS_G2dInitImagePaletteProxy( &p_wk->plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DMAIN, 
				&p_wk->plttproxy );
		NNS_G2dLoadPalette( 
				p_pltt, 
				0, 
				NNS_G2D_VRAM_TYPE_2DSUB, 
				&p_wk->plttproxy );
		GFL_HEAP_FreeMemory( p_buff );
	}

	// セルデータ読み込み
	{
		p_wk->p_cellbuff = DEBUG_CommonFileLoad( "src/sample_graphic/hero.NCER", heapID );
		result = NNS_G2dGetUnpackedCellBank( p_wk->p_cellbuff, &p_wk->p_cell );
		GF_ASSERT( result );
	}

	// セルアニメデータ読み込み
	{
		p_wk->p_cellanmbuff = DEBUG_CommonFileLoad( "src/sample_graphic/hero.NANR", heapID );
		result = NNS_G2dGetUnpackedAnimBank( p_wk->p_cellanmbuff, &p_wk->p_cellanm );
		GF_ASSERT( result );
	}
}

//----------------------------------------------------------------------------
/**
 *	@brief	リソースデータ破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void DEBUG_ClactWorkResRelease( DEBUG_CLACT_RES* p_wk )
{
	GFL_HEAP_FreeMemory( p_wk->p_cellbuff );
	GFL_HEAP_FreeMemory( p_wk->p_cellanmbuff );
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクター登録
 *	
 *	@param	p_unit		セルアクターユニット
 *	@param	p_res		リソースワーク
 *	@param	cp_data		登録データ
 *	@param	heapID		ヒープ
 *
 *	@return	生成したワーク
 */
//-----------------------------------------------------------------------------
static GFL_CLWK* DEBUG_ClactWorkAdd( GFL_CLUNIT* p_unit, DEBUG_CLACT_RES* p_res, const GFL_CLWK_DATA* cp_data, u32 heapID )
{
	GFL_CLWK_RES resdat;
	GFL_CLWK* p_wk;

	// リソースデータ代入
	GFL_CLACT_WK_SetCellResData( &resdat, 
			&p_res->imageproxy, &p_res->plttproxy,
			p_res->p_cell, p_res->p_cellanm );
	// 登録
	p_wk = GFL_CLACT_WK_Add( p_unit, 
			cp_data, &resdat,
			// **登録先サーフェース指定{setsf}**
			// セルアクターユニットに独自レンダラーを
			// 設定していない場合は
			// CLSYS_DRAW_MAIN	メイン画面に登録
			// CLSYS_DRAW_SUB	サブ画面に登録
			// 独自レンダラーを設定している場合は
			// サーフェースインデックスを指定
			// ↓
			CLWK_SETSF_NONE,
			heapID );

	// オートアニメーション設定
	GFL_CLACT_WK_SetAutoAnmFlag( p_wk, TRUE );

	return p_wk;
}

//----------------------------------------------------------------------------
/**
 *	@brief	アクター破棄
 *
 *	@param	p_wk	ワーク
 */
//-----------------------------------------------------------------------------
static void DEBUG_ClactWorkDel( GFL_CLWK* p_wk )
{
	GFL_CLACT_WK_Remove( p_wk );
}

//----------------------------------------------------------------------------
/**
 *	@brief	キー動作
 *
 *	@param	p_wk	ワーク
 *	@param	trg		trgキー
 *	@param	cont	contキー
 */
//-----------------------------------------------------------------------------
static void DEBUG_ClactWorkKeyMove( GFL_CLWK* p_wk, int trg, int cont )
{
	GFL_CLACTPOS pos;
	BOOL auto_anm;
	u16 anm_seq;
	
	// 座標操作には、絶対座標とサーフェース内相対座標の
	// ２パターンがあります。
	// GFL_CLACT_WK_SetWldPos	Wldとつくのが絶対座標
	// GFL_CLACT_WK_SetPos		何もつかないのがサーフェース内相対座標です。
	// サーフェース内相対座標は、GFL_CLACT_WK_Add時と同じで、
	// setsfを指定する必要があります。
	GFL_CLACT_WK_GetWldPos( p_wk, &pos );	
	
	// 十字キーで移動
	if( cont & PAD_KEY_UP ){
		pos.y --;
	}
	if( cont & PAD_KEY_DOWN ){
		pos.y ++;
	}
	if( cont & PAD_KEY_LEFT ){
		pos.x --;
	}
	if( cont & PAD_KEY_RIGHT ){
		pos.x ++;
	}
	GFL_CLACT_WK_SetWldPos( p_wk, &pos );

	// オートアニメーションON
	if( trg & PAD_BUTTON_A ){
		auto_anm = GFL_CLACT_WK_GetAutoAnmFlag( p_wk );
		GFL_CLACT_WK_SetAutoAnmFlag( p_wk, auto_anm ^ 1 );
	}

	// アニメーションを変える
	if( trg & PAD_BUTTON_B ){
		anm_seq = GFL_CLACT_WK_GetAnmSeq( p_wk );
		anm_seq ^= 1;
		GFL_CLACT_WK_SetAnmSeq( p_wk, anm_seq );

		// シーケンス数を表示
		OS_Printf( "seq %d\n", GFL_CLACT_WK_GetAnmSeqNum( p_wk ) );
		// シーケンスデータ取得
		OS_Printf( "seq data %d\n", GFL_CLACT_WK_GetUserAttrAnmSeqNow(p_wk) );
	}

	// メインからサブへ
	if( trg & PAD_BUTTON_Y ){
#if 1
		GFL_CLACT_WK_GetPos( p_wk, &pos, CLSYS_DRAW_MAIN );
		GFL_CLACT_WK_SetPos( p_wk, &pos, CLSYS_DRAW_SUB );
#else
		int pal;
		pal = GFL_CLACT_WK_GetSoftPri( p_wk );
		pal ++;
		GFL_CLACT_WK_SetSoftPri( p_wk, pal );
		OS_Printf( "pri %d\n", pal );
#endif
	}

	// サブからメインへ
	if( trg & PAD_BUTTON_X ){
#if 1
		GFL_CLACT_WK_GetPos( p_wk, &pos, CLSYS_DRAW_SUB );
		GFL_CLACT_WK_SetPos( p_wk, &pos, CLSYS_DRAW_MAIN );
#else
		int pal;
		pal = GFL_CLACT_WK_GetSoftPri( p_wk );
		pal --;
		GFL_CLACT_WK_SetSoftPri( p_wk, pal );
		OS_Printf( "pri %d\n", pal );
#endif
	}
}
