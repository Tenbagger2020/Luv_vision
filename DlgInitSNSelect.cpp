/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgInitSNSelect.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2023.10.30	M.Iwasaki	10.14.2.0	初版
 ** 注記：
 ** 説明：USBカメラシリアル番号選択ダイアログ
 **		東芝テリーUSBカメラのシリアル選択を行う
 **
 **/

// DlgInitSNSelect.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgInitSNSelect.h"
#include "afxdialogex.h"

extern CUsbControl			usb_grab;			//USBカメラ 制御メイン
extern COutputLog			outlog;				//ログ出力

// CDlgInitSNSelect ダイアログ

IMPLEMENT_DYNAMIC(CDlgInitSNSelect, CDialogEx)

CDlgInitSNSelect::CDlgInitSNSelect(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgInitSNSelect, pParent)
{
	select_serial = NULL;
	vender_code = 0x00;
	model_name = NULL;
}

CDlgInitSNSelect::~CDlgInitSNSelect()
{
}

void CDlgInitSNSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, list_sn);
}


BEGIN_MESSAGE_MAP(CDlgInitSNSelect, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgInitSNSelect::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitSNSelect::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInitSNSelect メッセージ ハンドラー


BOOL CDlgInitSNSelect::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgInitSNSelect in ***");

	dlg_x = 704;
	dlg_y = 448;
	dlg_width = 208;
	dlg_height = 208;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	LoadSerial();	//シリアル番号展開

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
void CDlgInitSNSelect::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** モデル名に一致するシリアル番号を展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSNSelect::LoadSerial()
{
	// TODO: ここに実装コードを追加します.

	//USBシリアル番号群の取得
	char serial_list[256] = { '\0' };
	int quantity = 0;
	if (usb_grab.GetSerials(serial_list, &quantity, vender_code, model_name) == 0) {
		char* serial_one = NULL;
		char* delim = ",";
		char* ctx = NULL;
		if (quantity > 0) {
			//1個目リストに追加
			serial_one = strtok_s(serial_list, delim, &ctx);
			list_sn.AddString(serial_one);

			//2個目以降リストに追加
			for (int i = 1; i < quantity; i++) {
				serial_one = strtok_s(NULL, delim, &ctx);
				list_sn.AddString(serial_one);
			}
		}
	}
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　char* s_number		選択したシリアル番号
 **　byte v_code		カメラのベンダーコード
 **　char* m_name		カメラのモデル名称
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSNSelect::SetParam(char* s_number, byte v_code, char* m_name)
{
	// TODO: ここに実装コードを追加します.
	select_serial = s_number;
	vender_code = v_code;
	model_name = m_name;
}


/** ****************************************************** **
 ** 選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSNSelect::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//選択されていれば文字列を取得する
	int item = list_sn.GetCurSel();
	if (item >= 0) {
		list_sn.GetText(item, select_serial);
	}

	outlog.LogOutFile(1, "*** DlgInitSNSelect out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSNSelect::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgInitSNSelect out (CANCEL) ***");

	CDialogEx::OnCancel();
}
