#ifndef __namelist_h_
#define __namelist_h_

typedef struct NameListRec {
	struct NameListRec *next;
	char *name;
	void *data;
} NameListRec, *NameList;

/* namelist.c */
NameListRec *namelist_new(void);
void namelist_delete(NameListRec *p);
void namelist_delete_list(NameList head);
void namelist_add(NameList head, NameListRec *na);
void *namelist_find(NameList head, char *name);

#endif
