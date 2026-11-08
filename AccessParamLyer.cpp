/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamLyer.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：機種レイヤパラメータの読込と書込と作成
 **		iniファイルの確認と新規作成する
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamLyer.h"

#include "LayerParam.h"			//機種毎レイヤパラメータ

#include "direct.h"

extern CLayerParam		layer_param[8];		//機種毎個別レイヤパラメータ(カメラ1～8)

extern char* APP_ROOT;

CAccessParamLyer::CAccessParamLyer()
{
}


CAccessParamLyer::~CAccessParamLyer()
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
int CAccessParamLyer::CheckDir(int work_number)
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

	//レイヤディレクトリ確認
	strcat_s(path, "\\LAYER");
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** WkLayer*.iniファイルの確認
 ** 引数：
 **　int work_number	機種番号
 **　int cam_number		カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamLyer::CheckIniFileParam(int work_number, int cam_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "LAYER\\WkLayer" + Tips0.IntToString(cam_number + 1) + ".ini";
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
 ** WkLayer*.iniファイルの作成
 ** 引数：
 **　int work_number	機種番号
 **　int cam_number		カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamLyer::CreateIniFileParam(int work_number, int cam_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFileParam(work_number, cam_number);

	//最後にiniファイルの確認
	if (CheckIniFileParam(work_number, cam_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** WkLayer*.iniファイルの書込
 ** 引数：
 **　int work_number	機種番号
 **　int cam_number		カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamLyer::WriteIniFileParam(int work_number, int cam_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "LAYER\\WkLayer" + Tips0.IntToString(cam_number + 1) + ".ini";

	//個別レイヤ1～3
	CString layer_name;
	CString rect_name;
	for (int i = 0; i < 3; i++) {
		//レイヤパラメータ
		layer_name = "Layer" + Tips0.IntToString(i + 1);		//例)Layer1
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].select_item);
		ret = WritePrivateProfileStringA(layer_name, "select_item", buf, filename);

		//矩形構造パラメータ
		rect_name = layer_name + "Rectangle";
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.enable);
		ret = WritePrivateProfileStringA(rect_name, "enable", buf, filename);
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.st_x);
		ret = WritePrivateProfileStringA(rect_name, "st_x", buf, filename);
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.st_y);
		ret = WritePrivateProfileStringA(rect_name, "st_y", buf, filename);
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.width);
		ret = WritePrivateProfileStringA(rect_name, "width", buf, filename);
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.height);
		ret = WritePrivateProfileStringA(rect_name, "height", buf, filename);
		buf = Tips0.IntToString(layer_param[cam_number].layer[i].rect.degree);
		ret = WritePrivateProfileStringA(rect_name, "degree", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** WkLayer*.iniファイルの読込
 ** 引数：
 **　int work_number	機種番号
 **　int cam_number		カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamLyer::ReadIniFileParam(int work_number, int cam_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "LAYER\\WkLayer" + Tips0.IntToString(cam_number + 1) + ".ini";

	//個別レイヤ1～3
	CString layer_name;
	CString rect_name;
	for (int i = 0; i < 3; i++) {
		//レイヤパラメータ
		layer_name = "Layer" + Tips0.IntToString(i + 1);		//例)Layer1
		ret = GetPrivateProfileStringA(layer_name, "select_item", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].select_item = atoi(buf);

		//矩形構造パラメータ
		rect_name = layer_name + "Rectangle";
		ret = GetPrivateProfileStringA(rect_name, "enable", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.enable = (BOOL)atoi(buf);
		ret = GetPrivateProfileStringA(rect_name, "st_x", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.st_x = atoi(buf);
		ret = GetPrivateProfileStringA(rect_name, "st_y", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.st_y = atoi(buf);
		ret = GetPrivateProfileStringA(rect_name, "width", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.width = atoi(buf);
		ret = GetPrivateProfileStringA(rect_name, "height", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.height = atoi(buf);
		ret = GetPrivateProfileStringA(rect_name, "degree", "", buf, sizeof(buf), filename);
		if (ret != 0) layer_param[cam_number].layer[i].rect.degree = atoi(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** 機種レイヤパラメータの全初期化
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CAccessParamLyer::InitParams()
{
	// TODO: ここに実装コードを追加します.
	for (int i = 0; i < 8; i++) {
		layer_param[i].InitialParam();
	}
}
