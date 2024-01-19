/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2015 A. Stebich (librecad@mail.lordofbikes.de)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
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

#include<climits>
#include<cmath>

//#include <QApplication>
//#include <QDesktopWidget>
//#include <QAction>
//#include <QMouseEvent>
#include <QtAlgorithms>

#include "rs_graphicview.h"

#include "rs_color.h"
#include "rs_debug.h"
// #include "rs_dialogfactory.h"
#include "rs_graphic.h"
#include "rs_grid.h"
#include "rs_line.h"
#include "rs_linetypepattern.h"
#include "rs_math.h"
#include "rs_painter.h"
//#include "rs_snapper.h"
// #include "rs_settings.h"
#include "rs_units.h"

#ifdef EMU_C99
#include "emu_c99.h"
#endif

namespace Colors
{
    const QString background        = "Black";
    const QString grid              = "Gray";
    const QString meta_grid         = "#404040";
}

struct RS_GraphicView::ColorData {
    /** background color (any color) */
    RS_Color background;
    /** foreground color (black or white) */
    RS_Color foreground;
    /** grid color */
    RS_Color gridColor = Qt::gray;
    /** meta grid color */
    RS_Color metaGridColor;
};

/**
 * Constructor.
 */
RS_GraphicView::RS_GraphicView(int w, int h)
    :m_colorData{std::make_unique<ColorData>()}
    ,grid{std::make_unique<RS_Grid>(this)}
	,width(w)
	,height(h)
{
    setBackground(Colors::background);
    setGridColor(Colors::grid);
    setMetaGridColor(Colors::meta_grid);
}

RS_GraphicView::~RS_GraphicView()
{
}

/**
 * Sets the pointer to the graphic which contains the entities
 * which are visualized by this widget.
 */
void RS_GraphicView::setContainer(RS_EntityContainer* container) {
	this->container = container;
	//adjustOffsetControls();
}



/**
 * Sets the zoom factor in X for this visualization of the graphic.
 */
void RS_GraphicView::setFactorX(double f) {
    factor.x = std::abs(f);
}



/**
 * Sets the zoom factor in Y for this visualization of the graphic.
 */
void RS_GraphicView::setFactorY(double f) {
    factor.y = std::abs(f);
}

void RS_GraphicView::setOffset(int ox, int oy) {
	//    DEBUG_HEADER
	//    RS_DEBUG->print(/*RS_Debug::D_WARNING, */"set offset from (%d, %d) to (%d, %d)", getOffsetX(), getOffsetY(), ox, oy);
	setOffsetX(ox);
	setOffsetY(oy);
}

/**
 * @return true if the grid is switched on.
 */
bool RS_GraphicView::isGridOn() const{
	if (container) {
        RS_Graphic* graphic = container->getGraphic();
        if (graphic != nullptr) {
            return graphic->isGridOn();
		}
	}
	return true;
}

/**
 * @return true if the grid is isometric
 *
 *@Author: Dongxu Li
 */
bool RS_GraphicView::isGridIsometric() const{
	return grid->isIsometric();
}


void RS_GraphicView::setCrosshairType(RS2::CrosshairType chType){
	grid->setCrosshairType(chType);
}

RS2::CrosshairType RS_GraphicView::getCrosshairType() const{
	return grid->getCrosshairType();
}


/*	*
 *	Function name:
 *
 *	Description:	- Sets the pen of the painter object to the suitable pen
 *						  for the given entity.
 *
 *	Author(s):		..., Claude Sylvain
 *	Created:			?
 *	Last modified:	17 November 2011
 *
 *	Parameters:		RS_Painter *painter:
 *							...
 *
 *						RS_Entity *e:
 *							...
 *
 *	Returns:			void
 */

void RS_GraphicView::setPenForEntity(RS_Painter *painter,RS_Entity *e, double& offset)
{
	// Getting pen from entity (or layer)
	RS_Pen pen = e->getPen(true);

    // Avoid negative widths
    int w = std::max(static_cast<int>(pen.getWidth()), 0);

	// - Scale pen width.
	// - By default pen width is not scaled on print and print preview.
	//   This is the standard (AutoCAD like) behaviour.
	// bug# 3437941
	// ------------------------------------------------------------
	double	uf = 1.0;	// Unit factor.
	double	wf = 1.0;	// Width factor.

	RS_Graphic* graphic = container->getGraphic();
	if (graphic)
	{
		uf = RS_Units::convert(1.0, RS2::Millimeter, graphic->getUnit());

		if (isPrinting() && graphic->getPaperScale() > RS_TOLERANCE )
		{
			wf = 1.0 / graphic->getPaperScale();
		}
	}

	if (pen.getAlpha() == 1.0) pen.setScreenWidth(toGuiDX(w / 100.0 * uf * wf));

	// prevent drawing with 1-width which is slow:
	if (RS_Math::round(pen.getScreenWidth())==1) {
		pen.setScreenWidth(0.0);
	}

    // prevent background color on background drawing
    // and enhance visibility of black lines on dark backgrounds
    RS_Color    penColor{pen.getColor().stripFlags()};
    if ( penColor == m_colorData->background.stripFlags()
         || (penColor.toIntColor() == RS_Color::Black
             && penColor.colorDistance(m_colorData->background) < RS_Color::MinColorDistance)) {
        pen.setColor(m_colorData->foreground);
    }

    pen.setDashOffset(offset);

	painter->setPen(pen);
}

/**
 * Draws an entity. Might be recursively called e.g. for polylines.
 * If the class wide painter is nullptr a new painter will be created
 * and destroyed afterwards.
 *
 * @param offset Offset of line pattern (used for connected
 *        lines e.g. in splines).
 * @param db Double buffering on (recommended) / off
 */
void RS_GraphicView::drawEntity(RS_Painter *painter, RS_Entity* e) {
	double offset(0.);
	drawEntity(painter,e,offset);
}

void RS_GraphicView::drawEntity(RS_Painter *painter, RS_Entity* e, double& offset) {

	// update is disabled:
    // given entity is nullptr:
	if (!e) {
		return;
	}

	// entity is not visible:
	if (!e->isVisible()) {
		return;
	}

	if (isPrinting() ) {
		// do not draw construction layer on print preview or print
		if( ! e->isPrint()
				||  e->isConstruction())
        return;
	}

    // test if the entity is in the viewport
    if (!isPrinting() &&
        e->rtti() != RS2::EntityGraphic &&
        e->rtti() != RS2::EntityLine &&
       (toGuiX(e->getMax().x)<0 || toGuiX(e->getMin().x)>getWidth() ||
        toGuiY(e->getMin().y)<0 || toGuiY(e->getMax().y)>getHeight())) {
        return;
    }

	// set pen (color):
    setPenForEntity(painter, e, offset);
	drawEntityPlain(painter, e, offset);

	//RS_DEBUG->print("draw plain OK");
	//RS_DEBUG->print("RS_GraphicView::drawEntity() end");
}

/**
 * Draws an entity.
 * The painter must be initialized and all the attributes (pen) must be set.
 */
void RS_GraphicView::drawEntityPlain(RS_Painter *painter, RS_Entity* e, double& offset) {
	if (!e) {
		return;
	}

	e->draw(painter, this, offset);
}

void RS_GraphicView::drawEntityPlain(RS_Painter *painter, RS_Entity* e) {
	if (!e) {
		return;
	}

	double offset(0.);
	e->draw(painter, this, offset);
}

/**
 * @return Pointer to the static pattern struct that belongs to the
 * given pattern type or nullptr.
 */
const RS_LineTypePattern* RS_GraphicView::getPattern(RS2::LineType t) {
    return RS_LineTypePattern::getPattern(t);
}

/**
 * Draws the paper border (for print previews).
 * This function can ONLY be called from within a paintEvent because it will
 * use the painter
 *
 * @see drawIt()
 */
void RS_GraphicView::drawPaper(RS_Painter *painter) {
	if (!container) {
		return;
	}

	RS_Graphic* graphic = container->getGraphic();
	if (graphic->getPaperScale()<1.0e-6) {
		return;
	}

	// draw paper:
	// RVT_PORT rewritten from     painter->setPen(Qt::gray);
	painter->setPen(QColor(Qt::gray));

	RS_Vector pinsbase = graphic->getPaperInsertionBase();
	RS_Vector printAreaSize = graphic->getPrintAreaSize();
	double scale = graphic->getPaperScale();

	RS_Vector v1 = toGui((RS_Vector(0,0)-pinsbase)/scale);
	RS_Vector v2 = toGui((printAreaSize-pinsbase)/scale);

	int marginLeft = (int)(graphic->getMarginLeftInUnits() * factor.x / scale);
	int marginTop = (int)(graphic->getMarginTopInUnits() * factor.y / scale);
	int marginRight = (int)(graphic->getMarginRightInUnits() * factor.x / scale);
	int marginBottom = (int)(graphic->getMarginBottomInUnits() * factor.y / scale);

	int printAreaW = (int)(v2.x-v1.x);
	int printAreaH = (int)(v2.y-v1.y);

	int paperX1 = (int)v1.x;
	int paperY1 = (int)v1.y;
	// Don't show margins between neighbor pages.
	int paperW = printAreaW + marginLeft + marginRight;
	int paperH = printAreaH - marginTop - marginBottom;

	int numX = graphic->getPagesNumHoriz();
	int numY = graphic->getPagesNumVert();

	// gray background:
	painter->fillRect(0,0, getWidth(), getHeight(),
					  RS_Color(200,200,200));

	// shadow:
	painter->fillRect(paperX1+6, paperY1+6, paperW, paperH,
					  RS_Color(64,64,64));

	// border:
	painter->fillRect(paperX1, paperY1, paperW, paperH,
					  RS_Color(64,64,64));

	// paper:
	painter->fillRect(paperX1+1, paperY1-1, paperW-2, paperH+2,
					  RS_Color(180,180,180));

	// print area:
	painter->fillRect(paperX1+1+marginLeft, paperY1-1-marginBottom,
					  printAreaW-2, printAreaH+2,
					  RS_Color(255,255,255));

	// don't paint boundaries if zoom is to small
    if (qMin(std::abs(printAreaW/numX), std::abs(printAreaH/numY)) > 2) {
		// boundaries between pages:
		for (int pX = 1; pX < numX; pX++) {
			double offset = ((double)printAreaW*pX)/numX;
			painter->fillRect(paperX1+marginLeft+offset, paperY1,
							  1, paperH,
							  RS_Color(64,64,64));
		}
		for (int pY = 1; pY < numY; pY++) {
			double offset = ((double)printAreaH*pY)/numY;
			painter->fillRect(paperX1, paperY1-marginBottom+offset,
							  paperW, 1,
							  RS_Color(64,64,64));
		}
	}
}


/**
 * Draws the grid.
 *
 * @see drawIt()
 */
void RS_GraphicView::drawGrid(RS_Painter *painter) {

	if (!(grid && isGridOn())) return;

	// draw grid:
	//painter->setPen(Qt::gray);
    painter->setPen(m_colorData->gridColor);

	//grid->updatePointArray();
	auto const& pts = grid->getPoints();
	for(auto const& v: pts){
		painter->drawGridPoint(toGui(v));
	}
}

/**
 * Draws the meta grid.
 *
 * @see drawIt()
 */
void RS_GraphicView::drawMetaGrid(RS_Painter *painter) {

	if (!(grid && isGridOn()) /*|| grid->getMetaSpacing()<0.0*/) {
		return;
	}

	//draw grid after metaGrid to avoid overwriting grid points by metaGrid lines
	//bug# 3430258
	grid->updatePointArray();
    RS_Pen pen(m_colorData->metaGridColor,
			   RS2::Width00,
               RS2::DotLineTiny);
	painter->setPen(pen);

	RS_Vector dv=grid->getMetaGridWidth().scale(factor);
    double dx=std::abs(dv.x);
    double dy=std::abs(dv.y); //potential bug, need to recover metaGrid.width
	// draw meta grid:
	auto mx = grid->getMetaX();
	for(auto const& x: mx){
		painter->drawLine(RS_Vector(toGuiX(x), 0),
						  RS_Vector(toGuiX(x), getHeight()));
		if(grid->isIsometric()){
			painter->drawLine(RS_Vector(toGuiX(x)+0.5*dx, 0),
							  RS_Vector(toGuiX(x)+0.5*dx, getHeight()));
		}
	}
	auto my = grid->getMetaY();
	if(grid->isIsometric()){//isometric metaGrid
        dx=std::abs(dx);
        dy=std::abs(dy);
		if(!my.size()|| dx<1||dy<1) return;
		RS_Vector baseMeta(toGui(RS_Vector(mx[0],my[0])));
		// x-x0=k*dx, x-remainder(x-x0,dx)
        RS_Vector vp0(-std::remainder(-baseMeta.x,dx)-dx,getHeight()-remainder(getHeight()-baseMeta.y,dy)+dy);
		RS_Vector vp1(vp0);
        RS_Vector vp2(getWidth()-std::remainder(getWidth()-baseMeta.x,dx)+dx,vp0.y);
		RS_Vector vp3(vp2);
        int cmx = std::round((vp2.x - vp0.x)/dx);
        int cmy = std::round((vp0.y + std::remainder(-baseMeta.y,dy)+dy)/dy);
		for(int i=cmx+cmy+2;i>=0;i--){
			if ( i <= cmx ) {
				vp0.x += dx;
				vp2.y -= dy;
			}else{
				vp0.y -= dy;
				vp2.x -= dx;
			}
			if ( i <= cmy ) {
				vp1.y -= dy;
				vp3.x -= dx;
			}else{
				vp1.x += dx;
				vp3.y -= dy;
			}
			painter->drawLine(vp0,vp1);
			painter->drawLine(vp2,vp3);
		}

	}else{//orthogonal

		for(auto const& y: my){
			painter->drawLine(RS_Vector(0, toGuiY(y)),
							  RS_Vector(getWidth(), toGuiY(y)));
		}
	}
}

/**
 * Translates a vector in real coordinates to a vector in screen coordinates.
 */
RS_Vector RS_GraphicView::toGui(RS_Vector v) const{
	return RS_Vector(toGuiX(v.x), toGuiY(v.y));
}

/**
 * Translates a real coordinate in X to a screen coordinate X.
 * @param visible Pointer to a boolean which will contain true
 * after the call if the coordinate is within the visible range.
 */
double RS_GraphicView::toGuiX(double x) const{
	return x*factor.x + offsetX;
}

/**
 * Translates a real coordinate in Y to a screen coordinate Y.
 */
double RS_GraphicView::toGuiY(double y) const{
	return -y*factor.y + getHeight() - offsetY;
}

/**
 * Translates a real coordinate distance to a screen coordinate distance.
 */
double RS_GraphicView::toGuiDX(double d) const{
	return d*factor.x;
}

/**
 * Translates a real coordinate distance to a screen coordinate distance.
 */
double RS_GraphicView::toGuiDY(double d) const{
	return d*factor.y;
}

/**
 * Translates a vector in screen coordinates to a vector in real coordinates.
 */
RS_Vector RS_GraphicView::toGraph(const RS_Vector& v) const{
	return RS_Vector(toGraphX(RS_Math::round(v.x)),
					 toGraphY(RS_Math::round(v.y)));
}

/**
 * Translates two screen coordinates to a vector in real coordinates.
 */
RS_Vector RS_GraphicView::toGraph(const QPointF& position) const
{
    return toGraph(position.x(), position.y());
}

/**
 * Translates two screen coordinates to a vector in real coordinates.
 */
RS_Vector RS_GraphicView::toGraph(int x, int y) const
{
    return RS_Vector(toGraphX(x), toGraphY(y));
}

/**
 * Translates a screen coordinate in X to a real coordinate X.
 */
double RS_GraphicView::toGraphX(int x) const{
	return (x - offsetX)/factor.x;
}

/**
 * Translates a screen coordinate in Y to a real coordinate Y.
 */
double RS_GraphicView::toGraphY(int y) const{
	return -(y - getHeight() + offsetY)/factor.y;
}

/**
 * Translates a screen coordinate distance to a real coordinate distance.
 */
double RS_GraphicView::toGraphDX(int d) const{
	return d/factor.x;
}

/**
 * Translates a screen coordinate distance to a real coordinate distance.
 */
double RS_GraphicView::toGraphDY(int d) const{
	return d/factor.y;
}

RS_Grid* RS_GraphicView::getGrid() const{
	return grid.get();
}

void RS_GraphicView::setBackground(const RS_Color& bg) {
    m_colorData->background = bg;

    RS_Color black(0,0,0);
    if (black.colorDistance( bg) >= RS_Color::MinColorDistance) {
        m_colorData->foreground = black;
    }
    else {
        m_colorData->foreground = RS_Color(255,255,255);
    }
}

RS_Color RS_GraphicView::getBackground() const{
    return m_colorData->background;
}

/**
     * @return Current foreground color.
     */
RS_Color RS_GraphicView::getForeground() const{
    return m_colorData->foreground;
}

/**
     * Sets the grid color.
     */
void RS_GraphicView::setGridColor(const RS_Color& c) {
    m_colorData->gridColor = c;
}

/**
     * Sets the meta grid color.
     */
void RS_GraphicView::setMetaGridColor(const RS_Color& c) {
    m_colorData->metaGridColor = c;
}

void RS_GraphicView::setBorders(int left, int top, int right, int bottom) {
	borderLeft = left;
	borderTop = top;
	borderRight = right;
	borderBottom = bottom;
}

RS_Graphic* RS_GraphicView::getGraphic() const{
	if (container && container->rtti()==RS2::EntityGraphic) {
		return static_cast<RS_Graphic *>(container);
	} else {
		return nullptr;
	}
}

RS_EntityContainer* RS_GraphicView::getContainer() const{
	return container;
}

void RS_GraphicView::setFactor(double f) {
	setFactorX(f);
	setFactorY(f);
}

RS_Vector RS_GraphicView::getFactor() const{
	return factor;
}
int RS_GraphicView::getBorderLeft() const{
	return borderLeft;
}
int RS_GraphicView::getBorderTop() const{
	return borderTop;
}
int RS_GraphicView::getBorderRight() const{
	return borderRight;
}
int RS_GraphicView::getBorderBottom() const{
	return borderBottom;
}
void RS_GraphicView::setOffsetX(int ox) {
	offsetX = ox;
}
void RS_GraphicView::setOffsetY(int oy) {
	offsetY = oy;
}
int RS_GraphicView::getOffsetX() const{
	return offsetX;
}
int RS_GraphicView::getOffsetY() const{
	return offsetY;
}

void RS_GraphicView::setPrinting(bool p) {
	printing = p;
}

bool RS_GraphicView::isPrinting() const{
	return printing;
}
