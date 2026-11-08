/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcMark.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：サーチ結果取得プログラム
 **
 **/

#include "stdafx.h"
#include "CalcMark.h"

#include "FVCLbasic.h"	//WIL FVCLクラスヘッダ

CCalcMark::CCalcMark()
{
}


CCalcMark::~CCalcMark()
{
}


/** ****************************************************** **
 ** グレイサーチ 検出位置取得
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　double* mark_x				検出位置X
 **　double* mark_y				検出位置Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcMark::GetMarkGS2(CTempParam* temp_param, double* mark_x, double* mark_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	F_GS_RESULT* data_ptr = temp_param->gs2_result.GetAddress();	//結果先頭アドレスの取得

	//探索個数
	UINT count_result = temp_param->gs2_result.GetCount();
	//探索成功
	if (count_result == 1) {
		//検出位置
		FVCL::Data::CFvPoint mark((double)data_ptr[0].x / (double)100, (double)data_ptr[0].y / (double)100);
		*mark_x = mark.x;
		*mark_y = mark.y;

		rtn_result = 0;	//検出位置取得成功
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 形状サーチ 検出位置取得
 ** 引数：
 **　CTempParam* temp_param		探索パラメータ
 **　double* mark_x				検出位置X
 **　double* mark_y				検出位置Y
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcMark::GetMarkFPM2(CTempParam* temp_param, double* mark_x, double* mark_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	int final_count = 0;	//閾値より上の数

	//探索個数
	UINT count_result = temp_param->fpm2_matching.GetResultSize();
	//探索成功
	if (count_result == 1) {
		FVCL::FPM2::CFvFPM2Data data = temp_param->fpm2_matching[0];

		// マッチング結果の取得.
		FVCL::Data::CFvPoint pos = data.GetPosition();      // 回答位置.
		FVCL::Data::CFvAngle angle = data.GetAngle();       // 回転角

		double scale = data.GetScale();                     // スケール (％)
		int score = data.GetScore();                        // スコア (％)

		if (score >= temp_param->basic.judge_score) {
			*mark_x = pos.x;
			*mark_y = pos.y;

			rtn_result = 0;	//検出位置取得成功
		}
	}

	return rtn_result;
}
