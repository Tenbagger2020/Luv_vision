/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：DioCtec1616.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：コンテック(DIO-1616L-PE)DIOボードを実行するクラス
 **		・コンテックの関数を実行し結果とエラー情報を返す
 **
 **/

#include "stdafx.h"
#include "DioCtec1616.h"
#include "cdio.h"	//CONTEC IO HEDDER FILE

CDioCtec1616::CDioCtec1616()
{
}


CDioCtec1616::~CDioCtec1616()
{
}


/** ****************************************************** **
 ** デバイスIDの取得とアクセス可能にする
 ** 引数：
 **　char* name_device	デバイス名
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::Initial(char* name_device, char* msg_err, long* rtn_code)
{
	*rtn_code = DioInit(name_device, &id_device);

	//エラー処理
	if (*rtn_code != 0) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 終了処理としてアクセス不能にする
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::Exit(char* msg_err, long* rtn_code)
{
	*rtn_code = DioExit(id_device);

	//エラー処理
	if (*rtn_code != 0) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 入力ポートから１ビット入力する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			ビットNo(0～15)
 **　byte* data			入力データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::InputBit(char* msg_err, long* rtn_code, short bit, byte* data)
{
	*rtn_code = DioInpBit(id_device, bit, data);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 入力ポートから１バイト入力する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short port			ポートNo(0:0～7, 1:8～F)
 **　byte* data			入力データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::InputByte(char* msg_err, long* rtn_code, short port, byte* data)
{
	*rtn_code = DioInpByte(id_device, port, data);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 出力ポートに１ビット出力する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			ビットNo(0～15)
 **　byte data			出力データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::OutputBit(char* msg_err, long* rtn_code, short bit, byte data)
{
	*rtn_code = DioOutBit(id_device, bit, data);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 出力ポートに１バイト出力する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short port			ポートNo(0:0～7, 1:8～F)
 **　byte data			出力データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::OutputByte(char* msg_err, long* rtn_code, short port, byte data)
{
	*rtn_code = DioOutByte(id_device, port, data);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 指定したビットの立ち上がりトリガ監視を開始する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			監視ビット
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::StartNotifyTriggerRise(char* msg_err, long* rtn_code, short bit, HWND &m_hWnd)
{
	*rtn_code = DioNotifyTrg(id_device, bit, DIO_TRG_RISE, 10, m_hWnd);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 指定したビットの立ち下がりトリガ監視を開始する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			監視ビット
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::StartNotifyTriggerFall(char* msg_err, long* rtn_code, short bit, HWND &m_hWnd)
{
	*rtn_code = DioNotifyTrg(id_device, bit, DIO_TRG_FALL, 10, m_hWnd);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 指定したビットの立ち上がり/立ち下がりトリガ監視を開始する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			監視ビット
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::StartNotifyTriggerAll(char* msg_err, long* rtn_code, short bit, HWND &m_hWnd)
{
	*rtn_code = DioNotifyTrg(id_device, bit, DIO_TRG_RISE | DIO_TRG_FALL, 10, m_hWnd);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 指定したビットのトリガ監視を停止する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			監視ビット
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::StopNotifyTrigger(char* msg_err, long* rtn_code, short bit)
{
	*rtn_code = DioStopNotifyTrg(id_device, bit);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 出力ビットの状態を１ビット入力する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			ビットNo(0～15)
 **　byte* data			入力データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CDioCtec1616::EchoBackBit(char* msg_err, long* rtn_code, short bit, byte* data)
{
	*rtn_code = DioEchoBackBit(id_device, bit, data);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 指定したビットの割り込み処理を設定する
 ** 引数：
 **　char* msg_err		エラーメッセージ
 **　long* rtn_code		エラーコード
 **　short bit			監視ビット
 **　short logic		割り込み論理(0～2)
 **　HWND &m_hWnd		ウィンドウハンドル
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		割り込み論理
 **		0:割り込みをマスクする
 **		1:入力値0→1
 **		2:入力値1→0
 ** ****************************************************** **/
int CDioCtec1616::NotifyInterrupt(char* msg_err, long* rtn_code, short bit, short logic, HWND &m_hWnd)
{
	short dio_logic;
	if (logic == 1) {
		dio_logic = DIO_INT_RISE;
	}
	else if (logic == 2) {
		dio_logic = DIO_INT_FALL;
	}
	else {
		dio_logic = DIO_INT_NONE;
	}

	*rtn_code = DioNotifyInterrupt(id_device, bit, dio_logic, m_hWnd);

	//エラー処理
	if (*rtn_code != DIO_ERR_SUCCESS) {
		DioGetErrorString(*rtn_code, msg_err);
		return 1;
	}

	return 0;
}
