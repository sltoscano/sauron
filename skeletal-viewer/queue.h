#pragma once

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class Item>
class SequentialQueue
{
	Item * ItemList;
	unsigned int Count;
	unsigned int FrontPos;
	unsigned int RearPos;
	unsigned int MaxQueueSize;

public:
	void Initialize(unsigned int size);
	bool IsEmpty();
	bool IsFull();
	Item Remove();
	void Insert(Item& item);

	~SequentialQueue();
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

template <class Item>
class LinkedQueue
{
private:
	struct ItemNode;
	ItemNode * FrontPos;
	ItemNode * RearPos;

public:
	void Initialize();
	bool IsEmpty();
	bool IsFull();
	Item& Front();
	Item Remove();
	void Insert(Item& item);

	~LinkedQueue();

private:
	struct ItemNode
	{
		Item value;
		ItemNode * next;
	};
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
