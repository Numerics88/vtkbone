#include "vtkboneStressStrainMatrix.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkboneStressStrainMatrix);


//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::PrintSelf(ostream& os, vtkIndent indent)
  {
  this->Superclass::PrintSelf(os,indent);
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetIsotropic(double E, double nu)
  {
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {this->StressStrainMatrix[i][j] = 0; }
  double c = E*(1.0-nu)/((1.0+nu)*(1.0-2.0*nu));
  double d = c*nu/(1.0-nu);
  double cG = c*(1.0-2.0*nu)/((2.0*(1.0-nu)));
  this->StressStrainMatrix[0][0] = c;
  this->StressStrainMatrix[0][1] = d;
  this->StressStrainMatrix[0][2] = d;
  this->StressStrainMatrix[1][0] = d;
  this->StressStrainMatrix[1][1] = c;
  this->StressStrainMatrix[1][2] = d;
  this->StressStrainMatrix[2][0] = d;
  this->StressStrainMatrix[2][1] = d;
  this->StressStrainMatrix[2][2] = c;
  this->StressStrainMatrix[3][3] = cG;
  this->StressStrainMatrix[4][4] = cG;
  this->StressStrainMatrix[5][5] = cG;
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetOrthotropic
  (
  double Exx,
  double Eyy,
  double Ezz,
  double nuyz,
  double nuzx,
  double nuxy,
  double Gyz,
  double Gzx,
  double Gxy
  )
  {
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {this->StressStrainMatrix[i][j] = 0; }
  double v32 = nuyz*Ezz/Eyy;
  double v13 = nuzx*Exx/Ezz;
  double v21 = nuxy*Eyy/Exx;
  double delta = (1.0-nuxy*v21-nuyz*v32-nuzx*v13-2*nuxy*nuyz*nuzx)/(Exx*Eyy*Ezz);
  this->StressStrainMatrix[0][0] = (1.0-nuyz*v32)/(Eyy*Ezz*delta);
  this->StressStrainMatrix[0][1] = (v21+nuzx*nuyz)/(Eyy*Ezz*delta);
  this->StressStrainMatrix[0][2] = (nuzx+v21*v32)/(Eyy*Ezz*delta);
  this->StressStrainMatrix[1][0] = (nuxy+v13*v32)/(Ezz*Exx*delta);
  this->StressStrainMatrix[1][1] = (1.0-nuzx*v13)/(Ezz*Exx*delta);
  this->StressStrainMatrix[1][2] = (v32+nuzx*nuxy)/(Ezz*Exx*delta);
  this->StressStrainMatrix[2][0] = (v13+nuxy*nuyz)/(Exx*Eyy*delta);
  this->StressStrainMatrix[2][1] = (nuyz+v13*v21)/(Exx*Eyy*delta);
  this->StressStrainMatrix[2][2] = (1.0-nuxy*v21)/(Exx*Eyy*delta);
  this->StressStrainMatrix[3][3] = Gyz;
  this->StressStrainMatrix[4][4] = Gzx;
  this->StressStrainMatrix[5][5] = Gxy;
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetOrthotropic
  (
  const double E[3],
  const double nu[3],
  const double G[3]
  )
  {
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {this->StressStrainMatrix[i][j] = 0; }
  double v32 = nu[0]*E[2]/E[1];
  double v13 = nu[1]*E[0]/E[2];
  double v21 = nu[2]*E[1]/E[0];
  double delta = (1.0-nu[2]*v21-nu[0]*v32-nu[1]*v13-2*nu[2]*nu[0]*nu[1])/(E[0]*E[1]*E[2]);
  this->StressStrainMatrix[0][0] = (1.0-nu[0]*v32)/(E[1]*E[2]*delta);
  this->StressStrainMatrix[0][1] = (v21+nu[1]*nu[0])/(E[1]*E[2]*delta);
  this->StressStrainMatrix[0][2] = (nu[1]+v21*v32)/(E[1]*E[2]*delta);
  this->StressStrainMatrix[1][0] = (nu[2]+v13*v32)/(E[2]*E[0]*delta);
  this->StressStrainMatrix[1][1] = (1.0-nu[1]*v13)/(E[2]*E[0]*delta);
  this->StressStrainMatrix[1][2] = (v32+nu[1]*nu[2])/(E[2]*E[0]*delta);
  this->StressStrainMatrix[2][0] = (v13+nu[2]*nu[0])/(E[0]*E[1]*delta);
  this->StressStrainMatrix[2][1] = (nu[0]+v13*v21)/(E[0]*E[1]*delta);
  this->StressStrainMatrix[2][2] = (1.0-nu[2]*v21)/(E[0]*E[1]*delta);
  this->StressStrainMatrix[3][3] = G[0];
  this->StressStrainMatrix[4][4] = G[1];
  this->StressStrainMatrix[5][5] = G[2];
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetOrthotropic
  (
  const float E[3],
  const float nu[3],
  const float G[3]
  )
  {
  this->SetOrthotropic (E[0], E[1], E[2], nu[0], nu[1], nu[2], G[0], G[1], G[2]);
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetStressStrainMatrix (const double* D)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  for (size_t k=0; k<36; ++k)
    { s[k] = D[k]; }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetStressStrainMatrix (const float* D)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  for (size_t k=0; k<36; ++k)
    { s[k] = D[k]; }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetStressStrainMatrix (vtkDataArray* D)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  if (D->GetNumberOfComponents() == 6)
    {
    for (size_t i=0; i<6; ++i)
      for (size_t j=0; j<6; ++j)
        {
        s[i*6+j] = D->GetComponent(i,j);
        }
    }
  else
    {
    for (size_t k=0; k<36; ++k)
      {
      s[k] = D->GetComponent(k,0);
      }
    }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetUpperTriangularPacked (const double* UT)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  *s = UT[ 0]; ++s;
  *s = UT[ 1]; ++s;
  *s = UT[ 3]; ++s;
  *s = UT[ 6]; ++s;
  *s = UT[10]; ++s;
  *s = UT[15]; ++s;
  *s = UT[ 1]; ++s;
  *s = UT[ 2]; ++s;
  *s = UT[ 4]; ++s;
  *s = UT[ 7]; ++s;
  *s = UT[11]; ++s;
  *s = UT[16]; ++s;
  *s = UT[ 3]; ++s;
  *s = UT[ 4]; ++s;
  *s = UT[ 5]; ++s;
  *s = UT[ 8]; ++s;
  *s = UT[12]; ++s;
  *s = UT[17]; ++s;
  *s = UT[ 6]; ++s;
  *s = UT[ 7]; ++s;
  *s = UT[ 8]; ++s;
  *s = UT[ 9]; ++s;
  *s = UT[13]; ++s;
  *s = UT[18]; ++s;
  *s = UT[10]; ++s;
  *s = UT[11]; ++s;
  *s = UT[12]; ++s;
  *s = UT[13]; ++s;
  *s = UT[14]; ++s;
  *s = UT[19]; ++s;
  *s = UT[15]; ++s;
  *s = UT[16]; ++s;
  *s = UT[17]; ++s;
  *s = UT[18]; ++s;
  *s = UT[19]; ++s;
  *s = UT[20];
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetUpperTriangularPacked (const float* UT)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  *s = UT[ 0]; ++s;
  *s = UT[ 1]; ++s;
  *s = UT[ 3]; ++s;
  *s = UT[ 6]; ++s;
  *s = UT[10]; ++s;
  *s = UT[15]; ++s;
  *s = UT[ 1]; ++s;
  *s = UT[ 2]; ++s;
  *s = UT[ 4]; ++s;
  *s = UT[ 7]; ++s;
  *s = UT[11]; ++s;
  *s = UT[16]; ++s;
  *s = UT[ 3]; ++s;
  *s = UT[ 4]; ++s;
  *s = UT[ 5]; ++s;
  *s = UT[ 8]; ++s;
  *s = UT[12]; ++s;
  *s = UT[17]; ++s;
  *s = UT[ 6]; ++s;
  *s = UT[ 7]; ++s;
  *s = UT[ 8]; ++s;
  *s = UT[ 9]; ++s;
  *s = UT[13]; ++s;
  *s = UT[18]; ++s;
  *s = UT[10]; ++s;
  *s = UT[11]; ++s;
  *s = UT[12]; ++s;
  *s = UT[13]; ++s;
  *s = UT[14]; ++s;
  *s = UT[19]; ++s;
  *s = UT[15]; ++s;
  *s = UT[16]; ++s;
  *s = UT[17]; ++s;
  *s = UT[18]; ++s;
  *s = UT[19]; ++s;
  *s = UT[20];
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::SetUpperTriangularPacked (vtkDataArray* UT)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  *s = UT->GetComponent( 0, 0); ++s;
  *s = UT->GetComponent( 1, 0); ++s;
  *s = UT->GetComponent( 3, 0); ++s;
  *s = UT->GetComponent( 6, 0); ++s;
  *s = UT->GetComponent(10, 0); ++s;
  *s = UT->GetComponent(15, 0); ++s;
  *s = UT->GetComponent( 1, 0); ++s;
  *s = UT->GetComponent( 2, 0); ++s;
  *s = UT->GetComponent( 4, 0); ++s;
  *s = UT->GetComponent( 7, 0); ++s;
  *s = UT->GetComponent(11, 0); ++s;
  *s = UT->GetComponent(16, 0); ++s;
  *s = UT->GetComponent( 3, 0); ++s;
  *s = UT->GetComponent( 4, 0); ++s;
  *s = UT->GetComponent( 5, 0); ++s;
  *s = UT->GetComponent( 8, 0); ++s;
  *s = UT->GetComponent(12, 0); ++s;
  *s = UT->GetComponent(17, 0); ++s;
  *s = UT->GetComponent( 6, 0); ++s;
  *s = UT->GetComponent( 7, 0); ++s;
  *s = UT->GetComponent( 8, 0); ++s;
  *s = UT->GetComponent( 9, 0); ++s;
  *s = UT->GetComponent(13, 0); ++s;
  *s = UT->GetComponent(18, 0); ++s;
  *s = UT->GetComponent(10, 0); ++s;
  *s = UT->GetComponent(11, 0); ++s;
  *s = UT->GetComponent(12, 0); ++s;
  *s = UT->GetComponent(13, 0); ++s;
  *s = UT->GetComponent(14, 0); ++s;
  *s = UT->GetComponent(19, 0); ++s;
  *s = UT->GetComponent(15, 0); ++s;
  *s = UT->GetComponent(16, 0); ++s;
  *s = UT->GetComponent(17, 0); ++s;
  *s = UT->GetComponent(18, 0); ++s;
  *s = UT->GetComponent(19, 0); ++s;
  *s = UT->GetComponent(20, 0);
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetStressStrainMatrix (double* D)
  {
  size_t k=0;
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {
      D[k] = this->StressStrainMatrix[i][j];
      ++k;
      }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetStressStrainMatrix (float* D)
  {
  size_t k=0;
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {
      D[k] = this->StressStrainMatrix[i][j];
      ++k;
      }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetStressStrainMatrix (vtkDataArray* D)
  {
  D->SetNumberOfComponents(1);
  D->SetNumberOfTuples(36);
  for (size_t i=0; i<6; ++i)
    for (size_t j=0; j<6; ++j)
      {
      D->SetComponent(i*6+j, 0, this->StressStrainMatrix[i][j]);
      }
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetUpperTriangularPacked (double* UT)
  {
  const double* s = &(this->StressStrainMatrix[0][0]);
  UT[ 0] = *s;  s += 6;
  UT[ 1] = *s;  ++s;
  UT[ 2] = *s;  s += 5;
  UT[ 3] = *s;  ++s;
  UT[ 4] = *s;  ++s;
  UT[ 5] = *s;  s += 4;
  UT[ 6] = *s;  ++s;
  UT[ 7] = *s;  ++s;
  UT[ 8] = *s;  ++s;
  UT[ 9] = *s;  s += 3;
  UT[10] = *s;  ++s;
  UT[11] = *s;  ++s;
  UT[12] = *s;  ++s;
  UT[13] = *s;  ++s;
  UT[14] = *s;  s += 2;
  UT[15] = *s;  ++s;
  UT[16] = *s;  ++s;
  UT[17] = *s;  ++s;
  UT[18] = *s;  ++s;
  UT[19] = *s;  ++s;
  UT[20] = *s;
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetUpperTriangularPacked (float* UT)
  {
  double* s = &(this->StressStrainMatrix[0][0]);
  UT[ 0] = *s;  s += 6;
  UT[ 1] = *s;  ++s;
  UT[ 2] = *s;  s += 5;
  UT[ 3] = *s;  ++s;
  UT[ 4] = *s;  ++s;
  UT[ 5] = *s;  s += 4;
  UT[ 6] = *s;  ++s;
  UT[ 7] = *s;  ++s;
  UT[ 8] = *s;  ++s;
  UT[ 9] = *s;  s += 3;
  UT[10] = *s;  ++s;
  UT[11] = *s;  ++s;
  UT[12] = *s;  ++s;
  UT[13] = *s;  ++s;
  UT[14] = *s;  s += 2;
  UT[15] = *s;  ++s;
  UT[16] = *s;  ++s;
  UT[17] = *s;  ++s;
  UT[18] = *s;  ++s;
  UT[19] = *s;  ++s;
  UT[20] = *s;
  }

//----------------------------------------------------------------------------
void vtkboneStressStrainMatrix::GetUpperTriangularPacked (vtkDataArray* UT)
  {
  UT->SetNumberOfComponents(1);
  UT->SetNumberOfTuples(21);
  double* s = &(this->StressStrainMatrix[0][0]);
  UT->SetComponent( 0, 0, *s);  s += 6;
  UT->SetComponent( 1, 0, *s);  ++s;
  UT->SetComponent( 2, 0, *s);  s += 5;
  UT->SetComponent( 3, 0, *s);  ++s;
  UT->SetComponent( 4, 0, *s);  ++s;
  UT->SetComponent( 5, 0, *s);  s += 4;
  UT->SetComponent( 6, 0, *s);  ++s;
  UT->SetComponent( 7, 0, *s);  ++s;
  UT->SetComponent( 8, 0, *s);  ++s;
  UT->SetComponent( 9, 0, *s);  s += 3;
  UT->SetComponent(10, 0, *s);  ++s;
  UT->SetComponent(11, 0, *s);  ++s;
  UT->SetComponent(12, 0, *s);  ++s;
  UT->SetComponent(13, 0, *s);  ++s;
  UT->SetComponent(14, 0, *s);  s += 2;
  UT->SetComponent(15, 0, *s);  ++s;
  UT->SetComponent(16, 0, *s);  ++s;
  UT->SetComponent(17, 0, *s);  ++s;
  UT->SetComponent(18, 0, *s);  ++s;
  UT->SetComponent(19, 0, *s);  ++s;
  UT->SetComponent(20, 0, *s);
  }
