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


	static const unsigned long long TEST_KEY = 0x100000000UL;
	private: System::Windows::Forms::ToolStripMenuItem^  gTSResetToolStripMenuItem;
	private: System::Windows::Forms::ToolStripContainer^  toolStripContainer1;
	private: System::Windows::Forms::PictureBox^  pictureBox1;
	private: System::Windows::Forms::ToolStripMenuItem^  pokemonToolStripMenuItem;



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
			this->timer = (gcnew System::Windows::Forms::Timer(this->components));
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->toolStripContainer1 = (gcnew System::Windows::Forms::ToolStripContainer());
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->pokemonToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->statusStrip1->SuspendLayout();
			this->menuStrip1->SuspendLayout();
			this->toolStripContainer1->BottomToolStripPanel->SuspendLayout();
			this->toolStripContainer1->ContentPanel->SuspendLayout();
			this->toolStripContainer1->TopToolStripPanel->SuspendLayout();
			this->toolStripContainer1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// webBrowser1
			// 
			this->webBrowser1->Location = System::Drawing::Point(21, 15);
			this->webBrowser1->MinimumSize = System::Drawing::Size(20, 20);
			this->webBrowser1->Name = L"webBrowser1";
			this->webBrowser1->Size = System::Drawing::Size(279, 212);
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
			this->statusStrip1->Size = System::Drawing::Size(576, 22);
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
			this->menuStrip1->Size = System::Drawing::Size(576, 24);
			this->menuStrip1->TabIndex = 2;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// fileFToolStripMenuItem
			// 
			this->fileFToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(6) {this->sToolStripMenuItem, 
				this->sendDataDToolStripMenuItem, this->gTSTestGToolStripMenuItem, this->gTSResetToolStripMenuItem, this->dSGTSSyncTToolStripMenuItem, 
				this->pokemonToolStripMenuItem});
			this->fileFToolStripMenuItem->Name = L"fileFToolStripMenuItem";
			this->fileFToolStripMenuItem->Size = System::Drawing::Size(51, 20);
			this->fileFToolStripMenuItem->Text = L"File(&F)";
			// 
			// sToolStripMenuItem
			// 
			this->sToolStripMenuItem->Name = L"sToolStripMenuItem";
			this->sToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->sToolStripMenuItem->Text = L"Sync(&S)";
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
			this->toolStripContainer1->ContentPanel->Controls->Add(this->pictureBox1);
			this->toolStripContainer1->ContentPanel->Controls->Add(this->webBrowser1);
			this->toolStripContainer1->ContentPanel->Size = System::Drawing::Size(576, 308);
			this->toolStripContainer1->Dock = System::Windows::Forms::DockStyle::Fill;
			this->toolStripContainer1->Location = System::Drawing::Point(0, 0);
			this->toolStripContainer1->Name = L"toolStripContainer1";
			this->toolStripContainer1->Size = System::Drawing::Size(576, 354);
			this->toolStripContainer1->TabIndex = 3;
			this->toolStripContainer1->Text = L"toolStripContainer1";
			// 
			// toolStripContainer1.TopToolStripPanel
			// 
			this->toolStripContainer1->TopToolStripPanel->Controls->Add(this->menuStrip1);
			// 
			// pictureBox1
			// 
			this->pictureBox1->Location = System::Drawing::Point(306, 57);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(250, 187);
			this->pictureBox1->TabIndex = 1;
			this->pictureBox1->TabStop = false;
			// 
			// pokemonToolStripMenuItem
			// 
			this->pokemonToolStripMenuItem->Name = L"pokemonToolStripMenuItem";
			this->pokemonToolStripMenuItem->Size = System::Drawing::Size(153, 22);
			this->pokemonToolStripMenuItem->Text = L"PutPokemon(&P)";
			this->pokemonToolStripMenuItem->Click += gcnew System::EventHandler(this, &MainForm::pokemonToolStripMenuItem_Click);
			// 
			// MainForm
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 12);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(576, 354);
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
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
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
private: void SetProxy(String^ proxy);
private: bool RequestCheckServerState(void);
private: int WaitForAsync(void);
private: bool RequestSetProfile(void);
private: bool RequestUpload(void);
private: void SetTrData(Dpw_Tr_Data* upload_data);
private: void SetProfile(Dpw_Common_Profile* profile);
private: void TestDownload(int pid,  String^ proxy);
private: bool RequestPickupTraded(void);

private: System::Void gTSTestGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void gTSResetToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

private: System::Void webBrowser1_DocumentCompleted(System::Object^  sender, System::Windows::Forms::WebBrowserDocumentCompletedEventArgs^  e) {
		 }
private: System::Void dSGTSSyncTToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

public: void CallProg(String^ message);

private: System::Void pokemonToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e);

};
}

