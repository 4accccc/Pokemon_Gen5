//============================================================================================
/**
 * @file  trcard_snd_def.h
 * @bfief トレーナーカードSE定義
 * @author  Nozomu Saito
 * @date  05.11.15
 */
//============================================================================================
#ifndef __TRCARD_SND_DEF_H__
#define __TRCARD_SND_DEF_H__

#define SND_TRCARD_REV      (SEQ_SE_SYS_27)   ///< カードをひっくり返す音

#define SND_TRCARD_CALL     (SEQ_SE_SELECT1)  ///< 呼び出した時
#define SND_TRCARD_DECIDE   (SEQ_SE_DECIDE1)  ///< 終了時
#define SND_TRCARD_CANCEL   (SEQ_SE_CANCEL1)  ///< 終了時
#define SND_TRCARD_END      (SEQ_SE_CLOSE1)   ///< 終了時

#define SND_TRCARD_SLIDE    (SEQ_SE_SYS_06)   ///< 裏面レコードをスライドさせる音
#define SND_TRCARD_SIGN     (SEQ_SE_SYS_29)   ///< サイン面に書いている時の音
#define SND_TRCARD_KESHI    (SEQ_SE_SYS_93)   ///< サイン面に消しゴムをかけている時の音


#define SND_TRCARD_BOOKMARK (SEQ_SE_SYS_07)   ///< Yボタン登録
#define SND_TRCARD_ANIME    (SEQ_SE_SELECT1)  ///< アニメON/OFF
#define SND_TRCARD_PEN      (SEQ_SE_SELECT1)  ///< ペン先変更
#define SND_TRCARD_SCALEUP    (SEQ_SE_SYS_09) ///< サイン画面を拡大した時
#define SND_TRCARD_SCALEDOWN  (SEQ_SE_SYS_10) ///< サイン画面を縮小した時

#define SND_TRCARD_TRTYPE       (SEQ_SE_SELECT1)  ///< トレーナータイプ
#define SND_TRCARD_PERSONALITY  (SEQ_SE_SELECT1)  ///< 性格
#define SND_TRCARD_PMS          (SEQ_SE_SELECT1)  ///< 簡易会話


#endif  //__TRCARD_SND_DEF_H__
