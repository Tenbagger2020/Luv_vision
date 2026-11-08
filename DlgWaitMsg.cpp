/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgWaitMsg.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2022.09.12	M.Iwasaki	処理待ちメッセージ追加
 ** 注記：
 ** 説明：処理待ちを案内するダイアログ
 **		メッセージにてにて案内を表示する
 **/

// DlgWaitMsg.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgWaitMsg.h"
#include "afxdialogex.h"

extern CLanguage		language;			//言語設定用

// CDlgWaitMsg ダイアログ

IMPLEMENT_DYNAMIC(CDlgWaitMsg, CDialogEx)

CDlgWaitMsg::CDlgWaitMsg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgWaitMsg, pParent)
{
	//ダイアログ配置
	dlg_x = 240;
	dlg_y = 240;
	dlg_width = 360;
	dlg_height = 80;

	msg_guide = Tips0.SelStr(language.jpn, "処理しています。しばらくお待ち下さい", "I'm processing. Please wait");
	title_window = Tips0.SelStr(language.jpn, "処理中", "Processing");
	count_dust = 0;
}

CDlgWaitMsg::~CDlgWaitMsg()
{
}

void CDlgWaitMsg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_GUIDE, static_msg_guide);
}


BEGIN_MESSAGE_MAP(CDlgWaitMsg, CDialogEx)
END_MESSAGE_MAP()


// CDlgWaitMsg メッセージ ハンドラー


BOOL CDlgWaitMsg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);
	this->SetWindowTextA(title_window);

	ControlInitialization();	//ダイアログ操作準備

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
void CDlgWaitMsg::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** メッセージ入力
 ** 引数：
 **　CString msg	メッセージ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWaitMsg::SetMessage(CString msg)
{
	// TODO: ここに実装コードを追加します.
	msg_guide = msg;
}


/** ****************************************************** **
 ** タイトル入力
 ** 引数：
 **　CString title	タイトル
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWaitMsg::SetTitle(CString title)
{
	// TODO: ここに実装コードを追加します.
	title_window = title;
}


/** ****************************************************** **
 ** メッセージ更新
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWaitMsg::UpdateMsg()
{
	// TODO: ここに実装コードを追加します.

	//埃が出てくる
	CString str_dust = "";
	for (int i = 0; i < count_dust; i++) {
		str_dust = str_dust + ".";
	}
	count_dust++;
	if (count_dust > 10) count_dust = 0;

	//メッセージ表示
	static_msg_guide.SetWindowTextA(msg_guide + str_dust);
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgWaitMsg::PreTranslateMessage(MSG* pMsg)
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
