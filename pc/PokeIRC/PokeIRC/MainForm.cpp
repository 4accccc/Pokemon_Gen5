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

#include < stdio.h >
#include < stdlib.h >
#include < vcclr.h >



using namespace PokeIRC;
using namespace System::Windows::Forms;
using namespace System::Threading;

static BoxTrayData GPokeBoxList[BOX_MAX_TRAY];


//--------------------------------------------------------------
/**
 * @breif   ファイル->Syncメニューが選択された時の処理
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::sToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{
	NetIRC::connect();

	threadA = gcnew Thread(gcnew ThreadStart(this,&MainForm::ThreadWork)); //

	threadA->IsBackground = true; // バックグラウンド・スレッドとする
	threadA->Priority = ThreadPriority::Highest; //優先度を「最優先」にする

    threadA->Start(); // 



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
	webBrowser1->Show();
	pictureBox1->Hide();


	this->Width = 10 * 8 * 6 + 10;
	this->Width *= 2;
	this->Height = 10 * 8 * 6 + 30;

//	button1->Hide();
//	button2->Hide();
//	button3->Hide();
	splitContainer1->Panel1->Hide();

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
			NetIRC::dataArray = userReader->ReadBytes((int)myStream->Length);
			NetIRC::sendData('A');
			myStream->Close();
		}
	}

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
	NetIRC::draw(this);
	Thread::Sleep(1); //2msec待つ 
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
		Thread::Sleep(3); //2msec待つ 
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
void MainForm::SetTrData(Dpw_Tr_Data* upload_data, int pokeNo)
{
	int i;
	memset(upload_data, 0, sizeof(*upload_data));

	for(i=0;i < 0x88;i++){
		upload_data->postData.data[i] = NetIRC::recvdataArray[i+2];
	}

	upload_data->postSimple.characterNo = pokeNo;
	upload_data->postSimple.gender = DPW_TR_GENDER_FEMALE;
	upload_data->postSimple.level = 20;
	upload_data->wantSimple.characterNo = 101;
	upload_data->wantSimple.gender = DPW_TR_GENDER_FEMALE;
	upload_data->wantSimple.level_min = 0;
	upload_data->wantSimple.level_max = 0;
	upload_data->name[0] = 0x64; // サ
	upload_data->name[1] = 0x77; // ト
	upload_data->name[2] = 0x62; // シ
	upload_data->name[3] = 0xffff;
	upload_data->trainerID = 1 << 12;
	upload_data->countryCode = 103;
	upload_data->langCode = 1;
	upload_data->localCode = 1;
	upload_data->trainerType = 3;
	upload_data->versionCode = 12;
	upload_data->gender = 1;
}

//--------------------------------------------------------------
/**
 * @breif   非同期関数が終了するまで待つ
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void MainForm::RequestUpload(void)
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

	{	// 預ける
		Dpw_Tr_Data data;
		SetTrData(&data, targetPoke);
		Dpw_Tr_UploadAsync(&data);
		result = WaitForAsync();
		if(result == 0 )
		{
			// 預けるのを確定
			Dpw_Tr_UploadFinishAsync();
			if(WaitForAsync() != 0 )
			{
				Debug::WriteLine("Failed to uploadFinish.");
				return;
			}
		}
		else if(result == DPW_TR_ERROR_ILLIGAL_REQUEST)
		{
			Debug::WriteLine("Already uploaded.");
		}
		else
		{
			Debug::WriteLine("Failed to upload.");
			return;
		}
	}
	MessageBox::Show("あずけました");
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
		//	return true;
		}
		else if(result == 1)
		{
			Debug::WriteLine("Traded.");
		}
		else
		{
			Debug::WriteLine("Failed to upload result.");
			return false;
		}

//		Dpw_Tr_DownloadAsync(&data);

//		if(WaitForAsync() != 0 )
//		{
//			puts("Failed to delete.");
//			return false;
//		}

		if(result == 1){
			Dpw_Tr_DeleteAsync();
		}
		else{
			Debug::WriteLine("RETURN ");
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
 * @breif   GTS接続テスト
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::gTSTestGToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e)
{

	// HTTPプロキシが必要な場合はプロキシのアドレスを"<server>[:port]"の様に指定してください。
	//String^ proxy = "proxy.server.com:8080";
	String^ proxy = "";

	// 預けて引き取る
	TestUploadDownload(100000, proxy);
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
	NetIRC::dataArray = gcnew array<unsigned char>(1);
	NetIRC::dataArray[0] = 'W';
	NetIRC::sendData('W');
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
	if(message =="PokeSend"){
		bBoxListRecv = false;
		threadA = gcnew Thread(gcnew ThreadStart(this,&MainForm::GetPokeBoxList)); //
		threadA->IsBackground = true; // バックグラウンド・スレッドとする
		threadA->Priority = ThreadPriority::Highest; //優先度を「最優先」にする
	    threadA->Start(); // 


		threadA->Join();

		pokemonListDisp(0);
	}
	else{
		MessageBox::Show(message, "client code");
	}

}

void MainForm::pokemonListDisp(int boxNo)
{
	int i;

	dispBoxNo = boxNo;
	Bitmap^ bmp = gcnew Bitmap(10 * 6 * 16, 10 * 16 * 5);

	int printBoxNo = boxNo + 1;
	button2->Text = "ボックス " + printBoxNo;

	for(i=0;i < POKMEON_BOX_POKENUM;i++){
		if(GPokeBoxList[boxNo].pokeno[i]==0){
//		if(putPoke[boxNo,i]==0){
			continue;
		}
		int tno = GPokeBoxList[boxNo].pokeno[i];
		if(tno >= POKEMON_MAX ){
			continue;
		}

		int no = PokeGraNoTable[tno].no;

		String^ dirname = "C:\\home\\wb\\pokemon_wb\\pc\\PokeIRC\\PokeIRC\\pokegra\\";
		String^ ncgname = dirname + "pmpl_" + no.ToString("000") + "_frnt_m.ncg";
		String^ nclname = dirname + "pmpl_" + no.ToString("000") + "_n.ncl";
	
		FileNCGRead^ fngc = gcnew FileNCGRead;

		fngc->readWithNcl(ncgname,"", nclname);
	
		pictureBox1->Image = fngc->PictureWriteOffset(bmp, pictureBox1,10*(i%6), 10*(i/6),10,10);

		pictureBox1->Dock = DockStyle::Fill;
		webBrowser1->Dock = DockStyle::Fill;

		webBrowser1->Visible = false;
		pictureBox1->Visible = true;
	}
	splitContainer1->Panel1->Show();
	button1->Width = this->Width / 5;
	button2->Width = (this->Width / 5) * 3;
	button3->Width = this->Width / 5;
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
	int i = x  / (10*8);
	if(i < 0){
		return -1;
	}
	else if(i >= 6){
		return -1;
	}
	int j = (y)  / (10*8);
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
		return GPokeBoxList[dispBoxNo].pokeno[no];
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
	static int backupPoke = -1;

	int pokeno = getPokemonNumberFromThePlace(e->X,e->Y);

	if((pokeno != 0) && (backupPoke != pokeno)){
		char* name = PokeGraNoTable[pokeno].name;
		String^ pknm = gcnew String(name);
		toolTip1->Show(pknm, this, e->X, e->Y+20, 30000);
		backupPoke = pokeno;
	}
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

//--------------------------------------------------------------
/**
 * @breif   マウスをクリックした時にポケモンを預けるかどうか問いあわせる
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

System::Void MainForm::pictureBox1_MouseClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e)
{
	int pokeno = getPokemonNumberFromThePlace(e->X,e->Y);

	if(pokeno != 0){
		String^ nm = gcnew String(PROGRAM_NAME);
		char* name = PokeGraNoTable[pokeno].name;
		String^ pknm = gcnew String(name);

		if ( MessageBox::Show(pknm + "を預けますか？", nm,
			 MessageBoxButtons::OKCancel,
			 MessageBoxIcon::Question )
				 == System::Windows::Forms::DialogResult::OK ) {
				targetPoke = pokeno;

				// 赤外線コマンド発行
				NetIRC::dataArray = gcnew array<unsigned char>(2);
				NetIRC::dataArray[0] = dispBoxNo;
				NetIRC::dataArray[1] = getBoxPositionFromThePlace(e->X,e->Y);
				NetIRC::sendData(IRC_COMMAND_BOXPOKE);

				threadA = gcnew Thread(gcnew ThreadStart(&RequestUpload));  //
				threadA->IsBackground = true;                // バックグラウンド・スレッドとする
				threadA->Priority = ThreadPriority::Highest; // 優先度を「最優先」にする
			    threadA->Start(); // 

		}
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

	NetIRC::dataArray = gcnew array<unsigned char>(1);
	NetIRC::dataArray[0] = IRC_COMMAND_BOXLIST;
	NetIRC::sendData(IRC_COMMAND_BOXLIST);


	while(!NetIRC::isRecvFlg()){
		Sleep(10);
	}
	
	
	k=2;
	for(i=0;i < BOX_MAX_TRAY;i++){
		for(j = 0;j < BOX_MAX_POS;j++){
			GPokeBoxList[i].pokeno[j] = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
		for(j = 0;j < BOX_MAX_POS;j++){
			GPokeBoxList[i].pokelv[j] = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
		for(j = 0;j < BOX_TRAYNAME_BUFSIZE+2;j++){
			GPokeBoxList[i].trayName[j] = NetIRC::recvdataArray[k]+NetIRC::recvdataArray[k+1]*0x100;
			k+=2;
		}
	}
	bBoxListRecv = true;
}