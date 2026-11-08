/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgInitOutResult.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：結果出力設定
 **		テンプレート毎の結果出力設定についてシステムで設定するもの
 **/

// DlgInitOutResult.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgInitOutResult.h"
#include "afxdialogex.h"

extern COutResult		sys_outrslt;		//結果出力パラメータ
extern CAccessParamSys	access_sys;			//システムIniファイルアクセス
extern COutputLog		outlog;				//ログ出力

// CDlgInitOutResult ダイアログ

IMPLEMENT_DYNAMIC(CDlgInitOutResult, CDialogEx)

CDlgInitOutResult::CDlgInitOutResult(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgInitOutResult, pParent)
{
	//ダイアログ配置
	dlg_x = 240;
	dlg_y = 100;
	dlg_width = 480;
	dlg_height = 480;
}

CDlgInitOutResult::~CDlgInitOutResult()
{
}

void CDlgInitOutResult::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, tab_select);
}


BEGIN_MESSAGE_MAP(CDlgInitOutResult, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgInitOutResult::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitOutResult::OnBnClickedCancel)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgInitOutResult メッセージ ハンドラー


BOOL CDlgInitOutResult::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgInitOutResult in ***");

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
void CDlgInitOutResult::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//タブコントロール準備
	ControlInitialTab();
}


/** ****************************************************** **
 ** タブコントロール準備
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitOutResult::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//パラメータ切替タブ
	tab_select.InsertItem(0, _T("オリジナル記号"));
	tab_select.SetItemSize(CSize(80, 20));		//タブサイズ ※プロパティStyleを固定幅に設定

	CRect r_tab;
	tab_select.GetClientRect(&r_tab);	//タブコントロールのサイズを取得
	tab_select.AdjustRect(FALSE, &r_tab);

	//オリジナル記号タブダイアログの配置
	dlg_TabOrg = new CTabInitOrgFormat(&tab_select);
	dlg_TabOrg->Create(IDD_TabInitOrgFormat, &tab_select);
	dlg_TabOrg->MoveWindow(&r_tab, TRUE);

	tab_select.SetCurSel(0);
	dlg_TabOrg->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitOutResult::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	for (int i = 0; i < 10; i++) {
		dlg_TabOrg->edit_symbol[i].SetWindowTextA(sys_outrslt.org_format[i].symbol);		//識別文字
		dlg_TabOrg->edit_content[i].SetWindowTextA(sys_outrslt.org_format[i].content);		//記号説明
		dlg_TabOrg->edit_init_val[i].SetWindowTextA(sys_outrslt.org_format[i].initial_val);	//初期文字列
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
void CDlgInitOutResult::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	char buf[128] = { '\0' };

	for (int i = 0; i < 10; i++) {
		dlg_TabOrg->edit_symbol[i].GetWindowTextA(buf, sizeof(buf));	//識別文字
		strcpy_s(sys_outrslt.org_format[i].symbol, 128, buf);
		dlg_TabOrg->edit_content[i].GetWindowTextA(buf, sizeof(buf));	//記号説明
		strcpy_s(sys_outrslt.org_format[i].content, 128, buf);
		dlg_TabOrg->edit_init_val[i].GetWindowTextA(buf, sizeof(buf));	//初期文字列
		strcpy_s(sys_outrslt.org_format[i].initial_val, 128, buf);
	}

	//Iniファイル書き込み
	access_sys.WriteIniFileOutRslt();
}


/** ****************************************************** **
 ** パラメータ確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:識別文字は2文字で設定してください
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitOutResult::CheckParam()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char buf[128] = { '\0' };
	for (int j = 0; j < 10; j++) {

		//識別文字
		dlg_TabOrg->edit_symbol[j].GetWindowTextA(buf, sizeof(buf));	//識別文字
		int len = strlen(buf);
		if ((len > 0) && (len != 2)) {
			goto END;
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitOutResult::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	this->GetWindowTextA(title);

	//パラメータ確認
	int rtn_chk = CheckParam();
	if (rtn_chk == 0) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgInitOutResult out (OK) ***");

		CDialogEx::OnOK();
	}
	else if (rtn_chk == 1) {
		MessageBoxA("識別文字は2文字で設定してください", title, MB_OK | MB_ICONWARNING);
	}

}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitOutResult::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgInitOutResult out (CANCEL) ***");

	CDialogEx::OnCancel();
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitOutResult::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	dlg_TabOrg->DestroyWindow();
	delete dlg_TabOrg;
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgInitOutResult::PreTranslateMessage(MSG* pMsg)
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
