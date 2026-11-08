/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgPLCOutFormula.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：計算検査PLC出力ダイアログ
 **		計算検査のPLC出力を設定する
 **
 **/

// DlgPLCOutFormula.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgPLCOutFormula.h"
#include "afxdialogex.h"

extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

// CDlgPLCOutFormula ダイアログ

IMPLEMENT_DYNAMIC(CDlgPLCOutFormula, CDialogEx)

CDlgPLCOutFormula::CDlgPLCOutFormula(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgPLCOutFormula, pParent)
{
	sheet_number = 0;
}

CDlgPLCOutFormula::~CDlgPLCOutFormula()
{
}

void CDlgPLCOutFormula::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, combo_set_result[0]);
	DDX_Control(pDX, IDC_COMBO2, combo_set_result[1]);
	DDX_Control(pDX, IDC_COMBO3, combo_set_result[2]);
}


BEGIN_MESSAGE_MAP(CDlgPLCOutFormula, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgPLCOutFormula::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgPLCOutFormula::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgPLCOutFormula メッセージ ハンドラー


BOOL CDlgPLCOutFormula::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgPLCOutFormula in ***");

	//タイトルの更新
	CString title;
	this->GetWindowTextA(title);
	this->SetWindowTextA(title + Tips0.SelChar(language.jpn, " (シート", " (Sheet") + Tips0.IntToString(sheet_number) + ")");

	dlg_x = 496;
	dlg_y = 160;
	dlg_width = 240;
	dlg_height = 240;
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
void CDlgPLCOutFormula::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//計測距離
	for (int j = 0; j < 3; j++) {
		combo_set_result[j].AddString(Tips0.SelChar(language.jpn, "無し", "None"));
		for (int i = 0; i < 20; i++) {
			combo_set_result[j].AddString(Tips0.SelChar(language.jpn, "アドレス", "Address") + Tips0.IntToString(i));
		}
	}
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPLCOutFormula::ChangeLanguage()
{
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPLCOutFormula::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//計算結果
	for (int i = 0; i < 3; i++) {
		int num_cursel = edit_param->out_mes_address.formula_result[i] + 1;
		combo_set_result[i].SetCurSel(num_cursel);
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
void CDlgPLCOutFormula::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//計算結果
	for (int i = 0; i < 3; i++) {
		int num_param = combo_set_result[i].GetCurSel() - 1;
		edit_param->out_mes_address.formula_result[i] = num_param;
	}
}


/** ****************************************************** **
 ** パラメータ確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:アドレス設定NG
 ** 特記事項：
 ** ****************************************************** **/
int CDlgPLCOutFormula::CheckParam()
{
	// TODO: ここに実装コードを追加します.

	//計算結果
	//　値はDWORDなのでアドレス16もNG
	for (int i = 0; i < 3; i++) {
		int num_param = combo_set_result[i].GetCurSel() - 1;
		if (num_param == 0 || num_param == 1 || num_param >= 16) return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** PLCパラメータセット
 ** 引数：
 **　int number		シート番号(0～7)
 **　CPLCParamFormula* param	PLCパラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPLCOutFormula::SetPLCParam(int number, CPLCParamFormula* param)
{
	// TODO: ここに実装コードを追加します.
	sheet_number = number;
	edit_param = param;
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPLCOutFormula::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);

	//パラメータチェック
	int rtn_chk = CheckParam();
	if (rtn_chk == 1) {
		msg = Tips0.SelStr(language.jpn, "計算結果のアドレス範囲は2～15", "Address range of the result is 2 to 15");
		MessageBoxA(msg, title, MB_OK | MB_ICONWARNING);
		return;
	}

	ChangeParameter();	//パラメータの変更

	outlog.LogOutFile(1, "*** DlgPLCOutFormula out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPLCOutFormula::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgPLCOutFormula out (CANCEL) ***");

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
BOOL CDlgPLCOutFormula::PreTranslateMessage(MSG* pMsg)
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
