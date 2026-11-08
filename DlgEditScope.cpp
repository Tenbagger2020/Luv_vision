/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgEditScope.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.11.10	J.Murata	英語化対応
 **			2022.05.19	M.Iwasaki	言語切替修正
 **			2024.07.16	M.Iwasaki	10.18.4.6	ブロブ機能拡張
 ** 注記：
 ** 説明：スコープデータ編集ダイアログ
 **		キャリブレーションで算出したスコープデータを編集する
 **/

// DlgEditScope.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgEditScope.h"
#include "afxdialogex.h"

#include "AccessParamScrp.h"	//機種毎スコープパラメータファイルアクセス
#include "CurrentStatus.h"		//現状共通データ

extern CScopeParam	scope_param[16];	//機種毎スコープパラメータ
extern CAccessParamScrp	access_scp;		//機種毎スコープパラメータファイルアクセス

extern CCurrentStatus	current;		//共有情報
extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

// CDlgEditScope ダイアログ

IMPLEMENT_DYNAMIC(CDlgEditScope, CDialogEx)

CDlgEditScope::CDlgEditScope(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgEditScope, pParent)
{
	select_scope = 0;	//初期スコープ
	select_row = 0;		//初期行
}

CDlgEditScope::~CDlgEditScope()
{
}

void CDlgEditScope::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_ITEM, str_item);
	DDX_Control(pDX, IDC_EDIT1, edit_set_view[0]);
	DDX_Control(pDX, IDC_EDIT2, edit_set_view[1]);
	DDX_Control(pDX, IDC_EDIT3, edit_set_view[2]);
	DDX_Control(pDX, IDC_EDIT4, edit_set_view[3]);
	DDX_Control(pDX, IDC_STATIC_1, static_view1);
	DDX_Control(pDX, IDC_STATIC_2, static_view2);
	DDX_Control(pDX, IDC_STATIC_3, static_view3);
	DDX_Control(pDX, IDC_STATIC_4, static_view4);
	DDX_Control(pDX, IDOK, btn_set);
	DDX_Control(pDX, IDCANCEL, btn_cancel);
}


BEGIN_MESSAGE_MAP(CDlgEditScope, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgEditScope::OnBnClickedOk)
END_MESSAGE_MAP()


// CDlgEditScope メッセージ ハンドラー


BOOL CDlgEditScope::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgEditScope in ***");

	dlg_x = 352;
	dlg_y = 336;
	dlg_width = 464;
	dlg_height = 192;
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
void CDlgEditScope::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.
	ChangeLanguage();

	//項目名
	CString item;
	if (select_row == 0) item = Tips0.SelStr(language.jpn, "画素サイズ[um]", "Pixel Size[um]");
	if (select_row == 1) item = Tips0.SelStr(language.jpn, "視野傾き[deg]", "View Angle[deg]");
	if (select_row == 2) item = Tips0.SelStr(language.jpn, "Cal基準X[pixel]", "Cal Base X[pixel]");
	if (select_row == 3) item = Tips0.SelStr(language.jpn, "Cal基準Y[pixel]", "Cal Base Y[pixel]");
	if (select_row == 4) item = Tips0.SelStr(language.jpn, "Cal基準位置X[mm]", "Cal Base Pos X[mm]");
	if (select_row == 5) item = Tips0.SelStr(language.jpn, "Cal基準位置Y[mm]", "Cal Base Pos Y[mm]");
	if (select_row == 6) item = Tips0.SelStr(language.jpn, "回転中心X[mm]", "Rot Center X[mm]");
	if (select_row == 7) item = Tips0.SelStr(language.jpn, "回転中心Y[mm]", "Rot Center Y[mm]");

	if (select_row >= 0 && select_row <= 7) str_item.SetWindowTextA(item);
}


/** ****************************************************** **
 ** 言語変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgEditScope::ChangeLanguage()
{
	if (!language.jpn) {
		this->SetWindowTextA(language.GetCharacterString(2301));
		static_view1.SetWindowTextA(language.GetCharacterString(2302));	//視野1
		static_view2.SetWindowTextA(language.GetCharacterString(2303));	//視野2
		static_view3.SetWindowTextA(language.GetCharacterString(2304));	//視野3
		static_view4.SetWindowTextA(language.GetCharacterString(2305));	//視野4
		btn_set.SetWindowTextA(language.GetCharacterString(2306));		//更新ボタン
		btn_cancel.SetWindowTextA(language.GetCharacterString(2307));	//キャンセルボタン
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
void CDlgEditScope::LoadParameter()
{
	// TODO: ここに実装コードを追加します.
	if (select_scope < 0 || select_scope > 15) return;
	if (select_row < 0 || select_row > 7) return;

	CScopeParam* param = &scope_param[select_scope];

	//視野1～4
	for (int i = 0; i < 4; i++) {
		CString str_data;
		char p_data[16] = { '\0' };

		//画素サイズ表示
		if (select_row == 0) {
			double pixel_um = param->view[i].calib.pixel_size * (double)1000;
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(pixel_um, 2, TRUE));

			//画素サイズ固定化確認
			if (access_scp.GetdecidedPixel(select_scope, i)) {
				edit_set_view[i].EnableWindow(FALSE);
			}
		}
		//視野傾き表示
		else if (select_row == 1) {
			double deg = Tips0.RadianToDeg(param->view[i].calib.crdnt_radian);
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(deg, 4, TRUE));
		}
		//Cal基準X表示
		else if (select_row == 2) {
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.pixel.cal_base_x, 4, TRUE));
		}
		//Cal基準Y表示
		else if (select_row == 3) {
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.pixel.cal_base_y, 4, TRUE));
		}
		//Cal基準位置X表示
		else if (select_row == 4) {
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.pixel.axis_base_x, 4, TRUE));
		}
		//Cal基準位置Y表示
		else if (select_row == 5) {
			strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.pixel.axis_base_y, 4, TRUE));
		}
		//回転中心X表示
		else if (select_row == 6) {
			if (param->conf_rotcent_view == i) {
				strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.theta.rotation_center_x, 4, TRUE));
			}
			else {
				p_data[0] = '\0';
			}
		}
		//回転中心Y表示
		else if (select_row == 7) {
			if (param->conf_rotcent_view == i) {
				strcpy_s(p_data, sizeof(p_data), Tips0.DblToString(param->view[i].calib.theta.rotation_center_y, 4, TRUE));
			}
			else {
				p_data[0] = '\0';
			}
		}
		edit_set_view[i].SetWindowTextA(p_data);
	}
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDlgEditScope::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (select_scope < 0 || select_scope > 15) goto END;
	if (select_row < 0 || select_row > 7) goto END;

	CScopeParam* param = &scope_param[select_scope];

	char buf[256] = { '\0' };

	//視野1～4取得
	for (int i = 0; i < 4; i++) {
		edit_set_view[i].GetWindowTextA(buf, sizeof(buf));
		double data = atof(buf);

		if (param->view[i].calib.pixel_complete) {
			//画素サイズ表示
			if (select_row == 0) {
				param->view[i].calib.pixel_size = data / 1000;
			}
			//視野傾き表示
			else if (select_row == 1) {
				param->view[i].calib.crdnt_radian = Tips0.DegToRadian(data);
			}
			//Cal基準X表示
			else if (select_row == 2) {
				param->view[i].calib.pixel.cal_base_x = data;
			}
			//Cal基準Y表示
			else if (select_row == 3) {
				param->view[i].calib.pixel.cal_base_y = data;
			}
			//Cal基準位置X表示
			else if (select_row == 4) {
				param->view[i].calib.pixel.axis_base_x = data;
			}
			//Cal基準位置Y表示
			else if (select_row == 5) {
				param->view[i].calib.pixel.axis_base_y = data;
			}
		}
		if (param->view[i].calib.theta_complete) {
			//回転中心X表示
			if (select_row == 6) {
				if (param->conf_rotcent_view == i) {
					param->view[i].calib.theta.rotation_center_x = data;
					param->conf_rotcent_x = data;
				}
			}
			//回転中心Y表示
			else if (select_row == 7) {
				if (param->conf_rotcent_view == i) {
					param->view[i].calib.theta.rotation_center_y = data;
					param->conf_rotcent_y = data;
				}
			}
		}
	}

	//Iniファイル書き込み
	if (access_scp.CheckIniFileParam(current.work_number, select_scope) == 0) {
		//iniファイル存在
		access_scp.WriteIniFileParam(current.work_number, select_scope);	//iniファイル更新

		rtn_result = 0;		//保存成功
	}

END:
	return rtn_result;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　int scope	編集スコープ(0～15)
 **　int row	編集行(0～)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgEditScope::SetParam(int scope, int row)
{
	select_scope = scope;
	select_row = row;

	//エディットコントロール設定
	for (int i = 0; i < 4; i++) {
		//コントロール設定
		edit_set_view[i].SetFuncLBtnDblClk(OPEN_TKY);
		if (select_row == 0) {
			edit_set_view[i].SetTenkeyDbl(2, 0.01, 9999.99);
		}
		else {
			edit_set_view[i].SetTenkeyDbl(4, -9999.9999, 9999.9999);
		}
	}
}


/** ****************************************************** **
 ** 更新ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgEditScope::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);
	msg = Tips0.SelStr(language.jpn, "データを更新します。よろしいですか？", "Update the data. Is it OK?");

	int judge = MessageBoxA(msg, title, MB_OKCANCEL | MB_ICONQUESTION);
	if (judge == IDOK) {

		//パラメータ更新とデータ保存
		if (ChangeParameter() != 0) {
			MessageBoxA(Tips0.SelStr(language.jpn,
				"データファイルが存在しません\nキャリブレーションを実施してください",
				"The data file does not exist\nPlease calibrate"),
				title, MB_OK | MB_ICONERROR);
			return;
		}

		outlog.LogOutFile(1, "*** DlgEditScope out (OK) ***");

		CDialogEx::OnOK();
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
BOOL CDlgEditScope::PreTranslateMessage(MSG* pMsg)
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
