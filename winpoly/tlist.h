/**********************************************************************

    ������������� ������ ������ "���������������� ������" (TList)

    ��������: ������� � ���������� ��������, ������ � ��������,
    ������� ��������.

    ��������� �� 1998-12-27:
    	���������� ������ � �������� += ��� add(TList) ��� ���������
        ������� ������

    (c) 1998 ������ ��������

**********************************************************************/

#ifndef _TList_H_
#define _TList_H_

#include <stddef.h>

//
// ����� �������� ������. ����� ������� ��� ������������� ��������
// � ��� ����.  ��� �������� ������������ ����� TList ��� TListPtr
//

template <class T>
class TListEntity {
	public:
      // data ������� ��������������������!
   	TListEntity() : Next(NULL) {};
      // ������ ������������� ��������
      TListEntity(T element) : Next(NULL), Data(element) {};
      // ��������� �� ���������
      TListEntity<T>*& next() { return Next; };
      // ���������� ������ � ������ ����
      T& data() { return Data; };
   private:
   	TListEntity<T> *Next;
      T Data;
};

//
// ����� ��������� ������.
// ��������� ��������� �� "�����" ������, ������� � �������� ��������
//

template <class T>
class TListPtr {
	public:
   	// ������������� ������ ��������� ���������
   	TListPtr(TListEntity<T> *first=NULL) : ptr(first) {}
      // ������ � �������� ��������
      T& operator*() { return ptr->data(); }
      // ��������� �������
      TListPtr<T> next() { return TListPtr<T>(ptr? ptr->next() : NULL); }
      // ������� � ���������� (prefix ++)
      TListPtr<T> operator++() {
      	if( ptr ) ptr = ptr->next();
        	return *this;
      }
      // ������� � ���������� (postfix ++)
      TListPtr<T> operator++(int) { return ++*this; }
      // ������������ ����������
      TListPtr<T> operator=(TListPtr<T> lp) { return ptr = lp.ptr; }
      // ��������� ����������
      bool operator==(TListPtr<T> lp) { return ptr == lp.ptr; }
      bool operator!=(TListPtr<T> lp) { return ptr != lp.ptr; }
      // �������� �� �������
      bool empty() { return ptr==NULL; }
      // �������� - ��������� �� �������?
      bool last() { return ptr->next()==NULL; }
      operator bool() { return !empty(); }
		// ������� �� ������ ��������� ������� (������� ������� ������)
      // ��������� ������� �����������
      TListPtr<T> removenext();
      // ������� � ���������� ��������� �� ������� �������
      void deletenext() { removenext().deletelist(); }
      // �������� ������� ����� ��������
      void insertnext(T element);
      // ���������� ��������� �� ��������� �������
      // ��������� ��������� ������ � ���������� ��������
      void setnext(TListPtr<T> p) {
         if( ptr )
            ptr->next() = p.ptr;
      }
      // �������� ������� � ������.
      // ���� ������� ������� - �� ������, �� ������ ������ �����������!
      void insertfirst(T element);
      // ������� � ������� ������ �������.
      // ���� ������� ������� - �� ������. �� ������ ����������!
      TListPtr<T> removefirst();
      // ������� � ���������� ������ �������.
      // ���� ������� ������� - �� ������. �� ������ ����������!
      void deletefirst() { removefirst().deletelist(); }
      // ��������� ������
      // ���� ������� ������� - �� ������. �� ������ ����������!
      void deletelist();
    private:
      // ����������������� ������ � ��������� �� ��������� �������
      TListEntity<T> *ptr;
};

//
// ����� ������. ������ ����� �������� � ��� ����, ��������� ������
// ��������� ������ TListPtr.  ����, ����� ��� ����� ������ ������
// ������.  ���� ����� �������� � ���� ��������� �� ������ � �����
// ������.
//

template <class T>
class TList {
	public:
   	// default constructor - empty
		TList() : Head(NULL), Tail(NULL) {};
      // ������������� �� ���������
      TList(TListPtr<T> phead) : Head(phead) { findtail(); };
      // ���������� ������ ������ (��� ������ ��������!
      void clear() { Head = Tail = TListPtr<T>(NULL); };
      // ����� ����� ������ � ���������� tail
      TListPtr<T> findtail();
      // �������� ������� � ������ �����
      void push(T element) {
      	Head.insertfirst(element);
         if( !Tail ) Tail = Head;
      }
      // ������� ������ ������� � ������� ��� �� ������
      T pop() {
      	T data = *Head;
         Head.deletefirst();
         if( !Head )
         	Tail = TListPtr<T>(NULL);
         return data;
      };
      // �������� ������� � ����� ������
      void add(T element) {
      	Tail.insertnext(element);
         if( !Head )
            Head = Tail;
         if( Tail.next() )
         	Tail++;
      }
      TList<T> operator +=(T element) {
         add(element);
         return *this;
      }
      // ��������� ������ ������ � ����� �����
      // ����������� ������ �� ������������!!!
      void add(TList<T> &list) {
         if( list.empty() )
            return;
         Tail.setnext(list.Head);
         Tail = list.Tail;
         if( !Head )
            Head = list.Head;
      }
      TList<T> operator +=(TList<T> &list) {
         add(list);
         return *this;
      }
      // ���������� �������. ��������! ���������� ��������������
      // ���������� ������� ������, ����� �������� ������ ������
      TList<T> operator=(TList<T> &list) {
         Head = list.Head;
         Tail = list.Tail;
         return *this;
      }
		// ������� ������ ������
      TListPtr<T> head() { return Head; }
		// ������� ��������� ������� ������
      TListPtr<T> tail() { return Tail; }
      // ���������� ������
      void deletelist() {
         if( Head )
         	Head.deletelist();
        	Head = Tail = TListPtr<T>(NULL);
      }
      // ���� �� ������
      bool empty() {
         return Head.empty();
      }

   private:
   	TListPtr<T> Head, Tail;  // ��������� �� ������ � ����� ������
};

// ------------------------------------------------------------------------
// ����� ���� ���������� ��������� �������� �������
// ------------------------------------------------------------------------

// ������� ��������� �� ������� ������� �� ������.
// ������������ ��������� �������
template <class T>
TListPtr<T> TListPtr<T>::removenext() {
   if( !ptr || !ptr->next() )
   	return NULL;	// ������ ���� ��� �������� ������������ �������
   TListEntity<T> *dead = ptr->next();
	ptr->next() = dead->next();
   dead->next() = NULL;
	return TListPtr<T>(dead);
}

// ������� ������ ������� �� ������.
// ��������������, ��� ������� ������� - ��������, ����� � ������ �������� ������
// ������������ ��������� �������
template <class T>
TListPtr<T> TListPtr<T>::removefirst() {
   if( !ptr )
   	return NULL;	// ������ ����
   TListEntity<T> *dead = ptr;
   ptr = dead->next();
   dead->next() = NULL;
	return TListPtr<T>(dead);
}

// �������� ������� � ������ ����� ��������
template <class T>
void TListPtr<T>::insertnext(T element) {
   TListEntity<T> *newborn = new TListEntity<T>( element );
   if( !ptr ) { // ��������� � ������ ������
		ptr = newborn;
      newborn->next() = NULL;
   } else {	// ��������� ����� ��������
		newborn->next() = ptr->next();
      ptr->next() = newborn;
	}
}

// �������� ������� � ������ ������
// ��������������, ��� ������� ������� - ��������, ����� ������ ������ �����������
template <class T>
void TListPtr<T>::insertfirst(T element) {
   TListEntity<T> *newborn = new TListEntity<T>( element );
   newborn->next() = ptr;
   ptr = newborn;
}

// ���������� ���� ������ ������� � �������� ��������
// ��������� ���������� ����������� ������
template <class T>
void TListPtr<T>::deletelist() {
   if( empty() )
      return;  // ������ �������

   // ���������� ������
   TListEntity<T> *head = ptr;
   (*this)++;

   // ������� �������� ���� ��� ��� ���� � �� ���������� ����� �� ������
	while( ptr && ptr != head ) {
		TListEntity<T> *dead = ptr;
		(*this)++;
		delete dead;
   }

   // ������� ������
   delete head;
}

// ����� ��������� ������� ������
template <class T>
TListPtr<T> TList<T>::findtail() {
	Tail = Head;
	if( !Tail )
   	return Tail;
   while( Tail.next() )
   	Tail++;
   return Tail;
}
#endif
