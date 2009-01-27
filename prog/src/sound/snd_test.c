//==============================================================================================
/**
 * @file	snd_test.c
 * @brief	サウンドテスト(サウンドチェック)
 * @author	nohara
 * @date	2005.07.04
 *
 * ＜サウンドテストの例外処理について＞
 *
 * １）
 * サウンドヒープを使用して、データのロードを行うようにすると、
 * 曲の再生、停止ごとに、確保、開放を行わないといけなくなります。
 *
 * これだと、BGMとSEを同時に再生することが難しいので、
 *
 * ヒープを全てクリアして、
 * PLAYER_BGMにBGMのシーケンス、バンク、波形アーカイブ分のプレイヤーヒープを確保、
 *
 * BGMはPLAYER_BGMのプレイヤーヒープで再生するようにします。
 * MEもPLAYER_BGMのプレイヤーヒープで再生するようにします。
 *
 * ２）
 * SEは、再生するたびに、シーケンス、バンク、波形アーカイブを
 * サウンドヒープでロードしなおすようにします。
 *
 * ３）
 * 上記の例外処理をしているので、
 * サウンド設定が、ゲーム内とは、全く異なります。
 * そのため、通常のサウンド初期化を通した方が、
 * 安全なので、終了処理で、ソフトリセットをかけています。
 */
//==============================================================================================
#include <gflib.h>
#include "sound/snd_tool.h"
#include "snd_test.h"

#include "sound/snd_system.h"

#include "arc_def.h"
#include "message.naix"
#include "font/font.naix"

#include "msg/msg_snd_test_str.h"
#include "msg/msg_snd_test_name.h"

#include "system/main.h"
#include "system/bmp_menuwork.h"
#include "system/bmp_menulist.h"
#include "system/bmp_winframe.h"
#include "msg/msg_debugname.h"

#include "print/str_tool.h"

//==============================================================================================
//
//	定義
//
//==============================================================================================
enum{
	BUF_BGM = 0,
	BUF_SE,
	BUF_PV,

	BUF_MAX,								//バッファの最大数
};

#define SND_TEST_BUF_SIZE		(48)		//バッファのサイズ
//#define BG_PLANE_MENU (SND_TEST_BMPWIN_FRAME)
#define ITEM_NAME_MAX (32)
#define TESTMODE_PLT_FONT	(15)
#define LIST_DISP_MAX (5)
#define FBMP_COL_RED  (3)
#define FBMP_COL_WHITE (15)


//==============================================================================================
//
//	構造体
//
//==============================================================================================
//サウンドテスト構造体
typedef	struct {
	BOOL end_flag;
	u8	seq;							//処理ナンバー
	s8  select;							//選択対象
	u16 dmy2;

	int	old_bgm;						//古いBGMナンバー
	int	bgm;							//BGMナンバー
	int	se;								//SEナンバー
	int	pv;								//ポケモン鳴き声ナンバー

	int bgm_lv;							//階層
	int se_lv[SE_HANDLE_MAX];			//階層
	int pv_lv;							//階層

	u8  bgm_play_flag;					//一度でも再生したかフラグ
	u8  se_play_flag[SE_HANDLE_MAX];	//一度でも再生したかフラグ
	u8  pv_play_flag;					//一度でも再生したかフラグ
	u8  dmy3;
	
	int pitch;							//音程
	int tempo;							//テンポ
	int var;							//シーケンスローカルワークにセットする値

	u16* pMsg;

	GFL_BG_INI * bgl;

//	GFL_BMPWIN bmpwin;				//BMPウィンドウデータ

	STRBUF* msg_buf[BUF_MAX];

	GFL_MSGDATA* msgman;			//メッセージマネージャー


	BMP_MENULIST_DATA	*menuList;
	BMPMENULIST_WORK	*menuWork;
	PRINT_UTIL			printUtil;
	PRINT_QUE			*printQue;
	GFL_FONT 			*fontHandle;
	GFL_BMPWIN			*bmpWin;

	PRINT_UTIL			printUtilSub;
	PRINT_QUE			*printQueSub;
//	GFL_BMPWIN			*bmpWinSub;	//情報画面用？

	HEAPID heapId;
    
}SND_TEST_WORK;


//==============================================================================================
//
//	プロトタイプ宣言
//
//==============================================================================================
static void SndTestCall(SND_TEST_WORK * wk);

static void SndTestWorkInit( SND_TEST_WORK* wk );
static void SndTestSeqNext( SND_TEST_WORK* wk );

static void SndTestSeqInit( SND_TEST_WORK* wk );
static void SndTestSeqBgmFadeWait( SND_TEST_WORK* wk );
static void SndTestSeqKeyCheck( SND_TEST_WORK* wk );
static void SndTestSeqExit( SND_TEST_WORK* wk );

static void SndTestStop( SND_TEST_WORK* wk );
static void SndTestPlay( SND_TEST_WORK* wk );

//static void (* const SndTestTable[])() = {
static void (* const SndTestTable[])(SND_TEST_WORK*) = {
	SndTestSeqInit,						//初期化
	SndTestSeqBgmFadeWait,				//BGMフェードアウト待ち
	SndTestSeqKeyCheck,					//キーチェック
	SndTestSeqExit,						//終了
};

static void SndTestNumMsgSet( SND_TEST_WORK* wk, int num, u8 x, u8 y );
static void SndTestNameMsgSet( SND_TEST_WORK* wk, const STRBUF* msg, u8 x, u8 y );
static void MsgRewrite( SND_TEST_WORK* wk, s8 select );
static void SndTestInit( SND_TEST_WORK* wk );
static void SndTestCursor( SND_TEST_WORK* wk );
static void SndTestSysMsgSet( SND_TEST_WORK* wk );
static void SndTestStrPrint( SND_TEST_WORK* wk,  u32 strID, u32 x, u32 y );
static void SndTestTrackMute( SND_TEST_WORK* wk );

//BG設定
static void SetBank();
static void BgSet( SND_TEST_WORK* wk );


//==================================================================================================
//
//	データ
//
//==================================================================================================
/*
static const BMPWIN_DAT	SndTestWinData = {
	SND_TEST_BMPWIN_FRAME,					//ウインドウ使用フレーム
	SND_TEST_BMPWIN_PX1,SND_TEST_BMPWIN_PY1,//ウインドウ領域の左上のX,Y座標（キャラ単位で指定）
	SND_TEST_BMPWIN_SX,	SND_TEST_BMPWIN_SY,	//ウインドウ領域のX,Yサイズ（キャラ単位で指定）
	SND_TEST_BMPWIN_PL,						//ウインドウ領域のパレットナンバー	
	SND_TEST_BMPWIN_CH						//ウインドウキャラ領域の開始キャラクタナンバー
};
*/

//==================================================================================================
//
//	関数
//
//==================================================================================================

//--------------------------------------------------------------
/**
 * @brief	サウンドテスト呼び出し
 *
 * @param	none
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestCall(SND_TEST_WORK * wk)
{
#if 0
	if( wk == NULL ){
		OS_Printf("snd_test.c Alloc ERROR!");
		return;
	}

	wk->bgl = GF_BGL_BglIniAlloc( HEAPID_BASE_DEBUG );

	//BG設定
	GF_Disp_GX_VisibleControlInit();	//メイン画面の各面の表示コントロール初期化
	SetBank();							//VRAMバンク設定
	BgSet(wk);							//BG設定
	GF_Disp_DispOn();					//表示有効設定
	MSG_PrintInit();					//文字表示初期化関数

	OS_Printf( "\n\n//==========================================\n" );
	OS_Printf( "//\tサウンドテスト　スタート\n" );
	OS_Printf( "//==========================================\n" );

	wk->end_flag = FALSE;
	wk->seq			= 0;									//処理ナンバー
	SndTestWorkInit(wk);									//ワーク初期化

	//メッセージデータマネージャー作成
//	wk->msgman = MSGMAN_Create( MSGMAN_TYPE_DIRECT, ARC_MSG, 
//									NARC_msg_snd_test_name_dat, HEAPID_BASE_DEBUG );

	//app初期化(fld_debug.c)
	//FieldBitMapWinCgxSet();
	
	//SystemFontPaletteLoad( PALTYPE_MAIN_BG, 0, HEAPID_BASE_DEBUG );

	GF_BGL_BmpWinAddEx( wk->bgl, &wk->bmpwin, &SndTestWinData );		//ビットマップ追加
	GF_BGL_BmpWinDataFill( &wk->bmpwin, FBMP_COL_WHITE );				//塗りつぶし

	SndTestSysMsgSet( wk );									//必要な情報メッセージ
	SndTestCursor( wk );									//カーソル更新

	//曲名メッセージ表示
	MsgRewrite( wk, SND_TEST_TYPE_BGM );					//BGMナンバー
	MsgRewrite( wk, SND_TEST_TYPE_SE );						//SEナンバー
	MsgRewrite( wk, SND_TEST_TYPE_PV );						//PVナンバー

	SndTestNumMsgSet( wk, wk->pitch, ST_TYPE_X+88, ST_BGM_MSG_Y );//音程
	SndTestNumMsgSet( wk, wk->tempo, ST_TYPE_X+168, ST_BGM_MSG_Y );//テンポ

	GF_BGL_BmpWinOn( &wk->bmpwin );
#endif

	GX_SetMasterBrightness(0);	
	GXS_SetMasterBrightness(-16);
	GFL_DISP_GX_SetVisibleControlDirect(0);		//全BG&OBJの表示OFF
	GFL_DISP_GXS_SetVisibleControlDirect(0);
	SetBank();							//VRAMバンク設定
    GFL_BG_Init(wk->heapId );
	BgSet(wk);							//BG設定
    GFL_BMPWIN_Init( wk->heapId );

    
	GFL_ARC_UTIL_TransVramPalette( ARCID_FONT , NARC_font_default_nclr , PALTYPE_MAIN_BG , TESTMODE_PLT_FONT * 32, 16*2, wk->heapId );
	wk->fontHandle = GFL_FONT_Create( ARCID_FONT , NARC_font_large_nftr , GFL_FONT_LOADTYPE_FILE , FALSE , wk->heapId );

	wk->bmpWin = GFL_BMPWIN_Create( SND_TEST_BMPWIN_FRAME , 1,1,30,LIST_DISP_MAX*2,TESTMODE_PLT_FONT,GFL_BMP_CHRAREA_GET_B );
	GFL_BMPWIN_MakeScreen( wk->bmpWin );
	GFL_BMPWIN_TransVramCharacter( wk->bmpWin );
	GFL_BMP_Clear( GFL_BMPWIN_GetBmp(wk->bmpWin), 15 );
	GFL_BG_LoadScreenReq( SND_TEST_BMPWIN_FRAME );
	wk->printQue = PRINTSYS_QUE_Create( wk->heapId );
	PRINT_UTIL_Setup( &wk->printUtil , wk->bmpWin );

	wk->msgman = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, 
									NARC_message_snd_test_name_dat, wk->heapId );
    
//	wk->printQueSub = PRINTSYS_QUE_Create( wk->heapId );
//	PRINT_UTIL_Setup( &wk->printUtilSub , wk->bmpWinSub );
    
	return;
}

//--------------------------------------------------------------
/**
 * @brief	ワーク初期化
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestWorkInit( SND_TEST_WORK* wk )
{
	int i;

	wk->select		= SND_TEST_TYPE_BGM;		//選択対象

	wk->bgm			= SND_TEST_BGM_START_NO;	//BGMスタートナンバー
	wk->se			= SND_TEST_SE_START_NO;		//SEスタートナンバー
	wk->pv			= SND_TEST_PV_START_NO;		//ポケモン鳴き声スタートナンバー

	//初期階層
	wk->bgm_lv		= 1;			
	wk->pv_lv		= 1;		

	for( i=0; i < SE_HANDLE_MAX ;i++ ){
		wk->se_lv[i] = 1;		
	}

	//一度でも再生したかフラグ
	wk->bgm_play_flag	= 0;					
	wk->pv_play_flag	= 0;

	for( i=0; i < SE_HANDLE_MAX ;i++ ){
		wk->se_play_flag[i]	= 0;
	}

	wk->pMsg = NULL;

	wk->pitch		= 0;	//音程
	wk->tempo		= 256;	//テンポ

	// バッファ作成
	for(i=0; i<BUF_MAX; i++)
	{
		wk->msg_buf[i] = GFL_STR_CreateBuffer(SND_TEST_BUF_SIZE*2, wk->heapId);
	}

	return;
}

//--------------------------------------------------------------
/**
 * @brief	次の処理へ
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 *
 * 必要な処理を追加していく(サブシーケンスのクリアなど)
 */
//--------------------------------------------------------------
static void SndTestSeqNext( SND_TEST_WORK* wk)
{
	wk->seq++;
	return;
}

//--------------------------------------------------------------
/**
 * @brief	初期化
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestSeqInit( SND_TEST_WORK* wk )
{
	//Snd_BgmFadeOut( 60 );				//BGMフェードアウト
	Snd_Stop();							//全停止
	Snd_BgmChannelSetAndReverbSet( 0 );	//使用可能チャンネル操作、リバーブ設定(クリア)
	SndTestSeqNext(wk);
	return;
}

//--------------------------------------------------------------
/**
 * @brief	BGMフェードアウト待ち
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestSeqBgmFadeWait( SND_TEST_WORK* wk )
{
	int ret;

	if( Snd_FadeCheck() ){						//BGMフェードアウト待ち
		return;
	}

	Snd_HeapStateClear();						//何もロードしていない状態にする(常駐も消した状態)
	
	//ベーシックバンク、波形ロード
	Snd_DebugLoadBank( BANK_BASIC );
	
	//プレイヤーヒープ作成
	ret = Snd_PlayerHeapCreate( PLAYER_BGM, 70000 );
	//Snd_UseHeapSizeOsPrint();					//追加したサウンドヒープの容量を出力する
	//OS_Printf( "player_bgm heap create = %d\n", ret );

	Snd_HeapSaveState( Snd_GetParamAdrs(SND_W_ID_HEAP_SAVE_SE) );	//階層保存

	SndTestSeqNext(wk);
	return;
}

//--------------------------------------------------------------
/**
 * @brief	キーチェック
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestSeqKeyCheck( SND_TEST_WORK* wk )
{
	int ret, i, spd;

	//音程変更
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ){
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			wk->pitch++;
		}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			wk->pitch--;
		}
		SndTestNumMsgSet( wk, wk->pitch, ST_TYPE_X+88, ST_BGM_MSG_Y );
		Snd_PlayerSetTrackPitch( SND_HANDLE_BGM, 0xffff, (wk->pitch*64) );
//		GF_BGL_BmpWinOn( &wk->bmpwin );
		return;
	}

	//シーケンスローカルワークを変更
	if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_R) && (GFL_UI_KEY_GetRepeat() & PAD_KEY_UP) ){

		wk->var++;
		if( wk->var >= 128 ){
			wk->var = 0;
		}
		return;
	}

	if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_R) && (GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN) ){

		wk->var--;
		if( wk->var < 0 ){
			wk->var = 127;
		}
		return;
	}

	//テンポ変更
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_START ){
		if( GFL_UI_KEY_GetRepeat() & PAD_KEY_UP ){
			wk->tempo+=32;
		}else if( GFL_UI_KEY_GetRepeat() & PAD_KEY_DOWN ){
			wk->tempo-=32;
		}
		SndTestNumMsgSet( wk, wk->tempo, ST_TYPE_X+168, ST_BGM_MSG_Y );
		Snd_PlayerSetTempoRatio( SND_HANDLE_BGM, wk->tempo );
//		GF_BGL_BmpWinOn( &wk->bmpwin );
		return;
	}

	//対象を変更(←BGM←PV←SE←)
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_DOWN ){
		wk->select++;
		if( wk->select >= SND_TEST_TYPE_MAX ){
			wk->select = SND_TEST_TYPE_BGM;
		}

		SndTestCursor( wk );					//カーソル更新
//		GF_BGL_BmpWinOn( &wk->bmpwin );
	}

	//対象を変更(→BGM→SE→PV→)
	if( GFL_UI_KEY_GetTrg() & PAD_KEY_UP ){
		wk->select--;
		if( wk->select < SND_TEST_TYPE_BGM ){
			wk->select = (SND_TEST_TYPE_MAX-1);
		}

		SndTestCursor( wk );					//カーソル更新
//		GF_BGL_BmpWinOn( &wk->bmpwin );
	}

	//シーケンスナンバーを進める
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_RIGHT ){

		if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_R) || (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) ){
			spd = 10;												//10ずつ
		}else{
			spd = 1;												//1ずつ
		}

		switch( wk->select ){
		case SND_TEST_TYPE_BGM:
			wk->old_bgm = wk->bgm;
			wk->bgm+=spd;
			if( wk->bgm >= SND_TEST_BGM_END_NO ){
				wk->bgm = SND_TEST_BGM_START_NO;
			}

			MsgRewrite( wk, SND_TEST_TYPE_BGM );					//BGMナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		case SND_TEST_TYPE_SE:
			wk->se+=spd;
			if( wk->se >= SND_TEST_SE_END_NO ){
				wk->se = SND_TEST_SE_START_NO;
			}

			MsgRewrite( wk, SND_TEST_TYPE_SE );						//SEナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		case SND_TEST_TYPE_PV:
			wk->pv+=spd;
			if( wk->pv >= SND_TEST_PV_END_NO ){
				wk->pv = SND_TEST_PV_START_NO;
			}

			MsgRewrite( wk, SND_TEST_TYPE_PV );						//PVナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		};
	}

	//シーケンスナンバーを戻す
	if( GFL_UI_KEY_GetRepeat() & PAD_KEY_LEFT ){

		if( (GFL_UI_KEY_GetCont() & PAD_BUTTON_R) || (GFL_UI_KEY_GetCont() & PAD_BUTTON_L) ){
			spd = 10;												//10ずつ
		}else{
			spd = 1;												//1ずつ
		}

		switch( wk->select ){
		case SND_TEST_TYPE_BGM:
			wk->old_bgm = wk->bgm;
			wk->bgm-=spd;
			if( wk->bgm < SND_TEST_BGM_START_NO ){
				wk->bgm = (SND_TEST_BGM_END_NO);
			}

			MsgRewrite( wk, SND_TEST_TYPE_BGM );					//BGMナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		case SND_TEST_TYPE_SE:
			wk->se-=spd;
			if( wk->se < SND_TEST_SE_START_NO ){
				wk->se = (SND_TEST_SE_END_NO);
			}

			MsgRewrite( wk, SND_TEST_TYPE_SE );						//SEナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		case SND_TEST_TYPE_PV:
			wk->pv-=spd;
			if( wk->pv < SND_TEST_PV_START_NO ){
				wk->pv = (SND_TEST_PV_END_NO);
			}

			MsgRewrite( wk, SND_TEST_TYPE_PV );						//PVナンバー
//			GF_BGL_BmpWinOn( &wk->bmpwin );
			break;
		};
	}

	//停止
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_B ){
		SndTestStop(wk);
	}

	//再生(停止させてから再生)
	if( GFL_UI_KEY_GetTrg() & PAD_BUTTON_A ){
		SndTestStop(wk);
		SndTestPlay(wk);

#if 1
		//シーケンスローカルワーク操作
		//サウンドハンドルが関連付けられた後に操作する！
		Snd_PlayerWriteVariable( wk->var );
		SndTestNumMsgSet( wk, wk->var, ST_TYPE_X+168, ST_BGM_MSG_Y+16 );
//		GF_BGL_BmpWinOn( &wk->bmpwin );
#endif

	}

	//初期化
	//if( GFL_UI_KEY_GetCont() & PAD_BUTTON_SELECT ){
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_Y ){
		Snd_Stop();						//全て停止
		SndTestInit( wk );				//ワーク、データなどクリア
//		GF_BGL_BmpWinOn( &wk->bmpwin );
	}

	//終了
	//if( GFL_UI_KEY_GetCont() & PAD_BUTTON_START ){
	if( GFL_UI_KEY_GetCont() & PAD_BUTTON_X ){
		Snd_Stop();						//全て停止
		SndTestSeqNext(wk);
		return;
	}

	return;
}

//--------------------------------------------------------------
/**
 * @brief	終了
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestSeqExit( SND_TEST_WORK* wk )
{
	int i;

	//BMPウィンドウOFF
//	GF_BGL_BmpWinOff( &wk->bmpwin );
	
	//BMPリスト破棄
	//BmpListExit( wp->bmplistContID,&DebugList,&DebugCursor );
	//BmpListExit( wk->blwin, NULL, NULL );

	//BMPWindow消去
//	GF_BGL_BmpWinDel( &wk->bmpwin );


	//開放処理
	PRINTSYS_QUE_Clear( wk->printQue );
	PRINTSYS_QUE_Delete( wk->printQue );
	GFL_BMPWIN_Delete( wk->bmpWin );
	GFL_FONT_Delete( wk->fontHandle );
	GFL_BMPWIN_Exit();
	GFL_BG_Exit();


	GX_SetMasterBrightness(0);
	GXS_SetMasterBrightness(0);



    
	GFL_MSG_Delete( wk->msgman );


	//文字バッファ削除
	for(i=0; i<BUF_MAX; i++)
	{
		if(wk->msg_buf[i]){
			GFL_STR_DeleteBuffer(wk->msg_buf[i]);
		}
	}



    
	wk->end_flag = TRUE;

}

//--------------------------------------------------------------
/**
 * @brief	サウンド停止処理
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestStop( SND_TEST_WORK* wk )
{
	int i,type;

	switch( wk->select ){
	case SND_TEST_TYPE_BGM:
		//Snd_BgmStop(Snd_NowBgmNoGet(), 0);
		//Snd_BgmStop( wk->old_bgm, 0 );
		NNS_SndPlayerStopSeqBySeqNo( wk->old_bgm, 0 );
		break;
	case SND_TEST_TYPE_SE:
		//Snd_SeStopBySeqNo( wk->se, 0 );
		Snd_SeStop( SND_HANDLE_SE_1, 0 );
		break;
	case SND_TEST_TYPE_PV:
		Snd_PMVoiceStop(0);
		break;
	};

}

//--------------------------------------------------------------
/**
 * @brief	サウンド再生処理
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestPlay( SND_TEST_WORK* wk )
{
	int type,ret;

	switch( wk->select ){
	case SND_TEST_TYPE_BGM:
        if(Snd_ArcPlayerStartSeqEx( SND_HANDLE_BGM, PLAYER_BGM, wk->bgm )){
            OS_TPrintf("SND_TEST_TYPE_BGMSTART\n");
        }
		break;

	case SND_TEST_TYPE_SE:
		//シーケンス、バンク、波形アーカイブロード(ヒープレベルは適当です)
		Snd_HeapLoadState( Snd_GetHeapSaveLv(SND_HEAP_SAVE_SE) );		//開放
		Snd_ArcLoadSeq( wk->se );										//ロード
		//Snd_HeapSaveState( Snd_GetParamAdrs(SND_W_ID_HEAP_SAVE_SE) );	//階層保存
		ret = Snd_ArcPlayerStartSeqEx( SND_HANDLE_SE_1, PLAYER_SE_1, wk->se );
		//OS_Printf( "se_play = %d\n", ret );
		break;

	case SND_TEST_TYPE_PV:
		Snd_PMVoicePlay( wk->pv, 0 );
		break;
	};

	return;
}


//==============================================================================================
//
//	設定関数
//
//==============================================================================================

//--------------------------------------------------------------
/**
 * @brief	VRAMバンク設定
 *
 * @param	none
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SetBank(void)
{
	//display.c
	
	GFL_DISP_VRAM vramSetTable = {	//VRAM設定構造体
		//GX_VRAM_BG_256_AB,			//メイン2DエンジンのBG
		GX_VRAM_BG_128_C,				//メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			//メイン2DエンジンのBG拡張パレット

		GX_VRAM_SUB_BG_32_H,			//サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		//サブ2DエンジンのBG拡張パレット

		//GX_VRAM_OBJ_64_E,				//メイン2DエンジンのOBJ
		GX_VRAM_OBJ_16_F,				//メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_NONE,		//メイン2DエンジンのOBJ拡張パレット

		//GX_VRAM_SUB_OBJ_NONE,			//サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJ_16_I,			//サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	//サブ2DエンジンのOBJ拡張パレット

		GX_VRAM_TEX_01_AB,			//テクスチャイメージスロット
		//GX_VRAM_TEX_0_A,				//テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E			//テクスチャパレットスロット
		//GX_VRAM_TEX_NONE,				//テクスチャイメージスロット
		//GX_VRAM_TEXPLTT_NONE			//テクスチャパレットスロット
		//GX_VRAM_TEXPLTT_0123_E		//テクスチャパレットスロット
	};

	GFL_DISP_SetBank( &vramSetTable );
}

//--------------------------------------------------------------
/**
 * @brief	BG設定
 *
 * @param	none
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void BgSet( SND_TEST_WORK* wk )
{
	//bg_system.c
	
#if 1
	{
		GFL_BG_SYS_HEADER BGsys_data = {	//BGシステム構造体
			GX_DISPMODE_GRAPHICS,			//表示モード指定 
			GX_BGMODE_0,					//ＢＧモード指定(メインスクリーン)
			GX_BGMODE_0,					//ＢＧモード指定(サブスクリーン)
			GX_BG0_AS_2D,					//ＢＧ０の２Ｄ、３Ｄモード選択
			//GX_BG0_AS_3D,					//ＢＧ０の２Ｄ、３Ｄモード選択
		};

		//メイン、サブのグラフィックスエンジンの表示モード設定
		//メイン、サブの画面の各面の表示コントロール初期化(display.c)
		//ビットマップウィンドウ設定構造体初期化
		GFL_BG_SetBGMode( &BGsys_data );
	}
#endif

	{	//BMPウィンドウ
		GFL_BG_BGCNT_HEADER BmpWinBgCntData = {	//BGコントロール設定構造体
	0, 0, 0x800, 0,
	GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
	GX_BG_SCRBASE_0x0000, GX_BG_CHARBASE_0x04000, GFL_BG_CHRSIZ_256x256,
	GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl(SND_TEST_BMPWIN_FRAME, 
							&BmpWinBgCntData, GFL_BG_MODE_TEXT );
        GFL_BG_SetVisible( SND_TEST_BMPWIN_FRAME, VISIBLE_ON );
        GFL_BG_ClearFrame( SND_TEST_BMPWIN_FRAME );
        GFL_BG_LoadScreenReq( SND_TEST_BMPWIN_FRAME );
        
	}

	return;
}

//--------------------------------------------------------------
/**
 * @brief	数字メッセージ表示
 *
 * @param	win_index	ビットマップINDEX
 * @param	num			数値
 * @param	x			表示位置X
 * @param	y			表示位置Y
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestNumMsgSet( SND_TEST_WORK* wk, int num, u8 x, u8 y )
{
	const u16 strLen = 64;
#if 0
    STRBUF* buf = STRBUF_Create( 6, HEAPID_BASE_DEBUG );

	STRBUF_SetNumber( buf, num, 4, NUMBER_DISPTYPE_SPACE, NUMBER_CODETYPE_DEFAULT );
	GF_STR_PrintSimple( win, FONT_SYSTEM, buf, x, y, MSG_NO_PUT, NULL );
	STRBUF_Delete(buf);
#endif
    STRBUF* msg = GFL_STR_CreateBuffer( strLen , wk->heapId );

	STRTOOL_SetNumber( msg, num, 4, STR_NUM_DISP_LEFT, STR_NUM_CODE_DEFAULT );
    
    PRINT_UTIL_Print(&wk->printUtil , wk->printQue , x, y, msg, wk->fontHandle);

	GFL_STR_DeleteBuffer(msg);
    
}

//--------------------------------------------------------------
/**
 * @brief	サウンドネームメッセージ表示
 *
 * @param	win_index	ビットマップINDEX
 * @param	num			数値
 * @param	x			表示位置X
 * @param	y			表示位置Y
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestNameMsgSet( SND_TEST_WORK* wk, const STRBUF* msg, u8 x, u8 y )
{
//	GF_STR_PrintSimple( win, FONT_SYSTEM, msg, x, y, MSG_NO_PUT, NULL );

    PRINT_UTIL_Print(&wk->printUtil , wk->printQue , x, y, msg, wk->fontHandle);

}

//------------------------------------------------------------------
/**
 * 固定文字列出力
 *
 * @param   win			BitmapWindow
 * @param   fontID		フォント
 * @param   strID		文字列ＩＤ
 * @param   x			描画Ｘ座標
 * @param   y			描画Ｙ座標
 * @param   wait		ウェイト
 * @param   callback	コールバック
 *
 */
//------------------------------------------------------------------
static void SndTestStrPrint( SND_TEST_WORK* wk, u32 strID, u32 x, u32 y )
{
	STRBUF *strbuf;
	const u16 strLen = 64;
	GFL_MSGDATA *msgMng = GFL_MSG_Create(GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_snd_test_str_dat, wk->heapId);

    strbuf = GFL_STR_CreateBuffer( strLen , wk->heapId );
    GFL_MSG_GetString(msgMng, strID, strbuf);
    PRINT_UTIL_Print(&wk->printUtil , wk->printQue , x, y, strbuf, wk->fontHandle);
	GFL_STR_DeleteBuffer(strbuf);
	GFL_MSG_Delete(msgMng);
}

//--------------------------------------------------------------
/**
 * @brief	BGMメッセージ再表示
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 * @param	select	選択対象
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void MsgRewrite( SND_TEST_WORK* wk, s8 select )
{
	int type;
	u32 msg_h_id;

	switch( select ){
	case SND_TEST_TYPE_BGM:
		//wk->bgm,se,pvには、シーケンスナンバーが入っているので、
		//スタートナンバーを引き、選択している値にして、
		//メッセージIDのスタートナンバーを足しメッセージを表示する
		msg_h_id = wk->bgm - SND_TEST_BGM_START_NO + msg_seq_pv_end + 1;

		//指定範囲を塗りつぶし
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->bmpWin), ST_TYPE_X, ST_BGM_MSG_Y+16, 8*24, 16*1 , 
							FBMP_COL_WHITE);

		SndTestStrPrint( wk,  msg_BGM, 
						ST_TYPE_X, ST_BGM_MSG_Y );
		SndTestNumMsgSet( wk, wk->bgm, ST_TYPE_X, ST_BGM_MSG_Y+16 );

		GFL_MSG_GetString( wk->msgman, msg_h_id, wk->msg_buf[BUF_BGM] );
		SndTestNameMsgSet( wk, wk->msg_buf[BUF_BGM], ST_NAME_X, ST_BGM_MSG_Y+16 );

		SndTestStrPrint( wk,  msg_ME, 
						ST_TYPE_X+32, ST_BGM_MSG_Y );

		SndTestStrPrint( wk,  msg_KEY, 
						ST_TYPE_X+56, ST_BGM_MSG_Y );

		SndTestStrPrint( wk,  msg_TMP, 
						ST_TYPE_X+136, ST_BGM_MSG_Y );

		break;

	case SND_TEST_TYPE_SE:
		msg_h_id = wk->se - SND_TEST_SE_START_NO + msg_seq_bgm_end + 1;

		//指定範囲を塗りつぶし
		GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->bmpWin)
							, ST_TYPE_X, ST_SE_MSG_Y+16, 8*24, 16*1,FBMP_COL_WHITE );

		//シーケンスナンバーから、SEのプレイヤーナンバーを取得
		type = Snd_GetPlayerNo(wk->se);
		type -= PLAYER_SE_1;

		//SEのみプレイヤーナンバー表示
		SndTestStrPrint(wk,  msg_PLAYER, 
						ST_TYPE_X+32, ST_SE_MSG_Y );
		SndTestNumMsgSet( wk, type, ST_TYPE_X+32+56, ST_SE_MSG_Y );

		SndTestStrPrint( wk,  msg_SE, 
						ST_TYPE_X, ST_SE_MSG_Y );

		SndTestNumMsgSet( wk, wk->se, ST_TYPE_X, ST_SE_MSG_Y+16 );

		GFL_MSG_GetString( wk->msgman, msg_h_id, wk->msg_buf[BUF_SE] );
		SndTestNameMsgSet( wk, wk->msg_buf[BUF_SE], ST_NAME_X, ST_SE_MSG_Y+16 );
		break;

	case SND_TEST_TYPE_PV:
		msg_h_id = wk->pv - SND_TEST_PV_START_NO + msg_seq_pv001;

		//指定範囲を塗りつぶし
		GFL_BMP_Fill(GFL_BMPWIN_GetBmp(wk->bmpWin)
							, ST_TYPE_X, ST_PV_MSG_Y+16, 8*24, 16*1,FBMP_COL_WHITE );

		SndTestStrPrint( wk,  msg_PV, 
						ST_TYPE_X, ST_PV_MSG_Y );
		SndTestNumMsgSet( wk, wk->pv, ST_TYPE_X, ST_PV_MSG_Y+16 );

		GFL_MSG_GetString( wk->msgman, msg_h_id, wk->msg_buf[BUF_PV] );
		SndTestNameMsgSet( wk, wk->msg_buf[BUF_PV], ST_NAME_X, ST_PV_MSG_Y+16 );
		break;
	};

	return;
}

//--------------------------------------------------------------
/**
 * @brief	初期状態にする(X_BUTTONを押した時の初期化！)
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestInit( SND_TEST_WORK* wk )
{
	//ワーク初期化
	SndTestWorkInit( wk );

	//書き直し
	MsgRewrite( wk, SND_TEST_TYPE_BGM );		//BGMナンバー
	MsgRewrite( wk, SND_TEST_TYPE_SE );			//SEナンバー
	MsgRewrite( wk, SND_TEST_TYPE_PV );			//PVナンバー

	SndTestCursor( wk );						//カーソル更新

	return;
}

//--------------------------------------------------------------
/**
 * @brief	カーソル
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestCursor( SND_TEST_WORK* wk )
{
    u16 y;

	switch( wk->select ){
	case SND_TEST_TYPE_BGM:
		y = ST_BGM_MSG_Y;
		break;
	case SND_TEST_TYPE_SE:
		y = ST_SE_MSG_Y;
		break;
	case SND_TEST_TYPE_PV:
		y = ST_PV_MSG_Y;
		break;
	};

	//指定範囲を塗りつぶし
	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->bmpWin), ST_CURSOR_X, ST_BGM_MSG_Y, 8*1, 16*8,FBMP_COL_WHITE );
	GFL_BMP_Fill( GFL_BMPWIN_GetBmp(wk->bmpWin), ST_CURSOR_X, y, 8*1, 16*1, FBMP_COL_RED );

	return;
}

//--------------------------------------------------------------
/**
 * @brief	システムメッセージ
 *
 * @param	wk		SND_TEST_WORKへのポインタ
 *
 * @retval	none
 */
//--------------------------------------------------------------
static void SndTestSysMsgSet( SND_TEST_WORK* wk )
{
	SndTestStrPrint( wk,  msg_SOUND_TEST, 
				ST_TITLE_X, ST_TITLE_Y );			//SOUND TEST
	SndTestStrPrint( wk,  msg_A_PLAY, 
				ST_KEY_X, ST_KEY_Y );				//A PLAY
	SndTestStrPrint( wk,  msg_B_STOP, 
				ST_KEY_X+64, ST_KEY_Y );			//B STOP
	SndTestStrPrint( wk,  msg_X_END, 
				ST_KEY_X, ST_KEY_Y+16 );			//X END
	SndTestStrPrint( wk,  msg_Y_INIT, 
				ST_KEY_X+64, ST_KEY_Y+16 );		//Y INIT

	return;
}


//==============================================================================================
//
//	PROC関連
//
//==============================================================================================

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTestProc_Init(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SND_TEST_WORK* wk = NULL;

	wk = GFL_PROC_AllocWork(proc, sizeof(SND_TEST_WORK), GFL_HEAPID_APP);

    GFL_STD_MemClear( wk,  sizeof(SND_TEST_WORK) );
    wk->heapId = GFL_HEAPID_APP;

	SndTestCall(wk);
	return GFL_PROC_RES_FINISH;
		
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTestProc_Main(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	SND_TEST_WORK *wk;
	wk = mywk;

	SndTestTable[wk->seq](wk);

	PRINTSYS_QUE_Main( wk->printQue );
	PRINT_UTIL_Trans( &wk->printUtil ,wk->printQue );

    
	if (wk->end_flag) {
		return GFL_PROC_RES_FINISH;
	} else {
		return GFL_PROC_RES_CONTINUE;
	}
}

//--------------------------------------------------------------
//--------------------------------------------------------------
static GFL_PROC_RESULT SoundTestProc_End(GFL_PROC * proc, int * seq, void * pwk, void * mywk)
{
	GFL_PROC_FreeWork(proc);
	//Main_SetNextProc(NO_OVERLAY_ID, &TitleProcData);

	//ソフトリセット
	//詳細はソース先頭の「サウンドテストの例外処理について」を参照して下さい
	//OS_InitReset();
//	OS_ResetSystem(0);									//ソフトリセット
	return GFL_PROC_RES_FINISH;
}

//--------------------------------------------------------------
//--------------------------------------------------------------
const GFL_PROC_DATA DebugSoundProcData = {
	SoundTestProc_Init,
	SoundTestProc_Main,
	SoundTestProc_End,
};


