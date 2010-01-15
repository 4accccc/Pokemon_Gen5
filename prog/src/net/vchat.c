//=============================================================================
/**
 * @file	vchat.c
 * @bfief	ボイスチャットラッパー。
 * @author	kazuki yoshihara
 * @date	06/02/27
 */
//=============================================================================

#include "gflib.h"

#include "net/dwc_rap.h"
#include "net/dwc_rapcommon.h"
#include "vchat.h"
#include <vct.h>

#define MYNNSFORMAT NNS_SND_STRM_FORMAT_PCM16

//----------------- この部分はデフォルトで決まっています。送信量とは関係しません
#define VCHAT_SAMPLING_RATE       (8000)   // Hz
#define SAMPLING_BYTE        (2)      // byte = 16bit
#define MAX_SAMPLING_TIME   (68)     // ms
#define MAX_CHANNELS 1
#define VCHAT_WAVE_SAMPLE ((int) (VCHAT_SAMPLING_RATE * MAX_SAMPLING_TIME * SAMPLING_BYTE) / 1000)  //1088
//-----------------
#define _SOUND_CHANNEL (0) //HGSS仕様

#define VCFINFO_SHOW
//#define VCT_ERROR_NONE (VCT_SUCCESS)


// デバッグ出力を大量に吐き出す場合定義
#if defined(DEBUG_ONLY_FOR_ohno)
#define DEBUGPRINT_ON (1)
#else
#define DEBUGPRINT_ON (0)
#endif


#if DEBUGPRINT_ON
#define VCT_PRINT(...) \
	(void) ((OS_TPrintf(__VA_ARGS__)))
#else   //DEBUGPRINT_ON
#define VCT_PRINT(...)           ((void) 0)
#endif  // DEBUGPRINT_ON



// マイクゲインを設定
#define MY_AMPGAIN PM_AMPGAIN_160

// 何フレームまで履歴を持って、チェックするか
#define HAWLING_CHECKFRAME 16

// 電話をかけてもう一度リトライする時間
#define VCT_START_TIMEOUT_NUM 60

#define _MAX_PLAYER_NUM  (2)

typedef struct{
	u8 sRecBuffer[VCHAT_WAVE_SAMPLE * 2 * MAX_CHANNELS];
	u8* pAudioBuffer;
	u8 sPlayBuffer[VCHAT_WAVE_SAMPLE * 2 * MAX_CHANNELS];
	// 無音サウンドとして渡す用のバッファ（常に０）
	u8 sSilentBuffer[VCHAT_WAVE_SAMPLE * 2 * MAX_CHANNELS];
	void (*disconnectCallback)();
	VCTSession sSession[(_MAX_PLAYER_NUM-1)];
	BOOL bConf[_MAX_PLAYER_NUM];  //対話モードかどうか
	int mode;     // vctの会話モード
	int state;
	int off_flag;
	int heapID;
	struct NNSSndStrm sSndStream;
	VCTSession *session;
	MICAutoParam micParam;
	u16 vctTime;  //リクエストの時間計測
	u8 bSendVoice;
	u8 firstCallback;

}MYVCT_WORK;

enum{
	VCTSTATE_INIT,
	VCTSTATE_WAIT,
	VCTSTATE_CALLING
};

static MYVCT_WORK* _vWork = NULL;

static void StartSoundLoop();
static void ClearSession(VCTSession *session);

// 初期化処理。親子共通。セッション確立後に呼ばれる。
//
// 音声系の初期化（最初の１回のみ）
//

static OSTick _tick_time;
static int _difftime = 0;


//----------------------------------------------------------------------------
/**
 *	@brief	初期化に呼ばれる処理
 *  @retval なし
 */
//-----------------------------------------------------------------------------

static void InitFirst(void)
{
	VCT_PRINT("Init sound system\n");
	// マイク関連の初期化
	MIC_Init();
	PM_Init();

	{
		u32 ret;
		ret = PM_SetAmp(PM_AMP_ON);
		if( ret == PM_RESULT_SUCCESS )
		{
			VCT_PRINT("AMPをオンにしました。\n");
		}
		else
		{
			VCT_PRINT("AMPの初期化に失敗（%d）", ret);
		}
	}

	{
		u32 ret;
		ret = PM_SetAmpGain(MY_AMPGAIN);
		if( ret == PM_RESULT_SUCCESS )
		{
			VCT_PRINT("AMPのゲインを設定しました。\n");
		}
		else
		{
			VCT_PRINT("AMPのゲイン設定に失敗（%d）", ret);
		}
	}

	VCT_EnableVAD( TRUE );
	// サウンドシステムの初期化
	//
	NNS_SndInit();
	NNS_SndStrmInit(&_vWork->sSndStream);

	MI_CpuClearFast(_vWork->sSilentBuffer, sizeof(_vWork->sSilentBuffer) );
	VCT_PRINT("Init sound system done.\n");

	_difftime = 0;

	VCT_EnableEchoCancel(TRUE);
}

//
// 音声処理コールバック（NitroSystem NNS_SndStrmを利用）
//
static void micCallback(MICResult result, void *arg)
{
#pragma unused(result, arg)
  NET_PRINT("MIC %d\n",result);

}

static void SndCallback(NNSSndStrmCallbackStatus sts,
												int nChannels,
												void* buffer[],
												u32 length,
												NNSSndStrmFormat format,
												void* arg)
{
#pragma unused(format)

	OSTick      start;
	const void *micAddr;
	u32         offset;
	u8*         micSrc;
	u32         ch;


	micSrc = (u8*)arg;

	if (sts == NNS_SND_STRM_CALLBACK_SETUP) {
		for (ch = 0; ch < nChannels; ++ch) {
			MI_CpuClear8(buffer[ch], length);
		}
		return;
	}

	if (_vWork->firstCallback) {
		GF_ASSERT(MIC_RESULT_SUCCESS == MIC_StartAutoSamplingAsync( &(_vWork->micParam), micCallback, NULL));
		_vWork->firstCallback = 0;
	}

	micAddr = MIC_GetLastSamplingAddress();
	offset  = (u32)((u8*)micAddr - micSrc);

	if ( offset < length ) {
		micSrc = micSrc + length;
	}



	if( PAD_DetectFold() )
	{
		// DSを閉じた場合
		micSrc = _vWork->sSilentBuffer;
	}

	// 音声送受信は受信は常に行います。VADとSSPのステート管理によって、セッションがあるときのみ
	// 実際に送信・受信を行います。
	//
	// lengthは現在オーディオのサンプリングレート・ビット数に応じた32ms分にのみ対応しています。
	// （例：8KHz, 8Bit で256バイト）。それ以外のサイズを渡すとassertionします。
	//

  for (ch = 0; ch < nChannels; ++ch) {
		if( !VCT_ReceiveAudio(buffer[ch], length, NULL) )
		{
			VCT_PRINT("音声データを受け取れません\n");
		}
	}
	if( _vWork->off_flag == 0 )
	{
    if(VCT_SendAudio(micSrc, length)){
      VCT_PRINT("○ length 0x%x\n", length);
		}
  }else{
    //   	    VCT_PRINT("×");
	}
	_vWork->bSendVoice = 2;
	return;
}




/////////////////////////////////////////////////////////////////////////////////////
//
// 電話をかける。
//

static int startCall( u8 aid )
{
	BOOL ret;
	VCTSession *session;


	if (_vWork->session == NULL) {
		session = VCT_CreateSession(aid);
		if (session == NULL) {
			// セッションを使い切っている、またはaidが自分自身の場合
			// CraeteSession が失敗します
			VCT_PRINT("Can't create session!\n");
			return 0;
		}

		ret = VCT_Request(session, VCT_REQUEST_INVITE);  //send
	}
	else{
		session = _vWork->session;
		ret = VCT_Request(session, VCT_REQUEST_INVITE);  //もう一度送る
	}
	if (ret != VCT_ERROR_NONE){
		VCT_PRINT("Can't request Invite [%d]\n", ret);
		VCT_DeleteSession(session);
		_vWork->session = NULL;
		return 0;
	}
	else {
		_vWork->session = session;
	}

	return 1;
}

static int receiveCall( u8 aid )
{
	BOOL ret;
	if ( _vWork->session != NULL && _vWork->session->state == VCT_STATE_INCOMING ) {
		// 他の端末から会話要求があった場合、ここでOKを返してストリーミングを開始

		ret = VCT_Response(_vWork->session, VCT_RESPONSE_OK);
		if (ret != VCT_ERROR_NONE){
			VCT_PRINT("Can't send response OK [%d]\n", ret);
			return 0;
		}

		if( !VCT_StartStreaming(_vWork->session) )
		{
			VCT_PRINT("can't start session! %d\n", _vWork->session->aid);
			return 0;
		} else {
			VCT_PRINT("ストリーミングを開始します%d\n", _vWork->session->aid);
		}


		return 1;
	}

	return 0;

}


/////////////////////////////////////////////////////////////////////////////////////
//
//  VoiceChatのイベントコールバック関数 カンファレンスモード
//
static void VoiceChatEventCallbackConference(u8 aid, VCTEvent event, VCTSession *session, void *data)
{
#pragma unused(aid, data)

	switch (event) {
	case VCT_EVENT_DISCONNECTED:
		VCT_PRINT("Disconnected\n");
		ClearSession(session);
		break;
	case VCT_EVENT_CONNECTED:
		VCT_StartStreaming(session);
		VCT_PRINT("Connected\n");
		break;
	case VCT_EVENT_ABORT:
		VCT_PRINT("Session abrot\n");
		ClearSession(session);
		break;
	default:
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
// VoiceChatのイベントコールバック関数 電話モード
//
static void VoiceChatEventCallbackPhone(u8 aid, VCTEvent event, VCTSession *session, void *data)
{
#pragma unused(aid, data)

	int ret;

	VCT_PRINT("event %d\n");
	switch (event) {
	case VCT_EVENT_INCOMING:
		if (_vWork->session) {
			VCT_PRINT("Return busy to %d\n", session->aid);
			ret = VCT_Response(session, VCT_RESPONSE_BUSY_HERE);
			VCT_DeleteSession(session);
			break;
		}

		VCT_PRINT("Invite From %d\n", session->aid);
		_vWork->session = session;
		break;

	case VCT_EVENT_RESPONDBYE:
		VCT_PRINT("Bye From %d\n", session->aid);
		ret = VCT_Response(session, VCT_RESPONSE_OK);
		if (ret != VCT_ERROR_NONE){
			VCT_PRINT("Can't send response Ok [%d]\n", ret);
		}
		ClearSession(session);
		GFL_NET_DWC_StopVChat();
		break;

	case VCT_EVENT_DISCONNECTED:
		VCT_PRINT("Disconnected\n");
		ClearSession(session);
		GFL_NET_DWC_StopVChat();
		break;

	case VCT_EVENT_CANCEL:
		VCT_PRINT("Cancel From %d\n", session->aid);
		ret = VCT_Response(session, VCT_RESPONSE_TERMINATED);
		if (ret != VCT_ERROR_NONE){
			VCT_PRINT("Can't send response RequestTerminated [%d]\n", ret);
		}
		ClearSession(session);
		break;

	case VCT_EVENT_CONNECTED:
		VCT_PRINT("200 OK From %d\n", session->aid);
		if(session->mode != _vWork->mode){
			ClearSession(session);
			VCT_PRINT("セッションが違うものが来た\n");
			return;
		}
		if( VCT_StartStreaming(session) )
		{
			VCT_PRINT("ストリーミングを開始します%d\n", session->aid);
			_vWork->state = VCTSTATE_CALLING;
		} else
		{
			VCT_PRINT("can't start session! %d\n", session->aid);
		}
		break;

	case VCT_EVENT_REJECT:
		VCT_PRINT("Reject From %d\n", session->aid);
		ClearSession(session);
		break;

	case VCT_EVENT_BUSY:
	case VCT_EVENT_TIMEOUT:
	case VCT_EVENT_ABORT:
		VCT_PRINT("Clear session by '%d'\n", event);
		ClearSession(session);
		break;

	default:
		VCT_PRINT(" not handler (%d)\n", event);
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////
//
// セッションのクリア
//
static void ClearSession(VCTSession *session)
{
	VCT_StopStreaming(session);
	VCT_DeleteSession(session);
	_vWork->session = NULL;
}


// 毎フレーム呼ばれる。
void myvct_main(BOOL offflg)
{

	_vWork->off_flag = offflg;
	{
		// ゲームフレームに一度呼び出すメイン関数。
		OSTick      start;
		start = OS_GetTick();
		_difftime += OS_TicksToMicroSeconds32(start - _tick_time) - 1000 * 1000 / 60;
		if( _difftime < -10000 ) _difftime = 0;
		_tick_time = start;
		VCT_Main();
		while( _difftime >= 1000 * 1000 / 60 )
		{
//			NET_PRINT("!");
			VCT_Main();
			_difftime -= 1000 * 1000 / 60 ;
		}
	}


	if(_vWork->mode != VCT_MODE_CONFERENCE){  // カンファレンスの場合すでに会話中扱いにする

//    VCT_PRINT("vctst: %d \n",_vWork->state );

		switch( _vWork->state )
		{
		case VCTSTATE_INIT:
			{
				if( DWC_GetMyAID() == 0 ){
					// 親が電話をかける。
					if( startCall(1) ) {
						// 相手の反応待ち
						_vWork->state = VCTSTATE_WAIT;
						_vWork->vctTime = VCT_START_TIMEOUT_NUM;
					}
				}
				else if(DWC_GetMyAID() == 1)
				{
					// 親から電話がかかってくるのを待つ
					if( receiveCall(0) )
					{
						// かかってきた
						_vWork->state = VCTSTATE_CALLING;
					}
				}
				break;
			}

		case VCTSTATE_WAIT:
			// 子機が電話に出るのを待っている。
			_vWork->vctTime --;
			if(_vWork->vctTime ==0){
				_vWork->state = VCTSTATE_INIT; //もう一度電話をかける
			}
			break;

		case VCTSTATE_CALLING:
			// 電話中
			break;
		}
	}
}


BOOL myvct_checkData( int aid, void *data, int size )
{
	if( _vWork == NULL ) return FALSE;

	if( VCT_HandleData (aid, data, size ) ){
#ifdef VCFINFO_SHOW
		VCT_PRINT(".");
#endif
		return TRUE;
	}
	return FALSE;
}

void myvct_init( int heapID, int codec,int maxEntry )
{
//	u8 cArray[3] = {13, 13, 13};
	u8 cArray[3] = {_SOUND_CHANNEL, _SOUND_CHANNEL, _SOUND_CHANNEL};  //サウンドのSOUND_CHANNEL
	u32 length;
	BOOL ret;
	int size;
	int vctmode = VCT_MODE_PHONE;

	VCT_PRINT("myvct%d %d\n",codec,maxEntry);
	if( _vWork == NULL )
	{
		void* vWork_temp=NULL;

#if 1
		_vWork = DWC_RAPCOMMON_Alloc(heapID, sizeof(MYVCT_WORK),32);
		_vWork->pAudioBuffer = DWC_RAPCOMMON_Alloc(heapID, VCT_AUDIO_BUFFER_SIZE * maxEntry * VCT_DEFAULT_AUDIO_BUFFER_COUNT + 32 ,32);
#else
    _vWork = GFL_NET_Align32Alloc(heapID, sizeof(MYVCT_WORK));
		_vWork->pAudioBuffer = GFL_NET_Align32Alloc(heapID, VCT_AUDIO_BUFFER_SIZE * maxEntry * VCT_DEFAULT_AUDIO_BUFFER_COUNT + 32 );
#endif
    //	_vWork->heapID = heapID;
		_vWork->disconnectCallback = NULL;
		InitFirst();
	}

	// マイクの初期化

	length = (u32)VCHAT_WAVE_SAMPLE;

	{
		_vWork->micParam.type   = MIC_SAMPLING_TYPE_SIGNED_12BIT;
		_vWork->micParam.buffer = _vWork->sRecBuffer;
		_vWork->micParam.size   = length * 2;
		_vWork->micParam.rate = (u32)((NNS_SND_STRM_TIMER_CLOCK / VCHAT_SAMPLING_RATE) * 64);
		_vWork->micParam.loop_enable = TRUE;
		_vWork->micParam.full_callback = NULL;
		_vWork->micParam.full_arg = NULL;
		_vWork->firstCallback = 1;
	}

	// サウンドストリーム再生の初期化。１対１会話限定
	GF_ASSERT(NNS_SndStrmAllocChannel(&_vWork->sSndStream, 1, cArray));
	NNS_SndStrmSetVolume(&_vWork->sSndStream, 0);

  VCT_PRINT("NNS_SndStrmSetup\n");
	ret = NNS_SndStrmSetup(&_vWork->sSndStream,
												 NNS_SND_STRM_FORMAT_PCM16,
												 _vWork->sPlayBuffer,
												 length * 2 * 1,
												 NNS_SND_STRM_TIMER_CLOCK / VCHAT_SAMPLING_RATE,
												 2,
												 SndCallback,
												 _vWork->sRecBuffer);
  GF_ASSERT(ret);

	_vWork->state = VCTSTATE_INIT;
	_vWork->session = NULL;

	{
		VCTConfig config;

		//        if(!CommLocalIsWiFiQuartetGroup(CommStateGetServiceNo())){   // ４人接続の時はボイスチャットを自動起動しない
		//    config.mode         = //VCT_MODE_PHONE;
		//  }
		//    else{
		//          config.mode         = //VCT_MODE_CONFERENCE;
		//        }
		config.mode = vctmode;
		_vWork->mode = config.mode;
		config.session      = _vWork->sSession;
		config.numSession   = maxEntry;
		config.aid          = DWC_GetMyAID();
		GF_ASSERT((config.aid != -1));
		if(_vWork->mode == VCT_MODE_CONFERENCE){
			config.callback = VoiceChatEventCallbackConference;
		}
		else{
			config.callback = VoiceChatEventCallbackPhone;
		}
		config.userData     = NULL;

		config.audioBuffer     = _vWork->pAudioBuffer;
		config.audioBufferSize = VCT_AUDIO_BUFFER_SIZE * maxEntry * VCT_DEFAULT_AUDIO_BUFFER_COUNT + 32;


		if (!VCT_Init(&config)) {
			VCT_PRINT("ERROR: Can't initialize VoiceChat!!!\n");
		}
	}

	_vWork->off_flag = 0;

	VCT_SetCodec( codec );
	StartSoundLoop();

	VCT_EnableEchoCancel( TRUE );

	VCT_PRINT("myvct_init\n");
	return;
}

static void StartSoundLoop()
{
	NNS_SndStrmStart(&_vWork->sSndStream);
}

void myvct_setCodec( int codec )
{
	VCT_SetCodec( codec );
}




//==============================================================================
/**
 * 会話終了要求をだします。まだ通話できていないときは即座に終了します。
 * myvct_setDisconnectCallbackで設定されたコールバックが呼び出されます。
 * @param   none
 * @retval  none
 */
//==============================================================================
void myvct_endConnection(){
	int ret;

	// まだ会話要求を出す前で、受け取る前
	if( _vWork->session == NULL || _vWork->state == VCTSTATE_INIT ) {
		GFL_NET_DWC_StopVChat();
		return;
	}

	if( _vWork->state == VCTSTATE_WAIT )
	{
		// 会話要求を出してまだ返事が来る前。
		ret = VCT_Request( _vWork->session, VCT_REQUEST_CANCEL );
		if (ret != VCT_ERROR_NONE){
			VCT_PRINT("Can't request Cancel [%d]\n", ret);
			GFL_NET_DWC_StopVChat();
			return;
		}
	}

	// 会話中。会話終了要求を出す。
	ret = VCT_Request( _vWork->session, VCT_REQUEST_BYE );
	if (ret != VCT_ERROR_NONE){
		VCT_PRINT("Can't request Bye [%d]\n", ret);
		GFL_NET_DWC_StopVChat();
		return;
	}
	return;
}

//==============================================================================
/**
 * 会話終了処理完了コールバックを設定します。
 * 相手から切られた場合も呼び出されます。
 * この関数が呼び出される直前に、vchat.c用のワークが解放されます。
 * @param   none
 * @retval  none
 */
//==============================================================================
void myvct_setDisconnectCallback( void (*disconnectCallback)() )
{
	_vWork->disconnectCallback = disconnectCallback;
}

//==============================================================================
/**
 * ライブラリ終了処理
 * @param   none
 * @retval  none
 */
//==============================================================================
void myvct_free(void){
	void (*callback)();

	if( _vWork != NULL )
	{
		callback = _vWork->disconnectCallback;

		// マイクのサンプリングとストリームをとめる。
		(void)MIC_StopAutoSampling();
		NNS_SndStrmStop(&_vWork->sSndStream);
		NNS_SndStrmFreeChannel(&_vWork->sSndStream);

		// VCTライブラリ終了処理
		VCT_Cleanup();

		// メモリを解放
		DWC_RAPCOMMON_Free( 0, _vWork->pAudioBuffer,0 );
		DWC_RAPCOMMON_Free( 0,_vWork,0  );
		_vWork = NULL;

		// コールバックの呼び出し。
		if( callback != NULL ) callback();
	}
}

//==============================================================================
/**
 * 音を拾ったのかどうかを調べる
 * @param
 * @retval  拾ったらTRUE
 */
//==============================================================================

BOOL myvct_IsSendVoiceAndInc(void)
{
	if(_vWork){
		VCTVADInfo outInfo;
		VCT_GetVADInfo( &outInfo );
		if(outInfo.scale > 2){
			return outInfo.activity;
		}
	}
	return FALSE;
}

//==============================================================================
/**
 * VCTカンファレンスに招待する
 * @param   bitmap   接続しているCLIENTのBITMAP
 * @param   myAid    自分のID
 * @retval  TRUE 設定できた
 * @retval  FALSE 現状のまま もしくは必要がない
 */
//==============================================================================

BOOL myvct_AddConference(int bitmap, int myAid)
{
	int i,ret;

	if(!(_vWork)  || (_vWork->mode != VCT_MODE_CONFERENCE)){
		return FALSE;
	}

	for(i = 0; i < _MAX_PLAYER_NUM;i++){
		if(i == myAid){
			continue;
		}
		if( bitmap & (1<<i)){  //いる場合
			if(_vWork->bConf[i]==TRUE){
				continue;
			}
			ret = VCT_AddConferenceClient(i);
			if (ret != VCT_ERROR_NONE){
				VCT_PRINT("AddConferenceError [%d]\n", ret);
				return FALSE;
			}
			else{
				_vWork->bConf[i] = TRUE;
				VCT_PRINT("AddConference設定 %d\n", i);
			}
		}
	}
	return TRUE;
}

//==============================================================================
/**
 * VCTカンファレンスから全員はずす
 * @param   bitmap   接続しているCLIENTのBITMAP
 * @param   myAid    自分のID
 * @retval  TRUE 設定できた
 * @retval  FALSE 現状のまま もしくは必要がない
 */
//==============================================================================

BOOL myvct_DelConference(int myAid)
{
	int i,ret;

	if(!(_vWork)  || (_vWork->mode != VCT_MODE_CONFERENCE)){
		return FALSE;
	}

	for(i = 0; i < _MAX_PLAYER_NUM;i++){
		if(i == myAid){
			continue;
		}
		if(_vWork->bConf[i]==TRUE){
			ret = VCT_RemoveConferenceClient(i);
			if (ret != VCT_ERROR_NONE){
				VCT_PRINT("DELConferenceError [%d]\n", ret);
				return FALSE;
			}
			else{
				_vWork->bConf[i] = FALSE;
			}
		}
	}
	return TRUE;
}

