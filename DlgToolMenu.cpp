/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgToolMenu.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：ツールメニュー
 **		ツールのメニュー
 **/

// DlgToolMenu.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgToolMenu.h"
#include "afxdialogex.h"

#include "DlgToolLayer.h"
#include "DlgToolMeas.h"

extern CLanguage			language;			//言語設定用
// CDlgToolMenu ダイアログ

IMPLEMENT_DYNAMIC(CDlgToolMenu, CDialogEx)

CDlgToolMenu::CDlgToolMenu(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgToolMenu, pParent)
{
	pParam_main = NULL;
}

CDlgToolMenu::~CDlgToolMenu()
{
}

void CDlgToolMenu::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, btn_layer_draw);
	DDX_Control(pDX, IDC_BUTTON2, btn_measure);
	DDX_Control(pDX, IDOK, btn_exit);
}


BEGIN_MESSAGE_MAP(CDlgToolMenu, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgToolMenu::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgToolMenu::OnBnClickedButton2)
	ON_BN_CLICKED(IDOK, &CDlgToolMenu::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgToolMenu メッセージ ハンドラー


BOOL CDlgToolMenu::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	ChangeLanguage();

	dlg_x = 80;
	dlg_y = 80;
	dlg_width = 128;	//DLU 64*2
	dlg_height = 272;	//DLU 160*1.6+16
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMenu::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(401));
		btn_layer_draw.SetWindowTextA(language.GetCharacterString(402));	//描画レイヤボタン
		btn_measure.SetWindowTextA(language.GetCharacterString(403));		//計測ボタン
		btn_exit.SetWindowTextA(language.GetCharacterString(404));			//戻るボタン
	}
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMenu::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
}


/** ****************************************************** **
 ** 描画レイヤボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMenu::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDlgToolLayer dlg_tool_layer;
	dlg_tool_layer.SetParam(pParam_main);	//パラメータのセット
	dlg_tool_layer.DoModal();
}


/** ****************************************************** **
 ** 計測ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMenu::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDlgToolMeas dlg_tool_meas;
	dlg_tool_meas.SetParam(pParam_main);	//パラメータのセット
	dlg_tool_meas.DoModal();
}


/** ****************************************************** **
 ** 戻るボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMenu::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgToolMenu::PreTranslateMessage(MSG* pMsg)
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
