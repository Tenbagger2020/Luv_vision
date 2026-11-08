/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CommData.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：シリアル通信データ
 **		PLCとのシリアル通信データクラス
 **/

#include "stdafx.h"
#include "CommData.h"


CCommData::CCommData()
{
	InitialData();
}


CCommData::~CCommData()
{
}


/** ****************************************************** **
 ** データ初期化
 ** 引数：
 **　なし
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCommData::InitialData()
{
	// TODO: ここに実装コードを追加します.

	for (int i = 0; i < 20; i++) {
		w_data[i] = 0x0000;
	}
}
