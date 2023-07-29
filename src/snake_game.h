typedef struct DoubleLinkedNode
{
	void *data;
	DoubleLinkedNode* next;
	DoubleLinkedNode* prev;
} DoubleLinkedNode;

typedef struct DoubleLinkedList
{
	DoubleLinkedNode* first;
	DoubleLinkedNode* last;
} DoubleLinkedList;

void InitDoubleLinkedList(DoubleLinkedList *list)
{
	list->first = 0;
	list->last = 0;
}

int AddNodeAtFront(DoubleLinkedList *list, void *data)
{
	DoubleLinkedNode *node = new DoubleLinkedNode;
	node->data = data;
	node->prev = 0;
	node->next = list->first;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return 0;
	}
	
	DoubleLinkedNode *first_node = list->first;
	first_node->prev = node;
	
	list->first = node;
	
	return 0;
}

int AddNodeAtBack(DoubleLinkedList *list, void *data)
{
	DoubleLinkedNode *node = new DoubleLinkedNode;
	node->data = data;
	node->prev = list->last;
	node->next = 0;
	
	if (list->first == 0)
	{
		list->first = list->last = node;
		return 0;
	}
	
	DoubleLinkedNode *last_node = list->last;
	last_node->next = node;
	
	list->last = node;
	
	return 0;
}

void *RemoveNodeAtFront(DoubleLinkedList *list)
{
	if (list->first == 0)
	{
		return 0;
	}
	
	DoubleLinkedNode *node = list->first;
	void *data = node->data;
	
	DoubleLinkedNode *first_node = node->next;
	
	list->first = first_node;
	
	if (first_node != 0)
	{
		first_node->prev = 0;
	} else {
		list->last = 0;
	}
	
	delete node;
	return data;
}

void *RemoveNodeAtBack(DoubleLinkedList *list)
{
	if (list->first == 0)
	{
		return 0;
	}
	
	DoubleLinkedNode *node = list->last;
	void *data = node->data;
	
	DoubleLinkedNode *last_node = node->prev;
	
	list->last = last_node;
	
	if (last_node != 0)
	{
		last_node->next = 0;
	} else {
		list->first = 0;
	}
	
	delete node;
	return data;
}

typedef struct
{
	int x;
	int y;
} Point2i;

typedef struct
{
	unsigned int field_width;
	unsigned int field_height;
	DoubleLinkedList snake_segments;
	DoubleLinkedList fruits;
	Point2i snake_direction;
} SnakeGameState;

int InitSnakeGame(SnakeGameState *state, int field_width, int field_height)
{
	InitDoubleLinkedList(&state->snake_segments);
	InitDoubleLinkedList(&state->fruits);
	state->field_width = field_width;
	state->field_height = field_height;
	
	Point2i* p1 = new Point2i{field_width/2, field_height/2};
	Point2i* p2 = new Point2i{p1->x+1, p1->y};
	AddNodeAtBack(&state->snake_segments, p1);
	AddNodeAtBack(&state->snake_segments, p2);
	
	p1 = new Point2i{1, 1};
	AddNodeAtBack(&state->fruits, p1);
	
	state->snake_direction.x = -1;
	state->snake_direction.y = 0;
	
	return 0;
}

int UpdateSnakeGameState(SnakeGameState *state)
{
	DoubleLinkedNode* h = state->snake_segments.first;
	//Point2i* head = (Point2i*)((state->snake_segments.first).data);
	Point2i* head = (Point2i*)(state->snake_segments.first->data);
	Point2i* p1 = new Point2i{head->x + state->snake_direction.x, head->y + state->snake_direction.y};
	
	if (p1->x < 0 || p1->x >= state->field_width || p1->y < 0 || p1->y >= state->field_height)
	{
		delete p1;
		return 1; // out of bounds
	}
	
	BOOL occupied_by_snake = 0;
	DoubleLinkedNode* node = state->snake_segments.first;
	while (node->next)
	{
		Point2i* p = (Point2i*)(node->data);
		if (p->x == p1->x && p->y == p1->y)
		{
			occupied_by_snake = 1;
			break;
		}
		node = node->next;
	}
	
	if (occupied_by_snake == 1)
	{
		delete p1;
		return 2; // snake has bitten itself :(
	}
	
	BOOL occupied_by_fruit = 0;
	node = state->fruits.first;
	while (node)
	{
		Point2i* p = (Point2i*)(node->data);
		if (p->x == p1->x && p->y == p1->y)
		{
			occupied_by_fruit = 1;
			break;
		}
		node = node->next;
	}
	
	
	AddNodeAtFront(&state->snake_segments, p1);
	if (occupied_by_fruit == 0)
	{
		Point2i* p = (Point2i*)RemoveNodeAtBack(&state->snake_segments);
		delete p;
	} else {
		//Point2i* head = (Point2i*)(state->snake_segments.first->data);
		Point2i* p = (Point2i*)(state->fruits.first->data);
		p->x = rand() % state->field_width;
		p->y = rand() % state->field_height;
	}
	
	
	return 0;
}

