find_package (PythonInterp REQUIRED)

set (Tests
  TestImageConnectivityMap.py
  TestImageConnectivityFilter.py
  TestTensorOfInertia.py
  TestLinearIsotropicMaterial.py
  TestLinearIsotropicMaterialArray.py
  TestLinearOrthotropicMaterial.py
  TestLinearOrthotropicMaterialArray.py
  TestLinearAnisotropicMaterial.py
  TestLinearAnisotropicMaterialArray.py
  TestMohrCoulombIsotropicMaterial.py
  TestMaterialTable.py
  TestGenerateHomogeneousMaterialTable.py
  TestGenerateHommingaMaterialTable.py
  TestFiniteElementModel.py
  TestNodeSetsByGeometry.py
  TestConstraint.py
  TestConstraintUtilities.py
  TestFiniteElementModelGenerator.py
  TestApplyTestBase.py
  TestApplyCompressionTest.py
  TestApplyBendingTest.py
  TestApplyDirectionalShearTest.py
  TestApplySymmetricShearTest.py
  TestApplyTorsionTest.py
  TestStressStrainMatrix.py
  TestCoarsenModel.py
  )

foreach (test ${Tests})
  get_filename_component (TName ${test} NAME_WE)
  add_test (${TName} ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test} -v)
endforeach (test)
