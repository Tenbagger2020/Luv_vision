/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgWorkChgCopyAll.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：機種切替一括コピーダイアログ
 **		機種一括コピーを行う
 **/

 // DlgWorkChgCopyAll.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgWorkChgCopyAll.h"
#include "afxdialogex.h"

#include "ProcFileOpe.h"

extern CAccessParamWork	access_work;		//機種パラメータファイルアクセス
extern CProcFileOpe		proc_file;			//ファイル処理クラス
extern CCurrentStatus	current;			//共有情報
extern COutputLog		outlog;				//ログ出力
extern CLanguage		language;			//言語設定用

extern char* APP_ROOT;

// CDlgWorkChgCopyAll ダイアログ

IMPLEMENT_DYNAMIC(CDlgWorkChgCopyAll, CDialogEx)

CDlgWorkChgCopyAll::CDlgWorkChgCopyAll(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgWorkChgCopyAll, pParent)
{
	select_org_work = -1;	//コピー元No初期化
	select_cpy_work = -1;	//コピー先No初期化
}

CDlgWorkChgCopyAll::~CDlgWorkChgCopyAll()
{
}

void CDlgWorkChgCopyAll::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WORK_LIST_ORG, list_origin.ctrl_work_list);
	DDX_Control(pDX, IDC_CHECK1, check_item_work);
	DDX_Control(pDX, IDC_CHECK2, check_item_calib);
	DDX_Control(pDX, IDC_EDIT1, edit_copy_origin);
	DDX_Control(pDX, IDC_EDIT2, edit_copy_copyto);
	DDX_Control(pDX, IDC_LIST3, copy_log);
	DDX_Control(pDX, IDC_STATIC_SELECT_MODEL, static_select_model);
	DDX_Control(pDX, IDC_STATIC_SOURCE, static_source);
	DDX_Control(pDX, IDC_STATIC_DESTINATION, static_destination);
	DDX_Control(pDX, IDC_BUTTON3, btn_select_source);
	DDX_Control(pDX, IDC_BUTTON4, btn_select_destination);
	DDX_Control(pDX, IDC_BUTTON5, btn_execute);
	DDX_Control(pDX, IDC_STATIC_LOG, static_log);
	DDX_Control(pDX, IDOK, btn_exit);
	DDX_Control(pDX, IDC_STATIC_ITEM, static_item);
}


BEGIN_MESSAGE_MAP(CDlgWorkChgCopyAll, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgWorkChgCopyAll::OnBnClickedOk)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_WORK_LIST_ORG, &CDlgWorkChgCopyAll::OnNMCustomdrawWorkListOrg)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgWorkChgCopyAll::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgWorkChgCopyAll::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgWorkChgCopyAll::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgWorkChgCopyAll::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgWorkChgCopyAll::OnBnClickedButton5)
END_MESSAGE_MAP()


// CDlgWorkChgCopyAll メッセージ ハンドラー


BOOL CDlgWorkChgCopyAll::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgWorkChgCopyAll in ***");

	dlg_x = 352;
	dlg_y = 192;
	dlg_width = 624;
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
void CDlgWorkChgCopyAll::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//コピー項目のチェック
	check_item_work.SetCheck(BST_CHECKED);		//初期値でチェックON
	check_item_calib.SetCheck(BST_CHECKED);		//初期値でチェックON

	//
	//コピーリスト
	//
	list_origin.page = 0;	//ページ初期化
	list_origin.work_list = new CWorkParam[100];

	//機種選択リストの準備
	list_origin.ctrl_work_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);				//Noカラム追加
	list_origin.ctrl_work_list.InsertColumn(1, Tips0.SelChar(language.jpn, "機種コメント", "Comment"), LVCFMT_LEFT, 200, -1);	//機種コメントカラム追加
	list_origin.ctrl_work_list.InsertColumn(2, Tips0.SelChar(language.jpn, "作成日", "Created Date"), LVCFMT_LEFT, 110, -1);	//作成日カラム追加

	//機種選択リスト内の初期処理
	//空文字でNo項目を100個作成する
	char p_org_number[4] = { '\0' };
	char p_org_comment[128] = { '\0' };
	char p_org_date[24] = { '\0' };
	LVITEM item_org_number;
	item_org_number.mask = LVIF_TEXT;
	item_org_number.iSubItem = 0;
	for (int i = 0; i < 100; i++) {
		//No表示
		item_org_number.pszText = p_org_number;
		item_org_number.iItem = i;
		list_origin.ctrl_work_list.InsertItem(&item_org_number);
	}

	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	list_origin.ctrl_work_list.SetExtendedStyle(list_origin.ctrl_work_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LoadParameterOrigin();	//コピー元リスト パラメータの展開
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1201));
		check_item_work.SetWindowTextA(language.GetCharacterString(1205));			//コピー項目　機種データ
		check_item_calib.SetWindowTextA(language.GetCharacterString(1206));			//コピー項目　キャリブレーションデータ
		static_select_model.SetWindowTextA(language.GetCharacterString(1202));		//機種を選択します
		static_source.SetWindowTextA(language.GetCharacterString(1209));			//コピー元
		static_destination.SetWindowTextA(language.GetCharacterString(1211));		//コピー先
		btn_select_source.SetWindowTextA(language.GetCharacterString(1210));		//選択ボタン元
		btn_select_destination.SetWindowTextA(language.GetCharacterString(1210));	//選択ボタン先
		btn_execute.SetWindowTextA(language.GetCharacterString(1207));				//実行ボタン
		static_log.SetWindowTextA(language.GetCharacterString(1208));				//実行ログ
		btn_exit.SetWindowTextA(language.GetCharacterString(1212));					//戻るボタン
		static_item.SetWindowTextA(language.GetCharacterString(1213));				//コピー項目
	}
}


/** ****************************************************** **
 ** コピーリスト パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::LoadParameterOrigin()
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
 ** 一括コピー実行 全項目
 ** 引数：
 **　int select_org_work	コピー元機種No
 **　int select_cpy_work	コピー先機種No
 **　CWorkParam::Registration* org_regi	コピー元機種パラメータ
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChgCopyAll::BulkCopyAll(int select_org_work, int select_cpy_work, CWorkParam::Registration* org_regi)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char org_path[MAX_PATH] = { '\0' };
	char cpy_path[MAX_PATH] = { '\0' };
	CString s_org_work;
	CString s_cpy_work;

	//機種No確認
	if (!(select_org_work >= 1 && select_org_work <= 999)) goto END;
	if (!(select_cpy_work >= 1 && select_cpy_work <= 999)) goto END;

	//コピー元パス
	strcpy_s(org_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_org_work = Tips0.IntToString(select_org_work, 3);
	strcat_s(org_path, s_org_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "元パス:", "Source path:"));
	copy_log.AddString((CString)org_path);

	//コピー先パス
	strcpy_s(cpy_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
	strcat_s(cpy_path, s_cpy_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "先パス:", "Distination path:"));
	copy_log.AddString((CString)cpy_path);

	copy_log.AddString(Tips0.SelStr(language.jpn, "一括コピー開始", "Start batch copy"));

	//一括コピー実行
	if (proc_file.CopyFileAllEx(org_path, cpy_path) == 0) {

		//機種パラメータの作成
		CreateParam(select_cpy_work, org_regi);

		rtn_result = 0;
	}

	//リスト更新
	LoadParameterOrigin();	//コピー元リスト パラメータの展開

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 一括コピー実行 テンプレートのみ
 ** 引数：
 **　int select_org_work	コピー元機種No
 **　int select_cpy_work	コピー先機種No
 **　CWorkParam::Registration* org_regi	コピー元機種パラメータ
 **　BOOL cpy_enable		コピー先登録状態
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChgCopyAll::BulkCopyTempl(int select_org_work, int select_cpy_work, CWorkParam::Registration* org_regi, BOOL cpy_enable)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char org_path[MAX_PATH] = { '\0' };
	char cpy_path[MAX_PATH] = { '\0' };
	CString s_org_work;
	CString s_cpy_work;

	//機種No確認
	if (!(select_org_work >= 1 && select_org_work <= 999)) goto END;
	if (!(select_cpy_work >= 1 && select_cpy_work <= 999)) goto END;

	//コピー元パス
	strcpy_s(org_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_org_work = Tips0.IntToString(select_org_work, 3);
	strcat_s(org_path, s_org_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "元パス:", "Source path:"));
	copy_log.AddString((CString)org_path);

	//コピー先パス
	strcpy_s(cpy_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
	strcat_s(cpy_path, s_cpy_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "先パス:", "Distination path:"));
	copy_log.AddString((CString)cpy_path);

	copy_log.AddString(Tips0.SelStr(language.jpn, "機種データコピー開始", "Model data copy started"));

	//ディレクトリ検索準備
	WIN32_FIND_DATA FindFiledata;
	HANDLE hFind = NULL;
	WIN32_FIND_DATA FindFiledata_del;
	HANDLE hFind_del = NULL;
	BOOL copy_complete = FALSE;		//コピー成功フラグ

	//コピー元ディレクトリの確認
	if (proc_file.CheckDir(org_path) != 0) {
		outlog.LogOutFile(1, "コピー元ディレクトリがありません(" + (CString)org_path + ")");
		goto END;
	}

	//コピー先ディレクトリの確認
	if (proc_file.CheckDir(cpy_path) == 0) {

		/********** コピー先テンプレートデータディレクトリを検索し削除 **********/

		//検索1回目
		hFind_del = FindFirstFileA((CString)cpy_path + "\\*.*", &FindFiledata_del);
		if (hFind_del == INVALID_HANDLE_VALUE) {
			//検索結果無し
			//通常はディレクトリは必ず見つかる　".."と"."が検索される

			//コピー先ディレクトリが存在しない場合
			goto FIND_END;
		}
		//ディレクトリ有り
		else {
			do {
				if ((FindFiledata_del.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& lstrcmpA(FindFiledata_del.cFileName, "..")
					&& lstrcmpA(FindFiledata_del.cFileName, ".")) {

					//検索されたディレクトリ名
					CString find_dir_del = FindFiledata_del.cFileName;

					//フォルダ文字'T'とフォルダ文字数でテンプレートデータフォルダを特定
					if (FindFiledata_del.cFileName[0] == 'T' && strlen(FindFiledata_del.cFileName) == 4) {
						//フォルダの削除
						if (proc_file.DeleteFileAllEx((CString)cpy_path + "\\" + find_dir_del, TRUE) == 0) {
							//特に処理なし
						}
						else {
							goto FIND_END;
							outlog.LogOutFile(1, "コピー先テンプレートディレクトリの削除に失敗しました(" + (CString)cpy_path + "\\" + find_dir_del + ")");
						}
					}
				}
			} while (FindNextFile(hFind_del, &FindFiledata_del));	//検索2回目以降
		}
		/********** ここまで **********/

	} else {
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

				//検索されたディレクトリ名
				CString find_dir = FindFiledata.cFileName;

				//フォルダ文字'T'とフォルダ文字数でテンプレートデータフォルダを特定
				if (FindFiledata.cFileName[0] == 'T' && strlen(FindFiledata.cFileName) == 4) {
					//一括コピー実行
					if (proc_file.CopyFileAllEx((CString)org_path + "\\" + find_dir, (CString)cpy_path + "\\" + find_dir) == 0) {
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
	if (hFind_del != NULL) FindClose(hFind_del);
	if (hFind != NULL) FindClose(hFind);

	//リスト更新
	LoadParameterOrigin();	//コピー元リスト パラメータの展開

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 一括コピー実行 スコープのみ
 ** 引数：
 **　int select_org_work	コピー元機種No
 **　int select_cpy_work	コピー先機種No
 **　CWorkParam::Registration* org_regi	コピー元機種パラメータ
 **　BOOL cpy_enable		コピー先登録状態
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗, 2:データなし
 ** 特記事項：
 ** ****************************************************** **/
int CDlgWorkChgCopyAll::BulkCopyScope(int select_org_work, int select_cpy_work, CWorkParam::Registration* org_regi, BOOL cpy_enable)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char org_path[MAX_PATH] = { '\0' };
	char cpy_path[MAX_PATH] = { '\0' };
	CString s_org_work;
	CString s_cpy_work;

	//機種No確認
	if (!(select_org_work >= 1 && select_org_work <= 999)) goto END;
	if (!(select_cpy_work >= 1 && select_cpy_work <= 999)) goto END;

	//コピー元パス
	strcpy_s(org_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_org_work = Tips0.IntToString(select_org_work, 3);
	strcat_s(org_path, s_org_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "元パス:", "Source path:"));
	copy_log.AddString((CString)org_path);

	//コピー先パス
	strcpy_s(cpy_path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
	strcat_s(cpy_path, s_cpy_work);
	copy_log.AddString(Tips0.SelStr(language.jpn, "先パス:", "Distination path:"));
	copy_log.AddString((CString)cpy_path);

	copy_log.AddString(Tips0.SelStr(language.jpn, "キャリブレーションデータコピー開始", "Calibration data copy started"));

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

		//コピー先スコープデータディレクトリを削除
		proc_file.DeleteFileAllEx((CString)cpy_path + "\\SCOPE", TRUE);
	}
	else {
		//コピー先ディレクトリの作成
		if (CreateDirectoryA(cpy_path, NULL) == 0) {
			outlog.LogOutFile(1, "コピー先ディレクトリの作成に失敗しました(" + (CString)cpy_path + ")");
			goto FIND_END;
		}
	}


	/********** コピー元スコープデータディレクトリの検索とコピー **********/

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

				//検索ディレクトリ名"SCOPE"
				char* search_folder = { "SCOPE" };

				//検索ディレクトリ名を比較
				if (Tips0.CompCharacters(FindFiledata.cFileName, search_folder) == 0) {
					//一括コピー実行
					if (proc_file.CopyFileAllEx((CString)org_path + "\\SCOPE", (CString)cpy_path + "\\SCOPE") == 0) {
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
	LoadParameterOrigin();	//コピー元リスト パラメータの展開

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
void CDlgWorkChgCopyAll::CreateParam(int select_cpy_work, CWorkParam::Registration* org_regi)
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
BOOL CDlgWorkChgCopyAll::GetReloadAns()
{
	// TODO: ここに実装コードを追加します.
	return current_update_ans;
}


/** ****************************************************** **
 ** コピーリストページ切替(<<)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	list_origin.page = list_origin.page - 1;
	if (list_origin.page < 0) list_origin.page = 9;

	LoadParameterOrigin();	//コピー元リスト パラメータの展開
}


/** ****************************************************** **
 ** コピーリストページ切替(>>)ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	list_origin.page = list_origin.page + 1;
	if (list_origin.page > 9) list_origin.page = 0;

	LoadParameterOrigin();	//コピー元リスト パラメータの展開
}


/** ****************************************************** **
 ** コピー元選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//コピー元選択行の取得
	int select_org_row = list_origin.ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_org_row < 0 || select_org_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "リストから機種を選択してください", "Please select a model from the list");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	select_org_work = list_origin.page * 100 + select_org_row;	//コピー元機種No

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
	org_work_regi = list_origin.work_list[select_org_row].regi;		//コピー元登録パラメータの確保

	CString comment1 = Tips0.SelStr(language.jpn, "機種No", "Model No") + Tips0.IntToString(select_org_work, 3);
	edit_copy_origin.SetWindowTextA(comment1 + " " + org_work_regi.comment);
}


/** ****************************************************** **
 ** コピー先選択ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	//コピー先選択行の取得
	int select_cpy_row = list_origin.ctrl_work_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_cpy_row < 0 || select_cpy_row > 99) {
		msg1 = Tips0.SelStr(language.jpn, "リストから機種を選択してください", "Please select a model from the list");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	select_cpy_work = list_origin.page * 100 + select_cpy_row;	//コピー先機種No

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
	cpy_work_regi = list_origin.work_list[select_cpy_row].regi;		//コピー先登録パラメータの確保

	CString comment1 = Tips0.SelStr(language.jpn, "機種No", "Model No") + Tips0.IntToString(select_cpy_work, 3);
	edit_copy_copyto.SetWindowTextA(comment1 + " " + cpy_work_regi.comment);
}


/** ****************************************************** **
 ** 実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2, msg3, msg4;
	this->GetWindowTextA(title);

	//コピー元選択の確認
	if (select_org_work < 0 || select_org_work > 999) {
		msg1 = Tips0.SelStr(language.jpn, "コピー元機種を選択してください", "Please select Source");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//コピー先選択の確認
	if (select_cpy_work < 0 || select_cpy_work > 999) {
		msg1 = Tips0.SelStr(language.jpn, "コピー先機種を選択してください", "Please select destination");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//コピー元とコピー先が同じ場合
	if (select_org_work == select_cpy_work) {
		//選択ミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種コピー失敗", "Copy failure");
		msg2 = Tips0.SelStr(language.jpn, "コピー元機種Noとコピー先機種Noが同じです", "Source No and Destination No are the same");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	//コピー項目チェック確認
	if (check_item_work.GetCheck() == BST_UNCHECKED && check_item_calib.GetCheck() == BST_UNCHECKED) {
		//項目チェックミスメッセージ表示
		msg1 = Tips0.SelStr(language.jpn, "機種コピー失敗", "Copy failure");
		msg2 = Tips0.SelStr(language.jpn, "コピー項目が選択されていません。", "No copy item selected");
		MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONWARNING);
		return;
	}

	//コピー実行確認
	if (org_work_regi.enable) {

		//コピー項目
		BOOL do_copy_templ = FALSE;	//テンプレートコピーフラグ
		BOOL do_copy_scope = FALSE;	//スコープコピーフラグ

		msg1 = Tips0.SelStr(language.jpn, "機種一括コピー実行", "Batch copy execution of models");

		CString comment1 = Tips0.SelStr(language.jpn, "コピー元機種No", "Source No") + Tips0.IntToString(select_org_work, 3);
		if (select_org_work == current.work_number) comment1 = comment1 + Tips0.SelStr(language.jpn, "(現在機種)", "(Current model)");
		
		CString comment2 = Tips0.SelStr(language.jpn, "をコピー先機種No", " to Destination No") + Tips0.IntToString(select_cpy_work, 3);
		if (select_cpy_work == current.work_number) comment2 = comment2 + Tips0.SelStr(language.jpn, "(現在機種)", "(Current model)");

		if (language.jpn) {
			msg2 = comment1 + comment2 + "に一括コピーします。";
		}
		else {
			msg2 = "Batch copy " + comment1 + comment2;
		}

		//コピー項目
		if (check_item_work.GetCheck() == BST_CHECKED && check_item_calib.GetCheck() == BST_CHECKED) {
			msg3 = Tips0.SelStr(language.jpn, "コピー対象データは全てです。", "All the data to be copied");
			do_copy_templ = TRUE;
			do_copy_scope = TRUE;
		}
		else if (check_item_calib.GetCheck() == BST_UNCHECKED) {
			msg3 = Tips0.SelStr(language.jpn, "コピー対象データは機種データのみです。", "The data to be copied is only model data");
			do_copy_templ = TRUE;
		}
		else if (check_item_work.GetCheck() == BST_UNCHECKED) {
			msg3 = Tips0.SelStr(language.jpn, "コピー対象データはキャリブレーションデータのみです。", "The data to be copied is only calibration data");
			do_copy_scope = TRUE;
		}

		msg4 = Tips0.SelStr(language.jpn, "よろしいですか？", "Is it OK?");

		//ボタン無効化
		btn_execute.EnableWindow(FALSE);
		btn_exit.EnableWindow(FALSE);

		int judge = MessageBoxA(msg1 + "\n" + msg2 + "\n" + msg3 + "\n" + msg4, title, MB_OKCANCEL | MB_ICONQUESTION);
		if (judge == IDOK) {
			//コピー実行
			int rtn_cpy = 1;

			copy_log.ResetContent();	//ログリストのクリア

			copy_log.AddString(Tips0.SelStr(language.jpn, "コピー元No:", "Source No:") + Tips0.IntToString(select_org_work, 3));
			copy_log.AddString(Tips0.SelStr(language.jpn, "コピー先No:", "Destination No:") + Tips0.IntToString(select_cpy_work, 3));

			//全項目コピー
			if (do_copy_templ && do_copy_scope) {
				rtn_cpy = BulkCopyAll(select_org_work, select_cpy_work, &org_work_regi);
			}
			//機種データのみ
			else if (do_copy_templ && !do_copy_scope) {
				rtn_cpy = BulkCopyTempl(select_org_work, select_cpy_work, &org_work_regi, cpy_work_regi.enable);
			}
			//キャリブレーションデータのみ
			else if (!do_copy_templ && do_copy_scope) {
				rtn_cpy = BulkCopyScope(select_org_work, select_cpy_work, &org_work_regi, cpy_work_regi.enable);
			}

			CString s_org_work = Tips0.IntToString(select_org_work, 3);
			CString s_cpy_work = Tips0.IntToString(select_cpy_work, 3);
			if (rtn_cpy == 0) {
				copy_log.AddString(Tips0.SelStr(language.jpn, "コピー成功", "Successful copy"));

				//成功メッセージ
				msg1 = Tips0.SelStr(language.jpn, "一括コピー実行", "Batch copy execution");
				msg2 = Tips0.SelStr(language.jpn, "一括コピーに成功しました。", "Batch copy was successful");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
				outlog.LogOutFile(1, "機種一括コピー成功 元機種No" + s_org_work + ", 先機種No" + s_cpy_work);
			}
			else if (rtn_cpy == 2){
				copy_log.AddString(Tips0.SelStr(language.jpn, "コピー失敗", "Copy failure"));

				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "一括コピー実行", "Batch copy execution");
				msg2 = Tips0.SelStr(language.jpn, "コピー対象のデータがありませんでした。", "There was no data to copy");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "機種一括コピー失敗 元機種No" + s_org_work + ", 先機種No" + s_cpy_work);
			}
			else {
				copy_log.AddString(Tips0.SelStr(language.jpn, "コピー失敗", "Copy failure"));

				//失敗メッセージ
				msg1 = Tips0.SelStr(language.jpn, "一括コピー実行", "Batch copy execution");
				msg2 = Tips0.SelStr(language.jpn, "一括コピーに失敗しました。", "Batch copy has failed");
				MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
				outlog.LogOutFile(1, "機種一括コピー失敗 元機種No" + s_org_work + ", 先機種No" + s_cpy_work);
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
void CDlgWorkChgCopyAll::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgWorkChgCopyAll out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgCopyAll::OnDestroy()
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
void CDlgWorkChgCopyAll::OnNMCustomdrawWorkListOrg(NMHDR *pNMHDR, LRESULT *pResult)
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
BOOL CDlgWorkChgCopyAll::PreTranslateMessage(MSG* pMsg)
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
