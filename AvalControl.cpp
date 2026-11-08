/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：AvalControl.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2022.05.19	M.Iwasaki	グラバーボード追加対応/VIEWORKS(VC-25MC-M30D0)カメラ対応
 ** 注記：
 ** 説明：アバールデータ製 画像入力ボードを制御するメインクラス
 **		・ボード初期化処理、終了処理を制御する
 **		・1画面入力を制御する
 **		・連続入力を制御する
 **		・ボードからのエラー情報を処理する
 **
 **/

#include "stdafx.h"
#include "AvalControl.h"

#include "CameraSettings.h"	//カメラパラメータ

#include "Errors.h"
//#include "DlgSystemMain.h" //honda 追加


extern CCameraSettings	cam_parameter[8];	//カメラパラメータ
extern CErrors			errors;				//エラー情報

extern char* APP_ROOT;

CImplAcapLib2		impl_acaplib2;		//AcapLib2 API実行
extern FVCL::Data::CFvImage grab_image[CAM_NUM_MAX][MEM_NUM];	//画像メモリ
//CDlgSystemMain  sys; //honda追加


CAvalControl::CAvalControl()
{
}


CAvalControl::~CAvalControl()
{
}


/** ****************************************************** **
 ** ボードイニシャル処理
 ** 引数：
 **　int board_number	ボード番号(0/1)
 **　WORD model_code	画像入力ボード モデルコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAvalControl::InitUtil(int board_number, WORD model_code)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード
	CString title, msg1, msg2;

	//ボード名の確認
	char name_board[10] = { '\0' };	//ボード名
	if (GetAvalBoardName(model_code, name_board) != 0) return 1;

	//ボード情報取得とオープン
	if (impl_acaplib2.InitialBoard(board_number, name_board, msg_err, &rtn_code) == 0) {
		//処理なし
	}
	else {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//ボードチャンネル数
	int ch_num = 0;
	if (model_code == 0x1101) ch_num = 4;	//APX-3324A
	if (model_code == 0x1102) ch_num = 1;	//APX-3323

	//カメラIniファイルの読み込み
	char path[MAX_PATH] = { '\0' };
	for (int i = 0; i < ch_num; i++) {
		int cam_num = board_number * CAM_NUM_BOARD + i;
		if (cam_parameter[cam_num].device.model_code != 0) {
			int ch = i + 1;
			int grab_width;
			int grab_height;
			strcpy_s(path, MAX_PATH, (CString)APP_ROOT + cam_parameter[cam_num].device.inifile_path);
			if (impl_acaplib2.LoadIniFile(board_number, ch, path, &grab_width, &grab_height, msg_err, &rtn_code) == 0) {
				//カメラサイズの展開
				cam_parameter[cam_num].device.grab_width = grab_width;
				cam_parameter[cam_num].device.grab_height = grab_height;
			}
			else {
				errors.SetErrorCode((int)rtn_code);
				errors.SetErrorMessage((CString)msg_err);
				errors.OutputLog();
				return 1;
			}
		}
	}

	//バッファの確保と登録
	for (int i = 0; i < ch_num; i++) {
		int cam_num = board_number * CAM_NUM_BOARD + i;
		if (cam_parameter[cam_num].device.model_code != 0) {
			int ch = i + 1;
			int grab_width = cam_parameter[cam_num].device.grab_width;
			int grab_height = cam_parameter[cam_num].device.grab_height;
			if (impl_acaplib2.SetBuffer(board_number, ch, grab_width, grab_height, msg_err, &rtn_code) == 0) {
				//処理なし
			}
			else {
				errors.SetErrorCode((int)rtn_code);
				errors.SetErrorMessage((CString)msg_err);
				errors.OutputLog();
				return 1;
			}
		}
	}

	return 0;
}


/** ****************************************************** **
 ** ボード終了処理
 ** 引数：
 **　int board_number　ボード番号(0/1)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAvalControl::ExitUtil(int board_number)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード

	//ボードクローズとバッファ開放
	if (impl_acaplib2.ExitBoard(board_number, msg_err, &rtn_code) == 0) {
		//処理なし
	}
	else {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	return 0;
}


/** ****************************************************** **
 ** 連続入力開始
 ** 引数：
 **　int board_number	ボード番号(0/1)
 **　WORD model_code	画像入力ボード モデルコード
 **　LPVOID pParam		パラメータ(CDlgSystemMain*)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAvalControl::ContinuousGrab(int board_number, WORD model_code, LPVOID pParam)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード

	//ボードチャンネル数
	int ch_num = 0;
	if (model_code == 0x1101) ch_num = 4;	//APX-3324A
	if (model_code == 0x1102) ch_num = 1;	//APX-3323

	//画像入力の開始
	for (int i = 0; i < ch_num; i++) {
		int cam_num = board_number * CAM_NUM_BOARD + i;
		if (cam_parameter[cam_num].device.model_code != 0) {
			int ch = i + 1;
			int input_num = 0;	//画像入力方法 0:連続入力
			if (impl_acaplib2.GrabStart(board_number, ch, input_num, pParam, msg_err, &rtn_code) == 0) {
				//処理なし
			}
			else {
				errors.SetErrorCode((int)rtn_code);
				errors.SetErrorMessage((CString)msg_err);
				errors.OutputLog();
				return 1;
			}
		}
	}

	return 0;
}


/** ****************************************************** **
 ** 入力停止
 ** 引数：
 **　int board_number	ボード番号(0/1)
 **　WORD model_code	画像入力ボード モデルコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAvalControl::StopGrab(int board_number, WORD model_code)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード

	//ボードチャンネル数
	int ch_num = 0;
	if (model_code == 0x1101) ch_num = 4;	//APX-3324A
	if (model_code == 0x1102) ch_num = 1;	//APX-3323

	//画像入力の停止
	for (int i = 0; i < ch_num; i++) {
		int cam_num = board_number * CAM_NUM_BOARD + i;
		if (cam_parameter[cam_num].device.model_code != 0) {
			int ch = i + 1;
			if (impl_acaplib2.GrabStop(board_number, ch, msg_err, &rtn_code) == 0) {
				//処理なし
			}
			else {
				errors.SetErrorCode((int)rtn_code);
				errors.SetErrorMessage((CString)msg_err);
				errors.OutputLog();
				return 1;
			}
		}
	}

	return 0;
}


/** ****************************************************** **
 ** 最終カメラ画像の取得
 ** 引数：
 **　int board_number　		ボード番号(0/1)
 **　int nCh				設定チャンネル(1-4)
 **　FVCL::Data::CFvImage*	画像メモリ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		現在の最後に入力完了したメモリ番号を取得し、そのメモリ番号の画像を取得する
 **		コマンド実行で画像を取得する場合に使用する
 ** ****************************************************** **/
int CAvalControl::GetLastImage(int board_number, int nCh, FVCL::Data::CFvImage* m_Image)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード

	//最終入力メモリ番号の取得
	int frame;
	int mem_index;	//現在の最後に入力完了したメモリ番号
	if (impl_acaplib2.GetMemIndex(board_number, nCh, &frame, &mem_index, msg_err, &rtn_code) == 0) {
		//処理なし
	}
	else {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//最新画像のコピー Copy()ではダメ
	int mem_ch = board_number * CAM_NUM_BOARD + nCh - 1;	//メモリch
	*m_Image = grab_image[mem_ch][mem_index - 1];

//	INT status_org = 
		FVCL::File::SaveImageFile("C:\\E35088\\B\\Live画像.bmp", *m_Image);
	return 0;
}


/** ****************************************************** **
 ** 最終カメラ画像の取得
 ** 引数：
 **　int board_number　		ボード番号(0/1)
 **　int nCh				設定チャンネル(1-4)
 **　nFrameNo				入力が完了したフレーム数
 **　int nMemoryNo			入力が完了したメモリ番号(1～4)
 **　FVCL::Data::CFvImage*	画像メモリ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		AcapLib2コールバック関数によって1フレーム取込完了の通知にて回答されたメモリ番号から画像を取得する
 **		メインウィンドウのカメラ画像の常時表示で使用する
 ** ****************************************************** **/
int CAvalControl::GetLastImage(int board_number, int nCh, int nFrameNo, int nMemoryNo, FVCL::Data::CFvImage* m_Image)
{
	char msg_err[256] = {'\0'};	//エラーメッセージ
	DWORD rtn_code = 0;			//処理コード

	//最終入力メモリ番号の取得
	int frame;
	int mem_index;	//現在の最後に入力完了したメモリ番号
	if (impl_acaplib2.GetMemIndex(board_number, nCh, &frame, &mem_index, msg_err, &rtn_code) == 0) {
		//コールバックされたときの最終メモリ番号と現在の最後に入力完了したメモリ番号の確認
		//コールバック内の処理時間が1フレーム以上かかる状態が発生した場合、バッファのオーバーライトが
		//発生してしまう可能性があるため、ここで問題ないか確認する
		if (((frame + 1) - nFrameNo) >= MEM_NUM) {
			//バッファのオーバーライトが発生
			//errors.SetErrorMessage("GetLastImage() バッファのオーバーライト発生 チャンネル:" + Tips0.IntToString(nCh));
			//errors.OutputLog();
			//return 1;

			//子ウィンドウを起動すると最初の数msだけオーバーライトが発生する模様
			//対策として現在の最後に入力完了したメモリ番号(mem_index)から画像を取得する
			//画像メモリのバッファ(MEM_NUM)を増やす方法もあるがPC内メモリを消費するので行わない
			nMemoryNo = mem_index;
		}
	}
	else {
		errors.SetErrorCode((int)rtn_code);
		errors.SetErrorMessage((CString)msg_err);
		errors.OutputLog();
		return 1;
	}

	//最新画像のコピー Copy()ではダメ
	int mem_ch = board_number * CAM_NUM_BOARD + nCh - 1;	//メモリch
	*m_Image = grab_image[mem_ch][nMemoryNo - 1];

	return 0;
}


/** ****************************************************** **
 ** AVALDATA AcapOpen識別名の確認
 ** 引数：
 **　WORD model_code	モデルコード
 **　char* name_board	ボード名
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CAvalControl::GetAvalBoardName(WORD model_code, char* name_board)
{
	//AVALDATA APX-3324A
	if (model_code == 0x1101) {
		strcpy_s(name_board, 10, "APX-3324");
	}
	//AVALDATA APX-3323
	else if (model_code == 0x1102) {
		strcpy_s(name_board, 10, "APX-3323");
	}
	//モデルコード異常
	else {
		return 1;
	}

	return 0;
}
