/******************************************************************************
**
** This file was created for the LibreCAD project, a 2D CAD program.
**
** Copyright (C) 2020 Nikita Letov <letovnn@gmail.com>
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

#include <memory>
#include <set>

#include <QCoreApplication>
#include <QImageWriter>
#include <QtCore>

#include <QFile>

#include <QStringList>

#define STR(x)   #x
#define XSTR(x)  STR(x)

#include "lc_makercamsvg.h"
#include "lc_xmlwriterqxmlstreamwriter.h"
#include "rs.h"
#include "rs_debug.h"
#include "rs_document.h"
#include "rs_fontlist.h"
#include "rs_graphic.h"
#include "rs_math.h"
#include "rs_units.h"
#include "rs_painterqt.h"
#include "rs_patternlist.h"
// #include "rs_settings.h"
#include "rs_graphicview.h"
// #include "rs_system.h"

#include <regex>


///////////////////////////////////////////////////////////////////////
/// \brief openDocAndSetGraphic opens a DXF file and prepares all its graphics content
/// for further manipulations
/// \return
//////////////////////////////////////////////////////////////////////
static std::unique_ptr<RS_Document> openDocAndSetGraphic(QString);

static void touchGraphic(RS_Graphic*);

static QSize parsePngSizeArg(QString);

bool slotFileExport(RS_Graphic* graphic,
                    const QString& name,
                    QSize size,
                    QSize borders,
                    bool black,
                    bool bw=true);

namespace {
// find the image format from the file extension; default to png
QString getFormatFromFile(const QString& fileName)
{
    QList<QByteArray> supportedImageFormats = QImageWriter::supportedImageFormats();
    supportedImageFormats.push_back("svg"); // add svg

    for (QString format: supportedImageFormats) {
        format = format.toLower();
        if (fileName.endsWith(format, Qt::CaseInsensitive))
            return format;
    }
    return "png";
}
}

/////////
/// \brief cad2pic tool for converting DXF/DWG to PNG/SVG.
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char* argv[])
{
    RS_DEBUG->setLevel(RS_Debug::D_NOTHING);

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("cad2pic");
    QCoreApplication::setApplicationName("cad2pic");
    QCoreApplication::setApplicationVersion(XSTR(LC_VERSION));

    QCommandLineParser parser;

    QString appDesc;
    QString cad2pic;
    appDesc += "\nPrint a DXF/DWG file to a PNG/SVG file.";
    appDesc += "\n\n";
    appDesc += "Examples:\n\n";
    appDesc += "  cad2pic *.dxf|*.dwg|*";
    appDesc += "    -- print a dxf/dwg file to a png/svg file with the same name.\n";
    parser.setApplicationDescription(appDesc);

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption outFileOpt(QStringList() << "o" << "outfile",
        "Output PNG/SVG file.", "file");
    parser.addOption(outFileOpt);

    QCommandLineOption pngSizeOpt(QStringList() << "r" << "resolution",
        "Output PNG size (Width x Height) in pixels.", "WxH");
    parser.addOption(pngSizeOpt);

    parser.addPositionalArgument("<dxf/dwg files>", "Input DXF/DWG file");

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty())
        parser.showHelp(EXIT_FAILURE);

    QStringList dxfFiles;
    for (auto arg : args) {
        QFileInfo fi(arg);
        if (fi.suffix().toLower() != "dxf" && fi.suffix().toLower() != "dwg")
            continue; // Skip files without .dxf/.dwg extension
        dxfFiles.append(arg);
    }

    if (dxfFiles.isEmpty())
        parser.showHelp(EXIT_FAILURE);

    // Output setup
    QString& dxfFile = dxfFiles[0];

    QFileInfo fi(dxfFile);
    QString fn = fi.completeBaseName(); // original DXF file name
    if(fn.isEmpty())
        fn = "unnamed";

    // Set output filename from user input if present
    QString outFile = parser.value(outFileOpt);
    if (outFile.isEmpty()) {
        outFile = fi.path() + "/" + fn + ".png";
    } else {
        outFile = fi.path() + "/" + outFile;
    }

    // Open the file and process the graphics
    std::unique_ptr<RS_Document> doc = openDocAndSetGraphic(dxfFile);
    if (doc == nullptr || doc->getGraphic() == nullptr)
        return 1;

    RS_Graphic *graphic = doc->getGraphic();

    LC_LOG << "Printing" << dxfFile << "to" << outFile << ">>>>";

    touchGraphic(graphic);

    // Start of the actual conversion

    LC_LOG << "QC_ApplicationWindow::slotFileExport()";

    // find out extension:
    QString format = getFormatFromFile(outFile).toUpper();

    // append extension to file:
    if (!QFileInfo(fn).fileName().contains(".")) {
        fn.push_back("." + format.toLower());
    }

    bool ret = false;
    if (format.compare("SVG", Qt::CaseInsensitive) == 0) {
        auto generator = std::make_unique<LC_MakerCamSVG>(std::make_unique<LC_XMLWriterQXmlStreamWriter>());
        if (generator->generate(graphic)) {
            QFile file{outFile};
            if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                LC_ERR << __func__ << "(): failed in creating file " << outFile << ", no SVG is generated";
                ret = false;
            } else {
                QTextStream out(&file);
                out << QString::fromStdString(generator->resultAsString());
                ret = true;
            }
        }
    } else {
        // Set PNG size from user input
        QSize pngSize = parsePngSizeArg(parser.value(pngSizeOpt)); // If nothing, use default values.
        QSize borders = QSize(5, 5);
        bool black = false;
        bool bw = false;
        ret = slotFileExport(graphic, outFile, pngSize, borders, black, bw);
    }

    qDebug() << "Printing" << dxfFile << "to" << outFile << (ret ? "Done" : "Failed");
    return 0;
}

static std::unique_ptr<RS_Document> openDocAndSetGraphic(QString dxfFile)
{
    auto doc = std::make_unique<RS_Graphic>();

    if (!doc->open(dxfFile, RS2::FormatUnknown)) {
        qDebug() << "ERROR: Failed to open document" << dxfFile;
        qDebug() << "Check if file exists";
        return {};
    }

    RS_Graphic* graphic = doc->getGraphic();
    if (graphic == nullptr) {
        qDebug() << "ERROR: No graphic in" << dxfFile;
        return {};
    }

    return doc;
}

static void touchGraphic(RS_Graphic* graphic)
{
    // If margin < 0.0, values from dxf file are used.
    double marginLeft = -1.0;
    double marginTop = -1.0;
    double marginRight = -1.0;
    double marginBottom = -1.0;

    int pagesH = 0;      // If number of pages < 1,
    int pagesV = 0;      // use value from dxf file.

    graphic->calculateBorders();
    graphic->setMargins(marginLeft, marginTop,
                        marginRight, marginBottom);
    graphic->setPagesNum(pagesH, pagesV);

    //if (params.pageSize != RS_Vector(0.0, 0.0))
    //    graphic->setPaperSize(params.pageSize);

    graphic->fitToPage(); // fit and center
}

bool slotFileExport(RS_Graphic* graphic, const QString& name, QSize size, QSize borders, bool black, bool bw) {

    if (graphic == nullptr) {
        RS_DEBUG->print(RS_Debug::D_WARNING,
                "QC_ApplicationWindow::slotFileExport: "
                "no graphic");
        return false;
    }

    QImage img(size, QImage::Format_ARGB32);
    if (black) {
        img.fill(Qt::black);
    }
    else {
        img.fill(Qt::white);
    }

    // set painter with buffer
    RS_PainterQt painter(&img);

    if (black) {
        painter.setBackground(Qt::black);
        if (bw) {
            painter.setDrawingMode(RS2::ModeWB);
        }
    }
    else {
        painter.setBackground(Qt::white);
        if (bw) {
            painter.setDrawingMode(RS2::ModeBW);
        }
    }

    double imgFx = (double)img.width() / img.widthMM();
    double imgFy = (double)img.height() / img.heightMM();

    double marginLeft = graphic->getMarginLeft();
    double marginTop = graphic-> getMarginTop();
    double marginRight = graphic->getMarginRight();
    double marginBottom = graphic->getMarginBottom();

    painter.setClipRect(marginLeft * imgFx, marginTop * imgFy,
                        img.width() - (marginLeft + marginRight) * imgFx,
                        img.height() - (marginTop + marginBottom) * imgFy);

    RS_GraphicView gv(img.width(), img.height());
    if (black) {
        gv.setBackground(Qt::black);
    } else {
        gv.setBackground(Qt::white);
    }
    gv.setPrinting(true);

    double fx = imgFx * RS_Units::getFactorToMM(graphic->getUnit());
    double fy = imgFy * RS_Units::getFactorToMM(graphic->getUnit());

    double f = (fx + fy) / 2.0;

    double scale = graphic->getPaperScale();

    gv.setOffset((int)(graphic->getPaperInsertionBase().x * f),
                 (int)(graphic->getPaperInsertionBase().y * f));
    gv.setFactor(f*scale);
    gv.setContainer(graphic);

    gv.setBorders(borders.width(), borders.height(), borders.width(), borders.height());
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
            gv.setOffset((int)((baseX - offsetX) * f),
                         (int)((baseY - offsetY) * f));
            gv.drawEntity(&painter, graphic);
        }
    }

    bool ret = img.save(name);

    // GraphicView deletes painter
    painter.end();

    return ret;
}

/////////////////
/// \brief Parses the user input of PNG output resolution and
/// converts it to a vector value
/// \param arg - input string
/// \return
///
static QSize parsePngSizeArg(QString arg)
{
    QSize v(2970, 2100); // default resolution

    if (arg.isEmpty())
        return v;

    std::regex re("^<width>(\\d+)[xX]{1}<height>(\\d+)$");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        v.setWidth(std::stod(match[1]));
        v.setHeight(std::stod(match[2]));
    } else {
        qDebug() << "WARNING: Ignoring incorrect PNG resolution:" << arg;
    }

    return v;
}
