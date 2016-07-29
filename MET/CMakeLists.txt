#
SET( MET_FILES
    
    metil_base_argv.met
    metil_base_array.met
    metil_base_default_behavior.met
    metil_base_dest.met
    metil_base_init.met
    metil_base_interprer.met
    metil_base_matrix.met
    metil_base_plot.met
    metil_base_program.met
    metil_base_stream.met
    metil_base_symbol.met
    metil_base_type_help.met
    metil_base_vecop.met
    metil_base_vector.met
    metil_base_tensor.met
    metil_base_map.met
    metil_base_polynomial.met
    metil_base_complex.met
    metil_base_big_unsigned.met
    metil_base_serialization.met
    metil_base_mpi.met
    MatSparse.met
    
    Field.met
    BuildEnv.met
    
#     lazy.met
#     lazy_scalar.met
#     lazy_stream.met
#     lazy_vec.met
#     lazy_mat.met
#     lazy_scheduling.met
#     lazy_cpp_function.met
#     lazy_mat_cholesky.met
#     metil_base_cpp_generator.met
)
SET( MET_FILES_EF
    EF/DisplayParaview.met
    EF/ElemList.met
    EF/Formulation.met
    EF/mechanical_tensors.met
    EF/InterfaceExtraction.met
    EF/InstanceParametricElement.met
    EF/Mesh.met
    EF/ParametricElement.met
    EF/read_mesh.met
    EF/read_msh.met
    EF/std_elements.met
    EF/SymbolicElement.met
    EF/LazyOperatorFieldProjection.met
)
SET( MET_FILES_LAZY
    lazy/CppCompiledFunctionSet.met
    lazy/generate_cpp_struct.met
    lazy/LazyCollection.met
    lazy/LazyMat.met
    lazy/LazyMatPartFull.met
    lazy/LazyMatPartFunctionnal.met
    lazy/LazyMatPartSparse.met
    lazy/LazyObjectAncestor.met
    lazy/LazyObjectStream.met
    lazy/LazyScalar.met
    lazy/LazySimpleScheduler.met
    lazy/LazyVec.met
    lazy/std_operators.met
    lazy/UnknownType.met
    lazy/LazyComputerInfo.met
)
SET( MET_FILES_ITER_SOLV
    iterative_solvers/iterative_solver.met
    iterative_solvers/orthogonalizer.met
    iterative_solvers/projector.met
)
#

INSTALL( FILES ${MET_FILES} DESTINATION ${CMAKE_METIL_INSTALL_PREFIX} )

INSTALL( FILES ${MET_FILES_EF} DESTINATION ${CMAKE_METIL_INSTALL_PREFIX}/EF )
INSTALL( FILES ${MET_FILES_LAZY} DESTINATION ${CMAKE_METIL_INSTALL_PREFIX}/lazy )
INSTALL( FILES ${MET_FILES_ITER_SOLV} DESTINATION ${CMAKE_METIL_INSTALL_PREFIX}/iterative_solvers )

INSTALL( CODE "EXEC_PROGRAM( ${CMAKE_CURRENT_BINARY_DIR}/../CPP/metil ARGS -t ${CMAKE_METIL_INSTALL_PREFIX}/*.met )" )

