/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgInitBC160.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.09.01	M.Iwasaki	カメラパラメータ設定追加
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2022.10.04	M.Iwasaki	K34004(ファーストボード対応)
 **			2023.04.03	M.Iwasaki	東芝テリー 画像反転対応
 **			2024.05.17	M.Iwasaki	10.17.2.0	オムロンセンテックカメラ対応
 ** 注記：
 ** 説明：BC160Mカメラパラメータ設定ダイアログ
 **		東芝テリーBC160Mのパラメータ設定を行う
 **/

// DlgInitBC160.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgInitBC160.h"
#include "afxdialogex.h"

extern CSysParamDevice		dev_parameter;		//デバイスパラメータ
extern CCameraSettings		cam_parameter[8];	//カメラパラメータ
extern COutputLog			outlog;				//ログ出力

// CDlgInitBC160 ダイアログ

IMPLEMENT_DYNAMIC(CDlgInitBC160, CDialogEx)

CDlgInitBC160::CDlgInitBC160(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgInitBC160, pParent)
{
	cam_number = 0;
	model_code = 0x0000;
}

CDlgInitBC160::~CDlgInitBC160()
{
}

void CDlgInitBC160::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, combo_com_port);
	DDX_Control(pDX, IDC_COMBO2, combo_clk);
	DDX_Control(pDX, IDC_COMBO3, combo_tap);
	DDX_Control(pDX, IDC_EDIT1, edit_exposure_time);
	DDX_Control(pDX, IDC_COMBO4, combo_flip);
}


BEGIN_MESSAGE_MAP(CDlgInitBC160, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgInitBC160::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CDlgInitBC160::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitBC160::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInitBC160 メッセージ ハンドラー


BOOL CDlgInitBC160::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgInitBC160 in ***");

	dlg_x = 512;
	dlg_y = 384;
	dlg_width = 320;
	dlg_height = 350;
	this->SetWindowPos(&wndTop, dlg_x, dlg_y, dlg_width, dlg_height, SWP_SHOWWINDOW);

	ControlInitialization();	//ダイアログ操作準備

	LoadComParam();		//通信パラメータの展開
	LoadRegiParam();	//内部パラメータの展開

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
void CDlgInitBC160::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	for (int i = 1; i <= 16; i++) {
		combo_com_port.AddString("COM" + Tips0.IntToString(i));
	}

	//基準クロック周波数
	combo_clk.AddString("37.5MHz");
	combo_clk.AddString("50MHz");
	combo_clk.AddString("83MHz");

	//tap数
	combo_tap.AddString("1tap");
	combo_tap.AddString("2tap");
	combo_tap.AddString("3tap");

	//画像反転
	combo_flip.AddString("無し");
	combo_flip.AddString("X反転");
	combo_flip.AddString("Y反転");
	combo_flip.AddString("XY反転");
}


/** ****************************************************** **
 ** 通信パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::LoadComParam()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	int com_port = regi_param->com_port;
	if (com_port == 0) {
		//初期設定ではカメラ1～4がCOM9～12に設定する
		combo_com_port.SetCurSel(cam_number + 8);
	}
	else {
		combo_com_port.SetCurSel(com_port - 1);
	}

	//ポート番号設定コンボボックスの無効化
	//　FASTボードの場合は無効化する
	int board_number = cam_number / 4;
	byte vender_code = dev_parameter.grab[board_number].vender_code;
	if (vender_code == 0x02) combo_com_port.EnableWindow(FALSE);

	//インターフェースがUSBの場合は通信パラメータを無効化する
	if ((model_code / 0x1000) == 2) {
		combo_com_port.SetCurSel(-1);
		combo_com_port.SetWindowTextA("");
		combo_com_port.EnableWindow(FALSE);
	}
}


/** ****************************************************** **
 ** 内部パラメータの展開
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::LoadRegiParam()
{
	// TODO: ここに実装コードを追加します.

	//基準クロック周波数
	combo_clk.SetCurSel(regi_param->bc160m.clk);

	//tap数
	combo_tap.SetCurSel(regi_param->bc160m.tap - 1);

	//露光時間
	int load_msec = LoadRegiTime();
	edit_exposure_time.SetWindowTextA(Tips0.IntToString(load_msec));

	//画像反転
	combo_flip.SetCurSel(LoadRegiFlip());

	//インターフェースがUSBの場合は基準クロック/tap数を無効化する
	if ((model_code / 0x1000) == 2) {
		combo_clk.SetCurSel(-1);
		combo_clk.SetWindowTextA("");
		combo_clk.EnableWindow(FALSE);
		combo_tap.SetCurSel(-1);
		combo_tap.SetWindowTextA("");
		combo_tap.EnableWindow(FALSE);
	}
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	regi_param->com_port = combo_com_port.GetCurSel() + 1;

	//基準クロック周波数
	regi_param->bc160m.clk = combo_clk.GetCurSel();

	//tap数
	regi_param->bc160m.tap = combo_tap.GetCurSel() + 1;

	//露光時間
	char buf[256] = { '\0' };
	edit_exposure_time.GetWindowTextA(buf, sizeof(buf));
	int change_msec = atoi(buf);
	SetRegiTime(change_msec);	//*ms

	//画像反転
	SetRegiFlip(combo_flip.GetCurSel());
}


/** ****************************************************** **
 ** パラメータ確認
 ** 引数：
 **　なし
 ** 戻り値：
**　int 結果　0:成功、1:未設定、2:露光時間設定範囲外
  ** 特記事項：
 ** ****************************************************** **/
int CDlgInitBC160::CheckParam()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	//設定の確認
	char buf[256] = { '\0' };

	//露光時間
	edit_exposure_time.GetWindowTextA(buf, sizeof(buf));
	int change_msec = atoi(buf);
	if (change_msec < 1 || change_msec > 1000) return 2;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 露光時間パラメータ取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int　露光時間(1-1000)
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitBC160::LoadRegiTime()
{
	// TODO: ここに実装コードを追加します.

	//露光時間
	int exposure_time = 16000;
	switch (model_code) {
		//BC160M
	case 0x1112:
		exposure_time = regi_param->bc160m.exposure_time / 37500;
		break;
		//BU160MG
	case 0x2112:
		exposure_time = regi_param->bu160mg.exposure_time / 1000;
		break;
	}

	return exposure_time;
}


/** ****************************************************** **
 ** 画像反転パラメータ取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int　画像反転(0-3)
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitBC160::LoadRegiFlip()
{
	// TODO: ここに実装コードを追加します.

	//画像反転
	int flip = 0;
	switch (model_code) {
		//BC160M
	case 0x1112:
		flip = regi_param->bc160m.flip;
		break;
		//BU160MG
	case 0x2112:
		flip = regi_param->bu160mg.flip;
		break;
	}

	return flip;
}


/** ****************************************************** **
 ** 露光時間パラメータセット
 ** 引数：
 **　int exposure_time	露光時間(1-1000)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::SetRegiTime(int exposure_time)
{
	// TODO: ここに実装コードを追加します.

	//露光時間
	switch (model_code) {
		//BC160M
	case 0x1112:
		regi_param->bc160m.exposure_time = exposure_time * 37500;
		break;
		//BU160MG
	case 0x2112:
		regi_param->bu160mg.exposure_time = exposure_time * 1000;
		break;
	}
}


/** ****************************************************** **
 ** 画像反転パラメータセット
 ** 引数：
 **　int flip	画像反転(0-3)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::SetRegiFlip(int flip)
{
	// TODO: ここに実装コードを追加します.

	//画像反転
	switch (model_code) {
		//BC160M
	case 0x1112:
		regi_param->bc160m.flip = flip;
		break;
		//BU160MG
	case 0x2112:
		regi_param->bu160mg.flip = flip;
		break;
	}
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　int number		カメラ番号(0～7)
 **　CCameraRegister* edit_param	編集パラメータ
 **　WORD model		モデルコード
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::SetParam(int number, CCameraRegister* edit_param, WORD model)
{
	// TODO: ここに実装コードを追加します.
	cam_number = number;
	regi_param = edit_param;
	model_code = model;
}


/** ****************************************************** **
 ** 初期値に戻すボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//パラメータ初期化
	switch (model_code) {
		//BC160M
	case 0x1112:
		regi_param->InitBC160M();
		break;
		//BU160MG
	case 0x2112:
		regi_param->InitBU160MG();
		break;
	}

	LoadRegiParam();	//内部パラメータの展開
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	this->GetWindowTextA(title);

	//パラメータ確認
	int rtn_chk = CheckParam();
	if (rtn_chk == 0) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgInitBC160 out (OK) ***");

		CDialogEx::OnOK();
	}
	else if (rtn_chk == 1) {
		MessageBoxA("設定を完了させてください", title, MB_OK | MB_ICONWARNING);
	}
	else if (rtn_chk == 2) {
		MessageBoxA("露光時間は1～1000msの範囲で設定してください", title, MB_OK | MB_ICONWARNING);
	}
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitBC160::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgInitBC160 out (CANCEL) ***");

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
BOOL CDlgInitBC160::PreTranslateMessage(MSG* pMsg)
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
