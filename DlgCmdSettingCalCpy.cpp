/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgCmdSettingCalCpy.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2022.09.12	M.Iwasaki	キャリブレーションデータコピー改善
 ** 注記：
 ** 説明：キャリブレーションデータコピー設定ダイアログ
 **		コピー先スコープ、視野の設定をする
 **/

// DlgCmdSettingCalCpy.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgCmdSettingCalCpy.h"
#include "afxdialogex.h"

extern COutputLog		outlog;				//ログ出力
extern CLanguage		language;			//言語設定用

// CDlgCmdSettingCalCpy ダイアログ

IMPLEMENT_DYNAMIC(CDlgCmdSettingCalCpy, CDialogEx)

CDlgCmdSettingCalCpy::CDlgCmdSettingCalCpy(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgCmdSettingCalCpy, pParent)
{
	//ダイアログ配置
	dlg_x = 240;
	dlg_y = 240;
	dlg_width = 256;
	dlg_height = 544;
}

CDlgCmdSettingCalCpy::~CDlgCmdSettingCalCpy()
{
}

void CDlgCmdSettingCalCpy::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, check_select_copy[0][0]);	//スコープ1
	DDX_Control(pDX, IDC_CHECK2, check_select_copy[1][0]);
	DDX_Control(pDX, IDC_CHECK3, check_select_copy[2][0]);
	DDX_Control(pDX, IDC_CHECK4, check_select_copy[3][0]);
	DDX_Control(pDX, IDC_CHECK5, check_select_copy[0][1]);	//スコープ2
	DDX_Control(pDX, IDC_CHECK6, check_select_copy[1][1]);
	DDX_Control(pDX, IDC_CHECK7, check_select_copy[2][1]);
	DDX_Control(pDX, IDC_CHECK8, check_select_copy[3][1]);
	DDX_Control(pDX, IDC_CHECK9, check_select_copy[0][2]);	//スコープ3
	DDX_Control(pDX, IDC_CHECK10, check_select_copy[1][2]);
	DDX_Control(pDX, IDC_CHECK11, check_select_copy[2][2]);
	DDX_Control(pDX, IDC_CHECK12, check_select_copy[3][2]);
	DDX_Control(pDX, IDC_CHECK13, check_select_copy[0][3]);	//スコープ4
	DDX_Control(pDX, IDC_CHECK14, check_select_copy[1][3]);
	DDX_Control(pDX, IDC_CHECK15, check_select_copy[2][3]);
	DDX_Control(pDX, IDC_CHECK16, check_select_copy[3][3]);
	DDX_Control(pDX, IDC_CHECK17, check_select_copy[0][4]);	//スコープ5
	DDX_Control(pDX, IDC_CHECK18, check_select_copy[1][4]);
	DDX_Control(pDX, IDC_CHECK19, check_select_copy[2][4]);
	DDX_Control(pDX, IDC_CHECK20, check_select_copy[3][4]);
	DDX_Control(pDX, IDC_CHECK21, check_select_copy[0][5]);	//スコープ6
	DDX_Control(pDX, IDC_CHECK22, check_select_copy[1][5]);
	DDX_Control(pDX, IDC_CHECK23, check_select_copy[2][5]);
	DDX_Control(pDX, IDC_CHECK24, check_select_copy[3][5]);
	DDX_Control(pDX, IDC_CHECK25, check_select_copy[0][6]);	//スコープ7
	DDX_Control(pDX, IDC_CHECK26, check_select_copy[1][6]);
	DDX_Control(pDX, IDC_CHECK27, check_select_copy[2][6]);
	DDX_Control(pDX, IDC_CHECK28, check_select_copy[3][6]);
	DDX_Control(pDX, IDC_CHECK29, check_select_copy[0][7]);	//スコープ8
	DDX_Control(pDX, IDC_CHECK30, check_select_copy[1][7]);
	DDX_Control(pDX, IDC_CHECK31, check_select_copy[2][7]);
	DDX_Control(pDX, IDC_CHECK32, check_select_copy[3][7]);
	DDX_Control(pDX, IDC_CHECK33, check_select_copy[0][8]);	//スコープ9
	DDX_Control(pDX, IDC_CHECK34, check_select_copy[1][8]);
	DDX_Control(pDX, IDC_CHECK35, check_select_copy[2][8]);
	DDX_Control(pDX, IDC_CHECK36, check_select_copy[3][8]);
	DDX_Control(pDX, IDC_CHECK37, check_select_copy[0][9]);	//スコープ10
	DDX_Control(pDX, IDC_CHECK38, check_select_copy[1][9]);
	DDX_Control(pDX, IDC_CHECK39, check_select_copy[2][9]);
	DDX_Control(pDX, IDC_CHECK40, check_select_copy[3][9]);
	DDX_Control(pDX, IDC_CHECK41, check_select_copy[0][10]);	//スコープ11
	DDX_Control(pDX, IDC_CHECK42, check_select_copy[1][10]);
	DDX_Control(pDX, IDC_CHECK43, check_select_copy[2][10]);
	DDX_Control(pDX, IDC_CHECK44, check_select_copy[3][10]);
	DDX_Control(pDX, IDC_CHECK45, check_select_copy[0][11]);	//スコープ12
	DDX_Control(pDX, IDC_CHECK46, check_select_copy[1][11]);
	DDX_Control(pDX, IDC_CHECK47, check_select_copy[2][11]);
	DDX_Control(pDX, IDC_CHECK48, check_select_copy[3][11]);
	DDX_Control(pDX, IDC_CHECK49, check_select_copy[0][12]);	//スコープ13
	DDX_Control(pDX, IDC_CHECK50, check_select_copy[1][12]);
	DDX_Control(pDX, IDC_CHECK51, check_select_copy[2][12]);
	DDX_Control(pDX, IDC_CHECK52, check_select_copy[3][12]);
	DDX_Control(pDX, IDC_CHECK53, check_select_copy[0][13]);	//スコープ14
	DDX_Control(pDX, IDC_CHECK54, check_select_copy[1][13]);
	DDX_Control(pDX, IDC_CHECK55, check_select_copy[2][13]);
	DDX_Control(pDX, IDC_CHECK56, check_select_copy[3][13]);
	DDX_Control(pDX, IDC_CHECK57, check_select_copy[0][14]);	//スコープ15
	DDX_Control(pDX, IDC_CHECK58, check_select_copy[1][14]);
	DDX_Control(pDX, IDC_CHECK59, check_select_copy[2][14]);
	DDX_Control(pDX, IDC_CHECK60, check_select_copy[3][14]);
	DDX_Control(pDX, IDC_CHECK61, check_select_copy[0][15]);	//スコープ16
	DDX_Control(pDX, IDC_CHECK62, check_select_copy[1][15]);
	DDX_Control(pDX, IDC_CHECK63, check_select_copy[2][15]);
	DDX_Control(pDX, IDC_CHECK64, check_select_copy[3][15]);
}


BEGIN_MESSAGE_MAP(CDlgCmdSettingCalCpy, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCmdSettingCalCpy::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgCmdSettingCalCpy::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgCmdSettingCalCpy メッセージ ハンドラー


BOOL CDlgCmdSettingCalCpy::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgCmdSettingCalCpy in ***");

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
void CDlgCmdSettingCalCpy::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingCalCpy::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			BOOL check = edit_param->custom_copy_scope[i].view[j];
			check_select_copy[j][i].SetCheck(check);
		}
	}
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingCalCpy::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 4; j++) {
			BOOL check = check_select_copy[j][i].GetCheck();
			edit_param->custom_copy_scope[i].view[j] = check;
		}
	}
}


/** ****************************************************** **
 ** パラメータセット
 ** 引数：
 **　CCommands::CalibParameter* param	キャリブレーションパラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingCalCpy::SetParam(CCommands::CalibParameter* param)
{
	// TODO: ここに実装コードを追加します.
	edit_param = param;
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingCalCpy::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ChangeParameter();	//パラメータの変更

	outlog.LogOutFile(1, "*** DlgCmdSettingCalCpy out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingCalCpy::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgCmdSettingCalCpy out (CANCEL) ***");

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
BOOL CDlgCmdSettingCalCpy::PreTranslateMessage(MSG* pMsg)
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
