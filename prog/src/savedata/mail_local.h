/**
 *  @file mail_local.h
 *  @brief  メールセーブデータ制御モジュールのみがincludeして良い
 *  @author Miyuki Iwasawa
 *  @date 06.03.02
 */

#ifndef __MAIL_LOCAL_H__
#define __MAIL_LOCAL_H__

#include "savedata/mail.h"

///メールデータ型
//=============================================================
//  外部公開していますが、直接構造体メンバへアクセスすることは
//  許可していません。
//
//  メンバアクセスはsavedata/mail_util.hに定義された
//  アクセス系関数を介してください
//
//  size=56byte
//
//  構造体サイズを4byte境界に合わせるため、MAIL_ICON配列要素数に+1して
//  パディングを行っています
//=============================================================
typedef struct _MAIL_DATA{
  u32 writerID; //<トレーナーID 4
  u8  sex;      //<主人公の性別 1
  u8  region;   //<国コード 1
  u8  version;  //<カセットバージョン 1
  u8  design;   //<デザインナンバー 1
  STRCODE name[PERSON_NAME_SIZE+EOM_SIZE];  // 16
  MAIL_ICON icon[MAILDAT_ICONMAX];          //<アイコンNO格納場所[]
  u16 form_bit;                             //padding領域をプラチナから3体のポケモンの
                                            // フォルム番号として使用(5bit単位)
  PMS_DATA  msg[MAILDAT_MSGMAX];            //<文章データ
}_MAIL_DATA;


///メールデータセーブデータブロック定義構造体
typedef struct _MAIL_BLOCK{

  MAIL_DATA paso[MAIL_STOCK_PASOCOM];   // メール最大数20個

}_MAIL_BLOCK;

#endif  //__MAIL_LOCAL_H__

