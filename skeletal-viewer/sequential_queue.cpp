#include "queue.h"

#include <iostream>
#include <exception>

using namespace std;

//-----------------------------------------------------------------------------

template <class Item>
void SequentialQueue<Item>::Initialize(unsigned int size)
{
  ItemList = new Item[size];
  if(!ItemList) throw exception("not enough storage to allocate queue");
  MaxQueueSize = size;
  Count = 0;
  FrontPos = 0;
  RearPos = 0;
}

//-----------------------------------------------------------------------------

template <class Item>
SequentialQueue<Item>::~SequentialQueue()
{
  if(ItemList)
    delete[] ItemList;
}

//-----------------------------------------------------------------------------

template <class Item>
bool SequentialQueue<Item>::IsEmpty()
{
  return (Count == 0);
}

//-----------------------------------------------------------------------------

template <class Item>
bool SequentialQueue<Item>::IsFull()
{
  return (Count == MaxQueueSize);
}

//-----------------------------------------------------------------------------

template <class Item>
Item SequentialQueue<Item>::Remove()
{
  if(IsEmpty())
  {
    throw exception("Trying to remove item from an empty queue");
  }
  else
  {
    unsigned int tmp = FrontPos;
    FrontPos = (FrontPos + 1) % MaxQueueSize;
    Count--;
    // call copy constructor
    return ItemList[tmp];
  }
}

//-----------------------------------------------------------------------------

template <class Item>
void SequentialQueue<Item>::Insert(Item& item)
{
  if(IsFull())
  {
    throw exception("Trying to add to a full queue");
  }
  else
  {
    // call copy constructor
    ItemList[RearPos] = item;
    RearPos = (RearPos + 1) % MaxQueueSize;
    Count++;
  }
}

//-----------------------------------------------------------------------------

#if 0

    if( (FrontPos + 1) == MaxQueueSize)
    {
      FrontPos = 0;
    }
    else
    {
      FrontPos++;
    }

    //FrontPos = (FrontPos + 1) % MaxQueueSize;

    // The above if/else block accomplishes the same thing as the mod 
    // above, because FrontPos is never >= MaxQueueSize, I like the
    // if/else because its more readable
    // We need this for a sequential static memory queue because we want
    // to move around the memory block in a circular fashion
    //
    // example with MaxSize=6; Front=0; Rear=3
    //
    // [0000] first
    // [0001] second
    // [0002] third
    // [0003] fourth
    // [0004]
    // [0005]
    //
    // queue->remove() Front=1; Rear=3
    //
    // [0000] ------
    // [0001] second
    // [0002] third
    // [0003] fourth
    // [0004]
    // [0005]
    //
    // queue->insert(fifth) Front=1; Rear=4
    //
    // [0000] ------
    // [0001] second
    // [0002] third
    // [0003] fourth
    // [0004] fifth
    // [0005]
    //
    // queue->insert(sixth) Front=1; Rear=5
    //
    // [0000] ------
    // [0001] second
    // [0002] third
    // [0003] fourth
    // [0004] fifth
    // [0005] sixth
    //
    // queue->insert(seventh) Front=1; Rear= (5+1)==6 ? 0 : 7;
    //
    // [0000] seventh
    // [0001] second
    // [0002] third
    // [0003] fourth
    // [0004] fifth
    // [0005] sixth
    //
    // queue->remove()
    //
    // [0000] seventh
    // [0001] -----
    // [0002] third
    // [0003] fourth
    // [0004] fifth
    // [0005] sixth

#endif