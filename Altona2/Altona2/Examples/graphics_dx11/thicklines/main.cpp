/****************************************************************************/
/***                                                                      ***/
/***   (C) 2012-2014 Dierk Ohlerich et al., all rights reserved.          ***/
/***                                                                      ***/
/***   Released under BSD 2 clause license, see LICENSE.TXT               ***/
/***                                                                      ***/
/****************************************************************************/

#include "main.hpp"
#include "shader.hpp"
#include "altona2/libs/util/graphicshelper.hpp"

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

void Altona2::Main()
{
  sInt flags = 0;
//  flags |= sSM_Fullscreen;
  sRunApp(new App,sScreenMode(flags,"thick lines (using gs)",1280,720));
}

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

App::App()
{
  Mtrl = 0;
  Geo = 0;
}

App::~App()
{
}

void App::OnInit()
{
  Screen = sGetScreen();
  Adapter = Screen->Adapter;
  Context = Adapter->ImmediateContext;

  Mtrl = new sMaterial(Adapter);
  Mtrl->SetShaders(ThickLinesShader.Get(0));
  Mtrl->SetState(sRenderStatePara(sMTRL_DepthOn|sMTRL_CullOff,sMB_Off));
  Mtrl->Prepare(Adapter->FormatP);


  const sInt tx = 16;
  const sInt ty = 8;

  const sInt ic = tx*ty*4*2;
  const sInt vc = (tx)*(ty);
  sU16 ib[ic];
  sVertexP vb[vc];
  sF32 ri = 0.25f;
  sF32 ro = 1.00f;

  sVertexP *vp = vb;
  for(sInt y=0;y<ty;y++)
  {
    sF32 fy = sF32(y)/ty*s2Pi;
    sF32 py = y!=ty ? fy : 0;
    for(sInt x=0;x<tx;x++)
    {
      sF32 fx = sF32(x)/tx*s2Pi;
      sF32 px = x!=tx ? fx : 0;
      vp->px = -sCos(px)*(ro+sCos(py)*ri);
      vp->py = -             sSin(py)*ri;
      vp->pz =  sSin(px)*(ro+sCos(py)*ri);
      vp++;
    }
  }
  sU16 *ip = ib;
  sInt n = 0;
  for(sInt y=0;y<ty;y++)
  {
    for(sInt x=0;x<tx;x++)
    {
      ip[n+0] = (x+0)%tx + (y+0)%ty*tx;
      ip[n+1] = (x+0)%tx + (y+0)%ty*tx;
      ip[n+2] = (x+1)%tx + (y+0)%ty*tx;
      ip[n+3] = (x+1)%tx + (y+1)%ty*tx;
      n+=4;
      ip[n+0] = (x+0)%tx + (y+1)%ty*tx;
      ip[n+1] = (x+0)%tx + (y+1)%ty*tx;
      ip[n+2] = (x+0)%tx + (y+0)%ty*tx;
      ip[n+3] = (x+1)%tx + (y+0)%ty*tx;
      n+=4;
    }
  }

  Geo = new sGeometry(Adapter);
  Geo->SetIndex(sResBufferPara(sRBM_Index|sRU_Static,sizeof(sU16),ic),ib);
  Geo->SetVertex(sResBufferPara(sRBM_Vertex|sRU_Static,sizeof(sVertexP),vc),vb);
  Geo->Prepare(Adapter->FormatP,sGMP_LinesAdj|sGMF_Index16,ic,0,vc,0);

  cbv0 = new sCBuffer<ThickLinesShader_cbvs>(Adapter,sST_Vertex,0);
  cbg0 = new sCBuffer<ThickLinesShader_cbgs>(Adapter,sST_Geometry,0);

  DPaint = new sDebugPainter(Adapter);
}

void App::OnExit()
{
  delete DPaint;
  delete Mtrl;
  delete Geo;
  delete cbv0;
  delete cbg0;
}

void App::OnFrame()
{
}

void App::OnPaint()
{
  sU32 utime = sGetTimeMS();
  sF32 time = utime*0.01f;
  sTargetPara tp(sTAR_ClearAll,0xff405060,Screen);
  sViewport view;

  Context->BeginTarget(tp);

  view.Camera.k.w = -2;
  view.Model = sEulerXYZ(time*0.0031f,time*0.0043f,time*0.0055f);
  view.ZoomX = 1/tp.Aspect;
  view.ZoomY = 1;
  view.Prepare(tp);

  cbv0->Map();
  cbv0->Data->mat = view.MS2SS;
  cbv0->Unmap();
  cbg0->Map();
  cbg0->Data->ldir.Set(-view.MS2CS.k.x,-view.MS2CS.k.y,-view.MS2CS.k.z);
  cbg0->Unmap();

  Context->Draw(sDrawPara(Geo,Mtrl,cbv0,cbg0));

  DPaint->PrintFPS();
  DPaint->PrintStats();
  DPaint->Draw(tp);

  Context->EndTarget();
}

void App::OnKey(const sKeyData &kd)
{
  if((kd.Key&(sKEYQ_Mask|sKEYQ_Break))==27)
    sExit();
}

void App::OnDrag(const sDragData &dd)
{
}

/****************************************************************************/
/***                                                                      ***/
/***                                                                      ***/
/***                                                                      ***/
/****************************************************************************/

/****************************************************************************/

