/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgTempOutResult.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：テンプレート毎の結果出力設定画面ダイアログ
 **		テンプレートを使用したコマンドの結果、画像の保存設定を行う
 **/

// DlgTempOutResult.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgTempOutResult.h"
#include "afxdialogex.h"

extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern COutResult		sys_outrslt;			//結果出力パラメータ
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;				//言語設定用

// CDlgTempOutResult ダイアログ

IMPLEMENT_DYNAMIC(CDlgTempOutResult, CDialogEx)

CDlgTempOutResult::CDlgTempOutResult(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgTempOutResult, pParent)
{
	//ダイアログ配置
	dlg_x = 80;
	dlg_y = 80;
	dlg_width = 368;
	dlg_height = 576;

	org_format_num = 0;	//オリジナル記号登録数

	//コントロール設定
	edit_folder_format.SetFuncLBtnDblClk(OPEN_OSK);
}

CDlgTempOutResult::~CDlgTempOutResult()
{
}

void CDlgTempOutResult::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, check_folder_enable);
	DDX_Control(pDX, IDC_EDIT1, edit_folder_format);
	DDX_Control(pDX, IDC_TAB1, tab_select_sheet);
	DDX_Control(pDX, IDC_LIST1, ctrl_format_list);
	DDX_Control(pDX, IDC_STATIC_SUB_FOLDER, static_sub_folder);
	DDX_Control(pDX, IDC_STATIC_FORMAT_FOLDER, static_format_folder);
	DDX_Control(pDX, IDC_STATIC_SYMBOL_LIST, static_symbol_list);
	DDX_Control(pDX, IDOK, btn_ok);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
}


BEGIN_MESSAGE_MAP(CDlgTempOutResult, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgTempOutResult::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgTempOutResult::OnBnClickedCancel)
	ON_WM_DESTROY()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDlgTempOutResult::OnTcnSelchangeTab1)
	ON_BN_CLICKED(IDC_CHECK1, &CDlgTempOutResult::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CDlgTempOutResult メッセージ ハンドラー


BOOL CDlgTempOutResult::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgTempOutResult in ***");

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
void CDlgTempOutResult::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//タブコントロール準備
	ControlInitialTab();

	//オリジナル記号の登録数確認
	for (int i = 0; i < 10; i++) {
		if (strlen(sys_outrslt.org_format[i].symbol) == 2) org_format_num++;
	}

	//記号リストの準備
	ctrl_format_list.InsertColumn(0, Tips0.SelChar(language.jpn, "記号", "Symbol"), LVCFMT_LEFT, 50, -1);		//記号カラム追加
	ctrl_format_list.InsertColumn(1, Tips0.SelChar(language.jpn, "説明", "Content"), LVCFMT_LEFT, 265, -1);		//表示内容カラム追加

	//記号リスト内の初期処理
	//空文字で必要個数作成する
	char p_symbol[128] = { '\0' };
	char p_content[128] = { '\0' };
	LVITEM item_symbol;
	item_symbol.mask = LVIF_TEXT;
	item_symbol.iSubItem = 0;
	int total_row = 1 + 12 + 1 + org_format_num;	//1(title) + 12(システム記号数) + 1(title) + org_format_num(オリジナル記号数)
	for (int i = 0; i < total_row; i++) {
		//No表示
		item_symbol.pszText = p_symbol;
		item_symbol.iItem = i;
		ctrl_format_list.InsertItem(&item_symbol);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	ctrl_format_list.SetExtendedStyle(ctrl_format_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//記号リストの作成
	AddFormatList();
}


/** ****************************************************** **
 ** タブコントロール準備
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//パラメータ切替タブ
	tab_select_sheet.InsertItem(0, Tips0.SelChar(language.jpn, "結果保存", "Save Results"));
	tab_select_sheet.InsertItem(1, Tips0.SelChar(language.jpn, "画像保存", "Save Image"));
	tab_select_sheet.SetItemSize(CSize(80, 20));		//タブサイズ ※プロパティStyleを固定幅に設定

	CRect r_tab;
	tab_select_sheet.GetClientRect(&r_tab);	//タブコントロールのサイズを取得
	tab_select_sheet.AdjustRect(FALSE, &r_tab);

	//結果保存タブダイアログの配置
	dlg_TabRlt = new CTabOutSettingRlt(&tab_select_sheet);
	dlg_TabRlt->Create(IDD_TabOutSettingRlt, &tab_select_sheet);
	dlg_TabRlt->MoveWindow(&r_tab, TRUE);

	//画像保存タブダイアログの配置
	dlg_TabImg = new CTabOutSettingImg(&tab_select_sheet);
	dlg_TabImg->Create(IDD_TabOutSettingImg, &tab_select_sheet);
	dlg_TabImg->MoveWindow(&r_tab, TRUE);

	tab_select_sheet.SetCurSel(0);
	dlg_TabRlt->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** 記号リストの作成
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::AddFormatList()
{
	// TODO: ここに実装コードを追加します.

	//記号リストにシステム記号とオリジナル記号を追加する
	char p_symbol[128] = { '\0' };
	char p_content[128] = { '\0' };
	LVITEM item_symbol;
	item_symbol.mask = LVIF_TEXT;
	item_symbol.iSubItem = 0;
	LVITEM item_content;
	item_content.mask = LVIF_TEXT;
	item_content.iSubItem = 1;

	//
	//システム記号の追加
	//
	char items[56][64] = {
		"", "(システム記号)","","(System Symbols)",
		"\"文字\"","文字の追加","\"Character\"","Add characters",
		"<ID>", "シリアル番号","<ID>", "Serial number",
		"<TN>", "テンプレート番号3桁 例)008","<TN>", "Template number 3digits ex)008",
		"<WN>", "機種No3桁 例)012","<WN>", "WorkNo 3digits ex)012",
		"<JD>","テンプレート結果 例)OK,NG","<JD>","Template result ex)OK,NG",
		"<YYYY>","実行年 例)2022","<YYYY>","Year ex)2022",
		"<MM>","実行月 例)02","<MM>","Month ex)02",
		"<DD>","実行日 例)28","<DD>","Day ex)28",
		"<hh>","実行時 例)09","<hh>","Hour ex)09",
		"<mm>","実行分 例)30","<mm>","Minutes ex)30",
		"<ss>","実行秒 例)46","<ss>","Seconds ex)46",
		"<ms>","実行ミリ秒 例)078","<ms>","Milliseconds ex)078"
	};

	//リスト追加
	int lang_offset = 0;
	if (!language.jpn) lang_offset = 2;
	for (int i = 0; i < 13; i++) {
		int item_num = i * 4 + lang_offset;
		item_symbol.pszText = items[item_num];
		item_symbol.iItem = i;
		ctrl_format_list.SetItem(&item_symbol);
		item_content.pszText = items[item_num + 1];
		item_content.iItem = i;
		ctrl_format_list.SetItem(&item_content);
	}

	//
	//オリジナル記号の追加
	//
	item_symbol.pszText = "";
	item_symbol.iItem = 13;
	ctrl_format_list.SetItem(&item_symbol);
	item_content.pszText = Tips0.SelChar(language.jpn, "(オリジナル記号)", "(Original Symbols)");
	item_content.iItem = 13;
	ctrl_format_list.SetItem(&item_content);

	//リスト追加
	int org_cnt = 0;
	for (int i = 0; i < 10; i++) {
		if (strlen(sys_outrslt.org_format[i].symbol) == 2) {
			if (org_cnt < org_format_num) {
				char symbol_mark[5] = { '\0' };
				strcat_s(symbol_mark, "(");
				strcat_s(symbol_mark, sys_outrslt.org_format[i].symbol);
				strcat_s(symbol_mark, ")");
				item_symbol.pszText = symbol_mark;
				item_symbol.iItem = 14 + org_cnt;
				ctrl_format_list.SetItem(&item_symbol);
				item_content.pszText = sys_outrslt.org_format[i].content;
				item_content.iItem = 14 + org_cnt;
				ctrl_format_list.SetItem(&item_content);
				org_cnt++;
			}
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
void CDlgTempOutResult::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//テンプレートパラメータを編集用パラメータへコピー
	edit_param = template_param[template_number];

	//サブフォルダ設定
	check_folder_enable.SetCheck(edit_param.out_result.enable_sub_folder);
	edit_folder_format.SetWindowTextA(edit_param.out_result.format_folder);
	EnableSubFolder(edit_param.out_result.enable_sub_folder);

	//結果保存
	dlg_TabRlt->check_rslt_enable.SetCheck(edit_param.out_result.enable_result);
	dlg_TabRlt->edit_rslt_format.SetWindowTextA(edit_param.out_result.format_result);
	EnableOutResult(edit_param.out_result.enable_result);

	//画像保存
	dlg_TabImg->check_image_enable.SetCheck(edit_param.out_result.enable_image);
	dlg_TabImg->edit_image_format.SetWindowTextA(edit_param.out_result.format_image);
	dlg_TabImg->combo_image_type.SetCurSel(edit_param.out_result.type_image);
	dlg_TabImg->combo_image_style.SetCurSel(edit_param.out_result.style_image - 1);
	EnableOutImage(edit_param.out_result.enable_image);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	char buf[64] = { '\0' };

	//サブフォルダ設定
	edit_param.out_result.enable_sub_folder = check_folder_enable.GetCheck();
	edit_folder_format.GetWindowTextA(buf, sizeof(buf));
	edit_param.out_result.format_folder = buf;

	//結果保存
	edit_param.out_result.enable_result = dlg_TabRlt->check_rslt_enable.GetCheck();
	dlg_TabRlt->edit_rslt_format.GetWindowTextA(buf, sizeof(buf));
	edit_param.out_result.format_result = buf;

	//画像保存
	edit_param.out_result.enable_image = dlg_TabImg->check_image_enable.GetCheck();
	dlg_TabImg->edit_image_format.GetWindowTextA(buf, sizeof(buf));
	edit_param.out_result.format_image = buf;
	edit_param.out_result.type_image = dlg_TabImg->combo_image_type.GetCurSel();
	edit_param.out_result.style_image = dlg_TabImg->combo_image_style.GetCurSel() + 1;

	//編集用パラメータをテンプレートパラメータへコピー
	template_param[template_number] = edit_param;
}


/** ****************************************************** **
 ** サブフォルダコントロール有効/無効
 ** 引数：
 **　BOOL enable	有効/無効
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::EnableSubFolder(BOOL enable)
{
	// TODO: ここに実装コードを追加します.
	edit_folder_format.EnableWindow(enable);
}


/** ****************************************************** **
 ** 結果保存コントロール有効/無効
 ** 引数：
 **　BOOL enable	有効/無効
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::EnableOutResult(BOOL enable)
{
	// TODO: ここに実装コードを追加します.
	dlg_TabRlt->edit_rslt_format.EnableWindow(enable);
}


/** ****************************************************** **
 ** 画像保存コントロール有効/無効
 ** 引数：
 **　BOOL enable	有効/無効
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::EnableOutImage(BOOL enable)
{
	// TODO: ここに実装コードを追加します.
	dlg_TabImg->edit_image_format.EnableWindow(enable);
	dlg_TabImg->combo_image_type.EnableWindow(enable);
	dlg_TabImg->combo_image_style.EnableWindow(enable);
}


/** ****************************************************** **
 ** 作成テンプレート番号の入力
 ** 引数：
 **　int number　テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(5101));

		static_sub_folder.SetWindowTextA(language.GetCharacterString(5102));	//サブフォルダ設定
		check_folder_enable.SetWindowTextA(language.GetCharacterString(5103));	//サブフォルダを作成する
		static_format_folder.SetWindowTextA(language.GetCharacterString(5104));	//フォルダ名書式

		static_symbol_list.SetWindowTextA(language.GetCharacterString(5105));	//記号一覧

		btn_ok.SetWindowTextA(language.GetCharacterString(5106));			//設定ボタン
		btn_cancel.SetWindowTextA(language.GetCharacterString(5107));		//キャンセルボタン
	}
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	this->GetWindowTextA(title);

	int judge = MessageBoxA("結果出力設定を更新します。よろしいですか？", title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgTempOutResult out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgTempOutResult out (CANCEL) ***");

	CDialogEx::OnCancel();
}


/** ****************************************************** **
 ** サブフォルダチェックボックスClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnBnClickedCheck1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	BOOL check = check_folder_enable.GetCheck();
	EnableSubFolder(check);
}


/** ****************************************************** **
 ** シート切替タブSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	switch (tab_select_sheet.GetCurSel()) {
		//結果保存を表示
	case 0:
		dlg_TabRlt->ShowWindow(SW_SHOW);
		dlg_TabImg->ShowWindow(SW_HIDE);
		break;
		//画像保存を表示
	case 1:
		dlg_TabRlt->ShowWindow(SW_HIDE);
		dlg_TabImg->ShowWindow(SW_SHOW);
		break;
	}
}


/** ****************************************************** **
 ** イベントメッセージ処理
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgTempOutResult::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	//イベントメッセージ処理(結果保存)
	//結果保存タブ(CTabOutSettingRlt)からのボタンClickedを受信し処理する
	OnCommandRlt(wParam);

	//イベントメッセージ処理(画像保存)
	//画像保存タブ(CTabOutSettingImg)からのボタンClickedを受信し処理する
	OnCommandImg(wParam);

	return CDialogEx::OnCommand(wParam, lParam);
}


/** ****************************************************** **
 ** イベントメッセージ処理(結果保存)
 ** 引数：
 **　WPARAM wParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnCommandRlt(WPARAM wParam)
{
	// TODO: ここに実装コードを追加します.

	//結果保存タブ(CTabOutSettingRlt)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//結果保存有効ボタンClickedを受信
	case IDC_CHECK1_TabOutSettingRlt:
		if (HIWORD(wParam) == BN_CLICKED) {
			BOOL check = dlg_TabRlt->check_rslt_enable.GetCheck();
			EnableOutResult(check);
		}
		break;
	}
}


/** ****************************************************** **
 ** イベントメッセージ処理(画像保存)
 ** 引数：
 **　WPARAM wParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnCommandImg(WPARAM wParam)
{
	// TODO: ここに実装コードを追加します.

	//画像保存タブ(CTabOutSettingImg)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//画像保存有効ボタンClickedを受信
	case IDC_CHECK1_TabOutSettingImg:
		if (HIWORD(wParam) == BN_CLICKED) {
			BOOL check = dlg_TabImg->check_image_enable.GetCheck();
			EnableOutImage(check);
		}
		break;
	}
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTempOutResult::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	dlg_TabRlt->DestroyWindow();
	delete dlg_TabRlt;
	dlg_TabImg->DestroyWindow();
	delete dlg_TabImg;
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgTempOutResult::PreTranslateMessage(MSG* pMsg)
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
