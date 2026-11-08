/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgSystemInfomation.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：システム情報ダイアログ
 **		システムの情報を表示する
 **/

// DlgSystemInfomation.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgSystemInfomation.h"
#include "afxdialogex.h"

extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;				//言語設定用

// CDlgSystemInfomation ダイアログ

IMPLEMENT_DYNAMIC(CDlgSystemInfomation, CDialogEx)

CDlgSystemInfomation::CDlgSystemInfomation(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgSystemInfomation, pParent)
{

}

CDlgSystemInfomation::~CDlgSystemInfomation()
{
}

void CDlgSystemInfomation::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_FILE, org_filename);
	DDX_Control(pDX, IDC_STATIC_PROD_NAME, product_name);
	DDX_Control(pDX, IDC_STATIC_PROD_VER, product_version);
	DDX_Control(pDX, IDC_STATIC_COPYRIGHT, copyright);
}


BEGIN_MESSAGE_MAP(CDlgSystemInfomation, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgSystemInfomation メッセージ ハンドラー


BOOL CDlgSystemInfomation::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ChangeLanguage();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgSystemInfomation in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = r_window.right - r_window.left;
	dlg_height = r_window.bottom - r_window.top;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	LoadParameter();	//パラメータの展開

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
void CDlgSystemInfomation::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1801));
	}
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgSystemInfomation::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	TCHAR filename[MAX_PATH + 1];
	DWORD size;
	BYTE* pVersion;

	UINT queryLen;
	WORD* pLangCode;
	CString subBlock;
	TCHAR* pStrInfo;

	::GetModuleFileNameA(NULL, filename, sizeof(filename));

	size = ::GetFileVersionInfoSizeA(filename, NULL);
	pVersion = new BYTE[size];
	if (::GetFileVersionInfoA(filename, NULL, size, pVersion)) {
		::VerQueryValueA(pVersion, _T("\\VarFileInfo\\Translation"), (void**)&pLangCode, &queryLen);

		subBlock.Format(_T("\\StringFileInfo\\%04x%04x\\"), pLangCode[0], pLangCode[1]);

		//ファイル名取得
		::VerQueryValueA(pVersion, (LPTSTR)(LPCTSTR)(subBlock + "originalFilename"), (void**)&pStrInfo, &queryLen);
		if (0 < queryLen) org_filename.SetWindowTextA(pStrInfo);

		//製品名取得
		::VerQueryValueA(pVersion, (LPTSTR)(LPCTSTR)(subBlock + "ProductName"), (void**)&pStrInfo, &queryLen);
		if (0 < queryLen) product_name.SetWindowTextA(pStrInfo);

		//バージョン取得
		::VerQueryValueA(pVersion, (LPTSTR)(LPCTSTR)(subBlock + "ProductVersion"), (void**)&pStrInfo, &queryLen);
		if (0 < queryLen) product_version.SetWindowTextA(pStrInfo);

		//著作権取得
		::VerQueryValueA(pVersion, (LPTSTR)(LPCTSTR)(subBlock + "LegalCopyright"), (void**)&pStrInfo, &queryLen);
		if (0 < queryLen) copyright.SetWindowTextA(pStrInfo);
	}

	delete[] pVersion;
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgSystemInfomation::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	outlog.LogOutFile(1, "*** DlgSystemInfomation out ***");
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgSystemInfomation::PreTranslateMessage(MSG* pMsg)
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
