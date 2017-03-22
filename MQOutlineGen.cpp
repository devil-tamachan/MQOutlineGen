

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "MQPlugin.h"
#include "MQ3DLib.h"
#include "MQWidget.h"
//#include <vld.h>

HINSTANCE hInstance;

BOOL OutlineGen(MQDocument doc);


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

//---------------------------------------------------------------------------
//  MQGetPlugInID
//    プラグインIDを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT void MQGetPlugInID(DWORD *Product, DWORD *ID)
{
	// プロダクト名(制作者名)とIDを、全部で64bitの値として返す
	// 値は他と重複しないようなランダムなもので良い
	*Product = 0xA8BEE201;
	*ID      = 0xCC9DA482;
}

//---------------------------------------------------------------------------
//  MQGetPlugInName
//    プラグイン名を返す。
//    この関数は[プラグインについて]表示時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQGetPlugInName(void)
{
	// プラグイン名
	return "OutlineGen           Copyright(C) 2017, tamachan";
}

//---------------------------------------------------------------------------
//  MQGetPlugInType
//    プラグインのタイプを返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT int MQGetPlugInType(void)
{
	// オブジェクト変形用プラグインである
	return MQPLUGIN_TYPE_OBJECT;
}

//---------------------------------------------------------------------------
//  MQEnumString
//    ポップアップメニューに表示される文字列を返す。
//    この関数は起動時に呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT const char *MQEnumString(int index)
{
	switch(index){
	case 0: return "OutlineGen";
	}
	return NULL;
}

//---------------------------------------------------------------------------
//  MQModifyObject
//    メニューから選択されたときに呼び出される。
//---------------------------------------------------------------------------
MQPLUGIN_EXPORT BOOL MQModifyObject(int index, MQDocument doc)
{
	switch(index){
	case 0: return OutlineGen(doc);
	}
	return FALSE;
}

class WidthDialog : public MQDialog
{
public:
	WidthDialog(MQWindowBase& parent);
	~WidthDialog();

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

	MQButton *cancelbtn = CreateButton(sideFrame, L"Cancel");
	cancelbtn->SetCancel(true);
	cancelbtn->SetModalResult(MQDialog::DIALOG_CANCEL);
}

WidthDialog::~WidthDialog()
{
}

BOOL OutlineGen(MQDocument doc)
{
	float width = 0.01;
	int shadowidx = -1;
  char tmp[52] = {0};
	int vertidx[101];

	MQWindow mainwin = MQWindow::GetMainWindow();
	WidthDialog dlg(mainwin);
	if(dlg.Execute() != MQDialog::DIALOG_OK){
		return FALSE;
	}
  width = dlg.edit_width->GetPosition();

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
    return FALSE;
  }

  int iMirrorType = MQOBJECT_MIRROR_NONE;
  DWORD dwMirrorAxis = MQOBJECT_MIRROR_AXIS_X;
  float fMirrorDistance = 100.0;
	
	for(int i=0;i<doc->GetObjectCount();i++)
	{
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    o->GetName(tmp, 50);
    if(strcmp(tmp, "OutlineGenPlugin")==0)
    {
      iMirrorType = o->GetMirrorType();
      dwMirrorAxis = o->GetMirrorAxis();
      fMirrorDistance = o->GetMirrorDistance();
      doc->DeleteObject(i);
      break;
    }
  }
  
  MQObject objOutline = MQ_CreateObject();
  objOutline->SetName("OutlineGenPlugin");
  objOutline->SetLocking(TRUE);
  objOutline->SetMirrorType(iMirrorType);
  objOutline->SetMirrorAxis(dwMirrorAxis);
  objOutline->SetMirrorDistance(fMirrorDistance);

  int numArrFaceIdx = 10;
  int *arrFaceIdx = (int *)malloc(sizeof(int)*numArrFaceIdx);
  if(arrFaceIdx==NULL){
    return FALSE;
  }
	
	for(int i=0;i<doc->GetObjectCount();i++)
	{
    int startvidx = objOutline->GetVertexCount();
    MQObject o = doc->GetObject(i);
    if(o==NULL)continue;
    MQObjNormal *normal = new MQObjNormal(o);
    if(normal==NULL)break;
    if(o->GetVisible()==0)continue;

    int numV = o->GetVertexCount();
    int numF = o->GetFaceCount();
    for(int j=0;j<numV;j++)
    {
      UINT numRelF = o->GetVertexRelatedFaces(j, NULL);
      if(numRelF > numArrFaceIdx)
      {
        numArrFaceIdx = numRelF;
        arrFaceIdx = (int *)realloc(arrFaceIdx, sizeof(int)*numArrFaceIdx);
      }
      if(arrFaceIdx==NULL)
      {
        if(normal)delete normal;
        return FALSE;
      }
      o->GetVertexRelatedFaces(j, arrFaceIdx);
      MQPoint p = o->GetVertex(j);
      int numVInF = o->GetFacePointCount(arrFaceIdx[0]);
      int *varr = (int*)malloc(sizeof(int)*numVInF);
      o->GetFacePointArray(arrFaceIdx[0], varr);
      int idxV = 0;
      for(int n=0;n<numVInF;n++)
      {
        if(varr[n]==j)
        {
          idxV=n;
          break;
        }
      }
      MQPoint nor = normal->Get(arrFaceIdx[0], idxV);
      free(varr);
      p += nor*width;
      objOutline->AddVertex(p);
    }
    for(int k=0;k<numF;k++)
    {
      int numV = o->GetFacePointCount(k);
      if(numV>100)break;
      o->GetFacePointArray(k, vertidx);
      for(int m=0;m<numV;m++)vertidx[m]+=startvidx;
      int newFIdx = objOutline->AddFace(numV, vertidx);
      objOutline->InvertFace(newFIdx);
      objOutline->SetFaceMaterial(newFIdx, shadowidx);
    }
    if(normal)delete normal;
  }

  if(arrFaceIdx)free(arrFaceIdx);
  
  doc->AddObject(objOutline);

	MQ_RefreshView(NULL);
	
	return TRUE;
}

