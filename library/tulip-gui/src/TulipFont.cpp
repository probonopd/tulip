#include "tulip/TulipFont.h"

#include <QtGui/QFontDatabase>
#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <tulip/TlpTools.h>
#include <QtCore/QDebug>

using namespace tlp;

QMap<QString,int> TulipFont::FONT_IDS = QMap<QString,int>();

QString TulipFont::tulipFontsDirectory() {
  return (TulipBitmapDir + "fonts/").c_str();
}

QStringList TulipFont::installedFontNames() {
  QStringList result;
  QDir installedFontsDir(tulipFontsDirectory());
  foreach(QFileInfo fontDirInfo, installedFontsDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs)) {
    QString fontName(fontDirInfo.fileName());
    // sanity checks
    TulipFont normalFont(fontName), boldFont(normalFont), italicFont(normalFont), boldItalicFont(normalFont);
    boldFont.setBold(true);
    italicFont.setItalic(true);
    boldItalicFont.setBold(true);
    boldItalicFont.setItalic(true);

    if (!normalFont.exists() || !boldFont.exists() || !italicFont.exists() || !boldItalicFont.exists())
      continue;

    result << fontName;
  }
  return result;
}

TulipFont TulipFont::fromFile(const QString &path) {
  TulipFont result("");
  result._fontFile = path;
  result._bold = path.endsWith("_Bold.ttf") || path.endsWith("_Bold_Italic.ttf");
  result._italic = path.endsWith("_Italic.ttf") || path.endsWith("_Bold_Italic.ttf");
  result._fontName = QFileInfo(path).fileName().remove("_Bold").remove("_Italic").remove(".ttf");
  return result;
}

TulipFont& TulipFont::operator =(const TulipFont& other) {
  _bold = other._bold;
  _italic = other._italic;
  _fontName = other._fontName;
  _fontFile = other._fontFile;
  return *this;
}

TulipFont::TulipFont(const TulipFont& other): QObject(other.parent()) {
  *this = other;
}

TulipFont::TulipFont(const QString fontName, QObject *parent): QObject(parent), _bold(false), _italic(false), _fontName(fontName) {
  refreshFontFile();
}

TulipFont::TulipFont(QObject *parent): QObject(parent), _bold(false), _italic(false), _fontName(QString::null) {
  refreshFontFile();
}

bool TulipFont::isBold() const {
  return _bold;
}

bool TulipFont::isItalic() const {
  return _italic;
}

QString TulipFont::fontName() const {
  return _fontName;
}

int TulipFont::fontId(const QString& path) {
  int result = -1;

  if (!FONT_IDS.contains(path)) {
    result = QFontDatabase::addApplicationFont(path);
    FONT_IDS[path] = result;
  }
  else {
    result = FONT_IDS[path];
  }

  return result;
}

int TulipFont::fontId() const {
  return fontId(_fontFile);
}

QString TulipFont::fontFamily() const {
  QStringList families = QFontDatabase::applicationFontFamilies(fontId());
  QString family = trUtf8("Unregistered font");

  if (families.size()>0)
    family = families[0];

  return family;
}

QString TulipFont::fontFile() const {
  return _fontFile;
}

void TulipFont::setItalic(bool i) {
  _italic = i;
  refreshFontFile();
}

void TulipFont::setBold(bool b) {
  _bold = b;
  refreshFontFile();
}

void TulipFont::setFontName(const QString& n) {
  _fontName = n;
  refreshFontFile();
}

bool TulipFont::exists() const {
  return !_fontFile.isNull() && QFileInfo(fontFile()).exists();
}

void TulipFont::refreshFontFile() {
  _fontFile = tulipFontsDirectory() + fontName() + "/" + fontName();

  if (isBold())
    _fontFile += "_Bold";

  if (isItalic())
    _fontFile += "_Italic";

  _fontFile += ".ttf";
}
