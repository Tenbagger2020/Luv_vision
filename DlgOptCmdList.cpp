/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgOptCmdList.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：コマンド一覧ダイアログ
 **		コマンド設定一覧を表示する
 **/

// DlgOptCmdList.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgOptCmdList.h"
#include "afxdialogex.h"

extern CCommands			cmd_param[256];		//コマンドパラメータ
extern CTemplates			sys_template[256];	//システムテンプレートパラメータ
extern CDefinedElementCmd	element_cmd;		//コマンド要素
extern COutputLog			outlog;				//ログ出力
extern CLanguage			language;			//言語設定用

// CDlgOptCmdList ダイアログ

IMPLEMENT_DYNAMIC(CDlgOptCmdList, CDialogEx)

CDlgOptCmdList::CDlgOptCmdList(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgOptCmdList, pParent)
{

}

CDlgOptCmdList::~CDlgOptCmdList()
{
}

void CDlgOptCmdList::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, ctrl_cmd_list);
	DDX_Control(pDX, IDC_STATIC01, static01);
}


BEGIN_MESSAGE_MAP(CDlgOptCmdList, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgOptCmdList::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgOptCmdList メッセージ ハンドラー


BOOL CDlgOptCmdList::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgOptCmdList in ***");

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = 640;
	dlg_height = 352;
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
void CDlgOptCmdList::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//コマンドリストの準備
	ctrl_cmd_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);	//Noカラム追加
	ctrl_cmd_list.InsertColumn(1, Tips0.SelChar(language.jpn, "実行機能", "Function"), LVCFMT_LEFT, 140, -1);	//実行機能カラム追加
	ctrl_cmd_list.InsertColumn(2, Tips0.SelChar(language.jpn, "テンプレート", "Template"), LVCFMT_LEFT, 160, -1);	//テンプレートカラム追加
	ctrl_cmd_list.InsertColumn(3, Tips0.SelChar(language.jpn, "取込", "Capture"), LVCFMT_CENTER, 40, -1);		//取込カラム追加
	ctrl_cmd_list.InsertColumn(4, Tips0.SelChar(language.jpn, "探索", "Search"), LVCFMT_CENTER, 40, -1);		//探索カラム追加
	ctrl_cmd_list.InsertColumn(5, Tips0.SelChar(language.jpn, "演算", "Calc."), LVCFMT_CENTER, 40, -1);			//演算カラム追加
	ctrl_cmd_list.InsertColumn(6, Tips0.SelChar(language.jpn, "読込", "Read"), LVCFMT_CENTER, 40, -1);			//読込カラム追加
	ctrl_cmd_list.InsertColumn(7, Tips0.SelChar(language.jpn, "書込", "Write"), LVCFMT_CENTER, 40, -1);			//書込カラム追加

	//コマンドリスト内の初期処理
	//空文字でNo項目を256個作成する
	char p_number[4] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	for (int i = 0; i < 256; i++) {
		//No表示
		item_number.pszText = p_number;
		item_number.iItem = i;
		ctrl_cmd_list.InsertItem(&item_number);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	ctrl_cmd_list.SetExtendedStyle(ctrl_cmd_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgOptCmdList::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2001));
		static01.SetWindowTextA(language.GetCharacterString(2002));	//コマンド割り当てを表示しています
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
void CDlgOptCmdList::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//コマンドパラメータの展開
	char p_number[4] = { '\0' };
	char p_method[128] = { '\0' };
	char p_template[128] = { '\0' };
	char p_grab[8] = { '\0' };
	char p_search[8] = { '\0' };
	char p_calc[8] = { '\0' };
	char p_read[8] = { '\0' };
	char p_write[8] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	LVITEM item_method;
	item_method.mask = LVIF_TEXT;
	item_method.iSubItem = 1;
	LVITEM item_template;
	item_template.mask = LVIF_TEXT;
	item_template.iSubItem = 2;
	LVITEM item_grab;
	item_grab.mask = LVIF_TEXT;
	item_grab.iSubItem = 3;
	LVITEM item_search;
	item_search.mask = LVIF_TEXT;
	item_search.iSubItem = 4;
	LVITEM item_calc;
	item_calc.mask = LVIF_TEXT;
	item_calc.iSubItem = 5;
	LVITEM item_read;
	item_read.mask = LVIF_TEXT;
	item_read.iSubItem = 6;
	LVITEM item_write;
	item_write.mask = LVIF_TEXT;
	item_write.iSubItem = 7;
	for (int i = 0; i < 256; i++) {
		//No表示
		Tips0.IntToChar(p_number, sizeof(p_number), i, 3);
		item_number.pszText = p_number;
		item_number.iItem = i;
		ctrl_cmd_list.SetItem(&item_number);

		byte base_method = cmd_param[i].base_method;
		WORD search_method = cmd_param[i].search.method;

		//実行機能表示
		CString method_text = "";
		BOOL calc_enable = cmd_param[i].search.calc_enable;
		int base_num = sizeof(element_cmd.base) / sizeof(element_cmd.base[0]);
		for (int j = 0; j < base_num; j++) {
			if (base_method == element_cmd.base[j].code) {
				//探索系は探索機能を表示
				if (base_method == 0x01 ||	//アライメント
					base_method == 0x02 ||	//計測モード
					base_method == 0x03 ||	//キャリブレーション
					base_method == 0x04) {	//バーコードリーダー
					int search_num = sizeof(element_cmd.search) / sizeof(element_cmd.search[0]);
					for (int k = 0; k < search_num; k++) {
						if (base_method == element_cmd.search[k].base_method &&
							search_method == element_cmd.search[k].method &&
							(base_method != 0x01 || calc_enable)) {
							method_text = element_cmd.search[k].s_text;
							break;
						}
					}
				}
				//ユーティリティ系は機能種別を表示
				else {
					method_text = element_cmd.base[j].s_text;
				}
				break;
			}
		}
		if (base_method == 0x01 && (search_method == 0x0000 || !calc_enable)) method_text = "(演算無し)";
		strcpy_s(p_method, sizeof(p_method), method_text);
		item_method.pszText = p_method;
		item_method.iItem = i;
		ctrl_cmd_list.SetItem(&item_method);

		//テンプレート表示
		CString template_name = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダー、外部オフセット設定の場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04 ||
			base_method == 0x41) {
			int template_number = cmd_param[i].template_number;
			template_name = Tips0.IntToString(template_number, 3) + " " + sys_template[template_number].name;
			if (strlen(template_name) == 0) template_name = "template" + Tips0.IntToString(template_number);
		}
		strcpy_s(p_template, sizeof(p_template), template_name);
		item_template.pszText = p_template;
		item_template.iItem = i;
		ctrl_cmd_list.SetItem(&item_template);

		//取込表示
		CString grab_sign = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダーの場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04) {
			if (cmd_param[i].grab.enable) grab_sign = "○";
		}
		strcpy_s(p_grab, sizeof(p_grab), grab_sign);
		item_grab.pszText = p_grab;
		item_grab.iItem = i;
		ctrl_cmd_list.SetItem(&item_grab);

		//探索表示
		CString search_sign = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダーの場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04) {
			if (cmd_param[i].search.search_enable) search_sign = "○";
		}
		strcpy_s(p_search, sizeof(p_search), search_sign);
		item_search.pszText = p_search;
		item_search.iItem = i;
		ctrl_cmd_list.SetItem(&item_search);

		//演算表示
		CString calc_sign = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダーの場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04) {
			if (cmd_param[i].search.calc_enable) calc_sign = "○";
		}
		strcpy_s(p_calc, sizeof(p_calc), calc_sign);
		item_calc.pszText = p_calc;
		item_calc.iItem = i;
		ctrl_cmd_list.SetItem(&item_calc);

		//読込表示
		CString read_sign = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダーの場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04) {
			if (cmd_param[i].read.enable) read_sign = "○";
		}
		strcpy_s(p_read, sizeof(p_read), read_sign);
		item_read.pszText = p_read;
		item_read.iItem = i;
		ctrl_cmd_list.SetItem(&item_read);

		//書込表示
		CString write_sign = "";
		//機能種別がアライメント、計測モード、キャリブレーション、バーコードリーダーの場合
		if (base_method == 0x01 ||
			base_method == 0x02 ||
			base_method == 0x03 ||
			base_method == 0x04) {
			if (cmd_param[i].write.enable) write_sign = "○";
		}
		strcpy_s(p_write, sizeof(p_write), write_sign);
		item_write.pszText = p_write;
		item_write.iItem = i;
		ctrl_cmd_list.SetItem(&item_write);
	}
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgOptCmdList::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgOptCmdList out ***");

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
BOOL CDlgOptCmdList::PreTranslateMessage(MSG* pMsg)
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
