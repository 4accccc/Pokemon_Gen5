#ifndef __VHCAT_H_
#define __VHCAT_H_
// ボイスチャット関係。
// dwc_rap.c とvchat.cからのみ呼び出される。
// ボイスチャットを呼ぶ場合は、dwc_rap.h経由で呼ぶ。

extern void myvct_init( int heapID, int codec,int maxEntry );
extern void myvct_setCodec( int codec );

//==============================================================================
/**
 * @brief   ボイスチャットメイン
 * @param   offflg   音を送らない場合＝一時的にオフにする場合 TRUE
 * @retval  none
 */
//==============================================================================
extern void myvct_main( BOOL offflg);
extern BOOL myvct_checkData( int aid, void *data, int size );

//==============================================================================
/**
 * 会話終了処理完了コールバックを設定します。
 * 相手から切られた場合も呼び出されます。
 * この関数が呼び出される直前に、vchat.c用のワークが解放されます。
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void myvct_setDisconnectCallback( void (*disconnectCallback)() );

//==============================================================================
/**
 * 会話終了要求をだします。まだ通話できていないときは即座に終了します。
 * myvct_setDisconnectCallbackで設定されたコールバックが呼び出されます。
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void myvct_endConnection();

//==============================================================================
/**
 * ライブラリ終了処理。この中から会話終了処理完了コールバックも呼ばれます。
 * 通常終了時はこの関数は、会話終了要求から呼び出されます。
 * @param   none
 * @retval  none
 */
//==============================================================================
extern void myvct_free();

//==============================================================================
/**
 * 音声のノイズカットレベルを調整します（外部からアクセスしたいので、こちらに）
 * @param   d … 今より閾値を下げるか、上げるか（下げるほど拾いやすくなる）
 * @retval  none
 */
//==============================================================================
extern void myvct_changeVADLevel(int d);


// 送信したかどうかを返します
extern BOOL myvct_IsSendVoiceAndInc(void);


//==============================================================================
/**
 * VCTカンファレンスに招待する
 * @param   bitmap   接続しているCLIENTのBITMAP
 * @param   myAid    自分のID
 * @retval  TRUE 設定できた
 */
//==============================================================================

extern BOOL myvct_AddConference(int bitmap, int myAid);

//==============================================================================
/**
 * VCTカンファレンスから全員はずす
 * @param   bitmap   接続しているCLIENTのBITMAP
 * @param   myAid    自分のID
 * @retval  TRUE 設定できた
 * @retval  FALSE 現状のまま もしくは必要がない
 */
//==============================================================================

extern BOOL myvct_DelConference(int myAid);

extern void VCHAT_PauesFlg(BOOL bPause);

#endif

