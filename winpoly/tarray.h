/**********************************************************************

    ������������� ������ ������ "������" (TArray)

    ��������: ������������ ������ � ��������.
    ���������� ��������������������� �������� �� ����������!

    ������ ����� ���� ����� � ������������� �����������
    ��� �������������

    ������� ����� ����������� ���� �����, ��� ���� �����������
    ����� �� ������ ��������� �� �� �� ������, � ����������
    ����������� ������, ���������� ����� ������ ��������� �������

    (c) 1998 ������ ��������

**********************************************************************/

#ifndef _TArray_H_
#define _TArray_H_

#include <stddef.h>

// ���������� "��������" ��������� ��������� � ������
#ifndef TARRAY_RESERVE_SPACE
#define TARRAY_RESERVE_SPACE 5
#endif

template <class T>
class TArray {
    public:
        // �����������
        TArray(unsigned inc=TARRAY_RESERVE_SPACE) : tlen(0),
            len(0), data(NULL), incr(inc) {}
        // ����������� �����������
        TArray(TArray<T>& arr);
		  // ����������
        virtual ~TArray();
        // ������ � ��������
        T& operator[](unsigned i);
        // ������������ ��������
        TArray<T>& operator=(TArray<T>& arr);
        // �������� �������
        long last() const { return (long)len-1; }
        // ���������� ���������
        unsigned length() const { return len; }
        // �� ������� ��������� ��������������� ������
        unsigned allocated() const { return tlen; }
        // �������� �������
        unsigned getrspace() const { return incr; }
        // ���������� �������� �������
        void setrspace(unsigned bs) { incr = bs; }
        // ���������� ����� �����. "������" �������� ���������
        void setlength(unsigned l);
         // ������� �������� �� n1 �� n2 ������������, ��������� ��������
        void remove(unsigned n1, unsigned n2);
        void remove(unsigned n1) { remove(n1,n1); }
        // �������� ������ � ������� n, �������� ������������
        void insert(TArray<T>& arr, unsigned n);
        // �������� ������� � ������� n, �������� ������������
        void insert(T elem, unsigned n);
        // �������� ������� � �����
        void push(T elem);
        // ������ ��������� ������� � �������� ���
        T pop();

    private:
        unsigned len;   // ����� �������
        unsigned incr;  // ������ �����
        unsigned tlen;  // �������� ���������� ���������� ���������
        T *data;        // ���������� ������
};

// ����������� �����������
// ������ ����� ������� ������, � �� ���������� ������
template <class T>
TArray<T>::TArray(TArray<T>& arr) : data(NULL) {
   *this = arr;
}

// ������������ ��������
// ������ ����� ������� ������, � �� ���������� ������
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
void TArray<T>::push(T elem) { // �������� ������� � ����� �������
    (*this)[len] = elem;
}

template <class T>
T TArray<T>::pop() { // ������� ��������� ������� � ������� ���
    T elem = data[last()];
    setlength( len-1 );
    return elem;
}

template <class T>
void TArray<T>::insert(T elem, unsigned n) { // �������� �������
    setlength(len>n?len+1:n+1);
    for( long i=last(); i>(long)n; i-- )
        data[i] = data[i-1];
    data[n] = elem;
}

template <class T>
void TArray<T>::insert(TArray<T>& arr, unsigned n) { // �������� ������
    setlength((len+arr.last()>n)? len+arr.length() : n+arr.last());
    for( long i=length(); i>(long)n; i-- )
        data[i] = data[i-arr.length()];
    for( unsigned j=0; j<arr.length(); j++ )
        data[j+n]=arr[j];
}

template <class T>
void TArray<T>::remove(unsigned n1, unsigned n2) { // ������� ��������
    if( n2 < n1 )
        return;
    n2++; // n2 ���������� �� ������ ������� �� ��������
    unsigned lst = n1;
    for( long i=n2; i<=last(); i++, lst++ ) 
        data[lst] = data[i];
    setlength( lst );
}

template <class T>
void TArray<T>::setlength(unsigned l) {  // ���������� ����� �����.
   if( l == 0 ) { // ��������� �������
      if( data )
         delete[] data;
      data = NULL;
      tlen = 0;
   } else if( l > tlen || (l + incr*2) < tlen ) { // ���� ������ �����
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
