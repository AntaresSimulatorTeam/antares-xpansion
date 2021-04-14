#include "define_datas.hpp"


void fill_datas(AllDatas& datas) {

    datas.clear();

    //==================================================================
    //1. mip toy
    InstanceData miptoy = InstanceData();
    miptoy._path = "../../data_test/mps/mip_toy_prob.mps";
    miptoy._ncols = 2;
    miptoy._nintegervars = 2;
    miptoy._nrows = 2;
    miptoy._nelems = 4;
    miptoy._obj = { -5.0, -4.0 };

    miptoy._matval = { 1.0, 1.0, 10.0, 6.0 };
    miptoy._mind = { 0, 1, 0, 1 };
    miptoy._mstart = { 0, 2, 4 };

    miptoy._matval_delete = { 10.0, 6.0 };
    miptoy._mind_delete = { 0, 1 };
    miptoy._mstart_delete = { 0, 2 };

    miptoy._rhs = { 5.0, 45.0 };
    miptoy._lb = { 0.0, 0.0 };
    miptoy._ub = { 1e20, 1e20 };
    miptoy._coltypes = { 'I', 'I' };
    miptoy._rowtypes = { 'L', 'L' };
    miptoy._optval = -23.0;
    miptoy._primals = { 3, 2 };
    miptoy._duals = {};
    miptoy._slacks = { 0, 1 };
    miptoy._reduced_costs = {};

    miptoy._status = { "OPTIMAL" };
    miptoy._status_int = { OPTIMAL };

    miptoy.first_col_name = "x1";
    miptoy.first_row_name = "C1";
    miptoy.iterations = 3;

    datas.push_back(miptoy);


    //==================================================================
    //2. multi knapsack
    InstanceData multikp = InstanceData();
    multikp._path = "../../data_test/mps/lp1.mps";
    multikp._ncols = 9;
    multikp._nintegervars = 0;
    multikp._nrows = 5;
    multikp._nelems = 15;
    multikp._obj = { 4.0, 4.0, 4.0, 2.0, 2.0, 2.0, 0.5, 0.5, 0.5 };

    multikp._matval = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    multikp._mind = { 0, 3, 6, 1, 4, 7, 2, 5, 8, 0, 1, 2, 3, 4, 5 };
    multikp._mstart = { 0, 3, 6, 9, 12, 15 };

    multikp._matval_delete = { 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0 };
    multikp._mind_delete = { 1, 4, 7, 2, 5, 8, 0, 1, 2, 3, 4, 5 };
    multikp._mstart_delete = { 0, 3, 6, 9, 12 };

    multikp._rhs = { 12.0, 7.1, 10.0, 5.5, 8.3 };
    multikp._lb = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
    multikp._ub = { 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20, 1e20 };
    multikp._coltypes = { 'C', 'C', 'C', 'C', 'C', 'C','C', 'C', 'C' };
    multikp._rowtypes = { 'L', 'L', 'L', 'L', 'L' };
    multikp._optval = 46.25;
    multikp._primals = { 0.0, 0.0, 5.5, 8.3, 0.0, 0.0, 3.7, 7.1, 4.5 };
    multikp._duals = {};
    multikp._slacks = { };
    multikp._reduced_costs = {};

    multikp._status = { "OPTIMAL" };
    multikp._status_int = { OPTIMAL };

    multikp.first_col_name = "x11";
    multikp.first_row_name = "sac1";
    multikp.iterations = 5;

    datas.push_back(multikp);

    //==================================================================
    //3. unbounded
    InstanceData unbd = InstanceData();
    unbd._path = "../../data_test/mps/unbounded.mps";
    unbd._ncols = 2;
    unbd._nintegervars = 0;
    unbd._nrows = 2;
    unbd._nelems = 3;
    unbd._obj = { 4.0, 2.67 };

    unbd._matval = { -1.0, 64.0, -21.0 };
    unbd._mind = { 0, 1, 1 };
    unbd._mstart = { 0, 2, 3 };

    unbd._matval_delete = { -21.0 };
    unbd._mind_delete = { 1 };
    unbd._mstart_delete = { 0, 1 };

    unbd._rhs = { 12.0, -3.0 };
    unbd._lb = { 0.0, 0.0 };
    unbd._ub = { 1e20, 1e20 };
    unbd._coltypes = { 'C', 'C' };
    unbd._rowtypes = { 'L', 'G' };
    unbd._optval = -1;
    unbd._primals = { };
    unbd._duals = { };
    unbd._slacks = { };
    unbd._reduced_costs = { };

    unbd._status = { "UNBOUNDED", "INForUNBOUND" };
    unbd._status_int = { UNBOUNDED, INForUNBOUND };

    unbd.first_col_name = "x1";
    unbd.first_row_name = "sac1";

    datas.push_back(unbd);

    //==================================================================
    //4. infeasible
    InstanceData infeas = InstanceData();
    infeas._path = "../../data_test/mps/infeas.mps";
    infeas._ncols = 2;
    infeas._nintegervars = 0;
    infeas._nrows = 2;
    infeas._nelems = 4;
    infeas._obj = { 4.0, -2.1 };

    infeas._matval = { 2.0, 1.0, 1.0, 1.0 };
    infeas._mind = { 0, 1, 0, 1 };
    infeas._mstart = { 0, 2, 4 };

    infeas._matval_delete = { 1.0, 1.0 };
    infeas._mind_delete = { 0, 1 };
    infeas._mstart_delete = { 0, 2 };

    infeas._rhs = { 2.0, 4.0 };
    infeas._lb = { 0.0, 0.0 };
    infeas._ub = { 1e20, 1e20 };
    infeas._coltypes = { 'C', 'C' };
    infeas._rowtypes = { 'L', 'G' };
    infeas._optval = -1;
    infeas._primals = { };
    infeas._duals = { };
    infeas._slacks = { };
    infeas._reduced_costs = { };

    infeas._status = { "INFEASIBLE", "INForUNBOUND" };
    infeas._status_int = { INFEASIBLE, INForUNBOUND };
    
    infeas.first_col_name = "x";
    infeas.first_row_name = "low";

    datas.push_back(infeas);


    //==================================================================
    // 5. NETWORK instance -- master
    InstanceData net_master = InstanceData();
    net_master._path = "../../data_test/mini_network/master.mps";
    net_master._ncols = 2;
    net_master._nintegervars = 0;
    net_master._nrows = 1;
    net_master._nelems = 2;
    net_master._obj = { 3.0, 2.0 };

    net_master._matval = { 1.0, 1.0 };
    net_master._mind = { 0, 1 };
    net_master._mstart = { 0, 2 };

    net_master._rhs = { 10.0 };
    net_master._lb = { 0.0, 0.0 };
    net_master._ub = { 1e20, 1e20 };
    net_master._coltypes = { 'C', 'C' };
    net_master._rowtypes = { 'L' };
    net_master._optval = 0.0;
    net_master._primals = { 0.0, 0.0 };
    net_master._duals = { };
    net_master._slacks = { };
    net_master._reduced_costs = { };

    net_master._status = { "OPTIMAL" };
    net_master._status_int = { OPTIMAL };

    net_master._varmap["t"] = 0;
    net_master._varmap["p"] = 1;

    datas.push_back(net_master);

    //==================================================================
    // 6. NETWORK instance -- SP1
    InstanceData net_sp1 = InstanceData();
    net_sp1._path = "../../data_test/mini_network/SP1.mps";
    net_sp1._ncols = 4;
    net_sp1._nintegervars = 0;
    net_sp1._nrows = 3;
    net_sp1._nelems = 6;
    net_sp1._obj = { 0.0, 0.0, 1.5, 100.0 };

    net_sp1._matval = { -1.0, 1.5, -1.0, 1.0, 1.0, 1.0 };
    net_sp1._mind = { 1, 2, 0, 2, 2, 3 };
    net_sp1._mstart = { 0, 2, 4, 6 };

    net_sp1._rhs = { 0.0, 0.0, 2.0 };
    net_sp1._lb = { 0.0, 0.0, 0.0, 0.0 };
    net_sp1._ub = { 1e20, 1e20, 1e20, 1e20 };
    net_sp1._coltypes = { 'C', 'C', 'C', 'C' };
    net_sp1._rowtypes = { 'L', 'L', 'G' };
    net_sp1._optval = 3.0;
    net_sp1._primals = { 2.0, 2.0, 2.0, 0.0 };
    net_sp1._duals = { };
    net_sp1._slacks = { };
    net_sp1._reduced_costs = { };

    net_sp1._status = { "OPTIMAL" };
    net_sp1._status_int = { OPTIMAL };

    net_sp1._varmap["t"] = 0;
    net_sp1._varmap["p"] = 1;

    net_sp1._subgrad["t"] = 0.0;
    net_sp1._subgrad["p"] = -65.6667;

    datas.push_back(net_sp1);

    //==================================================================
    // 7. NETWORK instance -- SP2
    InstanceData net_sp2 = InstanceData();
    net_sp2._path = "../../data_test/mini_network/SP2.mps";
    net_sp2._ncols = 4;
    net_sp2._nintegervars = 0;
    net_sp2._nrows = 3;
    net_sp2._nelems = 6;
    net_sp2._obj = { 0.0, 0.0, 1.5, 100.0 };

    net_sp2._matval = { -1.0, 1.5, -1.0, 1.0, 1.0, 1.0 };
    net_sp2._mind = { 1, 2, 0, 2, 2, 3 };
    net_sp2._mstart = { 0, 2, 4, 6 };

    net_sp2._rhs = { 0.0, 0.0, 6.0 };
    net_sp2._lb = { 0.0, 0.0, 0.0, 0.0 };
    net_sp2._ub = { 1e20, 1e20, 1e20, 1e20 };
    net_sp2._coltypes = { 'C', 'C', 'C', 'C' };
    net_sp2._rowtypes = { 'L', 'L', 'G' };
    net_sp2._optval = 9.0;
    net_sp2._primals = { 6.0, 6.0, 6.0, 0.0 };
    net_sp2._duals = { };
    net_sp2._slacks = { };
    net_sp2._reduced_costs = { };

    net_sp2._status = { "OPTIMAL" };
    net_sp2._status_int = { OPTIMAL };

    net_sp2._varmap["t"] = 0;
    net_sp2._varmap["p"] = 1;

    net_sp2._subgrad["t"] = 0.0;
    net_sp2._subgrad["p"] = -65.6667;

    datas.push_back(net_sp2);
}