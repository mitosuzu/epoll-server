#include "DataStructureLib.h"
#include <stdlib.h>

DSError initializeHash(LinkHash **pp_hash)
{
    if (pp_hash == NULL)
        return NULL_ERROR;
    *pp_hash = (LinkHash *)calloc(1, sizeof(LinkHash));
    if (*pp_hash == NULL)
        return NULL_ERROR;
    for (int i = 0; i < MAX_SIZE; ++i)
    {
        (*pp_hash)->arr_hash[i] = NULL;
    }
    (*pp_hash)->node_count = 0;
    return OK;
}

DSError addHashElement(LinkHash *p_hash, UserAccount element)
{
    int code = atoi(element.account);
    int index = code % MAX_SIZE;
    ListNode *new_node = (ListNode *)calloc(1, sizeof(ListNode));
    if (new_node == NULL)
        return NULL_ERROR;

    new_node->data = element;

    new_node->p_next = p_hash->arr_hash[index];
    p_hash->arr_hash[index] = new_node;
    p_hash->node_count++;
    return OK;
}

DSError deleteHashElementByCode(LinkHash *p_hash, int delete_code)
{
    if (p_hash == NULL)
        return NULL_ERROR;
    int index = delete_code % MAX_SIZE;
    int temp_code = atoi(p_hash->arr_hash[index]->data.account);

    if (p_hash->arr_hash[index]->p_next == NULL && temp_code == delete_code)
    {
        free(p_hash->arr_hash[index]);
        p_hash->arr_hash[index] = NULL;
        p_hash->node_count--;
        return OK;
    }
    if (p_hash->arr_hash[index]->p_next == NULL && temp_code != delete_code)
        return CODE_NOT_EXIST;

    ListNode *previous_node = p_hash->arr_hash[index];
    ListNode *current_node = p_hash->arr_hash[index]->p_next;
    ListNode *deleted_node = NULL;

    while (current_node != NULL)
    {
        temp_code = atoi(current_node->data.account);
        if (temp_code == delete_code)
        {
            deleted_node = current_node;
            previous_node->p_next = deleted_node->p_next;
            free(deleted_node);
            deleted_node = NULL;
            p_hash->node_count--;
            return OK;
        }
        previous_node = current_node;
        current_node = current_node->p_next;
    }

    return CODE_NOT_EXIST;
}

DSError modifyHashElementByCode(LinkHash *p_hash, UserAccount new_element, int modified_code)
{
    int index = modified_code % MAX_SIZE;
    ListNode *modified_node = NULL;
    DSError err;
    err = searchHashNodeByElementKey(p_hash, &modified_node, modified_code);
    if (err < 0)
        return MODIFY_DEFEAT;
    modified_node->data = new_element;
    return OK;
}

DSError searchHashNodeByElementKey(LinkHash *p_hash, ListNode **searched_node, int searched_code)
{
    if (p_hash == NULL)
        return NULL_ERROR;
    int index = searched_code % MAX_SIZE;
    if (p_hash->arr_hash[index] == NULL)
        return CODE_NOT_EXIST;
    int temp_code = atoi(p_hash->arr_hash[index]->data.account);

    if (p_hash->arr_hash[index]->p_next == NULL && temp_code == searched_code)
    {
        *searched_node = p_hash->arr_hash[index];
        return OK;
    }
    if (p_hash->arr_hash[index]->p_next == NULL && temp_code != searched_code)
        return CODE_NOT_EXIST;

    ListNode *current_node = p_hash->arr_hash[index];
    while (current_node != NULL)
    {
        temp_code = atoi(current_node->data.account);
        if (temp_code == searched_code)
        {
            *searched_node = current_node;
            return OK;
        }
        current_node = current_node->p_next;
    }

    return CODE_NOT_EXIST;
}

DSError destoryHash(LinkHash **pp_hash)
{
    if (pp_hash == NULL)
        return NULL_ERROR;

    for (int i = 0; i < MAX_SIZE; i++)
    {
        if ((*pp_hash)->arr_hash[i] == NULL)
            continue;
        if ((*pp_hash)->arr_hash[i]->p_next == NULL)
        {
            free((*pp_hash)->arr_hash[i]);
            (*pp_hash)->arr_hash[i] = NULL;
            continue;
        }

        ListNode *previous_node = (*pp_hash)->arr_hash[i];
        ListNode *current_node = (*pp_hash)->arr_hash[i]->p_next;
        ListNode *deleted_node = NULL;

        while (current_node != NULL)
        {
            deleted_node = current_node;
            previous_node->p_next = deleted_node->p_next;
            free(deleted_node);
            deleted_node = NULL;
            (*pp_hash)->node_count--;
            
            current_node = current_node->p_next;
        }
    }
    free(*pp_hash);
    *pp_hash = NULL;
    return OK;
}

DSError initLinkList(LinkList **pp_list)
{
    (*pp_list) = (LinkList *)calloc(1, sizeof(LinkList));
    ListNode *p_node = (ListNode *)calloc(1, sizeof(ListNode));
    if (p_node == NULL)
        return NULL_ERROR;
    p_node->p_next = NULL;

    (*pp_list)->p_head = p_node;
    return OK;
}

DSError insertElement(LinkList *p_list, UserAccount data, int pos)
{
    ListNode *p_node = p_list->p_head;
    ListNode *p_new_node = (ListNode *)calloc(1, sizeof(ListNode));
    p_new_node->data = data;
    p_new_node->p_next = NULL;
    if (pos < 1)
        return CODE_NOT_EXIST;
    for (int i = 0; i < pos - 1 && p_node != NULL; i++)
    {
        p_node = p_node->p_next;
    }

    if (p_node == NULL)
        return CODE_NOT_EXIST;
    p_new_node->p_next = p_node->p_next;
    p_node->p_next = p_new_node;

    return OK;
}

DSError modifyElement(LinkList *p_list, UserAccount new_data, int pos)
{
    ListNode *p_node = p_list->p_head;
    int i = 0;
    while (i < pos)
    {
        p_node = p_node->p_next;
        i++;
    }
    p_node->data = new_data;
    return OK;
}

DSError deleteElement(LinkList *p_list, int pos)
{
    if (p_list->p_head->p_next == NULL)
        return LIST_EMPTY;
    ListNode *p_node = p_list->p_head;
    ListNode *p_deleted_node = NULL;
    int i = 0;
    while (i < pos - 1)
    {
        p_node = p_node->p_next;
        i++;
    }
    p_deleted_node = p_node->p_next;
    p_node->p_next = p_node->p_next->p_next;
    free(p_deleted_node);
    p_deleted_node = NULL;
    return OK;
}

DSError destoryLinkList(LinkList **p_list)
{
    DSError err = OK;
    while (err != LIST_EMPTY)
    {
        err = deleteElement(*p_list, 0);
    }
    free(*p_list);
    *p_list = NULL;
    return OK;
}
