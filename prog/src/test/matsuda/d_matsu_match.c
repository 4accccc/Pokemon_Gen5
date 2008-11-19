//==============================================================================
/**
 * @file	d_matsu_match.c
 * @brief	赤外線マッチング
 * @author	matsuda
 * @date	2008.11.12(水)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include <backup_system.h>
#include "savedata/contest_savedata.h"
#include "savedata/save_tbl.h"
#include "savedata/save_control.h"

#include "net\network_define.h"

#include "print\printsys.h"
#include "print\gf_font.h"
#include "arc_def.h"
#include "message.naix"
#include "test_graphic\d_taya.naix"
#include "msg\msg_d_matsu.h"
#include "test/performance.h"


//==============================================================================
//	定数定義
//==============================================================================
///エントリー最大人数
#define ENTRY_MAX		(4)
///子の最大数
#define CHILD_MAX		(ENTRY_MAX - 1)

///赤外線送信コマンド
enum{
	//子から送信
	CMD_CHILD_PROFILE = 0,		///<子のプロフィール
	
	//親から送信
	CMD_PARENT_PROFILE = 2,		///<親のプロフィール
	CMD_PARENT_SUCCESS = 4,		///<全ての受信完了
};

enum{
	MY_CHILD,		///<子である
	MY_PARENT,		///<親である
};

//==============================================================================
//	構造体定義
//==============================================================================
typedef struct{
	u8 initialize;
	u8 connect;
	int connect_wait;
	u8 seq;
	u8 success;			///<TRUE:送受信完了して切断した
}DM_IRC_WORK;

///赤外線エントリー時に送信するデータ
typedef struct{
	u16 name[11];
	u16 name_len;
	
	u8 parent_MacAddress[6];
	u8 oya_child;
	u8 padding;
}IRC_MATCH_ENTRY_PARAM;


typedef struct{
	GFL_BMPWIN		*win;
	GFL_BMP_DATA	*bmp;
	PRINT_UTIL		printUtil[1];
	BOOL			message_req;		///<TRUE:メッセージリクエストがあった
}DM_MSG_DRAW_WIN;

typedef struct {

	u16		seq;
	HEAPID	heapID;
	int debug_mode;
	int timer;
	
	//セーブ関連
	GFL_SAVEDATA *sv_normal;	///<ノーマルセーブデータへのポインタ
	
	//通信
	BOOL connect_ok;
	int send_key;
	
	u8 huge_data[0x1000];
	u8 receive_huge_data[2][0x1000];
	volatile u16 cs;
	
	DM_IRC_WORK irc;
	u8 oya;				///<1:親　0:子
	IRC_MATCH_ENTRY_PARAM my_profile;
	IRC_MATCH_ENTRY_PARAM child_profile[CHILD_MAX];
	IRC_MATCH_ENTRY_PARAM parent_profile;
	u8 entry_num;		///<エントリー済みの数
	BOOL connect_max;	///<TRUE:通信最大人数に達した
	s32 msg_pos_x[ENTRY_MAX];
	s32 msg_pos_y[ENTRY_MAX];
	
	ARCHANDLE		*arcHandle;
	GFL_FONT		*fontHandle;
	PRINT_QUE		*printQue;
	PRINT_STREAM	*printStream;
	PRINT_STREAM	*ps_entry[ENTRY_MAX];
	GFL_MSGDATA		*mm;
	STRBUF			*strbuf;
	STRBUF			*strbuf_entry[ENTRY_MAX];
	GFL_TCBLSYS		*tcbl;
	DM_MSG_DRAW_WIN drawwin[ENTRY_MAX + 1];	//+1 = メインメッセージウィンドウ
}D_MATSU_WORK;


//==============================================================================
//	ローカル変数
//==============================================================================
D_MATSU_WORK * glb_d_matsu = NULL;


//==============================================================================
//	プロトタイプ宣言
//==============================================================================
static BOOL DebugMatsuda_IrcMatch(D_MATSU_WORK *wk);
static BOOL DebugMatsuda_Wireless(D_MATSU_WORK *wk);
static void DM_IRC_ReceiveCallback(u8 *data, u8 size, u8 command, u8 value);
static void Local_EntryAdd(D_MATSU_WORK *wk, IRC_MATCH_ENTRY_PARAM *profile);
static void Local_ParentProfileSet(D_MATSU_WORK *wk, IRC_MATCH_ENTRY_PARAM *profile);
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y);

static void* _netBeaconGetFunc(void* pWork);
static int _netBeaconGetSizeFunc(void* pWork);
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo);
static void _initCallBack(void* pWork);
static void _connectCallBack(void* pWork);
static void _endCallBack(void* pWork);
static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvHugeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static u8 * _RecvHugeBuffer(int netID, void* pWork, int size);
static void _RecvKeyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _RecvChildProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);



//--------------------------------------------------------------
/**
 * @brief   
 *
 * @param   proc		
 * @param   seq		
 * @param   pwk		
 * @param   mywk		
 *
 * @retval  
 */
//--------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcInit( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	static const GFL_BG_DISPVRAM vramBank = {
		GX_VRAM_BG_128_A,				// メイン2DエンジンのBG
		GX_VRAM_BGEXTPLTT_NONE,			// メイン2DエンジンのBG拡張パレット
		GX_VRAM_SUB_BG_128_C,			// サブ2DエンジンのBG
		GX_VRAM_SUB_BGEXTPLTT_NONE,		// サブ2DエンジンのBG拡張パレット
		GX_VRAM_OBJ_128_B,				// メイン2DエンジンのOBJ
		GX_VRAM_OBJEXTPLTT_0_F,			// メイン2DエンジンのOBJ拡張パレット
		GX_VRAM_SUB_OBJ_16_I,			// サブ2DエンジンのOBJ
		GX_VRAM_SUB_OBJEXTPLTT_NONE,	// サブ2DエンジンのOBJ拡張パレット
		GX_VRAM_TEX_NONE,				// テクスチャイメージスロット
		GX_VRAM_TEXPLTT_0123_E			// テクスチャパレットスロット
	};

	D_MATSU_WORK* wk;
	
	DEBUG_PerformanceSetActive(FALSE);
	
	GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_MATSUDA_DEBUG, 0x70000 );
	wk = GFL_PROC_AllocWork( proc, sizeof(D_MATSU_WORK), HEAPID_MATSUDA_DEBUG );
	glb_d_matsu = wk;
	MI_CpuClear8(wk, sizeof(D_MATSU_WORK));
	wk->heapID = HEAPID_MATSUDA_DEBUG;
	{
		int i;
		volatile u16 cs = 0;
		u8 maca[6];
		OS_GetMacAddress(maca);
		for(i = 0; i < 6; i++){
			OS_TPrintf("mac addrs = %x %x %x %x %x %x\n", maca[0], maca[1], maca[2], maca[3], maca[4], maca[5]);
		}
		for(i = 0; i < 0x1000; i++){
			if(i < 0x5000){
				wk->huge_data[i] = maca[1];
			}
			else{
				wk->huge_data[i] = maca[2];
			}
			cs += wk->huge_data[i];
			if(i % 0x100 == 0 || i == 0x1000-1 || i == 0x1000-2){
				OS_TPrintf("cs = %x, huge %x %x\n", cs, i, wk->huge_data[i]);
			}
		}
		wk->cs = cs;
		OS_TPrintf("巨大データチェックサム = 0x%x\n", wk->cs);
	}
	
//	GFL_ARC_Init( arcFiles, NELEMS(arcFiles) );	gfl_use.cで1回だけ初期化に変更

	GFL_DISP_SetBank( &vramBank );

	// 各種効果レジスタ初期化
	G2_BlendNone();

	// BGsystem初期化
	GFL_BG_Init( wk->heapID );
	GFL_BMPWIN_Init( wk->heapID );
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

	{//メッセージ描画の為の準備
		int i;

		wk->drawwin[ENTRY_MAX].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 0, 32, 10, 0, GFL_BMP_CHRAREA_GET_F );
		wk->drawwin[ENTRY_MAX].bmp = GFL_BMPWIN_GetBmp(wk->drawwin[ENTRY_MAX].win);
		GFL_BMP_Clear( wk->drawwin[ENTRY_MAX].bmp, 0xff );
		GFL_BMPWIN_MakeScreen( wk->drawwin[ENTRY_MAX].win );
		GFL_BMPWIN_TransVramCharacter( wk->drawwin[ENTRY_MAX].win );
		PRINT_UTIL_Setup( wk->drawwin[ENTRY_MAX].printUtil, wk->drawwin[ENTRY_MAX].win );
		for(i = 0; i < ENTRY_MAX; i++){
			wk->drawwin[i].win = GFL_BMPWIN_Create( GFL_BG_FRAME0_M, 0, 12+i*2, 32, 2, 0, GFL_BMP_CHRAREA_GET_F);
			wk->drawwin[i].bmp = GFL_BMPWIN_GetBmp(wk->drawwin[i].win);
			GFL_BMP_Clear( wk->drawwin[i].bmp, 0xff );
			GFL_BMPWIN_MakeScreen( wk->drawwin[i].win );
			GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
			PRINT_UTIL_Setup( wk->drawwin[i].printUtil, wk->drawwin[i].win );
		}

		GFL_BG_LoadScreenReq( GFL_BG_FRAME0_M );

		wk->arcHandle = GFL_ARC_OpenDataHandle( ARCID_D_TAYA, wk->heapID );
		wk->fontHandle = GFL_FONT_CreateHandle( wk->arcHandle, NARC_d_taya_lc12_2bit_nftr,
			GFL_FONT_LOADTYPE_FILE, FALSE, wk->heapID );

//		PRINTSYS_Init( wk->heapID );
		wk->printQue = PRINTSYS_QUE_Create( wk->heapID );

		wk->mm = GFL_MSG_Create( GFL_MSG_LOAD_NORMAL, ARCID_MESSAGE, NARC_message_d_matsu_dat, wk->heapID );
		wk->strbuf = GFL_STR_CreateBuffer( 1024, wk->heapID );
		for(i = 0; i < ENTRY_MAX; i++){
			wk->strbuf_entry[i] = GFL_STR_CreateBuffer( 64, wk->heapID );
		}

		wk->tcbl = GFL_TCBL_Init( wk->heapID, wk->heapID, 4, 32 );

		GFL_MSGSYS_SetLangID( 1 );	//JPN_KANJI
	}

	return GFL_PROC_RES_FINISH;
}
//--------------------------------------------------------------------------
/**
 * PROC Main
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcMain( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;
	BOOL ret = 0;
	int i;
	
	GFL_TCBL_Main( wk->tcbl );
	PRINTSYS_QUE_Main( wk->printQue );
	for(i = 0; i < ENTRY_MAX+1; i++){
		if( PRINT_UTIL_Trans( wk->drawwin[i].printUtil, wk->printQue ) == TRUE){
			//return GFL_PROC_RES_CONTINUE;
		}
		else{
			if(wk->drawwin[i].message_req == TRUE){
	//			GFL_BMP_Clear( wk->bmp, 0xff );
				GFL_BMPWIN_TransVramCharacter( wk->drawwin[i].win );
				wk->drawwin[i].message_req = FALSE;
			}
		}
	}
	
	switch(wk->debug_mode){
	case 0:		//赤外線マッチング
		ret = DebugMatsuda_IrcMatch(wk);
		if(ret == TRUE){
			wk->seq = 0;
			wk->debug_mode = 1;
		}
		break;
	case 1:		//ワイヤレス通信
		ret = DebugMatsuda_Wireless(wk);
		break;
	default:
		ret = TRUE;
	}
	
	if(ret == TRUE){
//		return GFL_PROC_RES_FINISH;
	}

	return GFL_PROC_RES_CONTINUE;
}
//--------------------------------------------------------------------------
/**
 * PROC Quit
 */
//--------------------------------------------------------------------------
static GFL_PROC_RESULT DebugMatsudaMainProcEnd( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
	D_MATSU_WORK* wk = mywk;

	GFL_ARC_CloseDataHandle(wk->arcHandle);
	FontDataMan_Delete(wk->fontHandle);
	GFL_TCBL_Exit(wk->tcbl);
	
	GFL_BG_Exit();
	GFL_BMPWIN_Exit();

	GFL_PROC_FreeWork(proc);
	GFL_HEAP_DeleteHeap(HEAPID_MATSUDA_DEBUG);
	
	return GFL_PROC_RES_FINISH;
}



//==============================================================================
//	
//==============================================================================
static const NetRecvFuncTable _CommPacketTbl[] = {
//    {_RecvMoveData,        GFL_NET_COMMAND_SIZE( 0 ), NULL},    ///NET_CMD_MOVE
//    {_RecvHugeData,        GFL_NET_COMMAND_SIZE( 0 ), NULL},    ///NET_CMD_HUGE
//    {_RecvKeyData,         GFL_NET_COMMAND_SIZE( 4 ), NULL},    ///NET_CMD_KEY
    {_RecvMoveData,         NULL},    ///NET_CMD_MOVE
    {_RecvHugeData,         _RecvHugeBuffer},    ///NET_CMD_HUGE
    {_RecvKeyData,          NULL},    ///NET_CMD_KEY
    {_RecvChildProfile,     NULL},    ///NET_CMD_CHILD_PROFILE
};

enum{
	NET_CMD_MOVE = GFL_NET_CMD_COMMAND_MAX,
	NET_CMD_HUGE,
	NET_CMD_KEY,
	NET_CMD_CHILD_PROFILE,
};

#define _MAXNUM   (4)         // 最大接続人数
#define _MAXSIZE  (32)        // 最大送信バイト数
#define _BCON_GET_NUM (16)    // 最大ビーコン収集数


static const GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,    ///< ハードで接続した時に呼ばれる
    NULL,    ///< ネゴシエーション完了時にコール
    NULL,   // ユーザー同士が交換するデータのポインタ取得関数
    NULL,   // ユーザー同士が交換するデータのサイズ取得関数
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    _endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
    0x532,//0x444,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    HEAPID_IRC,   //※check　赤外線通信用にcreateされるHEAPID
    GFL_WICON_POSX, GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    TRUE,     // CRC計算
    FALSE,     // MP通信＝親子型通信モードかどうか
    GFL_NET_TYPE_WIRELESS,//GFL_NET_TYPE_IRC,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
    WB_NET_DEBUG_MATSUDA_SERVICEID,  //GameServiceID
};

//--------------------------------------------------------------
/**
 * @brief   赤外線通信メイン
 *
 * @param   wk		
 * @param   irc		
 *
 * @retval  TRUE:子のエントリーが増えた or 親のプロフィールを受け取った
 */
//--------------------------------------------------------------
static BOOL DM_IrcMain(D_MATSU_WORK *wk, DM_IRC_WORK *irc)
{
	BOOL ret = 0;
	
	if(irc->initialize == TRUE){
		IRC_Move();
	}
	
	if(irc->connect == TRUE){
		if(IRC_IsConnect() == FALSE){
			irc->connect = FALSE;
			OS_TPrintf("切断された\n");
			irc->seq = 0;
		}
	}
	else{
		if(IRC_IsConnect() == TRUE){
			irc->connect = TRUE;
			irc->connect_wait = 0;
			OS_TPrintf("接続した\n");
		}
		else{
			if(wk->oya == MY_CHILD){
				irc->connect_wait++;
				if(irc->connect_wait > 60){
					irc->connect_wait = 0;
					irc->seq = 0;
					OS_TPrintf("なかなか接続しないので再度初期化して接続に行く\n");
				}
			}
		}
	}
	
	if(irc->success == TRUE && irc->connect == FALSE){
		irc->success = FALSE;
		return TRUE;
	}
	
	switch( irc->seq ){
	case 0:
		OS_TPrintf("IRC初期化\n");
		GFL_STD_MemClear(&wk->irc, sizeof(DM_IRC_WORK));
		IRC_Init();
		IRC_SetRecvCallback(DM_IRC_ReceiveCallback);
		if(wk->oya == MY_CHILD){	//子が通信を始める。親は常に受信側
			IRC_Connect();
		}
		irc->initialize = TRUE;
		irc->seq++;
		break;
	case 1:
		if(irc->connect == TRUE){
			//if(wk->oya == MY_PARENT){	//受信側である親から通信開始
			if(IRC_IsSender() == FALSE){	//受信側は通常、親
				IRC_Send((u8*)&wk->my_profile, sizeof(IRC_MATCH_ENTRY_PARAM), CMD_PARENT_PROFILE, 0);
			}
			irc->seq++;
		}
		break;
	case 2:
		if(irc->success == TRUE && irc->connect == FALSE){
			irc->seq = 0;
		}
		break;
	}
	
	return ret;
}

//--------------------------------------------------------------
/**
 * @brief   ワイヤレス通信テスト
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_IrcMatch(D_MATSU_WORK *wk)
{
	BOOL ret, irc_ret = 0;
	int msg_id;
	
	GF_ASSERT(wk);

	switch(wk->seq){
	case 0:
		{
			OSOwnerInfo info;
			int i;
			
			OS_GetOwnerInfo( &info );
			for(i = 0; i < 11; i++){
				wk->my_profile.name[i] = info.nickName[i];
			}
			wk->my_profile.name_len = info.nickNameLength;
			OS_TPrintf("名前の長さ=%d\n", wk->my_profile.name_len);
			OS_GetMacAddress(wk->my_profile.parent_MacAddress);
		}

		OS_TPrintf("親＝Aボタン、　子＝Xボタン\n");
		GFL_MSG_GetString( wk->mm, DM_MSG_MATCH001, wk->strbuf );
		wk->printStream = PRINTSYS_PrintStream( wk->drawwin[ENTRY_MAX].win, 0, 0,
						wk->strbuf, wk->fontHandle, 0, wk->tcbl, 0, wk->heapID, 0xff );

		wk->seq++;
		break;
	case 1:
		if( PRINTSYS_PrintStreamGetState(wk->printStream) == PRINTSTREAM_STATE_DONE ){
			PRINTSYS_PrintStreamDelete( wk->printStream );
			wk->seq++;
		}
		break;

	case 2:
		switch(GFL_UI_KEY_GetTrg()){
		case PAD_BUTTON_A:
			wk->oya = MY_PARENT;
			wk->my_profile.oya_child = MY_PARENT;
			msg_id = DM_MSG_MATCH002;
			GFL_MSG_GetString( wk->mm, msg_id, wk->strbuf );
			Local_MessagePut(wk, ENTRY_MAX, wk->strbuf, 0, 0);
			wk->seq++;
			break;
		case PAD_BUTTON_X:
			wk->oya = MY_CHILD;
			wk->my_profile.oya_child = MY_CHILD;
			msg_id = DM_MSG_MATCH003;
			GFL_MSG_GetString( wk->mm, msg_id, wk->strbuf );
			Local_MessagePut(wk, ENTRY_MAX, wk->strbuf, 0, 0);
			wk->seq++;
			break;
		default:
			break;
		}
		break;
	case 3:
		if(wk->oya == MY_PARENT && ((GFL_UI_KEY_GetTrg() & PAD_BUTTON_START) || wk->connect_max == TRUE)){
			IRC_Shutdown();
			wk->seq++;
			break;
		}
		
		if((wk->oya == MY_CHILD && wk->irc.success == FALSE) 
				|| (wk->oya == MY_PARENT && wk->connect_max == FALSE)){
			irc_ret = DM_IrcMain(wk, &wk->irc);
			if(irc_ret == TRUE && wk->entry_num >= CHILD_MAX){
				wk->connect_max = TRUE;
			}
		}
		
		if(irc_ret == TRUE){
			if(wk->oya == MY_CHILD){
				OS_TPrintf("子：ワイヤレス通信開始\n");
				GFL_STR_SetStringCodeOrderLength(wk->strbuf, wk->parent_profile.name, wk->parent_profile.name_len+1);
				Local_MessagePut(wk, 0, wk->strbuf, 0, 0);
				wk->seq++;
			}
			else{
				OS_TPrintf("エントリーが増えた\n");
				GFL_STR_SetStringCodeOrderLength(wk->strbuf, wk->child_profile[wk->entry_num - 1].name, wk->child_profile[wk->entry_num - 1].name_len+1);
				Local_MessagePut(wk, wk->entry_num, wk->strbuf, 0, 0);
			}
			break;
		}
		break;
	default:
		return TRUE;	//ワイヤレス通信処理へ
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   ワイヤレス通信：親のMACアドレスを指定して繋げる
 *
 * @param   wk		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_Wireless(D_MATSU_WORK *wk)
{
	BOOL ret;
	
	GF_ASSERT(wk);

	switch( wk->seq ){
	case 0:
		if(wk->oya == MY_PARENT){
			GFL_MSG_GetString( wk->mm, DM_MSG_MATCH006, wk->strbuf );
			Local_MessagePut(wk, ENTRY_MAX, wk->strbuf, 0, 0);
		}
		else{
			GFL_MSG_GetString( wk->mm, DM_MSG_MATCH004, wk->strbuf );
			Local_MessagePut(wk, ENTRY_MAX, wk->strbuf, 0, 0);
		}
		
		{
			GFLNetInitializeStruct net_ini_data;
			
			net_ini_data = aGFLNetInit;
			GFL_NET_Init(&net_ini_data, _initCallBack, wk);	//通信初期化
		}
		wk->seq++;
		break;
	case 1:
		if(GFL_NET_IsInit() == TRUE){	//初期化終了待ち
			wk->seq++;
		}
		break;
	case 2:
		if(wk->oya == MY_PARENT){
			GFL_NET_InitServer();
			OS_TPrintf("親機になってワイヤレス開始\n");
		}
		else{
			GFL_NET_InitClientAndConnectToParent(wk->parent_profile.parent_MacAddress);
			OS_TPrintf("子機になってワイヤレス開始\n");
		}
		wk->seq++;
		break;
	case 3:	//ワイヤレス接続待ち
		//※check　現状接続したか判断する手段がないので、暫定的にタイマーで繋がった事にする
		wk->timer++;
		if(wk->timer > 30){
			wk->timer = 0;
			//ネゴシエーション開始
			OS_TPrintf("ネゴシエーション送信\n");
			if(wk->oya == MY_CHILD){
				if(GFL_NET_HANDLE_RequestNegotiation() == TRUE){
					wk->seq++;
				}
			}
			else{
				wk->seq++;	//親機は最後にネゴシエーション送信をする
			}
		}
		break;
	case 4:	//ネゴシエーション完了待ち
		if(wk->oya == MY_PARENT){
			if(GFL_NET_HANDLE_GetNumNegotiation() >= wk->entry_num){
				wk->timer++;
				if(wk->timer > 30){
					wk->timer = 0;
					if(GFL_NET_HANDLE_RequestNegotiation() == TRUE){
						OS_TPrintf("子のネゴシエーション揃った\n");
					 //   wk->connect_ok = TRUE;
					    wk->seq++;
					}
				}
			}
		}
		else{
			wk->seq++;
		}
		break;

	case 5:
		if(wk->oya == MY_PARENT){
			if(GFL_NET_HANDLE_GetNumNegotiation() >= wk->entry_num + 1){
				OS_TPrintf("親のネゴシエーションも完了\n");
				wk->connect_ok = TRUE;
				wk->seq++;
			}
		}
		break;
	case 6:		//タイミングコマンド発行
//		wk->seq = 6;
//		break;
		if(wk->oya == MY_PARENT){
			wk->timer++;
			if(wk->timer < 30){
				break;
			}
			wk->timer = 0;
		}
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15);
		wk->seq++;
		break;
	case 7:
		if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),15) == TRUE){
			OS_TPrintf("タイミング取り成功\n");
			OS_TPrintf("接続人数 = %d\n", GFL_NET_GetConnectNum());
			GFL_MSG_GetString( wk->mm, DM_MSG_MATCH008, wk->strbuf );
			Local_MessagePut(wk, ENTRY_MAX, wk->strbuf, 0, 0);
			wk->seq++;
		}
		break;
	case 8:
		if(wk->oya == MY_PARENT){
			if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_CHILD_PROFILE, sizeof(IRC_MATCH_ENTRY_PARAM) * wk->entry_num, wk->child_profile) == TRUE){
				OS_TPrintf("子のプロフィールを全ての子に送信した\n");
				wk->seq++;
			}
		}
		else{
			wk->seq++;
		}
		break;
		
	case 9:	//キーを送信しあう
		ret = -1;
		wk->send_key = GFL_UI_KEY_GetTrg();
		if(wk->send_key & PAD_KEY_UP){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_DOWN){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_LEFT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		if(wk->send_key & PAD_KEY_RIGHT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &wk->send_key);
		}
		
		if(wk->send_key & PAD_BUTTON_B){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_HUGE, 0x1000, &wk->huge_data);
		}
		
		if(ret == FALSE){
			OS_TPrintf("送信失敗\n");
		}
		
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
//			wk->seq++;
		}
		break;
	case 10:	//通信終了
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL)){
			//GFL_NET_Exit(_endCallBack);	//通信終了
			wk->seq++;
		}
		break;
	case 11:	//通信終了待ち
		if(wk->connect_ok == FALSE){
			wk->seq++;
		}
		break;
	default:
		return TRUE;
	}
	
	return FALSE;
}

//--------------------------------------------------------------
/**
 * @brief   IRC受信時に呼ばれるコールバック
 *
 * @param   data		
 * @param   size		
 * @param   command		
 * @param   value		
 */
//--------------------------------------------------------------
static void DM_IRC_ReceiveCallback(u8 *data, u8 size, u8 command, u8 value)
{
	D_MATSU_WORK *wk = glb_d_matsu;
	int send_id;
	IRC_MATCH_ENTRY_PARAM *recv = (IRC_MATCH_ENTRY_PARAM *)data;
	
	//赤外線専用のシステムコマンド解釈
	switch(command){
	case 0xf4:		//赤外線ライブラリの切断コマンド
		OS_TPrintf("IRC切断コマンドを受信\n");
		return;
	}
	
	//子から送信してくるデータ
	switch(command){
	case CMD_CHILD_PROFILE:		//子のプロフィール受信
		if(wk->my_profile.oya_child == MY_CHILD){
			OS_TPrintf("自分も相手も子である\n");
			IRC_Shutdown();
			return;
		}
		Local_EntryAdd(wk, recv);
		IRC_Send(NULL, 0, CMD_PARENT_SUCCESS, 0);
		wk->irc.success = TRUE;
		break;
	}
	
	//親から送信してくるデータ
	switch(command){
	case CMD_PARENT_PROFILE:	//親のプロフィール受信
		Local_ParentProfileSet(wk, recv);
		IRC_Send((u8*)&wk->my_profile, sizeof(IRC_MATCH_ENTRY_PARAM), CMD_CHILD_PROFILE, 0);
		break;
	case CMD_PARENT_SUCCESS:		//親から受け取ったと返事を受信
		IRC_Shutdown();
		wk->irc.success = TRUE;
		break;
	}
}

//--------------------------------------------------------------
/**
 * @brief   子のプロフィールを登録
 *
 * @param   wk		
 * @param   profile		
 */
//--------------------------------------------------------------
static void Local_EntryAdd(D_MATSU_WORK *wk, IRC_MATCH_ENTRY_PARAM *profile)
{
	GF_ASSERT(wk->entry_num < CHILD_MAX);
	GFL_STD_MemCopy(profile, &wk->child_profile[wk->entry_num], sizeof(IRC_MATCH_ENTRY_PARAM));
	wk->entry_num++;
	OS_TPrintf("子をエントリー:%d人目\n", wk->entry_num);
}

//--------------------------------------------------------------
/**
 * @brief   親のプロフィールを登録
 *
 * @param   wk		
 * @param   profile		
 */
//--------------------------------------------------------------
static void Local_ParentProfileSet(D_MATSU_WORK *wk, IRC_MATCH_ENTRY_PARAM *profile)
{
	GFL_STD_MemCopy(profile, &wk->parent_profile, sizeof(IRC_MATCH_ENTRY_PARAM));
	OS_TPrintf("親のプロフィール登録\n");
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   wk			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void Local_MessagePut(D_MATSU_WORK *wk, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( wk->drawwin[win_index].bmp, 0xff );
	PRINTSYS_PrintQue(wk->printQue, wk->drawwin[win_index].bmp, x, y, strbuf, wk->fontHandle);
	wk->drawwin[win_index].message_req = TRUE;
}






//==============================================================================
//	
//==============================================================================
typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { WB_NET_DEBUG_MATSUDA_SERVICEID };

//--------------------------------------------------------------
/**
 * @brief   ビーコンデータ取得関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void* _netBeaconGetFunc(void* pWork)
{
	return &_testBeacon;
}

///< ビーコンデータサイズ取得関数
static int _netBeaconGetSizeFunc(void* pWork)
{
	return sizeof(_testBeacon);
}

///< ビーコンデータ取得関数
static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)
{
    if(myNo != beaconNo){
        return FALSE;
    }
    return TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   初期化完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _initCallBack(void* pWork)
{
	OS_TPrintf("初期化完了コールバックが呼び出された\n");
	return;
}

//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   接続完了コールバック
 * @param   pCtl    デバッグワーク
 * @retval  none
 */
//--------------------------------------------------------------
static void _connectCallBack(void* pWork)
{
	D_MATSU_WORK *wk = pWork;
	
    OS_TPrintf("ネゴシエーション完了\n");
    wk->connect_ok = TRUE;
}

static void _endCallBack(void* pWork)
{
	D_MATSU_WORK *wk = pWork;

    NET_PRINT("endCallBack終了\n");
    wk->connect_ok = FALSE;
}


//==============================================================================
//	
//==============================================================================
//--------------------------------------------------------------
/**
 * @brief   移動コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void _RecvMoveData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
}

//--------------------------------------------------------------
/**
 * @brief   巨大データコマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------

static void _RecvHugeData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	D_MATSU_WORK *wk = pWork;
	int i;
	const u8 *data = pData;
	u16 cs = 0;
	
	OS_TPrintf("巨大データ受信 netid = %d, size = 0x%x\n", netID, size);
	for(i = 0; i < size; i++){
		cs += data[i];
		//OS_TPrintf("%d, ", data[i]);
	//	if(i % 32 == 0){
	//		OS_TPrintf("\n");
	//	}
	}
	OS_TPrintf("巨大データ受信チェックサム=0x%x\n", cs);
}

static u8 * _RecvHugeBuffer(int netID, void* pWork, int size)
{
	D_MATSU_WORK *wk = pWork;
	return wk->receive_huge_data[netID - 1];
}


//--------------------------------------------------------------
/**
 * @brief   キー情報コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _RecvKeyData(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	D_MATSU_WORK *wk = pWork;
	int key_data;
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	
	key_data = *((int *)pData);
	OS_TPrintf("データ受信：netID=%d, size=%d, data=%d\n", netID, size, key_data);
	if(key_data & PAD_KEY_UP){
		OS_TPrintf("受信：上\n");
		wk->msg_pos_y[netID] -= 2;
	}
	if(key_data & PAD_KEY_DOWN){
		OS_TPrintf("受信：下\n");
		wk->msg_pos_y[netID] += 2;
	}
	if(key_data & PAD_KEY_LEFT){
		OS_TPrintf("受信：左\n");
		wk->msg_pos_x[netID] -= 2;
	}
	if(key_data & PAD_KEY_RIGHT){
		OS_TPrintf("受信：右\n");
		wk->msg_pos_x[netID] += 2;
	}
	
	if(netID == 0){
		if(wk->oya == MY_PARENT){
			GFL_STR_SetStringCodeOrderLength(wk->strbuf_entry[netID], wk->my_profile.name, wk->my_profile.name_len+1);
		}
		else{
			GFL_STR_SetStringCodeOrderLength(wk->strbuf_entry[netID], wk->parent_profile.name, wk->parent_profile.name_len+1);
		}
	}
	else{
		GFL_STR_SetStringCodeOrderLength(wk->strbuf_entry[netID], wk->child_profile[netID-1].name, wk->child_profile[netID-1].name_len+1);
	}
	Local_MessagePut(wk, netID, wk->strbuf_entry[netID], wk->msg_pos_x[netID], wk->msg_pos_y[netID]);
}

//--------------------------------------------------------------
/**
 * @brief   キー情報コマンド受信関数
 * @param   netID      送ってきたID
 * @param   size       パケットサイズ
 * @param   pData      データ
 * @param   pWork      ワークエリア
 * @param   pHandle    受け取る側の通信ハンドル
 * @retval  none  
 */
//--------------------------------------------------------------
static void _RecvChildProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	D_MATSU_WORK *wk = pWork;
	const IRC_MATCH_ENTRY_PARAM *recv = pData;
	int i;
	
	OS_TPrintf("RecvChildProfile, netID=%d, size=%d\n", netID, size);
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	
	if(wk->oya == MY_PARENT){
		//親の場合は子は既に描画されているので無視してよい
		//ただ、いいタイミングなので自分自身の情報を描画しておく
		GFL_STR_SetStringCodeOrderLength(wk->strbuf_entry[0], wk->my_profile.name, wk->my_profile.name_len+1);
		Local_MessagePut(wk, 0, wk->strbuf_entry[0], 0, 0);
		return;
	}

	for(i = 0; i < CHILD_MAX; i++){
		if(recv[i].name_len == 0){
			OS_TPrintf("子プロフィール：名前の長さが0\n");
			break;
		}
		OS_TPrintf("子の名前を描画 %d\n", i);
		GFL_STR_SetStringCodeOrderLength(wk->strbuf_entry[i], recv[i].name, recv[i].name_len+1);
		Local_MessagePut(wk, 1+i, wk->strbuf_entry[i], 0, 0);
		GFL_STD_MemCopy(&recv[i], &wk->child_profile[i], sizeof(IRC_MATCH_ENTRY_PARAM));
	}
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA DebugMatsudaIrcMatchProcData = {
	DebugMatsudaMainProcInit,
	DebugMatsudaMainProcMain,
	DebugMatsudaMainProcEnd,
};


