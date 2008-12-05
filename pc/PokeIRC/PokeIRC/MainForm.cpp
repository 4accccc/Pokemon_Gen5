//=============================================================================
/**
 * @file	MainForm.cpp
 * @brief	メインのフォーム
 * @author	k.ohno
 * @date    2008.09.30
 */
//=============================================================================

//---------------------------------------------------------
// include 
//---------------------------------------------------------
#include "stdafx.h"
#include "MainForm.h"
#include "NetIRC.h"
#include "FileNGC.h"
#include "PokemonSearch.h"
#include "pokemontbl.h"

#include < stdio.h >
#include < stdlib.h >
#include < vcclr.h >



using namespace PokeIRC;
using namespace System::Windows::Forms;
using namespace System::Threading;

static BoxTrayData GPokeBoxList[BOX_MAX_TRAY];
static PokemonData handPoke; // 手にしているポケモン
static Dpw_Tr_Data match_data_buf[DPW_TR_DOWNLOADMATCHDATA_MAX];	// データの検索結果を入れるバッファ。

//--------------------------------------------------------------
/**
 * @breif   
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pokemonDisukiClubDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	webBrowser1->Url= gcnew Uri("http://www.pokemon.jp/portal/top.cfm");
}



System::Void MainForm::globalPokemonFanClubGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	webBrowser1->Url= gcnew Uri("http://wb.gamefreak.co.jp/ando/world/index.php");
}

//--------------------------------------------------------------
/**
 * @breif   赤外線接続
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::connectIRC(void)
{
	NetIRC::connect();
	if(threadA==nullptr){
		threadA = gcnew Thread(gcnew ThreadStart(this,&MainForm::ThreadWork)); //

		threadA->IsBackground = true; // バックグラウンド・スレッドとする
		threadA->Priority = ThreadPriority::Highest; //優先度を「最優先」にする

		threadA->Start(); // 
	}
}

//--------------------------------------------------------------
/**
 * @breif   ファイル->Syncメニューが選択された時の処理
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::sToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	connectIRC();

}

//--------------------------------------------------------------
/**
 * @breif   フォームがロードされた時の処理 初期化
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::Form1_Load(System::Object^  sender, System::EventArgs^  e)
{
	NetIRC::Init();

	webBrowser1->Url= gcnew Uri("http://wb.gamefreak.co.jp/ando/world/index.php");


	//ウェブブラウザとのリンクテスト


	webBrowser1->AllowWebBrowserDrop = false;
    webBrowser1->IsWebBrowserContextMenuEnabled = false;
    webBrowser1->WebBrowserShortcutsEnabled = false;

    webBrowser1->ObjectForScripting = this;
	//フォームをweb1にリンクさせている
	//
    // Uncomment the following line when you are finished debugging.
    //webBrowser1->ScriptErrorsSuppressed = true;
/*
    webBrowser1->DocumentText =
           "<html><head><script>" +
            "function test3(message) { alert(message); }" +
            "</script></head><body><button " +
            "onclick=\"window.external.CallProg('called from script code')\">" +
            "このボタンを押すとサウンド転送プログラムをよびます</button>" +
            "</body></html>";
*/
	FormDispChange(FORM_DISPMODE_WEB);

	pictureBox4->BackColor = Color::FromArgb(0, 0, 0, 0);
	pictureBox1->Controls->Add(pictureBox4);

	this->Width = 1070;
	this->Height = 770;

	POKEBOX_OFFSETX = 50;
	POKEBOX_OFFSETY = 105;

	GTSDispMode=0;
	SoundBarSize = pictureBox3->Width;
	SoundBarTotalSize = 0;

	splitContainer1->Panel1->Hide();
	handPoke.pokeno = 0;
	splitContainer1->SplitterDistance = 0;  //最初ボタンは見せない
	dispBoxNo=0;

	{
		String^ proxy = "";
		Dpw_Tr_Init(100000, TEST_KEY | 100000);
		SetProxy(proxy);
	}



}

//--------------------------------------------------------------
/**
 * @breif   タイマーイベント 1/60 のさらに半分で呼ばれる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::timer_Tick(System::Object^  sender, System::EventArgs^  e)
{
	//NetIRC::draw(this);
}

//--------------------------------------------------------------
/**
 * @breif   フォームが閉じる時に呼ばれる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::MainForm_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e)
{
	//timer->Stop();
//	if(threadA != nullptr){
//		threadA->Suspend();
//	}
	//threadA->Finalize();
	NetIRC::shutdown();

}

//--------------------------------------------------------------
/**
 * @breif   データを送信する
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::sendDataDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	int index = -1;
	System::IO::Stream^ myStream;
	BinaryReader^ userReader;   // ファイルを読み込む為のハンドル

	openFileDialog1->InitialDirectory = "C:\\home";
	openFileDialog1->Filter = "bin files (*.swav)|*.swav|All files (*.*)|*.*";
	openFileDialog1->FilterIndex = 1;
	openFileDialog1->RestoreDirectory = true;

	if ( openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
	{
		if ( (myStream = openFileDialog1->OpenFile()) != nullptr )
		{
			userReader = gcnew BinaryReader(myStream);
//			NetIRC::dataArray = userReader->ReadBytes((int)myStream->Length);
			NetIRC::sendData('A',userReader->ReadBytes((int)myStream->Length));
			myStream->Close();
		}
	}

}


System::Void MainForm::pictureBox3_DragDrop(System::Object^  sender, System::Windows::Forms::DragEventArgs^  e)
{
//	System::IO::Stream^ myStream;
	BinaryReader^ userReader;   // ファイルを読み込む為のハンドル
	array<String^>^ files = dynamic_cast<array<String^>^>(e->Data->GetData(DataFormats::FileDrop));
		 
	if(!NetIRC::IsConnect()){  // つながっていない場合受け付けない
		return;
	}
	for each(String^ filename in files){
		if( IO::Path::GetExtension(filename) == ".swav" ){
			FileStream^ fs = File::OpenRead( filename );
			userReader = gcnew BinaryReader(fs);
			SoundBarTotalSize = (int)fs->Length;

			Debug::WriteLine(DateTime::Now.ToString("F")+"start");
		
//			NetIRC::dataArray = userReader->ReadBytes((int)fs->Length);
			NetIRC::sendData(IRC_COMMAND_SNDV,userReader->ReadBytes((int)fs->Length));
			fs->Close();
			label1->Text = "音楽を送信しています";
			break;
		}
	}
}

//--------------------------------------------------------------
/**
 * @breif   サウンド転送の中断を送る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::soundStopSToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = IRC_COMMAND_SNDSTOP;
	NetIRC::ResetDataSend();
	NetIRC::sendData(IRC_COMMAND_SNDSTOP, localarray);
}


//--------------------------------------------------------------
/**
 * @breif   BackGroundWorkerのタスク処理
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::DoWork( System::Object^ sender, System::ComponentModel::DoWorkEventArgs^ e )
{
    // ここに、処理を書く
	//NetIRC::draw(this);
	//Thread::Sleep(1); //2msec待つ 
}

//--------------------------------------------------------------
/**
 * @breif   スレッドが終了した時に呼ばれる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::RunWorkerCompleted( System::Object^ sender, System::ComponentModel::RunWorkerCompletedEventArgs^ e )
{
    // スレッドが終了したときに呼ばれる
}

void MainForm::ThreadWork(void)
{
	while(1){
		NetIRC::draw(this);
		Thread::Sleep(1); //2msec待つ 
	}
}

// プロフィールをセットする
void MainForm::SetProfile(Dpw_Common_Profile* profile)
{
	memset(profile, 0, sizeof(*profile));
	profile->version = 12;
	profile->language = 1;
	profile->countryCode = 103;
	profile->localCode = 1;
	profile->playerId = 87654322;
	profile->playerName[0] = 0x64;      // サ
	profile->playerName[1] = 0x77;      // ト
	profile->playerName[2] = 0x62;      // シ
	profile->playerName[3] = 0xffff;    // 終端
	profile->flag = 0;
	strcpy(profile->mailAddr, "ohno_katsumi@gamefreak.co.jp");
	profile->mailAddrAuthPass = DPW_MAIL_ADDR_AUTH_DEBUG_PASSWORD;   // デバッグ用パスワードを使用する。
	profile->mailAddrAuthVerification = 456;
	profile->mailRecvFlag = DPW_PROFILE_MAILRECVFLAG_EXCHANGE;
}

// Dpw_Tr_Dataをセットする
bool MainForm::SetTrData(Dpw_Tr_Data* upload_data, int pokeNo)
{
	int i;
	memset(upload_data, 0, sizeof(Dpw_Tr_Data));

	Debug::WriteLine("こうかん性別"+sendPokeSex);

	upload_data->postSimple.characterNo = pokeNo;
	upload_data->postSimple.gender = sendPokeSex;
	upload_data->postSimple.level = sendPokeLv;

	PokemonSearch^ psForm = gcnew PokemonSearch();

	psForm->label1->Text="交換するポケモンを決めてね";
	psForm->button1->Text = "きめた";
		psForm->listBox1->SelectedIndex = 2;
	if(psForm->ShowDialog() == System::Windows::Forms::DialogResult::OK){
	}
	else{
		return false;
	}


	for(i=0;i < sizeof(upload_data->postData.data);i++){
		upload_data->postData.data[i] = NetIRC::recvdataArray[i+2];
	}

	upload_data->wantSimple.characterNo = PokemonName2No(psForm->textBox1->Text);
	upload_data->wantSimple.gender = psForm->listBox1->SelectedIndex + 1;
	upload_data->wantSimple.level_min = 0;//System::Decimal::ToInt32(psForm->numericUpDown2->Value);
	upload_data->wantSimple.level_max = 0;//System::Decimal::ToInt32(psForm->numericUpDown3->Value);
	upload_data->name[0] = 0x30;
	upload_data->name[1] = 0x31;
	upload_data->name[2] = 0xffff;
	upload_data->trainerID = 1 << 12;
	upload_data->countryCode = 103;
	upload_data->langCode = 1;
	upload_data->localCode = 1;
	upload_data->trainerType = 3;
	upload_data->versionCode = 12;
	upload_data->gender = 1;
	return true;
}

//--------------------------------------------------------------
/**
 * @breif   預ける
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void MainForm::RequestUpload(void)
{
	s32 result = 0;
	int pid = 100000;
	String^ proxy = "";


	NetIRC::sendLock();
	Sleep(100);

	{
		// 赤外線コマンド発行
		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = sendBoxNo;
		localarray[1] = sendBoxPoke;
		NetIRC::sendUnLock();
		NetIRC::sendData(IRC_COMMAND_BOXPOKE, localarray);
		NetIRC::sendLock();
	}

	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}
	Debug::WriteLine("ポケモン受け取り");

	SetProxy(proxy);
	if(!RequestCheckServerState()){
		Debug::WriteLine("サーバがおかしかった");
		_errDisp(0);
		return;
	}
	if(!RequestSetProfile()){
		Debug::WriteLine("プロファイル設定に失敗");
		_errDisp(0);
		return;
	}
	if(!RequestPickupTraded()){
		Debug::WriteLine("引取りに失敗");
		_errDisp(0);
		return;
	}

	{	// 預ける
		Dpw_Tr_Data data;
								 Debug::WriteLine(targetPoke);
		if(SetTrData(&data, targetPoke)==false){
			_errDisp(0);
			return;
		}
		Debug::WriteLine("預け開始");
		Sleep(1000);

		Dpw_Tr_UploadAsync(&data);
		result = WaitForAsync();
		if(result == 0 )
		{
			Debug::WriteLine("預け完了　確定へ");
			Sleep(10);
			// 預けるのを確定
			Dpw_Tr_UploadFinishAsync();
			result = WaitForAsync();
			if(result != 0 )
			{
				Debug::WriteLine("Failed to uploadFinish."+result);
			_errDisp(0);
				return;
			}
		}
		else if(result == DPW_TR_ERROR_ILLIGAL_REQUEST)
		{
			Debug::WriteLine("Already uploaded.");
			_errDisp(0);
			return;
		}
		else
		{
			Debug::WriteLine("Failed to upload.");
			_errDisp(0);
			return;
		}
	}
	//DS側を消す
	{
		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = sendBoxNo;
		localarray[1] = sendBoxPoke;
		NetIRC::sendUnLock();
		NetIRC::sendData(IRC_COMMAND_BOXPOKEDEL, localarray);
	}
	NetIRC::sendLock();

	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}

	GetPokeBoxList();


	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = sendBoxNo;
	NetIRC::sendData(IRC_COMMAND_BOXNO,localarray);

	MessageBox::Show("あずけました");

	NetIRC::sendUnLock();


}


void MainForm::_errDisp(int errno)
{
	MessageBox::Show("しっぱいしました");
	NetIRC::sendUnLock();
}

//--------------------------------------------------------------
/**
 * @breif   ポケモンを交換する
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void MainForm::RequestChange(void)
{
	s32 result = 0;
	int pid = 100000;
	String^ proxy = "";

	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}
	SetProxy(proxy);
	if(!RequestCheckServerState()){
		return;
	}
	if(!RequestSetProfile()){
		return;
	}
	if(!RequestPickupTraded()){
		Debug::WriteLine("引取りに失敗");
		return;
	}



	{
		// トレード
		Dpw_Tr_Data trade_data;
		Dpw_Tr_Data download_buf;
		int retNo;

		trade_data.wantSimple.characterNo = targetPoke;
		trade_data.wantSimple.level_max = sendPokeLvMax;
		trade_data.wantSimple.level_min = sendPokeLv;
		trade_data.wantSimple.gender = sendPokeSex;


		SetTrData(&trade_data, targetPoke);
		Dpw_Tr_TradeAsync(match_data_buf[searchPokeIndex].id, &trade_data, &download_buf);
		retNo = WaitForAsync();
		if(retNo < 0 ){
			Debug::WriteLine("Failed to trade." + retNo);
			return;
		}
		
		//DSに交換したポケモンを送る

		array<unsigned char>^ localarray = gcnew array<unsigned char>(2+sizeof(Dpw_Tr_PokemonData));
//		NetIRC::dataArray = gcnew array<unsigned char>(2+sizeof(Dpw_Tr_PokemonData));
		localarray[0] = sendBoxNo;
		localarray[1] = sendBoxPoke;

		pin_ptr<unsigned char> wptr = &localarray[2];
		memcpy(wptr, &download_buf.postData, sizeof(Dpw_Tr_PokemonData));

		NetIRC::sendData(IRC_COMMAND_CHANGEPOKE_SEND, localarray);

		Dpw_Tr_TradeFinishAsync();
		if(WaitForAsync() != 0 )
		{
			Debug::WriteLine("Failed to trade finish.");
			return ;
		}

	}


	MessageBox::Show("こうかんしました" + sendBoxNo + " " + sendBoxPoke);
	return;
}




//--------------------------------------------------------------
/**
 * @breif   交換されたものを引き取る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

bool MainForm::RequestPickupTraded(void)
{
	s32 result = 0;

	// 交換されたものを引き取る
	{
		Dpw_Tr_Data data;
		Dpw_Tr_GetUploadResultAsync(&data);

		result = WaitForAsync();
		if(result == 0 )
		{
			Debug::WriteLine("Not traded.");
			Dpw_Tr_ReturnAsync();
		}
		else if(result == 1)
		{
			Debug::WriteLine("Traded.");
			Dpw_Tr_DeleteAsync();
		}
		else if(result == DPW_TR_ERROR_NO_DATA){
			Debug::WriteLine("データは無い");
			return true;
		}
		else
		{
			Debug::WriteLine("Failed to upload result." + result);
			Dpw_Tr_ReturnAsync();
		}

		if(WaitForAsync() != 0 )
		{
			Debug::WriteLine("Failed to delete.");
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------
/**
 * @breif   非同期関数が終了するまで待つ
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

bool MainForm::RequestSetProfile(void)
{
	// プロフィールセット
	{
		Dpw_Common_Profile profile;
		Dpw_Common_ProfileResult profile_result;
		SetProfile(&profile);
		Dpw_Tr_SetProfileAsync(&profile, &profile_result);
		if(WaitForAsync() != 0 )
		{
			Debug::WriteLine("Failed to set profile.");
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------
/**
 * @breif   非同期関数が終了するまで待つ
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::WaitForAsync(void)
{
	while(!Dpw_Tr_IsAsyncEnd())
	{
		Dpw_Tr_Main();
		Sleep(10);
	}
	// リクエストが終わったら再度プロキシをセットしなければならない。
	SetProxy("");
	return Dpw_Tr_GetAsyncResult();
}

//--------------------------------------------------------------
/**
 * @breif   サーバステータスチェック
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

bool MainForm::RequestCheckServerState(void)
{
	// サーバステータスチェック
	{
		Dpw_Tr_GetServerStateAsync();
		if(WaitForAsync() != DPW_TR_STATUS_SERVER_OK )
		{
			Debug::WriteLine("Invalid server status.");
			return false;
		}
	}
	return true;
}

//--------------------------------------------------------------
/**
 * @breif   HTTPプロキシをセットする。NULLもしくは""で解除
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void MainForm::SetProxy(String^ proxy)
{
	NetIRC::SetProxy(proxy);
}


//--------------------------------------------------------------
/**
 * @breif   GTS接続テスト
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::TestUploadDownload(int pid,  String^ proxy)
{
#if 0	
	static int count = 1;
	//Dpw_Tr_Init(pid, TEST_KEY | pid);

	SetProxy(proxy);

	if(!RequestCheckServerState())
	{
		return;
	}

	if(!RequestSetProfile())
	{
		return;
	}

	RequestUpload();
/*
	if(!RequestPickup())
	{
		return;
	}
	*/
	Dpw_Tr_Db_UpdateServer();
	Dpw_Tr_End();

	Debug::WriteLine("Finished Test\n");
#endif
}

//--------------------------------------------------------------
/**
 * @breif   GTS接続テスト
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::TestDownload(int pid,  String^ proxy)
{
	static int count = 1;
	//Dpw_Tr_Init(pid, TEST_KEY | pid);

	SetProxy(proxy);
	if(!RequestCheckServerState())
	{
		return;
	}
	if(!RequestSetProfile())
	{
		return;
	}
	if(!RequestPickupTraded())
	{
		return;
	}
	Dpw_Tr_End();
	Debug::WriteLine("Delete - Finish\n");
}

//--------------------------------------------------------------
/**
 * @breif   GTSにあるデータの削除を行う
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::gTSResetToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	String^ proxy = "";

	// 引き取る
	TestDownload(100000, proxy);
}

//--------------------------------------------------------------
/**
 * @breif   ボックス選択等からメインHTMLにもどる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::gTSTestGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{

	// HTTPプロキシが必要な場合はプロキシのアドレスを"<server>[:port]"の様に指定してください。
	//String^ proxy = "proxy.server.com:8080";
	//String^ proxy = "";

	// 預けて引き取る
	//TestUploadDownload(100000, proxy);


	FormDispChange(FORM_DISPMODE_WEB);


}

//--------------------------------------------------------------
/**
 * @breif   赤外線通信してDSからGTS操作を行う
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------


System::Void MainForm::dSGTSSyncTToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = 'W';
	NetIRC::sendData('W',localarray);
}

//--------------------------------------------------------------
/**
 * @breif   ポケモン名から番号を得る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::PokemonName2No(String^ spokename)
{
	for(int i=0;i< POKEMON_MAX;i++){
		char* name = PokemonDataTable::getPokemonName(i);
		String^ pknm = gcnew String(name);
		if(pknm == spokename){
			return i;
		}
	}
	return 0;
}




//--------------------------------------------------------------
/**
 * @breif   赤外線通信してDSからGTS操作を行う
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::CallProg(String^ message)
{
	//NetIRC::connect();
	connectIRC();

	if(message =="PokeSend"){
		bBoxListRecv = false;
		threadB = gcnew Thread(gcnew ThreadStart(this,&MainForm::GetPokeBoxList)); //
		threadB->IsBackground = true; // バックグラウンド・スレッドとする
		threadB->Priority = ThreadPriority::Highest; //優先度を「最優先」にする
	    threadB->Start(); // 
		threadB->Join();
		threadB=nullptr;
		FormDispChange(FORM_DISPMODE_BOX);
		GTSDispMode = MODE_BOXDISP;
		pokemonListDisp(0);
	}
	else if(message =="PokeSearch"){
		PokemonSearch^ pPoke = gcnew PokemonSearch;
		pPoke->listBox1->SelectedIndex = 2;
		if(pPoke->ShowDialog() == System::Windows::Forms::DialogResult::OK){
			GTSDispMode = MODE_SEARCHPOKE;

			targetPoke = PokemonName2No(pPoke->textBox1->Text);
			sendPokeSex = pPoke->listBox1->SelectedIndex + 1;
			sendPokeLvMax = System::Decimal::ToInt32(pPoke->numericUpDown3->Value);
			sendPokeLv = System::Decimal::ToInt32(pPoke->numericUpDown2->Value);
			//サーチスレッド起動
			threadB = gcnew Thread(gcnew ThreadStart(this,&MainForm::RequestSearch)); //
			threadB->IsBackground = true; // バックグラウンド・スレッドとする
			threadB->Priority = ThreadPriority::Highest; //優先度を「最優先」にする
		    threadB->Start(); // 
			threadB->Join();
			threadB=nullptr;
			FormDispChange(FORM_DISPMODE_BOX);
			pokemonSearchDisp();
		}
	}
	else if(message =="PokeSound"){
		FormDispChange(FORM_DISPMODE_SOUND);
		pictureBox3->AllowDrop = true;
		label1->Text = "音楽を♪に重ねてください";

	}
	else{
		MessageBox::Show(message, "client code");
	}

}

//--------------------------------------------------------------
/**
 * @breif   探してきたポケモンの表示
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::pokemonSearchDisp(void)
{
	int xof = POKEBOX_OFFSETX;
	int yof = POKEBOX_OFFSETY;

	Bitmap^ bmp = gcnew Bitmap("boximage.bmp"); 

	pictureBox1->Width = bmp->Width;
	pictureBox1->Height = bmp->Height;
	pictureBox1->Left = this->Width / 2 - bmp->Width / 2;

	for(int i=0;i < DownloadMatchNum;i++){
		Dpw_Tr_Data     *dtp = &match_data_buf[i];

		int no = PokemonDataTable::getPokemonNo(dtp->postSimple.characterNo);

		String^ dirname = "C:\\home\\wb\\pokemon_wb\\pc\\PokeIRC\\PokeIRC\\pokegra\\";
		String^ ncgname = dirname + "pmpl_" + no.ToString("000") + "_frnt_m.ncg";
		String^ nclname = dirname + "pmpl_" + no.ToString("000") + "_n.ncl";
	
		FileNCGRead^ fngc = gcnew FileNCGRead;

		fngc->readWithNcl(ncgname,"", nclname);
		fngc->PictureWriteOffset(bmp, pictureBox1, xof+80*(i%6), yof+80*(i/6),10,10);
	}
	pictureBox1->Image = bmp;


	//@@OO

}

//--------------------------------------------------------------
/**
 * @breif   ボックスのポケモンを表示する
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::pokemonListDisp(int boxNo)
{
	int i;
	int xof = POKEBOX_OFFSETX;
	int yof = POKEBOX_OFFSETY;

	dispBoxNo = boxNo;
//	Bitmap^ bmp = gcnew Bitmap(10 * 6 * 16, 10 * 16 * 5);
	Bitmap^ bmp = gcnew Bitmap("boximage.bmp"); 


	pictureBox1->Width = bmp->Width;
	pictureBox1->Height = bmp->Height;
	pictureBox1->Left = this->Width / 2 - bmp->Width / 2;


	int printBoxNo = boxNo + 1;
	button2->Text = "ボックス " + printBoxNo;

	for(i=0;i < POKMEON_BOX_POKENUM;i++){
		int tno = GPokeBoxList[boxNo].poke[i].pokeno;
		if(tno == 0){
			continue;
		}
		if(tno >= POKEMON_MAX ){
			continue;
		}

		int no = PokemonDataTable::getPokemonNo(tno);

		String^ dirname = "C:\\home\\wb\\pokemon_wb\\pc\\PokeIRC\\PokeIRC\\pokegra\\";
		String^ ncgname = dirname + "pmpl_" + no.ToString("000") + "_frnt_m.ncg";
		String^ nclname = dirname + "pmpl_" + no.ToString("000") + "_n.ncl";
	
		FileNCGRead^ fngc = gcnew FileNCGRead;

		fngc->readWithNcl(ncgname,"", nclname);
	
		//pictureBox1->Image = fngc->PictureWriteOffset(bmp, pictureBox1, xof+80*(i%6), yof+80*(i/6),10,10);
		fngc->PictureWriteOffset(bmp, pictureBox1, xof+80*(i%6), yof+80*(i/6),10,10);

	}
	pictureBox1->Image = bmp;
	splitContainer1->Panel1->Show();
	button1->Width = this->Width / 5;
	button2->Width = (this->Width / 5) * 3;
	button3->Width = this->Width / 5;

	pictureBox5->Top = 600;
	pictureBox5->Left = 700;

	pokemonMouse(handPoke.pokeno);
}

//--------------------------------------------------------------
/**
 * @breif   ボックスのポケモンを表示する
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::pokemonMouse(int pokeNo)
{
	//int xof = POKEBOX_OFFSETX;
//	int yof = POKEBOX_OFFSETY;

	Bitmap^ bmp = gcnew Bitmap(10 * 16, 10 * 16);

	pictureBox4->Width = 10*8;
	pictureBox4->Height = 10*8;
	int tno = pokeNo;

	if(tno == 0){
		pictureBox4->Visible = false;
		return;
	}
	if(tno >= POKEMON_MAX ){
		pictureBox4->Visible = false;
		return;
	}
	int no = PokemonDataTable::getPokemonNo(tno);

	String^ dirname = "C:\\home\\wb\\pokemon_wb\\pc\\PokeIRC\\PokeIRC\\pokegra\\";
	String^ ncgname = dirname + "pmpl_" + no.ToString("000") + "_frnt_m.ncg";
	String^ nclname = dirname + "pmpl_" + no.ToString("000") + "_n.ncl";
	
	FileNCGRead^ fngc = gcnew FileNCGRead;

	fngc->readWithNcl(ncgname,"", nclname);
	
	pictureBox4->Image = fngc->PictureWriteOffset(bmp, pictureBox1, 0, 0,10,10);


	pictureBox4->Visible = true;
}


//--------------------------------------------------------------
/**
 * @breif   ポケモンを探す
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::RequestSearch(void)
{
	s32 result = 0;


	{
		array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
		localarray[0] = IRC_COMMAND_POKEBOX_MOVE;
		NetIRC::sendData(IRC_COMMAND_POKEBOX_MOVE, localarray);
	}

	//Dpw_Tr_Data match_data_buf[DPW_TR_DOWNLOADMATCHDATA_MAX];	// データの検索結果を入れるバッファ。
	// 検索→トレード
	{
		// 検索
		Dpw_Tr_PokemonSearchData search_data;
		
		search_data.characterNo = targetPoke;
		search_data.gender = sendPokeSex;
		search_data.level_min = sendPokeLv;
		search_data.level_max = sendPokeLvMax;	// 0を指定すると、上限は設定しないことになる
//        search_data.maxNum = DPW_TR_DOWNLOADMATCHDATA_MAX;
//        search_data.countryCode = 103;
		
		Dpw_Tr_DownloadMatchDataAsync(&search_data,DPW_TR_DOWNLOADMATCHDATA_MAX, match_data_buf);
		result = WaitForAsync();
		if(result < 0 )
		{
			Debug::WriteLine("Failed to search.");
			return;
		}
		else if(result == 0)
		{
			Debug::WriteLine("No such pokemon.");
			return;
		}

		MessageBox::Show(result + "件みつかりました");
		Debug::WriteLine("Found pokemons." + result);
		DownloadMatchNum = result;
	}
	return;
}

//--------------------------------------------------------------
/**
 * @breif   手持ちポケモン表示
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pokemonToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	pokemonListDisp(0);

}


//--------------------------------------------------------------
/**
 * @breif   場所からPokemonBOXの場所番号を得る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::getBoxPositionFromThePlace(int x,int y)
{
	int xo = x - POKEBOX_OFFSETX;
	int yo = y - POKEBOX_OFFSETY;

	int i = xo  / (10*8);
	if(i < 0){
		return -1;
	}
	else if(i >= 6){
		return -1;
	}
	int j = (yo)  / (10*8);
	if(j < 0){
		return -1;
	}
	else if(j >= 5){
		return -1;
	}
	i = i + j * 6;
	if((i >= 0) && (i < POKMEON_BOX_POKENUM)){
		return i;
	}
	return -1;
}


//--------------------------------------------------------------
/**
 * @breif   場所からPokemon番号を得る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::getPokemonNumberFromThePlace(int x,int y)
{
	int no = getBoxPositionFromThePlace(x, y);
	if(no != -1){
		return GPokeBoxList[dispBoxNo].poke[no].pokeno;
	}
	return 0;
}


//--------------------------------------------------------------
/**
 * @breif   場所からPokemonsexを得る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::getPokemonSexFromThePlace(int x,int y)
{
	int no = getBoxPositionFromThePlace(x, y);
	if(no != -1){
		return GPokeBoxList[dispBoxNo].poke[no].pokesex;
	}
	return 0;
}

//--------------------------------------------------------------
/**
 * @breif   場所からPokemonsexを得る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

int MainForm::getPokemonLvFromThePlace(int x,int y)
{
	int no = getBoxPositionFromThePlace(x, y);
	if(no != -1){
		return GPokeBoxList[dispBoxNo].poke[no].pokelv;
	}
	return 0;
}

//--------------------------------------------------------------
/**
 * @breif   マウス移動時にポケモンの名前などをTipで表示
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------


System::Void MainForm::pictureBox1_MouseMove(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	int pokeno;
	int pokesex;
	int pokelv;
	int no;
	String^ sexs = "ふめい";
	String^ comment="\n ";
	String^ pknm="";
	no = getBoxPositionFromThePlace(e->X,e->Y);


	switch(GTSDispMode){
	case MODE_SEARCHPOKE:
	case MODE_SEARCHPOKE_BOXDISP:
		{
			if((no == -1)  || (no >= DownloadMatchNum)){
				return;
			}
			Dpw_Tr_Data *dtp = &match_data_buf[no];
			pokeno = dtp->postSimple.characterNo;
			pokesex = dtp->postSimple.gender;
			pokelv = dtp->postSimple.level;
			comment = comment + PokemonDataTable::DPSpellConv(dtp->name, DPW_TR_NAME_SIZE);
		}
		break;
	case MODE_BOXDISP:
		pokeno = getPokemonNumberFromThePlace(e->X,e->Y);
		pokesex = getPokemonSexFromThePlace(e->X,e->Y);
		pokelv = getPokemonLvFromThePlace(e->X,e->Y);
		break;

	default:
		return;
	}

	if(BackupPosNo != no){
		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = IRC_COMMAND_MOUSE;
		localarray[1] = no;
		NetIRC::sendData(IRC_COMMAND_MOUSE, localarray);
		mouseBoxPoke = no;
		BackupPosNo = no;
		if(no!=-1){
			pictureBox4->Top = 80*(no/6)+140;
			pictureBox4->Left = 80*(no%6)+80;
			pictureBox4->Refresh();
		}

	}
	else{
		//mouseBoxPoke = -1;
	}

	if(pokeno != 0){
		switch(pokesex){
			case 1:
				sexs = "オス";
				break;
			case 2:
				sexs = "メス";
				break;
		}
		char* name = PokemonDataTable::getPokemonName(pokeno);
		pknm = gcnew String(name);
		pknm = pknm + " " + sexs + " LV" + pokelv + comment; 
	}
	else{
		return;
	}


	if(BackupToolTipMsg != pknm){
		toolTip1->Show(pknm, this, e->X, e->Y+20, 3000);
		BackupToolTipMsg = pknm;
	}


//	pictureBox4->Top = e->Y + 2;
//	pictureBox4->Left = e->X + 2;


}

//--------------------------------------------------------------
/**
 * @breif   マウスを右ボタンで押した時にメニュー表示
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pictureBox1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	if((e->Button::get()==System::Windows::Forms::MouseButtons::Right)){
		contextMenuStrip1->Show(Control::MousePosition::get());
	}
}


System::Void MainForm::pictureBox4_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	pictureBox1_MouseDown(sender, e);
}



//--------------------------------------------------------------
/**
 * @breif   マウスをクリックした時にポケモンを預けるかどうか問いあわせる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pictureBox1_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	String^ nm = gcnew String(PROGRAM_NAME);
	int pokeno = 0;

	switch(GTSDispMode){
	case MODE_BOXDISP:

		pokeno = getPokemonNumberFromThePlace(e->X,e->Y);

		// 預ける場合
		if(pokeno != 0){
			char* name = PokemonDataTable::getPokemonName(pokeno);
			String^ pknm = gcnew String(name);

			if ( MessageBox::Show(pknm + "を預けますか？", nm,
				 MessageBoxButtons::OKCancel,
				 MessageBoxIcon::Question )
					 == System::Windows::Forms::DialogResult::OK ) {
						 Debug::WriteLine(pokeno);
					targetPoke = pokeno;
					sendBoxNo = dispBoxNo;
					sendBoxPoke = getBoxPositionFromThePlace(e->X,e->Y);
					sendPokeSex = getPokemonSexFromThePlace(e->X,e->Y);
					sendPokeLv = getPokemonLvFromThePlace(e->X,e->Y);

					threadB = gcnew Thread(gcnew ThreadStart(this,&MainForm::RequestUpload));  //
					threadB->IsBackground = true;                // バックグラウンド・スレッドとする
					threadB->Priority = ThreadPriority::Highest; // 優先度を「最優先」にする
				    threadB->Start(); // 
	
			}
		}
		break;
	case MODE_SEARCHPOKE:	//交換する場合
		if ( MessageBox::Show("このポケモンと交換しますか？", nm,
			 MessageBoxButtons::OKCancel,
			 MessageBoxIcon::Question )
				 == System::Windows::Forms::DialogResult::OK ) {


				// ボックス一覧を出す
				 searchPokeIndex = getBoxPositionFromThePlace(e->X,e->Y);
			

				GTSDispMode = MODE_SEARCHPOKE_BOXDISP;

				connectIRC();
				bBoxListRecv = false;
				threadB = gcnew Thread(gcnew ThreadStart(this,&MainForm::GetPokeBoxList)); //
				threadB->IsBackground = true; // バックグラウンド・スレッドとする
				threadB->Priority = ThreadPriority::Highest; //優先度を「最優先」にする
			    threadB->Start(); // 
				threadB->Join();
				pokemonListDisp(0);



		}



		break;

	case MODE_SEARCHPOKE_BOXDISP:	//交換する場合その２
		pokeno = getPokemonNumberFromThePlace(e->X,e->Y);

		if(pokeno != 0){
			char* name = PokemonDataTable::getPokemonName(pokeno);
			String^ pknm = gcnew String(name);

			if ( MessageBox::Show(pknm + "と交換しますか？", nm,
				 MessageBoxButtons::OKCancel,
				 MessageBoxIcon::Question )
					 == System::Windows::Forms::DialogResult::OK ) {
					targetPoke = pokeno;
					sendBoxNo = dispBoxNo;
					sendBoxPoke = getBoxPositionFromThePlace(e->X,e->Y);
					sendPokeSex = getPokemonSexFromThePlace(e->X,e->Y);
					sendPokeLv = getPokemonLvFromThePlace(e->X,e->Y);

					// 赤外線コマンド発行
					array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
					localarray[0] = sendBoxNo;
					localarray[1] = sendBoxPoke;
					NetIRC::sendData(IRC_COMMAND_CHANGE_BOXPOKE, localarray);

					threadB = gcnew Thread(gcnew ThreadStart(this, &MainForm::RequestChange));  //
					threadB->IsBackground = true;                // バックグラウンド・スレッドとする
					threadB->Priority = ThreadPriority::Highest; // 優先度を「最優先」にする
				    threadB->Start(); // 
	
			}
		}



	default:
		break;
	}



}

//--------------------------------------------------------------
/**
 * @breif   ツールを終了する
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::exitEToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	String^ nm = gcnew String(PROGRAM_NAME);
	if ( MessageBox::Show("アプリケーションを終了しますか？", nm,
		 MessageBoxButtons::OKCancel,
		 MessageBoxIcon::Question )
			 == System::Windows::Forms::DialogResult::OK ) {
				 this->Close();
	}
}

//--------------------------------------------------------------
/**
 * @breif   図鑑切り替え
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
System::Void MainForm::button3_Click(System::Object^  sender, System::EventArgs^  e)
{
	dispBoxNo++;
	if(dispBoxNo>=POKMEON_BOX_NUM){
		dispBoxNo = 0;
	}
	
	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = dispBoxNo;
	NetIRC::sendData(IRC_COMMAND_BOXNO, localarray);


	pokemonListDisp(dispBoxNo);
}

//--------------------------------------------------------------
/**
 * @breif   図鑑切り替え
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::button1_Click(System::Object^  sender, System::EventArgs^  e)
{
	if(dispBoxNo <= 0){
		dispBoxNo = POKMEON_BOX_NUM;
	}
	dispBoxNo--;
	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = dispBoxNo;
	NetIRC::sendData(IRC_COMMAND_BOXNO,localarray);

	pokemonListDisp(dispBoxNo);
}


//--------------------------------------------------------------
/**
 * @breif   ポケモンボックスリスト要求
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::GetPokeBoxList(void)
{
	//isRecv = false;
	int i,j,k=0;
	

	{
		array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
		localarray[0] = IRC_COMMAND_POKEBOX_MOVE;
		NetIRC::sendData(IRC_COMMAND_POKEBOX_MOVE, localarray);
		NetIRC::sendLock();
	}
	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}


	NetIRC::sendUnLock();
	array<unsigned char>^ localarray = gcnew array<unsigned char>(1);
	localarray[0] = IRC_COMMAND_BOXLIST;
	NetIRC::sendData(IRC_COMMAND_BOXLIST,localarray);
	NetIRC::sendLock();


	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}
	
	
	k=2;
	for(i=0;i < BOX_MAX_TRAY;i++){
		for(j = 0;j < BOX_MAX_POS;j++){
			GPokeBoxList[i].poke[j].pokeno = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
		for(j = 0;j < BOX_MAX_POS;j++){
			GPokeBoxList[i].poke[j].pokelv = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
		for(j = 0;j < BOX_MAX_POS;j++){
			GPokeBoxList[i].poke[j].pokesex = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
		for(j = 0;j < BOX_TRAYNAME_BUFSIZE+2;j++){
			GPokeBoxList[i].trayName[j] = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
	}
		NetIRC::sendUnLock();
	bBoxListRecv = true;
}

//--------------------------------------------------------------
/**
 * @breif   サウンドのもどるの絵がクリックされた
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pictureBox2_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	int pictX = 588;
	int pictY = 549;

	pictX = (this->splitContainer1->Panel2->Width - pictX) / 2;
	pictY = (this->splitContainer1->Panel2->Height - pictY) / 2;

	if((e->X > (321+pictX)) && (e->X < (558+pictX))){
		if((e->Y > (192+pictY)) && (e->Y < (236+pictY))){
			FormDispChange(FORM_DISPMODE_WEB);
		}
	}
}

//--------------------------------------------------------------
/**
 * @breif   描画処理
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::Draw(void)
{
	if(SoundBarTotalSize != 0){
		if(NetIRC::dataArray == nullptr){
			SoundBarTotalSize = 0;
			label1->Text = "音楽を送信しました";

			Debug::WriteLine(DateTime::Now.ToString("F")+"end");
			return;
		}
		else{
			int lest = NetIRC::dataArray->GetLength(0);
			if(lest == 0){
				SoundBarTotalSize = 0;
				label1->Text = "音楽を送信しました";
				Debug::WriteLine(DateTime::Now.ToString("F")+"end");
				return;
			}
			lest = (lest * SoundBarSize) / SoundBarTotalSize;
			pictureBox3->Width = lest;
		}
	}

	if(NetIRC::IsConnect()){
		if(StripStatusLabel->Text==""){
			StripStatusLabel->Text="ＤＳと接続しています";
		}
	}
	else{
		if(StripStatusLabel->Text!=""){
			StripStatusLabel->Text="";
		}
	}


	if(threadB!=nullptr){
		threadB->Join();
		pokemonListDisp(sendBoxNo);
		threadB=nullptr;
	}

	//else if(NetIRC::dataArray == nullptr){
	//	label1->Text = "音楽を送信しました";
	//}
}

//--------------------------------------------------------------
/**
 * @breif   つかむ選択
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::handHToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	int groundpoke = GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno;
	int curpoke = handPoke.pokeno;

	if((groundpoke!=0) && (curpoke==0)){ //つかむ
		pictureBox1->Cursor = System::Windows::Forms::Cursors::Cross;
		handBoxNo = dispBoxNo;
		handBoxPoke = mouseBoxPoke;
		handPoke.pokeno = GPokeBoxList[handBoxNo].poke[handBoxPoke].pokeno;
		handPoke.pokelv = GPokeBoxList[handBoxNo].poke[handBoxPoke].pokelv;
		handPoke.pokesex = GPokeBoxList[handBoxNo].poke[handBoxPoke].pokesex;
		GPokeBoxList[handBoxNo].poke[handBoxPoke].pokeno = 0;
		GPokeBoxList[handBoxNo].poke[handBoxPoke].pokelv = 0;
		GPokeBoxList[handBoxNo].poke[handBoxPoke].pokesex = 0;
		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = handBoxNo;
		localarray[1] = handBoxPoke;
		NetIRC::sendData(IRC_COMMAND_HAND, localarray);
		pokemonListDisp(handBoxNo);
	}
	else if((groundpoke!=0) && (curpoke!=0)){ //入れ替え


		pictureBox1->Cursor = System::Windows::Forms::Cursors::Cross;
		handBoxNo = dispBoxNo;
		handBoxPoke = mouseBoxPoke;
		PokemonData temp;

		temp.pokeno = GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno;
		temp.pokelv = GPokeBoxList[handBoxNo].poke[BackupPosNo].pokelv;
		temp.pokesex = GPokeBoxList[handBoxNo].poke[BackupPosNo].pokesex;

		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno = handPoke.pokeno;
		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokelv = handPoke.pokelv;
		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokesex = handPoke.pokesex;

		handPoke.pokeno = temp.pokeno;
		handPoke.pokelv = temp.pokelv;
		handPoke.pokesex = temp.pokesex;

		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = handBoxNo;
		localarray[1] = handBoxPoke;
		NetIRC::sendData(IRC_COMMAND_HANDCHANGE, localarray);
		pokemonListDisp(handBoxNo);

	}
	else if((groundpoke==0) && (curpoke!=0)){ // 置く
		pictureBox1->Cursor = System::Windows::Forms::Cursors::Default;
		handBoxNo = 0;
		handBoxPoke = 0;
		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno = handPoke.pokeno;
		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokelv = handPoke.pokelv;
		GPokeBoxList[handBoxNo].poke[BackupPosNo].pokesex = handPoke.pokesex;
		handPoke.pokeno = 0;
		handPoke.pokelv = 0;
		handPoke.pokesex = 0;
		array<unsigned char>^ localarray = gcnew array<unsigned char>(2);
		localarray[0] = dispBoxNo;
		localarray[1] = mouseBoxPoke;
		Debug::WriteLine("手を下ろす");
		NetIRC::sendData(IRC_COMMAND_HANDDOWN, localarray);
		pokemonListDisp(handBoxNo);
	}
}

//--------------------------------------------------------------
/**
 * @breif   右メニューが開いた時に発生
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------


System::Void MainForm::contextMenuStrip1_Opened(System::Object^  sender, System::EventArgs^  e)
{
	if((GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno!=0) && (handPoke.pokeno==0)){
		contextMenuStrip1->Items[0]->Text = "つかむ";
	}
	else if((GPokeBoxList[handBoxNo].poke[BackupPosNo].pokeno==0) && (handPoke.pokeno!=0)){
		contextMenuStrip1->Items[0]->Text = "ここにおく";
	}
	else if (handPoke.pokeno!=0){
		contextMenuStrip1->Items[0]->Text = "いれかえ";
	}


}

//--------------------------------------------------------------
/**
 * @breif   ようすをみる選択
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::infoIToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
}


#if 0
{
    //閉じた曲線を追加
    Point[] myArray = {
        new Point(20, 100),
        new Point(40, 150),
        new Point(60, 125),
        new Point(40, 100),
        new Point(60, 75),
        new Point(40, 50)};
	System::Drawing::Drawing2D::GraphicsPath path = gcnew System::Drawing::Drawing2D::GraphicsPath;
    path->AddClosedCurve(myArray, 0.5F);
    //形を変更
    this->Region = new Region(path);
}
#endif

//--------------------------------------------------------------
/**
 * @breif   表示変更
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

void MainForm::FormDispChange(int mode)
{
	switch(mode){
		case FORM_DISPMODE_WEB:
			GTSDispMode = MODE_NONE;
			webBrowser1->Visible = true;
			pictureBox1->Visible = false;
			pictureBox2->Visible = false;
			pictureBox3->Visible = false;
			pictureBox4->Visible = false;
			pictureBox5->Visible = false;
			label1->Visible = false;
			break;
		case FORM_DISPMODE_BOX:
			webBrowser1->Dock = DockStyle::Fill;
			webBrowser1->Visible = false;
			pictureBox1->Visible = true;
			pictureBox2->Visible = false;
			pictureBox3->Visible = false;
			pictureBox4->Visible = false;
			pictureBox5->Visible = true;
			label1->Visible = false;	
			pictureBox5->Top = 600;
			pictureBox5->Left = 700;
			break;
		case FORM_DISPMODE_SOUND:
			GTSDispMode = MODE_NONE;
			pictureBox2->Dock = DockStyle::Fill;
			webBrowser1->Hide();
			pictureBox1->Hide();
			pictureBox2->Show();
			pictureBox3->Show();

			int pictX = 588;
			int pictY = 549;
			pictureBox3->Width=236;
			pictureBox3->Left = (this->splitContainer1->Panel2->Width - pictX) / 2 + 32;
			pictureBox3->Top = (this->splitContainer1->Panel2->Height - pictY) / 2 + 435;
			pictureBox3->AllowDrop = true;

			pictY = 480;
			label1->Show();
			label1->Left = (this->splitContainer1->Panel2->Width - pictX) / 2 + 32;
			label1->Top = (this->splitContainer1->Panel2->Height - pictY) / 2 + 435;
			pictureBox4->Visible = false;
			pictureBox5->Visible = true;
			pictureBox5->Top = 525;
			pictureBox5->Left = 553;
			break;
	}

}

//--------------------------------------------------------------
/**
 * @breif   表示変更
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------


System::Void MainForm::pictureBox5_Click(System::Object^  sender, System::EventArgs^  e)
{
	FormDispChange(FORM_DISPMODE_WEB);
}

System::Void MainForm::pictureBox5_MouseEnter(System::Object^  sender, System::EventArgs^  e)
{
	pictureBox5->Image = gcnew Bitmap("backbuttonhi.bmp"); 
}

System::Void MainForm::pictureBox5_MouseLeave(System::Object^  sender, System::EventArgs^  e)
{
	pictureBox5->Image = gcnew Bitmap("backbutton.bmp"); 
}

System::Void MainForm::webBrowser1_NewWindow(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e)
{
//	Debug::WriteLine("Already uploaded.");
	e->Cancel=true;
//	webBrowser1->Url();
//	webBrowser1->

}

//--------------------------------------------------------------
/**
 * @breif   切断
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::disConnectDToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	if(threadA!=nullptr){
		threadA->Abort();
		threadA->Join();
		threadA=nullptr;
		NetIRC::shutdown();
	}

}
