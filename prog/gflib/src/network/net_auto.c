


typedef struct _NET_AUTO_STATE NET_AUTO_STATE ;

/// コールバック関数の書式 内部ステート遷移用
typedef void (*PTRStateFunc)(NET_AUTO_STATE* pState);

struct _NET_AUTO_STATE{
//  NET_PARENTSYS sParent;  ///< 親の情報を保持するポインタ
  PTRStateFunc state;      ///< ハンドルのプログラム状態
  PSyncroCallback pSCallBack;  ///< 自動接続時に同期が完了した場合呼ばれる
  u16 timer;           ///< 進行タイマー
  u8 autoConnectNum;     ///< 自動接続に必要な人数
};

static NET_AUTO_STATE* _pNetState = NULL;


//==============================================================================
// static宣言
//==============================================================================

// ステートの変更
static void _changeStateDebug(NET_AUTO_STATE* pState, PTRStateFunc state, int time, int line);  // ステートを変更する
static void _changeState(NET_AUTO_STATE* pState, PTRStateFunc state, int time);  // ステートを変更する

#ifdef GFL_NET_DEBUG
#define   _CHANGE_STATE(state, time)  _changeStateDebug(_pNetState ,state, time, __LINE__)
#else  //GFL_NET_DEBUG
#define   _CHANGE_STATE(state, time)  _changeState(_pNetState ,state, time)
#endif //GFL_NET_DEBUG


// その他一般的なステート
static void _stateNone(NET_AUTO_STATE* pState);            // 何もしない


//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================

static void _changeState(NET_AUTO_STATE* pState,PTRStateFunc state, int time)
{
    pState->state = state;
    pState->timer = time;
}

//==============================================================================
/**
 * @brief   通信管理ステートの変更
 * @param   state  変えるステートの関数
 * @param   time   ステート保持時間
 * @retval  none
 */
//==============================================================================
#ifdef GFL_NET_DEBUG
static void _changeStateDebug(NET_AUTO_STATE* pState,PTRStateFunc state, int time, int line)
{
    NET_PRINT("net_state: %d\n",line);
    _changeState(pState, state, time);
}
#endif

#if 0
BOOL KEY_YT_NET_Main(NET_PARAM* pNet)
{
    switch(pNet->seq){
      case _INIT_WAIT_PARENT:
        if(GFL_NET_IsInit()){
            _SEQCHANGE(_CONNECT_WAIT);
        }
        break;
      case _INIT_WAIT_CHILD:
        if(GFL_NET_IsInit()){
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
        if(KEY_YT_NET_IsParent(pNet)){  //親機の場合
            if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
                if(GFL_NET_HANDLE_RequestNegotiation()){
                    _SEQCHANGE( _TIM_START );
                }
            }
        }
        else{
//            if(GFL_NET_HANDLE_GetNumNegotiation( pNet->pNetHandle[1]) != 0){
                if(GFL_NET_HANDLE_RequestNegotiation( )){
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
#endif


//==============================================================================
/**
 * @brief   同期が取れたらコールバックを呼んで終わり
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _autoInitSyncroServer(GFL_NETSTATE* pState)
{
    GFLNetInitializeStruct* pIni = _GFL_NET_GetNETInitStruct();

    if(GFL_NET_IsTimingSync(GFL_NET_HANDLE_GetCurrentHandle() , _TEST_TIMING)){
        if(_pNetState->pSCallBack){
            _pNetState->pSCallBack(pIni->pWork);
            _CHANGE_STATE(_stateNone, 0);
        }
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーションが人数分そろったら同期をとる
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _autoInitHandlingServer(GFL_NETSTATE* pState)
{
    if(GFL_NET_HANDLE_GetNumNegotiation()==_pNetState->autoConnectNum){
        GFL_NET_TimingSyncStart(GFL_NET_HANDLE_GetCurrentHandle() , _TEST_TIMING);
        _CHANGE_STATE(_autoInitSyncroServer)
    }
}

//==============================================================================
/**
 * @brief   ネゴシエーションを監視して投げ返す
 * @param   none
 * @retval  none
 */
//==============================================================================

static void _autoInitWaitServer(GFL_NETSTATE* pState)
{
    if(GFL_NET_IsInit()){
        if(GFL_NET_HANDLE_GetNumNegotiation() != 0){
            if(GFL_NET_HANDLE_RequestNegotiation()){
                _CHANGE_STATE(_autoInitHandlingServer);
            }
        }
    }
}

//==============================================================================
/**
 * @brief   自動でサーバを立ち上げる 認証等はすべてお任せ
 * @param   num     接続同期に必要な人数
 * @param   num     人数分の同期が取れたとき呼ばれるコールバック
 * @retval  none
 */
//==============================================================================

void GFL_NET_AutoInitServer(int num,PSyncroCallback pSyncroCallback)
{
    _pNetState->autoConnectNum = num;
    _pNetState->pSCallBack = pSyncroCallback;
    _CHANGE_STATE(_autoInitWaitServer);


}
