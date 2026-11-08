/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgAdditionalMonitor.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：追加モニタダイアログ
 **		追加モニタにカメラ画像を表示する
 **/

// DlgAdditionalMonitor.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgAdditionalMonitor.h"
#include "afxdialogex.h"

#include "DlgSystemMain.h"

extern CCameraSettings	cam_parameter[8];		//カメラパラメータ

extern FVCL::Data::CFvImage m_ImageBuffer[8];	//画像メモリ

// CDlgAdditionalMonitor ダイアログ

IMPLEMENT_DYNAMIC(CDlgAdditionalMonitor, CDialogEx)

CDlgAdditionalMonitor::CDlgAdditionalMonitor(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgAdditionalMonitor, pParent)
{
	//画像表示サイズの初期化
	image_view_size_x = 0;
	image_view_size_y = 0;

	enable = FALSE;		//状態
	draw_tim = 0;
}

CDlgAdditionalMonitor::~CDlgAdditionalMonitor()
{
}

void CDlgAdditionalMonitor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ADDVIEW, m_View);
}


BEGIN_MESSAGE_MAP(CDlgAdditionalMonitor, CDialogEx)
END_MESSAGE_MAP()


// CDlgAdditionalMonitor メッセージ ハンドラー


BOOL CDlgAdditionalMonitor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください

	dlg_x = monitor_rect->left;
	dlg_y = monitor_rect->top;
	dlg_width = monitor_rect->right - monitor_rect->left;
	dlg_height = monitor_rect->bottom - monitor_rect->top;
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
void CDlgAdditionalMonitor::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	image_view_size_x = monitor_rect->right - monitor_rect->left;
	image_view_size_y = monitor_rect->bottom - monitor_rect->top;
	if (image_view_size_x < 1 || image_view_size_y < 1) return;

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_ADDVIEW, rect, this);			//画像ビューの生成
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgAdditionalMonitor::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	if (cam_num < 0 || cam_num > 7) return;

	//選択カメラの確認
	if (cam_parameter[cam_num].device.model_code == 0) return;	//カメラが存在しない場合は enable = FALSE

	//m_Imageは初期値が640×480なので、カメラ毎に画像サイスを変更する
	//　horz,vertのサイズで真っ黒画像が作成される
	UINT horz = cam_parameter[cam_num].device.grab_width;
	UINT vert = cam_parameter[cam_num].device.grab_height;
	m_Image.SetSize(horz, vert, FVCL::ImageType::UC8, 1, 0);	//画像サイズの変更

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());

	// 画像ビューの表示更新.
	m_EvView.SetImage(&m_Image); //表示する画像の設定

	//カーソルタイプを通常モードに指定
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);

	enable = TRUE;	//正常とする
}


/** ****************************************************** **
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgAdditionalMonitor::WholeMagnification()
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
 ** ライブ画像取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgAdditionalMonitor::LiveImageCapture()
{
	// TODO: ここに実装コードを追加します.

	m_Image = m_ImageBuffer[cam_num];

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
 ** モニタサイズ情報の設定
 ** 引数：
 **　RECT* rect		モニタサイズ情報
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgAdditionalMonitor::SetWindowRect(RECT* rect)
{
	// TODO: ここに実装コードを追加します.

	monitor_rect = rect;
}


/** ****************************************************** **
 ** 表示カメラの設定
 ** 引数：
 **　int num	カメラ番号(0～7)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgAdditionalMonitor::SetCamNumber(int num)
{
	// TODO: ここに実装コードを追加します.

	cam_num = num;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam	パラメータ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgAdditionalMonitor::SetParam(LPVOID pParam)
{
	pParam_main = pParam;
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
LRESULT CDlgAdditionalMonitor::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: ここに特定なコードを追加するか、もしくは基底クラスを呼び出してください。

	switch (message) {

	case WM_APP_COMMAND_ADD_IMG_CAPTURE:
		if (enable) LiveImageCapture();
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
BOOL CDlgAdditionalMonitor::PreTranslateMessage(MSG* pMsg)
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
