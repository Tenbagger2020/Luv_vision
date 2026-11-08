/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgWorkChgComment.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2022.09.12	M.Iwasaki	エディットコントロール機能追加
 ** 注記：
 ** 説明：機種切替コメント編集ダイアログ
 **		選択機種のコメントを編集する
 **/

 // DlgWorkChgComment.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgWorkChgComment.h"
#include "afxdialogex.h"

extern COutputLog		outlog;				//ログ出力
extern CLanguage		language;			//言語設定用

// CDlgWorkChgComment ダイアログ

IMPLEMENT_DYNAMIC(CDlgWorkChgComment, CDialogEx)

CDlgWorkChgComment::CDlgWorkChgComment(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgWorkChgComment, pParent)
{
	work_number = 0;
	current_comment = "";
	update_comment = "";

	//コントロール設定
	edit_comment_update.SetFuncLBtnDblClk(OPEN_OSK);
}

CDlgWorkChgComment::~CDlgWorkChgComment()
{
}

void CDlgWorkChgComment::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_WORK_CHG_CMT, disp_comment);
	DDX_Control(pDX, IDC_EDIT1, edit_comment_current);
	DDX_Control(pDX, IDC_EDIT2, edit_comment_update);
	DDX_Control(pDX, IDC_STATIC_BEFORE, static_before);
	DDX_Control(pDX, IDC_STATIC_AFTER, static_after);
	DDX_Control(pDX, IDC_STATIC_MAX30, static_max30);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
}


BEGIN_MESSAGE_MAP(CDlgWorkChgComment, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgWorkChgComment::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgWorkChgComment::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgWorkChgComment メッセージ ハンドラー


BOOL CDlgWorkChgComment::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	ChangeLanguage();
	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgWorkChgComment in ***");

	//コメント表示
	disp_comment.SetWindowTextA(Tips0.SelStr(language.jpn, "機種No", "Work No") +
		Tips0.IntToString(work_number, 3) +
		Tips0.SelStr(language.jpn, "のコメントを編集します", " Edit the comment"));

	//変更前コメント表示
	edit_comment_current.SetWindowTextA(current_comment);

	//変更後コメント表示
	edit_comment_update.SetWindowTextA(update_comment);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 例外 : OCX プロパティ ページは必ず FALSE を返します。
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgComment::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1101));
		disp_comment.SetWindowTextA(language.GetCharacterString(1102));		//画面メッセージ
		static_before.SetWindowTextA(language.GetCharacterString(1104));	//変更前
		static_after.SetWindowTextA(language.GetCharacterString(1105));		//変更後
		static_max30.SetWindowTextA(language.GetCharacterString(1106));		//最大30文字
		btn_cancel.SetWindowTextA(language.GetCharacterString(1107));		//キャンセルボタン
	}
}


/** ****************************************************** **
 ** 編集対象機種Noセット
 ** 引数：
 **　int number		編集機種No
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgComment::SetNumber(int number)
{
	// TODO: ここに実装コードを追加します.
	work_number = number;
}


/** ****************************************************** **
 ** 編集対象機種コメントセット
 ** 引数：
 **　CString comment		現在コメント
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgComment::SetComment(CString comment)
{
	// TODO: ここに実装コードを追加します.
	current_comment = comment;
	update_comment = comment;
}


/** ****************************************************** **
 ** 編集対象機種コメントゲット
 ** 引数：
 **　なし
 ** 戻り値：
 **　変更後コメント
 ** 特記事項：
 ** ****************************************************** **/
CString CDlgWorkChgComment::GetComment()
{
	// TODO: ここに実装コードを追加します.
	return update_comment;
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgComment::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	char buf[31] = { '\0' };
	edit_comment_update.GetWindowTextA(buf, sizeof(buf));
	update_comment = buf;

	outlog.LogOutFile(1, "*** DlgWorkChgComment out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgWorkChgComment::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgWorkChgComment out (CANCEL) ***");

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
BOOL CDlgWorkChgComment::PreTranslateMessage(MSG* pMsg)
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
