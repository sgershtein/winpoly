/**********************************************************************

    Универсальный шаблон класса "Массив" (TArray)

    Операции: произвольный доступ к элементу.
    Содержимое неинициализированного элемента не определено!

    Массив знает свою длину и автоматически расширяется
    при необходимости

    Массивы можно присваивать друг другу, при этом результатом
    будет не другой указатель на те же данные, а совершенно
    независимый массив, содержащий копию данных исходного массива

    (c) 1998 Сергей Герштейн

**********************************************************************/

#ifndef _TArray_H_
#define _TArray_H_

#include <stddef.h>

// количество "запасных" свободных элементов в хвосте
#ifndef TARRAY_RESERVE_SPACE
#define TARRAY_RESERVE_SPACE 5
#endif

template <class T>
class TArray {
    public:
        // конструктор
        TArray(unsigned inc=TARRAY_RESERVE_SPACE) : tlen(0),
            len(0), data(NULL), incr(inc) {}
        // конструктор копирования
        TArray(TArray<T>& arr);
		  // деструктор
        virtual ~TArray();
        // доступ к элементу
        T& operator[](unsigned i);
        // присваивание массивов
        TArray<T>& operator=(TArray<T>& arr);
        // поледний элемент
        long last() const { return (long)len-1; }
        // количество элементов
        unsigned length() const { return len; }
        // на сколько элементов зарезервировано памяти
        unsigned allocated() const { return tlen; }
        // величина резерва
        unsigned getrspace() const { return incr; }
        // установить величину резерва
        void setrspace(unsigned bs) { incr = bs; }
        // установить новую длину. "Лишние" элементы пропадают
        void setlength(unsigned l);
         // удалить элементы от n1 до n2 включительно, остальные сдвинуть
        void remove(unsigned n1, unsigned n2);
        void remove(unsigned n1) { remove(n1,n1); }
        // вставить массив с позиции n, элементы раздвигаются
        void insert(TArray<T>& arr, unsigned n);
        // вставить элемент с позиции n, элементы раздвигаются
        void insert(T elem, unsigned n);
        // добавить элемент в конец
        void push(T elem);
        // выдать последний элемент и выкинуть его
        T pop();

    private:
        unsigned len;   // длина массива
        unsigned incr;  // размер блока
        unsigned tlen;  // реальное количество выделенных элементов
        T *data;        // собственно данные
};

// конструктор копирования
// создаём новую область данных, а не используем старую
template <class T>
TArray<T>::TArray(TArray<T>& arr) : data(NULL) {
   *this = arr;
}

// присваивание массивов
// создаём новую область данных, а не используем старую
template <class T>
TArray<T>& TArray<T>::operator=(TArray<T>& arr) {
   if( data )
      delete[] data;
   data = new T[arr.tlen];
   tlen = arr.tlen;
   for( len = 0; len < arr.len; len++ )
      data[len] = arr.data[len];
   incr = arr.incr;
   return *this;
}

template <class T>
T& TArray<T>::operator[](unsigned i) {
   if( (long)i>last() )
      setlength( i+1 );
   return data[i];
}

template <class T>
void TArray<T>::push(T elem) { // добавить элемент в конец массива
    (*this)[len] = elem;
}

template <class T>
T TArray<T>::pop() { // вернуть последний элемент и удалить его
    T elem = data[last()];
    setlength( len-1 );
    return elem;
}

template <class T>
void TArray<T>::insert(T elem, unsigned n) { // вставить элемент
    setlength(len>n?len+1:n+1);
    for( long i=last(); i>(long)n; i-- )
        data[i] = data[i-1];
    data[n] = elem;
}

template <class T>
void TArray<T>::insert(TArray<T>& arr, unsigned n) { // вставить массив
    setlength((len+arr.last()>n)? len+arr.length() : n+arr.last());
    for( long i=length(); i>(long)n; i-- )
        data[i] = data[i-arr.length()];
    for( unsigned j=0; j<arr.length(); j++ )
        data[j+n]=arr[j];
}

template <class T>
void TArray<T>::remove(unsigned n1, unsigned n2) { // удалить элементы
    if( n2 < n1 )
        return;
    n2++; // n2 показывает на первый элемент за массивом
    unsigned lst = n1;
    for( long i=n2; i<=last(); i++, lst++ ) 
        data[lst] = data[i];
    setlength( lst );
}

template <class T>
void TArray<T>::setlength(unsigned l) {  // установить новую длину.
   if( l == 0 ) { // полностью очищаем
      if( data )
         delete[] data;
      data = NULL;
      tlen = 0;
   } else if( l > tlen || (l + incr*2) < tlen ) { // надо менять длину
      T *data1 = new T[tlen=l+incr];
      unsigned ncopy = len<tlen?len:tlen;
      for( unsigned i=0; i<ncopy; i++ )
         data1[i] = data[i];
      if( data )
         delete[] data;
      data = data1;
   }
   len = l;
}

template <class T>
TArray<T>::~TArray() {
	if( data ) delete[] data;
}

#endif
