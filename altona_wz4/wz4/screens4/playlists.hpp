/*+**************************************************************************/
/***                                                                      ***/
/***   This file is distributed under a BSD license.                      ***/
/***   See LICENSE.txt for details.                                       ***/
/***                                                                      ***/
/**************************************************************************+*/

#ifndef FILE_SCREENS4_PLAYLISTS_HPP
#define FILE_SCREENS4_PLAYLISTS_HPP

#include "base/types.hpp"
#include "base/types2.hpp"
#include "util/image.hpp"

/****************************************************************************/

class RefCountObj
{
public:
  int AddRef()  { return sAtomicInc(&RefCount); }
  int Release() { int rc=sAtomicDec(&RefCount);  if (!rc) delete this; return rc; }

  RefCountObj() : RefCount(1) {}

private:
  volatile sU32 RefCount;

protected:
  virtual ~RefCountObj() {};
};

/****************************************************************************/

class AssetMetaData
{
public:
  sString<128> ETag;

  void Serialize(sReader &r) { Serialize_(r); }
  void Serialize(sWriter &w) { Serialize_(w); }

private:

  template<class Streamer> void Serialize_(Streamer &s);

};

class Asset : public RefCountObj
{
public:
  sString<256> Path;
  
  AssetMetaData Meta;

  enum { INVALID, NOTCACHED, CACHED, } volatile CacheStatus;

  sDNode RefreshNode;

  Asset() : CacheStatus(INVALID) {}
};

/****************************************************************************/

class PlaylistItem
{
public:
  sString<64> ID;
  sString<64> Type;
  sString<256> Path;

  sF32 Duration;
  sInt TransitionId;
  sF32 TransitionDuration;
  sBool ManualAdvance;
  sBool Mute;

  sString<10> BarColor, BarBlinkColor1, BarBlinkColor2;
  sF32 BarAlpha;
  sArray<sFRect> BarPositions;

  void Serialize(sReader &r) { Serialize_(r); }
  void Serialize(sWriter &w) { Serialize_(w); }

  Asset *MyAsset;

  PlaylistItem() : MyAsset(0) {}
  ~PlaylistItem() { sRelease(MyAsset); }

private:

  template<class Streamer> void Serialize_(Streamer &s);

};

class Playlist : public RefCountObj
{
public:
  sString<64> ID;
  sString<256> CallbackUrl;
  sU32 Timestamp;
  sBool Loop;

  sInt LastPlayedItem;

  sArray<PlaylistItem*> Items;

  sBool Dirty; // needs saving?
  sDNode Node, CacheNode;

  Playlist()
  {
    Dirty = sTRUE;
    Loop = sTRUE;
    LastPlayedItem = 0;
  }

  void Serialize(sReader &r) { Serialize_(r); }
  void Serialize(sWriter &w) { Serialize_(w); }

private:
  
  ~Playlist()
  {
    sDeleteAll(Items);
  }

  template<class Streamer> void Serialize_(Streamer &s);
};


/****************************************************************************/

class PlayPosition
{
public:

  sString<256> PlaylistId;
  sInt SlideNo;

  sBool Dirty;

  PlayPosition() : Dirty(sFALSE), SlideNo(0) {}

  void Serialize(sReader &r) { Serialize_(r); }
  void Serialize(sWriter &w) { Serialize_(w); }

private:
  template<class Streamer> void Serialize_(Streamer &s);
};

/****************************************************************************/

enum SlideType
{
  UNKNOWN,
  IMAGE,
  SIEGMEISTER_BARS,
  SIEGMEISTER_WINNERS,
};

class SiegmeisterData
{
public:
  sU32 BarColor, BarBlinkColor1, BarBlinkColor2;
  sF32 BarAlpha;
  sArray<sFRect> BarPositions;
  sBool Winners;
};

class NewSlideData
{
public:

  SlideType Type;
  sInt TransitionId;
  sF32 TransitionTime;

  sImageData *ImgData;
  SiegmeisterData *SiegData;

  sBool Error;

  NewSlideData() : ImgData(0), SiegData(0) {}
  ~NewSlideData() { delete ImgData; delete SiegData; }
};

class PlaylistMgr
{
public:

  PlaylistMgr();
  ~PlaylistMgr();

  // Adds new play list to pool (takes ownership of pl)
  void AddPlaylist(Playlist *pl);

  // retrieve list of cached playlists
  // (call GetBegin first, then GetNext until either returns null)
  Playlist *GetBegin();
  Playlist *GetNext(Playlist *pl);

  // retrieve currently playing list and slide
  void GetCurrentPos(const sStringDesc &pl, const sStringDesc &slide);

  // commands
  void Seek(const sChar *plId, const sChar *slideId, sBool hard);
  void Next(sBool hard, sBool force);
  void Previous(sBool hard);

  NewSlideData* OnFrame(sF32 delta);
  sBool OnInput(const sInput2Event &ev);

private:

  const sChar *CacheDir;
  const sChar *PlaylistDir;
  const sChar *AssetDir;

  sDList<Playlist,&Playlist::Node> Playlists;

  sThreadLock Lock;
  sThreadEvent PlCacheEvent, AssetEvent, PrepareEvent;
  sThread *PlCacheThread, *AssetThread, *PrepareThread;

  sArray<Asset*> Assets;
  sDList<Asset, &Asset::RefreshNode> RefreshList;

  sF64 Time;
  PlayPosition CurrentPos, LastLoopPos;
  Playlist *CurrentPl;
  sInt CurrentPlTime;
  sF64 CurrentDuration, CurrentSwitchTime;
  sBool SwitchHard;
  NewSlideData * volatile PreparedSlide;

  Playlist *GetPlaylist(const sChar *id);
  sInt GetItem(Playlist *pl, const sChar *id);

  void RefreshAssets(Playlist *pl);
  Asset *GetAsset(const sChar *path);

  void RawSeek(Playlist *pl, sInt slide, sBool hard);
  void PrepareNextSlide();

  static void PlCacheThreadProxy(sThread *t, void *obj)
  {
    ((PlaylistMgr*)obj)->PlCacheThreadFunc(t);
  }

  static void AssetThreadProxy(sThread *t, void *obj)
  {
    ((PlaylistMgr*)obj)->AssetThreadFunc(t);
  }

  static void PrepareThreadProxy(sThread *t, void *obj)
  {
    ((PlaylistMgr*)obj)->PrepareThreadFunc(t);
  }

  void PlCacheThreadFunc(sThread *t);
  void AssetThreadFunc(sThread *t);
  void PrepareThreadFunc(sThread *t); 

  static void MakeFilename(const sStringDesc& buffer, const sChar *id, const sChar *path, const sChar *ext=L"");

};


#endif // FILE_SCREENS4_PLAYLISTS_HPP

