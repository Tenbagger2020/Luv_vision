/**	Copyright (C) 2023 Hagiwara Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CommBit.h
 ** 作成者：M.Iwasaki
 ** 履歴：	2023.10.30	M.Iwasaki	10.14.2.0	初版
 ** 注記：
 ** 説明：シリアル通信データ(ビット制御)
 **		PLCとのシリアル通信データクラス
 **		PLC内部デバイス(X/Y)を制御する
 **
 **/

#include "stdafx.h"
#include "CommBit.h"


CCommBit::CCommBit()
{
	for (int i = 0; i < 16; i++) {
		indata_bit[i] = BIT_OFF;
		outdata_bit[i] = BIT_OFF;

		in_mem[i] = BIT_OFF;
		rise_bit[i] = FALSE;
	}

	comm_err = FALSE;
}


CCommBit::~CCommBit()
{
}
