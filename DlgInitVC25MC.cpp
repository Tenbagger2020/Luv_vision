/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgInitVC25MC.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：VC-25MC-M30D0カメラパラメータ設定ダイアログ
 **		VIEWORKS VC-25MC-M30D0のパラメータ設定を行う
 **/

// DlgInitVC25MC.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgInitVC25MC.h"
#include "afxdialogex.h"

extern CCameraSettings		cam_parameter[8];	//カメラパラメータ
extern COutputLog			outlog;				//ログ出力

// CDlgInitVC25MC ダイアログ

IMPLEMENT_DYNAMIC(CDlgInitVC25MC, CDialogEx)

CDlgInitVC25MC::CDlgInitVC25MC(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgInitVC25MC, pParent)
{
	cam_number = 0;
}

CDlgInitVC25MC::~CDlgInitVC25MC()
{
}

void CDlgInitVC25MC::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, combo_com_port);
	DDX_Control(pDX, IDC_EDIT1, edit_exposure_time);
}


BEGIN_MESSAGE_MAP(CDlgInitVC25MC, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgInitVC25MC::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CDlgInitVC25MC::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitVC25MC::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInitVC25MC メッセージ ハンドラー


BOOL CDlgInitVC25MC::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgInitVC25MC in ***");

	dlg_x = 512;
	dlg_y = 384;
	dlg_width = 320;
	dlg_height = 320;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	LoadComParam();		//通信パラメータの展開
	LoadRegiParam();	//内部パラメータの展開

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
void CDlgInitVC25MC::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	for (int i = 1; i <= 24; i++) {
		combo_com_port.AddString("COM" + Tips0.IntToString(i));
	}
}


/** ****************************************************** **
 ** 通信パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::LoadComParam()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	int com_port = regi_param->com_port;
	if (com_port == 0) {
		//初期設定ではカメラ1～8がCOM9～16に設定する
		combo_com_port.SetCurSel(cam_number + 8);
	}
	else {
		combo_com_port.SetCurSel(com_port - 1);
	}
}


/** ****************************************************** **
 ** 内部パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::LoadRegiParam()
{
	// TODO: ここに実装コードを追加します.

	//露光時間
	int load_msec = regi_param->vc25mc.exposure_time / 1000;
	edit_exposure_time.SetWindowTextA(Tips0.IntToString(load_msec));
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	regi_param->com_port = combo_com_port.GetCurSel() + 1;

	//露光時間
	char buf[256] = { '\0' };
	edit_exposure_time.GetWindowTextA(buf, sizeof(buf));
	int change_msec = atoi(buf);
	regi_param->vc25mc.exposure_time = change_msec * 1000;
}


/** ****************************************************** **
 ** パラメータ確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:未設定、2:露光時間設定範囲外
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitVC25MC::CheckParam()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//設定の確認
	char buf[256] = { '\0' };

	//露光時間
	edit_exposure_time.GetWindowTextA(buf, sizeof(buf));
	int change_msec = atoi(buf);
	if (change_msec < 1 || change_msec > 1000) return 2;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　int number		カメラ番号(0～3)
 **　CCameraRegister* edit_param	編集パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::SetParam(int number, CCameraRegister* edit_param)
{
	// TODO: ここに実装コードを追加します.
	cam_number = number;
	regi_param = edit_param;
}


/** ****************************************************** **
 ** 初期値に戻すボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	regi_param->InitVC25MC();

	LoadRegiParam();	//内部パラメータの展開
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	this->GetWindowTextA(title);

	//パラメータ確認
	int rtn_chk = CheckParam();
	if (rtn_chk == 0) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgInitVC25MC out (OK) ***");

		CDialogEx::OnOK();
	}
	else if (rtn_chk == 1) {
		MessageBoxA("設定を完了させてください", title, MB_OK | MB_ICONWARNING);
	}
	else if (rtn_chk == 2) {
		MessageBoxA("露光時間は1～1000msの範囲で設定してください", title, MB_OK | MB_ICONWARNING);
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitVC25MC::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgInitVC25MC out (CANCEL) ***");

	CDialogEx::OnCancel();
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgInitVC25MC::PreTranslateMessage(MSG* pMsg)
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
