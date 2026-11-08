/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamFormula.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：機種毎計算検査パラメータ/PLC出力パラメータの読込と書込と作成
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamFormula.h"

extern char* APP_ROOT;

CAccessParamFormula::CAccessParamFormula()
{
}


CAccessParamFormula::~CAccessParamFormula()
{
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの書込(シート毎の計算パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 **　CFormulaParam* param	計算パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamFormula::WriteIniFilePrmFormula(
	int work_number,
	int temp_number,
	int sheet_number,
	CFormulaParam* param
)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasSh" + Tips0.IntToString(sheet_number) + ".ini";

	//計算式
	CString name;
	for (int i = 0; i < 3; i++) {
		name = "Formula(mes_type21):Formula" + Tips0.IntToString(i + 1);	//例)Formula(mes_type21):Formula1

		buf = Tips0.IntToString((int)param->formula[i].enable);
		ret = WritePrivateProfileStringA(name, "enable", buf, filename);
		buf = Tips0.IntToString(param->formula[i].elem1.sheet_num);
		ret = WritePrivateProfileStringA(name, "sheet_num1", buf, filename);
		buf = Tips0.IntToString(param->formula[i].elem1.type);
		ret = WritePrivateProfileStringA(name, "type1", buf, filename);
		buf = param->formula[i].elem1.variable;
		ret = WritePrivateProfileStringA(name, "variable1", buf, filename);

		buf = Tips0.IntToString(param->formula[i].elem2.sheet_num);
		ret = WritePrivateProfileStringA(name, "sheet_num2", buf, filename);
		buf = Tips0.IntToString(param->formula[i].elem2.type);
		ret = WritePrivateProfileStringA(name, "type2", buf, filename);
		buf = param->formula[i].elem2.variable;
		ret = WritePrivateProfileStringA(name, "variable2", buf, filename);
		buf = Tips0.IntToString((int)param->formula[i].elem2.enable_val);
		ret = WritePrivateProfileStringA(name, "enable_val", buf, filename);
		buf = Tips0.DblToString(param->formula[i].elem2.val_d);
		ret = WritePrivateProfileStringA(name, "val_d", buf, filename);

		buf = Tips0.IntToString((int)param->formula[i].operator_item);
		ret = WritePrivateProfileStringA(name, "operator_item", buf, filename);
	}

	//判定
	buf = Tips0.IntToString((int)param->judge.enable);
	ret = WritePrivateProfileStringA("Formula(mes_type21):Judgement", "enable", buf, filename);
	buf = Tips0.IntToString(param->judge.judge_calc);
	ret = WritePrivateProfileStringA("Formula(mes_type21):Judgement", "judge_calc", buf, filename);
	buf = Tips0.DblToString(param->judge.judge_min);
	ret = WritePrivateProfileStringA("Formula(mes_type21):Judgement", "judge_min", buf, filename);
	buf = Tips0.DblToString(param->judge.judge_max);
	ret = WritePrivateProfileStringA("Formula(mes_type21):Judgement", "judge_max", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの読込(シート毎の計算パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 **　CFormulaParam* param	計算パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamFormula::ReadIniFilePrmFormula(
	int work_number,
	int temp_number,
	int sheet_number,
	CFormulaParam* param
)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasSh" + Tips0.IntToString(sheet_number) + ".ini";

	//計算式
	CString name;
	for (int i = 0; i < 3; i++) {
		name = "Formula(mes_type21):Formula" + Tips0.IntToString(i + 1);	//例)Formula(mes_type21):Formula1

		ret = GetPrivateProfileStringA(name, "enable", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].enable = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(name, "sheet_num1", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem1.sheet_num = atoi(buf);
		ret = GetPrivateProfileStringA(name, "type1", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem1.type = atoi(buf);
		ret = GetPrivateProfileStringA(name, "variable1", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem1.variable = buf;

		ret = GetPrivateProfileStringA(name, "sheet_num2", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem2.sheet_num = atoi(buf);
		ret = GetPrivateProfileStringA(name, "type2", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem2.type = atoi(buf);
		ret = GetPrivateProfileStringA(name, "variable2", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem2.variable = buf;
		ret = GetPrivateProfileStringA(name, "enable_val", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem2.enable_val = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(name, "val_d", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].elem2.val_d = atof(buf);

		ret = GetPrivateProfileStringA(name, "operator_item", "", buf, sizeof(buf), filename);
		if (ret != 0) param->formula[i].operator_item = (byte)atoi(buf);
	}

	//判定
	ret = GetPrivateProfileStringA("Formula(mes_type21):Judgement", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) param->judge.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("Formula(mes_type21):Judgement", "judge_calc", "", buf, sizeof(buf), filename);
	if (ret != 0) param->judge.judge_calc = atoi(buf);
	ret = GetPrivateProfileStringA("Formula(mes_type21):Judgement", "judge_min", "", buf, sizeof(buf), filename);
	if (ret != 0) param->judge.judge_min = atof(buf);
	ret = GetPrivateProfileStringA("Formula(mes_type21):Judgement", "judge_max", "", buf, sizeof(buf), filename);
	if (ret != 0) param->judge.judge_max = atof(buf);

	return 0;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの書込(シート毎のPLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 **　CPLCParamFormula* param	PLC出力パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamFormula::WriteIniFilePlcFormula(
	int work_number,
	int temp_number,
	int sheet_number,
	CPLCParamFormula* param
)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TPlcSh" + Tips0.IntToString(sheet_number) + ".ini";

	//計測結果出力アドレス
	for (int i = 0; i < 3; i++) {
		CString key = "formula_result" + Tips0.IntToString(i + 1);
		buf = Tips0.IntToString(param->out_mes_address.formula_result[i]);
		ret = WritePrivateProfileStringA("Formula(mes_type21):OutMeasureAddress", key, buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの読込(シート毎のPLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 **　CPLCParamFormula* param	PLC出力パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamFormula::ReadIniFilePlcFormula(
	int work_number,
	int temp_number,
	int sheet_number,
	CPLCParamFormula* param
)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TPlcSh" + Tips0.IntToString(sheet_number) + ".ini";

	//計測結果出力アドレス
	for (int i = 0; i < 3; i++) {
		CString key = "formula_result" + Tips0.IntToString(i + 1);
		ret = GetPrivateProfileStringA("Formula(mes_type21):OutMeasureAddress", key, "", buf, sizeof(buf), filename);
		if (ret != 0) param->out_mes_address.formula_result[i] = atoi(buf);
	}

	return 0;
}
