/* 自動生成フラグ定義ファイル 開始*/#pragma once

#define WORKS_TOTAL_MIN  16384 /* 0x4000 */
/* flags "local_work.xls" 16384 ～ 16415 */

#define LOCALWORKAREA_START              16384 /* 0x4000  */

#define LOCALWORK0                       16384 /* 0x4000 マップ内限定ワーク */
#define LOCALWORK1                       16385 /* 0x4001 マップ内限定ワーク */
#define LOCALWORK2                       16386 /* 0x4002 マップ内限定ワーク */
#define LOCALWORK3                       16387 /* 0x4003 マップ内限定ワーク */
#define LOCALWORK4                       16388 /* 0x4004 マップ内限定ワーク */
#define LOCALWORK5                       16389 /* 0x4005 マップ内限定ワーク */
#define LOCALWORK6                       16390 /* 0x4006 マップ内限定ワーク */
#define LOCALWORK7                       16391 /* 0x4007 マップ内限定ワーク */
#define LOCALWORK8                       16392 /* 0x4008 マップ内限定ワーク */
#define LOCALWORK9                       16393 /* 0x4009 マップ内限定ワーク */
#define LOCALWORK10                      16394 /* 0x400a マップ内限定ワーク */
#define LOCALWORK11                      16395 /* 0x400b マップ内限定ワーク */
#define LOCALWORK12                      16396 /* 0x400c マップ内限定ワーク */
#define LOCALWORK13                      16397 /* 0x400d マップ内限定ワーク */
#define LOCALWORK14                      16398 /* 0x400e マップ内限定ワーク */
#define LOCALWORK15                      16399 /* 0x400f マップ内限定ワーク */
		/* not used 16400(0x4010) */
		/* not used 16401(0x4011) */
		/* not used 16402(0x4012) */
		/* not used 16403(0x4013) */

#define LOCALWORKAREA_END                16415 /* 0x401f  */


/* flags "objchr_work.xls" 16416 ～ 16431 */

#define OBJCHRWORKAREA_START             16416 /* 0x4020  */

#define OBJCHRWORK0                      16416 /* 0x4020 可変OBJ指定用ワーク */
#define OBJCHRWORK1                      16417 /* 0x4021 可変OBJ指定用ワーク */
#define OBJCHRWORK2                      16418 /* 0x4022 可変OBJ指定用ワーク */
#define OBJCHRWORK3                      16419 /* 0x4023 可変OBJ指定用ワーク */
#define OBJCHRWORK4                      16420 /* 0x4024 可変OBJ指定用ワーク */
#define OBJCHRWORK5                      16421 /* 0x4025 可変OBJ指定用ワーク */
#define OBJCHRWORK6                      16422 /* 0x4026 可変OBJ指定用ワーク */
#define OBJCHRWORK7                      16423 /* 0x4027 可変OBJ指定用ワーク */
#define OBJCHRWORK8                      16424 /* 0x4028 可変OBJ指定用ワーク */
#define OBJCHRWORK9                      16425 /* 0x4029 可変OBJ指定用ワーク */
#define OBJCHRWORK10                     16426 /* 0x402a 可変OBJ指定用ワーク */
#define OBJCHRWORK11                     16427 /* 0x402b 可変OBJ指定用ワーク */
#define OBJCHRWORK12                     16428 /* 0x402c 可変OBJ指定用ワーク */
#define OBJCHRWORK13                     16429 /* 0x402d 可変OBJ指定用ワーク */
#define OBJCHRWORK14                     16430 /* 0x402e 可変OBJ指定用ワーク */
#define OBJCHRWORK15                     16431 /* 0x402f 可変OBJ指定用ワーク */

#define OBJCHRWORKAREA_END               16431 /* 0x402f  */


/* flags "sys_work.xls" 16432 ～ 16495 */

#define WK_SYS_AREA_START                16432 /* 0x4030  */

#define WK_SYS_FIRST_POKETYPE            16432 /* 0x4030 最初の３体選択で選んだポケモンのタイプ。識別用定義名はuser_define.hで確認 */
#define WK_SYS_MAPREPLACE01              16433 /* 0x4031 マップ置き換え処理用ワーク定義：博物館 */
#define WK_SYS_MAPREPLACE02              16434 /* 0x4032 マップ置き換え処理用ワーク定義：チャンピオン部屋 */
#define WK_SYS_MAPREPLACE03              16435 /* 0x4033 マップ置き換え処理用ワーク定義：WF/BC制御用 */
#define WK_SYS_MAPREPLACE04              16436 /* 0x4034 マップ置き換え処理用ワーク定義：Nの城の謁見の間 */
#define WK_SYS_MAPREPLACE05              16437 /* 0x4035 マップ置き換え処理用ワーク定義：りゅうらせんの塔 屋外 */
#define WK_SYS_MAPREPLACE06              16438 /* 0x4036 マップ置き換え処理用ワーク定義：古代の城 */
#define WK_SYS_MAPREPLACE07              16439 /* 0x4037 マップ置き換え処理用ワーク定義：自分の部屋 */
#define WK_SYS_MAPREPLACE08              16440 /* 0x4038 マップ置き換え処理用ワーク定義 */
#define WK_SYS_MAPREPLACE09              16441 /* 0x4039 マップ置き換え処理用ワーク定義 */
#define WK_SYS_MAPREPLACE10              16442 /* 0x403a マップ置き換え処理用ワーク定義 */
		/* not used 16443(0x403b) */
		/* not used 16444(0x403c) */
		/* not used 16445(0x403d) */
		/* not used 16446(0x403e) */
		/* not used 16447(0x403f) */
		/* not used 16448(0x4040) */
#define WK_SYS_SCENE_COMM                16449 /* 0x4041 通信部屋から出てくる時の処理用シーン制御ワーク */
		/* not used 16450(0x4042) */
#define WK_SYS_PAIR_ID                   16451 /* 0x4043 連れ歩きペアID */
#define WK_SYS_HIDE_SHIP_PLACE           16452 /* 0x4044 アジト船の位置をタウンマップに通知するためのワーク SCR_HIDE_SHIP_PLACE_C05 (0) ///&lt; ホドモエ SCR_HIDE_SHIP_PLACE_W21 (1) ///&lt; ２１番水道 SCR_HIDE_SHIP_PLACE_D10 (2) ///&lt; ジャイアントホール SCR_HIDE_SHIP_PLACE_W17 (3) ///&lt; P2ラボにいるとき */
#define WK_SYS_LINK_EVENT_01             16453 /* 0x4045 リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_02             16454 /* 0x4046 リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_03             16455 /* 0x4047 リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_04             16456 /* 0x4048 リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_05             16457 /* 0x4049 リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_06             16458 /* 0x404a リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_07             16459 /* 0x404b リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_LINK_EVENT_08             16460 /* 0x404c リンクイベント状態 0 なし 1 イベント起動 2 イベント終了（なし） */
#define WK_SYS_FES_TUTORIAL              16461 /* 0x404d 0 初回訪問イベント有効 1 初回訪問イベント終了 2 ミッション失敗イベント有効 3 ミッション成功イベント有効 4 ハイルツリー機能制限状態（デルパワーを受け取るしかできない） 5 デルパワー使用説明イベント有効 6 フェスチュートリアル終了 */
		/* not used 16462(0x404e) */
		/* not used 16463(0x404f) */
		/* not used 16464(0x4050) */
		/* not used 16465(0x4051) */
		/* not used 16466(0x4052) */
		/* not used 16467(0x4053) */
		/* not used 16468(0x4054) */
		/* not used 16469(0x4055) */
		/* not used 16470(0x4056) */
		/* not used 16471(0x4057) */

#define WK_SYS_AREA_END                  16495 /* 0x406f  */


/* flags "scene_work.xls" 16496 ～ 16735 */

#define WK_SCENE_AREA_START              16496 /* 0x4070  */

#define WK_SCENE_D10R0101_SWPLAZMA_01    16496 /* 0x4070 ジャイアントホールエントランス：POS制御用ワーク */
#define WK_SCENE_D10R0301_SWPLAZMA_01    16497 /* 0x4071 ジャイアントホール広場：POS制御用ワーク */
#define WK_SCENE_D10R0401_SWSAGE1_01     16498 /* 0x4072 ジャイアントホール最深部：POS制御用ワーク */
#define WK_SCENE_H05_POKE488             16499 /* 0x4073 ワンダーブリッジ：クレセリアPOS制御用ワーク */
#define WK_SCENE_D24_POKE485             16500 /* 0x4074 リバースマウンテン：ヒードランPOS制御用ワーク */
#define WK_SCENE_D05_GIRL2               16501 /* 0x4075 ストレンジャーハウス：幽霊少女POSイベント制御 */
		/* not used 16502(0x4076) */
#define WK_SCENE_D03R0101_AFTER          16503 /* 0x4077 リゾートデザート：クリア後イベント用ワーク */
#define WK_SCENE_T02PC0101               16504 /* 0x4078 カラクサタウンＰＣ：イベント用ワーク */
#define WK_SCENE_R02                     16505 /* 0x4079 ２番道路：イベント用ワーク */
#define WK_SCENE_T01_SUPPORT             16506 /* 0x407a カノコタウン：サポートが自宅へ向かう演出用 */
#define WK_SCENE_T01                     16507 /* 0x407b カノコタウン（フィールド）：イベント用ワーク */
#define WK_SCENE_C02GYM0101              16508 /* 0x407c 博物館：イベント用ワーク */
#define WK_SCENE_T01R0101                16509 /* 0x407d 自宅１階：イベント用ワーク */
#define WK_SCENE_C04                     16510 /* 0x407e ライモンシティフィールド：イベント用ワーク */
#define WK_SCENE_VICTORY                 16511 /* 0x407f ビクティイベント用ワーク */
#define WK_SCENE_R05                     16512 /* 0x4080 ５番道路：イベント用ワーク */
#define WK_SCENE_C06                     16513 /* 0x4081 C06フキヨセシティ：POSイベント用ワーク */
#define WK_SCENE_D06R0101                16514 /* 0x4082 D06R0101電気洞窟の内部：イベント用ワーク */
#define WK_SCENE_H03_PLASMA              16515 /* 0x4083 H03：暴走族イベント用ワーク */
#define WK_SCENE_C08R0301                16516 /* 0x4084 C08ソウリュウシティ：ミュージカル大好きおじさんイベント用ワーク */
#define WK_SCENE_R15R0301                16517 /* 0x4085 パルパーク：イベント用ワーク */
#define WK_SCENE_C11GATE                 16518 /* 0x4086 国連（内部）：ゲート係員用のイベントワーク */
#define WK_SCENE_C04R0201                16519 /* 0x4087 ミュージカルホール：イベント用ワーク */
#define WK_SCENE_D07_RIVAL_01            16520 /* 0x4088 ネジ山入り口：イベント用ワーク */
#define WK_SCENE_R08R0101_LEG            16521 /* 0x4089 セッカ湿原：伝説ポケモンイベント用ワーク（ダルタニス） */
#define WK_SCENE_C02R0101                16522 /* 0x408a シッポウシティ：インテリアショップイベント */
#define WK_SCENE_PALACE01                16523 /* 0x408b パレス：イベント用ワーク */
#define WK_SCENE_C3CENTER_DANCE          16524 /* 0x408c C03中央部：ヒウンダンサーズイベント用ワーク */
#define WK_SCENE_R15R0301_RECV           16525 /* 0x408d パルパーク：電源OFFからの復帰イベント用ワーク */
#define WK_SCENE_C03R0101_QUESTION       16526 /* 0x408e C03R0101：すれ違い調査隊 強制アンケートイベント用ワーク */
#define WK_SCENE_D13R0101                16527 /* 0x408f P2研究所：インセクタイベント用ワーク */
#define WK_SCENE_T04R0101_BTL_FAMILY     16528 /* 0x4090 サザナミタウン：巨大別荘用ワーク */
#define WK_SCENE_C04_BTLREC              16529 /* 0x4091 ライモンシティ：バトルレコーダー入手イベント用 ※必須イベントでは無い */
#define WK_SCENE_C03GYM0101              16530 /* 0x4092 C03ヒウンジム：イベント用ワーク */
#define WK_SCENE_H03_BADRIDER            16531 /* 0x4093 シリンダーブリッジ：暴走族イベント */
#define WK_SCENE_D20R0501                16532 /* 0x4094 海底遺跡：１F・壁移動用 */
#define WK_SCENE_D20R0601                16533 /* 0x4095 海底遺跡：１F・壁移動用 */
#define WK_SCENE_D20R0701                16534 /* 0x4096 海底遺跡：１F・壁移動用 */
#define WK_SCENE_C05_BTLRIDER            16535 /* 0x4097 ホドモエ：トリプル／ローテバトルイベント用ワーク */
#define WK_SCENE_T04R0301                16536 /* 0x4098 サザナミタウン：シロナイベント用ワーク */
#define WK_SCENE_R07                     16537 /* 0x4099 7番道路：ライブキャスターイベント用ワーク */
#define WK_SCENE_D06_PAIR                16538 /* 0x409a 電気穴の洞窟　連れ歩きイベント制御 */
#define WK_SCENE_C04GYM0101_SHOW         16539 /* 0x409b スワンでんきジム：ジム進行状況管理ワーク */
#define WK_SCENE_C04GYM0101_SHOWEND      16540 /* 0x409c スワンでんきジム：ジム退場演出用ワーク */
#define WK_SCENE_C03GYM0101_GATE_3B      16541 /* 0x409d スワン虫ジム：3Bゲート POS制御ワーク */
#define WK_SCENE_MEDAL_EVENT             16542 /* 0x409e メダルおやじイベント制御ワーク */
#define WK_SCENE_C06GYM0101_GUIDE        16543 /* 0x409f スワン飛行ジム：ガイド POS制御ワーク */
#define WK_SCENE_C12R0101                16544 /* 0x40a0 ヒオウギシティ自宅：イベント用ワーク */
#define WK_SCENE_C12_WHITE_MEET          16545 /* 0x40a1 ヒオウギシティ：ベルと出会うイベント用ワーク */
#define WK_SCENE_C12PC0101_PCGUIDE       16546 /* 0x40a2 ヒオウギシティ：ポケモンセンターチュートリアル用ワーク */
#define WK_SCENE_R19                     16547 /* 0x40a3 19ばんどうろ：ポケモン捕獲イベント用ワーク */
#define WK_SCENE_C12_RIVAL_GUIDE_01      16548 /* 0x40a4 ヒオウギシティ：POSイベント */
#define WK_SCENE_T06                     16549 /* 0x40a5 サンギタウン：POＳ */
#define WK_SCENE_T06R0101                16550 /* 0x40a6 サンギタウン：SCENE_CHANGE制御ワーク */
#define WK_SCENE_D22R0101                16551 /* 0x40a7 牧場：POS制御ワーク */
#define WK_SCENE_C12GYM_OPEN             16552 /* 0x40a8 C12：ジム関連イベント用ワーク */
#define WK_SCENE_C12GYM0201              16553 /* 0x40a9 C12：ジム進行管理用ワーク */
#define WK_SCENE_C13GYM0101_GUIDE        16554 /* 0x40aa スワンどくジム：ガイド POS制御ワーク */
#define WK_SCENE_R20                     16555 /* 0x40ab 20番道路：POS制御ワーク */
#define WK_SCENE_C13                     16556 /* 0x40ac C13：ジムリーダーとの出会い用ワーク */
#define WK_SCENE_C13GYM0102              16557 /* 0x40ad C13GYM0102：ジムリーダーとの出会い用ワーク */
#define WK_SCENE_C03P15                  16558 /* 0x40ae C13R15：船着場到着イベント管理ワーク */
#define WK_SCENE_H01R0201                16559 /* 0x40af H01R0201：スカイアローブリッジストッパー（クリアまで） */
#define WK_SCENE_C03                     16560 /* 0x40b0 C03：ヒウンラリー開始ワーク */
#define WK_SCENE_C03GYM_STOPPER          16561 /* 0x40b1 C03ジムまわり：下水道クリアまでストッパー管理ワーク */
#define WK_SCENE_C03P17                  16562 /* 0x40b2 C03P17：下水道まわりイベント管理ワーク */
#define WK_SCENE_D11R0101                16563 /* 0x40b3 D11R0101：下水道連れ歩き管理ワーク */
#define WK_SCENE_C03P01                  16564 /* 0x40b4 C03P01：アクロマとの出会い用ワーク */
#define WK_SCENE_R04_POKE                16565 /* 0x40b5 R04：イワパレスイベント管理ワーク */
#define WK_SCENE_R04_SWBOSS              16566 /* 0x40b6 R04：アクロマイベント管理ワーク */
#define WK_SCENE_C05GYM0101_SHORTCUT     16567 /* 0x40b7 スワン地面ジム：ジム退出ショートカット用ワーク */
#define WK_SCENE_C04_RALLY               16568 /* 0x40b8 ライモンシティ：ラリー進捗管理ワーク */
#define WK_SCENE_C05GYM0101_TRBTL_01     16569 /* 0x40b9 じめんジム：１人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_02     16570 /* 0x40ba じめんジム：２人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_03     16571 /* 0x40bb じめんジム：３人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_04     16572 /* 0x40bc じめんジム：４人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_05     16573 /* 0x40bd じめんジム：５人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_06     16574 /* 0x40be じめんジム：６人目の視線トレーナー */
#define WK_SCENE_C05GYM0101_TRBTL_07     16575 /* 0x40bf じめんジム：７人目の視線トレーナー */
#define WK_SCENE_C04_SWRIVAL             16576 /* 0x40c0 ライモンシティ：ジムクリア後プラズマ団イベント制御ワーク */
#define WK_SCENE_C06GYM                  16577 /* 0x40c1 フキヨセシティ：イベント進行管理ワーク */
#define WK_SCENE_C06HAKASE               16578 /* 0x40c2 フキヨセシティ：アララギ娘イベント進行管理ワーク */
#define WK_SCENE_C05                     16579 /* 0x40c3 ホドモエシティ：イベント制御ワーク */
#define WK_SCENE_C05_STOPPER_01          16580 /* 0x40c4 ホドモエシティ：抜け道ストッパーPOS */
#define WK_SCENE_C05R0701_STOPPER_01     16581 /* 0x40c5 ホドモエシティ：WBT方面ストッパーPOS */
#define WK_SCENE_D05                     16582 /* 0x40c6 WBT前：イベント制御ワーク */
#define WK_SCENE_D05_STOPPER_01          16583 /* 0x40c7 WBT前：イベント制御ワーク抜け道ストッパーPOS */
#define WK_SCENE_C03P09                  16584 /* 0x40c8 ヒウンアイス：行列POS制御ワーク */
#define WK_SCENE_R06_SWLEADER1           16585 /* 0x40c9 6番道路：季節研究所誘導POS制御ワーク */
#define WK_SCENE_R06_LEGEND_MEET         16586 /* 0x40ca 6番道路：コバルオンとの出会いPOS制御ワーク */
#define WK_SCENE_T07                     16587 /* 0x40cb ヤマジタウン：アララギ娘イベント制御ワーク */
#define WK_SCENE_T03                     16588 /* 0x40cc カゴメタウン：アララギ娘イベント制御ワーク */
#define WK_SCENE_T04                     16589 /* 0x40cd サザナミタウン：ライバル戦POSイベント制御ワーク */
#define WK_SCENE_T03_SWRIVAL             16590 /* 0x40ce カゴメタウン：カゴメタウンのライバルイベント制御 */
#define WK_SCENE_C08GYM_TURN_BTN_01      16591 /* 0x40cf ソウリュウジム：1階用イベント用（POSイベントの起動制御に使用） */
#define WK_SCENE_C08GYM_TURN_BTN_02      16592 /* 0x40d0 ソウリュウジム：2階用イベント用（POSイベントの起動制御に使用） */
#define WK_SCENE_C08GYM_TURN_BTN_03      16593 /* 0x40d1 ソウリュウジム：3階用イベント用（POSイベントの起動制御に使用） */
#define WK_SCENE_D06R0101_SWBELL_01      16594 /* 0x40d2 電気石の洞穴：ベルイベントPOS制御用ワーク */
#define WK_SCENE_D06R0101_BOSS_01        16595 /* 0x40d3 電気石の洞穴：NイベントPOS制御用ワーク */
#define WK_SCENE_H04_STOP_01             16596 /* 0x40d4 ビレッジブリッジ：ストッパーPOS制御用ワーク */
#define WK_SCENE_C08_SWCHAMPION          16597 /* 0x40d5 ソウリュウシティ：アイリスイベントPOS制御用ワーク */
#define WK_SCENE_C08_LEADER7             16598 /* 0x40d6 ソウリュウシティ：シャガ誘導イベントPOS制御用ワーク */
#define WK_SCENE_C08_SWPLAZMA            16599 /* 0x40d7 ソウリュウシティ：プラズマ団戦制御用ワーク */
#define WK_SCENE_C08_NINJA               16600 /* 0x40d8 ソウリュウシティ：ダークトリニティPOS制御用ワーク */
#define WK_SCENE_C08R0101                16601 /* 0x40d9 ソウリュウシティ民家：シャガ家SCENE_CHANGE制御用ワーク */
#define WK_SCENE_R11_POKE6               16602 /* 0x40da 11番道路：ビリジオン遭遇イベントPOS制御ワーク */
#define WK_SCENE_R13_POKE4               16603 /* 0x40db 13番道路：コバルオン遭遇イベントPOS制御ワーク */
#define WK_SCENE_C02R0601_LOSTITEM       16604 /* 0x40dc シッポウシティ：カフェソーコ落し物イベント進行制御ワーク */
#define WK_SCENE_D23R0101_ASSISTANTM     16605 /* 0x40dd コンビナート：マニアイベントのワーク */
#define WK_SCENE_C14_SWRIVAL_01          16606 /* 0x40de セイガイハシティ：到着時のライバルイベントPOS制御ワーク */
#define WK_SCENE_C14_SWLEADER8_01        16607 /* 0x40df セイガイハシティ：ジムクリア後のジムリーダーイベント POS制御ワーク */
#define WK_SCENE_R19R0101_INFOW_01       16608 /* 0x40e0 19番道路：ストッパーPOS制御ワーク */
#define WK_SCENE_H01R0201_STOP_01        16609 /* 0x40e1 スカイアローブリッジ：ヒウン側ゲートストッパーPOS制御ワーク */
#define WK_SCENE_C03_RALLY_01            16610 /* 0x40e2 ヒウンシティ：名所巡り制御ワーク */
#define WK_SCENE_R22_POKE5               16611 /* 0x40e3 22番道路：テラキオン遭遇POSイベント制御ワーク */
#define WK_SCENE_D26R0101_GATE_01        16612 /* 0x40e4 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_02        16613 /* 0x40e5 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_03        16614 /* 0x40e6 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_04        16615 /* 0x40e7 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_05        16616 /* 0x40e8 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_06        16617 /* 0x40e9 バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_07        16618 /* 0x40ea バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_08        16619 /* 0x40eb バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_D26R0101_GATE_09        16620 /* 0x40ec バッジチェックゲート：POS制御ワーク */
#define WK_SCENE_C03R0601_SWBELL_01      16621 /* 0x40ed ヒウンシティゲート：ベルイベントPOS制御ワーク */
#define WK_SCENE_C14R0301_BOY4_01        16622 /* 0x40ee セイガイハシティ：セイガイハシティのお坊ちゃま（死亡遊戯）制御ワーク */
#define WK_SCENE_R07R0102_GIRL3_01       16623 /* 0x40ef タワーオブヘブン：ヒトモシガール戦闘POS制御ワーク */
#define WK_SCENE_D27R0101_FIRST_01       16624 /* 0x40f0 プラズマフリゲート甲板：搭乗一回目イベント制御ワーク */
#define WK_SCENE_D27R0101_SECONDW_01     16625 /* 0x40f1 プラズマフリゲート甲板：搭乗二回目イベント制御ワーク（ホワイト） */
#define WK_SCENE_D27R0101_SECONDB_01     16626 /* 0x40f2 プラズマフリゲート甲板：搭乗二回目イベント制御ワーク（ブラック） */
#define WK_SCENE_D27R0401_SWBOSS_01      16627 /* 0x40f3 プラズマフリゲート操舵室：POS制御ワーク */
#define WK_SCENE_D27R0601_SWSAGE1_01     16628 /* 0x40f4 プラズマフリゲート隠し部屋：SCENE_CHANGE制御ワーク */
#define WK_SCENE_D27_SWITCH_01           16629 /* 0x40f5 プラズマフリゲート：スイッチ・バリアー制御POS */
#define WK_SCENE_D27_SWITCH_02           16630 /* 0x40f6 プラズマフリゲート：スイッチ・バリアー制御POS */
#define WK_SCENE_D27_SWITCH_03           16631 /* 0x40f7 プラズマフリゲート：スイッチ・バリアー制御POS */
#define WK_SCENE_D27_SWITCH_04           16632 /* 0x40f8 プラズマフリゲート：スイッチ・バリアー制御POS */
#define WK_SCENE_C01R0501_QUIZ1_01       16633 /* 0x40f9 サンヨウシティ：トレーナーズスクールのふわふわボーイクイズ制御 */
#define WK_SCENE_D27R1001_BARRIER_01     16634 /* 0x40fa プラズマフリゲート：バリアー制御POS */
#define WK_SCENE_D27R1201_BARRIER_01     16635 /* 0x40fb プラズマフリゲート：バリアー制御POS */
#define WK_SCENE_C03R0301_BTLCOM_01      16636 /* 0x40fc バトルカンパニー：POS制御ワーク */
#define WK_SCENE_C03R0303_BTLCOM_01      16637 /* 0x40fd バトルカンパニー：POS制御ワーク */
#define WK_SCENE_D07R0401_QUIZ_01        16638 /* 0x40fe ネジ山：重機マニアイベント進行制御ワーク */
#define WK_SCENE_D27R0701_PLAZMAM_01     16639 /* 0x40ff プラズマフリゲート船室廊下：イベント制御ワーク */
#define WK_SCENE_D27R0501_SWSAGE7_01     16640 /* 0x4100 プラズマフリゲートエンジン部屋：キュレム前イベント進行制御ワーク */
#define WK_SCENE_D27R0601_STOPPER_01     16641 /* 0x4101 プラズマフリゲート隠し部屋：POS制御ワーク */
#define WK_SCENE_D27R1001_PLAZMAM_01     16642 /* 0x4102 プラズマフリゲートエンジン部屋：イベント制御ワーク */
#define WK_SCENE_D27R1301_PLAZMAM_01     16643 /* 0x4103 プラズマフリゲートエンジン部屋：イベント制御ワーク */
#define WK_SCENE_D27R1001_SWSAGE7_01     16644 /* 0x4104 プラズマフリゲートエンジン部屋：POS制御ワーク */
#define WK_SCENE_D27R1301_SWSAGE7_01     16645 /* 0x4105 プラズマフリゲートエンジン部屋：POS制御ワーク */
#define WK_SCENE_W21_01                  16646 /* 0x4106 21番水路：POS制御ワーク */
#define WK_SCENE_T02R0401_GIRL4_01       16647 /* 0x4107 カラクサタウン民家：スランプピアノガールイベント進行制御ワーク */
#define WK_SCENE_BRIDGE_MANIA_01         16648 /* 0x4108 h01&#xff5e;h06：橋マニア */
#define WK_SCENE_C09                     16649 /* 0x4109 ポケモンリーグ：初回挑戦時の解説POSイベントの制御 */
#define WK_SCENE_C15_TUTORIAL_01         16650 /* 0x410a ポケウッド：チュートリアルイベント制御ワーク */
#define WK_SCENE_C16_TUTORIAL_01         16651 /* 0x410b ジョインアベニュー：チュートリアルイベント制御ワーク */
#define WK_SCENE_D01R0101_RATI2          16652 /* 0x410c 夢の跡地：ラティオス（ラティアス）POSイベントの制御 */
#define WK_SCENE_D26R0101_BOSS           16653 /* 0x410d バッジチェックゲート：NイベントPOS制御ワーク */
#define WK_SCENE_C03P07_CGEAR_01         16654 /* 0x410e ヒウンシティ・チャレンジストリート：マコモＣギア入手イベント制御ワーク */
#define WK_SCENE_C03P08_FESGUIDE_01      16655 /* 0x410f ヒウンシティ：フェスチュートリアルPOS制御ワーク */
#define WK_SCENE_C16_TUTORIAL_SEQ        16656 /* 0x4110 ジョインアベニュー：チュートリアルイベント進行ワーク */
#define WK_SCENE_D02R0101_SWLEADER1      16657 /* 0x4111 ヤグルマの森：チェレン連れ歩きPOS制御ワーク */
#define WK_SCENE_N01R0401_BOSS           16658 /* 0x4112 N城：N連れ歩きPOS制御ワーク */
#define WK_SCENE_N01R0402_BOSS           16659 /* 0x4113 N城：イベントPOS制御ワーク */
#define WK_SCENE_N01R0502_BOSS           16660 /* 0x4114 N城：謁見の間イベント進行制御ワーク */
#define WK_SCENE_SPPOKE_EVENT_01         16661 /* 0x4115 ヒオウギシティ 心の空洞：ユクシー・エムリット・アグノム遭遇イベント制御 */
#define WK_SCENE_C02_SPPOKE1_01          16662 /* 0x4116 シッポウシティ：ユクシーイベント制御 */
#define WK_SCENE_R07R0105_SPPOKE2_01     16663 /* 0x4117 タワーオブヘブン：エムリットイベント制御 */
#define WK_SCENE_R23_SPPOKE3_01          16664 /* 0x4118 チャンピオンロード：アグノムイベント制御 */
#define WK_SCENE_C16R0101_PRIZE          16665 /* 0x4119 ジョインアベニュー：ご褒美イベントSECNE制御ワーク */
#define WK_SCENE_C03R1301_SUNGLASSES     16666 /* 0x411a ヒウンシティお金持ちビル：ボディチェックPOS制御ワーク */
#define WK_SCENE_D08R0901_LEGEND         16667 /* 0x411b リュウラセンの塔最上階：レシラム・ゼクロムPOS制御ワーク */
#define WK_SCENE_R09R0101_STOP_01        16668 /* 0x411c 9番道路ゲート：シリンダーブリッジのストッパーPOS制御 */
#define WK_SCENE_N01R0501_BOSS           16669 /* 0x411d N城：謁見の間前イベント進行制御ワーク */
#define WK_SCENE_T04R101_STOPPER_01      16670 /* 0x411e サザナミゲート：ストッパーPOS制御ワーク */
#define WK_SCENE_C14GYM0101_STOPPER      16671 /* 0x411f セイガイハシティジム：ストッパーPOS制御ワーク */
#define WK_SCENE_D24_PAIR_START          16672 /* 0x4120 リバースマウンテン：ベル連れ歩きイベント制御ワーク */
#define WK_SCENE_D24_BELL_COMMENT        16673 /* 0x4121 リバースマウンテン・ヒードラン部屋：POS制御ワーク */
#define WK_SCENE_C14_PHOTO_01            16674 /* 0x4122 セイガイハシティ：顔だし看板POS制御ワーク */
#define WK_SCENE_D26_POKE23_01           16675 /* 0x4123 チャンピオンロード：クリア後ゾロアークPOS制御ワーク */
#define WK_SCENE_D09R1401_SWRIVAL_01     16676 /* 0x4124 チャンピオンロード：ライバル戦POS制御ワーク */
#define WK_SCENE_D27R10R13_STOPPER       16677 /* 0x4125 プラズマフリゲート：ストッパーPOS制御ワーク */
#define WK_SCENE_C04_LOCALTVT            16678 /* 0x4126 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_C05_LOCALTVT            16679 /* 0x4127 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_C06_LOCALTVT            16680 /* 0x4128 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_T03_LOCALTVT            16681 /* 0x4129 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_T04_LOCALTVT            16682 /* 0x412a ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_T07_LOCALTVT            16683 /* 0x412b ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R05_LOCALTVT            16684 /* 0x412c ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R06_LOCALTVT            16685 /* 0x412d ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R07_LOCALTVT            16686 /* 0x412e ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R09_LOCALTVT            16687 /* 0x412f ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R11_LOCALTVT            16688 /* 0x4130 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R12_LOCALTVT            16689 /* 0x4131 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R13_LOCALTVT            16690 /* 0x4132 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R14_LOCALTVT            16691 /* 0x4133 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_R16_LOCALTVT            16692 /* 0x4134 ローカルライブキャスター隠しキャラPOS制御ワーク */
#define WK_SCENE_D05R0101                16693 /* 0x4135 WBT：チュートリアルイベント制御ワーク */
#define WK_SCENE_R01_CONV                16694 /* 0x4136 1番道路：データコンバートベル戦POS制御ワーク */
#define WK_SCENE_C03P12                  16695 /* 0x4137 ヒウンシティ：フラッシュ入手POS制御ワーク */
#define WK_SCENE_D12                     16696 /* 0x4138 リバティガーデン島：シーンチェンジイベント用 */
#define WK_SCENE_R05_SWBELL_01           16697 /* 0x4139 5番道路：ベルかくれあなイベントPOS制御ワーク */
#define WK_SCENE_C16_PARADE              16698 /* 0x413a ジョインアベニュー：パレード用イベント制御ワーク */
#define WK_SCENE_D07                     16699 /* 0x413b ネジ山フィールド：ストッパーイベント制御ワーク */
#define WK_SCENE_C15_BONUS_EVENT_01      16700 /* 0x413c ポケウッド：ごほうびイベント制御ワーク */
#define WK_SCENE_C15R0401                16701 /* 0x413d ポケウッド：初回入室時イベント */
#define WK_SCENE_C08_LINK_EVENT          16702 /* 0x413e ソウリュウシティ：リンクイベント制御ワーク */
#define WK_SCENE_C05R0501_LINK_EVENT     16703 /* 0x413f ホドモエシティ：リンクイベントSCENE_CHANGE制御ワーク */
#define WK_SCENE_C02_LINK_EVENT          16704 /* 0x4140 シッポウシティ：リンクイベント制御ワーク */
#define WK_SCENE_C04_TRIAL               16705 /* 0x4141 ライモンシティ：トライアルハウスイベント用 */
#define WK_SCENE_C04GYM0101_GUIDE        16706 /* 0x4142 ライモンシティジム：ジムガイドイベント用 */
#define WK_SCENE_D22_BOY2                16707 /* 0x4143 サンギ牧場：誘導用POS制御ワーク */
#define WK_SCENE_MUGENDUNGEON_CLEAR      16708 /* 0x4144 無限ダンジョン：自分のROMバージョンと同じダンジョンをクリアしたか */
#define WK_SCENE_C08GYM0101_GUIDE        16709 /* 0x4145 ドラゴンジム：ジムガイドイベント用 */
#define WK_SCENE_T04_D20_TVT             16710 /* 0x4146 サザナミタウン・サザナミ湾：ライブキャスターPOS制御ワーク */
#define WK_SCENE_C03P19                  16711 /* 0x4147 ヒウンシティはじまりの場所：ワルそうなチームイベント進行制御ワーク */
#define WK_SCENE_D08R0101                16712 /* 0x4148 リュウラセンの塔フィールド：アララギ父イベント用 */
#define WK_SCENE_D27_ANNOUNCE            16713 /* 0x4149 プラズマフリゲート：船内アナウンス */
#define WK_SCENE_D27_HINT_01             16714 /* 0x414a プラズマフリゲート：POS制御トレーナー用 */
#define WK_SCENE_D27_HINT_02             16715 /* 0x414b プラズマフリゲート：POS制御トレーナー用 */
#define WK_SCENE_D27_HINT_03             16716 /* 0x414c プラズマフリゲート：POS制御トレーナー用 */
#define WK_SCENE_D27_HINT_04             16717 /* 0x414d プラズマフリゲート：POS制御トレーナー用 */
#define WK_SCENE_D27_HINT_05             16718 /* 0x414e プラズマフリゲート：POS制御トレーナー用 */
#define WK_SCENE_H05_GIRL                16719 /* 0x414f ワンダーブリッジ：消える少女制御用 */
#define WK_SCENE_R14R0201_RANDORUS       16720 /* 0x4150 豊穣の社：うつしかがみイベント用ワーク */
#define WK_SCENE_R20R0101                16721 /* 0x4151 20番道路ゲート：POS制御用ワーク */
#define WK_SCENE_D32R0101_FIGHTERM_01    16722 /* 0x4152 海辺の洞穴：POS制御用ワーク */
#define WK_SCENE_T06_ZUKAN_01            16723 /* 0x4153 サンギタウン：せいそくちリストPOS制御用ワーク */
#define WK_SCENE_C06_LINK_EVENT          16724 /* 0x4154 フキヨセシティ：リンクイベント制御ワーク */
#define WK_SCENE_R14R0201_BOY2_01        16725 /* 0x4155 豊穣の社：伝承イベント用SCENE_CHANGE起動ワーク */

#define WK_SCENE_AREA_END                16735 /* 0x415f  */


/* flags "other_work.xls" 16736 ～ 16815 */

#define WK_AREA_START                    16736 /* 0x4160  */

#define WK_OTHER_SPORTS_EVENT            16736 /* 0x4160 ビッグスタジアム/リトルコート：種目判別用ワーク 戻り値は user_define.h を参照 */
#define WK_OTHER_SYMMAP_POS              16737 /* 0x4161 シンボルマップでのPOSイベント制御用 */
#define WK_OTHER_T05_TRAIN               16738 /* 0x4162 T05カナワタウン：転車台に配置される車両の状態を保存するワーク 00 車両が配置されていない 01 シングルトレイン 02 スーパーシングルトレイン 03 ダブルトレイン 04 スーパーダブルトレイン 05 マルチトレイン 06 スーパーマルチトレイン 07 i-Fiトレイン 08 トレインタウン 09 古いレアトレイン 10 新しいレアトレイン※特別な日に出現 */
#define WK_OTHER_EMPEROR_MONSNUM         16739 /* 0x4163 シリンダーブリッジ暴走族イベント：暴走族のヘッドを倒した時の 戦闘可能な先頭ポケモンのモンスターナンバーを格納 */
#define WK_OTHER_BSEVENT_ITEM            16740 /* 0x4164 カナワタウン民家：バトルサブウェイの落し物イベントの入手アイテムを格納 */
#define WK_OTHER_BSEVENT_NUM             16741 /* 0x4165 カナワタウン民家：バトルサブウェイの落し物イベントの入手アイテム数を格納 */
#define WK_OTHER_T04R0301_SHIRONA        16742 /* 0x4166 サザナミタウン別荘：日替わり固定のメッセージを格納（夏のみ） */
#define WK_OTHER_T04R0301_BIGFOUR4       16743 /* 0x4167 サザナミタウン別荘：日替わり固定のメッセージを格納（春のみ） */
#define WK_OTHER_T04R0301_BIGFOUR1       16744 /* 0x4168 サザナミタウン別荘：日替わり固定のメッセージを格納（春のみ） */
		/* not used 16745(0x4169) */
#define WK_OTHER_T04R0301_SWLEADER6      16746 /* 0x416a サザナミタウン別荘：日替わり固定のメッセージを格納（夏のみ） */
#define WK_OTHER_T04R0301_SWLEADER4      16747 /* 0x416b サザナミタウン別荘：日替わり固定のメッセージを格納（夏のみ） */
#define WK_OTHER_T04R0301_SWLEADER2      16748 /* 0x416c サザナミタウン別荘：日替わり固定のメッセージを格納（夏のみ） */
#define WK_OTHER_SPORTS_RND_TR0          16749 /* 0x416d ビッグスタジアムのランダム配置トレーナー１人目 */
#define WK_OTHER_SPORTS_RND_TR1          16750 /* 0x416e ビッグスタジアムのランダム配置トレーナー２人目 */
#define WK_OTHER_SPORTS_RND_TR2          16751 /* 0x416f ビッグスタジアムのランダム配置トレーナー３人目 */
#define WK_OTHER_SPORTS_RND_TR3          16752 /* 0x4170 リトルコートのランダム配置トレーナー１人目 */
#define WK_OTHER_SPORTS_RND_TR4          16753 /* 0x4171 リトルコートのランダム配置トレーナー２人目 */
#define WK_OTHER_SPORTS_RND_TR5          16754 /* 0x4172 リトルコートのランダム配置トレーナー３人目 */
#define WK_OTHER_WHEEL_SEASON            16755 /* 0x4173 観覧車イベント：季節を保存しておくワーク。 C04P01のゾーンチェンジで季節を取得し、前回から季節が変化していたら、観覧車トレーナー用のタイムフラグをリセットする */
#define WK_OTHER_D11R0101_PAIR           16756 /* 0x4174 下水道：ライバル連れ歩きセリフ */
#define WK_OTHER_R05_KINOMI              16757 /* 0x4175 スワン5番道路木の実販売 */
#define WK_OTHER_BSUBWAY_RECEIPT         16758 /* 0x4176 バトルサブウェイ、受付制御 */
#define WK_OTHER_BSUBWAY_TEST            16759 /* 0x4177 バトルサブウェイ、テスト */
#define WK_OTHER_BSUBWAY_TRAIN           16760 /* 0x4178 バトルサブウェイ、列車内制御 */
#define WK_OTHER_BSUBWAY_HOME            16761 /* 0x4179 バトルサブウェイ、ホーム制御 */
#define WK_OTHER_KASEKI_NUM              16762 /* 0x417a 化石復元イベント用ワーク 化石ポケモン預かり状態か識別するためのワーク。アイテムナンバーを格納。 */
#define WK_OTHER_PASSENGERBOAT           16763 /* 0x417b 遊覧船イベント用ワーク 遊覧船のイベント状態を保存するワーク。船内案内済みか判定に使用 00 初期状態 01 乗船後、案内イベント済み 02 全てのトレーナーを倒した */
#define WK_OTHER_STATION_CONNECT         16764 /* 0x417c C04とギアステーションの接続イベント用ワーク 00 ギアステーションからC04にでる階段前のPOSイベント有効 01 ギアステーション入場アニメ有効（シーンチェンジスクリプト） */
#define WK_OTHER_C03R0201_GALLERY        16765 /* 0x417d 日替わりタイプ指定ポケモン見せ用ワーク */
#define WK_OTHER_UN                      16766 /* 0x417e 国連イベント用ワーク 00 参加していない（受付がゲートを塞いでいる） 01 参加状態（受付を通過している→受付をどかした状態にする） */
#define WK_OTHER_DISCOVER_EGG            16767 /* 0x417f ３番道路：育て屋でタマゴが発見されたときに教えてくれるPOSイベント用のワーク 00 なし 01 POSイベント有効。「おーい！」と声をかけてくれる。 */
#define WK_OTHER_C05R0701_WAZA           16768 /* 0x4180 日替わり指定わざ見せ用ワーク */
#define WK_OTHER_C12_RIVAL_PAIR          16769 /* 0x4181 スワン：ヒオウギシティ連れ歩き用ワーク 0&#xff5e;2でセリフを変える */
#define WK_OTHER_FISHING_MONSNUM         16770 /* 0x4182 ビレッジブリッジ：日替わり指定ポケモン格納用ワーク */
#define WK_OTHER_C05R0301_WAZA           16771 /* 0x4183 日替わり指定わざ見せ用ワーク */
#define WK_OTHER_R9R0201_ITEMBUY         16772 /* 0x4184 アールナイン：１日１回買い物お願いお嬢様イベント制御用ワーク */
#define WK_OTHER_H03_BOUON_MONSNUM       16773 /* 0x4185 シリンダーブリッジ：特性「ぼうおん」のポケモンを見せた後、そのポケモン名を固定する */
#define WK_OTHER_R02_HAADOROKKU          16774 /* 0x4186 2番道路：特性「ハードロック」のポケモンを見せた後、そのポケモン名を固定する */
#define WK_OTHER_R04_PARA_CHECK_01       16775 /* 0x4187 パラメータチェッカーで指定されたパラメータ格納用ワーク。 ブラック版は「すばやさ」、ホワイト版は「ＨＰ」をチェックする */
#define WK_OTHER_AMUSEMENT_PARTNER       16776 /* 0x4188 ライモン遊園地：観覧車でのパートナーを保存しておく用ワーク */
#define WK_OTHER_LCC_TRADE_CNT           16777 /* 0x4189 ライモン遊園地：観覧車で隠しキャラとのデート後の交換回数カウント用ワーク */
#define WK_OTHER_D02R0101_PAIR           16778 /* 0x418a ヤグルマの森：チェレン連れ歩きセリフ */
#define WK_OTHER_N01R0401_PAIR           16779 /* 0x418b Ｎ城：N連れ歩きセリフ */
#define WK_OTHER_C07_SEASON              16780 /* 0x418c セッカシティ：ダークトリニティイベントに使用 ゾーンチェンジラベルで現季節とワークの季節で差分があれば０に戻す */
#define WK_OTHER_C01GYM0101_BTL          16781 /* 0x418d サンヨウレストラン：ジムリーダー戦闘後会話分岐用 */
#define WK_OTHER_C01GYM0101_SARU         16782 /* 0x418e サンヨウレストラン：３猿決定用ワーク */
#define WK_OTHER_D20_ARRIVE              16783 /* 0x418f 海底遺跡到達度・取得ワーク */
#define WK_OTHER_INF_ELEVATOR_UP_MOVE    16784 /* 0x4190 無限ダンジョン内のエレベーター移動方向 */
#define WK_OTHER_INF_EXIT_TYPE           16785 /* 0x4191 無限ダンジョン内の終了方法 */
#define WK_OTHER_LINKEVENT_ZONE          16786 /* 0x4192 PC：おもいでリンク起動箇所 */
#define WK_OTHER_C16_NEON_SELECT         16787 /* 0x4193 ジョインアベニュー　ネオン最終確認 */
#define WK_OTHER_N01R0502_N_SEASON       16788 /* 0x4194 N城：Nと戦闘した季節格納ワーク */
#define WK_OTHER_D24_PAIR                16789 /* 0x4195 リバースマウンテン：ベル連れ歩きセリフ */
#define WK_OTHER_C01GYM0101_LINK_TALK    16790 /* 0x4196 サンヨウジム：おもいでリンク起動人物 */

#define WK_AREA_END                      16815 /* 0x41af  */


/* flags "temp_work.xls" 32768 ～ 32799 */

#define SCWK_AREA_START                  32768 /* 0x8000  */

#define SCWK_PARAM0                      32768 /* 0x8000 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM1                      32769 /* 0x8001 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM2                      32770 /* 0x8002 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM3                      32771 /* 0x8003 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM4                      32772 /* 0x8004 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM5                      32773 /* 0x8005 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM6                      32774 /* 0x8006 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_PARAM7                      32775 /* 0x8007 共通スクリプト呼び出し処理用ワーク（使用禁止） */
#define SCWK_TEMP0                       32776 /* 0x8008 汎用ワーク */
#define SCWK_TEMP1                       32777 /* 0x8009 汎用ワーク */
#define SCWK_TEMP2                       32778 /* 0x800a 汎用ワーク */
#define SCWK_TEMP3                       32779 /* 0x800b 汎用ワーク */
#define SCWK_REG0                        32780 /* 0x800c スクリプト内部処理用ワーク（使用禁止） */
#define SCWK_REG1                        32781 /* 0x800d スクリプト内部処理用ワーク（使用禁止） */
#define SCWK_REG2                        32782 /* 0x800e スクリプト内部処理用ワーク（使用禁止） */
#define SCWK_REG3                        32783 /* 0x800f スクリプト内部処理用ワーク（使用禁止） */
#define SCWK_ANSWER                      32784 /* 0x8010 結果受け取り汎用ワーク */
#define SCWK_TARGET_OBJID                32785 /* 0x8011  */

#define SCWK_AREA_END                    32799 /* 0x801f  */


/* flags "user_work.xls" 32800 ～ 32863 */

#define USERWK_AREA_START                32800 /* 0x8020  */

#define USERWK_00                        32800 /* 0x8020 イベント起動中のみ機能するフラグ */
#define USERWK_01                        32801 /* 0x8021 イベント起動中のみ機能するフラグ */
#define USERWK_02                        32802 /* 0x8022 イベント起動中のみ機能するフラグ */
#define USERWK_03                        32803 /* 0x8023 イベント起動中のみ機能するフラグ */
#define USERWK_04                        32804 /* 0x8024 イベント起動中のみ機能するフラグ */
#define USERWK_05                        32805 /* 0x8025 イベント起動中のみ機能するフラグ */
#define USERWK_06                        32806 /* 0x8026 イベント起動中のみ機能するフラグ */
#define USERWK_07                        32807 /* 0x8027 イベント起動中のみ機能するフラグ */
#define USERWK_08                        32808 /* 0x8028 イベント起動中のみ機能するフラグ */
#define USERWK_09                        32809 /* 0x8029 イベント起動中のみ機能するフラグ */
#define USERWK_10                        32810 /* 0x802a イベント起動中のみ機能するフラグ */
#define USERWK_11                        32811 /* 0x802b イベント起動中のみ機能するフラグ */
#define USERWK_12                        32812 /* 0x802c イベント起動中のみ機能するフラグ */
#define USERWK_13                        32813 /* 0x802d イベント起動中のみ機能するフラグ */
#define USERWK_14                        32814 /* 0x802e イベント起動中のみ機能するフラグ */
#define USERWK_15                        32815 /* 0x802f イベント起動中のみ機能するフラグ */
#define USERWK_16                        32816 /* 0x8030 イベント起動中のみ機能するフラグ */
#define USERWK_17                        32817 /* 0x8031 イベント起動中のみ機能するフラグ */
#define USERWK_18                        32818 /* 0x8032 イベント起動中のみ機能するフラグ */
#define USERWK_19                        32819 /* 0x8033 イベント起動中のみ機能するフラグ */
#define USERWK_20                        32820 /* 0x8034 イベント起動中のみ機能するフラグ */
#define USERWK_21                        32821 /* 0x8035 イベント起動中のみ機能するフラグ */
#define USERWK_22                        32822 /* 0x8036 イベント起動中のみ機能するフラグ */
#define USERWK_23                        32823 /* 0x8037 イベント起動中のみ機能するフラグ */
#define USERWK_24                        32824 /* 0x8038 イベント起動中のみ機能するフラグ */
#define USERWK_25                        32825 /* 0x8039 イベント起動中のみ機能するフラグ */
#define USERWK_26                        32826 /* 0x803a イベント起動中のみ機能するフラグ */
#define USERWK_27                        32827 /* 0x803b イベント起動中のみ機能するフラグ */
#define USERWK_28                        32828 /* 0x803c イベント起動中のみ機能するフラグ */
#define USERWK_29                        32829 /* 0x803d イベント起動中のみ機能するフラグ */
#define USERWK_30                        32830 /* 0x803e イベント起動中のみ機能するフラグ */
#define USERWK_31                        32831 /* 0x803f イベント起動中のみ機能するフラグ */
#define USERWK_32                        32832 /* 0x8040 イベント起動中のみ機能するフラグ */
#define USERWK_33                        32833 /* 0x8041 イベント起動中のみ機能するフラグ */
#define USERWK_34                        32834 /* 0x8042 イベント起動中のみ機能するフラグ */
#define USERWK_35                        32835 /* 0x8043 イベント起動中のみ機能するフラグ */
#define USERWK_36                        32836 /* 0x8044 イベント起動中のみ機能するフラグ */
#define USERWK_37                        32837 /* 0x8045 イベント起動中のみ機能するフラグ */
#define USERWK_38                        32838 /* 0x8046 イベント起動中のみ機能するフラグ */
#define USERWK_39                        32839 /* 0x8047 イベント起動中のみ機能するフラグ */
#define USERWK_40                        32840 /* 0x8048 イベント起動中のみ機能するフラグ */
#define USERWK_41                        32841 /* 0x8049 イベント起動中のみ機能するフラグ */
#define USERWK_42                        32842 /* 0x804a イベント起動中のみ機能するフラグ */
#define USERWK_43                        32843 /* 0x804b イベント起動中のみ機能するフラグ */
#define USERWK_44                        32844 /* 0x804c イベント起動中のみ機能するフラグ */
#define USERWK_45                        32845 /* 0x804d イベント起動中のみ機能するフラグ */
#define USERWK_46                        32846 /* 0x804e イベント起動中のみ機能するフラグ */
#define USERWK_47                        32847 /* 0x804f イベント起動中のみ機能するフラグ */
#define USERWK_48                        32848 /* 0x8050 イベント起動中のみ機能するフラグ */
#define USERWK_49                        32849 /* 0x8051 イベント起動中のみ機能するフラグ */
#define USERWK_50                        32850 /* 0x8052 イベント起動中のみ機能するフラグ */
#define USERWK_51                        32851 /* 0x8053 イベント起動中のみ機能するフラグ */
#define USERWK_52                        32852 /* 0x8054 イベント起動中のみ機能するフラグ */
#define USERWK_53                        32853 /* 0x8055 イベント起動中のみ機能するフラグ */
#define USERWK_54                        32854 /* 0x8056 イベント起動中のみ機能するフラグ */
#define USERWK_55                        32855 /* 0x8057 イベント起動中のみ機能するフラグ */
#define USERWK_56                        32856 /* 0x8058 イベント起動中のみ機能するフラグ */
#define USERWK_57                        32857 /* 0x8059 イベント起動中のみ機能するフラグ */
#define USERWK_58                        32858 /* 0x805a イベント起動中のみ機能するフラグ */
#define USERWK_59                        32859 /* 0x805b イベント起動中のみ機能するフラグ */
#define USERWK_60                        32860 /* 0x805c イベント起動中のみ機能するフラグ */
#define USERWK_61                        32861 /* 0x805d イベント起動中のみ機能するフラグ */
#define USERWK_62                        32862 /* 0x805e イベント起動中のみ機能するフラグ */
#define USERWK_63                        32863 /* 0x805f イベント起動中のみ機能するフラグ */

#define USERWK_AREA_END                  32863 /* 0x805f  */




#define WORKS_TOTAL_MAX  32864 /* 0x8060 */


/* 自動生成フラグ定義ファイル 終了*/

