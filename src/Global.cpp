//////////////////////////////////////////////////////////
//														//
//			2D Euler Solver on Structured Grid			//
			//
//////////////////////////////////////////////////////////
#include <iostream>
#include "2D_Euler_Solver.h"
#include "QlQr_Solver.h"
#include "Global.h"
#include "Geometry.h"
#include <fstream>

#ifndef _WIN32
#include <sys/time.h>
#include <unistd.h>
#include <sys/times.h>
#endif

#ifdef WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

int			num_of_prim_vars;
int			current_step, max_num_of_steps;
double		cfl_num, time_step, physical_time, max_simu_time;
int			method_of_half_q;
int			method_of_limiter;
int			method_of_flux;
double		muscl_k;
double		entropy_fix_coeff;
char		solve_direction;
int			residual_output_steps;
int			flow_save_steps;
double		converge_criterion;
string		tec_file_name;
int			num_of_RK_stages;
VDouble2D	RK_Coeff;
clock_t		lastTime, nowTime;
int			global_case_id;
int			grid_refine_coeff;

void Input_Parameters()
{
	global_case_id					= 2;					//1-˫��շ���, 2-ƽͷ����

	if (global_case_id == 1)
	{
		num_grid_point_x			= 1 * 240 + 1;
		num_grid_point_y			= 1 * 60 + 1;

		max_num_of_steps			= 2000;
		residual_output_steps		= 2;		//�в�����������
		flow_save_steps				= 50;		//��������������
		converge_criterion			= 1e-8;		//�в�������׼
		tec_file_name				= "./results/flow.plt";

		cfl_num						= 0.3;
		max_simu_time				= 0.2;

		method_of_half_q			= 1;		//1-MUSCL,	  2-WENO(����ֵ),   3-WCNS
		muscl_k						= 1.0/3;	//0.0-����ӭ��ƫ�ã�		    1/3-����ӭ��ƫ��
		method_of_limiter			= 1;		//0-nolim,    1-vanleer,        2-minmod,	  3-superbee,	4-1st
		method_of_flux				= 1;		//1-Roe,	  2-Steger,			3-VanLeer,    4-WENO,		5-WCNS 
		entropy_fix_coeff			= 0.01;		//Roe��ʽ������ϵ��epsilon
	}
	else if (global_case_id == 2)
	{
		num_grid_point_x = 4 * 100 + 1;
		num_grid_point_y = 4 * 50 + 1;

		max_num_of_steps			= 5000;
		residual_output_steps		= 2;		//�в�����������
		flow_save_steps				= 250;		//��������������
		converge_criterion			= 1e-8;		//�в�������׼
		tec_file_name				= "./results/flow.plt";

		cfl_num						= 0.4;
		max_simu_time				= 10;

		method_of_half_q			= 1;		//1-MUSCL,	  2-WENO(����ֵ),   3-WCNS
		muscl_k						= 1.0/3;	//0.0-����ӭ��ƫ�ã�		    1/3-����ӭ��ƫ��
		method_of_limiter			= 1;		//0-nolim,    1-vanleer,        2-minmod,	  3-superbee,	4-1st
		method_of_flux				= 2; 		//1-Roe,	  2-Steger,			3-VanLeer,    4-WENO,		5-WCNS 
		entropy_fix_coeff			= 0.1;		//Roe��ʽ������ϵ��epsilon
	}
}

void Init_Global_Param()
{
	lastTime = Get_Current_Time(); 
	nowTime  = lastTime;

	num_of_prim_vars	= 4;			//ԭʼ�������������Ʒ��̸���
	physical_time		= 0.0;
	time_step			= 0.0;			//ʱ�䲽��Ҫ�����������ֵȷ��������ֻ�ǳ�ʼ��
	solve_direction		= 'x';
	
	//num_of_RK_stages	= 3;
	//RK_Coeff			= { {1.0, 0.0, 1.0},{3.0 / 4.0, 1.0 / 4.0, 1.0 / 4.0},{1.0 / 3.0, 2.0 / 3.0, 2.0 / 3.0} };
	
	num_of_RK_stages	= 2;
	RK_Coeff			= { {1.0, 0.0, 1.0},{0.5, 0.5, 0.5} };
	
	//�������в���
	Input_Parameters();

	MakeDirectory("./results");

#ifndef _WIN32
	Read_Parameter_File("./input.txt");	//�ڼ�Ⱥ�ϼ���ʱ����ͨ�������ļ����ò���
#endif // !_WIN32	
}

void Load_Q()
{
	qField = qField_N1;
}

void Set_Solve_Direction(char direction)
{
	solve_direction = direction;
	if (direction=='x')
	{		
		qField_N0 = qField;		//RK��ʽ���һ���ǰʱ�䲽Q
		qField_N1 = qField;		//RK��ʽ��ڶ����һstage��Q
	}
	else if(direction == 'y')
	{
		if (global_case_id == 2)
		{
			mesh->Set_Marker_Value();
		}		

		//��ؼ��ĵ㣺y��������qField��x��������qField����ͬ�ģ����������ӷ��ѷ��Ĺؼ���
		//Ҳ����Ϊ��ˣ��ڼ���y����ʱ���������¼���߽�����
		qField	  = qField_N0;		//��qField��ԭΪ����x����֮ǰ��Qֵ��y��������ԭ����qField������rhs
		qField_N0 = qField_N1;		//RK��ʽ���һ���x������stage�ƽ�����������Qֵ
		qField_N1 = qField;			//qField��qField_N1��RK�ƽ��еĹؼ���������Ȼ��Ϊ����x����֮ǰ��Qֵ
	}
}

void Primitive_To_Conservative( VDouble & primitive, VDouble &conservative )
{
	double gama = 1.4;
	double rho, u, v, p;
	rho = primitive[IR];
	u = primitive[IU];
	v = primitive[IV];
	p = primitive[IP];

	conservative[IR] = rho;
	conservative[IU] = rho * u;
	conservative[IV] = rho * v;
	conservative[IP] = p / (gama - 1) + 0.5 * rho * (u * u + v * v);
}

void Conservative_To_Primitive(VDouble& conservative, VDouble &primitive)
{
	double gama = 1.4;
	double rho		= conservative[IR];
	double rho_u	= conservative[IU];
	double rho_v	= conservative[IV];
	double E		= conservative[IP];

	double u = rho_u / rho;
	double v = rho_v / rho;
	double p = (gama - 1) * (E - 0.5 * rho * (u * u + v * v));

	primitive[IR] = rho;
	primitive[IU] = u;
	primitive[IV] = v;
	primitive[IP] = p;
}

//�����ڳ���ı�ŷ�Χ
void Get_IJK_Region(int& ist, int& ied, int& jst, int& jed)
{
	ist = num_ghost_point;
	ied = num_ghost_point + num_grid_point_x - 1;//ist->ied,��Χ������ied

	jst = num_ghost_point;
	jed = num_ghost_point + num_grid_point_y - 1;//jst->jed,��Χ������jed
}

bool Need_Stop_Iteration()
{
	return stop_by_residual || physical_time >= max_simu_time || current_step == max_num_of_steps;
}

bool IsNaN(VDouble& data)
{
	bool flag = 0;
	for (int i = 0; i < data.size(); i++)
	{
		if (data[i] != data[i])
		{
			flag = 1;
			break;
		}
	}
	return flag;
}

clock_t Get_Current_Time()
{
#ifdef _WIN32
	return clock();
#else
	struct tms tp;
	times(&tp);
	return tp.tms_stime;	//system time
	//return tp.tms_utime;	//cpu time
#endif
}

double GetClockTicksPerSecond()
{
	long clockTicksPerSecond;

#ifdef _WIN32
	clockTicksPerSecond = CLOCKS_PER_SEC;
#else
	clockTicksPerSecond = sysconf(_SC_CLK_TCK);
#endif
	return clockTicksPerSecond;
}

void ExtractValue(VDouble primitiveVector, double& rm, double& um, double& vm, double& pm)
{
	rm = primitiveVector[IR];
	um = primitiveVector[IU];
	vm = primitiveVector[IV];
	pm = primitiveVector[IP];
}

void Read_Parameter_File(string fileName)
{
	fstream file;
	file.open(fileName, ios_base::in);

	file >> global_case_id;		//1-˫��շ���, 2-ƽͷ����
	file >> grid_refine_coeff;	//������ܲ�����ȡ1��2��4��8
	file >> max_num_of_steps;	//����������

	file >> method_of_half_q;	//��ڵ��ֵ����	//1-MUSCL,	  2-WENO(����ֵ),   3-WCNS
	file >> method_of_limiter;	//MUSCL����������	//0-nolim,    1-vanleer,        2-minmod,	  3-superbee,	4-1st
	file >> method_of_flux;		//ͨ�����㷽��		//1-Roe,	  2-Steger,			3-VanLeer,    4-WENO,		5-WCNS
	
	if (global_case_id == 1)
	{
		num_grid_point_x = grid_refine_coeff * 240 + 1;
		num_grid_point_y = grid_refine_coeff * 60 + 1;
	}
	else if (global_case_id == 2)
	{
		num_grid_point_x = grid_refine_coeff * 200 + 1;
		num_grid_point_y = grid_refine_coeff * 100 + 1;
	}

	file.clear();
	file.close();
}

void MakeDirectory(const string& directoryName)
{
#ifdef WIN32
	int flag = _mkdir(directoryName.c_str());
#else
	int flag = mkdir(directoryName.c_str(), S_IRWXU);
#endif
	if (flag == 0)
	{
		cout << directoryName << " directory has been created successfully !\n";
	}
}
