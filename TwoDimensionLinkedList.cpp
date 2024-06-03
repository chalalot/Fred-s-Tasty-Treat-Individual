#include "TwoDimensionLinkedList.h"
#include "LinkedList.h"
#include <string>


TwoDimensionLinkedList::TwoDimensionLinkedList() {
    this->head = nullptr;
    this->count = 0;
}

TwoDimensionLinkedList::~TwoDimensionLinkedList()
{
  while (head != nullptr) {
    LinkedList *temp = this->head;
    this->head = this->head->next;
    delete temp;
  }
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

int TwoDimensionLinkedList::getNextId() {
  int largestId = 0;
  LinkedList *currentMealGroup = this->getFirst();
  // Traverse until the end of the LinkedList
  while (currentMealGroup != nullptr) {
    int largestFound = currentMealGroup->getNextId();
    if (largestFound > largestId) {
      largestId = largestFound;
    }
    currentMealGroup = currentMealGroup->next;
  }
  return largestId;
}