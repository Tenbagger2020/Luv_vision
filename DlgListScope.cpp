/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgListScope.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：スコープデータ一覧ダイアログ
 **		キャリブレーションで算出したスコープデータを表示する
 **/

// DlgListScope.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgListScope.h"
#include "afxdialogex.h"

#include "DlgEditScope.h"

extern CScopeParam	scope_param[16];	//機種毎スコープパラメータ
extern COutputLog	outlog;				//ログ出力
extern CLanguage	language;			//言語設定用

// CDlgListScope ダイアログ

IMPLEMENT_DYNAMIC(CDlgListScope, CDialogEx)

CDlgListScope::CDlgListScope(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgListScope, pParent)
{
	select_scope = 0;	//初期スコープ1
}

CDlgListScope::~CDlgListScope()
{
}

void CDlgListScope::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, combo_set_scope);
	DDX_Control(pDX, IDC_LIST1, ctrl_scrp_list);
	DDX_Control(pDX, IDC_STATIC_COMMENT, str_comment);
	DDX_Control(pDX, IDC_STATIC_SELECT_SCOPE, static_select_scope);
	DDX_Control(pDX, IDC_STATIC_EXPLAN, static_explan);
	DDX_Control(pDX, IDC_STATIC_PLZ_DOUBLE_CLICK, static_plz_double_click);
}


BEGIN_MESSAGE_MAP(CDlgListScope, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgListScope::OnCbnSelchangeCombo1)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, &CDlgListScope::OnNMDblclkList1)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, &CDlgListScope::OnNMClickList1)
	ON_BN_CLICKED(IDOK, &CDlgListScope::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgListScope メッセージ ハンドラー


BOOL CDlgListScope::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgListScope in ***");

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = 464;
	dlg_height = 320;
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
void CDlgListScope::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//選択スコープ
	for (int i = 1; i <= 16; i++) {
		combo_set_scope.AddString(Tips0.SelStr(language.jpn, "スコープ", "Scope") + Tips0.IntToString(i));
	}

	//スコープリストの準備
	ctrl_scrp_list.InsertColumn(0, Tips0.SelChar(language.jpn, "項目", "Item"), LVCFMT_LEFT, 120, -1);			//項目カラム追加
	ctrl_scrp_list.InsertColumn(1, Tips0.SelChar(language.jpn, "視野1  ", "View1  "), LVCFMT_RIGHT, 70, -1);	//視野1カラム追加
	ctrl_scrp_list.InsertColumn(2, Tips0.SelChar(language.jpn, "視野2  ", "View2  "), LVCFMT_RIGHT, 70, -1);	//視野2カラム追加
	ctrl_scrp_list.InsertColumn(3, Tips0.SelChar(language.jpn, "視野3  ", "View3  "), LVCFMT_RIGHT, 70, -1);	//視野3カラム追加
	ctrl_scrp_list.InsertColumn(4, Tips0.SelChar(language.jpn, "視野4  ", "View4  "), LVCFMT_RIGHT, 70, -1);	//視野4カラム追加

	//リスト内の初期処理
	//空文字で項目を作成する
	//画素サイズ
	//視野傾き
	//Cal基準
	//Cal基準位置
	//回転中心
	LVITEM item_name;
	item_name.mask = LVIF_TEXT;
	item_name.iSubItem = 0;
	for (int i = 0; i < 8; i++) {
		//項目追加
		if (i == 0) item_name.pszText = Tips0.SelChar(language.jpn, "画素サイズ[um]", "Pixel Size[um]");
		if (i == 1) item_name.pszText = Tips0.SelChar(language.jpn, "視野傾き[deg]", "View Angle[deg]");
		if (i == 2) item_name.pszText = Tips0.SelChar(language.jpn, "Cal基準X[pixel]", "Cal Base X[pixel]");
		if (i == 3) item_name.pszText = Tips0.SelChar(language.jpn, "Cal基準Y[pixel]", "Cal Base Y[pixel]");
		if (i == 4) item_name.pszText = Tips0.SelChar(language.jpn, "Cal基準位置X[mm]", "Cal Base Pos X[mm]");
		if (i == 5) item_name.pszText = Tips0.SelChar(language.jpn, "Cal基準位置Y[mm]", "Cal Base Pos Y[mm]");
		if (i == 6) item_name.pszText = Tips0.SelChar(language.jpn, "回転中心X[mm]", "Rot Center X[mm]");
		if (i == 7) item_name.pszText = Tips0.SelChar(language.jpn, "回転中心Y[mm]", "Rot Center Y[mm]");
		item_name.iItem = i;
		ctrl_scrp_list.InsertItem(&item_name);
	}
																	//リストのスタイルを拡張(全てのアイテムを選択/グリッド線を表示)
	ctrl_scrp_list.SetExtendedStyle(ctrl_scrp_list.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::ChangeLanguage()
{
	POINT pt;
	RECT re;
	int width, height;

	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2201));
		static_select_scope.SetWindowTextA(language.GetCharacterString(2202));		//参照するスコープ
		static_explan.SetWindowTextA(language.GetCharacterString(2203));			//説明文
		static_plz_double_click.SetWindowTextA(language.GetCharacterString(2217));	//ダブルクリックしてください

		//コントロール位置・サイズ調整
		combo_set_scope.GetWindowRect(&re);	//スコープコンボボックス
		pt.x = re.left;
		pt.y = re.top;
		width = re.right - re.left;
		height = re.bottom - re.top;
		this->ScreenToClient(&pt);
		combo_set_scope.MoveWindow(pt.x - 20, pt.y, width, height, TRUE);	//スコープコンボボックス
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
void CDlgListScope::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//スコープ選択
	combo_set_scope.SetCurSel(select_scope);

	LoadScorp();		//選択スコープの展開
}


/** ****************************************************** **
 ** 選択スコープの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::LoadScorp()
{
	// TODO: ここに実装コードを追加します.

	CScopeParam* param = &scope_param[select_scope];

	//対象スコープのキャリブレーション結果を表示する
	char p_pixel_size[16] = { '\0' };
	char p_crdnt_deg[16] = { '\0' };
	char p_cal_base_x[16] = { '\0' };
	char p_cal_base_y[16] = { '\0' };
	char p_axis_base_x[16] = { '\0' };
	char p_axis_base_y[16] = { '\0' };
	char p_rotation_center_x[16] = { '\0' };
	char p_rotation_center_y[16] = { '\0' };

	LVITEM item_view[4];
	for (int i = 0; i < 4; i++) {
		item_view[i].mask = LVIF_TEXT;
		item_view[i].iSubItem = i + 1;
	}
	for (int i = 0; i < 4; i++) {

		//画素サイズ表示
		double pixel_um = param->view[i].calib.pixel_size * (double)1000;
		strcpy_s(p_pixel_size, sizeof(p_pixel_size), Tips0.DblToString(pixel_um, 2, TRUE));
		item_view[i].pszText = p_pixel_size;
		item_view[i].iItem = 0;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//視野傾き表示
		double deg = Tips0.RadianToDeg(param->view[i].calib.crdnt_radian);
		strcpy_s(p_crdnt_deg, sizeof(p_crdnt_deg), Tips0.DblToString(deg, 4, TRUE));
		item_view[i].pszText = p_crdnt_deg;
		item_view[i].iItem = 1;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//Cal基準X表示
		strcpy_s(p_cal_base_x, sizeof(p_cal_base_x), Tips0.DblToString(param->view[i].calib.pixel.cal_base_x, 4, TRUE));
		item_view[i].pszText = p_cal_base_x;
		item_view[i].iItem = 2;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//Cal基準Y表示
		strcpy_s(p_cal_base_y, sizeof(p_cal_base_y), Tips0.DblToString(param->view[i].calib.pixel.cal_base_y, 4, TRUE));
		item_view[i].pszText = p_cal_base_y;
		item_view[i].iItem = 3;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//Cal基準位置X表示
		strcpy_s(p_axis_base_x, sizeof(p_axis_base_x), Tips0.DblToString(param->view[i].calib.pixel.axis_base_x, 4, TRUE));
		item_view[i].pszText = p_axis_base_x;
		item_view[i].iItem = 4;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//Cal基準位置Y表示
		strcpy_s(p_axis_base_y, sizeof(p_axis_base_y), Tips0.DblToString(param->view[i].calib.pixel.axis_base_y, 4, TRUE));
		item_view[i].pszText = p_axis_base_y;
		item_view[i].iItem = 5;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//回転中心X表示
		if (param->conf_rotcent_view == i) {
			strcpy_s(p_rotation_center_x, sizeof(p_rotation_center_x), Tips0.DblToString(param->view[i].calib.theta.rotation_center_x, 4, TRUE));
		}
		else {
			p_rotation_center_x[0] = '\0';
		}
		item_view[i].pszText = p_rotation_center_x;
		item_view[i].iItem = 6;
		ctrl_scrp_list.SetItem(&item_view[i]);
		//回転中心Y表示
		if (param->conf_rotcent_view == i) {
			strcpy_s(p_rotation_center_y, sizeof(p_rotation_center_y), Tips0.DblToString(param->view[i].calib.theta.rotation_center_y, 4, TRUE));
		}
		else {
			p_rotation_center_y[0] = '\0';
		}
		item_view[i].pszText = p_rotation_center_y;
		item_view[i].iItem = 7;
		ctrl_scrp_list.SetItem(&item_view[i]);
	}
}


/** ****************************************************** **
 ** スコープコンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	select_scope = combo_set_scope.GetCurSel();

	LoadScorp();		//選択スコープの展開
}


/** ****************************************************** **
 ** リストビューコントロール(スコープ)Dblclk
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::OnNMDblclkList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	//選択行の取得
	int select_row = ctrl_scrp_list.GetNextItem(-1, LVNI_SELECTED);

	if (select_row >= 0 && select_row <= 7) {
		CDlgEditScope dlg_edit_scope;
		dlg_edit_scope.SetParam(select_scope, select_row);	//パラメータのセット
		dlg_edit_scope.DoModal();

		LoadScorp();		//データの更新
	}
}


/** ****************************************************** **
 ** リストビューコントロール(スコープ)Click
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::OnNMClickList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	//選択行の取得
	int select_row = ctrl_scrp_list.GetNextItem(-1, LVNI_SELECTED);

	if (select_row >= 0 && select_row <= 7) {
		//コメント表示
		CString comment;
		if (select_row == 0) comment = Tips0.SelStr(language.jpn,
			"画素サイズ:1画素の大きさ",
			"Pixel Size : 1 pixel size");
		if (select_row == 1) comment = Tips0.SelStr(language.jpn,
			"視野傾き:カメラ視野の傾き",
			"View Angle : View angle of camera field of view");
		if (select_row == 2 || select_row == 3) comment = Tips0.SelStr(language.jpn,
			"Cal基準:左上からのキャリブレーションライン中心の位置",
			"Cal Base : Calibration line center position from top left");
		if (select_row == 4 || select_row == 5) comment = Tips0.SelStr(language.jpn,
			"Cal基準位置:画素キャリブレーション実施時のライン中心位置",
			"Cal Base Pos : Line center position when image calibration is performed");
		if (select_row == 6 || select_row == 7) comment = Tips0.SelStr(language.jpn,
			"回転中心:補正θ軸の回転中心の位置",
			"Rot Center : Rot center position of Align θ axis");

		if (select_row >= 0 && select_row <= 7) str_comment.SetWindowTextA(comment);
	}
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListScope::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgListScope out ***");

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
BOOL CDlgListScope::PreTranslateMessage(MSG* pMsg)
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
