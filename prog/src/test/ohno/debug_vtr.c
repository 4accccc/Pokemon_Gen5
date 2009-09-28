



//=============================================================================
/**
 * @file	  debug_vtr.c
 * @brief	  声の録音と再生
 * @author	ohno_katsumi@gamefreak.co.jp
 * @date	  09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"

#include "system/main.h"  //HEAPID
#include "message.naix"
#include "print/printsys.h"
#include "print/wordset.h"
#include "print/global_font.h"
#include "font/font.naix"
#include "print/str_tool.h"
#include "sound/pm_sndsys.h"
#include "system/wipe.h"
#include "net/network_define.h"
#include "savedata/wifilist.h"
#include "msg\msg_d_ohno.h"
#include "sound/snd_strm.h"


// マイクゲインを設定
#define MY_AMPGAIN PM_AMPGAIN_160

int micrate[]={
	MIC_SAMPLING_RATE_8K,
	MIC_SAMPLING_RATE_11K,
	MIC_SAMPLING_RATE_16K,
	MIC_SAMPLING_RATE_22K,
	MIC_SAMPLING_RATE_32K};



static int _SAMPLING_RATE = MIC_SAMPLING_RATE_8K;
#define _SAMPLING_SIZE (0xb000)

typedef struct _VOICETR_WORK VOICETR_WORK;

typedef void (StateFunc)(VOICETR_WORK* pState);



static void _changeState(VOICETR_WORK* pWork,StateFunc* state);
static void _changeStateDebug(VOICETR_WORK* pWork,StateFunc* state, int line);

#define _NET_DEBUG (1)

#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG



struct _VOICETR_WORK {
	GFL_FONT		  *fontHandle;
	GFL_MSGDATA		*mm;
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	HEAPID heapID;
	STRBUF			*strbuf;
	STRBUF			*strbufEx;
  WORDSET			*WordSet;								// メッセージ展開用ワークマネージャー

	SAVE_CONTROL_WORK* pSaveData;
	StateFunc* state;      ///< ハンドルのプログラム状態
	vu32 count;

	void* buffer;
	int bufferSize;
	int hz;
	int req;
	int getdataCount;
	BOOL bEnd;
	DWCGHTTPPost post;
	u8 mac[6];
//	u8 buffer[0x102];
};


static void _micKeyWait(VOICETR_WORK* pWork);




static GFLNetInitializeStruct aGFLNetInit = {
  NULL,  // 受信関数テーブル
  0, // 受信テーブル要素数
  NULL,    ///< ハードで接続した時に呼ばれる
  NULL,    ///< ネゴシエーション完了時にコール
  NULL,   // ユーザー同士が交換するデータのポインタ取得関数
  NULL,   // ユーザー同士が交換するデータのサイズ取得関数
  NULL,  // ビーコンデータ取得関数
  NULL,  // ビーコンデータサイズ取得関数
  NULL,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
  NULL,            // 普通のエラーが起こった場合 通信終了
  NULL,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
  NULL,  // 通信切断時に呼ばれる関数
  NULL,  // オート接続で親になった場合
  NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
  NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
  NULL,  ///< wifiフレンドリスト削除コールバック
  NULL,   ///< DWC形式の友達リスト
  NULL,  ///< DWCのユーザデータ（自分のデータ）
  GFL_NET_DWC_HEAPSIZE + 0x80000,   ///< DWCへのHEAPサイズ
  TRUE,        ///< デバック用サーバにつなぐかどうか
  0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
  GFL_HEAPID_APP,  //元になるheapid
  HEAPID_NETWORK,  //通信用にcreateされるHEAPID
  HEAPID_WIFI,  //wifi用にcreateされるHEAPID
  HEAPID_NETWORK,  //IRC用にcreateされるHEAPID
  GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
  1,     // 最大接続人数
  48,    // 最大送信バイト数
  32,    // 最大ビーコン収集数
  TRUE,     // CRC計算
  FALSE,     // MP通信＝親子型通信モードかどうか
  GFL_NET_TYPE_WIFI,  //通信種別
  TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
  WB_NET_WIFICLUB,  //GameServiceID
#if GFL_NET_IRC
  IRC_TIMEOUT_STANDARD,	// 赤外線タイムアウト時間
#endif
    0,//MP親最大サイズ 512まで
    0,//dummy
};




static void _msgPrint(VOICETR_WORK* pWork,int msg)
{
	GFL_BMP_Clear( pWork->bmp, 0xff );
	GFL_MSG_GetString(pWork->mm, msg, pWork->strbuf);
	PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
	GFL_BMPWIN_TransVramCharacter( pWork->win );
}

static void _msgPrintNo(VOICETR_WORK* pWork,int msg,int no)
{
	GFL_BMP_Clear( pWork->bmp, 0xff );
	GFL_MSG_GetString(pWork->mm, msg, pWork->strbufEx);

	WORDSET_RegisterNumber(pWork->WordSet, 0, no,
												 5, STR_NUM_DISP_ZERO, STR_NUM_CODE_DEFAULT);
	WORDSET_ExpandStr( pWork->WordSet, pWork->strbuf,pWork->strbufEx );


	PRINTSYS_Print( pWork->bmp, 0, 0, pWork->strbuf, pWork->fontHandle);
	GFL_BMPWIN_TransVramCharacter( pWork->win );
}


//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(VOICETR_WORK* pWork,StateFunc state)
{
	pWork->state = state;
}

//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------
#ifdef _NET_DEBUG
static void _changeStateDebug(VOICETR_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
	OS_TPrintf("ghttp: %d\n",line);
#endif
	_changeState(pWork, state);
}
#endif




//--------------------------------------------------------------
/**
 * @brief	ペラップの録音したデータを再生
 *
 * @param	perap	PERAPVOICE型のポインタ
 * @param	sex		性別
 * @param	vol		ボリューム
 * @param	pan		パン
 *
 * @retval	"再生成功=TRUE、失敗=FALSE"
 */
//--------------------------------------------------------------
#if 1
#define WAVEOUT_CH_NORMAL	(14)							//波形で使用するチャンネルNO
#define WAVEOUT_PLAY_SPDx1 (32768)


//波形データ再生に必要なデータ構造体
typedef struct{
	NNSSndWaveOutHandle	handle;					//波形再生ハンドル
	NNSSndWaveFormat		format;					//波形データフォーマット
	const void*				dataaddr;				//波形データの先頭アドレス
	BOOL					loopFlag;				//ループフラグ
	int						loopStartSample;		//ループ開始サンプル位置
	int						samples;				//波形データのサンプル数
	int						sampleRate;				//波形データのサンプリングレート
	int						volume;					//音量
	int						speed;					//再生スピード
	int						pan;					//パン(0-127)
}WAVEOUT_WORK;

static BOOL Snd_PerapVoicePlaySub( const void* perap, int vol, int pan )
{
	u16 add_spd;
	int ret,wave_pan;
	NNSSndWaveOutHandle handle;
//	u8* perap_play_flag		= Snd_GetParamAdrs( SND_W_ID_PERAP_PLAY_FLAG );


	//波形再生用チャンネルを確保する
	handle = NNS_SndWaveOutAllocChannel( WAVEOUT_CH_NORMAL );

	//ランダムに音程を変える
	//gf_srand( sys.vsync_counter );
	//add_spd = ( gf_rand() % PERAP_WAVEOUT_SPD_RAND );


	{
		WAVEOUT_WORK waveout_wk;
		WAVEOUT_WORK* p;
		
		waveout_wk.handle			= handle;	//波形再生ハンドル
		waveout_wk.format			= NNS_SND_WAVE_FORMAT_PCM8;		//波形データフォーマット
		waveout_wk.dataaddr			= perap;			//波形データの先頭アドレス
		waveout_wk.loopFlag			= FALSE;						//ループフラグ
		waveout_wk.loopStartSample	= 0;							//ループ開始サンプル位置
		waveout_wk.samples			= _SAMPLING_SIZE;			//波形データのサンプル数
		waveout_wk.sampleRate		= _SAMPLING_RATE;			//波形データのサンプリングレート
		waveout_wk.volume			= vol;							//音量
		waveout_wk.speed			= WAVEOUT_PLAY_SPDx1;	//再生スピード
		waveout_wk.pan				= 64;						//パン(0-127)
//		ret = Snd_WaveOutStart( &waveout_wk, WAVEOUT_CH_NORMAL );

		NNS_SndWaveOutSetVolume( handle, 127 );
		p = &waveout_wk;
		ret = NNS_SndWaveOutStart( p->handle, p->format, p->dataaddr, p->loopFlag, p->loopStartSample,
								p->samples, p->sampleRate, p->volume, p->speed, p->pan );

	}


	return ret;
}

#endif


static  void _endCallback(MICResult	result, void*	arg )
{
	VOICETR_WORK* pWork=arg;
	vu32 ans = OS_GetVBlankCount() - pWork->count;

	OS_TPrintf("サンプリング完了%d %d\n",result, ans/60);
	_CHANGE_STATE(_micKeyWait);
}


//--------------------------------------------------------------
/**
 * @brief	録音開始
 * @param	none
 * @retval	"MIC_RESULT_SUCCESS		処理が正常に完了"
 * @retval	"それ以外				何らかの原因で失敗"
 */
//--------------------------------------------------------------
static MICResult Snd_PerapVoiceRecStart( VOICETR_WORK* pWork )
{
	MICAutoParam mic;	//マイクパラメータ
	MICResult ret;


	mic.type			= MIC_SAMPLING_TYPE_SIGNED_8BIT;	//サンプリング種別

	//バッファは32バイトアラインされたアドレスでないとダメ！
	mic.size = pWork->bufferSize;
	mic.buffer = pWork->buffer;


	if( (mic.size&0x1f) != 0 ){
		mic.size &= 0xffffffe0;
	}

	//代表的なサンプリングレートをARM7のタイマー周期に換算した値の定義
	//mic.rate			= MIC_SAMPLING_RATE_8K;
	mic.rate			= _SAMPLING_RATE;
//	mic.rate			= HW_CPU_CLOCK_ARM7 / PERAP_SAMPLING_RATE;

	//連続サンプリング時にバッファをループさせるフラグ
	mic.loop_enable		= FALSE;

	//バッファが飽和した際に呼び出すコールバック関数へのポインタ
	mic.full_callback	= _endCallback;

	//バッファが飽和した際に呼び出すコールバック関数へ渡す引数
	mic.full_arg		= pWork;

	pWork->count = OS_GetVBlankCount();

	
	ret = MIC_StartAutoSampling( &mic );

	return ret;
}


//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _noneState(VOICETR_WORK* pWork)
{

}

static void _cancelState(VOICETR_WORK* pWork)
{
	switch(GFL_UI_KEY_GetTrg())
	{
	case PAD_BUTTON_Y:
		SND_STRM_Stop();
		_CHANGE_STATE(_micKeyWait);
		break;
	}
}



//------------------------------------------------------------------------------
/**
 * @brief   キー入力で動きを変える
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _micKeyWait(VOICETR_WORK* pWork)
{
	switch(GFL_UI_KEY_GetTrg())
	{
	case PAD_BUTTON_X:
		Snd_PerapVoiceRecStart(pWork);
		_msgPrint(pWork, DEBUG_OHNO_MSG0006);
		_CHANGE_STATE(_noneState);
		break;
	case PAD_BUTTON_Y:
//		Snd_PerapVoicePlaySub( pWork->buffer, 127, 0 );
		SND_STRM_SetUpStraightData( SND_STRM_PCM8, pWork->hz,
																 GFL_HEAPID_APP, pWork->buffer,pWork->bufferSize);
		SND_STRM_Play();
		OS_TPrintf("再生開始\n");
		_CHANGE_STATE(_cancelState);
		break;
	case PAD_BUTTON_L:
		pWork->hz++;
		if(pWork->hz >= SND_STRM_HZMAX){
			pWork->hz = 0;
		}
		OS_TPrintf("%d になった\n",pWork->hz);
		_SAMPLING_RATE = micrate[pWork->hz];
		break;
	case PAD_BUTTON_B:
		_CHANGE_STATE(NULL);
		break;
	}
}

static void _mic_init(VOICETR_WORK* pWork)
{
	MIC_Init();
	PM_Init();

	{
		u32 ret;
		ret = PM_SetAmp(PM_AMP_ON);
		if( ret == PM_RESULT_SUCCESS )
		{
			OS_TPrintf("AMPをオンにしました。\n");
		}
		else
		{
			OS_TPrintf("AMPの初期化に失敗（%d）", ret);
		}
	}

	{
		u32 ret;
		ret = PM_SetAmpGain(MY_AMPGAIN);
		if( ret == PM_RESULT_SUCCESS )
		{
			OS_TPrintf("AMPのゲインを設定しました。\n");
		}
		else
		{
			OS_TPrintf("AMPのゲイン設定に失敗（%d）", ret);
		}
	}
	pWork->buffer = GFL_NET_Align32Alloc(GFL_HEAPID_APP ,_SAMPLING_SIZE);
	pWork->bufferSize = _SAMPLING_SIZE;
	
	_CHANGE_STATE(_micKeyWait);

}




static const GFL_DISP_VRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E,			// テクスチャパレットスロット
		GX_OBJVRAMMODE_CHAR_1D_32K,	// メインOBJマッピングモード
		GX_OBJVRAMMODE_CHAR_1D_32K,		// サブOBJマッピングモード
};


static GFL_PROC_RESULT VTRProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_PROC, 0x70000 );//テスト

	{
		VOICETR_WORK* pWork = GFL_PROC_AllocWork( proc, sizeof(VOICETR_WORK), HEAPID_PROC );

		pWork->pSaveData = SaveControl_GetPointer();  //デバッグ
		pWork->heapID = HEAPID_PROC;

		GFL_DISP_SetBank( &vramBank );

		//バックグラウンドの色を入れておく
		GFL_STD_MemFill16((void*)HW_BG_PLTT, 0x5ad6, 2);
	
		// 各種効果レジスタ初期化
		G2_BlendNone();

		// BGsystem初期化
		GFL_BG_Init( pWork->heapID );
		GFL_BMPWIN_Init( pWork->heapID );
		GFL_FONTSYS_Init();

			//ＢＧモード設定
	{
		static const GFL_BG_SYS_HEADER sysHeader = {
			GX_DISPMODE_GRAPHICS, GX_BGMODE_0, GX_BGMODE_3, GX_BG0_AS_2D,
		};
		GFL_BG_SetBGMode( &sysHeader );
	}

	// 個別フレーム設定
	{
		static const GFL_BG_BGCNT_HEADER bgcntText = {
			0, 0, 0x800, 0,
			GFL_BG_SCRSIZ_256x256, GX_BG_COLORMODE_16,
			GX_BG_SCRBASE_0x5800, GX_BG_CHARBASE_0x10000, 0x8000,
			GX_BG_EXTPLTT_01, 0, 0, 0, FALSE
		};

		GFL_BG_SetBGControl( GFL_BG_FRAME0_M,   &bgcntText,   GFL_BG_MODE_TEXT );
		GFL_BG_SetBGControl( GFL_BG_FRAME0_S,   &bgcntText,   GFL_BG_MODE_TEXT );

		GFL_BG_SetVisible( GFL_BG_FRAME0_M,   VISIBLE_ON );
		GFL_BG_SetVisible( GFL_BG_FRAME1_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME2_M,   VISIBLE_OFF );
		GFL_BG_SetVisible( GFL_BG_FRAME3_M,   VISIBLE_OFF );

//		GFL_BG_SetClearCharacter( GFL_BG_FRAME0_M, 0x20, 0x22, wk->heapID );
		GFL_BG_FillCharacter( GFL_BG_FRAME0_M, 0xff, 1, 0 );

//		void GFL_BG_FillScreen( u8 frmnum, u16 dat, u8 px, u8 py, u8 sx, u8 sy, u8 mode )
		GFL_BG_FillScreen( GFL_BG_FRAME0_M, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );

		GFL_BG_FillCharacter( GFL_BG_FRAME0_S, 0x00, 1, 0 );
		GFL_BG_FillScreen( GFL_BG_FRAME0_S, 0x0000, 0, 0, 32, 32, GFL_BG_SCRWRT_PALIN );
		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_S );
	}

	// 上下画面設定
		GX_SetDispSelect( GX_DISP_SELECT_SUB_MAIN );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		pWork->win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 4, 12, 24, 16, 0, GFL_BMP_CHRAREA_GET_F );
		pWork->bmp = GFL_BMPWIN_GetBmp( pWork->win);
		GFL_BMP_Clear( pWork->bmp, 0xff );
		GFL_BMPWIN_MakeScreen( pWork->win );
		GFL_BMPWIN_TransVramCharacter( pWork->win );

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		pWork->fontHandle = GFL_FONT_Create( ARCID_FONT, NARC_font_large_gftr,
																			GFL_FONT_LOADTYPE_FILE, FALSE, pWork->heapID );

		pWork->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_ohno_dat, pWork->heapID );
		pWork->strbuf = GFL_STR_CreateBuffer(64, pWork->heapID);
		pWork->strbufEx = GFL_STR_CreateBuffer(64, pWork->heapID);
		pWork->WordSet    = WORDSET_Create( pWork->heapID );

	//フォントパレット転送
		GFL_ARC_UTIL_TransVramPalette(ARCID_FONT, NARC_font_default_nclr, PALTYPE_MAIN_BG, 
																	0, 0x20, HEAPID_PROC);

		
		_CHANGE_STATE(_mic_init);
	}
  return GFL_PROC_RES_FINISH;
}
static GFL_PROC_RESULT VTRProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	VOICETR_WORK* pWork = mywk;
	StateFunc* state = pWork->state;

	if( state ){
		state( pWork );
		return GFL_PROC_RES_CONTINUE;
	}
	return GFL_PROC_RES_FINISH;
}

static GFL_PROC_RESULT VTRProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	VOICETR_WORK* pWork = mywk;

	GFL_STR_DeleteBuffer(pWork->strbuf);
	GFL_STR_DeleteBuffer(pWork->strbufEx);
	GFL_MSG_Delete(pWork->mm);
	GFL_FONT_Delete(pWork->fontHandle);

	WORDSET_Delete( pWork->WordSet );

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_PROC);  //テスト
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA VTRProcData = {
  VTRProc_Init,
  VTRProc_Main,
  VTRProc_End,
};

