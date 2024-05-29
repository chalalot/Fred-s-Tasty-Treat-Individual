#include "DataManager.h"
#include "Coin.h"
#include "Helper.h"
#include "LinkedList.h"
#include "Node.h"
#include <fstream>
#include <string>
#include <vector>

DataManager::DataManager(std::string mealFileName, std::string moneyFileName) {
  // Create an empty LinkedList and balance
  this->meals_group = new TwoDimensionLinkedList();
  this->balance = new Balance();

  this->mealFile = mealFileName;
  this->moneyFile = moneyFileName;

  std::ifstream mealFile(mealFileName);
  std::ifstream moneyFile(moneyFileName);

  // Read the files
  // When assigning data from the read files, we need to check if the numbers
  // are correct, that is it fits into floats and unsigned data
  std::string line;
  while (std::getline(mealFile, line)) {
    // Process each line: Create a new FoodItem, assign it to a Node, then add
    // that Node to LinkedList
    std::vector<std::string> tokens = {};
    std::vector<std::string> prices = {};
    Helper::splitString(line, tokens, "|");
    if (!meals_group->isDistinct(tokens[1])) {
        LinkedList *meals = new LinkedList();
        meals->name = tokens[1];
        meals_group->append(meals);
    }
  
    LinkedList *currentMealGroup = meals_group->getFirst();
    while (currentMealGroup != nullptr) {
      if (currentMealGroup->name == tokens[1]) {
        if (tokens.size() == 5 && tokens[2].size() <= NAMELEN &&
          tokens[3].size() <= DESCLEN && FoodItem::isValidIdFormat(tokens[0]) &&
          Price::isValidPrice(tokens[4]) && !currentMealGroup->getById(tokens[0])) {
            // Slit the price into its integer and fractional part to create Price
            // object
            Helper::splitString(Helper::floatToString(std::stof(tokens[4]), 2),
                                prices, ".");
            Price price = Price();
            price.dollars = std::stoi(prices[0]);
            price.cents = std::stoi(prices[1]);
            // Create FoodItem object
            FoodItem *item = new FoodItem(tokens[0], tokens[1], tokens[2], tokens[3], price);
            // Create node
            Node *node = new Node();
            node->data = item;
            currentMealGroup->append(node);
        }
      }
      currentMealGroup = currentMealGroup->next;
    }
    
    
  }

  while (std::getline(moneyFile, line)) {
    // Process each line: Split by "." and create Coin objects
    std::vector<std::string> tokens = {};
    Helper::splitString(line, tokens, DELIM);

    if (tokens.size() == 2 && Helper::isNumber(tokens[0]) &&
        Helper::isNumber(tokens[1]) && Coin::isDenomination(tokens[0]) &&
        Helper::isValidInteger(tokens[1])) {
      Coin coin = Coin();
      coin.denom = Coin::intToDenomination(std::stoi(tokens[0]));
      coin.count = std::stoi(tokens[1]);
      // Insert coin to Balance
      this->balance->insert(coin);
    }
  }

  // Close the files
  mealFile.close();
  moneyFile.close();
}
// Save the file
void DataManager::save() {
  std::ofstream mealFile(this->mealFile);
  std::ofstream coinFile(this->moneyFile);

  if (!mealFile.is_open()) {
    std::cerr << "Failed to open file: " << this->mealFile << std::endl;
    return;
  }

  LinkedList *currentMealGroup = this->meals_group->getFirst();
  while (currentMealGroup != nullptr) {
    Node *current = currentMealGroup->getFirst();
    while (current) {
      mealFile << current->data->id << SEPARATOR << current->data->category 
              << SEPARATOR << current->data->name << SEPARATOR
              << current->data->description << SEPARATOR
              << Helper::floatToString(current->data->price.value(), 2) << "\n";
      current = current->next;
    }
    currentMealGroup = currentMealGroup->next;
  }

  for (Coin coin : this->balance->balance) {
    coinFile << coin.denom << DELIM << coin.count << "\n";
  }

  mealFile.close();
  coinFile.close();
};

DataManager::~DataManager() {
  delete this->meals_group;
  delete this->balance;
}
