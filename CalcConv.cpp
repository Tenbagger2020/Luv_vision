/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcConv.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	カメラ座標不具合修正
 ** 注記：
 ** 説明：座標変換プログラム
 **
 **/

#include "stdafx.h"
#include "CalcConv.h"


CCalcConv::CCalcConv()
{
}


CCalcConv::~CCalcConv()
{
}


/** ****************************************************** **
 ** カメラ姿勢へ座標変換
 ** 引数：
 **　int coordinate_type	視野座標の構成タイプ
 **　double cal_base_x		Cal基準X
 **　double cal_base_y		Cal基準Y
 **　double mark_x			検出位置X
 **　double mark_y			検出位置Y
 **　double* crdnt_x		座標変換X
 **　double* crdnt_y		座標変換Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcConv::ConvDirection(int coordinate_type, double cal_base_x, double cal_base_y, double mark_x, double mark_y, double* crdnt_x, double* crdnt_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	if (coordinate_type < 0 || coordinate_type > 7) goto END;

	switch (coordinate_type) {
		//構成1
	case 0:
		*crdnt_x = mark_x - cal_base_x;
		*crdnt_y = mark_y - cal_base_y;
		break;
		//構成2
	case 1:
		*crdnt_x = mark_y - cal_base_y;
		*crdnt_y = (mark_x - cal_base_x) * -1;
		break;
		//構成3
	case 2:
		*crdnt_x = (mark_x - cal_base_x) * -1;
		*crdnt_y = (mark_y - cal_base_y) * -1;
		break;
		//構成4
	case 3:
		*crdnt_x = (mark_y - cal_base_y) * -1;
		*crdnt_y = mark_x - cal_base_x;
		break;
		//構成5
	case 4:
		*crdnt_x = (mark_x - cal_base_x) * -1;
		*crdnt_y = mark_y - cal_base_y;
		break;
		//構成6
	case 5:
		*crdnt_x = mark_y - cal_base_y;
		*crdnt_y = mark_x - cal_base_x;
		break;
		//構成7
	case 6:
		*crdnt_x = mark_x - cal_base_x;
		*crdnt_y = (mark_y - cal_base_y) * -1;
		break;
		//構成8
	case 7:
		*crdnt_x = (mark_y - cal_base_y) * -1;
		*crdnt_y = (mark_x - cal_base_x) * -1;
		break;
	}

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** メカ座標変換
 ** 引数：
 **　double crdnt_x			座標変換X
 **　double crdnt_y			座標変換Y
 **　double axis_x			軸位置X
 **　double axis_y			軸位置Y
 **　double pixel_size		画素サイズ
 **　double crdnt_radian	座標系傾き
 **　double* mech_x			メカ座標X
 **　double* mech_y			メカ座標Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcConv::ConvMechCoord(double crdnt_x, double crdnt_y, double axis_x, double axis_y, double pixel_size, double crdnt_radian, double* mech_x, double* mech_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	if (pixel_size == 0) goto END;

	//既存Windows7 FVXシステムの計算
	//　検証したがメカ座標への変換計算が不正解
	//　このコードは使用しない
	//double Xoffset = 0;
	//double Yoffset = 0;
	//double Dx = 0;
	//double Dy = 0;
	//
	////Nolb
	//Dx = crdnt_x * pixel_size;
	//Dy = crdnt_y * pixel_size;
	//
	////マーク画像座標->機械座標系に変換
	//Xoffset = cos(crdnt_radian) * Dx - sin(crdnt_radian) * Dy;
	//Yoffset = sin(crdnt_radian) * Dx + cos(crdnt_radian) * Dy;
	//
	//*mech_x = axis_x + Xoffset;
	//*mech_y = axis_y + Yoffset;
	//ここまで


	//独自に計算を行った場合
	//　検証を行い問題ないことを確認(検証 2020/04/02)
	double a = crdnt_x / cos(crdnt_radian);
	double i = a * sin(crdnt_radian);
	double u = crdnt_y - i;
	double e = u * sin(crdnt_radian);
	double x = a + e;
	double ans_mech_x = axis_x + x * pixel_size;

	double o = u * cos(crdnt_radian);
	double y = o;
	double ans_mech_y = axis_y + y * pixel_size;

	*mech_x = ans_mech_x;
	*mech_y = ans_mech_y;

	rtn_result = 0;

END:
	return rtn_result;
}
