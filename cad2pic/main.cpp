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
    // RS_DEBUG->setLevel(RS_Debug::D_WARNING);

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
        qDebug() << "slotFileExport" << outFile;
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
    qDebug() << "painter" << name;

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

    painter.eraseRect(0, 0, size.width(), size.height());
    qDebug() << "eraseRect" << name;

    RS_GraphicView gv(size.width(), size.height());
    if (black) {
        gv.setBackground(Qt::black);
    } else {
        gv.setBackground(Qt::white);
    }
    gv.setPrinting(true);
    gv.setBorders(borders.width(), borders.height(), borders.width(), borders.height());
    gv.setContainer(graphic);
    gv.drawEntity(&painter, graphic);
    qDebug() << "drawEntity" << name;

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
    QSize v(2000, 1000); // default resolution

    if (arg.isEmpty())
        return v;

    QRegularExpression re("^(?<width>\\d+)[x|X]{1}(?<height>\\d+)$");
    QRegularExpressionMatch match = re.match(arg);

    if (match.hasMatch()) {
        QString width = match.captured("width");
        QString height = match.captured("height");
        v.setWidth(width.toDouble());
        v.setHeight(height.toDouble());
    } else {
        qDebug() << "WARNING: Ignoring incorrect PNG resolution:" << arg;
    }

    return v;
}
