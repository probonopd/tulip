/**
 *
 * This file is part of Tulip (www.tulip-software.org)
 *
 * Authors: David Auber and the Tulip development Team
 * from LaBRI, University of Bordeaux 1 and Inria Bordeaux - Sud Ouest
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
#include <iostream>

#ifdef  _WIN32
// compilation pb workaround
#include <windows.h>
#endif
#include <QtGui/qevent.h>

#include <tulip/Graph.h>
#include <tulip/GlMainWidget.h>
#include <tulip/GlTools.h>

#include "tulip/MouseBoxZoomer.h"
#include "tulip/QtGlSceneZoomAndPanAnimator.h"

using namespace std;
using namespace tlp;

MouseBoxZoomer::MouseBoxZoomer(Qt::MouseButton button,
			       Qt::KeyboardModifier modifier)
  : mButton(button), kModifier(modifier), x(0), y(0), w(0), h(0), started(false), graph(0) {}
MouseBoxZoomer::~MouseBoxZoomer() {}
//=====================================================================
bool MouseBoxZoomer::eventFilter(QObject *widget, QEvent *e) {
  if (e->type() == QEvent::MouseButtonPress) {
    QMouseEvent * qMouseEv = (QMouseEvent *) e;
    GlMainWidget *glw = (GlMainWidget *) widget;
    if (qMouseEv->buttons() == mButton &&
	(kModifier == Qt::NoModifier ||
	 ((QMouseEvent *) e)->modifiers() & kModifier)) {
      if (!started) {
  x = (double) glw->width() - qMouseEv->x();
  y = qMouseEv->y();
	w = 0; h = 0;
	started = true;
	glw->setMouseTracking(true);
	graph = glw->getScene()->getGlGraphComposite()->getInputData()->getGraph();
      }
      else {
	if (glw->getScene()->getGlGraphComposite()->getInputData()->getGraph() != graph) {
	  graph = NULL;
	  started = false;
	  glw->setMouseTracking(false);
	}
      }
      return true;
    }
    if (qMouseEv->buttons()==Qt::MidButton){
      started = false;
      glw->setMouseTracking(false);
      glw->redraw();
      return true;
    }
    return false;
  }
  if (e->type() == QEvent::MouseMove &&
      ((((QMouseEvent *) e)->buttons() & mButton) &&
       	(kModifier == Qt::NoModifier ||
	 ((QMouseEvent *) e)->modifiers() & kModifier))) {
    QMouseEvent * qMouseEv = (QMouseEvent *) e;
    GlMainWidget *glw = (GlMainWidget *) widget;
    if (glw->getScene()->getGlGraphComposite()->getInputData()->getGraph() != graph) {
      graph = NULL;
      started = false;
      glw->setMouseTracking(false);
    }
    if (started){
      if ((qMouseEv->x() > 0) && (qMouseEv->x() < glw->width()))
  w = (double) glw->width()- qMouseEv->x() - x;
      if ((qMouseEv->y() > 0) && (qMouseEv->y() < glw->height()))
  h = qMouseEv->y() - y;
      glw->redraw();
    return true;
    }
  }
  if (e->type() == QEvent::MouseButtonRelease &&
      (((QMouseEvent *) e)->button() == mButton &&
       	(kModifier == Qt::NoModifier ||
	 ((QMouseEvent *) e)->modifiers() & kModifier))) {
    GlMainWidget *glw = (GlMainWidget *) widget;
    if (glw->getScene()->getGlGraphComposite()->getInputData()->getGraph() != graph) {
      graph = NULL;
      started = false;
      glw->setMouseTracking(false);
    }
    if (started) {
      started = false;
      if(!(w==0 && h==0)){
        if (abs(w) < 10){
          w=10;
        }
        if(abs(h) < 10) {
          h=10;
        }

        Coord point1(x,y,0);
        Coord point2(x + w ,y + h,0);

        cout << point1 << " # " << point2 << endl;

        BoundingBox boundingBox;
        boundingBox.expand(glw->getScene()->getCamera()->screenTo3DWorld(point1));
        boundingBox.expand(glw->getScene()->getCamera()->screenTo3DWorld(point2));

        QtGlSceneZoomAndPanAnimator navigator(glw,boundingBox);
        navigator.animateZoomAndPan();
        //glw->draw(false);
      }
    }
    return true;
  }
  return false;
}
//=====================================================================
bool MouseBoxZoomer::draw(GlMainWidget *glw) {
  if (!started) return false;
  if (glw->getScene()->getGlGraphComposite()->getInputData()->getGraph() != graph) {
    graph = NULL;
    started = false;
    glw->setMouseTracking(false);
  }
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glMatrixMode (GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0, (GLdouble)glw->width(), 0.0, (GLdouble)glw->height());
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_SRC_COLOR);

  float col[4] = {0.8, 0.4, 0.4, 0.2};
  setColor(col);
  glBegin(GL_QUADS);
  glVertex2f((double) glw->width()-x, glw->height()-y);
  glVertex2f((double) glw->width()-x-w, glw->height()-y);
  glVertex2f((double) glw->width()-x-w, glw->height()-y-h);
  glVertex2f((double) glw->width()-x, glw->height()-y-h);
  glEnd();
  glDisable(GL_BLEND);

  glLineWidth(2);
  glLineStipple(2, 0xAAAA);
  glEnable(GL_LINE_STIPPLE);
  glBegin(GL_LINE_LOOP);
  glVertex2f((double) glw->width()-x, glw->height()-y);
  glVertex2f((double) glw->width()-x-w, glw->height()-y);
  glVertex2f((double) glw->width()-x-w, glw->height()-y-h);
  glVertex2f((double) glw->width()-x, glw->height()-y-h);
  glEnd();
  /*
  glLineWidth(1);
  glDisable(GL_LINE_STIPPLE);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_LIGHTING);
  */
  glPopMatrix();
  glMatrixMode (GL_PROJECTION);
  glPopMatrix();
  glMatrixMode (GL_MODELVIEW);
  glPopAttrib();
  return true;
}

//=====================================================================
