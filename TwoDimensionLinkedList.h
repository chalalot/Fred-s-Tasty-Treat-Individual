#ifndef TWO_DIMENSION_LINKED_LIST
#define TWO_DIMENSION_LINKED_LIST

#include "LinkedList.h"
class TwoDimensionLinkedList
{
private:
    /* data */
    LinkedList *head;
    LinkedList *tail;
    TwoDimensionLinkedList *next;
    unsigned count;
public:
    TwoDimensionLinkedList();
    ~TwoDimensionLinkedList();
    void append(LinkedList *LinkedListNode);
    void remove(std::string id);
    // Get Node
    LinkedList *getFirst();
    bool isDistinct(std::string name);
    // Utility methods
    bool isEmpty();
    // Get the next Id of all the Nodes in the LinkedList
    int getNextId();
    // Sort the LinkedList by alphabetical order
    void sortByAlpha();
};

#endif //TWO_DIMENSION_LINKED_LIST