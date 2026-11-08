/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgToolMeas.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.10.04	M.Iwasaki	K34004(ファーストボード対応)
 ** 注記：
 ** 説明：ツール計測ダイアログ
 **		画像内の長さや濃度を計測できる
 **/

// DlgToolMeas.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgToolMeas.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CScopes			sys_scope[16];			//システムスコープパラメータ
extern CCameraSettings	cam_parameter[8];		//カメラパラメータ
extern CLayer			sys_layer[8];			//システムレイヤパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ
extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;				//言語設定用

extern char* APP_ROOT;
extern FVCL::Data::CFvImage m_ImageBuffer[8];	//画像メモリ

// CDlgToolMeas ダイアログ

IMPLEMENT_DYNAMIC(CDlgToolMeas, CDialogEx)

CDlgToolMeas::CDlgToolMeas(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgToolMeas, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 768;
	image_view_size_y = 576;

	select_cam = 0;		//初期カメラ1
	select_scope = 0;	//初期スコープ1
	select_view = 0;	//初期視野1
	point_magn = 0;		//初期スライダーポイント0
	magn.min = 1.0;		//初期最小倍率
	magn.max = 2.0;		//最大倍率(2.0固定)
	magn.interval = (magn.max - magn.min) / 10;	//間隔
	draw_tim = 0;
}

CDlgToolMeas::~CDlgToolMeas()
{
}

void CDlgToolMeas::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_LIST1, ope_log);
	DDX_Control(pDX, IDC_COMBO1, combo_set_scope);
	DDX_Control(pDX, IDC_COMBO2, combo_set_view);
	DDX_Control(pDX, IDC_SLIDER1, slider_magn);
	DDX_Control(pDX, IDC_EDIT1, edit_magn);
	DDX_Control(pDX, IDC_CHECK1, check_calib_line);
	DDX_Control(pDX, IDC_STATIC_SCOPE, static_scope);
	DDX_Control(pDX, IDC_STATIC_TEXT_VIEW, static_view);
	DDX_Control(pDX, IDC_STATIC_VIEW_THE_LINE, static_view_the_line);
	DDX_Control(pDX, IDC_STATIC_VIEW_THE_DIS_ANGLE, static_dis_angle);
	DDX_Control(pDX, IDC_BUTTON6, btn_line_profile);
	DDX_Control(pDX, IDC_BUTTON8, btn_distance);
	DDX_Control(pDX, IDC_BUTTON7, btn_clear);
	DDX_Control(pDX, IDC_BUTTON1, btn_full);
	DDX_Control(pDX, IDC_BUTTON2, btn_1x);
	DDX_Control(pDX, IDC_BUTTON5, btn_fromfile);
	DDX_Control(pDX, IDC_BUTTON4, btn_1shot);
	DDX_Control(pDX, IDC_BUTTON3, btn_live);
	DDX_Control(pDX, IDC_STATIC_LOADIMAGE, static_loadimage);
	DDX_Control(pDX, IDOK, btn_exit);
}


BEGIN_MESSAGE_MAP(CDlgToolMeas, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgToolMeas::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgToolMeas::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgToolMeas::OnBnClickedButton2)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgToolMeas::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgToolMeas::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgToolMeas::OnBnClickedButton5)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgToolMeas::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CDlgToolMeas::OnCbnSelchangeCombo2)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgToolMeas::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON7, &CDlgToolMeas::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON8, &CDlgToolMeas::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_CHECK1, &CDlgToolMeas::OnBnClickedCheck1)
	ON_STN_CLICKED(IDC_STATIC_VIEW, &CDlgToolMeas::OnStnClickedStaticView)
END_MESSAGE_MAP()


// CDlgToolMeas メッセージ ハンドラー


BOOL CDlgToolMeas::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgToolMeas in ***");

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
void CDlgToolMeas::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成

	//基準線オーバーレイ
	m_Ovlay_Baseline = m_EvView.m_display.CreateOverlay();	//基準線 オーバーレイの生成
	m_Ovlay_Baseline->SetScaling(true);							//基準線 伸縮有無
	m_Ovlay_Baseline->SetEnable(true);							//基準線 オーバーレイを表示
	//原点(0,0)を通る十字線を生成 ※画像ビューを大きく超える範囲の十字線 例)±2000
	FVCL::GDI::CFvGdiLineSegment x_seg_Baseline = FVCL::Data::CFvLineSegment(-2000, 0, 2000, 0);	//基準線表示X 線分構造クラス
	FVCL::GDI::CFvGdiLineSegment y_seg_Baseline = FVCL::Data::CFvLineSegment(0, -2000, 0, 2000);	//基準線表示Y 線分構造クラス
	//基準線オーバーレイの色
	x_seg_Baseline.GetPen().SetColor(RGB(0x00, 0xFF, 0x00));	//黄緑色
	y_seg_Baseline.GetPen().SetColor(RGB(0x00, 0xFF, 0x00));	//黄緑色
	//線分を図形コレクションに追加
	m_clust_Baseline.AddFigure(x_seg_Baseline);	//X線
	m_clust_Baseline.AddFigure(y_seg_Baseline);	//Y線

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

	//スコープ選択
	for (int i = 0; i < 16; i++) {
		CString name = sys_scope[i].name;
		combo_set_scope.AddString(name + Tips0.SelStr(language.jpn, " (スコープ", " (Scope") + Tips0.IntToString(i + 1) + ")");
	}

	//視野選択
	combo_set_view.AddString(Tips0.SelChar(language.jpn, "視野1", "View1"));
	combo_set_view.AddString(Tips0.SelChar(language.jpn, "視野2", "View2"));
	combo_set_view.AddString(Tips0.SelChar(language.jpn, "視野3", "View3"));
	combo_set_view.AddString(Tips0.SelChar(language.jpn, "視野4", "View4"));

	//スライダーコントロール
	slider_magn.SetRange(0, 10);	//表示倍率

	//計測表示オーバーレイ
	m_Ovlay = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成
	m_Ovlay->SetScaling(true);						//伸縮有無
	m_Ovlay->SetEnable(false);						//オーバーレイを隠す
	m_GdiLine.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));		//直線 赤色
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::ChangeLanguage()
{
	POINT pt[2];
	RECT re[2];
	int width[2], height[2];

	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1701));
		check_calib_line.SetWindowTextA(language.GetCharacterString(1705));		//キャリブレーション線表示チェックボックス
		static_scope.SetWindowTextA(language.GetCharacterString(1702));			//スコープ
		static_view.SetWindowTextA(language.GetCharacterString(1703));			//視野
		static_view_the_line.SetWindowTextA(language.GetCharacterString(1707));	//濃度波形表示
		static_dis_angle.SetWindowTextA(language.GetCharacterString(1709));		//2点間距離傾き
		btn_line_profile.SetWindowTextA(language.GetCharacterString(1706));		//濃度波形表示ボタン
		btn_distance.SetWindowTextA(language.GetCharacterString(1708));			//2点計測ボタン
		btn_full.SetWindowTextA(language.GetCharacterString(1711));				//全体表示ボタン
		btn_1x.SetWindowTextA(language.GetCharacterString(1712));				//等倍表示ボタン
		btn_fromfile.SetWindowTextA(language.GetCharacterString(1713));			//ファイルからボタン
		btn_1shot.SetWindowTextA(language.GetCharacterString(1714));			//1ショットボタン
		btn_live.SetWindowTextA(language.GetCharacterString(1715));				//ライブボタン
		static_loadimage.SetWindowTextA(language.GetCharacterString(1716));		//画像の読み込み
		btn_exit.SetWindowTextA(language.GetCharacterString(1717));				//閉じるボタン
		btn_clear.SetWindowTextA(language.GetCharacterString(1710));			//表示クリアボタン

		//コントロールサイズ・位置調整
		combo_set_scope.GetWindowRect(&re[0]);	//スコープ選択コンボボックス
		combo_set_view.GetWindowRect(&re[1]);	//視野選択コンボボックス
		for (int i = 0; i < 2; i++) {
			pt[i].x = re[i].left;
			pt[i].y = re[i].top;
			width[i] = re[i].right - re[i].left;
			height[i] = re[i].bottom - re[i].top;
			this->ScreenToClient(&pt[i]);
		}
		combo_set_scope.MoveWindow(pt[0].x - 10, pt[0].y, width[0], height[0], TRUE);	//スコープ選択コンボボックス
		combo_set_view.MoveWindow(pt[1].x - 10, pt[1].y, width[1], height[1], TRUE);	//視野選択コンボボックス
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
void CDlgToolMeas::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//スコープ選択
	combo_set_scope.SetCurSel(select_scope);

	//視野選択
	combo_set_view.SetCurSel(select_view);

	//キャリブレーション線表示
	check_calib_line.SetCheck(TRUE);

	//カメラ選択
	select_cam = sys_scope[select_scope].view[select_view].assign_camera;

	//画像メモリサイズの設定
	SetImageSize();

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());
	double mag = m_EvView.GetMagnification();
	edit_magn.SetWindowTextA(Tips0.DblToString(mag, 2, TRUE));	//倍率表示更新
	SetPosMagn(mag);	//スライダーの移動

	// 画像ビューの表示更新.
	m_EvView.SetImage(&m_Image); //表示する画像の設定

	//カーソルタイプを通常モードに指定
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	DrawSysLayer();		//システムレイヤ描画

	LiveCaptureStart();		//ライブ画像取得開始

	LogOutScopeInfo();		//スコープの情報出力
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
}


/** ****************************************************** **
 ** BMPファイルの選択
 ** 引数：
 **　LPTSTR lpSelectPath
 ** 戻り値：
 **　BOOL：結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgToolMeas::SelectBMPFile(LPTSTR lpSelectPath)
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
 ** 画像の読み込み
 ** 引数：
 **　LPCTSTR filepath
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::LoadFvImage(LPCTSTR filepath)
{
	// TODO: ここに実装コードを追加します.
	FVCL::Data::CFvImage m_Image_buf;		//画像メモリ
	CString title, msg;
	this->GetWindowTextA(title);

	bool rtn = m_Image_buf.Load(filepath);	//画像の読込

	if (!rtn) {
		int err_code = m_Image_buf.GetErrorCode();

		msg = Tips0.IntToString(err_code);
		MessageBoxA(Tips0.SelStr(language.jpn, "ファイル読み込みに失敗しました(", "Failed to read the file(") + msg + ")", title, MB_OK | MB_ICONERROR);
		return;
	}

	//読み込んだ画像のサイズ
	int buf_width = m_Image_buf.GetHorzSize();
	int buf_height = m_Image_buf.GetVertSize();

	//画像サイズの確認
	if (cam_parameter[select_cam].device.grab_width != buf_width ||
		cam_parameter[select_cam].device.grab_height != buf_height) {
		MessageBoxA(Tips0.SelStr(language.jpn,
			"ファイル読み込みに失敗しました(画像サイズ不一致)",
			"Failed to read the file(Image size mismatch)"),
			title, MB_OK | MB_ICONERROR);
		return;
	}

	m_Image = m_Image_buf;
}


/** ****************************************************** **
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgToolMeas::WholeMagnification()
{
	// TODO: ここに実装コードを追加します.
	int image_width = m_Image.GetHorzSize();
	int image_height = m_Image.GetVertSize();
	double mag_x = (double)image_view_size_x / (double)image_width;
	double mag_y = (double)image_view_size_y / (double)image_height;
	double mag_min = min(mag_x, mag_y);
	//if (mag_min > 1) mag_min = 1;		※1倍以上もOKとする

	//表示倍率スライダーの設定
	magn.min = mag_min;
	if (magn.min > 1) magn.min = 1.0;
	magn.interval = (magn.max - magn.min) / 10;	//間隔

	return mag_min;
}


/** ****************************************************** **
 ** 画像メモリのサイズ変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::SetImageSize()
{
	// TODO: ここに実装コードを追加します.

	//カメラ毎に画像サイスを変更する
	//　horz,vertのサイズで真っ黒画像が作成される
	//　カメラが存在しない場合は640×480とする
	UINT horz = 640;
	UINT vert = 480;
	if (cam_parameter[select_cam].device.model_code != 0) {
		horz = cam_parameter[select_cam].device.grab_width;
		vert = cam_parameter[select_cam].device.grab_height;
	}
	m_Image.SetSize(horz, vert, FVCL::ImageType::UC8, 1, 0);	//画像サイズの変更
}


/** ****************************************************** **
 ** スライダーによる倍率指定
 ** 引数：
 **　int pos	スライダーポジション
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::SetSliderMagn(int pos)
{
	// TODO: ここに実装コードを追加します.
	double mag = magn.min + magn.interval * pos;
	if (pos == 0) mag = magn.min;
	if (pos == 10) mag = magn.max;
	m_EvView.SetMagnification(mag);	//倍率指定
	edit_magn.SetWindowTextA(Tips0.DblToString(m_EvView.GetMagnification(), 2, TRUE));
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** スライダーの移動
 ** 引数：
 **　double mag		倍率
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::SetPosMagn(double mag)
{
	// TODO: ここに実装コードを追加します.
	double p_mag = magn.min;
	point_magn = 10;

	for (int i = 0; i <= 10; i++) {
		if (mag <= p_mag) {
			point_magn = i;
			break;
		}
		p_mag += magn.interval;
	}
	slider_magn.SetPos(point_magn);	//初期値0
}


/** ****************************************************** **
 ** ライブ画像取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::LiveImageCapture()
{
	// TODO: ここに実装コードを追加します.
	m_Image = m_ImageBuffer[select_cam];

	//カメラ画像連続取り込みの3回に1回表示を更新する
	draw_tim++;
	if (draw_tim / 3) {
		// 画像ビューの表示更新.
		m_EvView.SetImage(&m_Image);	//表示する画像の設定

		m_EvView.ReDraw(false);		//表示更新
		draw_tim = 0;
	}
}


/** ****************************************************** **
 ** ライブ画像取得開始
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::LiveCaptureStart()
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//ライブ画像取得開始メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_TOL_CAPTURE_ST, 0, (LPARAM)this);
	if (rtn != 0x1042) {
		CString title;
		this->GetWindowTextA(title);
		MessageBoxA(Tips0.SelStr(language.jpn, "Live開始失敗", "Live start failed"), title, MB_OK | MB_ICONERROR);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** ライブ画像取得停止
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::LiveCaptureEnd()
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//ライブ画像取得停止メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_TOL_CAPTURE_ED, 0, (LPARAM)this);
	if (rtn != 0x1043) {
		CString title;
		this->GetWindowTextA(title);
		MessageBoxA(Tips0.SelStr(language.jpn, "Live停止失敗", "Live stop failed"), title, MB_OK | MB_ICONERROR);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** システムレイヤ描画
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::DrawSysLayer()
{
	// TODO: ここに実装コードを追加します.
	ovlay_sys.m_Overlay->ClearFigures();	//カメラと座標
	m_Ovlay_Baseline->ClearFigures();		//基準線

	//座標、キャリブレーション線を描画
	//　システムレイヤパラメータをもとに表示する
	if (check_calib_line.GetCheck()) {

		//カメラ座標表示を描画
		if (sys_layer[select_cam].cord_sign.enable) {
			//カメラ座標表示描画
			DrawSysCord();

			//レイヤを追加
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Line_Cord[0]);
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Line_Cord[1]);
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Achr_Cord[0]);
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Achr_Cord[1]);
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cord[0]);
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cord[1]);
		}

		//カメラ名称表示を描画
		if (sys_layer[select_cam].cam_sign.enable) {
			//カメラ名称表示描画
			DrawSysCam();

			//レイヤを追加
			ovlay_sys.m_Overlay->DrawFigure(ovlay_sys.m_Str_Cam);
		}

		//基準線レイヤの追加
		DrawBaseline();		//基準線表示
		m_Ovlay_Baseline->DrawFigure(m_clust_Baseline);
	}

	m_EvView.ReDraw(true);
}


/** ****************************************************** **
 ** カメラ座標表示描画
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::DrawSysCord()
{
	// TODO: ここに実装コードを追加します.
	int cord_pos = sys_layer[select_cam].cord_sign.position;			//座標表示位置 0:右上, 1:右下, 2:左上, 3:左下
	int coordinate_type = cam_parameter[select_cam].coordinate_type;	//カメラ構成(0～7)

	int offset_x = 12;
	int offset_y = 12;
	int line_length = 60;

	int base_x = 0;	//基準点X
	int base_y = 0;	//基準点Y
	switch (cord_pos) {
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
	int cam_pos = sys_layer[select_cam].cam_sign.position;		//カメラ表示位置 0:右上, 1:右下, 2:左上, 3:左下
	BOOL cam_enable = sys_layer[select_cam].cam_sign.enable;	//カメラ表示有無
	if ((cord_pos == cam_pos) && cam_enable) {
		switch (cord_pos) {
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

	COLORREF cord_color = sys_layer[select_cam].cord_sign.color;

	//横線
	ovlay_sys.m_Line_Cord[0].st.x = base_x;
	ovlay_sys.m_Line_Cord[0].st.y = base_y;
	ovlay_sys.m_Line_Cord[0].ed.x = base_x + line_length;
	ovlay_sys.m_Line_Cord[0].ed.y = base_y;
	ovlay_sys.m_Line_Cord[0].SetAxis(FVCL::Data::CFvPoint(0, 0));
	ovlay_sys.m_Line_Cord[0].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Line_Cord[0].GetPen().SetColor(cord_color);	//座標方向線 色指定

	//縦線
	ovlay_sys.m_Line_Cord[1].st.x = base_x;
	ovlay_sys.m_Line_Cord[1].st.y = base_y;
	ovlay_sys.m_Line_Cord[1].ed.x = base_x;
	ovlay_sys.m_Line_Cord[1].ed.y = base_y + line_length;
	ovlay_sys.m_Line_Cord[1].SetAxis(FVCL::Data::CFvPoint(0, 0));
	ovlay_sys.m_Line_Cord[1].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Line_Cord[1].GetPen().SetColor(cord_color);	//座標方向線 色指定

	//横線矢印
	ovlay_sys.m_Achr_Cord[0].st.x = ovlay_sys.m_Line_Cord[0].ed.x;
	ovlay_sys.m_Achr_Cord[0].st.y = ovlay_sys.m_Line_Cord[0].ed.y;
	ovlay_sys.m_Achr_Cord[0].ed.x = ovlay_sys.m_Line_Cord[0].st.x;
	ovlay_sys.m_Achr_Cord[0].ed.y = ovlay_sys.m_Line_Cord[0].st.y;
	ovlay_sys.m_Achr_Cord[0].SetAxis(FVCL::Data::CFvPoint(0 - line_length, 0));
	ovlay_sys.m_Achr_Cord[0].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Achr_Cord[0].GetPen().SetColor(cord_color);	//座標方向矢印 色指定

	//縦線矢印
	ovlay_sys.m_Achr_Cord[1].st.x = ovlay_sys.m_Line_Cord[1].ed.x;
	ovlay_sys.m_Achr_Cord[1].st.y = ovlay_sys.m_Line_Cord[1].ed.y;
	ovlay_sys.m_Achr_Cord[1].ed.x = ovlay_sys.m_Line_Cord[1].st.x;
	ovlay_sys.m_Achr_Cord[1].ed.y = ovlay_sys.m_Line_Cord[1].st.y;
	ovlay_sys.m_Achr_Cord[1].SetAxis(FVCL::Data::CFvPoint(0, 0 - line_length));
	ovlay_sys.m_Achr_Cord[1].SetAngle(angle);		//回転角を設定
	ovlay_sys.m_Achr_Cord[1].GetPen().SetColor(cord_color);	//座標方向矢印 色指定

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
	ovlay_sys.m_Str_Cord[0].SetColor(cord_color);	//座標文字 色指定
	ovlay_sys.m_Str_Cord[1].SetColor(cord_color);	//座標文字 色指定
}


/** ****************************************************** **
 ** カメラ名称表示描画
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::DrawSysCam()
{
	// TODO: ここに実装コードを追加します.
	int cam_pos = sys_layer[select_cam].cam_sign.position;		//カメラ表示位置 0:右上, 1:右下, 2:左上, 3:左下

	int offset_x = 3;
	int offset_y = 3;

	int base_x = 0;	//基準点X
	int base_y = 0;	//基準点Y
	switch (cam_pos) {
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
	COLORREF cam_color = sys_layer[select_cam].cam_sign.color;
	ovlay_sys.m_Str_Cam.SetColor(cam_color);		//名称文字 色指定
	ovlay_sys.m_Str_Cam.SetText(cam_parameter[select_cam].device.nick_name);
}


/** ****************************************************** **
 ** 基準線表示
 ** 引数：
 **　int scope		スコープ
 **　int view		視野
 **　int draw_type	描画タイプ 1:画像中心　2:キャリブレーション中心
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::DrawBaseline()
{
	// TODO: ここに実装コードを追加します.

	//キャリブレーション中心に基準線を引く
	if (scope_param[select_scope].view[select_view].calib.pixel_complete &&
		scope_param[select_scope].view[select_view].calib.pixel_size != 0) {

		//キャリブレーション中心の取得
		double cal_base_x = scope_param[select_scope].view[select_view].calib.pixel.cal_base_x;
		double cal_base_y = scope_param[select_scope].view[select_view].calib.pixel.cal_base_y;

		//キャリブレーション角度のdegree変換
		double crdnt_degree = Tips0.RadianToDeg(scope_param[select_scope].view[select_view].calib.crdnt_radian);

		//カメラ座標構成に合わせ角度を変更する
		int type = scope_param[select_scope].view[select_view].coordinate_type;
		double line_angle = crdnt_degree;
		if (type >= 4 && type <= 7) line_angle = crdnt_degree * -1;

		//十字線の配置
		m_clust_Baseline.SetPosition(FVCL::Data::CFvPoint(cal_base_x, cal_base_y));
		m_clust_Baseline.SetAngle(line_angle);
	}
	//画像中心に基準線を引く
	else {

		//十字線の配置
		int image_height = m_Image.GetVertSize();
		int image_width = m_Image.GetHorzSize();
		m_clust_Baseline.SetPosition(FVCL::Data::CFvPoint(image_width / 2, image_height / 2));
		m_clust_Baseline.SetAngle(0);
	}
}


/** ****************************************************** **
 ** 2点計測
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::TwoPointMeas()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//2点の座標取得
	double mark_x[2];
	double mark_y[2];
	mark_x[0] = m_GdiLine.st.x;
	mark_y[0] = m_GdiLine.st.y;
	mark_x[1] = m_GdiLine.ed.x;
	mark_y[1] = m_GdiLine.ed.y;
	
	//ポジション座標変換
	double crdnt_x[2];
	double crdnt_y[2];
	for (int i = 0; i < 2; i++) {
		double cal_base_x = 0;
		double cal_base_y = 0;
		int coordinate_type = scope_param[select_scope].view[select_view].coordinate_type;
		if (ConvDirection(coordinate_type, cal_base_x, cal_base_y, mark_x[i], mark_y[i], &crdnt_x[i], &crdnt_y[i]) != 0) goto END;
	}

	//1点目,2点目の座標確認
	//　2点目のX座標が1点目より小さい場合は1点目と2点目を入れ替える
	if (crdnt_x[0] > crdnt_x[1]) {
		double tempolary = crdnt_x[0];
		crdnt_x[0] = crdnt_x[1];
		crdnt_x[1] = tempolary;
	}

	//メカ座標変換
	double mech_x[2];
	double mech_y[2];
	double crdnt_radian = scope_param[select_scope].view[select_view].calib.crdnt_radian;
	for (int i = 0; i < 2; i++) {
		if (ConvMechCoord(crdnt_x[i], crdnt_y[i], crdnt_radian, &mech_x[i], &mech_y[i]) != 0) goto END;
	}

	double delta_t_rad;
	double delta_x;
	double delta_y;
	double length;
	if (TwoPointCalc(mech_x, mech_y, &delta_t_rad, &delta_x, &delta_y, &length) != 0) goto END;
	double delta_t_deg = Tips0.RadianToDeg(delta_t_rad);	//degreeに変換

	//pixelからmmに変換
	double delta_x_mm;
	double delta_y_mm;
	double length_mm;
	BOOL pixel_complete = scope_param[select_scope].view[select_view].calib.pixel_complete;
	if (pixel_complete) {
		double pixel_size = scope_param[select_scope].view[select_view].calib.pixel_size;
		delta_x_mm = delta_x * pixel_size;
		delta_y_mm = delta_y * pixel_size;
		length_mm = length * pixel_size;
	}

	//ログ出力
	LogOut("");
	LogOut(Tips0.SelStr(language.jpn, "2点間計測", "2 Point Distance"));

	if (pixel_complete) {
		LogOut("⊿X：" + Tips0.DblToString(delta_x_mm, 4, TRUE) + "mm (" + Tips0.DblToString(delta_x, 2, TRUE) + "pix)");
		LogOut("⊿Y：" + Tips0.DblToString(delta_y_mm, 4, TRUE) + "mm (" + Tips0.DblToString(delta_y, 2, TRUE) + "pix)");
		LogOut("⊿T：" + Tips0.DblToString(delta_t_deg, 4, TRUE) + "deg");
		LogOut("⊿L：" + Tips0.DblToString(length_mm, 4, TRUE) + "mm (" + Tips0.DblToString(length, 2, TRUE) + "pix)");
	}
	else {
		LogOut("⊿X：" + Tips0.DblToString(delta_x, 2, TRUE) + "pix");
		LogOut("⊿Y：" + Tips0.DblToString(delta_y, 2, TRUE) + "pix");
		LogOut("⊿T：" + Tips0.DblToString(delta_t_deg, 4, TRUE) + "deg");
		LogOut("⊿L：" + Tips0.DblToString(length, 2, TRUE) + "pix");
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** カメラ姿勢へ座標変換
 ** 引数：
 **　int coordinate_type	視野座標の構成タイプ
 **　double cal_base_x		Cal基準X
 **　double cal_base_y		Cal基準Y
 **　double mark_x			検出位置X
 **　double mark_y			検出位置Y
 **　double* crdnt_x		座標変換X
 **　double* crdnt_y		座標変換Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::ConvDirection(int coordinate_type, double cal_base_x, double cal_base_y, double mark_x, double mark_y, double* crdnt_x, double* crdnt_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	if (coordinate_type < 0 || coordinate_type > 7) goto END;

	switch (coordinate_type) {
		//構成1
	case 0:
		*crdnt_x = mark_x - cal_base_x;
		*crdnt_y = mark_y - cal_base_y;
		break;
		//構成2
	case 1:
		*crdnt_x = mark_y - cal_base_y;
		*crdnt_y = (mark_x - cal_base_x) * -1;
		break;
		//構成3
	case 2:
		*crdnt_x = (mark_x - cal_base_x) * -1;
		*crdnt_y = (mark_y - cal_base_y) * -1;
		break;
		//構成4
	case 3:
		*crdnt_x = (mark_y - cal_base_y) * -1;
		*crdnt_y = mark_x - cal_base_x;
		break;
		//構成5
	case 4:
		*crdnt_x = (mark_x - cal_base_x) * -1;
		*crdnt_y = mark_y - cal_base_y;
		break;
		//構成6
	case 5:
		*crdnt_x = mark_y - cal_base_y;
		*crdnt_y = mark_x - cal_base_x;
		break;
		//構成7
	case 6:
		*crdnt_x = mark_x - cal_base_x;
		*crdnt_y = (mark_y - cal_base_y) * -1;
		break;
		//構成8
	case 7:
		*crdnt_x = (mark_y - cal_base_y) * -1;
		*crdnt_y = (mark_x - cal_base_x) * -1;
		break;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** メカ座標変換
 ** 引数：
 **　double crdnt_x			座標変換X
 **　double crdnt_y			座標変換Y
 **　double crdnt_radian	座標系傾き
 **　double* mech_x			メカ座標X
 **　double* mech_y			メカ座標Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::ConvMechCoord(double crdnt_x, double crdnt_y, double crdnt_radian, double* mech_x, double* mech_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	double a = crdnt_x / cos(crdnt_radian);
	double i = a * sin(crdnt_radian);
	double u = crdnt_y - i;
	double e = u * sin(crdnt_radian);
	double x = a + e;
	double ans_mech_x = x;

	double o = u * cos(crdnt_radian);
	double y = o;
	double ans_mech_y = y;

	*mech_x = ans_mech_x;
	*mech_y = ans_mech_y;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 2点間算出
 ** 引数：
 **　double* mech_x			メカ座標X配列ポインタ
 **　double* mech_y			メカ座標Y配列ポインタ
 **　double* delta_t_rad	直線傾き
 **　double* delta_x		X成分(⊿X)
 **　double* delta_y		Y成分(⊿Y)
 **　double* length			2点間距離
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgToolMeas::TwoPointCalc(double* mech_x, double* mech_y, double* delta_t_rad, double* delta_x, double* delta_y, double* length)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	double pi = acos(double(-1));

	//傾き算出
	double base_theta = 0;
	//FVXでは視野1,2でX方向の変化がない場合、90度足していた
	//　WILでは下記に変更
	//　　・Y方向も変化がない場合 → 0
	//　　・Y方向視野1より視野2のY位置が大きい場合 → -(pi / 2) -90度
	//　　・Y方向視野1より視野2のY位置が小さい場合 → +(pi / 2) +90度
	//また視野1より視野2のX位置が小さい場合、180度足していた
	//　→ WILでは検討要
	if ((mech_x[1] - mech_x[0]) == 0) {
		//base_theta = pi / 2;
		if ((mech_y[1] - mech_y[0]) == 0) {
			base_theta = 0;
		}
		else if ((mech_y[1] - mech_y[0]) > 0) {
			base_theta = -(pi / 2);
		}
		else if ((mech_y[1] - mech_y[0]) < 0) {
			base_theta = pi / 2;
		}
	}
	else if ((mech_x[1] - mech_x[0]) > 0) {
		base_theta = atan((mech_y[1] - mech_y[0]) / (mech_x[1] - mech_x[0]));
	}
	else if ((mech_x[1] - mech_x[0]) < 0) {
		base_theta = atan((mech_y[1] - mech_y[0]) / (mech_x[1] - mech_x[0]));
		base_theta = pi + base_theta;
	}

	*delta_t_rad = base_theta;

	//⊿X,⊿Y算出
	*delta_x = fabs(mech_x[1] - mech_x[0]);
	*delta_y = fabs(mech_y[1] - mech_y[0]);

	//2点間距離算出
	*length = sqrt(pow(mech_x[1] - mech_x[0], 2.0) + pow(mech_y[1] - mech_y[0], 2.0));

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** スコープの情報出力
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::LogOutScopeInfo()
{
	// TODO: ここに実装コードを追加します.
	BOOL pixel_complete = scope_param[select_scope].view[select_view].calib.pixel_complete;
	double pixel_um = scope_param[select_scope].view[select_view].calib.pixel_size * (double)1000;
	double deg = Tips0.RadianToDeg(scope_param[select_scope].view[select_view].calib.crdnt_radian);

	if (ope_log.GetCount() > 0) LogOut("");

	CString scope_name = sys_scope[select_scope].name;
	if (strlen(scope_name) == 0) {
		scope_name = Tips0.SelStr(language.jpn, "スコープ", "Scope") + Tips0.IntToString(select_scope + 1);
	} 
	CString view_name;
	view_name = Tips0.SelStr(language.jpn, "視野", "View") + Tips0.IntToString(select_view + 1);
	LogOut(scope_name + ", " + view_name);

	if (pixel_complete) {
		LogOut(Tips0.SelStr(language.jpn, "画素サイズ：", "Pixel size：") + Tips0.DblToString(pixel_um, 2, TRUE) + "um");
		LogOut(Tips0.SelStr(language.jpn, "視野傾き：", "Field of view tilt：") + Tips0.DblToString(deg, 4, TRUE) + "deg");
	}
	else {
		LogOut(Tips0.SelStr(language.jpn, "キャリブレーションデータがありません", "No calibration data"));
	}
}


/** ****************************************************** **
 ** 操作ログ出力
 ** 引数：
 **　CString msg	出力メッセージ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::LogOut(CString msg)
{
	// TODO: ここに実装コードを追加します.

	ope_log.AddString(msg);

	//リスト内の表示管理
	int list_count = ope_log.GetCount();	//表示行数
	int start_index = list_count - 22;
	if (start_index > 0) ope_log.SetTopIndex(start_index);
}


/** ****************************************************** **
 ** 全体表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(WholeMagnification());	//倍率指定

	double mag = m_EvView.GetMagnification();
	edit_magn.SetWindowTextA(Tips0.DblToString(mag, 2, TRUE));	//倍率表示更新
	SetPosMagn(mag);	//スライダーの移動

	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定

	double mag = m_EvView.GetMagnification();
	edit_magn.SetWindowTextA(Tips0.DblToString(mag, 2, TRUE));	//倍率表示更新
	SetPosMagn(mag);	//スライダーの移動

	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ライブ画像連続ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	LiveCaptureStart();		//ライブ画像取得開始
}


/** ****************************************************** **
 ** ライブ画像1ショットボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	FVCL::Data::CFvImage m_Image_buf;		//画像メモリ

	//ライブ画像取得メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_GET_IMAGE_CHNL, select_cam, (LPARAM)&m_Image_buf);
	if (rtn != 0x1009) {
		outlog.LogOutFile(1, "コマンドメッセージ:CDlgToolMeasのライブ画像取得失敗");
		return;
	}

	m_Image = m_Image_buf;

	// 画像ビューの表示更新.
	m_EvView.SetImage(&m_Image); //表示する画像の設定
	m_EvView.ReDraw(false);		//表示更新
}


/** ****************************************************** **
 ** ファイルからボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	char path[MAX_PATH] = { '\0' };

	BOOL rtn_result = SelectBMPFile(path);	//BMPファイルの選択

	if (rtn_result) {
		//画像ファイルの読み込み
		LoadFvImage(path);

		// 画像ビューの表示更新.
		m_EvView.SetImage(&m_Image);	//表示する画像の設定
	}

	m_EvView.ReDraw(false);		//表示更新
}


/** ****************************************************** **
 ** 濃度波形表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//カーソルタイプを濃度波形表示に変更
	m_EvView.SetCursorType(FVCL::CursorType::ASCOPE);

	// Viewのグリップをオン
	m_EvView.SetGripOption(FVCL::GripOption::ENABLE);

	m_Ovlay->SetActive(false);	//マウス操作の無効化

	//オーバーレイを隠す
	m_Ovlay->SetEnable(false);

	m_EvView.Update(true);
}


/** ****************************************************** **
 ** 表示クリアボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton7()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//カーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// Viewのグリップをオン
	m_EvView.SetGripOption(FVCL::GripOption::ENABLE);

	m_Ovlay->SetActive(false);	//マウス操作の無効化

	//オーバーレイを隠す
	m_Ovlay->SetEnable(false);

	m_EvView.Update(true);
}


/** ****************************************************** **
 ** 2点計測ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedButton8()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//カーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// Viewのグリップをオフ
	m_EvView.SetGripOption(FVCL::GripOption::DISABLE);

	m_Ovlay->SetActive(true);	//マウス操作の有効化

	m_Ovlay->ClearFigures();
	m_GdiLine.SetSelect(false);		//図形の選択状態無効化

	m_EvView.ReDraw(false);			//表示更新

	line_click = 0;	//クリック回数リセット
}


/** ****************************************************** **
 ** 閉じるボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	outlog.LogOutFile(1, "*** DlgToolMeas out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** 水平スクロール操作
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	//表示倍率 スライダーコントロール
	if (*pScrollBar == slider_magn) {
		int curpos = slider_magn.GetPos();
		switch (nSBCode) {
		case SB_PAGELEFT:
			point_magn = curpos;
			break;
		case SB_PAGERIGHT:
			point_magn = curpos;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			point_magn = (int)nPos;
			break;
		}

		if (point_magn >= 0 && point_magn <= 10) SetSliderMagn(point_magn);	//スライダーによる倍率指定
	}

	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


/** ****************************************************** **
 ** スコープ選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	select_scope = combo_set_scope.GetCurSel();

	select_cam = sys_scope[select_scope].view[select_view].assign_camera;	//カメラ選択

	SetImageSize();		//画像メモリサイズの変更

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());
	double mag = m_EvView.GetMagnification();
	edit_magn.SetWindowTextA(Tips0.DblToString(mag, 2, TRUE));	//倍率表示更新
	SetPosMagn(mag);	//スライダーの移動

	//カーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// Viewのグリップをオン
	m_EvView.SetGripOption(FVCL::GripOption::ENABLE);

	m_Ovlay->SetActive(false);	//マウス操作の無効化

	//オーバーレイを隠す
	m_Ovlay->SetEnable(false);

	m_EvView.SetImage(&m_Image);
	m_EvView.ReDraw(false);		//表示更新

	DrawSysLayer();		//システムレイヤ描画

	LiveCaptureStart();		//ライブ画像取得開始

	LogOutScopeInfo();		//スコープの情報出力
}


/** ****************************************************** **
 ** 視野選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnCbnSelchangeCombo2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	select_view = combo_set_view.GetCurSel();

	select_cam = sys_scope[select_scope].view[select_view].assign_camera;	//カメラ選択

	SetImageSize();		//画像メモリサイズの変更

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());
	double mag = m_EvView.GetMagnification();
	edit_magn.SetWindowTextA(Tips0.DblToString(mag, 2, TRUE));	//倍率表示更新
	SetPosMagn(mag);	//スライダーの移動

	//カーソルタイプを通常モードに変更
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	// Viewのグリップをオン
	m_EvView.SetGripOption(FVCL::GripOption::ENABLE);

	m_Ovlay->SetActive(false);	//マウス操作の無効化

	//オーバーレイを隠す
	m_Ovlay->SetEnable(false);

	m_EvView.SetImage(&m_Image);
	m_EvView.ReDraw(false);		//表示更新

	DrawSysLayer();		//システムレイヤ描画

	LiveCaptureStart();		//ライブ画像取得開始

	LogOutScopeInfo();		//スコープの情報出力
}


/** ****************************************************** **
 ** キャリブレーション線表示チェックボックスClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgToolMeas::OnBnClickedCheck1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	DrawSysLayer();		//システムレイヤ描画
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
void CDlgToolMeas::ICommandPre(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolMeas*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolMeas::ICommandPost(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolMeas*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolMeas::IWndMsgPre(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolMeas*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgToolMeas::IWndMsgPost(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgToolMeas*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgToolMeas::OnCommandPre(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgToolMeas::OnCommandPost(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
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
void CDlgToolMeas::OnWndMsgPre(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
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
void CDlgToolMeas::OnWndMsgPost(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);

		switch (message)
		{
		case WM_PAINT:			// 再描画イベント.
			break;
		case WM_MOUSEMOVE:
			if (false == m_Ovlay->GetActive()) break;

			if (line_click == 1) {
				m_Ovlay->ClearFigures();

				m_GdiLine.ed.x = pos.x;
				m_GdiLine.ed.y = pos.y;

				m_Ovlay->DrawFigure(m_GdiLine);
				m_EvView.ReDraw(false);		//表示更新
			}
			break;
		case WM_LBUTTONDOWN:
			if (false == m_Ovlay->GetActive()) break;

			if (line_click == 0) {
				m_Ovlay->ClearFigures();
				m_GdiLine.SetSelect(true);	//図形の選択状態有効化

				m_GdiLine.st.x = pos.x;
				m_GdiLine.st.y = pos.y;
				m_GdiLine.ed.x = pos.x;
				m_GdiLine.ed.y = pos.y;

				m_Ovlay->DrawFigure(m_GdiLine);
				m_EvView.ReDraw(false);		//表示更新
				m_Ovlay->SetEnable(true);	//オーバーレイを表示
				line_click++;
			}
			else if (line_click == 1) {
				m_Ovlay->ClearFigures();
				m_GdiLine.SetSelect(false);	//図形の選択状態無効化

				m_GdiLine.ed.x = pos.x;
				m_GdiLine.ed.y = pos.y;

				m_Ovlay->DrawFigure(m_GdiLine);
				m_EvView.ReDraw(false);		//表示更新
				m_Ovlay->SetActive(false);	//マウス操作の無効化
				line_click++;

				// Viewのグリップをオン
				m_EvView.SetGripOption(FVCL::GripOption::ENABLE);

				TwoPointMeas();		//2点間計測
			}
			break;
		case WM_LBUTTONUP:
			break;
		}
	}
}


/** ****************************************************** **
 ** ウィンドウメッセージ処理
 ** 引数：
 **　UINT message	メッセージの識別子
 **　WPARAM wParam	メッセージの最初のパラメータ
 **　LPARAM lParam	メッセージの2番目のパラメータ
 ** 戻り値：
 **　LRESULT　処理の結果
 ** 特記事項：
 ** ****************************************************** **/
LRESULT CDlgToolMeas::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	switch (message) {

	case WM_APP_COMMAND_TOL_IMG_CAPTURE:
		LiveImageCapture();
		break;
	}

	return CDialogEx::DefWindowProc(message, wParam, lParam);
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgToolMeas::PreTranslateMessage(MSG* pMsg)
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


void CDlgToolMeas::OnStnClickedStaticView()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}
