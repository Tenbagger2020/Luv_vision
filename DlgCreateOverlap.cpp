/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgCreateOverlap.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：画像合成の設定ダイアログ
 **		計測モードで使用する
 **		他テンプレートでコピーした画像を合成する
 **		前処理の機能の一つ、現状仮の設定ダイアログ、今後フィルタ処理などと一緒にまとめ前処理機能の設定ダイアログを作成する
 **/

// DlgCreateOverlap.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgCreateOverlap.h"
#include "afxdialogex.h"

extern CTemplates		sys_template[256];		//システムテンプレートパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CCurrentStatus	current;				//共有情報
extern COutputLog		outlog;					//ログ出力
extern CLanguage		language;				//言語設定用

// CDlgCreateOverlap ダイアログ

IMPLEMENT_DYNAMIC(CDlgCreateOverlap, CDialogEx)

CDlgCreateOverlap::CDlgCreateOverlap(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgCreateOverlap, pParent)
{

}

CDlgCreateOverlap::~CDlgCreateOverlap()
{
}

void CDlgCreateOverlap::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK1, check_overlap_enable);
	DDX_Control(pDX, IDC_COMBO1, combo_select_template);
}


BEGIN_MESSAGE_MAP(CDlgCreateOverlap, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCreateOverlap::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgCreateOverlap::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK1, &CDlgCreateOverlap::OnBnClickedCheck1)
END_MESSAGE_MAP()


// CDlgCreateOverlap メッセージ ハンドラー


BOOL CDlgCreateOverlap::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgCreateOverlap in ***");

	dlg_x = 400;
	dlg_y = 240;
	dlg_width = 240;
	dlg_height = 240;
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
void CDlgCreateOverlap::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//画像合成選択テンプレート コンボボックス
	for (int i = 0; i < 256; i++) {

		//テンプレートの機能が計測モードのみ対象
		if (sys_template[i].method_type == 0x02) {
			if (i != template_number) {
				combo_select_template.AddString("テンプレート" + Tips0.IntToString(i));
			}
		}
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
void CDlgCreateOverlap::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//テンプレートパラメータを編集用パラメータへコピー
	edit_param = template_param[template_number];

	//画像合成有効チェックボックス
	BOOL enable = (edit_param.make_image[0].make_type == 0x01);
	check_overlap_enable.SetCheck(enable);

	//合成テンプレート
	int combo_number = combo_select_template.GetCount();
	if (combo_number > 0) {
		CString text;
		CString temp_name = "テンプレート" + Tips0.IntToString(edit_param.make_image[0].copy_template);
		for (int i = 0; i < combo_number; i++) {
			combo_select_template.GetLBText(i, text);
			if (temp_name == text) {
				combo_select_template.SetCurSel(i);
			}
		}
	}
	combo_select_template.EnableWindow(enable);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateOverlap::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//画像合成有効チェックボックス
	if (check_overlap_enable.GetCheck()) {
		edit_param.make_image[0].make_type = 0x01;
	}
	else {
		edit_param.make_image[0].make_type = 0x00;
	}

	//合成テンプレート
	int temp_num = -1;
	int point = combo_select_template.GetCurSel();
	if (point >= 0) {
		CString temp_text;
		combo_select_template.GetLBText(point, temp_text);
		int head_len = (int)strlen("テンプレート");
		int text_len = (int)strlen(temp_text);
		CString s_number = temp_text.Mid(head_len, text_len - head_len);
		temp_num = atoi(s_number);
	}
	if (temp_num >= 0 && temp_num <= 255) {
		edit_param.make_image[0].copy_template = (byte)temp_num;
	}

	//編集用パラメータをテンプレートパラメータへコピー
	template_param[template_number] = edit_param;
}


/** ****************************************************** **
 ** テンプレート番号の入力
 ** 引数：
 **　int number　テンプレート番号
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateOverlap::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
}


/** ****************************************************** **
 ** 画像合成有効チェックボックスClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateOverlap::OnBnClickedCheck1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	BOOL enable = check_overlap_enable.GetCheck();
	combo_select_template.EnableWindow(enable);		//テンプレート選択コンボボックス
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateOverlap::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	ChangeParameter();	//パラメータの変更

	outlog.LogOutFile(1, "*** DlgCreateOverlap out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCreateOverlap::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgCreateOverlap out (CANCEL) ***");

	CDialogEx::OnCancel();
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgCreateOverlap::PreTranslateMessage(MSG* pMsg)
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
