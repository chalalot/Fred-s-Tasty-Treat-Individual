#include "TwoDimensionLinkedList.h"
#include "LinkedList.h"
#include <string>


TwoDimensionLinkedList::TwoDimensionLinkedList() {
    this->head = nullptr;
    this->count = 0;
}

TwoDimensionLinkedList::~TwoDimensionLinkedList()
{
}

void TwoDimensionLinkedList::append(LinkedList *LinkedListNode) {
    if (!this->head) {
    this->head = LinkedListNode;
    this->tail = LinkedListNode;
  } else {
    this->tail->next = LinkedListNode;
    LinkedListNode->previous = this->tail;
    this->tail = LinkedListNode;
  }
  this->count++;
  LinkedListNode->next = nullptr;
}

void TwoDimensionLinkedList::remove(std::string id) {

}

bool TwoDimensionLinkedList::isDistinct(std::string name) {
    int count = 0;
    LinkedList* currentRow = head;
    bool result = false;
    while (currentRow) {
        if (currentRow->name == name) {
            count++;
        }
        currentRow = currentRow->next;  // Move to the next row
    }
    if (count == 1) {
        result = true;
    } else {
        result = false;
    }
    return result;
}

LinkedList *TwoDimensionLinkedList::getFirst() {
    return this->head;
};
