/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamScrp.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2024.07.16	M.Iwasaki	10.18.4.6	画素サイズ固定機能
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：機種毎スコープパラメータの読込と書込と作成
 **		iniファイルの確認と新規作成する
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamScrp.h"

#include "ScopeParam.h"		//機種毎スコープパラメータ

#include "direct.h"

extern CScopeParam		scope_param[16];	//機種毎スコープパラメータ

extern char* APP_ROOT;

CAccessParamScrp::CAccessParamScrp()
{
}


CAccessParamScrp::~CAccessParamScrp()
{
}


/** ****************************************************** **
 ** Iniファイルディレクトリの確認
 ** 引数：
 **　int work_number	機種番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::CheckDir(int work_number)
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

	//スコープディレクトリ確認
	strcat_s(path, "\\SCOPE");
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** SParameter*.iniファイルの確認
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::CheckIniFileParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SParameter" + Tips0.IntToString(scrp_number + 1) + ".ini";
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
 ** SParameter*.iniファイルの作成
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::CreateIniFileParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFileParam(work_number, scrp_number);

	//最後にiniファイルの確認
	if (CheckIniFileParam(work_number, scrp_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** SParameter*.iniファイルの書込
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::WriteIniFileParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SParameter" + Tips0.IntToString(scrp_number + 1) + ".ini";

	//スコープパラメータ
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1
	buf = Tips0.IntToString(scope_param[scrp_number].calib_theta_complete);
	ret = WritePrivateProfileStringA(scope_name, "calib_theta_complete", buf, filename);
	buf = Tips0.DblToString(scope_param[scrp_number].conf_rotcent_x);
	ret = WritePrivateProfileStringA(scope_name, "conf_rotcent_x", buf, filename);
	buf = Tips0.DblToString(scope_param[scrp_number].conf_rotcent_y);
	ret = WritePrivateProfileStringA(scope_name, "conf_rotcent_y", buf, filename);
	buf = Tips0.IntToString(scope_param[scrp_number].conf_rotcent_view);
	ret = WritePrivateProfileStringA(scope_name, "conf_rotcent_view", buf, filename);

	//視野パラメータ(1～4)
	CString view_name;
	CString calib_pixel_name;
	CString calib_theta_name;
	CString calib_move_name;
	for (int view_number = 0; view_number < 4; view_number++) {
		view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1
		buf = Tips0.IntToString(scope_param[scrp_number].view[view_number].coordinate_type);
		ret = WritePrivateProfileStringA(view_name, "coordinate_type", buf, filename);

		//キャリブレーションパラメータ
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel_size);
		ret = WritePrivateProfileStringA(view_name, "pixel_size", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.crdnt_radian);
		ret = WritePrivateProfileStringA(view_name, "crdnt_radian", buf, filename);
		buf = Tips0.IntToString(scope_param[scrp_number].view[view_number].calib.pixel_complete);
		ret = WritePrivateProfileStringA(view_name, "pixel_complete", buf, filename);
		buf = Tips0.IntToString(scope_param[scrp_number].view[view_number].calib.theta_complete);
		ret = WritePrivateProfileStringA(view_name, "theta_complete", buf, filename);

		//画素キャリブレーション詳細パラメータ
		calib_pixel_name = view_name + " CalibPixel";
		for (int i = 0; i < 2; i++) {
			buf = Tips0.IntToString((int)scope_param[scrp_number].view[view_number].calib.pixel.template_number[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "template_number" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.axis_x[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "axis_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.axis_y[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "axis_y" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.mark_x[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "mark_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.mark_y[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "mark_y" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.crdnt_x[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "crdnt_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.crdnt_y[i]);
			ret = WritePrivateProfileStringA(calib_pixel_name, "crdnt_y" + Tips0.IntToString(i), buf, filename);
		}
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.axis_base_x);
		ret = WritePrivateProfileStringA(calib_pixel_name, "axis_base_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.axis_base_y);
		ret = WritePrivateProfileStringA(calib_pixel_name, "axis_base_y", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.cal_base_x);
		ret = WritePrivateProfileStringA(calib_pixel_name, "cal_base_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.pixel.cal_base_y);
		ret = WritePrivateProfileStringA(calib_pixel_name, "cal_base_y", buf, filename);

		//回転中心キャリブレーション詳細パラメータ
		calib_theta_name = view_name + " CalibTheta";
		for (int i = 0; i < 2; i++) {
			buf = Tips0.IntToString((int)scope_param[scrp_number].view[view_number].calib.theta.template_number[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "template_number" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.axis_x[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "axis_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.axis_y[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "axis_y" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.axis_t[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "axis_t" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.mark_x[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "mark_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.mark_y[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "mark_y" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.crdnt_x[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "crdnt_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.crdnt_y[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "crdnt_y" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.mech_x[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "mech_x" + Tips0.IntToString(i), buf, filename);
			buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.mech_y[i]);
			ret = WritePrivateProfileStringA(calib_theta_name, "mech_y" + Tips0.IntToString(i), buf, filename);
		}
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.rotation_center_x);
		ret = WritePrivateProfileStringA(calib_theta_name, "rotation_center_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].calib.theta.rotation_center_y);
		ret = WritePrivateProfileStringA(calib_theta_name, "rotation_center_y", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** SParameter*.iniファイルの読込
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::ReadIniFileParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SParameter" + Tips0.IntToString(scrp_number + 1) + ".ini";

	//スコープパラメータ
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1
	ret = GetPrivateProfileStringA(scope_name, "calib_theta_complete", "", buf, sizeof(buf), filename);
	if (ret != 0) scope_param[scrp_number].calib_theta_complete = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA(scope_name, "conf_rotcent_x", "", buf, sizeof(buf), filename);
	if (ret != 0) scope_param[scrp_number].conf_rotcent_x = atof(buf);
	ret = GetPrivateProfileStringA(scope_name, "conf_rotcent_y", "", buf, sizeof(buf), filename);
	if (ret != 0) scope_param[scrp_number].conf_rotcent_y = atof(buf);
	ret = GetPrivateProfileStringA(scope_name, "conf_rotcent_view", "", buf, sizeof(buf), filename);
	if (ret != 0) scope_param[scrp_number].conf_rotcent_view = atoi(buf);

	//視野パラメータ(1～4)
	CString view_name;
	CString calib_pixel_name;
	CString calib_theta_name;
	CString calib_move_name;
	for (int view_number = 0; view_number < 4; view_number++) {
		view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1
		ret = GetPrivateProfileStringA(view_name, "coordinate_type", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].coordinate_type = atoi(buf);

		//キャリブレーションパラメータ
		ret = GetPrivateProfileStringA(view_name, "pixel_size", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel_size = atof(buf);
		ret = GetPrivateProfileStringA(view_name, "crdnt_radian", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.crdnt_radian = atof(buf);
		ret = GetPrivateProfileStringA(view_name, "pixel_complete", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel_complete = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(view_name, "theta_complete", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta_complete = (BOOL)atoi(buf);

		//画素キャリブレーション詳細パラメータ
		calib_pixel_name = view_name + " CalibPixel";
		for (int i = 0; i < 2; i++) {
			ret = GetPrivateProfileStringA(calib_pixel_name, "template_number", "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.template_number[i] = (byte)atoi(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "axis_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.axis_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "axis_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.axis_y[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "mark_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.mark_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "mark_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.mark_y[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "crdnt_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.crdnt_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_pixel_name, "crdnt_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.crdnt_y[i] = atof(buf);
		}
		ret = GetPrivateProfileStringA(calib_pixel_name, "axis_base_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.axis_base_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_pixel_name, "axis_base_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.axis_base_y = atof(buf);
		ret = GetPrivateProfileStringA(calib_pixel_name, "cal_base_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.cal_base_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_pixel_name, "cal_base_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.pixel.cal_base_y = atof(buf);

		//回転中心キャリブレーション詳細パラメータ
		calib_theta_name = view_name + " CalibTheta";
		for (int i = 0; i < 2; i++) {
			ret = GetPrivateProfileStringA(calib_theta_name, "template_number", "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.template_number[i] = (byte)atoi(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "axis_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.axis_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "axis_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.axis_y[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "axis_t" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.axis_t[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "mark_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.mark_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "mark_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.mark_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "crdnt_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.crdnt_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "crdnt_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.crdnt_y[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "mech_x" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.mech_x[i] = atof(buf);
			ret = GetPrivateProfileStringA(calib_theta_name, "mech_y" + Tips0.IntToString(i), "", buf, sizeof(buf), filename);
			if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.mech_y[i] = atof(buf);
		}
		ret = GetPrivateProfileStringA(calib_theta_name, "rotation_center_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.rotation_center_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_theta_name, "rotation_center_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].calib.theta.rotation_center_y = atof(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** ScopeCal.iniファイルの読込
 ** 引数：
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::ReadIniFilePixel(int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	//
	//画素Calサイズ固定化
	//

	CString filename_cal = (CString)APP_ROOT + "\\SYSTEM\\SPEC\\ScopeCal.ini";
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1
	for (int i = 0; i < 4; i++) {
		//もしScopeCal.iniが存在し指定する視野の情報が記述されていれば、画素サイズを設定し画素Cal完了とする
		//例)スコープ1視野1
		//[Scope1View1]
		//pixel_size=0.007444
		CString view_name = scope_name + "View" + Tips0.IntToString(i + 1);	//例)Scope1View1
		ret = GetPrivateProfileStringA(view_name, "pixel_size", "", buf, sizeof(buf), filename_cal);
		if (ret != 0) {
			scope_param[scrp_number].view[i].calib.pixel_size = atof(buf);
			scope_param[scrp_number].view[i].calib.pixel_complete = TRUE;
		}
	}

	return 0;
}


/** ****************************************************** **
 ** 固定サイズ判定
 ** 引数：
 **　int scrp_number	スコープ番号(0-15)
 **　int view_number	視野番号(0-3)
 ** 戻り値：
 **　BOOL 判定　0:画素サイズは指定されていない、1:画素サイズが指定されている
 ** 特記事項：
 ** ****************************************************** **/
BOOL CAccessParamScrp::GetdecidedPixel(int scrp_number, int view_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	//
	//画素Calサイズ固定化
	//

	CString filename_cal = (CString)APP_ROOT + "\\SYSTEM\\SPEC\\ScopeCal.ini";
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1
	CString view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1
	//もしScopeCal.iniが存在し指定する視野の情報が記述されていれば、TRUEを返す
	//例)スコープ1視野1
	//[Scope1View1]
	//pixel_size=0.007444
	ret = GetPrivateProfileStringA(view_name, "pixel_size", "", buf, sizeof(buf), filename_cal);
	if (ret != 0) {
		//画素サイズが指定されている
		return TRUE;
	}

	return FALSE;
}


/** ****************************************************** **
 ** SVariation*.iniファイルの確認
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::CheckVariationParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SVariation" + Tips0.IntToString(scrp_number + 1) + ".ini";
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
 ** SVariation*.iniファイルの作成
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::CreateVariationParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteVariationParam(work_number, scrp_number);

	//最後にiniファイルの確認
	if (CheckVariationParam(work_number, scrp_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** SVariation*.iniファイルの書込
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::WriteVariationParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SVariation" + Tips0.IntToString(scrp_number + 1) + ".ini";

	//スコープパラメータ
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1

	//視野パラメータ(1～4)
	CString view_name;
	CString calib_move_name;
	for (int view_number = 0; view_number < 4; view_number++) {
		view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1

		//経時変化補正
		calib_move_name = view_name + " CalibMove";
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.base_mark_x);
		ret = WritePrivateProfileStringA(calib_move_name, "base_mark_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.base_mark_y);
		ret = WritePrivateProfileStringA(calib_move_name, "base_mark_y", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.move_mark_x);
		ret = WritePrivateProfileStringA(calib_move_name, "move_mark_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.move_mark_y);
		ret = WritePrivateProfileStringA(calib_move_name, "move_mark_y", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.displacement_x);
		ret = WritePrivateProfileStringA(calib_move_name, "displacement_x", buf, filename);
		buf = Tips0.DblToString(scope_param[scrp_number].view[view_number].move.displacement_y);
		ret = WritePrivateProfileStringA(calib_move_name, "displacement_y", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** SVariation*.iniファイルの読込
 ** 引数：
 **　int work_number	機種番号
 **　int scrp_number	スコープ番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamScrp::ReadVariationParam(int work_number, int scrp_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "SCOPE\\SVariation" + Tips0.IntToString(scrp_number + 1) + ".ini";

	//スコープパラメータ
	CString scope_name = "Scope" + Tips0.IntToString(scrp_number + 1);		//例)Scope1

	//視野パラメータ(1～4)
	CString view_name;
	CString calib_move_name;
	for (int view_number = 0; view_number < 4; view_number++) {
		view_name = scope_name + "View" + Tips0.IntToString(view_number + 1);	//例)Scope1View1

		//経時変化補正
		calib_move_name = view_name + " CalibMove";
		ret = GetPrivateProfileStringA(calib_move_name, "base_mark_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.base_mark_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_move_name, "base_mark_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.base_mark_y = atof(buf);
		ret = GetPrivateProfileStringA(calib_move_name, "move_mark_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.move_mark_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_move_name, "move_mark_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.move_mark_y = atof(buf);
		ret = GetPrivateProfileStringA(calib_move_name, "displacement_x", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.displacement_x = atof(buf);
		ret = GetPrivateProfileStringA(calib_move_name, "displacement_y", "", buf, sizeof(buf), filename);
		if (ret != 0) scope_param[scrp_number].view[view_number].move.displacement_y = atof(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** 機種スコープパラメータの全初期化
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CAccessParamScrp::InitParams()
{
	// TODO: ここに実装コードを追加します.
	for (int i = 0; i < 16; i++) {
		scope_param[i].InitialParam();
	}
}
