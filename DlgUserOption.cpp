/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgUserOption.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2020.09.02	M.Iwasaki	検出位置十字サイズ設定追加
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.09.12	M.Iwasaki	デバックモード追加、エディットコントロール機能追加
 ** 注記：
 ** 説明：ユーザーオプション機能ダイアログ
 **		オプション設定ダイアログを表示する
 **/

// DlgUserOption.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgUserOption.h"
#include "afxdialogex.h"

#include "DlgAdditionalMonitor.h"

extern CSysOption		sys_option;			//システムオプションパラメータ
extern CAccessParamSys	access_sys;			//システムIniファイルアクセス
extern COutputLog			outlog;			//ログ出力

extern CDlgAdditionalMonitor* monitor_dlg;		//追加モニタダイアログ
extern CLanguage		language;				//言語設定用

// CDlgUserOption ダイアログ

IMPLEMENT_DYNAMIC(CDlgUserOption, CDialogEx)

CDlgUserOption::CDlgUserOption(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgUserOption, pParent)
{
	//コントロール設定
	edit_cmd_view_time.SetFuncLBtnDblClk(OPEN_TKY);
	edit_cmd_view_time.SetTenkeyInt(0, 60);
}

CDlgUserOption::~CDlgUserOption()
{
}

void CDlgUserOption::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, edit_cmd_view_time);
	DDX_Control(pDX, IDC_COMBO1, combo_image_save);
	DDX_Control(pDX, IDC_COMBO2, combo_file_type);
	DDX_Control(pDX, IDC_COMBO3, combo_add_camera);
	DDX_Control(pDX, IDC_COMBO4, combo_size_point_srch[0]);
	DDX_Control(pDX, IDC_COMBO5, combo_size_point_srch[1]);
	DDX_Control(pDX, IDC_COMBO6, combo_size_point_srch[2]);
	DDX_Control(pDX, IDC_COMBO7, combo_size_point_srch[3]);
	DDX_Control(pDX, IDC_COMBO8, combo_size_point_srch[4]);
	DDX_Control(pDX, IDC_COMBO9, combo_size_point_srch[5]);
	DDX_Control(pDX, IDC_COMBO10, combo_size_point_srch[6]);
	DDX_Control(pDX, IDC_COMBO11, combo_size_point_srch[7]);
	DDX_Control(pDX, IDC_STATIC_TIME, static_time);
	DDX_Control(pDX, IDC_STATIC_SAVE, static_save);
	DDX_Control(pDX, IDC_STATIC_FORMAT, static_format);
	DDX_Control(pDX, IDC_STATIC_ADD_MONITOR, static_add_monitor);
	DDX_Control(pDX, IDC_STATIC_CROSS_SIZE, static_cross_size);
	DDX_Control(pDX, IDC_STATIC_CAM1, static_cam[0]);
	DDX_Control(pDX, IDC_STATIC_CAM2, static_cam[1]);
	DDX_Control(pDX, IDC_STATIC_CAM3, static_cam[2]);
	DDX_Control(pDX, IDC_STATIC_CAM4, static_cam[3]);
	DDX_Control(pDX, IDC_STATIC_CAM5, static_cam[4]);
	DDX_Control(pDX, IDC_STATIC_CAM6, static_cam[5]);
	DDX_Control(pDX, IDC_STATIC_CAM7, static_cam[6]);
	DDX_Control(pDX, IDC_STATIC_CAM8, static_cam[7]);
	DDX_Control(pDX, IDOK, btn_set);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
	DDX_Control(pDX, IDC_CHECK1, check_mode_debug);
}


BEGIN_MESSAGE_MAP(CDlgUserOption, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgUserOption::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgUserOption::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgUserOption メッセージ ハンドラー


BOOL CDlgUserOption::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgUserOption in ***");

	dlg_x = 220;
	dlg_y = 80;
	dlg_width = 512;
	dlg_height = 352;
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
void CDlgUserOption::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//コマンド画像保存
	combo_image_save.AddString(Tips0.SelChar(language.jpn, "無し", "None"));
	combo_image_save.AddString(Tips0.SelChar(language.jpn, "NG画像のみ", "NG Image Only"));
	combo_image_save.AddString(Tips0.SelChar(language.jpn, "OK画像のみ", "OK Image Only"));
	combo_image_save.AddString(Tips0.SelChar(language.jpn, "全て", "All"));

	//画像形式
	combo_file_type.AddString(Tips0.SelChar(language.jpn, "BMP形式", "BMP"));
	combo_file_type.AddString(Tips0.SelChar(language.jpn, "JPEG形式", "JPEG"));

	//追加モニタ カメラ番号
	combo_add_camera.AddString(Tips0.SelChar(language.jpn, "無し", "None"));
	combo_add_camera.AddString(Tips0.SelChar(language.jpn, "カメラ1", "Cam1"));
	combo_add_camera.AddString(Tips0.SelChar(language.jpn, "カメラ2", "Cam2"));
	combo_add_camera.AddString(Tips0.SelChar(language.jpn, "カメラ3", "Cam3"));
	combo_add_camera.AddString(Tips0.SelChar(language.jpn, "カメラ4", "Cam4"));

	//検出十字サイズ
	for (int i = 0; i < 8; i++) {
		combo_size_point_srch[i].AddString(Tips0.SelChar(language.jpn, "通常", "Basic"));
		combo_size_point_srch[i].AddString(Tips0.SelChar(language.jpn, "大1", "L1"));
		combo_size_point_srch[i].AddString(Tips0.SelChar(language.jpn, "大2", "L2"));
		combo_size_point_srch[i].AddString(Tips0.SelChar(language.jpn, "大3", "L3"));
	}
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgUserOption::ChangeLanguage()
{
	POINT pt[8];
	RECT re[8];
	int width[8], height[8];

	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(1901));
		static_time.SetWindowTextA(language.GetCharacterString(1902));			//表示時間
		static_save.SetWindowTextA(language.GetCharacterString(1903));			//画像保存
		static_format.SetWindowTextA(language.GetCharacterString(1908));		//ファイル形式
		static_add_monitor.SetWindowTextA(language.GetCharacterString(1911));	//追加モニタ
		static_cross_size.SetWindowTextA(language.GetCharacterString(1914));	//十字サイズ
		static_cam[0].SetWindowTextA(language.GetCharacterString(1915) + "1");	//cam1
		static_cam[1].SetWindowTextA(language.GetCharacterString(1915) + "2");	//cam2
		static_cam[2].SetWindowTextA(language.GetCharacterString(1915) + "3");	//cam3
		static_cam[3].SetWindowTextA(language.GetCharacterString(1915) + "4");	//cam4
		static_cam[4].SetWindowTextA(language.GetCharacterString(1915) + "5");	//cam5
		static_cam[5].SetWindowTextA(language.GetCharacterString(1915) + "6");	//cam6
		static_cam[6].SetWindowTextA(language.GetCharacterString(1915) + "7");	//cam7
		static_cam[7].SetWindowTextA(language.GetCharacterString(1915) + "8");	//cam8
		btn_set.SetWindowTextA(language.GetCharacterString(1918));				//設定ボタン
		btn_cancel.SetWindowTextA(language.GetCharacterString(1919));			//キャンセルボタン

		//コントロール位置・サイズ調整
		static_time.GetWindowRect(&re[0]);			//表示時間
		static_save.GetWindowRect(&re[1]);			//画像保存
		static_format.GetWindowRect(&re[2]);		//ファイル形式
		static_add_monitor.GetWindowRect(&re[3]);	//追加モニタ
		edit_cmd_view_time.GetWindowRect(&re[4]);	//表示時間入力
		combo_image_save.GetWindowRect(&re[5]);		//画像保存コンボ
		combo_file_type.GetWindowRect(&re[6]);		//画像形式コンボ
		combo_add_camera.GetWindowRect(&re[7]);		//追加モニタカメラ番号コンボ

		for (int i = 0; i < 8; i++) {
			pt[i].x = re[i].left;
			pt[i].y = re[i].top;
			width[i] = re[i].right - re[i].left;
			height[i] = re[i].bottom - re[i].top;
			this->ScreenToClient(&pt[i]);
		}
		static_time.MoveWindow(pt[0].x, pt[0].y, width[0] - 15, height[0], TRUE);			//表示時間
		static_save.MoveWindow(pt[1].x, pt[1].y, width[1] - 25, height[1], TRUE);			//画像保存
		static_format.MoveWindow(pt[2].x, pt[2].y, width[2], height[2], TRUE);				//ファイル形式
		static_add_monitor.MoveWindow(pt[3].x, pt[3].y, width[3], height[3], TRUE);			//追加モニタ
		edit_cmd_view_time.MoveWindow(pt[4].x - 20, pt[4].y, width[4], height[4], TRUE);	//表示時間入力
		combo_image_save.MoveWindow(pt[5].x - 30, pt[5].y, width[5] + 10, height[5], TRUE);		//画像保存コンボ
		combo_file_type.MoveWindow(pt[6].x + 5, pt[6].y, width[6], height[6], TRUE);		//画像形式コンボ
		combo_add_camera.MoveWindow(pt[7].x, pt[7].y, width[7], height[7], TRUE);			//追加モニタカメラ番号コンボ

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
void CDlgUserOption::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//コマンド実行後の結果表示時間
	edit_cmd_view_time.SetWindowTextA(Tips0.IntToString(sys_option.cmd_view_time));

	//コマンド画像保存
	combo_image_save.SetCurSel(sys_option.image_save_mode);

	//画像形式
	combo_file_type.SetCurSel(sys_option.file_type);

	//追加モニタ カメラ番号
	combo_add_camera.SetCurSel(sys_option.add_monitor_cam);

	//検出十字サイズ
	for (int i = 0; i < 8; i++) {
		combo_size_point_srch[i].SetCurSel(sys_option.size_point_srch[i]);
	}

	//デバックモード
	check_mode_debug.SetCheck(sys_option.debug_mode);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgUserOption::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	char buf[256] = { '\0' };

	//コマンド実行後の結果表示時間
	edit_cmd_view_time.GetWindowTextA(buf, sizeof(buf));
	sys_option.cmd_view_time = atoi(buf);

	//コマンド画像保存
	sys_option.image_save_mode = (byte)combo_image_save.GetCurSel();

	//画像形式
	sys_option.file_type = (byte)combo_file_type.GetCurSel();

	//追加モニタ カメラ番号
	int add_camera = combo_add_camera.GetCurSel();
	if (sys_option.multi_display > 0) {
		if (monitor_dlg != NULL) {
			//追加モニタを使用しているときは追加モニタの設定を更新
			monitor_dlg->enable = FALSE;
			if (add_camera > 0) {
				monitor_dlg->SetCamNumber(add_camera - 1);
				monitor_dlg->LoadParameter();
			}
		}
	}
	sys_option.add_monitor_cam = add_camera;

	//検出十字サイズ
	for (int i = 0; i < 8; i++) {
		sys_option.size_point_srch[i] = combo_size_point_srch[i].GetCurSel();
	}

	//デバックモード ※Iniファイル保存はしない
	sys_option.debug_mode = check_mode_debug.GetCheck();

	//Iniファイル書き込み
	access_sys.WriteIniFileOption();
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgUserOption::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);

	msg = Tips0.SelStr(language.jpn, "設定を更新します。よろしいですか？", "Update the setting. Is it OK?");
	int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgUserOption out (OK) ***");

		CDialogEx::OnOK();
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgUserOption::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgUserOption out (CANCEL) ***");

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
BOOL CDlgUserOption::PreTranslateMessage(MSG* pMsg)
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
