/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgWorkChgCopyUniq.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：機種切替テンプレート個別コピーダイアログ
 **		機種テンプレート個別コピーを行う
 **/

 // DlgWorkChgCopyUniq.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgWorkChgCopyUniq.h"
#include "afxdialogex.h"

#include "ProcFileOpe.h"

extern CTemplates		sys_template[256];	//システムテンプレートパラメータ
extern CAccessParamWork	access_work;		//機種パラメータファイルアクセス
extern CProcFileOpe		proc_file;			//ファイル処理クラス
extern CCurrentStatus	current;			//共有情報
extern COutputLog		outlog;				//ログ出力
extern CLanguage		language;			//言語設定用

extern char* APP_ROOT;

// CDlgWorkChgCopyUniq ダイアログ

IMPLEMENT_DYNAMIC(CDlgWorkChgCopyUniq, CDialogEx)

CDlgWorkChgCopyUniq::CDlgWorkChgCopyUniq(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgWorkChgCopyUniq, pParent)
{
	select_org_work = -1;	//コピー元No初期化
	select_cpy_work = -1;	//コピー先No初期化
	select_template = -1;	//選択テンプレートNo初期化
}

CDlgWorkChgCopyUniq::~CDlgWorkChgCopyUniq()
{
}

void CDlgWorkChgCopyUniq::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WORK_ORG_UNIQ, list_origin.ctrl_work_list);
	DDX_Control(pDX, IDC_TMPL_ORG_UNIQ, list_origin.ctrl_tmpl_list);
	DDX_Control(pDX, IDC_EDIT1, edit_copy_origin);
	DDX_Control(pDX, IDC_EDIT2, edit_copy_copyto);
	DDX_Control(pDX, IDC_EDIT3, edit_template);
	DDX_Control(pDX, IDC_BUTTON3, btn_select_source);
	DDX_Control(pDX, IDC_BUTTON4, btn_select_destination);
	DDX_Control(pDX, IDC_BUTTON6, btn_select_template);
	DDX_Control(pDX, IDC_STATIC_TEMPLATE, static_template);
	DDX_Control(pDX, IDC_BUTTON5, btn_execute);
	DDX_Control(pDX, IDOK, btn_exit);
	DDX_Control(pDX, IDC_STATIC_SELECT_TEMPLATE, static_select_template);
	DDX_Control(pDX, IDC_STATIC_SOURCE, static_source);
	DDX_Control(pDX, IDC_STATIC_DESTINATION, static_destination);
	DDX_Control(pDX, IDC_STATIC_EXPLAN, static_explan);
}


BEGIN_MESSAGE_MAP(CDlgWorkChgCopyUniq, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgWorkChgCopyUniq::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WORK_ORG_UNIQ, &CDlgWorkChgCopyUniq::OnNMCustomdrawWorkOrgUniq)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_TMPL_ORG_UNIQ, &CDlgWorkChgCopyUniq::OnNMCustomdrawTmplOrgUniq)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgWorkChgCopyUniq::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgWorkChgCopyUniq::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgWorkChgCopyUniq::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgWorkChgCopyUniq::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgWorkChgCopyUniq::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgWorkChgCopyUniq::OnBnClickedButton6)
END_MESSAGE_MAP()


// CDlgWorkChgCopyUniq メッセージ ハンドラー


BOOL CDlgWorkChgCopyUniq::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgWorkChgCopyUniq in ***");

	dlg_x = 352;
	dlg_y = 192;
	dlg_width = 656;
	dlg_height = 432;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	current_update_ans = FALSE;		//現在機種パラメータの変更フラグ

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
void CDlgWorkChgCopyUniq::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//コピーリスト初期設定
	ControlInitOrgList();

	LoadParameterOriginWork();	//コピーリスト パラメータの展開(機種)
	LoadParameterOriginTmpl();	//コピーリスト パラメータの展開(テンプレート)
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1301));
		btn_select_source.SetWindowTextA(language.GetCharacterString(1304));		//選択ボタン元
		btn_select_destination.SetWindowTextA(language.GetCharacterString(1304));	//選択ボタン先
		btn_select_template.SetWindowTextA(language.GetCharacterString(1304));		//選択ボタンテンプレート
		static_template.SetWindowTextA(language.GetCharacterString(1306));			//テンプレート
		btn_execute.SetWindowTextA(language.GetCharacterString(1305));				//実行ボタン
		btn_exit.SetWindowTextA(language.GetCharacterString(1307));					//戻るボタン
		static_select_template.SetWindowTextA(language.GetCharacterString(1302));	//対象テンプレートを選択します
		static_source.SetWindowTextA(language.GetCharacterString(1308));			//コピー元
		static_destination.SetWindowTextA(language.GetCharacterString(1309));		//コピー先
		static_explan.SetWindowTextA(language.GetCharacterString(1310));			//機種を選択します
	}
}


/** ****************************************************** **
 ** コピーリスト初期設定
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::ControlInitOrgList()
{
	// TODO: ここに実装コードを追加します.

	//
	//コピーリスト 機種
	//
	list_origin.page = 0;	//ページ初期化
	list_origin.work_list = new CWorkParam[100];

	//機種選択リストの準備
	list_origin.ctrl_work_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);				//Noカラム追加
	list_origin.ctrl_work_list.InsertColumn(1, Tips0.SelChar(language.jpn, "機種コメント", "Comment"), LVCFMT_LEFT, 200, -1);	//機種コメントカラム追加
	list_origin.ctrl_work_list.InsertColumn(2, Tips0.SelChar(language.jpn, "作成日", "Created Date"), LVCFMT_LEFT, 110, -1);	//作成日カラム追加

	//機種選択リスト内の初期処理
	//空文字でNo項目を100個作成する
	char p_org_work_number[4] = { '\0' };
	char p_org_work_comment[128] = { '\0' };
	char p_org_work_date[24] = { '\0' };
	LVITEM item_org_work_number;
	item_org_work_number.mask = LVIF_TEXT;
	item_org_work_number.iSubItem = 0;
	for (int i = 0; i < 100; i++) {
		//No表示
		item_org_work_number.pszText = p_org_work_number;
		item_org_work_number.iItem = i;
		list_origin.ctrl_work_list.InsertItem(&item_org_work_number);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	list_origin.ctrl_work_list.SetExtendedStyle(list_origin.ctrl_work_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	//
	//コピーリスト テンプレート
	//

	//テンプレート選択リストの準備
	list_origin.ctrl_tmpl_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);		//Noカラム追加
	list_origin.ctrl_tmpl_list.InsertColumn(1, Tips0.SelChar(language.jpn, "名称", "Name"), LVCFMT_LEFT, 130, -1);	//名称カラム追加

	//テンプレート選択リスト内の初期処理
	//空文字でNo項目を256個作成する
	char p_org_tmpl_number[4] = { '\0' };
	char p_org_tmpl_name[32] = { '\0' };
	LVITEM item_org_tmpl_number;
	item_org_tmpl_number.mask = LVIF_TEXT;
	item_org_tmpl_number.iSubItem = 0;
	for (int i = 0; i < 256; i++) {
		//No表示
		item_org_tmpl_number.pszText = p_org_tmpl_number;
		item_org_tmpl_number.iItem = i;
		list_origin.ctrl_tmpl_list.InsertItem(&item_org_tmpl_number);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	list_origin.ctrl_tmpl_list.SetExtendedStyle(list_origin.ctrl_tmpl_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}


/** ****************************************************** **
 ** コピーリスト パラメータの展開(機種)
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::LoadParameterOriginWork()
{
	// TODO: ここに実装コードを追加します.
	int start_number = list_origin.page * 100;

	//機種選択リスト内の全アイテムの削除はしない
	//DeleteAllItems()→InsertItem()→SetItem()→SetItem()としても問題ないが
	//再描画時にチカチカするので、リスト更新ではDeleteAllItems()はせず全ての項目をSetItem()で上書きする
	//ctrl_work_list.DeleteAllItems();

	//Work.ini内の機種コメントと作成日時の展開
	char p_number[4] = { '\0' };
	char p_comment[128] = { '\0' };
	char p_date[24] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	LVITEM item_comment;
	item_comment.mask = LVIF_TEXT;
	item_comment.iSubItem = 1;
	LVITEM item_date;
	item_date.mask = LVIF_TEXT;
	item_date.iSubItem = 2;
	for (int i = 0; i < 100; i++) {
		//Work.iniの読込
		if (access_work.CheckIniFileParam(start_number + i) == 0) {
			access_work.ReadIniFileParam(start_number + i, &list_origin.work_list[i]);
		}
		else {
			list_origin.work_list[i].regi.enable = FALSE;
			list_origin.work_list[i].regi.comment = "";
			list_origin.work_list[i].regi.create_date = "";
		}

		//No表示
		Tips0.IntToChar(p_number, sizeof(p_number), start_number + i, 3);
		item_number.pszText = p_number;
		item_number.iItem = i;
		list_origin.ctrl_work_list.SetItem(&item_number);
		//機種コメント表示
		strcpy_s(p_comment, sizeof(p_comment), list_origin.work_list[i].regi.comment);
		item_comment.pszText = p_comment;
		item_comment.iItem = i;
		list_origin.ctrl_work_list.SetItem(&item_comment);
		//作成日表示
		strcpy_s(p_date, sizeof(p_date), list_origin.work_list[i].regi.create_date);
		item_date.pszText = p_date;
		item_date.iItem = i;
		list_origin.ctrl_work_list.SetItem(&item_date);
	}
}


/** ****************************************************** **
 ** コピーリスト パラメータの展開(テンプレート)
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::LoadParameterOriginTmpl()
{
	// TODO: ここに実装コードを追加します.

	//機種選択リスト内の全アイテムの削除はしない
	//DeleteAllItems()→InsertItem()→SetItem()→SetItem()としても問題ないが
	//再描画時にチカチカするので、リスト更新ではDeleteAllItems()はせず全ての項目をSetItem()で上書きする
	//ctrl_work_list.DeleteAllItems();

	//テンプレートの展開
	char p_number[4] = { '\0' };
	char p_name[32] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	LVITEM item_name;
	item_name.mask = LVIF_TEXT;
	item_name.iSubItem = 1;
	for (int i = 0; i < 256; i++) {
		//テンプレートの読込
		CString tmpl_name = sys_template[i].name;

		//No表示
		Tips0.IntToChar(p_number, sizeof(p_number), i, 3);
		item_number.pszText = p_number;
		item_number.iItem = i;
		list_origin.ctrl_tmpl_list.SetItem(&item_number);
		//名称表示
		strcpy_s(p_name, sizeof(p_name), sys_template[i].name);
		item_name.pszText = p_name;
		item_name.iItem = i;
		list_origin.ctrl_tmpl_list.SetItem(&item_name);
	}
}


/** ****************************************************** **
 ** 個別コピー実行 テンプレートのみ
 ** 引数：
 **　int select_org_work	コピー元機種No
 **　int select_cpy_work	コピー先機種No
 **　int select_org_tmpl	コピー元対象テンプレート
 **　int select_cpy_tmpl	コピー先対象テンプレート
 **　CWorkParam::Registration* org_regi	コピー元機種パラメータ
 **　BOOL cpy_enable		コピー先登録状態
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChgCopyUniq::UniqCopyTempl(int select_org_work, int select_cpy_work, int select_org_tmpl, int select_cpy_tmpl, CWorkParam::Registration* org_regi, BOOL cpy_enable)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char org_path[MAX_PATH] = { '\0' };
	char cpy_path[MAX_PATH] = { '\0' };
	char org_tmpl_folder[5] = { '\0' };
	char cpy_tmpl_folder[5] = { '\0' };
	CString s_org_work;
	CString s_cpy_work;

	//機種No確認
	if (!(select_org_work >= 1 && select_org_work <= 999)) goto END;
	if (!(select_cpy_work >= 1 && select_cpy_work <= 999)) goto END;

	//コピー元パス
	strcpy_s(org_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_org_work = Tips0.IntToString(select_org_work, 3);
	strcat_s(org_path, s_org_work);

	//コピー先パス
	strcpy_s(cpy_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
	strcat_s(cpy_path, s_cpy_work);

	//コピー元テンプレートフォルダ
	strcpy_s(org_tmpl_folder, 5, "T");
	strcat_s(org_tmpl_folder, Tips0.IntToString(select_org_tmpl, 3));

	//コピー先テンプレートフォルダ
	strcpy_s(cpy_tmpl_folder, 5, "T");
	strcat_s(cpy_tmpl_folder, Tips0.IntToString(select_cpy_tmpl, 3));

	//ディレクトリ検索準備
	WIN32_FIND_DATA FindFiledata;
	HANDLE hFind = NULL;
	BOOL copy_complete = FALSE;		//コピー成功フラグ

	//コピー元ディレクトリの確認
	if (proc_file.CheckDir(org_path) != 0) {
		outlog.LogOutFile(1, "コピー元ディレクトリがありません(" + (CString)org_path + ")");
		goto END;
	}

	//コピー先ディレクトリの確認
	if (proc_file.CheckDir(cpy_path) == 0) {

		//コピー先テンプレートディレクトリを削除
		proc_file.DeleteFileAllEx((CString)cpy_path + "\\" + (CString)cpy_tmpl_folder, TRUE);
	}
	else {
		//コピー先ディレクトリの作成
		if (CreateDirectoryA(cpy_path, NULL) == 0) {
			outlog.LogOutFile(1, "コピー先ディレクトリの作成に失敗しました(" + (CString)cpy_path + ")");
			goto FIND_END;
		}
	}


	/********** コピー元テンプレートデータディレクトリの検索とコピー **********/

	//検索1回目
	hFind = FindFirstFileA((CString)org_path + "\\*.*", &FindFiledata);
	if (hFind == INVALID_HANDLE_VALUE) {
		//検索結果無し
		//通常はディレクトリは必ず見つかる　".."と"."が検索される

		//コピー元ディレクトリが存在しない場合
		goto FIND_END;
	}

	//ディレクトリ有り
	else {
		do {
			if ((FindFiledata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				&& lstrcmpA(FindFiledata.cFileName, "..")
				&& lstrcmpA(FindFiledata.cFileName, ".")) {

				//検索テンプレートディレクトリ名
				char* search_folder = org_tmpl_folder;

				//検索ディレクトリ名を比較
				if (Tips0.CompCharacters(FindFiledata.cFileName, search_folder) == 0) {
					//コピー実行
					if (proc_file.CopyFileAllEx((CString)org_path + "\\" + (CString)org_tmpl_folder, (CString)cpy_path + "\\" + (CString)cpy_tmpl_folder) == 0) {
						copy_complete = TRUE;
					}
					else {
						goto FIND_END;
					}
				}
			}
		} while (FindNextFile(hFind, &FindFiledata));	//検索2回目以降
	}

	//コピー成功判断
	if (!copy_complete) {
		rtn_result = 2;
		goto FIND_END;
	}

	//未登録機種の場合は機種パラメータの作成
	if (!cpy_enable) CreateParam(select_cpy_work, org_regi);
	/********** ここまで **********/

	rtn_result = 0;	//処理成功

FIND_END:
	if (hFind != NULL) FindClose(hFind);

	//リスト更新
	LoadParameterOriginWork();	//コピーリスト パラメータの展開(機種)

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 機種パラメータの作成
 ** 引数：
 **　int select_cpy_work	コピー先機種No
 **　CWorkParam::Registration* org_regi	コピー元機種パラメータ
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::CreateParam(int select_cpy_work, CWorkParam::Registration* org_regi)
{
	// TODO: ここに実装コードを追加します.

	//コピー先コメントに"copy-"を付加
	char link_comment[36] = "copy-";
	strncat_s(link_comment, sizeof(link_comment), org_regi->comment, 30);
	char copy_comment[31] = { '\0' };
	Tips0.CopyCharacters(copy_comment, sizeof(copy_comment), link_comment, 0, 30);

	//コピー先機種パラメータの更新
	CWorkParam new_param;
	new_param.SetCreateDate();
	new_param.regi.comment = copy_comment;
	access_work.WriteIniFileParam(select_cpy_work, &new_param);
}


/** ****************************************************** **
 ** 現在機種の再読み込み回答
 ** 引数：
 **　なし
 ** 戻り値：
 **　BOOL：読込回答　TRUE:必要, FALSE:不必要
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgWorkChgCopyUniq::GetReloadAns()
{
	// TODO: ここに実装コードを追加します.
	return current_update_ans;
}


/** ****************************************************** **
 ** コピーリストページ切替(<<)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	list_origin.page = list_origin.page - 1;
	if (list_origin.page < 0) list_origin.page = 9;

	LoadParameterOriginWork();	//コピー元リスト パラメータの展開(機種)
}


/** ****************************************************** **
 ** コピーリストページ切替(>>)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	list_origin.page = list_origin.page + 1;
	if (list_origin.page > 9) list_origin.page = 0;

	LoadParameterOriginWork();	//コピー元リスト パラメータの展開(機種)
}


/** ****************************************************** **
 ** コピー元選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//コピー元選択行の取得(機種)
	int select_org_work_row = list_origin.ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_org_work_row < 0 || select_org_work_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "コピー元機種を選択してください", "Please select Source");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	select_org_work = list_origin.page * 100 + select_org_work_row;	//コピー元機種No

	//機種No.0はコピー対象でない
	if (select_org_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "選択失敗", "Selection failure");
		msg2 = Tips0.SelStr(language.jpn, "機種No000は選択できません", "Model No000 cannot be selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		select_org_work = -1;
		edit_copy_origin.SetWindowTextA("");
		return;
	}

	//選択成功
	org_work_regi = list_origin.work_list[select_org_work_row].regi;		//コピー元登録パラメータの確保

	CString comment1 = Tips0.SelStr(language.jpn, "機種No", "Model No") + Tips0.IntToString(select_org_work, 3);
	edit_copy_origin.SetWindowTextA(comment1 + " " + org_work_regi.comment);
}


/** ****************************************************** **
 ** コピー先選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//コピー先選択行の取得(機種)
	int select_cpy_work_row = list_origin.ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_cpy_work_row < 0 || select_cpy_work_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "コピー先機種を選択してください", "Please select destination");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	select_cpy_work = list_origin.page * 100 + select_cpy_work_row;	//コピー先機種No

	//機種No.0はコピー対象でない
	if (select_cpy_work == 0) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "選択失敗", "Selection failure");
		msg2 = Tips0.SelStr(language.jpn, "機種No000は選択できません", "Model No000 cannot be selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		select_cpy_work = -1;
		edit_copy_copyto.SetWindowTextA("");
		return;
	}

	//選択成功
	cpy_work_regi = list_origin.work_list[select_cpy_work_row].regi;		//コピー先登録パラメータの確保

	CString comment1 = Tips0.SelStr(language.jpn, "機種No", "Model No") + Tips0.IntToString(select_cpy_work, 3);
	edit_copy_copyto.SetWindowTextA(comment1 + " " + cpy_work_regi.comment);
}


/** ****************************************************** **
 ** テンプレート選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2, msg3, msg4;
	this->GetWindowTextA(title);

	//コピー元選択行の取得(テンプレート)
	int select_org_tmpl_row = list_origin.ctrl_tmpl_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_org_tmpl_row < 0 || select_org_tmpl_row > 255) {
		msg1 = Tips0.SelStr(language.jpn, "対象テンプレートを選択してください", "Please select the template");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//選択成功
	select_template = select_org_tmpl_row;

	CString comment1 = "No" + Tips0.IntToString(select_template, 3);
	edit_template.SetWindowTextA(comment1 + " " + sys_template[select_template].name);
}


/** ****************************************************** **
 ** 実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2, msg3, msg4;
	this->GetWindowTextA(title);

	//コピー元選択の確認(機種)
	if (select_org_work < 0 || select_org_work > 999) {
		msg1 = Tips0.SelStr(language.jpn, "コピー元機種を選択してください", "Please select Source");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//コピー先選択の確認(機種)
	if (select_cpy_work < 0 || select_cpy_work > 999) {
		msg1 = Tips0.SelStr(language.jpn, "コピー先機種を選択してください", "Please select destination");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//対象テンプレートの確認
	if (select_template < 0 || select_template > 255) {
		msg1 = Tips0.SelStr(language.jpn, "対象テンプレートを選択してください", "Please select the template");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	int select_org_tmpl = select_template;	//コピー元対象テンプレート
	int select_cpy_tmpl = select_template;	//コピー先対象テンプレート

	//コピー元とコピー先が同じ場合
	if (select_org_work == select_cpy_work) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種コピー失敗", "Copy failure");
		msg2 = Tips0.SelStr(language.jpn, "コピー元機種Noとコピー先機種Noが同じです", "Source No and Destination No are the same");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	//コピー実行確認
	if (org_work_regi.enable) {

		msg1 = Tips0.SelStr(language.jpn,
			"テンプレート個別コピー実行\n以下のテンプレートをコピーします。",
			"Template individual copy execution\nCopy the following template");

		msg2 = Tips0.SelStr(language.jpn, "　コピー元機種No", "　Source No") + Tips0.IntToString(select_org_work, 3);
		if (select_org_work == current.work_number) msg2 = msg2 + Tips0.SelStr(language.jpn, "(現在機種)", "(Current model)");
		msg2 = msg2 + Tips0.SelStr(language.jpn, "　テンプレートNo", "  Template No") + Tips0.IntToString(select_org_tmpl, 3);

		msg3 = Tips0.SelStr(language.jpn, "　コピー先機種No", "　Destination No") + Tips0.IntToString(select_cpy_work, 3);
		if (select_cpy_work == current.work_number) msg3 = msg3 + Tips0.SelStr(language.jpn, "(現在機種)", "(Current model)");
		msg3 = msg3 + Tips0.SelStr(language.jpn, "　テンプレートNo", "  TemplateNo") + Tips0.IntToString(select_cpy_tmpl, 3);

		msg4 = Tips0.SelStr(language.jpn, "よろしいですか？", "Is it OK?");

		//ボタン無効化
		btn_execute.EnableWindow(FALSE);
		btn_exit.EnableWindow(FALSE);

		int judge = MessageBoxA(msg1 + "\n" + msg2 + "\n" + msg3 + "\n" + msg4, title, MB_OKCANCEL | MB_ICONQUESTION);
		if (judge == IDOK) {
			//コピー実行
			int rtn_cpy = UniqCopyTempl(select_org_work, select_cpy_work,
										select_org_tmpl, select_cpy_tmpl,
										&org_work_regi, cpy_work_regi.enable);

			CString s_org_work = Tips0.IntToString(select_org_work, 3);
			CString s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
			CString s_org_tmpl = Tips0.IntToString(select_org_tmpl, 3);
			CString s_cpy_tmpl = Tips0.IntToString(select_cpy_tmpl, 3);
			if (rtn_cpy == 0) {
				//成功メッセージ
				msg1 = Tips0.SelStr(language.jpn, "テンプレート個別コピー実行", "Template individual copy execution");
				msg2 = Tips0.SelStr(language.jpn, "個別コピーに成功しました。", "Successful individual copy");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
				outlog.LogOutFile(1, "テンプレート個別コピー成功 元機種No" + s_org_work + ", 元テンプレートNo" + s_org_tmpl
								+ ", 先機種No" + s_cpy_work + ", 先テンプレートNo" + s_cpy_tmpl);
			}
			else if (rtn_cpy == 2) {
				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "テンプレート個別コピー実行", "Template individual copy execution");
				msg2 = Tips0.SelStr(language.jpn, "コピー対象のデータがありませんでした。", "There was no data to copy");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "テンプレート個別コピー失敗 元機種No" + s_org_work + ", 元テンプレートNo" + s_org_tmpl
					+ ", 先機種No" + s_cpy_work + ", 先テンプレートNo" + s_cpy_tmpl);
			}
			else {
				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "テンプレート個別コピー実行", "Template individual copy execution");
				msg2 = Tips0.SelStr(language.jpn, "個別コピーに失敗しました。", "Individual copy failed");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "テンプレート個別コピー失敗 元機種No" + s_org_work + ", 元テンプレートNo" + s_org_tmpl
					+ ", 先機種No" + s_cpy_work + ", 先テンプレートNo" + s_cpy_tmpl);
			}

			//現在機種パラメータの変更確認
			if (select_cpy_work == current.work_number) current_update_ans = TRUE;
		}
	}
	else {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種コピー失敗", "Model copy failure");
		msg2 = Tips0.SelStr(language.jpn, "コピー元機種に作成前の機種が選択されました", "The model before creation was selected as the copy source model");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
	}

	//ボタン有効化
	btn_execute.EnableWindow(TRUE);
	btn_exit.EnableWindow(TRUE);
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgWorkChgCopyUniq out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//リスト内のパラメータ郡の削除
	delete[] list_origin.work_list;
}


/** ****************************************************** **
 ** コピー機種選択リスト 選択背景の色変更
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnNMCustomdrawWorkOrgUniq(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {

		if (pNMCD->nmcd.uItemState & CDIS_SELECTED) {
			pNMCD->clrTextBk = RGB(181, 218, 255);		//薄青
			pNMCD->nmcd.uItemState &= ~CDIS_SELECTED;
		}
		else {
			pNMCD->clrTextBk = RGB(255, 255, 255);	//白
		}
		*pResult = CDRF_NEWFONT;
	}
}


/** ****************************************************** **
 ** コピーテンプレート選択リスト 選択背景の色変更
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyUniq::OnNMCustomdrawTmplOrgUniq(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	LPNMLVCUSTOMDRAW pNMCD = reinterpret_cast<LPNMLVCUSTOMDRAW>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	if (pNMCD->nmcd.dwDrawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
	}
	else if (pNMCD->nmcd.dwDrawStage == CDDS_ITEMPREPAINT) {

		if (pNMCD->nmcd.uItemState & CDIS_SELECTED) {
			pNMCD->clrTextBk = RGB(181, 218, 255);		//薄青
			pNMCD->nmcd.uItemState &= ~CDIS_SELECTED;
		}
		else {
			pNMCD->clrTextBk = RGB(255, 255, 255);	//白
		}
		*pResult = CDRF_NEWFONT;
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
BOOL CDlgWorkChgCopyUniq::PreTranslateMessage(MSG* pMsg)
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
