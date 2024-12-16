#ifndef _DATASTRUCTURELIB_H
#define _DATASTRUCTURELIB_H

#define MAX_SIZE 13

typedef struct element
{
    char account[MAX_SIZE];
    char password[MAX_SIZE];
} UserAccount;

typedef struct node
{
    UserAccount data;
    struct node *p_next;
} ListNode;

typedef struct list
{
    ListNode *p_head;
    int element_count;

} LinkList;

typedef struct hash
{
    ListNode *arr_hash[MAX_SIZE];
    int node_count;
} LinkHash;

typedef enum err
{
    NULL_ERROR = -5,
    CODE_NOT_EXIST,
    MODIFY_DEFEAT,
    LIST_EMPTY,
    ERROR,
    OK
} DSError;

DSError initializeHash(LinkHash **p_hash);
DSError addHashElement(LinkHash *p_hash, UserAccount element);
DSError deleteHashElementByCode(LinkHash *p_hash, int code);
DSError modifyHashElementByCode(LinkHash *p_hash, UserAccount new_element, int code);
DSError searchHashNodeByElementKey(LinkHash *p_hash, ListNode **searched_node, int code);
DSError destoryHash(LinkHash **pp_hash);

DSError initLinkList(LinkList **pp_list);
DSError insertElement(LinkList *p_list, UserAccount element, int pos);
DSError modifyElement(LinkList *p_list, UserAccount new_element, int pos);
DSError deleteElement(LinkList *p_list, int pos);
DSError destoryLinkList(LinkList **pp_list);

#endif
