#include <tulip/ColorProperty.h>
#include <tulip/StringProperty.h>
#include <tulip/DoubleProperty.h>

#include "tulip/Glyph.h"
#include "tulip/Graph.h"
#include "tulip/GlBox.h"

using namespace std;

namespace tlp {

/** \addtogroup glyph */
/*@{*/
/// A 3D glyph.
/** This glyph draws a textured cube using the "viewTexture" node
 * property value. If this property has no value, the cube is then colored
 * using the "viewColor" node property value. The "viewBorderColor"
 * node property value is used to draw its edges.
 */
  class CubeOutLined : public Glyph {
  public:
    CubeOutLined(GlyphContext *gc=NULL);
    virtual ~CubeOutLined();
    virtual void draw(node n,float lod);
    virtual Coord getAnchor(const Coord & vector) const;

  protected:

    static GlBox* box;

  };
}

#ifdef _WIN32
#ifdef DLL_EXPORT
tlp::GlBox* tlp::CubeOutLined::box=0;
#endif
#else
tlp::GlBox* tlp::CubeOutLined::box=0;
#endif

namespace tlp {
  GLYPHPLUGIN(CubeOutLined, "3D - Cube OutLined", "David Auber", "09/07/2002", "Textured cubeOutLined", "1.0", 1);
/*@}*/

  //===================================================================================
  CubeOutLined::CubeOutLined(GlyphContext *gc): Glyph(gc) {
    if(!box)
      box = new GlBox(Coord(0,0,0),Size(1,1,1),Color(0,0,0,255),Color(0,0,0,255));
  }

  CubeOutLined::~CubeOutLined() {
  }

  void CubeOutLined::draw(node n, float lod) {

    const string& texFile = glGraphInputData->elementTexture->getNodeValue(n);

    box->setFillColor(glGraphInputData->elementColor->getNodeValue(n));
    if (texFile.size() != 0){
      const string& texturePath=glGraphInputData->parameters->getTexturePath();
      box->setTextureName(texturePath+texFile);
    }else
      box->setTextureName("");

    box->setOutlineColor(glGraphInputData->elementBorderColor->getNodeValue(n));
    double lineWidth=glGraphInputData->elementBorderWidth->getNodeValue(n);
    if(lineWidth < 1e-6)
      lineWidth=1e-6;
    box->setOutlineSize(lineWidth);

    box->draw(lod,NULL);
  }


  Coord CubeOutLined::getAnchor(const Coord & vector ) const {
    float x,y,z, fmax;
    vector.get(x,y,z);
    fmax = std::max(std::max(fabsf(x), fabsf(y)), fabsf(z));
    if( fmax > 0.0f )
      return vector * (0.5f/fmax);
    else
      return vector;
  }
}
