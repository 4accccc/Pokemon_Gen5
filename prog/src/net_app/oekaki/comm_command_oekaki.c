//=============================================================================
/**
 * @file  comm_command_oekaki.c
 * @brief データを送るためのコマンドをテーブル化しています
 *        お絵かきボード用です
 * @author  Akito Mori
 * @date    2010.01.20
 */
//=============================================================================

#include <gflib.h>
#include "net/network_define.h"
#include "arc_def.h"

#include "print/wordset.h"
#include "system/bmp_menu.h"
#include "system/net_err.h"
#include "print/wordset.h"
#include "net_app/oekaki.h"
#include "oekaki_local.h"
#include "comm_command_oekaki.h"



static u8* _setPictureBuff( int netID, void* pWk, int size);
static int _getCRECW(void);
static int _getCOECW(void);

static void Oekaki_GraphicDataSend( OEKAKI_WORK *wk, int no, GFL_NETHANDLE *pNet);


static void CommOekakiBoardPicture(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardLinePos(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardLinePosServer(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardStop(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardReStart(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardEndChild(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardEnd(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);
static void CommOekakiBoardEndChildDecide(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle);


// CommCommandOekaki.cから定義するコマンドで対応するコールバック関数
const NetRecvFuncTable _OekakiCommPacketTbl[] = {
  // お絵かきが見る部分
  {CommOekakiBoardPicture,        _setPictureBuff}, // CO_OEKAKI_GRAPHICDATA  みんなで描いていた画像データ
  {CommOekakiBoardLinePos,        NULL, },          // CO_OEKAKI_LINEPOS,   タッチパネルで取得したポジションデータ
  {CommOekakiBoardLinePosServer,  NULL  },          // CO_OEKAKI_LINEPOS,   タッチパネルで取得したポジションデータ
  {CommOekakiBoardStop,           NULL  },          // CO_OEKAKI_STOP,      乱入者が来たので一旦ストップ
  {CommOekakiBoardReStart,        NULL  },          // CO_OEKAKI_RESTART,   乱入者処理が終わったので再会
  {CommOekakiBoardEndChild,       NULL  },          // CO_OEKAKI_END_CHILD,   子機が離脱
  {CommOekakiBoardEnd,            NULL  },          // CO_OEKAKI_END,     終了
  {CommOekakiBoardEndChildDecide, NULL  },          // CO_OEKAKI_END_CHILD_DECIDE, 子機が離脱シーケンスに移った
};


//==============================================================================
/**
 * お絵かきボード用通信コマンド登録処理
 *
 * @param   pWk   OEKAKI_WORK*
 *
 * @retval  none
 */
//==============================================================================
void OEKAKIBOARD_CommandInitialize( void* pWk )
{
  GFL_NET_AddCommandTable( GFL_NET_CMD_PICTURE,_OekakiCommPacketTbl,
                           NELEMS(_OekakiCommPacketTbl), pWk);
}


//==============================================================================
/**
 * 親機が接続するまでにかかれていた画像を一気に送信する
 * 
 * 
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardPicture(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK *)pWk;

  // 子機の場合は親機からもらった画像データを取り込む
  if(GFL_NET_SystemGetCurrentID()!=0){
    OEKAKIG_SPLIT_DATA *osd = (OEKAKIG_SPLIT_DATA *)pData;
    OS_Printf("画像取得 no=%d, xor=%08x\n",osd->no, osd->_xor);
    if(osd->no*1000 > OEKAKI_GRAPHI_SIZE){
      MI_CpuCopyFast( osd->chara, &wk->canvas_buf[osd->no*1000], OEKAKI_GRAPHI_SIZE%1000);
    }else{
      MI_CpuCopyFast( osd->chara, &wk->canvas_buf[osd->no*1000],  1000);
    }
    // BMPWINのバッファにコピーする
    {
      u8 *adr = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->OekakiBoard) );
      MI_CpuCopyFast( wk->canvas_buf, adr, OEKAKI_GRAPHI_SIZE);
    }
    // 画面に反映
    GFL_BMPWIN_MakeTransWindow( wk->OekakiBoard );

  // 親機は次のデータを送信する
  }else{
    OEKAKIG_SPLIT_DATA *osd = (OEKAKIG_SPLIT_DATA *)pData;
    OS_Printf("画像取得 no=%d, xor=%08x",osd->no, osd->_xor);
  
    if(wk->send_num*1000 < OEKAKI_GRAPHI_SIZE){
      wk->send_num++;
      Oekaki_GraphicDataSend(wk, wk->send_num, pNetHandle);
    }else{
      // 再開
      Oekaki_SendDataRequest( wk, CO_OEKAKI_RESTART, 0 );
      
    }
  }

}


//==============================================================================
/**
 * カーソル位置情報を受信したときのコールバック
 *
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardLinePos(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk     = (OEKAKI_WORK*)pWk;
  TOUCH_INFO  *result = (TOUCH_INFO*)pData;

  // OS_Printf("id[%d]line pos recv\n", netID);
  // 子機から貰ったデータを格納する
  if(netID!=0){
//  wk->ParentTouchResult[netID] = *result;
    OekakiTouchFifo_Set( result, &wk->TouchFifo, netID );
  }

}



//==============================================================================
/**
 * @brief   ラインデータを親機から受信した（全員描画)
 *
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardLinePosServer(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk     = (OEKAKI_WORK*)pWk;
  TOUCH_INFO  *result = (TOUCH_INFO*)pData;

    if(wk==NULL){  // このときはお絵かき準備ができていないので無視
        return;
    }

    
  if(netID==0){ // 絶対に親からしか来ないはずだが
    int i;
    for(i=0;i<OEKAKI_MEMBER_MAX;i++){
      wk->AllTouchResult[i] = result[i];
    }
  }
  
  if(wk->AllTouchResult[0].banFlag==OEKAKI_BAN_ON){
    OS_TPrintf("親機から操作禁止命令\n");
  }
}
//==============================================================================
/**
 * 通信画面を開始する
 *
 * @param   netID   通信ID
 * @param   size    データサイズ
 * @param   pData   受信データポインタ
 * @param   pWk     OEKAKI_WORK*
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardReStart(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;

  OS_Printf("親機からお絵かき再開通知\n");
  OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_NEWMEMBER_END, 0 );
  
  MI_CpuClearFast( wk->canvas_buf, OEKAKI_GRAPHI_SIZE );
  
  // 絵を共有した接続人数を更新
  wk->shareNum = Union_App_GetMemberNum( wk->param->uniapp );
  wk->shareBit = Union_App_GetMemberNetBit(wk->param->uniapp);
  wk->banFlag  = OEKAKI_BAN_OFF;
  OS_Printf("shareBit更新=%d\n", wk->shareBit);

}
//==============================================================================
/**
 * @brief   子機が離脱
 *
 * @param   netID   
 * @param   size    
 * @param   pData   離脱者のID(親からの送信で最上位ビットが立っている場合は離脱NG)
 * @param   pWk   OEKAKI_WORK*
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardEndChild(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;
  COMM_OEKAKI_END_CHILD_WORK *recieve_work = (COMM_OEKAKI_END_CHILD_WORK*)pData;
  u32 ret;

  // 親機からきた場合はみんなで「●●●がいなくなりました」とだす
  if(netID!=0){
    //子機(離脱者)からの送信
    //離脱します通知
    if(GFL_NET_SystemGetCurrentID()==0){
      // 子機から(親機が受け取る)
      // 子機がいなくなった事を全員に通知する
      wk->send_req.trans_work = *recieve_work;
      wk->send_req.trans_work.ridatu_id = netID;
      wk->send_req.trans_work.oya_share_num = wk->shareNum;
      switch(recieve_work->request){
      case COEC_REQ_RIDATU_CHECK:
        if(wk->shareNum != Union_App_GetMemberNum( wk->param->uniapp ) 
            || wk->shareNum != MATH_CountPopulation(Union_App_GetMemberNetBit(wk->param->uniapp))){
          wk->send_req.trans_work.ridatu_kyoka = FALSE;  //離脱NG！
        }
        else{
//          wk->ridatu_bit |= 1 << netID;
          wk->send_req.trans_work.ridatu_kyoka = TRUE;
          //離脱OKなので参加制限をかける(乱入があればそちら側で制限がはずされるはず)
          Union_App_Parent_ResetEntryBlock( wk->param->uniapp);   // 乱入NG
        }
        break;
      case COEC_REQ_RIDATU_EXE:
        break;
      }
      Oekaki_SendDataRequest( wk, CO_OEKAKI_END_CHILD, 0 );

      OS_Printf("子機%dから離脱を受け取った→送信\n",netID);
    }
  
  }else{
  // 親機から子機離脱の通知が来た
    switch(recieve_work->request){
    case COEC_REQ_RIDATU_CHECK:
      //離脱確認なので、離脱しようとした子機にのみ結果を送る
      if(recieve_work->ridatu_id == GFL_NET_SystemGetCurrentID()){
        if(recieve_work->ridatu_kyoka == FALSE){
          OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_END_SELECT_ANSWER_NG, recieve_work->ridatu_id );
          OS_Printf("離脱NGを受信\n");
        }
        else{
          wk->oya_share_num = recieve_work->oya_share_num;
          OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_END_SELECT_ANSWER_OK, recieve_work->ridatu_id );
          wk->status_end = TRUE;
          OS_Printf("離脱OKを受信\n");
        }
      }
      break;
    case COEC_REQ_RIDATU_EXE:
      OS_TPrintf("親機が子機%dの離脱通知した\n", recieve_work->ridatu_id);
      // 自分も離脱しているなら切り替える子機の離脱待ちに切り替える必要は無い
      OS_Printf("wk->status_end=%d\n",wk->status_end);
      if(wk->status_end){
        OS_Printf("既に離脱待ちなので人の離脱処理を待つ必要が無い\n");
      }else{
        OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_LOGOUT_CHILD, recieve_work->ridatu_id );
      }
      break;
    }
  }
}


//==============================================================================
/**
 * @brief   子機が本当に離脱を開始した(もうどんな事があっても子機のシーケンスは書き変わらない）
 *
 * @param   netID 
 * @param   size  
 * @param   pData 
 * @param   pWk   OEKAKI_WORK*
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardEndChildDecide(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;

  // 親機は離脱子機をbit情報で保存(この子機は絶対に抜ける）
  if(GFL_NET_SystemGetCurrentID()==0){
    wk->ridatu_bit |= 1 << netID;
    OS_Printf("子機%dから『絶対抜けます！』ってきたよ\n", netID);
  }
}


//==============================================================================
/**
 * @brief   「子機が乱入してきたので一旦絵を送るよ止まってね」
 *      と親機が送信してきた時のコールバック
 *
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardStop(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{

  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;
  u8 id;

  // 強制終了中だったら何もしない
  if(wk->force_end){
    return;
  }

  id = *(u8*)pData;
  // メインシーケンス変更
  OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_NEWMEMBER, id );

  // 親機が画像データ送信を開始する
  if(GFL_NET_SystemGetCurrentID()==0 && wk->proc_seq == SEQ_MAIN){
    wk->send_num = 0;

    //グラフィック分割送信
    Oekaki_GraphicDataSend(wk, wk->send_num, pNetHandle);
  }


  OS_Printf("親機からの「子機%dに絵を送るから止まってね」通知\n",id);

}


static int _shareBit_count( int shareBit )
{
  int i,count=0;
  for(i=0;i<5;i++){
    if(shareBit&1){
      count++;
    }
    shareBit>>=1;
  }
  return count;
}

//==============================================================================
/**
 * @brief   親機がやめるので強制終了させる
 *
 * @param   netID   
 * @param   size    
 * @param   pData   
 * @param   pWk   
 *
 * @retval  none    
 */
//==============================================================================
static void CommOekakiBoardEnd(
    const int netID, const int size, const void* pData, void* pWk, GFL_NETHANDLE* pNetHandle)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;
  OS_Printf("親機からの終了通知がきたのでやめる\n");

  // 乱入処理を無視する
  wk->force_end = TRUE;
  
  OS_Printf("share人数=%d\n",_shareBit_count(wk->shareBit));
  OS_Printf("union_app人数=%d\n",Union_App_GetMemberNum(wk->param->uniapp));
  // 親機以外はこのコマンドで強制しゅうりょうする
  if(GFL_NET_SystemGetCurrentID() != 0){
    OekakiBoard_MainSeqForceChange( wk, OEKAKI_MODE_FORCE_END, 0  );
  }
}


//------------------------------------------------------------------
/**
 * @brief   グラフィックデータを分割送信
 *
 * @param   wk    
 * @param   no    
 *
 * @retval  none    
 */
//------------------------------------------------------------------
static void Oekaki_GraphicDataSend( OEKAKI_WORK *wk, int no, GFL_NETHANDLE *pNet)
{
  // 送信バッファにコピー
  u8 sendbit;
  u8 *p = GFL_BMP_GetCharacterAdrs( GFL_BMPWIN_GetBmp(wk->OekakiBoard) );
  MI_CpuCopyFast( &p[no*1000], wk->send_buf.chara, 1000 ); 

  // xor取得・格納
  {
    int i;
    u32 *p,result;
    p = (u32*)wk->send_buf.chara;
    for(result=0,i=0;i<1000/4;i++){
      result ^= p[i];
    }
    wk->send_buf._xor        = result;
  }

  // 分割番号を書き込む
  wk->send_buf.no = no;
  
  
  // 送信開始
  sendbit = Union_App_GetMemberNetBit( wk->param->uniapp );
  GFL_NET_COMMAND_SendHugeData( GFL_NET_GetNetHandle( GFL_NET_NETID_SERVER), sendbit, 
                                CO_OEKAKI_GRAPHICDATA, sizeof(OEKAKIG_SPLIT_DATA),&wk->send_buf);
  OS_TPrintf("sendbit=%d, systemConnect=%d\n", sendbit, GFL_NET_SystemGetConnectNum());

  OS_Printf("送信データ no=%d, xor = %08x\n",  wk->send_buf.no,  wk->send_buf._xor);
}




//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------
// パレットサイズ通知関数群
//--------------------------------------------------------------------------------
//--------------------------------------------------------------------------------



//--------------------------------------------------------------------------------
// 受信データサイズを返す関数
//--------------------------------------------------------------------------------
static u8* _setPictureBuff( int netID, void* pWk, int size)
{
  OEKAKI_WORK *wk = (OEKAKI_WORK*)pWk;

  return (u8*)&wk->split_temp[netID];
}

//--------------------------------------------------------------------------------
// バイトを表す関数
//--------------------------------------------------------------------------------
static int _getCRECW(void)
{
  return 4;
}

//--------------------------------------------------------------------------------
// バイトを表す関数
//--------------------------------------------------------------------------------
static int _getCOECW(void)
{
  return sizeof(COMM_OEKAKI_END_CHILD_WORK);
}
