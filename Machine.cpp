#include "Machine.h"
#include "Coin.h"
#include "DataManager.h"
#include "Helper.h"
#include "Node.h"
#include "iostream"
#include <string>
#include <vector>

Machine::Machine(std::string mealFileName, std::string moneyFileName, bool useColor) {
  DataManager *data = new DataManager(mealFileName, moneyFileName);
  this->data = data;
  if (useColor) {
    this->color = "\033[33m";
  } else {
    this->color = "";
  }
}

Machine::~Machine() {
  // Deallocate the LinkedList
  delete this->data;
}

void Machine::start() {
  bool run = true;

  while (run) {
    std::cout << "Main Menu:\n"
                 "\t1. Display Meal Options\n"
                 "\t2. Purchase Meal\n"
                 "\t3. Save and Exit\n"
                 "Administrator-Only Menu:\n"
                 "\t4. Add Food\n"
                 "\t5. Remove Food\n"
                 "\t6. Display Balance\n"
                 "\t7. Abort Program\n"
                 "Select your option (1-7) : ";
    std::string input = Helper::readInput();
    std::cout << "\n";

    if (input == "1") {
      this->displayMeals();
    } else if (input == "2") {
      this->purchaseMeal();
    } else if (input == "3") {
      this->data->save();
      run = false;
    } else if (input == "4") {
      this->addFood();
    } else if (input == "5") {
      std::cout << "Please enter the ID of the food to remove from the menu: ";
      this->removeFood();
    } else if (input == "6") {
      this->displayBalance();
    } else if (input == "7") {
      run = false;
    } else {
      Helper::printInvalidInput();
      std::cin.clear();
    }
    std::cout << "\n";
  }
}

void Machine::purchaseMeal() {
  // print out prompt
  std::cout << "Purchase Meal\n"
               "-------------\n";
  std::cout << "Please enter the ID of the food you wish to purchase: ";

  // Check if valid ID
  bool run = false;
  bool prompt = true;
  Node *meal = nullptr;
  int priceAsCents = 0;

  while (prompt) {
    std::string mealID = Helper::readInput();
    // Check if user wants to exit
    if (std::cin.eof() || mealID.empty()) {
      std::cout << "Cancel purchase" << "\n";
      std::cin.clear(); // Clear the error flags
      // terminate loop
      run = false;
      prompt = false;
    } else {
      // get the selected meal by ID
      bool found = false;
      LinkedList *currentMealGroup = this->data->meals_group->getFirst();
      while (currentMealGroup != nullptr && !found) {
        meal = currentMealGroup->getById(mealID);
        if (meal != nullptr) {
          found = true;
        }  
        currentMealGroup = currentMealGroup->next;
      }
      
      // check in-stock conditions
      if (meal && meal->data->on_hand > 0 && prompt) {
        run = true;
        // denominate the price to cents
        priceAsCents = meal->data->price.valueAsDenom();
        // print out messages
        std::cout << "You have selected \"" << meal->data->name << " - "
                  << meal->data->description << "\""
                  << ". This will cost you $ " << this->color << (float)priceAsCents / 100 << RESET_COLOR
                  << ".\n";
        std::cout << "Please hand over the money - type in the value of each "
                     "note/coin in cents."
                  << "\n";
        std::cout
            << "Please enter ctrl-D or enter on a new line to Cancel this "
               "purchase."
            << "\n";
        prompt = false;
      } else {
        std::cout << "Item not found. Please check the food ID and try again: ";
        std::cin.clear();
      }
    }
  }

  // Buying phase
  // Store all coins paid by the customer so we can delete they later if the
  // customer decides to exit
  std::vector<int> payings = {};
  std::vector<int> changes = {};
  // A bool representing that user fully paid for the item
  bool reachedToRegister = false;

  while (run) {
    if (priceAsCents > 0) {
      // If the user hasn't fully paid for the item
      std::cout << "You still need to give us: $ " << this->color << (float)priceAsCents / 100 << RESET_COLOR
                << ": ";
    }

    std::string input = Helper::readInput();

    // Check if user wants to exit
    if (std::cin.eof() || input.empty()) {
      std::cout << "Cancel purchase" << "\n";
      std::cin.clear(); // Clear the error flags

      // When exit, remove the coins from the system
      // Sort the payings first
      Helper::sortIntVector(payings);
      std::cout << "Refund: ";
      for (int newCoin : payings) {
        for (Coin &coin : this->data->balance->balance) {
          if (coin.denom == newCoin) {
            coin.count -= 1;
            // Print out the refund
            if (coin.denom < 100) {
              std::cout << coin.denom << "c ";
            } else {
              std::cout << "$" << coin.denom / 100 << " ";
            }
          }
        }
      }

      run = false;
    } else if (Coin::isDenomination(input)) {
      int pay = std::stoi(input);
      priceAsCents -= pay;

      // Add new coins to the system
      Coin newCoin = Coin();
      newCoin.count = 1;
      newCoin.denom = Coin::intToDenomination(std::stoi(input));
      // Keep track of newly added coins so that when user cancle purchase, we
      // can remove them
      payings.push_back(newCoin.denom);
      this->data->balance->insert(newCoin);

      // Check if the register can afford to pay for the change
      int change = priceAsCents;
      bool canReturnChange = true;

      while (change < 0 && canReturnChange) {
        Coin *coin = this->data->balance->getMaxDenomForValue(-change);
        if (!coin) {
          canReturnChange = false;
        } else {
          change += coin->denom;
        }
      }

      if (!canReturnChange) {
        // Force the user to pay for the item again and remove the previous
        // paid Coin from the system
        priceAsCents += payings.back();
        this->data->balance->getDenom(payings.back())->count--;

        std::cout << "The register doesn't have enough coins for change. "
                     "Please try a different denom"
                  << "\n";
      }
    } else {
      std::cout << "Error: invalid denomination encountered" << "\n";
      std::cin.clear();
    }

    if (priceAsCents < 0) {
      std::cout << "Your change is ";
      reachedToRegister = true;
    }

    while (priceAsCents < 0) {
      Coin *maxCoinPtr =
          this->data->balance->getMaxDenomForValue(-priceAsCents);

      // Decrement the coin count and get change
      if (maxCoinPtr) {
        maxCoinPtr->count--;
        priceAsCents += maxCoinPtr->denom;
        changes.push_back(maxCoinPtr->denom);
      }
    }

    // Exit out of the while loop once the system fully paid for the change
    if (priceAsCents == 0) {
      run = false;
    }
  }
  // If reached to the end, remove the number of item by 1 and output the
  // changes
  if (reachedToRegister) {
    // Decrement the amount
    meal->data->on_hand--;
    // Sort the changes first
    Helper::sortIntVector(changes);

    for (int change : changes) {
      // Print the denomination
      if (change < 100) {
        std::cout << this->color << change << RESET_COLOR << "c ";
      } else {
        std::cout << "$" << this->color << change / 100 << RESET_COLOR << " ";
      }
    }
  }
}

void Machine::displayMeals() {
  LinkedList *currentMealGroup = this->data->meals_group->getFirst();
  while (currentMealGroup != nullptr) {
    currentMealGroup->sortByAlpha();
    Node *currentMeal = currentMealGroup->getFirst();
    // Get the maximum size of PRICELEN
    int longestInteger =
        Helper::getLongestIntegerPart(currentMealGroup->getPrices());
    int pricelen = longestInteger + 3;
    //  Display first row
    for (int i = 0; i < (IDLEN + NAMELEN + pricelen + 3 + SEPARATOR_NUM)/2 - 3; i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << currentMealGroup->name << "\n";
    std::cout << "ID";
    for (int i = 0; i < IDLEN - 2;
        i++) { // Accounting for the length of string ID
      std::cout << EMPTY_SPACE;
    }
    std::cout << SEPARATOR << "Name";
    for (int i = 0; i < NAMELEN - 4; i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << SEPARATOR << "Length";
    for (int i = 0; i < pricelen - 6; i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << "\n";
    // Display separator line
    for (int i = 0; i < IDLEN + NAMELEN + pricelen + 3 + SEPARATOR_NUM; i++) {
      std::cout << LINE;
    }
    std::cout << "\n";
    while (currentMeal != nullptr) {
      std::cout << currentMeal->data->id << SEPARATOR << currentMeal->data->name;
      for (int i = 0; i < NAMELEN - (int)currentMeal->data->name.size(); i++) {
        std::cout << EMPTY_SPACE;
      }
      std::cout << SEPARATOR << MONEY_SYMBOL;

      std::cout << Helper::formatFloatToString(currentMeal->data->price.value(),
                                              longestInteger)
                << "\n";
      currentMeal = currentMeal->next;
    }
    currentMealGroup = currentMealGroup->next;
  }
}

void Machine::displayBalance() {
  // Get maximum quantity and value length
  int quantlen =
      Helper::floatToString(this->data->balance->getMaxValue(), 0).size();
  int valuelen =
      Helper::floatToString(this->data->balance->getMaxValue(), 2).size();

  if (quantlen < DEFAULT_QUANTITY_LENGTH) {
    quantlen = DEFAULT_QUANTITY_LENGTH;
  }

  if (valuelen < DEFAULT_VALUE_LENGTH) {
    valuelen = DEFAULT_VALUE_LENGTH;
  }

  // Display the first row
  std::cout << "Denom  " << SEPARATOR;

  std::cout << EMPTY_SPACE << "Quantity";
  for (int i = 0; i < quantlen - 8; i++) {
    std::cout << EMPTY_SPACE;
  }
  std::cout << SEPARATOR << EMPTY_SPACE << "Value";

  for (int i = 0; i < valuelen - 5; i++) {
    std::cout << EMPTY_SPACE;
  }
  std::cout << "\n";

  // Display separator line
  for (int i = 0;
       i < DENOM_LENGTH + quantlen + valuelen + SEPARATOR_NUM + EXCESS_LINE;
       i++) {
    std::cout << LINE;
  }
  std::cout << "\n";

  for (Coin coin : this->data->balance->balance) {
    std::cout << coin.denom;
    for (int i = 0; i < DENOM_LENGTH - (int)std::to_string(coin.denom).size();
         i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << SEPARATOR << EMPTY_SPACE << coin.count;

    for (int i = 0; i < quantlen - (int)std::to_string(coin.count).size();
         i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << SEPARATOR << MONEY_SYMBOL << EMPTY_SPACE;

    for (int i = 0;
         i < valuelen - (int)Helper::floatToString(coin.getTotal(), 2).size();
         i++) {
      std::cout << EMPTY_SPACE;
    }
    std::cout << Helper::floatToString(coin.getTotal(), 2) << "\n";
  }

  // Display separator line
  for (int i = 0;
       i < DENOM_LENGTH + quantlen + valuelen + SEPARATOR_NUM + EXCESS_LINE;
       i++) {
    std::cout << LINE;
  }
  std::cout << "\n";

  // Display total value
  // The additional 1 is accounting for the extra EMPTY_SPACE
  for (int i = 0; i < DENOM_LENGTH + quantlen + SEPARATOR_NUM + 1; i++) {
    std::cout << EMPTY_SPACE;
  }
  std::cout << MONEY_SYMBOL << EMPTY_SPACE;

  float total = this->data->balance->getTotalValue();
  for (int i = 0; i < valuelen - (int)Helper::floatToString(total, 2).size();
       i++) {
    std::cout << EMPTY_SPACE;
  }
  std::cout << Helper::floatToString(total, 2) << "\n";
}

void Machine::addFood() {
  std::string mealID = FoodItem::constructID(this->data->meals_group->getNextId());
  
  std::string itemCat = "";
  std::string itemName = "";
  std::string itemDesc = "";
  std::string itemPrice = "";

  // booleans to manage the flow
  bool cat = false;
  bool name = false;
  bool desc = false;
  bool price = false;
  bool success = false;

  if (mealID.size() == IDLEN) {
    cat = true;
    std::cout << "This new meal item will have the Item ID of " << mealID
              << ".\n";
  }
  while (cat) {
    std::cout << "Enter the item category: ";
    itemCat = Helper::readInput();
    if (std::cin.eof()) {
      std::cout << "Cancel add" << "\n";
      std::cin.clear();
      cat = false;
    }

    if (cat && !FoodItem::isValidName(itemCat)) {
      std::cout << "Invalid name" << "\n";
    } else if (cat && FoodItem::isValidName(itemCat)) {
      cat = false;
      name = true;
    }
  }

  while (name) {
    std::cout << "Enter the item name: ";
    itemName = Helper::readInput();
    if (std::cin.eof()) {
      std::cout << "Cancel add" << "\n";
      std::cin.clear();
      name = false;
    }

    if (name && !FoodItem::isValidName(itemName)) {
      std::cout << "Invalid name" << "\n";
    } else if (name && FoodItem::isValidName(itemName)) {
      name = false;
      desc = true;
    }
  }

  while (desc) {
    std::cout << "Enter the item description: ";
    itemDesc = Helper::readInput();

    if (std::cin.eof()) {
      std::cout << "Cancel add" << "\n";
      std::cin.clear();
      desc = false;
    }

    if (desc && !FoodItem::isValidDesc(itemDesc)) {
      std::cout << "Invalid description" << "\n";
    } else if (desc && FoodItem::isValidDesc(itemDesc)) {
      desc = false;
      price = true;
    }
  }

  while (price) {
    std::cout << "Enter the price for this item (in cents): ";
    itemPrice = Helper::readInput();

    if (std::cin.eof()) {
      std::cout << "Cancel add" << "\n";
      std::cin.clear();
      price = false;
    }

    if (price && !Price::isValidPrice(itemPrice)) {
      std::cout << "Invalid price" << "\n";
    } else if (price && Price::isValidPrice(itemPrice)) {
      price = false;
      success = true;
    }
  }

  // Create FoodItem obj and assign it to LinkedList
  if (success) {
    std::vector<std::string> prices = {};
    Helper::splitString(itemPrice, prices, ".");
    Price price = Price();
    price.dollars = std::stoi(prices[0]);
    price.cents = std::stoi(prices[1]);

    FoodItem *newMeal = new FoodItem(mealID, itemCat, itemName, itemDesc, price);
    Node *newNode = new Node();
    newNode->data = newMeal;
    LinkedList *currentMealGroup = this->data->meals_group->getFirst();
    bool append = false;
    while (!append) {
      if (currentMealGroup != nullptr) {
        if (itemCat == currentMealGroup->name) {
          currentMealGroup->append(newNode);
          append = true;
        } else {
          currentMealGroup = currentMealGroup->next;
        }
      } else {
        LinkedList *newCat = new LinkedList();
        newCat->name = itemCat;
        this->data->meals_group->append(newCat);
        newCat->append(newNode);
        append = true;
      }
    }
    
    std::cout << "This item \"" << itemName << " - " << itemDesc << "\""
              << "has now been added to the food menu" << "\n";
  }
}

void Machine::removeFood() {
  std::string input = Helper::readInput();

  if (input.empty()) {
    std::cout << "Cancel delete" << "\n";
  } else {
    LinkedList *currentMealGroup = this->data->meals_group->getFirst();
    bool found = false;
    Node *meal = nullptr;
    while (!found && currentMealGroup != nullptr) {
      if (currentMealGroup->getById(input)) {
        meal = currentMealGroup->getById(input);
        std::cout << "\"" << meal->data->id << " - " << meal->data->name << " - "
                << meal->data->description << "\""
                << "has been removed from the system" << "\n";
        currentMealGroup->remove(input);
        found = true;
      } else {
        currentMealGroup = currentMealGroup->next;
      }
    }
    if (found == false) {
      std::cout << "Meal not found";
    }
  }
}

void Machine::save() {}
