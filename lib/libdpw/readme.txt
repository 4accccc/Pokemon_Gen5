■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
■                                                                          ■
■  readme.txt                                                              ■
■                                                                          ■
■  DP WiFi Library                                                         ■
■                                                                          ■
■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■


【アプリケーションへの組み込みについて】

　世界通信交換ライブラリを使用するためには、libdpw/common以下のヘッダーと
libdpw/dpw_tr 以下のヘッダー及びライブラリファイルをプロジェクトへ含めてください。
詳しくはlibdpw/demos/tr_sampleのMakefileをご参照ください。

　バトルタワーライブラリを使用するためには、libdpw/common以下のヘッダーと
libdpw/dpw_bt 以下のヘッダー及びライブラリファイルをプロジェクトへ含めてください。
詳しくはlibdpw/demos/bt_sampleのMakefileをご参照ください。

ライブラリの初期化を行うDpw_Bt_Init関数、Dpw_Tr_Init関数の引数で
デバッグ用またはリリース用サーバを指定してください。

【DP韓国語版以降の個人情報登録】

　DP韓国語版やプラチナ以降はDpw_Bt_Init関数もしくはDpw_Tr_Init関数を呼んで初期
化した後、必ず一度Dpw_Tr_SetProfileAsync関数もしくはDpw_Bt_SetProfileAsync関数
で個人情報を登録してください。どちらの関数も機能は同一です。


【プラチナ以降のメール機能】

　メールアドレス認証時(認証メール)とGTSで預けてあるポケモンが交換されたとき
(交換通知メール)にEメールをサーバ側で送信します。送信するメールのサブジェクト
と本文は管理ツールでイベント、言語ごとに設定できます。メールアドレスはGTSもし
くはバトルタワーに31日間接続がないと自動的に無効になり、メールが送信されなくな
ります。ただしその後接続すると自動的に有効になります。(現在のところ、預けてあ
るポケモンは31日で削除され、その後交換通知メールが発生することはありませんの
でこの様子を確認することはできません。)

　Eメールをサーバに登録するにはDpw_Tr_SetProfileAsync関数もしくは
Dpw_Bt_SetProfileAsync関数を使用します。(どちらでも構いません。) 実際の登録の
流れは下記のようになります。

1. Dpw_xx_SetProfileAsync関数に指定するDpw_Common_Profile構造体のmailAddrメン
　 バにメールアドレスを設定し、mailAddrAuthPassメンバに
　 DPW_MAIL_ADDR_AUTH_START_PASSWORDを設定し、mailAddrAuthVerificationメンバに
　 0～999の整数をランダムに設定します。
　 結果としてDpw_Common_ProfileResult構造体のmailAddrAuthResultメンバに
　 DPW_PROFILE_AUTHRESULT_SENDがセットされます。
2. サーバからxxx-yyyy形式のパスワードが入力されたメールアドレスへ送信されます。
　 ただし、xxxはクライアントが指定したmailAddrAuthVerification、yyyyはサーバで
　 生成されたパスワードです。
3. クライアントはパスワード入力画面を表示し、パスワードの内yyyyの部分のみを入
　 力させます。xxxは初めから表示させます。(わかりやすさのための配慮)
4. 再度Dpw_xx_SetProfileAsync関数でmailAddrAuthPassメンバに正しいパスワードを
　 指定し、mailAddrメンバに前回と同じメールアドレスを指定します。指定されたパ
　 スワードとメールアドレスが正しかった場合、Dpw_Common_ProfileResult構造体の
　 mailAddrAuthResultメンバにDPW_PROFILE_AUTHRESULT_SUCCESSが返ります。
5. GTSもしくはバトルタワーに接続するときは必ずDpw_xx_SetProfileAsync関数を一度
　 呼んでください。これによりメールアドレスの有効期限が31日に延長されます。
　 このときDpw_Common_ProfileResult構造体のmailAddrAuthResultメンバに
　 DPW_PROFILE_AUTHRESULT_SUCCESS以外の値が入った場合はパスワードなどのパラ
　 メータの指定が間違っています。この場合、メール機能は無効になります。
　 (自分の友達コードが変化したときに以前と同じメールアドレスとパスワードを送
　 信したときにこのようになる可能性があります。通常はあり得ません。)

　認証メールが送信される条件は下記の通りです。
1. 個人情報登録時(Dpw_xx_SetProfileAsync関数)に指定するDpw_Common_Profile構造
　 体のmailAddrメンバにメールアドレスを設定し、mailAddrAuthPassメンバに
　 DPW_MAIL_ADDR_AUTH_START_PASSWORDを設定し、mailAddrAuthVerificationメンバに
　 0～999の整数を設定している。
2. 認証メール用の本文を受信者がlanguageメンバで指定した言語であらかじめ登録し
　 ている。

　交換通知メールが送信される条件は下記の通りです。
1. 受信者がメールアドレスを登録している。
2. 受信者のメールアドレスが有効期限(31日)を過ぎていない。
3. 受信者がメール受信フラグ(mailRecvFlagメンバ)に
　 DPW_PROFILE_MAILRECVFLAG_EXCHANGEをセットしている。
4. 交換通知メール用の本文を受信者がlanguageメンバで登録した言語であらかじめ登録
　 している。

　Dpw_xx_SetProfileAsync関数に指定するDpw_Common_Profile構造体の
mailAddrAuthPassにDPW_MAIL_ADDR_AUTH_DEBUG_PASSWORD(9999)を指定するとメールア
ドレスの認証を強制的に成功させます。常に認証結果が
DPW_PROFILE_AUTHRESULT_SUCCESSになります。指定したメールアドレスが有効になり
ます。認証メールは送信されません。

　メールは本文登録時に指定した文字コードで送信されます。特に問題がない限りデフォ
ルトから変更しないでください。

　サーバにメールのフォーマットが設定されていない場合はメールが送信されません。
認証メール、交換通知メールのメールフォーマットを管理ツールであらかじめ設定して
おいてください。詳しい設定方法は$libdpw/man/dpwadmin_manual.txtを参照してくだ
さい。

　送信されたメールの本文の最後に送信者を識別するためのサポート用IDが自動的に付
加されます。


【メモリ管理】

　本ライブラリは、通信時にDWCライブラリのアロケータを用います。
...Asyncという名前の非同期関数を呼んだ後にメモリを確保し、非同期関数が終了すると
解放されます。


【動作確認バージョン】

　本ライブラリの動作確認が取れているライブラリのバージョンは以下の通りです。

・TwlSDK-5_3_patch6-20091125-jp
・TwlDWC-5_4-test1-20091108-jp
・CW4DSi_1_1_patch3plus


【管理用ツール】

　$libdpw/tools内にNGワードの登録やサーバの状態を変更できる管理用ツールが収録さ
れています。
本ツールはデバッグサーバにのみ接続できます。


【デバッグサーバへ接続させるダミーDNS】

　リリースサーバへの接続を強制的に変更し、デバッグサーバに接続させるダミーDNSを
設置しています。
下記URLで現在のダミーDNSのIPアドレスを確認できます。このアドレスをWi-Fi設定で
DNSアドレスとして登録してください。

　　http://kiyoshi.servebeer.com/cgi-bin/getipaddr.cgi

　このダミーDNSのアドレスは不定期に変更される可能性があります。


【 ディレクトリマップ 】

$libdpw       DPW の ROOT ディレクトリ
 │
 ├─common         共通ライブラリ
 │
 ├─demos          サンプル
 │  ├─bt_sample      -  バトルタワーライブラリのサンプル
 │  ├─dbs            -  デバッグスクリーンライブラリ
 │  └─tr_sample      -  世界通信交換ライブラリのサンプル
 │
 ├─dpw_bt         バトルタワーライブラリ
 │
 ├─dpw_tr         世界通信交換ライブラリ
 │
 ├─man            マニュアル
 │
 └─tool           管理用ツール


【 更新内容 】

 beta24 (Jan. 29, 2010)
  －世界通信交換－
    ・Dpw_Tr_Data 構造体に変数を3バイト追加しました。
    
 beta23 (Dec. 28, 2009)
  －バトルタワー、世界通信交換－
    ・Dpw_Bt_Init関数、Dpw_Tr_Init関数に引数を追加し、
      デバッグ用/リリース用サーバを指定するように変更しました。
    ・Dpw_Bt_UploadPlayerAsync関数、Dpw_Tr_UploadAsync関数、Dpw_Tr_TradeAsync関数に引数を追加し、
      トークンを受け付けるように変更しました。

 beta22 (Nov. 9, 2009)
  －共通－
    ・Twl環境でプライベートなヘッダを参照するようになっており
      コンパイルできませんでしたのでこの問題を修正しました。

 beta21 (Nov. 5, 2009)
  －共通－
    ・TwlSDKに対応しました。

 beta20 (Dec. 6, 2007)
  －世界通信交換－
    ・Dpw_Tr_DownloadMatchDataExAsync関数及びDpw_Tr_PokemonSearchDataEx構造体を
      追加し、国コードで検索できるようにしました。

 beta19 (Oct. 12, 2007)
  －世界通信交換－
    ・メール機能を追加しました。
  －管理用ツール－
    ・メールフォーマット設定機能を追加しました。

 beta18 (Oct. 9, 2007)
  －共通－
    ・プロジェクト組み込み用ソースコードインクルードが誤っていたのを修正しました。

 beta17 (Oct. 5, 2007)
  －世界通信交換、バトルタワー－
    ・Dpw_Tr_SetProfileAsync関数及びDpw_Bt_SetProfileAsync関数を追加しました。
  －管理用ツール－
    ・韓国版に対応しました。

 beta16 (July. 27, 2006)
  －世界通信交換－
    ・Dpw_Tr_ReturnAsync() を追加しました。ポケモンを引き取る際にはDpw_Tr_ReturnAsync()
      を使用し、Dpw_Tr_DeleteAsync()は交換済みのデータを削除する場合にのみ使用してください。
  －管理用ツール－
    ・ポケモンコードとユニコードの変換が正しくなかったのを修正しました。
    ・NGネームフラグの表示に対応しました。
  －バトルタワールームデータ閲覧ツール－
    ・新規追加しました。

 beta15 (July. 7, 2006)
  －バトルタワー－
    ・Dpw_Bt_Player及びDpw_Bt_Leaderの性別を入れていた部分をビットフラグとし、
      性別フラグとNGネームフラグを持つようにしました。

 beta14 (July. 5, 2006)
  －世界通信交換－
    ・サーバーアップデート中にアクセスした場合、DPW_TR_ERROR_SERVER_TIMEOUTエラーになる
      ようにしました。
  －バトルタワー－
    ・ルーム数の最大値DPW_BT_ROOM_NUM_MAXを100に変更しました。
    ・サーバーアップデート中にアクセスした場合、DPW_BT_ERROR_SERVER_TIMEOUTエラーになる
      ようにしました。
  －管理用ツール－
    ・接続するサーバーを選択できるようにしました。
    ・バトルタワーのルーム数の最大値が100になった変更に対応しました。

 beta13 (June. 30, 2006)
  －世界通信交換－
    ・Dpw_Tr_TradeAsync() で正しくサーバーからのレスポンスを受け取れていなかったバグを
      修正しました。

 beta12 (June. 15, 2006)
  －世界通信交換－
    ・DPW_TR_ERROR_ILLEGAL_DATA, DPW_TR_ERROR_CHEAT_DATA, DPW_TR_ERROR_NG_POKEMON_NAME,
      DPW_TR_ERROR_NG_PARENT_NAME, DPW_TR_ERROR_NG_MAIL_NAME, DPW_TR_ERROR_NG_OWNER_NAME
      が、 Dpw_Tr_Data::name を特定の値にしたときに返るようにしました。

 beta11 (June. 9, 2006)
  －世界通信交換－
    ・DPW_TR_ERROR_SERVER_TIMEOUT の際のエラー表示を細かくしました。
  －バトルタワー－
    ・Dpw_Bt_UploadPlayerAsync() の引数 win が0のときにアサートになっていたバグ
      を修正しました。
    ・DPW_BT_ERROR_SERVER_TIMEOUT の際のエラー表示を細かくしました。

 beta10 (May. 23, 2006)
  －世界通信交換－
    ・Dpw_Tr_Data::friend_keyを削除しました。
  －バトルタワー－
    ・サーバーの初期化でルームデータが初期化されなくなりました。
    ・Dpw_Bt_Player::friend_keyを削除しました。
    ・Dpw_Bt_Leader::friend_keyを削除しました。

 beta9 (May. 22, 2006)
  －世界通信交換－
    ・Dpw_Tr_Init() に引数 friend_key を追加しました。
    ・Dpw_Tr_Data::friend_keyを追加しました。
  －バトルタワー－
    ・Dpw_Bt_Init() に引数 friend_key を追加しました。
    ・Dpw_Bt_Player::friend_keyを追加しました。
    ・Dpw_Bt_Leader::friend_keyを追加しました。
    ・アップデート時に、リーダー候補が一人もいない場合もルームの更新が行われていたバグを
      修正しました。

 beta8 (May. 19, 2006)
  －バトルタワー－
    ・Dpw_Bt_PokemonData のサイズを変更しました。

 beta7 (May. 15, 2006)
  －世界通信交換－
    ・Dpw_Tr_Data::genderを追加しました。
  －バトルタワー－
    ・Dpw_Bt_Leader::genderを追加しました。
    ・Dpw_Bt_Player::genderを追加しました。
    ・Dpw_Bt_Player::playerNameにNGネームが含まれる場合、アップデート時に置き換えられる
      ようになりました。
  －管理用ツール－
    ・置換ネームリストのアップロードを追加しました。
    ・利用者数の表示機能を追加しました。
    ・ルームデータの取得機能を追加しました。
    ・ルームデータの変更機能を追加しました。
    ・部屋のデータを表示する機能を追加しました。
    
 beta6 (May. 8, 2006)
  －世界通信交換－
    ・::DPW_TR_GENDER_NONE のコメントを修正しました。
    ・Dpw_Tr_PokemonSearchData を追加しました。
    ・Dpw_Tr_Data::wantSimple の型を Dpw_Tr_PokemonSearchData へ変更しました。
    ・Dpw_Tr_DownloadMatchDataAsync() の引数を変更しました。
    ・Dpw_Tr_Db_InitServer(), Dpw_Tr_Db_UpdateServer(), Dpw_Tr_Db_ShuffleServerData()
      でエラー処理が誤っていたバグを修正しました。
    ・Dpw_Tr_Data::postDate, Dpw_Tr_Data::tradeDate へ入る月が、1マイナスされた値に
      なっていたバグを修正しました。
  －バトルタワー－
    ・::DPW_BT_RANK_NUMを追加しました。
    ・Dpw_Bt_GetRoomNumAsync(), Dpw_Bt_DownloadRoomAsync(), Dpw_Bt_UploadPlayerAsync()
      で、引数rankの範囲チェックが誤っていたバグを修正しました。
    ・Dpw_Bt_Db_InitServer(), Dpw_Bt_Db_UpdateServer() でエラー処理が誤っていたバグを
      修正しました。

 beta5 (April. 27, 2006)
  －共通－
  　・GameSpyのサーバーへ接続するように変更しました。
    ・ハンドルしていなかった内部エラーがあった問題を修正しました。
    ・管理用ツール（libdpw/tool/DpwAdmin.exe）を追加しました。
  －世界通信交換－
    ・DPW_TR_ERROR_NG_TRAINER_NAMEを廃止し、DPW_TR_ERROR_NG_PARENT_NAMEと
      DPW_TR_ERROR_NG_OWNER_NAMEを追加しました。
    ・主人公名（Dpw_Tr_Data::name）にNGワードが含まれていた場合、
      DPW_TR_ERROR_NG_OWNER_NAMEが返るようになりました。
  －バトルタワー－
    ・DPW_BT_ROOM_NUM_MAXを追加しました。（暫定的に20としています。）

 beta4 (April. 17, 2006)
  －共通－
    ・DPWライブラリを使用する前に、WiFiコネクションへのログインを行い、すぐログアウトする
      処理をサンプルへ追加しました。
  －バトルタワー－
    ・Dpw_Bt_Leader::leaderMessage のサイズを変更しました。
    ・Dpw_Bt_Player::leaderMessage, Dpw_Bt_Player::message のサイズを変更しました。
    ・DWC_GetDateTime()の使用例をサンプルへ追加しました。

 beta3 (April. 10, 2006)
  －世界通信交換－
    ・Dpw_Tr_PokemonData のサイズを変更しました。
    ・Dpw_Tr_Data::comment を削除しました。
    ・Dpw_Tr_Data::trainerID を追加しました。
    ・Dpw_Tr_Data::langCode を追加しました。    
    ・特定のタイミングで Dpw_Tr_CancelAsync() を呼ぶと止まっていた不具合を修正しました。
  －バトルタワー－
    ・Dpw_Bt_PokemonData のサイズを変更しました。
    ・Dpw_Bt_Leader::versionCode, Dpw_Bt_Leader::langCode, Dpw_Bt_Leader::countryCode, 
      Dpw_Bt_Leader::localCode を追加しました。
    ・Dpw_Bt_Player::versionCode, Dpw_Bt_Player::langCode, Dpw_Bt_Player::countryCode,
      Dpw_Bt_Player::localCode を追加しました。
    ・Dpw_Bt_Room::leader のサイズを変更しました。
    ・特定のタイミングで Dpw_Bt_CancelAsync() を呼ぶと止まっていた不具合を修正しました。

 beta2 (March. 27, 2006)
  －共通－
    ・HTTPライブラリのレイヤーでエラーが起こったときに、エラーをクリアできてい
      なかった不具合を修正しました。
    ・関数のコメントに、返ってくるエラーの種類を追加しました。
    ・Dpw_Bt_PokemonDataの大きさを172バイトに変更しました。
  －世界通信交換－
    ・Dpw_Tr_DownloadMatchDataAsync() で検索されるデータをランダムに変更しまし
      た。異なるPIDでは、異なる検索結果となります。
    ・Dpw_Tr_Data::trainerType が交換されるようにしました。
    ・Dpw_Tr_Data::mail を削除しました。
    ・Dpw_Tr_PokemonDataの大きさを172バイトに変更しました。
    ・DpwTrError で、NGワードに関するエラーを細分化しました。
    ・世界通信交換の検索順の変更する関数 Dpw_Tr_Db_ShuffleServerData() を追加し
    　ました。

 beta1 (March. 17, 2006)

    ・初版をリリースしました。