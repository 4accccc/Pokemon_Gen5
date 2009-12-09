 /**
 *  @file mailtool.h
 *  @brief  メールシステムヘッダ
 *  @author Miyuki Iwasaw
 *  @date 06.02.07
 */

#pragma once


//メールシステム　パラメータ引継ぎワーク
#include "app/mail/mail_param.h"

extern GFL_PROC_RESULT MailSysProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MailSysProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MailSysProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );


extern const GFL_PROC_DATA MailSysProcData;

/**
 *  @brief  メール画面呼び出しワーク作成(Create)
 *
 *  @retval NULL  新規作成領域がない
 *  @retval ワークポインタ  メール作成画面呼び出し
 */
extern MAIL_PARAM* MAILSYS_GetWorkCreate( GAMEDATA* gamedata,
  MAILBLOCK_ID blockID,u8 poke_pos,u8 design, int heapID);


/**
 *  @brief  メール画面呼び出しワーク作成(View)
 *
 *  @retval NULL  新規作成領域がない
 *  @retval ワークポインタ  メール描画画面呼び出し
 */
extern MAIL_PARAM* MailSys_GetWorkView( GAMEDATA* gamedata, MAILBLOCK_ID blockID, 
                                        u16 dataID, int heapID );

/**
 *  @brief  メール画面呼び出しワーク作成(View/POKEMON_PARAM)
 */
extern MAIL_PARAM* MailSys_GetWorkViewPoke( GAMEDATA* gamedata, POKEMON_PARAM* poke, int heapID );


/**
 *  @brief  メール画面呼び出しワーク作成(View/空メールプレビュー)
 */
extern MAIL_PARAM* MailSys_GetWorkViewPrev( GAMEDATA* gamedata, u8 designNo, int heapID );



/**
 *  @brief  直前のモジュール呼び出しでデータが作成されたかどうか？
 */
extern BOOL MailSys_IsDataCreate( MAIL_PARAM* wk);

/**
 *  @brief  直前のモジュール呼び出しで作成されたデータをセーブデータに反映
 */
extern int MailSys_PushDataToSave( MAIL_PARAM* wk,MAILBLOCK_ID blockID,u8 dataID );

/**
 *  @brief  直前のモジュール呼び出しで作成されたデータをセーブデータに反映(Poke)
 */
extern int MailSys_PushDataToSavePoke( MAIL_PARAM* wk, POKEMON_PARAM* poke );

/**
 *  @brief  メールモジュール呼び出しワークを解放
 */
extern void MailSys_ReleaseCallWork( MAIL_PARAM* wk);




/**
 *  @brief  ポケモンメールをパソメールに移動
 *
 *  @retval MAILDATA_NULLID 空きがないので転送できない
 *  @retval "その他"　転送したデータID
 */
extern int  MailSys_MoveMailPoke2Paso(MAIL_BLOCK* block,POKEMON_PARAM* poke,HEAPID heapID);

/**
 *  @brief  パソメールをポケモンに移動
 *
 *  @retval MAILDATA_NULLID IDが不正なのでなにもしなかった
 *  @retval "その他"　転送したデータID
 */
extern int  MailSys_MoveMailPaso2Poke(MAIL_BLOCK* block,u16 id,POKEMON_PARAM* poke,HEAPID heapID);



