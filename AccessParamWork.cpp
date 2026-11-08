/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamWork.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：機種パラメータの読込と書込と作成
 **		iniファイルの確認と新規作成する
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamWork.h"

#include "direct.h"

extern char* APP_ROOT;

CAccessParamWork::CAccessParamWork()
{
}


CAccessParamWork::~CAccessParamWork()
{
}


/** ****************************************************** **
 ** Iniファイルディレクトリの確認
 ** 引数：
 **　int number		機種番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		ディレクトリがなければ作成する
 ** ****************************************************** **/
int CAccessParamWork::CheckDir(int number)
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
	CString s_work = Tips0.IntToString(number, 3);
	strcat_s(path, s_work);
	if (_chdir(path) != 0) {
		if (_mkdir(path) != 0) return 1;
	}
	if (_chdir(path) != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** Work.iniファイルの確認
 ** 引数：
 **　int number		機種番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamWork::CheckIniFileParam(int number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(number, 3) + "\\Work.ini";
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
 ** Work.iniファイルの作成
 ** 引数：
 **　int number		機種番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamWork::CreateIniFileParam(int number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CWorkParam new_param;

	//ディレクトリの確認
	if (CheckDir(number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	new_param.SetInitComment(number);
	new_param.SetCreateDate();
	WriteIniFileParam(number, &new_param);

	//最後にiniファイルの確認
	if (CheckIniFileParam(number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Work.iniファイルの書込
 ** 引数：
 **　int number			機種番号
 **　CWorkParam* param	機種パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamWork::WriteIniFileParam(int number, CWorkParam* param)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(number, 3) + "\\Work.ini";

	//登録パラメータ
	buf = Tips0.IntToString(param->regi.enable);
	ret = WritePrivateProfileStringA("Registration", "enable", buf, filename);
	buf = param->regi.comment;
	ret = WritePrivateProfileStringA("Registration", "comment", buf, filename);
	buf = param->regi.create_date;
	ret = WritePrivateProfileStringA("Registration", "create_date", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** Work.iniファイルの読込
 ** 引数：
 **　int number			機種番号
 **　CWorkParam* param	機種パラメータ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamWork::ReadIniFileParam(int number, CWorkParam* param)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(number, 3) + "\\Work.ini";

	//登録パラメータ
	ret = GetPrivateProfileStringA("Registration", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) param->regi.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("Registration", "comment", "", buf, sizeof(buf), filename);
	if (ret != 0) param->regi.comment = buf;
	ret = GetPrivateProfileStringA("Registration", "create_date", "", buf, sizeof(buf), filename);
	if (ret != 0) param->regi.create_date = buf;

	return 0;
}
