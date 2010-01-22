//=============================================================================
/**
 * @file    gsync_state.c
 * @brief   ゲームシンク
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include <nitro.h>
#include "gflib.h"
#include "arc_def.h"
#include "libdpw/dwci_ghttp.h"
#include "net_app/gsync.h"
#include <nitroWiFi/nhttp.h>
#include "net/nitrowifidummy.h"

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
#include "savedata/system_data.h"
#include "msg/msg_d_ohno.h"
#include "gsync_local.h"
#include "net/nhttp_rap.h"
#include "../../field/event_gsync.h"
#include "savedata/dreamworld_data.h"
#include "savedata/symbol_save.h"
#include "gsync_obj_NANR_LBLDEFS.h"
#include "gsync_poke.cdat"
#include "msg/msg_gsync.h"
#include "gsync.naix"

/*
■BGM■

SEQ_BGM_GAME_SYNC	ゲームシンクBGM

■SE■

SEQ_SE_SYS_24			アップ・ダウンロード
SEQ_SE_SYS_25			アップロード完了音
SEQ_SE_SYS_26			ポケモン眠る
*/
#define _ZZZCOUNT (110)

typedef void (StateFunc)(G_SYNC_WORK* pState);



//
// PHP とのインターフェース構造体
//
typedef struct tag_EVENT_DATA
{
  unsigned long rom_code;
  unsigned short country_code;
  unsigned char id;
  unsigned char send_flag;
  unsigned long dec_code;
  unsigned short cat_id;
  unsigned short present;
  unsigned char status;
  unsigned char idname1[26];
  unsigned char idname2[26];
  unsigned char idname3[26];
  unsigned char idname4[26];
  unsigned char idname5[26];
}
  EVENT_DATA;



static void _changeState(G_SYNC_WORK* pWork,StateFunc* state);
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc* state, int line);

#if DEBUG_ONLY_FOR_ohno
#define _NET_DEBUG (1)
#else
#define _NET_DEBUG (0)
#endif
#ifdef _NET_DEBUG
#define   _CHANGE_STATE(state)  _changeStateDebug(pWork ,state, __LINE__)
#else  //_NET_DEBUG
#define   _CHANGE_STATE(state)  _changeState(pWork ,state)
#endif //_NET_DEBUG




struct _G_SYNC_WORK {
  HEAPID heapID;
  POKEMON_PARAM* pp;
  NHTTP_RAP_WORK* pNHTTPRap;
  GSYNC_DISP_WORK* pDispWork;  // 描画系
  GSYNC_MESSAGE_WORK* pMessageWork; //メッセージ系
  APP_TASKMENU_WORK* pAppTask;
  EVENT_GSYNC_WORK* pParent;
  BOX_MANAGER * pBox;
  SAVE_CONTROL_WORK* pSaveData;
  void* pTopAddr;
  int trayno;
  int indexno;
  StateFunc* state;      ///< ハンドルのプログラム状態
  vu32 count;
  int req;
  int getdataCount;
  int countTimer;
  BOOL bEnd;
  int zzzCount;
};


static void _ghttpKeyWait(G_SYNC_WORK* pWork);



//------------------------------------------------------------------------------
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _changeState(G_SYNC_WORK* pWork,StateFunc state)
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
static void _changeStateDebug(G_SYNC_WORK* pWork,StateFunc state, int line)
{
#ifdef DEBUG_ONLY_FOR_ohno
  OS_TPrintf("gsync: %d\n",line);
#endif
  _changeState(pWork, state);
}
#endif

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeout(G_SYNC_WORK* pWork)
{
	if(WIPE_SYS_EndCheck()){
		_CHANGE_STATE( NULL);        // 終わり
	}
}

//------------------------------------------------------------------------------
/**
 * @brief   フェードアウト処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _modeFadeStart(G_SYNC_WORK* pWork)
{
  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEOUT , WIPE_TYPE_FADEOUT , 
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );
  _CHANGE_STATE(_modeFadeout);        // 終わり
}



static void _networkClose1(G_SYNC_WORK* pWork)
{
  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(_modeFadeout);
  }
}

static void _networkClose(G_SYNC_WORK* pWork)
{
  pWork->pParent->selectType = GAMESYNC_RETURNMODE_EXIT;
  if(!GFL_NET_IsInit()){
    _CHANGE_STATE(_modeFadeout);
  }
  else{
    GFL_NET_Exit(NULL);
    _CHANGE_STATE(_networkClose1);
  }
}


static void _wakeupActio8(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  if(GFL_UI_KEY_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ゲームシンク完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction7(G_SYNC_WORK* pWork)
{
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
    pWork->pp=NULL;
  }

  if(BOXDAT_GetEmptyTrayNumberAndPos( pWork->pBox, &pWork->trayno, &pWork->indexno )){
    POKEMON_PARAM* pp = GFL_HEAP_AllocClearMemory( pWork->heapID, POKETOOL_GetWorkSize()) ;

    GFL_STD_MemCopy(DREAMWORLD_SV_GetSleepPokemon(pDream), pp, POKETOOL_GetWorkSize());

    BOXDAT_PutPokemon(pWork->pBox, PP_GetPPPPointerConst(pp));
    
    pWork->pp = pp;
    DREAMWORLD_SV_SetSleepPokemonFlg(pDream,FALSE);
  }


  
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_005);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani3,NANR_gsync_obj_rug_ani4);

  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed_ani);
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,FALSE);
  
  
  GSYNC_DISP_PokemonIconCreate(pWork->pDispWork, PP_GetPPPPointer(pWork->pp),CLSYS_DRAW_MAIN);
  GSYNC_DISP_PokemonIconJump(pWork->pDispWork);

  PMSND_PlaySE(SEQ_SE_SYS_25);

  _CHANGE_STATE(_wakeupActio8);
}








//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_test4(G_SYNC_WORK* pWork)
{
  _CHANGE_STATE(_wakeupAction7);
}

//テスト 
static void _wakeupAction_test3(G_SYNC_WORK* pWork)
{
  _CHANGE_STATE(_wakeupAction_test4);
}
//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction_test2(G_SYNC_WORK* pWork)
{
  _CHANGE_STATE(_wakeupAction_test3);
}

//テスト 

static void _wakeupAction_test1(G_SYNC_WORK* pWork)
{
  _CHANGE_STATE(_wakeupAction_test2);
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction6(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        DREAM_WORLD_SERVER_DOWNLOAD_DATA* pDream = (DREAM_WORLD_SERVER_DOWNLOAD_DATA*)&pEvent[sizeof(gs_response)];

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);

        SymbolSave_Set(SymbolSave_GetSymbolData(pWork->pSaveData), pDream->findPokemon, 
          1, PTL_SEX_MALE, 0);

      }
      _CHANGE_STATE(_wakeupAction_test1);
    }
  }
  else{
    _CHANGE_STATE(_wakeupAction7);
  }
}

static void _wakeupAction5(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_DOWNLOAD, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_wakeupAction6);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_wakeupAction6);
    }
  }
}

static void _wakeupActio4_2(G_SYNC_WORK* pWork)
{
 if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_wakeupAction5);
}


static void _wakeupAction4(G_SYNC_WORK* pWork)
{
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_004);
  GSYNC_DISP_ObjEnd(pWork->pDispWork, NANR_gsync_obj_zzz_ani);
  GSYNC_DISP_DreamSmokeBgStart(pWork->pDispWork);
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,TRUE);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani3,NANR_gsync_obj_rug_ani2);
  PMSND_PlaySE(SEQ_SE_SYS_24);

  _CHANGE_STATE(_wakeupActio4_2);

}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction3(G_SYNC_WORK* pWork)
{

  if(pWork->zzzCount%_ZZZCOUNT==0){
    PMSND_PlaySE(SEQ_SE_SYS_26);
  }
  pWork->zzzCount++;

  
  if(APP_TASKMENU_IsFinish(pWork->pAppTask)){
    int selectno = APP_TASKMENU_GetCursorPos(pWork->pAppTask);

    if(selectno==0){
      _CHANGE_STATE(_wakeupAction4);
    }
    else{
      //@todo 切断処理へ
      _CHANGE_STATE(_networkClose);
    }
    GSYNC_MESSAGE_InfoMessageEnd(pWork->pMessageWork);
    APP_TASKMENU_CloseMenu(pWork->pAppTask);
    pWork->pAppTask=NULL;
  }
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction2(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  pWork->pAppTask = GSYNC_MESSAGE_YesNoStart(pWork->pMessageWork,GSYNC_YESNOTYPE_INFO);

  _CHANGE_STATE(_wakeupAction3);
}



//------------------------------------------------------------------------------
/**
 * @brief   ポケモン起こし処理
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _wakeupAction1(G_SYNC_WORK* pWork)
{

  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_rug_ani3);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_zzz_ani);

  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_003);
  PMSND_PlaySE(SEQ_SE_SYS_26);

  _CHANGE_STATE(_wakeupAction2);

}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait1(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        DREAM_WORLD_SERVER_STATUS_DATA* pDream = (DREAM_WORLD_SERVER_STATUS_DATA*)&pEvent[sizeof(gs_response)];

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        NET_PRINT("PlayStatus %d\n",pDream->PlayStatus);

      }
      _CHANGE_STATE(_wakeupAction1);
    }
  }
  else{
    _CHANGE_STATE(_wakeupAction1);
  }
    
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpInfoWait0(G_SYNC_WORK* pWork)
{

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ACCOUNTINFO, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_ghttpInfoWait1);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpInfoWait1);
    }
  }
}




//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPokemonListDownload1(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);

        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);

        GFL_STD_MemCopy(&pEvent[sizeof(gs_response)],pWork->pParent->selectPokeList.pokemonList,
                        DREAM_WORLD_SERVER_POKMEONLIST_MAX/8);
      }
      pWork->pParent->selectType = GAMESYNC_RETURNMODE_BOXJUMP;
      _CHANGE_STATE(_modeFadeStart);
    }
  }
  else{
      pWork->pParent->selectType = GAMESYNC_RETURNMODE_BOXJUMP;
    _CHANGE_STATE(_modeFadeStart);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   ポケモン状態検査
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _ghttpPokemonListDownload(G_SYNC_WORK* pWork)
{

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_POKEMONLIST, pWork->pNHTTPRap)){
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_ghttpPokemonListDownload1);
      }
    }
  }
  else{
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_ghttpPokemonListDownload1);
    }
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   データアップロード完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop8(G_SYNC_WORK* pWork)
{

  if(pWork->zzzCount%_ZZZCOUNT==0){
    PMSND_PlaySE(SEQ_SE_SYS_26);
  }
  pWork->zzzCount++;

  if(GFL_UI_KEY_GetTrg()){
    _CHANGE_STATE(_networkClose);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   データアップロード完了
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop7(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }
  _CHANGE_STATE(_upeffectLoop8);

}

//------------------------------------------------------------------------------
/**
 * @brief   眠る完了
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop6(G_SYNC_WORK* pWork)
{
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
    pWork->pTopAddr = NULL;
  }

  
  GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_002);
  GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_zzz_ani);
  GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,FALSE);
  GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani1,NANR_gsync_obj_rug_ani3);
  PMSND_PlaySE(SEQ_SE_SYS_25);

  _CHANGE_STATE(_upeffectLoop7);
}


static int dummy=0;

static void _upeffectLoop5(G_SYNC_WORK* pWork)
{
  if(GFL_NET_IsInit()){
    GSYNC_DISP_SetPerfomance(pWork->pDispWork,dummy/3);
    dummy++;
    if(NHTTP_ERROR_NONE== NHTTP_RAP_Process(pWork->pNHTTPRap)){
      NET_PRINT("終了\n");
      {
        u8* pEvent = (u8*)NHTTP_RAP_GetRecvBuffer(pWork->pNHTTPRap);
        int d,j;
        u32 size;
        NHTTP_DEBUG_GPF_HEADER_PRINT((gs_response*)pEvent);
        

      }
      _CHANGE_STATE(_upeffectLoop6);
    }
  }
  else{
    GSYNC_DISP_SetPerfomance(pWork->pDispWork,dummy/3);
    dummy++;
    if(GFL_UI_KEY_GetTrg()){
      _CHANGE_STATE(_upeffectLoop6);
    }
  }
    
}



//------------------------------------------------------------------------------
/**
 * @brief   データアップロード中
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectLoop4(G_SYNC_WORK* pWork)
{
  if(!GSYNC_MESSAGE_InfoMessageEndCheck(pWork->pMessageWork)){
    return;
  }

  if(GFL_NET_IsInit()){
    if(NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_UPLOAD, pWork->pNHTTPRap)){
      if(1){
        u32 size;
        u8* topAddr = (u8*)SaveControl_GetSaveWorkAdrs(pWork->pSaveData, &size);
        NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), topAddr, 0x80000 );
        dummy=0;
        GSYNC_DISP_SetPerfomance(pWork->pDispWork,0);


      }
//      else{
//        ARCHANDLE* p_handle = GFL_ARC_OpenDataHandle( ARCID_GSYNC, pWork->heapID );
//        u32 size;
//        pWork->pTopAddr = GFL_ARCHDL_UTIL_LoadEx(p_handle,NARC_gsync_save3_bin,FALSE,pWork->heapID,&size );
//        NHTTP_AddPostDataRaw(NHTTP_RAP_GetHandle(pWork->pNHTTPRap), pWork->pTopAddr, 0x80000 );
//        GFL_ARC_CloseDataHandle(p_handle);
//      }
      if(NHTTP_RAP_StartConnect(pWork->pNHTTPRap)){
        _CHANGE_STATE(_upeffectLoop5);
      }
    }
  }
  else{
    dummy=0;
    GSYNC_DISP_SetPerfomance(pWork->pDispWork,0);
    _CHANGE_STATE(_upeffectLoop5);
  }

}

//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop3(G_SYNC_WORK* pWork)
{

  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_DreamSmokeBgStart(pWork->pDispWork);

    GSYNC_MESSAGE_InfoMessageDisp(pWork->pMessageWork,GSYNC_001);

    GSYNC_DISP_BlendSmokeStart(pWork->pDispWork,TRUE);
    GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_rug_ani1,NANR_gsync_obj_rug_ani2);

    PMSND_PlaySE(SEQ_SE_SYS_24);

//    test

    _CHANGE_STATE(_upeffectLoop4);
  }
}


//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop2(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_ObjInit(pWork->pDispWork, NANR_gsync_obj_rug_ani1);
    pWork->countTimer = _DREAMSMOKE_TIME;
    _CHANGE_STATE(_upeffectLoop3);
  }

}


//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop1(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_ObjChange(pWork->pDispWork,NANR_gsync_obj_bed,NANR_gsync_obj_bed_ani);

    GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_kemuri_r);
    GSYNC_DISP_ObjInit(pWork->pDispWork,NANR_gsync_obj_kemuri_l);
    
    pWork->countTimer = _HAND_UP_TIME;
    _CHANGE_STATE(_upeffectLoop2);
  }
}





//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト中
 * @retval  none
 */
//------------------------------------------------------------------------------
static void _upeffectLoop(G_SYNC_WORK* pWork)
{
  pWork->countTimer--;
  if(pWork->countTimer==0){
    GSYNC_DISP_HandInit(pWork->pDispWork);
    GSYNC_DISP_PokemonIconMove(pWork->pDispWork);
    pWork->countTimer = _BEDIN_TIME;
    _CHANGE_STATE(_upeffectLoop1);
  }
}



//------------------------------------------------------------------------------
/**
 * @brief   眠るエフェクト開始
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _upeffectStart(G_SYNC_WORK* pWork)
{
  GSYNC_DISP_PokemonIconCreate(pWork->pDispWork, PP_GetPPPPointer(pWork->pp),CLSYS_DRAW_SUB);
  pWork->countTimer=10;

  _CHANGE_STATE(_upeffectLoop);
}

//------------------------------------------------------------------------------
/**
 * @brief   ポケモンを眠るエリアに移動
 * @retval  none
 */
//------------------------------------------------------------------------------

static void _BoxPokeMove(G_SYNC_WORK* pWork)
{
  POKEMON_PASO_PARAM* ppp;
  DREAMWORLD_SAVEDATA* pDream = DREAMWORLD_SV_GetDreamWorldSaveData(pWork->pSaveData);

  //GF_ASSERT(DREAMWORLD_SV_GetSleepPokemonFlg(pDream)==FALSE);

  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
    pWork->pp=NULL;
  }

  ppp = BOXDAT_GetPokeDataAddress( pWork->pBox, pWork->trayno, pWork->indexno );
  GF_ASSERT(ppp);
  if(ppp){
    POKEMON_PARAM* pp = PP_CreateByPPP( ppp, pWork->heapID );
    DREAMWORLD_SV_SetSleepPokemon(pDream, pp);
    pWork->pp = pp;
    BOXDAT_ClearPokemon(pWork->pBox, pWork->trayno, pWork->indexno );
    DREAMWORLD_SV_SetSleepPokemonFlg(pDream,TRUE);
  }
  _CHANGE_STATE(_upeffectStart);

}

FS_EXTERN_OVERLAY(dpw_common);


#if PM_DEBUG
static  G_SYNC_WORK* _pWork;
#endif

//------------------------------------------------------------------------------
/**
 * @brief   PROC初期化
 * @retval  none
 */
//------------------------------------------------------------------------------

static GFL_PROC_RESULT GSYNCProc_Init( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  EVENT_GSYNC_WORK* pParent = pwk;
  G_SYNC_WORK* pWork;
  s32 profileID;

  GFL_OVERLAY_Load( FS_OVERLAY_ID(dpw_common));
  GFL_HEAP_CreateHeap( GFL_HEAPID_APP, HEAPID_GAMESYNC, 0x88000 );
  pWork = GFL_PROC_AllocWork( proc, sizeof( G_SYNC_WORK ), HEAPID_GAMESYNC );
  GFL_STD_MemClear(pWork, sizeof(G_SYNC_WORK));

  pWork->heapID = HEAPID_GAMESYNC;

  if(pParent){
    pWork->pParent = pParent;
    pWork->pSaveData = GAMEDATA_GetSaveControlWork(pParent->gameData);
    profileID = MyStatus_GetProfileID( GAMEDATA_GetMyStatus(pParent->gameData) );
    GF_ASSERT(profileID);
    pWork->pNHTTPRap = NHTTP_RAP_Init(HEAPID_GAMESYNC, profileID);
    pWork->pBox = GAMEDATA_GetBoxManager(GAMESYSTEM_GetGameData(pParent->gsys));
    pWork->trayno = pParent->boxNo;
    pWork->indexno = pParent->boxIndex;
    switch(pParent->selectType){
    case GSYNC_CALLTYPE_INFO:
      _CHANGE_STATE(_ghttpInfoWait0);
      break;
    case GSYNC_CALLTYPE_BOXSET:
      _CHANGE_STATE(_BoxPokeMove);
      break;
    case GSYNC_CALLTYPE_POKELIST:
      _CHANGE_STATE(_ghttpPokemonListDownload);
      break;
    }
  }
  else{
#if PM_DEBUG
    pWork->pNHTTPRap = NHTTP_RAP_Init(HEAPID_GAMESYNC, 1234);
    _pWork=pWork;
    pWork->trayno=0;
    pWork->indexno=0;
    pWork->pSaveData = SaveControl_GetPointer();
    pWork->pBox = BOX_DAT_InitManager(pWork->heapID,SaveControl_GetPointer());

    {
      POKEMON_PARAM *pp;
      BOX_MANAGER* pBox = pWork->pBox;
      pp = PP_Create(MONSNO_MARIRU, 100, 123456, pWork->heapID);
      {
        u16 oyaName[5] = {L'デ',L'バ',L'ッ',L'グ',0xFFFF};
        POKEMON_PERSONAL_DATA* ppd = POKE_PERSONAL_OpenHandle(MONSNO_MARIRU, 0, GFL_HEAPID_APP);
        u32 ret = POKE_PERSONAL_GetParam(ppd, POKEPER_ID_sex);
        PP_SetupEx(pp, MONSNO_MARIRU, 3, 123456,PTL_SETUP_POW_AUTO, ret);
        PP_Put( pp , ID_PARA_oyaname_raw , (u32)oyaName );

        BOXDAT_PutPokemonBox(pBox, 0, (POKEMON_PASO_PARAM*)PP_GetPPPPointerConst(pp));

        POKE_PERSONAL_CloseHandle(ppd);
      }
      GFL_HEAP_FreeMemory(pp);
    }
    _CHANGE_STATE(_BoxPokeMove);
#endif
  }

  pWork->pDispWork = GSYNC_DISP_Init(pWork->heapID);
  pWork->pMessageWork = GSYNC_MESSAGE_Init(pWork->heapID, NARC_message_gsync_dat);


  WIPE_SYS_Start( WIPE_PATTERN_WMS , WIPE_TYPE_FADEIN , WIPE_TYPE_FADEIN ,
                  WIPE_FADE_BLACK , WIPE_DEF_DIV , WIPE_DEF_SYNC , pWork->heapID );

  PMSND_PlayBGM(SEQ_BGM_GAME_SYNC);

  
  return GFL_PROC_RES_FINISH;
}

#if PM_DEBUG

static GFL_PROC_RESULT GSYNCProc_InitDbg( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  GSYNCProc_Init( proc, seq, pwk,  mywk);
  {
    G_SYNC_WORK* pWork = _pWork;
    _BoxPokeMove(pWork);
    _CHANGE_STATE(_ghttpInfoWait0);
  }
  return GFL_PROC_RES_FINISH;
}

#endif


static GFL_PROC_RESULT GSYNCProc_Main( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;
  StateFunc* state = pWork->state;
  GFL_PROC_RESULT ret = GFL_PROC_RES_FINISH;

  if( state ){
    state( pWork );
    ret = GFL_PROC_RES_CONTINUE;
  }
  if(pWork->pAppTask){
    APP_TASKMENU_UpdateMenu(pWork->pAppTask);
  }

  GSYNC_DISP_Main(pWork->pDispWork);
  GSYNC_MESSAGE_Main(pWork->pMessageWork);

  return ret;
}

static GFL_PROC_RESULT GSYNCProc_End( GFL_PROC * proc, int * seq, void * pwk, void * mywk )
{
  G_SYNC_WORK* pWork = mywk;

  GSYNC_MESSAGE_End(pWork->pMessageWork);
  GSYNC_DISP_End(pWork->pDispWork);
  if(pWork->pp){
    GFL_HEAP_FreeMemory(pWork->pp);
  }
  if(pWork->pTopAddr){
    GFL_HEAP_FreeMemory(pWork->pTopAddr);
  }

  NHTTP_RAP_End(pWork->pNHTTPRap);

#if PM_DEBUG
  if(pwk==NULL){
    _pWork = NULL;
    BOX_DAT_ExitManager(pWork->pBox);
  }
#endif
  
  GFL_PROC_FreeWork(proc);
 
  GFL_HEAP_DeleteHeap(HEAPID_GAMESYNC);
  GFL_OVERLAY_Unload( FS_OVERLAY_ID(dpw_common));
  return GFL_PROC_RES_FINISH;
}

// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData = {
  GSYNCProc_Init,
  GSYNCProc_Main,
  GSYNCProc_End,
};

#if PM_DEBUG
// プロセス定義データ
const GFL_PROC_DATA G_SYNC_ProcData_Dbg = {
  GSYNCProc_InitDbg,
  GSYNCProc_Main,
  GSYNCProc_End,
};
#endif
