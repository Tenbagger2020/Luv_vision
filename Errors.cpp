/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：Errors.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2023.10.30	M.Iwasaki	10.14.2.0	エラーコード表記修正
 ** 注記：
 ** 説明：エラー情報を格納するクラス
 **		・各々のクラスで発生したエラーの内容をコードと文字列で保持する
 **		・エラー情報をログに記述する
 **
 **/

#include "stdafx.h"
#include "Errors.h"

#include "OutputLog.h"

extern COutputLog		outlog;		//ログ出力

CErrors::CErrors()
{
	//初期化
	error_code = 0;
	error_message = new char[1024];
	for (int i = 0; i < 1024; i++) {
		error_message[i] = '\0';
	}
	proc_err_code = 0x0000;
}


CErrors::~CErrors()
{
	delete[] error_message;
}


/** ****************************************************** **
 ** エラーコードの書込み
 ** 引数：
 **　int code	エラーコード
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CErrors::SetErrorCode(int code)
{
	error_code = code;
}


/** ****************************************************** **
 ** エラーメッセージの書込み
 ** 引数：
 **　CString message	エラーメッセージ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CErrors::SetErrorMessage(CString message)
{
	strcpy_s(error_message, 1024, message);

	//文字列端の不要な改行の削除
	int len_msg = (int)strlen(error_message);
	if (len_msg >= 2) {
		for (int i = len_msg - 2; i < len_msg; i++) {
			if (error_message[i] == 0x0A || error_message[i] == 0x0D) error_message[i] = '\0';
		}
	}
}


/** ****************************************************** **
 ** エラー情報の読込み
 ** 引数：
 **　WORD* code		エラーコード
 **　char* message	エラーメッセージ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CErrors::GetLastError(int* code, char* message)
{
	*code = error_code;
	strcpy_s(message, strlen(error_message) + 1, error_message);
}


/** ****************************************************** **
 ** エラー情報の読込み(文字列返し)
 ** 引数：
 **　なし
 ** 戻り値：
 **　CString エラー情報文字列
 ** 特記事項：
 **		エラー情報文字列:"メッセージ文字列(コード)"
 ** ****************************************************** **/
CString CErrors::GetLastErrorStr(void)
{
	CString str_error = "";

	if (strlen(error_message) > 0) str_error.Append((CString)error_message);			//メッセージがあればメッセージ書込み
	if (error_code != 0) str_error.Append("(ErrorCode=" + Tips0.IntToString(error_code) + ")");	//コードがあれば括弧でコード追記
	if (strlen(str_error) == 0) str_error = "エラーなし";								//エラー情報なければ"エラーなし"書込み

	return str_error;
}


/** ****************************************************** **
 ** 処理エラーコードの書込み
 ** 引数：
 **　WORD code	処理エラーコード
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CErrors::SetProcErrorCode(WORD code)
{
	proc_err_code = code;
}


/** ****************************************************** **
 ** 処理エラーコードの読込み
 ** 引数：
 **　なし
 ** 戻り値：
 **　WORD　処理エラーコード
 ** 特記事項：
 ** ****************************************************** **/
WORD CErrors::GetProcErrorCode(void)
{
	return proc_err_code;
}


/** ****************************************************** **
 ** ログ出力
 ** 引数：
 **　なし
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CErrors::OutputLog(void)
{
	int out_result = 1;
	CString msg_log = "";

	msg_log = GetLastErrorStr();
	out_result = outlog.LogOutFile(1, msg_log);

	return out_result;
}