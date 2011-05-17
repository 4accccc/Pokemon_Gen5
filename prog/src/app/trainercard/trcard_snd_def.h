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

#if 0		// 旧ラベル(しばらく残し)
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
#endif

//-----------
// 以下、新ラベル
//-----------
#define SND_TRCARD_REV			(SEQ_SE_SW_CARD_06)		///< カードをひっくり返す音
//
#define SND_TRCARD_CALL			(SEQ_SE_SW_CARD_02)		///< 呼び出した時
#define SND_TRCARD_DECIDE		(SEQ_SE_SW_CARD_01)		///< 終了時
#define SND_TRCARD_CANCEL		(SEQ_SE_SW_CARD_04)		///< 終了時
#define SND_TRCARD_END			(SEQ_SE_SW_CARD_05)		///< 終了時
//
#define SND_TRCARD_SLIDE		(SEQ_SE_SW_CARD_07)		///< 裏面レコードをスライドさせる音
#define SND_TRCARD_SIGN			(SEQ_SE_SW_CARD_08)		///< サイン面に書いている時の音
#define SND_TRCARD_KESHI		(SEQ_SE_SW_CARD_09)		///< サイン面に消しゴムをかけている時の音
//
#define SND_TRCARD_BOOKMARK		(SEQ_SE_SW_CARD_03)		///< Yボタン登録
#define SND_TRCARD_ANIME		(SEQ_SE_SW_CARD_02)		///< アニメON/OFF
#define SND_TRCARD_PEN			(SEQ_SE_SW_CARD_02)		///< ペン先変更
#define SND_TRCARD_SCALEUP		(SEQ_SE_SW_CARD_10)		///< サイン画面を拡大した時
#define SND_TRCARD_SCALEDOWN	(SEQ_SE_SW_CARD_11)		///< サイン画面を縮小した時
//
#define SND_TRCARD_TRTYPE		(SEQ_SE_SW_CARD_02)		///< トレーナータイプ
#define SND_TRCARD_PERSONALITY	(SEQ_SE_SW_CARD_02)		///< 性格
#define SND_TRCARD_PMS			(SEQ_SE_SW_CARD_02)		///< 簡易会話
//
#define SND_TRCARD_MEDAL_TOUCH	(SEQ_SE_SW_CARD_01)		///< メダルタッチ
//
#define SND_TRBADGE_00			(SEQ_SE_SW_CARD_14)		///< 音階(ド)
#define SND_TRBADGE_01			(SEQ_SE_SW_CARD_15)		///< 音階(レ)
#define SND_TRBADGE_02			(SEQ_SE_SW_CARD_16)		///< 音階(ミ)
#define SND_TRBADGE_03			(SEQ_SE_SW_CARD_17)		///< 音階(ファ)
#define SND_TRBADGE_04			(SEQ_SE_SW_CARD_18)		///< 音階(ソ)
#define SND_TRBADGE_05			(SEQ_SE_SW_CARD_19)		///< 音階(ラ)
#define SND_TRBADGE_06			(SEQ_SE_SW_CARD_20)		///< 音階(シ)
#define SND_TRBADGE_07			(SEQ_SE_SW_CARD_21)		///< 音階(ド2)
//
#define SND_TRBADGE_SLIDE_IN	(SEQ_SE_SW_CARD_12)		///< スライドイン
#define SND_TRBADGE_SLIDE_OUT	(SEQ_SE_SW_CARD_13)		///< スライドアウト
//
#define SND_TRBADGE_WIN_TOUCH	(SEQ_SE_SW_CARD_02)		///< ウインドウタッチ音

#endif  //__TRCARD_SND_DEF_H__
