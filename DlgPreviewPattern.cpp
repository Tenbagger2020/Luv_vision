/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgPreviewPattern.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 ** 注記：
 ** 説明：パターンのプレビューダイアログ
 **		テンプレートに登録されているパターン画像を表示する
 **/

// DlgPreviewPattern.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgPreviewPattern.h"
#include "afxdialogex.h"

//DLL
#include "AlignBarcode.h"	//AlignBarcode DLL Header

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ
extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;		//言語設定用

//DLL
extern AlignBarcode::Param::CBarcodeParam bcr_param[256];	//機種毎バーコードリーダーパラメータ

extern char* APP_ROOT;

// CDlgPreviewPattern ダイアログ

IMPLEMENT_DYNAMIC(CDlgPreviewPattern, CDialogEx)

CDlgPreviewPattern::CDlgPreviewPattern(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgPreviewPattern, pParent)
{
	//画像表示サイズの指定
	image_view_size_x = 256;
	image_view_size_y = 256;
}

CDlgPreviewPattern::~CDlgPreviewPattern()
{
}

void CDlgPreviewPattern::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_VIEW, m_View);
	DDX_Control(pDX, IDC_STATIC_NAME, temp_name);
	DDX_Control(pDX, IDC_STATIC_DATE, create_date);
	DDX_Control(pDX, IDC_STATIC_MAG_INFO, mag_info);
}


BEGIN_MESSAGE_MAP(CDlgPreviewPattern, CDialogEx)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CDlgPreviewPattern メッセージ ハンドラー


BOOL CDlgPreviewPattern::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgPreviewPattern in ***");

	CRect r_window, r_client;
	this->GetWindowRect(&r_window);
	this->GetClientRect(&r_client);

	dlg_x = 400;
	dlg_y = 240;
	dlg_width = (r_window.right - r_window.left) - (r_client.right - r_client.left) + image_view_size_x;
	dlg_height = (r_window.bottom - r_window.top) - (r_client.bottom - r_client.top) + image_view_size_y + 60;
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
void CDlgPreviewPattern::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	m_View.MoveWindow(0, 0, image_view_size_x, image_view_size_y, FALSE);	//PictureControlのサイズ指定

	//PictureControlのサイズ取得
	CRect rect;
	m_View.GetWindowRect(&rect);
	ScreenToClient(&rect);		//rectをクライアント座標(ダイアログ上の座標)に変換

	m_EvView.Create(IDC_STATIC_VIEW, rect, this);			//画像ビューの生成
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPreviewPattern::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2501));
		create_date.SetWindowTextA(language.GetCharacterString(2502));	//作成日
		mag_info.SetWindowTextA(language.GetCharacterString(2503));		//倍率表示
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
void CDlgPreviewPattern::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//名称
	CString name = sys_template[template_number].name;
	temp_name.SetWindowTextA("No." + Tips0.IntToString(template_number, 3) + "(" + name + ")");

	//登録パターンの読込
	int err_code = 0;	//エラーコード
	if (LoadPattern(&err_code) == 0) {
		//全体表示倍率の設定
		m_EvView.SetMagnification(WholeMagnification());

		mag_info.SetWindowTextA(Tips0.SelStr(language.jpn, "表示倍率:", "Magnification:") + Tips0.DblToString(magnification_now, 2, TRUE));

		//作成日表示
		CString disp_date = Tips0.SelStr(language.jpn, "作成日:", "Created Date:") + template_param[template_number].create_date;

		byte method_type = sys_template[template_number].method_type;
		if (method_type == 0x01) {
			//マーク探索
			if (template_param[template_number].additional.add_type != 0x00 && !template_param[template_number].additional.base_search_enable) {

				disp_date = Tips0.SelStr(language.jpn, "基準点探索で使用していません", "Not used in reference point search");
			}
		}
		if (method_type == 0x02) {
			//計測モード
			if (!mes_param[template_number].base_search_enable) {

				disp_date = Tips0.SelStr(language.jpn, "基準点探索で使用していません", "Not used in reference point search");
			}
		}
		if (method_type == 0x03) {
			//バーコード
			if (!bcr_param[template_number].base_search_enable) {

				disp_date = Tips0.SelStr(language.jpn, "基準点探索で使用していません", "Not used in reference point search");
			}
		}
		create_date.SetWindowTextA(disp_date);

		//マスク表示設定
		m_EvView.m_display.SetMaskEnable(FVCL::Pattern::MaskIndex::MASK, true);			//マスク部分を表示する
		//m_EvView.m_display.SetMaskEnable(FVCL::Pattern::MaskIndex::ESSENTIAL, true);	//不可欠部分を表示する ※不可欠部分については未定

		// 画像ビューの表示更新.
		m_EvView.SetImage(&pattern);	//表示するパターン画像の設定
	}
	else {
		//読込失敗
		create_date.SetWindowTextA(Tips0.SelStr(language.jpn, "登録されていません(", "Not registered(") + Tips0.IntToString(err_code) + ")");
	}
}


/** ****************************************************** **
 ** 登録パターンの読込
 ** 引数：
 **　int* err_code	エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgPreviewPattern::LoadPattern(int* err_code)
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
 ** 全体表示倍率の計算
 ** 引数：
 **　なし
 ** 戻り値：
 **　double　倍率
 ** 特記事項：
 ** ****************************************************** **/
double CDlgPreviewPattern::WholeMagnification()
{
	// TODO: ここに実装コードを追加します.
	int image_width = pattern.GetHorzSize();
	int image_height = pattern.GetVertSize();
	double mag_x = (double)image_view_size_x / (double)image_width;
	double mag_y = (double)image_view_size_y / (double)image_height;
	double mag_min = min(mag_x, mag_y);
	//if (mag_min > 1) mag_min = 1;		※1倍以上もOKとする

	magnification_now = mag_min;	//現在の倍率

	return mag_min;
}


/** ****************************************************** **
 ** プレビューテンプレート番号の入力
 ** 引数：
 **　int number　テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPreviewPattern::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgPreviewPattern::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	outlog.LogOutFile(1, "*** DlgPreviewPattern out ***");
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgPreviewPattern::PreTranslateMessage(MSG* pMsg)
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
