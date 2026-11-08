/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgToolLayer.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：描画レイヤ作成のメインダイアログ
 **		カメラ毎に個別レイヤを作成し、図形や文字を表示する
 **/

// DlgToolLayer.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgToolLayer.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"

#define FOCUS_LEFT 0x0001
#define FOCUS_TOP 0x0002
#define FOCUS_RIGHT 0x0004
#define FOCUS_BOTTOM 0x0008
#define FOCUS_CENTER 0x000F

extern CCameraSettings	cam_parameter[8];	//カメラパラメータ
extern CLayerParam		layer_param[8];		//機種毎個別レイヤパラメータ(カメラ1～8)
extern CLayer			sys_layer[8];		//システムレイヤパラメータ
extern CAccessParamLyer	access_lyr;			//機種毎個別レイヤパラメータファイルアクセス
extern CAccessParamSys	access_sys;			//システムIniファイルアクセス

extern CCurrentStatus	current;			//共有情報
extern COutputLog		outlog;				//ログ出力
extern CLanguage		language;			//言語設定用

// CDlgToolLayer ダイアログ

IMPLEMENT_DYNAMIC(CDlgToolLayer, CDialogEx)

CDlgToolLayer::CDlgToolLayer(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgToolLayer, pParent)
{
	image_view_size_x = 768;
	image_view_size_y = 576;

	pParam_main = NULL;

	//変更前機能の初期化
	for (int i = 0; i < 3; i++) {
		befour_item[i] = 0;
	}

	befour_edit_layer = -1;	//編集していたレイヤの初期化
	befour_edit_cam = -1;	//編集していたカメラの初期化
}

CDlgToolLayer::~CDlgToolLayer()
{
}

void CDlgToolLayer::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_COMBO1, combo_select_cam);
	DDX_Control(pDX, IDC_TAB1, tab_setting_param);
	DDX_Control(pDX, IDC_STATIC_LAYER_CAM, static_select_cam);
	DDX_Control(pDX, IDC_BUTTON5, btn_read_data);
	DDX_Control(pDX, IDC_BUTTON1, btn_full);
	DDX_Control(pDX, IDC_BUTTON2, btn_x1);
	DDX_Control(pDX, IDC_BUTTON3, btn_x2);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
	DDX_Control(pDX, IDOK, btn_set);
	DDX_Control(pDX, IDC_STATIC_NOT_USED, static_not_used);
	DDX_Control(pDX, IDC_BUTTON4, btn_not_used);
}


BEGIN_MESSAGE_MAP(CDlgToolLayer, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgToolLayer::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgToolLayer::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgToolLayer::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgToolLayer::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgToolLayer::OnBnClickedButton3)
	ON_BN_CLICKED(IDOK, &CDlgToolLayer::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgToolLayer::OnBnClickedCancel)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgToolLayer::OnCbnSelchangeCombo1)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CDlgToolLayer::OnTcnSelchangeTab1)
	ON_WM_DESTROY()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CDlgToolLayer メッセージ ハンドラー


BOOL CDlgToolLayer::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgToolLayer in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);
	this->GetClientRect(&r_client);

	dlg_x = 200;
	dlg_y = 120;
	dlg_width = (r_window.right - r_window.left) - (r_client.right - r_client.left) + image_view_size_x + 260;
	dlg_height = (r_window.bottom - r_window.top) - (r_client.bottom - r_client.top) + image_view_size_y + 100;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	//個別レイヤ編集用パラメータへコピー
	edit_param[0] = layer_param[0];	//カメラ1
	edit_param[1] = layer_param[1];	//カメラ2
	edit_param[2] = layer_param[2];	//カメラ3
	edit_param[3] = layer_param[3];	//カメラ4
	edit_param[4] = layer_param[4];	//カメラ5
	edit_param[5] = layer_param[5];	//カメラ6
	edit_param[6] = layer_param[6];	//カメラ7
	edit_param[7] = layer_param[7];	//カメラ8

	//システムレイヤ編集用パラメータへコピー
	edit_sys[0] = sys_layer[0];		//カメラ1
	edit_sys[1] = sys_layer[1];		//カメラ2
	edit_sys[2] = sys_layer[2];		//カメラ3
	edit_sys[3] = sys_layer[3];		//カメラ4
	edit_sys[4] = sys_layer[4];		//カメラ5
	edit_sys[5] = sys_layer[5];		//カメラ6
	edit_sys[6] = sys_layer[6];		//カメラ7
	edit_sys[7] = sys_layer[7];		//カメラ8

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
void CDlgToolLayer::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//タブコントロール準備
	ControlInitialTab();

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成

	//システムレイヤ表示
	ovlay_sys.m_Overlay = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 システムレイヤ
	ovlay_sys.m_Overlay->SetScaling(false);						//伸縮有無
	FVCL::GDI::CFvGdiFontA font_cord;
	font_cord.SetSize(16, 0);		//フォントサイズ 高さ16, 平均文字幅0
	for (int i = 0; i < 2; i++) {
		ovlay_sys.m_Achr_Cord[i].SetStyle(FVCL::GDI::FigureStyle::Triangle);	//座標方向矢印 スタイル(矢印)
		ovlay_sys.m_Achr_Cord[i].SetSize(9, 9);									//座標方向矢印 サイズ
		ovlay_sys.m_Str_Cord[i].SetBkMode(FVCL::GDI::BkMode::Transparent);		//座標文字 背景透過
		ovlay_sys.m_Str_Cord[1].SetFont(font_cord);								//座標文字 フォント指定
	}
	FVCL::GDI::CFvGdiFontA font_cam;
	font_cam.SetSize(32, 0);		//フォントサイズ 高さ32, 平均文字幅0
	ovlay_sys.m_Str_Cam.SetBkMode(FVCL::GDI::BkMode::Transparent);		//名称文字 背景透過
	ovlay_sys.m_Str_Cam.SetFont(font_cam);								//名称文字 フォント指定

	//個別レイヤ表示
	for (int i = 0; i < 3; i++) {
		ovlay_wrk[i].m_Overlay = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 個別レイヤ
		ovlay_wrk[i].m_Overlay->SetScaling(true);						//伸縮有無
	}
	for (int i = 0; i < 3; i++) {
		ovlay_wrk[i].m_Rect_Layer.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//矩形描画 黄色
	}
	m_EvView.SetFocusOption(FVCL::FocusOption::LDOWN);		//ビュー上での左クリック有効

	m_focus_pos = 0;	//フォーカス位置情報
	do_click = FALSE;	//左クリックOFF

	//カメラ選択 コンボボックス
	for (int i = 1; i <= 8; i++) {
		combo_select_cam.AddString(Tips0.SelStr(language.jpn, "カメラ", "Cam") + Tips0.IntToString(i));
	}
	//combo_select_cam.SetCurSel(0);	//カメラ1指定	※最初は選択しない

	tab_setting_param.EnableWindow(FALSE);	//タブ無効化
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::ChangeLanguage()
{
	POINT pt;
	RECT re;
	int width, height;

	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1401));
		static_select_cam.SetWindowTextA(language.GetCharacterString(1402));	//カメラ選択
		btn_read_data.SetWindowTextA(language.GetCharacterString(1403));		//データ読込ボタン
		btn_full.SetWindowTextA(language.GetCharacterString(1416));				//全体表示ボタン
		btn_x1.SetWindowTextA(language.GetCharacterString(1418));				//等倍表示ボタン
		btn_x2.SetWindowTextA(language.GetCharacterString(1417));				//x2ボタン
		btn_cancel.SetWindowTextA(language.GetCharacterString(1419));			//キャンセルボタン
		btn_set.SetWindowTextA(language.GetCharacterString(1420));				//設定ボタン
		static_not_used.SetWindowTextA("Not used");								//スタティック
		btn_not_used.SetWindowTextA("Not used");								//全体表示ボタン

		//コントロール位置サイズ調整
		static_select_cam.GetWindowRect(&re);									//カメラ選択
		pt.x = re.left;
		pt.y = re.top;
		width = re.right - re.left;
		height = re.bottom - re.top;
		this->ScreenToClient(&pt);
		static_select_cam.MoveWindow(pt.x + 15, pt.y, width - 15, height, 1);	//カメラ選択
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
void CDlgToolLayer::ControlInitialTab()
{
	// TODO: ここに実装コードを追加します.

	//パラメータ切替タブ
	tab_setting_param.InsertItem(0, Tips0.SelChar(language.jpn, "描画選択", "Select"));
	tab_setting_param.InsertItem(1, Tips0.SelChar(language.jpn, "システムレイヤ", "System"));
	tab_setting_param.InsertItem(2, Tips0.SelChar(language.jpn, "個別レイヤ1", "Layer1"));
	tab_setting_param.InsertItem(3, Tips0.SelChar(language.jpn, "個別レイヤ2", "Layer2"));
	tab_setting_param.InsertItem(4, Tips0.SelChar(language.jpn, "個別レイヤ3", "Layer3"));

	tab_setting_param.SetItemSize(CSize(75, 20));		//タブサイズ ※プロパティStyleを固定幅に設定

	CRect r_tab;
	tab_setting_param.GetClientRect(&r_tab);	//タブコントロールのサイズを取得
	tab_setting_param.AdjustRect(FALSE, &r_tab);

	//基本タブダイアログの配置
	dlg_TabMain = new CTabLayerParamMain(&tab_setting_param);
	dlg_TabMain->Create(IDD_TabLayerParamMain, &tab_setting_param);
	dlg_TabMain->MoveWindow(&r_tab, TRUE);

	//システムレイヤタブダイアログの配置
	dlg_TabSys1 = new CTabLayerParamSys1(&tab_setting_param);
	dlg_TabSys1->Create(IDD_TabLayerParamSys1, &tab_setting_param);
	dlg_TabSys1->MoveWindow(&r_tab, TRUE);

	//矩形編集タブダイアログの配置
	dlg_TabRect = new CTabLayerParamRect(&tab_setting_param);
	dlg_TabRect->Create(IDD_TabLayerParamRect, &tab_setting_param);
	dlg_TabRect->MoveWindow(&r_tab, TRUE);

	tab_setting_param.SetCurSel(0);
	//dlg_TabMain->ShowWindow(SW_SHOW);		最初は何も表示しない
}


/** ****************************************************** **
 ** カメラ画像の読み込み
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::GetImage()
{
	int select_cam = combo_select_cam.GetCurSel();

	//選択カメラが存在する場合、画像を要求
	if (cam_parameter[select_cam].device.model_code != 0) {

		CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

		//ライブ画像取得メッセージ処理
		LRESULT rtn2 = dlg->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image);
		if (rtn2 != 0x1001) {
			outlog.LogOutFile(1, "コマンドメッセージ:CDlgToolLayerのライブ画像取得失敗");
			return;
		}

		//全体表示倍率の設定
		m_EvView.SetMagnification(WholeMagnification());

		// 画像ビューの表示更新.
		m_EvView.SetImage(&m_Image); //表示する画像の設定

		//カーソルタイプを通常モードに指定
		m_EvView.SetCursorType(FVCL::CursorType::CROSS);
	}
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　int cam_num	カメラ番号(0～3)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//基本タブの展開
	LoadParameterTabMain();

	//システムレイヤタブの展開
	LoadParameterTabSys1();

	//パラメータ切替タブの初期化
	tab_setting_param.SetCurSel(0);
	dlg_TabMain->ShowWindow(SW_SHOW);
	dlg_TabSys1->ShowWindow(SW_HIDE);
	dlg_TabRect->ShowWindow(SW_HIDE);
	tab_setting_param.EnableWindow(TRUE);	//タブ有効化

	int select_cam = combo_select_cam.GetCurSel();

	//オーバーレイ描画(システムレイヤ)
	DrawSysLayer();

	//オーバーレイ描画(個別レイヤ)
	for (int i = 0; i < 3; i++) {
		ovlay_wrk[i].m_Overlay->ClearFigures();

		//展開パラメータの選択
		int select_item = edit_param[select_cam].layer[i].select_item;
		DrawWorkLayer(&edit_param[select_cam].layer[i], i, select_item);
	}

	m_EvView.ReDraw(true);
}


/** ****************************************************** **
 ** 基本タブの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::LoadParameterTabMain()
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();

	//個別レイヤの選択
	for (int i = 0; i < 3; i++) {
		dlg_TabMain->combo_select_layer[i].SetCurSel(edit_param[select_cam].layer[i].select_item);
	}
}


/** ****************************************************** **
 ** システムレイヤタブの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::LoadParameterTabSys1()
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();

	//カメラ座標表示
	BOOL cord_enable = edit_sys[select_cam].cord_sign.enable;
	dlg_TabSys1->check_cord_sign.SetCheck(cord_enable);
	dlg_TabSys1->combo_cord_sign.SetCurSel(edit_sys[select_cam].cord_sign.position);
	dlg_TabSys1->combo_cord_sign.EnableWindow(cord_enable);
	dlg_TabSys1->color_cord = edit_sys[select_cam].cord_sign.color;

	//カメラ名称表示
	BOOL cam_enable = edit_sys[select_cam].cam_sign.enable;
	dlg_TabSys1->check_cam_sign.SetCheck(cam_enable);
	dlg_TabSys1->combo_cam_sign.SetCurSel(edit_sys[select_cam].cam_sign.position);
	dlg_TabSys1->combo_cam_sign.EnableWindow(cam_enable);
	dlg_TabSys1->color_cam = edit_sys[select_cam].cam_sign.color;
}


/** ****************************************************** **
 ** 矩形編集タブの展開
 ** 引数：
 **　int layer_num	レイヤ番号(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::LoadParameterTabRect(int layer_num)
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();

	//展開パラメータ
	int edit_x = edit_param[select_cam].layer[layer_num].rect.st_x;
	int edit_y = edit_param[select_cam].layer[layer_num].rect.st_y;
	int width = edit_param[select_cam].layer[layer_num].rect.width;
	int height = edit_param[select_cam].layer[layer_num].rect.height;
	int degree = edit_param[select_cam].layer[layer_num].rect.degree;

	//始点とサイズと回転
	UpdateRectEditXY(edit_x, edit_y, width, height);
	UpdateRectEditDeg(degree);

	EnableEdit(FALSE, 3);	//図形の入力無効化
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		編集したレイヤパラメータを全て保存する
 ** ****************************************************** **/
void CDlgToolLayer::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	int select_cam = combo_select_cam.GetCurSel();

	//システムレイヤパラメータ・個別レイヤ選択の保管
	if (befour_edit_cam != -1) KeepParamComm(befour_edit_cam);

	//編集していたレイヤの保管
	if (befour_edit_layer != -1) {
		//編集レイヤの保管
		KeepParamWorkLayer(select_cam, befour_edit_layer);
	}

	//個別レイヤ編集用パラメータを保存
	layer_param[0] = edit_param[0];	//カメラ1
	layer_param[1] = edit_param[1];	//カメラ2
	layer_param[2] = edit_param[2];	//カメラ3
	layer_param[3] = edit_param[3];	//カメラ4
	layer_param[4] = edit_param[4];	//カメラ5
	layer_param[5] = edit_param[5];	//カメラ6
	layer_param[6] = edit_param[6];	//カメラ7
	layer_param[7] = edit_param[7];	//カメラ8

	//システムレイヤ編集用パラメータを保存
	sys_layer[0] = edit_sys[0];		//カメラ1
	sys_layer[1] = edit_sys[1];		//カメラ2
	sys_layer[2] = edit_sys[2];		//カメラ3
	sys_layer[3] = edit_sys[3];		//カメラ4
	sys_layer[4] = edit_sys[4];		//カメラ5
	sys_layer[5] = edit_sys[5];		//カメラ6
	sys_layer[6] = edit_sys[6];		//カメラ7
	sys_layer[7] = edit_sys[7];		//カメラ8

	//機種レイヤパラメータ WkLayer.iniファイル書き込み
	for (int i = 0; i < 8; i++) {
		if (access_lyr.CheckIniFileParam(current.work_number, i) == 0) {
			//iniファイル存在
			access_lyr.WriteIniFileParam(current.work_number, i);	//iniファイル更新
		}
		else {
			//iniファイルの作成
			if (access_lyr.CreateIniFileParam(current.work_number, i) != 0) {
				CString title;
				this->GetWindowTextA(title);
				MessageBoxA(Tips0.SelStr(language.jpn,
					"機種レイヤパラメータファイルの作成に失敗しました",
					"Failed to create the model layer parameter file"),
					title, MB_OK | MB_ICONERROR);
			}
		}
	}

	//システムレイヤパラメータのIniファイル保存
	access_sys.WriteIniFileLayer();
}


/** ****************************************************** **
 ** パラメータの保管(システム・個別レイヤ選択)
 ** 引数：
 **　int cam_number		カメラ番号(0～7)
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		システムレイヤ、機種レイヤの共通パラメータを一時保管する
 ** ****************************************************** **/
void CDlgToolLayer::KeepParamComm(int cam_number)
{
	// TODO: ここに実装コードを追加します.
	if (cam_number < 0 || cam_number > 7) return;

	//
	//システムレイヤ
	//
	edit_sys[cam_number].cord_sign.enable = dlg_TabSys1->check_cord_sign.GetCheck();	//カメラ座標表示 表示する
	edit_sys[cam_number].cord_sign.position = dlg_TabSys1->combo_cord_sign.GetCurSel();	//カメラ座標表示 表示位置
	edit_sys[cam_number].cord_sign.color = dlg_TabSys1->color_cord;						//カメラ座標表示 選択色
	edit_sys[cam_number].cam_sign.enable = dlg_TabSys1->check_cam_sign.GetCheck();		//カメラ名称表示 表示する
	edit_sys[cam_number].cam_sign.position = dlg_TabSys1->combo_cam_sign.GetCurSel();	//カメラ名称表示 表示位置
	edit_sys[cam_number].cam_sign.color = dlg_TabSys1->color_cam;						//カメラ名称表示 選択色

	//
	//機種レイヤ
	//
	for (int i = 0; i < 3; i++) {
		edit_param[cam_number].layer[i].select_item = dlg_TabMain->combo_select_layer[i].GetCurSel();	//個別レイヤ選択
	}
}


/** ****************************************************** **
 ** パラメータの保管(機種レイヤ)
 ** 引数：
 **　int cam_number		カメラ番号(0～7)
 **　int layer_num		レイヤ番号(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		機種レイヤの個別レイヤパラメータを一時保管する
 ** ****************************************************** **/
void CDlgToolLayer::KeepParamWorkLayer(int cam_number, int layer_num)
{
	// TODO: ここに実装コードを追加します.
	if (cam_number < 0 || cam_number > 7) return;
	if (layer_num < 0 || layer_num > 2) return;

	char buf[256] = { '\0' };

	int item = dlg_TabMain->combo_select_layer[layer_num].GetCurSel();

	switch (item) {
		//矩形構造
	case 3:
		edit_param[cam_number].layer[layer_num].rect.enable = TRUE;			//設定有り
		dlg_TabRect->edit_set_x.GetWindowTextA(buf, sizeof(buf));
		edit_param[cam_number].layer[layer_num].rect.st_x = atoi(buf);		//始点X
		dlg_TabRect->edit_set_y.GetWindowTextA(buf, sizeof(buf));
		edit_param[cam_number].layer[layer_num].rect.st_y = atoi(buf);		//始点Y
		dlg_TabRect->edit_set_width.GetWindowTextA(buf, sizeof(buf));
		edit_param[cam_number].layer[layer_num].rect.width = atoi(buf);		//幅
		dlg_TabRect->edit_set_height.GetWindowTextA(buf, sizeof(buf));
		edit_param[cam_number].layer[layer_num].rect.height = atoi(buf);	//高さ
		dlg_TabRect->edit_set_degree.GetWindowTextA(buf, sizeof(buf));
		edit_param[cam_number].layer[layer_num].rect.degree = atoi(buf);	//回転
		break;
	}
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
}


/** ****************************************************** **
 ** システムレイヤ描画
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawSysLayer()
{
	// TODO: ここに実装コードを追加します.
	ovlay_sys.m_Overlay->ClearFigures();

	//カメラ座標表示を描画
	if (dlg_TabSys1->check_cord_sign.GetCheck()) {
		int position = dlg_TabSys1->combo_cord_sign.GetCurSel();	//表示位置
		//カメラ座標表示描画
		DrawSysCord(position);

		//レイヤを追加
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Line_Cord[0]);
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Line_Cord[1]);
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Achr_Cord[0]);
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Achr_Cord[1]);
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cord[0]);
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cord[1]);
	}

	//カメラ名称表示を描画
	if (dlg_TabSys1->check_cam_sign.GetCheck()) {
		int position = dlg_TabSys1->combo_cam_sign.GetCurSel();		//表示位置
		//カメラ名称表示描画
		DrawSysCam(position);

		//レイヤを追加
		ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cam);
	}

	m_EvView.ReDraw(true);
}


/** ****************************************************** **
 ** カメラ座標表示描画
 ** 引数：
 **　int position	表示位置 0:右上, 1:右下, 2:左上, 3:左下
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawSysCord(int position)
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();
	int coordinate_type = cam_parameter[select_cam].coordinate_type;	//カメラ構成(0～7)

	int offset_x = 12;
	int offset_y = 12;
	int line_length = 60;

	int base_x = 0;	//基準点X
	int base_y = 0;	//基準点Y
	switch (position) {
		//右上
	case 0:
		base_x = image_view_size_x - offset_x - line_length;
		base_y = offset_y;
		break;
		//右下
	case 1:
		base_x = image_view_size_x - offset_x - line_length;
		base_y = image_view_size_y - offset_y - line_length;
		break;
		//左上
	case 2:
		base_x = offset_x;
		base_y = offset_y;
		break;
		//左下
	case 3:
		base_x = offset_x;
		base_y = image_view_size_y - offset_y - line_length;
		break;
	}
	switch (coordinate_type) {
	case 0:
	case 5:
		break;
	case 1:
	case 4:
		base_x += line_length;
		break;
	case 2:
	case 7:
		base_x += line_length;
		base_y += line_length;
		break;
	case 3:
	case 6:
		base_y += line_length;
		break;
	}

	//カメラ座標とカメラ名称が同じ位置で表示する場合
	if ((position == dlg_TabSys1->combo_cam_sign.GetCurSel()) && dlg_TabSys1->check_cam_sign.GetCheck()) {
		switch (position) {
			//右上・左上
		case 0:
		case 2:
			base_y += 32;
			break;
			//右下・左下
		case 1:
		case 3:
			base_y -= 32;
			break;
		}
	}

	int angle = 0;	//回転角
	switch (coordinate_type) {
	case 0:
	case 5:
		angle = 0;
		break;
	case 1:
	case 4:
		angle = 90;
		break;
	case 2:
	case 7:
		angle = 180;
		break;
	case 3:
	case 6:
		angle = 270;
		break;
	}

	//横線
	ovlay_sys.m_Line_Cord[0].st.x = base_x;
	ovlay_sys.m_Line_Cord[0].st.y = base_y;
	ovlay_sys.m_Line_Cord[0].ed.x = base_x + line_length;
	ovlay_sys.m_Line_Cord[0].ed.y = base_y;
	ovlay_sys.m_Line_Cord[0].SetAxis(FVCL::Data::CFvPoint(0, 0));
	ovlay_sys.m_Line_Cord[0].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Line_Cord[0].GetPen().SetColor(dlg_TabSys1->color_cord);	//座標方向線 色指定

	//縦線
	ovlay_sys.m_Line_Cord[1].st.x = base_x;
	ovlay_sys.m_Line_Cord[1].st.y = base_y;
	ovlay_sys.m_Line_Cord[1].ed.x = base_x;
	ovlay_sys.m_Line_Cord[1].ed.y = base_y + line_length;
	ovlay_sys.m_Line_Cord[1].SetAxis(FVCL::Data::CFvPoint(0, 0));
	ovlay_sys.m_Line_Cord[1].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Line_Cord[1].GetPen().SetColor(dlg_TabSys1->color_cord);	//座標方向線 色指定

	//横線矢印
	ovlay_sys.m_Achr_Cord[0].st.x = ovlay_sys.m_Line_Cord[0].ed.x;
	ovlay_sys.m_Achr_Cord[0].st.y = ovlay_sys.m_Line_Cord[0].ed.y;
	ovlay_sys.m_Achr_Cord[0].ed.x = ovlay_sys.m_Line_Cord[0].st.x;
	ovlay_sys.m_Achr_Cord[0].ed.y = ovlay_sys.m_Line_Cord[0].st.y;
	ovlay_sys.m_Achr_Cord[0].SetAxis(FVCL::Data::CFvPoint(0 - line_length, 0));
	ovlay_sys.m_Achr_Cord[0].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Achr_Cord[0].GetPen().SetColor(dlg_TabSys1->color_cord);	//座標方向矢印 色指定

	//縦線矢印
	ovlay_sys.m_Achr_Cord[1].st.x = ovlay_sys.m_Line_Cord[1].ed.x;
	ovlay_sys.m_Achr_Cord[1].st.y = ovlay_sys.m_Line_Cord[1].ed.y;
	ovlay_sys.m_Achr_Cord[1].ed.x = ovlay_sys.m_Line_Cord[1].st.x;
	ovlay_sys.m_Achr_Cord[1].ed.y = ovlay_sys.m_Line_Cord[1].st.y;
	ovlay_sys.m_Achr_Cord[1].SetAxis(FVCL::Data::CFvPoint(0, 0 - line_length));
	ovlay_sys.m_Achr_Cord[1].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Achr_Cord[1].GetPen().SetColor(dlg_TabSys1->color_cord);	//座標方向矢印 色指定

	//矢印文字
	FVCL::Data::CFvPoint pos0;
	FVCL::Data::CFvPoint pos1;
	switch (coordinate_type) {
	case 0:
	case 5:
		pos0.x = ovlay_sys.m_Line_Cord[0].ed.x;
		pos0.y = ovlay_sys.m_Line_Cord[0].ed.y + 9;
		ovlay_sys.m_Str_Cord[0].SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Top);	//座標文字 位置
		pos1.x = ovlay_sys.m_Line_Cord[1].ed.x + 9;
		pos1.y = ovlay_sys.m_Line_Cord[1].ed.y;
		ovlay_sys.m_Str_Cord[1].SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Bottom);	//座標文字 位置
		break;
	case 1:
	case 4:
		pos0.x = ovlay_sys.m_Line_Cord[1].ed.x - 9;
		pos0.y = ovlay_sys.m_Line_Cord[1].ed.y;
		ovlay_sys.m_Str_Cord[0].SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Bottom);	//座標文字 位置
		pos1.x = ovlay_sys.m_Line_Cord[0].st.x - line_length;
		pos1.y = ovlay_sys.m_Line_Cord[0].ed.y + 9;
		ovlay_sys.m_Str_Cord[1].SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Top);		//座標文字 位置
		break;
	case 2:
	case 7:
		pos0.x = ovlay_sys.m_Line_Cord[0].st.x - line_length;
		pos0.y = ovlay_sys.m_Line_Cord[0].ed.y - 9;
		ovlay_sys.m_Str_Cord[0].SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Bottom);	//座標文字 位置
		pos1.x = ovlay_sys.m_Line_Cord[1].ed.x - 9;
		pos1.y = ovlay_sys.m_Line_Cord[1].st.y - line_length;
		ovlay_sys.m_Str_Cord[1].SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Top);	//座標文字 位置
		break;
	case 3:
	case 6:
		pos0.x = ovlay_sys.m_Line_Cord[1].ed.x + 9;
		pos0.y = ovlay_sys.m_Line_Cord[1].st.y - line_length;
		ovlay_sys.m_Str_Cord[0].SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Top);		//座標文字 位置
		pos1.x = ovlay_sys.m_Line_Cord[0].ed.x;
		pos1.y = ovlay_sys.m_Line_Cord[0].ed.y - 9;
		ovlay_sys.m_Str_Cord[1].SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Bottom);	//座標文字 位置
		break;
	}
	ovlay_sys.m_Str_Cord[0].SetPosition(pos0);
	ovlay_sys.m_Str_Cord[1].SetPosition(pos1);
	if (coordinate_type >= 0 && coordinate_type <= 3) {
		ovlay_sys.m_Str_Cord[0].SetText("X");
		ovlay_sys.m_Str_Cord[1].SetText("Y");
	}
	else {
		ovlay_sys.m_Str_Cord[0].SetText("Y");
		ovlay_sys.m_Str_Cord[1].SetText("X");
	}
	ovlay_sys.m_Str_Cord[0].SetColor(dlg_TabSys1->color_cord);	//座標文字 色指定
	ovlay_sys.m_Str_Cord[1].SetColor(dlg_TabSys1->color_cord);	//座標文字 色指定
}


/** ****************************************************** **
 ** カメラ名称表示描画
 ** 引数：
 **　int position	表示位置 0:右上, 1:右下, 2:左上, 3:左下
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawSysCam(int position)
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();

	int offset_x = 3;
	int offset_y = 3;

	int base_x = 0;	//基準点X
	int base_y = 0;	//基準点Y
	switch (position) {
		//右上
	case 0:
		base_x = image_view_size_x - offset_x;
		base_y = offset_y;
		ovlay_sys.m_Str_Cam.SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Top);	//名称文字 位置
		break;
		//右下
	case 1:
		base_x = image_view_size_x - offset_x;
		base_y = image_view_size_y - offset_y;
		ovlay_sys.m_Str_Cam.SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Bottom);	//名称文字 位置
		break;
		//左上
	case 2:
		base_x = offset_x;
		base_y = offset_y;
		ovlay_sys.m_Str_Cam.SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Top);		//名称文字 位置
		break;
		//左下
	case 3:
		base_x = offset_x;
		base_y = image_view_size_y - offset_y;
		ovlay_sys.m_Str_Cam.SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Bottom);	//名称文字 位置
		break;
	}

	FVCL::Data::CFvPoint pos0;
	pos0.x = base_x;
	pos0.y = base_y;

	ovlay_sys.m_Str_Cam.SetPosition(pos0);

	//カメラ名称
	ovlay_sys.m_Str_Cam.SetColor(dlg_TabSys1->color_cam);		//名称文字 色指定
	ovlay_sys.m_Str_Cam.SetText(cam_parameter[select_cam].device.nick_name);
}


/** ****************************************************** **
 ** 個別レイヤ描画の更新
 ** 引数：
 **　CLayerParam::LayerParameter* layer		レイヤ設定
 **　int select_layer	選択レイヤ(0～2)
 **　int select_item	選択機能(0～5)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkLayer(CLayerParam::LayerParameter* layer, int select_layer, int select_item)
{
	// TODO: ここに実装コードを追加します.

	bool test = false;
	//現在のオーバーレイ図形の削除
	if (befour_item[select_layer] > 0) {
		switch (befour_item[select_layer]) {
			//点構造パラメータ
		case 1:
//			m_Overlay_WorkLayer->DeleteFigure();
			break;
			//直線構造パラメータ
		case 2:
//			m_Overlay_WorkLayer->DeleteFigure();
			break;
			//矩形構造パラメータ
		case 3:
			test = ovlay_wrk[select_layer].m_Overlay->DeleteFigure(ovlay_wrk[select_layer].p_Gdi_Figure);
			break;
			//楕円構造パラメータ
		case 4:
//			m_Overlay_WorkLayer->DeleteFigure();
			break;
			//文字列構造パラメータ
		case 5:
//			m_Overlay_WorkLayer->DeleteFigure();
			break;
		}
	}


	//個別レイヤ描画
	switch (select_item) {
		//点構造描画
	case 1:
		DrawWorkPoint(&layer->point);
		break;
		//直線構造描画
	case 2:
		DrawWorkLine(&layer->line);
		break;
		//矩形構造描画
	case 3:
		DrawWorkRect(&layer->rect, select_layer);
		break;
		//楕円構造描画
	case 4:
		DrawWorkEllipse(&layer->ellipse);
		break;
		//文字列構造描画
	case 5:
		DrawWorkString(&layer->str);
		break;
	}

	//レイヤの変更前機能の更新
	befour_item[select_layer] = select_item;
}


/** ****************************************************** **
 ** 点構造図形の描画
 ** 引数：
 **　CLayerParam::PointParam* param		点構造パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkPoint(CLayerParam::PointParam* param)
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** 直線構造図形の描画
 ** 引数：
 **　CLayerParam::LineParam* param		直線構造パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkLine(CLayerParam::LineParam* param)
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** 矩形構造図形の描画
 ** 引数：
 **　CLayerParam::RectangleParam* param		矩形構造パラメータ
 **　int select_layer	選択レイヤ(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkRect(CLayerParam::RectangleParam* param, int select_layer)
{
	// TODO: ここに実装コードを追加します.

	//未設定の場合は始点/終点を画像サイズの範囲で初期設定
	if (!param->enable) {
		param->st_x = 0;
		param->st_y = 0;
		param->width = 100;		//100にする
		param->height = 100;	//100にする
		param->degree = 0;
	}

	int xs = param->st_x;
	int ys = param->st_y;
	int x_size = param->width;
	int y_size = param->height;
	int degree = param->degree;

	//想定される描画矩形の作成
	FVCL::Data::CFvRectangle draw_rect;
	draw_rect.st.x = xs;
	draw_rect.st.y = ys;
	draw_rect.ed.x = xs + x_size - 1;
	draw_rect.ed.y = ys + y_size - 1;
	draw_rect.angle = degree;

	//描画矩形に外接する矩形の取得
	FVCL::Data::CFvRegion region_rect(draw_rect);	//描画矩形をRegion化
	int x_min = region_rect.GetXmin();
	int y_min = region_rect.GetYmin();
	int x_max = region_rect.GetXmax();
	int y_max = region_rect.GetYmax();

	//描画矩形が視野範囲内か確認
	int image_width = m_Image.GetHorzSize();
	int image_height = m_Image.GetVertSize();
	if (x_min < 0 || image_width <= x_max ||
		y_min < 0 || image_height <= y_max) {
		//初期設定を入れる
		param->st_x = 0;
		param->st_y = 0;
		param->width = 100;		//100にする
		param->height = 100;	//100にする
		param->degree = 0;

		CString title;
		this->GetWindowTextA(title);
		MessageBoxA(Tips0.SelStr(language.jpn,
			"描画矩形がカメラ視野範囲外のため、図形を初期化しました",
			"The shapes has been initialized because the drawing rectangle is out of the camera field of view"),
			title, MB_OK | MB_ICONINFORMATION);
	}

	//検査エリア
	ovlay_wrk[select_layer].m_Rect_Layer.st.x = param->st_x;
	ovlay_wrk[select_layer].m_Rect_Layer.st.y = param->st_y;
	ovlay_wrk[select_layer].m_Rect_Layer.ed.x = param->st_x + param->width - 1;
	ovlay_wrk[select_layer].m_Rect_Layer.ed.y = param->st_y + param->height - 1;
	ovlay_wrk[select_layer].m_Rect_Layer.SetAngle(param->degree);		//回転角を設定

	ovlay_wrk[select_layer].p_Gdi_Figure = ovlay_wrk[select_layer].m_Overlay->DrawFigure(ovlay_wrk[select_layer].m_Rect_Layer);
}


/** ****************************************************** **
 ** 楕円構造図形の描画
 ** 引数：
 **　CLayerParam::EllipseParam* param		楕円構造パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkEllipse(CLayerParam::EllipseParam* param)
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** 文字列構造図形の描画
 ** 引数：
 **　CLayerParam::StringParam* param		文字列構造パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::DrawWorkString(CLayerParam::StringParam* param)
{
	// TODO: ここに実装コードを追加します.
}


/** ****************************************************** **
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgToolLayer::WholeMagnification()
{
	// TODO: ここに実装コードを追加します.
	int image_width = m_Image.GetHorzSize();
	int image_height = m_Image.GetVertSize();
	double mag_x = (double)image_view_size_x / (double)image_width;
	double mag_y = (double)image_view_size_y / (double)image_height;
	double mag_min = min(mag_x, mag_y);
	//if (mag_min > 1) mag_min = 1;		※1倍以上もOKとする

	return mag_min;
}


/** ****************************************************** **
 ** 矩形図形のカーソル描画
 ** 引数：
 **　CPoint* pos	マウスポジション
 **　int select_layer	選択レイヤ(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::ChangeCurselRect(CPoint* pos, int select_layer)
{
	// TODO: ここに実装コードを追加します.

	BOOL focus = FALSE;	//検査エリア図形内フラグ

	//検査エリア 外接矩形の位置確認(±5pix範囲)
	int focus_pos = ovlay_wrk[select_layer].m_Rect_Layer.CheckFocusMarkPosition(*pos, 5.0);

	//回転による辺変更
	int angle = (int)ovlay_wrk[select_layer].m_Rect_Layer.GetAngle();
	int rotation = 0;
	if (0 <= angle && angle <= 44) {
		//何もしない
	}
	else if (45 <= angle && angle <= 134) {
		rotation = 1;
	}
	else if (135 <= angle && angle <= 224) {
		rotation = 2;
	}
	else if (225 <= angle && angle <= 314) {
		rotation = 3;
	}
	else if (315 <= angle && angle <= 359) {
		//何もしない
	}

	if (focus_pos != 0) focus = TRUE;

	// カーソル位置が矩形範囲内だったらカーソルを変更
	if (focus) {
		switch (focus_pos) {
			//中央
		case FOCUS_CENTER:
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
			break;
			//左辺もしくは右辺
		case FOCUS_LEFT:
		case FOCUS_RIGHT:
			if (rotation == 0 || rotation == 2) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
			if (rotation == 1 || rotation == 3) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
			break;
			//上辺もしくは下辺
		case FOCUS_TOP:
		case FOCUS_BOTTOM:
			if (rotation == 0 || rotation == 2) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENS));
			if (rotation == 1 || rotation == 3) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZEWE));
			break;
			//左上もしくは右下
		case (FOCUS_TOP | FOCUS_LEFT):
		case (FOCUS_BOTTOM | FOCUS_RIGHT):
			if (rotation == 0 || rotation == 2) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENWSE));
			if (rotation == 1 || rotation == 3) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENESW));
			break;
			//右上もしくは左下
		case (FOCUS_TOP | FOCUS_RIGHT):
		case (FOCUS_BOTTOM | FOCUS_LEFT):
			if (rotation == 0 || rotation == 2) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENESW));
			if (rotation == 1 || rotation == 3) SetCursor(AfxGetApp()->LoadStandardCursor(IDC_SIZENWSE));
			break;
		}
	}
	else {
		//図形範囲外でマウス左ボタンがクリックされていることを確認
		byte keys[256];
		GetKeyboardState(keys);
		if (keys[VK_LBUTTON] & 0x80) {
			EditFigure(true, false, select_layer, 3);	//図形の選択解除
		}
	}

	m_focus_pos = focus_pos;	//フォーカス情報記憶
}


/** ****************************************************** **
 ** 矩形図形XYの更新
 ** 引数：
 **　CPoint* pos	マウスポジション
 **　int select_layer	選択レイヤ(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::UpdateFigureRectXY(CPoint* pos, int select_layer)
{
	// TODO: ここに実装コードを追加します.
	FVCL::GDI::CFvGdiRectangle* m_Rect = &ovlay_wrk[select_layer].m_Rect_Layer;

	ovlay_wrk[select_layer].m_Overlay->ClearFigures();

	//(左クリックしている場合)矩形移動、サイズ変更処理 移動量
	int dx = pos->x - m_pos.x;
	int dy = pos->y - m_pos.y;

	//回転による辺変更
	int angle = (int)m_Rect->GetAngle();
	int rotation = 0;
	if (0 <= angle && angle <= 44) {
		//何もしない
	}
	else if (45 <= angle && angle <= 134) {
		rotation = 1;
	}
	else if (135 <= angle && angle <= 224) {
		rotation = 2;
	}
	else if (225 <= angle && angle <= 314) {
		rotation = 3;
	}
	else if (315 <= angle && angle <= 359) {
		//何もしない
	}

	//現在の検査エリア矩形位置
	FVCL::Data::CFvPoint rect_pos = m_Rect->GetPosition();

	//更新する検査エリア矩形位置
	FVCL::Data::CFvPoint new_pos = rect_pos;

	switch (m_focus_pos) {
		// 移動
	case FOCUS_CENTER:
		m_Rect->st.x += dx;
		m_Rect->st.y += dy;
		m_Rect->ed.x += dx;
		m_Rect->ed.y += dy;
		break;
		// 左側サイズ変更
	case FOCUS_LEFT:
		if (rotation == 0) m_Rect->st.x += dx;
		if (rotation == 1) {
			m_Rect->ed.x -= dy;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 2) {
			m_Rect->ed.x += dx;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 3) {
			m_Rect->ed.x += dy;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		break;
		// 右側サイズ変更
	case FOCUS_RIGHT:
		if (rotation == 0) m_Rect->ed.x += dx;
		if (rotation == 1) m_Rect->ed.x += dy;
		if (rotation == 2) m_Rect->ed.x -= dx;
		if (rotation == 3) m_Rect->ed.x -= dy;
		break;
		// 上側サイズ変更
	case FOCUS_TOP:
		if (rotation == 0) m_Rect->st.y += dy;
		if (rotation == 1) {
			m_Rect->ed.y += dx;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 2) {
			m_Rect->ed.y += dy;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 3) {
			m_Rect->ed.y -= dx;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		break;
		// 下側サイズ変更
	case FOCUS_BOTTOM:
		if (rotation == 0) m_Rect->ed.y += dy;
		if (rotation == 1) m_Rect->ed.y -= dx;
		if (rotation == 2) m_Rect->ed.y -= dy;
		if (rotation == 3) m_Rect->ed.y += dx;
		break;
		// 左上サイズ変更
	case (FOCUS_TOP | FOCUS_LEFT):
		if (rotation == 0) {
			m_Rect->st.x += dx;
			m_Rect->st.y += dy;
		}
		if (rotation == 1) {
			m_Rect->ed.x -= dy;
			m_Rect->ed.y += dx;
			new_pos.x += dx;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 2) {
			m_Rect->ed.x += dx;
			m_Rect->ed.y += dy;
			new_pos.x += dx;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 3) {
			m_Rect->ed.x += dy;
			m_Rect->ed.y -= dx;
			new_pos.x += dx;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		break;
		// 左下サイズ変更
	case (FOCUS_BOTTOM | FOCUS_LEFT):
		if (rotation == 0) {
			m_Rect->st.x += dx;
			m_Rect->ed.y += dy;
		}
		if (rotation == 1) {
			m_Rect->ed.x -= dy;
			m_Rect->ed.y -= dx;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 2) {
			m_Rect->ed.x += dx;
			m_Rect->ed.y -= dy;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 3) {
			m_Rect->ed.x += dy;
			m_Rect->ed.y += dx;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		break;
		// 右下サイズ変更
	case (FOCUS_BOTTOM | FOCUS_RIGHT):
		if (rotation == 0) {
			m_Rect->ed.x += dx;
			m_Rect->ed.y += dy;
		}
		if (rotation == 1) {
			m_Rect->ed.x += dy;
			m_Rect->ed.y -= dx;
		}
		if (rotation == 2) {
			m_Rect->ed.x -= dx;
			m_Rect->ed.y -= dy;
		}
		if (rotation == 3) {
			m_Rect->ed.x -= dy;
			m_Rect->ed.y += dx;
		}
		break;
		// 右上サイズ変更
	case (FOCUS_TOP | FOCUS_RIGHT):
		if (rotation == 0) {
			m_Rect->ed.x += dx;
			m_Rect->st.y += dy;
		}
		if (rotation == 1) {
			m_Rect->ed.x += dy;
			m_Rect->ed.y += dx;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 2) {
			m_Rect->ed.x -= dx;
			m_Rect->ed.y += dy;
			new_pos.y += dy;
			m_Rect->SetPosition(new_pos);
		}
		if (rotation == 3) {
			m_Rect->ed.x -= dy;
			m_Rect->ed.y -= dx;
			new_pos.x += dx;
			m_Rect->SetPosition(new_pos);
		}
		break;
	}
	m_pos = *pos;

	ovlay_wrk[select_layer].p_Gdi_Figure = ovlay_wrk[select_layer].m_Overlay->DrawFigure(*m_Rect);

	// 描画更新
	m_EvView.ReDraw();

	int edit_x = (int)m_Rect->st.x;
	int edit_y = (int)m_Rect->st.y;
	int edit_width = (int)m_Rect->GetWidth() + 1;
	int edit_height = (int)m_Rect->GetHeight() + 1;

	//始点とサイズの更新
	UpdateRectEditXY(edit_x, edit_y, edit_width, edit_height);
}


/** ****************************************************** **
 ** 矩形図形回転の更新
 ** 引数：
 **　int select_layer	選択レイヤ(0～2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::UpdateFigureRectDeg(int select_layer)
{
	// TODO: ここに実装コードを追加します.
	char buf[256] = { '\0' };

	dlg_TabRect->edit_set_degree.GetWindowTextA(buf, sizeof(buf));
	int degree = atoi(buf);	//回転

	ovlay_wrk[select_layer].m_Overlay->ClearFigures();
	ovlay_wrk[select_layer].m_Rect_Layer.SetAngle(degree);

	ovlay_wrk[select_layer].p_Gdi_Figure = ovlay_wrk[select_layer].m_Overlay->DrawFigure(ovlay_wrk[select_layer].m_Rect_Layer);

	// 描画更新
	m_EvView.ReDraw();
}


/** ****************************************************** **
 ** 始点XYとサイズ入力の更新
 ** 引数：
 **　int st_x	X入力
 **　int st_y	Y入力
 **　int width	幅入力
 **　int height	高さ入力
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::UpdateRectEditXY(int st_x, int st_y, int width, int height)
{
	// TODO: ここに実装コードを追加します.

	//基本タブの入力コントロールを更新
	dlg_TabRect->edit_set_x.SetWindowTextA(Tips0.IntToString(st_x));
	dlg_TabRect->edit_set_y.SetWindowTextA(Tips0.IntToString(st_y));
	dlg_TabRect->edit_set_width.SetWindowTextA(Tips0.IntToString(width));
	dlg_TabRect->edit_set_height.SetWindowTextA(Tips0.IntToString(height));
}


/** ****************************************************** **
 ** 回転入力の更新
 ** 引数：
 **　int degree	回転入力
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::UpdateRectEditDeg(int degree)
{
	// TODO: ここに実装コードを追加します.

	//基本タブの入力コントロールを更新
	dlg_TabRect->edit_set_degree.SetWindowTextA(Tips0.IntToString(degree));
	dlg_TabRect->slider_degree.SetPos(degree);
}


/** ****************************************************** **
 ** 図形入力の有効無効化
 ** 引数：
 **　BOOL enable		入力の有効無効化
 **　int select_item	選択機能(0～5)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::EnableEdit(BOOL enable, int select_item)
{
	// TODO: ここに実装コードを追加します.

	//基本タブの入力コントロールを更新
	switch (select_item) {
		//矩形編集タブダイアログ
	case 3:
		dlg_TabRect->edit_set_x.EnableWindow(enable);
		dlg_TabRect->edit_set_y.EnableWindow(enable);
		dlg_TabRect->edit_set_width.EnableWindow(enable);
		dlg_TabRect->edit_set_height.EnableWindow(enable);
		dlg_TabRect->edit_set_degree.EnableWindow(enable);
		dlg_TabRect->slider_degree.EnableWindow(enable);
		break;
	}
}


/** ****************************************************** **
 ** 図形の編集操作
 ** 引数：
 **　bool enable_edit	編集有無
 **　bool enable_select	図形選択有無
 **　int select_layer	選択レイヤ(0～2)
 **　int select_item	選択機能(0～5)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::EditFigure(bool enable_edit, bool enable_select, int select_layer, int select_item)
{
	// TODO: ここに実装コードを追加します.

	ovlay_wrk[select_layer].m_Overlay->SetActive(enable_edit);	//マウス操作の有効化

	ovlay_wrk[select_layer].m_Overlay->ClearFigures();

	switch (select_item) {
		//矩形オーバーレイ
	case 3:
		ovlay_wrk[select_layer].m_Rect_Layer.SetSelect(enable_select);	//図形の選択状態有効化
		ovlay_wrk[select_layer].m_Rect_Layer.SetFillColor(RGB(0xFF, 0xFF, 0x00));	//図形の色 黄色
		ovlay_wrk[select_layer].m_Rect_Layer.SetFillAlpha(0x1F);		//図形の透明度
		ovlay_wrk[select_layer].m_Rect_Layer.SetFillEnable(enable_edit);	//図形の塗りつぶし有効化
		break;
	}

	ovlay_wrk[select_layer].p_Gdi_Figure = ovlay_wrk[select_layer].m_Overlay->DrawFigure(ovlay_wrk[select_layer].m_Rect_Layer);

	m_EvView.ReDraw(true);			//表示更新
}


/** ****************************************************** **
 ** 図形のやり直し操作
 ** 引数：
 **　int select_layer	選択レイヤ(0～2)
 **　int select_item	選択機能(0～5)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::RedoFigure(int select_layer, int select_item)
{
	// TODO: ここに実装コードを追加します.
	int select_cam = combo_select_cam.GetCurSel();
	CLayerParam::LayerParameter* layer = &edit_param[select_cam].layer[select_layer];	//レイヤパラメータ

	//個別レイヤオーバーレイの更新
	ovlay_wrk[select_layer].m_Overlay->ClearFigures();

	switch (select_item) {
		//矩形オーバーレイ
	case 3:
		ovlay_wrk[select_layer].m_Rect_Layer.st.x = layer->rect.st_x;
		ovlay_wrk[select_layer].m_Rect_Layer.st.y = layer->rect.st_y;
		ovlay_wrk[select_layer].m_Rect_Layer.ed.x = layer->rect.st_x + layer->rect.width - 1;
		ovlay_wrk[select_layer].m_Rect_Layer.ed.y = layer->rect.st_y + layer->rect.height - 1;
		ovlay_wrk[select_layer].m_Rect_Layer.SetAngle(layer->rect.degree);
		ovlay_wrk[select_layer].m_Rect_Layer.SetSelect(false);	//図形の選択状態無効化
		ovlay_wrk[select_layer].m_Rect_Layer.SetFillEnable(false);	//図形の塗りつぶし無効化

		ovlay_wrk[select_layer].p_Gdi_Figure = ovlay_wrk[select_layer].m_Overlay->DrawFigure(ovlay_wrk[select_layer].m_Rect_Layer);
		break;
	}

	ovlay_wrk[select_layer].m_Overlay->SetActive(false);	//マウス操作の無効化

	m_EvView.ReDraw(true); //表示更新

	//展開パラメータ
	int edit_x = 0;
	int edit_y = 0;
	int width = 0;
	int height = 0;
	int degree = 0;
	switch (select_item) {
		//矩形編集タブ
	case 3:
		edit_x = layer->rect.st_x;
		edit_y = layer->rect.st_y;
		width = layer->rect.width;
		height = layer->rect.height;
		degree = layer->rect.degree;
		UpdateRectEditXY(edit_x, edit_y, width, height); //XYとサイズ入力の更新
		UpdateRectEditDeg(degree);	//回転入力の更新
		break;
	}

	EnableEdit(FALSE, select_item);	//図形入力の有効無効化

	do_click = FALSE;
}


/** ****************************************************** **
 ** カメラ座標表示 色選択
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::SelectCordColor()
{
	// TODO: ここに実装コードを追加します.

	CColorDialog dlg(dlg_TabSys1->color_cord);

	if (dlg.DoModal() == IDOK) {
		dlg_TabSys1->color_cord = dlg.GetColor();
	}
}


/** ****************************************************** **
 ** カメラ名称表示 色選択
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::SelectCamColor()
{
	// TODO: ここに実装コードを追加します.

	CColorDialog dlg(dlg_TabSys1->color_cam);

	if (dlg.DoModal() == IDOK) {
		dlg_TabSys1->color_cam = dlg.GetColor();
	}
}


/** ****************************************************** **
 ** データ読込ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int select_cam = combo_select_cam.GetCurSel();

	if (select_cam >= 0 && select_cam <= 3) {
		//選択カメラが存在する場合、画像を要求
		if (cam_parameter[select_cam].device.model_code != 0) {
			GetImage();		//カメラ画像の読み込み
			m_EvView.ReDraw(true);

			LoadParameter();	//パラメータの展開
		}

		befour_edit_cam = select_cam;
	}
}


/** ****************************************************** **
 ** 全て表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}


/** ****************************************************** **
 ** 全体表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(WholeMagnification());	//倍率指定
	m_EvView.ReDraw(true);			//表示更新
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定
	m_EvView.ReDraw(true);			//表示更新
}


/** ****************************************************** **
 ** ×2ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(2.0);	//倍率指定
	m_EvView.ReDraw(true);			//表示更新
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);

	msg = Tips0.SelStr(language.jpn, "画像レイヤ設定を更新します。よろしいですか？", "Update the image layer setting. Is it OK?");
	int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgToolLayer out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgToolLayer out (CANCEL) ***");

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
void CDlgToolLayer::ICommandPre(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolLayer*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolLayer::ICommandPost(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolLayer*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolLayer::IWndMsgPre(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolLayer*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolLayer::IWndMsgPost(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolLayer*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgToolLayer::OnCommandPre(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgToolLayer::OnCommandPost(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
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
void CDlgToolLayer::OnWndMsgPre(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
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
void CDlgToolLayer::OnWndMsgPost(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		//個別レイヤの確認
		int select_layer = tab_setting_param.GetCurSel() - 2;	//レイヤ(0～2)
		if (select_layer < 0 || select_layer > 2) return;

		//選択図形の確認
		int select_item = dlg_TabMain->combo_select_layer[select_layer].GetCurSel();

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);
		switch (message)
		{
		case WM_PAINT:			// 再描画イベント.
			break;
		case WM_MOUSEMOVE:

			if (false == ovlay_wrk[select_layer].m_Overlay->GetActive()) break;

			//クリック前のカーソル描画変更
			if (!do_click) {
				switch (select_item) {
					//点十字
				case 1:
//					ChangeCurselRect(&pos, select_layer);
					break;
					//直線図形
				case 2:
//					ChangeCurselRect(&pos, select_layer);
					break;
					//矩形図形
				case 3:
					ChangeCurselRect(&pos, select_layer);
					break;
					//楕円図形
				case 4:
//					ChangeCurselRect(&pos, select_layer);
					break;
					//文字列
				case 5:
//					ChangeCurselRect(&pos, select_layer);
					break;
				}
			}

			//クリック中の検査エリア描画
			if (do_click) {
				switch (select_item) {
					//点十字
				case 1:
//					UpdateFigureRectXY(&pos, select_layer);
					break;
					//直線図形
				case 2:
//					UpdateFigureRectXY(&pos, select_layer);
					break;
					//矩形図形
				case 3:
					UpdateFigureRectXY(&pos, select_layer);
					break;
					//楕円図形
				case 4:
//					UpdateFigureRectXY(&pos, select_layer);
					break;
					//文字列
				case 5:
//					UpdateFigureRectXY(&pos, select_layer);
					break;
				}
			}
			break;
		case WM_LBUTTONDOWN:
			if (m_focus_pos != 0) {
				// Viewのグリップをオフ
				m_EvView.SetGripOption(FVCL::GripOption::DISABLE);

				//図形の選択状態有効化
				EditFigure(true, true, select_layer, select_item);

				// クリックフラグTRUE＆クリック位置を保持
				do_click = TRUE;
				m_pos = pos;
			}
			break;
		case WM_LBUTTONUP:
			if (do_click) {
				// Viewのグリップをオン
				m_EvView.SetGripOption(FVCL::GripOption::ENABLE);
			}

			do_click = FALSE;
			break;
		}
	}
}


/** ****************************************************** **
 ** マウス左ボタン解放
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	if (do_click) {
		// Viewのグリップをオン
		m_EvView.SetGripOption(FVCL::GripOption::ENABLE);
	}

	//画像ビュー以外でのボタン操作でも処理する
	do_click = FALSE;

	CDialogEx::OnLButtonUp(nFlags, point);
}


/** ****************************************************** **
 ** カメラ選択 コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int select_cam = combo_select_cam.GetCurSel();

	//レイヤパラメータの保管
	if (befour_edit_cam != -1) KeepParamComm(befour_edit_cam);

	//画像を黒く塗りつぶし
	m_Image.Clear(0, 1);
	m_EvView.SetImage(&m_Image);
	m_EvView.ReDraw(true);

	//選択カメラが存在する場合、カメラ切替を要求
	if (cam_parameter[select_cam].device.model_code != 0) {
		CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

		//カメラ切替メッセージ処理
		LRESULT rtn1 = dlg->SendMessageA(WM_APP_COMMAND_CAM_CHANGE, select_cam);
		if (rtn1 != 0x1000) {
			outlog.LogOutFile(1, "コマンドメッセージ:CDlgToolLayerのカメラ切替失敗");
			return;
		}
	}

	tab_setting_param.EnableWindow(FALSE);	//タブ無効化
	dlg_TabMain->ShowWindow(SW_HIDE);		//描画選択タブ
	dlg_TabSys1->ShowWindow(SW_HIDE);		//システムレイヤタブ
	dlg_TabRect->ShowWindow(SW_HIDE);		//矩形編集タブ

	befour_edit_cam = -1;	//カメラ編集していない
}


/** ****************************************************** **
 ** パラメータ切替タブSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnTcnSelchangeTab1(NMHDR *pNMHDR, LRESULT *pResult)
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	*pResult = 0;

	int select_cam = combo_select_cam.GetCurSel();

	//編集していたレイヤの保存と表示状態変更
	if (befour_edit_layer != -1) {
		//編集レイヤの保管
		KeepParamWorkLayer(select_cam, befour_edit_layer);
		//表示状態変更
		int select_item = dlg_TabMain->combo_select_layer[befour_edit_layer].GetCurSel();
		if (select_item > 0) {
			EditFigure(false, false, befour_edit_layer, select_item);	//図形の編集操作の解除
		}
	}

	int sw[5] = { SW_HIDE, SW_HIDE, SW_HIDE, SW_HIDE, SW_HIDE };
	int layer1 = dlg_TabMain->combo_select_layer[0].GetCurSel();
	int layer2 = dlg_TabMain->combo_select_layer[1].GetCurSel();
	int layer3 = dlg_TabMain->combo_select_layer[2].GetCurSel();

	switch (tab_setting_param.GetCurSel()) {
		//描画選択を表示
	case 0:
		dlg_TabMain->ShowWindow(SW_SHOW);
		dlg_TabSys1->ShowWindow(SW_HIDE);
		break;
		//システムレイヤを表示
	case 1:
		dlg_TabMain->ShowWindow(SW_HIDE);
		dlg_TabSys1->ShowWindow(SW_SHOW);
		break;
		//個別レイヤ1を表示
	case 2:
		dlg_TabMain->ShowWindow(SW_HIDE);
		dlg_TabSys1->ShowWindow(SW_HIDE);
		//描画図形を選択
		if (layer1 > 0) sw[layer1 - 1] = SW_SHOW;
		break;
		//個別レイヤ2を表示
	case 3:
		dlg_TabMain->ShowWindow(SW_HIDE);
		dlg_TabSys1->ShowWindow(SW_HIDE);
		//描画図形を選択
		if (layer2 > 0) sw[layer2 - 1] = SW_SHOW;
		break;
		//個別レイヤ3を表示
	case 4:
		dlg_TabMain->ShowWindow(SW_HIDE);
		dlg_TabSys1->ShowWindow(SW_HIDE);
		//描画図形を選択
		if (layer3 > 0) sw[layer3 - 1] = SW_SHOW;
		break;
	}

	//描画編集タブの展開
	int layer_num = tab_setting_param.GetCurSel() - 2;
	if (sw[2] == SW_SHOW) LoadParameterTabRect(layer_num);

	//個別レイヤの編集ダイアログを表示
	//dlg_TabRect->ShowWindow(sw[0]);		//十字点編集
	//dlg_TabRect->ShowWindow(sw[1]);		//直線編集
	dlg_TabRect->ShowWindow(sw[2]);		//矩形編集
	//dlg_TabRect->ShowWindow(sw[3]);		//円編集
	//dlg_TabRect->ShowWindow(sw[4]);		//文字列編集

	//個別レイヤ編集タブの場合
	if (layer_num >= 0 && layer_num <= 2) {
		befour_edit_layer = layer_num;
	}
	else {
		befour_edit_layer = -1;		//描画設定またはシステムレイヤタブの場合
	}
}


/** ****************************************************** **
 ** イベントメッセージ処理
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgToolLayer::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	//描画選択タブ(CTabLayerParamMain)からのボタンClickedを受信し処理する
	//システムレイヤタブ(CTabLayerParamSys1)からのボタンClickedを受信し処理する
	//矩形描画タブ(CTabLayerParamRect)からのボタンClickedを受信し処理する
	switch (LOWORD(wParam)) {
		//
		//描画選択タブ
		//

		//描画選択タブの個別レイヤ1コンボボックス選択を受信
	case IDC_COMBO1_TabLayerParamMain:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			int select_item = dlg_TabMain->combo_select_layer[0].GetCurSel();
			int select_cam = combo_select_cam.GetCurSel();
			DrawWorkLayer(&edit_param[select_cam].layer[0], 0, select_item);	//個別レイヤ描画の更新
			m_EvView.ReDraw(false);			//表示更新
		}
		break;
		//描画選択タブの個別レイヤ2コンボボックス選択を受信
	case IDC_COMBO2_TabLayerParamMain:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			int select_item = dlg_TabMain->combo_select_layer[1].GetCurSel();
			int select_cam = combo_select_cam.GetCurSel();
			DrawWorkLayer(&edit_param[select_cam].layer[1], 1, select_item);	//個別レイヤ描画の更新
			m_EvView.ReDraw(false);			//表示更新
		}
		break;
		//描画選択タブの個別レイヤ3コンボボックス選択を受信
	case IDC_COMBO3_TabLayerParamMain:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			int select_item = dlg_TabMain->combo_select_layer[2].GetCurSel();
			int select_cam = combo_select_cam.GetCurSel();
			DrawWorkLayer(&edit_param[select_cam].layer[2], 2, select_item);	//個別レイヤ描画の更新
			m_EvView.ReDraw(false);			//表示更新
		}
		break;

		//
		//システムレイヤタブ
		//

		//システムレイヤタブのカメラ座標表示 表示するボタンClickedを受信
	case IDC_CHECK1_TabLayerParamSys1:
		if (HIWORD(wParam) == BN_CLICKED) {
			DrawSysLayer();	//システムレイヤ描画
		}
		break;
		//システムレイヤタブのカメラ座標表示 表示位置コンボボックス選択を受信
	case IDC_COMBO1_TabLayerParamSys1:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			DrawSysLayer();	//システムレイヤ描画
		}
		break;
		//システムレイヤタブのカメラ座標表示 色選択ボタンClickedを受信
	case IDC_BUTTON1_TabLayerParamSys1:
		if (HIWORD(wParam) == BN_CLICKED) {
			SelectCordColor();	//カメラ座標表示 色選択
			dlg_TabSys1->m_cord_sign.Invalidate();
			DrawSysLayer();		//システムレイヤ描画
		}
		break;
		//システムレイヤタブのカメラ名称表示 表示するボタンClickedを受信
	case IDC_CHECK2_TabLayerParamSys1:
		if (HIWORD(wParam) == BN_CLICKED) {
			DrawSysLayer();	//システムレイヤ描画
		}
		break;
		//システムレイヤタブのカメラ名称表示 表示位置コンボボックス選択を受信
	case IDC_COMBO2_TabLayerParamSys1:
		if (HIWORD(wParam) == CBN_SELCHANGE) {
			DrawSysLayer();	//システムレイヤ描画
		}
		break;
		//システムレイヤタブのカメラ名称表示 色選択ボタンClickedを受信
	case IDC_BUTTON2_TabLayerParamSys1:
		if (HIWORD(wParam) == BN_CLICKED) {
			SelectCamColor();	//カメラ名称表示 色選択
			dlg_TabSys1->m_cam_sign.Invalidate();
			DrawSysLayer();		//システムレイヤ描画
		}
		break;

		//
		//矩形描画タブ
		//

		//矩形描画タブの編集ボタンClickedを受信
	case IDC_BUTTON1_TabLayerParamRect:
		if (HIWORD(wParam) == BN_CLICKED) {
			int select_layer = tab_setting_param.GetCurSel() - 2;	//レイヤ(0～2)
			EnableEdit(TRUE, 3);	//図形の入力有効化
			EditFigure(true, false, select_layer, 3);		//図形の編集操作
		}
		break;
		//矩形描画タブのやり直しボタンClickedを受信
	case IDC_BUTTON2_TabLayerParamRect:
		if (HIWORD(wParam) == BN_CLICKED) {
			int select_layer = tab_setting_param.GetCurSel() - 2;	//レイヤ(0～2)
			RedoFigure(select_layer, 3);
		}
		break;
		//矩形描画タブの回転変更を受信
	case IDC_EDIT5_TabLayerParamRect:
		if (HIWORD(wParam) == EN_UPDATE) {
			int select_layer = tab_setting_param.GetCurSel() - 2;	//レイヤ(0～2)
			if (false == ovlay_wrk[select_layer].m_Overlay->GetActive()) break;

			UpdateFigureRectDeg(select_layer);
		}
		break;
	}

	return CDialogEx::OnCommand(wParam, lParam);
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolLayer::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。

	//タブダイアログの削除
	dlg_TabMain->DestroyWindow();
	delete dlg_TabMain;
	dlg_TabSys1->DestroyWindow();
	delete dlg_TabSys1;
	dlg_TabRect->DestroyWindow();
	delete dlg_TabRect;
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgToolLayer::PreTranslateMessage(MSG* pMsg)
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
