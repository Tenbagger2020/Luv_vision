/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgProgressbar.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：処理経過を表示するダイアログ
 **		プログレスバーにて経過を表示する
 **/

 // DlgProgressbar.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgProgressbar.h"
#include "afxdialogex.h"


// CDlgProgressbar ダイアログ

IMPLEMENT_DYNAMIC(CDlgProgressbar, CDialogEx)

CDlgProgressbar::CDlgProgressbar(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgProgressbar, pParent)
	, disp_message(_T(""))
{
	range_upper = 100;
}

CDlgProgressbar::~CDlgProgressbar()
{
}

void CDlgProgressbar::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC1, disp_message);
	DDX_Control(pDX, IDC_PROGRESS1, ctrl_progress);
}


BEGIN_MESSAGE_MAP(CDlgProgressbar, CDialogEx)
END_MESSAGE_MAP()


// CDlgProgressbar メッセージ ハンドラー


BOOL CDlgProgressbar::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	dlg_x = 332;
	dlg_y = 334;
	dlg_width = 360;
	dlg_height = 100;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	//ダイアログ操作準備
	ctrl_progress.SetRange(0, range_upper);
	ctrl_progress.SetPos(0);

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


/** ****************************************************** **
 ** メッセージの設定
 ** 引数：
 **　CString msg
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgProgressbar::SetMessage(CString msg)
{
	disp_message = msg;
}


/** ****************************************************** **
 ** バーの上限設定
 ** 引数：
 **　short upper
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgProgressbar::SetBarUpper(short upper)
{
	range_upper = upper;
}


/** ****************************************************** **
 ** バーの位置設定
 ** 引数：
 **　int pos
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgProgressbar::SetBarPos(int pos)
{
	if (pos <= range_upper) ctrl_progress.SetPos(pos);
}
