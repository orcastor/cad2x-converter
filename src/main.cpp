/******************************************************************************
**
** This file was created for the LibreCAD project, a 2D CAD program.
**
** Copyright (C) 2018 Alexander Pravdin <aledin@mail.ru>
** Copyright (C) 2022 A. Stebich (librecad@mail.lordofbikes.de)
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
#include <QCoreApplication>
#include <QImageWriter>
#include <QPdfWriter>
#include <QImageWriter>

#include "rs.h"
#include "rs_debug.h"
#include "rs_fontlist.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_painterqt.h"
#include "rs_units.h"
#include "rs_vector.h"
#include "rs_filterdxfrw.h"

#include "lc_makercamsvg.h"
#include "lc_xmlwriterqxmlstreamwriter.h"

#include <regex>

#define STR(x)   #x
#define XSTR(x)  STR(x)

struct PrintParams {
    QStringList files;
    QString outDir;
    QString outFile;
    int resolution = 1200; // QPrinter::HighResolution
    bool fitToPage = false;
    bool monochrome = false;
    bool autoOrientation = false;
    double scale = 0.0;  // If scale <= 0.0, use value from dxf file.
    RS_Vector paperSize;  // If zeros, use value from dxf file.
    struct {
        double left = -1.0;
        double top = -1.0;
        double right = -1.0;
        double bottom = -1.0;
    } margins;           // If margin < 0.0, use value from dxf file.
    int pagesH = 0;      // If number of pages < 1,
    int pagesV = 0;      // use value from dxf file.
};

void parsePaperSizeArg(const QString&, PrintParams&);
void parsePagesNumArg(const QString&, PrintParams&);
void parseMarginsArg(const QString&, PrintParams&);

QPageSize toPageSize(RS2::PaperFormat paper);
std::unique_ptr<RS_Document> openDocAndSetGraphic(const QString& file);
void touchGraphic(RS_Graphic* graphic, const PrintParams& params);
void drawPage(RS_Graphic* graphic, std::unique_ptr<QPaintDevice>& pd, RS_PainterQt& painter);
QString getFormatFromFile(const QString& fileName);

int main(int argc, char* argv[])
{
    // RS_DEBUG->setLevel(RS_Debug::D_DEBUGGING);

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("cad2x");
    QCoreApplication::setApplicationName("cad2x");
    QCoreApplication::setApplicationVersion(XSTR(LC_VERSION));

    QStringList appDesc;
    appDesc << "";
    appDesc << "Convert DXF/DWG files to DXF v2007/PDF/PNG/SVG files.";
    appDesc << "";
    appDesc << "Examples:";
    appDesc << "";
    appDesc << "  cad2x *.dxf|*.dwg|*";
    appDesc << "    -- print all dxf/dwg files to pdf files with the same base names.";
    appDesc << "";
    appDesc << "  cad2x -o svg *.dxf|*.dwg|*";
    appDesc << "    -- print all dxf/dwg files to svg format.";
    appDesc << "";
    appDesc << "  cad2x -o b.png a.dwg";
    appDesc << "    -- print a.dwg file to b.png.";

    QCommandLineParser parser;
    parser.setApplicationDescription( appDesc.join( "\n"));

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption autoOrientationOpt(QStringList() << "a" << "auto-orientation", "Auto paper orientation (landscape or portrait) based on document bounding box.");
    parser.addOption(autoOrientationOpt);

    QCommandLineOption monoOpt(QStringList() << "b" << "monochrome", "Print monochrome (black/white).");
    parser.addOption(monoOpt);

    QCommandLineOption fitOpt(QStringList() << "c" << "fit", "Auto fit and center drawing to page.");
    parser.addOption(fitOpt);

    QCommandLineOption codePageOpt(QStringList() << "e" << "code-page", "Set default code page (default is ANSI_1252).", "codepage");
    parser.addOption(codePageOpt);

    QCommandLineOption fontOpt(QStringList() << "f" << "default-font", "Set default font (default is standard).", "font");
    parser.addOption(fontOpt);

    QCommandLineOption marginsOpt(QStringList() << "m" << "margins", "Paper margins in mm (integer or float).", "L,T,R,B");
    parser.addOption(marginsOpt);

    QCommandLineOption pagesNumOpt(QStringList() << "n" << "pages", "Print on multiple pages (Horiz. x Vert.).", "HxV");
    parser.addOption(pagesNumOpt);

    QCommandLineOption outFileOpt(QStringList() << "o" << "outfile", "Output DXF v2007/PDF/PNG/SVG file.", "file");
    parser.addOption(outFileOpt);

    QCommandLineOption paperSizeOpt(QStringList() << "p" << "paper", "Paper size (Width x Height) in mm.", "WxH");
    parser.addOption(paperSizeOpt);

    QCommandLineOption resOpt(QStringList() << "r" << "resolution", "Output resolution (DPI).", "integer");
    parser.addOption(resOpt);

    QCommandLineOption scaleOpt(QStringList() << "s" << "scale", "Output scale. E.g.: 0.01 (for 1:100 scale).", "double");
    parser.addOption(scaleOpt);

    QCommandLineOption outDirOpt(QStringList() << "t" << "directory", "Target output directory.", "path");
    parser.addOption(outDirOpt);

    parser.addPositionalArgument("<dxf/dwg files>", "Input DXF/DWG file(s)");

    parser.process(app);

    const QStringList args = parser.positionalArguments();
    if (args.isEmpty())
        parser.showHelp(EXIT_FAILURE);

    PrintParams params;
    params.autoOrientation = parser.isSet(autoOrientationOpt);
    params.fitToPage = parser.isSet(fitOpt);
    params.monochrome = parser.isSet(monoOpt);

    bool resOk;
    int res = parser.value(resOpt).toInt(&resOk);
    if (resOk)
        params.resolution = res;

    bool scaleOk;
    double scale = parser.value(scaleOpt).toDouble(&scaleOk);
    if (scaleOk)
        params.scale = scale;

    parsePaperSizeArg(parser.value(paperSizeOpt), params);
    parseMarginsArg(parser.value(marginsOpt), params);
    parsePagesNumArg(parser.value(pagesNumOpt), params);

    if (parser.isSet(codePageOpt)) {
        defaultCodePage = parser.value(codePageOpt);
    }
    if (parser.isSet(fontOpt)) {
        RS_FONTLIST->setDefaultFont(parser.value(fontOpt));
    }

    params.outFile = parser.value(outFileOpt);
    if (!params.outFile.contains(".")) {
        params.outFile = "." + params.outFile;
    }
    params.outDir = parser.value(outDirOpt);

    for (auto arg : args) {
        QFileInfo fi(arg);
        if (fi.suffix().toLower() != "dxf" && fi.suffix().toLower() != "dwg")
            continue; // Skip files without .dxf/.dwg extension
        params.files.append(arg);
    }

    if (params.files.isEmpty())
        parser.showHelp(EXIT_FAILURE);

    if (!params.outDir.isEmpty()) {
        // Create output directory
        if (!QDir().mkpath(params.outDir)) {
            qDebug() << "ERROR: Cannot create directory" << params.outDir;
            return EXIT_FAILURE;
        }
    }

    RS_FONTLIST->init();

    for (auto file : params.files) {
        // find out extension:
        QString format = getFormatFromFile(params.outFile);
        QFileInfo fi(file);

        QString baseName;
        if (params.files.length() == 1) {
            QFileInfo ofi(params.outFile);
            baseName = ofi.completeBaseName();
            if (baseName.isEmpty()) {
                baseName = fi.completeBaseName();
            }
        } else {
            baseName = fi.completeBaseName();
        }

        params.outFile = (params.outDir.isEmpty() ? fi.path() : params.outDir) + "/" + baseName + "." + format;

        qDebug() << "Convert" << file
                 << "to" << params.outFile << ">>>>";

        bool ok = false;
        do {
            // Open the file and process the graphics
            std::unique_ptr<RS_Document> doc = openDocAndSetGraphic(file);
            if (doc == nullptr || doc->getGraphic() == nullptr)
                break;

            if (format == "dxf") {
                ok = doc->saveAs(params.outFile, RS2::FormatDXFRW); // DXF - v2007 defaulted
                break;
            }

            RS_Graphic *graphic = doc->getGraphic();
            touchGraphic(graphic, params);

            if (format.toLower() == "svg") {
                auto generator = std::make_unique<LC_MakerCamSVG>(std::make_unique<LC_XMLWriterQXmlStreamWriter>());
                generator->generate(graphic);
                QFile file{params.outFile};
                if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                    LC_ERR << __func__ << "(): failed in creating file " << params.outFile << ", no SVG is generated";
                    break;
                }

                QTextStream out(&file);
                out << QString::fromStdString(generator->resultAsString());
                ok = true;
            } else { // pdf / img
                bool landscape = false;
                RS2::PaperFormat pf = graphic->getPaperFormat(&landscape);

                if (params.autoOrientation) {
                    // auto detect orientation
                    landscape = (graphic->getMax().x - graphic->getMin().x) > (graphic->getMax().y - graphic->getMin().y);
                }

                QPageSize pageSize = toPageSize(pf);
                if (pageSize.id() == QPageSize::Custom){
                    RS_Vector r = graphic->getPaperSize();
                    RS_Vector s = RS_Units::convert(r, graphic->getUnit(), RS2::Millimeter);
                    if (landscape) s = s.flipXY();
                    pageSize = QPageSize{QSizeF{s.x, s.y}, QPageSize::Millimeter};
                }

                std::unique_ptr<QPaintDevice> pd;
                if (format.toLower() == "pdf") {
                    pd = std::make_unique<QPdfWriter>(params.outFile);
                    ((QPdfWriter *)pd.get())->setPageSize(pageSize);
                    ((QPdfWriter *)pd.get())->setPageOrientation(landscape ? QPageLayout::Landscape : QPageLayout::Portrait);
                    ((QPdfWriter *)pd.get())->setResolution(params.resolution);
                } else { // img
                    QSize size = pageSize.sizePoints();
                    int w = size.width();
                    int h = size.height();
                    if (landscape ^ (w > h)) {w = h; h = w;} // auto change orientation

                    pd = std::make_unique<QImage>(w, h, QImage::Format_ARGB32);
                    ((QImage *)pd.get())->fill(Qt::white);
                }

                RS_PainterQt painter(pd.get());
                painter.setBackground(Qt::white);
                if (params.monochrome) {
                    painter.setDrawingMode(RS2::ModeBW);
                }
                drawPage(graphic, pd, painter);
                painter.end();
                
                ok = format.toLower() == "pdf" ? true: ((QImage *)pd.get())->save(params.outFile);
            }
        } while (false);

        qDebug() << "Convert" << file
                 << "to" << params.outFile << (ok ? "Done" : "Failed");
    }
    return 0;
}

void parsePaperSizeArg(const QString& arg, PrintParams& params)
{
    if (arg.isEmpty())
        return;

    std::regex re("^(\\d+(?:\\.\\d+)?)[xX]{1}(\\d+(?:\\.\\d+)?)$");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        params.paperSize.x = std::stod(match[1]);
        params.paperSize.y = std::stod(match[2]);
    } else {
        qDebug() << "WARNING: Ignoring bad page size:" << arg;
    }
}

void parsePagesNumArg(const QString& arg, PrintParams& params)
{
    if (arg.isEmpty())
        return;

    std::regex re("^(\\d+)[xX]{1}(\\d+)$");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        params.pagesH = std::stoi(match[1]);
        params.pagesV = std::stoi(match[2]);
    } else {
        qDebug() << "WARNING: Ignoring bad number of pages:" << arg;
    }
}

void parseMarginsArg(const QString& arg, PrintParams& params)
{
    if (arg.isEmpty())
        return;

    std::regex re("(\\d+(?:\\.\\d+)?),(\\d+(?:\\.\\d+)?),(\\d+(?:\\.\\d+)?),(\\d+(?:\\.\\d+)?)");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        params.margins.left = std::stod(match[1]);
        params.margins.top = std::stod(match[2]);
        params.margins.right = std::stod(match[3]);
        params.margins.bottom = std::stod(match[4]);
    } else {
        qDebug() << "WARNING: Ignoring bad paper margins:" << arg;
    }
}

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

std::unique_ptr<RS_Document> openDocAndSetGraphic(const QString& file)
{
    auto doc = std::make_unique<RS_Graphic>();

    if (!doc->open(file, RS2::FormatUnknown)) {
        qDebug() << "ERROR: Failed to open document" << file;
        qDebug() << "Check if file exists";
        return {};
    }

    RS_Graphic* graphic = doc->getGraphic();
    if (graphic == nullptr) {
        qDebug() << "ERROR: No graphic in" << file;
        return {};
    }

    return doc;
}

void touchGraphic(RS_Graphic* graphic, const PrintParams& params)
{
    graphic->calculateBorders();
    graphic->setMargins(params.margins.left, params.margins.top,
                        params.margins.right, params.margins.bottom);
    graphic->setPagesNum(params.pagesH, params.pagesV);

    if (params.scale > 0.0)
        graphic->setPaperScale(params.scale);

    if (params.paperSize != RS_Vector(0.0, 0.0))
        graphic->setPaperSize(params.paperSize);

    if (params.fitToPage)
        graphic->fitToPage(); // fit and center
}

void drawPage(RS_Graphic* graphic, std::unique_ptr<QPaintDevice>& pd, RS_PainterQt& painter)
{
    double deviceFx = (double)pd->width() / pd->widthMM();
    double deviceFy = (double)pd->height() / pd->heightMM();

    double marginLeft = graphic->getMarginLeft();
    double marginTop = graphic-> getMarginTop();
    double marginRight = graphic->getMarginRight();
    double marginBottom = graphic->getMarginBottom();

    painter.setClipRect(marginLeft * deviceFx, marginTop * deviceFy,
                        pd->width() - (marginLeft + marginRight) * deviceFx,
                        pd->height() - (marginTop + marginBottom) * deviceFy);

    RS_GraphicView gv(pd->width(), pd->height());
    gv.setBackground(Qt::white);
    gv.setPrinting(true);

    double fx = deviceFx * RS_Units::getFactorToMM(graphic->getUnit());
    double fy = deviceFy * RS_Units::getFactorToMM(graphic->getUnit());
    double f = (fx + fy) / 2.0;

    double scale = graphic->getPaperScale();

    gv.setOffset((int)ceil(graphic->getPaperInsertionBase().x * f),
                 (int)ceil(graphic->getPaperInsertionBase().y * f));
    gv.setFactor(f*scale);
    gv.setContainer(graphic);
    gv.zoomAuto(false);

    int numX = graphic->getPagesNumHoriz();
    int numY = graphic->getPagesNumVert();
    if (numX * numY == 1) {
        gv.drawEntity(&painter, graphic);
        return;
    }

    double baseX = graphic->getPaperInsertionBase().x;
    double baseY = graphic->getPaperInsertionBase().y;
    RS_Vector printArea = graphic->getPrintAreaSize(false);
    for (int pY = 0; pY < numY; pY++) {
        double offsetY = printArea.y * pY;
        for (int pX = 0; pX < numX; pX++) {
            double offsetX = printArea.x * pX;
            // First page is created automatically.
            // Extra pages must be created manually.
            QPagedPaintDevice* paged = dynamic_cast<QPagedPaintDevice*>(pd.get());
            if (paged) if (pX > 0 || pY > 0) paged->newPage();
            gv.setOffset((int)ceil((baseX - offsetX) * f),
                         (int)ceil((baseY - offsetY) * f));
            gv.drawEntity(&painter, graphic);
        }
    }
}

// find the image format from the file extension; default to pdf
QString getFormatFromFile(const QString& fileName)
{
    QList<QByteArray> supportedImageFormats = QImageWriter::supportedImageFormats();
    supportedImageFormats.push_back("svg"); // add svg
    supportedImageFormats.push_back("dxf"); // add dxf

    for (QString format: supportedImageFormats) {
        if (fileName.endsWith(format, Qt::CaseInsensitive))
            return format;
    }
    return "pdf";
}
