#include "vtkboneStressStrainMatrix.h"
#include <vector>
#include <gtest/gtest.h>


// Create a test fixture class.
class StressStrainMatrixTests : public ::testing::Test {};

// --------------------------------------------------------------------
// test implementations

TEST_F (StressStrainMatrixTests, square_to_upper_packed)
  {
  double s[] ={ 0,  1,  3,  6, 10, 15,
                1,  2,  4,  7, 11, 16,
                3,  4,  5,  8, 12, 17,
                6,  7,  8,  9, 13, 18,
               10, 11, 12, 13, 14, 19,
               15, 16, 17, 18, 19, 20};
  vtkboneStressStrainMatrix* SS = vtkboneStressStrainMatrix::New();
  SS->SetStressStrainMatrix (s);
  std::vector<double> ut (21);
  // Fill with garbage
  for (size_t i=0; i<21; ++i)
    {
    ut[i] = -1;
    }
  SS->GetUpperTriangularPacked(ut.data());
  for (size_t i=0; i<21; ++i)
    {
    ASSERT_EQ (ut[i], i);
    }
  }

TEST_F (StressStrainMatrixTests, upper_packed_to_square)
  {
  double ut[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
  double s_ref[] ={ 0,  1,  3,  6, 10, 15,
                    1,  2,  4,  7, 11, 16,
                    3,  4,  5,  8, 12, 17,
                    6,  7,  8,  9, 13, 18,
                   10, 11, 12, 13, 14, 19,
                   15, 16, 17, 18, 19, 20};
  vtkboneStressStrainMatrix* SS = vtkboneStressStrainMatrix::New();
  SS->SetUpperTriangularPacked(ut);
  std::vector<double> s (36);
  // Fill with garbage
  for (size_t i=0; i<36; ++i)
    {
    s[i] = -1;
    }
  SS->GetStressStrainMatrix(s.data());
  for (size_t i=0; i<36; ++i)
    {
    ASSERT_EQ (s[i], s_ref[i]);
    }
  }
