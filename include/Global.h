#pragma once
#include<vector>
using namespace std;

#define IR 0
#define IU 1
#define IV 2
#define IP 3

extern int num_of_prim_vars;
extern int current_step, max_num_of_steps;
extern double cfl_num, time_step;
extern int method_of_half_q;
extern int method_of_limiter;
extern int method_of_flux;
extern double muscl_k;
extern double entropy_fix_coeff;
extern char solve_direction;
extern int residual_output_steps;
extern int flow_save_steps;
extern double converge_criterion;
extern bool stop_by_residual;

template < typename T >
void Allocate_2D_Vector(vector< vector< T > >& array, int dim1, int dim2)
{
	array.resize(dim1);
	for (int i = 0; i < dim1; i++)
	{
		array[i].resize(dim2);
	}
}

//C(m*n)=A(m*p)xB(p*n)
template < typename T1, typename T2, typename T3 >
void MatrixMultiply(vector< vector< T1 > >& a, vector< vector< T2 > >& b, vector< vector< T3 > >& c, int m, int p, int n)
{
	for (int i = 0; i < m; ++i)
	{
		for (int j = 0; j < n; ++j)
		{
			c[i][j] = 0;
			for (int k = 0; k < p; ++k)
			{
				c[i][j] += a[i][k] * b[k][j];
			}
		}
	}
}

//C(m*1)=A(m*n)xB(n*1)
template < typename T1, typename T2, typename T3 >
void MatrixMultiply(vector< vector< T1 > >& a, vector< T2 >& b, vector< T3 >& c, int m, int n)
{
	for (int i = 0; i < m; ++i)
	{
		c[i] = 0;
		for (int j = 0; j < n; ++j)
		{
			c[i] += a[i][j] * b[j];
		}
	}
}

double Energy_2_Pressure(double E, double rho, double u, double v);