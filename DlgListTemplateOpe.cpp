/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgListTemplateOpe.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.09.12	M.Iwasaki	削除での検査パラメータ初期化追加
 ** 注記：
 ** 説明：機種データ テンプレート操作ダイアログ
 **		テンプレートの操作を選択する
 **/

// DlgListTemplateOpe.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgListTemplateOpe.h"
#include "afxdialogex.h"

#include "DlgListTemplateMain.h"
#include "DlgCreateTemplate.h"
#include "DlgTrialTemplate.h"
#include "DlgCopyTemplate.h"
#include "ProcFileOpe.h"

extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ
extern CProcFileOpe		proc_file;		//ファイル処理クラス
extern CCurrentStatus	current;		//共有情報
extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

extern char* APP_ROOT;

// CDlgListTemplateOpe ダイアログ

IMPLEMENT_DYNAMIC(CDlgListTemplateOpe, CDialogEx)

CDlgListTemplateOpe::CDlgListTemplateOpe(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgListTemplateOpe, pParent)
{

}

CDlgListTemplateOpe::~CDlgListTemplateOpe()
{
}

void CDlgListTemplateOpe::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, btn_create);
	DDX_Control(pDX, IDC_BUTTON2, btn_testing);
	DDX_Control(pDX, IDC_BUTTON3, btn_copy);
	DDX_Control(pDX, IDC_BUTTON4, btn_delete);
}


BEGIN_MESSAGE_MAP(CDlgListTemplateOpe, CDialogEx)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgListTemplateOpe::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CDlgListTemplateOpe::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CDlgListTemplateOpe::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CDlgListTemplateOpe::OnBnClickedButton4)
END_MESSAGE_MAP()


// CDlgListTemplateOpe メッセージ ハンドラー


BOOL CDlgListTemplateOpe::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgListTemplateOpe in ***");

	dlg_x = 640;
	dlg_y = 240;
	dlg_width = 320;
	dlg_height = 128;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

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
void CDlgListTemplateOpe::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2401));
		btn_create.SetWindowTextA(language.GetCharacterString(2402));	//作成ボタン
		btn_testing.SetWindowTextA(language.GetCharacterString(2403));	//テスト実行ボタン
		btn_copy.SetWindowTextA(language.GetCharacterString(2404));		//コピーボタン
		btn_delete.SetWindowTextA(language.GetCharacterString(2405));	//削除ボタン
	}
}


/** ****************************************************** **
 ** テンプレート削除実行
 ** 引数：
 **　なし
** 戻り値：
 **　int：結果　0:成功, 1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgListTemplateOpe::DeleteTemplate()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	char path[MAX_PATH] = { '\0' };
	CString s_work;
	CString s_template;

	//テンプレート削除実行
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	s_work = Tips0.IntToString(current.work_number, 3);
	strcat_s(path, s_work);
	s_template = "\\T" + Tips0.IntToString(template_number, 3);
	strcat_s(path, s_template);

	if (proc_file.DeleteFileAllEx(path, TRUE) != 0) goto END;

	template_param[template_number].InitialParam();		//テンプレートの初期化
	mes_param[template_number].InitialParam();			//検査パラメータの初期化

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 作成テンプレート番号の入力
 ** 引数：
 **　int number		テンプレート番号
 **　int group		グループ番号(0～3)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::SetNumber(int number, int group)
{
	// TODO: ここに実装コードを追加します.
	template_number = number;
	group_number = group;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　LPVOID pParam1	パラメータ(メインダイアログのポインタ)
 **　LPVOID pParam2	パラメータ(テンプレートリストダイアログのポインタ)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::SetParam(LPVOID pParam1, LPVOID pParam2)
{
	pParam_main = pParam1;
	pParam_list = pParam2;
}


/** ****************************************************** **
 ** 作成ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	this->ShowWindow(SW_HIDE);	//非表示

	//テンプレート一覧ダイアログを非表示
	CDlgListTemplateMain* dlg = dynamic_cast<CDlgListTemplateMain*>(reinterpret_cast<CWnd*>(pParam_list));
	dlg->ShowWindow(SW_HIDE);

	CDlgCreateTemplate dlg_create_template;
	dlg_create_template.SetParam(pParam_main);		//パラメータのセット
	dlg_create_template.SetNumber(template_number);
	dlg_create_template.DoModal();

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** テスト実行ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::OnBnClickedButton2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	this->ShowWindow(SW_HIDE);	//非表示

	//テンプレート一覧ダイアログを非表示
	CDlgListTemplateMain* dlg = dynamic_cast<CDlgListTemplateMain*>(reinterpret_cast<CWnd*>(pParam_list));
	dlg->ShowWindow(SW_HIDE);

	CDlgTrialTemplate dlg_trial_template;
	dlg_trial_template.SetParam(pParam_main);		//パラメータのセット
	dlg_trial_template.SetNumber(template_number);
	dlg_trial_template.DoModal();

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** コピーボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::OnBnClickedButton3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	this->ShowWindow(SW_HIDE);	//非表示

	CDlgCopyTemplate dlg_copy_template;
	dlg_copy_template.SetNumber(template_number, group_number);
	dlg_copy_template.DoModal();

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** 削除ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::OnBnClickedButton4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title, msg1, msg2;
	this->GetWindowTextA(title);

	msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
	msg2 = Tips0.SelStr(language.jpn, "テンプレートNo", "Delete template No") +
		Tips0.IntToString(template_number, 3) +
		Tips0.SelStr(language.jpn, "を削除します。よろしいですか？", ". Is it OK?");
	int judge = MessageBoxA(msg1 + "\n" + msg2, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		//削除実行
		if (DeleteTemplate() == 0) {
			//成功メッセージ
			msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
			msg2 = Tips0.SelStr(language.jpn, "テンプレート削除に成功しました。", "The template has been deleted successfully");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONINFORMATION);
			outlog.LogOutFile(1, "テンプレート削除成功 No" + Tips0.IntToString(template_number, 3));
		}
		else {
			//失敗メッセージ
			msg1 = Tips0.SelStr(language.jpn, "削除実行", "Delete execution");
			msg2 = Tips0.SelStr(language.jpn, "テンプレート削除に失敗しました。\n未登録の状態かデータがありません。", "Failed to delete the template\nUnregistered status or no data");
			MessageBoxA(msg1 + "\n" + msg2, title, MB_OK | MB_ICONERROR);
			outlog.LogOutFile(1, "テンプレート削除失敗 No" + Tips0.IntToString(template_number, 3));
		}
	}
}


/** ****************************************************** **
 ** ウィンドウの破棄
 ** 特記事項：
 ** ****************************************************** **/
void CDlgListTemplateOpe::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ここにメッセージ ハンドラー コードを追加します。
	outlog.LogOutFile(1, "*** DlgListTemplateOpe out ***");
}


/** ****************************************************** **
 ** メッセージ変換
 ** 引数：
 **　MSG* pMsg	メッセージの構造体
 ** 戻り値：
 **　BOOL 結果
 ** 特記事項：
 ** ****************************************************** **/
BOOL CDlgListTemplateOpe::PreTranslateMessage(MSG* pMsg)
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
