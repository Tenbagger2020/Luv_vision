/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamMes.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：機種毎検査パラメータの読込と書込と作成
 **		iniファイルの確認と新規作成する
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamMes.h"

#include "MesParam.h"		//機種毎検査パラメータ
#include "direct.h"

//DLL
#include "AlignBlob.h"	//AlignBlob DLL Header
#include "AlignEdge.h"	//AlignEdge DLL Header
#include "AlignMeas.h"	//AlignMeas DLL Header
#include "AlignMultiSrh.h"	//AlignMultiSrh DLL Header
#include "AlignDisEdge.h"	//AlignDisEdge DLL Header
#include "AlignDisEdgeToEdge.h"	//AlignDisEdgeToEdge DLL Header

#include "AccessParamFormula.h"

extern CMesParam		mes_param[256];	//機種毎検査パラメータ
extern char* APP_ROOT;

CAccessParamMes::CAccessParamMes()
{
}


CAccessParamMes::~CAccessParamMes()
{
}


/** ****************************************************** **
 ** Iniファイルディレクトリの確認
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CheckDir(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	if (_chdir((CString)APP_ROOT) != 0) {
		if (_mkdir((CString)APP_ROOT) != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT) != 0) return 1;
	if (_chdir((CString)APP_ROOT + "\\WORK") != 0) {
		if (_mkdir((CString)APP_ROOT + "\\WORK") != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT + "\\WORK") != 0) return 1;

	char path[MAX_PATH] = { '\0' };

	//機種Noディレクトリ確認
	strcpy_s(path, MAX_PATH, (CString)APP_ROOT + "\\WORK\\");
	CString s_work = Tips0.IntToString(work_number, 3);
	strcat_s(path, s_work);
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	//テンプレートNoディレクトリ確認
	strcat_s(path, "\\T");
	CString s_temp = Tips0.IntToString(temp_number, 3);
	strcat_s(path, s_temp);
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	//検査設定ディレクトリ確認
	strcat_s(path, "\\MES");
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** TMeasComm.iniファイルの確認(共通パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CheckIniFilePrmCom(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasComm.ini";
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, filename, "r");

	//ファイルが開ければ結果0
	if (err == 0) rtn_result = 0;

	if (fp) {
		fclose(fp);
	}

	return rtn_result;
}


/** ****************************************************** **
 ** TMeasComm.iniファイルの作成(共通パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CreateIniFilePrmCom(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number, temp_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFilePrmCom(work_number, temp_number);

	//最後にiniファイルの確認
	if (CheckIniFilePrmCom(work_number, temp_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** TMeasComm.iniファイルの書込(共通パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::WriteIniFilePrmCom(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasComm.ini";

	//基準点探索有無
	buf = Tips0.IntToString((int)mes_param[temp_number].base_search_enable);
	ret = WritePrivateProfileStringA("BaseSearch", "base_search_enable", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** TMeasComm.iniファイルの読込(共通パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::ReadIniFilePrmCom(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasComm.ini";

	//基準点探索有無
	ret = GetPrivateProfileStringA("BaseSearch", "base_search_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].base_search_enable = (BOOL)atoi(buf);

	return 0;
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの確認(シートパラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CheckIniFilePrmSht(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasSh" + Tips0.IntToString(sheet_number) + ".ini";
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, filename, "r");

	//ファイルが開ければ結果0
	if (err == 0) rtn_result = 0;

	if (fp) {
		fclose(fp);
	}

	return rtn_result;
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの作成(シートパラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CreateIniFilePrmSht(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number, temp_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFilePrmSht(work_number, temp_number, sheet_number);

	//最後にiniファイルの確認
	if (CheckIniFilePrmSht(work_number, temp_number, sheet_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの書込(シートパラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::WriteIniFilePrmSht(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasSh" + Tips0.IntToString(sheet_number) + ".ini";

	//検査機能
	buf = Tips0.IntToString((int)mes_param[temp_number].sheet[sheet_number].mes_type);
	ret = WritePrivateProfileStringA("Method", "mes_type", buf, filename);

	//検査エリア指定
	buf = Tips0.IntToString((int)mes_param[temp_number].sheet[sheet_number].mes_area.enable);
	ret = WritePrivateProfileStringA("MesArea", "enable", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.st_x);
	ret = WritePrivateProfileStringA("MesArea", "st_x", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.st_y);
	ret = WritePrivateProfileStringA("MesArea", "st_y", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.width);
	ret = WritePrivateProfileStringA("MesArea", "width", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.height);
	ret = WritePrivateProfileStringA("MesArea", "height", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.mark_dis_x);
	ret = WritePrivateProfileStringA("MesArea", "mark_dis_x", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.mark_dis_y);
	ret = WritePrivateProfileStringA("MesArea", "mark_dis_y", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_area.area_degree);
	ret = WritePrivateProfileStringA("MesArea", "area_degree", buf, filename);

	//2値化パラメータ
	buf = Tips0.IntToString((int)mes_param[temp_number].sheet[sheet_number].mes_binz.base_code);
	ret = WritePrivateProfileStringA("Binarize", "base_code", buf, filename);
	buf = Tips0.IntToString((int)mes_param[temp_number].sheet[sheet_number].mes_binz.type);
	ret = WritePrivateProfileStringA("Binarize", "type", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.fix_threshoid[0]);
	ret = WritePrivateProfileStringA("Binarize", "fix_threshoid1", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.fix_threshoid[1]);
	ret = WritePrivateProfileStringA("Binarize", "fix_threshoid2", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna2_edge_facter);
	ret = WritePrivateProfileStringA("Binarize", "dyna2_edge_facter", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna3_edge_facter);
	ret = WritePrivateProfileStringA("Binarize", "dyna3_edge_facter", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_area_size);
	ret = WritePrivateProfileStringA("Binarize", "dyna4_area_size", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_coefficientK);
	ret = WritePrivateProfileStringA("Binarize", "dyna4_coefficientK", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_border_mode);
	ret = WritePrivateProfileStringA("Binarize", "dyna4_border_mode", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna5_rate);
	ret = WritePrivateProfileStringA("Binarize", "dyna5_rate", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_area_size);
	ret = WritePrivateProfileStringA("Binarize", "dyna6_area_size", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_coefficientK);
	ret = WritePrivateProfileStringA("Binarize", "dyna6_coefficientK", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_coefficientR);
	ret = WritePrivateProfileStringA("Binarize", "dyna6_coefficientR", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_border_mode);
	ret = WritePrivateProfileStringA("Binarize", "dyna6_border_mode", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_area_size);
	ret = WritePrivateProfileStringA("Binarize", "dyna7_area_size", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_offset);
	ret = WritePrivateProfileStringA("Binarize", "dyna7_offset", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_border_mode);
	ret = WritePrivateProfileStringA("Binarize", "dyna7_border_mode", buf, filename);
	buf = Tips0.IntToString(mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_border_val);
	ret = WritePrivateProfileStringA("Binarize", "dyna7_border_val", buf, filename);

	//ブロブパラメータ
	AlignBlob::Param::CAccessParamBlob access_blob;
	AlignBlob::Param::CBlobParam* blob_param;
	blob_param = &mes_param[temp_number].sheet[sheet_number].mes_blob;
	access_blob.WriteIniFilePrmBlob(work_number, temp_number, sheet_number, blob_param);

	//エッジパラメータ
	AlignEdge::Param::CAccessParamEdge access_edge;
	AlignEdge::Param::CEdgeParam* edge_param;
	edge_param = &mes_param[temp_number].sheet[sheet_number].mes_edge;
	access_edge.WriteIniFilePrmEdge(work_number, temp_number, sheet_number, edge_param);

	//濃度パラメータ
	AlignMeas::Param::CAccessParamMeas access_meas;
	AlignMeas::Param::CMeasParam* meas_param;
	meas_param = &mes_param[temp_number].sheet[sheet_number].mes_meas;
	access_meas.WriteIniFilePrmMeas(work_number, temp_number, sheet_number, meas_param);

	//マルチ探索パラメータ
	AlignMultiSrh::Param::CAccessParamMultiSrh access_multisrh;
	AlignMultiSrh::Param::CMultiSrhParam* multisrh_param;
	multisrh_param = &mes_param[temp_number].sheet[sheet_number].mes_multisrh;
	access_multisrh.WriteIniFilePrmMultiSrh(work_number, temp_number, sheet_number, multisrh_param);

	//基準-エッジ距離計測パラメータ
	AlignDisEdge::Param::CAccessParamDisEdge access_disedge;
	AlignDisEdge::Param::CDisEdgeParam* disedge_param;
	disedge_param = &mes_param[temp_number].sheet[sheet_number].mes_disedge;
	access_disedge.WriteIniFilePrmDisEdge(work_number, temp_number, sheet_number, disedge_param);

	//エッジ-エッジ距離計測パラメータ
	AlignDisEdgeToEdge::Param::CAccessParamDisEdgeToEdge access_disetoe;
	AlignDisEdgeToEdge::Param::CDisEdgeToEdgeParam* disetoe_param;
	disetoe_param = &mes_param[temp_number].sheet[sheet_number].mes_disetoe;
	access_disetoe.WriteIniFilePrmDisEdgeToEdge(work_number, temp_number, sheet_number, disetoe_param);

	//計算パラメータ
	CAccessParamFormula access_formula;
	CFormulaParam* formula_param;
	formula_param = &mes_param[temp_number].sheet[sheet_number].mes_formula;
	access_formula.WriteIniFilePrmFormula(work_number, temp_number, sheet_number, formula_param);

	return 0;
}


/** ****************************************************** **
 ** TMeasSh*.iniファイルの読込(シートパラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::ReadIniFilePrmSht(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TMeasSh" + Tips0.IntToString(sheet_number) + ".ini";

	//検査機能
	ret = GetPrivateProfileStringA("Method", "mes_type", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_type = (byte)atoi(buf);

	//検査エリア指定
	ret = GetPrivateProfileStringA("MesArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.st_x = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.st_y = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.width = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.height = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "mark_dis_x", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.mark_dis_x = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "mark_dis_y", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.mark_dis_y = atoi(buf);
	ret = GetPrivateProfileStringA("MesArea", "area_degree", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_area.area_degree = atoi(buf);

	//2値化パラメータ
	ret = GetPrivateProfileStringA("Binarize", "base_code", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.base_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.type = (WORD)atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "fix_threshoid1", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.fix_threshoid[0] = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "fix_threshoid2", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.fix_threshoid[1] = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna2_edge_facter", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna2_edge_facter = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna3_edge_facter", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna3_edge_facter = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna4_area_size", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_area_size = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna4_coefficientK", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_coefficientK = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna4_border_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna4_border_mode = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna5_rate", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna5_rate = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna6_area_size", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_area_size = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna6_coefficientK", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_coefficientK = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna6_coefficientR", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_coefficientR = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna6_border_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna6_border_mode = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna7_area_size", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_area_size = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna7_offset", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_offset = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna7_border_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_border_mode = atoi(buf);
	ret = GetPrivateProfileStringA("Binarize", "dyna7_border_val", "", buf, sizeof(buf), filename);
	if (ret != 0) mes_param[temp_number].sheet[sheet_number].mes_binz.dyna7_border_val = atoi(buf);

	//ブロブパラメータ
	AlignBlob::Param::CAccessParamBlob access_blob;
	AlignBlob::Param::CBlobParam* blob_param;
	blob_param = &mes_param[temp_number].sheet[sheet_number].mes_blob;
	access_blob.ReadIniFilePrmBlob(work_number, temp_number, sheet_number, blob_param);

	//エッジパラメータ
	AlignEdge::Param::CAccessParamEdge access_edge;
	AlignEdge::Param::CEdgeParam* edge_param;
	edge_param = &mes_param[temp_number].sheet[sheet_number].mes_edge;
	access_edge.ReadIniFilePrmEdge(work_number, temp_number, sheet_number, edge_param);

	//濃度パラメータ
	AlignMeas::Param::CAccessParamMeas access_meas;
	AlignMeas::Param::CMeasParam* meas_param;
	meas_param = &mes_param[temp_number].sheet[sheet_number].mes_meas;
	access_meas.ReadIniFilePrmMeas(work_number, temp_number, sheet_number, meas_param);

	//マルチ探索パラメータ
	AlignMultiSrh::Param::CAccessParamMultiSrh access_multisrh;
	AlignMultiSrh::Param::CMultiSrhParam* multisrh_param;
	multisrh_param = &mes_param[temp_number].sheet[sheet_number].mes_multisrh;
	access_multisrh.ReadIniFilePrmMultiSrh(work_number, temp_number, sheet_number, multisrh_param);

	//基準-エッジ距離計測パラメータ
	AlignDisEdge::Param::CAccessParamDisEdge access_disedge;
	AlignDisEdge::Param::CDisEdgeParam* disedge_param;
	disedge_param = &mes_param[temp_number].sheet[sheet_number].mes_disedge;
	access_disedge.ReadIniFilePrmDisEdge(work_number, temp_number, sheet_number, disedge_param);

	//エッジ-エッジ距離計測パラメータ
	AlignDisEdgeToEdge::Param::CAccessParamDisEdgeToEdge access_disetoe;
	AlignDisEdgeToEdge::Param::CDisEdgeToEdgeParam* disetoe_param;
	disetoe_param = &mes_param[temp_number].sheet[sheet_number].mes_disetoe;
	access_disetoe.ReadIniFilePrmDisEdgeToEdge(work_number, temp_number, sheet_number, disetoe_param);

	//計算パラメータ
	CAccessParamFormula access_formula;
	CFormulaParam* formula_param;
	formula_param = &mes_param[temp_number].sheet[sheet_number].mes_formula;
	access_formula.ReadIniFilePrmFormula(work_number, temp_number, sheet_number, formula_param);

	return 0;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの確認(PLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CheckIniFilePrmPlc(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\MES\\TPlcSh" + Tips0.IntToString(sheet_number) + ".ini";
	FILE* fp;
	errno_t err;

	err = fopen_s(&fp, filename, "r");

	//ファイルが開ければ結果0
	if (err == 0) rtn_result = 0;

	if (fp) {
		fclose(fp);
	}

	return rtn_result;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの作成(PLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::CreateIniFilePrmPlc(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number, temp_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFilePrmPlc(work_number, temp_number, sheet_number);

	//最後にiniファイルの確認
	if (CheckIniFilePrmPlc(work_number, temp_number, sheet_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの書込(PLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::WriteIniFilePrmPlc(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	//マルチ探索 PLC出力パラメータ
	AlignMultiSrh::Param::CAccessParamMultiSrh access_multisrh;
	AlignMultiSrh::Param::CPLCParamMultiSrh* multisrh_param;
	multisrh_param = &mes_param[temp_number].plcout[sheet_number].plc_multisrh;
	access_multisrh.WriteIniFilePlcMultiSrh(work_number, temp_number, sheet_number, multisrh_param);

	//基準-エッジ距離計測 PLC出力パラメータ
	AlignDisEdge::Param::CAccessParamDisEdge access_disedge;
	AlignDisEdge::Param::CPLCParamDisEdge* disedge_param;
	disedge_param = &mes_param[temp_number].plcout[sheet_number].plc_disedge;
	access_disedge.WriteIniFilePlcDisEdge(work_number, temp_number, sheet_number, disedge_param);

	//エッジ-エッジ距離計測 PLC出力パラメータ
	AlignDisEdgeToEdge::Param::CAccessParamDisEdgeToEdge access_disetoe;
	AlignDisEdgeToEdge::Param::CPLCParamDisEdgeToEdge* disetoe_param;
	disetoe_param = &mes_param[temp_number].plcout[sheet_number].plc_disetoe;
	access_disetoe.WriteIniFilePlcDisEdgeToEdge(work_number, temp_number, sheet_number, disetoe_param);

	//計算 PLC出力パラメータ
	CAccessParamFormula access_formula;
	CPLCParamFormula* formula_param;
	formula_param = &mes_param[temp_number].plcout[sheet_number].plc_formula;
	access_formula.WriteIniFilePlcFormula(work_number, temp_number, sheet_number, formula_param);

	return 0;
}


/** ****************************************************** **
 ** TPlcSh*.iniファイルの読込(PLC出力パラメータ)
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 **　int sheet_number	シート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamMes::ReadIniFilePrmPlc(int work_number, int temp_number, int sheet_number)
{
	// TODO: ここに実装コードを追加します.

	//マルチ探索 PLC出力パラメータ
	AlignMultiSrh::Param::CAccessParamMultiSrh access_multisrh;
	AlignMultiSrh::Param::CPLCParamMultiSrh* multisrh_param;
	multisrh_param = &mes_param[temp_number].plcout[sheet_number].plc_multisrh;
	access_multisrh.ReadIniFilePlcMultiSrh(work_number, temp_number, sheet_number, multisrh_param);

	//基準-エッジ距離計測 PLC出力パラメータ
	AlignDisEdge::Param::CAccessParamDisEdge access_disedge;
	AlignDisEdge::Param::CPLCParamDisEdge* disedge_param;
	disedge_param = &mes_param[temp_number].plcout[sheet_number].plc_disedge;
	access_disedge.ReadIniFilePlcDisEdge(work_number, temp_number, sheet_number, disedge_param);

	//エッジ-エッジ距離計測 PLC出力パラメータ
	AlignDisEdgeToEdge::Param::CAccessParamDisEdgeToEdge access_disetoe;
	AlignDisEdgeToEdge::Param::CPLCParamDisEdgeToEdge* disetoe_param;
	disetoe_param = &mes_param[temp_number].plcout[sheet_number].plc_disetoe;
	access_disetoe.ReadIniFilePlcDisEdgeToEdge(work_number, temp_number, sheet_number, disetoe_param);

	//計算 PLC出力パラメータ
	CAccessParamFormula access_formula;
	CPLCParamFormula* formula_param;
	formula_param = &mes_param[temp_number].plcout[sheet_number].plc_formula;
	access_formula.ReadIniFilePlcFormula(work_number, temp_number, sheet_number, formula_param);

	return 0;
}


/** ****************************************************** **
 ** 機種検査パラメータの全初期化(共通パラメータ/シートパラメータ)
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CAccessParamMes::InitParams()
{
	// TODO: ここに実装コードを追加します.
	for (int i = 0; i < 256; i++) {
		mes_param[i].InitialParam();
	}
}
