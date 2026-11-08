/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgListTemplateMain.h
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：機種データ テンプレート一覧ダイアログ
 **		テンプレート一覧タブ表示、テンプレートコピー
 **/

// DlgListTemplateMain.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgListTemplateMain.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"
#include "DlgPreviewPattern.h"
#include "DlgListTemplateOpe.h"

extern CTemplates	sys_template[256];	//システムテンプレートパラメータ
extern CScopes		sys_scope[16];		//システムスコープパラメータ
extern CTemplatesGroup		grp_template;		//テンプレートグループパラメータ
extern COutputLog	outlog;				//ログ出力
extern CLanguage		language;		//言語設定用

// CDlgListTemplateMain ダイアログ

IMPLEMENT_DYNAMIC(CDlgListTemplateMain, CDialogEx)

CDlgListTemplateMain::CDlgListTemplateMain(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgListTemplateMain, pParent)
{
	select_template = 0;	//選択番号初期化
	group_val = 0;			//グループ数
}

CDlgListTemplateMain::~CDlgListTemplateMain()
{
}

void CDlgListTemplateMain::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, tab_select_list);
	DDX_Control(pDX, IDC_STATIC_EXPLAN, static_explan);
}


BEGIN_MESSAGE_MAP(CDlgListTemplateMain, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgListTemplateMain::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDlgListTemplateMain::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CDlgListTemplateMain メッセージ ハンドラー


BOOL CDlgListTemplateMain::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgListTemplateMain in ***");

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = 944;
	dlg_height = 464;
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
void CDlgListTemplateMain::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//グループ数の確認
	for (int i = 0; i < 4; i++) {
		if (grp_template.group[i].first_number < 0 || grp_template.group[i].first_number > 255) break;	//テンプレートNo範囲外
		if (grp_template.group[i].end_number < 0 || grp_template.group[i].end_number > 255) break;		//テンプレートNo範囲外
		if (grp_template.group[i].first_number > grp_template.group[i].end_number) break;				//テンプレートNo範囲NG
		group_val++;
	}

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
void CDlgListTemplateMain::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(3801));
		static_explan.SetWindowTextA(language.GetCharacterString(3802));	//説明文
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
void CDlgListTemplateMain::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//テンプレート一覧タブ
	for (int i = 0; i < group_val; i++) {
		tab_select_list.InsertItem(i, _T(grp_template.group[i].group_name));
	}
	tab_select_list.SetItemSize(CSize(120, 20));		//タブサイズ ※プロパティStyleを固定幅に設定

	CRect r_tab;
	tab_select_list.GetClientRect(&r_tab);	//タブコントロールのサイズを取得
	tab_select_list.AdjustRect(FALSE, &r_tab);

	//テンプレート一覧タブダイアログの配置
	for (int i = 0; i < group_val; i++) {
		dlg_TabTempList[i] = new CTabListTemplate(&tab_select_list);
		dlg_TabTempList[i]->SetTempNo(&select_template);
		dlg_TabTempList[i]->SetGroupNo(grp_template.group[i].first_number, grp_template.group[i].end_number);
		dlg_TabTempList[i]->Create(IDD_TabListTemplate, &tab_select_list);
		dlg_TabTempList[i]->MoveWindow(&r_tab, TRUE);
	}

	//グループ1のテンプレートを表示
	if (group_val > 0) {
		tab_select_list.SetCurSel(0);
		dlg_TabTempList[0]->ShowWindow(SW_SHOW);
		for (int i = 1; i < group_val; i++) {
			dlg_TabTempList[i]->ShowWindow(SW_HIDE);
		}
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
void CDlgListTemplateMain::LoadParameter()
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
}


/** ****************************************************** **
 ** テンプレート操作
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::CreateTemplate()
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//カメラ切替メッセージ処理
	int scope = sys_template[select_template].scope;
	int view = sys_template[select_template].view;
	int target_cam = sys_scope[scope].view[view].assign_camera;
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_CAM_CHANGE, target_cam);
	if (rtn != 0x1000) {
		outlog.LogOutFile(1, "コマンドメッセージ:CDlgListTemplateMainのカメラ切替失敗");
		return;
	}

	//テンプレート操作ダイアログを表示
	CDlgListTemplateOpe dlg_template_ope;
	dlg_template_ope.SetParam(pParam_main, (LPVOID)this);		//パラメータのセット
	dlg_template_ope.SetNumber(select_template, tab_select_list.GetCurSel());
	dlg_template_ope.DoModal();

	this->ShowWindow(SW_SHOW);	//再表示
}


/** ****************************************************** **
 ** パターンのプレビュー
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::PreviewPattern()
{
	CDlgPreviewPattern dlg_prev;
	dlg_prev.SetNumber(select_template);
	dlg_prev.DoModal();
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgListTemplateMain out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** リスト切替タブSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	int select_tab = tab_select_list.GetCurSel();	//選択したタブ

	for (int i = 0; i < group_val; i++) {
		if (i == select_tab) {
			dlg_TabTempList[i]->ResetPage();
			dlg_TabTempList[i]->ShowWindow(SW_SHOW);
		}
		else {
			dlg_TabTempList[i]->ShowWindow(SW_HIDE);
		}
	}
}


/** ****************************************************** **
 ** イベントメッセージ処理
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgListTemplateMain::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	//テンプレート一覧タブ(CTabListTemplate)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//テンプレート一覧タブの1～8行目操作ボタンClickedを受信
	case IDC_BUTTON1_TabListTemplate:
	case IDC_BUTTON2_TabListTemplate:
	case IDC_BUTTON3_TabListTemplate:
	case IDC_BUTTON4_TabListTemplate:
	case IDC_BUTTON5_TabListTemplate:
	case IDC_BUTTON6_TabListTemplate:
	case IDC_BUTTON7_TabListTemplate:
	case IDC_BUTTON8_TabListTemplate:
		if (HIWORD(wParam) == BN_CLICKED) {
			CreateTemplate();	//テンプレート操作
		}
		break;
		//テンプレート一覧タブの1～8行目プレビューボタンClickedを受信
	case IDC_BUTTON9_TabListTemplate:
	case IDC_BUTTON10_TabListTemplate:
	case IDC_BUTTON11_TabListTemplate:
	case IDC_BUTTON12_TabListTemplate:
	case IDC_BUTTON13_TabListTemplate:
	case IDC_BUTTON14_TabListTemplate:
	case IDC_BUTTON15_TabListTemplate:
	case IDC_BUTTON16_TabListTemplate:
		if (HIWORD(wParam) == BN_CLICKED) {
			PreviewPattern();	//パターンのプレビュー
		}
		break;
	}

	return CDialogEx::OnCommand(wParam, lParam);
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateMain::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	for (int i = 0; i < group_val; i++) {
		dlg_TabTempList[i]->DestroyWindow();
		delete dlg_TabTempList[i];
	}
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgListTemplateMain::PreTranslateMessage(MSG* pMsg)
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
