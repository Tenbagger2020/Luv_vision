/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AccessParamTemp.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	結果出力パラメータ追加
 **			2022.09.12	M.Iwasaki	グレイサーチパラメータ追加
 **			2023.09.06	M.Iwasaki	10.13.0.0	TParameter.ini(FPM2サーチ極性)追加
 **			2023.11.08	M.Iwasaki	10.15.1.1	画像合成機能追加
 **			2026.06.05	M.Iwasaki	10.21.1.0	赤外カメラ対応
 ** 注記：
 ** 説明：機種毎テンプレートパラメータの読込と書込と作成
 **		iniファイルの確認と新規作成する
 **		iniファイルからパラメータを読み込む
 **		iniファイルへパラメータを書き込む
 **/

#include "stdafx.h"
#include "AccessParamTemp.h"

#include "TempParam.h"		//機種毎テンプレートパラメータ

#include "direct.h"

extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ

extern char* APP_ROOT;

CAccessParamTemp::CAccessParamTemp()
{
}


CAccessParamTemp::~CAccessParamTemp()
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
int CAccessParamTemp::CheckDir(int work_number, int temp_number)
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

	return 0;
}


/** ****************************************************** **
 ** TParameter.iniファイルの確認
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamTemp::CheckIniFileParam(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\TParameter.ini";
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
 ** TParameter.iniファイルの作成
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamTemp::CreateIniFileParam(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;	//結果

	//ディレクトリの確認
	if (CheckDir(work_number, temp_number) != 0) goto End;

	//ここでIniファイル作成しコメント書く

	//ファイル作成
	WriteIniFileParam(work_number, temp_number);

	//最後にiniファイルの確認
	if (CheckIniFileParam(work_number, temp_number) == 0) rtn_result = 0;

End:
	return rtn_result;
}


/** ****************************************************** **
 ** TParameter.iniファイルの書込
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamTemp::WriteIniFileParam(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	CString buf;
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\TParameter.ini";

	//作成日
	buf = template_param[temp_number].create_date;
	ret = WritePrivateProfileStringA("Image", "create_date", buf, filename);

	//パターン登録
	buf = Tips0.IntToString((int)template_param[temp_number].pattern.enable);
	ret = WritePrivateProfileStringA("Pattern", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.extract_st_x);
	ret = WritePrivateProfileStringA("Pattern", "extract_st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.extract_st_y);
	ret = WritePrivateProfileStringA("Pattern", "extract_st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.extract_width);
	ret = WritePrivateProfileStringA("Pattern", "extract_width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.extract_height);
	ret = WritePrivateProfileStringA("Pattern", "extract_height", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.detection_x);
	ret = WritePrivateProfileStringA("Pattern", "detection_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].pattern.detection_y);
	ret = WritePrivateProfileStringA("Pattern", "detection_y", buf, filename);

	//サーチエリア指定
	buf = Tips0.IntToString((int)template_param[temp_number].base_area.enable);
	ret = WritePrivateProfileStringA("SearchArea", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].base_area.st_x);
	ret = WritePrivateProfileStringA("SearchArea", "st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].base_area.st_y);
	ret = WritePrivateProfileStringA("SearchArea", "st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].base_area.width);
	ret = WritePrivateProfileStringA("SearchArea", "width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].base_area.height);
	ret = WritePrivateProfileStringA("SearchArea", "height", buf, filename);

	//探索基本
	buf = Tips0.IntToString((int)template_param[temp_number].basic.matching);
	ret = WritePrivateProfileStringA("SearchBasic", "matching", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].basic.judge_score);
	ret = WritePrivateProfileStringA("SearchBasic", "judge_score", buf, filename);

	//グレイサーチパラメータ
	buf = Tips0.IntToString(template_param[temp_number].gs2_param.precision);
	ret = WritePrivateProfileStringA("GS2Parameter", "precision", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].gs2_param.filter);
	ret = WritePrivateProfileStringA("GS2Parameter", "filter", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].gs2_param.threshold_mid);
	ret = WritePrivateProfileStringA("GS2Parameter", "threshold_mid", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].gs2_param.first_unit);
	ret = WritePrivateProfileStringA("GS2Parameter", "first_unit", buf, filename);

	//形状サーチパラメータ
	buf = Tips0.IntToString(template_param[temp_number].fpm2_param.angle);
	ret = WritePrivateProfileStringA("FPM2Parameter", "angle", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_param.scale);
	ret = WritePrivateProfileStringA("FPM2Parameter", "scale", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_param.custom_select);
	ret = WritePrivateProfileStringA("FPM2Parameter", "custom_select", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_param.polarity);
	ret = WritePrivateProfileStringA("FPM2Parameter", "polarity", buf, filename);

	//形状サーチ詳細パラメータ 基本パラメータ
	buf = Tips0.IntToString(template_param[temp_number].fpm2_base.comp_level);
	ret = WritePrivateProfileStringA("FPM2_Base", "comp_level", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_base.err_range);
	ret = WritePrivateProfileStringA("FPM2_Base", "err_range", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_base.score_threshL);
	ret = WritePrivateProfileStringA("FPM2_Base", "score_threshL", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_base.score_threshH);
	ret = WritePrivateProfileStringA("FPM2_Base", "score_threshH", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_base.select_edge);
	ret = WritePrivateProfileStringA("FPM2_Base", "select_edge", buf, filename);

	//形状サーチ詳細パラメータ 領域スコア算出用パラメータ
	buf = Tips0.IntToString((int)template_param[temp_number].fpm2_area.enable);
	ret = WritePrivateProfileStringA("FPM2_Area", "enable", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_area.err_range_tx);
	ret = WritePrivateProfileStringA("FPM2_Area", "err_range_tx", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_area.err_range_ty);
	ret = WritePrivateProfileStringA("FPM2_Area", "err_range_ty", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_area.err_range_tq);
	ret = WritePrivateProfileStringA("FPM2_Area", "err_range_tq", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_area.err_range_ts);
	ret = WritePrivateProfileStringA("FPM2_Area", "err_range_ts", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_area.err_range);
	ret = WritePrivateProfileStringA("FPM2_Area", "err_range", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_area.noise_weight);
	ret = WritePrivateProfileStringA("FPM2_Area", "noise_weight", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_area.score_threshold);
	ret = WritePrivateProfileStringA("FPM2_Area", "score_threshold", buf, filename);

	//形状サーチ詳細パラメータ 不可欠領域スコア算出用パラメータ
	buf = Tips0.IntToString((int)template_param[temp_number].fpm2_esse.enable);
	ret = WritePrivateProfileStringA("FPM2_Essential", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_esse.err_range);
	ret = WritePrivateProfileStringA("FPM2_Essential", "err_range", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_esse.score_threshold);
	ret = WritePrivateProfileStringA("FPM2_Essential", "score_threshold", buf, filename);

	//形状サーチ詳細パラメータ 相関エッジパラメータ
	buf = Tips0.IntToString(template_param[temp_number].fpm2_corre.region_width);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "region_width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_corre.region_height);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "region_height", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_corre.sigmoidK);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "sigmoidK", buf, filename);
	buf = Tips0.DblToString(template_param[temp_number].fpm2_corre.var_threshold);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "var_threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_corre.edge_threshold);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "edge_threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_corre.nms_length);
	ret = WritePrivateProfileStringA("FPM2_Correlation", "nms_length", buf, filename);

	//形状サーチ詳細パラメータ ソーベルエッジパラメータ
	buf = Tips0.IntToString(template_param[temp_number].fpm2_sobel.edge_threshold);
	ret = WritePrivateProfileStringA("FPM2_Sobel", "edge_threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].fpm2_sobel.nms_length);
	ret = WritePrivateProfileStringA("FPM2_Sobel", "nms_length", buf, filename);

	//追加機能1
	buf = Tips0.IntToString((int)template_param[temp_number].additional.add_type);
	ret = WritePrivateProfileStringA("SearchAdditional", "add_type", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].additional.base_search_enable);
	ret = WritePrivateProfileStringA("SearchAdditional", "base_search_enable", buf, filename);

	//追加機能1 エッジサーチ 縦エリア
	buf = Tips0.IntToString((int)template_param[temp_number].additional.edge_area[0].enable);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].st_x);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].st_y);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].width);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].height);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "height", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].mark_dis_x);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "mark_dis_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[0].mark_dis_y);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertArea", "mark_dis_y", buf, filename);

	//追加機能1 エッジサーチ 縦閾値
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].number);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "number", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].scan_direction);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "scan_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].line_width);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "line_width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].edge_direction);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "edge_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].len);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "len", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].foot);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "foot", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].additional.edge_param[0].edge_peak);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "edge_peak", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].threshold);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].around);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "around", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].max_threshold);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "max_threshold", buf, filename);

	//追加機能1 エッジサーチ 縦判定
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].judge_score);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "judge_score", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].judge_area_lower);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "judge_area_lower", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[0].judge_area_upper);
	ret = WritePrivateProfileStringA("AdditionalEdgeVertDetc", "judge_area_upper", buf, filename);

	//追加機能1 エッジサーチ 横エリア
	buf = Tips0.IntToString((int)template_param[temp_number].additional.edge_area[1].enable);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].st_x);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].st_y);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].width);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].height);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "height", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].mark_dis_x);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "mark_dis_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_area[1].mark_dis_y);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzArea", "mark_dis_y", buf, filename);

	//追加機能1 エッジサーチ 横閾値
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].number);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "number", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].scan_direction);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "scan_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].line_width);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "line_width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].edge_direction);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "edge_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].len);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "len", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].foot);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "foot", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].additional.edge_param[1].edge_peak);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "edge_peak", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].threshold);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].around);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "around", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].max_threshold);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "max_threshold", buf, filename);

	//追加機能1 エッジサーチ 横判定
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].judge_score);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_score", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].judge_area_lower);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_area_lower", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.edge_param[1].judge_area_upper);
	ret = WritePrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_area_upper", buf, filename);

	//追加機能1 2値化重心サーチ エリア
	buf = Tips0.IntToString((int)template_param[temp_number].additional.blob_area.enable);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.st_x);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.st_y);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.width);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.height);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "height", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.mark_dis_x);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "mark_dis_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_area.mark_dis_y);
	ret = WritePrivateProfileStringA("AdditionalBlobArea", "mark_dis_y", buf, filename);

	//追加機能1 2値化重心サーチ 2値化
	buf = Tips0.IntToString((int)template_param[temp_number].additional.blob_param.base_code);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "base_code", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].additional.blob_param.type);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "type", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_param.fix_threshoid[0]);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "fix_threshoid1", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].additional.blob_param.fix_threshoid[1]);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "fix_threshoid2", buf, filename);

	//追加機能1 2値化重心サーチ 抽出
	buf = Tips0.IntToString((int)template_param[temp_number].additional.blob_param.color_mode);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "color_mode", buf, filename);
	buf = Tips0.UintToString(template_param[temp_number].additional.blob_param.area_min);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "area_min", buf, filename);
	buf = Tips0.UintToString(template_param[temp_number].additional.blob_param.area_max);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "area_max", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].additional.blob_param.enable_area_unlimit);
	ret = WritePrivateProfileStringA("AdditionalBlobBinz", "enable_area_unlimit", buf, filename);

	//追加機能2
	buf = Tips0.IntToString((int)template_param[temp_number].calc_add.add_type);
	ret = WritePrivateProfileStringA("SearchCalcAdd", "add_type", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].calc_add.base_search_enable);
	ret = WritePrivateProfileStringA("SearchCalcAdd", "base_search_enable", buf, filename);

	//追加機能2 傾き算出 エリア
	buf = Tips0.IntToString((int)template_param[temp_number].calc_add.tilt_area.enable);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "enable", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.st_x);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "st_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.st_y);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "st_y", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.width);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.height);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "height", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.area_degree);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "area_degree", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.mark_dis_x);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "mark_dis_x", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_area.mark_dis_y);
	ret = WritePrivateProfileStringA("CalcAddlTiltArea", "mark_dis_y", buf, filename);

	//追加機能2 傾き算出 閾値
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.number);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "number", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.scan_direction);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "scan_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.line_width);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "line_width", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.edge_direction);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "edge_direction", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.len);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "len", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.foot);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "foot", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].calc_add.tilt_param.edge_peak);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "edge_peak", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.threshold);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "threshold", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.around);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "around", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.max_threshold);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "max_threshold", buf, filename);

	//追加機能2 傾き算出 判定
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.judge_score);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "judge_score", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.judge_area_lower);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "judge_area_lower", buf, filename);
	buf = Tips0.IntToString(template_param[temp_number].calc_add.tilt_param.judge_area_upper);
	ret = WritePrivateProfileStringA("CalcAddlTiltDetc", "judge_area_upper", buf, filename);

	//結果出力パラメータ
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.enable);
	ret = WritePrivateProfileStringA("OutResult", "enable", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.enable_sub_folder);
	ret = WritePrivateProfileStringA("OutResult", "enable_sub_folder", buf, filename);
	buf = "\"" + template_param[temp_number].out_result.format_folder + "\"";		//文字列の先頭または終わりに\"が入る場合の対応
	ret = WritePrivateProfileStringA("OutResult", "format_folder", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.enable_result);
	ret = WritePrivateProfileStringA("OutResult", "enable_result", buf, filename);
	buf = "\"" + template_param[temp_number].out_result.format_result + "\"";		//文字列の先頭または終わりに\"が入る場合の対応
	ret = WritePrivateProfileStringA("OutResult", "format_result", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.enable_image);
	ret = WritePrivateProfileStringA("OutResult", "enable_image", buf, filename);
	buf = "\"" + template_param[temp_number].out_result.format_image + "\"";		//文字列の先頭または終わりに\"が入る場合の対応
	ret = WritePrivateProfileStringA("OutResult", "format_image", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.type_image);
	ret = WritePrivateProfileStringA("OutResult", "type_image", buf, filename);
	buf = Tips0.IntToString((int)template_param[temp_number].out_result.style_image);
	ret = WritePrivateProfileStringA("OutResult", "style_image", buf, filename);

	//画像作成(1～4)
	for (int i = 0; i < 4; i++) {
		CString name = "MakeImage" + Tips0.IntToString(i + 1);	//例)MakeImage1
		buf = Tips0.IntToString((int)template_param[temp_number].make_image[i].make_type);
		ret = WritePrivateProfileStringA(name, "make_type", buf, filename);

		buf = Tips0.IntToString((int)template_param[temp_number].make_image[i].copy_template);
		ret = WritePrivateProfileStringA(name, "copy_template", buf, filename);
	}

	return 0;
}


/** ****************************************************** **
 ** TParameter.iniファイルの読込
 ** 引数：
 **　int work_number	機種番号
 **　int temp_number	テンプレート番号
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAccessParamTemp::ReadIniFileParam(int work_number, int temp_number)
{
	// TODO: ここに実装コードを追加します.
	char buf[256];
	DWORD ret;

	CString filename = (CString)APP_ROOT + "\\WORK\\"
		+ Tips0.IntToString(work_number, 3) + "\\"
		+ "T" + Tips0.IntToString(temp_number, 3) + "\\TParameter.ini";

	//作成日
	ret = GetPrivateProfileStringA("Image", "create_date", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].create_date = buf;

	//パターン登録
	ret = GetPrivateProfileStringA("Pattern", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "extract_st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.extract_st_x = atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "extract_st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.extract_st_y = atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "extract_width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.extract_width = atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "extract_height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.extract_height = atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "detection_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.detection_x = atoi(buf);
	ret = GetPrivateProfileStringA("Pattern", "detection_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].pattern.detection_y = atoi(buf);

	//サーチエリア指定
	ret = GetPrivateProfileStringA("SearchArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].base_area.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("SearchArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].base_area.st_x = atoi(buf);
	ret = GetPrivateProfileStringA("SearchArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].base_area.st_y = atoi(buf);
	ret = GetPrivateProfileStringA("SearchArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].base_area.width = atoi(buf);
	ret = GetPrivateProfileStringA("SearchArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].base_area.height = atoi(buf);

	//探索基本
	ret = GetPrivateProfileStringA("SearchBasic", "matching", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].basic.matching = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("SearchBasic", "judge_score", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].basic.judge_score = atoi(buf);

	//グレイサーチパラメータ
	ret = GetPrivateProfileStringA("GS2Parameter", "precision", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].gs2_param.precision = atoi(buf);
	ret = GetPrivateProfileStringA("GS2Parameter", "filter", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].gs2_param.filter = atoi(buf);
	ret = GetPrivateProfileStringA("GS2Parameter", "threshold_mid", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].gs2_param.threshold_mid = atoi(buf);
	ret = GetPrivateProfileStringA("GS2Parameter", "first_unit", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].gs2_param.first_unit = atoi(buf);

	//形状サーチパラメータ
	ret = GetPrivateProfileStringA("FPM2Parameter", "angle", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_param.angle = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2Parameter", "scale", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_param.scale = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2Parameter", "custom_select", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_param.custom_select = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2Parameter", "polarity", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_param.polarity = atoi(buf);

	//形状サーチ詳細パラメータ 基本パラメータ
	ret = GetPrivateProfileStringA("FPM2_Base", "comp_level", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_base.comp_level = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Base", "err_range", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_base.err_range = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Base", "score_threshL", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_base.score_threshL = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Base", "score_threshH", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_base.score_threshH = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Base", "select_edge", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_base.select_edge = atoi(buf);

	//形状サーチ詳細パラメータ 領域スコア算出用パラメータ
	ret = GetPrivateProfileStringA("FPM2_Area", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.enable = (bool)atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "err_range_tx", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.err_range_tx = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "err_range_ty", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.err_range_ty = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "err_range_tq", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.err_range_tq = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "err_range_ts", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.err_range_ts = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "err_range", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.err_range = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "noise_weight", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.noise_weight = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Area", "score_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_area.score_threshold = atoi(buf);

	//形状サーチ詳細パラメータ 不可欠領域スコア算出用パラメータ
	ret = GetPrivateProfileStringA("FPM2_Essential", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_esse.enable = (bool)atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Essential", "err_range", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_esse.err_range = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Essential", "score_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_esse.score_threshold = atoi(buf);

	//形状サーチ詳細パラメータ 相関エッジパラメータ
	ret = GetPrivateProfileStringA("FPM2_Correlation", "region_width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.region_width = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Correlation", "region_height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.region_height = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Correlation", "sigmoidK", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.sigmoidK = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Correlation", "var_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.var_threshold = atof(buf);
	ret = GetPrivateProfileStringA("FPM2_Correlation", "edge_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.edge_threshold = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Correlation", "nms_length", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_corre.nms_length = atoi(buf);

	//形状サーチ詳細パラメータ ソーベルエッジパラメータ
	ret = GetPrivateProfileStringA("FPM2_Sobel", "edge_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_sobel.edge_threshold = atoi(buf);
	ret = GetPrivateProfileStringA("FPM2_Sobel", "nms_length", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].fpm2_sobel.nms_length = atoi(buf);

	//追加機能1
	ret = GetPrivateProfileStringA("SearchAdditional", "add_type", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.add_type = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("SearchAdditional", "base_search_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.base_search_enable = (BOOL)atoi(buf);

	//追加機能1 エッジサーチ 縦エリア
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].st_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].st_y = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].width = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].height = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "mark_dis_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].mark_dis_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertArea", "mark_dis_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[0].mark_dis_y = atoi(buf);

	//追加機能1 エッジサーチ 縦閾値
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "number", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].number = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "scan_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].scan_direction = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "line_width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].line_width = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "edge_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].edge_direction = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "len", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].len = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "foot", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].foot = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "edge_peak", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].edge_peak = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].threshold = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "around", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].around = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "max_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].max_threshold = atoi(buf);

	//追加機能1 エッジサーチ 縦判定
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "judge_score", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].judge_score = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "judge_area_lower", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].judge_area_lower = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeVertDetc", "judge_area_upper", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[0].judge_area_upper = atoi(buf);

	//追加機能1 エッジサーチ 横エリア
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].st_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].st_y = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].width = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].height = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "mark_dis_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].mark_dis_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzArea", "mark_dis_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_area[1].mark_dis_y = atoi(buf);

	//追加機能1 エッジサーチ 横閾値
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "number", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].number = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "scan_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].scan_direction = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "line_width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].line_width = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "edge_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].edge_direction = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "len", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].len = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "foot", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].foot = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "edge_peak", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].edge_peak = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].threshold = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "around", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].around = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "max_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].max_threshold = atoi(buf);

	//追加機能1 エッジサーチ 横判定
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_score", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].judge_score = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_area_lower", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].judge_area_lower = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalEdgeHorzDetc", "judge_area_upper", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.edge_param[1].judge_area_upper = atoi(buf);

	//追加機能1 2値化重心サーチ エリア
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.st_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.st_y = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.width = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.height = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "mark_dis_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.mark_dis_x = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobArea", "mark_dis_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_area.mark_dis_y = atoi(buf);

	//追加機能1 2値化重心サーチ 2値化
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "base_code", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.base_code = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "type", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.type = (WORD)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "fix_threshoid1", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.fix_threshoid[0] = atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "fix_threshoid2", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.fix_threshoid[1] = atoi(buf);

	//追加機能1 2値化重心サーチ 抽出
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "color_mode", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.color_mode = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "area_min", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.area_min = Tips0.CharToUint(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "area_max", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.area_max = Tips0.CharToUint(buf);
	ret = GetPrivateProfileStringA("AdditionalBlobBinz", "enable_area_unlimit", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].additional.blob_param.enable_area_unlimit = (BOOL)atoi(buf);

	//追加機能2
	ret = GetPrivateProfileStringA("SearchCalcAdd", "add_type", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.add_type = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("SearchCalcAdd", "base_search_enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.base_search_enable = (BOOL)atoi(buf);

	//追加機能2 傾き算出 エリア
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "st_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.st_x = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "st_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.st_y = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.width = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "height", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.height = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "area_degree", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.area_degree = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "mark_dis_x", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.mark_dis_x = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltArea", "mark_dis_y", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_area.mark_dis_y = atoi(buf);

	//追加機能2 傾き算出 閾値
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "number", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.number = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "scan_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.scan_direction = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "line_width", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.line_width = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "edge_direction", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.edge_direction = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "len", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.len = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "foot", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.foot = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "edge_peak", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.edge_peak = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.threshold = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "around", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.around = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "max_threshold", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.max_threshold = atoi(buf);

	//追加機能2 傾き算出 判定
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "judge_score", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.judge_score = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "judge_area_lower", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.judge_area_lower = atoi(buf);
	ret = GetPrivateProfileStringA("CalcAddlTiltDetc", "judge_area_upper", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].calc_add.tilt_param.judge_area_upper = atoi(buf);

	//結果出力パラメータ
	ret = GetPrivateProfileStringA("OutResult", "enable", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.enable = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("OutResult", "enable_sub_folder", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.enable_sub_folder = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("OutResult", "format_folder", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.format_folder = (CString)buf;
	ret = GetPrivateProfileStringA("OutResult", "enable_result", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.enable_result = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("OutResult", "format_result", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.format_result = (CString)buf;
	ret = GetPrivateProfileStringA("OutResult", "enable_image", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.enable_image = (BOOL)atoi(buf);
	ret = GetPrivateProfileStringA("OutResult", "format_image", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.format_image = (CString)buf;
	ret = GetPrivateProfileStringA("OutResult", "type_image", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.type_image = (byte)atoi(buf);
	ret = GetPrivateProfileStringA("OutResult", "style_image", "", buf, sizeof(buf), filename);
	if (ret != 0) template_param[temp_number].out_result.style_image = (byte)atoi(buf);

	//画像作成(1～4)
	for (int i = 0; i < 4; i++) {
		CString name = "MakeImage" + Tips0.IntToString(i + 1);	//例)MakeImage1
		ret = GetPrivateProfileStringA(name, "make_type", "", buf, sizeof(buf), filename);
		if (ret != 0) template_param[temp_number].make_image[i].make_type = (byte)atoi(buf);

		ret = GetPrivateProfileStringA(name, "copy_template", "", buf, sizeof(buf), filename);
		if (ret != 0) template_param[temp_number].make_image[i].copy_template = (byte)atoi(buf);
	}

	return 0;
}


/** ****************************************************** **
 ** 機種毎テンプレートパラメータの全初期化
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CAccessParamTemp::InitParams()
{
	// TODO: ここに実装コードを追加します.
	for (int i = 0; i < 256; i++) {
		template_param[i].InitialParam();
	}
}
