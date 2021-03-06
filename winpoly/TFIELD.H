//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         tfield.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Class definition for TField class - the field where all polygons lay
//
//----------------------------------------------------------------------------

#ifndef _TFIELD_H_
#define _TFIELD_H_

#include "tpoly.h"
#include "tdraw.h"

class TField {
	private:
		// ����������
      long double xmin,ymin,xmax,ymax; // ������������ � ����������� ����������
      bool minmaxfound;				 		// min-� � max-� ���������� ��� ��� ���
      TArray<TWinPoly*> poly;				// ������ ���������� �� ��������������
      TMemoryDC *mDC;  				      // DC ��� ��������� ����
      TDC *clientDC;							// �������� ���������� DC
      TBitmap *mBitmap;						// Bitmap ��� ��������� ����
      TDrawThread *drawThread;			// Thread, �������� ��������

      // ��������� ��� �������� � ���������. �� ���������, ������� ������
      // ���� !mimmaxfound.  �������� force ������� ������������ ���������
      void calcminmax(bool force=false);

	public:
   	TField() : minmaxfound(false), mDC(NULL), mBitmap(NULL),
      	drawThread(NULL), clientDC(NULL) {};
      // ��������������� �� ������ ���������������
      // ������ �� �����������
      TField(TList<TWinPoly*> pl);
      // copy constructor
      TField(TField &other);
/*      TField& operator=(TField other); */
      ~TField(); // ��� �������������� ������������

      // ���������� ���������������
      unsigned nPoly() { return poly.length(); }

      // i-��� �������������
      TWinPoly* getPoly(unsigned i) {
      	return (i<poly.length())? poly[i]:NULL;
      };

      // �������� ������������� �� ����
      void operator+=(TWinPoly *p) { poly[poly.length()] = p; }

      // ������� i-��� ������������� � ���� (������������� ������������)
      void removePoly(unsigned i) {
      	if( i < poly.length() && poly[i] )
         	delete poly[i];
      	poly.remove(i);
      }

      // ���������� ����������� � ������������ ���������� ������
		long double minx() {
      	if( !minmaxfound ) calcminmax();
         return xmin;
      }
      long double maxx() {
      	if( !minmaxfound ) calcminmax();
         return xmax;
      }
      long double miny() {
      	if( !minmaxfound ) calcminmax();
         return ymin;
      }
      long double maxy() {
      	if( !minmaxfound ) calcminmax();
         return ymax;
      }

		// �������� � ���� / ��������� �� �����
      // � ������ ������ ������������� ����������
      void fileSave(const char *fname);
      void fileOpen(const char *fname);

      // �������� ��� ����.  ���������� ������ ���������������,
      // ���������� �����������
      TList<TWinPoly*> intersect(TField *f,
         bool volatile *ShouldTerminate = NULL);
      // ���������� ��� ����.  ���������� ������ ���������������,
      // ���������� �����������
      TList<TWinPoly*> unite(TField *f,
         bool volatile *ShouldTerminate = NULL);

      // ����� ���������� (t-delta) ������� �����, ���� *this - ��� �������
      // (�������� ���������)
      // ��������� �������� ������� TBrData
      // � �������� ��������� ����������� ������� ������ �������
      TField* Wprev(long double t, bool volatile *ShouldTerminate = NULL);

      // "��������" ������� ����, ������� �� ���� "������" �������
      // ������� ���������� ���������� ���������� �� ���� ������
      unsigned long sift(bool volatile *ShouldTerminate = NULL);

      // ��������� �������������� �� ������� "���������������"
      // � ���������� �� ���
      void checkselfcross(bool volatile *ShouldTerminate = NULL);

      // ��������� �������������� �� ������� "���������������"
      // ������� ������, ���� ������� �������
      bool selfcrosscontrol(bool volatile *ShouldTerminate = NULL);

      // ����� �� ���� �����, ��������� � ��������
      CPoint findClosest(CPoint &p);

      // �������� ����
      void clearAll();

      // ---- �������, ��������� � ���������� �� ��������� DC -------

      // ������ �������� �������� ���������� ������� ��� ������� DC
      // (������� �������� ����� DC � ������ � �������� �� ���)
      // ����� �������� ������, ���������� invalidate->Invalidate() ����
      // invalidate ����������
      // ����� ���� ������� ���������� ��������������, ������� �������
      // �������������� �� ��������.  ���� �� �������, �� ������� ����������
      // ������������� ���, ����� ����� ��� ��������.
      void preparePicture(TDC &DC, TRect region, TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);
      void preparePicture(TRect region, TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);

      // ������ �������� �������� � ���������� ����������� ���������������
      // �� ������ DC. ��������! � ���� ������ ������� getPictureDC() �
      // getPictureBitmap() �� ��������, �.�. ������ ������� TField
      // ������� �� �����������.
      // ����� ���� ������� ���������� ��������������, ������� �������
      // �������������� �� ��������.  ���� �� �������, �� ������� ����������
      // ������������� ���, ����� ����� ��� ��������.
      void preparePictureOnDC(TDC &DC, TRect region,
         TWindow *invalidate = NULL,
         TDrawThread::TDrawType drtype = TDrawThread::unspecified,
         long double xmin = 0,
         long double ymin = 0,
         long double xmax = 0,
         long double ymax = 0);

      // ���������� ��������� ��������
      void stopPreparingPicture() {
      	if( drawThread ) {
         	if( drawThread->GetStatus() == TThread::Running )
      	   	drawThread->TerminateAndWait();
            delete drawThread;
            drawThread = NULL;
         }
      }

      // ������� ��� ������������ �������� (memory DC) ��� NULL
      TDC* getPictureDC();

      // ������� ������������ �������� � ���� bitmap ��� NULL
      TBitmap* getPictureBitmap();

      // ������ �� ��������?
      bool PictureReady();

      // ���� ���� �������� ����� ��������� ����������
      void PictureWaitReady();

};

#endif
