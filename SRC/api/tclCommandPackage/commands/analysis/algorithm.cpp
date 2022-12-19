/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
** ****************************************************************** */
//
// Description: This file implements commands that allow for construction
// and interaction with Algorithm objects. Any command which requires
// access to specific Algorithm types (from the standard library) should
// be implemented here.
//
#include <stdio.h>
#include <assert.h>
#include <G3_Logging.h>
#include "analysis.h"
#include <tcl.h>
#include <api/InputAPI.h>
#include "runtime/BasicAnalysisBuilder.h"

// soln algorithms
#include <Linear.h>
#include <NewtonRaphson.h>
#include <ModifiedNewton.h>
#include <Broyden.h>
#include <BFGS.h>
#include <KrylovNewton.h>
#include <PeriodicNewton.h>
#include <AcceleratedNewton.h>
#include <ExpressNewton.h>

// line search
#include <NewtonLineSearch.h>
#include <BisectionLineSearch.h>
#include <InitialInterpolatedLineSearch.h>
#include <RegulaFalsiLineSearch.h>
#include <SecantLineSearch.h>

// accelerators
#include <RaphsonAccelerator.h>
#include <PeriodicAccelerator.h>
#include <KrylovAccelerator.h>
#include <SecantAccelerator1.h>
#include <SecantAccelerator2.h>
#include <SecantAccelerator3.h>
#include <MillerAccelerator.h>


extern "C" int OPS_ResetInputNoBuilder(ClientData clientData,
                                       Tcl_Interp *interp, int cArg, int mArg,
                                       TCL_Char **argv, Domain *domain);

typedef EquiSolnAlgo *(TclEquiSolnAlgo)(ClientData, Tcl_Interp *, int,
                                        TCL_Char **);
TclEquiSolnAlgo G3Parse_newEquiSolnAlgo;
TclEquiSolnAlgo G3Parse_newSecantNewtonAlgorithm;
TclEquiSolnAlgo G3Parse_newLinearAlgorithm;
TclEquiSolnAlgo G3_newNewtonLineSearch;

//
// command invoked to allow the SolnAlgorithm object to be built
//
int
TclCommand_specifyAlgorithm(ClientData clientData, Tcl_Interp *interp, int argc,
                 TCL_Char **argv)
{

  BasicAnalysisBuilder *builder = (BasicAnalysisBuilder *)clientData;
  assert(builder != nullptr);

  // Make sure at least one other argument to contain numberer
  if (argc < 2) {
    opserr << G3_ERROR_PROMPT << "Need to specify an Algorithm type.\n";
    return TCL_ERROR;
  }

  OPS_ResetInputNoBuilder(nullptr, interp, 2, argc, argv, nullptr);

  // G3_Runtime *rt = G3_getRuntime(interp);
  EquiSolnAlgo *theNewAlgo = nullptr;
  theNewAlgo = G3Parse_newEquiSolnAlgo(clientData, interp, argc, argv);

  if (theNewAlgo == nullptr) {
    // Leave it to parsing routine to print error info, this way
    // we get more detail.
    return TCL_ERROR;

  } else {
    builder->set(theNewAlgo);
  }
  return TCL_OK;
}

int
TclCommand_totalCPU(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *algo = ((BasicAnalysisBuilder *)clientData)->getAlgorithm();

  char buffer[20];

  if (algo == 0)
    return TCL_ERROR;

  sprintf(buffer, "%f", algo->getTotalTimeCPU());
  Tcl_SetResult(interp, buffer, TCL_VOLATILE);

  return TCL_OK;
}

int
TclCommand_solveCPU(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *algo = ((BasicAnalysisBuilder *)clientData)->getAlgorithm();

  char buffer[20];

  if (algo == 0)
    return TCL_ERROR;

  sprintf(buffer, "%f", algo->getSolveTimeCPU());
  Tcl_SetResult(interp, buffer, TCL_VOLATILE);

  return TCL_OK;
}


int
TclCommand_numIter(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *algo = ((BasicAnalysisBuilder *)clientData)->getAlgorithm();


  if (algo == nullptr)
    return TCL_ERROR;

  char buffer[20];
  sprintf(buffer, "%d", algo->getNumIterations());
  Tcl_SetResult(interp, buffer, TCL_VOLATILE);

  return TCL_OK;
}

EquiSolnAlgo *
G3Parse_newEquiSolnAlgo(ClientData clientData, Tcl_Interp *interp, int argc,
                        TCL_Char **argv)
{
  EquiSolnAlgo *theNewAlgo = nullptr;

  G3_Runtime *rt = G3_getRuntime(interp);

  // check argv[1] for type of Algorithm and create the object
  if (strcmp(argv[1], "Linear") == 0) {
    theNewAlgo = G3Parse_newLinearAlgorithm(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "Newton") == 0) {
    void *theNewtonAlgo = OPS_NewtonRaphsonAlgorithm(rt);
    theNewAlgo = (EquiSolnAlgo *)theNewtonAlgo;
  }

  else if ((strcmp(argv[1], "NewtonHallM") == 0) ||
           (strcmp(argv[1], "NewtonHall") == 0)) {
    void *theNewtonAlgo = OPS_NewtonHallM(rt);
    theNewAlgo = (EquiSolnAlgo *)theNewtonAlgo;
  }

  else if (strcmp(argv[1], "ModifiedNewton") == 0) {
    void *theNewtonAlgo = OPS_ModifiedNewton(rt);
    theNewAlgo = (EquiSolnAlgo *)theNewtonAlgo;
  }

  else if (strcmp(argv[1], "SecantNewton") == 0) {
    theNewAlgo =
        G3Parse_newSecantNewtonAlgorithm(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ExpressNewton") == 0) {
    void *theNewtonAlgo = OPS_ExpressNewton(rt);
    theNewAlgo = (EquiSolnAlgo *)theNewtonAlgo;
  }

  else if (strcmp(argv[1], "NewtonLineSearch") == 0)
    theNewAlgo = G3_newNewtonLineSearch(clientData, interp, argc, argv);

  else {
    opserr << G3_ERROR_PROMPT << "No EquiSolnAlgo type " << argv[1] << " exists\n";
    return nullptr;
  }

  return theNewAlgo;
}

EquiSolnAlgo *
G3Parse_newLinearAlgorithm(ClientData clientData, Tcl_Interp *interp, int argc,
                           TCL_Char **argv)
{
  int formTangent = CURRENT_TANGENT;
  int factorOnce = 0;
  int count = 2;
  while (count < argc) {
    if ((strcmp(argv[count], "-secant") == 0) ||
        (strcmp(argv[count], "-Secant") == 0)) {
      formTangent = CURRENT_SECANT;
    } else if ((strcmp(argv[count], "-initial") == 0) ||
               (strcmp(argv[count], "-Initial") == 0)) {
      formTangent = INITIAL_TANGENT;
    } else if ((strcmp(argv[count], "-factorOnce") == 0) ||
               (strcmp(argv[count], "-FactorOnce") == 0)) {
      factorOnce = 1;
    }
    count++;
  }
  return new Linear(formTangent, factorOnce);
}

EquiSolnAlgo *
G3Parse_newSecantNewtonAlgorithm(ClientData clientData, Tcl_Interp *interp,
                                 int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  int incrementTangent = CURRENT_TANGENT;
  int iterateTangent = CURRENT_TANGENT;
  int maxDim = 3;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-iterate") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        iterateTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        iterateTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        iterateTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-increment") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        incrementTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        incrementTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        incrementTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-maxDim") == 0 && i + 1 < argc) {
      i++;
      maxDim = atoi(argv[i]);
    }
  }

  Accelerator *theAccel;
  theAccel = new SecantAccelerator2(maxDim, iterateTangent);
  return new AcceleratedNewton(*theTest, theAccel, incrementTangent);
}

EquiSolnAlgo *
G3_newBFGS(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{

  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  if (strcmp(argv[1], "BFGS") == 0) {

    int formTangent = CURRENT_TANGENT;
    int count = -1;
    for (int i = 2; i < argc; i++) {
      if (strcmp(argv[i], "-secant") == 0) {
        formTangent = CURRENT_SECANT;
      } else if (strcmp(argv[i], "-initial") == 0) {
        formTangent = INITIAL_TANGENT;
      } else if (strcmp(argv[i++], "-count") == 0 && i < argc) {
        count = atoi(argv[i]);
      }
    }

    if (count == -1)
      theNewAlgo = new BFGS(*theTest, formTangent);
    else
      theNewAlgo = new BFGS(*theTest, formTangent, count);
  }
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newNewtonLineSearch(ClientData clientData, Tcl_Interp *interp, int argc,
                       TCL_Char **argv)
{

  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (strcmp(argv[1], "NewtonLineSearch") == 0) {
    if (theTest == nullptr) {
      opserr << G3_ERROR_PROMPT << " No ConvergenceTest yet specified\n";
      return nullptr;
    }

    int count = 2;

    // set some default variable
    double tol = 0.8;
    int maxIter = 10;
    double maxEta = 10.0;
    double minEta = 0.1;
    int pFlag = 1;
    int typeSearch = 0;

    while (count < argc) {
      if (strcmp(argv[count], "-tol") == 0) {
        count++;
        if (Tcl_GetDouble(interp, argv[count], &tol) != TCL_OK)
          return nullptr;
        count++;
      } else if (strcmp(argv[count], "-maxIter") == 0) {
        count++;
        if (Tcl_GetInt(interp, argv[count], &maxIter) != TCL_OK)
          return nullptr;
        count++;
      } else if (strcmp(argv[count], "-pFlag") == 0) {
        count++;
        if (Tcl_GetInt(interp, argv[count], &pFlag) != TCL_OK)
          return nullptr;
        count++;
      } else if (strcmp(argv[count], "-minEta") == 0) {
        count++;
        if (Tcl_GetDouble(interp, argv[count], &minEta) != TCL_OK)
          return nullptr;
        count++;
      } else if (strcmp(argv[count], "-maxEta") == 0) {
        count++;
        if (Tcl_GetDouble(interp, argv[count], &maxEta) != TCL_OK)
          return nullptr;
        count++;
      } else if (strcmp(argv[count], "-type") == 0) {
        count++;
        if (strcmp(argv[count], "Bisection") == 0)
          typeSearch = 1;
        else if (strcmp(argv[count], "Secant") == 0)
          typeSearch = 2;
        else if (strcmp(argv[count], "RegulaFalsi") == 0)
          typeSearch = 3;
        else if (strcmp(argv[count], "LinearInterpolated") == 0)
          typeSearch = 3;
        else if (strcmp(argv[count], "InitialInterpolated") == 0)
          typeSearch = 0;
        count++;
      } else
        count++;
    }

    LineSearch *theLineSearch = 0;
    if (typeSearch == 0)
      theLineSearch = new InitialInterpolatedLineSearch(tol, maxIter, minEta,
                                                        maxEta, pFlag);
    else if (typeSearch == 1)
      theLineSearch =
          new BisectionLineSearch(tol, maxIter, minEta, maxEta, pFlag);
    else if (typeSearch == 2)
      theLineSearch = new SecantLineSearch(tol, maxIter, minEta, maxEta, pFlag);
    else if (typeSearch == 3)
      theLineSearch =
          new RegulaFalsiLineSearch(tol, maxIter, minEta, maxEta, pFlag);

    theNewAlgo = new NewtonLineSearch(*theTest, theLineSearch);
  }
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newKrylovNewton(ClientData clientData, Tcl_Interp *interp, int argc,
                   TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  int incrementTangent = CURRENT_TANGENT;
  int iterateTangent = CURRENT_TANGENT;
  int maxDim = 3;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-iterate") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        iterateTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        iterateTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        iterateTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-increment") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        incrementTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        incrementTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        incrementTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-maxDim") == 0 && i + 1 < argc) {
      i++;
      maxDim = atoi(argv[i]);
    }
  }

  Accelerator *theAccel;
  theAccel = new KrylovAccelerator(maxDim, iterateTangent);

  theNewAlgo = new AcceleratedNewton(*theTest, theAccel, incrementTangent);
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newRaphsonNewton(ClientData clientData, Tcl_Interp *interp, int argc,
                    TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  int incrementTangent = CURRENT_TANGENT;
  int iterateTangent = CURRENT_TANGENT;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-iterate") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        iterateTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        iterateTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        iterateTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-increment") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        incrementTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        incrementTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        incrementTangent = NO_TANGENT;
    }
  }

  Accelerator *theAccel;
  theAccel = new RaphsonAccelerator(iterateTangent);

  theNewAlgo = new AcceleratedNewton(*theTest, theAccel, incrementTangent);
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newMillerNewton(ClientData clientData, Tcl_Interp *interp, int argc,
                   TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  int incrementTangent = CURRENT_TANGENT;
  int iterateTangent = CURRENT_TANGENT;
  int maxDim = 3;

  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-iterate") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        iterateTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        iterateTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        iterateTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-increment") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        incrementTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        incrementTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        incrementTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-maxDim") == 0 && i + 1 < argc) {
      i++;
      maxDim = atoi(argv[i]);
    }
  }

  Accelerator *theAccel = 0;
  // theAccel = new MillerAccelerator(maxDim, 0.01, iterateTangent);

  theNewAlgo = new AcceleratedNewton(*theTest, theAccel, incrementTangent);
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newPeriodicNewton(ClientData clientData, Tcl_Interp *interp, int argc,
                     TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }

  int incrementTangent = CURRENT_TANGENT;
  int iterateTangent = CURRENT_TANGENT;
  int maxDim = 3;
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-iterate") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        iterateTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        iterateTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        iterateTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-increment") == 0 && i + 1 < argc) {
      i++;
      if (strcmp(argv[i], "current") == 0)
        incrementTangent = CURRENT_TANGENT;
      if (strcmp(argv[i], "initial") == 0)
        incrementTangent = INITIAL_TANGENT;
      if (strcmp(argv[i], "noTangent") == 0)
        incrementTangent = NO_TANGENT;
    } else if (strcmp(argv[i], "-maxDim") == 0 && i + 1 < argc) {
      i++;
      maxDim = atoi(argv[i]);
    }
  }

  Accelerator *theAccel;
  theAccel = new PeriodicAccelerator(maxDim, iterateTangent);

  theNewAlgo = new AcceleratedNewton(*theTest, theAccel, incrementTangent);
  return theNewAlgo;
}

EquiSolnAlgo *
G3_newBroyden(ClientData clientData, Tcl_Interp *interp, int argc,
              TCL_Char **argv)
{
  assert(clientData != nullptr);
  EquiSolnAlgo *theNewAlgo = nullptr;
  ConvergenceTest *theTest =
      ((BasicAnalysisBuilder *)clientData)->getConvergenceTest();

  int formTangent = CURRENT_TANGENT;
  int count = -1;

  if (theTest == nullptr) {
    opserr << G3_ERROR_PROMPT << "No ConvergenceTest yet specified\n";
    return nullptr;
  }
  for (int i = 2; i < argc; i++) {
    if (strcmp(argv[i], "-secant") == 0) {
      formTangent = CURRENT_SECANT;
    } else if (strcmp(argv[i], "-initial") == 0) {
      formTangent = INITIAL_TANGENT;
    } else if (strcmp(argv[i++], "-count") == 0 && i < argc) {
      count = atoi(argv[i]);
    }
  }

  if (count == -1)
    theNewAlgo = new Broyden(*theTest, formTangent);
  else
    theNewAlgo = new Broyden(*theTest, formTangent, count);

  return theNewAlgo;
}

int
printAlgorithm(ClientData clientData, Tcl_Interp *interp, int argc,
               TCL_Char **argv, OPS_Stream &output)
{
  assert(clientData != nullptr);
  BasicAnalysisBuilder *builder = (BasicAnalysisBuilder *)clientData;
  EquiSolnAlgo* theAlgorithm = builder->getAlgorithm();

  int eleArg = 0;
  if (theAlgorithm == nullptr)
    return TCL_OK;

  // if just 'print <filename> algorithm'- no flag
  if (argc == 0) {
    theAlgorithm->Print(output);
    return TCL_OK;
  }

  // if 'print <filename> Algorithm flag' get the flag
  int flag;
  if (Tcl_GetInt(interp, argv[eleArg], &flag) != TCL_OK) {
    opserr << "WARNING print algorithm failed to get integer flag: \n";
    opserr << argv[eleArg] << endln;
    return TCL_ERROR;
  }
  theAlgorithm->Print(output, flag);
  return TCL_OK;
}

int
TclCommand_accelCPU(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  BasicAnalysisBuilder *builder = (BasicAnalysisBuilder *)clientData;
  EquiSolnAlgo* theAlgorithm = builder->getAlgorithm();

  char buffer[20];
  if (theAlgorithm == nullptr)
    return TCL_ERROR;

  sprintf(buffer, "%f", theAlgorithm->getAccelTimeCPU());
  Tcl_SetResult(interp, buffer, TCL_VOLATILE);

  return TCL_OK;
}

int
TclCommand_numFact(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **argv)
{
  assert(clientData != nullptr);
  BasicAnalysisBuilder *builder = (BasicAnalysisBuilder *)clientData;
  EquiSolnAlgo* theAlgorithm = builder->getAlgorithm();

  char buffer[20];

  if (theAlgorithm == nullptr)
    return TCL_ERROR;

  sprintf(buffer, "%d", theAlgorithm->getNumFactorizations());
  Tcl_SetResult(interp, buffer, TCL_VOLATILE);

  return TCL_OK;
}

