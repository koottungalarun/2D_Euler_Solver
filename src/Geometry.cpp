//////////////////////////////////////////////////////////
//														//
//			2D Euler Solver on Structured Grid						//
//////////////////////////////////////////////////////////
#include "Geometry.h"
#include "Global.h"
#include "2D_Euler_Solver.h"

Structured_Mesh* mesh;
int num_ghost_point;
int num_grid_point_x, num_grid_point_y;
int total_points_x,   total_points_y;
int num_half_point_x, num_half_point_y;
int Iw, Jw1, Jw2;
double dx, dy;

void Generate_Mesh()
{
	if (global_case_id == 1)
	{
		Generate_Mesh_Double_Mach();
	}
	else if (global_case_id == 2)
	{
		Generate_Mesh_Blunt_Body();
	}
}

void Generate_Mesh_Double_Mach()
{
	double hx = 3.0, hy = 1.0;

	num_ghost_point = 2;
	total_points_x = num_grid_point_x + 2 * num_ghost_point;
	total_points_y = num_grid_point_y + 2 * num_ghost_point;

	num_half_point_x = total_points_x - 1;   //������������Ԫ����������1
	num_half_point_y = total_points_y - 1;
	
	dx = hx / (num_grid_point_x - 1);
	dy = hy / (num_grid_point_y - 1);

	mesh = new Structured_Mesh(total_points_x, total_points_y);
	vector< vector< Point > >& grid_points = mesh->Get_Grid_Points();
	VInt2D& marker_q = mesh->Get_Marker_Q();
	VInt2D& marker_f = mesh->Get_Marker_F();
	VInt2D& iBlank = mesh->Get_IBlank();

	mesh->Set_Mesh_Dimension (num_grid_point_x, num_grid_point_y);
	mesh->Set_Num_Ghost_Point(num_ghost_point);
	mesh->Set_Dxdy(dx, dy);

	int ist, ied, jst, jed;
	Get_IJK_Region(ist, ied, jst, jed);
	mesh->Set_Range(ist, ied, jst, jed);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i <= ied + 2; i++)
	{
		for (int j = 0; j <= jed + 2; j++)
		{
			double x_node = (i - ist) * dx;
			double y_node = (j - jst) * dy;

			grid_points[i][j].Set_Point_Coord(x_node, y_node);

			marker_q[i][j] = 1;
			marker_f[i][j] = 1;
			iBlank  [i][j] = 1;
		}
	}
}

void Generate_Mesh_Blunt_Body()
{
	double hx  = 4.0, hy  = 2.0;
	double hx1 = 0.6, hy1 = 0.8, hy2 = 1.2;
	Iw  = hx1 / hx * (num_grid_point_x - 1); //������߽�ı��I����ʼ���Ϊ0�������������
	Jw1 = hy1 / hy * (num_grid_point_y - 1); //�����±߽�ı��J
	Jw2 = hy2 / hy * (num_grid_point_y - 1); //�����ϱ߽�ı��J

	num_ghost_point = 2;
	total_points_x = num_grid_point_x + 2 * num_ghost_point;
	total_points_y = num_grid_point_y + 2 * num_ghost_point;

	num_half_point_x = total_points_x - 1;   //������������Ԫ����������1
	num_half_point_y = total_points_y - 1;
	
	dx = hx / (num_grid_point_x - 1);
	dy = hy / (num_grid_point_y - 1);

	mesh = new Structured_Mesh(total_points_x, total_points_y);
	vector< vector< Point > >& grid_points = mesh->Get_Grid_Points();
	VInt2D& marker_q = mesh->Get_Marker_Q();
	VInt2D& marker_f = mesh->Get_Marker_F();
	VInt2D& iBlank = mesh->Get_IBlank();

	mesh->Set_Mesh_Dimension (num_grid_point_x, num_grid_point_y);
	mesh->Set_Num_Ghost_Point(num_ghost_point);
	mesh->Set_Dxdy(dx, dy);

	int ist, ied, jst, jed;
	Get_IJK_Region(ist, ied, jst, jed);
	mesh->Set_Range(ist, ied, jst, jed);

#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i <= ied + 2; i++)
	{
		for (int j = 0; j <= jed + 2; j++)
		{
			double x_node = (i - ist) * dx;
			double y_node = (j - jst) * dy;

			grid_points[i][j].Set_Point_Coord(x_node, y_node);

			marker_q[i][j] = 1;
			marker_f[i][j] = 1;
			iBlank  [i][j] = 1;
			
			if (i > ist + Iw && j > jst + Jw1 && j < jst + Jw2)
			{
				iBlank[i][j] = 0;		//�����Щ��������������
			}

			if (i > ist + Iw + 2 && j > jst + Jw1 && j < jst + Jw2)
			{
				marker_q[i][j] = 0;		//�����Щ�����������������棬���μӼ���
			}

			if (i > ist + Iw + 1 && j > jst + Jw1 && j < jst + Jw2)
			{
				marker_f[i][j] = 0;		//�����Щͨ���������������棬���μӼ���
			}
		}
	}
}

void Structured_Mesh::Set_Marker_Value()
{
	for (int i = 0; i <= ied + 2; i++)
	{
		for (int j = 0; j <= jed + 2; j++)
		{
			marker_q[i][j] = 1;
			marker_f[i][j] = 1;

			if (i > ist + Iw && j > jst + Jw1 + 2 && j < jst + Jw2 - 2)
			{
				marker_q[i][j] = 0;		//�����Щ�����������������棬���μӼ���
			}

			if (i > ist + Iw && j > jst + Jw1 + 1 && j < jst + Jw2 - 2)
			{
				marker_f[i][j] = 0;		//�����Щͨ���������������棬���μӼ���
			}
		}
	}
}

void Set_Mesh_Dimension(int NI, int NJ)
{
	num_grid_point_x = NI; num_grid_point_y = NJ;
}

Structured_Mesh::Structured_Mesh(int NI_tot, int NJ_tot)
{
	this->dx = 0.0;
	this->dy = 0.0;
	this->NI = 0;
	this->NJ = 0;
	this->NI_tot = NI_tot;
	this->NJ_tot = NJ_tot;

	this->ist = 0;
	this->ied = 0;
	this->jst = 0;
	this->jed = 0;
	this->num_ghost_point = 0;

	Allocate_2D_Vector(grid_points, NI_tot, NJ_tot);
	Allocate_2D_Vector(marker_q,    NI_tot, NJ_tot);
	Allocate_2D_Vector(marker_f,    NI_tot, NJ_tot);
	Allocate_2D_Vector(iBlank,      NI_tot, NJ_tot);
}

void Structured_Mesh::Set_Range(int ist, int ied, int jst, int jed)
{
	this->ist = ist;
	this->ied = ied;
	this->jst = jst;
	this->jed = jed;
}

Point::Point()
{
	xPoint = 0.0;
	yPoint = 0.0;
}
