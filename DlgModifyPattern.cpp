/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgModifyPattern.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：マスク設定ダイアログ
 **		テンプレートのマスクを設定する
 **/

// DlgModifyPattern.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgModifyPattern.h"
#include "afxdialogex.h"

#include "direct.h"

extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;		//言語設定用

extern char* APP_ROOT;

// CDlgModifyPattern ダイアログ

IMPLEMENT_DYNAMIC(CDlgModifyPattern, CDialogEx)

CDlgModifyPattern::CDlgModifyPattern(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgModifyPattern, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 320;
	image_view_size_y = 320;

	mod_item = 1;		//機能 1:マスク
	ope_mode = 1;		//操作 1:ペン

	pattern_enable = FALSE;		//パタン有効/無効
}

CDlgModifyPattern::~CDlgModifyPattern()
{
}

void CDlgModifyPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_RADIO1, radio_item_mask);
	DDX_Control(pDX, IDC_RADIO2, radio_mode_pen);
	DDX_Control(pDX, IDC_RADIO3, radio_mode_eraser);
	DDX_Control(pDX, IDC_COMBO1, combo_mode_size);
	DDX_Control(pDX, IDOK, btn_ok);
	DDX_Control(pDX, IDC_BUTTON1, btn_1x);
	DDX_Control(pDX, IDC_BUTTON2, btn_2x);
	DDX_Control(pDX, IDC_STATIC_FUNCTION, static_function);
	DDX_Control(pDX, IDC_STATIC_OPERATION, static_operation);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
}


BEGIN_MESSAGE_MAP(CDlgModifyPattern, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgModifyPattern::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgModifyPattern::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgModifyPattern::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgModifyPattern::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_RADIO2, &CDlgModifyPattern::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CDlgModifyPattern::OnBnClickedRadio3)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgModifyPattern::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CDlgModifyPattern メッセージ ハンドラー


BOOL CDlgModifyPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgModifyPattern in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);
	this->GetClientRect(&r_client);

	dlg_x = 400;
	dlg_y = 240;
	dlg_width = (r_window.right - r_window.left) - (r_client.right - r_client.left) + image_view_size_x + 112;
	dlg_height = (r_window.bottom - r_window.top) - (r_client.bottom - r_client.top) + image_view_size_y + 30;
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
void CDlgModifyPattern::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	m_View.MoveWindow(106, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成
	m_EvView.SetFocusOption(FVCL::FocusOption::ENTER);		//マウスが領域内に入ったらフォーカスを与える

	//操作サイズ
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ1", "Size1"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ3", "Size3"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ5", "Size5"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ7", "Size7"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ11", "Size11"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ21", "Size21"));
	combo_mode_size.AddString(Tips0.SelChar(language.jpn, "サイズ41", "Size41"));
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(3101));
		radio_item_mask.SetWindowTextA(language.GetCharacterString(3105));		//マスク機能ラジオ
		radio_mode_pen.SetWindowTextA(language.GetCharacterString(3107));		//ペン操作ラジオ
		radio_mode_eraser.SetWindowTextA(language.GetCharacterString(3108));	//消しゴムラジオ
		btn_ok.SetWindowTextA(language.GetCharacterString(3109));				//設定ボタン
		btn_1x.SetWindowTextA(language.GetCharacterString(3102));				//等倍ボタン
		btn_2x.SetWindowTextA(language.GetCharacterString(3103));				//x2ボタン
		static_function.SetWindowTextA(language.GetCharacterString(3104));		//機能
		static_operation.SetWindowTextA(language.GetCharacterString(3106));		//操作
		btn_cancel.SetWindowTextA(language.GetCharacterString(3110));			//キャンセルボタン
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
void CDlgModifyPattern::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//機能
	if (mod_item == 1) {
		radio_item_mask.SetCheck(1);
		SwitchMaskEnable(FVCL::Pattern::MaskIndex::MASK);	//マスク可視属性の設定
	}
	else {
		radio_item_mask.SetCheck(0);
	}

	//操作
	if (ope_mode == 1) {
		radio_mode_pen.SetCheck(1);
		radio_mode_eraser.SetCheck(0);
		m_EvView.SetCursorType(FVCL::CursorType::PEN);
	}
	else if (ope_mode == 2) {
		radio_mode_pen.SetCheck(0);
		radio_mode_eraser.SetCheck(1);
		m_EvView.SetCursorType(FVCL::CursorType::ERASER);
	}

	//操作サイズ
	combo_mode_size.SetCurSel(0);

	//登録パターンの読込
	int err_code = 0;	//エラーコード
	if (LoadPattern(&err_code) == 0) {
		//全体表示倍率の設定
		m_EvView.SetMagnification(1.0);		//等倍で表示

		pattern_enable = TRUE;	//パタン有効
	}
	else {
		//読込失敗
		CString title;
		this->GetWindowTextA(title);
		MessageBoxA(Tips0.SelStr(language.jpn,
			"登録マークがありません(",
			"There is no registration mark(") +
			Tips0.IntToString(err_code) + ")",
			title, MB_OK | MB_ICONINFORMATION);

		btn_ok.EnableWindow(FALSE);	//設定ボタン無効化
	}

	// 画像ビューの表示更新.
	// LoadPattern()の結果にかかわらず、SetImage()する
	// Loadに失敗した場合は初期値の黒画像が入る、CPatternViewでpatternの情報を参照するため
	m_EvView.SetImage(&pattern);	//表示するパターン画像の設定
}


/** ****************************************************** **
 ** 登録パターンの読込
 ** 引数：
 **　int* err_code	エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgModifyPattern::LoadPattern(int* err_code)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//ファイル保存
	CString filename = "BaseMark.pat";
	char path[MAX_PATH] = { '\0' };
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	strcat_s(path, Tips0.IntToString(current.work_number, 3));
	strcat_s(path, "\\T" + Tips0.IntToString(template_number, 3));
	strcat_s(path, "\\" + filename);

	if (pattern.Load(_T(path))) {
		rtn_result = 0;		//読込成功
	}
	else {
		*err_code = pattern.GetErrorCode();
	}

	return rtn_result;
}


/** ****************************************************** **
 ** マスク可視属性の設定
 ** 引数：
 **　INT index	マスクのビット指標
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::SwitchMaskEnable(INT index)
{
	// TODO: ここに実装コードを追加します.
	int mask_bit = m_EvView.m_display.GetMaskBitCount();	//マスクの種類

	if (mask_bit < 1) return;

	//マスクと不可欠部分の可視属性を設定
	// i:MaskIndex
	//　　0x00:MASK(マスク部分)
	//　　0x01:ESSENTIAL(不可欠部分)
	for (int i = 0; i < mask_bit; i++) {
		if (i == index) {
			m_EvView.m_display.SetMaskEnable(i, true);
		}
		else {
			m_EvView.m_display.SetMaskEnable(i, false);
		}
	}
}


/** ****************************************************** **
 ** パタンの更新
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgModifyPattern::SavePattern()
{
	// TODO: ここに実装コードを追加します.

	//ファイル保存
	CString filename = "BaseMark.pat";
	char path[MAX_PATH] = { '\0' };
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	strcat_s(path, Tips0.IntToString(current.work_number, 3));
	strcat_s(path, "\\T" + Tips0.IntToString(template_number, 3));

	if (_chdir(path) != 0) return 1;	//ディレクトリが無い場合は失敗

	strcat_s(path, "\\" + filename);
	if (!pattern.Save(path)) return 1;

	return 0;
}


/** ****************************************************** **
 ** 修正テンプレート番号の入力
 ** 引数：
 **　int number　テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
}


/** ****************************************************** **
 ** 等倍表示ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(1.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ×2ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	m_EvView.SetMagnification(2.0);	//倍率指定
	m_EvView.ReDraw(false);			//表示更新
}


/** ****************************************************** **
 ** ペン操作ラジオボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedRadio2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	ope_mode = 1;
	m_EvView.SetCursorType(FVCL::CursorType::PEN);
	m_EvView.ReDraw();
}


/** ****************************************************** **
 ** 消しゴム操作ラジオボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedRadio3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	ope_mode = 2;
	m_EvView.SetCursorType(FVCL::CursorType::ERASER);
	m_EvView.ReDraw();
}


/** ****************************************************** **
 ** 操作サイズコンボボックスSelchange
 ** 特記事項：
 **		操作サイズ 0:1、1:3、2:5、3:7、4:11、5:21、6:41
 ** ****************************************************** **/
void CDlgModifyPattern::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	int select_size = combo_mode_size.GetCurSel();

	UINT	pensize = 1;
	switch (select_size)
	{
	default:
	case 0:	pensize = 1; break;
	case 1:	pensize = 3; break;
	case 2:	pensize = 5; break;
	case 3:	pensize = 7; break;
	case 4:	pensize = 11; break;
	case 5:	pensize = 21; break;
	case 6:	pensize = 41; break;
	}
	m_EvView.SetPenSize(pensize);
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	CString title;
	CString msg;
	this->GetWindowTextA(title);

	msg = Tips0.SelStr(language.jpn, "マスク設定を更新します。よろしいですか？", "Update the mask setting. Is it OK?");
	int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		//パタン作成
		int rtn_result = SavePattern();

		//パタン登録成功
		if (rtn_result == 0) {
			//何もなし
		}
		//パタン登録失敗
		else {
			MessageBoxA(Tips0.SelStr(language.jpn, "マスク更新に失敗しました", "Mask update failed"), title, MB_OK | MB_ICONERROR);
		}

		outlog.LogOutFile(1, "*** DlgModifyPattern out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgModifyPattern::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgModifyPattern out (CANCEL) ***");

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
void CDlgModifyPattern::ICommandPre(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgModifyPattern*>(pvParam)->OnCommandPre(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgModifyPattern::ICommandPost(void *pvParam, CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgModifyPattern*>(pvParam)->OnCommandPost(dlg, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgModifyPattern::IWndMsgPre(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgModifyPattern*>(pvParam)->OnWndMsgPre(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	メッセージを dispatch するためのインターフェース関数
*/
void CDlgModifyPattern::IWndMsgPost(void *pvParam, CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static_cast<CDlgModifyPattern*>(pvParam)->OnWndMsgPost(dlg, message, wParam, lParam);
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理前)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgModifyPattern::OnCommandPre(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
{
}

// ========================================================================
/*!
	@brief	コマンドメッセージの処理 (処理後)

	@param[in]	dlg		メッセージ送信元のインスタンス
	@param[in]	wParam	引数1 (メッセージ依存の追加情報)
	@param[in]	lParam	引数2  (メッセージ依存の追加情報)
*/
void CDlgModifyPattern::OnCommandPost(CEvDialog *dlg, WPARAM wParam, LPARAM lParam)
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
void CDlgModifyPattern::OnWndMsgPre(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
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
void CDlgModifyPattern::OnWndMsgPost(CEvDialog *dlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	// CEvView からのメッセージを処理する.
	if (dlg->GetDlgCtrlFixedID() == IDC_STATIC_VIEW) {

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
BOOL CDlgModifyPattern::PreTranslateMessage(MSG* pMsg)
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
