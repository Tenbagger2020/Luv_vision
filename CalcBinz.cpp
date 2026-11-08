/**	Copyright (C) 2020 Osaki Engineering Co.,Ltd. All Rights Reserved
 ** システム：Luv_Vision_AlignSystem(Windows10汎用アライメントシステム)
 ** ファイル：CalcBinz.cpp
 ** 作成者：M.Iwasaki
 ** 履歴：	2020.08.20	M.Iwasaki	初版リリース
 ** 注記：
 ** 説明：2値化演算プログラム
 **
 **/

#include "stdafx.h"
#include "CalcBinz.h"


CCalcBinz::CCalcBinz()
{
}


CCalcBinz::~CCalcBinz()
{
}


/** ****************************************************** **
 ** 2値化演算(固定閾値:単一閾値指定)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int threshold1		閾値1
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzFix(int xs, int ys, int x_size, int y_size,
							int threshold1,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarize parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarize::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetThreshold((DOUBLE)threshold1);	//閾値1

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(固定閾値:バンド閾値指定)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int threshold1		閾値1
 **　int threshold2		閾値2
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzFixBand(int xs, int ys, int x_size, int y_size,
							int threshold1, int threshold2,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeBandThreshold parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeBandThreshold::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	if (threshold1 <= threshold2) {
		parser.SetThresholdL(threshold1);
		parser.SetThresholdH(threshold2);
	}
	else {
		parser.SetThresholdL(threshold2);
		parser.SetThresholdH(threshold1);
	}

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:閾値判別分析法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna1(int xs, int ys, int x_size, int y_size,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeDiscrimination parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeDiscrimination::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定 無し

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:微分ヒストグラム法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int dyna2_edge_facter		係数
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna2(int xs, int ys, int x_size, int y_size,
							int dyna2_edge_facter,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeGradient parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeGradient::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetEdgeFactor(dyna2_edge_facter);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:ラプラシアンヒストグラム法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int dyna3_edge_facter		係数
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna3(int xs, int ys, int x_size, int y_size,
							int dyna3_edge_facter,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeLaplacian parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeLaplacian::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetEdgeFactor(dyna3_edge_facter);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:Niblack法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int horz_size	局所領域サイズ横
 **　int vert_size	局所領域サイズ縦
 **　double coefficientK	係数
 **　int border_mode		ボーダーモード
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna4(int xs, int ys, int x_size, int y_size,
							int horz_size, int vert_size, double coefficientK, int border_mode,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeNiblack parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeNiblack::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetAreaHorzSize(horz_size);
	parser.SetAreaVertSize(vert_size);
	parser.SetCoefficientK(coefficientK);
	parser.SetBorderMode(border_mode);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:Pタイル法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int rate		割合
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna5(int xs, int ys, int x_size, int y_size,
							int rate,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizePTile parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizePTile::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetRate(rate);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:Sauvola法)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int horz_size	局所領域サイズ横
 **　int vert_size	局所領域サイズ縦
 **　double coefficientK	係数K
 **　double coefficientR	係数R
 **　int border_mode		ボーダーモード
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna6(int xs, int ys, int x_size, int y_size,
							int horz_size, int vert_size, double coefficientK, double coefficientR, int border_mode,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeSauvola parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeSauvola::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetAreaHorzSize(horz_size);
	parser.SetAreaVertSize(vert_size);
	parser.SetCoefficientK(coefficientK);
	parser.SetCoefficientR(coefficientR);
	parser.SetBorderMode(border_mode);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}


/** ****************************************************** **
 ** 2値化演算(動的閾値:局所領域平均値)
 ** 引数：
 **　int xs			2値化範囲 始点X
 **　int ys			2値化範囲 始点Y
 **　int x_size		2値化範囲 幅
 **　int y_size		2値化範囲 高さ
 **　int horz_size	局所領域サイズ横
 **　int vert_size	局所領域サイズ縦
 **　int offset		平均値オフセット
 **　int border_mode		ボーダーモード
 **　int val		一定値
 **　FVCL::Data::CFvImage* m_Image		画像メモリ(元画像)
 **　FVCL::Data::CFvImage* binz_Image	画像メモリ(2値化)
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int CCalcBinz::CalcBinzDyna7(int xs, int ys, int x_size, int y_size,
							int horz_size, int vert_size, int offset, int border_mode, int val,
							FVCL::Data::CFvImage* m_Image, FVCL::Data::CFvImage* binz_Image)
{
	// TODO: ここに実装コードを追加します.
	int rtn_result = 1;

	FVCL::Conversion::CFvBinarizeShiftAverage parser;

	//2値化範囲(処理ウィンドウ)の設定
	m_Image->SetWindow(xs, ys, x_size, y_size);

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizeShiftAverage::CheckValidity(*m_Image)) return 1;

	//画像処理準備
	parser.SetSrcImage(0, m_Image);
	parser.SetDstImage(0, binz_Image);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetAreaHorzSize(horz_size);
	parser.SetAreaVertSize(vert_size);
	parser.SetOffset(offset);
	parser.SetBorderMode(border_mode);
	if (border_mode == FVCL::BorderMode::Value) parser.SetBorderValue(val);

	//画像処理実行
	if (parser.Execute()) {
		rtn_result = 0;
	}

	return rtn_result;
}
