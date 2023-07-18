/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
** ****************************************************************** */
//
// Description: This file contains the implementation of the TclElementCommands.
// The file contains the routine TclElementCommands which is invoked by the
// TclBasicBuilder.
//
#include <assert.h>
#include <stdlib.h>

#ifdef _MSC_VER 
#  include <string.h>
#  define strcasecmp _stricmp
#else
#  include <strings.h>
#endif
#define strcmp strcasecmp

#include <g3_api.h>
#include <runtime/BasicModelBuilder.h>

#include <OPS_Stream.h>
#include <G3_Logging.h>
#include <packages.h>
#include <Domain.h>
#include <Element.h>

class TclBasicBuilder;
class CrdTransf;

#include <UniaxialMaterial.h>
#include <MultipleShearSpring.h>
#include <MultipleNormalSpring.h>
#include <KikuchiBearing.h>
#include <YamamotoBiaxialHDR.h>
#include <WheelRail.h>


typedef struct elementPackageCommand {
  char *funcName;
  void *(*funcPtr)();
  struct elementPackageCommand *next;
} ElementPackageCommand;

static ElementPackageCommand *theElementPackageCommands = nullptr;

extern "C" int OPS_ResetInputNoBuilder(ClientData clientData, Tcl_Interp *interp, int cArg,
                          int mArg, TCL_Char ** const argv, Domain *domain);

//
// THE PROTOTYPES OF THE FUNCTIONS INVOKED BY THE INTERPRETER
//
typedef void *OPS_Routine(G3_Runtime* , int, const char** const);
extern OPS_Routine OPS_ComponentElement2d;
// extern  void *OPS_ComponentElementDamp2d(G3_Runtime*);
extern OPS_Routine OPS_TrussElement;
extern OPS_Routine OPS_TrussSectionElement;
extern OPS_Routine OPS_CorotTrussElement;
extern OPS_Routine OPS_CorotTrussSectionElement;
extern OPS_Routine OPS_ElasticTubularJoint;
extern OPS_Routine OPS_ZeroLengthContactNTS2D;
extern OPS_Routine OPS_ZeroLengthVG_HG;
extern OPS_Routine OPS_ZeroLengthInterface2D;
extern OPS_Routine OPS_ZeroLengthImpact3D;
extern OPS_Routine OPS_ZeroLengthContactASDimplex; 
extern "C" OPS_Routine OPS_PY_Macro2D;
extern OPS_Routine OPS_SimpleContact2D;
extern OPS_Routine OPS_SimpleContact3D;

extern OPS_Routine OPS_SurfaceLoad;
extern OPS_Routine OPS_TriSurfaceLoad;

extern OPS_Routine OPS_ModElasticBeam2d;
extern void *OPS_ElasticBeam2d(G3_Runtime *, const ID &);
extern OPS_Routine OPS_ElasticBeam3d;
extern OPS_Routine OPS_ElasticTimoshenkoBeam2d;
extern OPS_Routine OPS_ElasticTimoshenkoBeam3d;
extern OPS_Routine OPS_AxEqDispBeamColumn2d;
extern OPS_Routine OPS_BeamGT;
// extern void* OPS_GradientInelasticBeamColumn2d();
// extern void* OPS_GradientInelasticBeamColumn3d();
extern OPS_Routine OPS_DispBeamColumnAsym3dTcl;  // Xinlong Du
extern OPS_Routine OPS_MixedBeamColumnAsym3dTcl; // Xinlong Du
#if defined(_HAVE_LHNMYS) || defined(OPSDEF_ELEMENT_LHNMYS)
  extern void *OPS_BeamColumn2DwLHNMYS(G3_Runtime*);
  extern void *OPS_Beam2dDamage(G3_Runtime*);
  extern void *OPS_BeamColumn2DwLHNMYS_Damage(G3_Runtime*);
  extern void *OPS_BeamColumn3DwLHNMYS(G3_Runtime*);
#endif

extern OPS_Routine OPS_TPB1D;
extern OPS_Routine OPS_TFP_Bearing;
extern OPS_Routine OPS_FPBearingPTV;
extern OPS_Routine OPS_MultiFP2d;
extern OPS_Routine OPS_CoupledZeroLength;
extern OPS_Routine OPS_FourNodeQuad3d;
extern OPS_Routine OPS_Quad4FiberOverlay;
extern OPS_Routine OPS_QuadBeamEmbedContact;
extern OPS_Routine OPS_ASID8QuadWithSensitivity;
extern OPS_Routine OPS_AV3D4QuadWithSensitivity;

extern OPS_Routine OPS_Brick8FiberOverlay;
extern OPS_Routine OPS_TripleFrictionPendulum;
extern OPS_Routine OPS_Truss2;
extern OPS_Routine OPS_PML3D;
extern OPS_Routine OPS_PML2D;
extern OPS_Routine OPS_CorotTruss2;
extern OPS_Routine OPS_HDR;
extern OPS_Routine OPS_LeadRubberX;
extern OPS_Routine OPS_ElastomericX;
extern OPS_Routine OPS_N4BiaxialTruss;
extern OPS_Routine OPS_AC3D8HexWithSensitivity;
extern OPS_Routine OPS_VS3D4WuadWithSensitivity;
extern OPS_Routine OPS_MVLEM;        // Kristijan Kolozvari
extern OPS_Routine OPS_SFI_MVLEM;    // Kristijan Kolozvari
extern OPS_Routine OPS_MVLEM_3D;     // Kristijan Kolozvari
extern OPS_Routine OPS_SFI_MVLEM_3D; // Kristijan Kolozvari
extern OPS_Routine OPS_ElastomericBearingBoucWenMod3d;
extern OPS_Routine OPS_InertiaTrussElement; // Added by Xiaodong Ji, Yuhao Cheng, Yue Yu
extern OPS_Routine OPS_CatenaryCableElement;
extern OPS_Routine OPS_ASDEmbeddedNodeElement; // Massimo Petracca (ASDEA)
extern OPS_Routine OPS_FourNodeTetrahedron;
extern OPS_Routine OPS_LysmerTriangle;
extern OPS_Routine OPS_ASDAbsorbingBoundary2D; // Massimo Petracca (ASDEA)
extern OPS_Routine OPS_ASDAbsorbingBoundary3D; // Massimo Petracca (ASDEA)
extern OPS_Routine OPS_TwoNodeLink;
extern OPS_Routine OPS_LinearElasticSpring;
extern OPS_Routine OPS_Inerter;
extern OPS_Routine OPS_Adapter;
extern OPS_Routine OPS_Actuator;
extern OPS_Routine OPS_ActuatorCorot;
extern OPS_Routine OPS_ElastomericBearingPlasticity2d;
extern OPS_Routine OPS_ElastomericBearingPlasticity3d;
extern OPS_Routine OPS_ElastomericBearingBoucWen2d;
extern OPS_Routine OPS_ElastomericBearingBoucWen3d;
extern OPS_Routine OPS_ElastomericBearingUFRP2d;
extern OPS_Routine OPS_FlatSliderSimple2d;
extern OPS_Routine OPS_FlatSliderSimple3d;
extern OPS_Routine OPS_SingleFPSimple2d;
extern OPS_Routine OPS_SingleFPSimple3d;
extern OPS_Routine OPS_RJWatsonEQS2d;
extern OPS_Routine OPS_RJWatsonEQS3d;
extern OPS_Routine OPS_RockingBC;
extern OPS_Routine OPS_LehighJoint2d;
extern OPS_Routine OPS_MasonPan12;
extern OPS_Routine OPS_MasonPan3D;

#if 0 // cmp - commented out to eliminate use of TclBasicBuilder
extern int TclBasicBuilder_addFeapTruss(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char ** const argv, Domain *, TclBasicBuilder *, int argStart);
extern int Tcl_addWrapperElement(eleObj *, ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char ** const argv, Domain *, TclBuilder *);
// Added by Quan Gu and Yongdou Liu, et al. on 2018/10/31 (Xiamen University)
int TclBasicBuilder_addWheelRail(ClientData, Tcl_Interp *, int, TCL_Char **, Domain *, TclBasicBuilder *, int);
#endif

extern int TclBasicBuilder_addJoint2D(ClientData, Tcl_Interp *, int,
                                      TCL_Char **const, Domain *);

typedef int (G3_TclElementCommand)(ClientData, Tcl_Interp*, int, const char** const, Domain*, TclBasicBuilder*);

// Zero-length
G3_TclElementCommand TclBasicBuilder_addZeroLength;
G3_TclElementCommand TclBasicBuilder_addZeroLengthContact2D;
G3_TclElementCommand TclBasicBuilder_addZeroLengthContact3D;
G3_TclElementCommand TclBasicBuilder_addZeroLengthRocking;
G3_TclElementCommand TclBasicBuilder_addZeroLengthSection;
G3_TclElementCommand TclBasicBuilder_addZeroLengthND;

G3_TclElementCommand TclBasicBuilder_addBeamWithHinges;
G3_TclElementCommand TclBasicBuilder_addDispBeamColumnInt;
G3_TclElementCommand TclBasicBuilder_addForceBeamColumn;
G3_TclElementCommand TclBasicBuilder_addGradientInelasticBeamColumn;

G3_TclElementCommand TclBasicBuilder_addEnhancedQuad;
G3_TclElementCommand TclBasicBuilder_addNineNodeMixedQuad;
G3_TclElementCommand TclBasicBuilder_addNineNodeQuad;
G3_TclElementCommand TclBasicBuilder_addEightNodeQuad;
G3_TclElementCommand TclBasicBuilder_addFourNodeQuadWithSensitivity;
G3_TclElementCommand TclBasicBuilder_addFourNodeQuad;
G3_TclElementCommand TclBasicBuilder_addFourNodeQuadUP;
G3_TclElementCommand TclBasicBuilder_addNineFourNodeQuadUP;
G3_TclElementCommand TclBasicBuilder_addBBarFourNodeQuadUP;
G3_TclElementCommand TclBasicBuilder_addConstantPressureVolumeQuad;
G3_TclElementCommand TclBasicBuilder_addSixNodeTri;



// Other
G3_TclElementCommand TclBasicBuilder_addJoint3D;
G3_TclElementCommand TclBasicBuilder_addElastic2dGNL;
G3_TclElementCommand TclBasicBuilder_addElement2dYS;
G3_TclElementCommand TclBasicBuilder_addMultipleShearSpring;
G3_TclElementCommand TclBasicBuilder_addMultipleNormalSpring;
G3_TclElementCommand TclBasicBuilder_addKikuchiBearing;
G3_TclElementCommand TclBasicBuilder_addYamamotoBiaxialHDR;
G3_TclElementCommand TclBasicBuilder_addMasonPan12;
G3_TclElementCommand TclBasicBuilder_addMasonPan3D;
G3_TclElementCommand TclBasicBuilder_addBeamGT;

int TclBasicBuilder_addBeamColumnJoint(ClientData, Tcl_Interp *, int, TCL_Char **const, Domain *, int);

Element* TclDispatch_newTri31(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char ** const argv);


// Shells
Element* TclDispatch_newASDShellQ4(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellANDeS(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellDKGQ(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellDKGT(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellMITC4(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellMITC4Thermal(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellMITC9(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellNLDKGQ(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellNLDKGQThermal(ClientData, Tcl_Interp*, int, TCL_Char** const);
Element* TclDispatch_newShellNLDKGT(ClientData, Tcl_Interp*, int, TCL_Char** const);

// Brick
int TclBasicBuilder_addBrickUP(ClientData, Tcl_Interp *, int , TCL_Char ** const);
int TclBasicBuilder_addBBarBrickUP(ClientData, Tcl_Interp *, int , TCL_Char ** const);
int TclBasicBuilder_addTwentyEightNodeBrickUP(ClientData, Tcl_Interp *, int , TCL_Char ** const);
int TclBasicBuilder_addTwentyNodeBrick(ClientData, Tcl_Interp *, int , TCL_Char ** const);
int TclBasicBuilder_addBrick(ClientData, Tcl_Interp *, int , TCL_Char ** const);

Tcl_CmdProc TclBasicBuilder_addGenericCopy;
Tcl_CmdProc TclBasicBuilder_addGenericClient;


int
TclCommand_addElement(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char ** const argv)
{
  G3_Runtime *rt = G3_getRuntime(interp);
  TclBasicBuilder *theTclBuilder = (TclBasicBuilder*)G3_getSafeBuilder(rt);

  assert(clientData != nullptr);
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain *theTclDomain = builder->getDomain();

  OPS_ResetInputNoBuilder(clientData, interp, 2, argc, argv, theTclDomain);

  // check at least two arguments so don't segemnt fault on strcmp
  if (argc < 2) {
    opserr << G3_ERROR_PROMPT << "insufficient arguments, expected:\n";
    opserr << "      element eleType <specific element args> .. \n";
    return TCL_ERROR;
  }

  void* theEle = nullptr;
  Element *theElement = nullptr;
  int ndm = builder->getNDM();

  if (strcasecmp(argv[1], "truss") == 0) {

    theEle = OPS_TrussElement(rt, argc, argv);

    // for backward compatibility
    if (theEle == nullptr) {
      theEle = OPS_TrussSectionElement(rt, argc, argv);
    }

  } else if (strcasecmp(argv[1], "TrussSection") == 0) {

    theEle = OPS_TrussSectionElement(rt, argc, argv);
  }

  else if (strcasecmp(argv[1], "CorotTruss") == 0) {
    theEle = OPS_CorotTrussElement(rt, argc, argv);

    // for backward compatibility
    if (theEle == nullptr)
      theEle = OPS_CorotTrussSectionElement(rt, argc, argv);
  }

  else if (strcmp(argv[1], "CorotTrussSection") == 0) {
    theEle = OPS_CorotTrussSectionElement(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "N4BiaxialTruss") == 0)) {

    theEle = OPS_N4BiaxialTruss(rt, argc, argv);
  }

  else if (strcmp(argv[1], "Truss2") == 0) {
    theEle = OPS_Truss2(rt, argc, argv);
  }

  else if (strcmp(argv[1], "CorotTruss2") == 0) {
    theEle = OPS_CorotTruss2(rt, argc, argv);
  }

  else if (strcmp(argv[1], "InertiaTruss") == 0) {

    theEle = OPS_InertiaTrussElement(rt, argc, argv);
  }

  else if (strcmp(argv[1], "zeroLengthContactNTS2D") == 0) {
    theEle = OPS_ZeroLengthContactNTS2D(rt, argc, argv);
  }

  else if (strcmp(argv[1], "zeroLengthInterface2D") == 0) {
    theEle = OPS_ZeroLengthInterface2D(rt, argc, argv);
  }

  else if (strcmp(argv[1], "componentElement2d") == 0) {
    theEle = OPS_ComponentElement2d(rt, argc, argv);

#if 0
  } else if (strcmp(argv[1],"componentElementDamp2d") == 0) {
    theEle = OPS_ComponentElementDamp2d(rt, argc, argv);
#endif

  } else if (strcmp(argv[1], "zeroLengthImpact3D") == 0) {
    theEle = OPS_ZeroLengthImpact3D(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "ModElasticBeam2d") == 0) ||
             (strcmp(argv[1], "modElasticBeam2d")) == 0) {
    theEle = OPS_ModElasticBeam2d(rt, argc, argv);
  }

  else if ((strcasecmp(argv[1], "elasticBeamColumn") == 0) ||
             (strcasecmp(argv[1], "elasticBeam")) == 0) {

    ID info;
    if (ndm == 2)
      theEle = OPS_ElasticBeam2d(rt, info);
    else
      theEle = OPS_ElasticBeam3d(rt, argc, argv);

  } else if (strcasecmp(argv[1], "PML") == 0) {
    if (ndm == 2)
      theEle = OPS_PML2D(rt, argc, argv);
    else
      theEle = OPS_PML3D(rt, argc, argv);

#if 0
  } else if (strcmp(argv[1], "gradientInelasticBeamColumn") == 0) {

      Element *theEle = 0;
      if (ndm == 2)
        theEle = OPS_GradientInelasticBeamColumn2d(rt, argc, argv);
      else
        theEle = OPS_GradientInelasticBeamColumn3d(rt, argc, argv);

      if (theEle != 0)
        theElement = theEle;
      else {
        
        return TCL_ERROR;
      }
    }
#endif

#if defined(_HAVE_LHNMYS) || defined(OPSDEF_ELEMENT_LHNMYS)
  } else if (strcmp(argv[1], "beamColumn2DwLHNMYS") == 0) {
    theEle = OPS_BeamColumn2DwLHNMYS(rt, argc, argv);
  } else if (strcmp(argv[1], "beamColumn2dDamage") == 0) {
    theEle = OPS_Beam2dDamage(rt, argc, argv);
  } else if (strcmp(argv[1], "beamColumn2DwLHNMYS_Damage") == 0) {
    theEle = OPS_BeamColumn2DwLHNMYS_Damage(rt, argc, argv);
  } else if (strcmp(argv[1], "beamColumn3DwLHNMYS") == 0) {
    theEle = OPS_BeamColumn3DwLHNMYS(rt, argc, argv);
#endif

  } else if (strcmp(argv[1], "ElasticTimoshenkoBeam") == 0) {
    if (ndm == 2)
      theEle = OPS_ElasticTimoshenkoBeam2d(rt, argc, argv);
    else
      theEle = OPS_ElasticTimoshenkoBeam3d(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "pyMacro2D") == 0) ||
             (strcmp(argv[1], "PY_Macro2D") == 0)) {

    theEle = OPS_PY_Macro2D(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "TFPbearing") == 0) ||
             (strcmp(argv[1], "TFP") == 0) ||
             (strcmp(argv[1], "TPFbearing") == 0) ||
             (strcmp(argv[1], "TPF") == 0)) {

    theEle = OPS_TFP_Bearing(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "FPBearingPTV") == 0)) {

    theEle = OPS_FPBearingPTV(rt, argc, argv);
  }

  else if (strcmp(argv[1], "TripleFrictionPendulum") == 0) {

    theEle = OPS_TripleFrictionPendulum(rt, argc, argv);
  }

  else if (strcmp(argv[1], "HDR") == 0) {

    theEle = OPS_HDR(rt, argc, argv);
  }

  else if (strcmp(argv[1], "LeadRubberX") == 0) {

    theEle = OPS_LeadRubberX(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ElastomericX") == 0) {

    theEle = OPS_ElastomericX(rt, argc, argv);
  }

  else if (strcmp(argv[1], "AxEqDispBeamColumn2d") == 0) {

    theEle = OPS_AxEqDispBeamColumn2d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "MVLEM") == 0) { // Kristijan Kolozvari

    theEle = OPS_MVLEM(rt, argc, argv);
  }

  else if (strcmp(argv[1], "SFI_MVLEM") == 0) { // Kristijan Kolozvari

    theEle = OPS_SFI_MVLEM(rt, argc, argv);
  }

  else if (strcmp(argv[1], "MVLEM_3D") == 0) { // Kristijan Kolozvari

    theEle = OPS_MVLEM_3D(rt, argc, argv);
  }

  else if (strcmp(argv[1], "SFI_MVLEM_3D") == 0) { // Kristijan Kolozvari

    theEle = OPS_SFI_MVLEM_3D(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "MasonPan12") == 0)) {

    theEle = OPS_MasonPan12(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "MasonPan3D") == 0)) {

    theEle = OPS_MasonPan3D(rt, argc, argv);

  } else if ((strcmp(argv[1], "BeamGT") == 0)) {

    theEle = OPS_BeamGT(rt, argc, argv);

  } else if ((strcmp(argv[1], "MultiFP2d") == 0) ||
             (strcmp(argv[1], "MultiFPB2d") == 0)) {

    theEle = OPS_MultiFP2d(rt, argc, argv);
  }

// Other
  else if ((strcmp(argv[1], "CoupledZeroLength") == 0) ||
             (strcmp(argv[1], "ZeroLengthCoupled") == 0)) {
    theEle = OPS_CoupledZeroLength(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ZeroLengthVG_HG") == 0) {
    theEle = OPS_ZeroLengthVG_HG(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ZeroLengthContactASDimplex") == 0) {
    theEle = OPS_ZeroLengthContactASDimplex(rt, argc, argv);
  }

  else if (strcmp(argv[1], "twoNodeLink") == 0) {
    theEle = OPS_TwoNodeLink(rt, argc, argv);
  }
  
  else if ((strcmp(argv[1], "SurfaceLoad") == 0)) {
    theEle = OPS_SurfaceLoad(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "TriSurfaceLoad") == 0)) {
    theEle = OPS_TriSurfaceLoad(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "TPB1D") == 0)) {
    theEle = OPS_TPB1D(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "elasticTubularJoint") == 0) ||
           (strcmp(argv[1], "ElasticTubularJoint") == 0)) {
    theEle = OPS_ElasticTubularJoint(rt, argc, argv);
  }

  else if (strcmp(argv[1], "quad3d") == 0) {
    theEle = OPS_FourNodeQuad3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "AC3D8") == 0) {
    theEle = OPS_AC3D8HexWithSensitivity(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ASI3D8") == 0) {
    theEle = OPS_ASID8QuadWithSensitivity(rt, argc, argv);
  }

  else if (strcmp(argv[1], "AV3D4") == 0) {
    theEle = OPS_AV3D4QuadWithSensitivity(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ElastomericBearingBoucWenMod") == 0) {
    theEle = OPS_ElastomericBearingBoucWenMod3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "VS3D4") == 0) {
    theEle = OPS_VS3D4WuadWithSensitivity(rt, argc, argv);
  }

  else if (strcmp(argv[1], "CatenaryCable") == 0) {
    theEle = OPS_CatenaryCableElement(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ASDEmbeddedNodeElement") == 0) {
    theEle = OPS_ASDEmbeddedNodeElement(rt, argc, argv);
  }

  else if (strcmp(argv[1], "LysmerTriangle") == 0) {
    theEle = OPS_LysmerTriangle(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ASDAbsorbingBoundary2D") == 0) {
    theEle = OPS_ASDAbsorbingBoundary2D(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ASDAbsorbingBoundary3D") == 0) {
    theEle = OPS_ASDAbsorbingBoundary3D(rt, argc, argv);
  }

  else if (strcmp(argv[1], "FourNodeTetrahedron") == 0) {
    theEle = OPS_FourNodeTetrahedron(rt, argc, argv);
  }


  else if (strcmp(argv[1], "LinearElasticSpring") == 0) {
    theEle = OPS_LinearElasticSpring(rt, argc, argv);
  }

  else if (strcmp(argv[1], "Inerter") == 0) {
    theEle = OPS_Inerter(rt, argc, argv);
  }

  else if (strcmp(argv[1], "Adapter") == 0) {
    theEle = OPS_Adapter(rt, argc, argv);
  }

  else if (strcmp(argv[1], "Actuator") == 0) {
    theEle = OPS_Actuator(rt, argc, argv);
  }

  else if (strcmp(argv[1], "CorotActuator") == 0) {
    theEle = OPS_ActuatorCorot(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ElastomericBearing") == 0 ||
          (strcmp(argv[1], "ElastomericBearingPlasticity")) == 0) {

    if (ndm == 2)
      theEle = OPS_ElastomericBearingPlasticity2d(rt, argc, argv);
    else
      theEle = OPS_ElastomericBearingPlasticity3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ElastomericBearingBoucWen") == 0 ||
          (strcmp(argv[1], "ElastomericBearingBW")) == 0) {
    if (ndm == 2)
      theEle = OPS_ElastomericBearingBoucWen2d(rt, argc, argv);
    else
      theEle = OPS_ElastomericBearingBoucWen3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "ElastomericBearingUFRP") == 0) {
    if (ndm == 2)
      theEle = OPS_ElastomericBearingUFRP2d(rt, argc, argv);
    else {;}
      // theEle = OPS_ElastomericBearingUFRP3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "FlatSliderBearing") == 0) {
    if (ndm == 2)
      theEle = OPS_FlatSliderSimple2d(rt, argc, argv);
    else
      theEle = OPS_FlatSliderSimple3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "SingleFPBearing") == 0 ||
          (strcmp(argv[1], "SinglePFBearing")) == 0 ||
          (strcmp(argv[1], "SFPBearing")) == 0 ||
          (strcmp(argv[1], "SPFBearing")) == 0) {
    if (ndm == 2)
      theEle = OPS_SingleFPSimple2d(rt, argc, argv);
    else
      theEle = OPS_SingleFPSimple3d(rt, argc, argv);
  }

  else if (strcmp(argv[1], "RJWatsonEqsBearing") == 0 ||
           strcmp(argv[1], "RJWatsonBearing") == 0 ||
           strcmp(argv[1], "EQSBearing") == 0) {
    if (ndm == 2)
      theEle = OPS_RJWatsonEQS2d(rt, argc, argv);
    else
      theEle = OPS_RJWatsonEQS3d(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "RockingBC") == 0)) {
    theEle = OPS_RockingBC(rt, argc, argv);
  }

  // Xinlong Du
  else if ((strcmp(argv[1], "DispBeamColumnAsym") == 0) ||
           (strcmp(argv[1], "DispBeamAsym")) == 0) {
    if (ndm == 3)
      theEle = OPS_DispBeamColumnAsym3dTcl(rt, argc, argv);
  }

  else if ((strcmp(argv[1], "MixedBeamColumnAsym") == 0) ||
           (strcmp(argv[1], "MixedBeamAsym") == 0)) {

    if (ndm == 3)
      theEle = OPS_MixedBeamColumnAsym3dTcl(rt, argc, argv);
  }
  // Xinlong Du


  else if (strcmp(argv[1], "LehighJoint2D") == 0) {
    theEle = OPS_LehighJoint2d(rt, argc, argv);
  }

//
// Shells
//
  else if ((strcmp(argv[1], "Shell") == 0) ||
           (strcmp(argv[1], "ShellMITC4") == 0)) {

    theEle = TclDispatch_newShellMITC4(clientData, interp, argc, argv);

  } else if (strcmp(argv[1], "ShellMITC4Thermal") == 0) {

    theEle = TclDispatch_newShellMITC4Thermal(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ShellNLDKGQThermal") == 0) {

    theEle = TclDispatch_newShellNLDKGQThermal(clientData, interp, argc, argv);

  } else if ((strcmp(argv[1], "ShellNL") == 0) ||
             (strcmp(argv[1], "ShellMITC9") == 0)) {

    theEle = TclDispatch_newShellMITC9(clientData, interp, argc, argv);
  }

  else if ((strcmp(argv[1], "shellDKGQ") == 0) ||
           (strcmp(argv[1], "ShellDKGQ") == 0)) {

    theEle = TclDispatch_newShellDKGQ(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ShellNLDKGQ") == 0) {
    theEle = TclDispatch_newShellNLDKGQ(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ShellDKGT") == 0) {
    theEle = TclDispatch_newShellDKGT(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ShellNLDKGT") == 0) {
    theEle = TclDispatch_newShellNLDKGT(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ASDShellQ4") == 0) {
    theEle = TclDispatch_newASDShellQ4(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "ShellANDeS") == 0) {
    theEle = TclDispatch_newShellANDeS(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "Tri31") == 0) {
    // ID info;
    // theEle = OPS_Tri31(info);
    theEle = TclDispatch_newTri31(clientData, interp, argc, argv);
  }


  // if one of the above worked
  theElement = (Element*)theEle;

  if (theElement != nullptr) {
    if (theTclDomain->addElement(theElement) == false) {
      opserr << "WARNING could not add element of with tag: "
             << theElement->getTag()
             << " and of type: " << theElement->getClassType()
             << " to the Domain\n";
      delete theElement;
      return TCL_ERROR;
    } else
      return TCL_OK;
  }

#if 0 && defined(OPSDEF_ELEMENT_FEAP)
  if (strcmp(argv[1], "fTruss") == 0) {
    int eleArgStart = 1;
    int result = TclBasicBuilder_addFeapTruss(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder, eleArgStart);
    return result;

  }
#endif // _OPS_Element_FEAP

#if 0
    // Beginning of WheelRail element TCL command
    // Added by Quan Gu and Yongdou Liu, et al. on 2018/10/31

  } else if ((strcmp(argv[1], "WheelRail") == 0)) {
    // ------------------------------add------------------------------------------
    int eleArgStart = 1;
    int result = TclBasicBuilder_addWheelRail(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder, eleArgStart);

#endif

  else if (strcmp(argv[1], "dispBeamColumnInt") == 0) {
    int result = TclBasicBuilder_addDispBeamColumnInt(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "ForceBeamColumn") == 0 ||
             strcmp(argv[1], "DispBeamColumn") == 0 ||
             strcmp(argv[1], "DispBeamColumn") == 0 ||
             strcmp(argv[1], "TimoshenkoBeamColumn") == 0 ||
             strcmp(argv[1], "ForceBeamColumnCBDI") == 0 ||
             strcmp(argv[1], "ForceBeamColumnCSBDI") == 0 ||
             strcmp(argv[1], "ForceBeamColumnWarping") == 0 ||
             strcmp(argv[1], "ForceBeamColumnThermal") == 0 ||
             strcmp(argv[1], "ElasticForceBeamColumnWarping") == 0 ||
             strcmp(argv[1], "DispBeamColumnNL") == 0 ||
             strcmp(argv[1], "DispBeamColumnThermal") == 0 ||
             strcmp(argv[1], "ElasticForceBeamColumn") == 0 ||
             strcmp(argv[1], "NonlinearBeamColumn") == 0 ||
             strcmp(argv[1], "DispBeamColumnWithSensitivity") == 0) {

    int result = TclBasicBuilder_addForceBeamColumn(clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if ((strstr(argv[1], "BeamWithHinges") != 0) ||
             (strcmp(argv[1], "BeamWithHinges") != 0)) {
    int result = TclBasicBuilder_addBeamWithHinges(clientData, interp, argc, argv,
                                                   theTclDomain, theTclBuilder);
    return result;

  } else if ((strcmp(argv[1], "Quad") == 0) ||
             (strcmp(argv[1], "stdQuad") == 0)) {
    int result = TclBasicBuilder_addFourNodeQuad(clientData, interp, argc, argv,
                                                 theTclDomain, theTclBuilder);
    return result;


  } else if (strcmp(argv[1], "quadWithSensitivity") == 0) {
    int result = TclBasicBuilder_addFourNodeQuadWithSensitivity(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "enhancedQuad") == 0) {
    int result = TclBasicBuilder_addEnhancedQuad(clientData, interp, argc, argv,
                                                 theTclDomain, theTclBuilder);
    return result;

  } else if ((strcmp(argv[1], "bbarQuad") == 0) ||
             (strcmp(argv[1], "mixedQuad") == 0)) {
    int result = TclBasicBuilder_addConstantPressureVolumeQuad(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if ((strcmp(argv[1], "nineNodeMixedQuad") == 0) ||
             (strcmp(argv[1], "nineNodeQuad") == 0)) {
    int result = TclBasicBuilder_addNineNodeMixedQuad(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "quad9n") == 0) {
    int result = TclBasicBuilder_addNineNodeQuad(clientData, interp, argc, argv,
                                                 theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "quad8n") == 0) {
    int result = TclBasicBuilder_addEightNodeQuad(clientData, interp, argc, argv,
                                                  theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "tri6n") == 0) {
    int result = TclBasicBuilder_addSixNodeTri(clientData, interp, argc, argv,
                                               theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "quadUP") == 0) {
    int result = TclBasicBuilder_addFourNodeQuadUP(clientData, interp, argc, argv,
                                                   theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "9_4_QuadUP") == 0) {
    int result = TclBasicBuilder_addNineFourNodeQuadUP(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "bbarQuadUP") == 0) {
    int result = TclBasicBuilder_addBBarFourNodeQuadUP(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
//
// Brick
//
  } else if (strcmp(argv[1], "BrickUP") == 0) {
    int result = TclBasicBuilder_addBrickUP(clientData, interp, argc, argv);
    return result;

  } else if (strcmp(argv[1], "20_8_BrickUP") == 0) {
    int result = TclBasicBuilder_addTwentyEightNodeBrickUP(clientData, interp, argc, argv);
    return result;

  } else if (strcmp(argv[1], "20NodeBrick") == 0) {
    int result = TclBasicBuilder_addTwentyNodeBrick(clientData, interp, argc, argv);
    return result;

  } else if (strcmp(argv[1], "bbarBrickUP") == 0) {
    int result = TclBasicBuilder_addBBarBrickUP(clientData, interp, argc, argv);
    return result;

  } else if (strcmp(argv[1], "stdBrick") == 0 ||
             strcmp(argv[1], "bbarBrick") == 0 ||
             strcmp(argv[1], "bbarBrickWithSensitivity") == 0 ||
             strcmp(argv[1], "flBrick") == 0) {

    return TclBasicBuilder_addBrick(clientData, interp, argc, argv);
  
  } else if ((strcasecmp(argv[1], "SSPquad")==0)   ||
           (strcasecmp(argv[1], "SSPquadUP")==0) ||
           (strcasecmp(argv[1], "SSPbrick")==0)) {
    int TclCommand_SSP_Element(ClientData, Tcl_Interp*, int, TCL_Char** const);
    return TclCommand_SSP_Element(clientData, interp, argc, argv);
  }
//
// Zero-Length
//
  else if (strcmp(argv[1], "zeroLength") == 0) {
    int result = TclBasicBuilder_addZeroLength(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "zeroLengthSection") == 0) {
    int result = TclBasicBuilder_addZeroLengthSection(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "zeroLengthRocking") == 0) {
    int result = TclBasicBuilder_addZeroLengthRocking(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "zeroLengthContact2D") == 0) {
    int result = TclBasicBuilder_addZeroLengthContact2D(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  } else if (strcmp(argv[1], "zeroLengthContact3D") == 0) {
    int result = TclBasicBuilder_addZeroLengthContact3D(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;

  } else if (strcmp(argv[1], "zeroLengthND") == 0) {
    int result = TclBasicBuilder_addZeroLengthND(clientData, interp, argc, argv,
                                                 theTclDomain, theTclBuilder);
    return result;
  } else if ((strcmp(argv[1], "Joint2D") == 0) ||
             (strcmp(argv[1], "Joint2d") == 0)) {
    int result =
        TclBasicBuilder_addJoint2D(clientData, interp, argc, argv, theTclDomain);

    return result;
  } else if ((strcmp(argv[1], "Joint3D") == 0) ||
             (strcmp(argv[1], "Joint3d") == 0)) {
    int result = TclBasicBuilder_addJoint3D(clientData, interp, argc, argv,
                                            theTclDomain, theTclBuilder);
    return result;
  }

  else if (strcmp(argv[1], "genericClient") == 0) {
    return TclBasicBuilder_addGenericClient(clientData, interp, argc, argv);
  }

  else if (strcmp(argv[1], "genericCopy") == 0) {
    return TclBasicBuilder_addGenericCopy(clientData, interp, argc, argv);

  } else if ((strcmp(argv[1], "inelastic2dYS01") == 0) ||
             (strcmp(argv[1], "inelastic2dYS02") == 0) ||
             (strcmp(argv[1], "inelastic2dYS03") == 0) ||
             (strcmp(argv[1], "inelastic2dYS04") == 0) ||
             (strcmp(argv[1], "inelastic2dYS05") == 0)) {
    int result = TclBasicBuilder_addElement2dYS(clientData, interp, argc, argv,
                                                theTclDomain, theTclBuilder);
    return result;

  } else if ((strcmp(argv[1], "element2dGNL") == 0) ||
             (strcmp(argv[1], "elastic2dGNL") == 0)) {
    int result = TclBasicBuilder_addElastic2dGNL(clientData, interp, argc, argv,
                                                 theTclDomain, theTclBuilder);
    return result;
  }

  else if (strcmp(argv[1], "beamColumnJoint") == 0) {
    int eleArgStart = 1;
    int result = TclBasicBuilder_addBeamColumnJoint(clientData, interp, argc, argv,
                                                    theTclDomain, eleArgStart);
    return result;
  }

  // Kikuchi
  else if ((strcmp(argv[1], "multipleShearSpring") == 0) ||
           (strcmp(argv[1], "MSS") == 0)) {
    int result = TclBasicBuilder_addMultipleShearSpring(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  }

  else if ((strcmp(argv[1], "multipleNormalSpring") == 0) ||
           (strcmp(argv[1], "MNS") == 0)) {
    int result = TclBasicBuilder_addMultipleNormalSpring(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  }

  else if (strcmp(argv[1], "KikuchiBearing") == 0) {
    int result = TclBasicBuilder_addKikuchiBearing(clientData, interp, argc, argv,
                                                   theTclDomain, theTclBuilder);
    return result;
  }

  else if (strcmp(argv[1], "YamamotoBiaxialHDR") == 0) {
    int result = TclBasicBuilder_addYamamotoBiaxialHDR(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  }

  // MSN
  else if (strcmp(argv[1], "gradientInelasticBeamColumn") == 0) {
    int result = TclBasicBuilder_addGradientInelasticBeamColumn(
        clientData, interp, argc, argv, theTclDomain, theTclBuilder);
    return result;
  }

  else {

    //
    // maybe element already loaded as c++ class from a package
    //

    // try existing loaded packages

    ElementPackageCommand *eleCommands = theElementPackageCommands;
    bool found = false;
    int result = TCL_ERROR;
    while (eleCommands != NULL && found == false) {
      if (strcmp(argv[1], eleCommands->funcName) == 0) {

        // OPS_ResetInput(clientData, interp, 2, argc, argv, theTclDomain,
        //                theTclBuilder);
        OPS_ResetInputNoBuilder(clientData, interp, 2, argc, argv, theTclDomain);
        void *theRes = (*(eleCommands->funcPtr))();
        if (theRes != 0) {
          Element *theEle = (Element *)theRes;
          result = theTclDomain->addElement(theEle);

          if (result >= 0)
            return TCL_OK;
          else
            return TCL_ERROR;
        }
        return TCL_ERROR;
        ;
      } else
        eleCommands = eleCommands->next;
    }
#if 0
    //
    // maybe element in a routine, check existing ones or try loading new ones
    //

    char *eleType = new char[strlen(argv[1]) + 1];
    strcpy(eleType, argv[1]);
    eleObj *eleObject = OPS_GetElementType(eleType, (int)strlen(eleType));

    delete[] eleType;

    if (eleObject != 0) {

      int result = Tcl_addWrapperElement(eleObject, clientData, interp, argc, argv,
                                         theTclDomain, theTclBuilder);

      if (result != 0)
        delete eleObject;
      else
        return result;
    }
#endif
    //
    // try loading new dynamic library containing a C++ class
    //

    void *libHandle;
    void *(*funcPtr)();
    int eleNameLength = (int)strlen(argv[1]);
    char *tclFuncName = new char[eleNameLength + 5];
    strcpy(tclFuncName, "OPS_");

    strcpy(&tclFuncName[4], argv[1]);

    opserr << "checking library: " << tclFuncName << endln;
    int res =
        getLibraryFunction(argv[1], tclFuncName, &libHandle, (void **)&funcPtr);

    delete[] tclFuncName;

    if (res == 0) {

      char *eleName = new char[eleNameLength + 1];
      strcpy(eleName, argv[1]);
      ElementPackageCommand *theEleCommand = new ElementPackageCommand;
      theEleCommand->funcPtr = funcPtr;
      theEleCommand->funcName = eleName;
      theEleCommand->next = theElementPackageCommands;
      theElementPackageCommands = theEleCommand;

      // OPS_ResetInput(clientData, interp, 2, argc, argv, theTclDomain,
      //                theTclBuilder);

      OPS_ResetInputNoBuilder(clientData, interp, 2, argc, argv, theTclDomain);
      void *theRes = (*funcPtr)();

      if (theRes != 0) {
        Element *theEle = (Element *)theRes;
        result = theTclDomain->addElement(theEle);
        if (result >= 0)
          return TCL_OK;
        else
          return TCL_ERROR;
      } else {
        return TCL_ERROR;
      }
    }
  }

  // If we get here, the element type is unknown
  opserr << "ERROR -- element of type " << argv[1] << " not known" << endln;
  return TCL_ERROR;
}

int
TclBasicBuilder_addMultipleShearSpring(ClientData clientData, Tcl_Interp *interp,
                                       int argc, TCL_Char ** const argv,
                                       Domain *theTclDomain, 
                                       [[maybe_unused]] TclBasicBuilder* unused)
{
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed - multipleShearSpring\n";
    return TCL_ERROR;
  }

  // 3-dim, 6-dof
  int ndm = builder->getNDM();
  int ndf = builder->getNDF();

  if (ndm != 3 || ndf != 6) {
    opserr << "ndm=" << ndm << ", ndf=" << ndf << endln;
    opserr << "WARNING multipleShearSpring command only works when ndm is 3 "
              "and ndf is 6"
           << endln;
    return TCL_ERROR;
  }

  // arguments (necessary)
  int eleTag;
  int iNode;
  int jNode;
  int nSpring;
  int matTag;

  // material
  UniaxialMaterial *material = nullptr;
  UniaxialMaterial **theMaterials = nullptr;
  int recvMat = 0;

  // arguments (optional)
  double limDisp = 0.0;
  Vector oriX(0);
  Vector oriYp(3);
  oriYp(0) = 0.0;
  oriYp(1) = 1.0;
  oriYp(2) = 0.0;
  double mass = 0.0;

  //
  Element *theElement = nullptr;

  // error flag
  bool ifNoError = true;

  if (argc < 8) { // element multipleShearSpring eleTag? iNode? jNode? nSpring?
                  // -mat matTag?

    opserr << "WARNING insufficient arguments\n";
    ifNoError = false;

  } else {

    // argv[2~5]
    if (Tcl_GetInt(interp, argv[2], &eleTag) != TCL_OK) {
      opserr << "WARNING invalid multipleShearSpring eleTag\n";
      ifNoError = false;
    }

    if (Tcl_GetInt(interp, argv[3], &iNode) != TCL_OK) {
      opserr << "WARNING invalid iNode\n";
      ifNoError = false;
    }

    if (Tcl_GetInt(interp, argv[4], &jNode) != TCL_OK) {
      opserr << "WARNING invalid jNode\n";
      ifNoError = false;
    }

    if (Tcl_GetInt(interp, argv[5], &nSpring) != TCL_OK || nSpring <= 0) {
      opserr << "WARNING invalid nSpring\n";
      ifNoError = false;
    }

    // argv[6~]
    for (int i = 6; i <= (argc - 1); i++) {

      double value;

      if (strcmp(argv[i], "-mat") == 0 &&
          (i + 1) <= (argc - 1)) { // -mat matTag?

        if (Tcl_GetInt(interp, argv[i + 1], &matTag) != TCL_OK) {
          opserr << "WARNING invalid matTag\n";
          ifNoError = false;
        }

        material = builder->getUniaxialMaterial(matTag);
        if (material == 0) {
          opserr << "WARNING material model not found\n";
          opserr << "uniaxialMaterial: " << matTag << endln;
          opserr << "multipleShearSpring element: " << eleTag << endln;
          return TCL_ERROR;
        }

        // opserr << "org material " << material->getClassType() << "\n";
        recvMat++;
        i += 1;

      } else if (strcmp(argv[i], "-nMat") == 0 &&
                 (i + nSpring) <= (argc - 1)) { // -mat matTag?

        theMaterials = new UniaxialMaterial *[nSpring];
        for (int j = 0; j < nSpring; j++) {
          if (Tcl_GetInt(interp, argv[j + i + 1], &matTag) != TCL_OK) {
            opserr << "WARNING invalid matTag\n";
            ifNoError = false;
          }

          theMaterials[j] = builder->getUniaxialMaterial(matTag);
          if (theMaterials[j] == 0) {
            opserr << "WARNING material model not found\n";
            opserr << "uniaxialMaterial: " << matTag << endln;
            opserr << "multipleShearSpring element: " << eleTag << endln;
            return TCL_ERROR;
          }
        }
        recvMat++;
        i += nSpring;

      } else if (strcmp(argv[i], "-orient") == 0 && (i + 6) <= (argc - 1) &&
                 Tcl_GetDouble(interp, argv[i + 4], &value) == TCL_OK) { 
        // <-orient x1? x2? x3? yp1? yp2? yp3?>

        oriX.resize(3);

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriX(j - 1) = value;
          }
        }

        i += 3;

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriYp(j - 1) = value;
          }
        }

        i += 3;

      } else if (strcmp(argv[i], "-orient") == 0 && (i + 3) <= (argc - 1)) { 
        // <-orient yp1? yp2? yp3?> の読み込み

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriYp(j - 1) = value;
          }
        }

        i += 3;

      } else if (strcmp(argv[i], "-mass") == 0 && (i + 1) <= (argc - 1)) { 
        // <-mass m?>

        if (Tcl_GetDouble(interp, argv[i + 1], &mass) != TCL_OK || mass <= 0) {
          opserr << "WARNING invalid mass\n";
          ifNoError = false;
        }

        i += 1;

      } else if (strcmp(argv[i], "-lim") == 0 && (i + 1) <= (argc - 1)) {
        // <-lim limDisp?>

        if (Tcl_GetDouble(interp, argv[i + 1], &limDisp) != TCL_OK || limDisp < 0) {
          opserr << "WARNING invalid limDisp\n";
          ifNoError = false;
        }

        i += 1;

      } else { // invalid option

        opserr << "WARNING invalid optional arguments \n";
        ifNoError = false;
        break;
      }
    }

  } // end input

  // confirm material
  if (recvMat != 1) {
    opserr << "WARNING wrong number of -mat inputs\n";
    opserr << "got " << recvMat << " inputs, but want 1 input\n";
    ifNoError = false;
  }

  // if error detected
  if (!ifNoError) {
    opserr << "Want: element multipleShearSpring eleTag? iNode? jNode? "
              "nSpring? -mat matTag? <-lim dsp> <-orient <x1? x2? x3?> yp1? "
              "yp2? yp3?> <-mass m?>\n";
    return TCL_ERROR;
  }

  // now create the multipleShearSpring
  if (theMaterials == 0) {
    theElement = new MultipleShearSpring(eleTag, iNode, jNode, nSpring,
                                         material, limDisp, oriYp, oriX, mass);
  } else {
    theElement = new MultipleShearSpring(eleTag, iNode, jNode, theMaterials,
                                         nSpring, limDisp, oriYp, oriX, mass);
    delete[] theMaterials;
  }

  // then add the multipleShearSpring to the domain
  if (theTclDomain->addElement(theElement) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "multipleShearSpring element: " << eleTag << endln;
    delete theElement;
    return TCL_ERROR;
  }

  // if get here we have successfully created the multipleShearSpring and added
  // it to the domain
  return TCL_OK;
}

static bool
errDetected(bool ifNoError, const char *msg)
{

 if (ifNoError) {
    opserr << "" << endln;
    opserr << "========================================" << endln;
    opserr << " element : input error detected" << endln;
    opserr << "------------------------------" << endln;
  }
  opserr << "  " << msg << endln;
  return false;
};

int
TclBasicBuilder_addMultipleNormalSpring(ClientData clientData, Tcl_Interp *interp,
                                        int argc, TCL_Char ** const argv,
                                        Domain *theTclDomain, TclBasicBuilder *theTclBuilder)
{

  assert(clientData != nullptr);
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;

  // 3-dim, 6-dof
  int ndm = builder->getNDM();
  int ndf = builder->getNDF();

  if (ndm != 3 || ndf != 6) {
    opserr << "ndm=" << ndm << ", ndf=" << ndf << endln;
    opserr << "WARNING multipleNormalSpring command only works when ndm is 3 "
              "and ndf is 6"
           << endln;
    return TCL_ERROR;
  }

  // arguments (necessary)
  int eleTag;
  int iNode;
  int jNode;
  int nDivide;

  // arguments (necessary, input with -???)
  int matTag;
  UniaxialMaterial *material = nullptr;
  int shape = 0;
  double size;

  // arguments (optional, input with -???)
  double lambda = -1.0;
  Vector oriX(0);
  Vector oriYp(3);
  oriYp(0) = 0.0;
  oriYp(1) = 1.0;
  oriYp(2) = 0.0;
  double mass = 0.0;

  // input comfirmation
  int recvMat = 0;
  int recvShape = 0;
  int recvSize = 0;
  int recvLambda = 0;
  int recvOrient = 0;
  int recvMass = 0;

  //
  Element *theElement = nullptr;

  // error flag
  bool ifNoError = true;

  if (argc < 6) { // element multipleNormalSpring eleTag? iNode? jNode? nDivide?

    ifNoError = errDetected(ifNoError, "insufficient arguments");

  } else {

    // argv[2~5]
    if (Tcl_GetInt(interp, argv[2], &eleTag) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid eleTag");
    }

    if (Tcl_GetInt(interp, argv[3], &iNode) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid iNode");
    }

    if (Tcl_GetInt(interp, argv[4], &jNode) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid jNode");
    }

    if (Tcl_GetInt(interp, argv[5], &nDivide) != TCL_OK || nDivide <= 0) {
      ifNoError = errDetected(ifNoError, "invalid nDivide");
    }

    // argv[6~]
    for (int i = 6; i <= (argc - 1); i++) {

      double value;

      if (strcmp(argv[i], "-mat") == 0 &&
          (i + 1) <= (argc - 1)) { // -mat matTag?

        if (Tcl_GetInt(interp, argv[i + 1], &matTag) != TCL_OK) {
          ifNoError = errDetected(ifNoError, "invalid matTag");
        }

        material = builder->getUniaxialMaterial(matTag);
        if (material == nullptr) {
          ifNoError = errDetected(ifNoError, "material model not found");
        }

        recvMat++;
        i += 1;

      } else if (strcmp(argv[i], "-shape") == 0 &&
                 (i + 1) <= (argc - 1)) { // -shape shape?

        if (strcmp(argv[i + 1], "round") == 0) {
          shape = 1; // round shape
        } else if (strcmp(argv[i + 1], "square") == 0) {
          shape = 2; // square
        } else {
          ifNoError = errDetected(
              ifNoError,
              "invalid shape (\"round\" or \"square\" are available)");
          goto error;
        }

        recvShape++;
        i += 1;

      } else if (strcmp(argv[i], "-size") == 0 &&
                 (i + 1) <= (argc - 1)) { // -size size?

        if (Tcl_GetDouble(interp, argv[i + 1], &size) != TCL_OK || size <= 0) {
          ifNoError = errDetected(ifNoError, "invalid size");
        }

        recvSize++;
        i += 1;

      } else if (strcmp(argv[i], "-lambda") == 0 &&
                 (i + 1) <= (argc - 1)) {
        // <-lambda lambda?>

        if (Tcl_GetDouble(interp, argv[i + 1], &lambda) != TCL_OK || lambda < 0) {
          ifNoError = errDetected(ifNoError, "invalid lambda");
        }

        recvLambda++;
        i += 1;

      } else if (strcmp(argv[i], "-orient") == 0 && (i + 6) <= (argc - 1) &&
                 Tcl_GetDouble(interp, argv[i + 4], &value) ==
                     TCL_OK) {
        // <-orient x1? x2? x3? yp1? yp2? yp3?>

        oriX.resize(3);

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriX(j - 1) = value;
          }
        }

        i += 3;

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriYp(j - 1) = value;
          }
        }

        recvOrient++;
        i += 3;

      } else if (strcmp(argv[i], "-orient") == 0 &&
                 (i + 3) <= (argc - 1)) {
        // <-orient yp1? yp2? yp3?>

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriYp(j - 1) = value;
          }
        }

        recvOrient++;
        i += 3;

      } else if (strcmp(argv[i], "-mass") == 0 &&
                 (i + 1) <= (argc - 1)) {
        // <-mass m?> の読み込み

        if (Tcl_GetDouble(interp, argv[i + 1], &mass) != TCL_OK || mass <= 0) {
          ifNoError = errDetected(ifNoError, "invalid mass");
        }

        recvMass++;
        i += 1;

      } else { // invalid option
        ifNoError = errDetected(ifNoError, "invalid optional arguments");
        break;
      }
    }

  } // end input

  // input cofirmation
  // necessary arguments
  if (recvMat != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -mat inputs (got %d inputs, but want 1 input)",
            recvMat);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvShape != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -shape inputs (got %d inputs, but want 1 input)",
            recvShape);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvSize != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -size inputs (got %d inputs, but want 1 input)",
            recvSize);
    ifNoError = errDetected(ifNoError, buf);
  }

  // optional arguments
  if (recvLambda >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -lambda inputs (got %d inputs, but want 1 input)",
            recvLambda);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvOrient >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -ori inputs (got %d inputs, but want 1 input)",
            recvOrient);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvMass >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -mass inputs (got %d inputs, but want 1 input)",
            recvMass);
    ifNoError = errDetected(ifNoError, buf);
  }

  // if error detected
  if (!ifNoError) {
error:
    opserr << "Want: element multipleNormalSpring eleTag? iNode? jNode? "
              "\n    nDivide? -mat matTag? -shape shape? -size size? <-lambda "
              "\n    lambda?> <-orient <x1? x2? x3?> yp1? yp2? yp3?> <-mass m?>\n";
    opserr << "" << endln;
    return TCL_ERROR;
  }

  // now create the multipleNormalSpring
  theElement = new MultipleNormalSpring(eleTag, iNode, jNode, nDivide,
                    material, shape, size, lambda, oriYp, oriX, mass);

  // then add the multipleNormalSpring to the domain
  if (theTclDomain->addElement(theElement) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "multipleNormalSpring element: " << eleTag << endln;
    delete theElement;
    return TCL_ERROR;
  }

  // if get here we have successfully created the multipleNormalSpring and added
  // it to the domain
  return TCL_OK;
}

int
TclBasicBuilder_addKikuchiBearing(ClientData clientData, Tcl_Interp *interp,
                                  int argc, TCL_Char ** const argv,
                                  Domain *theTclDomain, TclBasicBuilder* unused)
{
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;

  if (builder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed - KikuchiBearing\n";
    return TCL_ERROR;
  }

  // 3-dim, 6dof
  int ndm = builder->getNDM();
  int ndf = builder->getNDF();

  if (ndm != 3 || ndf != 6) {
    opserr << "ndm=" << ndm << ", ndf=" << ndf << endln;
    opserr << "WARNING KikuchiBearing command only works when ndm is 3 and ndf "
              "is 6"
           << endln;
    return TCL_ERROR;
  }

  // arguments (necessary)
  int eleTag;
  int iNode;
  int jNode;

  // arguments (necessary, input with -???)
  int shape = 0;
  double size;
  double totalRubber;
  int nMSS;
  int matMSSTag;
  UniaxialMaterial *matMSS = nullptr;
  int nMNS;
  int matMNSTag;
  UniaxialMaterial *matMNS = nullptr;

  // arguments (optional, input with -???)
  double totalHeight = -1.0; // default: Norm(I->J)
  double limDisp = -1.0;     // default: INF
  double lambda = -1.0;      // default: INF
  Vector oriX(0);            // default: local-x Vec(I->J)
  Vector oriYp(3);
  oriYp(0) = 0.0;
  oriYp(1) = 1.0;
  oriYp(2) = 0.0; // default: global-Y
  double mass = 0.0;
  bool ifPDInput = true;
  bool ifTilt = true;
  double adjCi = 0.5;
  double adjCj = 0.5;
  bool ifBalance = false;
  double limFo = -1.0; // default: INF
  double limFi = -1.0; // default: INF
  int nIter = 1;

  // input comfirmation
  int recvShape = 0;
  int recvSize = 0;
  int recvHeight = 0;
  int recvNMSS = 0;
  int recvMatMSS = 0;
  int recvLimDisp = 0;
  int recvNMNS = 0;
  int recvMatMNS = 0;
  int recvLambda = 0;
  int recvOrient = 0;
  int recvMass = 0;
  int recvIfPD = 0;
  int recvIfTl = 0;
  int recvAdj = 0;
  int recvBal = 0;

  //
  Element *theElement = nullptr;

  // error flag
  bool ifNoError = true;

  if (argc < 5) { // element KikuchiBearing eleTag? iNode? jNode?

    ifNoError = errDetected(ifNoError, "insufficient arguments");

  } else {

    // argv[2~4]
    if (Tcl_GetInt(interp, argv[2], &eleTag) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid eleTag");
    }

    if (Tcl_GetInt(interp, argv[3], &iNode) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid iNode");
    }

    if (Tcl_GetInt(interp, argv[4], &jNode) != TCL_OK) {
      ifNoError = errDetected(ifNoError, "invalid jNode");
    }

    // argv[5~]
    for (int i = 5; i <= (argc - 1); i++) {

      double value;

      if (strcmp(argv[i], "-shape") == 0 &&
          (i + 1) <= (argc - 1)) { // -shape shape?

        if (strcmp(argv[i + 1], "round") == 0) {
          shape = 1; // round
        } else if (strcmp(argv[i + 1], "square") == 0) {
          shape = 2; // square
        } else {
          ifNoError = errDetected(
              ifNoError,
              "invalid shape (\"round\" or \"square\" are available)");
        }

        recvShape++;
        i += 1;

      } else if (strcmp(argv[i], "-size") == 0 && (i + 2) <= (argc - 1)) { 
        // -size size? totalRubber?

        if (Tcl_GetDouble(interp, argv[i + 1], &size) != TCL_OK || size <= 0) {
          ifNoError = errDetected(ifNoError, "invalid size");
        }

        if (Tcl_GetDouble(interp, argv[i + 2], &totalRubber) != TCL_OK ||
            totalRubber <= 0) {
          ifNoError = errDetected(ifNoError, "invalid totalRubber");
        }

        recvSize++;
        i += 2;

      } else if (strcmp(argv[i], "-totalHeight") == 0 && (i + 1) <= (argc - 1)) {
        // -totalHeight totalHeight?

        if (Tcl_GetDouble(interp, argv[i + 1], &totalHeight) != TCL_OK ||
            totalHeight <= 0) {
          ifNoError = errDetected(ifNoError, "invalid totalHeight");
        }

        recvHeight++;
        i += 1;

      } else if (strcmp(argv[i], "-nMSS") == 0 && (i + 1) <= (argc - 1)) {
        // -nMSS nMSS?

        if (Tcl_GetInt(interp, argv[i + 1], &nMSS) != TCL_OK || nMSS <= 0) {
          ifNoError = errDetected(ifNoError, "invalid nMSS");
        }

        recvNMSS++;
        i += 1;

      } else if (strcmp(argv[i], "-matMSS") == 0 && (i + 1) <= (argc - 1)) {
        // -matMSS matMSSTag?

        if (Tcl_GetInt(interp, argv[i + 1], &matMSSTag) != TCL_OK) {
          ifNoError = errDetected(ifNoError, "invalid matMSSTag");
        }

        matMSS = builder->getUniaxialMaterial(matMSSTag);
        if (matMSS == 0) {
          ifNoError =
              errDetected(ifNoError, "material for MSS model not found");
        }

        recvMatMSS++;
        i += 1;

      } else if (strcmp(argv[i], "-limDisp") == 0 &&
                 (i + 1) <= (argc - 1)) {
        // <-limDisp limDisp?>

        if (Tcl_GetDouble(interp, argv[i + 1], &limDisp) != TCL_OK || limDisp < 0) {
          ifNoError = errDetected(ifNoError, "invalid limDisp");
        }

        recvLimDisp++;
        i += 1;

      } else if (strcmp(argv[i], "-nMNS") == 0 &&
                 (i + 1) <= (argc - 1)) { // -nMNS nMNS?

        if (Tcl_GetInt(interp, argv[i + 1], &nMNS) != TCL_OK || nMNS <= 0) {
          ifNoError = errDetected(ifNoError, "invalid nMNS");
        }

        recvNMNS++;
        i += 1;

      } else if (strcmp(argv[i], "-matMNS") == 0 &&
                 (i + 1) <= (argc - 1)) { // -matMNS matMNSTag?

        if (Tcl_GetInt(interp, argv[i + 1], &matMNSTag) != TCL_OK) {
          ifNoError = errDetected(ifNoError, "invalid matMNSTag");
        }

        matMNS = builder->getUniaxialMaterial(matMNSTag);
        if (matMNS == 0) {
          ifNoError =
              errDetected(ifNoError, "material for MNS model not found");
        }

        recvMatMNS++;
        i += 1;

      } else if (strcmp(argv[i], "-lambda") == 0 &&
                 (i + 1) <= (argc - 1)) {
        // <-lambda lambda?>

        if (Tcl_GetDouble(interp, argv[i + 1], &lambda) != TCL_OK || lambda < 0) {
          ifNoError = errDetected(ifNoError, "invalid lambda");
        }

        recvLambda++;
        i += 1;

      } else if (strcmp(argv[i], "-orient") == 0 && (i + 6) <= (argc - 1) &&
                 Tcl_GetDouble(interp, argv[i + 4], &value) ==
                     TCL_OK) {
        // <-orient x1? x2? x3? yp1? yp2? yp3?>

        oriX.resize(3);

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriX(j - 1) = value;
          }
        }

        i += 3;

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriYp(j - 1) = value;
          }
        }

        recvOrient++;
        i += 3;

      } else if (strcmp(argv[i], "-orient") == 0 &&
                 (i + 3) <= (argc - 1)) {
        // <-orient yp1? yp2? yp3?>

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            ifNoError = errDetected(ifNoError, "invalid orient");
          } else {
            oriYp(j - 1) = value;
          }
        }

        recvOrient++;
        i += 3;

      } else if (strcmp(argv[i], "-mass") == 0 &&
                 (i + 1) <= (argc - 1)) {
        // <-mass mass?>

        if (Tcl_GetDouble(interp, argv[i + 1], &mass) != TCL_OK || mass <= 0) {
          ifNoError = errDetected(ifNoError, "invalid mass");
        }

        recvMass++;
        i += 1;

      } else if (strcmp(argv[i], "-noPDInput") == 0) {
        // <-noPDInput>

        ifPDInput = false;

        recvIfPD++;
        i += 0;

      } else if (strcmp(argv[i], "-noTilt") == 0) {
        // <-noTilt>

        ifTilt = false;

        recvIfTl++;
        i += 0;

      } else if (strcmp(argv[i], "-adjustPDOutput") == 0 &&
                 (i + 2) <= (argc - 1)) { // -adjustPDOutput ci? cj?

        if (Tcl_GetDouble(interp, argv[i + 1], &adjCi) != TCL_OK) {
          ifNoError = errDetected(ifNoError, "invalid ci");
        }

        if (Tcl_GetDouble(interp, argv[i + 2], &adjCj) != TCL_OK) {
          ifNoError = errDetected(ifNoError, "invalid cj");
        }

        recvAdj++;
        i += 2;

      } else if (strcmp(argv[i], "-doBalance") == 0 &&
                 (i + 3) <= (argc - 1)) { // -doBalance limFo? limFi? nIter?

        if (Tcl_GetDouble(interp, argv[i + 1], &limFo) != TCL_OK || limFo <= 0) {
          ifNoError = errDetected(ifNoError, "invalid limFo");
        }

        if (Tcl_GetDouble(interp, argv[i + 2], &limFi) != TCL_OK || limFi <= 0) {
          ifNoError = errDetected(ifNoError, "invalid limFi");
        }

        if (Tcl_GetInt(interp, argv[i + 3], &nIter) != TCL_OK || nIter <= 0) {
          ifNoError = errDetected(ifNoError, "invalid nIter");
        }

        ifBalance = true;

        recvBal++;
        i += 3;

      } else { // invalid option

        ifNoError = errDetected(ifNoError, "invalid optional arguments");
        break;
      }
    }

  } // end input

  // input cofirmation
  // necessary arguments
  if (recvShape != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -shape inputs (got %d inputs, but want 1 input)",
            recvShape);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvSize != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -size inputs (got %d inputs, but want 1 input)",
            recvSize);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvNMSS != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -NMSS inputs (got %d inputs, but want 1 input)",
            recvNMSS);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvMatMSS != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -matMSS inputs (got %d inputs, but want 1 input)",
            recvMatMSS);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvNMNS != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -NMNS inputs (got %d inputs, but want 1 input)",
            recvNMNS);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvMatMNS != 1) {
    char buf[100];
    sprintf(buf,
            "wrong number of -matMNS inputs (got %d inputs, but want 1 input)",
            recvMatMNS);
    ifNoError = errDetected(ifNoError, buf);
  }

  // optional arguments
  if (recvHeight >= 2) {
    char buf[100];
    sprintf(
        buf,
        "wrong number of -totalHeight inputs (got %d inputs, but want 1 input)",
        recvHeight);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvLimDisp >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -limDisp inputs (got %d inputs, but want 1 input)",
            recvLimDisp);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvLambda >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -lambda inputs (got %d inputs, but want 1 input)",
            recvLambda);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvOrient >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -ori inputs (got %d inputs, but want 1 input)",
            recvOrient);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvMass >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -mass inputs (got %d inputs, but want 1 input)",
            recvMass);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvIfPD >= 2) {
    char buf[100];
    sprintf(
        buf,
        "wrong number of -noPDInput inputs (got %d inputs, but want 1 input)",
        recvIfPD);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvIfTl >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -noTilt inputs (got %d inputs, but want 1 input)",
            recvIfTl);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvAdj >= 2) {
    char buf[100];
    sprintf(buf,
            "wrong number of -adjustPDOutput inputs (got %d inputs, but want 1 "
            "input)",
            recvAdj);
    ifNoError = errDetected(ifNoError, buf);
  }

  if (recvBal >= 2) {
    char buf[100];
    sprintf(
        buf,
        "wrong number of -doBalance inputs (got %d inputs, but want 1 input)",
        recvBal);
    ifNoError = errDetected(ifNoError, buf);
  }

  // if error detected
  if (!ifNoError) {
    opserr << "Want: element KikuchiBearing eleTag? iNode? jNode?\n";
    opserr << "                             -shape shape? -size size? "
              "totalRubber? <-totalHeight totalHeight?>\n";
    opserr << "                             -nMSS nMSS? -matMSS matMSSTag? "
              "<-lim limDisp?>\n";
    opserr << "                             -nMNS nMNS? -matMNS matMNSTag? "
              "<-lambda lambda?>\n";
    opserr << "                             <-orient <x1? x2? x3?> yp1? yp2? "
              "yp3?> <-mass m?>\n";
    opserr << "                             <-noPDInput> <-noTilt> "
              "<-adjustPDOutput ci? cj?> <-doBalance limFo? limFi? nIter?>\n";
    opserr << "" << endln;
    return TCL_ERROR;
  }

  // now create the KikuchiBearing
  theElement = new KikuchiBearing(
      eleTag, iNode, jNode, shape, size, totalRubber, totalHeight, nMSS, matMSS,
      limDisp, nMNS, matMNS, lambda, oriYp, oriX, mass, ifPDInput, ifTilt,
      adjCi, adjCj, ifBalance, limFo, limFi, nIter);

  // then add the KikuchiBearing to the domain
  if (theTclDomain->addElement(theElement) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "KikuchiBearing element: " << eleTag << endln;
    delete theElement;
    return TCL_ERROR;
  }

  return TCL_OK;
}

int
TclBasicBuilder_addYamamotoBiaxialHDR(ClientData clientData, Tcl_Interp *interp,
                                      int argc, TCL_Char ** const argv,
                                      [[maybe_unused]] Domain *theTclDomain_, 
                                      [[maybe_unused]] TclBasicBuilder *unused)
{
  assert(clientData != nullptr);
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;
  Domain *theTclDomain = builder->getDomain();
  

  // 3-dim, 6-dof
  int ndm = builder->getNDM();
  int ndf = builder->getNDF();

  if (ndm != 3 || ndf != 6) {
    opserr << "ndm=" << ndm << ", ndf=" << ndf << endln;
    opserr << "WARNING YamamotoBiaxialHDR command only works when ndm is 3 and "
              "ndf is 6" << endln;
    return TCL_ERROR;
  }

  // arguments (necessary)
  int eleTag;
  int iNode;
  int jNode;

  int Tp = 1;
  double DDo;
  double DDi;
  double Hr;

  // arguments (optional)
  double Cr = 1.0;
  double Cs = 1.0;
  Vector oriX(0);
  Vector oriYp(3);
  oriYp(0) = 0.0;
  oriYp(1) = 1.0;
  oriYp(2) = 0.0;
  double mass = 0.0;

  //
  Element *theElement = 0;

  // error flag
  bool ifNoError = true;

  if (argc < 9) { 
    // element YamamotoBiaxialHDR eleTag? iNode? jNode? Tp? DDo? DDi? Hr?
    opserr << "WARNING insufficient arguments\n";
    ifNoError = false;

  } else {
    // argv[2~8]
    if (Tcl_GetInt(interp, argv[2], &eleTag) != TCL_OK) {
      opserr << "WARNING invalid YamamotoBiaxialHDR eleTag\n";
      ifNoError = false;
    }

    // iNode
    if (Tcl_GetInt(interp, argv[3], &iNode) != TCL_OK) {
      opserr << "WARNING invalid iNode\n";
      ifNoError = false;
    }

    // jNode
    if (Tcl_GetInt(interp, argv[4], &jNode) != TCL_OK) {
      opserr << "WARNING invalid jNode\n";
      ifNoError = false;
    }

    // Tp
    if (strcmp(argv[5], "1") == 0) {
      Tp = 1; // Bridgestone X0.6R (EESD version)
    } else {
      opserr << "WARNING invalid YamamotoBiaxialHDR Tp" << endln;
      ifNoError = false;
    }

    // DDo
    if (Tcl_GetDouble(interp, argv[6], &DDo) != TCL_OK || DDo <= 0.0) {
      opserr << "WARNING invalid YamamotoBiaxialHDR DDo" << endln;
      ifNoError = false;
    }

    // DDi
    if (Tcl_GetDouble(interp, argv[7], &DDi) != TCL_OK || DDi < 0.0) {
      opserr << "WARNING invalid YamamotoBiaxialHDR DDi" << endln;
      ifNoError = false;
    }

    // Hr
    if (Tcl_GetDouble(interp, argv[8], &Hr) != TCL_OK || Hr <= 0.0) {
      opserr << "WARNING invalid YamamotoBiaxialHDR Hr" << endln;
      ifNoError = false;
    }

    // argv[9~]
    for (int i = 9; i <= (argc - 1); i++) {
      double value;

      if (strcmp(argv[i], "-orient") == 0 && (i + 6) <= (argc - 1) &&
          Tcl_GetDouble(interp, argv[i + 4], &value) == TCL_OK) {
        // <-orient x1? x2? x3? yp1? yp2? yp3?>

        oriX.resize(3);

        // x1, x2, x3
        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriX(j - 1) = value;
          }
        }

        i += 3;

        // yp1, yp2, yp3
        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriYp(j - 1) = value;
          }
        }

        i += 3;

      } else if (strcmp(argv[i], "-orient") == 0 && (i + 3) <= (argc - 1)) {
        // <-orient yp1? yp2? yp3?>

        for (int j = 1; j <= 3; j++) {
          if (Tcl_GetDouble(interp, argv[i + j], &value) != TCL_OK) {
            opserr << "WARNING invalid -orient value\n";
            ifNoError = false;
          } else {
            oriYp(j - 1) = value;
          }
        }

        i += 3;

      } else if (strcmp(argv[i], "-mass") == 0 && (i + 1) <= (argc - 1)) {
        // <-mass m?>

        if (Tcl_GetDouble(interp, argv[i + 1], &mass) != TCL_OK || mass <= 0) {
          opserr << "WARNING invalid mass\n";
          ifNoError = false;
        }

        i += 1;

      } else if (strcmp(argv[i], "-coRS") == 0 && (i + 2) <= (argc - 1)) {
        // <-coRS cr? cs?>

        if (Tcl_GetDouble(interp, argv[i + 1], &Cr) != TCL_OK || Cr <= 0) {
          opserr << "WARNING invalid cr\n";
          ifNoError = false;
        }
        if (Tcl_GetDouble(interp, argv[i + 2], &Cs) != TCL_OK || Cs <= 0) {
          opserr << "WARNING invalid cs\n";
          ifNoError = false;
        }

        i += 2;

      } else {

        opserr << "WARNING invalid optional arguments \n";
        ifNoError = false;
        break;
      }
    }

  } // end input

  if (!ifNoError) {
    // want:
    opserr << "Want: element YamamotoBiaxialHDR eleTag? iNode? jNode? Tp? DDo? "
              "DDi? Hr?  <-coRS cr? cs?> <-orient <x1? x2? x3?> y1? y2? y3?> "
              "<-mass m?>\n";
    return TCL_ERROR;
  }

  // now create the YamamotoBiaxialHDR
  theElement = new YamamotoBiaxialHDR(eleTag, iNode, jNode, Tp, DDo, DDi, Hr,
                                      Cr, Cs, oriYp, oriX, mass);

  // then add the YamamotoBiaxialHDR to the domain
  if (theTclDomain->addElement(theElement) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "YamamotoBiaxialHDR element: " << eleTag << endln;
    delete theElement;
    return TCL_ERROR;
  }

  // if get here we have successfully created the YamamotoBiaxialHDR and added
  // it to the domain
  return TCL_OK;
}

int
TclBasicBuilder_addWheelRail(ClientData clientData, Tcl_Interp *interp, int argc,
                             TCL_Char ** const argv, Domain *theTclDomain,
                             TclBasicBuilder *theTclBuilder, int eleArgStart)
{
  G3_Runtime *rt = G3_getRuntime(interp);
  assert(clientData != nullptr);
  BasicModelBuilder *builder = (BasicModelBuilder*)clientData;

  if (theTclBuilder == 0 || clientData == 0) {
    opserr << "WARNING builder has been destroyed - elasticBeamColumn \n";
    return TCL_ERROR;
  }

  int ndm = builder->getNDM();
  int ndf = builder->getNDF();

  Element *theElement = nullptr;

  int pTag, pnLoad;
  //-------------Beginning of a 2D wheel-rail element(By Quan Gu, Yongdou Liu,
  // et al.) on 2018/10/29
  if (ndm == 2) {

    // check plane frame problem has 3 dof per node
    if (ndf != 3) {
      opserr << "WARNING invalid ndf: " << ndf;
      opserr << ", for plane problem need 3 - elasticBeamColumn \n";
      return TCL_ERROR;
    }

    // check the number of arguments
    if ((argc - eleArgStart) < 8) {
      opserr << "WARNING bad command - want: elasticBeamColumn beamId iNode "
                "jNode A E I <alpha> <d> transTag <-mass m> <-cMass>\n";
      return TCL_ERROR;
    }

    // get the id, end nodes, and section properties
    int pNd1, transTag;

    double pDeltT, pVel, pInitLocation, pRWheel, pI, pE, pA;

    if (Tcl_GetInt(interp, argv[1 + eleArgStart], &pTag) != TCL_OK) {
      opserr << "WARNING invalid pTag: " << argv[1 + eleArgStart];
      opserr << " - WheelRail pTag iNode jNode";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[2 + eleArgStart], &pDeltT) != TCL_OK) {
      opserr << "WARNING invalid pDeltT - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[3 + eleArgStart], &pVel) != TCL_OK) {
      opserr << "WARNING invalid pVel - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[4 + eleArgStart], &pInitLocation) != TCL_OK) {
      opserr << "WARNING invalid pInitLocation - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetInt(interp, argv[5 + eleArgStart], &pNd1) != TCL_OK) {
      opserr << "WARNING invalid pNd1 - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[6 + eleArgStart], &pRWheel) != TCL_OK) {
      opserr << "WARNING invalid pRWheel - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[7 + eleArgStart], &pI) != TCL_OK) {
      opserr << "WARNING invalid pI - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[8 + eleArgStart], &pE) != TCL_OK) {
      opserr << "WARNING invalid pE - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetDouble(interp, argv[9 + eleArgStart], &pA) != TCL_OK) {
      opserr << "WARNING invalid pA - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }

    if (Tcl_GetInt(interp, argv[10 + eleArgStart], &transTag) != TCL_OK) {
      opserr << "WARNING invalid transTag - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }
    CrdTransf *theTransRWheel = builder->getCrdTransf(transTag);

    if (Tcl_GetInt(interp, argv[11 + eleArgStart], &pnLoad) != TCL_OK) {
      opserr << "WARNING invalid I - WheelRail " << pTag
             << " iNode jNode A E I\n";
      return TCL_ERROR;
    }
    //----------------------------------
    Vector *pNodeList = 0;
    Vector *pDeltaYList = 0;
    Vector *pDeltaYLocationList = 0;

    if (strcmp(argv[12 + eleArgStart], "-NodeList") == 0) {
      int pathSize;
      TCL_Char **pathStrings;

      // int debug =
      //     Tcl_SplitList(interp, argv[13 + eleArgStart], &pathSize, &pathStrings);

      if (Tcl_SplitList(interp, argv[13 + eleArgStart], &pathSize, &pathStrings) !=
          TCL_OK) {
        opserr << "WARNING problem splitting path list "
               << argv[13 + eleArgStart] << " - ";
        opserr << " NodeList -values {path} ... \n";
        return TCL_OK;
      }
      pNodeList = new Vector(pathSize);
      for (int i = 0; i < pathSize; i++) {
        double value;
        // int debug = Tcl_GetDouble(interp, pathStrings[i], &value);
        if (Tcl_GetDouble(interp, pathStrings[i], &value) != TCL_OK) {
          opserr << "WARNING problem reading path data value " << pathStrings[i]
                 << " - ";
          opserr << " -strain {path} ... \n";
          return 0;
        }
        (*pNodeList)(i) = value;
      } // for
    }
    if (strcmp(argv[14 + eleArgStart], "-DeltaYList") == 0) {
      int pathSize;
      TCL_Char **pathStrings;
      if (Tcl_SplitList(interp, argv[15 + eleArgStart], &pathSize, &pathStrings) !=
          TCL_OK) {
        opserr << "WARNING problem splitting path list "
               << argv[15 + eleArgStart] << " - ";
        opserr << " NodeList -values {path} ... \n";
        return TCL_OK;
      }
      pDeltaYList = new Vector(pathSize);
      for (int i = 0; i < pathSize; i++) {
        double value;
        if (Tcl_GetDouble(interp, pathStrings[i], &value) != TCL_OK) {
          opserr << "WARNING problem reading path data value " << pathStrings[i]
                 << " - ";
          opserr << " -strain {path} ... \n";
          return 0;
        }
        (*pDeltaYList)(i) = value;
      } // for
    }
    if (strcmp(argv[16 + eleArgStart], "-LocationList") == 0) {
      int pathSize;
      TCL_Char **pathStrings;
      if (Tcl_SplitList(interp, argv[17 + eleArgStart], &pathSize, &pathStrings) !=
          TCL_OK) {
        opserr << "WARNING problem splitting path list "
               << argv[17 + eleArgStart] << " - ";
        opserr << " NodeList -values {path} ... \n";
        return TCL_OK;
      }
      pDeltaYLocationList = new Vector(pathSize);
      for (int i = 0; i < pathSize; i++) {
        double value;
        if (Tcl_GetDouble(interp, pathStrings[i], &value) != TCL_OK) {
          opserr << "WARNING problem reading path data value " << pathStrings[i]
                 << " - ";
          opserr << " -strain {path} ... \n";
          return 0;
        }
        (*pDeltaYLocationList)(i) = value;
      }
    }
    theElement = new WheelRail(pTag, pDeltT, pVel, pInitLocation, pNd1, pRWheel,
                               pI, pE, pA, theTransRWheel, pnLoad, pNodeList,
                               pDeltaYList, pDeltaYLocationList);

  } 
  // -- End of a 2D wheel-rail element(By Quan Gu, Yongdou Liu, et al.) on 2018/10/29

  else if (ndm == 3) {

    opserr << G3_ERROR_PROMPT << "Unimplemented." << endln;
    return TCL_ERROR;
  }

  // add the WheelRail element to the Domain
  if (theTclDomain->addElement(theElement) == false) {
    opserr << "WARNING could not add element to the domain\n";
    opserr << "YamamotoBiaxialHDR element: " << pTag << endln;
    delete theElement;
    return TCL_ERROR;
  }

  return 0;
}


#include <Tri31.h>
Element *
TclDispatch_newTri31(ClientData clientData, Tcl_Interp *interp, int argc, TCL_Char **const argv)
{
  BasicModelBuilder* builder = (BasicModelBuilder*)clientData;

  // Pointer to an element that will be returned
  Element *theElement = nullptr;
  
  if (argc < 7) {
    opserr << "Invalid #args, want: element element Tri31 eleTag? iNode? jNode? kNode? thk? type? matTag? <pressure? rho? b1? b2?>\n";
    return nullptr;
  }

  int iData[5];
  char *theType;
  double dData[5];
  dData[1] = 0.0;
  dData[2] = 0.0;
  dData[3] = 0.0;
  dData[4] = 0.0;
  
  int numData = 4;
  if (OPS_GetIntInput(&numData, iData) != 0) {
    opserr << "WARNING invalid integer data: element Tri31\n";
    return 0;
  }
  
  numData = 1;
  if (OPS_GetDoubleInput(&numData, dData) != 0) {
    opserr << "WARNING invalid thickness data: element Tri31 " << iData[0] << endln;
    return 0;
  }
  
  theType = (char*)OPS_GetString();
  
  numData = 1;
  if (OPS_GetIntInput(&numData, &iData[4]) != 0) {
    opserr << "WARNING invalid integer data: element Tri31\n";
    return 0;
  }
  int matID = iData[4];
  
  NDMaterial *theMaterial = builder->getNDMaterial(matID);
  if (theMaterial == 0) {
    opserr << "WARNING element Tri31 " << iData[0] << endln;
    opserr << " Material: " << matID << "not found\n";
    return 0;
  }
  
  if (argc == 11) {
    numData = 4;
    if (OPS_GetDoubleInput(&numData, &dData[1]) != 0) {
      opserr << "WARNING invalid optional data: element Tri31 " << iData[0] << endln;
      return 0;
    }
  }
  
  // parsing was successful, allocate the element
  theElement = new Tri31(iData[0], iData[1], iData[2], iData[3],
			 *theMaterial, theType, 
			 dData[0], dData[1], dData[2], dData[3], dData[4]);
  return theElement;
}

