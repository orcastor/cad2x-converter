/******************************************************************************
**
** This file was created for the LibreCAD project, a 2D CAD program.
**
** Copyright (C) 2018 Alexander Pravdin <aledin@mail.ru>
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
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
******************************************************************************/

#include <QtCore>

#include "rs.h"
#include "rs_graphic.h"
#include "rs_painterqt.h"
#include "rs_graphicview.h"
#include "rs_units.h"
#include "rs_debug.h"

#include "pdf_print_loop.h"


static bool openDocAndSetGraphic(RS_Document**, RS_Graphic**, const QString&);
static void touchGraphic(RS_Graphic*, PdfPrintParams&);
static void setupWriter(RS_Graphic*, QPdfWriter&, PdfPrintParams&);
static void drawPage(RS_Graphic*, QPdfWriter&, RS_PainterQt&);

// Convert RS2::PaperFormat to QPageSize
QPageSize toPageSize(RS2::PaperFormat paper)
{
    // supported paper formats should be added here
    static const std::map<RS2::PaperFormat, QPageSize::PageSizeId> lookUp = {
        {
                {RS2::A0, QPageSize::A0},
                {RS2::A1, QPageSize::A1},
                {RS2::A2, QPageSize::A2},
                {RS2::A3, QPageSize::A3},
                {RS2::A4, QPageSize::A4},

                    /* Removed ISO "B" and "C" series,  C5E,  Comm10E,  DLE,  (envelope sizes) */

                {RS2::Letter, QPageSize::Letter},
                {RS2::Legal,  QPageSize::Legal},
                {RS2::Tabloid, QPageSize::Tabloid},

                //{RS2::Ansi_A, QPageSize::AnsiA},
                //{RS2::Ansi_B, QPageSize::AnsiB},
                {RS2::Ansi_C, QPageSize::AnsiC},
                {RS2::Ansi_D, QPageSize::AnsiD},
                {RS2::Ansi_E, QPageSize::AnsiE},

                {RS2::Arch_A, QPageSize::ArchA},
                {RS2::Arch_B, QPageSize::ArchB},
                {RS2::Arch_C, QPageSize::ArchC},
                {RS2::Arch_D, QPageSize::ArchD},
                {RS2::Arch_E, QPageSize::ArchE},
        }
    };
    QPageSize::PageSizeId id = lookUp.count(paper) == 1 ?
                                   lookUp.at(paper) : QPageSize::Custom;
    return QPageSize{id};
}

void PdfPrintLoop::run()
{
    if (params.outFile.isEmpty()) {
        for (auto &&f : params.files) {
            printOneToOnePdf(f);
        }
    } else {
        printManyToOnePdf();
    }

    emit finished();
}

void PdfPrintLoop::printOneToOnePdf(const QString& file) {

    // Main code logic and flow for this method is originally stolen from
    // QC_ApplicationWindow::slotFilePrint(bool printPDF) method.
    // But finally it was split in to smaller parts.

    QFileInfo fileInfo(file);
    params.outFile =
        (params.outDir.isEmpty() ? fileInfo.path() : params.outDir)
        + "/" + fileInfo.completeBaseName() + ".pdf";

    RS_Document *doc;
    RS_Graphic *graphic;

    if (!openDocAndSetGraphic(&doc, &graphic, file))
        return;

    qDebug() << "Printing" << file << "to" << params.outFile << ">>>>";

    touchGraphic(graphic, params);

    QPdfWriter pdfWriter(params.outFile);

    setupWriter(graphic, pdfWriter, params);

    RS_PainterQt painter(&pdfWriter);

    if (params.monochrome)
        painter.setDrawingMode(RS2::ModeBW);

    drawPage(graphic, pdfWriter, painter);

    painter.end();

    qDebug() << "Printing" << file << "to" << params.outFile << "DONE";

    delete doc;
}

void PdfPrintLoop::printManyToOnePdf() {

    struct Page {
        RS_Document* doc;
        RS_Graphic* graphic;
        QString file;
        QPdfWriter::PageSize paperSize;
    };

    if (!params.outDir.isEmpty()) {
        QFileInfo outFileInfo(params.outFile);
        params.outFile = params.outDir + "/" + outFileInfo.fileName();
    }

    QVector<Page> pages;
    int nrPages = 0;

    // FIXME: Should probably open and print all dxf files in one 'for' loop.
    // Tried but failed to do this. It looks like some 'chicken and egg'
    // situation for the QPdfWriter and RS_PainterQt. Therefore, first open
    // all dxf files and apply required actions. Then run another 'for'
    // loop for actual printing.
    for (auto file : params.files) {

        Page page;

        page.file = file;

        if (!openDocAndSetGraphic(&page.doc, &page.graphic, file))
            continue;

        qDebug() << "Opened" << file;

        touchGraphic(page.graphic, params);

        pages.append(page);

        nrPages++;
    }

    QPdfWriter pdfWriter(params.outFile);

    if (nrPages > 0) {
        // FIXME: Is it possible to set up pdfWriter and paper for every
        // opened dxf file and tie them with painter? For now just using
        // data extracted from the first opened dxf file for all pages.
        setupWriter(pages.at(0).graphic, pdfWriter, params);
    }

    RS_PainterQt painter(&pdfWriter);

    if (params.monochrome)
        painter.setDrawingMode(RS2::ModeBW);

    // And now it's time to actually print all previously opened dxf files.
    for (auto page : pages) {
        nrPages--;

        qDebug() << "Printing" << page.file
                 << "to" << params.outFile << ">>>>";

        drawPage(page.graphic, pdfWriter, painter);

        qDebug() << "Printing" << page.file
                 << "to" << params.outFile << "DONE";

        delete page.doc;

        if (nrPages > 0)
            pdfWriter.newPage();
    }

    painter.end();
}

static bool openDocAndSetGraphic(RS_Document** doc, RS_Graphic** graphic,
    const QString& file)
{
    *doc = new RS_Graphic();

    if (!(*doc)->open(file, RS2::FormatUnknown)) {
        qDebug() << "ERROR: Failed to open document" << file;
        delete *doc;
        return false;
    }

    *graphic = (*doc)->getGraphic();
    if (*graphic == nullptr) {
        qDebug() << "ERROR: No graphic in" << file;
        delete *doc;
        return false;
    }

    return true;
}

static void touchGraphic(RS_Graphic* graphic, PdfPrintParams& params)
{
    graphic->calculateBorders();
    graphic->setMargins(params.margins.left, params.margins.top,
                        params.margins.right, params.margins.bottom);
    graphic->setPagesNum(params.pagesH, params.pagesV);

    if (params.scale > 0.0)
        graphic->setPaperScale(params.scale);

    if (params.pageSize != RS_Vector(0.0, 0.0))
        graphic->setPaperSize(params.pageSize);

    if (params.fitToPage)
        graphic->fitToPage(); // fit and center
    else if (params.centerOnPage)
        graphic->centerToPage();
}

static void setupWriter(RS_Graphic* graphic, QPdfWriter& pdfWriter,
    PdfPrintParams& params)
{
    bool landscape = false;
    RS2::PaperFormat pf = graphic->getPaperFormat(&landscape);
    QPageSize pageSize = toPageSize(pf);

    if (pageSize.id() == QPageSize::Custom){
        RS_Vector r = graphic->getPaperSize();
        RS_Vector s = RS_Units::convert(r, graphic->getUnit(),
            RS2::Millimeter);
        if (landscape)
            s = s.flipXY();
        pdfWriter.setPageSize(QPageSize{QSizeF{s.x, s.y}, QPageSize::Millimeter});
    } else {
        pdfWriter.setPageSize(pageSize);
    }

    pdfWriter.setPageOrientation(landscape ? QPageLayout::Landscape : QPageLayout::Portrait);
    pdfWriter.setResolution(params.resolution);
}

static void drawPage(RS_Graphic* graphic, QPdfWriter& pdfWriter,
    RS_PainterQt& painter)
{
    double pdfWriterFx = (double)pdfWriter.width() / pdfWriter.widthMM();
    double pdfWriterFy = (double)pdfWriter.height() / pdfWriter.heightMM();

    double marginLeft = graphic->getMarginLeft();
    double marginTop = graphic-> getMarginTop();
    double marginRight = graphic->getMarginRight();
    double marginBottom = graphic->getMarginBottom();

    painter.setClipRect(marginLeft * pdfWriterFx, marginTop * pdfWriterFy,
                        pdfWriter.width() - (marginLeft + marginRight) * pdfWriterFx,
                        pdfWriter.height() - (marginTop + marginBottom) * pdfWriterFy);

    RS_GraphicView gv(pdfWriter.width(), pdfWriter.height());
    gv.setBackground(Qt::white);
    gv.setPrinting(true);

    double fx = pdfWriterFx * RS_Units::getFactorToMM(graphic->getUnit());
    double fy = pdfWriterFy * RS_Units::getFactorToMM(graphic->getUnit());

    double f = (fx + fy) / 2.0;

    double scale = graphic->getPaperScale();

    gv.setOffset((int)(graphic->getPaperInsertionBase().x * f),
                 (int)(graphic->getPaperInsertionBase().y * f));
    gv.setFactor(f*scale);
    gv.setContainer(graphic);

    double baseX = graphic->getPaperInsertionBase().x;
    double baseY = graphic->getPaperInsertionBase().y;
    int numX = graphic->getPagesNumHoriz();
    int numY = graphic->getPagesNumVert();

    RS_Vector printArea = graphic->getPrintAreaSize(false);
    for (int pY = 0; pY < numY; pY++) {
        double offsetY = printArea.y * pY;
        for (int pX = 0; pX < numX; pX++) {
            double offsetX = printArea.x * pX;
            // First page is created automatically.
            // Extra pages must be created manually.
            if (pX > 0 || pY > 0) pdfWriter.newPage();
            gv.setOffset((int)((baseX - offsetX) * f),
                         (int)((baseY - offsetY) * f));
            gv.drawEntity(&painter, graphic);
        }
    }
}
