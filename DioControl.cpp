/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DioControl.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2023.10.30	M.Iwasaki	10.14.2.0	東芝テリーUSBカメラ対応
 ** 注記：
 ** 説明：DIOを制御するクラス
 **		・IOボード初期化処理、終了処理を制御する
 **		・特定ビットの立上りを監視する
 **		・ビット単位/バイト単位の入出力操作を行う
 **		・DIOからのエラー情報を処理する
 **
 **/

#include "stdafx.h"
#include "DioControl.h"

#include "SysOption.h"			//システムオプションパラメータ
#include "DioCtec1616.h"
#include "CommBit.h"			//シリアル通信データ(ビット制御)
#include "Errors.h"

CDioCtec1616		ctec1616;			//DigitalI/O制御

extern CSysOption	sys_option;			//システムオプションパラメータ
extern CCommBit		comm_bit;			//I/O入出力(PLC内部デバイス)
extern CErrors		errors;				//エラー情報

CDioControl::CDioControl()
{
}


CDioControl::~CDioControl()
{
}


/** ****************************************************** **
 ** I/Oイニシャル処理
 ** 引数：
 **　CString name_device_io	DIO-1616L-PE デバイスID
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::DioInitUtil(CString name_device_io)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード
	CString title, msg1, msg2;

	char name_device[8] = { '\0' };	//デバイス名
	strcpy_s(name_device, 8, name_device_io);

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		//Initial処理
		if (ctec1616.Initial(name_device, msg_err, &rtn_code) == 0) {
			//処理なし
		}
		else {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	return 0;
}


/** ****************************************************** **
 ** I/O終了処理
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::DioExitUtil(void)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		//Exit処理
		if (ctec1616.Exit(msg_err, &rtn_code) == 0) {
			//処理なし
		}
		else {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	return 0;
}


/** ****************************************************** **
 ** コマンド取得
 ** 引数：
 **　byte* data　コマンドNo(8bit)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::GetCommand(byte* data)
{
	//
	// ビット0～7(Y*0～*7)のON/OFF状態を取得する
	//
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード
	short port = 0;		//入力読取ポートNo

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		if (ctec1616.InputByte(msg_err, &rtn_code, port, data) != 0) {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	//I/Oボード無し
	else {
		int two = 1;
		int answer = 0;
		for (int i = 0; i < 8; i++) {
			if (i != 0) two = two * 2;
			if (comm_bit.indata_bit[i] == BIT_ON) {
				if (i == 0) {
					answer = 1;
				}
				else {
					answer = answer + two;
				}
			}
		}
		*data = (byte)answer;
	}

	return 0;
}


/** ****************************************************** **
 ** エラーコード出力
 ** 引数：
 **　byte data　エラーコード(8bit)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
//int CDioControl::OutErrorCode(byte data)
//{
//	char msg_err[256];	//エラーメッセージ
//	long rtn_code;		//処理コード
//
//	if (ctec1616.OutputByte(msg_err, &rtn_code, 0, data) != 0) {
//		errors.SetErrorCode((int)rtn_code);
//		errors.SetErrorMessage((CString)msg_err);
//		errors.OutputLog();
//		return 1;
//	}
//	return 0;
//}


/** ****************************************************** **
 ** DIOボード 出力ポートリセット
 ** 引数：
 **　short port	出力書込ポートNo
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::OutPortReset(short port)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード
	byte data = 0x00;	//出力byte

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		if (ctec1616.OutputByte(msg_err, &rtn_code, port, data) != 0) {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	//I/Oボード無し
	else {
		int start_io = port * 8;
		int end_io = port * 8 + 7;
		for (int i = start_io; i <= end_io; i++) {
			comm_bit.outdata_bit[i] = BIT_OFF;
		}
	}

	return 0;
}


/** ****************************************************** **
 ** DIOボード トリガ監視開始
 ** 引数：
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::StartTRG(HWND &m_hWnd)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード
	short bit;			//監視ビットNo

	if (sys_option.io_unit_less) return 0;	//I/Oユニット無し

	//
	// ビット8(Y*8)の立ち上がりトリガを監視開始する
	//
	bit = 8;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット8(Y*8)の立ち下がり割り込みイベントを設定する
	//
	if (ctec1616.NotifyInterrupt(msg_err, &rtn_code, bit, 2, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット9(Y*9)の立ち上がりトリガを監視開始する
	//
	bit = 9;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット10(Y*A)の立ち上がりトリガを監視開始する
	//
	bit = 10;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット11(Y*B)の立ち上がりトリガを監視開始する
	//
	bit = 11;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット12(Y*C)の立ち上がりトリガを監視開始する
	//
	bit = 12;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット13(Y*D)の立ち上がりトリガを監視開始する
	//
	bit = 13;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット14(Y*E)の立ち上がりトリガを監視開始する
	//
	bit = 14;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerRise(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット15(Y*F)の立ち上がり/立ち下がりトリガを監視開始する
	//
	bit = 15;		//監視ビットNo
	if (ctec1616.StartNotifyTriggerAll(msg_err, &rtn_code, bit, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** DIOボード トリガ監視停止
 ** 引数：
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::StopTRG(HWND &m_hWnd)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード
	short bit;			//監視ビットNo

	if (sys_option.io_unit_less) return 0;	//I/Oユニット無し

	//
	// ビット8(Y*8)のトリガを監視停止する
	//
	bit = 8;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット8(Y*8)の立ち下がり割り込みイベントを停止する
	//
	if (ctec1616.NotifyInterrupt(msg_err, &rtn_code, bit, 0, m_hWnd) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット9(Y*9)のトリガを監視停止する
	//
	bit = 9;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット10(Y*A)のトリガを監視停止する
	//
	bit = 10;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット11(Y*B)のトリガを監視停止する
	//
	bit = 11;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット12(Y*C)のトリガを監視停止する
	//
	bit = 12;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット13(Y*D)のトリガを監視停止する
	//
	bit = 13;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット14(Y*E)のトリガを監視停止する
	//
	bit = 14;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//
	// ビット15(Y*F)のトリガを監視停止する
	//
	bit = 15;		//監視ビットNo
	if (ctec1616.StopNotifyTrigger(msg_err, &rtn_code, bit) != 0) {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** DIOボード ビット出力
 ** 引数：
 **　short bit	出力書込ビットNo
 **　byte data	出力　0x00:OFF, 0x01:ON
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::OutBit(short bit, byte data)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		if (ctec1616.OutputBit(msg_err, &rtn_code, bit, data) != 0) {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	//I/Oボード無し
	else {
		comm_bit.outdata_bit[bit] = data;
	}

	return 0;
}


/** ****************************************************** **
 ** DIOボード 出力ビット状態読取
 ** 引数：
 **　short bit	出力読取ビットNo
 **　byte* data	出力状態　0x00:OFF, 0x01:ON
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::EchoBit(short bit, byte* data)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		if (ctec1616.EchoBackBit(msg_err, &rtn_code, bit, data) != 0) {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	//I/Oボード無し
	else {
		*data = comm_bit.outdata_bit[bit];
	}

	return 0;
}


/** ****************************************************** **
 ** DIOボード ビット入力
 ** 引数：
 **　short bit	入力読取ビットNo
 **　byte* data	結果byte　0x00:OFF, 0x01:ON
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioControl::InBit(short bit, byte* data)
{
	char msg_err[256];	//エラーメッセージ
	long rtn_code;		//処理コード

	//I/Oボード有り
	if (!sys_option.io_unit_less) {
		if (ctec1616.InputBit(msg_err, &rtn_code, bit, data) != 0) {
			errors.SetErrorCode((int)rtn_code);
			errors.SetErrorMessage((CString)msg_err);
			errors.OutputLog();
			return 1;
		}
	}

	//I/Oボード無し
	else {
		*data = comm_bit.indata_bit[bit];
	}

	return 0;
}