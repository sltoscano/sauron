#include "queue.h"

#include <iostream>
#include <exception>

using namespace std;

//-----------------------------------------------------------------------------

template <class Item>
void LinkedQueue<Item>::Initialize()
{
  FrontPos = NULL;
  RearPos = NULL;
}

//-----------------------------------------------------------------------------

template <class Item>
LinkedQueue<Item>::~LinkedQueue()
{
  while(IsEmpty() == false)
  {
    Remove();
  }
}

//-----------------------------------------------------------------------------

template <class Item>
bool LinkedQueue<Item>::IsEmpty()
{
  return (FrontPos == NULL);
}

//-----------------------------------------------------------------------------

template <class Item>
bool LinkedQueue<Item>::IsFull()
{
  // Since this is a linked queue, the only limit is the system memory
  return false;
}

//-----------------------------------------------------------------------------

template <class Item>
Item& LinkedQueue<Item>::Front()
{
  if(IsEmpty())
  {
    throw exception("Trying to inspect an item from an empty queue");
  }
  else
  {
    return FrontPos->value;
  }
}

//-----------------------------------------------------------------------------

template <class Item>
Item LinkedQueue<Item>::Remove()
{
  if(IsEmpty())
  {
    throw exception("Trying to remove item from an empty queue");
  }
  else
  {
    // call copy ctor
    Item result = FrontPos->value;
    ItemNode * tmp = FrontPos;
    FrontPos = tmp->next;
    delete tmp;
    if(FrontPos == NULL)
    {
      RearPos = NULL;
    }
    return result;
  }
}

//-----------------------------------------------------------------------------

template <class Item>
void LinkedQueue<Item>::Insert(Item& item)
{
  ItemNode * tmp = new ItemNode;
  if(!tmp)
  {
    throw exception("Out of memory");
  }
  else
  {
    // call copy ctor
    tmp->value = item;
    tmp->next = NULL;
    if(RearPos == NULL)
    {
      FrontPos = tmp;
      RearPos = tmp;
    }
    else
    {
      RearPos->next = tmp;
      RearPos = tmp;
    }
  }
}

//-----------------------------------------------------------------------------
