//============================================================================================
/**
 * @file  gfl_msgdata.h
 * @brief 複数言語に対応したメッセージデータ
 * @author  taya GAME FREAK inc.
 */
//============================================================================================
#ifndef __GFL_MSGDATA_H__
#define __GFL_MSGDATA_H__

#include <strbuf.h>


#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------
/**
 *  メッセージデータハンドル型
 */
//----------------------------------------------------
typedef struct _GFL_MSGDATA   GFL_MSGDATA;

//----------------------------------------------------
/**
 *  メッセージデータのロード方式
 */
//----------------------------------------------------
typedef enum {
  GFL_MSG_LOAD_NORMAL,  ///< 最低限のヘッダだけをオンメモリにし、必要に応じてファイルアクセスする。
  GFL_MSG_LOAD_FAST,    ///< 必要な全てのデータをオンメモリにする。高速化が必要な個所で使用してください。
}GflMsgLoadType;



extern GFL_MSGDATA* GFL_MSG_Create( GflMsgLoadType loadType, u16 ArcID, u16 datID, HEAPID heapID );
extern GFL_MSGDATA* GFL_MSG_Construct( void* adrs, HEAPID heapID );
extern void GFL_MSG_Delete( GFL_MSGDATA* handle );
extern void GFL_MSG_GetString( const GFL_MSGDATA* handle, u32 strID, STRBUF* dst );
extern STRBUF* GFL_MSG_CreateString( const GFL_MSGDATA* handle, u32 strID );
extern u32 GFL_MSG_GetStrCount( const GFL_MSGDATA* handle );
extern u16 GFL_MSG_GetDispAreaWidth( const GFL_MSGDATA* handle, u32 strID );

/*============================================================================================*/
/*                                                                                            */
/*  ロード対象言語の切り替え処理                                                              */
/*                                                                                            */
/*  同一の文字列IDを使って、実際に読み出される文字列を切り替えるための仕組み。                */
/*  日本語版の『ひらがな－漢字』切り替えとか、                                                */
/*  欧州版の『英・仏・独・伊・西』５ヶ国語切り替えとかに使ってもらうことを想定。              */
/*                                                                                            */
/*============================================================================================*/

//--------------------------------------------------------------------------------------
/**
 * ロード対象となる言語IDを設定する。
 * 以降、文字列取得関数を介して取得できる文字列が切り替わる。
 *
 * @param   langID    言語ID
 */
//--------------------------------------------------------------------------------------
extern void GFL_MSGSYS_SetLangID( u8 langID );

//--------------------------------------------------------------------------------------
/**
 * 設定されたロード対象の言語IDを取得。
 *
 * @retval  u8    設定されている言語ID
 */
//--------------------------------------------------------------------------------------
extern u8 GFL_MSGSYS_GetLangID( void );




/*============================================================================================*/
/*                                                                                            */
/*  以下の関数はSTRBUFで隠蔽されていない生の文字配列を扱う。使用個所は限定＆許可制。          */
/*                                                                                            */
/*============================================================================================*/
extern void GFL_MSG_GetStringRaw( const GFL_MSGDATA* handle, u32 strID, STRCODE* dstAry, u32 aryElems );



#ifdef __cplusplus
}/* extern "C" */
#endif


#endif  /* #ifndef __GFL_MSGDATA_H__ */

