/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
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


#ifndef RS_GRAPHICVIEW_H
#define RS_GRAPHICVIEW_H

#include <memory>
#include <tuple>
#include <vector>

#include <QPointF>

#include "lc_rect.h"
#include "rs.h"

class RS_Entity;
class RS_EntityContainer;
class RS_Color;
class RS_CommandEvent;
class RS_Graphic;
class RS_Grid;
class RS_Painter;

struct RS_LineTypePattern;
//struct RS_SnapMode;


/**
 * This class is a common GUI interface for the graphic viewer
 * widget which has to be implemented by real GUI classes such
 * as the Qt graphical view.
 *
 * Note that this is just an interface used as a slot to
 * communicate with the LibreCAD from a GUI level.
 */
class RS_GraphicView
{
public:
    RS_GraphicView(int w, int h);
	virtual ~RS_GraphicView();

	/**
	 * @return Pointer to the graphic entity if the entity container
	 * connected to this view is a graphic and valid.
	 * NULL otherwise.
	 */
	RS_Graphic* getGraphic() const;

	/** This virtual method must be overwritten to return
	  the width of the widget the graphic is shown in */
	int getWidth() const {
		return width;
	}
	/** This virtual method must be overwritten to return
	  the height of the widget the graphic is shown in */
	int getHeight() const {
		return height;
	}

	void setContainer(RS_EntityContainer* container);
	RS_EntityContainer* getContainer() const;
	void setFactor(double f);
	void setFactorX(double f);
	void setFactorY(double f);
	RS_Vector getFactor() const;
	/**
	 * @brief setOffset
	 * @param ox, offset X
	 * @param oy, offset Y
	 */
	virtual void setOffset(int ox, int oy);
	void setOffsetX(int ox);
	void setOffsetY(int oy);
	int getOffsetX() const;
	int getOffsetY() const;
	/**
	 * Sets a fixed border in pixel around the graphic. This border
	 * specifies how far the user can scroll outside the graphic
	 * area.
	 */
	void setBorders(int left, int top, int right, int bottom);

	int getBorderLeft() const;
	int getBorderTop() const;
	int getBorderRight() const;
	int getBorderBottom() const;

	virtual void drawEntity(RS_Painter *painter, RS_Entity* e, double& offset);
	virtual void drawEntity(RS_Painter *painter, RS_Entity* e);
	virtual void drawEntityPlain(RS_Painter *painter, RS_Entity* e);
	virtual void drawEntityPlain(RS_Painter *painter, RS_Entity* e, double& offset);
    virtual void setPenForEntity(RS_Painter *painter, RS_Entity* e, double& offset);

	virtual const RS_LineTypePattern* getPattern(RS2::LineType t);

	virtual void drawPaper(RS_Painter *painter);
	virtual void drawGrid(RS_Painter *painter);
	virtual void drawMetaGrid(RS_Painter *painter);

	RS_Grid* getGrid() const;

	bool isGridOn() const;
	bool isGridIsometric() const;
	void setCrosshairType(RS2::CrosshairType chType);
	RS2::CrosshairType getCrosshairType() const;

	RS_Vector toGui(RS_Vector v) const;
	double toGuiX(double x) const;
	double toGuiY(double y) const;
	double toGuiDX(double d) const;
	double toGuiDY(double d) const;

    RS_Vector toGraph(const RS_Vector& v) const;
    RS_Vector toGraph(const QPointF& v) const;
    RS_Vector toGraph(int x, int y) const;
	double toGraphX(int x) const;
	double toGraphY(int y) const;
	double toGraphDX(int d) const;
	double toGraphDY(int d) const;

	/**
		 * Enables or disables printing.
		 */
	void setPrinting(bool p);

	/**
		 * @retval true This is a graphic view for printing.
		 * @retval false setSnapOtherwise.
		 */
	bool isPrinting() const;
	
    const LC_Rect& getViewRect() const {
        return view_rect;
    }

	/**
	 * Sets the background color. Note that applying the background
	 * color for the widget is up to the implementing class.
	 */
	virtual void setBackground(const RS_Color& bg);

	/**
		 * @return Current background color.
		 */
    RS_Color getBackground() const;

	/**
		 * @return Current foreground color.
		 */
    RS_Color getForeground() const;

	/**
		 * Sets the grid color.
		 */
    void setGridColor(const RS_Color& c);

	/**
		 * Sets the meta grid color.
		 */
    void setMetaGridColor(const RS_Color& c);

protected:

    RS_EntityContainer* container = nullptr; // Holds a pointer to all the enties

    /** colors for different usages*/
    struct ColorData;
    std::unique_ptr<ColorData> m_colorData;
	/** Grid */
	std::unique_ptr<RS_Grid> grid;

    LC_Rect view_rect;

private:
    //! Width
    int width = 0;

    //! Height
    int height = 0;

    RS_Vector factor{1.,1.};
	int offsetX=0;
	int offsetY=0;

	int borderLeft=0;
	int borderTop=0;
	int borderRight=0;
	int borderBottom=0;

	//! Active when printing only:
	bool printing=false;
};

#endif
