#pragma once

#include "dpw_bt.h"
#include "dpw_tr.h"
#include < stdio.h >
#include < stdlib.h >
#include < vcclr.h >

namespace PokeIRC {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

#define BOX_MAX_RAW				(5)
#define BOX_MAX_COLUMN			(6)
#define BOX_MAX_TRAY			(18)
#define BOX_TRAYNAME_MAXLEN		(8)
#define BOX_TRAYNAME_BUFSIZE	(20)	// 日本語８文字＋EOM。海外版用の余裕も見てこの程度。
#define BOX_MAX_POS				(BOX_MAX_RAW*BOX_MAX_COLUMN)


	static const char PROGRAM_NAME[] = "Global Pokemon FunClub";

	typedef struct{
		int no;
		char* name;
	} PokeData;

	// ボックスリスト
	typedef struct {
	    short pokeno[BOX_MAX_POS];
	    short pokelv[BOX_MAX_POS];
	    unsigned short trayName[BOX_TRAYNAME_BUFSIZE+2];
	} BoxTrayData;


	static const	PokeData	PokeGraNoTable[]={
	0,			"０オリジン",
	  1,		"フシギダネ",
	  2,		"フシギソウ",
	  3,		"フシギバナ",
	  4,		"ヒトカゲ",
	  5,		"リザード",
	  6,		"リザードン",
	  7,		"ゼニガメ",
	  8,		"カメール",
	  9,		"カメックス",
	 10,		"キャタピー",
	 11,		"トランセル",
	 12,		"バタフリー",
	 13,		"ビードル",
	 14,		"コクーン",
	 15,		"スピアー",
	 16,		"ポッポ",
	 17,		"ピジョン",
	 18,		"ピジョット",
	 19,		"コラッタ",
	 20,		"ラッタ",
	 21,		"オニスズメ",
	 22,		"オニドリル",
	 23,		"アーボ",
	 24,		"アーボック",
	 25,		"ピカチュウ",
	 26,		"ライチュウ",
	 27,		"サンド",
	 28,		"サンドパン",
	 29,		"ニドラン♀",
	 30,		"ニドリーナ",
	 31,		"ニドクイン",
	 32,		"ニドラン♂",
	 33,		"ニドリーノ",
	 34,		"ニドキング",
	 35,		"ピッピ",
	 36,		"ピクシー",
	 37,		"ロコン",
	 38,		"キュウコン",
	 39,		"プリン",
	 40,		"プクリン",
	 41,		"ズバット",
	 42,		"ゴルバット",
	 43,		"ナゾノクサ",
	 44,		"クサイハナ",
	 45,		"ラフレシア",
	 46,		"パラス",
	 47,		"パラセクト",
	 48,		"コンパン",
	 49,		"モルフォン",
	 50,		"ディグダ",
	 51,		"ダグトリオ",
	 52,		"ニャース",
	 53,		"ペルシアン",
	 54,		"コダック",
	 55,		"ゴルダック",
	 56,		"マンキー",
	 57,		"オコリザル",
	 58,		"ガーディ",
	 59,		"ウインディ",
	 60,		"ニョロモ",
	 61,		"ニョロゾ",
	 62,		"ニョロボン",
	 63,		"ケーシィ",
	 64,		"ユンゲラー",
	 65,		"フーディン",
	 66,		"ワンリキー",
	 67,		"ゴーリキー",
	 68,		"カイリキー",
	 69,		"マダツボミ",
	 70,		"ウツドン",
	 71,		"ウツボット",
	 72,		"メノクラゲ",
	 73,		"ドククラゲ",
	 74,		"イシツブテ",
	 75,		"ゴローン",
	 76,		"ゴローニャ",
	 77,		"ポニータ",
	 78,		"ギャロップ",
	 79,		"ヤドン",
	 80,		"ヤドラン",
	 81,		"コイル",
	 82,		"レアコイル",
	 83,		"カモネギ",
	 84,		"ドードー",
	 85,		"ドードリオ",
	 86,		"パウワウ",
	 87,		"ジュゴン",
	 88,		"ベトベター",
	 89,		"ベトベトン",
	 90,		"シェルダー",
	 91,		"パルシェン",
	 92,		"ゴース",
	 93,		"ゴースト",
	 94,		"ゲンガー",
	 95,		"イワーク",
	 96,		"スリープ",
	 97,		"スリーパー",
	 98,		"クラブ",
	 99,		"キングラー",
	100,		"ビリリダマ",
	101,		"マルマイン",
	102,		"タマタマ",
	103,		"ナッシー",
	104,		"カラカラ",
	105,		"ガラガラ",
	106,		"サワムラー",
	107,		"エビワラー",
	108,		"ベロリンガ",
	109,		"ドガース",
	110,		"マタドガス",
	111,		"サイホーン",
	112,		"サイドン",
	113,		"ラッキー",
	114,		"モンジャラ",
	115,		"ガルーラ",
	116,		"タッツー",
	117,		"シードラ",
	118,		"トサキント",
	119,		"アズマオウ",
	120,		"ヒトデマン",
	121,		"スターミー",
	122,		"バリヤード",
	123,		"ストライク",
	124,		"ルージュラ",
	125,		"エレブー",
	126,		"ブーバー",
	127,		"カイロス",
	128,		"ケンタロス",
	129,		"コイキング",
	130,		"ギャラドス",
	131,		"ラプラス",
	132,		"メタモン",
	133,		"イーブイ",
	134,		"シャワーズ",
	135,		"サンダース",
	136,		"ブースター",
	137,		"ポリゴン",
	138,		"オムナイト",
	139,		"オムスター",
	140,		"カブト",
	141,		"カブトプス",
	142,		"プテラ",
	143,		"カビゴン",
	144,		"フリーザー",
	145,		"サンダー",
	146,		"ファイヤー",
	147,		"ミニリュウ",
	148,		"ハクリュー",
	149,		"カイリュー",
	150,		"ミュウツー",
	151,		"ミュウ",
	152,		"チコリータ",
	153,		"ベイリーフ",
	154,		"メガニウム",
	155,		"ヒノアラシ",
	156,		"マグマラシ",
	157,		"バクフーン",
	158,		"ワニノコ",
	159,		"アリゲイツ",
	160,		"オーダイル",
	161,		"オタチ",
	162,		"オオタチ",
	163,		"ホーホー",
	164,		"ヨルノズク",
	165,		"レディバ",
	166,		"レディアン",
	167,		"イトマル",
	168,		"アリアドス",
	169,		"クロバット",
	170,		"チョンチー",
	171,		"ランターン",
	172,		"ピチュー",
	173,		"ピィ",
	174,		"ププリン",
	175,		"トゲピー",
	176,		"トゲチック",
	177,		"ネイティ",
	178,		"ネイティオ",
	179,		"メリープ",
	180,		"モココ",
	181,		"デンリュウ",
	182,		"キレイハナ",
	183,		"マリル",
	184,		"マリルリ",
	185,		"ウソッキー",
	186,		"ニョロトノ",
	187,		"ハネッコ",
	188,		"ポポッコ",
	189,		"ワタッコ",
	190,		"エイパム",
	191,		"ヒマナッツ",
	192,		"キマワリ",
	193,		"ヤンヤンマ",
	194,		"ウパー",
	195,		"ヌオー",
	196,		"エーフィ",
	197,		"ブラッキー",
	198,		"ヤミカラス",
	199,		"ヤドキング",
	200,		"ムウマ",
	201,		"アンノーン",
	202,		"ソーナンス",
	203,		"キリンリキ",
	204,		"クヌギダマ",
	205,		"フォレトス",
	206,		"ノコッチ",
	207,		"グライガー",
	208,		"ハガネール",
	209,		"ブルー",
	210,		"グランブル",
	211,		"ハリーセン",
	212,		"ハッサム",
	213,		"ツボツボ",
	214,		"ヘラクロス",
	215,		"ニューラ",
	216,		"ヒメグマ",
	217,		"リングマ",
	218,		"マグマッグ",
	219,		"マグカルゴ",
	220,		"ウリムー",
	221,		"イノムー",
	222,		"サニーゴ",
	223,		"テッポウオ",
	224,		"オクタン",
	225,		"デリバード",
	226,		"マンタイン",
	227,		"エアームド",
	228,		"デルビル",
	229,		"ヘルガー",
	230,		"キングドラ",
	231,		"ゴマゾウ",
	232,		"ドンファン",
	233,		"ポリゴン２",
	234,		"オドシシ",
	235,		"ドーブル",
	236,		"バルキー",
	237,		"カポエラー",
	238,		"ムチュール",
	239,		"エレキッド",
	240,		"ブビィ",
	241,		"ミルタンク",
	242,		"ハピナス",
	243,		"ライコウ",
	244,		"エンテイ",
	245,		"スイクン",
	246,		"ヨーギラス",
	247,		"サナギラス",
	248,		"バンギラス",
	249,		"ルギア",
	250,		"ホウオウ",
	251,		"セレビィ",
	252,		"キモリ",
	253,		"ジュプトル",
	254,		"ジュカイン",
	255,		"アチャモ",
	256,		"ワカシャモ",
	257,		"バシャーモ",
	258,		"ミズゴロウ",
	259,		"ヌマクロー",
	260,		"ラグラージ",
	261,		"ポチエナ",
	262,		"グラエナ",
	263,		"ジグザグマ",
	264,		"マッスグマ",
	265,		"ケムッソ",
	266,		"カラサリス",
	267,		"アゲハント",
	268,		"マユルド",
	269,		"ドクケイル",
	270,		"ハスボー",
	271,		"ハスブレロ",
	272,		"ルンパッパ",
	273,		"タネボー",
	274,		"コノハナ",
	275,		"ダーテング",
	276,		"スバメ",
	277,		"オオスバメ",
	278,		"キャモメ",
	279,		"ペリッパー",
	280,		"ラルトス",
	281,		"キルリア",
	282,		"サーナイト",
	283,		"アメタマ",
	284,		"アメモース",
	285,		"キノココ",
	286,		"キノガッサ",
	287,		"ナマケロ",
	288,		"ヤルキモノ",
	289,		"ケッキング",
	290,		"ツチニン",
	291,		"テッカニン",
	292,		"ヌケニン",
	293,		"ゴニョニョ",
	294,		"ドゴーム",
	295,		"バクオング",
	296,		"マクノシタ",
	297,		"ハリテヤマ",
	298,		"ルリリ",
	299,		"ノズパス",
	300,		"エネコ",
	301,		"エネコロロ",
	302,		"ヤミラミ",
	303,		"クチート",
	304,		"ココドラ",
	305,		"コドラ",
	306,		"ボスゴドラ",
	307,		"アサナン",
	308,		"チャーレム",
	309,		"ラクライ",
	310,		"ライボルト",
	311,		"プラスル",
	312,		"マイナン",
	313,		"バルビート",
	314,		"イルミーゼ",
	315,		"ロゼリア",
	316,		"ゴクリン",
	317,		"マルノーム",
	318,		"キバニア",
	319,		"サメハダー",
	320,		"ホエルコ",
	321,		"ホエルオー",
	322,		"ドンメル",
	323,		"バクーダ",
	324,		"コータス",
	325,		"バネブー",
	326,		"ブーピッグ",
	327,		"パッチール",
	328,		"ナックラー",
	329,		"ビブラーバ",
	330,		"フライゴン",
	331,		"サボネア",
	332,		"ノクタス",
	333,		"チルット",
	334,		"チルタリス",
	335,		"ザングース",
	336,		"ハブネーク",
	337,		"ルナトーン",
	338,		"ソルロック",
	339,		"ドジョッチ",
	340,		"ナマズン",
	341,		"ヘイガニ",
	342,		"シザリガー",
	343,		"ヤジロン",
	344,		"ネンドール",
	345,		"リリーラ",
	346,		"ユレイドル",
	347,		"アノプス",
	348,		"アーマルド",
	349,		"ヒンバス",
	350,		"ミロカロス",
	351,		"ポワルン",
	352,		"カクレオン",
	353,		"カゲボウズ",
	354,		"ジュペッタ",
	355,		"ヨマワル",
	356,		"サマヨール",
	357,		"トロピウス",
	358,		"チリーン",
	359,		"アブソル",
	360,		"ソーナノ",
	361,		"ユキワラシ",
	362,		"オニゴーリ",
	363,		"タマザラシ",
	364,		"トドグラー",
	365,		"トドゼルガ",
	366,		"パールル",
	367,		"ハンテール",
	368,		"サクラビス",
	369,		"ジーランス",
	370,		"ラブカス",
	371,		"タツベイ",
	372,		"コモルー",
	373,		"ボーマンダ",
	374,		"ダンバル",
	375,		"メタング",
	376,		"メタグロス",
	377,		"レジロック",
	378,		"レジアイス",
	379,		"レジスチル",
	380,		"ラティアス",
	381,		"ラティオス",
	382,		"カイオーガ",
	383,		"グラードン",
	384,		"レックウザ",
	385,		"ジラーチ",
	386,		"デオキシス",
	432,		"ナエトル",
	433,		"ハヤシガメ",
	434,		"ドダイトス",
	430,		"ヒコザル",
	435,		"モウカザル",
	436,		"ゴウカザル",
	431,		"ポッチャマ",
	437,		"ポッタイシ",
	438,		"エンペルト",
	429,		"ムックル",
	448,		"ムクバード",
	449,		"ムクホーク",
	461,		"ビッパ",
	462,		"ビーダル",
	480,		"コロボーシ",
	481,		"コロトック",
	472,		"コリンク",
	473,		"ルクシオ",
	474,		"レントラー",
	428,		"スボミー",
	421,		"ロズレイド",
	489,		"ズガイドス",
	490,		"ラムパルド",
	487,		"タテトプス",
	488,		"トリデプス",
	455,		"ミノムッチ",
	457,		"ミノマダム",
	456,		"ガーメイル",
	470,		"ミツハニー",
	471,		"ビークイン",
	477,		"パチリス",
	418,		"ブイゼル",
	440,		"フローゼル",
	482,		"チェリンボ",
	483,		"チェリム",
	458,		"カラナクシ",
	459,		"トリトドン",
	496,		"エテボース",
	450,		"フワンテ",
	451,		"フワライド",
	420,		"ミミロル",
	454,		"ミミロップ",
	500,		"ムウマージ",
	494,		"ドンカラス",
	426,		"ニャルマー",
	463,		"ブニャット",
	445,		"リーシャン",
	468,		"スカンプー",
	469,		"スカタンク",
	478,		"ドーミラー",
	479,		"ドータクン",
	413,		"ウソハチ",
	414,		"マネネ",
	444,		"ピンプク",
	419,		"ペラップ",
	460,		"ミカルゲ",
	491,		"フカマル",
	492,		"ガバイト",
	493,		"ガブリアス",
	412,		"ゴンベ",
	439,		"リオル",
	416,		"ルカリオ",
	464,		"ヒポポタス",
	465,		"カバルドン",
	485,		"スコルピ",
	486,		"ドラピオン",
	452,		"グレッグル",
	453,		"ドクロッグ",
	484,		"マスキッパ",
	466,		"ケイコウオ",
	467,		"ネオラント",
	417,		"タマンタ",
	475,		"ユキカブリ",
	476,		"ユキノオー",
	415,		"マニューラ",
	443,		"ジバコイル",
	501,		"ベロベルト",
	446,		"ドサイドン",
	422,		"モジャンボ",
	427,		"エレキブル",
	502,		"ブーバーン",
	495,		"トゲキッス",
	499,		"メガヤンマ",
	505,		"リーフィア",
	506,		"グレイシア",
	447,		"グライオン",
	424,		"マンムー",
	504,		"ポリゴンＺ",
	498,		"エルレイド",
	497,		"ダイノーズ",
	423,		"ヨノワール",
	425,		"ユキメノコ",
	519,		"ロトム",
	511,		"ユクシー",
	512,		"エムリット",
	513,		"アグノム",
	514,		"ディアルガ",
	515,		"パルキア",
	508,		"ヒードラン",
	510,		"レジギガス",
	509,		"ギラティナ",
	507,		"クレセリア",
	441,		"フィオネ",
	442,		"マナフィ",
	517,		"ダークライ",
	516,		"シェイミ",
	518,		"アルセウス",
};



	/// <summary>
	/// Form1 の概要
	///
	/// 警告: このクラスの名前を変更する場合、このクラスが依存するすべての .resx ファイルに関連付けられた
	///          マネージ リソース コンパイラ ツールに対して 'Resource File Name' プロパティを
	///          変更する必要があります。この変更を行わないと、
	///          デザイナと、このフォームに関連付けられたローカライズ済みリソースとが、
	///          正しく相互に利用できなくなります。
	/// </summary>
	public ref class MainForm : public System::Windows::Forms::Form
	{
	public:
		MainForm(void)
		{
			InitializeComponent();
			//
			//TODO: ここにコンストラクタ コードを追加します
			//

     //   button1->Text = "これはボタン";
	//	button1->Dock = DockStyle::Top;
  //      button1->Click += new EventHandler(button1_Click);
		webBrowser1->Dock = DockStyle::Fill;
    //    Controls->Add(webBrowser1);
      //  Controls->Add(button1);
//        Load += new EventHandler(Form1_Load);

		}

	protected:
		/// <summary>
		/// 使用中のリソースをすべてクリーンアップします。
		/// </summary>
		~MainForm()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::WebBrowser^  webBrowser1;
	private: System::Windows::Forms::StatusStrip^  statusStrip1;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  fileFToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  sToolStripMenuItem;
	public: System::Windows::Forms::ToolStripStatusLabel^  StripStatusLabel;
	private: System::Windows::Forms::Timer^  timer;
	private: System::Windows::Forms::ToolStripMenuItem^  sendDataDToolStripMenuItem;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	public: System::Windows::Forms::ToolStripStatusLabel^  StripStatusLabelCenter;

	private: System::ComponentModel::IContainer^  components;
	public: System::ComponentModel::BackgroundWorker^ cmBackgroundWorker;
	private: System::Windows::Forms::ToolStripMenuItem^  gTSTestGToolStripMenuItem;
	public: 
	private: System::Threading::Thread^ threadA;

	static const int POKMEON_BOX_NUM = 18;  //BOX18こ
	static const int POKMEON_BOX_POKENUM = 30; //ポケモン３０体
	int dispBoxNo;
	static int targetPoke;   // 今選択しているポケモンNo
	static const unsigned long long TEST_KEY = 0x100000000UL;
	static const int POKEMON_MAX = 494;
	bool bBoxListRecv;

	private: System::Windows::Forms::ToolStripMenuItem^  gTSResetToolStripMenuItem;
	private: System::Windows::Forms::ToolStripContainer^  toolStripContainer1;
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::ToolStripMenuItem^  pokemonToolStripMenuItem;
	private: System::Windows::Forms::SplitContainer^  splitContainer1;
	private: System::Windows::Forms::Button^  button3;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::ToolTip^  toolTip1;
	private: System::Windows::Forms::ToolStripMenuItem^  exitEToolStripMenuItem;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  sendGTSSToolStripMenuItem;









	private: System::Windows::Forms::ToolStripMenuItem^  dSGTSSyncTToolStripMenuItem;




	protected: 

	private:
		/// <summary>
		/// 必要なデザイナ変数です。
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// デザイナ サポートに必要なメソッドです。このメソッドの内容を
		/// コード エディタで変更しないでください。
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->webBrowser1 = (gcnew System::Windows::Forms::WebBrowser());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->StripStatusLabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->StripStatusLabelCenter = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileFToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->sToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->sendDataDToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->gTSTestGToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->gTSResetToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->dSGTSSyncTToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->pokemonToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitEToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->timer = (gcnew System::Windows::Forms::Timer(this->components));
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->sendGTSSToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip1->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->toolStripContainer1->BottomToolStripPanel->SuspendLayout();
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->TopToolStripPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->contextMenuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// webBrowser1
			// 
			this->webBrowser1->Location = System::Drawing::Point(176, 198);
			this->webBrowser1->MinimumSize = System::Drawing::Size(20, 20);
			this->webBrowser1->Name = L"webBrowser1";
			this->webBrowser1->Size = System::Drawing::Size(115, 77);
			this->webBrowser1->TabIndex = 0;
			this->webBrowser1->DocumentCompleted += gcnew System::Windows::Forms::WebBrowserDocumentCompletedEventHandler(this, &MainForm::webBrowser1_DocumentCompleted);
			// 
			// statusStrip1
			// 
			this->statusStrip1->Dock = System::Windows::Forms::DockStyle::None;
			this->statusStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->StripStatusLabel, 
				this->StripStatusLabelCenter});
			this->statusStrip1->Location = System::Drawing::Point(0, 0);
			this->statusStrip1->Name = L"statusStrip1";
			this->statusStrip1->Size = System::Drawing::Size(638, 22);
			this->statusStrip1->TabIndex = 1;
			this->statusStrip1->Text = L"statusStrip1";
			// 
			// StripStatusLabel
			// 
			this->StripStatusLabel->Name = L"StripStatusLabel";
			this->StripStatusLabel->Size = System::Drawing::Size(11, 17);
			this->StripStatusLabel->Text = L"1";
			// 
			// StripStatusLabelCenter
			// 
			this->StripStatusLabelCenter->Name = L"StripStatusLabelCenter";
			this->StripStatusLabelCenter->Size = System::Drawing::Size(11, 17);
			this->StripStatusLabelCenter->Text = L"2";
			// 
			// menuStrip1
			// 
			this->menuStrip1->Dock = System::Windows::Forms::DockStyle::None;
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->fileFToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(638, 24);
			this->menuStrip1->TabIndex = 2;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileFToolStripMenuItem
			// 
			this->fileFToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(7) {this->sToolStripMenuItem, 
				this->sendDataDToolStripMenuItem, this->gTSTestGToolStripMenuItem, this->gTSResetToolStripMenuItem, this->dSGTSSyncTToolStripMenuItem, 
				this->pokemonToolStripMenuItem, this->exitEToolStripMenuItem});
			this->fileFToolStripMenuItem->Name = L"fileFToolStripMenuItem";
			this->fileFToolStripMenuItem->Size = System::Drawing::Size(51, 20);
			this->fileFToolStripMenuItem->Text = L"File(&F)";
			// 
			// sToolStripMenuItem
			// 
			this->sToolStripMenuItem->Name = L"sToolStripMenuItem";
			this->sToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->sToolStripMenuItem->Text = L"DSと接続(&S)";
			this->sToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::sToolStripMenuItem_Click);
			// 
			// sendDataDToolStripMenuItem
			// 
			this->sendDataDToolStripMenuItem->Name = L"sendDataDToolStripMenuItem";
			this->sendDataDToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->sendDataDToolStripMenuItem->Text = L"SendData(&D)";
			this->sendDataDToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::sendDataDToolStripMenuItem_Click);
			// 
			// gTSTestGToolStripMenuItem
			// 
			this->gTSTestGToolStripMenuItem->Name = L"gTSTestGToolStripMenuItem";
			this->gTSTestGToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->gTSTestGToolStripMenuItem->Text = L"GTSTest(&G)";
			this->gTSTestGToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::gTSTestGToolStripMenuItem_Click);
			// 
			// gTSResetToolStripMenuItem
			// 
			this->gTSResetToolStripMenuItem->Name = L"gTSResetToolStripMenuItem";
			this->gTSResetToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->gTSResetToolStripMenuItem->Text = L"GTSReset(&R)";
			this->gTSResetToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::gTSResetToolStripMenuItem_Click);
			// 
			// dSGTSSyncTToolStripMenuItem
			// 
			this->dSGTSSyncTToolStripMenuItem->Name = L"dSGTSSyncTToolStripMenuItem";
			this->dSGTSSyncTToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->dSGTSSyncTToolStripMenuItem->Text = L"DS-GTSSync(&T)";
			this->dSGTSSyncTToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::dSGTSSyncTToolStripMenuItem_Click);
			// 
			// pokemonToolStripMenuItem
			// 
			this->pokemonToolStripMenuItem->Name = L"pokemonToolStripMenuItem";
			this->pokemonToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->pokemonToolStripMenuItem->Text = L"PutPokemon(&P)";
			this->pokemonToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::pokemonToolStripMenuItem_Click);
			// 
			// exitEToolStripMenuItem
			// 
			this->exitEToolStripMenuItem->Name = L"exitEToolStripMenuItem";
			this->exitEToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->exitEToolStripMenuItem->Text = L"Exit(&E)";
			this->exitEToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::exitEToolStripMenuItem_Click);
			// 
			// timer
			// 
			this->timer->Interval = 8;
			this->timer->Tick += gcnew System::EventHandler(this, &MainForm::timer_Tick);
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			// 
			// toolStripContainer1
			// 
			// 
			// toolStripContainer1.BottomToolStripPanel
			// 
			this->toolStripContainer1->BottomToolStripPanel->Controls->Add(this->statusStrip1);
			// 
			// toolStripContainer1.ContentPanel
			// 
			this->toolStripContainer1->ContentPanel->AutoScroll = true;
			this->toolStripContainer1->ContentPanel->Controls->Add(this->splitContainer1);
			this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(638, 452);
			this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->toolStripContainer1->Location = System::Drawing::Point(0, 0);
			this->toolStripContainer1->Name = L"toolStripContainer1";
			this->toolStripContainer1->Size = System::Drawing::Size(638, 498);
			this->toolStripContainer1->TabIndex = 3;
			this->toolStripContainer1->Text = L"toolStripContainer1";
			// 
			// toolStripContainer1.TopToolStripPanel
			// 
			this->toolStripContainer1->TopToolStripPanel->Controls->Add(this->menuStrip1);
			// 
			// splitContainer1
			// 
			this->splitContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->splitContainer1->Location = System::Drawing::Point(0, 0);
			this->splitContainer1->Name = L"splitContainer1";
			this->splitContainer1->Orientation = System::Windows::Forms::Orientation::Horizontal;
			// 
			// splitContainer1.Panel1
			// 
			this->splitContainer1->Panel1->Controls->Add(this->button3);
			this->splitContainer1->Panel1->Controls->Add(this->button2);
			this->splitContainer1->Panel1->Controls->Add(this->button1);
			// 
			// splitContainer1.Panel2
			// 
			this->splitContainer1->Panel2->Controls->Add(this->webBrowser1);
			this->splitContainer1->Panel2->Controls->Add(this->pictureBox1);
			this->splitContainer1->Size = System::Drawing::Size(638, 452);
			this->splitContainer1->SplitterDistance = 91;
			this->splitContainer1->TabIndex = 2;
			// 
			// button3
			// 
			this->button3->Dock = System::Windows::Forms::DockStyle::Fill;
			this->button3->Location = System::Drawing::Point(526, 0);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(112, 91);
			this->button3->TabIndex = 2;
			this->button3->Text = L">>";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &MainForm::button3_Click);
			// 
			// button2
			// 
			this->button2->Dock = System::Windows::Forms::DockStyle::Left;
			this->button2->Location = System::Drawing::Point(68, 0);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(458, 91);
			this->button2->TabIndex = 1;
			this->button2->Text = L"ボックス１";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &MainForm::button2_Click);
			// 
			// button1
			// 
			this->button1->Dock = System::Windows::Forms::DockStyle::Left;
			this->button1->Location = System::Drawing::Point(0, 0);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(68, 91);
			this->button1->TabIndex = 0;
			this->button1->Text = L"<<";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &MainForm::button1_Click);
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(41, 47);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(250, 83);
			this->pictureBox1->TabIndex = 1;
			this->pictureBox1->TabStop = false;
			this->toolTip1->SetToolTip(this->pictureBox1, L"ポケモン");
			this->pictureBox1->MouseMove += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox1_MouseMove);
			this->pictureBox1->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &MainForm::pictureBox1_DragDrop);
			this->pictureBox1->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox1_MouseClick);
			this->pictureBox1->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox1_MouseDown);
			this->pictureBox1->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &MainForm::pictureBox1_DragEnter);
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->sendGTSSToolStripMenuItem});
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(133, 26);
			// 
			// sendGTSSToolStripMenuItem
			// 
			this->sendGTSSToolStripMenuItem->Name = L"sendGTSSToolStripMenuItem";
			this->sendGTSSToolStripMenuItem->Size = System::Drawing::Size(132, 22);
			this->sendGTSSToolStripMenuItem->Text = L"SendGTS(&S)";
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(638, 498);
			this->Controls->Add(this->toolStripContainer1);
			this->MainMenuStrip = this->menuStrip1;
			this->Name = L"MainForm";
			this->Text = L"PC-IRC-DS";
			this->Load += gcnew System::EventHandler(this, &MainForm::Form1_Load);
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &MainForm::MainForm_FormClosing);
			this->statusStrip1->ResumeLayout(false);
			this->statusStrip1->PerformLayout();
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->toolStripContainer1->BottomToolStripPanel->ResumeLayout(false);
			this->toolStripContainer1->BottomToolStripPanel->PerformLayout();
			this->toolStripContainer1->ContentPanel->ResumeLayout(false);
			this->toolStripContainer1->TopToolStripPanel->ResumeLayout(false);
			this->toolStripContainer1->TopToolStripPanel->PerformLayout();
			this->toolStripContainer1->ResumeLayout(false);
			this->toolStripContainer1->PerformLayout();
			this->splitContainer1->Panel1->ResumeLayout(false);
			this->splitContainer1->Panel2->ResumeLayout(false);
			this->splitContainer1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->contextMenuStrip1->ResumeLayout(false);
			this->ResumeLayout(false);

		}
#pragma endregion
private: System::Void sToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e);
private: System::Void timer_Tick(System::Object^  sender, System::EventArgs^  e);
private: System::Void MainForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e);
private: System::Void sendDataDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void DoWork( System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e );
private: System::Void RunWorkerCompleted( System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e );
private: void ThreadWork(void);
private: void TestUploadDownload(int pid, String^ proxy);
private: static void SetProxy(String^ proxy);
private: static bool RequestCheckServerState(void);
private: static int WaitForAsync(void);
private: static bool RequestSetProfile(void);
private: static void RequestUpload(void);
private: static void SetTrData(Dpw_Tr_Data* upload_data, int pokeNo);
private: static void SetProfile(Dpw_Common_Profile* profile);
private: void TestDownload(int pid,  String^ proxy);
private: static bool RequestPickupTraded(void);
private: int getPokemonNumberFromThePlace(int x,int y);
private: int getBoxPositionFromThePlace(int x,int y);
private: void GetPokeBoxList(void);




private: System::Void gTSTestGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void gTSResetToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void webBrowser1_DocumentCompleted(System::Object^  sender, System::Windows::Forms::WebBrowserDocumentCompletedEventArgs^  e) {
		 }
private: System::Void dSGTSSyncTToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

public: void CallProg(String^ message);

private: System::Void pokemonToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void pictureBox1_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pictureBox1_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {



		 }
private: System::Void pictureBox1_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {
			 if ( e->Data->GetDataPresent(DataFormats::FileDrop) ){
					 e->Effect = DragDropEffects::All;
				}
		 }

private: void pokemonListDisp(int boxNo);
private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void exitEToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void pictureBox1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pictureBox1_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);

private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}

