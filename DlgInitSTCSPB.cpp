/**	Copyright (C) 2024 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgInitSTCSPB.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.05.17	M.Iwasaki	10.17.2.0	オムロンセンテックカメラ対応
 ** 注記：
 ** 説明：オムロンセンテック STC-SPBモノクロカメラパラメータ設定ダイアログ
 **		オムロンセンテックSTC-SPB***のパラメータ設定を行う
 **/

// DlgInitSTCSPB.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgInitSTCSPB.h"
#include "afxdialogex.h"

extern CSysParamDevice		dev_parameter;		//デバイスパラメータ
extern CCameraSettings		cam_parameter[8];	//カメラパラメータ
extern COutputLog			outlog;				//ログ出力

// CDlgInitSTCSPB ダイアログ

IMPLEMENT_DYNAMIC(CDlgInitSTCSPB, CDialogEx)

CDlgInitSTCSPB::CDlgInitSTCSPB(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgInitSTCSPB, pParent)
{
	cam_number = 0;
	model_code = 0x0000;
}

CDlgInitSTCSPB::~CDlgInitSTCSPB()
{
}

void CDlgInitSTCSPB::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, combo_com_port);
	DDX_Control(pDX, IDC_COMBO2, combo_clk);
	DDX_Control(pDX, IDC_COMBO3, combo_tap);
	DDX_Control(pDX, IDC_EDIT1, edit_exposure_time);
	DDX_Control(pDX, IDC_COMBO4, combo_flip);
}


BEGIN_MESSAGE_MAP(CDlgInitSTCSPB, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON1, &CDlgInitSTCSPB::OnBnClickedButton1)
	ON_BN_CLICKED(IDOK, &CDlgInitSTCSPB::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgInitSTCSPB::OnBnClickedCancel)
END_MESSAGE_MAP()


// CDlgInitSTCSPB メッセージ ハンドラー


BOOL CDlgInitSTCSPB::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgInitSTCSPB in ***");

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
void CDlgInitSTCSPB::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	for (int i = 1; i <= 16; i++) {
		combo_com_port.AddString("COM" + Tips0.IntToString(i));
	}

	//基準クロック周波数
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		combo_clk.AddString("37.5MHz");
		combo_clk.AddString("75MHz");
		combo_clk.AddString("84.8MHz");
		break;
		//STC-SPB163PCL
	case 0x1412:
		combo_clk.AddString("37.5MHz");
		combo_clk.AddString("75MHz");
		combo_clk.AddString("84.8MHz");
		break;
		//STC-SPB312PCL
	case 0x1413:
		combo_clk.AddString("39.6MHz");
		combo_clk.AddString("66MHz");
		combo_clk.AddString("84.8MHz");
		break;
		//STC-SPB500PCL
	case 0x1414:
		combo_clk.AddString("39.6MHz");
		combo_clk.AddString("66MHz");
		combo_clk.AddString("84.8MHz");
		break;
	}

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
void CDlgInitSTCSPB::LoadComParam()
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
void CDlgInitSTCSPB::LoadRegiParam()
{
	// TODO: ここに実装コードを追加します.

	//基準クロック周波数
	combo_clk.SetCurSel(LoadRegiClk());

	//tap数
	combo_tap.SetCurSel(LoadRegiTap() - 1);

	//露光時間
	int load_msec = LoadRegiTime();	//*ms
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
void CDlgInitSTCSPB::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.

	//通信ポート
	regi_param->com_port = combo_com_port.GetCurSel() + 1;

	//基準クロック周波数
	SetRegiClk(combo_clk.GetCurSel());

	//tap数
	SetRegiTap(combo_tap.GetCurSel() + 1);

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
int CDlgInitSTCSPB::CheckParam()
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
 ** 基準クロック周波数パラメータ取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int　基準クロック周波数(0-2)
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitSTCSPB::LoadRegiClk()
{
	// TODO: ここに実装コードを追加します.

	//基準クロック周波数
	int clk = 0;
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		clk = regi_param->spb43pcl.clk;
		break;
		//STC-SPB163PCL
	case 0x1412:
		clk = regi_param->spb163pcl.clk;
		break;
		//STC-SPB312PCL
	case 0x1413:
		clk = regi_param->spb312pcl.clk;
		break;
		//STC-SPB500PCL
	case 0x1414:
		clk = regi_param->spb500pcl.clk;
		break;
	}

	return clk;
}


/** ****************************************************** **
 ** tap数パラメータ取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int　tap数(1-3)
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitSTCSPB::LoadRegiTap()
{
	// TODO: ここに実装コードを追加します.

	//tap数
	int tap = 2;
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		tap = regi_param->spb43pcl.tap;
		break;
		//STC-SPB163PCL
	case 0x1412:
		tap = regi_param->spb163pcl.tap;
		break;
		//STC-SPB312PCL
	case 0x1413:
		tap = regi_param->spb312pcl.tap;
		break;
		//STC-SPB500PCL
	case 0x1414:
		tap = regi_param->spb500pcl.tap;
		break;
	}

	return tap;
}


/** ****************************************************** **
 ** 露光時間パラメータ取得
 ** 引数：
 **　なし
 ** 戻り値：
 **　int　露光時間(1-1000)
 ** 特記事項：
 ** ****************************************************** **/
int CDlgInitSTCSPB::LoadRegiTime()
{
	// TODO: ここに実装コードを追加します.

	//露光時間
	int exposure_time = 8;
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		exposure_time = regi_param->spb43pcl.exposure_time / 1000;
		break;
		//STC-SPB163PCL
	case 0x1412:
		exposure_time = regi_param->spb163pcl.exposure_time / 1000;
		break;
		//STC-SPB312PCL
	case 0x1413:
		exposure_time = regi_param->spb312pcl.exposure_time / 1000;
		break;
		//STC-SPB500PCL
	case 0x1414:
		exposure_time = regi_param->spb500pcl.exposure_time / 1000;
		break;
		//STC-MBS43U3V
	case 0x2411:
		exposure_time = regi_param->mbs43u3v.exposure_time / 1000;
		break;
		//STC-MBS163U3V
	case 0x2412:
		exposure_time = regi_param->mbs163u3v.exposure_time / 1000;
		break;
		//STC-MBS322U3V
	case 0x2413:
		exposure_time = regi_param->mbs322u3v.exposure_time / 1000;
		break;
		//STC-MBS510U3V
	case 0x2414:
		exposure_time = regi_param->mbs510u3v.exposure_time / 1000;
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
int CDlgInitSTCSPB::LoadRegiFlip()
{
	// TODO: ここに実装コードを追加します.

	//画像反転
	int flip = 0;
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		flip = regi_param->spb43pcl.flip;
		break;
		//STC-SPB163PCL
	case 0x1412:
		flip = regi_param->spb163pcl.flip;
		break;
		//STC-SPB312PCL
	case 0x1413:
		flip = regi_param->spb312pcl.flip;
		break;
		//STC-SPB500PCL
	case 0x1414:
		flip = regi_param->spb500pcl.flip;
		break;
		//STC-MBS43U3V
	case 0x2411:
		flip = regi_param->mbs43u3v.flip;
		break;
		//STC-MBS163U3V
	case 0x2412:
		flip = regi_param->mbs163u3v.flip;
		break;
		//STC-MBS322U3V
	case 0x2413:
		flip = regi_param->mbs322u3v.flip;
		break;
		//STC-MBS510U3V
	case 0x2414:
		flip = regi_param->mbs510u3v.flip;
		break;
	}

	return flip;
}


/** ****************************************************** **
 ** 基準クロック周波数パラメータセット
 ** 引数：
 **　int clk	基準クロック周波数(0-2)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSTCSPB::SetRegiClk(int clk)
{
	// TODO: ここに実装コードを追加します.

	//基準クロック周波数
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		regi_param->spb43pcl.clk = clk;
		break;
		//STC-SPB163PCL
	case 0x1412:
		regi_param->spb163pcl.clk = clk;
		break;
		//STC-SPB312PCL
	case 0x1413:
		regi_param->spb312pcl.clk = clk;
		break;
		//STC-SPB500PCL
	case 0x1414:
		regi_param->spb500pcl.clk = clk;
		break;
	}
}


/** ****************************************************** **
 ** tap数パラメータセット
 ** 引数：
 **　int tap	tap数(1-3)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSTCSPB::SetRegiTap(int tap)
{
	// TODO: ここに実装コードを追加します.

	//tap数
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		regi_param->spb43pcl.tap = tap;
		break;
		//STC-SPB163PCL
	case 0x1412:
		regi_param->spb163pcl.tap = tap;
		break;
		//STC-SPB312PCL
	case 0x1413:
		regi_param->spb312pcl.tap = tap;
		break;
		//STC-SPB500PCL
	case 0x1414:
		regi_param->spb500pcl.tap = tap;
		break;
	}
}


/** ****************************************************** **
 ** 露光時間パラメータセット
 ** 引数：
 **　int exposure_time	露光時間(1-1000)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSTCSPB::SetRegiTime(int exposure_time)
{
	// TODO: ここに実装コードを追加します.

	//露光時間
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		regi_param->spb43pcl.exposure_time = exposure_time * 1000;
		break;
		//STC-SPB163PCL
	case 0x1412:
		regi_param->spb163pcl.exposure_time = exposure_time * 1000;
		break;
		//STC-SPB312PCL
	case 0x1413:
		regi_param->spb312pcl.exposure_time = exposure_time * 1000;
		break;
		//STC-SPB500PCL
	case 0x1414:
		regi_param->spb500pcl.exposure_time = exposure_time * 1000;
		break;
		//STC-MBS43U3V
	case 0x2411:
		regi_param->mbs43u3v.exposure_time = exposure_time * 1000;
		break;
		//STC-MBS163U3V
	case 0x2412:
		regi_param->mbs163u3v.exposure_time = exposure_time * 1000;
		break;
		//STC-MBS322U3V
	case 0x2413:
		regi_param->mbs322u3v.exposure_time = exposure_time * 1000;
		break;
		//STC-MBS510U3V
	case 0x2414:
		regi_param->mbs510u3v.exposure_time = exposure_time * 1000;
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
void CDlgInitSTCSPB::SetRegiFlip(int flip)
{
	// TODO: ここに実装コードを追加します.

	//画像反転
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		regi_param->spb43pcl.flip = flip;
		break;
		//STC-SPB163PCL
	case 0x1412:
		regi_param->spb163pcl.flip = flip;
		break;
		//STC-SPB312PCL
	case 0x1413:
		regi_param->spb312pcl.flip = flip;
		break;
		//STC-SPB500PCL
	case 0x1414:
		regi_param->spb500pcl.flip = flip;
		break;
		//STC-MBS43U3V
	case 0x2411:
		regi_param->mbs43u3v.flip = flip;
		break;
		//STC-MBS163U3V
	case 0x2412:
		regi_param->mbs163u3v.flip = flip;
		break;
		//STC-MBS322U3V
	case 0x2413:
		regi_param->mbs322u3v.flip = flip;
		break;
		//STC-MBS510U3V
	case 0x2414:
		regi_param->mbs510u3v.flip = flip;
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
void CDlgInitSTCSPB::SetParam(int number, CCameraRegister* edit_param, WORD model)
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
void CDlgInitSTCSPB::OnBnClickedButton1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	//パラメータ初期化
	switch (model_code) {
		//STC-SPB43PCL
	case 0x1411:
		regi_param->InitSPB43PCL();
		break;
		//STC-SPB163PCL
	case 0x1412:
		regi_param->InitSPB163PCL();
		break;
		//STC-SPB312PCL
	case 0x1413:
		regi_param->InitSPB312PCL();
		break;
		//STC-SPB500PCL
	case 0x1414:
		regi_param->InitSPB500PCL();
		break;
		//STC-MBS43U3V
	case 0x2411:
		regi_param->InitMBS43U3V();
		break;
		//STC-MBS163U3V
	case 0x2412:
		regi_param->InitMBS163U3V();
		break;
		//STC-MBS322U3V
	case 0x2413:
		regi_param->InitMBS322U3V();
		break;
		//STC-MBS510U3V
	case 0x2414:
		regi_param->InitMBS510U3V();
		break;
	}

	LoadRegiParam();	//内部パラメータの展開
}


/** ****************************************************** **
 ** OKボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgInitSTCSPB::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	this->GetWindowTextA(title);

	//パラメータ確認
	int rtn_chk = CheckParam();
	if (rtn_chk == 0) {
		ChangeParameter();	//パラメータの変更

		outlog.LogOutFile(1, "*** DlgInitSTCSPB out (OK) ***");

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
void CDlgInitSTCSPB::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgInitSTCSPB out (CANCEL) ***");

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
BOOL CDlgInitSTCSPB::PreTranslateMessage(MSG* pMsg)
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
