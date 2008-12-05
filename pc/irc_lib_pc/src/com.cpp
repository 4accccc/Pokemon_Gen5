#include "../include/system.h"
#include "../include/com.h"
/******************************************************************************
******************************************************************************/
#define COM_MAX			(16)
//#define COM_PORT_NAME	"COM8"
//#define BAUD_RATE		(115200)
#define BAUD_RATE		(256000)
#define BYTE_SIZE		(8)			// 8bit
#define PARITY			NOPARITY	// パリティー EVENPARITY
#define STOP_BIT		ONESTOPBIT	// ストップビット
#define F_PARITY		FALSE //TRUE		// フロー制御：
/******************************************************************************
******************************************************************************/
#define TIMEOUT			(20)
/******************************************************************************
******************************************************************************/
static bool isConnect;
static int connectIdx;
/******************************************************************************
******************************************************************************/
HANDLE hComm, hEvent;							// シリアルポートとの通信ハンドル
OVERLAPPED overlap;

static TCHAR *port_tbl_l[COM_MAX] = 
{
  L"COM1", L"COM2", L"COM3", L"COM4", L"COM5", L"COM6", L"COM7", L"COM8", 
  L"COM9", L"COM10", L"COM11", L"COM12", L"COM13", L"COM14", L"COM15", L"COM16", 
};

static char *port_tbl[COM_MAX] = 
{
  "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", 
  "COM9", "\\\\.\\COM10", "\\\\.\\COM11", "\\\\.\\COM12", "\\\\.\\COM13", "\\\\.\\COM14", "\\\\.\\COM15", "\\\\.\\COM16", 
};
/******************************************************************************
******************************************************************************/
bool ComIsConnect(void){ return isConnect; }
TCHAR *ComGetPort(void)
{
  if(connectIdx == -1){ return NULL; }
  return port_tbl_l[connectIdx];
}
/******************************************************************************
******************************************************************************/
static int _search_port(void)
{
  HKEY hKey = NULL;
  DWORD dwCount = 0;
  TCHAR cNameBuff[256];
  BYTE byValueBuff[256];
  DWORD dwNameBuffSize = 0;
  DWORD dwValueBuffSize = 0;
  DWORD dwType = 0;
  //LONG ret;

  int i;
  /****************************************************************************
  ****************************************************************************/
  if(RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SYSTEM\\CurrentControlSet\\Enum\\USB\\Vid_ffff&Pid_0015\\00000001\\Device Parameters"), 
				  0, 
				  KEY_READ, 
				  &hKey) != ERROR_SUCCESS){
	//printf("USB機器のGUID検索のためのレジストリオープン失敗\n");
	return (COM_MAX);
  }
  /****************************************************************************
  要素数を得る
  ****************************************************************************/
  DWORD subKeys;
  DWORD maxSubKeyLen;
  if(RegQueryInfoKey(hKey, NULL, NULL, NULL, &subKeys, &maxSubKeyLen, NULL, &dwCount, // レジストリエントリの数
					 NULL, NULL, NULL, NULL) != ERROR_SUCCESS ){
	//printf("COMポートの数取得に失敗しました\n");
	return (COM_MAX);
  }
  //printf("レジストリ中の サブキー数 %d 最大文字列数 %d レジストリ数 %d\n", subKeys, maxSubKeyLen, dwCount);
  /****************************************************************************
  要素数分を繰り返す
  ****************************************************************************/
  for(i = 0 ; i < (int)dwCount ; i++){
	dwNameBuffSize = sizeof(cNameBuff);
	dwValueBuffSize = sizeof(byValueBuff);
	
	if(RegEnumValue(hKey, i, &cNameBuff[0], &dwNameBuffSize, NULL, &dwType, // 一覧を取り出す
					&byValueBuff[0], &dwValueBuffSize) != ERROR_SUCCESS ){
	  //printf("名前の取得に失敗しました\n");
	  return (COM_MAX);
	}
	if(wcscmp(L"PortName", cNameBuff) == 0){ /*printf("%ls: %ls\n", cNameBuff, byValueBuff);*/ break;}
#if 0
	dwNameBuffSize = sizeof(keyBuff);
	dwValueBuffSize = sizeof(subBuff);
	ret = RegEnumKeyExA(hKey, i, &keyBuff[0], &dwNameBuffSize, NULL,
					   &subBuff[0], &dwValueBuffSize, &filetime);
	if(ret != ERROR_SUCCESS ){
	  DWORD r;
	  printf("キーの取得に失敗しました %d\n", ret);
	  r = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
						NULL, // 無視される
						ret,//GetLastError(),
						LANG_USER_DEFAULT, // 言語ID
						&cNameBuff[0], 256,
						NULL);
	  printf("戻り値 %d\n", r);
	  MessageBox(g_hWnd, cNameBuff, L"error: RegEnumKeyEx", MB_ICONHAND|MB_OK);
	  return (COM_MAX);
	}
	printf("%ls\n", cNameBuff);
#endif 
  }
  /****************************************************************************
  ****************************************************************************/
  for(i=0;i<COM_MAX;i++){
	if(wcscmp(port_tbl_l[i], (TCHAR*)byValueBuff) == 0){ return (i); }
  }
  return (COM_MAX);
}
/******************************************************************************
シリアルポートを開ける
******************************************************************************/
bool ComInit(unsigned char idx)
{
  hEvent = NULL; hComm = NULL;
  isConnect = FALSE; connectIdx = -1;
  /****************************************************************************
  ****************************************************************************/
  hEvent = CreateEvent(NULL, FALSE, FALSE, L"overlap");
  memset(&overlap, 0, sizeof(OVERLAPPED));
  overlap.hEvent = hEvent;
  /****************************************************************************
  ****************************************************************************/
  if(idx == COM_AUTO){ // 赤外線ユニットを探す
	idx = _search_port(); 
	if(idx == COM_MAX){ /*printf("COMポートが見つかりませんでした\n");*/ return false; }
  }
  /****************************************************************************
  シリアルポートを開ける
  ****************************************************************************/
  hComm = CreateFileA(port_tbl[idx],					// シリアルポートの文字列
					 GENERIC_READ | GENERIC_WRITE,	// アクセスモード：読み書き 
					 0,								// 共有モード：他からはアクセス不可 
					 NULL,							// セキュリティ属性：ハンドル継承せず 
					 OPEN_EXISTING,					// 作成フラグ：
					 FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,			// 属性：
					 NULL);							// テンプレートのハンドル： 
  
  
  if(hComm == INVALID_HANDLE_VALUE) {
	//printf("%sを開くことが出来ませんでした。\n", port_tbl[idx]); // これ出すと再接続で流れまくる
	return false;
  }
  //printf("%sのオープン成功。\n", port_tbl[idx]);
  isConnect = TRUE; connectIdx = idx;
  /****************************************************************************
  通信属性を設定する
  ****************************************************************************/
  DCB dcb;
  GetCommState(hComm, &dcb); /* DCB を取得 */
  dcb.BaudRate = BAUD_RATE;
  dcb.ByteSize = BYTE_SIZE;
  dcb.Parity = PARITY;
  dcb.StopBits = STOP_BIT;
  dcb.fParity = F_PARITY;
  SetCommState(hComm, &dcb); /* DCB を設定 */
  
  return true;
}
/******************************************************************************
ハンドルを閉じる
******************************************************************************/
void ComEnd()
{
  CloseHandle(hComm); CloseHandle(hEvent);
  isConnect = FALSE; connectIdx = -1;
}
/******************************************************************************
******************************************************************************/
bool ComCheck(void)
{
  DWORD dwErrors;  /* エラー情報 */
  COMSTAT ComStat; /* デバイスの状態 */

  ClearCommError(hComm, &dwErrors, &ComStat);
#if 0
  switch(dwErrors){
  case CE_BREAK: printf("CE_BREAK\n"); break;
  case CE_DNS: printf("CE_DNS\n"); break;
  case CE_FRAME: printf("CE_FRAME\n"); break;
  case CE_IOE: printf("CE_IOE\n"); break;
  case CE_MODE: printf("CE_MODE\n"); break;
  case CE_OOP: printf("CE_OOP\n"); break;
  case CE_OVERRUN: printf("CE_OVERRUN\n"); break;
  case CE_PTO: printf("CE_PTO\n"); break;
  case CE_RXOVER: printf("CE_RXOVER\n"); break;
  case CE_RXPARITY: printf("CE_RXPARITY\n"); break;
  case CE_TXFULL: printf("CE_TXFULL\n"); break;
  case 0: break;
  default: printf("CE_UNKNOWN %d\n", dwErrors); break;
  }
#else
  if(dwErrors != 0){ return FALSE; }
#endif
  return TRUE;
}
/******************************************************************************
******************************************************************************/
bool WriteData(unsigned char *buff, unsigned int data_size)
{
  BOOL bRet, bLError;
  DWORD dwWritten; /* ポートへ書き込んだバイト数 */
	
  if(hComm == INVALID_HANDLE_VALUE) { return false; }
  
  bRet = WriteFile(hComm, buff, data_size, &dwWritten, &overlap);

  if(!bRet){
	DWORD timeout;
	bLError = GetLastError();
	
	if(bLError == ERROR_IO_PENDING){
	  timeout = GetTickCount();
	  while(1){
		bRet = GetOverlappedResult(hComm, &overlap, &dwWritten, FALSE);
		if(bRet){ break; } // 書き込み終了
		if((GetTickCount() - timeout) > TIMEOUT){ 
		  //printf("書き込みタイムアウト\n"); 
		  return false; 
		}
	  }
	}else{
	  //printf("書き込みエラー\n");
	  return false;
	}
  }
  if (dwWritten!=data_size) {
	//printf("データの送信に失敗しました。size %d -> %d: 0x%X\n", data_size, dwWritten, buff[0]);
	return false;
  }
  return true;
}
/******************************************************************************
******************************************************************************/
int ReadData(unsigned char *buff, unsigned int max_size)
{
  BOOL bRet, bLError;
  DWORD dwErrors;  /* エラー情報 */
  COMSTAT ComStat; /* デバイスの状態 */
  DWORD dwCount;   /* 受信データのバイト数 */
  DWORD dwRead;    /* ポートから読み出したバイト数 */
  
  ClearCommError(hComm, &dwErrors, &ComStat);
  dwCount = ComStat.cbInQue;
  
  if (dwCount > max_size){ dwRead = 0; return (dwRead); }
  
  bRet = ReadFile(hComm, buff, dwCount, &dwRead, &overlap);
  if(!bRet){
	DWORD timeout;
	bLError = GetLastError();

	if(bLError == ERROR_IO_PENDING){
	  timeout = GetTickCount();
	  while(1){
		bRet = GetOverlappedResult(hComm, &overlap, &dwRead, FALSE);
		if(bRet){ break; } // 書き込み終了
		if((GetTickCount() - timeout) > TIMEOUT){ 
		  //printf("読み込みタイムアウト\n"); 
		  return false; 
		}
	  }
	}else{
	  //printf("読み込みエラー\n");
	  CloseHandle(hComm); CloseHandle(hEvent);
	  return false;
	}
  }
  
  if(dwCount != dwRead){ dwRead = 0; return (dwRead); }
  
  return dwRead;
}
/******************************************************************************
******************************************************************************/
#if 0
void ComOpen(void)
{
  ComInit(COM_PORT12); // COM12 をオープン
}
#endif
/******************************************************************************
******************************************************************************/
#if 0
void ComCheck(void)
{
  for(int i=0;i<COM_MAX;i++){ ComInit(i); }
}
#endif
