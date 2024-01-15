//////////////////////////////////////////////////////////
//														//
//			2D Euler Solver on Structured Grid			//
			//
//////////////////////////////////////////////////////////
#include <iostream>
#include <cmath>
#include "QlQr_Solver.h"
#include "Geometry.h"
#include "2D_Euler_Solver.h"

void Solve_QlQr()
{
	QlQr_Solver* half_node_q = new QlQr_Solver();

	half_node_q->Solve_QlQr();

	delete half_node_q;
}

VDouble3D qField;
VDouble3D qField1;
VDouble3D qField2;
VDouble3D qField_N0;
VDouble3D qField_N1;

QlQr_Solver::QlQr_Solver()
{
	Get_IJK_Region(ist, ied, jst, jed);
	Allocate_3D_Vector(qField1, num_half_point_x, num_half_point_y, num_of_prim_vars);
	Allocate_3D_Vector(qField2, num_half_point_x, num_half_point_y, num_of_prim_vars);
}

void QlQr_Solver::Solve_QlQr()
{
	if (method_of_half_q == 1)
	{
		this->QlQr_MUSCL();
	}
	else if (method_of_half_q == 2)
	{
		//WENO������а�ڵ��ֵ
	}
	else if (method_of_half_q == 3)
	{
		this->QlQr_WCNS();
	}
	else
	{
		cout << "��ڵ����ֵ��ֵ�����������飡" << endl;
	}
}

void QlQr_Solver::QlQr_MUSCL()
{
	if (solve_direction == 'x')
	{
		this->QlQr_MUSCL_X();
		this->Boundary_QlQr_MUSCL_X();
	}
	else if (solve_direction == 'y')
	{
		this->QlQr_MUSCL_Y();
		this->Boundary_QlQr_MUSCL_Y();
	}
	else
	{
		cout << "MUSCL��ֵ�������飡" << endl;
	}
}

void QlQr_Solver::Boundary_QlQr_MUSCL_X()
{
	//VInt2D& marker = mesh->Get_Marker();

	for (int j = jst; j <= jed; j++)
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[0][j][iVar] = qField[0][j][iVar]; //�����i=0, ied + 1��ֵ��û��
			qField2[0][j][iVar] = qField[1][j][iVar];
			//qField1[1][j][iVar] = qField[1][j][iVar];
			//qField2[1][j][iVar] = qField[2][j][iVar];

			//qField1[ied    ][j][iVar] = qField[ied    ][j][iVar];
			//qField2[ied    ][j][iVar] = qField[ied + 1][j][iVar];
			qField1[ied + 1][j][iVar] = qField[ied + 1][j][iVar];
			qField2[ied + 1][j][iVar] = qField[ied + 2][j][iVar];
		}
	}

	for (int j = jst + Jw1 + 1; j <= jst + Jw2 - 1; j++)
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[ist + Iw][j][iVar] = qField[ist + Iw    ][j][iVar];
			qField2[ist + Iw][j][iVar] = qField[ist + Iw + 1][j][iVar];

			qField1[ist + Iw + 1][j][iVar] = qField[ist + Iw + 1][j][iVar];
			qField2[ist + Iw + 1][j][iVar] = qField[ist + Iw + 2][j][iVar];
		}
	}
}

void QlQr_Solver::Boundary_QlQr_MUSCL_Y()
{
	//VInt2D& marker = mesh->Get_Marker();

	for (int i = ist; i <= ied; i++)
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)//�����j=0, jed + 1��ֵ��û��
		{
			qField1[i][0][iVar] = qField[i][0][iVar];
			qField2[i][0][iVar] = qField[i][1][iVar];
			//qField1[i][1][iVar] = qField[i][1][iVar];
			//qField2[i][1][iVar] = qField[i][2][iVar];

			//qField1[i][jed    ][iVar] = qField[i][jed    ][iVar];
			//qField2[i][jed    ][iVar] = qField[i][jed + 1][iVar];
			qField1[i][jed + 1][iVar] = qField[i][jed + 1][iVar];
			qField2[i][jed + 1][iVar] = qField[i][jed + 2][iVar];
		}
	}

	for (int i = ist + Iw + 1; i <= ied; i++)
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[i][jst + Jw1][iVar] = qField[i][jst + Jw1    ][iVar];
			qField2[i][jst + Jw1][iVar] = qField[i][jst + Jw1 + 1][iVar];

			qField1[i][jst + Jw1 + 1][iVar] = qField[i][jst + Jw1 + 1][iVar];
			qField2[i][jst + Jw1 + 1][iVar] = qField[i][jst + Jw1 + 2][iVar];

			qField1[i][jst + Jw2 - 1][iVar] = qField[i][jst + Jw2 - 1][iVar];
			qField2[i][jst + Jw2 - 1][iVar] = qField[i][jst + Jw2    ][iVar];

			qField1[i][jst + Jw2 - 2][iVar] = qField[i][jst + Jw2 - 2][iVar];
			qField2[i][jst + Jw2 - 2][iVar] = qField[i][jst + Jw2 - 1][iVar];
		}
	}
}

void QlQr_Solver::QlQr_MUSCL_X()
{
	//��x������в�ֵ
	VInt2D& marker = mesh->Get_Marker_F();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 1; i <= ied; i++)  //�����i=0, ied + 1��ֵ��û��
	{
		for (int j = jst; j <= jed; j++)
		{
			if (marker[i][j] == 0) continue;

			VDouble qVector_m1 = qField[i - 1][j];
			VDouble qVector_c0 = qField[i    ][j];
			VDouble qVector_p1 = qField[i + 1][j];
			VDouble qVector_p2 = qField[i + 2][j];

			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				double du_p1 = qVector_p1[iVar] - qVector_c0[iVar] + SMALL;
				double du_m1 = qVector_c0[iVar] - qVector_m1[iVar] + SMALL;
				double du_p3 = qVector_p2[iVar] - qVector_p1[iVar] + SMALL;

				double ita_m1_p = du_p1 / du_m1;
				double ita_p1_m = du_m1 / du_p1;
				double ita_p3_m = du_p1 / du_p3;
				double ita_p1_p = du_p3 / du_p1;

				double fai1 = Limiter_Function(ita_m1_p);
				double fai2 = Limiter_Function(ita_p1_m);
				double fai3 = Limiter_Function(ita_p3_m);
				double fai4 = Limiter_Function(ita_p1_p);

				qField1[i][j][iVar] = qVector_c0[iVar] + 1.0 / 4.0 * ((1 - muscl_k) * fai1 * du_m1
																	+ (1 + muscl_k) * fai2 * du_p1);

				qField2[i][j][iVar] = qVector_p1[iVar] - 1.0 / 4.0 * ((1 - muscl_k) * fai3 * du_p3
																	+ (1 + muscl_k) * fai4 * du_p1);
			}
		}
	}
}

void QlQr_Solver::QlQr_MUSCL_Y()
{
	//��y������в�ֵ
	VInt2D& marker = mesh->Get_Marker_F();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)
	{
		for (int j = 1; j <= jed; j++) //�����j=0, jed + 1��ֵ��û��
		{
			if (marker[i][j] == 0) continue;

			VDouble qVector_m1 = qField[i][j - 1];
			VDouble qVector_c0 = qField[i][j    ];
			VDouble qVector_p1 = qField[i][j + 1];
			VDouble qVector_p2 = qField[i][j + 2];

			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				double du_p1 = qVector_p1[iVar] - qVector_c0[iVar] + SMALL;
				double du_m1 = qVector_c0[iVar] - qVector_m1[iVar] + SMALL;
				double du_p3 = qVector_p2[iVar] - qVector_p1[iVar] + SMALL;

				double ita_m1_p = du_p1 / du_m1;
				double ita_p1_m = du_m1 / du_p1;
				double ita_p3_m = du_p1 / du_p3;
				double ita_p1_p = du_p3 / du_p1;

				double fai1 = Limiter_Function(ita_m1_p);
				double fai2 = Limiter_Function(ita_p1_m);
				double fai3 = Limiter_Function(ita_p3_m);
				double fai4 = Limiter_Function(ita_p1_p);

				qField1[i][j][iVar] = qVector_c0[iVar] + 1.0 / 4.0 * ((1 - muscl_k) * fai1 * du_m1
																	+ (1 + muscl_k) * fai2 * du_p1);

				qField2[i][j][iVar] = qVector_p1[iVar] - 1.0 / 4.0 * ((1 - muscl_k) * fai3 * du_p3
																	+ (1 + muscl_k) * fai4 * du_p1);
			}
		}
	}
}

void QlQr_Solver::QlQr_WCNS()
{
	if (solve_direction == 'x')
	{
		this->QlQr_WCNS_X();
		this->Boundary_QlQr_WCNS_X();
		//this->Boundary_QlQr_WCNS_X_5th();
	}
	else if (solve_direction == 'y')
	{
		this->QlQr_WCNS_Y();
		this->Boundary_QlQr_WCNS_Y();
		//this->Boundary_QlQr_WCNS_Y_5th();
	}
}

void QlQr_Solver::Boundary_QlQr_WCNS_X()
{	
	for (int j = jst; j <= jed; j++)//ȷ��i=0,1,ied,ied+1��ֵ
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[0][j][iVar] = 1.0 / 16 * ( 5 * qField[0][j][iVar] + 15 * qField[1][j][iVar]
											 - 5 * qField[2][j][iVar] +      qField[3][j][iVar]);

			qField1[1][j][iVar] = 1.0 / 16 * (-    qField[0][j][iVar] + 9 * qField[1][j][iVar]
											 + 9 * qField[2][j][iVar] -     qField[3][j][iVar]);

			qField1[ied    ][j][iVar] = 1.0 / 16 * (-    qField[ied + 2][j][iVar] + 9 * qField[ied + 1][j][iVar]
											       + 9 * qField[ied    ][j][iVar] -     qField[ied - 1][j][iVar]);

			qField1[ied + 1][j][iVar] = 1.0 / 16 * ( 5 * qField[ied + 2][j][iVar] + 15 * qField[ied + 1][j][iVar]
											       - 5 * qField[ied    ][j][iVar] +      qField[ied - 1][j][iVar]);

			qField2[0][j][iVar] = qField1[0][j][iVar];
			qField2[1][j][iVar] = qField1[1][j][iVar];

			qField2[ied    ][j][iVar] = qField1[ied    ][j][iVar];
			qField2[ied + 1][j][iVar] = qField1[ied + 1][j][iVar];
		}
	}
}

void QlQr_Solver::Boundary_QlQr_WCNS_Y()
{	 
	for (int i = ist; i <= ied; i++)//ȷ��j=0,1,jed,jed+1��ֵ
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[i][0][iVar] = 1.0 / 16 * ( 5 * qField[i][0][iVar] + 15 * qField[i][1][iVar]
											 - 5 * qField[i][2][iVar] +      qField[i][3][iVar]);

			qField1[i][1][iVar] = 1.0 / 16 * (-    qField[i][0][iVar] + 9 * qField[i][1][iVar]
											 + 9 * qField[i][2][iVar] -     qField[i][3][iVar]);

			qField1[i][jed    ][iVar] = 1.0 / 16 * (-    qField[i][jed + 2][iVar] + 9 * qField[i][jed + 1][iVar]
											       + 9 * qField[i][jed    ][iVar] -     qField[i][jed - 1][iVar]);

			qField1[i][jed + 1][iVar] = 1.0 / 16 * ( 5 * qField[i][jed + 2][iVar] + 15 * qField[i][jed + 1][iVar]
											       - 5 * qField[i][jed    ][iVar] +      qField[i][jed - 1][iVar]);

			qField2[i][0][iVar] = qField1[i][0][iVar];
			qField2[i][1][iVar] = qField1[i][1][iVar];

			qField2[i][jed    ][iVar] = qField1[i][jed    ][iVar];
			qField2[i][jed + 1][iVar] = qField1[i][jed + 1][iVar];
		}
	}
}

void QlQr_Solver::Boundary_QlQr_WCNS_X_5th()
{
	for (int j = jst; j <= jed; j++)//ȷ��i=0,1,ied,ied+1��ֵ
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[0][j][iVar] = 1.0 / 128 * (315 * qField[1][j][iVar] - 420 * qField[2][j][iVar]
											 + 378 * qField[3][j][iVar] - 180 * qField[4][j][iVar] 
											 + 35  * qField[5][j][iVar]);

			qField1[1][j][iVar] = 1.0 / 128 * (35  * qField[1][j][iVar] + 140 * qField[2][j][iVar]
											 - 70  * qField[3][j][iVar] + 28  * qField[4][j][iVar] 
											 - 5   * qField[5][j][iVar]);

			qField1[ied + 1][j][iVar] = 1.0 / 128 * (315 * qField[ied + 1][j][iVar] - 420 * qField[ied    ][j][iVar]
											       + 378 * qField[ied - 1][j][iVar] - 180 * qField[ied - 2][j][iVar] 
											       + 35  * qField[ied - 3][j][iVar]);

			qField1[ied    ][j][iVar] = 1.0 / 128 * (35 * qField[ied + 1][j][iVar] + 140 * qField[ied    ][j][iVar]
											       - 70 * qField[ied - 1][j][iVar] + 28  * qField[ied - 2][j][iVar] 
											       - 5  * qField[ied - 3][j][iVar]);

			qField2[0][j][iVar] = qField1[0][j][iVar];
			qField2[1][j][iVar] = qField1[1][j][iVar];

			qField2[ied    ][j][iVar] = qField1[ied    ][j][iVar];
			qField2[ied + 1][j][iVar] = qField1[ied + 1][j][iVar];
		}
	}
}

void QlQr_Solver::Boundary_QlQr_WCNS_Y_5th()
{
	for (int i = ist; i <= ied; i++)//ȷ��j=0,1,jed,jed+1��ֵ
	{
		for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
		{
			qField1[i][0][iVar] = 1.0 / 128 * (315 * qField[i][1][iVar] - 420 * qField[i][2][iVar]
											 + 378 * qField[i][3][iVar] - 180 * qField[i][4][iVar] 
											 + 35  * qField[i][5][iVar]);

			qField1[i][1][iVar] = 1.0 / 128 * (35  * qField[i][1][iVar] + 140 * qField[i][2][iVar]
											 - 70  * qField[i][3][iVar] + 28  * qField[i][4][iVar] 
											 - 5   * qField[i][5][iVar]);

			qField1[i][jed + 1][iVar] = 1.0 / 128 * (315 * qField[i][jed + 1][iVar] - 420 * qField[i][jed    ][iVar]
											       + 378 * qField[i][jed - 1][iVar] - 180 * qField[i][jed - 2][iVar] 
											       + 35  * qField[i][jed - 3][iVar]);

			qField1[i][jed    ][iVar] = 1.0 / 128 * (35 * qField[i][jed + 1][iVar] + 140 * qField[i][jed    ][iVar]
											       - 70 * qField[i][jed - 1][iVar] + 28  * qField[i][jed - 2][iVar] 
											       - 5  * qField[i][jed - 3][iVar]);

			qField2[i][0][iVar] = qField1[i][0][iVar];
			qField2[i][1][iVar] = qField1[i][1][iVar];

			qField2[i][jed    ][iVar] = qField1[i][jed    ][iVar];
			qField2[i][jed + 1][iVar] = qField1[i][jed + 1][iVar];
		}
	}
}

void QlQr_Solver::QlQr_WCNS_X()
{
	//����Lagrange��ֵϵ��
	VDouble3D g1, g2, g3;
	Allocate_3D_Vector(g1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(g2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(g3, total_points_x, total_points_y, num_of_prim_vars);
														
	VDouble3D s1, s2, s3;								
	Allocate_3D_Vector(s1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(s2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(s3, total_points_x, total_points_y, num_of_prim_vars);

	double ds = dx;

	VInt2D& marker = mesh->Get_Marker_Q();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)//i=0,1,ied+1,ied+2û��ֵ
	{
		for (int j = jst; j <= jed; j++)
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				g1[i][j][iVar] = (		 qField[i - 2][j][iVar] - 4.0 * qField[i - 1][j][iVar] + 3.0 *	qField[i    ][j][iVar]) / 2.0 / ds;
				g2[i][j][iVar] = (		 qField[i + 1][j][iVar] -		qField[i - 1][j][iVar]								  ) / 2.0 / ds;
				g3[i][j][iVar] = (-3.0 * qField[i    ][j][iVar] + 4.0 * qField[i + 1][j][iVar] -		qField[i + 2][j][iVar]) / 2.0 / ds;

				s1[i][j][iVar] = (		 qField[i - 2][j][iVar] - 2.0 * qField[i - 1][j][iVar] +		qField[i    ][j][iVar]) / ds / ds;
				s2[i][j][iVar] = (		 qField[i - 1][j][iVar] - 2.0 * qField[i    ][j][iVar] +		qField[i + 1][j][iVar]) / ds / ds;
				s3[i][j][iVar] = (		 qField[i    ][j][iVar] - 2.0 * qField[i + 1][j][iVar] +		qField[i + 2][j][iVar]) / ds / ds;
			}
		}
	}
	//����⻬����
	VDouble3D IS1, IS2, IS3;
	Allocate_3D_Vector(IS1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(IS2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(IS3, total_points_x, total_points_y, num_of_prim_vars);
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)//i=0,1,ied+1,ied+2û��ֵ
	{
		for (int j = jst; j <= jed; j++)
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				IS1[i][j][iVar] = pow((ds * g1[i][j][iVar]), 2) + pow((ds * ds * s1[i][j][iVar]), 2);
				IS2[i][j][iVar] = pow((ds * g2[i][j][iVar]), 2) + pow((ds * ds * s2[i][j][iVar]), 2);
				IS3[i][j][iVar] = pow((ds * g3[i][j][iVar]), 2) + pow((ds * ds * s3[i][j][iVar]), 2);
			}
		}
	}

	double C11 = 1.0 / 16.0, C21 = 10.0 / 16.0, C31 = 5.0 / 16.0;
	double C12 = 5.0 / 16.0, C22 = 10.0 / 16.0, C32 = 1.0 / 16.0;
	double eps = 1e-6;

	//j+1/2���ı�������ֵ
#ifdef _OPENMP
#pragma omp parallel for
#endif									//g,s,IS��i=0,1,ied+1,ied+2û��ֵ
	for (int i = ist; i <= ied - 1; i++)//���ڳ��㿪ʼ��������ql��qr�ں����ñ߽��ʽ����
	{
		for (int j = jst; j <= jed; j++)
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				//j+1/2������ֵ��������3��ģ���ֵ�õ�3��ֵ
				double qField_Left1 = qField[i][j][iVar] + ds * g1[i][j][iVar] / 2.0 + ds * ds * s1[i][j][iVar] / 8.0;
				double qField_Left2 = qField[i][j][iVar] + ds * g2[i][j][iVar] / 2.0 + ds * ds * s2[i][j][iVar] / 8.0;
				double qField_Left3 = qField[i][j][iVar] + ds * g3[i][j][iVar] / 2.0 + ds * ds * s3[i][j][iVar] / 8.0;

				//ȡ�����Լ�Ȩ������ֵ�ֱ�ȡ��ͬ��Ȩֵ
				double a1 = C11 / pow((eps + IS1[i][j][iVar]), 2);
				double a2 = C21 / pow((eps + IS2[i][j][iVar]), 2);
				double a3 = C31 / pow((eps + IS3[i][j][iVar]), 2);

				double w1 = a1 / (a1 + a2 + a3);
				double w2 = a2 / (a1 + a2 + a3);
				double w3 = a3 / (a1 + a2 + a3);

				qField1[i][j][iVar] = w1 * qField_Left1 + w2 * qField_Left2 + w3 * qField_Left3;
				
				//j+1/2������ֵ��������3��ģ���ֵ�õ�3��ֵ
				double qField_Right1 = qField[i + 1][j][iVar] - ds * g1[i + 1][j][iVar] / 2.0 + ds * ds * s1[i + 1][j][iVar] / 8.0;
				double qField_Right2 = qField[i + 1][j][iVar] - ds * g2[i + 1][j][iVar] / 2.0 + ds * ds * s2[i + 1][j][iVar] / 8.0;
				double qField_Right3 = qField[i + 1][j][iVar] - ds * g3[i + 1][j][iVar] / 2.0 + ds * ds * s3[i + 1][j][iVar] / 8.0;

				//ȡ�����Լ�Ȩ������ֵ�ֱ�ȡ��ͬ��Ȩֵ
				a1 = C12 / pow((eps + IS1[i + 1][j][iVar]), 2);
				a2 = C22 / pow((eps + IS2[i + 1][j][iVar]), 2);
				a3 = C32 / pow((eps + IS3[i + 1][j][iVar]), 2);

				w1 = a1 / (a1 + a2 + a3);
				w2 = a2 / (a1 + a2 + a3);
				w3 = a3 / (a1 + a2 + a3);
				qField2[i][j][iVar] = w1 * qField_Right1 + w2 * qField_Right2 + w3 * qField_Right3;
			}
		}
	}
}

void QlQr_Solver::QlQr_WCNS_Y()
{
	//����Lagrange��ֵϵ��
	VDouble3D g1, g2, g3;
	Allocate_3D_Vector(g1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(g2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(g3, total_points_x, total_points_y, num_of_prim_vars);

	VDouble3D s1, s2, s3;
	Allocate_3D_Vector(s1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(s2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(s3, total_points_x, total_points_y, num_of_prim_vars);

	double ds = dy;

	VInt2D& marker = mesh->Get_Marker_Q();
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)
	{
		for (int j = jst; j <= jed; j++)//j=0,1,jed+1,jed+2û��ֵ
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				g1[i][j][iVar] = (		 qField[i][j - 2][iVar] - 4.0 * qField[i][j - 1][iVar] + 3.0 *	qField[i][j    ][iVar]) / 2.0 / ds;
				g2[i][j][iVar] = (		 qField[i][j + 1][iVar] -		qField[i][j - 1][iVar]								  ) / 2.0 / ds;
				g3[i][j][iVar] = (-3.0 * qField[i][j    ][iVar] + 4.0 * qField[i][j + 1][iVar] -		qField[i][j + 2][iVar]) / 2.0 / ds;

				s1[i][j][iVar] = (		 qField[i][j - 2][iVar] - 2.0 * qField[i][j - 1][iVar] +		qField[i][j    ][iVar]) / ds / ds;
				s2[i][j][iVar] = (		 qField[i][j - 1][iVar] - 2.0 * qField[i][j    ][iVar] +		qField[i][j + 1][iVar]) / ds / ds;
				s3[i][j][iVar] = (		 qField[i][j    ][iVar] - 2.0 * qField[i][j + 1][iVar] +		qField[i][j + 2][iVar]) / ds / ds;
			}
		}
	}
	//����⻬����
	VDouble3D IS1, IS2, IS3;
	Allocate_3D_Vector(IS1, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(IS2, total_points_x, total_points_y, num_of_prim_vars);
	Allocate_3D_Vector(IS3, total_points_x, total_points_y, num_of_prim_vars);
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)
	{
		for (int j = jst; j <= jed; j++)//j=0,1,jed+1,jed+2û��ֵ
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				IS1[i][j][iVar] = pow((ds * g1[i][j][iVar]), 2) + pow((ds * ds * s1[i][j][iVar]), 2);
				IS2[i][j][iVar] = pow((ds * g2[i][j][iVar]), 2) + pow((ds * ds * s2[i][j][iVar]), 2);
				IS3[i][j][iVar] = pow((ds * g3[i][j][iVar]), 2) + pow((ds * ds * s3[i][j][iVar]), 2);
			}
		}
	}

	double C11 = 1.0 / 16.0, C21 = 10.0 / 16.0, C31 = 5.0 / 16.0;
	double C12 = 5.0 / 16.0, C22 = 10.0 / 16.0, C32 = 1.0 / 16.0;
	double eps = 1e-6;

	//j+1/2���ı�������ֵ��ͨ��
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = ist; i <= ied; i++)
	{
		for (int j = jst; j <= jed - 1; j++)//��ڵ�j=0,1,jed,jed+1û��ֵ,�����ɱ߽��ʽ����
		{
			if (marker[i][j] == 0) continue;
			for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
			{
				//j+1/2������ֵ��������3��ģ���ֵ�õ�3��ֵ
				double qField_Left1 = qField[i][j][iVar] + ds * g1[i][j][iVar] / 2.0 + ds * ds * s1[i][j][iVar] / 8.0;
				double qField_Left2 = qField[i][j][iVar] + ds * g2[i][j][iVar] / 2.0 + ds * ds * s2[i][j][iVar] / 8.0;
				double qField_Left3 = qField[i][j][iVar] + ds * g3[i][j][iVar] / 2.0 + ds * ds * s3[i][j][iVar] / 8.0;

				//ȡ�����Լ�Ȩ������ֵ�ֱ�ȡ��ͬ��Ȩֵ
				double a1 = C11 / pow((eps + IS1[i][j][iVar]), 2);
				double a2 = C21 / pow((eps + IS2[i][j][iVar]), 2);
				double a3 = C31 / pow((eps + IS3[i][j][iVar]), 2);

				double w1 = a1 / (a1 + a2 + a3);
				double w2 = a2 / (a1 + a2 + a3);
				double w3 = a3 / (a1 + a2 + a3);

				qField1[i][j][iVar] = w1 * qField_Left1 + w2 * qField_Left2 + w3 * qField_Left3;
				
				//j+1/2������ֵ��������3��ģ���ֵ�õ�3��ֵ
				double qField_Right1 = qField[i][j + 1][iVar] - ds * g1[i][j + 1][iVar] / 2.0 + ds * ds * s1[i][j + 1][iVar] / 8.0;
				double qField_Right2 = qField[i][j + 1][iVar] - ds * g2[i][j + 1][iVar] / 2.0 + ds * ds * s2[i][j + 1][iVar] / 8.0;
				double qField_Right3 = qField[i][j + 1][iVar] - ds * g3[i][j + 1][iVar] / 2.0 + ds * ds * s3[i][j + 1][iVar] / 8.0;

				//ȡ�����Լ�Ȩ������ֵ�ֱ�ȡ��ͬ��Ȩֵ
				a1 = C12 / pow((eps + IS1[i][j + 1][iVar]), 2);
				a2 = C22 / pow((eps + IS2[i][j + 1][iVar]), 2);
				a3 = C32 / pow((eps + IS3[i][j + 1][iVar]), 2);

				w1 = a1 / (a1 + a2 + a3);
				w2 = a2 / (a1 + a2 + a3);
				w3 = a3 / (a1 + a2 + a3);
				qField2[i][j][iVar] = w1 * qField_Right1 + w2 * qField_Right2 + w3 * qField_Right3;
			}
		}
	}
}

double QlQr_Solver::Limiter_Function( double ita )
{
	if (method_of_limiter == 0)			// nolim
	{
		return 1.0;
	}
	else if (method_of_limiter == 1)	//vanleer
	{
		return vanleer_limiter(ita, 1.0);
	}
	else if (method_of_limiter == 2)	//minmod
	{
		return minmod_limiter(ita, 1.0);
	}
	else if (method_of_limiter == 3)	//superbee
	{
		return superbee_limiter(ita, 1.0);
	}
	else if (method_of_limiter == 4)	//1st-order
	{
		return 0.0;
	}
}

//����������
double minmod_limiter(double a, double b)
{
	if (a * b <= 0)
	{
		return 0.0;
	}	
	else
	{
		if ((fabs(a) - fabs(b)) > 0)
		{
			return b;
		}	
		else
		{
			return a;
		}		
	}
}

double vanleer_limiter(double a, double)
{
	return (a + fabs(a)) / (1.0 + fabs(a));
}

double superbee_limiter(double a, double)
{
	double tmp1 = min(2.0 * a, 1.0);
	double tmp2 = min(a, 2.0);

	return max(0.0, max(tmp1, tmp2));
}
