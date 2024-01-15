//////////////////////////////////////////////////////////
//														//
//			2D Euler Solver on Structured Grid						//
//////////////////////////////////////////////////////////
#include "2D_Euler_Solver.h"
#include "Spatial_Derivative.h"
#include "Geometry.h"
#include "Global.h"
#include "Flux_Solver.h"
#include "QlQr_Solver.h"
#include "Time_Integral.h"
#include <cmath>
#include <iostream>

void Time_Integration()
{
	Time_Marching_Solver* time_marching = new Time_Marching_Solver();
	time_marching->Time_Marching();
	delete time_marching;
}

Time_Marching_Solver::Time_Marching_Solver()
{

}

void Time_Marching_Solver::Time_Marching()
{
	for (int iStage = 0; iStage < num_of_RK_stages; iStage++)
	{
		Load_Q();

		Compute_Boundary();

		Solve_QlQr();

		Solve_Flux();

		Solve_Spatial_Derivative();

		Update_Flowfield(iStage);
	}	
}

void Update_Flowfield(int iStage)
{
	if (solve_direction == 'x')
	{
		Update_Flowfield_X(iStage);
	}
	else if (solve_direction == 'y')
	{
		Update_Flowfield_Y(iStage);
	}
}

void Update_Flowfield_X(int iStage)
{
	double RK_Coeff_a = RK_Coeff[iStage][0];
	double RK_Coeff_b = RK_Coeff[iStage][1];
	double RK_Coeff_c = RK_Coeff[iStage][2];

	VInt2D& marker = mesh->Get_Marker_Q();
	int ist, ied, jst, jed;
	Get_IJK_Region(ist, ied, jst, jed);

	double minimumPressureLimit = 1.0e-6 * 116.5;
	double minimumDensityLimit = 1.0e-6 * 8;

#ifndef _OPENMP
	VDouble rhsVector(num_of_prim_vars);
	VDouble qPrimitive0(num_of_prim_vars);
	VDouble qPrimitive1(num_of_prim_vars);
	VDouble qConservative0(num_of_prim_vars);
	VDouble qConservative1(num_of_prim_vars);
#endif // !_OPENMP

#ifdef _OPENMP
#pragma omp parallel
	{
		VDouble rhsVector(num_of_prim_vars);
		VDouble qPrimitive0(num_of_prim_vars);
		VDouble qPrimitive1(num_of_prim_vars);
		VDouble qConservative0(num_of_prim_vars);
		VDouble qConservative1(num_of_prim_vars);
#pragma omp  for
#endif	
		for (int i = 1; i <= ied + 1; i++)
		//for (int i = ist; i <= ied; i++)
		{
			for (int j = jst; j <= jed; j++)
			{
				if (marker[i][j] == 0) continue;

				rhsVector = rhs[i][j];			//rhs(q0),����һ����q����õ���rhs

				qPrimitive0 = qField_N0[i][j];	//RK��ʽ���һ��,q0
				Primitive_To_Conservative(qPrimitive0, qConservative0);

				qPrimitive1 = qField_N1[i][j];	//RK��ʽ��ڶ��q0��q1��q2��Ҳ����һstage��qֵ
				Primitive_To_Conservative(qPrimitive1, qConservative1);

				for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
				{
					qConservative1[iVar] = RK_Coeff_a * qConservative0[iVar] + RK_Coeff_b * qConservative1[iVar] + RK_Coeff_c * time_step * rhsVector[iVar];
				}

				Conservative_To_Primitive(qConservative1, qPrimitive1);

				if (qPrimitive1[IR] < minimumDensityLimit || qPrimitive1[IP] < minimumPressureLimit)
				{
					int kkk = 1;
					SolutionFix(qPrimitive1, i, j);
				}
				//RK��ʽ������q1��q2��q3������һstage��qֵ����Ҫ�����ø�ֵ����rhs(q1)��rhs(q2)
				qField_N1[i][j] = qPrimitive1;
			}
		}
#ifdef _OPENMP
	}
#endif // _OPENMP

}

void Update_Flowfield_Y(int iStage)
{
	double RK_Coeff_a = RK_Coeff[iStage][0];
	double RK_Coeff_b = RK_Coeff[iStage][1];
	double RK_Coeff_c = RK_Coeff[iStage][2];

	VInt2D& marker = mesh->Get_Marker_Q();
	int ist, ied, jst, jed;
	Get_IJK_Region(ist, ied, jst, jed);

	double minimumPressureLimit = 1.0e-6 * 116.5;
	double minimumDensityLimit = 1.0e-6 * 8;

#ifndef _OPENMP
	VDouble rhsVector(num_of_prim_vars);
	VDouble qPrimitive0(num_of_prim_vars);
	VDouble qPrimitive1(num_of_prim_vars);
	VDouble qConservative0(num_of_prim_vars);
	VDouble qConservative1(num_of_prim_vars);
#endif // !_OPENMP

#ifdef _OPENMP
#pragma omp parallel
	{
		VDouble rhsVector(num_of_prim_vars);
		VDouble qPrimitive0(num_of_prim_vars);
		VDouble qPrimitive1(num_of_prim_vars);
		VDouble qConservative0(num_of_prim_vars);
		VDouble qConservative1(num_of_prim_vars);
#pragma omp  for
#endif	
		for (int i = ist; i <= ied; i++)
		{
			for (int j = 1; j <= jed + 1; j++)
			//for (int j = jst; j <= jed; j++)
			{
				if (marker[i][j] == 0) continue;

				rhsVector = rhs[i][j];			//rhs(q0),����һ����q����õ���rhs

				qPrimitive0 = qField_N0[i][j];	//RK��ʽ���һ��,q0
				Primitive_To_Conservative(qPrimitive0, qConservative0);

				qPrimitive1 = qField_N1[i][j];	//RK��ʽ��ڶ��q0��q1��q2��Ҳ����һstage��qֵ
				Primitive_To_Conservative(qPrimitive1, qConservative1);

				for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
				{
					qConservative1[iVar] = RK_Coeff_a * qConservative0[iVar] + RK_Coeff_b * qConservative1[iVar] + RK_Coeff_c * time_step * rhsVector[iVar];
				}

				Conservative_To_Primitive(qConservative1, qPrimitive1);

				if (qPrimitive1[IR] < minimumDensityLimit || qPrimitive1[IP] < minimumPressureLimit)
				{
					int kkk = 1;
					SolutionFix(qPrimitive1, i, j);
				}
				//RK��ʽ������q1��q2��q3������һstage��qֵ����Ҫ�����ø�ֵ����rhs(q1)��rhs(q2)
				qField_N1[i][j] = qPrimitive1;
			}
		}
#ifdef _OPENMP
	}
#endif // _OPENMP
}

void Set_Field()
{
	//�ಽRK�ƽ����֮����������������qField_N1�У�Ҫ���·���qField���Ա���һ��ʱ�䲽����
	qField = qField_N1;
}

void SolutionFix(VDouble& primitiveVector, int iCell, int jCell)
{
	//cout << "ѹ��������i = " << iCell << " j = " << jCell << endl;

	for (int iEquation = 0; iEquation < num_of_prim_vars; ++iEquation)
	{
		primitiveVector[iEquation] = 0.0;
	}

	int iiStart = -1;
	int iiEnd = 1;
	int jjStart = -1;
	int jjEnd = 1;
	int kkStart = -1;
	int kkEnd = 1;

	kkStart = 0;
	kkEnd = 0;

	int numberOfPoints = 0;

	for (int ii = iiStart; ii <= iiEnd; ++ii)
	{
		for (int jj = jjStart; jj <= jjEnd; ++jj)
		{
			for (int kk = kkStart; kk <= kkEnd; ++kk)
			{
				if ((abs(ii) + abs(jj) + abs(kk)) == 1)
				{
					numberOfPoints += 1;
					for (int iEquation = 0; iEquation < num_of_prim_vars; ++iEquation)
					{
						double fValue = qField[iCell + ii][jCell + jj][iEquation];
						if (iEquation == IR || iEquation == IP)
						{
							fValue = fabs(fValue);
						}
						primitiveVector[iEquation] += fValue;
					}
				}
			}
		}
	}

	for (int iEquation = 0; iEquation < num_of_prim_vars; ++iEquation)
	{
		primitiveVector[iEquation] /= numberOfPoints;
	}
}
