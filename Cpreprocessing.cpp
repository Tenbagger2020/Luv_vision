#include "stdafx.h"
#include "Cpreprocessing.h"
#include "FVCLbasic.h"	//WIL FVCLクラスヘッダ

void Cpreprocessing::Improve5()

{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
	//extern ProcGrabImage ss;
	//撮像画像の読み込み
	FVCL::Data::CFvImage src_org;
	FVCL::Data::CFvImage m_Image;
//	FVCL::File::SaveImageFile("C:\\E35088\\B\\Live画像.bmp", m_Image);
	FVCL::File::LoadImageFile(_T("C:\\E35088\\B\\Live画像.bmp"), &src_org);


	//INT status_org = FVCL::File::LoadImageFile(_T("C:\\E35088\\B\\Live画像.bmp"), &src_org);




	/*if (status_org != FVCL_ErrorCode::_SUCCESS) {
		MessageBoxA(NULL, "画像読み込みエラー", "エラー", MB_OK | MB_ICONERROR);
		return;
	}*/

	//ソース画像の輝度調整
	//
	FVCL::Data::CFvImage src_adjust;
	if (Imp5_MeanAdj(&src_org, &src_adjust) != 0) return;

	//平均化画像の作成
	//
	FVCL::Data::CFvImage img_avemn;
	if (Imp5_AveMN(&src_adjust, &img_avemn) != 0) return;

	//ソース画像の加算処理
	//
	FVCL::Data::CFvImage src_add;
	if (Imp5_AddScl(&src_adjust, &src_add) != 0) return;

	//差分画像の作成
	//
	FVCL::Data::CFvImage img_sub;
	if (Imp5_SubOpe(&src_add, &img_avemn, &img_sub) != 0) return;

	//画素パターン周囲領域を割り出す2値化閾値の自動算出
	//
//	int rate = 82;			//黒割合
	int temp55= GetPrivateProfileIntA("rate", "rate", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");			//黒割合 iniファイルから読み込み（by honda）
	int rate = temp55;


	int threshold_auto = 0;	//算出閾値
	if (Imp5_PTile(&img_sub, rate, &threshold_auto) != 0) return;

	//2値化調整領域画像の取得
	//
	FVCL::Data::CFvImage img_vert;
	if (Imp5_CopyRect(&img_sub, &img_vert) != 0) return;

	//画素パターン周囲幅の2値化閾値調整
	//　最大10回まで調整とする、10回で調整しきれない場合はエラーとする
	//double min_limit = 23.0;	//min/maxは柔軟に変更できたほうがいい
	//double max_limit = 25.0;

	double temp56= GetPrivateProfileIntA("min_limit", "min_limit", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");	//min/maxは柔軟に変更できたほうがいい　iniファイルから読み込み（by honda）
	double temp57 =GetPrivateProfileIntA("max_limit", "max_limit", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	double min_limit = temp56;
	double max_limit = temp57;


	double threshold_adjust = (double)threshold_auto;	//開始調整閾値
	double threshold_end = 0;	//最終調整閾値
	int cnt_adjust = 0;
	for (int i = 0; i < 10; i++) {

		//平均縦幅の算出
		double ave_width = 0;
		if (Imp5_GetAveWidth(&img_vert, threshold_adjust, &ave_width) != 0) return;

		//平均縦幅の判定
		if (min_limit <= ave_width && ave_width <= max_limit) {
			//調整成功
			threshold_end = threshold_adjust;
			break;
		}


		//honda 探索NGになってしまうのでコメントアウト
		//調整カウンタ判定
		//cnt_adjust++;
		//if (cnt_adjust >= 10) {
		//	MessageBoxA(NULL, "調整に失敗しました", "エラー", MB_OK | MB_ICONERROR);
		//	return;
		//}

		//2値閾値の調整 細かく閾値を変更するかどうかは検討の余地がある
		if (ave_width < min_limit) {
			//閾値を減らす
			threshold_adjust -= 0.5;
		}
		else if (max_limit < ave_width) {
			//閾値を増やす
			threshold_adjust += 0.5;
		}
	}

	//ノイズ除去
	//
	//UINT max_noize = 79;	//除去ノイズサイズ 79(選択可に)
	UINT temp13= GetPrivateProfileIntA("max_noize", "max_noize", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");	//除去ノイズサイズ 79(選択可に)  iniファイルから読み込み（by honda）	
	UINT max_noize = temp13;


	FVCL::Data::CFvRegion reg_around;
	if (Imp5_RemoveNoize(&img_sub, threshold_end, max_noize, &reg_around) != 0) return;

	//画素パターン周囲の濃度アップ補正
	//
	FVCL::Data::CFvImage img_around;
	if (Imp5_ProcessImage(&src_adjust, &img_around, &reg_around, threshold_end) != 0) return;

	//画素パターン周囲のくり抜き
	//
	FVCL::Data::CFvImage img_cut;
	if (Imp5_CutAround(&src_adjust, &img_cut, &reg_around) != 0) return;

	//画像の合成
	//
	if (Imp5_AddImage(&img_around, &img_cut) != 0) return;
}


/** ****************************************************** **
 ** 画像輝度調整
 ** 引数：
 **　FVCL::Data::CFvImage* src	入力画像
 **　FVCL::Data::CFvImage* mean	出力画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_MeanAdj(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* mean)
{
	//画像全体の輝度平均の算出
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	FVCL::Data::CFvRegion reg_rect = FVCL::Data::CFvRegion(FVCL::Data::CFvRectangle(0, 0, horz_src - 1, vert_src - 1));
	double whole_score = FVCL::Region::Measure::MeanPixel(*src, reg_rect, FVCL::OriginMode::Image);
	//double adjust_value = 96.0 - whole_score;

	double temp = GetPrivateProfileIntA("adjust_value", "adjust_value", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	double adjust_value = temp- whole_score; 
	//GetPrivateProfileIntA("adjust_value", "adjust_value", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");	// iniファイルから読み込み（by honda）


	//画像を明るく調整する
	if (adjust_value > 0) {
		FVCL::Operation::CFvAddScalar parser;
		FVCL::Data::CFvImage dst;

		//処理対象画像の有効性検査
		if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvAddScalar::CheckValidity(*src)) {
			MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_MeanAdj", MB_OK | MB_ICONERROR);
			return 1;
		}

		//処理ウィンドウ
		src->SetWindow(0, 0, horz_src, vert_src);

		//画像処理準備
		parser.SetSrcImage(0, src);
		parser.SetDstImage(0, &dst);
		if (!parser.IsValid()) parser.Validate();	//有効化

		//パラメータ設定
		parser.SetValue(adjust_value);	//加算値

		//画像処理実行
		if (!parser.Execute()) {
			MessageBoxA(NULL, "スカラー加算実行エラー", "Imp5_MeanAdj", MB_OK | MB_ICONERROR);
			return 1;
		}
		else {
			*mean = dst;
			FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_0_MeanAdj.bmp"), dst);
		}
	}
	//画像を暗く調整する
	else if (adjust_value < 0) {
		FVCL::Operation::CFvSubScalar parser;
		FVCL::Data::CFvImage dst;

		//処理対象画像の有効性検査
		if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvSubScalar::CheckValidity(*src)) {
			MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_MeanAdj", MB_OK | MB_ICONERROR);
			return 1;
		}

		//処理ウィンドウ
		src->SetWindow(0, 0, horz_src, vert_src);

		//画像処理準備
		parser.SetSrcImage(0, src);
		parser.SetDstImage(0, &dst);
		if (!parser.IsValid()) parser.Validate();	//有効化

		//パラメータ設定
		parser.SetValue(0 - adjust_value);	//減算値

		//画像処理実行
		if (!parser.Execute()) {
			MessageBoxA(NULL, "スカラー減算実行エラー", "Imp5_MeanAdj", MB_OK | MB_ICONERROR);
			return 1;
		}
		else {
			*mean = dst;
			FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_0_MeanAdj.bmp"), dst);
		}
	}
	else {
		//調整不要
		*mean = *src;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_0_MeanAdj.bmp"), *src);
	}

	return 0;
}


/** ****************************************************** **
 ** 平滑化フィルタ
 ** 引数：
 **　FVCL::Data::CFvImage* src	入力画像
 **　FVCL::Data::CFvImage* ave	出力画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_AveMN(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* ave)
{
	FVCL::Filter::CFvAverageMxN parser;
	FVCL::Data::CFvImage dst;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Filter::CFvAverageMxN::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_AveMN", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	//画像処理準備
	parser.SetSrcImage(0, src);
	parser.SetDstImage(0, &dst);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	//parser.SetHorz(101);		//フィルタ幅
	//parser.SetVert(101);		//フィルタ高さ
	//parser.SetBorderMode(5);	//ボーダーモード　反転モード


	//パラメータ設定
	parser.SetHorz(GetPrivateProfileIntA("SET", "SetHorz", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini"));		//フィルタ幅
	parser.SetVert(GetPrivateProfileIntA("SET", "SetVert", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini"));		//フィルタ高さ
	parser.SetBorderMode(GetPrivateProfileIntA("SET", "SetBorderMode", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini"));	//ボーダーモード　反転モード			// iniファイルから読み込み（by honda）




	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "平滑化実行エラー", "Imp5_AveMN", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*ave = dst;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_1_AveMN.bmp"), dst);
	}

	return 0;
}


/** ****************************************************** **
 ** スカラー加算
 ** 引数：
 **　FVCL::Data::CFvImage* src	入力画像
 **　FVCL::Data::CFvImage* add	出力画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_AddScl(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* add)
{
	FVCL::Operation::CFvAddScalar parser;
	FVCL::Data::CFvImage dst;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvAddScalar::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_AddScl", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	//画像処理準備
	parser.SetSrcImage(0, src);
	parser.SetDstImage(0, &dst);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	//parser.SetValue(32.0);	//加算値
	
	int  temp1 = GetPrivateProfileIntA("SET", "SetValue", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	parser.SetValue(temp1);	//加算値		// iniファイルから読み込み（by honda）


	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "スカラー加算実行エラー", "Imp5_AddScl", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*add = dst;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_2_AddScl.bmp"), dst);
	}

	return 0;
}


/** ****************************************************** **
 ** 画像間減算
 ** 引数：
 **　FVCL::Data::CFvImage* src1	入力画像1
 **　FVCL::Data::CFvImage* src2	入力画像2
 **　FVCL::Data::CFvImage* sub	出力画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_SubOpe(FVCL::Data::CFvImage* src1, FVCL::Data::CFvImage* src2, FVCL::Data::CFvImage* sub)
{
	FVCL::Operation::CFvSubOperation parser;
	FVCL::Data::CFvImage dst;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvSubOperation::CheckValidity(*src1, *src2)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_SubOpe", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src1->GetHorzSize();
	UINT vert_src = src1->GetVertSize();
	src1->SetWindow(0, 0, horz_src, vert_src);
	src2->SetWindow(0, 0, horz_src, vert_src);

	//画像処理準備
	parser.SetSrcImage(0, src1);
	parser.SetSrcImage(1, src2);
	parser.SetDstImage(0, &dst);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定

	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "画像間減算実行エラー", "Imp5_AveMN", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*sub = dst;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_3_SubOpe.bmp"), dst);
	}

	return 0;
}


/** ****************************************************** **
 ** 2値化(PTile法)
 ** 引数：
 **　FVCL::Data::CFvImage* src	入力画像
 **　int rate					黒割合(%)
 **　int* threshold				閾値結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_PTile(FVCL::Data::CFvImage* src, int rate, int* threshold)
{
	FVCL::Conversion::CFvBinarizePTile parser;
	FVCL::Data::CFvImage dst;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarizePTile::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_PTile", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	//画像処理準備
	parser.SetSrcImage(0, src);
	parser.SetDstImage(0, &dst);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetRate(rate);	//黒が設定割合を占める閾値が自動的に算出される 例)82

	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "2値化実行エラー", "Imp5_PTile", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*threshold = parser.GetThreshold(0);	//算出された閾値を取得

		//画像保存する場合
		//1) 1bit2値画像の保存
		//FVCL::File::SaveImageFile(_T("C:\\E35088\\I4_1_PTile.bmp"), dst);
		//2) 8bit画像の保存(8bit２値化画像を取得する場合)
		FVCL::Data::CFvImage img_uc8;
		dst.ConvertToGray(0, 0, &img_uc8);
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_4_PTile.bmp"), img_uc8);
		//		UpdateMainView(1);
	}

	return 0;
}


/** ****************************************************** **
 ** 範囲画像の抜き出し
 ** 引数：
 **　FVCL::Data::CFvImage* src	入力画像
 **　FVCL::Data::CFvImage* copy	出力画像
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_CopyRect(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* copy)
{
	// E02 xs(400,350) xe(449,389)
	// E03 xs(650,450) xe(699,489)
	// PB2 xs(560,530) xe(609,569)
	// MM2 xs(610,420) xe(659,459)
	// MM3 xs(610,360) xe(659,399)
	// MM4 xs(620,330) xe(669,369)
	// MM5 xs(410,700) xe(459,739)
	// MM6 xs(580,560) xe(629,599)
	// BASE_B xs(530,480) xe(579,519)

	//FVCL::Data::CFvImage img(50, 40, FVCL::ImageType::UC8, 1, 0);	//抜き出し画像サイズを指定


	int temp1 = GetPrivateProfileIntA("IMG", "img1", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	int temp2 = GetPrivateProfileIntA("IMG", "img2", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");

	//FVCL::Data::CFvImage img(GetPrivateProfileIntA("IMG", "img1", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini"), GetPrivateProfileIntA("IMG", "img2", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini"), FVCL::ImageType::UC8, 1, 0);	//抜き出し画像サイズを指定		// iniファイルから読み込み（by honda）
	FVCL::Data::CFvImage img(temp1,temp2, FVCL::ImageType::UC8, 1, 0);	//抜き出し画像サイズを指定		// iniファイルから読み込み（by honda）


	//src->SetWindow(400, 350, 50, 40);	//E02
	//src->SetWindow(650, 450, 50, 40);	//E03
	//src->SetWindow(560, 530, 50, 40);	//PB2


	int temp3 = GetPrivateProfileIntA("SetWindow", "SetWindow1", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	int temp4 = GetPrivateProfileIntA("SetWindow", "SetWindow2", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	int temp5 = GetPrivateProfileIntA("SetWindow", "SetWindow3", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	int temp6 = GetPrivateProfileIntA("SetWindow", "SetWindow4", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");

	src->SetWindow(temp3, temp4, temp5, temp6);	//PB2	// iniファイルから読み込み（by honda）
	
																																																																																																					
																																																																																																					//src->SetWindow(610, 420, 50, 40);	//MM2
	//src->SetWindow(610, 360, 50, 40);	//MM3
	//src->SetWindow(620, 330, 50, 40);	//MM4
	//src->SetWindow(410, 700, 50, 40);	//MM5
	//src->SetWindow(580, 560, 50, 40);	//MM6
	//src->SetWindow(530, 480, 50, 40);	//BASE
	if (!img.Copy(*src, 0)) {
		MessageBoxA(NULL, "画像コピーエラー", "Imp5_CopyRect", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*copy = img;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\vert_copy.bmp"), img);
	}

	return 0;
}


/** ****************************************************** **
 ** 画像の2値化と平均幅の算出
 ** 引数：
 **　FVCL::Data::CFvImage* image	入力画像
 **　double threshold	2値化閾値
 **　double* ave_width	平均幅結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_GetAveWidth(FVCL::Data::CFvImage* image, double threshold, double* ave_width)
{
	//2値化の実施
	FVCL::Conversion::CFvBinarize parser_binz;
	FVCL::Data::CFvImage dst;

	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarize::CheckValidity(*image)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_GetAveWidth", MB_OK | MB_ICONERROR);
		return 1;
	}

	parser_binz.SetSrcImage(0, image);
	parser_binz.SetDstImage(0, &dst);
	if (!parser_binz.IsValid()) parser_binz.Validate();

	parser_binz.SetThreshold(threshold);		//閾値設定
	if (!parser_binz.Execute()) {
		MessageBoxA(NULL, "2値化実行エラー", "Imp5_GetAveWidth", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\vert_binz.bmp"), dst);
	}

	//幅データの初期化
	UINT vert_size = image->GetVertSize();
	int* max_width = new int[vert_size];
	for (int i = 0; i < vert_size; i++) {
		max_width[i] = -1;
	}

	//各行毎の最大幅を取得
	FVCL::Data::CFvRegion reg;
	reg.Create(dst);
	UINT run_count = reg.GetCount();
	for (UINT i = 0; i < run_count; i++) {
		FVCL::Data::CFvRunLength run = reg.GetRunLength(i);
		int row = run.y;
		int i_width = run.xe - run.xs + 1;

		if (row >= 0 && row < vert_size) {
			if (max_width[row] < i_width) {
				max_width[row] = i_width;
			}
		}
	}

	//honda 探索NGになってしまうのでコメントアウト

	////全行のデータ取得チェック
	//for (UINT i = 0; i < vert_size; i++) {
	//	if (max_width[i] == -1) {
	//		MessageBoxA(NULL, "幅データ取得失敗", "Imp5_GetAveWidth", MB_OK | MB_ICONERROR);
	//		return 1;
	//	}
	//}

	//平均縦幅の算出
	int sum_width = 0;
	for (int i = 0; i < vert_size; i++) {
		sum_width += max_width[i];
	}
	*ave_width = (double)sum_width / (double)vert_size;

	//配列解放
	if (max_width != NULL) delete[] max_width;

	return 0;
}


/** ****************************************************** **
 ** ブロブ処理によるノイズ除去
 ** 引数：
 **　FVCL::Data::CFvImage* image	入力画像
 **　double threshold	2値化閾値
 **　UINT max_noize		最大ノイズサイズ
 **　FVCL::Data::CFvRegion* region	領域結果
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_RemoveNoize(FVCL::Data::CFvImage* src, double threshold, UINT max_noize, FVCL::Data::CFvRegion* region)
{
	// ///////////////////////////////
	// 2値化の実施
	// ///////////////////////////////
	FVCL::Conversion::CFvBinarize parser_binz;
	FVCL::Data::CFvImage dst_binz;

	if (FVCL_ErrorCode::_SUCCESS != FVCL::Conversion::CFvBinarize::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	parser_binz.SetSrcImage(0, src);
	parser_binz.SetDstImage(0, &dst_binz);
	if (!parser_binz.IsValid()) parser_binz.Validate();

	parser_binz.SetThreshold(threshold);		//閾値設定
	if (!parser_binz.Execute()) {
		MessageBoxA(NULL, "2値化実行エラー", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
		return 1;
	}

	// ///////////////////////////////
	// ブロブ処理
	// ///////////////////////////////
	FVCL::Blob::CFvBlob	parser_blob;
	FVCL::Blob::CFvBlobResult result_blob;

	//処理対象画像の有効性検査
	if (FVCL_ErrorCode::_SUCCESS != FVCL::Blob::CFvBlob::CheckValidity(dst_binz)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
		return 1;
	}

	//画像処理準備
	parser_blob.SetSrcImage(0, &dst_binz);
	parser_blob.SetResult(&result_blob);

	//パラメータ設定
	FVCL::Blob::CFvBlobParam& param = parser_blob.GetParam();
	//byte color_mode = 0x01;	//抽出色 白:0x01

	byte temp8= GetPrivateProfileIntA("Color_mode", "Color_mode", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");	// iniファイルから読み込み（by honda）
	byte color_mode = temp8;


	if (color_mode == 0x00) param.SetColorMode(FVCL::Blob::ObjectColor::BlackFG);	//前景:黒、背景:計測しない
	if (color_mode == 0x01) param.SetColorMode(FVCL::Blob::ObjectColor::WhiteFG);	//前景:白、背景:計測しない
	param.SetMaxBlobs(0); // 0:上限なし
	param.SetMaxRows(0);  // 0:上限なし
	param.SetMaxRuns(0);  // 0:上限なし
	param.SetNeighborhood(FVCL::Blob::Neighborhood::Eight);   // 8近傍
	param.SetPrecalcFeatures(0);

	//抽出個数
	UINT total_count = 0;
	//抽出総面積
	UINT total_area = 0;

	//結果配列 初期化
	struct ResultArray {
		BOOL blob_enable;
		UINT select_area;
	};
	ResultArray* result_array = NULL;
	int pick_up_num = 0;

	//画像処理実行
	if (parser_blob.Execute()) {

		//検出個数
		//　ブローブ数は背景ブローブが必ずあるので1以上になる
		//　0の場合は異常終了
		UINT count_result = result_blob.GetCount();

		//検出成功
		if (count_result > 0) {
			result_array = new ResultArray[count_result];	//判定結果Array作成

			//結果の取得
			for (UINT i = 0; i < count_result; i++) {
				FVCL::Blob::CFvBlobData data;
				result_blob.GetData(i, &data);

				//結果初期化
				result_array[i].blob_enable = FALSE;
				result_array[i].select_area = 0;

				if (data.GetBlobNo() == 0) goto LOOP_END;	//背景ブローブ(ブローブ番号0)の結果は取得しない

				//結果の抽出と描画
				{
					//リージョンの取得
					FVCL::Data::CFvRegion m_Region_Blob;		//ブロブ結果のリージョン
					result_blob.GetRegion(i, &m_Region_Blob);

					//ブロブ面積抽出
					//　抽出サイズ(min～)から外れるものは対象外
					UINT select_area = data.GetArea();	//面積取得
					result_array[i].select_area = select_area;
					UINT area_min = max_noize + 1;	//抽出サイズmin
					if (!(select_area >= area_min)) goto LOOP_END;

					result_array[i].blob_enable = TRUE;

					//抽出個数と面積計算
					total_count++;				//個数inc
					total_area += select_area;	//総面積add

					//Regionの加算
					if (total_count == 1) {
						*region = m_Region_Blob;
					}
					else {
						*region = *region | m_Region_Blob;
					}
				}

				//ループEND
			LOOP_END:;
			}
		}
		else {
			MessageBoxA(NULL, "ブロブ実行エラー", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
			return 1;
		}
	}

	//領域を画像化
	if (total_count > 0) {
		//画像保存する場合
		UINT horz_binz = dst_binz.GetHorzSize();
		UINT vert_binz = dst_binz.GetVertSize();
		bool rslt = false;
		FVCL::Data::CFvImage dst_reg;

		//1) 1bit2値画像の保存
		FVCL::Data::CFvImage img_bin(horz_binz, vert_binz, FVCL::ImageType::BIN, 1, 0);	//ブロブ後抽出画像(BIN画像指定)
		dst_reg.Clear(0, 1);	//作成画像背景を黒
		rslt = region->Draw(&img_bin, 1.0);
		dst_reg = img_bin;

		//2) 8bit画像の保存(8bit２値化画像を取得する場合)
		FVCL::Data::CFvImage img_uc8;
		rslt = img_bin.ConvertToGray(0, 0, &img_uc8);
		dst_reg = img_uc8;

		if (rslt) {
			FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_5_AdjArea.bmp"), dst_reg);
		}
		else {
			MessageBoxA(NULL, "ブロブ画像作成", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
			return 1;
		}
	}
	else {
		MessageBoxA(NULL, "ブロブ抽出なし", "Imp5_RemoveNoize", MB_OK | MB_ICONERROR);
		return 1;
	}

	//結果配列の削除
	if (result_array != NULL) delete[] result_array;

	return 0;
}


/** ****************************************************** **
 ** 画像補正
 ** 引数：
 **　FVCL::Data::CFvImage* src		入力画像
 **　FVCL::Data::CFvImage* around	出力画像
 **　FVCL::Data::CFvRegion* region	補正領域
 **　double threshold	2値閾値
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_ProcessImage(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* around, FVCL::Data::CFvRegion* region, double threshold)
{
	//補正前 画素パターン周囲の輝度平均の算出
	double ave_around = FVCL::Region::Measure::MeanPixel(*src, *region, FVCL::OriginMode::Image);
	//濃度アップ量の算出 ※輝度MAXの3/4まで上げるイメージで..
	//double upValue = 192.0 - ave_around;	//192..
	double temp9 = GetPrivateProfileIntA("Value", "upValue", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
	double upValue = temp9- ave_around;
		// iniファイルから読み込み（by honda）


	if (upValue < 0) {
		//MessageBoxA("画素パターン周囲の輝度算出エラー", "Imp5_ProcessImage", MB_OK | MB_ICONERROR);
		//return 1;
		upValue = 0;
	}

	//各画素と指定値の加算
	FVCL::Operation::CFvAddScalar parser;
	FVCL::Data::CFvImage dst;

	if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvAddScalar::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_ProcessImage", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ(とりあえず)
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	parser.SetSrcImage(0, src);
	parser.SetDstImage(0, &dst);
	parser.SetMask(region);			//Regionでマスク、2値画像でもよい
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetValue(upValue);	//加算値

	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "画素加算実行エラー", "Imp5_ProcessImage", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*around = dst;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_5_Process.bmp"), dst);
	}

	//確認用
	//画素パターン周囲の輝度平均の算出
	double score_fin = FVCL::Region::Measure::MeanPixel(dst, *region, FVCL::OriginMode::Image);

	//画像全体の輝度平均の算出
	{
		UINT horz_src = src->GetHorzSize();
		UINT vert_src = src->GetVertSize();
		FVCL::Data::CFvRegion reg_rect = FVCL::Data::CFvRegion(FVCL::Data::CFvRectangle(80, 28, horz_src - 81, vert_src - 29));

		FVCL::Data::CFvRegion reg_cut = reg_rect ^ *region;
		double score = FVCL::Region::Measure::MeanPixel(*src, reg_cut, FVCL::OriginMode::Image);
		/*double adjust_value = 128.0;*/
		double temp10= GetPrivateProfileIntA("Value", "adlust_value", 0, "C:\\N-Vision\\WORK\\pre_parameter.ini");
		double adjust_value = temp10;
		// iniファイルから読み込み（by honda）

	}

	return 0;
}


/** ****************************************************** **
 ** 画素パターン周囲領域のくり抜き
 ** 引数：
 **　FVCL::Data::CFvImage* src		入力画像
 **　FVCL::Data::CFvRegion* region	補正領域
 **　double threshold	2値閾値
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_CutAround(FVCL::Data::CFvImage* src, FVCL::Data::CFvImage* cut, FVCL::Data::CFvRegion* region)
{
	FVCL::Operation::CFvSubScalar parser;
	FVCL::Data::CFvImage dst = *src;

	if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvSubScalar::CheckValidity(*src)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_CutAround", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src->GetHorzSize();
	UINT vert_src = src->GetVertSize();
	src->SetWindow(0, 0, horz_src, vert_src);

	parser.SetSrcImage(0, src);
	parser.SetDstImage(0, &dst);
	parser.SetMask(region);			//Regionでマスク、2値画像でもよい
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定
	parser.SetSwapOrder(false);
	parser.SetValue(255.0);	//減算値

	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "画素減算実行エラー", "Imp5_CutAround", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		*cut = dst;
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_6_CutAround.bmp"), dst);
	}

	return 0;
}


/** ****************************************************** **
 ** 補正画像とくり抜き画像の合成
 ** 引数：
 **　FVCL::Data::CFvImage* src1		入力画像1
 **　FVCL::Data::CFvImage* src2		入力画像2
 ** 戻り値：
 **　int 結果　0:成功、1:失敗
 ** 特記事項：
 ** ****************************************************** **/
int Cpreprocessing::Imp5_AddImage(FVCL::Data::CFvImage* src1, FVCL::Data::CFvImage* src2)
{
	FVCL::Operation::CFvAddOperation parser;
	FVCL::Data::CFvImage dst;

	if (FVCL_ErrorCode::_SUCCESS != FVCL::Operation::CFvAddOperation::CheckValidity(*src1, *src2)) {
		MessageBoxA(NULL, "読み込み画像の有効性エラー", "Imp5_AddImage", MB_OK | MB_ICONERROR);
		return 1;
	}

	//処理ウィンドウ
	UINT horz_src = src1->GetHorzSize();
	UINT vert_src = src1->GetVertSize();
	src1->SetWindow(0, 0, horz_src, vert_src);
	src2->SetWindow(0, 0, horz_src, vert_src);

	parser.SetSrcImage(0, src1);
	parser.SetSrcImage(1, src2);
	parser.SetDstImage(0, &dst);
	if (!parser.IsValid()) parser.Validate();	//有効化

	//パラメータ設定

	//画像処理実行
	if (!parser.Execute()) {
		MessageBoxA(NULL, "画素間加算実行エラー", "Imp5_AddImage", MB_OK | MB_ICONERROR);
		return 1;
	}
	else {
		FVCL::File::SaveImageFile(_T("C:\\E35088\\B\\I5_7_AddImage.bmp"), dst);
	}

	return 0;
}


void Cpreprocessing::OnStnClickedStaticMainview()
{
	// TODO: ここにコントロール通知ハンドラー コードを追加します。
}


