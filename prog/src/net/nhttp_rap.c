//=============================================================================
/**
 * @file    nhttp_rap.c
 * @brief   nhttpを利用して、GameSyncに必要な
 * @author  ohno_katsumi@gamefreak.co.jp
 * @date    09/04/30
 */
//=============================================================================

#include "net/nhttp_rap.h"
#include "net_app/gsync.h"
#include "net/dwc_rapcommon.h"

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
#include "msg/msg_d_ohno.h"



static G_SYNC_WORK* _pWork;

typedef void (StateFunc)(G_SYNC_WORK* pState);

/*
#define POSTURL "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsync.cgi"
#define GETURL1 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data1&mac=1"
#define GETURL2 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data2&mac=1"
#define GETURL3 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data3&mac=1"
#define GETURL4 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data4&mac=1"
#define GETURL5 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data5&mac=1"
#define GETURL6 "https://wbext.gamefreak.co.jp:10610/cgi-bin/cgeartest/gsyncget.cgi?name=data6&mac=1"
 */
//#define POSTURL "https://pokemon-ds.basementfactorysystems.com/bindata-test/data2.php"
//#define GETURL1 "https://pokemon-ds.basementfactorysystems.com/bindata-test/data1.php"


const static char ACCOUNT_INFOURL[] = "https://pokemon-ds.basementfactorysystems.com/gs?p=test.get&gsid=%d&rom=%d&langcode=%d&dreamw=%d";  //GET
const static char  POKEMONLISTURL[] ="https://pokemon-ds.basementfactorysystems.com/gs?p=sleepily.bitlist&gsid=%d&rom=%d&langcode=%d&dreamw=%d";  //GET
const static char  DOWNLOAD_URL[] ="https://pokemon-ds.basementfactorysystems.com/gs?p=test.get&gsid=%d&rom=%d&langcode=%d&dreamw=%d";  //GET
const static char  UPLOAD_URL[] ="https://pokemon-ds.basementfactorysystems.com/gs?p=savedata.upload&gsid=%d&rom=%d&langcode=%d&dreamw=%d";  //POST
const static char ACCOUNT_URL[] ="https://pokemon-ds.basementfactorysystems.com/gs?p=account.createdata"; //POST




  //rom, langcode, dreamw 

typedef struct{
  const char* url;
  int type;
} NHTTPRAP_URLTBL;

static NHTTPRAP_URLTBL urltable[]={
  {ACCOUNT_INFOURL, NHTTP_REQMETHOD_GET},
  {POKEMONLISTURL, NHTTP_REQMETHOD_GET},
  {DOWNLOAD_URL, NHTTP_REQMETHOD_GET},
  {UPLOAD_URL, NHTTP_REQMETHOD_POST},
  {ACCOUNT_URL, NHTTP_REQMETHOD_POST},
};


extern CPSCaInfo PDW_CA;

static CPSCaInfo* cainfos[] = {
  &PDW_CA,
};

#define _GET_MAXSIZE  (1024)
#define _URL_BUFFER   (200)


struct _NHTTP_RAP_WORK {
  NHTTPConnectionHandle   handle;
  int a;
  char getbuffer[_GET_MAXSIZE];
  int b;
  char urlbuff[_URL_BUFFER];
  int c;
  HEAPID heapID;
  int d;
  u32 profileid;
};



static void* AllocForNhttp(u32 size, int alignment)
{
  return DWC_RAPCOMMON_Alloc(0,size,alignment);
}

static void FreeForNhttp(void* ptr)
{
  DWC_RAPCOMMON_Free(0,ptr,0);
}



static int ConnectionCallback( NHTTPConnectionHandle handle, NHTTPConnectionEvent event, void* arg )
{
  (void)handle;
  switch ( event )
  {
  case NHTTP_EVENT_BODY_RECV_FULL:
  case NHTTP_EVENT_BODY_RECV_DONE:
    {
      NHTTPBodyBufArg*    bbArg = (NHTTPBodyBufArg*)arg;
      bbArg->offset = 0;
    }
  case NHTTP_EVENT_POST_SEND:
    {
      NHTTPPostSendArg*   psArg = (NHTTPPostSendArg*)arg;
    }
  }
  return NHTTP_ERROR_NONE;
}



BOOL NHTTP_RAP_ConectionCreate(NHTTPRAP_URL_ENUM urlno,NHTTP_RAP_WORK* pWork)
{
  int     result;
  NHTTPConnectionHandle   handle;
  NHTTPError              err;
  OSTick                  tickStart, tickPrevious;
  u32                     receivedCurrent = 0, receivedPrevious = 0;
  u32                     contentLength;
  u32                     averageSpeed = 0, currentSpeed = 0, maxSpeed = 0;
  char pidbuff[_URL_BUFFER];

  if(0!=NHTTPStartup(AllocForNhttp, FreeForNhttp, 12)){
    GF_ASSERT(0);
    return FALSE;
  }



  GFL_STD_MemClear(pWork->urlbuff,sizeof(_URL_BUFFER));
  GFL_STD_MemCopy(urltable[urlno].url, pWork->urlbuff, GFL_STD_StrLen(urltable[urlno].url));

  // ID合成
  GFL_STD_MemCopy(pWork->urlbuff, pidbuff, _URL_BUFFER);
  STD_TSNPrintf(pWork->urlbuff, _URL_BUFFER, pidbuff, pWork->profileid, PM_VERSION, PM_LANG, NET_DREAMWORLD_VER);
 // GFL_STD_StrCat(pWork->urlbuff,pidbuff,sizeof(_URL_BUFFER));

  
  NET_PRINT(" Target URL: %s\n", pWork->urlbuff);
  handle = NHTTPCreateConnection( pWork->urlbuff,
                                  urltable[urlno].type,
                                  pWork->getbuffer, _GET_MAXSIZE,
                                  ConnectionCallback, pWork);
  GF_ASSERT(handle);
  if(handle==NULL){
    return FALSE;
  }
  pWork->handle = handle;

  if ( 0 > ( err = (NHTTPError)NHTTP_SetRootCA( handle, (const char *)cainfos, sizeof(cainfos)/sizeof(CPSCaInfo*) )))
  {
    GF_ASSERT_MSG(0," NHTTP_SetRootCA(%d)\n",err);
    return FALSE;
  }
  if(0!=NHTTP_AddHeaderField(handle, "Accept", "*/*" )){
    GF_ASSERT(0);
    return FALSE;
  }
  if(0!=NHTTP_AddHeaderField(handle, "User-Agent", "Test"  )){
    GF_ASSERT(0);
    return FALSE;
  }

  if(0!=NHTTP_SetBasicAuthorization( handle, "pokemon", "2Phfv9MY")){
    GF_ASSERT(0);
    return FALSE;
  }
  return TRUE;
}


NHTTPConnectionHandle NHTTP_RAP_GetHandle(NHTTP_RAP_WORK* pWork)
{
  return pWork->handle;
}


BOOL NHTTP_RAP_StartConnect(NHTTP_RAP_WORK* pWork)
{
  if ( 0 !=  NHTTPStartConnection(pWork->handle))
  {
    GF_ASSERT(0);
    return FALSE;
  }

  return TRUE;
}


/*
NHTTP_ERROR_SYSTEM  	-1  	ハンドル不正によるエラー。
引数で渡しているコネクションハンドルが不正の場合に返ります。
NHTTP_ERROR_NONE 	0 	成功。
NHTTP_ERROR_ALLOC 	1 	アローケーションに失敗しています。
NHTTP_Startupで設定したメモリ確保関数と解放関数を確認してください。
NHTTP_ERROR_TOOMANYREQ 	2 	リクエストがいっぱいです。このエラーが発生することはありません。
NHTTP_ERROR_SOCKET 	3 	ソケットエラー。
ソケットの作成に失敗した場合、またはストリーミング送信のPOSTデータ送信のサイズ調査時に設定したコールバックが負の値を返した場合に返ります。
NHTTP_ERROR_DNS 	4 	DNS解決に失敗しています。
リクエストURLが間違っている場合や、存在しない場合に返ります。
リクエストURLを確認してください。
NHTTP_ERROR_CONNECT 	5 	接続が失敗しています。
DNS解決後、リクエストURLのサーバーとの接続前に通信が遮断された場合や、
リクエストURLで指定した接続ポート番号が間違っている場合に返ります。
NHTTP_ERROR_BUFFULL 	6 	受信バッファが一杯です。
BodyのサイズがNHTTP_CreateConnectionで設定したバッファよりも大きい場合に返ります。
NHTTP_EVENT_BODY_RECV_FULLイベントコールバック内から受信バッファを再設定することで解決可能です。
NHTTP_ERROR_HTTPPARSE 	7 	HTTPヘッダの解析に失敗しています。
サーバーから受信したHTTPヘッダーが異常な場合に返ります。
NHTTP_ERROR_CANCELED 	8 	リクエストをキャンセルしています。
NHTTP_CancelConnectionで実行中のリクエストにキャンセルを要求した場合に返ります。
NHTTP_ERROR_SDK 	9 	NHTTP_スレッドの生成に失敗しています。
NHTTP_Startup実行時、スレッド生成に失敗した場合に返ります。
NHTTP_ERROR_NETWORK 	10 	送信、受信、ソケット終了時に問題がある場合に発生するエラー。
送信、受信、終了時に通信が遮断された場合に返ります。
NHTTP_ERROR_UNKNOWN 	11 	NHTTP_CreateConnectionでメソッドに異常な値を設定している場合。
または現在実行中のリクエストに対して、
NHTTP_StartConnectionで再度リクエスト開始を要求した場合に発生するエラー。
NHTTP_ERROR_DNS_PROXY 	12 	proxyサーバーのDNS解決に失敗しています。
本体設定のproxyサーバーが間違っている場合や、存在しない場合に返ります。
本体設定のproxyサーバーを確認してください。
NHTTP_ERROR_CONNECT_PROXY 	13 	proxyサーバーとの接続に失敗しています。
DNS解決後、proxyサーバーと接続前に通信が遮断された場合や、
本体設定のproxyサーバーのポート番号を間違えている場合に返ります。
NHTTP_ERROR_SSL 	14 	SSL通信に失敗しています。
SSL通信でサーバーとの接続が失敗した場合に返ります。
SSL通信エラーの詳細はNHTTP_GetConnectionSSLError()で取得可能です。
NHTTP_ERROR_BUSY 	15 	リクエストが現在実行中であることを表します。
NHTTP_ERROR_SSL_ROOTCA 	16 	ルートCAの設定に失敗しています。
NHTTP_ERROR_SSL_CLIENTCERT 	17 	クライアント証明書の設定に失敗しています。


 */
NHTTPError NHTTP_RAP_Process(NHTTP_RAP_WORK* pWork)
{
  int     result;
  NHTTPError              err;
  u32                     receivedCurrent = 0;
  u32                     contentLength;

  if(pWork->handle==NULL){
    return NHTTP_ERROR_NONE;
  }
  
  err = NHTTPGetConnectionError(pWork->handle);
  switch(err){
  case NHTTP_ERROR_BUSY:
    break;
  case NHTTP_ERROR_NONE:  //すでに完了
  default:
    OS_TPrintf("err = %d  response %d \n", err,NHTTP_GetResultCode( pWork->handle ));

    NHTTPDeleteConnection(pWork->handle);
    pWork->handle=NULL;
    NHTTPCleanup();
    return err;
  }
  
  {
    NHTTPConnectionStatus   status = NHTTPGetConnectionStatus(pWork->handle);
    OSTick tickNow;

    if ( NHTTP_ERROR_NONE == NHTTPGetConnectionProgress(pWork->handle, &receivedCurrent, &contentLength))
    {

      NET_PRINT(" %d recv  \n",100*contentLength/receivedCurrent);

    }
  }
  return err;
}



void* NHTTP_RAP_GetRecvBuffer(NHTTP_RAP_WORK* pWork)
{
  return pWork->getbuffer;
}



NHTTP_RAP_WORK* NHTTP_RAP_Init(HEAPID heapID,u32 profileid)
{
  NHTTP_RAP_WORK* pWork = GFL_HEAP_AllocClearMemory( heapID, sizeof(NHTTP_RAP_WORK) );
  pWork->profileid=profileid;

  pWork->a=1234;
  pWork->b=1323;
  pWork->c=1222;
  pWork->d=1444;

  
  return pWork;
}


void NHTTP_RAP_End(NHTTP_RAP_WORK* pWork)
{
  OS_TPrintf("%d\n", pWork->a);
  OS_TPrintf("%d\n", pWork->b);
  OS_TPrintf("%d\n", pWork->c);
  OS_TPrintf("%d\n", pWork->d);
  
  GFL_HEAP_FreeMemory(pWork);
}


#if PM_DEBUG
void NHTTP_DEBUG_GPF_HEADER_PRINT(gs_response* prep)
{

  NET_PRINT("処理結果コード       %d\n",prep->ret_cd);
  NET_PRINT("データ部バイトサイズ %d\n",prep->body_size);
  NET_PRINT("詳細エラーコード     %d\n",prep->desc_cd);
  NET_PRINT("%s\n",prep->desc_msg);
}
#endif


