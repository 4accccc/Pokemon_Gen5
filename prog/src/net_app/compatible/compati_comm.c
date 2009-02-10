//==============================================================================
/**
 * @file	compati_comm.c
 * @brief	相性チェック：通信
 * @author	matsuda
 * @date	2009.02.10(火)
 */
//==============================================================================
#include <gflib.h>
#include <procsys.h>
#include <tcbl.h>
#include "system\main.h"
#include "net\network_define.h"
#include "system\gfl_use.h"



//==============================================================================
//	データ
//==============================================================================
///通信コマンドテーブル
static const NetRecvFuncTable _CommPacketTbl[] = {
    {_RecvMoveData,         NULL},    ///NET_CMD_MOVE
    {_RecvHugeData,         NULL},    ///NET_CMD_HUGE
    {_RecvKeyData,          NULL},    ///NET_CMD_KEY
    {_RecvMyProfile,        NULL},    ///NET_CMD_MY_PROFILE
};

enum{
	NET_CMD_MOVE = GFL_NET_CMD_COMPATI_CHECK,
	NET_CMD_HUGE,
	NET_CMD_KEY,
	NET_CMD_MY_PROFILE,
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
    NULL,            // 普通のエラーが起こった場合 通信終了
    FatalError_Disp,  // 通信不能なエラーが起こった場合呼ばれる 切断するしかない
    _endCallBack,  // 通信切断時に呼ばれる関数
    NULL,  // オート接続で親になった場合
#if GFL_NET_WIFI
    NULL,     ///< wifi接続時に自分のデータをセーブする必要がある場合に呼ばれる関数
    NULL, ///< wifi接続時にフレンドコードの入れ替えを行う必要がある場合呼ばれる関数
    NULL,  ///< wifiフレンドリスト削除コールバック
    NULL,   ///< DWC形式の友達リスト	
    NULL,  ///< DWCのユーザデータ（自分のデータ）
    0,   ///< DWCへのHEAPサイズ
    TRUE,        ///< デバック用サーバにつなぐかどうか
#endif  //GFL_NET_WIFI
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
 * @param   cs		
 * @param   irc		
 *
 * @retval  TRUE:子のエントリーが増えた or 親のプロフィールを受け取った
 */
//--------------------------------------------------------------
static BOOL DM_IrcMain(COMPATI_SYS *cs, DM_IRC_WORK *irc)
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
			if(cs->oya == MY_CHILD){
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
		GFL_STD_MemClear(&cs->irc, sizeof(DM_IRC_WORK));
		IRC_Init();
		IRC_SetRecvCallback(DM_IRC_ReceiveCallback);
		if(cs->oya == MY_CHILD){	//子が通信を始める。親は常に受信側
			IRC_Connect();
		}
		irc->initialize = TRUE;
		irc->seq++;
		break;
	case 1:
		if(irc->connect == TRUE){
			//if(cs->oya == MY_PARENT){	//受信側である親から通信開始
			if(IRC_IsSender() == FALSE){	//受信側は通常、親
				IRC_Send((u8*)&cs->my_profile, sizeof(IRC_MATCH_ENTRY_PARAM), CMD_PARENT_PROFILE, 0);
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


static void _endCallBackSeq(void* vwk)
{
    COMPATI_SYS *cs = vwk;
    cs->seq++;
}

//--------------------------------------------------------------
/**
 * @brief   赤外線通信テスト
 *
 * @param   cs		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_IrcMatch(COMPATI_SYS *cs)
{
	BOOL ret, irc_ret = 0;
	int msg_id;
	
	GF_ASSERT(cs);

    OS_TPrintf("-- %d --\n",cs->seq);
	switch(cs->seq){
    case 0:
		{
			GFLNetInitializeStruct net_ini_data;
			
			net_ini_data = aGFLNetInit;
            net_ini_data.bNetType = GFL_NET_TYPE_IRC_WIRELESS;
			GFL_NET_Init(&net_ini_data, _initCallBack, cs);	//通信初期化
		}
		cs->seq++;
        break;
	case 1:
		{
			OSOwnerInfo info;
			int i;
			
			OS_GetOwnerInfo( &info );
			for(i = 0; i < 11; i++){
				cs->my_profile.name[i] = info.nickName[i];
			}
			cs->my_profile.name_len = info.nickNameLength;
			OS_TPrintf("名前の長さ=%d\n", cs->my_profile.name_len);
			OS_GetMacAddress(cs->my_profile.parent_MacAddress);
			OS_TPrintf("MAC: %d, %d, %d, %d, %d, %d\n", cs->my_profile.parent_MacAddress[0], cs->my_profile.parent_MacAddress[1], cs->my_profile.parent_MacAddress[2], cs->my_profile.parent_MacAddress[3], cs->my_profile.parent_MacAddress[4], cs->my_profile.parent_MacAddress[5]);
		}

		OS_TPrintf("親＝Aボタン、　子＝Xボタン\n");
		GFL_MSG_GetString( cs->mm, DM_MSG_MATCH001, cs->strbuf );
		cs->printStream = PRINTSYS_PrintStream( cs->drawwin[CCBMPWIN_MAX].win, 0, 0,
						cs->strbuf, cs->fontHandle, 0, cs->tcbl, 0, HEAPID_COMPATI, 0xff );

		cs->seq++;
		break;
	case 2:
		if( PRINTSYS_PrintStreamGetState(cs->printStream) == PRINTSTREAM_STATE_DONE ){
			PRINTSYS_PrintStreamDelete( cs->printStream );
			cs->seq++;
		}
		break;

	case 3:
		switch(GFL_UI_KEY_GetTrg()){
		case PAD_BUTTON_A:
			cs->oya = MY_PARENT;
			cs->my_profile.oya_child = MY_PARENT;
			msg_id = DM_MSG_MATCH002;
			GFL_MSG_GetString( cs->mm, msg_id, cs->strbuf );
			CCLocal_MessagePut(cs, CCBMPWIN_MAX, cs->strbuf, 0, 0);
			cs->seq++;
			break;
		case PAD_BUTTON_X:
			cs->oya = MY_CHILD;
			cs->my_profile.oya_child = MY_CHILD;
			msg_id = DM_MSG_MATCH003;
			GFL_MSG_GetString( cs->mm, msg_id, cs->strbuf );
			CCLocal_MessagePut(cs, CCBMPWIN_MAX, cs->strbuf, 0, 0);
			cs->seq++;
			break;
		default:
			break;
		}
		break;
	case 4:
		if(cs->oya == MY_PARENT && ((GFL_UI_KEY_GetTrg() & PAD_BUTTON_START) || cs->connect_max == TRUE)){
			IRC_Shutdown();
			cs->seq++;
			break;
		}
		
		if((cs->oya == MY_CHILD && cs->irc.success == FALSE) 
				|| (cs->oya == MY_PARENT && cs->connect_max == FALSE)){
			irc_ret = DM_IrcMain(cs, &cs->irc);
            OS_TPrintf("DM_IrcMain%d \n",irc_ret);
			if(irc_ret == TRUE && cs->entry_num >= CHILD_MAX){
				cs->connect_max = TRUE;
			}
		}
		
		if(irc_ret == TRUE){
			if(cs->oya == MY_CHILD){
				OS_TPrintf("子：ワイヤレス通信開始\n");
				GFL_STR_SetStringCodeOrderLength(cs->strbuf, cs->parent_profile.name, cs->parent_profile.name_len+1);
				CCLocal_MessagePut(cs, 0, cs->strbuf, 0, 0);
                GFL_NET_Exit(_endCallBackSeq);	//通信終了
				cs->seq++;
			}
			else{
				OS_TPrintf("エントリーが増えた\n");
				GFL_STR_SetStringCodeOrderLength(cs->strbuf, cs->child_profile[cs->entry_num - 1].name, cs->child_profile[cs->entry_num - 1].name_len+1);
				CCLocal_MessagePut(cs, cs->entry_num, cs->strbuf, 0, 0);
			}
			break;
		}
		break;
      case 5:
        //_endCallBackSeq待ち
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
 * @param   cs		
 *
 * @retval  TRUE:終了。　FALSE:処理継続中
 */
//--------------------------------------------------------------
static BOOL DebugMatsuda_Wireless(COMPATI_SYS *cs)
{
	BOOL ret;
	
	GF_ASSERT(cs);

	switch( cs->seq ){
	case 0:
		if(cs->oya == MY_PARENT){
			GFL_MSG_GetString( cs->mm, DM_MSG_MATCH006, cs->strbuf );
			CCLocal_MessagePut(cs, CCBMPWIN_MAX, cs->strbuf, 0, 0);
		}
		else{
			GFL_MSG_GetString( cs->mm, DM_MSG_MATCH004, cs->strbuf );
			CCLocal_MessagePut(cs, CCBMPWIN_MAX, cs->strbuf, 0, 0);
		}
		
		{
			GFLNetInitializeStruct net_ini_data;
			
			net_ini_data = aGFLNetInit;
            
			GFL_NET_Init(&net_ini_data, _initCallBack, cs);	//通信初期化
		}
		cs->seq++;
		break;
	case 1:
		if(GFL_NET_IsInit() == TRUE){	//初期化終了待ち
			cs->seq++;
		}
		break;
	case 2:
		if(cs->oya == MY_PARENT){
			GFL_NET_InitServer();
			OS_TPrintf("親機になってワイヤレス開始\n");
		}
		else{
			GFL_NET_InitClientAndConnectToParent(cs->parent_profile.parent_MacAddress);
			OS_TPrintf("子機になってワイヤレス開始\n");
		}
		cs->seq++;
		break;
	case 3:	//ワイヤレス接続待ち
		//※check　現状接続したか判断する手段がないので、暫定的にタイマーで繋がった事にする
		cs->timer++;
		if(cs->timer > 30){
			cs->timer = 0;
			//ネゴシエーション開始
			OS_TPrintf("ネゴシエーション送信\n");
			if(cs->oya == MY_CHILD){
				if(GFL_NET_HANDLE_RequestNegotiation() == TRUE){
					cs->seq++;
				}
			}
			else{
				cs->seq++;	//親機は最後にネゴシエーション送信をする
			}
		}
		break;
	case 4:	//ネゴシエーション完了待ち
		if(cs->oya == MY_PARENT){
			if(GFL_NET_HANDLE_GetNumNegotiation() >= cs->entry_num){
				cs->timer++;
				if(cs->timer > 30){
					cs->timer = 0;
					if(GFL_NET_HANDLE_RequestNegotiation() == TRUE){
						OS_TPrintf("子のネゴシエーション揃った\n");
					 //   cs->connect_ok = TRUE;
					    cs->seq++;
					}
				}
			}
		}
		else{
			cs->seq++;
		}
		break;

	case 5:
		if(cs->oya == MY_PARENT){
			if(GFL_NET_HANDLE_GetNumNegotiation() >= cs->entry_num + 1){
				OS_TPrintf("親のネゴシエーションも完了\n");
				cs->connect_ok = TRUE;
				cs->seq++;
			}
		}
		else{
			cs->seq++;
		}
		break;
	case 6:		//タイミングコマンド発行
		if(cs->oya == MY_PARENT){
			cs->timer++;
			if(cs->timer < 30){
				break;
			}
			cs->timer = 0;
		}
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,15);
		cs->seq++;
		break;
	case 7:
		if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),15) == TRUE){
			OS_TPrintf("タイミング取り成功\n");
			OS_TPrintf("接続人数 = %d\n", GFL_NET_GetConnectNum());
			GFL_MSG_GetString( cs->mm, DM_MSG_MATCH008, cs->strbuf );
			CCLocal_MessagePut(cs, CCBMPWIN_MAX, cs->strbuf, 0, 0);
			cs->seq++;
		}
		break;
	case 8:
		//netID順に配列に埋め込む為、再度自分自身のプロフィールを全員に送信
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_MY_PROFILE, sizeof(IRC_MATCH_ENTRY_PARAM), &cs->my_profile) == TRUE){
			OS_TPrintf("自分のプロフィールを全てのプレイヤーに送信した\n");
			OS_TPrintf("MAC: %d, %d, %d, %d, %d, %d\n", cs->my_profile.parent_MacAddress[0], cs->my_profile.parent_MacAddress[1], cs->my_profile.parent_MacAddress[2], cs->my_profile.parent_MacAddress[3], cs->my_profile.parent_MacAddress[4], cs->my_profile.parent_MacAddress[5]);
			cs->seq++;
		}
		else{
			OS_TPrintf("自分プロフィール送信失敗\n");
		}
		break;
	case 9:	//プロフィールを送信しあったので再度同期取り
		GFL_NET_HANDLE_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() ,16);
		cs->seq++;
		break;
	case 10:
		if(GFL_NET_HANDLE_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle(),16) == TRUE){
			OS_TPrintf("タイミング取り成功 2回目\n");
			cs->seq++;
		}
		break;
	
	case 11:	//キーを送信しあう
		ret = -1;
		cs->send_key = GFL_UI_KEY_GetRepeat();
		if(cs->send_key & PAD_KEY_UP){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &cs->send_key);
		}
		if(cs->send_key & PAD_KEY_DOWN){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &cs->send_key);
		}
		if(cs->send_key & PAD_KEY_LEFT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &cs->send_key);
		}
		if(cs->send_key & PAD_KEY_RIGHT){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_KEY, 4, &cs->send_key);
		}
		
		if(cs->send_key & PAD_BUTTON_B){
			ret = GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), NET_CMD_HUGE, 0x1000, &cs->huge_data);
		}
		
		if(ret == FALSE){
			OS_TPrintf("送信失敗\n");
		}
		
		if(GFL_UI_KEY_GetTrg() & PAD_BUTTON_SELECT){
//			cs->seq++;
		}
		break;
	case 12:	//通信終了
		if(GFL_NET_SendData(GFL_NET_HANDLE_GetCurrentHandle(), GFL_NET_CMD_EXIT_REQ, 0, NULL)){
			//GFL_NET_Exit(_endCallBack);	//通信終了
			cs->seq++;
		}
		break;
	case 13:	//通信終了待ち
		if(cs->connect_ok == FALSE){
			cs->seq++;
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
	COMPATI_SYS *cs = glb_d_matsu;
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
		if(cs->my_profile.oya_child == MY_CHILD){
			OS_TPrintf("自分も相手も子である\n");
			IRC_Shutdown();
			return;
		}
		CCLocal_EntryAdd(cs, recv);
		IRC_Send(NULL, 0, CMD_PARENT_SUCCESS, 0);
		cs->irc.success = TRUE;
		break;
	}
	
	//親から送信してくるデータ
	switch(command){
	case CMD_PARENT_PROFILE:	//親のプロフィール受信
		CCLocal_ParentProfileSet(cs, recv);
		IRC_Send((u8*)&cs->my_profile, sizeof(IRC_MATCH_ENTRY_PARAM), CMD_CHILD_PROFILE, 0);
		break;
	case CMD_PARENT_SUCCESS:		//親から受け取ったと返事を受信
		IRC_Shutdown();
		cs->irc.success = TRUE;
		break;
	}
}

//--------------------------------------------------------------
/**
 * @brief   子のプロフィールを登録
 *
 * @param   cs		
 * @param   profile		
 */
//--------------------------------------------------------------
static void CCLocal_EntryAdd(COMPATI_SYS *cs, IRC_MATCH_ENTRY_PARAM *profile)
{
	GF_ASSERT(cs->entry_num < CHILD_MAX);
	GFL_STD_MemCopy(profile, &cs->child_profile[cs->entry_num], sizeof(IRC_MATCH_ENTRY_PARAM));
	cs->entry_num++;
	OS_TPrintf("子をエントリー:%d人目\n", cs->entry_num);
}

//--------------------------------------------------------------
/**
 * @brief   親のプロフィールを登録
 *
 * @param   cs		
 * @param   profile		
 */
//--------------------------------------------------------------
static void CCLocal_ParentProfileSet(COMPATI_SYS *cs, IRC_MATCH_ENTRY_PARAM *profile)
{
	GFL_STD_MemCopy(profile, &cs->parent_profile, sizeof(IRC_MATCH_ENTRY_PARAM));
	OS_TPrintf("親のプロフィール登録\n");
}

//--------------------------------------------------------------
/**
 * @brief   メッセージを表示する
 *
 * @param   cs			
 * @param   strbuf		表示するメッセージデータ
 * @param   x			X座標
 * @param   y			Y座標
 */
//--------------------------------------------------------------
static void CCLocal_MessagePut(COMPATI_SYS *cs, int win_index, STRBUF *strbuf, int x, int y)
{
	GFL_BMP_Clear( cs->drawwin[win_index].bmp, 0xff );
	PRINTSYS_PrintQue(cs->printQue, cs->drawwin[win_index].bmp, x, y, strbuf, cs->fontHandle);
	cs->drawwin[win_index].message_req = TRUE;
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンの共通データ登録
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_PokeIconCommonDataSet(COMPATI_SYS *cs)
{
	//パレット
	NNS_G2dInitImagePaletteProxy( &cs->icon_pal_proxy );
	GFL_ARCHDL_UTIL_TransVramPaletteMakeProxy(ARCID_POKEICON, POKEICON_GetPalArcIndex(), 
		NNS_G2D_VRAM_TYPE_2DMAIN, D_MATSU_ICON_PALNO, HEAPID_COMPATI, &cs->icon_pal_proxy);
	
	//セル
	cs->icon_cell_res = GFL_ARCHDL_UTIL_LoadCellBank(ARCID_POKEICON, POKEICON_GetCellArcIndex(), 
		FALSE, &cs->icon_cell_data, HEAPID_COMPATI);
	
	//セルアニメ
	cs->icon_anm_res = GFL_ARCHDL_UTIL_LoadAnimeBank(ARCID_POKEICON , POKEICON_GetAnmArcIndex(),
		FALSE, &cs->icon_anm_data, HEAPID_COMPATI);
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコンの共通データ破棄
 *
 * @param   cs		
 */
//--------------------------------------------------------------
static void CCLocal_PokeIconCommonDataFree(COMPATI_SYS *cs)
{
	int i;
	
	for(i = 0; i < CCBMPWIN_MAX; i++){
		if(cs->clwk[i] != NULL){
			GFL_CLACT_WK_Remove(cs->clwk[i]);
		}
	}
	GFL_HEAP_FreeMemory(cs->icon_cell_res);
	GFL_HEAP_FreeMemory(cs->icon_anm_res);
}

//--------------------------------------------------------------
/**
 * @brief   ポケモンアイコン登録
 *
 * @param   cs		
 * @param   monsno		ポケモン番号
 * @param   net_id		ネットID
 */
//--------------------------------------------------------------
static void CCLocal_PokeIconAdd(COMPATI_SYS *cs, int monsno, int net_id)
{
	GFL_CLWK_DATA clwk_data;
	GFL_CLWK_RES clwk_res;
	NNSG2dImageProxy imgProxy;
	u32 icon_index;
	u32 vram_offset = 0;	//byte単位
	
	GF_ASSERT(cs->clwk[net_id] == NULL);

	vram_offset = POKEICON_SIZE_CGX * net_id;
	
	OS_TPrintf("monsno = %d\n", monsno);
	
	//キャラクタ設定
	NNS_G2dInitImageProxy(&imgProxy);
	icon_index = POKEICON_GetCgxArcIndexByMonsNumber(monsno, 0, FALSE);
	//VRAM転送&イメージプロキシ作成
	GFL_ARCHDL_UTIL_TransVramCharacterMakeProxy(ARCID_POKEICON, icon_index, FALSE, CHAR_MAP_1D, 0, 
		NNS_G2D_VRAM_TYPE_2DMAIN, vram_offset, HEAPID_COMPATI, &imgProxy);
	
	//セルアニメ用リソースデータ作成
	GFL_CLACT_WK_SetCellResData(&clwk_res, &imgProxy, &cs->icon_pal_proxy, 
		cs->icon_cell_data, cs->icon_anm_data);
	
	//アクター登録
	clwk_data = PokeIconClwkData;
	clwk_data.pos_x = 120;
	clwk_data.pos_y = 24 + net_id*32;
	cs->clwk[net_id] = GFL_CLACT_WK_Add(
		cs->clunit, &clwk_data, &clwk_res, CLWK_SETSF_NONE, HEAPID_COMPATI);
	
	//アニメオート設定
	GFL_CLACT_WK_SetAutoAnmSpeed(cs->clwk[net_id], FX32_ONE);
	GFL_CLACT_WK_SetAutoAnmFlag(cs->clwk[net_id], TRUE);
	
	//パレットNo設定
	GFL_CLACT_WK_SetPlttOffs(cs->clwk[net_id], POKEICON_GetPalNum(monsno, 0, FALSE));
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
	COMPATI_SYS *cs = pWork;
	
    OS_TPrintf("ネゴシエーション完了\n");
    cs->connect_ok = TRUE;
}

static void _endCallBack(void* pWork)
{
	COMPATI_SYS *cs = pWork;

    NET_PRINT("endCallBack終了\n");
    cs->connect_ok = FALSE;
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
	COMPATI_SYS *cs = pWork;
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
	COMPATI_SYS *cs = pWork;
	return cs->receive_huge_data[netID - 1];
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
	COMPATI_SYS *cs = pWork;
	int key_data;
	int dx = 0, dy = 0;
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	
	key_data = *((int *)pData);
	OS_TPrintf("データ受信：netID=%d, size=%d, data=%d\n", netID, size, key_data);
	if(key_data & PAD_KEY_UP){
		OS_TPrintf("受信：上\n");
		dy -= 4;
	}
	if(key_data & PAD_KEY_DOWN){
		OS_TPrintf("受信：下\n");
		dy += 4;
	}
	if(key_data & PAD_KEY_LEFT){
		OS_TPrintf("受信：左\n");
		dx -= 4;
	}
	if(key_data & PAD_KEY_RIGHT){
		OS_TPrintf("受信：右\n");
		dx += 4;
	}

#if 0	//名前ではなくポケモンアイコンを動かすように変更
	cs->msg_pos_x[netID] += dx;
	cs->msg_pos_y[netID] += dy;
	if(netID == 0){
		if(cs->oya == MY_PARENT){
			GFL_STR_SetStringCodeOrderLength(cs->strbuf_win[netID], cs->my_profile.name, cs->my_profile.name_len+1);
		}
		else{
			GFL_STR_SetStringCodeOrderLength(cs->strbuf_win[netID], cs->parent_profile.name, cs->parent_profile.name_len+1);
		}
	}
	else{
		GFL_STR_SetStringCodeOrderLength(cs->strbuf_win[netID], cs->child_profile[netID-1].name, cs->child_profile[netID-1].name_len+1);
	}
	CCLocal_MessagePut(cs, netID, 
		cs->strbuf_win[netID], cs->msg_pos_x[netID], cs->msg_pos_y[netID])
#else
	if(cs->clwk[netID] != NULL){
		GFL_CLACTPOS pos;
		GFL_CLACT_WK_GetPos(cs->clwk[netID], &pos, CLWK_SETSF_NONE);
		pos.x += dx;
		pos.y += dy;
		GFL_CLACT_WK_SetPos(cs->clwk[netID], &pos, CLWK_SETSF_NONE);
	}
#endif
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
static void _RecvMyProfile(const int netID, const int size, const void* pData, void* pWork, GFL_NETHANDLE* pNetHandle)
{
	COMPATI_SYS *cs = pWork;
	const IRC_MATCH_ENTRY_PARAM *recv = pData;
	int i;
	
	OS_TPrintf("RecvChildProfile, netID=%d, size=%d\n", netID, size);
	
	if(pNetHandle != GFL_NET_HANDLE_GetCurrentHandle()){
		return;	//自分のハンドルと一致しない場合、親としてのデータ受信なので無視する
	}
	
	if(netID == 0){
		GFL_STD_MemCopy(recv, &cs->parent_profile, size);
	}
	else{
		GFL_STD_MemCopy(recv, &cs->child_profile[netID - 1], size);
	}
	GFL_STR_SetStringCodeOrderLength(
		cs->strbuf_win[netID], recv->name, recv->name_len+1);
	CCLocal_MessagePut(cs, netID, cs->strbuf_win[netID], 0, 0);

	CCLocal_PokeIconAdd(cs, recv->parent_MacAddress[5], netID);
}




//----------------------------------------------------------
/**
 *
 */
//----------------------------------------------------------
const GFL_PROC_DATA CompatiCheckProcData = {
	CompatiCheck_ProcInit,
	CompatiCheck_ProcMain,
	CompatiCheck_ProcEnd,
};


