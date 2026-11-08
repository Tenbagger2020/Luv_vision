/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgCopyTemplate.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 ** 注記：
 ** 説明：テンプレートコピーダイアログ
 **		テンプレートを選択しコピーする
 **/

// DlgCopyTemplate.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgCopyTemplate.h"
#include "afxdialogex.h"

#include "ProcFileOpe.h"

//DLL
#include "AlignBarcode.h"	//AlignBarcode DLL Header

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CScopes			sys_scope[16];			//システムスコープパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CTemplatesGroup	grp_template;			//テンプレートグループパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ
extern CAccessParamTemp	access_tmp;		//機種毎テンプレートパラメータファイルアクセス
extern CAccessParamMes	access_mes;		//機種毎検査パラメータファイルアクセス
extern CProcFileOpe		proc_file;		//ファイル処理クラス
extern CCurrentStatus	current;		//共有情報
extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

//DLL
extern AlignBarcode::Param::CBarcodeParam bcr_param[256];	//機種毎バーコードリーダーパラメータ

extern char* APP_ROOT;

// CDlgCopyTemplate ダイアログ

IMPLEMENT_DYNAMIC(CDlgCopyTemplate, CDialogEx)

CDlgCopyTemplate::CDlgCopyTemplate(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgCopyTemplate, pParent)
{
	//コピーモード 初期設定
	use_copy_mode = 0;	//コピー元

	item_count = 0;	//検索された数の初期化
}

CDlgCopyTemplate::~CDlgCopyTemplate()
{
}

void CDlgCopyTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_TEMP_NO, temp_num);
	DDX_Control(pDX, IDC_STATIC_COMMENT, str_comment);
	DDX_Control(pDX, IDC_RADIO1, radio_copy_org);
	DDX_Control(pDX, IDC_RADIO2, radio_copy_to);
	DDX_Control(pDX, IDC_LIST1_TEMP, ctrl_tmpl_list);
	DDX_Control(pDX, IDC_BUTTON1, btn_copy);
	DDX_Control(pDX, IDOK, btn_exit);
}


BEGIN_MESSAGE_MAP(CDlgCopyTemplate, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCopyTemplate::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RADIO1, &CDlgCopyTemplate::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CDlgCopyTemplate::OnBnClickedRadio2)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST1_TEMP, &CDlgCopyTemplate::OnNMCustomdrawList1Temp)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgCopyTemplate::OnBnClickedButton1)
END_MESSAGE_MAP()


// CDlgCopyTemplate メッセージ ハンドラー


BOOL CDlgCopyTemplate::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgCopyTemplate in ***");

	dlg_x = 480;
	dlg_y = 240;
	dlg_width = 448;
	dlg_height = 384;
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
void CDlgCopyTemplate::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//テンプレート選択リストの準備
	ctrl_tmpl_list.InsertColumn(0, "No", LVCFMT_LEFT, 50, -1);		//Noカラム追加
	ctrl_tmpl_list.InsertColumn(1, Tips0.SelChar(language.jpn, "名称", "Name"), LVCFMT_LEFT, 130, -1);	//名称カラム追加
	ctrl_tmpl_list.InsertColumn(2, Tips0.SelChar(language.jpn, "説明", "Comment"), LVCFMT_LEFT, 184, -1);	//説明カラム追加
																
	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	ctrl_tmpl_list.SetExtendedStyle(ctrl_tmpl_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2701));
		radio_copy_org.SetWindowTextA(language.GetCharacterString(2703));	//コピー元ラジオ
		radio_copy_to.SetWindowTextA(language.GetCharacterString(2704));	//コピー先ラジオ
		btn_copy.SetWindowTextA(language.GetCharacterString(2708));			//コピー実行ボタン
		btn_exit.SetWindowTextA(language.GetCharacterString(2709));			//閉じるボタン
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
void CDlgCopyTemplate::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//名称
	CString name = sys_template[template_number].name;
	temp_num.SetWindowTextA(Tips0.SelStr(language.jpn, "テンプレートNo.", "Template No.") + Tips0.IntToString(template_number, 3) + "(" + name + ")");

	//コピー選択
	radio_copy_org.SetCheck(1);		//コピー元

	UpdateComment(use_copy_mode);	//コメント更新

	LoadTemplate();		//選択テンプレートの展開
}


/** ****************************************************** **
 ** コメント更新
 ** 引数：
 **　int copy_mode	コピーモード 0:コピー元、1:コピー先
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::UpdateComment(int copy_mode)
{
	// TODO: ここに実装コードを追加します.

	switch (copy_mode) {
	case 0:
		str_comment.SetWindowTextA(Tips0.SelChar(language.jpn, "コピー先を選択してください", "Please select a copy destination"));
		break;
	case 1:
		str_comment.SetWindowTextA(Tips0.SelChar(language.jpn, "コピー元を選択してください", "Please select the copy source"));
		break;
	}
}


/** ****************************************************** **
 ** 選択テンプレートの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::LoadTemplate()
{
	// TODO: ここに実装コードを追加します.

	int scope = sys_template[template_number].scope;
	int view = sys_template[template_number].view;
	int method_type = sys_template[template_number].method_type;
	int target_cam = sys_scope[scope].view[view].assign_camera;

	int first_number = grp_template.group[group_number].first_number;	//グループ先頭No
	int end_number = grp_template.group[group_number].end_number;		//グループ終端No

	//コピー対象テンプレートの検索
	//検索されたテンプレートのみ表示する
	//対象テンプレートはグループ内のみ
	char tmpl_number[4] = { '\0' };
	char tmpl_name[32] = { '\0' };
	char tmpl_comment[32] = { '\0' };
	LVITEM item_number;
	item_number.mask = LVIF_TEXT;
	item_number.iSubItem = 0;
	LVITEM item_name;
	item_name.mask = LVIF_TEXT;
	item_name.iSubItem = 1;
	LVITEM item_comment;
	item_comment.mask = LVIF_TEXT;
	item_comment.iSubItem = 2;
	item_count = 0;	//追加数
	for (int i = first_number; i <= end_number; i++) {
		if (i == template_number) goto FOR_END;

		int select_scope = sys_template[i].scope;
		int select_view = sys_template[i].view;
		int select_method_type = sys_template[i].method_type;
		int select_cam = sys_scope[select_scope].view[select_view].assign_camera;

		//テンプレート追加
		if (target_cam == select_cam && method_type == select_method_type) {
			//No表示
			Tips0.IntToChar(tmpl_number, sizeof(tmpl_number), i, 3);
			item_number.pszText = tmpl_number;
			item_number.iItem = item_count;
			ctrl_tmpl_list.InsertItem(&item_number);
			//名称表示
			strcpy_s(tmpl_name, sizeof(tmpl_name), sys_template[i].name);
			item_name.pszText = tmpl_name;
			item_name.iItem = item_count;
			ctrl_tmpl_list.SetItem(&item_name);
			//説明表示
			strcpy_s(tmpl_comment, sizeof(tmpl_comment), sys_template[i].comment);
			item_comment.pszText = tmpl_comment;
			item_comment.iItem = item_count;
			ctrl_tmpl_list.SetItem(&item_comment);

			item_temp[item_count] = i;	//検索されたテンプレート番号

			item_count++;
		}

	FOR_END:;
	}
}


/** ****************************************************** **
 ** テンプレートコピー実行
 ** 引数：
 **　int select_org_tmpl	コピー元対象テンプレート
 **　int select_cpy_tmpl	コピー先対象テンプレート
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCopyTemplate::CopyTemplate(int select_org_tmpl, int select_cpy_tmpl)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char path[MAX_PATH] = { '\0' };
	char org_tmpl_folder[5] = { '\0' };
	char cpy_tmpl_folder[5] = { '\0' };
	CString s_work;

	//パス
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_work = Tips0.IntToString(current.work_number, 3);
	strcat_s(path, s_work);

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

	//ディレクトリの確認
	if (proc_file.CheckDir(path) == 0) {
		//コピー先テンプレートディレクトリを削除
		proc_file.DeleteFileAllEx((CString)path + "\\" + (CString)cpy_tmpl_folder, TRUE);
	}
	else {
		outlog.LogOutFile(1, "機種ディレクトリがありません(" + (CString)path + ")");
		goto END;
	}

	/********** コピー元テンプレートデータディレクトリの検索とコピー **********/

	//検索1回目
	hFind = FindFirstFileA((CString)path + "\\*.*", &FindFiledata);
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
					if (proc_file.CopyFileAllEx((CString)path + "\\" + (CString)org_tmpl_folder, (CString)path + "\\" + (CString)cpy_tmpl_folder) == 0) {
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

	/********** ここまで **********/

	rtn_result = 0;	//処理成功

FIND_END:
	if (hFind != NULL) FindClose(hFind);

END:
	return rtn_result;
}


/** ****************************************************** **
 ** コピーテンプレート番号の入力
 ** 引数：
 **　int number		テンプレート番号
 **　int group		グループ番号(0～3)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::SetNumber(int number, int group)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
	group_number = group;
}


/** ****************************************************** **
 ** コピー元ラジオボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::OnBnClickedRadio1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	use_copy_mode = 0;	//コピー元

	UpdateComment(use_copy_mode);	//コメント更新
}


/** ****************************************************** **
 ** コピー先ラジオボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::OnBnClickedRadio2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	use_copy_mode = 1;	//コピー先

	UpdateComment(use_copy_mode);	//コメント更新
}


/** ****************************************************** **
 ** コピー実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2, msg3, msg4;
	this->GetWindowTextA(title);

	//テンプレート選択行の取得
	int select_tmpl_row = ctrl_tmpl_list.GetNextItem(-1, LVNI_SELECTED);
	if (select_tmpl_row < 0 || select_tmpl_row >= item_count) {
		msg1 = Tips0.SelStr(language.jpn, "対象テンプレートを選択してください", "Please select the template");
		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
		return;
	}

	//選択したテンプレート
	int select_tmpl_num = item_temp[select_tmpl_row];

	//コピー実行確認
	int copy_org;
	int copy_to;
	if (use_copy_mode == 0) {
		copy_org = template_number;
		copy_to = select_tmpl_num;
	}
	else {
		copy_org = select_tmpl_num;
		copy_to = template_number;
	}

	msg1 = Tips0.SelStr(language.jpn, "テンプレートコピー実行\n以下のテンプレートをコピーします。", "Execute template copy\nCopy the following template");
	msg2 = Tips0.SelStr(language.jpn, "　コピー元テンプレートNo", "  Source No") + Tips0.IntToString(copy_org, 3);
	msg3 = Tips0.SelStr(language.jpn, "　コピー先テンプレートNo", "  Destination No") + Tips0.IntToString(copy_to, 3);
	msg4 = Tips0.SelStr(language.jpn, "よろしいですか？", "Is it OK?");

	int judge = MessageBoxA(msg1 + "\n" + msg2 + "\n" + msg3 + "\n" + msg4, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {

		//コピー実行
		int rtn_cpy = CopyTemplate(copy_org, copy_to);

		CString s_org_tmpl = Tips0.IntToString(copy_org, 3);
		CString s_cpy_tmpl = Tips0.IntToString(copy_to, 3);
		if (rtn_cpy == 0) {
			//成功メッセージ
			msg1 = Tips0.SelStr(language.jpn, "テンプレートコピー実行", "Execute template copy");
			msg2 = Tips0.SelStr(language.jpn, "コピーに成功しました。", "The copy was successful");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
			outlog.LogOutFile(1, "テンプレートコピー成功 元テンプレートNo" + s_org_tmpl + ", 先テンプレートNo" + s_cpy_tmpl);
		}
		else if (rtn_cpy == 2) {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "テンプレートコピー実行", "Execute template copy");
			msg2 = Tips0.SelStr(language.jpn, "コピー対象のデータがありませんでした。", "There was no data to copy");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "テンプレートコピー失敗 元テンプレートNo" + s_org_tmpl + ", 先テンプレートNo" + s_cpy_tmpl);
		}
		else {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "テンプレートコピー実行", "Execute template copy");
			msg2 = Tips0.SelStr(language.jpn, "コピーに失敗しました。", "Copy failed");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "テンプレートコピー失敗 元テンプレートNo" + s_org_tmpl + ", 先テンプレートNo" + s_cpy_tmpl);
		}

		//テンプレートパラメータ読込実行
		ChangeParam(copy_to);
	}
}


/** ****************************************************** **
 ** テンプレートパラメータ読込実行
 ** 引数：
 **　int change_temp	読込テンプレート番号(0～255)
 ** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCopyTemplate::ChangeParam(int change_temp)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//機種テンプレートの読み込み
	template_param[change_temp].InitialParam();
	if (sys_template[change_temp].method_type != 0x00) {
		if (access_tmp.CheckIniFileParam(current.work_number, change_temp) == 0) access_tmp.ReadIniFileParam(current.work_number, change_temp);
	}

	//機種検査パラメータの読み込み
	mes_param[change_temp].InitialParam();
	//計測モードを確認
	if (sys_template[change_temp].method_type == 0x02) {
		//TMeasComm.iniファイルの読込
		if (access_mes.CheckIniFilePrmCom(current.work_number, change_temp) == 0) access_mes.ReadIniFilePrmCom(current.work_number, change_temp);

		//シート0～7を確認
		for (int j = 0; j < 8; j++) {
			//TMeasSh*.iniの読込
			if (access_mes.CheckIniFilePrmSht(current.work_number, change_temp, j) == 0) access_mes.ReadIniFilePrmSht(current.work_number, change_temp, j);
			//TPlcSh*.iniの読込
			if (access_mes.CheckIniFilePrmPlc(current.work_number, change_temp, j) == 0) access_mes.ReadIniFilePrmPlc(current.work_number, change_temp, j);
		}
	}

	//機種バーコードパラメータの読み込み
	bcr_param[change_temp].InitialParam();
	//バーコードを確認
	if (sys_template[change_temp].method_type == 0x03) {
		//Barcode.iniファイルの読込
		AlignBarcode::Param::CAccessParamBarcode access_bcr;
		if (access_bcr.CheckIniFilePrmBarcode(current.work_number, change_temp) == 0) {
			access_bcr.ReadIniFilePrmBarcode(current.work_number, change_temp, &bcr_param[change_temp]);
		}
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCopyTemplate::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgCopyTemplate out ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
** テンプレート選択リスト 選択背景の色変更
** 特記事項：
** ****************************************************** **/
void CDlgCopyTemplate::OnNMCustomdrawList1Temp(NMHDR *pNMHDR, LRESULT *pResult)
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
BOOL CDlgCopyTemplate::PreTranslateMessage(MSG* pMsg)
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
