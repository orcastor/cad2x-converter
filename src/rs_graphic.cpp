/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
** Copyright (C) 2016 ravas (github.com/r-a-v-a-s)
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/

#include <iostream>
#include <cmath>

#include <QDir>

#include "rs_graphic.h"

#include "dxf_format.h"
#include "lc_defaults.h"
#include "rs_block.h"
#include "rs_debug.h"
// #include "rs_dialogfactory.h"
#include "rs_fileio.h"
#include "rs_layer.h"
#include "rs_math.h"
#include "rs_units.h"


/**
 * Default constructor.
 */
RS_Graphic::RS_Graphic(RS_EntityContainer* parent)
        : RS_Document(parent),
        layerList(),
        blockList(true),
        paperScaleFixed(false),
        marginLeft(0.0),
        marginTop(0.0),
        marginRight(0.0),
        marginBottom(0.0),
        pagesNumH(1),
        pagesNumV(1)
{
    setUnit(RS_Units::stringToUnit("None"));
    addVariable("$SNAPSTYLE",static_cast<int>(0),70);
    crosshairType=static_cast<RS2::CrosshairType>(0);
    RS2::Unit unit = getUnit();

    if (unit==RS2::Inch) {
        addVariable("$DIMASZ", 0.1, 40);
        addVariable("$DIMEXE", 0.05, 40);
        addVariable("$DIMEXO", 0.025, 40);
        addVariable("$DIMGAP", 0.025, 40);
        addVariable("$DIMTXT", 0.1, 40);
    } else {
        addVariable("$DIMASZ",
                    RS_Units::convert(2.5, RS2::Millimeter, unit), 40);
        addVariable("$DIMEXE",
                    RS_Units::convert(1.25, RS2::Millimeter, unit), 40);
        addVariable("$DIMEXO",
                    RS_Units::convert(0.625, RS2::Millimeter, unit), 40);
        addVariable("$DIMGAP",
                    RS_Units::convert(0.625, RS2::Millimeter, unit), 40);
        addVariable("$DIMTXT",
                    RS_Units::convert(2.5, RS2::Millimeter, unit), 40);
    }
    addVariable("$DIMTIH", 0, 70);
    //initialize printer vars bug #3602444
    setPaperScale(getPaperScale());
    setPaperInsertionBase(getPaperInsertionBase());

	//set default values for point style
	addVariable("$PDMODE", LC_DEFAULTS_PDMode, DXF_FORMAT_GC_PDMode);
	addVariable("$PDSIZE", LC_DEFAULTS_PDSize, DXF_FORMAT_GC_PDSize);
}



/**
 * Destructor.
 */
RS_Graphic::~RS_Graphic() = default;



/**
 * Counts the entities on the given layer.
 */
unsigned long int RS_Graphic::countLayerEntities(RS_Layer* layer) {

    int c=0;

	if (layer) {
        for(RS_Entity* t: entities){

			if (t->getLayer() &&
                    t->getLayer()->getName()==layer->getName()) {
                c+=t->countDeep();
            }
        }
    }

    return c;
}



/**
 * Removes the given layer and undoes all entities on it.
 */
void RS_Graphic::removeLayer(RS_Layer* layer) {

    if (layer && layer->getName()!="0") {

		std::vector<RS_Entity*> toRemove;
		//find entities on layer
        for(RS_Entity* e: entities){
			if (e->getLayer() &&
                    e->getLayer()->getName()==layer->getName()) {
				toRemove.push_back(e);
            }
        }
		// remove all entities on that layer:
		if(toRemove.size()){
            for(RS_Entity* e: toRemove){
				e->setLayer("0");
			}
		}

		toRemove.clear();
        // remove all entities in blocks that are on that layer:
		for(RS_Block* blk: blockList){
			if(!blk) continue;
			for(auto e: *blk){

				if (e->getLayer() &&
						e->getLayer()->getName()==layer->getName()) {
					toRemove.push_back(e);
				}
			}
		}

        for(RS_Entity* e: toRemove){
			e->setLayer("0");
		}

        layerList.remove(layer);
    }
}


/**
 * Clears all layers, blocks and entities of this graphic.
 * A default layer (0) is created.
 */
void RS_Graphic::newDoc() {

    RS_DEBUG->print("RS_Graphic::newDoc");

    clear();

    clearLayers();
    clearBlocks();

    addLayer(new RS_Layer("0"));
    //addLayer(new RS_Layer("ByBlock"));
}

/*
 *	Description:	Saves this graphic with the current filename and settings.
 *	Author(s):		..., Claude Sylvain
 * Last modified:	13 July 2011
 *	Parameters:
 *
 *	Returns:			bool:
 *							false:	Operation failed.
 *							true:		Operation successful.
 *
 * Notes:			- If this is not an AutoSave, backup the drawing file
 * 					  (if necessary).
 * 					- Drawing is saved only when it has been modified.
 * 					  This prevent lost of backup file when file
 * 					  is saved more than one time without being modified.
 */

bool RS_Graphic::save()
{
    bool ret	= false;

    RS_DEBUG->print("RS_Graphic::save: Entering...");

    /*	- Save drawing file only if it has been modified.
    *	- Notes: Potentially dangerous in case of an internal
    *	  coding error that make LibreCAD not aware of modification
    *	  when some kind of drawing modification is done.
    *	----------------------------------------------------------- */
    QString actualName = filename;
    RS2::FormatType	actualType = formatType;

    /*	Save drawing file if able to created associated object.
                *	------------------------------------------------------- */
    if (!actualName.isEmpty())
    {
        RS_DEBUG->print("RS_Graphic::save: File: %s", actualName.toLatin1().data());
        RS_DEBUG->print("RS_Graphic::save: Format: %d", (int) actualType);
        RS_DEBUG->print("RS_Graphic::save: Export...");

        ret = RS_FileIO::instance()->fileExport(*this, actualName, actualType);
    } else {
        RS_DEBUG->print("RS_Graphic::save: Can't create object!");
        RS_DEBUG->print("RS_Graphic::save: File not saved!");
    }

    RS_DEBUG->print("RS_Graphic::save: Done!");

    RS_DEBUG->print("RS_Graphic::save: Exiting...");

    return ret;
}



/*
 *	Description:	- Saves this graphic with the given filename and current
 *						  settings.
 *
 *	Author(s):		..., Claude Sylvain
 *	Created:			?
 *	Last modified:	13 July 2011
 *	Parameters:         QString: name to save
 *                      RS2::FormatType: format to save
 *
 *	Returns:			bool:
 *							false:	Operation failed.
 *							true:		Operation successful.
 *
 * Notes:			Backup the drawing file (if necessary).
 */

bool RS_Graphic::saveAs(const QString &filename, RS2::FormatType type)
{
	RS_DEBUG->print("RS_Graphic::saveAs: Entering...");

	// Set to "failed" by default.
	bool ret	= false;

	// Check/memorize if file name we want to use as new file
	// name is the same as the actual file name.
	auto const filenameSaved = this->filename;
	auto const formatTypeSaved = this->formatType;

	this->filename = filename;
	this->formatType = type;

	ret	= save();		//	Save file.

	if (!ret) {
		//do not modify filenames:
		this->filename = filenameSaved;
		this->formatType = formatTypeSaved;
	}

	return ret;
}

/**
 * Loads the given file into this graphic.
 */
bool RS_Graphic::open(const QString &filename, RS2::FormatType type) {
    RS_DEBUG->print("RS_Graphic::open(%s)", filename.toLatin1().data());

        bool ret = false;

    this->filename = filename;

    // clean all:
    newDoc();

    // import file:
    ret = RS_FileIO::instance()->fileImport(*this, filename, type);

    if( ret) {
        RS_DEBUG->print("RS_Graphic::open(%s): OK", filename.toLatin1().data());
    }

    return ret;
}

void RS_Graphic::clearVariables() {
    variableDict.clear();
}
int RS_Graphic::countVariables() {
    return variableDict.count();
}

void RS_Graphic::addVariable(const QString& key, const RS_Vector& value, int code) {
    variableDict.add(key, value, code);
}
void RS_Graphic::addVariable(const QString& key, const QString& value, int code) {
    variableDict.add(key, value, code);
}
void RS_Graphic::addVariable(const QString& key, int value, int code) {
    variableDict.add(key, value, code);
}
void RS_Graphic::addVariable(const QString& key, double value, int code) {
    variableDict.add(key, value, code);
}
void RS_Graphic::removeVariable(const QString& key) {
    variableDict.remove(key);
}

RS_Vector RS_Graphic::getVariableVector(const QString& key, const RS_Vector& def) const {
    return variableDict.getVector(key, def);
}

QString RS_Graphic::getVariableString(const QString& key, const QString& def) const {
    return variableDict.getString(key, def);
}

int RS_Graphic::getVariableInt(const QString& key, int def) const {
    return variableDict.getInt(key, def);
}

double RS_Graphic::getVariableDouble(const QString& key, double def) const {
    return variableDict.getDouble(key, def);
}

QHash<QString, RS_Variable>& RS_Graphic::getVariableDict() {
    return variableDict.getVariableDict();
}

/**
 * @return true if the grid is switched on (visible).
 */
bool RS_Graphic::isGridOn() const {
        int on = getVariableInt("$GRIDMODE", 1);
        return on != 0;
}


/**
 * Enables / disables the grid.
 */
void RS_Graphic::setGridOn(bool on) {
        addVariable("$GRIDMODE", (int)on, 70);
}

/**
 * @return true if the isometric grid is switched on (visible).
 */
bool RS_Graphic::isIsometricGrid() const{
    //$ISOMETRICGRID == $SNAPSTYLE
        int on = getVariableInt("$SNAPSTYLE", 0);
        return on!=0;
}



/**
 * Enables / disables isometric grid.
 */
void RS_Graphic::setIsometricGrid(bool on) {
    //$ISOMETRICGRID == $SNAPSTYLE
        addVariable("$SNAPSTYLE", (int)on, 70);
}

void RS_Graphic::setCrosshairType(RS2::CrosshairType chType){
    crosshairType=chType;
}

RS2::CrosshairType RS_Graphic::getCrosshairType() const {
    return crosshairType;
}

/**
 * Sets the unit of this graphic to 'u'
 */
void RS_Graphic::setUnit(RS2::Unit u) {

    setPaperSize(RS_Units::convert(getPaperSize(), getUnit(), u));

    addVariable("$INSUNITS", (int)u, 70);

    //unit = u;
}



/**
 * Gets the unit of this graphic
 */
RS2::Unit RS_Graphic::getUnit() const {
    return static_cast<RS2::Unit>(getVariableInt("$INSUNITS", 0));
}



/**
 * @return The linear format type for this document.
 * This is determined by the variable "$LUNITS".
 */
RS2::LinearFormat RS_Graphic::getLinearFormat() {
    int lunits = getVariableInt("$LUNITS", 2);
    return getLinearFormat(lunits);
/* changed by RS2::LinearFormat getLinearFormat(int f)
    switch (lunits) {
    default:
    case 2:
        return RS2::Decimal;
        break;

    case 1:
        return RS2::Scientific;
        break;

    case 3:
        return RS2::Engineering;
        break;

    case 4:
        return RS2::Architectural;
        break;

    case 5:
        return RS2::Fractional;
        break;
    }

    return RS2::Decimal;*/
}

/**
 * @return The linear format type used by the variable "$LUNITS" & "$DIMLUNIT".
 */
RS2::LinearFormat RS_Graphic::getLinearFormat(int f){
    switch (f) {
    default:
    case 2:
        return RS2::Decimal;
        break;

    case 1:
        return RS2::Scientific;
        break;

    case 3:
        return RS2::Engineering;
        break;

    case 4:
        return RS2::Architectural;
        break;

    case 5:
        return RS2::Fractional;
        break;

    case 6:
        return RS2::ArchitecturalMetric;
        break;
    }

    return RS2::Decimal;
}


/**
 * @return The linear precision for this document.
 * This is determined by the variable "$LUPREC".
 */
int RS_Graphic::getLinearPrecision() {
    return getVariableInt("$LUPREC", 4);
}



/**
 * @return The angle format type for this document.
 * This is determined by the variable "$AUNITS".
 */
RS2::AngleFormat RS_Graphic::getAngleFormat() {
    int aunits = getVariableInt("$AUNITS", 0);

    switch (aunits) {
    default:
    case 0:
        return RS2::DegreesDecimal;
        break;

    case 1:
        return RS2::DegreesMinutesSeconds;
        break;

    case 2:
        return RS2::Gradians;
        break;

    case 3:
        return RS2::Radians;
        break;

    case 4:
        return RS2::Surveyors;
        break;
    }

    return RS2::DegreesDecimal;
}



/**
 * @return The linear precision for this document.
 * This is determined by the variable "$LUPREC".
 */
int RS_Graphic::getAnglePrecision() {
    return getVariableInt("$AUPREC", 4);
}



/**
 * @return The insertion point of the drawing into the paper space.
 * This is the distance from the lower left paper edge to the zero
 * point of the drawing. DXF: $PINSBASE.
 */
RS_Vector RS_Graphic::getPaperInsertionBase() {
    return getVariableVector("$PINSBASE", RS_Vector(0.0,0.0));
}


/**
 * Sets the PINSBASE variable.
 */
void RS_Graphic::setPaperInsertionBase(const RS_Vector& p) {
    addVariable("$PINSBASE", p, 10);
}


/**
 * @return Paper size in graphic units.
 */
RS_Vector RS_Graphic::getPaperSize() {
    double sX = 0.;
    double sY = 0.;
    RS_Vector def ;
    if(sX>RS_TOLERANCE && sY>RS_TOLERANCE) {
        def=RS_Units::convert(RS_Vector(sX,sY),
                              RS2::Millimeter, getUnit());
    }else{
        def= RS_Units::convert(RS_Vector(210.0,297.0),
                               RS2::Millimeter, getUnit());
    }

    RS_Vector v1 = getVariableVector("$PLIMMIN", RS_Vector(0.0,0.0));
    RS_Vector v2 = getVariableVector("$PLIMMAX", def);

    return v2-v1;
}


/**
 * Sets a new paper size.
 */
void RS_Graphic::setPaperSize(const RS_Vector& s) {
    addVariable("$PLIMMIN", RS_Vector(0.0,0.0), 10);
    addVariable("$PLIMMAX", s, 10);
    //set default paper size
    RS_Vector def = RS_Units::convert(s,
                                     getUnit(), RS2::Millimeter);

}


/**
 * @return Print Area size in graphic units.
 */
RS_Vector RS_Graphic::getPrintAreaSize(bool total) {
    RS_Vector printArea = getPaperSize();
    printArea.x -= RS_Units::convert(marginLeft+marginRight, RS2::Millimeter, getUnit());
    printArea.y -= RS_Units::convert(marginTop+marginBottom, RS2::Millimeter, getUnit());
    if (total) {
        printArea.x *= pagesNumH;
        printArea.y *= pagesNumV;
    }
    return printArea;
}



/**
 * @return Paper format.
 * This is determined by the variables "$PLIMMIN" and "$PLIMMAX".
 *
 * @param landscape will be set to true for landscape and false for portrait if not nullptr.
 */
RS2::PaperFormat RS_Graphic::getPaperFormat(bool* landscape) {
    RS_Vector size = RS_Units::convert(getPaperSize(),
                                       getUnit(), RS2::Millimeter);

	if (landscape) {
        *landscape = (size.x>size.y);
    }

    return RS_Units::paperSizeToFormat(size);
}



/**
 * Sets the paper format to the given format.
 */
void RS_Graphic::setPaperFormat(RS2::PaperFormat f, bool landscape) {
    RS_Vector size = RS_Units::paperFormatToSize(f);

    if (landscape != (size.x > size.y)) {
		std::swap(size.x, size.y);
    }

	setPaperSize(RS_Units::convert(size, RS2::Millimeter, getUnit()));
}



/**
 * @return Paper space scaling (DXF: $PSVPSCALE).
 */
double RS_Graphic::getPaperScale() const {
    double paperScale = getVariableDouble("$PSVPSCALE", 1.0);
    if (paperScale < 1.0e-6) {
        RS_DEBUG->print(RS_Debug::D_ERROR, "RS_Graphic:: %s(), invalid paper scale %lg\n", __func__, paperScale);
    }

    return paperScale;
}



/**
 * Sets a new scale factor for the paper space.
 */
void RS_Graphic::setPaperScale(double s) {
    if(paperScaleFixed==false) addVariable("$PSVPSCALE", s, DXF_FORMAT_GC_PSVPScale);
}



/**
 * Centers drawing on page. Affects DXF variable $PINSBASE.
 */
void RS_Graphic::centerToPage() {
    RS_Vector size = getPrintAreaSize();
    double scale = getPaperScale();
	auto s=getSize();
	auto sMin=getMin();
    /** avoid zero size, bug#3573158 */
    if(std::abs(s.x)<RS_TOLERANCE) {
        s.x=10.;
        sMin.x=-5.;
    }
    if(std::abs(s.y)<RS_TOLERANCE) {
        s.y=10.;
        sMin.y=-5.;
    }

    RS_Vector pinsbase = (size-s*scale)/2.0 - sMin*scale;
    pinsbase.x += RS_Units::convert(marginLeft, RS2::Millimeter, getUnit());
    pinsbase.y += RS_Units::convert(marginBottom, RS2::Millimeter, getUnit());

    setPaperInsertionBase(pinsbase);
}



/**
 * Fits drawing on page. Affects DXF variable $PINSBASE.
 */
bool RS_Graphic::fitToPage() {
    bool ret = true;
    RS_Vector ps = getPrintAreaSize();
    RS_Vector s = getSize();
    /** avoid zero size, bug#3573158 */
    if(std::abs(s.x)<RS_TOLERANCE) s.x=10.;
    if(std::abs(s.y)<RS_TOLERANCE) s.y=10.;
    double fx = RS_MAXDOUBLE;
    double fy = RS_MAXDOUBLE;
    //ps = RS_Units::convert(ps, getUnit(), RS2::Millimeter);

    // tin-pot 2011-12-30: TODO: can s.x < 0.0 (==> fx < 0.0) happen?
    if (std::abs(s.x) > RS_TOLERANCE) {
        fx = ps.x / s.x;
        // ret=false;
    }
    if (std::abs(s.y) > RS_TOLERANCE) {
        fy = ps.y / s.y;
        // ret=false;
    }

    double fxy = std::min(fx, fy);
    if (fxy >= RS_MAXDOUBLE || fxy <= 1.0e-10) {
        setPaperSize(
                    RS_Units::convert(RS_Vector(210.,297.)
                                      , RS2::Millimeter
                                      , getUnit()
                                      )
                    );
        ret=false;
    }
    setPaperScale(fxy);
    centerToPage();
    return ret;
}


bool RS_Graphic::isBiggerThanPaper() {
    RS_Vector ps = getPrintAreaSize();
    RS_Vector s = getSize() * getPaperScale();
    return !s.isInWindow(RS_Vector(0.0, 0.0), ps);
}


void RS_Graphic::addEntity(RS_Entity* entity)
{
    RS_EntityContainer::addEntity(entity);
    if( entity->rtti() == RS2::EntityBlock ||
            entity->rtti() == RS2::EntityContainer){
        RS_EntityContainer* e=static_cast<RS_EntityContainer*>(entity);
		for(auto e1: *e){
            addEntity(e1);
        }
    }
}

/**
 * Removes invalid objects.
 * @return how many objects were removed
 */
int RS_Graphic::clean()
{
    // author: ravas

    int how_many = 0;

    foreach (RS_Entity* e, entities)
    {
        if    (e->getMin().x > e->getMax().x
            || e->getMin().y > e->getMax().y
            || e->getMin().x > RS_MAXDOUBLE
            || e->getMax().x > RS_MAXDOUBLE
            || e->getMin().x < RS_MINDOUBLE
            || e->getMax().x < RS_MINDOUBLE
            || e->getMin().y > RS_MAXDOUBLE
            || e->getMax().y > RS_MAXDOUBLE
            || e->getMin().y < RS_MINDOUBLE
            || e->getMax().y < RS_MINDOUBLE)
        {
            removeEntity(e);
            how_many += 1;
        }
    }
    return how_many;
}

/**
 * Paper margins in graphic units
 */
void RS_Graphic::setMarginsInUnits(double left, double top, double right, double bottom) {
    setMargins(
        RS_Units::convert(left, getUnit(), RS2::Millimeter),
        RS_Units::convert(top, getUnit(), RS2::Millimeter),
        RS_Units::convert(right, getUnit(), RS2::Millimeter),
        RS_Units::convert(bottom, getUnit(), RS2::Millimeter));
}
double RS_Graphic::getMarginLeftInUnits() {
    return RS_Units::convert(marginLeft, RS2::Millimeter, getUnit());
}
double RS_Graphic::getMarginTopInUnits() {
    return RS_Units::convert(marginTop, RS2::Millimeter, getUnit());
}
double RS_Graphic::getMarginRightInUnits() {
    return RS_Units::convert(marginRight, RS2::Millimeter, getUnit());
}
double RS_Graphic::getMarginBottomInUnits() {
    return RS_Units::convert(marginBottom, RS2::Millimeter, getUnit());
}

void RS_Graphic::setPagesNum(int horiz, int vert) {
    if (horiz > 0)
        pagesNumH = horiz;
    if (vert > 0)
        pagesNumV = vert;
}
void RS_Graphic::setPagesNum(const QString &horizXvert) {
    if (horizXvert.contains('x')) {
        bool ok1 = false;
        bool ok2 = false;
        int i = horizXvert.indexOf('x');
        int h = (int)RS_Math::eval(horizXvert.left(i), &ok1);
        int v = (int)RS_Math::eval(horizXvert.mid(i+1), &ok2);
        if (ok1 && ok2)
            setPagesNum(h, v);
    }
}
