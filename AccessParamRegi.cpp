/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamRegi.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.09.01	M.Iwasaki	カメラパラメータ設定追加
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 **			2023.04.03	M.Iwasaki	東芝テリー 画像反転対応
 **			2024.05.17	M.Iwasaki	10.17.2.0	オムロンセンテックカメラ対応
 ** 注記：
 ** 説明：カメラ内部パラメータの読込と書込
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamRegi.h"

#include "CameraSettings.h"

#include "direct.h"

extern CCameraSettings		cam_parameter[8];	//カメラパラメータ

extern char* APP_ROOT;

CAccessParamRegi::CAccessParamRegi()
{
}


CAccessParamRegi::~CAccessParamRegi()
{
}


/** ****************************************************** **
 ** Iniファイルディレクトリの確認
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamRegi::CheckDir()
{
	// TODO: ここに実装コードを追加します.
	if (_chdir((CString)APP_ROOT) != 0) {
		if (_mkdir((CString)APP_ROOT) != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT) != 0) return 1;
	if (_chdir((CString)APP_ROOT + "\\SYSTEM") != 0) {
		if (_mkdir((CString)APP_ROOT + "\\SYSTEM") != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT + "\\SYSTEM") != 0) return 1;
	if (_chdir((CString)APP_ROOT + "\\SYSTEM\\REGI") != 0) {
		if (_mkdir((CString)APP_ROOT + "\\SYSTEM\\REGI") != 0) return 1;
	}
	if (_chdir((CString)APP_ROOT + "\\SYSTEM\\REGI") != 0) return 1;

	return 0;
}


/** ****************************************************** **
 ** Regist*.iniファイルの確認
 ** 引数：
 **　int cam_number　カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamRegi::CheckIniFile(int cam_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\REGI\\Regist" + Tips0.IntToString(cam_number + 1) + ".ini";
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
 ** Regist*.iniファイルの作成
 ** 引数：
 **　int cam_number　カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamRegi::CreateIniFile(int cam_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir() != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//デフォルト設定でファイル作成
	WriteIniFile(cam_number);

	//最後にiniファイルの確認
	if (CheckIniFile(cam_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** Regist*.iniファイルの書込
 ** 引数：
 **　int cam_number　カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamRegi::WriteIniFile(int cam_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	if (cam_number < 0 || cam_number > 7) return 1;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\REGI\\Regist" + Tips0.IntToString(cam_number + 1) + ".ini";

	//通信ポート
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.com_port);
	ret = WritePrivateProfileStringA("Interface", "com_port", buf, filename);

	//
	// 東芝テリー
	//

	//BC040M
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc040m.clk);
	ret = WritePrivateProfileStringA("BC040M", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc040m.tap);
	ret = WritePrivateProfileStringA("BC040M", "tap", buf, filename);
	buf = Tips0.HexToStringW(cam_parameter[cam_number].regi_param.bc040m.exposure_time);
	ret = WritePrivateProfileStringA("BC040M", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc040m.flip);
	ret = WritePrivateProfileStringA("BC040M", "flip", buf, filename);

	//BC160M
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc160m.clk);
	ret = WritePrivateProfileStringA("BC160M", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc160m.tap);
	ret = WritePrivateProfileStringA("BC160M", "tap", buf, filename);
	buf = Tips0.HexToStringW(cam_parameter[cam_number].regi_param.bc160m.exposure_time);
	ret = WritePrivateProfileStringA("BC160M", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc160m.flip);
	ret = WritePrivateProfileStringA("BC160M", "flip", buf, filename);

	//BC302LMG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc302lmg.clk);
	ret = WritePrivateProfileStringA("BC302LMG", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc302lmg.tap);
	ret = WritePrivateProfileStringA("BC302LMG", "tap", buf, filename);
	buf = Tips0.HexToStringW(cam_parameter[cam_number].regi_param.bc302lmg.exposure_time);
	ret = WritePrivateProfileStringA("BC302LMG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc302lmg.flip);
	ret = WritePrivateProfileStringA("BC302LMG", "flip", buf, filename);

	//BC505LMG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc505lmg.clk);
	ret = WritePrivateProfileStringA("BC505LMG", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc505lmg.tap);
	ret = WritePrivateProfileStringA("BC505LMG", "tap", buf, filename);
	buf = Tips0.HexToStringW(cam_parameter[cam_number].regi_param.bc505lmg.exposure_time);
	ret = WritePrivateProfileStringA("BC505LMG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bc505lmg.flip);
	ret = WritePrivateProfileStringA("BC505LMG", "flip", buf, filename);

	//BU040MG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu040mg.exposure_time);
	ret = WritePrivateProfileStringA("BU040MG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu040mg.flip);
	ret = WritePrivateProfileStringA("BU040MG", "flip", buf, filename);

	//BU160MG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu160mg.exposure_time);
	ret = WritePrivateProfileStringA("BU160MG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu160mg.flip);
	ret = WritePrivateProfileStringA("BU160MG", "flip", buf, filename);

	//BU302MG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu302mg.exposure_time);
	ret = WritePrivateProfileStringA("BU302MG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu302mg.flip);
	ret = WritePrivateProfileStringA("BU302MG", "flip", buf, filename);

	//BU505MG
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu505mg.exposure_time);
	ret = WritePrivateProfileStringA("BU505MG", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.bu505mg.flip);
	ret = WritePrivateProfileStringA("BU505MG", "flip", buf, filename);

	//
	// VIEWORKS
	//

	//VC-25MC-M30D0
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.vc25mc.exposure_time);
	ret = WritePrivateProfileStringA("VC-25MC", "exposure_time", buf, filename);

	//
	// OMRONSENTECH
	//

	//STC-SPB43PCL
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb43pcl.clk);
	ret = WritePrivateProfileStringA("SPB43PCL", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb43pcl.tap);
	ret = WritePrivateProfileStringA("SPB43PCL", "tap", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb43pcl.exposure_time);
	ret = WritePrivateProfileStringA("SPB43PCL", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb43pcl.flip);
	ret = WritePrivateProfileStringA("SPB43PCL", "flip", buf, filename);

	//STC-SPB163PCL
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb163pcl.clk);
	ret = WritePrivateProfileStringA("SPB163PCL", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb163pcl.tap);
	ret = WritePrivateProfileStringA("SPB163PCL", "tap", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb163pcl.exposure_time);
	ret = WritePrivateProfileStringA("SPB163PCL", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb163pcl.flip);
	ret = WritePrivateProfileStringA("SPB163PCL", "flip", buf, filename);

	//STC-SPB312PCL
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb312pcl.clk);
	ret = WritePrivateProfileStringA("SPB312PCL", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb312pcl.tap);
	ret = WritePrivateProfileStringA("SPB312PCL", "tap", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb312pcl.exposure_time);
	ret = WritePrivateProfileStringA("SPB312PCL", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb312pcl.flip);
	ret = WritePrivateProfileStringA("SPB312PCL", "flip", buf, filename);

	//STC-SPB500PCL
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb500pcl.clk);
	ret = WritePrivateProfileStringA("SPB500PCL", "clk", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb500pcl.tap);
	ret = WritePrivateProfileStringA("SPB500PCL", "tap", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb500pcl.exposure_time);
	ret = WritePrivateProfileStringA("SPB500PCL", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.spb500pcl.flip);
	ret = WritePrivateProfileStringA("SPB500PCL", "flip", buf, filename);

	//STC-MBS43U3V
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs43u3v.exposure_time);
	ret = WritePrivateProfileStringA("MBS43U3V", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs43u3v.flip);
	ret = WritePrivateProfileStringA("MBS43U3V", "flip", buf, filename);

	//STC-MBS163U3V
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs163u3v.exposure_time);
	ret = WritePrivateProfileStringA("MBS163U3V", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs163u3v.flip);
	ret = WritePrivateProfileStringA("MBS163U3V", "flip", buf, filename);

	//STC-MBS322U3V
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs322u3v.exposure_time);
	ret = WritePrivateProfileStringA("MBS322U3V", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs322u3v.flip);
	ret = WritePrivateProfileStringA("MBS322U3V", "flip", buf, filename);

	//STC-MBS510U3V
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs510u3v.exposure_time);
	ret = WritePrivateProfileStringA("MBS510U3V", "exposure_time", buf, filename);
	buf = Tips0.IntToString(cam_parameter[cam_number].regi_param.mbs510u3v.flip);
	ret = WritePrivateProfileStringA("MBS510U3V", "flip", buf, filename);

	return 0;
}


/** ****************************************************** **
 ** Regist*.iniファイルの読込
 ** 引数：
 **　int cam_number　カメラ番号(0～7)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamRegi::ReadIniFile(int cam_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[32];
	DWORD ret;

	if (cam_number < 0 || cam_number > 7) return 1;

	CString filename = (CString)APP_ROOT + "\\SYSTEM\\REGI\\Regist" + Tips0.IntToString(cam_number + 1) + ".ini";

	//通信ポート
	ret = GetPrivateProfileStringA("Interface", "com_port", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.com_port = atoi(buf);

	//
	// 東芝テリー
	//

	//BC040M
	ret = GetPrivateProfileStringA("BC040M", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc040m.clk = atoi(buf);
	ret = GetPrivateProfileStringA("BC040M", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc040m.tap = atoi(buf);
	ret = GetPrivateProfileStringA("BC040M", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc040m.exposure_time = (DWORD)Tips0.CharToBin(buf, 16);
	ret = GetPrivateProfileStringA("BC040M", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc040m.flip = atoi(buf);

	//BC160M
	ret = GetPrivateProfileStringA("BC160M", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc160m.clk = atoi(buf);
	ret = GetPrivateProfileStringA("BC160M", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc160m.tap = atoi(buf);
	ret = GetPrivateProfileStringA("BC160M", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc160m.exposure_time = (DWORD)Tips0.CharToBin(buf, 16);
	ret = GetPrivateProfileStringA("BC160M", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc160m.flip = atoi(buf);

	//BC302LMG
	ret = GetPrivateProfileStringA("BC302LMG", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc302lmg.clk = atoi(buf);
	ret = GetPrivateProfileStringA("BC302LMG", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc302lmg.tap = atoi(buf);
	ret = GetPrivateProfileStringA("BC302LMG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc302lmg.exposure_time = (DWORD)Tips0.CharToBin(buf, 16);
	ret = GetPrivateProfileStringA("BC302LMG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc302lmg.flip = atoi(buf);

	//BC505LMG
	ret = GetPrivateProfileStringA("BC505LMG", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc505lmg.clk = atoi(buf);
	ret = GetPrivateProfileStringA("BC505LMG", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc505lmg.tap = atoi(buf);
	ret = GetPrivateProfileStringA("BC505LMG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc505lmg.exposure_time = (DWORD)Tips0.CharToBin(buf, 16);
	ret = GetPrivateProfileStringA("BC505LMG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bc505lmg.flip = atoi(buf);

	//BU040MG
	ret = GetPrivateProfileStringA("BU040MG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu040mg.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("BU040MG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu040mg.flip = atoi(buf);

	//BU160MG
	ret = GetPrivateProfileStringA("BU160MG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu160mg.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("BU160MG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu160mg.flip = atoi(buf);

	//BU302MG
	ret = GetPrivateProfileStringA("BU302MG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu302mg.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("BU302MG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu302mg.flip = atoi(buf);

	//BU505MG
	ret = GetPrivateProfileStringA("BU505MG", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu505mg.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("BU505MG", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.bu505mg.flip = atoi(buf);

	//
	// VIEWORKS
	//

	//VC-25MC-M30D0
	ret = GetPrivateProfileStringA("VC-25MC", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.vc25mc.exposure_time = atoi(buf);

	//
	// OMRONSENTECH
	//

	//STC-SPB43PCL
	ret = GetPrivateProfileStringA("SPB43PCL", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb43pcl.clk = atoi(buf);
	ret = GetPrivateProfileStringA("SPB43PCL", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb43pcl.tap = atoi(buf);
	ret = GetPrivateProfileStringA("SPB43PCL", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb43pcl.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("SPB43PCL", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb43pcl.flip = atoi(buf);

	//STC-SPB163PCL
	ret = GetPrivateProfileStringA("SPB163PCL", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb163pcl.clk = atoi(buf);
	ret = GetPrivateProfileStringA("SPB163PCL", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb163pcl.tap = atoi(buf);
	ret = GetPrivateProfileStringA("SPB163PCL", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb163pcl.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("SPB163PCL", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb163pcl.flip = atoi(buf);

	//STC-SPB312PCL
	ret = GetPrivateProfileStringA("SPB312PCL", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb312pcl.clk = atoi(buf);
	ret = GetPrivateProfileStringA("SPB312PCL", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb312pcl.tap = atoi(buf);
	ret = GetPrivateProfileStringA("SPB312PCL", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb312pcl.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("SPB312PCL", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb312pcl.flip = atoi(buf);

	//STC-SPB500PCL
	ret = GetPrivateProfileStringA("SPB500PCL", "clk", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb500pcl.clk = atoi(buf);
	ret = GetPrivateProfileStringA("SPB500PCL", "tap", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb500pcl.tap = atoi(buf);
	ret = GetPrivateProfileStringA("SPB500PCL", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb500pcl.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("SPB500PCL", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.spb500pcl.flip = atoi(buf);

	//STC-MBS43U3V
	ret = GetPrivateProfileStringA("MBS43U3V", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs43u3v.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("MBS43U3V", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs43u3v.flip = atoi(buf);

	//STC-MBS163U3V
	ret = GetPrivateProfileStringA("MBS163U3V", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs163u3v.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("MBS163U3V", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs163u3v.flip = atoi(buf);

	//STC-MBS322U3V
	ret = GetPrivateProfileStringA("MBS322U3V", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs322u3v.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("MBS322U3V", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs322u3v.flip = atoi(buf);

	//STC-MBS510U3V
	ret = GetPrivateProfileStringA("MBS510U3V", "exposure_time", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs510u3v.exposure_time = atoi(buf);
	ret = GetPrivateProfileStringA("MBS510U3V", "flip", "", buf, sizeof(buf), filename);
	if (ret != 0) cam_parameter[cam_number].regi_param.mbs510u3v.flip = atoi(buf);

	return 0;
}
