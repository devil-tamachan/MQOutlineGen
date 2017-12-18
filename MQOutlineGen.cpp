

#if defined _WIN32 || defined __CYGWIN__
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MQPlugin.h"
#include "MQBasePlugin.h"
#include "MQ3DLib.h"
#include "MQWidget.h"
#include "MQSetting.h"
//#include <vld.h>
#include "TamaMQLib.h"

BOOL OutlineGen(MQDocument doc);


#if defined _WIN32 || defined __CYGWIN__

HINSTANCE hInstance;

//---------------------------------------------------------------------------
//  DllMain
//---------------------------------------------------------------------------
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	//ダイアログボックスの表示に必要なので、インスタンスを保存しておく
	hInstance = (HINSTANCE)hModule;

	//プラグインとしては特に必要な処理はないので、何もせずにTRUEを返す
    return TRUE;
}
#endif

class OutlineGenPlugin : public MQObjectPlugin
{
public:
  OutlineGenPlugin(){}
  ~OutlineGenPlugin(){}

  void GetPlugInID(DWORD *Product, DWORD *ID) override
  {
    *Product = 0xA8BEE201;
    *ID      = 0xCC9DA482;
  }
  const char *GetPlugInName(void) override
  {
    return "OutlineGen           Copyright(C) 2017, tamachan";
  }
  const char *EnumString(int index) override
  {
    switch(index)
    {
      case 0: return "OutlineGen";
    }
    return NULL;
  }
  BOOL Execute(int index, MQDocument doc) override
  {
    switch(index){
      case 0: return OutlineGen(doc);
    }
    return FALSE;
  }
};

MQBasePlugin *GetPluginClass()
{
  static OutlineGenPlugin plugin;
  return &plugin;
}

OutlineGenPlugin *GetOutlineGenPluginClass()
{
  return static_cast<OutlineGenPlugin*>(GetPluginClass());
}


class WidthDialog : public MQDialog
{
public:
  WidthDialog(MQWindowBase& parent);
  ~WidthDialog();
  
  MQSetting* OpenSetting()
  {
    OutlineGenPlugin *plugin = GetOutlineGenPluginClass();
    if(plugin==NULL)return NULL;
    MQSetting *config = plugin->OpenSetting();
    //if(config==NULL)return NULL;
    return config;
  }
  
  void SaveConfig()
  {
    OutlineGenPlugin *plugin = GetOutlineGenPluginClass();
    MQSetting *config = OpenSetting();
    if(config==NULL)return;
    
    double dbltmp = 1.0;
    
    dbltmp = edit_width->GetPosition();
    config->Save("width", dbltmp);
    
    plugin->CloseSetting(config);
  }
  bool LoadConfig()
  {
    OutlineGenPlugin *plugin = GetOutlineGenPluginClass();
    MQSetting *config = OpenSetting();
    if(config==NULL)return false;
    
    double dbltmp = 1.0;
    
    config->Load("width", dbltmp, 0.5);
    edit_width->SetPosition(dbltmp);
    
    plugin->CloseSetting(config);
    return true;
  }
  
  BOOL OnOK(MQWidgetBase *sender, MQDocument doc)
  {
    SaveConfig();
    return FALSE;
  }

  MQDoubleSpinBox *edit_width;
};

WidthDialog::WidthDialog(MQWindowBase& parent) : MQDialog(parent)
{
  SetTitle(L"OutlineGen");

  MQFrame *mainFrame = CreateHorizontalFrame(this);

  MQFrame *paramFrame = CreateHorizontalFrame(mainFrame);
  paramFrame->SetMatrixColumn(2);
  
  CreateLabel(paramFrame, L"width");
  edit_width = CreateDoubleSpinBox(paramFrame);
  edit_width->SetIncrement(0.01);
  edit_width->SetPosition(0.04);
  edit_width->SetVariableDigit(true);

  MQFrame *sideFrame = CreateVerticalFrame(mainFrame);

  MQButton *okbtn = CreateButton(sideFrame, L"OK");
  okbtn->SetDefault(true);
  okbtn->SetModalResult(MQDialog::DIALOG_OK);
  okbtn->AddClickEvent(this, &WidthDialog::OnOK, true);

  MQButton *cancelbtn = CreateButton(sideFrame, L"Cancel");
  cancelbtn->SetCancel(true);
  cancelbtn->SetModalResult(MQDialog::DIALOG_CANCEL);
  
  LoadConfig();
}

WidthDialog::~WidthDialog()
{
}

int CreateShadowMaterial(MQDocument doc)
{
  MQMaterial mat = MQ_CreateMaterial();
  mat->SetName("shadow");
  mat->SetShader(MQMATERIAL_SHADER_CONSTANT);
  mat->SetVertexColor(MQMATERIAL_VERTEXCOLOR_DISABLE);
  mat->SetDoubleSided(0/*FALSE*/);
  MQColor c;
  c.r = c.g = c.b = 0;
  mat->SetColor(c);
  mat->SetAlpha(1.0);
  
  return doc->AddMaterial(mat);
}

int GetShadowMaterialIdx(MQDocument doc)
{
  int shadowidx = -1;
  char tmp[52] = {0};
  for(int i=0;i<doc->GetMaterialCount();i++)
  {
    MQMaterial m = doc->GetMaterial(i);
    if(m==NULL)continue;
    m->GetName(tmp, 50);
    if(strcmp(tmp, "shadow")==0)
    {
      shadowidx = i;
      break;
    }
  }
  if(shadowidx==-1)
  {
    shadowidx = CreateShadowMaterial(doc);
  }
  return shadowidx;
}

MQObject GetOutlineObjIdx(MQDocument doc, int *idx = NULL)
{
  char tmp[52] = {0};
  for(int i=0;i<doc->GetObjectCount();i++)
  {
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    o->GetName(tmp, 50);
    if(strcmp(tmp, "OutlineGenPlugin")==0)
    {
      if(idx!=NULL)*idx = i;
      return o;
    }
  }
  return NULL;
}

MQPoint CalcFaceNormal(MQObject o, int fi)
{
  int numFV = o->GetFacePointCount(fi);
  if(numFV<3)return MQPoint(0.0f, 0.0f, 0.0f);
  std::vector<MQPoint> pts(numFV);
  GetPoint(o, fi, numFV, pts);
  switch(numFV)
  {
    case 3:
      return GetNormal(pts[0], pts[1], pts[2]);
    case 4:
      return GetQuadNormal(pts[0], pts[1], pts[2], pts[3]);
    default:
      return GetPolyNormal(&(pts[0]), numFV);
  }
}

MQPoint CalcVertexNormal(MQObject o, int vi)
{
  std::vector<int> fidx;
  GetVertexRelatedFaces(o, vi, fidx);
  int numRelF = fidx.size();
  MQPoint v(0.0f, 0.0f, 0.0f);
  for(int i=0;i<numRelF;i++)
  {
    v += CalcFaceNormal(o, fidx[i]);
  }
  return Normalize(v);
}

BOOL OutlineGen(MQDocument doc)
{
  float width = 0.01;
  int shadowidx = -1;

  MQWindow mainwin = MQWindow::GetMainWindow();
  WidthDialog dlg(mainwin);
  if(dlg.Execute() != MQDialog::DIALOG_OK)
  {
    return FALSE;
  }
  width = dlg.edit_width->GetPosition();
  
  shadowidx = GetShadowMaterialIdx(doc);
  if(shadowidx==-1)
  {
    return FALSE;
  }

  int iMirrorType = MQOBJECT_MIRROR_NONE;
  DWORD dwMirrorAxis = MQOBJECT_MIRROR_AXIS_X;
  float fMirrorDistance = 100.0;
  
  {
    int objIdx = -1;
    MQObject o = GetOutlineObjIdx(doc, &objIdx);
    if(o!=NULL)
    {
      iMirrorType = o->GetMirrorType();
      dwMirrorAxis = o->GetMirrorAxis();
      fMirrorDistance = o->GetMirrorDistance();
      doc->DeleteObject(objIdx);
    }
  }
  
  MQObject objOutline = MQ_CreateObject();
  objOutline->SetName("OutlineGenPlugin");
  objOutline->SetLocking(TRUE);
  objOutline->SetMirrorType(iMirrorType);
  objOutline->SetMirrorAxis(dwMirrorAxis);
  objOutline->SetMirrorDistance(fMirrorDistance);
  
  for(int oi=0;oi<doc->GetObjectCount();oi++)
  {
    int startvidx = objOutline->GetVertexCount();
    MQObject o = doc->GetObject(oi);
    if(o==NULL)continue;
    if(o->GetVisible()==0)continue;

    int numV = o->GetVertexCount();
    int numF = o->GetFaceCount();
    for(int vi=0;vi<numV;vi++)
    {
      MQPoint vn = CalcVertexNormal(o, vi);
      MQPoint p = o->GetVertex(vi);
      p += vn*width;
      objOutline->AddVertex(p);
    }
    std::vector<int> vidx(16);
    for(int fi=0;fi<numF;fi++)
    {
      int numFV = o->GetFacePointCount(fi);
      vidx.resize(numFV);
      o->GetFacePointArray(fi, &(vidx[0]));
      for(int m=0;m<numFV;m++)vidx[m]+=startvidx;
      int newFIdx = objOutline->AddFace(numFV, &(vidx[0]));
      objOutline->InvertFace(newFIdx);
      objOutline->SetFaceMaterial(newFIdx, shadowidx);
    }
  }
  
  doc->AddObject(objOutline);

	MQ_RefreshView(NULL);
	
	return TRUE;
}

