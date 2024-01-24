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

#include "rs_debug.h"
#include "rs_fontlist.h"

#include "pdf_print_loop.h"

#include <QStringList>

#include <regex>

#define STR(x)   #x
#define XSTR(x)  STR(x)

static RS_Vector parsePageSizeArg(QString);
static void parsePagesNumArg(QString, PdfPrintParams&);
static void parseMarginsArg(QString, PdfPrintParams&);

/////////
/// \brief cad2pdf tool for converting DXF/DWG to PNG/SVG.
/// \param argc
/// \param argv
/// \return
///
int main(int argc, char* argv[])
{
    RS_DEBUG->setLevel(RS_Debug::D_NOTHING);

    QCoreApplication app(argc, argv);
    QCoreApplication::setOrganizationName("cad2pdf");
    QCoreApplication::setApplicationName("cad2pdf");
    QCoreApplication::setApplicationVersion(XSTR(LC_VERSION));

    QStringList appDesc;
    appDesc << "";
    appDesc << "cad2pdf " + QObject::tr( "usage: cad2pdf [options] <dxf/dwg files>");
    appDesc << "";
    appDesc << "Print a bunch of DXF/DWG files to PDF file(s).";
    appDesc << "";
    appDesc << "Examples:";
    appDesc << "";
    appDesc << "  cad2pdf *.dxf|*.dwg|*";
    appDesc << "    " + QObject::tr( "-- print all dxf/dwg files to pdf files with the same names.");
    appDesc << "";
    appDesc << "  cad2pdf -o some.pdf *.dxf|*.dwg|*";
    appDesc << "    " + QObject::tr( "-- print all dxf/dwg files to 'some.pdf' file.");

    QCommandLineParser parser;
    parser.setApplicationDescription( appDesc.join( "\n"));

    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption fitOpt(QStringList() << "a" << "fit",
        QObject::tr( "Auto fit and center drawing to page."));
    parser.addOption(fitOpt);

    QCommandLineOption centerOpt(QStringList() << "c" << "center",
        QObject::tr( "Auto center drawing on page."));
    parser.addOption(centerOpt);

    QCommandLineOption monoOpt(QStringList() << "m" << "monochrome",
        QObject::tr( "Print monochrome (black/white)."));
    parser.addOption(monoOpt);

    QCommandLineOption pageSizeOpt(QStringList() << "p" << "paper",
        QObject::tr( "Paper size (Width x Height) in mm.", "WxH"));
    parser.addOption(pageSizeOpt);

    QCommandLineOption resOpt(QStringList() << "r" << "resolution",
        QObject::tr( "Output resolution (DPI).", "integer"));
    parser.addOption(resOpt);

    QCommandLineOption scaleOpt(QStringList() << "s" << "scale",
        QObject::tr( "Output scale. E.g.: 0.01 (for 1:100 scale)."), "double");
    parser.addOption(scaleOpt);

    QCommandLineOption marginsOpt(QStringList() << "f" << "margins",
        QObject::tr( "Paper margins in mm (integer or float)."), "L,T,R,B");
    parser.addOption(marginsOpt);

    QCommandLineOption pagesNumOpt(QStringList() << "z" << "pages",
        QObject::tr( "Print on multiple pages (Horiz. x Vert.)."), "HxV");
    parser.addOption(pagesNumOpt);

    QCommandLineOption outFileOpt(QStringList() << "o" << "outfile",
        QObject::tr( "Output PDF file.", "file"));
    parser.addOption(outFileOpt);

    QCommandLineOption outDirOpt(QStringList() << "t" << "directory",
        QObject::tr( "Target output directory."), "path");
    parser.addOption(outDirOpt);

    parser.addPositionalArgument(QObject::tr( "<dxf/dwg files>"), QObject::tr( "Input DXF/DWG file(s)"));

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty() || args.size() == 1)
        parser.showHelp(EXIT_FAILURE);

    PdfPrintParams params;

    params.fitToPage = parser.isSet(fitOpt);
    params.centerOnPage = parser.isSet(centerOpt);
    params.monochrome = parser.isSet(monoOpt);
    params.pageSize = parsePageSizeArg(parser.value(pageSizeOpt));

    bool resOk;
    int res = parser.value(resOpt).toInt(&resOk);
    if (resOk)
        params.resolution = res;

    bool scaleOk;
    double scale = parser.value(scaleOpt).toDouble(&scaleOk);
    if (scaleOk)
        params.scale = scale;

    parseMarginsArg(parser.value(marginsOpt), params);
    parsePagesNumArg(parser.value(pagesNumOpt), params);

    params.outFile = parser.value(outFileOpt);
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

    PdfPrintLoop(params).run();
    return 0;
}


static RS_Vector parsePageSizeArg(QString arg)
{
    RS_Vector v(0.0, 0.0);

    if (arg.isEmpty())
        return v;

    std::regex re("^<width>(\\d+)[xX]{1}<height>(\\d+)$");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        v.x = std::stod(match[1]);
        v.y = std::stod(match[2]);
    } else {
        qDebug() << "WARNING: Ignoring bad page size:" << arg;
    }

    return v;
}


static void parsePagesNumArg(QString arg, PdfPrintParams& params)
{
    if (arg.isEmpty())
        return;

    std::regex re("^<horiz>(\\d+)[xX]{1}<vert>(\\d+)$");
    std::smatch match;

    std::string s = arg.toStdString();
    if (std::regex_search(s, match, re)) {
        params.pagesH = std::stoi(match[1]);
        params.pagesV = std::stoi(match[2]);
    } else {
        qDebug() << "WARNING: Ignoring bad number of pages:" << arg;
    }
}


static void parseMarginsArg(QString arg, PdfPrintParams& params)
{
    if (arg.isEmpty())
        return;


    std::regex re(R"(<left>(\d+(?:\.\d+)?),(?=<top>(\d+(?:\.\d+)?),<right>(\d+(?:\.\d+)?),<bottom>(\d+(?:\.\d+)?)))");
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
