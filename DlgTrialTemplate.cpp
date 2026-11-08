/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgTrialTemplate.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.09.02	M.Iwasaki	検出位置十字サイズ可変対応
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2022.10.04	M.Iwasaki	K34004(ファーストボード対応)
 **			2023.11.08	M.Iwasaki	10.15.1.1	画像合成機能追加
 **			2024.07.16	M.Iwasaki	10.18.4.6	ライブ画像取得処理改善
 ** 注記：
 ** 説明：テスト実行ダイアログ
 **		テンプレートの探索、検査をテスト実行する
 **/

// DlgTrialTemplate.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgTrialTemplate.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"

#include "ProcSearch.h"			//マーク探索クラス
#include "AddEdge.h"			//マーク探索処理の追加機能(エッジ演算)
#include "AddBlob.h"			//マーク探索処理の追加機能(2値化重心演算)
#include "CalcMark.h"			//サーチ結果取得
#include "CalcMes.h"			//計測モード演算
#include "CalcBcrCtrl.h"		//バーコードリーダー演算制御
#include "ProcCopyImage.h"		//画像コピー処理クラス
#include "ExcParamAlignBlob.h"		//AlignBlobパラメータ変換
#include "ExcParamAlignEdge.h"		//AlignEdgeパラメータ変換
#include "ExcParamAlignMeas.h"		//AlignMeasパラメータ変換
#include "ExcParamAlignBarcode.h"	//AlignBarcodeパラメータ変換
#include "ExcParamAlignMultiSrh.h"	//AlignMultiSrhパラメータ変換
#include "ExcParamAlignDisEdge.h"	//AlignDisEdgeパラメータ変換

extern CProcSearch*		proc_search;	//マーク探索プログラム
extern CCalcMes			calc_mes;		//計測モード演算
extern CCalcBcrCtrl		calc_bcr_ctrl;	//バーコードリーダー演算制御
extern CAddEdge			search_edge;	//マーク探索処理の追加機能(エッジ演算)
extern CAddBlob			search_blob;	//マーク探索処理の追加機能(2値化重心演算)
extern CCalcMark		calc_mark;		//サーチ結果取得
extern CProcCopyImage*	proc_cpyimg;	//画像コピー処理クラス

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CScopes			sys_scope[16];			//システムスコープパラメータ
extern CSysOption		sys_option;				//システムオプションパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ
extern CCameraSettings	cam_parameter[8];		//カメラパラメータ
extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;				//言語設定用

extern char* APP_ROOT;
extern FVCL::Data::CFvImage m_ImageBuffer[8];	//画像メモリ

//DLL
extern AlignBlob::Proc::CCalcBlob	calc_blob;		//ブロブ演算
extern AlignEdge::Proc::CCalcEdge	calc_edge;		//エッジ演算
extern AlignMeas::Proc::CCalcMeas	calc_meas;		//濃度演算
extern AlignMultiSrh::Proc::CCalcMultiSrh	calc_multisrh;	//マルチ探索演算
extern AlignDisEdge::Proc::CCalcDisEdge		calc_disedge;	//基準-エッジ距離計測演算
extern AlignDisEdgeToEdge::Proc::CCalcDisEdgeToEdge	calc_disetoe;	//エッジ-エッジ距離計測演算
extern AlignBarcode::Proc::CCalcBarcode		calc_bcr;		//バーコードリーダー演算
extern AlignBarcode::Param::CBarcodeParam	bcr_param[256];	//機種毎バーコードリーダーパラメータ


// CDlgTrialTemplate ダイアログ

IMPLEMENT_DYNAMIC(CDlgTrialTemplate, CDialogEx)

CDlgTrialTemplate::CDlgTrialTemplate(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgTrialTemplate, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 768;
	image_view_size_y = 576;

	status = RSLT_NR;

	flag_ReDraw = FALSE;
}

CDlgTrialTemplate::~CDlgTrialTemplate()
{
}

void CDlgTrialTemplate::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_STATIC_RESULT, result_icon.text);
	DDX_Control(pDX, IDC_LIST1, trial_log);
	DDX_Control(pDX, IDC_STATIC_EXECTION, static_exection);
	DDX_Control(pDX, IDC_STATIC_LOAD_IMAGE, static_load_image);
	DDX_Control(pDX, IDC_BUTTON7, btn_testing);
	DDX_Control(pDX, IDC_BUTTON1, btn_full);
	DDX_Control(pDX, IDC_BUTTON2, btn_1x);
	DDX_Control(pDX, IDC_BUTTON3, btn_2x);
	DDX_Control(pDX, IDC_BUTTON6, btn_fromfile);
	DDX_Control(pDX, IDC_BUTTON5, btn_live);
	DDX_Control(pDX, IDC_BUTTON4, btn_base_image);
	DDX_Control(pDX, IDOK, btn_exit);
}


BEGIN_MESSAGE_MAP(CDlgTrialTemplate, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgTrialTemplate::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgTrialTemplate::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgTrialTemplate::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgTrialTemplate::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgTrialTemplate::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CDlgTrialTemplate::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON6, &CDlgTrialTemplate::OnBnClickedButton6)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDC_BUTTON7, &CDlgTrialTemplate::OnBnClickedButton7)
	ON_WM_CTLCOLOR()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CDlgTrialTemplate メッセージ ハンドラー


BOOL CDlgTrialTemplate::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgTrialTemplate in ***");

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

	UpdateRsltInfo(status);		//実行結果表示の更新

	//ライブ画像取得時のReDrawタイマ開始(33ms周期で更新)
	if (SetTimer(TIM_REDRAW, 33, NULL) == 0) {
		//失敗した場合
		outlog.LogOutFile(1, "CDlgTrialTemplate:ReDrawTimer()失敗");
	}

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
void CDlgTrialTemplate::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成

	OverlayInitSrch();	//WILオーバーレイ準備 探索

	OverlayInitAdd();		//WILオーバーレイ準備 追加機能 オーバーレイ
	OverlayInitAddEdge();	//WILオーバーレイ準備 追加機能 エッジサーチ
	OverlayInitAddBlob();	//WILオーバーレイ準備 追加機能 2値化重心サーチ

	OverlayInitShet();	//WILオーバーレイ準備 検査シート
	OverlayInitBcr();	//WILオーバーレイ準備 バーコードリーダー

	//テスト実行結果アイコン表示
	result_icon.font.CreatePointFont(384, "MS UI Gothic");	//フォント指定
	result_icon.text.SetFont(&result_icon.font);
	result_icon.brush[0].CreateSolidBrush(RGB(224, 224, 224));	//コントロール背景:薄灰
	result_icon.brush[1].CreateSolidBrush(RGB(0, 224, 64));		//コントロール背景:緑
	result_icon.brush[2].CreateSolidBrush(RGB(255, 128, 128));	//コントロール背景:薄赤
	result_icon.brush[3].CreateSolidBrush(RGB(255, 0, 0));		//コントロール背景:赤
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2601));
		static_exection.SetWindowTextA(language.GetCharacterString(2611));		//探索/検査実行
		static_load_image.SetWindowTextA(language.GetCharacterString(2609));	//画像の読み込み
		btn_testing.SetWindowTextA(language.GetCharacterString(2602));			//テスト実行ボタン
		btn_full.SetWindowTextA(language.GetCharacterString(2603));				//全体表示ボタン
		btn_1x.SetWindowTextA(language.GetCharacterString(2605));				//等倍表示ボタン
		btn_2x.SetWindowTextA(language.GetCharacterString(2604));				//x2ボタン
		btn_fromfile.SetWindowTextA(language.GetCharacterString(2606));			//ファイルからボタン
		btn_live.SetWindowTextA(language.GetCharacterString(2607));				//ライブボタン
		btn_base_image.SetWindowTextA(language.GetCharacterString(2608));		//元画像ボタン
		btn_exit.SetWindowTextA(language.GetCharacterString(2610));				//閉じるボタン
	}
}


/** ****************************************************** **
 ** WILオーバーレイ準備 探索
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitSrch()
{
	// TODO: ここに実装コードを追加します.

	//サーチ範囲オーバーレイ
	ovlay_srch.m_Ovlay_Area = m_EvView.m_display.CreateOverlay();			//サーチ範囲 オーバーレイの生成
	ovlay_srch.m_Ovlay_Area->SetScaling(true);								//サーチ範囲 伸縮有無
	//サーチ範囲オーバーレイの色
	ovlay_srch.m_Rect_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));		//黄色
	//サーチ範囲FPM2サーチの特徴量表示
	ovlay_srch.m_Point_FPM2Edge.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));	//エッジ位置 赤色
	ovlay_srch.m_Point_FPM2Edge.SetStyle(FVCL::GDI::FigureStyle::None);		//エッジ位置 スタイル(なし)
	ovlay_srch.m_Point_FPM2Edge.SetSize(1, 1);								//エッジ位置 サイズ

	//探索結果オーバーレイ
	ovlay_srch.m_Ovlay_Rslt = m_EvView.m_display.CreateOverlay();			//探索実行結果 オーバーレイの生成
	ovlay_srch.m_Ovlay_Rslt->SetScaling(true);								//探索実行結果 伸縮有無
	ovlay_srch.m_Ovlay_Rslt->SetEnable(false);								//探索実行結果 オーバーレイを隠す
	//探索結果オーバーレイの色
	ovlay_srch.m_Rect_Rslt.GetPen().SetColor(RGB(0x00, 0x00, 0xFF));		//マーク登録範囲 青色
	ovlay_srch.m_Point_Rslt.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));		//検出位置 赤色
	ovlay_srch.m_Point_Rslt.SetStyle(FVCL::GDI::FigureStyle::Cross);		//検出位置 スタイル(十字)
	ovlay_srch.m_Point_Rslt.SetSize(16, 16);								//検出位置 サイズ
	ovlay_srch.m_Score_Rslt.SetColor(RGB(0xFF, 0x00, 0x00));				//スコア表示 赤色
	ovlay_srch.m_Score_Rslt.SetBkMode(FVCL::GDI::BkMode::Transparent);		//スコア表示 背景透過
	ovlay_srch.m_Score_Rslt.SetAlign(FVCL::GDI::TextAlign::Right | FVCL::GDI::TextAlign::Bottom);	//スコア表示 位置
}


/** ****************************************************** **
 ** WILオーバーレイ準備 追加機能 オーバーレイ
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitAdd()
{
	// TODO: ここに実装コードを追加します.

	//サーチ範囲表示
	ovlay_add.m_Ovlay_Area = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 追加機能範囲
	ovlay_add.m_Ovlay_Area->SetScaling(true);						//伸縮有無

	//サーチ結果
	ovlay_add.m_Ovlay_Rslt = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 追加機能実行結果
	ovlay_add.m_Ovlay_Rslt->SetScaling(true);						//伸縮有無
	ovlay_add.m_Ovlay_Rslt->SetEnable(false);						//追加機能実行結果オーバーレイを隠す
}


/** ****************************************************** **
 ** WILオーバーレイ準備 追加機能 エッジサーチ
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitAddEdge()
{
	// TODO: ここに実装コードを追加します.

	//エッジdirection 0～1
	for (int i = 0; i < 2; i++) {
		//サーチ範囲表示
		gdi_add_edge[i].m_Rect_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//検査範囲 黄色
		gdi_add_edge[i].m_Achr_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//検出線方向矢印 黄色
		gdi_add_edge[i].m_Achr_Area.SetStyle(FVCL::GDI::FigureStyle::Arrow);	//検出線方向矢印 スタイル(矢印)
		gdi_add_edge[i].m_Achr_Area.SetSize(15, 15);							//検出線方向矢印 サイズ
		gdi_add_edge[i].m_Line_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//検出線方向線 黄色
		gdi_add_edge[i].m_Rect_Judg.GetPen().SetColor(RGB(0x00, 0xFF, 0x00));	//判定範囲(枠) 緑色
		gdi_add_edge[i].m_Rect_Judg.SetFillColor(RGB(0x00, 0xFF, 0x00));		//判定範囲(中) 緑色
		gdi_add_edge[i].m_Rect_Judg.SetFillAlpha(0x0F);							//判定範囲(中) 透明度
		gdi_add_edge[i].m_Rect_Judg.SetFillEnable(true);						//判定範囲(中) 塗りつぶし有効化

		//サーチ結果
		gdi_add_edge[i].m_Point_Rslt.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));	//エッジ位置 赤色
		gdi_add_edge[i].m_Point_Rslt.SetStyle(FVCL::GDI::FigureStyle::None);	//エッジ位置 スタイル(なし)
		gdi_add_edge[i].m_Point_Rslt.SetSize(1, 1);								//エッジ位置 サイズ
		gdi_add_edge[i].m_Line_Rslt.GetPen().SetColor(RGB(0xFF, 0x7F, 0x00));	//検出線方向線 橙色
	}
}


/** ****************************************************** **
 ** WILオーバーレイ準備 追加機能 2値化重心サーチ
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitAddBlob()
{
	// TODO: ここに実装コードを追加します.

	//検査範囲表示
	gdi_add_blob.m_Rect_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//検査範囲 黄色

	//検査結果
	gdi_add_blob.m_Point_Rslt.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));		//重心位置 赤色
	gdi_add_blob.m_Point_Rslt.SetStyle(FVCL::GDI::FigureStyle::Cross);		//重心位置 スタイル(十字)
	gdi_add_blob.m_Point_Rslt.SetSize(16, 16);								//重心位置 サイズ
	gdi_add_blob.m_Area_Rslt.SetColor(RGB(0xFF, 0x00, 0x00));				//面積表示 赤色
	gdi_add_blob.m_Area_Rslt.SetBkMode(FVCL::GDI::BkMode::Transparent);		//面積表示 背景透過
	gdi_add_blob.m_Area_Rslt.SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Top);	//面積表示 位置
}


/** ****************************************************** **
 ** WILオーバーレイ準備 検査シート
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitShet()
{
	// TODO: ここに実装コードを追加します.

	//検査シート0～7
	for (int i = 0; i < 8; i++) {
		//検査範囲表示
		ovlay_shet[i].m_Ovlay_Area = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 検査範囲
		ovlay_shet[i].m_Ovlay_Area->SetScaling(true);						//伸縮有無

		//検査結果
		ovlay_shet[i].m_Ovlay_Rslt = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 検査実行結果
		ovlay_shet[i].m_Ovlay_Rslt->SetScaling(true);						//伸縮有無
		ovlay_shet[i].m_Ovlay_Rslt->SetEnable(false);						//検査結果オーバーレイを隠す

		//画像コピー範囲オーバーレイの色
		gdi_cpyimg[i].m_Rect_Copy.GetPen().SetColor(RGB(0x00, 0xFF, 0x00));		//緑色
		gdi_cpyimg[i].m_Str_Copy.SetColor(RGB(0x00, 0xFF, 0x00));				//文字 緑色
		gdi_cpyimg[i].m_Str_Copy.SetBkMode(FVCL::GDI::BkMode::Transparent);		//文字 背景透過
		gdi_cpyimg[i].m_Str_Copy.SetAlign(FVCL::GDI::TextAlign::Left | FVCL::GDI::TextAlign::Top);	//表示 位置
	}
}


/** ****************************************************** **
 ** WILオーバーレイ準備 バーコードリーダー
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OverlayInitBcr()
{
	// TODO: ここに実装コードを追加します.

	//読取範囲表示
	ovlay_bcr.m_Ovlay_Area = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 読取範囲
	ovlay_bcr.m_Ovlay_Area->SetScaling(true);						//伸縮有無

	//読取結果
	ovlay_bcr.m_Ovlay_Rslt = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 読取実行結果
	ovlay_bcr.m_Ovlay_Rslt->SetScaling(true);						//伸縮有無
	ovlay_bcr.m_Ovlay_Rslt->SetEnable(false);						//読取結果オーバーレイを隠す
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//テンプレートパラメータ
	trial_temp_param = template_param[template_number];

	//検査パラメータ
	trial_mes_param = mes_param[template_number];

	//バーコードリーダーパラメータ
	trial_bcr_param = bcr_param[template_number];

	//スコープパラメータ
	trial_scp_param = scope_param[sys_template[template_number].scope];

	//m_Imageは初期値が640×480なので、カメラ毎に画像サイスを変更する
	//　horz,vertのサイズで真っ黒画像が作成される
	int scope = sys_template[template_number].scope;
	int view = sys_template[template_number].view;
	int load_cam = sys_scope[scope].view[view].assign_camera;
	UINT horz = cam_parameter[load_cam].device.grab_width;
	UINT vert = cam_parameter[load_cam].device.grab_height;
	m_Image.SetSize(horz, vert, FVCL::ImageType::UC8, 1, 0);	//画像サイズの変更

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());

	// 画像ビューの表示更新.
	m_EvView.SetImage(&m_Image); //表示する画像の設定

	//カーソルタイプを通常モードに指定
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);
}


/** ****************************************************** **
 ** 探索実行
 ** 引数：
 **　BOOL* srch_ok	探索結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::GoSearch(BOOL* srch_ok)
{
	// TODO: ここに実装コードを追加します.
	int result_srch = 1;	//結果

	CTempParam* temp_param = &trial_temp_param;

	//サーチ範囲(処理ウィンドウ)の展開
	int st_x = temp_param->base_area.st_x;
	int st_y = temp_param->base_area.st_y;
	int width = temp_param->base_area.width;
	int height = temp_param->base_area.height;
	m_Image.SetWindow(st_x, st_y, width, height);

	//エッジサーチ結果初期化
	for (int i = 0; i < 2; i++) {
		temp_param->add_edge.area_ok[i] = FALSE;		//サーチエリア有効判定
		temp_param->add_edge.edge_ok[i] = FALSE;		//エッジサーチ結果
	}
	//2値化重心サーチ結果初期化
	temp_param->add_blob.area_ok = FALSE;		//サーチエリア有効判定
	temp_param->add_blob.blob_ok = FALSE;		//2値化重心サーチ結果

	//マーク探索を実施
	if (temp_param->additional.add_type == 0x00 || temp_param->additional.base_search_enable) {
		byte matching = temp_param->basic.matching;	//探索手法 0x00:グレイサーチ、0x01:形状サーチ

		//マーク登録済
		if (temp_param->pattern.enable) {
			//グレイサーチ実行
			if (matching == 0x00) result_srch = proc_search->ExecuteSearchGS2(template_number, temp_param, &m_Image, 1, srch_ok);
			//形状サーチ実行
			if (matching == 0x01) result_srch = proc_search->ExecuteSearchFPM2(template_number, temp_param, &m_Image, 1, srch_ok);

			if (result_srch != 0) {
				trial_log.AddString(Tips0.SelStr(language.jpn, "GS2/FPM2サーチ失敗", "GS2/FPM2 Search failure"));	//エラーコード0x9301
				goto SEARCH_END;	//処理失敗の場合は処理中断
			}
		}
		//マーク登録未
		else {
			result_srch = 0;
			trial_log.AddString(Tips0.SelStr(language.jpn, "マーク未登録", "Mark unregistered"));
		}

		if (!(*srch_ok)) {
			trial_log.AddString(Tips0.SelStr(language.jpn, "基準点探索NG(NotFound)", "Reference point search NG(NotFound)"));
			goto SEARCH_END;	//探索NGの場合は処理中断
		}
	}

	//追加機能 エッジサーチを実施
	if (temp_param->additional.add_type == 0x01) {
		int result_edge = search_edge.AddMain(template_number, temp_param, srch_ok, &trial_log);

		//サーチ成功もしくはサーチエリア設定NGの場合
		if (result_edge != 1) {
			if (result_edge == -2 || !(*srch_ok)) trial_log.AddString(Tips0.SelStr(language.jpn, "エッジサーチNG", "Edge search NG"));
			result_srch = 0;
		}
		//サーチ失敗の場合
		else {
			trial_log.AddString(Tips0.SelStr(language.jpn, "エッジサーチ失敗", "Edge search failure"));	//エラーコード0x9302
			result_srch = 1;
		}
		if (result_srch != 0 || !(*srch_ok)) goto SEARCH_END;	//処理失敗もしくは探索NGの場合は処理中断
	}
	//追加機能 2値化重心サーチを実施
	else if (temp_param->additional.add_type == 0x02) {
		int result_blob = search_blob.AddMain(template_number, temp_param, srch_ok, &trial_log);

		//サーチ成功もしくはサーチエリア設定NGの場合
		if (result_blob != 1) {
			if (result_blob == -2 || !(*srch_ok)) trial_log.AddString(Tips0.SelStr(language.jpn, "2値化重心サーチNG", "Gravity search NG"));
			result_srch = 0;
		}
		//サーチ失敗の場合
		else {
			trial_log.AddString(Tips0.SelStr(language.jpn, "2値化重心サーチ失敗", "Gravity search failure"));	//エラーコード0x9303
			result_srch = 1;
		}
		if (result_srch != 0 || !(*srch_ok)) goto SEARCH_END;	//処理失敗もしくは探索NGの場合は処理中断
	}

SEARCH_END:
	return result_srch;
}


/** ****************************************************** **
 ** 検査実行
 ** 引数：
 **　BOOL* mes_ok	検査結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::MesMain(BOOL* mes_ok)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//使用するパラメータ
	CTempParam* temp_param = &trial_temp_param;		//テンプレートパラメータ
	CMesParam* param = &trial_mes_param;			//検査パラメータ
	CScopeParam* scp_param = &trial_scp_param;		//スコープパラメータ

	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野

	//計測モードのデータセット
	if (param->base_search_enable) {
		if (calc_mes.CalcDataSet(temp_param, scp_param, 1) != 0) {
			if (language.jpn) {
				trial_log.AddString("計測モード演算失敗(データセット)");	//エラーコード0x9421
			}
			else {
				trial_log.AddString("Measurement mode");
				trial_log.AddString("Calculation failure(Data set)");
			}
			goto MES_END;
		}
	}

	//計測モードの演算実行
	for (int i = 0; i < 8; i++) {
		//検査機能を確認
		int mes_type = param->sheet[i].mes_type;

		//検査を実行
		switch (mes_type) {
			//無し
		case 0x00:
			mes_ok[i] = TRUE;
			break;
			//ブロブ
		case 0x01:
		{
			FVCL::Blob::CFvBlobResult* blob_result = &temp_param->blob_result[i];
			FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

			CExcParamAlignBlob exc_param;
			AlignBlob::Param::CMesParamBlob dll_param_mes;
			exc_param.SetBlobMes(param, &dll_param_mes, i);

			double base_mark_x = scp_param->view[0].align.mark_x;
			double base_mark_y = scp_param->view[0].align.mark_y;

			double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
			BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
			//キャリブレーション未完了の場合
			if (!pixel_complete) {
				pixel_size = 0;		//画素サイズ0
			}

			//ブロブ検査の演算実行
			if (calc_blob.CalcMain(m_Image, blob_result, &dll_param_mes, base_mark_x, base_mark_y, i, pixel_size, &mes_ok[i], &trial_log) == 1) {
				if (language.jpn) {
					trial_log.AddString("計測モード演算失敗(ブロブ)");	//エラーコード0x9422
				}
				else {
					trial_log.AddString("Measurement mode");
					trial_log.AddString("Calculation failure(Blob)");
				}
				goto MES_END;
			}
		}
			break;
			//エッジ
		case 0x02:
		{
			FVCL::Edge::CFvEdgeResult* edge_result = &temp_param->edge_result[i];
			FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

			CExcParamAlignEdge exc_param;
			AlignEdge::Param::CMesParamEdge dll_param_mes;
			exc_param.SetEdgeMes(param, &dll_param_mes, i);

			double base_mark_x = scp_param->view[0].align.mark_x;
			double base_mark_y = scp_param->view[0].align.mark_y;

			//エッジ検査の演算実行
			if (calc_edge.CalcMain(m_Image, edge_result, &dll_param_mes, base_mark_x, base_mark_y, i, &mes_ok[i], &trial_log) == 1) {
				if (language.jpn) {
					trial_log.AddString("計測モード演算失敗(エッジ)");	//エラーコード0x9423
				}
				else {
					trial_log.AddString("Measurement mode");
					trial_log.AddString("Calculation failure(Edge)");
				}
				goto MES_END;
			}
		}
			break;
			//濃度
		case 0x03:
		{
			double* meas_result = &temp_param->meas_result[i];
			FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

			CExcParamAlignMeas exc_param;
			AlignMeas::Param::CMesParamMeas dll_param_mes;
			exc_param.SetMeasMes(param, &dll_param_mes, i);

			double base_mark_x = scp_param->view[0].align.mark_x;
			double base_mark_y = scp_param->view[0].align.mark_y;

			//濃度検査の演算実行
			if (calc_meas.CalcMain(m_Image, meas_result, &dll_param_mes, base_mark_x, base_mark_y, i, &mes_ok[i], &trial_log) == 1) {
				if (language.jpn) {
					trial_log.AddString("計測モード演算失敗(濃度)");	//エラーコード0x9424
				}
				else {
					trial_log.AddString("Measurement mode");
					trial_log.AddString("Calculation failure(Brightness)");
				}
				goto MES_END;
			}
		}
			break;
			//マルチ探索
		case 0x04:
		{
			AlignMultiSrh::Param::CMultiSrhResult* multisrh_result = &temp_param->multisrh_result[i];
			FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

			CExcParamAlignMultiSrh exc_param;
			AlignMultiSrh::Param::CMesParamMultiSrh dll_param_mes;
			exc_param.SetMultiSrhMes(param, &dll_param_mes, i);

			double base_mark_x = scp_param->view[0].align.mark_x;
			double base_mark_y = scp_param->view[0].align.mark_y;

			//マルチ探索検査の演算実行
			if (calc_multisrh.CalcMain(m_Image, multisrh_result, &dll_param_mes, base_mark_x, base_mark_y, i, template_number, current.work_number, &mes_ok[i], &trial_log) == 1) {
				if (language.jpn) {
					trial_log.AddString("計測モード演算失敗(マルチ探索)");	//エラーコード0x9425
				}
				else {
					trial_log.AddString("Measurement mode");
					trial_log.AddString("Calculation failure(MultiSearch)");
				}
				goto MES_END;
			}
		}
			break;
			//基準-エッジ距離計測
		case 0x05:
		{
			AlignDisEdge::Param::CDisEdgeResult* disedge_result = &temp_param->disedge_result[i];
			FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

			CExcParamAlignDisEdge exc_param;
			AlignDisEdge::Param::CMesParamDisEdge dll_param_mes;
			exc_param.SetDisEdgeMes(param, &dll_param_mes, i);

			double base_mark_x = scp_param->view[0].align.mark_x;
			double base_mark_y = scp_param->view[0].align.mark_y;

			int assign_camera = sys_scope[scope].view[view].assign_camera;		//割り当てカメラ番号 0～7
			int coordinate_type = cam_parameter[assign_camera].coordinate_type;	//視野座標の構成タイプ
			double pixel_size = scp_param->view[view].calib.pixel_size;			//画素サイズ
			double crdnt_radian = scp_param->view[view].calib.crdnt_radian;		//座標系傾き(radian)
			BOOL pixel_complete = scp_param->view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
			//キャリブレーション未完了の場合
			if (!pixel_complete) {
				pixel_size = 0;		//画素サイズ0
				crdnt_radian = 0;	//座標系傾き0
			}

			//基準-エッジ距離計測検査の演算実行
			if (calc_disedge.CalcMain(m_Image, disedge_result, &dll_param_mes, base_mark_x, base_mark_y, i, coordinate_type, pixel_size, crdnt_radian, pixel_complete, &mes_ok[i], &trial_log) == 1) {
				if (language.jpn) {
					trial_log.AddString("計測モード演算失敗(基準-エッジ距離計測)");	//エラーコード0x9426
				}
				else {
					trial_log.AddString("Measurement mode");
					trial_log.AddString("Calculation failure(Distance(Mark-Edge))");
				}
				goto MES_END;
			}
		}
			break;
			//画像コピー(矩形)
		case 0x11:
		{
			//コピー先サイズリセット
			proc_cpyimg->SizeReset(temp_param, &temp_param->m_Image, i);

			//内部メモリへコピー
			proc_cpyimg->CopyToMem(temp_param, &temp_param->m_Image, param, i, &mes_ok[i], &trial_log);
		}
			break;
			//その他(想定外)
		default:
			if (language.jpn) {
				trial_log.AddString("計測モード演算失敗(その他)");	//エラーコード0x9429
			}
			else {
				trial_log.AddString("Measurement mode");
				trial_log.AddString("Calculation failure(Other)");
			}
			break;
		}

		//画像コピー結果は実際のテンプレートパラメータ結果にコピー
		template_param[template_number].m_CpyImg[i] = temp_param->m_CpyImg[i];
		template_param[template_number].copy_result[i] = temp_param->copy_result[i];
	}

	rtn_result = 0;

MES_END:
	return rtn_result;
}


/** ****************************************************** **
 ** バーコードリーダー実行
 ** 引数：
 **　BOOL* bcr_ok	検査結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::BCRMain(BOOL* bcr_ok)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//使用するパラメータ
	CTempParam* temp_param = &trial_temp_param;		//テンプレートパラメータ
	AlignBarcode::Param::CBarcodeParam* param = &trial_bcr_param;	//バーコードリーダーパラメータ
	CScopeParam* scp_param = &trial_scp_param;		//スコープパラメータ
	AlignBarcode::Data::CComDataBarcode com_data_bcr;		//読取文字列データ

	//バーコードリーダーのデータセット
	if (param->base_search_enable) {
		if (calc_bcr_ctrl.CalcDataSet(temp_param, scp_param, 1) != 0) {
			if (language.jpn) {
				trial_log.AddString("バーコードリーダー演算失敗(データセット)");	//エラーコード0x9441
			}
			else {
				trial_log.AddString("Barcode calculation failure");
				trial_log.AddString("(Data set)");
			}
			goto BCR_END;
		}
	}

	FVCL::Barcode::CFvBarcodeResult* bc_result = &temp_param->barcode.bc_result;
	FVCL::QR::CFvQRResult* qr_result = &temp_param->barcode.qr_result;
	FVCL::DM::CFvDMResult* dm_result = &temp_param->barcode.dm_result;
	int* base_search_result = &temp_param->barcode.base_search_result;
	FVCL::Data::CFvImage* m_Image = &temp_param->m_Image;

	double base_mark_x = scp_param->view[0].align.mark_x;
	double base_mark_y = scp_param->view[0].align.mark_y;

	//バーコードリーダーの演算実行
	if (calc_bcr.CalcMain(param, &com_data_bcr, m_Image, bc_result, qr_result, dm_result, base_mark_x, base_mark_y, bcr_ok, base_search_result, &trial_log) == 1) {
		trial_log.AddString(Tips0.SelStr(language.jpn, "バーコードリーダー演算失敗", "Barcode calculation failure"));	//エラーコード0x9442
		goto BCR_END;
	}

	rtn_result = 0;

BCR_END:
	return rtn_result;
}


/** ****************************************************** **
 ** アライメントの画像ビュー更新
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::ReDrawViewSrch(CTempParam* temp_param)
{
	// TODO: ここに実装コードを追加します.

	//基準点サーチ
	if (temp_param->additional.add_type == 0x00 || temp_param->additional.base_search_enable) {
		byte matching = temp_param->basic.matching;	//探索手法 0x00:グレイサーチ、0x01:形状サーチ

		//グレイサーチ実行
		if (matching == 0x00) DrawViewGS2();
		//形状サーチ実行
		if (matching == 0x01) DrawViewFPM2(temp_param->fpm2_rslt_edge);
	}

	//エッジサーチ
	if (temp_param->additional.add_type == 0x01) {
		DrawViewAddEdge();
	}
	//2値化重心サーチ
	else if (temp_param->additional.add_type == 0x02) {
		DrawViewAddBlob();
	}
}


/** ****************************************************** **
 ** 追加機能 エッジサーチ結果表示
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::DrawViewAddEdge()
{
	// TODO: ここに実装コードを追加します.
	CTempParam* temp_param = &trial_temp_param;

	//追加サーチ オーバーレイ描画クリア
	ovlay_add.m_Ovlay_Rslt->ClearFigures();
	ovlay_add.m_Ovlay_Area->ClearFigures();
	ovlay_add.m_Ovlay_Rslt->SetEnable(false);	//追加サーチ結果オーバーレイを隠す

	for (int direction = 0; direction < 2; direction++) {

		//サーチエリア有効判定OK
		if (temp_param->add_edge.area_ok[direction]) {

			//検出個数
			UINT count_result = temp_param->add_edge.result[direction].GetCount();

			//検出成功
			if (count_result > 0) {

				//結果の取得
				for (UINT i = 0; i < count_result; i++) {
					FVCL::Edge::CFvEdgeData data;
					temp_param->add_edge.result[direction].GetData(i, &data);

					bool judge_in = false;	//判定範囲内フラグ

					//判定範囲をRegion化しエッジ点が範囲内か確認する
					FVCL::Data::CFvRegion rect_region(temp_param->add_edge.rect_judge[direction]);
					judge_in = rect_region.Contains(data.GetPosition().x, data.GetPosition().y);

					if (judge_in) {
						//エッジ点
						gdi_add_edge[direction].m_Point_Rslt.SetPosition(data.GetPosition());

						//追加サーチ結果 エッジ点の追加
						ovlay_add.m_Ovlay_Rslt->DrawFigure(gdi_add_edge[direction].m_Point_Rslt);
					}
				}

				//追加サーチ結果 近似線の追加
				if (temp_param->add_edge.edge_ok[direction]) {
					gdi_add_edge[direction].m_Line_Rslt.a = temp_param->add_edge.rslt_line[direction].a;
					gdi_add_edge[direction].m_Line_Rslt.b = temp_param->add_edge.rslt_line[direction].b;
					gdi_add_edge[direction].m_Line_Rslt.c = temp_param->add_edge.rslt_line[direction].c;
					ovlay_add.m_Ovlay_Rslt->DrawFigure(gdi_add_edge[direction].m_Line_Rslt);
				}

				ovlay_add.m_Ovlay_Rslt->SetEnable(true);	//追加サーチ結果オーバーレイを表示
			}

			//追加サーチ範囲オーバーレイの更新
			FVCL::GDI::CFvGdiRectangle* rect = &gdi_add_edge[direction].m_Rect_Area;
			//サーチ範囲
			gdi_add_edge[direction].m_Rect_Area.st.x = temp_param->add_edge.rect_area[direction].st.x;
			gdi_add_edge[direction].m_Rect_Area.st.y = temp_param->add_edge.rect_area[direction].st.y;
			gdi_add_edge[direction].m_Rect_Area.ed.x = temp_param->add_edge.rect_area[direction].ed.x;
			gdi_add_edge[direction].m_Rect_Area.ed.y = temp_param->add_edge.rect_area[direction].ed.y;
			//矢印と線
			DrawScanDirection(direction, rect);
			//判定範囲
			gdi_add_edge[direction].m_Rect_Judg.st.x = temp_param->add_edge.rect_judge[direction].st.x;
			gdi_add_edge[direction].m_Rect_Judg.st.y = temp_param->add_edge.rect_judge[direction].st.y;
			gdi_add_edge[direction].m_Rect_Judg.ed.x = temp_param->add_edge.rect_judge[direction].ed.x;
			gdi_add_edge[direction].m_Rect_Judg.ed.y = temp_param->add_edge.rect_judge[direction].ed.y;

			ovlay_add.m_Ovlay_Area->DrawFigure(gdi_add_edge[direction].m_Rect_Judg);
			ovlay_add.m_Ovlay_Area->DrawFigure(gdi_add_edge[direction].m_Rect_Area);
			ovlay_add.m_Ovlay_Area->DrawFigure(gdi_add_edge[direction].m_Achr_Area);
			ovlay_add.m_Ovlay_Area->DrawFigure(gdi_add_edge[direction].m_Line_Area);
		}
	}

	//追加サーチ範囲オーバーレイを再表示
	ovlay_add.m_Ovlay_Area->SetEnable(true);

	return 0;
}


/** ****************************************************** **
 ** 追加機能エッジ 検出線方向の描画
 ** 引数：
 **　int direction	方向(0:縦, 1:横)
 **　FVCL::GDI::CFvGdiRectangle* rect	サーチエリアの矩形
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawScanDirection(int direction, FVCL::GDI::CFvGdiRectangle* rect)
{
	// TODO: ここに実装コードを追加します.
	CTempParam* temp_param = &trial_temp_param;

	int distance = 20;

	int scan_direction = temp_param->additional.edge_param[direction].scan_direction;	//検出方向
	double st_x, st_y;
	double ed_x, ed_y;

	//縦エッジ
	if (direction == 0) {

		switch (scan_direction) {
			//上から下(↓)
		case 2:
			st_x = rect->st.x - distance;
			st_y = rect->ed.y;
			ed_x = rect->st.x - distance;
			ed_y = rect->st.y;
			break;
			//下から上(↑)
		case 3:
			st_x = rect->st.x - distance;
			st_y = rect->st.y;
			ed_x = rect->st.x - distance;
			ed_y = rect->ed.y;
			break;
		}

		//矢印
		gdi_add_edge[0].m_Achr_Area.st.x = st_x;
		gdi_add_edge[0].m_Achr_Area.st.y = st_y;
		gdi_add_edge[0].m_Achr_Area.ed.x = ed_x;
		gdi_add_edge[0].m_Achr_Area.ed.y = ed_y;

		//線
		gdi_add_edge[0].m_Line_Area.st.x = st_x;
		gdi_add_edge[0].m_Line_Area.st.y = st_y;
		gdi_add_edge[0].m_Line_Area.ed.x = ed_x;
		gdi_add_edge[0].m_Line_Area.ed.y = ed_y;
	}
	//横エッジ
	else if (direction == 1) {

		switch (scan_direction) {
			//左から右(→)
		case 0:
			st_x = rect->ed.x;
			st_y = rect->ed.y + distance;
			ed_x = rect->st.x;
			ed_y = rect->ed.y + distance;
			break;
			//右から左(←)
		case 1:
			st_x = rect->st.x;
			st_y = rect->ed.y + distance;
			ed_x = rect->ed.x;
			ed_y = rect->ed.y + distance;
			break;
		}

		//矢印
		gdi_add_edge[1].m_Achr_Area.st.x = st_x;
		gdi_add_edge[1].m_Achr_Area.st.y = st_y;
		gdi_add_edge[1].m_Achr_Area.ed.x = ed_x;
		gdi_add_edge[1].m_Achr_Area.ed.y = ed_y;

		//線
		gdi_add_edge[1].m_Line_Area.st.x = st_x;
		gdi_add_edge[1].m_Line_Area.st.y = st_y;
		gdi_add_edge[1].m_Line_Area.ed.x = ed_x;
		gdi_add_edge[1].m_Line_Area.ed.y = ed_y;
	}
}


/** ****************************************************** **
 ** 追加機能 2値化重心サーチ結果表示
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::DrawViewAddBlob()
{
	// TODO: ここに実装コードを追加します.
	CTempParam* temp_param = &trial_temp_param;

	//追加サーチ オーバーレイ描画クリア
	ovlay_add.m_Ovlay_Rslt->ClearFigures();
	ovlay_add.m_Ovlay_Area->ClearFigures();
	ovlay_add.m_Ovlay_Rslt->SetEnable(false);	//追加サーチ結果オーバーレイを隠す

	//表示サイズの設定
	double magni_size = 1 / WholeMagnification();	//倍率サイズ
	int cross_size = 16 * magni_size;	//十字サイズ
	int font_size = 16 * magni_size;	//文字サイズ
	FVCL::GDI::CFvGdiFontA font_str;
	font_str.SetSize(font_size, 0);		//フォントサイズ 高さ(font_size), 平均文字幅0

	//サーチエリア有効判定OK
	if (temp_param->add_blob.area_ok) {

		//検出個数
		UINT count_result = temp_param->add_blob.result.GetCount();
		//検出成功
		if (count_result > 0) {
			UINT result_blob_number = temp_param->add_blob.blob_number;	//結果ブロブ番号

			//結果ブロブ番号の確認
			if (result_blob_number > 0 && result_blob_number <= count_result) {
				FVCL::Data::CFvRegion m_Region_Binz;	//ブロブ結果のリージョン
				FVCL::GDI::CFvGdiImage	m_Layer_Binz;	//GDI画像オブジェクト

				//抽出ブロブ描画(薄赤)
				temp_param->add_blob.result.GetRegion(result_blob_number, &m_Region_Binz);
				m_Layer_Binz.CopyFrom(m_Region_Binz, FVCL::Conversion::ToRGB32(0xFF, 0xDF, 0xDF, 0x00));	//薄赤
				FVCL::Blob::CFvBlobData data;
				int st_x = temp_param->add_blob.rect_area.st.x;		//サーチエリア始点X
				int st_y = temp_param->add_blob.rect_area.st.y;		//サーチエリア始点Y
				temp_param->add_blob.result.GetData(result_blob_number, &data);
				m_Layer_Binz.SetPosition(FVCL::Data::CFvPoint(st_x + data.GetXmin(), st_y + data.GetYmin()));

				//重心描画
				FVCL::Data::CFvPoint m_point_center;	//ブロブ重心位置
				data.GetCenter(&m_point_center);		//ブロブ重心取得
				gdi_add_blob.m_Point_Rslt.SetPosition(m_point_center + FVCL::Data::CFvPoint(st_x, st_y));
				gdi_add_blob.m_Point_Rslt.SetSize(cross_size, cross_size);		//重心位置 サイズ

				//面積描画
				FVCL::Data::CFvString strBlobNo;	//ブローブ番号
				UINT select_area = data.GetArea();	//ブロブ面積取得
				if (language.jpn) {
					strBlobNo.Format(_T("面積:%u"), select_area);	//面積表示を更新
				}
				else {
					strBlobNo.Format(_T("Area:%u"), select_area);	//面積表示を更新
				}
				gdi_add_blob.m_Area_Rslt.SetText(LPCSTR(strBlobNo));
				gdi_add_blob.m_Area_Rslt.SetPosition(m_point_center + FVCL::Data::CFvPoint(st_x, st_y) + FVCL::Data::CFvPoint(2, 2));	//中心から2ドットずらす
				gdi_add_blob.m_Area_Rslt.SetFont(font_str);

				ovlay_add.m_Ovlay_Rslt->DrawFigure(m_Layer_Binz);				//抽出ブロブの追加
				ovlay_add.m_Ovlay_Rslt->DrawFigure(gdi_add_blob.m_Point_Rslt);	//重心十字線の追加
				ovlay_add.m_Ovlay_Rslt->DrawFigure(gdi_add_blob.m_Area_Rslt);	//ブロブ面積の追加

				ovlay_add.m_Ovlay_Rslt->SetEnable(true);	//追加サーチ結果オーバーレイを表示
			}
		}

		//サーチ範囲
		gdi_add_blob.m_Rect_Area.st.x = temp_param->add_blob.rect_area.st.x;
		gdi_add_blob.m_Rect_Area.st.y = temp_param->add_blob.rect_area.st.y;
		gdi_add_blob.m_Rect_Area.ed.x = temp_param->add_blob.rect_area.ed.x;
		gdi_add_blob.m_Rect_Area.ed.y = temp_param->add_blob.rect_area.ed.y;

		ovlay_add.m_Ovlay_Area->DrawFigure(gdi_add_blob.m_Rect_Area);
	}

	//追加サーチ範囲オーバーレイを再表示
	ovlay_add.m_Ovlay_Area->SetEnable(true);

	return 0;
}


/** ****************************************************** **
 ** 計測モードの画像ビュー更新
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* param			検査パラメータ
 **　BOOL* mes_ok				判定結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::ReDrawViewMes(CTempParam* temp_param, CMesParam* param, BOOL* mes_ok)
{
	// TODO: ここに実装コードを追加します.

	//検査シート0～8
	for (int i = 0; i < 8; i++) {
		//検査機能を確認
		int mes_type = param->sheet[i].mes_type;

		switch (mes_type) {
			//ブロブ
		case 0x01:
			DrawViewBlob(temp_param, param, i, mes_ok);	//ブロブ検査結果表示
			break;
			//エッジ
		case 0x02:
			DrawViewEdge(temp_param, param, i);	//エッジ検査結果表示
			break;
			//濃度
		case 0x03:
			DrawViewMeas(temp_param, param, i);	//濃度検査結果表示
			break;
			//マルチ探索
		case 0x04:
			DrawViewMultiSrh(temp_param, param, i);	//マルチ探索検査結果表示
			break;
			//基準-エッジ距離計測
		case 0x05:
			DrawViewDisEdge(temp_param, param, i);	//基準-エッジ距離計測検査結果表示
			break;
			//画像コピー
		case 0x11:
			DrawViewCpyImg(temp_param, param, i);		//画像コピー結果表示
			break;
			//無し
		default:
			break;
		}
	}
}


/** ****************************************************** **
 ** ブロブ検査結果表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number			シート番号
 **　BOOL* mes_ok				判定結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewBlob(CTempParam* temp_param, CMesParam* meas_param, int sheet_number, BOOL* mes_ok)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignBlob exc_param;

	//テンプレートパラメータセット
	AlignBlob::Param::CTempParamBlob dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp, sheet_number);

	//検査パラメータセット
	AlignBlob::Param::CMesParamBlob dll_param_mes;
	exc_param.SetBlobMes(meas_param, &dll_param_mes, sheet_number);

	int scope = sys_template[template_number].scope;		//対象スコープ
	int view = sys_template[template_number].view;			//対象視野
	double pixel_size = scope_param[scope].view[view].calib.pixel_size;			//画素サイズ
	BOOL pixel_complete = scope_param[scope].view[view].calib.pixel_complete;	//画素キャリブレーション完了フラグ
	//キャリブレーション未完了の場合
	if (!pixel_complete) {
		pixel_size = 0;		//画素サイズ0
	}

	double magnification = WholeMagnification();

	AlignBlob::Draw::CDrawViewBlob draw_blob;
	draw_blob.DrawMain(
		&dll_param_temp,
		&dll_param_mes,
		ovlay_shet[sheet_number].m_Ovlay_Area,
		ovlay_shet[sheet_number].m_Ovlay_Rslt,
		&gdi_blob[sheet_number],
		pixel_size,
		magnification,
		mes_ok[sheet_number],
		sys_template[template_number].spec.blob.draw_cmd_result
	);
}


/** ****************************************************** **
 ** エッジ検査結果表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number		シート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewEdge(CTempParam* temp_param, CMesParam* meas_param, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignEdge exc_param;

	//テンプレートパラメータセット
	AlignEdge::Param::CTempParamEdge dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp, sheet_number);

	//検査パラメータセット
	AlignEdge::Param::CMesParamEdge dll_param_mes;
	exc_param.SetEdgeMes(meas_param, &dll_param_mes, sheet_number);

	double magnification = WholeMagnification();

	AlignEdge::Draw::CDrawViewEdge draw_edge;
	draw_edge.DrawMain(
		&dll_param_temp,
		&dll_param_mes,
		ovlay_shet[sheet_number].m_Ovlay_Area,
		ovlay_shet[sheet_number].m_Ovlay_Rslt,
		&gdi_edge[sheet_number],
		magnification);
}


/** ****************************************************** **
 ** 濃度検査結果表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number		シート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewMeas(CTempParam* temp_param, CMesParam* meas_param, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignMeas exc_param;

	//テンプレートパラメータセット
	AlignMeas::Param::CTempParamMeas dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp, sheet_number);

	//検査パラメータセット
	AlignMeas::Param::CMesParamMeas dll_param_mes;
	exc_param.SetMeasMes(meas_param, &dll_param_mes, sheet_number);

	double magnification = WholeMagnification();

	AlignMeas::Draw::CDrawViewMeas draw_meas;
	draw_meas.DrawMain(
		&dll_param_temp,
		&dll_param_mes,
		ovlay_shet[sheet_number].m_Ovlay_Area,
		ovlay_shet[sheet_number].m_Ovlay_Rslt,
		&gdi_meas[sheet_number],
		magnification);
}


/** ****************************************************** **
 ** マルチ探索検査結果表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number		シート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewMultiSrh(CTempParam* temp_param, CMesParam* meas_param, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignMultiSrh exc_param;

	//テンプレートパラメータセット
	AlignMultiSrh::Param::CTempParamMultiSrh dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp, sheet_number);

	//検査パラメータセット
	AlignMultiSrh::Param::CMesParamMultiSrh dll_param_mes;
	exc_param.SetMultiSrhMes(meas_param, &dll_param_mes, sheet_number);

	double magnification = WholeMagnification();

	AlignMultiSrh::Draw::CDrawViewMultiSrh draw_multisrh;
	draw_multisrh.DrawMain(
		&dll_param_temp,
		&dll_param_mes,
		ovlay_shet[sheet_number].m_Ovlay_Area,
		ovlay_shet[sheet_number].m_Ovlay_Rslt,
		&gdi_multisrh[sheet_number],
		magnification);
}


/** ****************************************************** **
 ** 基準-エッジ距離計測検査結果表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number		シート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewDisEdge(CTempParam* temp_param, CMesParam* meas_param, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignDisEdge exc_param;

	//テンプレートパラメータセット
	AlignDisEdge::Param::CTempParamDisEdge dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp, sheet_number);

	//検査パラメータセット
	AlignDisEdge::Param::CMesParamDisEdge dll_param_mes;
	exc_param.SetDisEdgeMes(meas_param, &dll_param_mes, sheet_number);

	double magnification = WholeMagnification();

	AlignDisEdge::Draw::CDrawViewDisEdge draw_disedge;
	draw_disedge.DrawMain(
		&dll_param_temp,
		&dll_param_mes,
		ovlay_shet[sheet_number].m_Ovlay_Area,
		ovlay_shet[sheet_number].m_Ovlay_Rslt,
		&gdi_disedge[sheet_number],
		magnification);
}


/** ****************************************************** **
 ** 画像コピー範囲表示
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　CMesParam* meas_param		検査パラメータ
 **　int sheet_number		シート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewCpyImg(CTempParam* temp_param, CMesParam* meas_param, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	CTempParam::CopyResult* result = &temp_param->copy_result[sheet_number];

	//検査結果オーバーレイ描画クリア
	ovlay_shet[sheet_number].m_Ovlay_Rslt->ClearFigures();


	//コピー範囲が存在する場合、描画する
	if (result->width > 0 && result->height > 0) {

		//表示サイズの設定
		double magni_size = 1 / WholeMagnification();	//倍率サイズ
		int font_size = 14 * magni_size;	//文字サイズ
		FVCL::GDI::CFvGdiFontA font_str;
		font_str.SetSize(font_size, 0);		//フォントサイズ 高さ(font_size), 平均文字幅0

		gdi_cpyimg[sheet_number].m_Rect_Copy.st.x = result->st_x;
		gdi_cpyimg[sheet_number].m_Rect_Copy.st.y = result->st_y;
		gdi_cpyimg[sheet_number].m_Rect_Copy.ed.x = result->st_x + result->width - 1;
		gdi_cpyimg[sheet_number].m_Rect_Copy.ed.y = result->st_y + result->height - 1;
		ovlay_shet[sheet_number].m_Ovlay_Rslt->DrawFigure(gdi_cpyimg[sheet_number].m_Rect_Copy);

		//文字
		gdi_cpyimg[sheet_number].m_Str_Copy.SetText(LPCSTR("画像コピー範囲"));
		gdi_cpyimg[sheet_number].m_Str_Copy.SetPosition(FVCL::CFvPoint(result->st_x + 3, result->st_y + 3));
		gdi_cpyimg[sheet_number].m_Str_Copy.SetFont(font_str);
		ovlay_shet[sheet_number].m_Ovlay_Rslt->DrawFigure(gdi_cpyimg[sheet_number].m_Str_Copy);

		ovlay_shet[sheet_number].m_Ovlay_Rslt->SetEnable(true);		//検査結果オーバーレイを表示
	}
	else {
		ovlay_shet[sheet_number].m_Ovlay_Rslt->SetEnable(false);	//検査結果オーバーレイを表示
	}
}


/** ****************************************************** **
 ** バーコードリーダーの画像ビュー更新
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　AlignBarcode::Param::CBarcodeParam* param	バーコードリーダーパラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::ReDrawViewBcr(CTempParam* temp_param, AlignBarcode::Param::CBarcodeParam* param)
{
	// TODO: ここに実装コードを追加します.

	CExcParamAlignBarcode exc_param;

	//テンプレートパラメータセット
	AlignBarcode::Param::CTempParamBarcode dll_param_temp;
	exc_param.SetTempParam(temp_param, &dll_param_temp);
	exc_param.SetTempResult(temp_param, &dll_param_temp);

	//バーコードリーダーパラメータセット
	AlignBarcode::Param::CBarcodeParam dll_param_bcr = *param;

	double magnification = WholeMagnification();

	//バーコード結果表示
	AlignBarcode::Draw::CDrawViewBarcode draw_bcr;
	draw_bcr.DrawMain(
		&dll_param_temp,
		&dll_param_bcr,
		ovlay_bcr.m_Ovlay_Area,
		ovlay_bcr.m_Ovlay_Rslt,
		&gdi_bcr,
		magnification);
}


/** ****************************************************** **
 ** 探索検出位置の取得
 ** 引数：
 **　double* mark_x		検出位置X
 **　double* mark_x		検出位置Y
 **　CTempParam* temp_param		探索パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::GetSrchMark(double* mark_x, double* mark_y, CTempParam* temp_param)
{
	// TODO: ここに実装コードを追加します.

	//基準点探索(グレイサーチ)
	if (temp_param->basic.matching == 0x00) {
		if (calc_mark.GetMarkGS2(temp_param, mark_x, mark_y) != 0) return 1;
	}
	//基準点探索(形状サーチ)
	else if (temp_param->basic.matching == 0x01) {
		if (calc_mark.GetMarkFPM2(temp_param, mark_x, mark_y) != 0) return 1;
	}
	else {
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** グレイサーチ結果表示
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewGS2()
{
	// TODO: ここに実装コードを追加します.
	CTempParam* param = &trial_temp_param;

	//結果の取得
	FVCL::Data::CFvPoint pat_mark = param->fv_pattern.GetCenterMark();		//パタン登録時の基準点
	FVCL::Data::CFvRectangle pat_rect = param->fv_pattern.GetRegistRect();	//パタン登録時の矩形

	F_GS_RESULT* data_ptr = param->gs2_result.GetAddress();	//結果先頭アドレスの取得

	//探索個数
	UINT count_result = param->gs2_result.GetCount();

	//探索結果オーバーレイ描画クリア
	ovlay_srch.m_Ovlay_Rslt->ClearFigures();

	//探索成功
	if (count_result > 0) {

		//表示サイズの設定
		double magni_size = 1 / WholeMagnification();		//倍率サイズ
		int cross_size = 16 * magni_size * GetPointMagni();	//十字サイズ
		int font_size = 16 * magni_size;	//文字サイズ
		FVCL::GDI::CFvGdiFontA font_str;
		font_str.SetSize(font_size, 0);		//フォントサイズ 高さ(font_size), 平均文字幅0

		for (UINT i = 0; i < count_result; i++) {
			//検出位置
			FVCL::Data::CFvPoint mark((double)data_ptr[0].x / (double)100, (double)data_ptr[0].y / (double)100);

			//検出位置とパタン情報から矩形を復元
			FVCL::Data::CFvRectangle rect;
			rect.st.x = mark.x - pat_mark.x;
			rect.st.y = mark.y - pat_mark.y;
			rect.ed.x = rect.st.x + pat_rect.GetWidth();
			rect.ed.y = rect.st.y + pat_rect.GetHeight();

			//相関値
			FVCL::Data::CFvString str_score;
			str_score.Format(_T("%5.2f"), (double)data_ptr[0].score / (double)100);

			//矩形枠描画
			ovlay_srch.m_Rect_Rslt.st.x = rect.st.x;
			ovlay_srch.m_Rect_Rslt.st.y = rect.st.y;
			ovlay_srch.m_Rect_Rslt.ed.x = rect.ed.x;
			ovlay_srch.m_Rect_Rslt.ed.y = rect.ed.y;

			//十字描画
			ovlay_srch.m_Point_Rslt.x = mark.x;
			ovlay_srch.m_Point_Rslt.y = mark.y;
			ovlay_srch.m_Point_Rslt.SetSize(cross_size, cross_size);	//検出位置 サイズ

			//スコア描画
			ovlay_srch.m_Score_Rslt.SetText(LPCSTR(str_score));
			ovlay_srch.m_Score_Rslt.SetPosition(mark);
			ovlay_srch.m_Score_Rslt.SetFont(font_str);

			//探索結果オーバーレイの追加
			ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Rect_Rslt);
			ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Point_Rslt);
			ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Score_Rslt);

			data_ptr += 1;
		}
		ovlay_srch.m_Ovlay_Rslt->SetEnable(true);		//探索結果オーバーレイを表示
	}
	//探索失敗
	else {
		ovlay_srch.m_Ovlay_Rslt->SetEnable(false);		//探索結果オーバーレイを隠す
	}

	//サーチ範囲オーバーレイの更新
	ovlay_srch.m_Ovlay_Area->ClearFigures();
	ovlay_srch.m_Rect_Area.st.x = param->base_area.st_x;
	ovlay_srch.m_Rect_Area.st.y = param->base_area.st_y;
	ovlay_srch.m_Rect_Area.ed.x = param->base_area.st_x + param->base_area.width - 1;
	ovlay_srch.m_Rect_Area.ed.y = param->base_area.st_y + param->base_area.height - 1;
	ovlay_srch.m_Ovlay_Area->DrawFigure(ovlay_srch.m_Rect_Area);

	//サーチ範囲オーバーレイを再表示
	ovlay_srch.m_Ovlay_Area->SetEnable(true);
}


/** ****************************************************** **
 ** 形状サーチ結果表示
 ** 引数：
 **　BOOL rslt_edge		FPM2特徴量検出結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::DrawViewFPM2(BOOL rslt_edge)
{
	// TODO: ここに実装コードを追加します.
	CTempParam* param = &trial_temp_param;

	//結果の取得
	FVCL::Data::CFvPoint offset = param->fpm2_templ.GetOffset();        // パタン登録時の基準点.
	double width = param->fpm2_templ.GetWidth();
	double height = param->fpm2_templ.GetHeight();

	int final_count = 0;	//閾値より上の数

	//探索結果オーバーレイ描画クリア
	ovlay_srch.m_Ovlay_Rslt->ClearFigures();

	//探索成功
	if (rslt_edge) {
		//探索個数
		UINT count_result = param->fpm2_matching.GetResultSize();

		//探索成功
		if (count_result > 0) {

			//表示サイズの設定
			double magni_size = 1 / WholeMagnification();		//倍率サイズ
			int cross_size = 16 * magni_size * GetPointMagni();	//十字サイズ
			int font_size = 16 * magni_size;	//文字サイズ
			FVCL::GDI::CFvGdiFontA font_str;
			font_str.SetSize(font_size, 0);		//フォントサイズ 高さ(font_size), 平均文字幅0

			for (UINT i = 0; i < count_result; i++) {
				FVCL::FPM2::CFvFPM2Data data = param->fpm2_matching[i];

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
					ed.x = st.x + width;
					ed.y = st.y + height;
					rect.st = st;
					rect.ed = ed;
					rect.center = offset;
					rect.angle = angle;

					//矩形枠描画
					ovlay_srch.m_Rect_Rslt.st.x = rect.st.x;
					ovlay_srch.m_Rect_Rslt.st.y = rect.st.y;
					ovlay_srch.m_Rect_Rslt.ed.x = rect.ed.x;
					ovlay_srch.m_Rect_Rslt.ed.y = rect.ed.y;
					ovlay_srch.m_Rect_Rslt.SetAxis(rect.center);		//回転の機軸座標
					ovlay_srch.m_Rect_Rslt.SetAngle(rect.angle.GetDegree());		//図形傾き

					//十字描画
					ovlay_srch.m_Point_Rslt.x = pos.x;
					ovlay_srch.m_Point_Rslt.y = pos.y;
					ovlay_srch.m_Point_Rslt.SetAngle(rect.angle.GetDegree());	//図形傾き
					ovlay_srch.m_Point_Rslt.SetSize(cross_size, cross_size);	//検出位置 サイズ

					//相関値
					FVCL::Data::CFvString str_score;
					str_score.Format(_T("%d"), score);

					//スコア描画
					ovlay_srch.m_Score_Rslt.SetText(LPCSTR(str_score));
					ovlay_srch.m_Score_Rslt.SetPosition(pos);
					ovlay_srch.m_Score_Rslt.SetAngle(rect.angle.GetDegree());	//図形傾き
					ovlay_srch.m_Score_Rslt.SetFont(font_str);

					//探索結果オーバーレイの追加
					ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Rect_Rslt);
					ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Point_Rslt);
					ovlay_srch.m_Ovlay_Rslt->DrawFigure(ovlay_srch.m_Score_Rslt);

					final_count++;
				}
			}

			//最終結果
			if (final_count > 0) {
				//			ovlay_srch.m_Ovlay_Rslt->SetEnable(true);		//探索結果オーバーレイを表示
			}
			else {
				//			ovlay_srch.m_Ovlay_Rslt->SetEnable(false);		//探索結果オーバーレイを隠す
			}
		}
		//探索失敗
		else {
			//		ovlay_srch.m_Ovlay_Rslt->SetEnable(false);		//探索結果オーバーレイを隠す
		}
	}

	//探索結果オーバーレイを表示
	ovlay_srch.m_Ovlay_Rslt->SetEnable(true);

	//サーチ範囲オーバーレイの更新
	ovlay_srch.m_Ovlay_Area->ClearFigures();
	ovlay_srch.m_Rect_Area.st.x = param->base_area.st_x;
	ovlay_srch.m_Rect_Area.st.y = param->base_area.st_y;
	ovlay_srch.m_Rect_Area.ed.x = param->base_area.st_x + param->base_area.width - 1;
	ovlay_srch.m_Rect_Area.ed.y = param->base_area.st_y + param->base_area.height - 1;
	ovlay_srch.m_Ovlay_Area->DrawFigure(ovlay_srch.m_Rect_Area);

	//サーチ範囲特徴量エッジ点表示
	if (rslt_edge) {
		FVCL::GDI::CFvGdiCluster m_clust_edge;		//複合図形クラス
		FVCL::CFvArray<FVCL::Edge::CFvEdgeData> edges_image = param->fpm2_feature.GetFeatureEdges();
		UINT edge_num = edges_image.size();
		for (UINT i = 0; i < edge_num; i++) {
			ovlay_srch.m_Point_FPM2Edge.SetPosition(edges_image[i].GetPosition());
			m_clust_edge.AddFigure(ovlay_srch.m_Point_FPM2Edge);
		}
		ovlay_srch.m_Ovlay_Area->DrawFigure(m_clust_edge);
	}

	//サーチ範囲オーバーレイを再表示
	ovlay_srch.m_Ovlay_Area->SetEnable(true);
}


/** ****************************************************** **
 ** 探索・検査結果オーバーレイのクリア
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::ReDrawOvlayClr()
{
	// TODO: ここに実装コードを追加します.

	//探索関係オーバーレイを非表示
	ovlay_srch.m_Ovlay_Area->SetEnable(false);	//サーチ範囲オーバーレイを隠す
	ovlay_srch.m_Ovlay_Rslt->SetEnable(false);	//探索結果オーバーレイを隠す

	//追加機能オーバーレイを非表示
	ovlay_add.m_Ovlay_Area->SetEnable(false);	//追加サーチ範囲オーバーレイを隠す
	ovlay_add.m_Ovlay_Rslt->SetEnable(false);	//追加サーチ結果オーバーレイを隠す

	//検査関係オーバーレイを非表示
	for (int i = 0; i < 8; i++) {
		ovlay_shet[i].m_Ovlay_Area->SetEnable(false);		//検査エリアオーバーレイを隠す
		ovlay_shet[i].m_Ovlay_Rslt->SetEnable(false);		//検査結果オーバーレイを隠す
	}
}


/** ****************************************************** **
 ** 作成テンプレート番号の入力
 ** 引数：
 **　int number　テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::SetParam(LPVOID pParam)
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
BOOL CDlgTrialTemplate::SelectBMPFile(LPTSTR lpSelectPath)
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
 **　LPCTSTR filepath	画像ファイルパス
 **　byte type			画像タイプ 0:元画像、1:ファイルから
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::LoadFvImage(LPCTSTR filepath, byte type)
{
	// TODO: ここに実装コードを追加します.
	CString title, msg;
	this->GetWindowTextA(title);

	FVCL::Data::CFvImage* load_Image;
	if (type == 0x00) {
		load_Image = &m_Image_base;	//元画像
	}
	else {
		load_Image = &m_file_Image;	//ファイル指定画像
	}

	//画像の読込
	bool rtn = load_Image->Load(filepath);

	if (!rtn) {
		int err_code = load_Image->GetErrorCode();

		msg = Tips0.IntToString(err_code);
		MessageBoxA(Tips0.SelStr(language.jpn,
			"ファイル読み込みに失敗しました(",
			"Failed to read the file(") +
			msg + ")", title, MB_OK | MB_ICONERROR);
		return 1;
	}

	//読み込んだ画像のサイズ
	int buf_width = load_Image->GetHorzSize();
	int buf_height = load_Image->GetVertSize();

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
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgTrialTemplate::WholeMagnification()
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
 ** 実行結果表示の更新
 ** 引数：
 **　WORD status	テスト実行結果
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::UpdateRsltInfo(WORD status)
{
	// TODO: ここに実装コードを追加します.

	//結果Icon表示
	switch (status) {
		//実行中
	case RSLT_BU:
		result_icon.text.SetWindowTextA("BSY");
		break;
		//結果OK
	case RSLT_OK:
		result_icon.text.SetWindowTextA("OK");
		break;
		//結果NG
	case RSLT_NG:
		result_icon.text.SetWindowTextA("NG");
		break;
		//エラー発生
	case RSLT_ER:
		result_icon.text.SetWindowTextA("ERR");
		break;
	default:
		result_icon.text.SetWindowTextA("");
		break;
	}
}


/** ****************************************************** **
 ** ライブ画像取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::LiveImageCapture()
{
	// TODO: ここに実装コードを追加します.

	if (flag_ReDraw) {

		int scorp = sys_template[this->template_number].scope;
		int view = sys_template[template_number].view;
		int cam = sys_scope[scorp].view[view].assign_camera;

		m_Image = m_ImageBuffer[cam];

		// 画像ビューの表示更新.
		m_EvView.SetImage(&m_Image);	//表示する画像の設定

		m_EvView.ReDraw(false);		//表示更新
	}

	flag_ReDraw = FALSE;
}


/** ****************************************************** **
 ** 検出十字倍率の取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 倍率
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::GetPointMagni()
{
	// TODO: ここに実装コードを追加します.
	int scope = sys_template[template_number].scope;
	int view = sys_template[template_number].view;
	int cam = sys_scope[scope].view[view].assign_camera;

	switch (sys_option.size_point_srch[cam]) {
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 4;
	case 3:
		return 8;
	default:
		return 1;
	}

	return 1;
}


/** ****************************************************** **
 ** ライブ画像取得開始
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgTrialTemplate::LiveCaptureStart()
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//ライブ画像取得開始メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_TRL_CAPTURE_ST, 0, (LPARAM)this);
	if (rtn != 0x1022) {
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
int CDlgTrialTemplate::LiveCaptureEnd()
{
	// TODO: ここに実装コードを追加します.
	CDlgSystemMain* dlg = dynamic_cast<CDlgSystemMain*>(reinterpret_cast<CWnd*>(pParam_main));

	//ライブ画像取得停止メッセージ処理
	//	補足:下記の様に親ウィンドウをたどってポインタを取得することも可能
	//		GetParent()->GetParent()->GetParent()->GetParent()->SendMessageA(WM_APP_COMMAND_GET_IMAGE, 0, (LPARAM)&m_Image_base);
	LRESULT rtn = dlg->SendMessageA(WM_APP_COMMAND_TRL_CAPTURE_ED, 0, (LPARAM)this);
	if (rtn != 0x1023) {
		CString title;
		this->GetWindowTextA(title);
		MessageBoxA(Tips0.SelStr(language.jpn, "Live停止失敗", "Live stop failed"), title, MB_OK | MB_ICONERROR);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 元画像ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	//ファイル名作成　例)C:\\N-Vision\\WORK\\T000\\BaseImage.bmp
	CString filename = "BaseImage.bmp";
	char path[MAX_PATH] = { '\0' };
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	strcat_s(path, Tips0.IntToString(current.work_number, 3));
	strcat_s(path, "\\T" + Tips0.IntToString(template_number, 3));
	strcat_s(path, "\\" + filename);

	//元画像の読み込み
	if (LoadFvImage(path, 0x00) == 0) {
		m_Image = m_Image_base;

		// 画像ビューの表示更新.
		m_EvView.SetImage(&m_Image);	//表示する画像の設定
	}

	ReDrawOvlayClr();			//探索・検査オーバーレイを非表示
	m_EvView.ReDraw(false);		//表示更新

}


/** ****************************************************** **
 ** ライブ画像ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ReDrawOvlayClr();		//探索・検査オーバーレイを非表示
	m_EvView.ReDraw(false);		//表示更新

	LiveCaptureStart();		//ライブ画像取得開始
}


/** ****************************************************** **
 ** ファイルからボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	char path[MAX_PATH] = { '\0' };

	BOOL rtn_result = SelectBMPFile(path);	//BMPファイルの選択

	if (rtn_result) {
		//画像ファイルの読み込み
		if (LoadFvImage(path, 0x01) == 0) {
			m_Image = m_file_Image;

			// 画像ビューの表示更新.
			m_EvView.SetImage(&m_Image);	//表示する画像の設定
		}
	}

	ReDrawOvlayClr();			//探索・検査オーバーレイを非表示
	m_EvView.ReDraw(false);		//表示更新
}


/** ****************************************************** **
 ** テスト実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton7()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int rs_srch = 1;		//探索実行結果(0:成功/1:失敗)
	int rs_mes = 1;			//検査実行結果(0:成功/1:失敗)
	int rs_bcr = 1;		//バーコード実行結果(0:成功/1:失敗)
	BOOL srch_ok = FALSE;	//探索結果(OK/NG)
	BOOL mes_ok[8];			//検査結果(OK/NG)
	for (int i = 0; i < 8; i++) {
		mes_ok[i] = FALSE;
	}
	BOOL bcr_ok = FALSE;	//バーコード結果(OK/NG)

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	status = RSLT_BU;
	UpdateRsltInfo(status);		//実行結果表示の更新

	//試行ログ出力
	trial_log.ResetContent();	//試行ログリストのクリア

	//テンプレートの機能(0x01:マーク探索、0x02:計測モード、0x03:バーコード)
	byte method_type = sys_template[template_number].method_type;

	//探索有無
	BOOL rqst_srch = (method_type == 0x01) ||
		((method_type == 0x02) && trial_mes_param.base_search_enable) ||
		((method_type == 0x03) && trial_bcr_param.base_search_enable);

	//検査有無
	BOOL rqst_mes = (method_type == 0x02);

	//バーコード有無
	BOOL rqst_bcr = (method_type == 0x03);

	//
	// 処理実行	
	//	

	//画像をセット
	trial_temp_param.m_Image = m_Image;

	//画像作成
	//※現在、make_image4工程の0を画像コピーとして利用している
	//　最終的には4工程の機能を選択可能にする
	if (trial_temp_param.make_image[0].make_type == 0x01) proc_cpyimg->CopyToGrab(&trial_temp_param, &trial_temp_param.m_Image);

	//探索実行
	if (rqst_srch) {
		rs_srch = GoSearch(&srch_ok);
		if (!(rs_srch == 0 && srch_ok)) goto JUMP;
	}

	//検査実行
	if (rqst_mes) rs_mes = MesMain(mes_ok);

	//バーコード実行
	if (rqst_bcr) rs_bcr = BCRMain(&bcr_ok);

JUMP:
	//
	// 画像ビュー更新	
	//	

	m_Image = trial_temp_param.m_Image;	//画像のコピー	
	ReDrawOvlayClr();					//探索・検査オーバーレイを非表示

	switch (method_type) {
	case 0x01:	//マーク探索
		if (rs_srch == 0) ReDrawViewSrch(&trial_temp_param);
		break;
	case 0x02:	//計測モード
		if (rs_srch == 0) ReDrawViewSrch(&trial_temp_param);
		if (rs_mes == 0) ReDrawViewMes(&trial_temp_param, &trial_mes_param, mes_ok);
		break;
	case 0x03:	//バーコード
		if (rs_srch == 0) ReDrawViewSrch(&trial_temp_param);
		if (rs_bcr == 0) ReDrawViewBcr(&trial_temp_param, &trial_bcr_param);
		break;
	}

	m_EvView.SetImage(&m_Image);	//表示する画像の更新
	m_EvView.ReDraw(false);			//表示更新

	//結果表示更新
	if ((!rqst_srch || srch_ok) &&
		(!rqst_mes || (mes_ok[0] && mes_ok[1] && mes_ok[2] && mes_ok[3] && mes_ok[4] && mes_ok[5] && mes_ok[6] && mes_ok[7])) &&
		(!rqst_bcr || bcr_ok)){
		status = RSLT_OK;
	}
	else {
		status = RSLT_NG;
	}
	UpdateRsltInfo(status);		//実行結果表示の更新
}


/** ****************************************************** **
 ** 全体表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(WholeMagnification());	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ×2ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(2.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 閉じるボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//ライブ画像取得停止
	if (LiveCaptureEnd() != 0) return;

	outlog.LogOutFile(1, "*** DlgTrialTemplate out (OK) ***");

	CDialogEx::OnOK();
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
void CDlgTrialTemplate::ICommandPre(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgTrialTemplate*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgTrialTemplate::ICommandPost(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgTrialTemplate*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgTrialTemplate::IWndMsgPre(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgTrialTemplate*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgTrialTemplate::IWndMsgPost(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgTrialTemplate*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgTrialTemplate::OnCommandPre(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgTrialTemplate::OnCommandPost(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
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
void CDlgTrialTemplate::OnWndMsgPre(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
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
void CDlgTrialTemplate::OnWndMsgPost(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);

		switch (message)
		{
		case WM_PAINT:			// 再描画イベント.
			break;
		case WM_MOUSEMOVE:
			break;
		case WM_LBUTTONDOWN:
			break;
		case WM_LBUTTONUP:
			break;
		}
	}
}


/** ****************************************************** **
 ** マウス左ボタン解放
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	//画像ビュー以外でのボタン操作でも処理する

	CDialogEx::OnLButtonUp(nFlags, point);
}


/** ****************************************************** **
 ** タイマ経過処理
 ** 引数：
 **　UINT_PTR nIDEvent	タイマ識別子
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgTrialTemplate::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ここにメッセージ ハンドラー コードを追加するか、既定の処理を呼び出します。

	//画像更新タイマ
	if (nIDEvent == TIM_REDRAW) {
		flag_ReDraw = TRUE;
	}

	CDialogEx::OnTimer(nIDEvent);
}


/** ****************************************************** **
 ** コントロール色指定
 ** 引数：
 **　CDC* pDC
 **　CWnd* pWnd
 **　UINT nCtlColor
 ** 戻り値：
 **　HBRUSH　ブラシ
 ** 特記事項：
 ** ****************************************************** **/
HBRUSH CDlgTrialTemplate::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: ここで DC の属性を変更してください。
	switch (nCtlColor) {
		//スタティックコントロール
	case CTLCOLOR_STATIC:
		switch (pWnd->GetDlgCtrlID()) {
		case IDC_STATIC_RESULT:
			//コマンド結果 未実行・実行中
			if (status == RSLT_NR || status == RSLT_BU) {
				pDC->SetBkColor(RGB(224, 224, 224));	//文字背景:薄灰
				pDC->SetTextColor(RGB(0, 0, 0));		//文字:黒
				return (HBRUSH)result_icon.brush[0];	//コントロール背景(灰)
			}
			//コマンド結果 OK終了
			if (status == RSLT_OK) {
				pDC->SetBkColor(RGB(0, 224, 64));		//文字背景:緑
				pDC->SetTextColor(RGB(255, 255, 255));	//文字:白
				return (HBRUSH)result_icon.brush[1];	//コントロール背景(緑)
			}
			//コマンド結果 NG終了
			if (status == RSLT_NG) {
				pDC->SetBkColor(RGB(255, 128, 128));	//文字背景:薄赤
				pDC->SetTextColor(RGB(255, 255, 255));	//文字:白
				return (HBRUSH)result_icon.brush[2];	//コントロール背景(薄赤)
			}
			//コマンド結果 異常終了
			if (status == RSLT_ER) {
				pDC->SetBkColor(RGB(255, 0, 0));		//文字背景:赤
				pDC->SetTextColor(RGB(255, 255, 255));	//文字:白
				return (HBRUSH)result_icon.brush[3];	//コントロール背景(赤)
			}
			break;
		}
	}

	// TODO: 既定値を使用したくない場合は別のブラシを返します。
	return hbr;
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
LRESULT CDlgTrialTemplate::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	switch (message) {

	case WM_APP_COMMAND_TRL_IMG_CAPTURE:
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
BOOL CDlgTrialTemplate::PreTranslateMessage(MSG* pMsg)
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
