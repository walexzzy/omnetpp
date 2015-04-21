//==========================================================================
//  CANVASINSPECTOR.CC - part of
//
//                     OMNeT++/OMNEST
//            Discrete System Simulation in C++
//
//==========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2015 Andras Varga
  Copyright (C) 2006-2015 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "common/stringutil.h"
#include "canvasinspector.h"
#include "figurerenderers.h"
#include "tklib.h"
#include "tkutil.h"
#include "inspectorfactory.h"
#include "canvasrenderer.h"

NAMESPACE_BEGIN
namespace qtenv {


class CanvasInspectorFactory : public InspectorFactory
{
  public:
    CanvasInspectorFactory(const char *name) : InspectorFactory(name) {}

    bool supportsObject(cObject *obj) {return dynamic_cast<cCanvas *>(obj)!=NULL;}
    int getInspectorType() {return INSP_GRAPHICAL;}
    double getQualityAsDefault(cObject *object) {return 3.0;}
    Inspector *createInspector() {return new CanvasInspector(this);}
};

Register_InspectorFactory(CanvasInspectorFactory);


CanvasInspector::CanvasInspector(InspectorFactory *f) : Inspector(f)
{
    canvasRenderer = new CanvasRenderer();
}

CanvasInspector::~CanvasInspector()
{
    delete canvasRenderer;
}

void CanvasInspector::doSetObject(cObject *obj)
{
    if (obj == object)
        return;

    Inspector::doSetObject(obj);

    canvasRenderer->setCanvas(getCanvas());

    CHK(Tcl_VarEval(interp, canvas, " delete all",NULL));

    if (object)
        CHK(Tcl_VarEval(interp, "CanvasInspector:onSetObject ", windowName, NULL ));
}

void CanvasInspector::createWindow(const char *window, const char *geometry)
{
    Inspector::createWindow(window, geometry);

    strcpy(canvas,windowName);
    strcat(canvas,".c");

    CHK(Tcl_VarEval(interp, "createCanvasInspector ", windowName, " ", TclQuotedString(geometry).get(), NULL ));

    canvasRenderer->setTkCanvas(interp, canvas);
}

void CanvasInspector::useWindow(const char *window)
{
    Inspector::useWindow(window);

    strcpy(canvas,windowName);
    strcat(canvas,".c");

    canvasRenderer->setTkCanvas(interp, canvas);
}

void CanvasInspector::refresh()
{
    Inspector::refresh();

    if (!object)
    {
        CHK(Tcl_VarEval(interp, canvas," delete all", NULL));
        return;
    }

    updateBackgroundColor();

    FigureRenderingHints hints;
    fillFigureRenderingHints(&hints);
    canvasRenderer->refresh(&hints);
}

void CanvasInspector::redraw()
{
    updateBackgroundColor();

    FigureRenderingHints hints;
    fillFigureRenderingHints(&hints);
    canvasRenderer->redraw(&hints);
}

void CanvasInspector::clearObjectChangeFlags()
{
    if (object)
        getCanvas()->getRootFigure()->clearChangeFlags();
}

cCanvas *CanvasInspector::getCanvas()
{
    return static_cast<cCanvas *>(object);
}

void CanvasInspector::fillFigureRenderingHints(FigureRenderingHints *hints)
{
    const char *s;

    // read $inspectordata($c:zoomfactor)
    s = Tcl_GetVar2(interp, "inspectordata", TCLCONST((std::string(canvas)+":zoomfactor").c_str()), TCL_GLOBAL_ONLY);
    hints->zoom = opp_atof(s);

    // read $inspectordata($c:imagesizefactor)
    s = Tcl_GetVar2(interp, "inspectordata", TCLCONST((std::string(canvas)+":imagesizefactor").c_str()), TCL_GLOBAL_ONLY);
    hints->iconMagnification = opp_atof(s);

    // read $inspectordata($c:showlabels)
    s = Tcl_GetVar2(interp, "inspectordata", TCLCONST((std::string(canvas)+":showlabels").c_str()), TCL_GLOBAL_ONLY);
    hints->showSubmoduleLabels = opp_atol(s) != 0;

    // read $inspectordata($c:showarrowheads)
    s = Tcl_GetVar2(interp, "inspectordata", TCLCONST((std::string(canvas)+":showarrowheads").c_str()), TCL_GLOBAL_ONLY);
    hints->showArrowHeads = opp_atol(s) != 0;

    Tcl_Eval(interp, "font actual CanvasFont -family");
    hints->defaultFont = Tcl_GetStringResult(interp);

    Tcl_Eval(interp, "font actual CanvasFont -size");
    s = Tcl_GetStringResult(interp);
    hints->defaultFontSize = opp_atol(s) * 16 / 10;  //FIXME figure out conversion factor (point to pixel?)...
}

void CanvasInspector::updateBackgroundColor()
{
    cCanvas *canvas = getCanvas();
    if (canvas) {
        char buf[16];
        cFigure::Color color = canvas->getBackgroundColor();
        sprintf(buf, "#%2.2x%2.2x%2.2x", color.red, color.green, color.blue);
        CHK(Tcl_VarEval(interp, this->canvas, " config -bg {", buf, "}", NULL));
    }
}

int CanvasInspector::inspectorCommand(int argc, const char **argv)
{
    if (argc<1) {Tcl_SetResult(interp, TCLCONST("wrong number of args"), TCL_STATIC); return TCL_ERROR;}

    E_TRY
    if (strcmp(argv[0],"redraw")==0) {
        redraw();
        return TCL_OK;
    }
    if (strcmp(argv[0],"getalltags")==0) {
        Tcl_SetResult(interp, TCLCONST(canvasRenderer->getAllTags().c_str()), TCL_VOLATILE);
        return TCL_OK;
    }
    if (strcmp(argv[0],"getenabledtags")==0) {
        Tcl_SetResult(interp, TCLCONST(canvasRenderer->getEnabledTags().c_str()), TCL_VOLATILE);
        return TCL_OK;
    }
    if (strcmp(argv[0],"getexcepttags")==0) {
        Tcl_SetResult(interp, TCLCONST(canvasRenderer->getExceptTags().c_str()), TCL_VOLATILE);
        return TCL_OK;
    }
    if (strcmp(argv[0],"setenabledtags")==0) {
        if (argc!=2) {Tcl_SetResult(interp, TCLCONST("wrong number of args"), TCL_STATIC); return TCL_ERROR;}
        canvasRenderer->setEnabledTags(argv[1]);
        return TCL_OK;
    }
    if (strcmp(argv[0],"setexcepttags")==0) {
        if (argc!=2) {Tcl_SetResult(interp, TCLCONST("wrong number of args"), TCL_STATIC); return TCL_ERROR;}
        canvasRenderer->setExceptTags(argv[1]);
        return TCL_OK;
    }
    E_CATCH

    return Inspector::inspectorCommand(argc, argv);
}

} //namespace
NAMESPACE_END
