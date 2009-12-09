/**
 *  @file mailview.h
 *  @brief  メール描画ルーチン　ローカルヘッダ
 *  @author MiyukiIwasawa
 *  @date 06.02.08
 */

#ifndef _H_MAIL_VIEW_H_
#define _H_MAIL_VIEW_H_

#define MAIL_KEY_IN (APP_END_KEY)
#define MAIL_TOUCH_IN (APP_END_TOUCH)

typedef enum{
  MAILMODE_CREATE,  ///<作成する
  MAILMODE_VIEW_D,  ///<デザインをみる
  MAILMODE_VIEW,    ///<作成済みのメールをみる
}MAILMODE_ENUM;

enum{
 VIEW_END_LINE0 = 0,
 VIEW_END_LINE1,
 VIEW_END_LINE2,
 VIEW_END_DECIDE,
 VIEW_END_CANCEL = 0xffff,
};

///メールデータテンポラリ
typedef struct _MAIL_TMP_DATA{
  u16 val;
  u8  cntNo;
  u8  flags;
//  CONFIG* configSave; ///<コンフィグセーブデータ
//  KEYTOUCH_STATUS* kt_status; ///<キーorタッチステータス

  u32 writerID;
  u8  sex;
  u8  lang;
  u8  ver;
  u8  design;
  STRBUF  *name;
  MAIL_ICON icon[MAILDAT_ICONMAX];
  PMS_DATA  msg[MAILDAT_MSGMAX];
}MAIL_TMP_DATA;

extern GFL_PROC_RESULT MailViewProc_Init( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MailViewProc_Main( GFL_PROC * proc, int *seq, void *pwk, void *mywk );
extern GFL_PROC_RESULT MailViewProc_End( GFL_PROC * proc, int *seq, void *pwk, void *mywk );

#endif  //_H_MAIL_VIEW_H_


