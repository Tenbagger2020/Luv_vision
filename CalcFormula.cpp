/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcFormula.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：計算演算プログラム
 **/

#include "stdafx.h"
#include "CalcFormula.h"

#include "CurrentStatus.h"		//現状共通データ
#include "OutputLog.h"			//ログ出力
#include "Language.h"			//言語設定用

extern COutputLog		outlog;			//ログ出力
extern CLanguage		language;		//言語設定用

CCalcFormula::CCalcFormula()
{
}


CCalcFormula::~CCalcFormula()
{
}


/** ****************************************************** **
 ** 計算検査メイン
 ** 引数：
 **　CMesParam* param		検査パラメータ
 **　CTempParam* param		テンプレートパラメータ
 **　int sheet_number		検査シート番号(0～7)
 **　BOOL* mes_ok			検査結果
 **　CListBox* lbox			ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcFormula::CalcMain(
	CMesParam* mes_param,
	CTempParam* temp_param,
	int sheet_number,
	BOOL* mes_ok,
	CListBox* lbox
)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	CString log_err = "";	//エラーログ

	//結果初期化
	temp_param->formula_result[sheet_number].InitialResult();

	//
	//計算実施
	//
	if (ExecuteFormula(mes_param, temp_param, sheet_number, mes_ok, lbox) != 0) goto END;

	rtn_result = 0;

END:
	//エラーログ出力
	if ((int)strlen(log_err) > 0) {
		if (lbox == NULL) {
			outlog.LogOutListCmd(0, 2, log_err);
		}
		else {
			lbox->AddString(log_err);
		}
	}

	//コマンドログ出力
	CString str_ok = "NG";
	if (*mes_ok) str_ok = "OK";
	CString log_str1 = Tips0.SelStr(language.jpn, "検査シート", "Inspection sheet ") + Tips0.IntToString(sheet_number)
		+ Tips0.SelStr(language.jpn, "(計算) 結果:", "(Calculation)) Result:") + str_ok;
	CString log_str2 = "";
	for (int i = 0; i < 3; i++) {
		if (mes_param->sheet[sheet_number].mes_formula.formula[i].enable) {
			if (strlen(log_str2) != 0) log_str2 = "\n" + log_str2;
			log_str2 = Tips0.SelStr(language.jpn, "計算値", "value") + Tips0.IntToString(i + 1) + ":"
				+ Tips0.DblToString(temp_param->formula_result[sheet_number].formula[i].d, 4, TRUE);
		}
	}
	if (lbox == NULL) {
		outlog.LogOutListCmd(0, 2, log_str1);	//コマンドログ 検査シート結果
		outlog.LogOutListCmd(0, 2, log_str2);
	}
	else {
		lbox->AddString(log_str1);
		lbox->AddString(log_str2);
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 計算実施
 ** 引数：
 **　CMesParam* param		検査パラメータ
 **　CTempParam* param		テンプレートパラメータ
 **　int sheet_number		検査シート番号(0～7)
 **　BOOL* mes_ok		検査結果
 **　CListBox* lbox		ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcFormula::ExecuteFormula(
	CMesParam* mes_param,
	CTempParam* temp_param,
	int sheet_number,
	BOOL* mes_ok,
	CListBox* lbox
)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;
	int rtn_result = 1;

	*mes_ok = FALSE;
	CString log_err = "";	//エラーログ

	//計算
	for (int i = 0; i < 3; i++) {

		CFormulaParam::Formula* p_formula = &mes_param->sheet[sheet_number].mes_formula.formula[i];
		if (p_formula->enable) {
			double result[2] = { 0 };

			//第一要素の取得
			GetElementVal(&result[0], mes_param, temp_param, sheet_number, i, 0);

			//第二要素の取得
			if (p_formula->elem2.sheet_num > 0) {
				//結果を取得
				GetElementVal(&result[1], mes_param, temp_param, sheet_number, i, 1);
			}
			else if (p_formula->elem2.enable_val) {
				//値を使用
				result[1] = p_formula->elem2.val_d;
			}

			//計算実行
			double* p_result_formula = &temp_param->formula_result[sheet_number].formula[i].d;
			switch (p_formula->operator_item) {
			case 0x00:
				*p_result_formula = result[0] + result[1];
				break;
			case 0x01:
				*p_result_formula = result[0] - result[1];
				break;
			case 0x02:
				*p_result_formula = result[0] * result[1];
				break;
			case 0x03:
				*p_result_formula = result[0] / result[1];
				break;
			}
		}
	}

	//判定処理
	BOOL judge_enable = mes_param->sheet[sheet_number].mes_formula.judge.enable;
	double judge_min = mes_param->sheet[sheet_number].mes_formula.judge.judge_min;
	double judge_max = mes_param->sheet[sheet_number].mes_formula.judge.judge_max;
	int judge_calc = mes_param->sheet[sheet_number].mes_formula.judge.judge_calc;
	double judge_result = temp_param->formula_result[sheet_number].formula[judge_calc].d;
	if (judge_enable) {
		if (judge_min <= judge_result && judge_result <= judge_max) *mes_ok = TRUE;
	}
	else {
		*mes_ok = TRUE;
	}

END:
	rtn_result = 0;

	//エラーログ出力
	if ((int)strlen(log_err) > 0) {
		if (lbox == NULL) {
			outlog.LogOutListCmd(0, 2, log_err);
		}
		else {
			lbox->AddString(log_err);
		}
	}

	return rtn_result;
}




/** ****************************************************** **
 ** 結果データの取得
 ** 引数：
 **　CFormulaResult* formula_result		計算結果
 **　CMesParam* param		検査パラメータ
 **　int sheet_number		検査シート番号(0～7)
 **　BOOL* mes_ok		検査結果
 **　CListBox* lbox		ログ出力先 NULL:メイン画面ログ、ポインタ指定:指定CListBox
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcFormula::GetElementVal(
	double* result,
	CMesParam* p_mes_param,
	CTempParam* p_temp_param,
	int sheet_number,
	int formula_num,
	int element
)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	*result = 0;	//結果0

	CFormulaParam::Formula* p_formula = &p_mes_param->sheet[sheet_number].mes_formula.formula[formula_num];

	if (p_formula->enable) {
		int choice_sheet = -1;
		if (element == 0) choice_sheet = p_formula->elem1.sheet_num;
		if (element == 1) choice_sheet = p_formula->elem2.sheet_num;

		byte type = 0x00;
		if (element == 0) type = p_formula->elem1.type;
		if (element == 1) type = p_formula->elem2.type;

		CString variable = "";
		if (element == 0) variable = p_formula->elem1.variable;
		if (element == 1) variable = p_formula->elem2.variable;

		switch (type) {
			//無し
		case 0x00:
			break;
			//ブロブ
		case 0x01:
			break;
			//エッジ
		case 0x02:
			break;
			//濃度
		case 0x03:
			break;
			//マルチ探索
		case 0x04:
		{
			if (variable == "count") *result = (double)p_temp_param->multisrh_result[choice_sheet].count;
		}
		break;
		//基準-エッジ距離計測
		case 0x05:
		{
			if (variable == "distance") {
				byte sign = p_mes_param->sheet[choice_sheet].mes_disedge.mes_judge.sign;
				byte unit = p_mes_param->sheet[choice_sheet].mes_disedge.mes_judge.unit;

				//符号ごとの結果
				//符号なし
				if (sign == 0x00) {
					if (unit == 0x00) *result = p_temp_param->disedge_result[choice_sheet].distance_pix;
					if (unit == 0x01) *result = p_temp_param->disedge_result[choice_sheet].distance_mm;
				}
				//X符号
				else if (sign == 0x01) {
					if (unit == 0x00) *result = p_temp_param->disedge_result[choice_sheet].x_sign_pix;
					if (unit == 0x01) *result = p_temp_param->disedge_result[choice_sheet].x_sign_mm;
				}
				//Y符号
				else if (sign == 0x02) {
					if (unit == 0x00) *result = p_temp_param->disedge_result[choice_sheet].y_sign_pix;
					if (unit == 0x01) *result = p_temp_param->disedge_result[choice_sheet].y_sign_mm;
				}
			}
		}
		break;
		//エッジ-エッジ距離計測
		case 0x06:
		{
			if (variable == "distance") {
				byte sign = p_mes_param->sheet[choice_sheet].mes_disetoe.mes_judge.sign;
				byte unit = p_mes_param->sheet[choice_sheet].mes_disetoe.mes_judge.unit;

				//符号ごとの結果
				//符号なし
				if (sign == 0x00) {
					if (unit == 0x00) *result = p_temp_param->disetoe_result[choice_sheet].distance_pix;
					if (unit == 0x01) *result = p_temp_param->disetoe_result[choice_sheet].distance_mm;
				}
				//X符号
				else if (sign == 0x01) {
					if (unit == 0x00) *result = p_temp_param->disetoe_result[choice_sheet].x_sign_pix;
					if (unit == 0x01) *result = p_temp_param->disetoe_result[choice_sheet].x_sign_mm;
				}
				//Y符号
				else if (sign == 0x02) {
					if (unit == 0x00) *result = p_temp_param->disetoe_result[choice_sheet].y_sign_pix;
					if (unit == 0x01) *result = p_temp_param->disetoe_result[choice_sheet].y_sign_mm;
				}
			}
		}
		break;
		//画像コピー(矩形)
		case 0x11:
			break;
			//計算
		case 0x21:
		{
			if (variable == "calclation1") *result = p_temp_param->formula_result[choice_sheet].formula[0].d;
			if (variable == "calclation2") *result = p_temp_param->formula_result[choice_sheet].formula[1].d;
			if (variable == "calclation3") *result = p_temp_param->formula_result[choice_sheet].formula[2].d;
		}
		break;
		}
	}

	return 0;
}
