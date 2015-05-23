/****************************************************************************/
/***                                                                      ***/
/***   (C) 2012-2014 Dierk Ohlerich et al., all rights reserved.          ***/
/***                                                                      ***/
/***   Released under BSD 2 clause license, see LICENSE.TXT               ***/
/***                                                                      ***/
/****************************************************************************/

#ifndef FILE_ALTONA2_EXAMPLES_GRAPHICS_CUBE_MAIN_HPP
#define FILE_ALTONA2_EXAMPLES_GRAPHICS_CUBE_MAIN_HPP

#include "altona2/libs/base/base.hpp"
#include "altona2/libs/util/debugpainter.hpp"
#include "shader.hpp"

using namespace Altona2;

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

class App : public sApp
{
  sMaterial *Mtrl[2];
  sGeometry *Geo;
  sCBuffer<TesselateShader_cbd0> *cbd0;
  sCBuffer<TesselateShader_cbh0> *cbh0;
  sDebugPainter *DPaint;

  sAdapter *Adapter;
  sContext *Context;
  sScreen *Screen;
public:
  App();
  ~App();

  void OnInit();
  void OnExit();
  void OnFrame();
  void OnPaint();
  void OnKey(const sKeyData &kd);
  void OnDrag(const sDragData &dd);

  sInt Quality;
  sInt Wireframe;
  sInt FreeCam;
};

/****************************************************************************/

#endif  // FILE_ALTONA2_EXAMPLES_GRAPHICS_CUBE_MAIN_HPP

