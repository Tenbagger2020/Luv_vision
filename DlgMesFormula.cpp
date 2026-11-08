/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DlgMesFormula.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：検査シート計算設定ダイアログ
 **		各シートの検査結果をもとに計算・判定する
 **
 **/

// DlgMesFormula.cpp : 実装ファイル
//

#include "stdafx.h"
#include "Luv_Vision.h"
#include "DlgMesFormula.h"
#include "afxdialogex.h"

extern CDefinedElementMtd	element_mtd;		//機能要素

extern CCurrentStatus	current;		//共有情報
extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

// CDlgMesFormula ダイアログ

IMPLEMENT_DYNAMIC(CDlgMesFormula, CDialogEx)

CDlgMesFormula::CDlgMesFormula(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DlgMesFormula, pParent)
{
	sheet_number = 0;

	//コントロール設定
	for (int i = 0; i < 3; i++) {
		edit_input_val[i].SetFuncLBtnDblClk(OPEN_TKY);
		edit_input_val[i].SetTenkeyDbl(4, -99999.9999, 99999.9999);
	}
	edit_judge_min.SetFuncLBtnDblClk(OPEN_TKY);
	edit_judge_min.SetTenkeyDbl(4, -99999.9999, 99999.9999);
	edit_judge_max.SetFuncLBtnDblClk(OPEN_TKY);
	edit_judge_max.SetTenkeyDbl(4, -99999.9999, 99999.9999);
}

CDlgMesFormula::~CDlgMesFormula()
{
}

void CDlgMesFormula::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	//計算1
	DDX_Control(pDX, IDC_CHECK1, check_set_enable[0]);		//計算有効チェック
	DDX_Control(pDX, IDC_COMBO1, combo_set_sheet[0][0]);	//第一要素 シート選択
	DDX_Control(pDX, IDC_COMBO2, combo_set_method[0][0]);	//第一要素 機能選択
	DDX_Control(pDX, IDC_COMBO3, combo_set_var[0][0]);		//第一要素 計算値選択
	DDX_Control(pDX, IDC_COMBO4, combo_set_opera[0]);		//演算子選択
	DDX_Control(pDX, IDC_COMBO5, combo_set_sheet[0][1]);	//第二要素 シート選択
	DDX_Control(pDX, IDC_COMBO6, combo_set_method[0][1]);	//第二要素 機能選択
	DDX_Control(pDX, IDC_COMBO7, combo_set_var[0][1]);		//第二要素 計算値選択
	DDX_Control(pDX, IDC_CHECK5, check_enable_val[0]);		//第二要素 値有効チェック
	DDX_Control(pDX, IDC_EDIT1, edit_input_val[0]);			//第二要素 数値入力
	//計算2
	DDX_Control(pDX, IDC_CHECK2, check_set_enable[1]);		//計算有効チェック
	DDX_Control(pDX, IDC_COMBO9, combo_set_sheet[1][0]);	//第一要素 シート選択
	DDX_Control(pDX, IDC_COMBO10, combo_set_method[1][0]);	//第一要素 機能選択
	DDX_Control(pDX, IDC_COMBO11, combo_set_var[1][0]);		//第一要素 計算値選択
	DDX_Control(pDX, IDC_COMBO12, combo_set_opera[1]);		//演算子選択
	DDX_Control(pDX, IDC_COMBO13, combo_set_sheet[1][1]);	//第二要素 シート選択
	DDX_Control(pDX, IDC_COMBO14, combo_set_method[1][1]);	//第二要素 機能選択
	DDX_Control(pDX, IDC_COMBO15, combo_set_var[1][1]);		//第二要素 計算値選択
	DDX_Control(pDX, IDC_CHECK6, check_enable_val[1]);		//第二要素 値有効チェック
	DDX_Control(pDX, IDC_EDIT2, edit_input_val[1]);			//第二要素 数値入力
	//計算3
	DDX_Control(pDX, IDC_CHECK3, check_set_enable[2]);		//計算有効チェック
	DDX_Control(pDX, IDC_COMBO17, combo_set_sheet[2][0]);	//第一要素 シート選択
	DDX_Control(pDX, IDC_COMBO18, combo_set_method[2][0]);	//第一要素 機能選択
	DDX_Control(pDX, IDC_COMBO19, combo_set_var[2][0]);		//第一要素 計算値選択
	DDX_Control(pDX, IDC_COMBO20, combo_set_opera[2]);		//演算子選択
	DDX_Control(pDX, IDC_COMBO21, combo_set_sheet[2][1]);	//第二要素 シート選択
	DDX_Control(pDX, IDC_COMBO22, combo_set_method[2][1]);	//第二要素 機能選択
	DDX_Control(pDX, IDC_COMBO23, combo_set_var[2][1]);		//第二要素 計算値選択
	DDX_Control(pDX, IDC_CHECK7, check_enable_val[2]);		//第二要素 値有効チェック
	DDX_Control(pDX, IDC_EDIT3, edit_input_val[2]);			//第二要素 数値入力
	//判定
	DDX_Control(pDX, IDC_CHECK4, check_judge_enable);	//判定有効チェック
	DDX_Control(pDX, IDC_COMBO25, combo_judge_calc);	//判定選択
	DDX_Control(pDX, IDC_EDIT4, edit_judge_min);		//判定min入力
	DDX_Control(pDX, IDC_EDIT5, edit_judge_max);		//判定max入力
}


BEGIN_MESSAGE_MAP(CDlgMesFormula, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDlgMesFormula::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDlgMesFormula::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHECK1, &CDlgMesFormula::OnBnClickedCheck1)
	ON_BN_CLICKED(IDC_CHECK2, &CDlgMesFormula::OnBnClickedCheck2)
	ON_BN_CLICKED(IDC_CHECK3, &CDlgMesFormula::OnBnClickedCheck3)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDlgMesFormula::OnCbnSelchangeCombo1)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CDlgMesFormula::OnCbnSelchangeCombo2)
	ON_CBN_SELCHANGE(IDC_COMBO5, &CDlgMesFormula::OnCbnSelchangeCombo5)
	ON_CBN_SELCHANGE(IDC_COMBO6, &CDlgMesFormula::OnCbnSelchangeCombo6)
	ON_CBN_SELCHANGE(IDC_COMBO9, &CDlgMesFormula::OnCbnSelchangeCombo9)
	ON_CBN_SELCHANGE(IDC_COMBO10, &CDlgMesFormula::OnCbnSelchangeCombo10)
	ON_CBN_SELCHANGE(IDC_COMBO13, &CDlgMesFormula::OnCbnSelchangeCombo13)
	ON_CBN_SELCHANGE(IDC_COMBO14, &CDlgMesFormula::OnCbnSelchangeCombo14)
	ON_CBN_SELCHANGE(IDC_COMBO17, &CDlgMesFormula::OnCbnSelchangeCombo17)
	ON_CBN_SELCHANGE(IDC_COMBO18, &CDlgMesFormula::OnCbnSelchangeCombo18)
	ON_CBN_SELCHANGE(IDC_COMBO21, &CDlgMesFormula::OnCbnSelchangeCombo21)
	ON_CBN_SELCHANGE(IDC_COMBO22, &CDlgMesFormula::OnCbnSelchangeCombo22)
	ON_BN_CLICKED(IDC_CHECK4, &CDlgMesFormula::OnBnClickedCheck4)
	ON_BN_CLICKED(IDC_CHECK5, &CDlgMesFormula::OnBnClickedCheck5)
	ON_BN_CLICKED(IDC_CHECK6, &CDlgMesFormula::OnBnClickedCheck6)
	ON_BN_CLICKED(IDC_CHECK7, &CDlgMesFormula::OnBnClickedCheck7)
END_MESSAGE_MAP()


// CDlgMesFormula メッセージ ハンドラー


BOOL CDlgMesFormula::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO: ここに初期化を追加してください
	outlog.LogOutFile(1, "'");
	outlog.LogOutFile(1, "*** DlgMesFormula in ***");

	dlg_x = 496;
	dlg_y = 160;
	dlg_width = 496;
	dlg_height = 544;
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
void CDlgMesFormula::ControlInitialization()
{
	// TODO: ここに実装コードを追加します.

	//要素コンボボックス
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 2; j++) {
			//シート選択
			for (int k = 0; k < 8; k++) {
				combo_set_sheet[i][j].AddString("シート" + Tips0.IntToString(k));
			}
			//機能選択
			int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
			for (int k = 0; k < method_num; k++) {
					combo_set_method[i][j].AddString(element_mtd.mes[k].s_text);
			}
		}
	}
	
	//演算子コンボボックス
	for (int i = 0; i < 3; i++) {
		combo_set_opera[i].AddString("＋");
		combo_set_opera[i].AddString("－");
		combo_set_opera[i].AddString("×");
		combo_set_opera[i].AddString("÷");
	}

	//判定コンボボックス
	for (int k = 0; k < 3; k++) {
		combo_judge_calc.AddString("計算" + Tips0.IntToString(k + 1));	//判定選択
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
void CDlgMesFormula::LoadParameter()
{
	// TODO: ここに実装コードを追加します.

	//要素コンボボックス
	for (int i = 0; i < 3; i++) {
		CFormulaParam::Element1* p_elem1 = &sheet_param.mes_formula.formula[i].elem1;
		CFormulaParam::Element2* p_elem2 = &sheet_param.mes_formula.formula[i].elem2;

		//第一要素 コンボボックス
		int sheet_num1 = p_elem1->sheet_num;
		if (sheet_num1 > -1) {
			combo_set_sheet[i][0].SetCurSel(sheet_num1);
			UpdateCmbMethod(&combo_set_method[i][0], p_elem1->type);
			UpdateCmbVar(&combo_set_var[i][0], p_elem1->type, p_elem1->variable);
		}
		else {
			combo_set_sheet[i][0].SetWindowTextA("シート選択");
			combo_set_method[i][0].SetWindowTextA("");
			combo_set_var[i][0].SetWindowTextA("");
		}

		//第二要素 コンボボックス
		int sheet_num2 = p_elem2->sheet_num;
		if (sheet_num2 > -1) {
			combo_set_sheet[i][1].SetCurSel(sheet_num2);
			UpdateCmbMethod(&combo_set_method[i][1], p_elem2->type);
			UpdateCmbVar(&combo_set_var[i][1], p_elem2->type, p_elem2->variable);
		}
		else {
			combo_set_sheet[i][1].SetWindowTextA("シート選択");
			combo_set_method[i][1].SetWindowTextA("");
			combo_set_var[i][1].SetWindowTextA("");
		}
		check_enable_val[i].SetCheck(p_elem2->enable_val);
		edit_input_val[i].SetWindowTextA(Tips0.DblToString(p_elem2->val_d, 4, TRUE));

		//演算子
		combo_set_opera[i].SetCurSel((int)sheet_param.mes_formula.formula[i].operator_item);

		//計算式有効無効
		BOOL enable = sheet_param.mes_formula.formula[i].enable;
		check_set_enable[i].SetCheck(enable);
		EnableFormula(i, enable);
	}

	//判定
	CFormulaParam::Judgement* p_judge = &sheet_param.mes_formula.judge;
	check_judge_enable.SetCheck(p_judge->enable);
	combo_judge_calc.SetCurSel(p_judge->judge_calc);
	edit_judge_min.SetWindowTextA(Tips0.DblToString(p_judge->judge_min, 4, TRUE));
	edit_judge_max.SetWindowTextA(Tips0.DblToString(p_judge->judge_max, 4, TRUE));
	EnableJudgement(p_judge->enable);
}


/** ****************************************************** **
 ** パラメータの変更
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::ChangeParameter()
{
	// TODO: ここに実装コードを追加します.
	char buf[32] = { '\0' };

	sheet_param.mes_formula.InitialParam();	//パラメータ初期化

	//要素コンボボックス
	for (int i = 0; i < 3; i++) {
		CFormulaParam::Element1* p_elem1 = &sheet_param.mes_formula.formula[i].elem1;
		CFormulaParam::Element2* p_elem2 = &sheet_param.mes_formula.formula[i].elem2;

		//第一要素 コンボボックス
		int sheet_num1 = combo_set_sheet[i][0].GetCurSel();
		if (sheet_num1 > -1) {
			p_elem1->sheet_num = sheet_num1;

			combo_set_method[i][0].GetWindowTextA(buf, sizeof(buf));
			int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
			for (int j = 0; j < method_num; j++) {
				if (element_mtd.mes[j].s_text == (CString)buf) p_elem1->type = element_mtd.mes[j].type;
			}

			combo_set_var[i][0].GetWindowTextA(buf, sizeof(buf));
			int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
			for (int k = 0; k < var_num; k++) {
				if (element_mtd.var[k].type == p_elem1->type) {
					if (element_mtd.var[k].s_text == (CString)buf) p_elem1->variable = element_mtd.var[k].variable;
				}
			}
		}

		//第二要素 コンボボックス
		int sheet_num2 = combo_set_sheet[i][1].GetCurSel();
		if (sheet_num2 > -1) {
			p_elem2->sheet_num = sheet_num2;

			combo_set_method[i][1].GetWindowTextA(buf, sizeof(buf));
			int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
			for (int j = 0; j < method_num; j++) {
				if (element_mtd.mes[j].s_text == (CString)buf) p_elem2->type = element_mtd.mes[j].type;
			}

			combo_set_var[i][1].GetWindowTextA(buf, sizeof(buf));
			int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
			for (int k = 0; k < var_num; k++) {
				if (element_mtd.var[k].type == p_elem2->type) {
					if (element_mtd.var[k].s_text == (CString)buf) p_elem2->variable = element_mtd.var[k].variable;
				}
			}
		}
		p_elem2->enable_val = check_enable_val[i].GetCheck();	//値有効
		edit_input_val[i].GetWindowTextA(buf, sizeof(buf));
		double input_val = atof(buf);
		CString str_input_val = Tips0.DblToString(input_val, 4, FALSE);
		p_elem2->val_d = atof((LPCSTR)str_input_val);		//値

		//演算子
		sheet_param.mes_formula.formula[i].operator_item = (byte)combo_set_opera[i].GetCurSel();
		
		//計算式有効無効
		sheet_param.mes_formula.formula[i].enable = check_set_enable[i].GetCheck();
	}

	//判定
	CFormulaParam::Judgement* p_judge = &sheet_param.mes_formula.judge;
	p_judge->enable = check_judge_enable.GetCheck();
	p_judge->judge_calc = combo_judge_calc.GetCurSel();
	edit_judge_min.GetWindowTextA(buf, sizeof(buf));
	double judge_min = atof(buf);
	CString str_judge_min = Tips0.DblToString(judge_min, 4, FALSE);
	p_judge->judge_min = atof((LPCSTR)str_judge_min);		//判定min
	edit_judge_max.GetWindowTextA(buf, sizeof(buf));
	double judge_max = atof(buf);
	CString str_judge_max = Tips0.DblToString(judge_max, 4, FALSE);
	p_judge->judge_max = atof((LPCSTR)str_judge_max);		//判定max
}


/** ****************************************************** **
 ** パラメータの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果
 ** 特記事項：
 ** ****************************************************** **/
int CDlgMesFormula::CheckParam(char* err_msg)
{
	// TODO: ここに実装コードを追加します.
	char buf[32] = { '\0' };
	byte code = 0x00;

	//計算式チェック
	for (int i = 0; i < 3; i++) {
		if (check_set_enable[i].GetCheck()) {
			//第一要素
			if (combo_set_sheet[i][0].GetCurSel() < 0) {
				code = 0x01 * (i + 1);	//シート未選択
				break;
			}
			byte type1 = 0x00;
			combo_set_method[i][0].GetWindowTextA(buf, sizeof(buf));
			int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
			BOOL err_method = TRUE;
			for (int j = 0; j < method_num; j++) {
				if (element_mtd.mes[j].s_text == (CString)buf) {
					err_method = FALSE;
					type1 = element_mtd.mes[j].type;
				}
			}
			if (err_method) {
				code = 0x01 * (i + 1);		//機能未選択
				break;
			}
			combo_set_var[i][0].GetWindowTextA(buf, sizeof(buf));
			int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
			BOOL err_var = TRUE;
			for (int k = 0; k < var_num; k++) {
				if (element_mtd.var[k].type == type1) {
					if (element_mtd.var[k].s_text == (CString)buf) err_var = FALSE;
				}
			}
			if (err_var) {
				code = 0x01 * (i + 1);		//要素未選択
				break;
			}

			//第二要素
			BOOL select_sht = (combo_set_sheet[i][1].GetCurSel() >= 0);		//シート要素選択フラグ
			if (select_sht) {
				byte type2 = 0x00;
				combo_set_method[i][1].GetWindowTextA(buf, sizeof(buf));
				int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
				BOOL err_method = TRUE;
				for (int j = 0; j < method_num; j++) {
					if (element_mtd.mes[j].s_text == (CString)buf) {
						err_method = FALSE;
						type2 = element_mtd.mes[j].type;
					}
				}
				if (err_method) {
					code = 0x01 * (i + 1);		//機能未選択
					break;
				}
				combo_set_var[i][1].GetWindowTextA(buf, sizeof(buf));
				int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
				BOOL err_var = TRUE;
				for (int k = 0; k < var_num; k++) {
					if (element_mtd.var[k].type == type2) {
						if (element_mtd.var[k].s_text == (CString)buf) err_var = FALSE;
					}
				}
				if (err_var) {
					code = 0x01 * (i + 1);		//要素未選択
					break;
				}
			}
			BOOL select_deci = check_enable_val[i].GetCheck();		//数値要素選択フラグ
			if (select_deci) {
				edit_input_val[i].GetWindowTextA(buf, sizeof(buf));
				if (strlen(buf) == 0) {
					code = 0x01 * (i + 1);		//数値未入力
					break;
				}
			}
			if (!select_sht && !select_deci) {
				code = 0x01 * (i + 1);			//シート&数値未選択
				break;
			}
		}
	}

	//判定チェック
	if (code == 0x00 && check_judge_enable.GetCheck()) {
		edit_judge_min.GetWindowTextA(buf, sizeof(buf));
		if (strlen(buf) == 0) code = 0x04;
		edit_judge_max.GetWindowTextA(buf, sizeof(buf));
		if (strlen(buf) == 0) code = 0x04;
	}

	if (code != 0x00) {
		if (code == 0x01) strcpy_s(err_msg, 256, "計算1の設定が未完了");
		if (code == 0x02) strcpy_s(err_msg, 256, "計算2の設定が未完了");
		if (code == 0x03) strcpy_s(err_msg, 256, "計算3の設定が未完了");
		if (code == 0x04) strcpy_s(err_msg, 256, "判定の設定が未完了");
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 機能選択コンボボックスの更新
 ** 引数：
 **　CComboBox* box		コンボボックスのポインタ
 **　byte type			検査機能
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::UpdateCmbMethod(CComboBox* box, byte type)
{
	// TODO: ここに実装コードを追加します.

	//機能更新
	int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
	CString method_text = "";
	for (int i = 0; i < method_num; i++) {
		if (element_mtd.mes[i].type == type) method_text = element_mtd.mes[i].s_text;
	}
	box->SetWindowTextA(method_text);
}


/** ****************************************************** **
 ** 計算値選択コンボボックスの更新
 ** 引数：
 **　CComboBox* box		コンボボックスのポインタ
 **　byte type			検査機能
 **　CString variable	要素
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::UpdateCmbVar(CComboBox* box, byte type, CString variable)
{
	// TODO: ここに実装コードを追加します.

	box->ResetContent();

	//要素更新
	int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
	CString var_text = "";
	for (int i = 0; i < var_num; i++) {
		if (element_mtd.var[i].type == type) {
			box->AddString(element_mtd.var[i].s_text);

			if (element_mtd.var[i].variable == variable) var_text = element_mtd.var[i].s_text;
		}
	}
	box->SetWindowTextA(var_text);
}


/** ****************************************************** **
 ** 計算の有効無効化
 ** 引数：
 **　int formula	計算		0～2:計算1～3
 **　BOOL enable	有効無効	TRUE:有効、FALSE:無効
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::EnableFormula(int formula, BOOL enable)
{
	// TODO: ここに実装コードを追加します.

	//第一要素
	combo_set_sheet[formula][0].EnableWindow(enable);
	combo_set_method[formula][0].EnableWindow(enable);
	combo_set_var[formula][0].EnableWindow(enable);

	//第二要素
	combo_set_sheet[formula][1].EnableWindow(enable);
	combo_set_method[formula][1].EnableWindow(enable);
	combo_set_var[formula][1].EnableWindow(enable);
	check_enable_val[formula].EnableWindow(enable);
	edit_input_val[formula].EnableWindow(enable);

	//演算子
	combo_set_opera[formula].EnableWindow(enable);

	//有効化する場合の制限
	if (enable) {
		//第一要素 シートが選択されていない
		if (combo_set_sheet[formula][0].GetCurSel() < 0) {
			combo_set_method[formula][0].EnableWindow(FALSE);
			combo_set_var[formula][0].EnableWindow(FALSE);
		}
		//第二要素 シートが選択されていない
		if (combo_set_sheet[formula][1].GetCurSel() < 0) {
			combo_set_method[formula][1].EnableWindow(FALSE);
			combo_set_var[formula][1].EnableWindow(FALSE);
		}
		//第二要素 値入力が選択されていない
		if (!check_enable_val[formula].GetCheck()) {
				edit_input_val[formula].EnableWindow(FALSE);
		}
	}
}


/** ****************************************************** **
 ** 判定の有効無効化
 ** 引数：
 **　BOOL enable	有効無効	TRUE:有効、FALSE:無効
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::EnableJudgement(BOOL enable)
{
	// TODO: ここに実装コードを追加します.

	combo_judge_calc.EnableWindow(enable);
	edit_judge_min.EnableWindow(enable);
	edit_judge_max.EnableWindow(enable);
}


/** ****************************************************** **
 ** シート選択処理
 ** 引数：
 **　int formula	計算 0～2:計算1～3
 **　int element	要素 0:第一要素、1:第二要素
 ** 戻り値：
 **　CString　要素
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::SelectCmbSheet(int formula, int element)
{
	// TODO: ここに実装コードを追加します.

	//選択シートの検査機能と初期要素を自動で選択させる
	int select_sht = combo_set_sheet[formula][element].GetCurSel();
	byte type = edit_mes->sheet[select_sht].mes_type;

	UpdateCmbMethod(&combo_set_method[formula][element], type);
	UpdateCmbVar(&combo_set_var[formula][element], type, Get1stVariable(type));

	//その他要素コンボボックスの有効化
	combo_set_method[formula][element].EnableWindow(TRUE);
	combo_set_var[formula][element].EnableWindow(TRUE);

	//第二要素では小数点選択を無効化する
	if (element == 1) {
		check_enable_val[formula].SetCheck(FALSE);
		edit_input_val[formula].EnableWindow(FALSE);
	}
}


/** ****************************************************** **
 ** 機能選択処理
 ** 引数：
 **　int formula	計算 0～2:計算1～3
 **　int element	要素 0:第一要素、1:第二要素
 ** 戻り値：
 **　CString　要素
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::SelectCmbMethod(int formula, int element)
{
	// TODO: ここに実装コードを追加します.

	//選択機能の初期要素を自動で選択させる
	CString select_mtd;
	combo_set_method[formula][element].GetLBText(combo_set_method[formula][element].GetCurSel(), select_mtd);

	int method_num = sizeof(element_mtd.mes) / sizeof(element_mtd.mes[0]);
	byte method_type = 0x00;
	for (int i = 0; i < method_num; i++) {
		if (element_mtd.mes[i].s_text == select_mtd) method_type = element_mtd.mes[i].type;
	}

	UpdateCmbVar(&combo_set_var[formula][element], method_type, Get1stVariable(method_type));
}


/** ****************************************************** **
 ** 値選択処理
 ** 引数：
 **　int formula	計算 0～2:計算1～3
 ** 戻り値：
 **　CString　要素
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::SelectCheckEblVal(int formula)
{
	// TODO: ここに実装コードを追加します.

	BOOL enable = check_enable_val[formula].GetCheck();

	//小数点選択を有効化しシート選択要素を無効化する
	edit_input_val[formula].EnableWindow(enable);

	//値有効時 第二要素 シート選択要素の初期化
	if (enable) {
		combo_set_sheet[formula][1].SetCurSel(-1);
		combo_set_sheet[formula][1].SetWindowTextA("シート選択");
		combo_set_method[formula][1].ResetContent();
		combo_set_method[formula][1].EnableWindow(FALSE);
		combo_set_var[formula][1].ResetContent();
		combo_set_var[formula][1].EnableWindow(FALSE);
	}
}


/** ****************************************************** **
 ** 初期要素の取得
 ** 引数：
 **　byte type			検査機能
 ** 戻り値：
 **　CString　要素
 ** 特記事項：
 ** ****************************************************** **/
CString CDlgMesFormula::Get1stVariable(byte type)
{
	// TODO: ここに実装コードを追加します.
	int var_num = sizeof(element_mtd.var) / sizeof(element_mtd.var[0]);
	for (int i = 0; i < var_num; i++) {
		if (type == element_mtd.var[i].type) return element_mtd.var[i].variable;
	}
	return "";
}


/** ****************************************************** **
 ** 作成テンプレート番号の入力/シート番号
 ** 引数：
 **　int template_num	テンプレート番号
 **　int sheet_num		シート番号(0～7)
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::SetNumber(int template_num, int sheet_num)
{
	// TODO: ここに実装コードを追加します.
	template_number = template_num;
	sheet_number = sheet_num;
}


/** ****************************************************** **
 ** パラメータのセット
 ** 引数：
 **　CMesParam* edit_param	編集中検査パラメータのポインタ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::SetParam(CMesParam* edit_param)
{
	//検査パラメータを編集用パラメータへコピー
	edit_mes = edit_param;
	sheet_param = edit_mes->sheet[sheet_number];
}


/** ****************************************************** **
 ** 計算1有効チェックボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	EnableFormula(0, check_set_enable[0].GetCheck());
}


/** ****************************************************** **
 ** 計算2有効チェックボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	EnableFormula(1, check_set_enable[1].GetCheck());
}


/** ****************************************************** **
 ** 計算3有効チェックボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck3()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	EnableFormula(2, check_set_enable[2].GetCheck());
}


/** ****************************************************** **
 ** 判定有効チェックボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck4()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	EnableJudgement(check_judge_enable.GetCheck());
}


/** ****************************************************** **
 ** 第一要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo1()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(0, 0);
}


/** ****************************************************** **
 ** 第一要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo2()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(0, 0);
}


/** ****************************************************** **
 ** 第二要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(0, 1);
}


/** ****************************************************** **
 ** 第二要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(0, 1);
}


/** ****************************************************** **
 ** 第二要素 値有効チェックボックス
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck5()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCheckEblVal(0);
}

/** ****************************************************** **
 ** 第一要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo9()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(1, 0);
}


/** ****************************************************** **
 ** 第一要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo10()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(1, 0);
}


/** ****************************************************** **
 ** 第二要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo13()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(1, 1);
}


/** ****************************************************** **
 ** 第二要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo14()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(1, 1);
}


/** ****************************************************** **
 ** 第二要素 値有効チェックボックス
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck6()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCheckEblVal(1);
}


/** ****************************************************** **
 ** 第一要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo17()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(2, 0);
}


/** ****************************************************** **
 ** 第一要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo18()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(2, 0);
}


/** ****************************************************** **
 ** 第二要素 シート選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo21()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbSheet(2, 1);
}


/** ****************************************************** **
 ** 第二要素 機能選択コンボボックスSelchange
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnCbnSelchangeCombo22()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCmbMethod(2, 1);
}


/** ****************************************************** **
 ** 第二要素 値有効チェックボックス
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCheck7()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	SelectCheckEblVal(2);
}


/** ****************************************************** **
 ** 設定ボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedOk()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	CString title;
	CString msg;
	this->GetWindowTextA(title);

	//パラメータチェック
	char err_msg[256] = { '\0' };
	int rtn_chk = CheckParam(err_msg);
	if (rtn_chk == 1) {
		MessageBoxA(err_msg, title, MB_OK | MB_ICONWARNING);
		return;
	}

	ChangeParameter();	//パラメータの変更

	//シートパラメータを編集中検査パラメータへコピー
	edit_mes->sheet[sheet_number] = sheet_param;

	outlog.LogOutFile(1, "*** DlgMesFormula out (OK) ***");

	CDialogEx::OnOK();
}


/** ****************************************************** **
 ** キャンセルボタンClicked
 ** 特記事項：
 ** ****************************************************** **/
void CDlgMesFormula::OnBnClickedCancel()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。

	outlog.LogOutFile(1, "*** DlgMesFormula out (CANCEL) ***");

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
BOOL CDlgMesFormula::PreTranslateMessage(MSG* pMsg)
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
