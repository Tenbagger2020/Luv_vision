//#include "stdafx.h"
//#include "CalcAlign_NEW.h"
//
///** ****************************************************** **
// ** 2視野対応アライメントの演算実行
// ** 引数：
// **　int parts					アライメント視野数
// **　WORD method				探索機能
// **　double* target_mech_x		目標視野座標X(目標視野座標X配列の先頭アドレス)
// **　double* target_mech_y		目標視野座標Y(目標視野座標Y配列の先頭アドレス)
// **　double* base_mech_x		対象視野座標X(対象視野座標X配列の先頭アドレス)
// **　double* base_mech_y		対象視野座標Y(対象視野座標Y配列の先頭アドレス)
// **　double rotation_center_x	回転中心X
// **　double rotation_center_y	回転中心Y
// **　BOOL theta_complete		回転中心キャリブレーション完了フラグ
// **　double offset_x			オフセットX
// **　double offset_y			オフセットY
// **　double offset_t			オフセットT(radian)
// **　double* theta_x			補正量θX
// **　double* theta_y			補正量θY
// **　double* delta_t_rad		補正量⊿T(radian)
// **　double* delta_x			補正量⊿X
// **　double* delta_y			補正量⊿Y
// ** 戻り値：
// **　int 結果　0:成功、1:失敗
// ** 特記事項：
// **		アライメント視野数は2
// **		対象とする探索機能は
// **		0x0101:中点一致
// **		0x0102:端点一致 視野1
// **		0x0103:端点一致 視野2
// ** ****************************************************** **/
//int CCalcAlign::TwoPartsAlignment(int parts,
//	WORD method,
//	double* target_mech_x, double* target_mech_y,
//	double* base_mech_x, double* base_mech_y,
//	double rotation_center_x, double rotation_center_y, BOOL theta_complete,
//	double offset_x, double offset_y, double offset_t,
//	double* theta_x, double* theta_y, double* delta_t_rad, double* delta_x, double* delta_y)
//{
//	// TODO: ここに実装コードを追加します.
//	int rtn_result = 1;
//
//	//アライメント視野数の確認
//	if (parts != 2) goto END;
//
//	//探索機能の確認
//	if (method != 0x0101 && method != 0x0102 && method != 0x0103) goto END;
//
//	double pi = acos(double(-1));
//
//	//対象側傾き
//	double base_theta = 0;
//	//FVXでは視野1,2でX方向の変化がない場合、90度足していた
//	//　WILでは下記に変更
//	//　　・Y方向も変化がない場合 → 0
//	//　　・Y方向視野1より視野2のY位置が大きい場合 → -(pi / 2) -90度
//	//　　・Y方向視野1より視野2のY位置が小さい場合 → +(pi / 2) +90度
//	//また視野1より視野2のX位置が小さい場合、180度足していた
//	//　→ WILでは検討要
//	if ((base_mech_x[1] - base_mech_x[0]) == 0) {
//		//base_theta = pi / 2;
//		if ((base_mech_y[1] - base_mech_y[0]) == 0) {
//			base_theta = 0;
//		}
//		else if ((base_mech_y[1] - base_mech_y[0]) > 0) {
//			base_theta = -(pi / 2);
//		}
//		else if ((base_mech_y[1] - base_mech_y[0]) < 0) {
//			base_theta = pi / 2;
//		}
//	}
//	else if ((base_mech_x[1] - base_mech_x[0]) > 0) {
//		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
//	}
//	else if ((base_mech_x[1] - base_mech_x[0]) < 0) {
//		base_theta = atan((base_mech_y[1] - base_mech_y[0]) / (base_mech_x[1] - base_mech_x[0]));
//		base_theta = pi + base_theta;
//	}
//
//	//目標側傾き
//	double target_theta = 0;
//	//FVXでは視野1,2でX方向の変化がない場合、90度足していた
//	//　WILでは下記に変更
//	//　　・Y方向も変化がない場合 → 0
//	//　　・Y方向視野1より視野2のY位置が大きい場合 → -(pi / 2) -90度
//	//　　・Y方向視野1より視野2のY位置が小さい場合 → +(pi / 2) +90度
//	//また視野1より視野2のX位置が小さい場合、180度足していた
//	//　→ WILでは検討要
//	if ((target_mech_x[1] - target_mech_x[0]) == 0) {
//		//target_theta = pi / 2;
//		if ((target_mech_y[1] - target_mech_y[0]) == 0) {
//			target_theta = 0;
//		}
//		else if ((target_mech_y[1] - target_mech_y[0]) > 0) {
//			target_theta = -(pi / 2);
//		}
//		else if ((target_mech_y[1] - target_mech_y[0]) < 0) {
//			target_theta = pi / 2;
//		}
//	}
//	else if ((target_mech_x[1] - target_mech_x[0]) > 0) {
//		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
//	}
//	else if ((target_mech_x[1] - target_mech_x[0]) < 0) {
//		target_theta = atan((target_mech_y[1] - target_mech_y[0]) / (target_mech_x[1] - target_mech_x[0]));
//		target_theta = pi + target_theta;
//	}
//
//	//補正値Δθ
//	double dan = target_theta - base_theta;
//	double offset_target_theta = target_theta + offset_t;
//	//dT = dan*-1;
//	*delta_t_rad = dan + offset_t;
//
//	//対象側センター座標
//	double base_center_x = 0;
//	double base_center_y = 0;
//	switch (method) {
//		//中点一致
//	case 0x0101:
//		base_center_x = (base_mech_x[0] + base_mech_x[1]) / 2;
//		base_center_y = (base_mech_y[0] + base_mech_y[1]) / 2;
//		break;
//		//端点一致 視野1
//	case 0x0102:
//		base_center_x = base_mech_x[0];
//		base_center_y = base_mech_y[0];
//		break;
//		//端点一致 視野2
//	case 0x0103:
//		base_center_x = base_mech_x[1];
//		base_center_y = base_mech_y[1];
//		break;
//	}
//
//	//目標側センター座標
//	//どこに合わせこむかにより場合分け
//	double target_center_x = 0;
//	double target_center_y = 0;
//	switch (method) {
//		//中点一致
//	case 0x0101:
//		target_center_x = (target_mech_x[0] + target_mech_x[1]) / 2;
//		target_center_y = (target_mech_y[0] + target_mech_y[1]) / 2;
//		//ｵﾌｾｯﾄ加算
//		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
//		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
//		break;
//		//端点一致 視野1
//	case 0x0102:
//		target_center_x = target_mech_x[0];
//		target_center_y = target_mech_y[0];
//		//ｵﾌｾｯﾄ加算
//		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
//		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
//		break;
//		//端点一致 視野2
//	case 0x0103:
//		target_center_x = target_mech_x[1];
//		target_center_y = target_mech_y[1];
//		//ｵﾌｾｯﾄ加算
//		target_center_x = target_center_x + offset_x * cos(offset_target_theta) - offset_y * sin(offset_target_theta);
//		target_center_y = target_center_y + offset_y * cos(offset_target_theta) + offset_x * sin(offset_target_theta);
//		break;
//	}
//
//	//θ成分含むXY(θX,θY)の算出
//	if (theta_complete) {
//		//θ回転後の対象マークセンター座標
//		double theta_aligned_center_x = (base_center_x - rotation_center_x) * cos(*delta_t_rad) - (base_center_y - rotation_center_y) * sin(*delta_t_rad) + rotation_center_x;
//		double theta_aligned_center_y = (base_center_x - rotation_center_x) * sin(*delta_t_rad) + (base_center_y - rotation_center_y) * cos(*delta_t_rad) + rotation_center_y;
//
//		//デバック 20200403
//		//　演算方法を替え算出してみたが結果は同じだったので、既存の算出方法のままとする
//		//double sub_x = rotation_center_x - base_center_x;
//		//double sub_y = rotation_center_y - base_center_y;
//		//double L = sqrt(pow(sub_x, 2.0) + pow(sub_y, 2.0));
//
//		//double theta_base_rad = acos(sub_x / L);
//		//double theta_base_rad2 = atan(sub_y / sub_x);
//		//double theta_algn_rad = theta_base_rad + *delta_t_rad;
//
//		//double debug_align_x = L * cos(theta_algn_rad);
//		//double debug_align_y = L * sin(theta_algn_rad);
//		//double debug_theta_aligned_center_x = rotation_center_x - debug_align_x;
//		//double debug_theta_aligned_center_y = rotation_center_y - debug_align_y;
//		//デバック ここまで
//
//
//		//最終アライメントセンター位置座標   
//		//RESULT_DATA.mXac=nx;
//		//RESULT_DATA.mYac=ny;
//
//		//アライメント視野数2の場合
//		if (parts == 2) {
//			//符号を変える
//			*theta_x = target_center_x - theta_aligned_center_x;
//			*theta_y = target_center_y - theta_aligned_center_y;
//		}
//	}
//	//回転中心キャリブレーションが完了していない場合(回転中心キャリブレーションをしていない場合)
//	else {
//		*theta_x = 0;
//		*theta_y = 0;
//	}
//
//	//補正値Δｘ、補正値Δｙ
//	*delta_x = target_center_x - base_center_x;
//	*delta_y = target_center_y - base_center_y;
//
//	rtn_result = 0;
//
//END:
//	return rtn_result;
//}
//
//
//
///** ****************************************************** **
// ** 対角中点座標を求める
// ** 引数：
// **　int parts				アライメント視野数
// **　double* center_x		中心座標X
// **　double* center_y		中心座標Y
// **　const double* mech_x	視野座標X(目標視野座標X配列の先頭アドレス)
// **　const double* mech_y	視野座標Y(目標視野座標X配列の先頭アドレス)
// ** 戻り値：
// **　int 結果　0:成功、1:失敗
// ** 特記事項：
// ** ****************************************************** **/
//int CCalcAlign::GetPartsCenter(int parts, double* center_x, double* center_y, const double* mech_x, const double* mech_y)
//{
//	// TODO: ここに実装コードを追加します.
//	int rtn_result = 1;
//
//	//アライメント視野数の確認
//	if (parts < 3 || parts > 4) goto END;
//
//	double a1 = 0, b1 = 0, a2 = 0, b2 = 0;
//
//	double mech_cent_x[4];	//マーク間センターX
//	double mech_cent_y[4];	//マーク間センターY
//
//	//アライメント視野数4
//	if (parts == 4) {
//
//		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
//		mech_cent_x[1] = (mech_x[1] + mech_x[2]) / 2;
//		mech_cent_x[2] = (mech_x[2] + mech_x[3]) / 2;
//		mech_cent_x[3] = (mech_x[3] + mech_x[0]) / 2;
//
//		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
//		mech_cent_y[1] = (mech_y[1] + mech_y[2]) / 2;
//		mech_cent_y[2] = (mech_y[2] + mech_y[3]) / 2;
//		mech_cent_y[3] = (mech_y[3] + mech_y[0]) / 2;
//
//		if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0 && mech_cent_x[0] != 0 && mech_cent_x[1] != 0) {
//
//			b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
//			a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];
//
//			b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
//			a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];
//
//			//交点
//			*center_y = (a1 * b2 - a2 * b1) / (a1 - a2);
//			*center_x = (*center_y - b1) / a1;
//		}
//		else {
//			if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) != 0) {
//
//				b2 = (mech_cent_x[1] * mech_cent_y[3] - mech_cent_x[3] * mech_cent_y[1]) / (mech_cent_x[1] - mech_cent_x[3]);
//				a2 = (mech_cent_y[1] - b2) / mech_cent_x[1];
//
//				//交点
//				*center_x = mech_cent_x[0];
//				*center_y = a2 * (*center_x) + b2;
//			}
//			else if ((mech_cent_x[0] - mech_cent_x[2]) != 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {
//
//				b1 = (mech_cent_x[0] * mech_cent_y[2] - mech_cent_x[2] * mech_cent_y[0]) / (mech_cent_x[0] - mech_cent_x[2]);
//				a1 = (mech_cent_y[0] - b1) / mech_cent_x[0];
//
//				//交点
//				*center_x = mech_cent_x[1];
//				*center_y = a1 * (*center_x) + b1;
//			}
//			else if ((mech_cent_x[0] - mech_cent_x[2]) == 0 && (mech_cent_x[1] - mech_cent_x[3]) == 0) {
//
//				*center_x = mech_cent_x[0];
//				*center_y = (mech_y[0] + mech_y[3]) / 2;
//			}
//		}
//	}
//	//アライメント視野数3
//	else if (parts == 3) {
//		mech_cent_x[0] = (mech_x[0] + mech_x[1]) / 2;
//		mech_cent_y[0] = (mech_y[0] + mech_y[1]) / 2;
//		mech_cent_x[1] = (mech_x[0] + mech_x[2]) / 2;
//		mech_cent_y[1] = (mech_y[0] + mech_y[2]) / 2;
//
//		if ((mech_x[1] - mech_cent_x[1]) != 0 && (mech_x[2] - mech_cent_x[0]) != 0 && mech_x[1] != 0 && mech_x[2] != 0) {
//
//			b1 = (mech_x[1] * mech_cent_y[1] - mech_cent_x[1] * mech_y[1]) / (mech_x[1] - mech_cent_x[1]);
//			a1 = (mech_y[1] - b1) / mech_x[1];
//
//			b2 = (mech_x[2] * mech_cent_y[0] - mech_cent_x[0] * mech_y[2]) / (mech_x[2] - mech_cent_x[0]);
//			a2 = (mech_y[2] - b2) / mech_x[2];
//
//			//交点
//			*center_y = (a1 * b2 - a2 * b1) / (a1 - a2);
//			*center_x = (*center_y - b1) / a1;
//		}
//		else {
//			goto END;
//		}
//	}
//
//	rtn_result = 0;
//
//END:
//	return rtn_result;