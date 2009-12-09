/**
 *  @file mail.c
 *  @brief  メールシステム
 *  @author Miyuki Iwasaw
 *  @date 06.02.07
 */

#include <gflib.h>
#include "poke_tool/poke_tool.h"
#include "item/itemsym.h"
#include "item/item.h"
#include "gamesystem/game_data.h"
#include "savedata/mail_util.h"
#include "app/mailtool.h"

/*
#include "system/procsys.h"
#include "system/snd_tool.h"
#include "system/fontproc.h"
#include "system/pm_str.h"
#include "system/buflen.h"
#include "system/palanm.h"
#include "system/savedata.h"
#include "itemtool/itemsym.h"
#include "itemtool/item.h"
#include "gflib/strbuf_family.h"

#include "savedata/config.h"
#include "savedata/record.h"
#include "application/mail/mail_param.h"
#include "application/mailtool.h"
#include "application/mail/mailview.h"
#include "application/pms_input.h"
#include "mail_snd_def.h"
*/
/**
 *  @brief  メール画面呼び出しワーク作成(Create)
 *
 *  @retval NULL  新規作成領域がない
 *  @retval ワークポインタ  メール作成画面呼び出し
 */
//=============================================================================================
/**
 * @brief メール画面呼び出しワーク作成(Create)
 *
 * @param   gamedata  GAMEDATA
 * @param   blockID   メールブロック位置ID
 * @param   poke_pos  ポケモン参照開始位置
 * @param   design    デザインNO
 * @param   heapID    ヒープID
 *
 *  @retval NULL  新規作成領域がない
 *  @retval ワークポインタ  メール作成画面呼び出し
 */
//=============================================================================================
MAIL_PARAM* MAILSYS_GetWorkCreate( GAMEDATA* gamedata, MAILBLOCK_ID blockID, 
            u8 poke_pos,u8 design,int heapID)
{
  MAIL_PARAM* wk;
  MAIL_BLOCK* pBlock;
  int id;
  
  //メールセーブデータブロック取得
  pBlock = GAMEDATA_GetMailBlock( gamedata ); 

  //モジュール呼び出し用のワークを作成
  wk = GFL_HEAP_AllocMemory( GetHeapLowID(heapID), sizeof(MAIL_PARAM));

  MI_CpuClearFast(wk,sizeof(MAIL_PARAM));
  wk->designNo = design;
  wk->poke_pos = poke_pos;
  wk->pBlock   = pBlock;

  //呼び出しモード設定
  wk->mode     = MAIL_MODE_CREATE;
  wk->blockID  = blockID; //データブロックID取得
  wk->dataID   = 0; //IDは0初期化
  wk->savedata = GAMEDATA_GetSaveControlWork(gamedata);

  //データテンポラリ作成
  wk->pDat = MailData_CreateWork(heapID);
  MailData_Clear(wk->pDat);

  //初期データ格納(デザインNoは無効にして呼び出す)
  MAILDATA_CreateFromSaveData(wk->pDat,MAIL_DESIGN_NULL,poke_pos, gamedata );
  return wk;
}


/**
 *  @brief  メール画面呼び出しワーク作成(View)
 *
 *  @retval NULL  新規作成領域がない
 *  @retval ワークポインタ  メール描画画面呼び出し
 */
//=============================================================================================
/**
 * @brief メール画面呼び出しワーク作成(View)
 *
 * @param   gamedata  GAMEDATA
 * @param   blockID   メールブロック位置ID
 * @param   dataID    
 * @param   heapID    
 *
 * @retval  MAIL_PARAM*   
 */
//=============================================================================================
MAIL_PARAM* MailSys_GetWorkView( GAMEDATA* gamedata, MAILBLOCK_ID blockID, u16 dataID,int heapID )
{
  MAIL_PARAM* wk;
  MAIL_BLOCK* pBlock;

  //メールセーブデータブロック取得
  pBlock = GAMEDATA_GetMailBlock( gamedata ); 

  //モジュール呼び出し用のワークを作成
  wk = GFL_HEAP_AllocMemory( GetHeapLowID(heapID), sizeof(MAIL_PARAM));
  MI_CpuClearFast(wk,sizeof(MAIL_PARAM));

  //呼び出しモード設定
  wk->mode     = MAIL_MODE_VIEW;
  wk->blockID  = blockID;
  wk->dataID   = dataID;
  wk->savedata = GAMEDATA_GetSaveControlWork(gamedata);
  wk->pBlock   = pBlock;
  
  //セーブデータ取得
  wk->pDat = MAIL_AllocMailData(pBlock,blockID,dataID,heapID);
  return wk;
}

/**
 *  @brief  メール画面呼び出しワーク作成(View/POKEMON_PARAM)
 */
//=============================================================================================
/**
 * @brief   メール画面呼び出しワーク作成(View/POKEMON_PARAM)
 *
 * @param   gamedata  GAMEDATA
 * @param   poke      入れるPOKEMON_PARAM
 * @param   heapID    ヒープID
 *
 * @retval  MAIL_PARAM*   作成したメール表示PARAM
 */
//=============================================================================================
MAIL_PARAM* MailSys_GetWorkViewPoke( GAMEDATA* gamedata, POKEMON_PARAM* poke, int heapID )
{
  MAIL_PARAM* wk;
  
  //モジュール呼び出し用のワークを作成
  wk = GFL_HEAP_AllocMemory( GetHeapLowID(heapID), sizeof(MAIL_PARAM));
  MI_CpuClearFast(wk,sizeof(MAIL_PARAM));
  
  //呼び出しモード設定
  wk->mode     = MAIL_MODE_VIEW;
  wk->savedata = GAMEDATA_GetSaveControlWork(gamedata);
  
  //ワークを取得
  wk->pDat = MailData_CreateWork(heapID);
  //メールデータ取得
  PP_Get( poke, ID_PARA_mail_data, wk->pDat );
  return wk;
}


/**
 *  @brief  
 */
//=============================================================================================
/**
 * @brief メール画面呼び出しワーク作成(View/空メールプレビュー)
 *
 * @param   gamedata  GAMEDATA
 * @param   designNo  メールデザインID
 * @param   heapID    ヒープID
 *
 * @retval  MAIL_PARAM*   メール表示用ワーク
 */
//=============================================================================================
MAIL_PARAM* MailSys_GetWorkViewPrev( GAMEDATA *gamedata, u8 designNo, int heapID )
{
  MAIL_PARAM* wk;
  
  //モジュール呼び出し用のワークを作成
  wk = GFL_HEAP_AllocMemory( GetHeapLowID(heapID), sizeof(MAIL_PARAM));
  MI_CpuClearFast(wk,sizeof(MAIL_PARAM));

  
  //呼び出しモード設定
  wk->mode     = MAIL_MODE_VIEW;
  wk->savedata = GAMEDATA_GetSaveControlWork(gamedata);

  //ワークを取得
  wk->pDat = MailData_CreateWork(heapID);

  //デザインNoのみ設定
  MailData_SetDesignNo(wk->pDat,designNo);  
  return wk;
}



//=============================================================================================
/**
 * @brief 直前のモジュール呼び出しでデータが作成されたかどうか？
 *
 * @param   wk    
 *
 * @retval  BOOL  TRUE:作成された FALSE：作成されてない
 */
//=============================================================================================
BOOL MailSys_IsDataCreate( MAIL_PARAM* wk )
{
  return wk->ret_val;
}

//=============================================================================================
/**
 * @brief 直前のモジュール呼び出しで作成されたデータをセーブデータに反映
 *
 * @param   wk      
 * @param   blockID 手持ちかメールボックスか
 * @param   dataID  ボックスの場合に格納する場所を指定する
 *
 * @retval  int 0:作成されていない  1:格納された
 */
//=============================================================================================
int MailSys_PushDataToSave( MAIL_PARAM* wk, MAILBLOCK_ID blockID, u8 dataID )
{
  if(!MailSys_IsDataCreate(wk)){
    return 0; //作成されていない
  }
  MAIL_AddMailFormWork( wk->pBlock, blockID, dataID, wk->pDat );
  return 1;
}

//=============================================================================================
/**
 * @brief   直前のモジュール呼び出しで作成されたデータをセーブデータに反映(Poke)
 *
 * @param   wk    
 * @param   poke  メールを格納するポケモン構造体
 *
 * @retval  int   0:作成されていない  1:格納された
 */
//=============================================================================================
int MailSys_PushDataToSavePoke( MAIL_PARAM* wk, POKEMON_PARAM* poke )
{
  if(!MailSys_IsDataCreate(wk)){
    return 0; //作成されていない
  }
  PP_Put( poke, ID_PARA_mail_data, (u32)wk->pDat );
  return 1;
}

//=============================================================================================
/**
 * @brief メールモジュール呼び出しワークを解放
 *
 * @param   wk    
 */
//=============================================================================================
void MailSys_ReleaseCallWork(MAIL_PARAM* wk)
{
  //データテンポラリを確保していた場合は解放
  if(wk->pDat != NULL){
    GFL_HEAP_FreeMemory( wk->pDat );
  }
  GFL_HEAP_FreeMemory( wk );
}

/**
 *  @brief  ポケモンメールをパソメールに移動
 *
 *  @retval MAILDATA_NULLID 空きがないので転送できない
 *  @retval "その他"　転送したデータID
 */

//=============================================================================================
/**
 * @brief ポケモンメールをパソメールに移動
 *
 * @param   block   メールブロックへのポインタ
 * @param   poke    ポケモン構造体
 * @param   heapID  ヒープID
 *
 *  @retval MAILDATA_NULLID 空きがないので転送できない
 *  @retval "その他"　転送したデータID
 */
//=============================================================================================
int MailSys_MoveMailPoke2Paso(MAIL_BLOCK* block,POKEMON_PARAM* poke,HEAPID heapID)
{
  int id;
  int itemno = ITEM_DUMMY_DATA;
  MAIL_DATA* src = NULL;

  //空き領域検索
  id = MAIL_SearchNullID(block,MAILBLOCK_PASOCOM);
  if(id == MAILDATA_NULLID){
    return MAILDATA_NULLID;
  }
  //ワーク作成
  src = MailData_CreateWork(heapID);

  //ポケモンからメールを取得
  PP_Get(poke,ID_PARA_mail_data,src);

  //パソコン領域にデータコピー
  MAIL_AddMailFormWork(block,MAILBLOCK_PASOCOM,id,src);

  //ポケモンからメールを外す
  MailData_Clear(src);
  PP_Put(poke,ID_PARA_mail_data,(u32)src);
  PP_Put(poke,ID_PARA_item,itemno);

  //領域解放
  GFL_HEAP_FreeMemory(src);

  return id;
}

//=============================================================================================
/**
 * @brief   パソメールをポケモンに移動
 *
 * @param   block   メールブロックへのポインタ
 * @param   id      メールを取り出す場所
 * @param   poke    格納するポケモン構造体
 * @param   heapID  ヒープID
 *
 *  @retval MAILDATA_NULLID IDが不正なのでなにもしなかった
 *  @retval "その他"　転送したデータID
 */
//=============================================================================================
int MailSys_MoveMailPaso2Poke(MAIL_BLOCK* block,u16 id,POKEMON_PARAM* poke,HEAPID heapID)
{
  int itemno = ITEM_DUMMY_DATA;
  MAIL_DATA* src = NULL;

  //メールデータを取得
  src = MAIL_AllocMailData(block,MAILBLOCK_PASOCOM,id,heapID);
  if(src == NULL){
    return MAILDATA_NULLID;
  }
  //メールのデザインNoからアイテムNoに復元
  itemno = ITEM_MailNumGet(MailData_GetDesignNo(src));
  //ポケモンにメールを移す
  PP_Put(poke,ID_PARA_mail_data,(u32)src);
  PP_Put(poke,ID_PARA_item,itemno);

  //パソコン領域のデータを消す
  MAIL_DelMailData(block,MAILBLOCK_PASOCOM,id);

  //領域解放
  GFL_HEAP_FreeMemory(src);
  return id;
}




//===========================================================================
//デバッグ用ルーチン
//===========================================================================
