#pragma once

typedef struct Plt_Linked_List_Node Plt_Linked_List_Node;
typedef struct Plt_Linked_List_Node {
	void *data;
	Plt_Linked_List_Node *next;
} Plt_Linked_List_Node;

typedef struct Plt_Linked_List {
	Plt_Linked_List_Node *root;
} Plt_Linked_List;