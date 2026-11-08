/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DefinedElementMtd.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2023.11.08	M.Iwasaki	10.15.1.1	画像合成機能追加
 **			2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 ** 注記：
 ** 説明：機能パラメータ 各要素のコードと名称の紐づけデータベース
 **/

#include "stdafx.h"
#include "DefinedElementMtd.h"


CDefinedElementMtd::CDefinedElementMtd()
{
	/** ****************************************************** **
	 ** 検査機能
	 **		計測モードの検査タイプ
	 **		機能コード(4桁) search_method
	 **			0x0201:計測モード
	 **		検査タイプ(2桁) type
	 **			0x01:ブロブ
	 **			0x02:エッジ
	 **			0x03:濃度
	 **			0x04:マルチ探索
	 **			0x05:距離計測(基準-エッジ)
	 **			0x06:距離計測(エッジ-エッジ)
	 **			0x11:画像コピー(矩形)
	 **			0x21:計算
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	mes[0].search_method = 0x0201;
	mes[0].type = 0x00;
	mes[0].s_text = "無し";
	mes[0].e_text = "None";

	mes[1].search_method = 0x0201;
	mes[1].type = 0x01;
	mes[1].s_text = "ブロブ";
	mes[1].e_text = "Blob";

	mes[2].search_method = 0x0201;
	mes[2].type = 0x02;
	mes[2].s_text = "エッジ";
	mes[2].e_text = "Edge";

	mes[3].search_method = 0x0201;
	mes[3].type = 0x03;
	mes[3].s_text = "濃度";
	mes[3].e_text = "Brightness";

	mes[4].search_method = 0x0201;
	mes[4].type = 0x04;
	mes[4].s_text = "マルチ探索";
	mes[4].e_text = "MultiSearch";

	mes[5].search_method = 0x0201;
	mes[5].type = 0x05;
	mes[5].s_text = "距離計測(基準-エッジ)";
	mes[5].e_text = "Distance(Mark-Edge)";

	mes[6].search_method = 0x0201;
	mes[6].type = 0x06;
	mes[6].s_text = "距離計測(エッジ-エッジ)";
	mes[6].e_text = "Distance(Edge-Edge)";

	mes[7].search_method = 0x0201;
	mes[7].type = 0x11;
	mes[7].s_text = "画像コピー(矩形)";
	mes[7].e_text = "CopyImage(Rectangle)";

	mes[8].search_method = 0x0201;
	mes[8].type = 0x21;
	mes[8].s_text = "計算";
	mes[8].e_text = "Calculation";



	/** ****************************************************** **
	 ** 計算機能
	 **		計測モードの検査タイプ
	 **		検査タイプ(2桁) type
	 **			0x01:ブロブ
	 **			0x02:エッジ
	 **			0x03:濃度
	 **			0x04:マルチ探索
	 **			0x05:距離計測(基準-エッジ)
	 **			0x06:距離計測(エッジ-エッジ)
	 **			0x11:画像コピー(矩形)
	 **			0x21:計算
	 **		変数名 variable
	 **			名称
	 **		テキスト s_text
	 **			名称
	 **
	 ** ****************************************************** **/
	var[0].type = 0x04;
	var[0].variable = "count";
	var[0].s_text = "探索個数";

	var[1].type = 0x05;
	var[1].variable = "distance";
	var[1].s_text = "計測距離";

	var[2].type = 0x06;
	var[2].variable = "distance";
	var[2].s_text = "計測距離";

	var[3].type = 0x21;
	var[3].variable = "calclation1";
	var[3].s_text = "計算1";

	var[4].type = 0x21;
	var[4].variable = "calclation2";
	var[4].s_text = "計算2";

	var[5].type = 0x21;
	var[5].variable = "calclation3";
	var[5].s_text = "計算3";
}


CDefinedElementMtd::~CDefinedElementMtd()
{
}
