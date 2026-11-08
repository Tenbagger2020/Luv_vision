/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgCreateCopyImage.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2023.11.08	M.Iwasaki	10.15.1.1	初版
 ** 注記：
 ** 説明：画像コピーダイアログ
 **		画像コピー範囲を選択しコピーする
 **
 **/


// DlgCreateCopyImage.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgCreateCopyImage.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"
#include "ProcSearch.h"			//マーク探索クラス
#include "AddEdge.h"			//マーク探索処理の追加機能(エッジ演算)
#include "AddBlob.h"			//マーク探索処理の追加機能(2値化重心演算)
#include "ProcCopyImage.h"		//画像コピー処理クラス

extern CProcSearch*		proc_search;	//マーク探索プログラム
extern CAddEdge			search_edge;	//マーク探索処理の追加機能(エッジ演算)
extern CAddBlob			search_blob;	//マーク探索処理の追加機能(2値化重心演算)
extern CProcCopyImage*	proc_cpyimg;	//画像コピー処理クラス

extern CMesParam		mes_param[256];			//機種毎検査パラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CScopes			sys_scope[16];			//システムスコープパラメータ
extern CCameraSettings	cam_parameter[8];		//カメラパラメータ
extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;		//言語設定用

// CDlgCreateCopyImage ダイアログ

IMPLEMENT_DYNAMIC(CDlgCreateCopyImage, CDialogEx)

CDlgCreateCopyImage::CDlgCreateCopyImage(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgCreateCopyImage, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 768;
	image_view_size_y = 576;

	//エリア編集可能フラグ
	edit_ok = FALSE;

	// 基準点検出位置初期化
	base_mark_x = 0;
	base_mark_y = 0;
}

CDlgCreateCopyImage::~CDlgCreateCopyImage()
{
}

void CDlgCreateCopyImage::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_TAB1, tab_setting_param);
	DDX_Control(pDX, IDC_LIST1, trial_log);
	DDX_Control(pDX, IDOK, btn_ok);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
}


BEGIN_MESSAGE_MAP(CDlgCreateCopyImage, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgCreateCopyImage::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgCreateCopyImage::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgCreateCopyImage::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgCreateCopyImage::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgCreateCopyImage::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgCreateCopyImage::OnBnClickedButton6)
	ON_BN_CLICKED(IDOK, &CDlgCreateCopyImage::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgCreateCopyImage::OnBnClickedCancel)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDlgCreateCopyImage::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CDlgCreateCopyImage メッセージ ハンドラー


BOOL CDlgCreateCopyImage::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgCreateCopyImage in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);
	this->GetClientRect(&r_client);

	dlg_x = 200;
	dlg_y = 120;
	dlg_width = (r_window.right - r_window.left) - (r_client.right - r_client.left) + image_view_size_x + 260;
	dlg_height = (r_window.bottom - r_window.top) - (r_client.bottom - r_client.top) + image_view_size_y + 100;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	LoadParameter();	//パラメータの展開

	// ==================================================
	//
	// 5. イベント監視用: インスタンス初期化
	//
	// ==================================================

	// ※ m_messagemap が メッセージを受け取ると関数がコールバックされるように設定します.

	m_messagemap.SetCommandPre(ICommandPre, this);
	m_messagemap.SetCommandPost(ICommandPost, this);
	m_messagemap.SetWndMsgPre(IWndMsgPre, this);
	m_messagemap.SetWndMsgPost(IWndMsgPost, this);

	// ===== ここまで =====

	// 画像ビューのメッセージ監視の準備.
	m_messagemap.AddDialog(&m_EvView);

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
void CDlgCreateCopyImage::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	//ChangeLanguage();

	//タブコントロール準備
	ControlInitialTab();

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	magni_size = 1 / WholeMagnification();	//倍率サイズ

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成

	//コピー範囲表示
	m_Overlay_Area = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 画像コピー範囲
	m_Overlay_Area->SetScaling(true);						//伸縮有無
	m_Rectangle.GetPen().SetColor(RGB(0x00, 0xFF, 0x00));	//範囲 緑色
	m_EvView.SetFocusOption(FVCL::FocusOption::LDOWN);		//ビュー上での左クリック有効

	//コピー範囲の処理範囲編集色指定
	COLORREF default_color = RGB(0x7F, 0x7F, 0xFF);		//初期色
	m_EvView.GetDrawProcArea().SetFrameColor(RGB(0x00, 0xFF, 0x00));	//緑色

	int cross_size = 16 * magni_size;	//十字サイズ
	int font_size = 16 * magni_size;	//文字サイズ
	FVCL::GDI::CFvGdiFontA font_str;
	font_str.SetSize(font_size, 0);		//フォントサイズ 高さ(font_size), 平均文字幅0

	//基準点探索結果
	m_Overlay_Srch = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 探索実行結果
	m_Overlay_Srch->SetScaling(true);						//伸縮有無
	m_Overlay_Srch->SetEnable(false);						//探索結果オーバーレイを隠す
	m_Rect_Srch.GetPen().SetColor(RGB(0x00, 0x00, 0xFF));	//マーク登録範囲 青色
	m_Point_Srch.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));	//検出位置 赤色
	m_Point_Srch.SetStyle(FVCL::GDI::FigureStyle::Cross);	//検出位置 スタイル(十字)
	m_Point_Srch.SetSize(cross_size, cross_size);			//検出位置 サイズ
	m_Score_Srch.SetColor(RGB(0xFF, 0x00, 0x00));			//スコア表示 赤色
	m_Score_Srch.SetBkMode(FVCL::GDI::BkMode::Transparent);	//スコア表示 背景透過
	m_Score_Srch.SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Bottom);	//スコア表示 位置
	m_Score_Srch.SetFont(font_str);

	do_click = FALSE;	//左クリックOFF
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::ChangeLanguage()
{
	if (!language.jpn) {
		//this->SetWindowTextA(language.GetCharacterString(101));
		//btn_mes_trial.SetWindowTextA(language.GetCharacterString(105));		//検査実行ボタン
		//btn_ok.SetWindowTextA(language.GetCharacterString(115));			//設定ボタン
		//btn_full.SetWindowTextA(language.GetCharacterString(107));			//全体表示ボタン
		//btn_1x.SetWindowTextA(language.GetCharacterString(108));			//等倍表示ボタン
		//btn_2x.SetWindowTextA(language.GetCharacterString(109));			//x2ボタン
		//btn_base_image.SetWindowTextA(language.GetCharacterString(112));	//元画像ボタン
		//btn_live.SetWindowTextA(language.GetCharacterString(111));			//ライブ画像ボタン
		//btn_fromfile.SetWindowTextA(language.GetCharacterString(110));		//ファイルからボタン
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
void CDlgCreateCopyImage::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//パラメータ切替タブ
	tab_setting_param.InsertItem(0, Tips0.SelChar(language.jpn, "基本", "Area"));
	tab_setting_param.SetItemSize(CSize(80, 20));		//タブサイズ ※プロパティStyleを固定幅に設定

	CRect r_tab;
	tab_setting_param.GetClientRect(&r_tab);	//タブコントロールのサイズを取得
	tab_setting_param.AdjustRect(FALSE, &r_tab);

	//基本タブダイアログの配置
	dlg_TabMain = new CTabCpyImgParamMain(&tab_setting_param);
	dlg_TabMain->Create(IDD_TabCpyImgParamMain, &tab_setting_param);
	dlg_TabMain->MoveWindow(&r_tab, TRUE);

	tab_setting_param.SetCurSel(0);
	dlg_TabMain->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//画像コピー始点/サイズの初期設定
	//画像1/6サイスで中央に配置
	if (!sheet_param.mes_area.enable) {
		UINT temp_width = m_Image->GetHorzSize() / 6;
		UINT temp_height = m_Image->GetVertSize() / 6;
		sheet_param.mes_area.st_x = (m_Image->GetHorzSize() / 2) - (temp_width / 2);
		sheet_param.mes_area.st_y = (m_Image->GetVertSize() / 2) - (temp_height / 2);
		sheet_param.mes_area.width = temp_width;
		sheet_param.mes_area.height = temp_height;
		sheet_param.mes_area.mark_dis_x = 0;
		sheet_param.mes_area.mark_dis_y = 0;
		sheet_param.mes_area.area_degree = 0;
	}

	//画像コピーエリアの取得
	if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

	//編集可能コントール処理
	ControlEditOK(edit_ok);

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());

	// 画像ビューの表示更新.
	m_EvView.SetImage(m_Image); //表示する画像の設定

	//カーソルタイプを通常モードに指定
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	//基本タブパラメータの展開
	LoadParameterTabMain();
}


/** ****************************************************** **
 ** 基本タブの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::LoadParameterTabMain()
{
	// TODO: ここに実装コードを追加します.

	//展開パラメータ
	int* edit_x = &sheet_param.mes_area.st_x;
	int* edit_y = &sheet_param.mes_area.st_y;

	//基準点探索する場合は基準点距離を設定する
	if (base_search_enable) {
		edit_x = &sheet_param.mes_area.mark_dis_x;
		edit_y = &sheet_param.mes_area.mark_dis_y;
	}
	int* width = &sheet_param.mes_area.width;
	int* height = &sheet_param.mes_area.height;

	//始点とサイズ
	UpdateBaseEdit(*edit_x, *edit_y, *width, *height);

	EnableBaseEdit(FALSE);	//サーチエリア 始点とサイズの入力無効化

	//項目名の更新
	dlg_TabMain->SetSrchEnable(base_search_enable);
	dlg_TabMain->UpdateData(FALSE);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//
	// 基本タブ
	//

	//画像コピー範囲設定
	if (edit_ok) {

		int st_x;	//始点X
		int st_y;	//始点Y
		int size_x;	//エリア幅
		int size_y;	//エリア高さ
		m_Image->GetWindow(&st_x, &st_y, &size_x, &size_y);
		//基準点からの距離
		if (base_search_enable) {
			sheet_param.mes_area.mark_dis_x = st_x - (int)base_mark_x;
			sheet_param.mes_area.mark_dis_y = st_y - (int)base_mark_y;
		}
		//画像内絶対座標
		else {
			sheet_param.mes_area.st_x = st_x;
			sheet_param.mes_area.st_y = st_y;
		}
		sheet_param.mes_area.width = size_x;
		sheet_param.mes_area.height = size_y;
		sheet_param.mes_area.enable = TRUE;	//設定済フラグ
	}
}


/** ****************************************************** **
 ** パラメータ編集コントロール処理
 ** 引数：
 **　BOOL enable	コントロールENABLE
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::ControlEditOK(BOOL enable)
{
	// TODO: ここに実装コードを追加します.

	//メイン画面
	btn_ok.EnableWindow(enable);

	//基本タブ
	dlg_TabMain->btn_area_edit.EnableWindow(enable);
	dlg_TabMain->btn_area_redo.EnableWindow(enable);
}


/** ****************************************************** **
 ** エリア取得メイン
 ** 引数：
 **　なし
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgCreateCopyImage::LoadArea()
{
	// TODO: ここに実装コードを追加します.

	//画像コピーエリアの取得
	//(基準点を使用する)
	//	登録マークを基準点探索に使用する場合、最初に基準点を探索し検出位置を出す
	//　検出位置よりエリアを設定する
	//(基準点を使用しない)
	//　基準点は探索しない
	//　設定されたエリアを使用する
	int area_xs = 0;		//設定するエリア始点X
	int area_ys = 0;		//設定するエリア始点Y
	int area_x_size = 0;	//設定するエリア幅
	int area_y_size = 0;	//設定するエリア高さ
	BOOL window_ok = FALSE;
	int result_load_area = LoadParamArea(&area_xs, &area_ys, &area_x_size, &area_y_size);
	if (result_load_area == 0) {
		window_ok = TRUE;
	}
	//エリアの取得失敗
	else {
		//基準点検出もしくはエリアの指定が失敗したためエリアは編集不可能とする
		//パラメータの更新も不可能
		CString title;
		CString msg;
		this->GetWindowTextA(title);

		//エリアが視野範囲外
		if (result_load_area == -2) {
			msg = Tips0.SelStr(language.jpn,
				"画像コピーエリアがカメラ視野範囲外となっています。\nエリア設定を初期化してもよろしいですか？",
				"The copy area is out of camera field of view\nIs it OK to initialize the area setting?");
			int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
			if (judge == IDOK) {
				window_ok = TRUE;
			}
		}
		//基準点が未検出
		else if (result_load_area == -1) {
			MessageBoxA(Tips0.SelStr(language.jpn,
				"基準点を検出できませんでした。パラメータの設定はできません",
				"The reference point could not be detected. Parameters cannot be set."),
				title, MB_OK | MB_ICONINFORMATION);
		}
		//処理異常
		else {
			MessageBoxA(Tips0.SelStr(language.jpn,
				"基準点探索失敗。パラメータの設定はできません",
				"Reference point search failed. Parameters cannot be set."),
				title, MB_OK | MB_ICONERROR);
		}
	}

	m_Overlay_Area->ClearFigures();

	//画像コピー範囲(処理ウィンドウ)を設定
	if (window_ok) {
		m_Image->SetWindow(area_xs, area_ys, area_x_size, area_y_size);

		//オーバーレイ描画(画像コピー範囲)
		m_Rectangle.st.x = area_xs;
		m_Rectangle.st.y = area_ys;
		m_Rectangle.ed.x = area_xs + area_x_size - 1;
		m_Rectangle.ed.y = area_ys + area_y_size - 1;
		m_Overlay_Area->DrawFigure(m_Rectangle);
	}

	//画像コピー範囲オーバーレイを再表示
	m_Overlay_Area->SetEnable(true);

	return window_ok;
}


/** ****************************************************** **
 ** エリアの取得と判定
 ** 引数：
 **　int* xs		設定するエリア始点X
 **　int* ys		設定するエリア始点Y
 **　int* x_size	設定するエリア幅
 **　int* y_size	設定するエリア高さ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗、-1:基準点探索NG、-2:エリア設定NG
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCreateCopyImage::LoadParamArea(int* xs, int* ys, int* x_size, int* y_size)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//エリアを基準点追従する
	if (base_search_enable) {

		//基準点探索ウィンドウの設定
		int srch_st_x = temp_param->base_area.st_x;
		int srch_st_y = temp_param->base_area.st_y;
		int srch_width = temp_param->base_area.width;
		int srch_height = temp_param->base_area.height;

		//サーチ範囲(処理ウィンドウ)を設定
		if (!m_Image->SetWindow(srch_st_x, srch_st_y, srch_width, srch_height)) goto END;

		rtn_result = BaseSearch(&base_mark_x, &base_mark_y);
		if (rtn_result != 0) goto END;

		//画像コピーエリアがすでに設定済
		if (sheet_param.mes_area.enable) {
			*xs = base_mark_x + sheet_param.mes_area.mark_dis_x;
			*ys = base_mark_y + sheet_param.mes_area.mark_dis_y;
		}
		//エリアが未設定
		else {
			//画像中心に1/6サイズで設定する
			*xs = sheet_param.mes_area.st_x;
			*ys = sheet_param.mes_area.st_y;
			sheet_param.mes_area.mark_dis_x = sheet_param.mes_area.st_x - base_mark_x;
			sheet_param.mes_area.mark_dis_y = sheet_param.mes_area.st_y - base_mark_y;
		}
		*x_size = sheet_param.mes_area.width;	//設定幅
		*y_size = sheet_param.mes_area.height;	//設定高さ
	}
	//エリアを選択範囲で固定
	else {
		rtn_result = 0;
		*xs = sheet_param.mes_area.st_x;		//設定始点X
		*ys = sheet_param.mes_area.st_y;		//設定始点Y
		*x_size = sheet_param.mes_area.width;	//設定幅
		*y_size = sheet_param.mes_area.height;	//設定高さ

		m_Overlay_Srch->SetEnable(false);		//探索結果オーバーレイを隠す
	}

	int image_width = m_Image->GetHorzSize();
	int image_height = m_Image->GetVertSize();

	//エリアが視野範囲からはみ出る場合
	if (*xs < 0 || *xs >= image_width ||
		*ys < 0 || *ys >= image_height ||
		(*xs + *x_size) < 0 || (*xs + *x_size) > image_width ||
		(*ys + *y_size) < 0 || (*ys + *y_size) > image_height) {
		//エリア設定NG
		rtn_result = -2;
		//初期設定を入れる
		UINT temp_width = m_Image->GetHorzSize() / 6;
		UINT temp_height = m_Image->GetVertSize() / 6;
		*xs = (m_Image->GetHorzSize() / 2) - (temp_width / 2);
		*ys = (m_Image->GetVertSize() / 2) - (temp_height / 2);
		*x_size = temp_width;
		*y_size = temp_height;
	}

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 基準点探索
 ** 引数：
 **　double* mark_x		基準点検出位置X
 **　double* mark_y		基準点検出位置Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗、-1:基準点探索NG
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCreateCopyImage::BaseSearch(double* mark_x, double* mark_y)
{
	// TODO: ここに実装コードを追加します.
	int result_srch = 1;
	BOOL srch_ok = FALSE;

	trial_log.ResetContent();		//試行ログリストのクリア

	//マーク探索を実施
	if (temp_param->additional.add_type == 0x00 || temp_param->additional.base_search_enable) {
		if (temp_param->basic.matching == 0x00) result_srch = BaseSearchGS2(mark_x, mark_y, temp_param);
		if (temp_param->basic.matching == 0x01) result_srch = BaseSearchFPM2(mark_x, mark_y, temp_param);
		if (result_srch == 0) {
			srch_ok = TRUE;
		}
		else if (result_srch == -1) {
			trial_log.AddString(Tips0.SelStr(language.jpn, "マーク探索NG", "Mark search NG"));
			goto REDRAW;
		}
		else {
			trial_log.AddString(Tips0.SelStr(language.jpn, "マーク探索失敗", "Mark search fail"));
			goto SEARCH_END;
		}
	}

	//エッジサーチ結果初期化
	for (int i = 0; i < 2; i++) {
		temp_param->add_edge.area_ok[i] = FALSE;		//サーチエリア有効判定
		temp_param->add_edge.edge_ok[i] = FALSE;		//エッジサーチ結果
	}
	//2値化重心サーチ結果初期化
	temp_param->add_blob.area_ok = FALSE;		//サーチエリア有効判定
	temp_param->add_blob.blob_ok = FALSE;		//2値化重心サーチ結果

	//追加機能 エッジサーチを実施
	if (temp_param->additional.add_type == 0x01) {

		temp_param->m_Image = *m_Image;
		int result_edge = search_edge.AddMain(template_number, temp_param, &srch_ok, &trial_log);

		//サーチ成功もしくはサーチエリア設定NGの場合
		if (result_edge != 1) {
			if (result_edge == -2 || !srch_ok) {
				trial_log.AddString(Tips0.SelStr(language.jpn, "エッジサーチNG", "Edge search NG"));
				result_srch = -1;
			}
			else {
				result_srch = 0;	//エッジサーチ成功
			}
		}
		//サーチ失敗の場合
		else {
			trial_log.AddString(Tips0.SelStr(language.jpn, "エッジサーチ失敗", "Edge search fail"));	//エラーコード0x9302
			result_srch = 1;
		}
		if (result_srch != 0 || !srch_ok) goto SEARCH_END;	//処理失敗もしくは探索NGの場合は処理中断

		*mark_x = temp_param->add_edge.rslt_cross.x;
		*mark_y = temp_param->add_edge.rslt_cross.y;
	}
	//追加機能 2値化重心サーチを実施
	else if (template_param->additional.add_type == 0x02) {

		temp_param->m_Image = *m_Image;
		int result_blob = search_blob.AddMain(template_number, temp_param, &srch_ok, &trial_log);

		//サーチ成功もしくはサーチエリア設定NGの場合
		if (result_blob != 1) {
			if (result_blob == -2 || !srch_ok) {
				trial_log.AddString(Tips0.SelStr(language.jpn, "2値化重心サーチNG", "Gravity search NG"));
				result_srch = -1;
			}
			else {
				result_srch = 0;	//2値化重心サーチ成功
			}
		}
		//サーチ失敗の場合
		else {
			trial_log.AddString(Tips0.SelStr(language.jpn, "2値化重心サーチ失敗", "Gravity search fail"));	//エラーコード0x9303
			result_srch = 1;
		}
		if (result_srch != 0 || !srch_ok) goto SEARCH_END;	//処理失敗もしくは探索NGの場合は処理中断

		*mark_x = temp_param->add_blob.rslt_center.x;
		*mark_y = temp_param->add_blob.rslt_center.y;
	}

REDRAW:
	//基準点描画
	ReDrawViewSrch(temp_param, srch_ok);

SEARCH_END:
	return result_srch;
}


/** ****************************************************** **
 ** 基準点探索試行(グレイサーチ)
 ** 引数：
 **　double* mark_x		基準点検出位置X
 **　double* mark_y		基準点検出位置Y
 ** 戻り値：
 **　int 結果　0:探索OK、1:失敗、-1:探索NG
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCreateCopyImage::BaseSearchGS2(double* mark_x, double* mark_y, CTempParam* param)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//探索実行(探索個数1)
	BOOL srch_ok = FALSE;	//探索結果
	rtn_result = proc_search->ExecuteSearchGS2(template_number, param, m_Image, 1, &srch_ok);
	if (rtn_result == 0 && !srch_ok) rtn_result = -1;	//探索NGの場合、-1

	//正常実行
	if (rtn_result != 1) {

		F_GS_RESULT* data_ptr = param->gs2_result.GetAddress();	//結果先頭アドレスの取得

		//探索個数
		UINT count_result = param->gs2_result.GetCount();
		//探索成功
		if (count_result == 1) {

			//検出位置
			FVCL::Data::CFvPoint mark((double)data_ptr[0].x / (double)100, (double)data_ptr[0].y / (double)100);

			//検出位置の回答
			*mark_x = mark.x;
			*mark_y = mark.y;
		}
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 基準点探索試行(形状サーチ)
 ** 引数：
 **　double* mark_x		基準点検出位置X
 **　double* mark_y		基準点検出位置Y
 ** 戻り値：
 **　int 結果　0:探索OK、1:失敗、-1:探索NG
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCreateCopyImage::BaseSearchFPM2(double* mark_x, double* mark_y, CTempParam* param)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//探索実行(探索個数1)
	BOOL srch_ok = FALSE;	//探索結果
	rtn_result = proc_search->ExecuteSearchFPM2(template_number, param, m_Image, 1, &srch_ok);
	if (rtn_result == 0 && !srch_ok) rtn_result = -1;	//探索NGの場合、-1

	//探索成功(特徴量検出成功)
	if (rtn_result == 0 && param->fpm2_rslt_edge) {

		int final_count = 0;	//閾値より上の数

		//探索個数
		UINT count_result = param->fpm2_matching.GetResultSize();
		//探索成功
		if (count_result == 1) {

			FVCL::FPM2::CFvFPM2Data data = param->fpm2_matching[0];

			// マッチング結果の取得.
			FVCL::Data::CFvPoint pos = data.GetPosition();      // 回答位置.

			int score = data.GetScore();                        // スコア (％)

			if (score >= param->basic.judge_score) {
				final_count++;
			}

			//最終結果
			if (final_count > 0) {
				//検出位置の回答
				*mark_x = pos.x;
				*mark_y = pos.y;
			}
		}
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 基準点探索の画像ビュー更新
 ** 引数：
 **　CTempParam* param	探索パラメータ
 **　BOOL srch_ok		探索結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::ReDrawViewSrch(CTempParam* param, BOOL srch_ok)
{
	// TODO: ここに実装コードを追加します.

	//探索結果オーバーレイ描画クリア
	m_Overlay_Srch->ClearFigures();

	if (srch_ok) {
		//基準点サーチ
		if (param->additional.add_type == 0x00 || param->additional.base_search_enable) {
			byte matching = param->basic.matching;	//探索手法 0x00:グレイサーチ、0x01:形状サーチ

			//グレイサーチ結果表示
			if (matching == 0x00) DrawViewGS2(param);
			//形状サーチ結果表示
			if (matching == 0x01) DrawViewFPM2(param);
		}

		//エッジサーチ
		if (param->additional.add_type == 0x01) {
			double cross_x = param->add_edge.rslt_cross.x;
			double cross_y = param->add_edge.rslt_cross.y;

			//十字描画
			m_Point_Srch.x = cross_x;
			m_Point_Srch.y = cross_y;
			m_Point_Srch.SetAngle(0);	//図形傾き

			//エッジ交点表記
			m_Score_Srch.SetText(Tips0.SelChar(language.jpn, "エッジ交点", "Edge intersection"));
			m_Score_Srch.SetPosition(FVCL::Data::CFvPoint(cross_x, cross_y));
			m_Score_Srch.SetAngle(0);	//図形傾き

			//エッジサーチ結果オーバーレイの追加
			m_Overlay_Srch->DrawFigure(m_Point_Srch);
			m_Overlay_Srch->DrawFigure(m_Score_Srch);
		}
		//2値化重心サーチ
		else if (param->additional.add_type == 0x02) {
			double center_x = param->add_blob.rslt_center.x;
			double center_y = param->add_blob.rslt_center.y;

			//十字描画
			m_Point_Srch.x = center_x;
			m_Point_Srch.y = center_y;
			m_Point_Srch.SetAngle(0);	//図形傾き

			//エッジ交点表記
			m_Score_Srch.SetText(Tips0.SelChar(language.jpn, "2値化重心", "Binarized center of gravity"));
			m_Score_Srch.SetPosition(FVCL::Data::CFvPoint(center_x, center_y));
			m_Score_Srch.SetAngle(0);	//図形傾き

			//2値化重心サーチ結果オーバーレイの追加
			m_Overlay_Srch->DrawFigure(m_Point_Srch);
			m_Overlay_Srch->DrawFigure(m_Score_Srch);
		}

		m_Overlay_Srch->SetEnable(true);		//探索結果オーバーレイを表示
	}
	//探索失敗
	else {
		m_Overlay_Srch->SetEnable(false);		//探索結果オーバーレイを隠す
	}

	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** グレイサーチ結果表示
 ** 引数：
 **　CTempParam* param	テンプレートパラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::DrawViewGS2(CTempParam* param)
{
	// TODO: ここに実装コードを追加します.

	//結果の取得
	FVCL::Data::CFvPoint pat_mark = param->fv_pattern.GetCenterMark();		//パタン登録時の基準点
	FVCL::Data::CFvRectangle pat_rect = param->fv_pattern.GetRegistRect();	//パタン登録時の矩形

	F_GS_RESULT* data_ptr = param->gs2_result.GetAddress();	//結果先頭アドレスの取得

	//探索個数
	UINT count_result = param->gs2_result.GetCount();

	//探索成功
	if (count_result == 1) {

		//検出位置
		FVCL::Data::CFvPoint mark((double)data_ptr[0].x / (double)100, (double)data_ptr[0].y / (double)100);

		//検出位置とパタン情報から矩形を復元
		FVCL::Data::CFvRectangle rect;
		rect.st.x = mark.x - pat_mark.x;
		rect.st.y = mark.y - pat_mark.y;
		rect.ed.x = rect.st.x + pat_rect.GetWidth() - 1;
		rect.ed.y = rect.st.y + pat_rect.GetHeight() - 1;

		//相関値
		FVCL::Data::CFvString str_score;
		str_score.Format(_T("%5.2f"), (double)data_ptr[0].score / (double)100);

		//矩形枠描画
		m_Rect_Srch.st.x = rect.st.x;
		m_Rect_Srch.st.y = rect.st.y;
		m_Rect_Srch.ed.x = rect.ed.x;
		m_Rect_Srch.ed.y = rect.ed.y;

		//十字描画
		m_Point_Srch.x = mark.x;
		m_Point_Srch.y = mark.y;

		//スコア描画
		m_Score_Srch.SetText(LPCSTR(str_score));
		m_Score_Srch.SetPosition(mark);

		//探索結果オーバーレイの追加
		m_Overlay_Srch->DrawFigure(m_Rect_Srch);
		m_Overlay_Srch->DrawFigure(m_Point_Srch);
		m_Overlay_Srch->DrawFigure(m_Score_Srch);
	}
}


/** ****************************************************** **
 ** 形状サーチ結果表示
 ** 引数：
 **　CTempParam* param	テンプレートパラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::DrawViewFPM2(CTempParam* param)
{
	// TODO: ここに実装コードを追加します.

	//特徴量検出成功
	if (param->fpm2_rslt_edge) {

		//結果の取得
		FVCL::Data::CFvPoint offset = param->fpm2_templ.GetOffset();        // パタン登録時の基準点.
		double width = param->fpm2_templ.GetWidth();
		double height = param->fpm2_templ.GetHeight();

		int final_count = 0;	//閾値より上の数

		//探索個数
		UINT count_result = param->fpm2_matching.GetResultSize();

		//探索成功
		if (count_result == 1) {

			FVCL::FPM2::CFvFPM2Data data = param->fpm2_matching[0];

			// マッチング結果の取得.
			FVCL::Data::CFvPoint pos = data.GetPosition();      // 回答位置.
			FVCL::Data::CFvAngle angle = data.GetAngle();       // 回転角

			double scale = data.GetScale();                     // スケール (％)
			int score = data.GetScore();                        // スコア (％)

			if (score >= param->basic.judge_score) {

				FVCL::Data::CFvRectangle rect;  // 矩形.

				FVCL::Data::CFvPoint st, ed;
				st.x = pos.x - offset.x;
				st.y = pos.y - offset.y;
				ed.x = st.x + width - 1;
				ed.y = st.y + height - 1;
				rect.st = st;
				rect.ed = ed;
				rect.center = offset;
				rect.angle = angle;

				//矩形枠描画
				m_Rect_Srch.st.x = rect.st.x;
				m_Rect_Srch.st.y = rect.st.y;
				m_Rect_Srch.ed.x = rect.ed.x;
				m_Rect_Srch.ed.y = rect.ed.y;
				m_Rect_Srch.SetAxis(rect.center);		//回転の機軸座標
				m_Rect_Srch.SetAngle(rect.angle.GetDegree());		//図形傾き

				//十字描画
				m_Point_Srch.x = pos.x;
				m_Point_Srch.y = pos.y;
				m_Point_Srch.SetAngle(rect.angle.GetDegree());	//図形傾き

				//相関値
				FVCL::Data::CFvString str_score;
				str_score.Format(_T("%d"), score);

				//スコア描画
				m_Score_Srch.SetText(LPCSTR(str_score));
				m_Score_Srch.SetPosition(pos);
				m_Score_Srch.SetAngle(rect.angle.GetDegree());	//図形傾き

				//探索結果オーバーレイの追加
				m_Overlay_Srch->DrawFigure(m_Rect_Srch);
				m_Overlay_Srch->DrawFigure(m_Point_Srch);
				m_Overlay_Srch->DrawFigure(m_Score_Srch);

				final_count++;
			}
		}
	}
}


/** ****************************************************** **
 ** 作成テンプレート番号の入力/シート番号
 ** 引数：
 **　int template_num	テンプレート番号
 **　int sheet_num		シート番号(0～7)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::SetNumber(int template_num, int sheet_num)
{
	// TODO: ここに実装コードを追加します.
	template_number = template_num;
	sheet_number = sheet_num;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 **　CMesParam* edit_param	編集中検査パラメータのポインタ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::SetParam(LPVOID pParam, CMesParam* edit_param)
{
	pParam_main = pParam;

	//検査パラメータを編集用パラメータへコピー
	edit_mes = edit_param;
	sheet_param = edit_mes->sheet[sheet_number];
	base_search_enable = edit_mes->base_search_enable;

	//テンプレートパラメータ
	temp_param = &template_param[template_number];
}


/** ****************************************************** **
 ** サーチ設定基本画像入力
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::SetImage(FVCL::Data::CFvImage* image)
{
	// TODO: ここに実装コードを追加します.
	m_Image_base = image;
	m_Image = m_Image_base;
}


/** ****************************************************** **
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgCreateCopyImage::WholeMagnification()
{
	// TODO: ここに実装コードを追加します.
	int image_width = m_Image->GetHorzSize();
	int image_height = m_Image->GetVertSize();
	double mag_x = (double)image_view_size_x / (double)image_width;
	double mag_y = (double)image_view_size_y / (double)image_height;
	double mag_min = min(mag_x, mag_y);
	//if (mag_min > 1) mag_min = 1;		※1倍以上もOKとする

	return mag_min;
}


/** ****************************************************** **
 ** 始点とサイズの更新
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::UpdateBaseArea()
{
	// TODO: ここに実装コードを追加します.
	if (m_EvView.GetCursorType() == FVCL::CursorType::PROCAREA) {
		int st_x;	//始点X
		int st_y;	//始点Y
		int size_x;	//エリア幅
		int size_y;	//エリア高さ
		m_Image->GetWindow(&st_x, &st_y, &size_x, &size_y);

		int edit_x = st_x;
		int edit_y = st_y;

		//基準点探索する場合は基準点距離を設定する
		if (base_search_enable) {
			edit_x = st_x - (int)base_mark_x;
			edit_y = st_y - (int)base_mark_y;
		}

		UpdateBaseEdit(st_x, st_y, size_x, size_y);	//入力の更新
	}
}


/** ****************************************************** **
 ** 始点とサイズ入力の更新
 ** 引数：
 **　int st_x, int st_y, int width, int height
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::UpdateBaseEdit(int st_x, int st_y, int width, int height)
{
	// TODO: ここに実装コードを追加します.

	//基本タブの入力コントロールを更新
	dlg_TabMain->edit_set_x.SetWindowTextA(Tips0.IntToString(st_x));
	dlg_TabMain->edit_set_y.SetWindowTextA(Tips0.IntToString(st_y));
	dlg_TabMain->edit_set_width.SetWindowTextA(Tips0.IntToString(width));
	dlg_TabMain->edit_set_height.SetWindowTextA(Tips0.IntToString(height));
}


/** ****************************************************** **
 ** 画像コピーエリアの有効無効化
 ** 引数：
 **　BOOL enable	入力の有効無効化
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::EnableBaseEdit(BOOL enable)
{
	// TODO: ここに実装コードを追加します.

	//基本タブの入力コントロールを更新
	dlg_TabMain->edit_set_x.EnableWindow(enable);
	dlg_TabMain->edit_set_y.EnableWindow(enable);
	dlg_TabMain->edit_set_width.EnableWindow(enable);
	dlg_TabMain->edit_set_height.EnableWindow(enable);
}


/** ****************************************************** **
 ** 始点とサイズ 編集操作
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::EditArea()
{
	// TODO: ここに実装コードを追加します.

	//サーチ範囲オーバーレイを隠す
	m_Overlay_Area->SetEnable(false);

	//処理ウィンドウのカーソルタイプを処理範囲編集モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::PROCAREA);

	m_EvView.ReDraw(true);			//表示更新
}


/** ****************************************************** **
 ** 始点とサイズ やり直し操作
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::RedoArea()
{
	// TODO: ここに実装コードを追加します.

	//処理ウィンドウのカーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	//エリアの取得
	edit_ok = FALSE;
	if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

	//編集可能コントール処理
	ControlEditOK(edit_ok);

	m_EvView.ReDraw(true); //表示更新

	//展開パラメータ
	int* edit_x = &sheet_param.mes_area.st_x;
	int* edit_y = &sheet_param.mes_area.st_y;

	//基準点探索する場合は基準点距離を設定する
	if (base_search_enable) {
		edit_x = &sheet_param.mes_area.mark_dis_x;
		edit_y = &sheet_param.mes_area.mark_dis_y;
	}
	int* width = &sheet_param.mes_area.width;
	int* height = &sheet_param.mes_area.height;

	UpdateBaseEdit(*edit_x, *edit_y, *width, *height);	//入力の更新

	EnableBaseEdit(FALSE);	//サーチエリア 始点とサイズの入力無効化

	do_click = FALSE;
}


/** ****************************************************** **
 ** エリア再取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::ReLoadArea()
{
	// TODO: ここに実装コードを追加します.

	ChangeParameter();	//パラメータ読み出し

	//処理ウィンドウのカーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// 画像ビューの表示更新.
	m_EvView.SetImage(m_Image); //表示する画像の設定
	m_EvView.ReDraw(false);		//表示更新

	//エリアの取得
	edit_ok = FALSE;
	if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

	//編集可能コントール処理
	ControlEditOK(edit_ok);

	m_EvView.ReDraw(false);		//表示更新
}


/** ****************************************************** **
 ** BMPファイルの選択
 ** 引数：
 **　LPTSTR lpSelectPath
 ** 戻り値：
 **　BOOL：結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgCreateCopyImage::SelectBMPFile(LPTSTR lpSelectPath)
{
	BOOL rtn_result = FALSE;

	OPENFILENAME open_file;
	ZeroMemory(&open_file, sizeof(open_file));
	open_file.lStructSize = sizeof(OPENFILENAME);
	if (language.jpn) {
		open_file.lpstrFilter = TEXT("BMPファイル(*.bmp)\0*.bmp\0\0");
	}
	else {
		open_file.lpstrFilter = TEXT("BMP file(*.bmp)\0*.bmp\0\0");
	}
	open_file.lpstrFile = lpSelectPath;
	open_file.nMaxFile = MAX_PATH;
	open_file.Flags = OFN_FILEMUSTEXIST;
	open_file.hwndOwner = this->m_hWnd;

	rtn_result = GetOpenFileNameA(&open_file);

	return rtn_result;
}


/** ****************************************************** **
 ** 画像ファイルの読み込み
 ** 引数：
 **　LPCTSTR filepath
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgCreateCopyImage::LoadFvImage(LPCTSTR filepath)
{
	// TODO: ここに実装コードを追加します.
	CString title, msg;
	this->GetWindowTextA(title);

	bool rtn = m_file_Image.Load(filepath);	//画像の読込

	if (!rtn) {
		int err_code = m_file_Image.GetErrorCode();

		msg = Tips0.IntToString(err_code);
		MessageBoxA(Tips0.SelStr(language.jpn, "ファイル読み込みに失敗しました(", "Failed to read the file(") + msg + ")",
			title, MB_OK | MB_ICONERROR);
		return 1;
	}

	//読み込んだ画像のサイズ
	int buf_width = m_file_Image.GetHorzSize();
	int buf_height = m_file_Image.GetVertSize();

	//対象のカメラ
	int scope = sys_template[template_number].scope;
	int view = sys_template[template_number].view;
	int target_cam = sys_scope[scope].view[view].assign_camera;

	//画像サイズの確認
	if (cam_parameter[target_cam].device.grab_width != buf_width ||
		cam_parameter[target_cam].device.grab_height != buf_height) {
		MessageBoxA(Tips0.SelStr(language.jpn,
			"ファイル読み込みに失敗しました(画像サイズ不一致)",
			"Failed to read the file(Image size mismatch)"),
			title, MB_OK | MB_ICONERROR);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 全体表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(WholeMagnification());	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ×2ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(2.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 元画像ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ChangeParameter();	//パラメータ読み出し

	m_Image = m_Image_base;

	//処理ウィンドウのカーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// 画像ビューの表示更新.
	m_EvView.SetImage(m_Image); //表示する画像の設定
	m_EvView.ReDraw(false);		//表示更新

	//エリアの取得
	edit_ok = FALSE;
	if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

	//編集可能コントール処理
	ControlEditOK(edit_ok);

	m_EvView.ReDraw(false);		//表示更新

	//基本タブに切り替え
	tab_setting_param.SetCurSel(0);
	dlg_TabMain->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** ライブ画像ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ChangeParameter();	//パラメータ読み出し

	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//ライブ画像取得メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_live_Image);
	if (rtn != 0x1001) {
		outlog.LogOutFile(1, "コマンドメッセージ:CDlgCreateCopyImageのライブ画像取得失敗");
		return;
	}

	m_Image = &m_live_Image;

	//処理ウィンドウのカーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// 画像ビューの表示更新.
	m_EvView.SetImage(m_Image); //表示する画像の設定
	m_EvView.ReDraw(false);		//表示更新

	//エリアの取得
	edit_ok = FALSE;
	if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

	//編集可能コントール処理
	ControlEditOK(edit_ok);

	m_EvView.ReDraw(false);		//表示更新

	//基本タブに切り替え
	tab_setting_param.SetCurSel(0);
	dlg_TabMain->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** ファイルからボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedButton6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ChangeParameter();	//パラメータ読み出し

	char path[MAX_PATH] = { '\0' };

	BOOL rtn_result = SelectBMPFile(path);	//BMPファイルの選択

	if (rtn_result) {
		//画像ファイルの読み込み
		if (LoadFvImage(path) == 0) {
			m_Image = &m_file_Image;

			//処理ウィンドウのカーソルタイプを通常モードに変更
			m_EvView.SetCursorType(FVCL::CursorType::CROSS);

			// 画像ビューの表示更新.
			m_EvView.SetImage(m_Image); //表示する画像の設定
			m_EvView.ReDraw(false);		//表示更新

			//エリアの取得
			edit_ok = FALSE;
			if (LoadArea()) edit_ok = TRUE;	//パラメータ編集可能

			//編集可能コントール処理
			ControlEditOK(edit_ok);

			m_EvView.ReDraw(false);		//表示更新
		}
	}

	//基本タブに切り替え
	tab_setting_param.SetCurSel(0);
	dlg_TabMain->ShowWindow(SW_SHOW);
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);

	msg = Tips0.SelStr(language.jpn, "画像コピー設定を更新します。よろしいですか？", "Update copy image setting. Is it OK?");
	int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {

		ChangeParameter();	//パラメータの変更

		//シートパラメータを編集中検査パラメータへコピー
		edit_mes->sheet[sheet_number] = sheet_param;

		//設定した後に現在の設定で画像をコピーし他テンプレートで画像合成ができるようにする
		{
			//コピー先サイズリセット
			proc_cpyimg->SizeReset(temp_param, m_Image, sheet_number);

			//内部メモリへコピー
			BOOL mes_ok;
			proc_cpyimg->CopyToMem(temp_param, m_Image, edit_mes, sheet_number, &mes_ok, &trial_log);
		}

		outlog.LogOutFile(1, "*** DlgCreateCopyImage out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgCreateCopyImage out (CANCEL) ***");

	CDialogEx::OnCancel();
}


// ==================================================
//
// 5. イベント監視用: 関数定義
//
// ==================================================

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgCreateCopyImage::ICommandPre(void* pvParam, CEvDialog* dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgCreateCopyImage*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgCreateCopyImage::ICommandPost(void* pvParam, CEvDialog* dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgCreateCopyImage*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgCreateCopyImage::IWndMsgPre(void* pvParam, CEvDialog* dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgCreateCopyImage*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgCreateCopyImage::IWndMsgPost(void* pvParam, CEvDialog* dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgCreateCopyImage*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgCreateCopyImage::OnCommandPre(CEvDialog* dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgCreateCopyImage::OnCommandPost(CEvDialog* dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	ウィンドウメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	message	メッセージID
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgCreateCopyImage::OnWndMsgPre(CEvDialog* dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);
		switch (message) {
		case WM_MOUSEMOVE:
			break;
		case WM_LBUTTONDOWN:
			break;
		}
	}
}

// ========================================================================
/*!
	@brief	ウィンドウメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	message	メッセージID
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgCreateCopyImage::OnWndMsgPost(CEvDialog* dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);
		switch (message)
		{
		case WM_PAINT:			// 再描画イベント.
			break;
		case WM_MOUSEMOVE:
			if (do_click) UpdateBaseArea();
			break;
		case WM_LBUTTONDOWN:
			do_click = TRUE;
			break;
		case WM_LBUTTONUP:
			do_click = FALSE;
			break;
		}
	}
}


/** ****************************************************** **
 ** マウス左ボタン解放
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	//画像ビュー以外でのボタン操作でも処理する
	do_click = FALSE;

	CDialogEx::OnLButtonUp(nFlags, point);
}


/** ****************************************************** **
 ** パラメータ切替タブSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;
}


/** ****************************************************** **
 ** イベントメッセージ処理
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgCreateCopyImage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	//基本タブ(CTabCpyImgParamMain)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//基本タブの編集ボタンClickedを受信
	case IDC_BUTTON1_TabCpyImgParamMain:
		if (HIWORD(wParam) == BN_CLICKED) {
			EnableBaseEdit(TRUE);	//サーチエリア 始点とサイズの入力有効化
			EditArea();
		}
		break;
		//基本タブのやり直しボタンClickedを受信
	case IDC_BUTTON2_TabCpyImgParamMain:
		if (HIWORD(wParam) == BN_CLICKED) {
			RedoArea();
		}
		break;
		//基本タブの基準点探索チェックボックスClickedを受信
	case IDC_CHECK1_TabCpyImgParamMain:
		if (HIWORD(wParam) == BN_CLICKED) {
			ReLoadArea();	//エリア再取得
		}
		break;
	}
	return CDialogEx::OnCommand(wParam, lParam);
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateCopyImage::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	dlg_TabMain->DestroyWindow();
	delete dlg_TabMain;
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgCreateCopyImage::PreTranslateMessage(MSG* pMsg)
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
