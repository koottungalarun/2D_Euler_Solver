//////////////////////////////////////////////////////////
//														//
//			2D Euler Solver on Structured Grid			//
			//
//////////////////////////////////////////////////////////
#include <fstream>
#include <iostream>
#include <cmath>
#include "Post_Process.h"
#include "Global.h"
#include "QlQr_Solver.h"
#include "Geometry.h"
#include <iomanip>
#include "Spatial_Derivative.h"

bool stop_by_residual = 0;
void Compute_Residual()
{
	Residual* residual_solver = new Residual();

	residual_solver->Compute_Residual();

	delete residual_solver;
}

Residual::Residual()
{
	res_L1.resize (num_of_prim_vars);
	res_L2.resize (num_of_prim_vars);
	res_Loo.resize(num_of_prim_vars);
	Allocate_2D_Vector(max_index, num_of_prim_vars, 2);

	Get_IJK_Region(ist, ied, jst, jed);
}

void Residual::Compute_Residual()
{
	VInt2D& marker = mesh->Get_Marker_Q();
	for (int iVar = 0; iVar < num_of_prim_vars; iVar++)
	{
		int count = 0;
		double res_max = -1e40;
		
		for (int i = ist; i <= ied; i++)
		{
			for (int j = jst; j <= jed; j++)
			{
				if (marker[i][j] == 0) continue;
				
				double res1 = fabs( qField_N1[i][j][iVar] - qField[i][j][iVar]);
				//double res1 = fabs(rhs[i][j][iVar]);
				double res2 = res1 * res1;

				if (res1 > res_max)
				{
					res_max = res1;
					max_index[iVar][0] = i;
					max_index[iVar][1] = j;
				}
				//res_max = max(res1, res_max);

				res_L1 [iVar] += res1;
				res_L2 [iVar] += res2;
				res_Loo[iVar] = res_max;

				count++;
			}
		}
		res_L1[iVar] /= count;
		res_L2[iVar] = sqrt(res_L2[iVar] / count);
	}
	
	this->OutputResidual();
}

void Residual::OutputResidual()
{
	bool flag1 = current_step % residual_output_steps;//flag1=0,����ʱ����вflag1=1��������ʱ�����
	bool flag2 = current_step == 1;					  //flag2=0,�м䲽�������  flag2=1���ײ����
	if (flag1 && flag2 == 0) return;

	if (flag2)
	{
		cout << "Iteration\trho_res_L2\tu_res_L2\tv_res_L2\tp_res_L2" << endl;
	}
	cout << setiosflags(ios::left);
	cout << setiosflags(ios::scientific);
	cout << setprecision(5);
	cout << current_step	<< "\t        "
		 << res_L2[IR]		<< "\t" << res_L2[IU] << "\t"
		 << res_L2[IV]		<< "\t" << res_L2[IP] << endl;

	if (res_Loo[IR] < converge_criterion &&
		res_Loo[IU] < converge_criterion &&
		res_Loo[IV] < converge_criterion &&
		res_Loo[IP] < converge_criterion   )
	{
		stop_by_residual = 1;
	}
}

bool Residual::Stop_by_Residual()
{
	return false;
}

bool Stop_by_Residual()
{
	return false;
}

void Output_Flowfield()
{
	bool flag0 = current_step % flow_save_steps;//flag0=0,����ʱ�������		  flag0=1��������ʱ�������
	bool flag1 = Need_Stop_Iteration();			//flag1=1,�˳���������Ҫ���������flag1=0,�м䲽�������
	if (flag0 && flag1==0) return;

	nowTime = Get_Current_Time();
	cout << "dumping flowfield..."  << "\tIter = "   << current_step 
		 << "\tphysical_time = "    << physical_time
		 << "\ttime_elapsed  = "	<< static_cast<double>(nowTime - lastTime) / GetClockTicksPerSecond()
		 << " seconds"  << endl << endl;
	//lastTime = nowTime;

	if (flag1 == 0) //flag1=0,�м䲽�����̧ͷ
	{
		cout << "Iteration\trho_res_Loo\tu_res_Loo\tv_res_Loo\tp_res_Loo" << endl;
	}
	
	fstream file;
	file.open(tec_file_name, ios_base::out);

	file << "VARIABLES = \"x\", \"y\", \"rho\", \"u\", \"v\", \"p\", \"m\", \"iblank\" " << endl;
	file << "ZONE T = \"Zone 1\"" << endl;
	file << "I = " << num_grid_point_x << "  J = " << num_grid_point_y << "  K =1" << "  ZONETYPE=Ordered" << endl;
	file << "DATAPACKING=POINT"   << endl;

	file << setiosflags(ios::right);
	file << setiosflags(ios::scientific);
	file << setprecision(8);

	int ist, ied, jst, jed;
	Get_IJK_Region(ist, ied, jst, jed);

	double gama = 1.4;
	vector< vector< Point > >& grid_points = mesh->Get_Grid_Points();
	VInt2D& marker = mesh->Get_IBlank();
	for (int j = jst; j <= jed; j++)
	{
		for (int i = ist; i <= ied; i++)
		{
			double xcoord, ycoord;
			grid_points[i][j].Get_Point_Coord(xcoord, ycoord);

			double rho = qField_N1[i][j][IR] + SMALL;
			double u   = qField_N1[i][j][IU];
			double v   = qField_N1[i][j][IV];
			double p   = qField_N1[i][j][IP];
			double a   = sqrt(gama * p / rho) + SMALL;
			double m   = sqrt(u * u + v * v) / a;

			file << xcoord << "    " << ycoord << "    " << rho << "    " << u << "    " 
				 << v      << "    " << p      << "    " << m   << "    " << marker[i][j] << endl;
		}
	}
	
	file.close();
}

void Post_Solve()
{
	Compute_Residual();

	Output_Flowfield();
}
