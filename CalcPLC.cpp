/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcPLC.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 **			2021.08.31	M.Iwasaki	K33004 検査機能DLL化、BCR機能追加
 **			2022.09.12	M.Iwasaki	マルチ探索機能追加、基準-エッジ距離計測機能追加
 **			2024.03.21	M.Iwasaki	10.16.2.0	エッジtoエッジ計測機能追加
 **			2026.05.23	M.Iwasaki	10.20.2.1	経時変化補正機能追加
 ** 注記：
 ** 説明：PLCデータ入出力プログラム
 **
 **/

#include "stdafx.h"
#include "CalcPLC.h"

#include "Commands.h"		//コマンドパラメータ
#include "TempParam.h"		//機種毎テンプレートパラメータ
#include "ScopeParam.h"		//機種毎スコープパラメータ
#include "MesParam.h"		//機種毎検査パラメータ

//DLL
#include "AlignMultiSrh.h"		//AlignMultiSrh DLL Header
#include "AlignDisEdge.h"		//AlignDisEdge DLL Header
#include "AlignDisEdgeToEdge.h"	//AlignDisEdgeToEdge DLL Header

extern CCommands		cmd_param[256];			//コマンドパラメータ
extern CTempParam		template_param[256];	//機種毎テンプレートパラメータ
extern CScopeParam		scope_param[16];		//機種毎スコープパラメータ
extern CMesParam		mes_param[256];			//機種毎検査パラメータ

CCalcPLC::CCalcPLC()
{
}


CCalcPLC::~CCalcPLC()
{
}


/** ****************************************************** **
 ** PLC軸位置取得XY
 ** 引数：
 **　int template_number	テンプレート番号
 **　double* axis_x			軸位置X[mm]
 **　double* axis_y			軸位置Y[mm]
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetAxisPosXY(int template_number, double* axis_x, double* axis_y)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//機能種別の確認
	//	0x01:アライメント
	//	0x02:計測モード
	//	0x03:キャリブレーション
	if (template_param[template_number].plc.type == 0x01 ||
		template_param[template_number].plc.type == 0x02 ||
		template_param[template_number].plc.type == 0x03) {
		//軸位置X取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[0];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[1];
		int data1 = Tips0.conv1.val_i;
		*axis_x = (double)data1 / 10000;
		//軸位置Y取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[2];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[3];
		int data2 = Tips0.conv1.val_i;
		*axis_y = (double)data2 / 10000;

		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** PLC軸位置取得T
 ** 引数：
 **　int template_number	テンプレート番号
 **　double* axis_t			軸位置T[deg]
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetAxisPosT(int template_number, double* axis_t)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//機能種別の確認
	//	0x03:キャリブレーション
	if (template_param[template_number].plc.type == 0x03) {
		//軸位置T取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[4];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[5];
		int data1 = Tips0.conv1.val_i;
		*axis_t = (double)data1 / 10000;

		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** PLC外部オフセット取得XYT
 ** 引数：
 **　byte cmd			コマンド番号
 **　double* offset_x		外部オフセットX[mm]
 **　double* offset_y		外部オフセットY[mm]
 **　double* offset_t		外部オフセットT[deg]
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetExtOffsetXYT(byte cmd, double* offset_x, double* offset_y, double* offset_t)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//機能種別の確認
	//	0x41:外部オフセット設定
	if (cmd_param[cmd].plc_read.type == 0x41) {
		//オフセットX取得
		Tips0.conv1.val_w[0] = cmd_param[cmd].plc_read.read_data.w_data[0];
		Tips0.conv1.val_w[1] = cmd_param[cmd].plc_read.read_data.w_data[1];
		int data1 = Tips0.conv1.val_i;
		*offset_x = (double)data1 / 10000;
		//オフセットY取得
		Tips0.conv1.val_w[0] = cmd_param[cmd].plc_read.read_data.w_data[2];
		Tips0.conv1.val_w[1] = cmd_param[cmd].plc_read.read_data.w_data[3];
		int data2 = Tips0.conv1.val_i;
		*offset_y = (double)data2 / 10000;
		//オフセットT取得
		Tips0.conv1.val_w[0] = cmd_param[cmd].plc_read.read_data.w_data[4];
		Tips0.conv1.val_w[1] = cmd_param[cmd].plc_read.read_data.w_data[5];
		int data3 = Tips0.conv1.val_i;
		*offset_t = (double)data3 / 10000;

		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** PLC読み込みオフセット取得XYT
 ** 引数：
 **　int template_number	テンプレート番号
 **　double* offset_x		外部オフセットX[mm]
 **　double* offset_y		外部オフセットY[mm]
 **　double* offset_t		外部オフセットT[deg]
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetReadOffsetXYT(int template_number, double* offset_x, double* offset_y, double* offset_t)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	//機能種別の確認
	//	0x01:アライメント
	if (template_param[template_number].plc.type == 0x01) {
		//オフセットX取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[0];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[1];
		int data1 = Tips0.conv1.val_i;
		*offset_x = (double)data1 / 10000;
		//オフセットY取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[2];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[3];
		int data2 = Tips0.conv1.val_i;
		*offset_y = (double)data2 / 10000;
		//オフセットT取得
		Tips0.conv1.val_w[0] = template_param[template_number].plc.read_data.w_data[4];
		Tips0.conv1.val_w[1] = template_param[template_number].plc.read_data.w_data[5];
		int data3 = Tips0.conv1.val_i;
		*offset_t  = (double)data3 / 10000;

		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 1ワード取得
 ** 引数：
 **　CCommData* com_data	PLC通信データ
 **　int index				データ位置
 **　WORD* w_data			取得データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetWORDData(CCommData* com_data, int index, WORD* w_data)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	if (index < 0 || index > 19) goto END;

	*w_data = com_data->w_data[index];

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** 1ダブルワード取得
 ** 引数：
 **　CCommData* com_data	PLC通信データ
 **　int index				データ位置
 **　WORD* w_data			取得データ
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcPLC::GetDWORDData(CCommData* com_data, int index, DWORD* dw_data)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;
	CToolTips0 Tips0;

	if (index < 0 || index > 18) goto END;

	Tips0.conv1.val_w[0] = com_data->w_data[index];
	Tips0.conv1.val_w[1] = com_data->w_data[index + 1];
	*dw_data = Tips0.conv1.val_dw;

	rtn_result = 0;

END:
	return rtn_result;
}


/** ****************************************************** **
 ** アライメント演算結果のデータ出力
 ** 引数：
 **　byte cmd					実行コマンド
 **　int scope					対象スコープ
 **　byte base_method			機能種別
 **　byte calc_cmd				演算実施コマンド
 **　double theta_x				補正結果 θX
 **　double theta_y				補正結果 θY
 **　double delta_t_degree		補正結果 ⊿T(degree)
 **　double delta_x				補正結果 ⊿X
 **　double delta_y				補正結果 ⊿Y
 **　double* base_mark_width	対象スコープ マーク間距離(配列の先頭アドレス) [] 0:視野1-2間、1:視野2-3間、2:視野3-4間、3:視野4-1間
 **　double* target_mark_width	目標スコープ マーク間距離(配列の先頭アドレス) [] 0:視野1-2間、1:視野2-3間、2:視野3-4間、3:視野4-1間
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:補正結果 θX or ⊿X
 **		w_data[1]:補正結果 θX or ⊿X
 **		w_data[2]:補正結果 θY or ⊿Y
 **		w_data[3]:補正結果 θY or ⊿Y
 **		w_data[4]:補正結果 ⊿T
 **		w_data[5]:補正結果 ⊿T
 **		w_data[6]:対象スコープ マーク間距離(視野1-2)
 **		w_data[7]:対象スコープ マーク間距離(視野1-2)
 **		w_data[8]:対象スコープ マーク間距離(視野2-3) or 目標スコープ マーク間距離(視野1-2)
 **		w_data[9]:対象スコープ マーク間距離(視野2-3) or 目標スコープ マーク間距離(視野1-2)
 **		w_data[10]:対象スコープ マーク間距離(視野3-4) or (視野3-1)
 **		w_data[11]:対象スコープ マーク間距離(視野3-4) or (視野3-1)
 **		w_data[12]:対象スコープ マーク間距離(視野4-1)
 **		w_data[13]:対象スコープ マーク間距離(視野4-1)
 **		w_data[14]:
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::AlignCalcDataOut(byte cmd, int scope, byte base_method, byte calc_cmd,
								double theta_x, double theta_y,
								double delta_t_degree, double delta_x, double delta_y,
								double* base_mark_width, double* target_mark_width)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	int parts = cmd_param[calc_cmd].align.parts;	//アライメント視野数 演算を実施したコマンドパラメータから取得

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//
	//XYの出力
	//

	//データ書込設定:θX, θY, Δθ, W
	if (write_type == 0x01) {
		//補正結果 θX 0.1um単位で出力 1.0000mm = 10000
		int data1 = Tips0.RoundConv(theta_x, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data1;
		scope_param[scope].plc.write_data.w_data[0] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[1] = Tips0.conv1.val_w[1];

		//補正結果 θY 0.1um単位で出力 1.0000mm = 10000
		int data2 = Tips0.RoundConv(theta_y, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data2;
		scope_param[scope].plc.write_data.w_data[2] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[3] = Tips0.conv1.val_w[1];
	}
	//データ書込設定:ΔX,ΔY,Δθ,W
	else if (write_type == 0x02){
		//補正結果 ⊿X 0.1um単位で出力 1.0000mm = 10000
		int data1 = Tips0.RoundConv(delta_x, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data1;
		scope_param[scope].plc.write_data.w_data[0] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[1] = Tips0.conv1.val_w[1];

		//補正結果 ⊿Y 0.1um単位で出力 1.0000mm = 10000
		int data2 = Tips0.RoundConv(delta_y, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data2;
		scope_param[scope].plc.write_data.w_data[2] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[3] = Tips0.conv1.val_w[1];
	}

	//
	//θの出力
	//

	//データ書込設定:θX, θY, Δθ, W、ΔX,ΔY,Δθ,W
	if (write_type == 0x01 || write_type == 0x02) {
		//補正結果 ⊿T 0.1mdeg単位 1.0000deg = 10000
		int data3 = Tips0.RoundConv(delta_t_degree, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data3;
		scope_param[scope].plc.write_data.w_data[4] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[5] = Tips0.conv1.val_w[1];
	}

	//
	//対象スコープW(マーク間距離)の出力
	//

	//データ書込設定:θX, θY, Δθ, W、ΔX,ΔY,Δθ,W
	if (write_type == 0x01 || write_type == 0x02) {

		//視野数が2以上
		if (parts >= 2) {
			//対象スコープ 視野1-2 マーク間距離 0.1um単位で出力 1.0000mm = 10000
			int data4 = Tips0.RoundConv(base_mark_width[0], 4);		//送信データ桁繰り上げと四捨五入
			Tips0.conv1.val_i = data4;
			scope_param[scope].plc.write_data.w_data[6] = Tips0.conv1.val_w[0];
			scope_param[scope].plc.write_data.w_data[7] = Tips0.conv1.val_w[1];
		}

		//視野数が3以上
		if (parts >= 3) {
			//対象スコープ 視野2-3 マーク間距離 0.1um単位で出力 1.0000mm = 10000
			int data5 = Tips0.RoundConv(base_mark_width[1], 4);		//送信データ桁繰り上げと四捨五入
			Tips0.conv1.val_i = data5;
			scope_param[scope].plc.write_data.w_data[8] = Tips0.conv1.val_w[0];
			scope_param[scope].plc.write_data.w_data[9] = Tips0.conv1.val_w[1];

			if (parts == 3) {
				//ここに視野3-1間の距離を追記
			}
			else if (parts == 4) {
				//対象スコープ 視野3-4 マーク間距離 0.1um単位で出力 1.0000mm = 10000
				int data6 = Tips0.RoundConv(base_mark_width[2], 4);		//送信データ桁繰り上げと四捨五入
				Tips0.conv1.val_i = data6;
				scope_param[scope].plc.write_data.w_data[10] = Tips0.conv1.val_w[0];
				scope_param[scope].plc.write_data.w_data[11] = Tips0.conv1.val_w[1];

				//対象スコープ 視野4-1 マーク間距離 0.1um単位で出力 1.0000mm = 10000
				int data7 = Tips0.RoundConv(base_mark_width[3], 4);		//送信データ桁繰り上げと四捨五入
				Tips0.conv1.val_i = data7;
				scope_param[scope].plc.write_data.w_data[12] = Tips0.conv1.val_w[0];
				scope_param[scope].plc.write_data.w_data[13] = Tips0.conv1.val_w[1];
			}
		}
	}

	//
	//目標スコープW(マーク間距離)の出力
	//

	//データ書込設定:θX, θY, Δθ, W、ΔX,ΔY,Δθ,W
	if (write_type == 0x01 || write_type == 0x02) {

		//視野数が2に限り、目標スコープを出力
		if (parts == 2) {
			//目標スコープ マーク間距離 0.1um単位で出力 1.0000mm = 10000
			int data5 = Tips0.RoundConv(target_mark_width[0], 4);		//送信データ桁繰り上げと四捨五入
			Tips0.conv1.val_i = data5;
			scope_param[scope].plc.write_data.w_data[8] = Tips0.conv1.val_w[0];
			scope_param[scope].plc.write_data.w_data[9] = Tips0.conv1.val_w[1];
		}
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** 検査結果のデータ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　int scope				対象スコープ
 **　byte base_method		機能種別
 **　BOOL* mes_ok			シート0～7の検査結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:各シートの検査結果
 **		w_data[1]:
 **		w_data[2]:検査結果PLC出力 アドレス2
 **		w_data[3]:検査結果PLC出力 アドレス3
 **		w_data[4]:検査結果PLC出力 アドレス4
 **		w_data[5]:検査結果PLC出力 アドレス5
 **		w_data[6]:検査結果PLC出力 アドレス6
 **		w_data[7]:検査結果PLC出力 アドレス7
 **		w_data[8]:検査結果PLC出力 アドレス8
 **		w_data[9]:検査結果PLC出力 アドレス9
 **		w_data[10]:検査結果PLC出力 アドレス10
 **		w_data[11]:検査結果PLC出力 アドレス11
 **		w_data[12]:検査結果PLC出力 アドレス12
 **		w_data[13]:検査結果PLC出力 アドレス13
 **		w_data[14]:検査結果PLC出力 アドレス14
 **		w_data[15]:検査結果PLC出力 アドレス15
 **		w_data[16]:検査結果PLC出力 アドレス16
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::MesCalcDataOut(byte cmd, int scope, byte base_method, BOOL* mes_ok)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//シート0～7の検査結果
	//　w_data[0] = ********76543210の結果
	//　例) シート0:OK、シート1:NG、シート2:OK、シート3～7:無し
	//		w_data[0] = (2進)0000000011111101
	if (write_type == 0x20) {
		scope_param[scope].plc.write_data.w_data[0] = Tips0.BitToWord(mes_ok, 8);
	}

	//各検査PLC出力
	if (write_type == 0x20) {
		for (int i = 0; i < 8; i++) {
			//検査機能を確認
			int template_number = cmd_param[cmd].template_number;	//テンプレートNo
			int mes_type = mes_param[template_number].sheet[i].mes_type;

			//検査を実行
			switch (mes_type) {
				//無し
			case 0x00:
				break;
				//ブロブ
			case 0x01:
				break;
			//エッジ
			case 0x02:
				break;
			//濃度
			case 0x03:
				break;
			//マルチ探索
			case 0x04:
				DataOutMultiSrh(template_number, i, &scope_param[scope].plc.write_data);
				break;
			//基準-エッジ距離計測
			case 0x05:
				DataOutDisEdge(template_number, i, &scope_param[scope].plc.write_data);
				break;
			//エッジ-エッジ距離計測
			case 0x06:
				DataOutDisEdgeToEdge(template_number, i, &scope_param[scope].plc.write_data);
				break;
				//計算
			case 0x21:
				DataOutFormula(template_number, i, &scope_param[scope].plc.write_data);
				break;
				//その他(想定外)
			default:
				break;
			}
		}
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** マルチ探索データ出力
 ** 引数：
 **　int template_number	テンプレート番号
 **　int sheet_number		シート番号
 **　CCommData* data		出力データ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcPLC::DataOutMultiSrh(int template_number, int sheet_number, CCommData* data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	//データ準備
	AlignMultiSrh::Proc::CCalcPLCMultiSrh calc_plc;
	AlignMultiSrh::Param::CMultiSrhParam* multisrh_param = &mes_param[template_number].sheet[sheet_number].mes_multisrh;
	AlignMultiSrh::Param::CPLCParamMultiSrh* plc_param = &mes_param[template_number].plcout[sheet_number].plc_multisrh;
	AlignMultiSrh::Param::CMultiSrhResult* result = &template_param[template_number].multisrh_result[sheet_number];
	AlignMultiSrh::Data::CCommDataMultiSrh comm_data_multisrh;
	for (int i = 0; i < 20; i++) {
		comm_data_multisrh.w_data[i] = data->w_data[i];
	}

	//データ取得
	calc_plc.GetPLCData(multisrh_param, plc_param, result, &comm_data_multisrh);

	//データを回収
	for (int i = 0; i < 20; i++) {
		data->w_data[i] = comm_data_multisrh.w_data[i];
	}
}


/** ****************************************************** **
 ** 基準-エッジ距離計測データ出力
 ** 引数：
 **　int template_number	テンプレート番号
 **　int sheet_number		シート番号
 **　CCommData* data		出力データ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcPLC::DataOutDisEdge(int template_number, int sheet_number, CCommData* data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	//データ準備
	AlignDisEdge::Proc::CCalcPLCDisEdge calc_plc;
	AlignDisEdge::Param::CDisEdgeParam* disedge_param = &mes_param[template_number].sheet[sheet_number].mes_disedge;
	AlignDisEdge::Param::CPLCParamDisEdge* plc_param = &mes_param[template_number].plcout[sheet_number].plc_disedge;
	AlignDisEdge::Param::CDisEdgeResult* result = &template_param[template_number].disedge_result[sheet_number];
	AlignDisEdge::Data::CCommDataDisEdge comm_data_disedge;
	for (int i = 0; i < 20; i++) {
		comm_data_disedge.w_data[i] = data->w_data[i];
	}

	//データ取得
	calc_plc.GetPLCData(disedge_param, plc_param, result, &comm_data_disedge);

	//データを回収
	for (int i = 0; i < 20; i++) {
		data->w_data[i] = comm_data_disedge.w_data[i];
	}
}


/** ****************************************************** **
 ** エッジ-エッジ距離計測データ出力
 ** 引数：
 **　int template_number	テンプレート番号
 **　int sheet_number		シート番号
 **　CCommData* data		出力データ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcPLC::DataOutDisEdgeToEdge(int template_number, int sheet_number, CCommData* data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	//データ準備
	AlignDisEdgeToEdge::Proc::CCalcPLCDisEdgeToEdge calc_plc;
	AlignDisEdgeToEdge::Param::CDisEdgeToEdgeParam* disetoe_param = &mes_param[template_number].sheet[sheet_number].mes_disetoe;
	AlignDisEdgeToEdge::Param::CPLCParamDisEdgeToEdge* plc_param = &mes_param[template_number].plcout[sheet_number].plc_disetoe;
	AlignDisEdgeToEdge::Param::CDisEdgeToEdgeResult* result = &template_param[template_number].disetoe_result[sheet_number];
	AlignDisEdgeToEdge::Data::CCommDataDisEdgeToEdge comm_data_disetoe;
	for (int i = 0; i < 20; i++) {
		comm_data_disetoe.w_data[i] = data->w_data[i];
	}

	//データ取得
	calc_plc.GetPLCData(disetoe_param, plc_param, result, &comm_data_disetoe);

	//データを回収
	for (int i = 0; i < 20; i++) {
		data->w_data[i] = comm_data_disetoe.w_data[i];
	}
}


/** ****************************************************** **
 ** 計算データ出力
 ** 引数：
 **　int template_number	テンプレート番号
 **　int sheet_number		シート番号
 **　CCommData* data		出力データ
 ** 戻り値：
 **　なし
 ** 特記事項：
 ** ****************************************************** **/
void CCalcPLC::DataOutFormula(int template_number, int sheet_number, CCommData* data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	//データ準備
	CFormulaParam* formula_param = &mes_param[template_number].sheet[sheet_number].mes_formula;
	CPLCParamFormula* plc_param = &mes_param[template_number].plcout[sheet_number].plc_formula;
	CFormulaResult* result = &template_param[template_number].formula_result[sheet_number];

	//計算結果
	for (int i = 0; i < 3; i++) {
		int adrs_distance = plc_param->out_mes_address.formula_result[i];
		if (adrs_distance >= 2 && adrs_distance <= 15) {
			//計測結果
			double rslt_val = result->formula[i].d;
			int rc_data = Tips0.RoundConv(rslt_val, 4);	//送信データ桁繰り上げと四捨五入
			Tips0.conv1.val_i = rc_data;

			data->w_data[adrs_distance] = Tips0.conv1.val_w[0];
			data->w_data[adrs_distance + 1] = Tips0.conv1.val_w[1];
		}
	}
}


/** ****************************************************** **
 ** 画素キャリブレーション結果のデータ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　int scope				対象スコープ
 **　byte base_method		機能種別
 **　double pixel_size		画素サイズ
 **　double crdnt_radian	座標系傾き
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:画素サイズ
 **		w_data[1]:画素サイズ
 **		w_data[2]:カメラ傾き
 **		w_data[3]:カメラ傾き
 **		w_data[4]:
 **		w_data[5]:
 **		w_data[6]:
 **		w_data[7]:
 **		w_data[8]:
 **		w_data[9]:
 **		w_data[10]:
 **		w_data[11]:
 **		w_data[12]:
 **		w_data[13]:
 **		w_data[14]:
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::PixelCalcDataOut(byte cmd, int scope, byte base_method, double pixel_size, double crdnt_radian)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//画素サイズ 0.1nm単位で出力 1.0000um = 10000
	if (write_type == 0x30) {
		//0.1nm単位で出力 12.3456um(123456[0.1nm])
		int data1 = Tips0.RoundConv(pixel_size, 7);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data1;
		scope_param[scope].plc.write_data.w_data[0] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[1] = Tips0.conv1.val_w[1];
	}

	//カメラ傾き 0.1mdeg単位 1.0000deg = 10000
	if (write_type == 0x30) {
		double crdnt_degree = Tips0.RadianToDeg(crdnt_radian);	//degreeに変換
		int data2 = Tips0.RoundConv(crdnt_degree, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data2;
		scope_param[scope].plc.write_data.w_data[2] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[3] = Tips0.conv1.val_w[1];
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** 回転中心キャリブレーション結果のデータ出力
 ** 引数：
 **　byte cmd					実行コマンド
 **　int scope					対象スコープ
 **　byte base_method			機能種別
 **　double rotation_center_x	回転中心X
 **　double rotation_center_y	回転中心Y
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:回転中心X
 **		w_data[1]:回転中心X
 **		w_data[2]:回転中心Y
 **		w_data[3]:回転中心Y
 **		w_data[4]:
 **		w_data[5]:
 **		w_data[6]:
 **		w_data[7]:
 **		w_data[8]:
 **		w_data[9]:
 **		w_data[10]:
 **		w_data[11]:
 **		w_data[12]:
 **		w_data[13]:
 **		w_data[14]:
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::ThetaCalcDataOut(byte cmd, int scope, byte base_method, double rotation_center_x, double rotation_center_y)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//回転中心X 0.1um単位で出力 1.0000mm = 10000
	if (write_type == 0x31) {
		int data1 = Tips0.RoundConv(rotation_center_x, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data1;
		scope_param[scope].plc.write_data.w_data[0] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[1] = Tips0.conv1.val_w[1];
	}

	//回転中心Y 0.1um単位で出力 1.0000mm = 10000
	if (write_type == 0x31) {
		int data2 = Tips0.RoundConv(rotation_center_y, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data2;
		scope_param[scope].plc.write_data.w_data[2] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[3] = Tips0.conv1.val_w[1];
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** 経時変化結果のデータ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　int scope				対象スコープ
 **　byte base_method		機能種別
 **　double displacement_x	変化量X
 **　double displacement_y	変化量Y
 ** 戻り値：
 **　なし
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:変化量X
 **		w_data[1]:変化量X
 **		w_data[2]:変化量Y
 **		w_data[3]:変化量Y
 **		w_data[4]:
 **		w_data[5]:
 **		w_data[6]:
 **		w_data[7]:
 **		w_data[8]:
 **		w_data[9]:
 **		w_data[10]:
 **		w_data[11]:
 **		w_data[12]:
 **		w_data[13]:
 **		w_data[14]:
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::VariationDataOut(byte cmd, int scope, byte base_method, double displacement_x, double displacement_y)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//変化量X 0.1mpix単位で出力 1.0000pix = 10000
	if (write_type == 0x50) {
		int data1 = Tips0.RoundConv(displacement_x, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data1;
		scope_param[scope].plc.write_data.w_data[0] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[1] = Tips0.conv1.val_w[1];
	}

	//変化量Y 0.1mpix単位で出力 1.0000pix = 10000
	if (write_type == 0x50) {
		int data2 = Tips0.RoundConv(displacement_y, 4);		//送信データ桁繰り上げと四捨五入
		Tips0.conv1.val_i = data2;
		scope_param[scope].plc.write_data.w_data[2] = Tips0.conv1.val_w[0];
		scope_param[scope].plc.write_data.w_data[3] = Tips0.conv1.val_w[1];
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** バーコードリーダー結果のデータ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　int scope				対象スコープ
 **　byte base_method		機能種別
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:読取文字(2文字)
 **		w_data[1]:読取文字(2文字)
 **		w_data[2]:読取文字(2文字)
 **		w_data[3]:読取文字(2文字)
 **		w_data[4]:読取文字(2文字)
 **		w_data[5]:読取文字(2文字)
 **		w_data[6]:読取文字(2文字)
 **		w_data[7]:読取文字(2文字)
 **		w_data[8]:読取文字(2文字)
 **		w_data[9]:読取文字(2文字)
 **		w_data[10]:読取文字(2文字)
 **		w_data[11]:読取文字(2文字)
 **		w_data[12]:読取文字(2文字)
 **		w_data[13]:読取文字(2文字)
 **		w_data[14]:読取文字(2文字)
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:(エラーコード)
 ** ****************************************************** **/
void CCalcPLC::BcrCalcDataOut(byte cmd, int scope, byte base_method, WORD* w_data)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	byte write_type = cmd_param[cmd].write.type;	//データ書込設定

	//データ初期化
	scope_param[scope].plc.write_data.InitialData();

	//読取文字列結果
	if (write_type == 0x40) {
		for (int i = 0; i < 15; i++) {
			scope_param[scope].plc.write_data.w_data[i] = w_data[i];
		}
	}

	scope_param[scope].plc.type = base_method;	//データタイプ
}


/** ****************************************************** **
 ** ユーティリティ系のデータ出力
 ** 引数：
 **　byte cmd				実行コマンド
 **　CCommData* com_data	PLC通信データ
 **　byte base_method		機能種別
 **　WORD calc_err			演算エラーコード
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 **		PLC出力データ
 **		w_data[0]:
 **		w_data[1]:
 **		w_data[2]:
 **		w_data[3]:
 **		w_data[4]:
 **		w_data[5]:
 **		w_data[6]:
 **		w_data[7]:
 **		w_data[8]:
 **		w_data[9]:
 **		w_data[10]:
 **		w_data[11]:
 **		w_data[12]:
 **		w_data[13]:
 **		w_data[14]:
 **		w_data[15]:
 **		w_data[16]:
 **		w_data[17]:(最終実行コマンド番号)
 **		w_data[18]:(カメラ番号/機種番号)
 **		w_data[19]:演算エラーコード
 ** ****************************************************** **/
void CCalcPLC::UtilCalcDataOut(byte cmd, CCommData* com_data, byte base_method, WORD calc_err)
{
	// TODO: ここに実装コードを追加します.
	CToolTips0 Tips0;

	//データ初期化
	com_data->InitialData();

	//演算エラーコード
	com_data->w_data[19] = calc_err;

	cmd_param[cmd].plc_write.type = base_method;	//データタイプ
}
