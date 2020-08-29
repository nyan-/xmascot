#include <stdio.h>
#include <stdlib.h>
#include "namelist.h"

NameListRec *
namelist_new(void)
{
	NameListRec *p;
	if ((p = malloc(sizeof(*p))) == NULL) {
		perror("newlist_new");
		exit(1);
	}
	p->next = NULL;
	p->name = NULL;
	p->data = NULL;
	return p;
}

void
namelist_delete(NameListRec *p)
{
	if (p) {
		if (p->name)
			free(p->name);
		free(p);
	}
}

void
namelist_delete_list(NameList head)
{
	NameListRec *p, *q;
	p = head->next;
	while (p != NULL) {
		q = p;
		p = p->next;
		namelist_delete(q);
	}
	namelist_delete(head);
}

void
namelist_add(NameList head, NameListRec *na)
{
	na->next = head->next;
	head->next = na;
}
	
void*
namelist_find(NameList head, char *name)
{
	NameListRec *p = head->next;
	while (p != NULL) {
		if (!strcmp(p->name, name))
			return p->data;
		p = p->next;
	}
	return NULL;
}

