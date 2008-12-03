#pragma once

#include "dpw_bt.h"
#include "dpw_tr.h"
#include < stdio.h >
#include < stdlib.h >
#include < vcclr.h >
#include "pokemontbl.h"

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

	public enum FORM_DISPMODE_ENUM : int {
		FORM_DISPMODE_WEB = 0,
		FORM_DISPMODE_BOX,
		FORM_DISPMODE_SOUND,
		FORM_DISPMODE_MAX
	};



	static const char PROGRAM_NAME[] = "Global Pokemon FunClub";


	typedef struct {
		short pokeno;
		short pokelv;
		short pokesex;
	} PokemonData;


	// ボックスリスト
	typedef struct {
		PokemonData poke[BOX_MAX_POS];
	    unsigned short trayName[BOX_TRAYNAME_BUFSIZE+2];
	} BoxTrayData;




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
	private: System::Windows::Forms::ToolStripStatusLabel^  StripStatusLabel;
	private: System::Windows::Forms::Timer^  timer;
	private: System::Windows::Forms::ToolStripMenuItem^  sendDataDToolStripMenuItem;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;
	private: System::Windows::Forms::ToolStripStatusLabel^  StripStatusLabelCenter;

	private: System::ComponentModel::IContainer^  components;
	public: System::ComponentModel::BackgroundWorker^ cmBackgroundWorker;
	private: System::Windows::Forms::ToolStripMenuItem^  gTSTestGToolStripMenuItem;
	public: 
	private: System::Threading::Thread^ threadA;
	private: System::Threading::Thread^ threadB;

	static const int POKMEON_BOX_NUM = 18;  //BOX18こ
	static const int POKMEON_BOX_POKENUM = 30; //ポケモン３０体
	int dispBoxNo;
	int dispBoxPoke;
	int mouseBoxPoke;  // マウスが指しているポケモンindex

	int handBoxNo;     // つかんでいるポケモンボックス番号
	int handBoxPoke;   // つかんでいるポケモンindex

	static int sendBoxNo;
	static int sendBoxPoke;
	static int targetPoke;   // 今選択しているポケモンNo
	static int sendPokeLv;
	static int sendPokeLvMax;
	static int searchPokeIndex;
	static int SoundBarTotalSize;
	static int SoundBarSize;

	static int sendPokeSex;
	static const unsigned long long TEST_KEY = 0x100000000UL;
	static const int POKEMON_MAX = 494;
	bool bBoxListRecv;
	static int DownloadMatchNum;  // 落としてきた数
	int GTSDispMode;   //ＧＴＳの画面遷移状態
	String^ BackupToolTipMsg;
	int BackupPosNo;
	static int POKEBOX_OFFSETX;
	static int POKEBOX_OFFSETY;

	static const int MODE_NONE = 0;
	static const int MODE_SEARCHPOKE = 1;
	static const int MODE_BOXDISP = 2;
	static const int MODE_SEARCHPOKE_BOXDISP = 3;



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
	private: System::Windows::Forms::PictureBox^  pictureBox2;
	private: System::Windows::Forms::PictureBox^  pictureBox3;
	private: System::Windows::Forms::ToolStripMenuItem^  handHToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  infoIToolStripMenuItem;
	private: System::Windows::Forms::PictureBox^  pictureBox4;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::PictureBox^  pictureBox5;
private: System::Windows::Forms::ToolStripMenuItem^  debugToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  pokemonDisukiClubDToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  globalPokemonFanClubGToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  disConnectDToolStripMenuItem;
private: System::Windows::Forms::ToolStripMenuItem^  soundStopSToolStripMenuItem;



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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(MainForm::typeid));
			this->webBrowser1 = (gcnew System::Windows::Forms::WebBrowser());
			this->statusStrip1 = (gcnew System::Windows::Forms::StatusStrip());
			this->StripStatusLabel = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->StripStatusLabelCenter = (gcnew System::Windows::Forms::ToolStripStatusLabel());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->fileFToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->sToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->disConnectDToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->sendDataDToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->gTSTestGToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->gTSResetToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->dSGTSSyncTToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->pokemonToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitEToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->debugToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->pokemonDisukiClubDToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->globalPokemonFanClubGToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->timer = (gcnew System::Windows::Forms::Timer(this->components));
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->splitContainer1 = (gcnew System::Windows::Forms::SplitContainer());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->pictureBox5 = (gcnew System::Windows::Forms::PictureBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->pictureBox4 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox3 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox2 = (gcnew System::Windows::Forms::PictureBox());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->toolTip1 = (gcnew System::Windows::Forms::ToolTip(this->components));
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->handHToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->infoIToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->sendGTSSToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->soundStopSToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip1->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->toolStripContainer1->BottomToolStripPanel->SuspendLayout();
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->TopToolStripPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			this->splitContainer1->Panel1->SuspendLayout();
			this->splitContainer1->Panel2->SuspendLayout();
			this->splitContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox5))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox4))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox3))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->contextMenuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// webBrowser1
			// 
			this->webBrowser1->Location = System::Drawing::Point(41, 151);
			this->webBrowser1->MinimumSize = System::Drawing::Size(20, 20);
			this->webBrowser1->Name = L"webBrowser1";
			this->webBrowser1->Size = System::Drawing::Size(115, 77);
			this->webBrowser1->TabIndex = 0;
			this->webBrowser1->NewWindow += gcnew System::ComponentModel::CancelEventHandler(this, &MainForm::webBrowser1_NewWindow);
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
			this->StripStatusLabelCenter->Size = System::Drawing::Size(0, 17);
			// 
			// menuStrip1
			// 
			this->menuStrip1->Dock = System::Windows::Forms::DockStyle::None;
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->fileFToolStripMenuItem, 
				this->debugToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(638, 24);
			this->menuStrip1->TabIndex = 2;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileFToolStripMenuItem
			// 
			this->fileFToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(8) {this->sToolStripMenuItem, 
				this->disConnectDToolStripMenuItem, this->sendDataDToolStripMenuItem, this->gTSTestGToolStripMenuItem, this->gTSResetToolStripMenuItem, 
				this->dSGTSSyncTToolStripMenuItem, this->pokemonToolStripMenuItem, this->exitEToolStripMenuItem});
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
			// disConnectDToolStripMenuItem
			// 
			this->disConnectDToolStripMenuItem->Name = L"disConnectDToolStripMenuItem";
			this->disConnectDToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->disConnectDToolStripMenuItem->Text = L"DisConnect(&D)";
			this->disConnectDToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::disConnectDToolStripMenuItem_Click);
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
			this->gTSTestGToolStripMenuItem->Text = L"トップに戻る(&T)";
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
			// debugToolStripMenuItem
			// 
			this->debugToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->pokemonDisukiClubDToolStripMenuItem, 
				this->globalPokemonFanClubGToolStripMenuItem, this->soundStopSToolStripMenuItem});
			this->debugToolStripMenuItem->Name = L"debugToolStripMenuItem";
			this->debugToolStripMenuItem->Size = System::Drawing::Size(65, 20);
			this->debugToolStripMenuItem->Text = L"Debug(&D)";
			// 
			// pokemonDisukiClubDToolStripMenuItem
			// 
			this->pokemonDisukiClubDToolStripMenuItem->Name = L"pokemonDisukiClubDToolStripMenuItem";
			this->pokemonDisukiClubDToolStripMenuItem->Size = System::Drawing::Size(206, 22);
			this->pokemonDisukiClubDToolStripMenuItem->Text = L"PokemonDaisukiClub(&D)";
			this->pokemonDisukiClubDToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::pokemonDisukiClubDToolStripMenuItem_Click);
			// 
			// globalPokemonFanClubGToolStripMenuItem
			// 
			this->globalPokemonFanClubGToolStripMenuItem->Name = L"globalPokemonFanClubGToolStripMenuItem";
			this->globalPokemonFanClubGToolStripMenuItem->Size = System::Drawing::Size(206, 22);
			this->globalPokemonFanClubGToolStripMenuItem->Text = L"GlobalPokemonFanClub(&G)";
			this->globalPokemonFanClubGToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::globalPokemonFanClubGToolStripMenuItem_Click);
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
			this->splitContainer1->Panel2->BackColor = System::Drawing::Color::White;
			this->splitContainer1->Panel2->Controls->Add(this->pictureBox5);
			this->splitContainer1->Panel2->Controls->Add(this->label1);
			this->splitContainer1->Panel2->Controls->Add(this->pictureBox4);
			this->splitContainer1->Panel2->Controls->Add(this->pictureBox3);
			this->splitContainer1->Panel2->Controls->Add(this->pictureBox2);
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
			// pictureBox5
			// 
			this->pictureBox5->Cursor = System::Windows::Forms::Cursors::Hand;
			this->pictureBox5->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox5.Image")));
			this->pictureBox5->Location = System::Drawing::Point(227, 18);
			this->pictureBox5->Name = L"pictureBox5";
			this->pictureBox5->Size = System::Drawing::Size(244, 48);
			this->pictureBox5->SizeMode = System::Windows::Forms::PictureBoxSizeMode::AutoSize;
			this->pictureBox5->TabIndex = 6;
			this->pictureBox5->TabStop = false;
			this->pictureBox5->MouseLeave += gcnew System::EventHandler(this, &MainForm::pictureBox5_MouseLeave);
			this->pictureBox5->Click += gcnew System::EventHandler(this, &MainForm::pictureBox5_Click);
			this->pictureBox5->MouseEnter += gcnew System::EventHandler(this, &MainForm::pictureBox5_MouseEnter);
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Font = (gcnew System::Drawing::Font(L"ＭＳ Ｐゴシック", 14.25F, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(128)));
			this->label1->Location = System::Drawing::Point(135, 298);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(0, 19);
			this->label1->TabIndex = 5;
			// 
			// pictureBox4
			// 
			this->pictureBox4->BackColor = System::Drawing::Color::Transparent;
			this->pictureBox4->Location = System::Drawing::Point(40, 255);
			this->pictureBox4->Name = L"pictureBox4";
			this->pictureBox4->Size = System::Drawing::Size(37, 30);
			this->pictureBox4->TabIndex = 4;
			this->pictureBox4->TabStop = false;
			this->pictureBox4->MouseDown += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox4_MouseDown);
			// 
			// pictureBox3
			// 
			this->pictureBox3->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox3.Image")));
			this->pictureBox3->Location = System::Drawing::Point(317, 61);
			this->pictureBox3->Name = L"pictureBox3";
			this->pictureBox3->Size = System::Drawing::Size(237, 32);
			this->pictureBox3->TabIndex = 3;
			this->pictureBox3->TabStop = false;
			this->pictureBox3->DragDrop += gcnew System::Windows::Forms::DragEventHandler(this, &MainForm::pictureBox3_DragDrop);
			this->pictureBox3->DragEnter += gcnew System::Windows::Forms::DragEventHandler(this, &MainForm::pictureBox3_DragEnter);
			// 
			// pictureBox2
			// 
			this->pictureBox2->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox2.Image")));
			this->pictureBox2->Location = System::Drawing::Point(268, 151);
			this->pictureBox2->Name = L"pictureBox2";
			this->pictureBox2->Size = System::Drawing::Size(258, 148);
			this->pictureBox2->SizeMode = System::Windows::Forms::PictureBoxSizeMode::CenterImage;
			this->pictureBox2->TabIndex = 2;
			this->pictureBox2->TabStop = false;
			this->pictureBox2->MouseClick += gcnew System::Windows::Forms::MouseEventHandler(this, &MainForm::pictureBox2_MouseClick);
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
			this->contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(3) {this->handHToolStripMenuItem, 
				this->infoIToolStripMenuItem, this->sendGTSSToolStripMenuItem});
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(161, 70);
			this->contextMenuStrip1->Opened += gcnew System::EventHandler(this, &MainForm::contextMenuStrip1_Opened);
			// 
			// handHToolStripMenuItem
			// 
			this->handHToolStripMenuItem->Name = L"handHToolStripMenuItem";
			this->handHToolStripMenuItem->Size = System::Drawing::Size(160, 22);
			this->handHToolStripMenuItem->Text = L"つかむ(&H)";
			this->handHToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::handHToolStripMenuItem_Click);
			// 
			// infoIToolStripMenuItem
			// 
			this->infoIToolStripMenuItem->Name = L"infoIToolStripMenuItem";
			this->infoIToolStripMenuItem->Size = System::Drawing::Size(160, 22);
			this->infoIToolStripMenuItem->Text = L"ようすをみる(&I)";
			this->infoIToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::infoIToolStripMenuItem_Click);
			// 
			// sendGTSSToolStripMenuItem
			// 
			this->sendGTSSToolStripMenuItem->Name = L"sendGTSSToolStripMenuItem";
			this->sendGTSSToolStripMenuItem->Size = System::Drawing::Size(160, 22);
			this->sendGTSSToolStripMenuItem->Text = L"ＧＰＦへあずける(&S)";
			// 
			// soundStopSToolStripMenuItem
			// 
			this->soundStopSToolStripMenuItem->Name = L"soundStopSToolStripMenuItem";
			this->soundStopSToolStripMenuItem->Size = System::Drawing::Size(206, 22);
			this->soundStopSToolStripMenuItem->Text = L"SoundStop(&S)";
			this->soundStopSToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::soundStopSToolStripMenuItem_Click);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(638, 498);
			this->Controls->Add(this->toolStripContainer1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->MainMenuStrip = this->menuStrip1;
			this->Name = L"MainForm";
			this->Text = L"GlobalPokemonFanClub";
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
			this->splitContainer1->Panel2->PerformLayout();
			this->splitContainer1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox5))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox4))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox3))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox2))->EndInit();
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
private: void pokemonMouse(int pokeNo);
private: void TestUploadDownload(int pid, String^ proxy);
private: static void SetProxy(String^ proxy);
private: static bool RequestCheckServerState(void);
private: static int WaitForAsync(void);
private: static bool RequestSetProfile(void);
private: void RequestUpload(void);
private: static void SetTrData(Dpw_Tr_Data* upload_data, int pokeNo);
private: static void SetProfile(Dpw_Common_Profile* profile);
private: void TestDownload(int pid,  String^ proxy);
private: static bool RequestPickupTraded(void);
private: int getPokemonNumberFromThePlace(int x,int y);
private: int getPokemonLvFromThePlace(int x,int y);
private: int getPokemonSexFromThePlace(int x,int y);
private: int getBoxPositionFromThePlace(int x,int y);
private: void GetPokeBoxList(void);
private: void connectIRC(void);
private: void RequestSearch(void);
private: void pokemonSearchDisp(void);
private: int PokemonName2No(String^ name);
private: static void RequestChange(void);
public: void Draw(void);
private: void FormDispChange(int mode);





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
private: System::Void pictureBox2_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void pictureBox3_DragEnter(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e) {

			 if ( e->Data->GetDataPresent(DataFormats::FileDrop) ){
					 e->Effect = DragDropEffects::All;
				}

		 }
private: System::Void pictureBox3_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e);
private: System::Void handHToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void infoIToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void pictureBox5_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void pictureBox5_MouseEnter(System::Object^  sender, System::EventArgs^  e);
private: System::Void pictureBox5_MouseLeave(System::Object^  sender, System::EventArgs^  e);
private: System::Void pokemonDisukiClubDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void globalPokemonFanClubGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void webBrowser1_NewWindow(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e);
private: System::Void contextMenuStrip1_Opened(System::Object^  sender, System::EventArgs^  e);
private: System::Void pictureBox4_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e);
private: System::Void disConnectDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
private: System::Void soundStopSToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);
};
}

