#ifndef AUTOMATCH_H_
#define AUTOMATCH_H_

#include "utility.h"

extern void NitroMain ();
extern void drawConsole( void );

extern KeyControl		g_KeyCtrl;

// バトルデータビデオをアップロードするかどうか
// 現在は署名チェックが行われるため、サンプルからではアップロードできません
// アップロードメニューを有効にする場合、以下の定義を有効にします
//#define BATTLEVIDEO_UPLOAD_ENABLE


// メニュータイプ
enum {
    TYPE_NORMAL,        // 通常処理
    TYPE_SELECT,        // 選択
    TYPE_INPUT          // 入力
};

// 入力関連
enum {
    INPUT_CONFIRM,    // 入力確定
    INPUT_NOW,        // 入力中
    INPUT_CANCEL      // キャンセル
};
// 入力制御構造体
// メニューの場合はメッセージ部分のみ参照される
// 小数は含まない。数字のみ。
typedef struct tagInputElement
{
    const char* head; // 入力メッセージ
    char buf[20];     // 入力バッファ
    u8   inputKeta;   // 何文字目を入力しているか？
    u64  inputValue;  // 入力された値
}InputElement;

// 入力シーケンス
#define MAX_INPUT_ELEMENTS      12
typedef struct tagInputSequence
{
    const int type;                                     // 入力タイプ
    const int numElements;                              // 要素の数(メニュー用 INPUT では無視)
    // メニューの場合はメニューアイテムが個数分
    // 入力の場合は入力要素が一つ
    // 入力要素は複数ではない
    InputElement elements[MAX_INPUT_ELEMENTS];          // 要素(MAX_INPUT_ELEMENTS 以内)
    int currentElement;                                 // メニューで現在選択されている番号(入力では使用しない)
    u64 result;                                         // 結果(メニューなら選択番号, 入力なら値)
}InputSequence;

// ゲームシーケンス制御リスト構造体
#define MAX_SCENE_ELEMENTS      5
typedef struct tagGameSequence
{
    const char* modeName;                               // モード名
    const int   numElements;                            // 要素数(MAX_SCENE_ELEMENTS 以内)
    int (*fpResult)(struct tagGameSequence*);           // 結果を処理する関数
    InputSequence   inputList[MAX_SCENE_ELEMENTS];      // 入力リスト
    u64 result;                                         // 操作結果
    int currentElement;                                 // 現在実行中の処理番号
} GameSequence;

// シーン定義
enum
{
    state_select_connect,
    state_init,
    state_connect,
    state_select_pid,
    state_input_pid,
    state_gds_init,
    state_menu,
    state_debug_message,
    state_musical_update,
    state_musical_download,
#ifdef BATTLEVIDEO_UPLOAD_ENABLE
    state_battle_upload,
#endif
    state_battle_search,
    state_battle_rank_search,
    state_battle_download,
    state_event_battle_download,
    state_event_battle_bookmark,
    state_error,
    state_query_disconnect,
    state_disconnect,
    state_end
};

#endif // AUTOMATCH_H_
