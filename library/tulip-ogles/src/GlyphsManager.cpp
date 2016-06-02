/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux
 *
 * Tulip is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3
 * of the License, or (at your option) any later version.
 *
 * Tulip is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 */

/**
 *
 * tulip-ogles library : rewrite of old tulip-ogl library using OpenGL ES API
 * Copyright (c) 2016 Antoine Lambert, Thales Services
 * antoine-e.lambert@thalesgroup.com / antoine.lambert33@gmail.com
 *
 */

#include <tulip/TulipViewSettings.h>
#include <tulip/TulipFontAwesome.h>
#include <tulip/GlyphsManager.h>
#include <tulip/CircleGlyph.h>
#include <tulip/TriangleGlyph.h>
#include <tulip/FontAwesomeGlyph.h>


using namespace std;
using namespace tlp;

GlyphsManager GlyphsManager::_instance;

GlyphsManager& GlyphsManager::instance() {
  return _instance;
}

GlyphsManager::GlyphsManager() {
  CircleGlyph *circleGlyph = new CircleGlyph();
  TriangleGlyph *triangleGlyph = new TriangleGlyph();
  FontAwesomeGlyph *fontAwesomeGlyph = new FontAwesomeGlyph(TulipFontAwesome::getFontAwesomeIconCodePoint(TulipFontAwesome::QuestionCircle));

  _glyphs[circleGlyph->id()] = circleGlyph;
  _glyphs[triangleGlyph->id()] = triangleGlyph;
  _glyphs[fontAwesomeGlyph->id()] = fontAwesomeGlyph;
}

GlyphsManager::~GlyphsManager() {
  for(const pair<int, Glyph*> glyph : _glyphs) {
    delete glyph.second;
  }
  for(const pair<int, Glyph*> glyph : _faGlyphs) {
    delete glyph.second;
  }
}

Glyph *GlyphsManager::getGlyph(int glyphId) {
  // for backward compatibility with Tulip 4.x (50 was the id for the Arrow edge extremity shape)
  if (glyphId == 50) {
    return _glyphs[tlp::NodeShape::Triangle];
  }
  if (glyphId >= 0xf000) {
    if(_faGlyphs.find(glyphId) == _faGlyphs.end()) {
      _faGlyphs[glyphId] = new FontAwesomeGlyph(glyphId);
    }
    return _faGlyphs[glyphId];
  }
  if (_glyphs.find(glyphId) == _glyphs.end()) {
    return _glyphs[tlp::NodeShape::Circle];
  } else {
    return _glyphs[glyphId];
  }
}

void GlyphsManager::loadGlyphPlugins() {
  list<string> glyphs(PluginLister::instance()->availablePlugins<Glyph>());

  for(const std::string &glyphName : glyphs) {
    Glyph *glyph = PluginLister::instance()->getPluginObject<Glyph>(glyphName, nullptr);
    if (_glyphs.find(glyph->id()) == _glyphs.end()) {
      _glyphs[glyph->id()] = glyph;
    } else {
      delete glyph;
    }
  }
}