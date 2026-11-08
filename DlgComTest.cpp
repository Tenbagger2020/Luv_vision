/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgComTest.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2023.10.30	M.Iwasaki	10.14.2.0	東芝テリーUSBカメラ対応
 ** 注記：
 ** 説明：通信テストダイアログ
 **		通信テストダイアログを表示する
 **/

// DlgComTest.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgComTest.h"
#include "afxdialogex.h"

#include "CommData.h"	//シリアル通信データクラス

extern CDioControl			dio;		//デジタルI/O制御
extern CPLCCommunication	plc_com;	//シリアル通信
extern COutputLog			outlog;		//ログ出力
extern CLanguage			language;	//言語設定用

// CDlgComTest ダイアログ

IMPLEMENT_DYNAMIC(CDlgComTest, CDialogEx)

CDlgComTest::CDlgComTest(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgComTest, pParent)
{

}

CDlgComTest::~CDlgComTest()
{
}

void CDlgComTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, tab_select_test);
}


BEGIN_MESSAGE_MAP(CDlgComTest, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgComTest::OnBnClickedOk)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDlgComTest::OnTcnSelchangeTab1)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgComTest メッセージ ハンドラー


BOOL CDlgComTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgComTest in ***");

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = 496;
	dlg_height = 560;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	LoadParameter();	//パラメータの展開

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


/** ****************************************************** **
 ** ダイアログ操作準備
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();
	//タブコントロール準備
	ControlInitialTab();
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2101));
	}
}


/** ****************************************************** **
 ** タブコントロール準備
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//テスト切替タブ
	tab_select_test.InsertItem(0, Tips0.SelChar(language.jpn, "パラレルI/O", "Parallel"));
	tab_select_test.InsertItem(1, Tips0.SelChar(language.jpn, "シリアル通信", "Serial"));

	tab_select_test.SetItemSize(CSize(100, 20));	//タブサイズ ※プロパティStyleを固定幅に設定
	CRect r_tab;
	tab_select_test.GetClientRect(&r_tab);			//タブコントロールのサイズを取得
	tab_select_test.AdjustRect(FALSE, &r_tab);

	//パラレルI/Oテスト タブダイアログの配置
	dlg_TabTestIO = new CTabTestIO(&tab_select_test);		//ウィンドウを配置
	dlg_TabTestIO->Create(IDD_TabTestIO, &tab_select_test);
	dlg_TabTestIO->MoveWindow(&r_tab, TRUE);

	//シリアル通信テスト タブダイアログの配置
	dlg_TabTestCom = new CTabTestCom(&tab_select_test);		//ウィンドウを配置
	dlg_TabTestCom->Create(IDD_TabTestCom, &tab_select_test);
	dlg_TabTestCom->MoveWindow(&r_tab, TRUE);

	tab_select_test.SetCurSel(0);
	dlg_TabTestIO->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//入力情報
	GetSignalIn();	//入力信号取得

	//出力情報
	for (short i = 0; i < 16; i++) {
		EchoSignalOut(i);	//出力信号読取
	}

	dlg_TabTestIO->Invalidate();
}


/** ****************************************************** **
 ** 入力信号取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::GetSignalIn()
{
	// TODO: ここに実装コードを追加します.
	for (short i = 0; i < 16; i++) {
		byte status;
		dio.InBit(i, &status);
		if (status == 0x01) {
			dlg_TabTestIO->status_in[i] = TRUE;
		}
		else {
			dlg_TabTestIO->status_in[i] = FALSE;
		}
	}
}


/** ****************************************************** **
 ** 出力信号変更
 ** 引数：
 **　short bit	出力書込ビットNo
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::SetSignalOut(short bit)
{
	// TODO: ここに実装コードを追加します.

	byte out = 0x00;
	if (!dlg_TabTestIO->status_out[bit]) out = 0x01;

	dio.OutBit(bit, out);

	EchoSignalOut(bit);	//出力信号読取
}


/** ****************************************************** **
 ** 出力信号読取
 ** 引数：
 **　short bit	出力書込ビットNo
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::EchoSignalOut(short bit)
{
	// TODO: ここに実装コードを追加します.

	byte status;
	dio.EchoBit(bit, &status);
	if (status == 0x01) {
		dlg_TabTestIO->status_out[bit] = TRUE;
	}
	else {
		dlg_TabTestIO->status_out[bit] = FALSE;
	}
}


/** ****************************************************** **
 ** データ読込
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::DataRead()
{
	// TODO: ここに実装コードを追加します.
	CString title, msg;
	this->GetWindowTextA(title);

	CCommData read_data;	

	//シーケンサからデータ読込
	if (plc_com.ReadDataWord(&read_data, TRUE) == 0) {

		dlg_TabTestCom->SetReadData(read_data);		//読込データを展開
	}
	else {
		msg = Tips0.SelStr(language.jpn, "データ読み込みに失敗しました", "Failed to read data");
		MessageBoxA(msg, title, MB_OK | MB_ICONERROR);
	}

}


/** ****************************************************** **
 ** データ書込
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::DataWrite()
{
	// TODO: ここに実装コードを追加します.
	CString title, msg;
	this->GetWindowTextA(title);

	CCommData write_data;

	//書込データを取得
	if (dlg_TabTestCom->GetWriteData(&write_data) == 0) {
		//シーケンサへデータ書込
		if (plc_com.WriteDataWord(&write_data, TRUE) != 0) {
			msg = Tips0.SelStr(language.jpn, "データ書き込みに失敗しました", "Data write failed");
			MessageBoxA(msg, title, MB_OK | MB_ICONERROR);
		}
	}
	else {
		msg = Tips0.SelStr(language.jpn, "書込データを-32768～32767の範囲で設定してください", "Set the write data in the range of -32768 to 32767");
		MessageBoxA(msg, title, MB_OK | MB_ICONWARNING);
	}
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgComTest out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** テスト切替タブSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	switch (tab_select_test.GetCurSel()) {
		//パラレルI/Oを表示
	case 0:
		dlg_TabTestIO->ShowWindow(SW_SHOW);
		dlg_TabTestCom->ShowWindow(SW_HIDE);
		break;
		//シリアル通信を表示
	case 1:
		dlg_TabTestIO->ShowWindow(SW_HIDE);
		dlg_TabTestCom->ShowWindow(SW_SHOW);
		break;
	}
}


/** ****************************************************** **
 ** イベントメッセージ処理
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgComTest::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	//パラレルI/Oタブ(CTabTestIO)からのボタンClickedを受信し処理する
	//シリアル通信タブ(CTabTestCom)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//パラレルI/Oタブの読込ボタンClickedを受信
	case IDC_BUTTON1_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			GetSignalIn();	//入力信号取得
			dlg_TabTestIO->Invalidate();
		}
		break;
		//パラレルI/Oタブの出力切替ボタンClickedを受信
	case IDC_BUTTON2_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(0);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON3_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(1);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON4_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(2);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON5_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(3);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON6_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(4);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON7_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(5);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON8_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(6);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON9_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(7);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON10_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(8);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON11_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(9);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON12_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(10);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON13_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(11);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON14_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(12);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON15_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(13);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON16_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(14);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
	case IDC_BUTTON17_TabTestIO:
		if (HIWORD(wParam) == BN_CLICKED) {
			SetSignalOut(15);	//出力信号変更
			dlg_TabTestIO->Invalidate();
		}
		break;
		//シリアル通信タブの読込ボタンClickedを受信
	case IDC_BUTTON1_TabTestCom:
		if (HIWORD(wParam) == BN_CLICKED) {
			DataRead();		//データ読込
		}
		break;
		//シリアル通信タブの書込ボタンClickedを受信
	case IDC_BUTTON2_TabTestCom:
		if (HIWORD(wParam) == BN_CLICKED) {
			DataWrite();	//データ書込
		}
		break;
	}

	return CDialogEx::OnCommand(wParam, lParam);
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgComTest::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	dlg_TabTestIO->DestroyWindow();
	delete dlg_TabTestIO;
	dlg_TabTestCom->DestroyWindow();
	delete dlg_TabTestCom;
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgComTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	if (WM_KEYDOWN == pMsg->message) {
		switch (pMsg->wParam) {
		case VK_RETURN:		//Enterキーで画面を閉じない
			return FALSE;
		case VK_ESCAPE:		//ESCキーで画面を閉じない
			return FALSE;
		default:
			break;
		}
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
