//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright � 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         intersec.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Implementation of intersection and union of two polygons (Class TWinPoly)
//
//----------------------------------------------------------------------------

// ��������� �� ������� ��������� ��������������� �������� ���
// ��������� ����������� � ����������� ���������������.
// ���� ������� ��������, ��� ������� ���, �� ��� �� �����.
// �� ���� ��������������� �������� (���, ��������,
// ��� ���������� ����������� ������� �� ��������� � ��������� ��������
// ��� ���������� �����), �� ����� �������� ����������.
#ifndef CHECK_SELFCROSS
   #define CHECK_SELFCROSS 0
#endif

#if IU=='I'

# ifndef INTERSEC_H_INTERSECTION
#  define INTERSEC_H_INTERSECTION
#  define INTERSECT
# else
#  error "This file has already been included for INTERSECTION"
# endif

#elif IU=='U'

# ifndef INTERSEC_H_UNION
#  define INTERSEC_H_UNION
#  define UNITE
# else
#  error "This file has already been included for UNION"

# endif

#else
# error "IU must be defined as either 'I' or 'U'"
#endif

#if !defined(INTERSEC_H_PASS_1) && !defined(INTERSEC_H_PASS_2)
# define INTERSEC_H_PASS_1
#elif !defined(INTERSEC_H_PASS_2)
# define INTERSEC_H_PASS_2
# undef INTERSEC_H_PASS_1
#else
# pragma message pass 3 - this is strange!
#endif

// ���������� ������ ������ �� ������ �������
#ifdef INTERSEC_H_PASS_1

class SCross;

// ���������� ��������� �������� ������ ������
class SVertex {
   public:
      CPoint v;         // ���������� ����� - ������� ��������������
      long cross;       // ����� ��������� ������, ����������� � ������
                        // (������ � ������� scross) ��� -1 ���� �������� ���
      long processed;   // ����� �������, � ������� ����� ��� ������� ��� 0,
                        // ���� ��� ��� �� ����������
      long double angle;   // ���� ������� �� ������� ������� �
                           // ���������� ��� �������� �� �������.
                           // ���������� ������ � ������ ����������� ��������!

      // �����������, ������ ������������� �������� ��������� ������
      SVertex(CPoint V, long double ang = 10)
         : v(V), processed(0), angle(ang), cross(-1) {};
      // default constructor
      SVertex() {};
      operator CPoint() const { return v; };

};

// ���������, ������������ ����� ����������� ������
class SCross {
   public:
      TArray< TListPtr<SVertex> > sv;   // ����������� �������
      TListPtr<SVertex> operator[](unsigned i) {
         return sv[i];
      }
      long last() {
         return sv.last();
      }
      long length() {
         return sv.length();
      }
      void push( TListPtr<SVertex> V ) {
         sv.push(V);
      }
      TListPtr<SVertex> pop() {
         return sv.pop();
      }
};

// ��������� �� ����������� � �������� ��� � ������ ��� �������������
void cpCross( TArray<SCross> &scross,
   TListPtr<SVertex> &pv0, TListPtr<SVertex> &pv1 ) {

   // ���������� ��������� ������� �� ������� 0
   TListPtr<SVertex> pv0n = pv0.next();
   // ���������� ��������� ������� �� ������� 1
   TListPtr<SVertex> pv1n = pv1.next();

   // ������� ����� ����������� ��������
   // (*pv0,*pv0n) � (*pv1,*pv1n), ���� ������� ����������
   CrossPoint cxp = CEdge(*pv0,*pv0n).cross(CEdge(*pv1,*pv1n));

   // ����� ����������� ����
   CPoint *xp;

   if( cxp.type() & CT_CROSS ) { // ���� ������������
      xp = cxp;
      // ���� ����������� ��������� � ������ *pv0 ��� *pv1, ��
      // �� ��� ��� ���������� (��� ������� ��� � �����).
      // �.�. ����������� � ���������� ������� ������ ����������
      if( *xp == *pv0 || *xp == *pv1 )
         return;
   } else if( cxp.type() & CT_COLLINEAR ) // ����� �� ����� ������
      // � ���� ������ ������ ����������� ����� ������� ������
      // �� �� ����� pv1n, pv0n, ������� ����� �� �����
      // (���������� �� �������, �� �������!) ����������������
      // �������.
      // ����� pv0 � pv1 ��� �� ����������, �� ��� �� �������,
      // ��� � � ������ ����������� �����������.
      if( CEdge(*pv0,*pv0n).on(*pv1n) &&
         (*pv1n).v != (*pv0).v )
         // *pv1n ����� �� ����� (*pv0,*pv0n) �� �� ��������� � *pv0
         xp = &(*pv1n).v;
      else if( CEdge(*pv1,*pv1n).on(*pv0n) &&
         (*pv0n).v != (*pv1).v )
         // *pv0n ����� �� ����� (*pv1,*pv1n) �� �� ��������� � *pv1
         xp = &(*pv0n).v;
      else
         return;
   else
      return;   // ���� �� ������������

   // ���� ����� ����������� ����� �� �������� �����, ��
   // ��������� � �� ����� � �������� ����� �������
   if( *xp != *pv0n ) {
      pv0.insertnext(SVertex(*xp));
      pv0n = pv0.next();
   }
   if( *xp != *pv1n ) {
      pv1.insertnext(SVertex(*xp));
      pv1n = pv1.next();
   }

   // ������������� �������� ����� ������� � ������ �����������
   // �� ����� ���� ��� ���� �� ����� ����������� � �������
   // ���������� ����� �������� �� ������� �����
   (*pv0n).angle = TVector(*pv0n,*pv0).angle();
   (*pv1n).angle = TVector(*pv1n,*pv1).angle();

   // ����, ������� *pv0n � *pv1n � ��� ���������.
   // ������ ������ � ��� � ����� ����������� ������
   if( (*pv0n).cross < 0 && (*pv1n).cross < 0 ) {
      long cno = scross.last()+1;
      scross[cno].push(pv0n);
      scross[cno].push(pv1n);
      (*pv0n).cross = cno;
      (*pv1n).cross = cno;
   } else if( (*pv0n).cross >= 0 && (*pv1n).cross >= 0 ) {
      if( (*pv0n).cross != (*pv1n).cross ) {
         // ���������� ���������� ��� ������
         long pv1no = (*pv1n).cross;
         while( scross[pv1no].length() > 0 ) {
            TListPtr<SVertex> sp = scross[pv1no].pop();
            (*sp).cross = (*pv0n).cross;
            scross[(*pv0n).cross].push(sp);
         }
      }
   } else if( (*pv0n).cross >= 0 ) {
      scross[(*pv0n).cross].push(pv1n);
      (*pv1n).cross = (*pv0n).cross;
   } else if( (*pv1n).cross >= 0 ) {
      scross[(*pv1n).cross].push(pv0n);
      (*pv0n).cross = (*pv1n).cross;
   } else
      Throw("Impossible situation!");

} // ����� ������� cpCross

#endif   // INTERSEC_H_PASS_1

#ifdef INTERSECT

// ����������� ���� ��������������� - ���������� ��������������
// ������ ���������������.  ������� ������������� �� ��������
TList<TWinPoly*> TWinPoly::intersect(TWinPoly *P,
   bool volatile *ShouldTerminate) {

#elif defined(UNITE)

// ����������� ���� ��������������� - ���������� ��������������
// ������ ���������������.  ������� ������������� �� ��������
TList<TWinPoly*> TWinPoly::unite(TWinPoly *P,
   bool volatile *ShouldTerminate) {

#endif

   // ���������� �������� ������� ������� ������ ���� ��������������� �
   // ����� � ���.  ������� �������� ������� - ������ ������ �������
   // �������� ���������������, ��������� �������� - ������ ������ ��������
   // �����.
   // lst[0] - ������ ��� �������������� *this
   // lst[1] - ������ ��� �������������� *P

   TArray< TListPtr< SVertex > > lst[2];

   // �� �� ����� ���������� ��������� *this � *P, � ������� ������
   // �������������� ������ �� �������� �� ��� ��� ��������������.

   TWinPoly* pp[2] = { this, P };

   // ������ �������� ����������� ������

   TArray<SCross> scross;

   // ������ ������ ������.  ���������� ��� � ������ ����

   TList<TWinPoly*> EmptyList;

   // ������� � ������ �������� ������� ���������������
   // ��� ����� ���������� ��������� � ������ ������ ������� �������,
   // � �������� �������.  �.�. � ������ ��� �������� � ����������
   // �������

   for( int n = 0; n < 2; n++ ) { // ���� �� ������ ��������������

      // ������� - ������� ������
      for( long i = pp[n]->vertex.last(); i >= 0; i-- )
         lst[n][0].insertfirst(SVertex(pp[n]->vertex[i]));

      // ������ ������� �����
      for( unsigned j = 0; j < pp[n]->hole.length(); j++ ) {
         TPolygon *hole = pp[n]->hole[j];
         for( long i = hole->nVertex()-1; i >= 0; i-- ) {
            lst[n][j+1].insertfirst(SVertex(*(hole->getVertex(i))));
            if( ShouldTerminate && *ShouldTerminate )
               break;
         }
         if( ShouldTerminate && *ShouldTerminate )
            break;
      }
      if( ShouldTerminate && *ShouldTerminate )
         break;
   }

   if( ShouldTerminate && *ShouldTerminate ) { // ��������������� ����������
      // ����������� ������ ��-��� lst
      for( int n=0; n<2; n++ )
         while( lst[n].length() > 0 )
            lst[n].pop().deletelist();
      return EmptyList;
   }

   // ������ ���� ��� ����� ����������� ����� ��������� _������_
   // ��������������� � ������� �� � ���� �������������� ������
   // � ��� �������������� �������.
   // �������� ���� ������ �������� ����������� ������.

   // ���� �� ���� �������� ��-�� 0
   for( unsigned k0 = 0; k0 < lst[0].length(); k0++ ) {
      // ���� �� ���� �������� ������� ��-�� 0
      bool dopv0 = true; // ���� ����������� �����
      for( TListPtr<SVertex> pv0 = lst[0][k0]; dopv0;
         dopv0 = ( ++pv0 != lst[0][k0] ) )
         // ���� �� ���� �������� ��-�� 1
         for( unsigned k1 = 0; k1 < lst[1].length(); k1++ ) {
            // ���� �� ���� �������� ������� ��-�� 1
            bool dopv1 = true; // ���� ����������� �����
            for( TListPtr<SVertex> pv1 = lst[1][k1]; dopv1;
               dopv1 = ( ++pv1 != lst[1][k1] ) ) {
               // ����������� ������
               if( pv0.last() )
                  pv0.setnext(lst[0][k0]);
               if( pv1.last() )
                  pv1.setnext(lst[1][k1]);

               // ��������, �� ����� �� �������������� �����������
               if( ShouldTerminate && *ShouldTerminate ) {
                  // ����������� ������ ��-��� lst
                  for( int n=0; n<2; n++ )
                     while( lst[n].length() > 0 )
                        lst[n].pop().deletelist();
                  return EmptyList;
               }

               // ������ ��� �����������:
               // ������� ����� ����������� �����, � ���� ������� ����,
               // �� ������� �� �� ������ �����, � ����� � ������ �����
               // �����������
               cpCross( scross, pv0, pv1 );

            }
         }
   } // ������� ������� ��������� ������ �� ���� ����� ������

   // ������ ���� ����� �������� ������� �� ���� ��������������� �� �������
   // ���������������. ���� ������� ��������, ��� ������� ���, �� ����
   // ����� ���� �� �����.  �� ���� ��������������� �������� (���, ��������,
   // ��� ���������� ����������� ������� �� ��������� � ��������� ��������
   // ��� ���������� �����), �� ���� ����� ���������.
   #if CHECK_SELFCROSS == 1

   for( int n=0; n<2; n++ ) // ���� �� ����� ���������������
      // ���� �� ���� �������� ��-�� n
      for( unsigned k0 = 0; k0 < lst[n].length(); k0++ ) {
         // ���� �� ���� �������� ������� ��-�� n
         bool dopv0 = true; // ���� ����������� �����
         for( TListPtr<SVertex> pv0 = lst[n][k0]; dopv0;
            dopv0 = ( ++pv0 != lst[n][k0] ) )
            // ���� �� ���� �������� ���� �� ��-��
            for( unsigned k1 = k0; k1 < lst[n].length(); k1++ ) {
               // ���� �� ���� �������� ������� ���� �� ��-��
               bool dopv1 = true; // ���� ����������� �����
               for( TListPtr<SVertex> pv1 = lst[n][k1]; dopv1;
                  dopv1 = ( ++pv1 != lst[n][k1] ) ) {

                  if( pv1 == pv0 ||
                     pv1.next() == pv0 )
                     break;

                  // ��������, �� ����� �� �������������� �����������
                  if( ShouldTerminate && *ShouldTerminate ) {
                     // ����������� ������ ��-��� lst
                     for( int n=0; n<2; n++ )
                        while( lst[n].length() > 0 )
                           lst[n].pop().deletelist();
                     return EmptyList;
                  }

                  // ������ ��� �����������:
                  // ������� ����� ����������� �����, � ���� ������� ����,
                  // �� ������� �� �� ������ �����, � ����� � ������ �����
                  // �����������
                  cpCross( scross, pv0, pv1 );

               }
            }
      }

   #endif   // �������� �������������� ��������

   // � ���� ����� �� ����� ��� ����� �����������.  ����� �������� �������
   // �������������� �������.

   // ������ ������������ ������������� � ������������� ��������
   TList<TWinPoly*> rplus, rminus;

   // ����� �������� �������
   long bno = 0;

   // ���� �� ���� ��������. ��� ������ ������� ��� ��������������,
   // �������� ����� �� �� ����� �������...
   for( int n=0; n<2; n++ )
      for( unsigned j = 0; j < lst[n].length(); j++ ) {
         bool dopv = true; // ���� ����������� �����. ���� ���� ������������
                           // ��� �������� � �������� �������
         for( TListPtr<SVertex> pv = lst[n][j]; dopv;
            dopv = ( ++pv != lst[n][j] ) )
            if( (*pv).processed == 0 ) {
               // ����� ���������� �������. �������� ����� �������
               bno++;
               TWinPoly *wp = new TWinPoly;  // ������� ������
               TListPtr<SVertex> cv = pv;    // ������� �������

               // ����� �� �������, ���� �� ������ � �����, � ������� ���
               // ����
               while( 1 ) {

                  // ��������, �� ����� �� �������������� �����������
                  if( ShouldTerminate && *ShouldTerminate ) {
                     // ����������� ������ ��-��� lst
                     for( int n=0; n<2; n++ )
                        while( lst[n].length() > 0 )
                           lst[n].pop().deletelist();
                     if( wp ) delete wp;
                     rplus.deletelist();
                     rminus.deletelist();
                     return EmptyList;
                  }

                  // ��������, �� ������ �� �� � �������, ������� ���
                  // ����� � ����� �������?  ���� ��, �� �������
                  // �������� ���� � ���� ���������� �������.
                  bool loopchk = false;   // ����� ��������� ��������?
                  if( (*cv).v == (*pv).v )
                     loopchk = true;   // �������� ������
                  else if( (*cv).cross >= 0 )
                     for( long sj=0; sj < scross[(*cv).cross].length(); sj++ )
                        if( (*(scross[(*cv).cross][sj])).processed == bno ) {
                           loopchk = true; // ���������� ��������� ��������
                           break;
                        }

                  if( loopchk )
                     // �������� ��������� ��������, �.�. ����������
                     // ���������� ���� ������ ������ ������� � �������
                     for( unsigned i=0; i<wp->vertex.length(); i++ )
                        if( wp->vertex[i] == (*cv).v ) {
                           // ������� �������!
                           // �������� ��������� ������ � ��������� ��-���

                           TWinPoly *wp1 = new TWinPoly;
                           long double angle = 0; // ��������� ���� ��������
                           long double aprev =
                              TVector(wp->vertex[wp->vertex.last()],
                              wp->vertex[i]).angle();
                           for( long j=i; j<=wp->vertex.last(); j++ ) {
                              *wp1 += wp->vertex[j];
                              long double a = TVector(wp->vertex[j],
                                 wp->vertex[(j<wp->vertex.last())?j+1:i]).angle();
                              if( fabsl(a-aprev) <= M_PI )
                                 angle += a-aprev;
                              else if( fabsl(a + 2*M_PI - aprev) <= M_PI )
                                 angle += a + 2*M_PI - aprev;
                              else if( fabsl(a - 2*M_PI - aprev) <= M_PI )
                                 angle += a - 2*M_PI - aprev;
                              else
                                 Throw("������ ����������� ���������� �������"
                                 " - ���������� �������� ����");
                              aprev = a;
                           }

                           if( wp1->nVertex() < 3 ) {
                              delete wp1; // ������� ������� ������ -
                                          // ������ ����������
                           } else {
                              // ��������� ���� ��-��� � �������������� ������
                              // ��� � ������ �����, � ����������� �� ����������
                              if( EQU( angle, 2*M_PI ) ) {
                                 rplus.add(wp1); // ������������� ����������
                              } else if( EQU( angle, -2*M_PI ) ) {
                                 rminus.add(wp1); // ������������� ����������
                              } else {
                                 Throw("������ ����������� ���������� �������:"
                                    " �������� ����");
                              }
                           }

                           // ���������� ���� ������ �� ������ ��������������
                           // (���������� ������� � i-��� �������)
                           wp->vertex.setlength(i);
                           break;
                        }

                  // ����������� ������, ���� ��� ����� - ���
                  // ���������� ����
                  if( (*cv).processed )
                     break;

                  // ��������� ������� ������� � ������ ������ ������ ��-��
                  *wp += *cv;
                  // �������� ������� ��� ������������
                  (*cv).processed = bno;

                  // ������ ���������� ���������� ����������� ��������
                  // �� ���� �������.
                  // ���� ������� �� ��������� � ������, �� ������ ��������
                  // ������ �� �������
                  if( (*cv).cross >= 0 ) {
                     // ���������... �������� ����������, ����� �� ������
                     // ������
                     unsigned bj = -1;
#ifdef INTERSECT
                     long double ba = -10;
#elif defined( UNITE )
                     long double ba = 10;
#endif
                     for( long sj=0; sj < scross[(*cv).cross].length(); sj++ ) {
                        long double a = TVector(*cv,
                           *(scross[(*cv).cross][sj].next())).angle();
#ifdef INTERSECT
                        // ������� ������������ (����� �����) ����
                        if( a < (*cv).angle )
                           a += 2*M_PI;
                        if( a > ba ) {
                           bj = sj;
                           ba = a;
                        }
#elif defined( UNITE )
                        // ������� ����������� (����� ������) ����
                        if( a > (*cv).angle )
                           a -= 2*M_PI;
                        if( a < ba ) {
                           bj = sj;
                           ba = a;
                        }
#endif
                     }
                     // ����, ����� ����� ������� �� ����������� �������
                     // � ������� bj. ���������.
                     cv =  scross[(*cv).cross][bj];
                     // �������, � ������� �� ��������, �� ��������
                     // ��� ������������.  ��������, �� ����� � ���
                     // ������ ���� � ���� ���� ������.
                  }

                  // ��������� � ��������� ������� � ������
                  cv++;
               }

               // �� ������ � �����, ��� ������������� �������,
               // ������������ �����. ������, ��� ����� (���� �� ����)
               // �������� ������, ��� ������� ������ ��������
               delete wp;

            } // if( (*pv).processed )
      } // ��� ��������� ����� for �� ���� ��������

   // ������� �� ����� ��� ������ ��������, rplus � rminus, ��������������
   // ������������� � ������������� ����������.

   // ������ ��������� ������
   TList<TWinPoly*> rplus1;
   TList<TWinPoly*> rminus1;

   // ��� �� �������, ��� ����������� ����������� ��� ������ ��������
   // �� ������� ���������������...

#ifdef INTERSECT

# define RPLUS rplus
# define RPLUS1 rplus1
# define RMINUS rminus
# define RMINUS1 rminus1

#elif defined( UNITE )

# define RPLUS rminus
# define RPLUS1 rminus1
# define RMINUS rplus
# define RMINUS1 rplus1

#endif

   // ������������� ������ ������������� (INTERSECT) �������� �� �������
   // �� ���������� � ����� �������� ���������������.
   // [������������� (UNION)]

   while( !RPLUS.empty() ) {
      TWinPoly* wp = RPLUS.pop();
      // ���������, ��� wp ������� ����� ������ ��� pp[0], ��� � pp[1]
      // ��� ����� ���������, ��� �������� ���� ��� ������ ����� ������
      unsigned outcnt = 0;
      for( int n = 0; n < 2; n++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // ����������� �������
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // ����������� �����
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)pp[n])->placement(cp);
            if( pl == PL_OUTSIDE ) {
               outcnt++;
               break;
            } else if( pl == PL_INSIDE ) {
#ifdef UNITE
               // ���������, �� ������ �� �����-������ ����� pp[n]
               // �� ���������? ���� ��, �� => outside!
               // �.�. �� �������� ����������� (��������, ��������)
               // ����� ��������� ���������������

               for( unsigned j = 0; j < pp[n]->hole.length(); j++ ) {
                  TPolygon *hole = pp[n]->hole[j];
                  for( long i = wp->nVertex()-1; i >= 0; i-- ) {
                     // ����������� �������
                     TVector v = TVector( wp->vertex[i],
                        i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
                     // ����������� �����
                     CPoint cp = wp->vertex[i] + v*0.5;
                     EPlacement pl = hole->placement(cp);
                     if( pl == PL_OUTSIDE ) {
                        break;
                     } else if( pl == PL_INSIDE ) {
                        outcnt++;
                        break;
                     }
                     if( i <= 0 )
                        outcnt++; // ���� ���� ����� ��������� �
                                  // �������� ������ hole
                  }
               }
#endif
               break;
            }
         }
      }
#ifdef INTERSECT
      if( outcnt == 0 )
#elif defined( UNITE )
      if( outcnt > 0 )
#endif
         RPLUS1.push(wp);
      else
         delete wp;
   }
   RPLUS = RPLUS1;
   RPLUS1.clear();

   // ����� ������� �������� ������������� ������� � �������� �� �� ���,
   // ������� �������� ������ ������ (�����������) [������� ����������
   // ������ ������ (�����������)]
   // *** ����� ����� [21.11.99]: � ������ ����������� ����� ����������
   // ��������������, ���������� ������ ���� ������? ���� �� ��� ���������,
   // ��� ��� ��� ����� ������ ����� ��������, ������������� ��� ���
   // ����� � �����������... ���� ���������� ������������� ��������
   // ������ ����� ������ ����� �������� - � ��� ������ ���������!

   // ����� ������� �������� ������������� ������� � �������� �� �� ���,
   // ������� ����� ������ ������ (intersection)
   // [�������� ������ ���� ������ (union). �� ��� �������� - ������, �.�.
   // �� ��� �������� �� �����, ��� �����, ���������� �� ��� ��������
   // ������� �������]
   // ����, ��� �������� �������� ������ ��� ������ INTERSECT

//#ifdef INTERSECT  -- ��. ����� ����� [21.11.99] ����.
#if 0
   while( !RPLUS.empty() ) {
      TWinPoly* wp = RPLUS.pop();
      bool isinside = false; // ���� �� ���-�� ������ ���?
      for( TListPtr<TWinPoly*> ppl = RPLUS.empty()?
         RPLUS1.head() : RPLUS.head();
         !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = (*ppl)->nVertex()-1; i >= 0; i-- ) {
            // ����������� �������
            TVector v = TVector( (*ppl)->vertex[i],
               i? (*ppl)->vertex[i-1]: (*ppl)->vertex[(*ppl)->nVertex()-1] );
            // ����������� �����
            CPoint cp = (*ppl)->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)wp)->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               break;
            }
         }
         // ���������� ���������, �� ����� �� ������ �������� ���-�� �� ���
         // �����������... ��������� � ����� ������ rplus �� ������ ������
         // rplus1
         if( ppl == RPLUS.tail() )
            ppl = RPLUS1.head();
      }
      if( !isinside )
         RPLUS1.push(wp);  // ����� � ��� �� ����� => ����
   }
   RPLUS = RPLUS1;
   RPLUS1.clear();
#endif

   while( !RMINUS.empty() ) {
      TWinPoly* wp = RMINUS.pop();
      bool isinside = false;
      for( TListPtr<TWinPoly*> ppl = RMINUS.empty()?
         RMINUS1.head(): RMINUS.head();
         !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // ����������� �������
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // ����������� �����
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)(*ppl))->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               break;
            }
         }
         // ���������� ���������, �� ����� �� ������� ������ ���
         // �����������... ��������� � ����� ������ rminus �� ������ ������
         // rminus1
         if( ppl == RMINUS.tail() )
            ppl = RMINUS1.head();
      }
      if( !isinside )
         RMINUS1.push(wp);  // �� ������ �� � ���� => ����
   }
   RMINUS = RMINUS1;
   RMINUS1.clear();

   // �������, �������� ������������ ������������� ������� �������
   // ������������� (����� �������� ��� ��� �����������, ��� � ��� �����������)
   while( !rminus.empty() ) {
      TWinPoly* wp = rminus.pop();
      bool isinside = false;
      for( TListPtr<TWinPoly*> ppl = rplus.head(); !ppl.empty() && !isinside;
         ppl++ ) {
         for( long i = wp->nVertex()-1; i >= 0; i-- ) {
            // ����������� �������
            TVector v = TVector( wp->vertex[i],
               i? wp->vertex[i-1]: wp->vertex[wp->nVertex()-1] );
            // ����������� �����
            CPoint cp = wp->vertex[i] + v*0.5;
            EPlacement pl = ((TPolygon*)(*ppl))->placement(cp);
            if( pl == PL_OUTSIDE )
               break;
            else if( pl == PL_INSIDE ) {
               isinside = true;
               // ��������� ����� � �������������
               (*ppl)->addHole((TPolygon*)wp);
               break;
            }
         }
      }
   }

   // ����������� ������ ��-��� lst
   for( int n=0; n<2; n++ )
      while( lst[n].length() > 0 )
         lst[n].pop().deletelist();

   return rplus;
}

#undef INTERSECT
#undef UNITE
#undef RPLUS
#undef RPLUS1
#undef RMINUS
#undef RMINUS1

