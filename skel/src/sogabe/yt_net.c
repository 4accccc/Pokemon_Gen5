//============================================================================================
/**
 * @file	yt_net.c
 * @brief	DS版ヨッシーのたまご ネットワーク関連のプログラム
 * @author	ohno
 * @date	2007.03.05
 */
//============================================================================================


#include "gflib.h"
#include "procsys.h"
#include "ui.h"

#include "main.h"
#include "yt_common.h"
#include "yt_net.h"
#include "fallchr.h"
#include "player.h"
#include "../ohno/fatal_error.h"
#include "net_icondata.h"

#if GFL_NET_WIFI //GFL_NET_WIFI WIFI通信テスト
// この二つのデータが「ともだちこーど」になります。本来はセーブする必要があります
#define _BCON_GET_NUM  (1)
static DWCUserData _testUserData;
static DWCFriendData _testFriendData[_BCON_GET_NUM];
#endif

// static 関数定義
static void _recvCLACTPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTPlayerAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTDelete(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvYossyAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvCLACTAnimCreate(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvScreenMake(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvScreenMakeRot(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvPlaySe(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvPlayGameover(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);
static void _recvPlayEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle);

//通信用構造体
struct _NET_PARAM
{
    GFL_NETHANDLE* pNetHandle[2];  //通信用ハンドル 親と子のハンドルを管理するから２個必要
    GFL_CLWK* pCLACT[YT_CLACT_MAX];  // 描画してるclact
    PLAYER_PARAM* pp;
    u32 seq;
    BOOL bGameStart;
};


#define _BCON_GET_NUM  (1)

typedef struct{
    int gameNo;   ///< ゲーム種類
} _testBeaconStruct;

static _testBeaconStruct _testBeacon = { 1 };


static void* _netBeaconGetFunc(void)    ///< ビーコンデータ取得関数
{
    return &_testBeacon;
}

static int _netBeaconGetSizeFunc(void)    ///< ビーコンデータサイズ取得関数
{
    return sizeof(_testBeacon);
}

#define SSID "GF_ORG01"

static u8* _netGetSSID(void)
{
    return (u8*)SSID;
}

static BOOL _netBeaconCompFunc(GameServiceID myNo,GameServiceID beaconNo)    ///< ビーコンデータ取得関数
{
    OS_TPrintf("比較してます%d\n",beaconNo);
    return TRUE;
}

#define _MAXNUM (2)     // 接続最大
#define _MAXSIZE (200)  // 送信最大サイズ

//普通のワイヤレス通信


/// バトル専用通信コマンドの定義。
enum CommCommandBattle_e {
    YT_NET_COMMAND_ANIM = GFL_NET_CMD_COMMAND_MAX,  ///< アニメコマンド送信  b
    YT_NET_COMMAND_POS,                             ///< 位置コマンド送信c
    YT_NET_COMMAND_PLAYERANIM,            ///< プレーヤーのアニメーション情報の送信d
    YT_NET_COMMAND_DELETE,            /// CLACTを消す命令e
    YT_NET_COMMAND_YOSSYANIM,     ///ヨッシーのアニメーションf
    YT_NET_COMMAND_ANIM_CREATE,   ///< アニメーション作成10
    TY_NET_COMMAND_SCREENMAKE,
    TY_NET_COMMAND_SCREENMAKE_ROT,
    YT_NET_COMMAND_PLAY_SE,
    YT_NET_COMMAND_GAMERESULT,     ///< ゲーム結果
    YT_NET_COMMAND_END,
    //------------------------------------------------ここまで
    YT_NET_COMMAND_MAX   // 終端--------------これは移動させないでください
};

//----------------------------------------------------------------------------
// @brief	アニメーション設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
    u16 anmseq;
} COMM_ANIM_ST;


//----------------------------------------------------------------------------
// @brief	勝ち負けを送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 player;
    u8 bWin;
} COMM_RESULT_ST;


//----------------------------------------------------------------------------
// @brief	スクリーン設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 player_no;
    u8 old_line_no;
    u8 new_line_no;
} COMM_SCREENMAKE_ST;

//----------------------------------------------------------------------------
// @brief	スクリーン設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 player_no;
    u8 line_no;
    u8 flag;
} COMM_SCREENMAKE_ROT_ST;

//----------------------------------------------------------------------------
// @brief	アニメーション作成を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
    u16 type;
} COMM_ANIM_CREATE_ST;

//----------------------------------------------------------------------------
// @brief	プレイヤーアニメーション設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u16 anm_no;
    u8 player_no;
    u8 pat_no;
    u8 line_no;
    u8 rot;
} COMM_PLAYER_ANIM_ST;

//----------------------------------------------------------------------------
// @brief	位置設定を送信する
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
    s16 x;
    s16 y;
} COMM_POS_ST;

//----------------------------------------------------------------------------
// @brief	ヨッシーアニメ送信
//-----------------------------------------------------------------------------
typedef struct {
    u8 count;
    u8 posx;
    u8 posy;
} COMM_YOSSY_ANIM_ST;

//-----------------------------------------------------------------------------
// @brief	消す命令
//-----------------------------------------------------------------------------
typedef struct {
    u8 clactNo;
} COMM_DELETE_ST;



// ローカル通信テーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    {_recvCLACTAnim, GFL_NET_COMMAND_SIZE(sizeof(COMM_ANIM_ST)), NULL},
    {_recvCLACTPos, GFL_NET_COMMAND_SIZE(sizeof(COMM_POS_ST)), NULL},
    {_recvCLACTPlayerAnim, GFL_NET_COMMAND_SIZE(sizeof(COMM_PLAYER_ANIM_ST)), NULL},
    {_recvCLACTDelete, GFL_NET_COMMAND_SIZE(sizeof(COMM_DELETE_ST)), NULL},
    {_recvYossyAnim, GFL_NET_COMMAND_SIZE(sizeof(COMM_YOSSY_ANIM_ST)), NULL},
    {_recvCLACTAnimCreate, GFL_NET_COMMAND_SIZE(sizeof(COMM_ANIM_CREATE_ST)), NULL},
    {_recvScreenMake, GFL_NET_COMMAND_SIZE(sizeof(COMM_SCREENMAKE_ST)), NULL},
    {_recvScreenMakeRot, GFL_NET_COMMAND_SIZE(sizeof(COMM_SCREENMAKE_ROT_ST)), NULL},
    {_recvPlaySe, GFL_NET_COMMAND_SIZE(sizeof(int)), NULL},
    {_recvPlayGameover, GFL_NET_COMMAND_SIZE(sizeof(COMM_RESULT_ST)), NULL},
    {_recvPlayEnd, GFL_NET_COMMAND_SIZE(0), NULL},
    
};



// 通信初期化構造体  WIFI兼用
static GFLNetInitializeStruct aGFLNetInit = {
    _CommPacketTbl,  // 受信関数テーブル
    NELEMS(_CommPacketTbl), // 受信テーブル要素数
    NULL,   // ワークポインタ
    _netBeaconGetFunc,  // Infomationデータ取得関数
    _netBeaconGetSizeFunc,  // Infomationデータサイズ取得関数
    _netBeaconGetFunc,  // ビーコンデータ取得関数
    _netBeaconGetSizeFunc,  // ビーコンデータサイズ取得関数
    _netBeaconCompFunc,  // ビーコンのサービスを比較して繋いで良いかどうか判断する
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    NULL,  // 通信切断時に呼ばれる関数
    NULL, //_netAutoParent,    //自動接続の際に親になる場合に呼ばれる関数
    NET_ICONDATA_GetTableData,   // 通信アイコンのファイルARCテーブルを返す関数
    NET_ICONDATA_GetNoBuff,      // 通信アイコンのファイルARCの番号を返す関数
#if GFL_NET_WIFI
    NULL,  // wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL,  // wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    &_testFriendData[0],  // DWC形式の友達リスト	
    &_testUserData,  // DWCのユーザデータ（自分のデータ）
#endif
    _netGetSSID,  // 親子接続時に認証する為のバイト列  
    1,  //gsid
    0,  //ggid  DP=0x333,RANGER=0x178,WII=0x346
    GFL_HEAPID_APP,  //元になるheapid
    HEAPID_NETWORK,  //通信用にcreateされるHEAPID
    HEAPID_WIFI,  //wifi用にcreateされるHEAPID
    GFL_WICON_POSX,GFL_WICON_POSY,        // 通信アイコンXY位置
    _MAXNUM,     // 最大接続人数
    _MAXSIZE,  //最大送信バイト数
    _BCON_GET_NUM,    // 最大ビーコン収集数
    TRUE,  //CRCを自動計算するかどうか TRUEの場合すべて計算する
    FALSE,     // MP通信＝親子型通信モードかどうか
    FALSE,  //wifi通信を行うかどうか
    TRUE,     // 親が再度初期化した場合、つながらないようにする場合TRUE
};



enum{
    _CONNECT_WIFI,
    _CONNECTING_WIFI,
    _WIFI_MATCHING,
    _INIT_WAIT_PARENT,
    _INIT_WAIT_CHILD,
    _SEARCH_CHILD,
    _CONNECT_WAIT,
    _NEGO_START,
    _TIM_START,
    _TIM_OK,
    _LOOP,
};


#define _TEST_TIMING (14)

static void _seqChange(NET_PARAM* pNet, int no)
{
    NET_PRINT("ytst change %d\n",no);
    pNet->seq = no;
}


#define _SEQCHANGE(no)   _seqChange(pNet,no)



//----------------------------------------------------------------------------
/**
 *	@brief	位置命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTPos(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_POS_ST* pPos = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;
	GFL_CLACTPOS	pos;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    //OS_TPrintf("testPos %d %d %d\n",pPos->clactNo, pPos->x, pPos->y);
    pos.x = pPos->x;
    pos.y = pPos->y;
    if(pNet->pCLACT[pPos->clactNo] != NULL){
        GFL_CLACT_WK_SetWldPos(pNet->pCLACT[pPos->clactNo],&pos);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	SE命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvPlaySe(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const int* pSe_no = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    GFL_SOUND_PlaySE(*pSe_no);
}

//----------------------------------------------------------------------------
/**
 *	@brief	GAMEOVER命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvPlayGameover(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_RESULT_ST* pResult = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){  // 送信者本人
        YT_PlayerSetWinLoseFlag(&gp->ps[pResult->player], pResult->bWin);
    }
    else{
        YT_PlayerSetWinLoseFlag(&gp->ps[pResult->player^1], !pResult->bWin);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	END命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvPlayEnd(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const int* pSe_no = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    // 子機が先に通信を終了させる
}



//----------------------------------------------------------------------------
/**
 *	@brief	消す命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTDelete(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_POS_ST* pPos = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;
	GFL_CLACTPOS	pos;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    if(pNet->pCLACT[pPos->clactNo] != NULL){
        GFL_AREAMAN_Release(gp->clact_area,pPos->clactNo,1);
        GFL_CLACT_WK_Remove(pNet->pCLACT[pPos->clactNo]);
        pNet->pCLACT[pPos->clactNo] = NULL;
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	ヨッシーのアニメーションが送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvYossyAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_YOSSY_ANIM_ST* pYossy = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;
    
    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    YT_YossyBirthAnimeTaskSet(gp,NULL,pYossy->posx,pYossy->posy,pYossy->count);
}

//----------------------------------------------------------------------------
/**
 *	@brief	スクリーン作成が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvScreenMake(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_SCREENMAKE_ST* pSMake = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    if(pNet->pp){
        YT_PlayerScreenMakeNetFunc(pNet->pp, pSMake->player_no, pSMake->old_line_no, pSMake->new_line_no);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	Rot時のスクリーン作成が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvScreenMakeRot(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_SCREENMAKE_ROT_ST* pSMake = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }
    if(pNet->pp){
        YT_PlayerRotateScreenMakeNetFunc(pNet->pp, pSMake->player_no, pSMake->line_no, pSMake->flag);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション作成命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTAnimCreate(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_ANIM_CREATE_ST* pAnim = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;


    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }

    if(pNet->pCLACT[pAnim->clactNo]==NULL){
        pNet->pCLACT[pAnim->clactNo] = YT_InitNetworkFallChr(gp,( netID - 1),pAnim->type,0);
        GF_ASSERT(pNet->pCLACT[pAnim->clactNo]);
    }
    else{
        GF_ASSERT(0);
    }

}


//----------------------------------------------------------------------------
/**
 *	@brief	アニメーション命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_ANIM_ST* pAnim = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;


    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }

    //OS_TPrintf("testAnim %d %d\n",pAnim->clactNo,pAnim->anmseq);

    GF_ASSERT(pAnim->clactNo < YT_CLACT_MAX);
    
    if(pNet->pCLACT[pAnim->clactNo]==NULL){
        GF_ASSERT(0);
    }
    else{
        GFL_CLACT_WK_SetAnmSeq(pNet->pCLACT[pAnim->clactNo],pAnim->anmseq);
    }

}

//----------------------------------------------------------------------------
/**
 *	@brief	プレイヤーアニメーション命令が送られてきた
 */
//-----------------------------------------------------------------------------

static void _recvCLACTPlayerAnim(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
    const COMM_PLAYER_ANIM_ST* pAnim = pData;
    GAME_PARAM* gp = pWork;
    NET_PARAM* pNet = gp->pNetParam;

    if(pNet->pNetHandle[1]!=pNetHandle){
        return;
    }
    if(netID==GFL_NET_GetNetID(pNet->pNetHandle[1])){
        return;
    }

    if(pNet->pp == NULL){
        pNet->pp = YT_InitPlayer(gp,pAnim->player_no,pAnim->player_no,FALSE);
    }
    else{
        YT_NetPlayerChrTrans(gp, pNet->pp, pAnim->player_no, pAnim->anm_no, pAnim->line_no, pAnim->rot, pAnim->pat_no);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	親機かどうかを判断する
 */
//-----------------------------------------------------------------------------

BOOL YT_NET_IsParent(NET_PARAM* pNet)
{
    return (pNet->pNetHandle[0] != NULL);
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信の場合はアニメーションＮＯを送信
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPlayerAnmReq(int player_no,int anm_no,int pat_no,u16 line_no,int rot,NET_PARAM* pNet)
{
    COMM_PLAYER_ANIM_ST CommPlayerAnmSt;

    if(pNet){
        CommPlayerAnmSt.player_no = player_no;
        CommPlayerAnmSt.anm_no = anm_no;
        CommPlayerAnmSt.line_no = line_no;
        CommPlayerAnmSt.rot = rot;
        CommPlayerAnmSt.pat_no = pat_no;

        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_PLAYERANIM, &CommPlayerAnmSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、ヨッシーアニメーションを送信
 */
//-----------------------------------------------------------------------------

void YT_NetSendYossyBirthAnime(u8 posx,u8 posy,u8 count,NET_PARAM* pNet)
{
    COMM_YOSSY_ANIM_ST sYossy;
    if(pNet){
        sYossy.posx = posx;
        sYossy.posy = posy;
        sYossy.count = count;

        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_YOSSYANIM, &sYossy);
    }
}




//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、アニメーションtypeを送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendAnmCreate(int clactno,u16 type,NET_PARAM* pNet)
{
    COMM_ANIM_CREATE_ST CommAnmCreateSt;
    if(pNet){

        CommAnmCreateSt.clactNo = clactno;
        CommAnmCreateSt.type = type;

        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_ANIM_CREATE, &CommAnmCreateSt);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、アニメーションナンバーを送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendAnmReq(int clactno,u16 anmseq,NET_PARAM* pNet)
{
    COMM_ANIM_ST CommAnmSt;
    if(pNet){

        CommAnmSt.clactNo = clactno;
        CommAnmSt.anmseq = anmseq;

        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_ANIM, &CommAnmSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、スクリーンMakeを送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPlayerScreenMake(int player_no, int old_line_no, int new_line_no,NET_PARAM* pNet)
{
    COMM_SCREENMAKE_ST CommScreenMakeSt;
    if(pNet){

        CommScreenMakeSt.player_no = player_no;
        CommScreenMakeSt.old_line_no = old_line_no;
        CommScreenMakeSt.new_line_no = new_line_no;

        GFL_NET_SendData(pNet->pNetHandle[1], TY_NET_COMMAND_SCREENMAKE, &CommScreenMakeSt);
    }

}

GLOBAL void	YT_PlayerRotateScreenMakeNetFunc(PLAYER_PARAM *pp, u8 player_no, u8 line_no, u8 flag);



//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、スクリーンMakeを送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPlayerRotateScreenMake(u8 player_no, u8 line_no, u8 flag,NET_PARAM* pNet)
{
    COMM_SCREENMAKE_ROT_ST CommScreenMakeRotSt;
    if(pNet){

        CommScreenMakeRotSt.player_no = player_no;
        CommScreenMakeRotSt.line_no = line_no;
        CommScreenMakeRotSt.flag = flag;

        GFL_NET_SendData(pNet->pNetHandle[1], TY_NET_COMMAND_SCREENMAKE_ROT, &CommScreenMakeRotSt);
    }

}


//----------------------------------------------------------------------------
/**
 *	@brief	CLACTを消す
 */
//-----------------------------------------------------------------------------

void YT_NET_DeleteReq(int clactno,NET_PARAM* pNet)
{
    COMM_DELETE_ST CommDelSt;

    if(pNet){

        CommDelSt.clactNo = clactno;
        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_DELETE, &CommDelSt);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	通信対戦時には、位置を送信する
 */
//-----------------------------------------------------------------------------

void YT_NET_SendPosReq(int clactno,s16 x, s16 y,NET_PARAM* pNet)
{
    COMM_POS_ST CommPosSt;

    if(pNet){
        CommPosSt.clactNo = clactno;
        CommPosSt.x = x;
        CommPosSt.y = y;
        //OS_TPrintf("位置送信 %d %d\n",x,y);
        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_POS, &CommPosSt);
    }
}

//----------------------------------------------------------------------------
/**
 *	@brief	通信のメインシーケンス
 */
//-----------------------------------------------------------------------------

BOOL YT_NET_Main(NET_PARAM* pNet)
{
    //OS_TPrintf("m %d\n", pNet->seq);
    switch(pNet->seq){
#if GFL_NET_WIFI
      case _CONNECT_WIFI:
        GFL_NET_WIFI_InitUserData(aGFLNetInit.myUserData);
        pNet->pNetHandle[1] = GFL_NET_CreateHandle();
        GFL_NET_WifiLogin( );    // wifiLogin マッチング待機へ移動
        _SEQCHANGE( _CONNECTING_WIFI );
        break;
      case _CONNECTING_WIFI:
        if( GFL_NET_IsWifiLobby() ){
            GFL_NET_StartRandomMatch(); // ランダムマッチ状態にする
            _SEQCHANGE( _WIFI_MATCHING );
        }
        break;
      case _WIFI_MATCHING:
        {
            int localConnect= GFL_NET_WIFI_GetLocalConnectNo();
            if(localConnect != -1){
                if(localConnect==0){
                    pNet->pNetHandle[0] = GFL_NET_CreateHandle();   // ハンドル作成
                    GFL_NET_InitServer(pNet->pNetHandle[0]);       // サーバーにする
                }
                _SEQCHANGE( _CONNECT_WAIT );
            }
        }
        break;
#endif
      case _INIT_WAIT_PARENT:
        if(GFL_NET_IsInit()){
            GFL_NET_InitServer(); // 自動接続
            _SEQCHANGE(_CONNECT_WAIT);
        }
        break;
      case _INIT_WAIT_CHILD:
        if(GFL_NET_IsInit()){
//            pNet->pNetHandle[1] = GFL_NET_CreateHandle();   // ハンドル作成
            GFL_NET_StartBeaconScan(); // 自動接続
            _SEQCHANGE( _SEARCH_CHILD );
        }
        break;
      case _SEARCH_CHILD:
        {
            u8* pData = GFL_NET_GetBeaconMacAddress(0);//ビーコンリストの0番目を得る
            if(pData){
                GFL_NET_ConnectToParent(pData);
                _SEQCHANGE( _CONNECT_WAIT );
            }
        }
        break;
      case _CONNECT_WAIT:
        _SEQCHANGE( _NEGO_START );
        break;
      case _NEGO_START:
        if(YT_NET_IsParent(pNet)){  //親機の場合
            if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
                if(GFL_NET_HANDLE_RequestNegotiation(  )){
                    _SEQCHANGE( _TIM_START );
                }
            }
        }
        else{
//            if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
                if(GFL_NET_HANDLE_RequestNegotiation()){
                    _SEQCHANGE( _TIM_START );
                }
  //          }
        }
        break;
      case _TIM_START:
        if(GFL_NET_HANDLE_GetNumNegotiation()==2){
            pNet->pNetHandle[1] = GFL_NET_HANDLE_GetCurrentHandle();   // ハンドル作成
            GFL_NET_TimingSyncStart(pNet->pNetHandle[1] , _TEST_TIMING);
            _SEQCHANGE( _TIM_OK );
        }
        break;
      case _TIM_OK:
        if(GFL_NET_IsTimingSync(pNet->pNetHandle[1] , _TEST_TIMING)){
            pNet->bGameStart = TRUE;
#if GFL_NET_WIFI
            GFL_NET_SetWifiBothNet(FALSE);  // WIFIはdefaultで非同期接続にしておく
#endif
            GFL_NET_SetNoChildErrorCheck(TRUE);
            GFL_NET_SetAutoErrorCheck(TRUE);
            _SEQCHANGE( _LOOP );
        }
        break;

    }
    return pNet->bGameStart;
}

//----------------------------------------------------------------------------
/**
 *	@brief	
 */
//-----------------------------------------------------------------------------

void YT_NET_Init(GAME_PARAM* gp, BOOL bParent)
{
    GFL_TCBSYS* tcbsys;
    NET_PARAM* pNet = GFL_HEAP_AllocMemory(gp->heapID, sizeof(NET_PARAM));

    GFL_STD_MemClear(pNet, sizeof(NET_PARAM));
    aGFLNetInit.pWork = gp;
    gp->pNetParam = pNet;

    //イクニューモンを使用する前に VRAMDをdisableにする必要があるのだが
    //VRAMDが何に使われていたのかがわからないと、消すことができない
    GX_DisableBankForLCDC(); // LCDCにVRAMDが割り当てられてるようなので打ち消す

    GFL_NET_Init(&aGFLNetInit);

    if(aGFLNetInit.bWiFi){
        if(bParent){
            pNet->seq = _CONNECT_WIFI;
        }
        else{
            pNet->seq = _CONNECT_WIFI;
        }
    }
    else{
        if(bParent){
            pNet->seq = _INIT_WAIT_PARENT;
        }
        else{
            pNet->seq = _INIT_WAIT_CHILD;
        }
    }

    
//	GFL_TCB_AddTask(gp->tcbsys,YT_NET_Main,pNet,1);
}


//----------------------------------------------------------------------------
/**
 *	@brief	SEを鳴らす(+通信で送る)
 */
//-----------------------------------------------------------------------------
void YT_NET_PlaySE(int se_no, NET_PARAM* pNet)
{
    GFL_SOUND_PlaySE(se_no);
    if(pNet){
        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_PLAY_SE, &se_no);
    }
}


//----------------------------------------------------------------------------
/**
 *	@brief	ゲーム結果を送る
 */
//-----------------------------------------------------------------------------
void YT_NET_SendGameResult(int player, int bWin, NET_PARAM* pNet)
{
    COMM_RESULT_ST sResult;
    if(pNet){
        sResult.player = player;
        sResult.bWin = bWin;
        GFL_NET_SendData(pNet->pNetHandle[1], YT_NET_COMMAND_GAMERESULT, &sResult);
    }
}


