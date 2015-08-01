/**********************************************************************

    Универсальный шаблон класса "Однонаправленный список" (TList)

    Операции: переход к следующему элементу, доступ к элементу,
    вставка элемента.

    Изменения от 1998-12-27:
    	исправлена ошибка в операции += или add(TList) при добалении
        пустого списка

    (c) 1998 Сергей Герштейн

**********************************************************************/

#ifndef _TList_H_
#define _TList_H_

#include <stddef.h>

//
// Класс элемента списка. Почти никогда нет необходимости работать
// с ним явно.  Все операции производятся через TList или TListPtr
//

template <class T>
class TListEntity {
	public:
      // data остаётся неинициализированной!
   	TListEntity() : Next(NULL) {};
      // полная инициализация элемента
      TListEntity(T element) : Next(NULL), Data(element) {};
      // указатель на следующий
      TListEntity<T>*& next() { return Next; };
      // собственно данные в чистом виде
      T& data() { return Data; };
   private:
   	TListEntity<T> *Next;
      T Data;
};

//
// Класс указателя списка.
// Указатель указывает на "хвост" списка, начиная с текущего элемента
//

template <class T>
class TListPtr {
	public:
   	// инициализация списка начальным элементом
   	TListPtr(TListEntity<T> *first=NULL) : ptr(first) {}
      // доступ к текущему элементу
      T& operator*() { return ptr->data(); }
      // следующий элемент
      TListPtr<T> next() { return TListPtr<T>(ptr? ptr->next() : NULL); }
      // перейти к следующему (prefix ++)
      TListPtr<T> operator++() {
      	if( ptr ) ptr = ptr->next();
        	return *this;
      }
      // перейти к следующему (postfix ++)
      TListPtr<T> operator++(int) { return ++*this; }
      // присваивание указателей
      TListPtr<T> operator=(TListPtr<T> lp) { return ptr = lp.ptr; }
      // сравнение указателей
      bool operator==(TListPtr<T> lp) { return ptr == lp.ptr; }
      bool operator!=(TListPtr<T> lp) { return ptr != lp.ptr; }
      // проверка на пустоту
      bool empty() { return ptr==NULL; }
      // проверка - последний ли элемент?
      bool last() { return ptr->next()==NULL; }
      operator bool() { return !empty(); }
		// удалить из списка следующий элемент (ТЕКУЩИЙ УДАЛИТЬ НЕЛЬЗЯ)
      // Удаленный элемент возвращаетс
      TListPtr<T> removenext();
      // Удалить и уничтожить следующий за текущим элемент
      void deletenext() { removenext().deletelist(); }
      // вставить элемент после текущего
      void insertnext(T element);
      // установить указатель на следующий элемент
      // БЕЗОПАСНО ПРИМЕНЯТЬ ТОЛЬКО К ПОСЛЕДНЕМУ ЭЛЕМЕНТУ
      void setnext(TListPtr<T> p) {
         if( ptr )
            ptr->next() = p.ptr;
      }
      // вставить элемент в начало.
      // Если текущий элемент - не голова, то СПИСОК СТАНЕТ ДВУХГОЛОВЫМ!
      void insertfirst(T element);
      // удалить и вернуть первый элемент.
      // Если текущий элемент - не голова. то СПИСОК РАЗОРВЕТСЯ!
      TListPtr<T> removefirst();
      // удалить и уничтожить первый элемент.
      // Если текущий элемент - не голова. то СПИСОК РАЗОРВЕТСЯ!
      void deletefirst() { removefirst().deletelist(); }
      // уничожить список
      // Если текущий элемент - не голова. то СПИСОК РАЗОРВЕТСЯ!
      void deletelist();
    private:
      // инкапсулированные данные и указатель на следующий элемент
      TListEntity<T> *ptr;
};

//
// Класс списка. Иногда можно обойтись и без него, используя только
// указатели списка TListPtr.  Хотя, часто сам класс списка бывает
// удобен.  Этот класс содержит в себе указатели на начало и конец
// списка.
//

template <class T>
class TList {
	public:
   	// default constructor - empty
		TList() : Head(NULL), Tail(NULL) {};
      // инициализация по указателю
      TList(TListPtr<T> phead) : Head(phead) { findtail(); };
      // установить пустой список (все данные теряются!
      void clear() { Head = Tail = TListPtr<T>(NULL); };
      // найти конец списка и установить tail
      TListPtr<T> findtail();
      // добавить элемент в начало спика
      void push(T element) {
      	Head.insertfirst(element);
         if( !Tail ) Tail = Head;
      }
      // вернуть первый элемент и удалить его из списка
      T pop() {
      	T data = *Head;
         Head.deletefirst();
         if( !Head )
         	Tail = TListPtr<T>(NULL);
         return data;
      };
      // добавить элемент в конец списка
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
      // приписать другой список в конец этого
      // копирование списка не производится!!!
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
      // присвоение списков. ВНИМАНИЕ! Необходимо предварительно
      // уничтожить текущий список, иначе возможна утечка памяти
      TList<T> operator=(TList<T> &list) {
         Head = list.Head;
         Tail = list.Tail;
         return *this;
      }
		// вернуть начало списка
      TListPtr<T> head() { return Head; }
		// вернуть последний элемент списка
      TListPtr<T> tail() { return Tail; }
      // уничтожить список
      void deletelist() {
         if( Head )
         	Head.deletelist();
        	Head = Tail = TListPtr<T>(NULL);
      }
      // пуст ли список
      bool empty() {
         return Head.empty();
      }

   private:
   	TListPtr<T> Head, Tail;  // указатели на начало и конец списка
};

// ------------------------------------------------------------------------
// Далее идет реазизация описанных шаблонов классов
// ------------------------------------------------------------------------

// удалить СЛЕДУЮЩИЙ ЗА ТЕКУЩИМ элемент из списка.
// Возвращается удаленный элемент
template <class T>
TListPtr<T> TListPtr<T>::removenext() {
   if( !ptr || !ptr->next() )
   	return NULL;	// список пуст или содержит единственный элемент
   TListEntity<T> *dead = ptr->next();
	ptr->next() = dead->next();
   dead->next() = NULL;
	return TListPtr<T>(dead);
}

// удалить первый элемент из списка.
// Предполагается, что текущий элемент - ГОЛОВНОЙ, иначе в списке появится РАЗРЫВ
// Возвращается удаленный элемент
template <class T>
TListPtr<T> TListPtr<T>::removefirst() {
   if( !ptr )
   	return NULL;	// список пуст
   TListEntity<T> *dead = ptr;
   ptr = dead->next();
   dead->next() = NULL;
	return TListPtr<T>(dead);
}

// Вставить элемент в список после текущего
template <class T>
void TListPtr<T>::insertnext(T element) {
   TListEntity<T> *newborn = new TListEntity<T>( element );
   if( !ptr ) { // добавляем в пустой список
		ptr = newborn;
      newborn->next() = NULL;
   } else {	// добавляем после текущего
		newborn->next() = ptr->next();
      ptr->next() = newborn;
	}
}

// Вставить элемент в начало списка
// Предполагается, что текущий элемент - головной, иначе список станет ДВУХГОЛОВЫМ
template <class T>
void TListPtr<T>::insertfirst(T element) {
   TListEntity<T> *newborn = new TListEntity<T>( element );
   newborn->next() = ptr;
   ptr = newborn;
}

// Уничтожить весь список начиная с текущего элемента
// Корректно уничтожает зациклинный список
template <class T>
void TListPtr<T>::deletelist() {
   if( empty() )
      return;  // нечего удалять

   // запоминаем голову
   TListEntity<T> *head = ptr;
   (*this)++;

   // удаляем элементы пока они еще есть и не наткнулись снова на голову
	while( ptr && ptr != head ) {
		TListEntity<T> *dead = ptr;
		(*this)++;
		delete dead;
   }

   // удаляем голову
   delete head;
}

// найти последний элемент списка
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
