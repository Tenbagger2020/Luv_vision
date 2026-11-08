/**	Copyright (C) 2025 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgCmdSettingGOTFtp.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2026.03.26	M.Iwasaki	10.19.1.2	FTP機能追加
 ** 注記：
 ** 説明：コマンド設定のファイル転送ダイアログ
 **		FTPファイル転送コマンドの詳細設定をする
 **/

// DlgCmdSettingGOTFtp.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgCmdSettingGOTFtp.h"
#include "afxdialogex.h"

extern COutputLog			outlog;					//ログ出力

// CDlgCmdSettingGOTFtp ダイアログ

IMPLEMENT_DYNAMIC(CDlgCmdSettingGOTFtp, CDialogEx)

CDlgCmdSettingGOTFtp::CDlgCmdSettingGOTFtp(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgCmdSettingGOTFtp, pParent)
{

}

CDlgCmdSettingGOTFtp::~CDlgCmdSettingGOTFtp()
{
}

void CDlgCmdSettingGOTFtp::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, edit_user);
	DDX_Control(pDX, IDC_EDIT2, edit_pw);
	DDX_Control(pDX, IDC_EDIT3, edit_address);
	DDX_Control(pDX, IDC_EDIT4, edit_sv_directory);
	DDX_Control(pDX, IDC_EDIT5, edit_download_path);
	DDX_Control(pDX, IDC_EDIT6, edit_file_name);
}


BEGIN_MESSAGE_MAP(CDlgCmdSettingGOTFtp, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgCmdSettingGOTFtp::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgCmdSettingGOTFtp::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgCmdSettingGOTFtp メッセージ ハンドラー


BOOL CDlgCmdSettingGOTFtp::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgCmdSettingGOTFtp in ***");

	dlg_x = 400;
	dlg_y = 400;
	dlg_width = 336;
	dlg_height = 336;
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
void CDlgCmdSettingGOTFtp::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//FTPファイル削除の場合
	if (edit_cmd_parameter->base_method == 0x53) {
		//転送先パス入力の無効化
		edit_download_path.EnableWindow(FALSE);
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
void CDlgCmdSettingGOTFtp::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//ログインユーザー
	edit_user.SetWindowTextA(edit_cmd_parameter->ftp_login.login_user);

	//ログインパスワード
	edit_pw.SetWindowTextA(edit_cmd_parameter->ftp_login.login_pw);

	//IPアドレス
	edit_address.SetWindowTextA(edit_cmd_parameter->ftp_login.ip_address);

	//転送元ディレクトリ
	edit_sv_directory.SetWindowTextA(edit_cmd_parameter->ftp_dlgot.sv_directory);

	//転送先パス
	edit_download_path.SetWindowTextA(edit_cmd_parameter->ftp_dlgot.download_path);

	//指定ファイル名
	edit_file_name.SetWindowTextA(edit_cmd_parameter->ftp_dlgot.file_name);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingGOTFtp::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	char buf[32] = { '\0' };

	//ログインユーザー
	edit_user.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_login.login_user = buf;
	
	//ログインパスワード
	edit_pw.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_login.login_pw = buf;

	//FTPサーバーIPアドレス
	edit_address.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_login.ip_address = buf;


	//転送元ディレクトリ
	edit_sv_directory.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_dlgot.sv_directory = buf;

	//転送先パス
	edit_download_path.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_dlgot.download_path = buf;

	//指定ファイル名
	edit_file_name.GetWindowTextA(buf, sizeof(buf));
	edit_cmd_parameter->ftp_dlgot.file_name = buf;
}


/** ****************************************************** **
 ** 設定パラメータチェック
 ** 引数：
 **　なし
 ** 戻り値：
 **　WORD	結果コード
 **		0x0101:ログインユーザーを5文字以上入力してください
 **		0x0201:ログインパスワードを8文字以上入力してください
 **		0x0301:IPアドレスを入力してください
 **		0x0401:転送元ディレクトリを入力してください
 **		0x0501:転送先ディレクトリを入力してください
 **		0x0601:指定ファイル名を入力してください
 ** 特記事項：
 ** ****************************************************** **/
WORD CDlgCmdSettingGOTFtp::CheckParameter()
{
	// TODO: ここに実装コードを追加します.
	WORD rtn_code = 0x0000;

	char buf[32] = { '\0' };
	int len_ipt = 0;

	//ログインユーザー入力確認
	edit_user.GetWindowTextA(buf, sizeof(buf));
	len_ipt = (int)strlen(buf);
	if (len_ipt < 5) return 0x0101;	//入力ミス

	//ログインパスワード入力確認
	edit_pw.GetWindowTextA(buf, sizeof(buf));
	len_ipt = (int)strlen(buf);
	if (len_ipt < 8) return 0x0201;	//入力ミス

	//IPアドレス入力確認
	edit_address.GetWindowTextA(buf, sizeof(buf));
	len_ipt = (int)strlen(buf);
	if (len_ipt <7) return 0x0301;	//入力ミス

	//転送元ディレクトリ入力確認
	edit_sv_directory.GetWindowTextA(buf, sizeof(buf));
	len_ipt = (int)strlen(buf);
	if (len_ipt == 0) return 0x0401;	//入力ミス

	//転送先ディレクトリ入力確認
	if (edit_cmd_parameter->base_method == 0x52) {
		edit_download_path.GetWindowTextA(buf, sizeof(buf));
		len_ipt = (int)strlen(buf);
		if (len_ipt == 0) return 0x0501;	//入力ミス
	}

	//指定ファイル名入力確認
	edit_file_name.GetWindowTextA(buf, sizeof(buf));
	len_ipt = (int)strlen(buf);
	if (len_ipt == 0) return 0x0601;	//入力ミス

	return rtn_code;
}


/** ****************************************************** **
 ** 編集用パラメータの入力
 ** 引数：
 **　CCommands* param	編集用パラメータ
 **　BOOL* flag			詳細設定完了フラグ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingGOTFtp::SetParameter(CCommands* param, BOOL* flag)
{
	// TODO: ここに実装コードを追加します.
	edit_cmd_parameter = param;

	edit_ok = flag;
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingGOTFtp::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//設定チェック
	WORD rtn_code = CheckParameter();
	if (rtn_code != 0x0000) {
		CString title, msg1;
		this->GetWindowTextA(title);

		switch (rtn_code) {
		case 0x0101:
			msg1 = "ログインユーザーを5文字以上入力してください";
			break;
		case 0x0201:
			msg1 = "ログインパスワードを8文字以上入力してください";
			break;
		case 0x0301:
			msg1 = "IPアドレスを入力してください";
			break;
		case 0x0401:
			msg1 = "転送元ディレクトリを入力してください";
			break;
		case 0x0501:
			msg1 = "転送先ディレクトリを入力してください";
			break;
		case 0x0601:
			msg1 = "指定ファイル名を入力してください";
			break;
		default:
			msg1 = "設定ミスです";
			break;
		}

		MessageBoxA(msg1, title, MB_OK | MB_ICONINFORMATION);
	}
	//設定正常
	else {
		ChangeParameter();	//パラメータの変更
		*edit_ok = TRUE;	//フラグ更新

		outlog.LogOutFile(1, "*** DlgCmdSettingGOTFtp out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgCmdSettingGOTFtp::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgCmdSettingGOTFtp out (CANCEL) ***");

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
BOOL CDlgCmdSettingGOTFtp::PreTranslateMessage(MSG* pMsg)
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
