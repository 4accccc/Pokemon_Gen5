//=============================================================================
/**
 * @file	NetIRC.cpp
 * @brief	赤外線ライブラリラッパークラス
 * @author	k.ohno
 * @date    2008.09.30
 */
//=============================================================================

#include "stdafx.h"
#include "MainForm.h" 
#include "NetIRC.h"

typedef int BOOL;
typedef wchar_t TCHAR;
typedef unsigned char u8;

#include "irc.h"

#pragma comment(lib,"libpokemon.lib") // libpokemon.libをリンクする

using namespace PokeIRC;
using namespace System::Windows::Forms;

#define COMMAND_SND (2)
#define COMMAND_SNDV (3)
#define COMMAND_SNDE (9)

//--------------------------------------------------------------
/**
 * @breif   コールバック
 * @param   data     受け取ったデータ
 * @param   size     サイズ
 * @param   command  コマンド
 * @param   value    さらに値
 * @retval  none
 */
//--------------------------------------------------------------
static bool flgsend = false;


//void NetIRC::callback(u8 *data, u8 size, u8 command, u8 value)
static void callback(u8 *data, u8 size, u8 command, u8 value)
{
	unsigned char data_up[64]={0};


	switch(command){
	case 0x02: // センダー は2を受け取る
		IRC_Send((u8*)data_up, 64, 0x04, (u8)1);
		break;
	case 0x04: // レシーバー は4を受け取る
		if(NetIRC::SendLoop()){
			IRC_Send(data_up, 64, 2, (u8)1 ); 
		}
		flgsend=true;
		break;
	case 0xF4: // COMMAND_SHUTDOWN:
		Debug::WriteLine("CALLBACK: シャットダウンコマンドを受信\n");
		break;
	}
}



//--------------------------------------------------------------
/**
 * @breif   初期化
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void NetIRC::Init(void)
{
	isEmulate = false;
	isDataSend = false;
	isSend = false;
	IRC_Init();
	IRC_SetRecvCallback(callback);
	IRC_SetDisconnectTime(500);
}

//--------------------------------------------------------------
/**
 * @breif   描画（というよりプロセスが混ざっている）
 * @param   mform    メインフォーム
 * @retval  none
 */
//--------------------------------------------------------------
void NetIRC::draw(Form^ fm)
{
	MainForm^ mform = dynamic_cast<MainForm^>(fm);
	unsigned char data_up[64]={0,1,2,3};
	
	if(IRC_IsConnectUsb()){
		String^ coms = gcnew String((wchar_t *)IRC_GetPortName());
		mform->StripStatusLabel->Text = "USB認識中" + coms;
	}else{
		mform->StripStatusLabel->Text = "USB切断中";
	}
	if(IRC_IsConnect()){
		if(IRC_IsSender()){
			mform->StripStatusLabel->Text += "Ir 親";
		}else{
			mform->StripStatusLabel->Text += "Ir 子";
		}
	}else{
		mform->StripStatusLabel->Text += "Ir 切";
	}
	if(dataArray!=nullptr){
		mform->StripStatusLabelCenter->Text = dataArray->Length.ToString();
	}

	{
		bool tmp;
		tmp = IRC_IsConnect();
		IRC_Move();

		if(IRC_IsConnect() && (tmp == false)){ // つながった瞬間のみ
			Debug::WriteLine("最初のコマンド(2)発信" + IRC_IsSender());
			if(!IRC_IsSender()){  // つながった瞬間のみ子が最初のパケットを投げる
				IRC_Send((u8*)NULL, 0, 0x02, 0);
				isSend = true;
			}
		}
	}
	if((!IRC_IsConnect()) && isEmulate){
		IRC_Connect();
	} // 自動再接続処理
}


//--------------------------------------------------------------
/**
 * @breif   接続
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void NetIRC::connect(void)
{
	isEmulate = true;
}

//--------------------------------------------------------------
/**
 * @breif   切断
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
void NetIRC::shutdown(void)
{
	IRC_Shutdown();
	isEmulate = false;
}

//--------------------------------------------------------------
/**
 * @breif   データを送る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------
bool NetIRC::sendData(void)
{
	if(isDataSend){
		return false;
	}
	isDataSend = true;
	connect();
	return true;
}

//--------------------------------------------------------------
/**
 * @breif   データを送る
 * @param   none
 * @retval  none
 */
//--------------------------------------------------------------

#define SENDBYTE_TEST (100)

bool NetIRC::SendLoop(void)
{
	if(!isDataSend){
		return true;
	}
	if(!IRC_IsConnect()){
		return true;
	}


	int length = dataArray->GetLength(0);

	pin_ptr<unsigned char> wptr = &dataArray[0];

	if(length > SENDBYTE_TEST){
		IRC_Send(wptr, SENDBYTE_TEST, COMMAND_SND, COMMAND_SNDV);
		array<unsigned char>^ dummyArray = {};
		Array::Resize(dummyArray,length-SENDBYTE_TEST);
		Array::ConstrainedCopy(dataArray,SENDBYTE_TEST,dummyArray,0,length-SENDBYTE_TEST);
		delete dataArray;
		dataArray = dummyArray;
	}
	else{
		IRC_Send(wptr, length, COMMAND_SND, COMMAND_SNDE);
		isDataSend=false;
	}
	Debug::WriteLine("snd送信"+length);
	return false;
	
}