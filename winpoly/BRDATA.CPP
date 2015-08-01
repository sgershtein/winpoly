//----------------------------------------------------------------------------
//  Project Winpoly
//
//  Copyright © 1998. All Rights Reserved.
//
//  SUBSYSTEM:    Winpoly Application
//  FILE:         brdata.h
//  AUTHOR:       Sergey Gershtein
//
//  OVERVIEW
//  ~~~~~~~~
//  Initial data for stable bridge
// (goal set M, Q and P sets, differential functions...)
//
//----------------------------------------------------------------------------

#include "brdata.h"
#include "texcept.h"
#include "dllfunc.h"

TField *TBrData::M = NULL;          // целевое множество первого игрока
TArray< CPoint > TBrData::P;        // набор точек вдоль границы множества P
                                    // векторов u (управление 1-ого игрока)
TArray< CPoint > TBrData::Q;        // набор точек вдоль границы множества Q
                                    // векторов v (управление 2-ого игрока)
char* TBrData::pComment;            // краткое описание задачи (комментарий)
TWPS* TBrData::wps = NULL;          // связанный с задачей wps-файл

long double TBrData::delta = 0.05;  // шаг разбиения по времени
long double TBrData::T     = 2;     // Целевое время (T)
long double TBrData::sieve = 0.015; // "сито" для вершин
bool TBrData::fixedT       = true;  // фикс. момент / к моменту времени
long double TBrData::tstart = TBrData::T; // время начала счета
bool TBrData::survive      = true;  // расчет траекторий выживания

// Переменные по связи с DLL
HINSTANCE hLib = 0;  // идентификатор загруженной DLL
Fcalcprev *calcprev = NULL; // ссылка на функцию вычисления попятного движения
FSecField *secfield = NULL; // ссылка на функцию отсекающего множества
FfindPvalue *findpvalue = NULL;  // ссылка на функцию решения уравнения
                                 // движения относительно управления 1 игрока

// этот класс необходим для финального вызова FreeLibrary при завершении
// программы. Вызов производится из деструктора объекта класса. Сам класс
// не содержит ничего кроме деструктора.
class ThLibCleaner {
   public:
      ~ThLibCleaner() {
         if( hLib )
            FreeLibrary( hLib );
      }
} hLibCleaner;

// загружаем DLL с задачей, считываем данные.
// В случае ошибки выбрасываем программное исключение с текстом ошибки
void TBrData::loadProblemDLL(const char *dllpath) {
   // освобождаем текущую DLL, если она уже была загружена
   if( hLib )
      FreeLibrary( hLib );

   // подгружаем DLL
   hLib = LoadLibrary(dllpath);
   if( (unsigned)hLib <= HINSTANCE_ERROR ) {
      calcprev = NULL;
      pComment = NULL;
      findpvalue = NULL;
      hLib = 0;
      Throw("Ошибка загрузки DLL описания задачи");
   }

   // находим соответствующие функции
   calcprev = (Fcalcprev*)GetProcAddress(hLib,calcprevName);
   if( !calcprev )
      Throw("В DLL описания задачи не найдена функция попятного движения");

   findpvalue = (FfindPvalue*)GetProcAddress(hLib,findPvalueName);

   secfield = (FSecField*)GetProcAddress(hLib,SecFieldName);
   survive = secfield != NULL;

   Fcomment *comment = (Fcomment*)GetProcAddress(hLib,commentName);
   if( comment )
      pComment = (*comment)();
   else
      pComment = "[описание задачи отсутствует]";

   FPQset *Pset = (FPQset*)GetProcAddress(hLib,PsetName);
   FPQset *Qset = (FPQset*)GetProcAddress(hLib,QsetName);
   if( !Pset || !Qset )
      Throw("В DLL описания задачи не найдены множеста Q и/или P");

   // запрашиваем у DLL значения Q и P и устанавливаем их
   long double *pld; unsigned cnt;

   P.setlength(0);
   (*Pset)(cnt,pld);
   for( unsigned i=0; i<cnt*2; i+=2 )
      P.push( CPoint(pld[i], pld[i+1]) );

   Q.setlength(0);
   (*Qset)(cnt,pld);
   for( unsigned i=0; i<cnt*2; i+=2 )
      Q.push( CPoint(pld[i], pld[i+1]) );

   // значение M по умолчанию - квадрат со стороной 2
   // с центром в начале координат
   if( M ) delete M;
   FMField *MField = (FMField*)GetProcAddress(hLib,MFieldName);
   if( MField ) {
      TField *far MF = (*MField)();
      M = new TField(*MF);
   } else
      M = NULL;

   // значения параметров задачи по умолчанию
   // считываем их ТОЛЬКО, если это не продолжение прерванных расчетов
   if( !wps || wps->getSectCount() <= 0 ) {
      Fparams *params = (Fparams*)GetProcAddress(hLib,paramsName);
      if( params )
         (*params)( T, fixedT, delta, sieve );
   }

}

CPoint TBrData::findPvalue(CPoint& cur, CPoint& nxt, CPoint& v,
   long double t, long double h) {
   CPoint u;
   if( findpvalue )
      (*findpvalue)(nxt.x,nxt.y,cur.x,cur.y,v.x,v.y,u.x,u.y,t,h);
   else
      Throw("В DLL задачи не определена функция вычисления "
         "управления по движению");

   return u;
}

// функция попятного движения
CPoint TBrData::prev(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h) {
   CPoint res;

   if( calcprev )
      (*calcprev)(res.x,res.y,cur.x,cur.y,v.x,v.y,u.x,u.y,t,h);
   else
      Throw("Не загружена DLL описания задачи");

   return res;
}

// функция прямого движения
CPoint TBrData::next(CPoint& cur, CPoint& u, CPoint &v,
         long double t, long double h) {
   return prev( cur, u, v, t, -h );
}

// функция отсекающего множества. Возвращает отсекающее множество
// (точнее его сечение) в заданный момент времени
TField* TBrData::sect(long double t) {
   if( !secfield )
      Throw("Не определена функция отсекающего множества");

   return (*secfield)(t);
}


