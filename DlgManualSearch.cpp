/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgManualSearch.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.09.02	M.Iwasaki	検出位置十字サイズ可変対応
 ** 注記：
 ** 説明：マニュアルサーチダイアログ
 **		マニュアルサーチにて検出位置を指定する
 **/

// DlgManualSearch.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgManualSearch.h"
#include "afxdialogex.h"

extern CTemplates			sys_template[256];		//システムテンプレートパラメータ
extern CScopes				sys_scope[16];			//システムスコープパラメータ
extern CSysOption			sys_option;				//システムオプションパラメータ
extern CTempParam			template_param[256];	//機種毎テンプレートパラメータ
extern CCurrentStatus		current;				//共有情報
extern COutputLog			outlog;					//ログ出力

// CDlgManualSearch ダイアログ

IMPLEMENT_DYNAMIC(CDlgManualSearch, CDialogEx)

CDlgManualSearch::CDlgManualSearch(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgManualSearch, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 768;
	image_view_size_y = 576;
}

CDlgManualSearch::~CDlgManualSearch()
{
}

void CDlgManualSearch::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_STATIC_MANU_MSG, message.text);
}


BEGIN_MESSAGE_MAP(CDlgManualSearch, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgManualSearch::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgManualSearch::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgManualSearch::OnBnClickedButton3)
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(IDOK, &CDlgManualSearch::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgManualSearch::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgManualSearch メッセージ ハンドラー


BOOL CDlgManualSearch::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgManualSearch in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);
	this->GetClientRect(&r_client);

	dlg_x = 200;
	dlg_y = 120;
	dlg_width = (r_window.right - r_window.left) - (r_client.right - r_client.left) + image_view_size_x + 2;
	dlg_height = (r_window.bottom - r_window.top) - (r_client.bottom - r_client.top) + image_view_size_y + 120;
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
void CDlgManualSearch::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	m_View.MoveWindow(0, 30, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成
															
	//探索範囲表示
	m_Overlay_Area = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 探索範囲
	m_Overlay_Area->SetScaling(true);						//伸縮有無
	m_Rect_Area.GetPen().SetColor(RGB(0xFF, 0xFF, 0x00));	//探索範囲 黄色

	//基準点表示
	m_Overlay_Srch = m_EvView.m_display.CreateOverlay();	//オーバーレイの生成 基準点
	m_Overlay_Srch->SetScaling(true);						//伸縮有無
	m_Point_Srch.GetPen().SetColor(RGB(0xFF, 0x00, 0x00));	//検出位置 赤色
	m_Point_Srch.SetStyle(FVCL::GDI::FigureStyle::Cross);	//検出位置 スタイル(十字)

	//十字サイズ可変対応
	double magni_size = 1 / WholeMagnification();		//倍率サイズ
	int cross_size = 16 * magni_size * GetPointMagni();	//十字サイズ
	if (cross_size < 1) cross_size = 1;
	m_Point_Srch.SetSize(cross_size, cross_size);		//検出位置 サイズ

	m_EvView.SetFocusOption(FVCL::FocusOption::LDOWN);		//ビュー上での左クリック有効

	m_focus_pos = 0;	//フォーカス位置情報
	do_click = FALSE;	//左クリックOFF

	//左上メッセージ表示
	message.font.CreatePointFont(128, "MS UI Gothic");	//フォント指定
	message.text.SetFont(&message.font);
}


/** ****************************************************** **
 ** パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//探索範囲表示
	m_Overlay_Area->ClearFigures();
	m_Rect_Area.st.x = template_param[template_number].base_area.st_x;
	m_Rect_Area.st.y = template_param[template_number].base_area.st_y;
	m_Rect_Area.ed.x = template_param[template_number].base_area.st_x + template_param[template_number].base_area.width - 1;
	m_Rect_Area.ed.y = template_param[template_number].base_area.st_y + template_param[template_number].base_area.height - 1;
	m_Overlay_Area->DrawFigure(m_Rect_Area);

	//検出位置表示(画像の中心を初期値とする)
	m_Overlay_Srch->SetActive(true);	//マウス操作の有効化
	m_Overlay_Srch->ClearFigures();
	m_Point_Srch.x = m_Image->GetHorzSize() / 2;
	m_Point_Srch.y = m_Image->GetVertSize() / 2;
	m_Point_Srch.SetSelect(true);
	m_Overlay_Srch->DrawFigure(m_Point_Srch);

	//全体表示倍率の設定
	m_EvView.SetMagnification(WholeMagnification());

	// 画像ビューの表示更新.
	m_EvView.SetImage(m_Image); //表示する画像の設定

	//カーソルタイプを通常モードに指定
	m_EvView.SetCursorType(FVCL::CursorType::CROSS);
}


/** ****************************************************** **
 ** マニュアルサーチ テンプレート番号の入力
 ** 引数：
 **　int tmpl_number	テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::SetNumber(int tmpl_number)
{
	// TODO: ここに実装コードを追加します.
	template_number = tmpl_number;
}


/** ****************************************************** **
 ** サーチ画像入力
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::SetImage(FVCL::Data::CFvImage* image)
{
	// TODO: ここに実装コードを追加します.
	m_Image = image;


}


/** ****************************************************** **
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgManualSearch::WholeMagnification()
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
 ** 検出位置のカーソル描画
 ** 引数：
 **　CPoint* pos	マウスポジション
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::ChangeCursel(CPoint* pos)
{
	// TODO: ここに実装コードを追加します.

	BOOL focus = FALSE;	//検査エリア図形内フラグ

	//検出位置 外接矩形の位置確認(±5pix範囲)
	int focus_pos = m_Point_Srch.CheckFocusMarkPosition(*pos, 16.0);

	if (focus_pos != 0) focus = TRUE;

	//※カーソルの変更はしない
	// カーソル位置が矩形範囲内だったらカーソルを変更
	//if (focus) {
	//	switch (focus_pos) {
	//		//フォーカスON
	//	case 1:
	//		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
	//		break;
	//	}
	//}

	m_focus_pos = focus_pos;	//フォーカス情報記憶
}


/** ****************************************************** **
 ** 検出位置の更新
 ** 引数：
 **　CPoint* pos	マウスポジション
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::UpdatePoint(CPoint* pos)
{
	// TODO: ここに実装コードを追加します.

	m_Overlay_Srch->ClearFigures();

	//(左クリックしている場合)十字移動量
	int dx = pos->x - m_pos.x;
	int dy = pos->y - m_pos.y;

	//現在の検出位置
	FVCL::Data::CFvPoint srch_pos = m_Point_Srch.GetPosition();

	//更新する検査エリア矩形位置
	FVCL::Data::CFvPoint new_pos = srch_pos;

	switch (m_focus_pos) {
		// 移動
	case 1:
		m_Point_Srch.x += dx;
		m_Point_Srch.y += dy;
		break;
	}
	m_pos = *pos;

	m_Overlay_Srch->DrawFigure(m_Point_Srch);

	// 描画更新
	m_EvView.ReDraw(false);
/*
	int edit_x = (int)m_Rect_Area.st.x;
	int edit_y = (int)m_Rect_Area.st.y;
	int edit_width = (int)m_Rect_Area.GetWidth() + 1;
	int edit_height = (int)m_Rect_Area.GetHeight() + 1;

	//基準点探索する場合は基準点距離を設定する
	if (edit_param->base_search_enable) {
		edit_x = (int)m_Rect_Area.st.x - (int)base_mark_x;
		edit_y = (int)m_Rect_Area.st.y - (int)base_mark_y;
	}

	//始点とサイズの更新
	UpdateAreaEditXY(edit_x, edit_y, edit_width, edit_height);
*/
}


/** ****************************************************** **
 ** 検出十字倍率の取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 倍率
 ** 特記事項：
 ** ****************************************************** **/
int CDlgManualSearch::GetPointMagni()
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
 ** 全体表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(WholeMagnification());	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ×2ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(2.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//現在の検出位置
	FVCL::Data::CFvPoint srch_pos = m_Point_Srch.GetPosition();

	//テンプレートパラメータの指定位置を設定
	template_param[template_number].manu.manual_srch = srch_pos;
	template_param[template_number].manu.do_point = TRUE;	//実施した

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//マニュアルサーチリセット
	template_param[template_number].manu.manual_srch.x = 0;
	template_param[template_number].manu.manual_srch.y = 0;
	template_param[template_number].manu.do_point = FALSE;

	outlog.LogOutFile(1, "*** DlgManualSearch out (CANCEL) ***");

	CDialogEx::OnCancel();
}


/** ****************************************************** **
 ** マウス左ボタン解放
 ** 特記事項：
 ** ****************************************************** **/
void CDlgManualSearch::OnLButtonUp(UINT nFlags, CPoint point)
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
 ** コントロール色指定
 ** 引数：
 **　CDC* pDC
 **　CWnd* pWnd
 **　UINT nCtlColor
 ** 戻り値：
 **　HBRUSH　ブラシ
 ** 特記事項：
 ** ****************************************************** **/
HBRUSH CDlgManualSearch::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	// TODO: ここで DC の属性を変更してください。
	switch (nCtlColor) {
		//スタティックコントロール
	case CTLCOLOR_STATIC:
		switch (pWnd->GetDlgCtrlID()) {
		case IDC_STATIC_MANU_MSG:
			pDC->SetTextColor(RGB(255, 0, 0));		//文字:赤
			break;
		}
	}

	// TODO: 既定値を使用したくない場合は別のブラシを返します。
	return hbr;
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
void CDlgManualSearch::ICommandPre(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgManualSearch*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgManualSearch::ICommandPost(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgManualSearch*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgManualSearch::IWndMsgPre(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgManualSearch*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgManualSearch::IWndMsgPost(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgManualSearch*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgManualSearch::OnCommandPre(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgManualSearch::OnCommandPost(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
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
void CDlgManualSearch::OnWndMsgPre(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
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
void CDlgManualSearch::OnWndMsgPost(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

		CPoint pos = this->m_EvView.GetMousePosition(FVCL::MousePosition::ImageRect);
		switch (message)
		{
		case WM_PAINT:			// 再描画イベント.
			break;
		case WM_MOUSEMOVE:
			if (false == m_Overlay_Srch->GetActive()) break;

			//クリック前のカーソル描画変更
			if (!do_click) ChangeCursel(&pos);

			//クリック中の検出位置描画
			if (do_click) UpdatePoint(&pos);
			break;
		case WM_LBUTTONDOWN:
			if (m_focus_pos != 0) {
				// Viewのグリップをオフ
				m_EvView.SetGripOption(FVCL::GripOption::DISABLE);

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
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgManualSearch::PreTranslateMessage(MSG* pMsg)
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
