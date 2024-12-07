#define _CRT_SECURE_NO_WARNINGS //FOR CHRONO

//LIBRARIES
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <regex>
#include <fstream>
#include <sstream>


//NAMESPACES
using namespace std;
using namespace chrono;

//GLOBAL VARIABLES
const string bankName = "JPMC";
const string userFile = "usersFile";

//======================================== STRUCTURES ========================================
//SINGLY LINKED LISTS
struct transaction { //HOLDS ALL TRANSACTIONS
	string date;
	double amount;
	transaction* next;
};

struct account { //HOLDS ALL ACOUNTS
	string IBAN;
	string accountName;
	double balance;
	string currency; //ADOPTED CURRENCIES: USD, EUR, GBP, YEN
	double limitDepositPerDay;
	double depositTrack; //ADDED FOR BETTER DEPOSIT TRACKING
	double limitWithdrawPerMonth;
	double withdrawTrack; //ADDED FOR BETTER WITHDRAWAL TRACKING
	string lastDailyDeposit; //ADDED FOR DAILY DEPOSIT DATE UPDATING
	string lastMonthlyWithdraw; //ADDED FOR MONTHLY WITHDRAWAL DATE UPDATING
	transaction* txn; //POINTER TO THE USER'S TRANSACTIONS
	account* next;
};

//DOUBLY LINKED LISTS
struct user { //HOLDS ALL USERS (a user can have multiple accounts)
	int userID;
	string fname;
	string lname;
	account* acct;
	user* next, * previous;
};

struct userList {
	user* head, * tail;
};


//====================================== FUNCTION PROTOTYPES ======================================
//======================================== USER FUNCTIONS =========================================
//FUNCTION TO STORE USER ID AND KEEP TRACK OF IT IN A TEXT FILE
int generateUserID() {
	static int userID = 0;

	// read the last used ID from the file
	ifstream infile("userID.txt");
	if (infile) {
		infile >> userID;
	}
	infile.close();

	// increment the userID
	userID++;

	// write the new userID back to the file
	ofstream outfile("userID.txt");
	if (outfile) {
		outfile << userID;
	}
	outfile.close();

	return userID;
}

//FUNCTION TO CHECK IF USER ID EXISTS
bool IDexists(userList *ul, int ID){
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        if (currentUser->userID == ID) return true;
        currentUser = currentUser->next;
	}
    return false;
}

//====================================== ACCOUNT FUNCTIONS ========================================
//FUNCTION FOR ACCOUNT TYPE
string findAccountType(account acct) {
    if (acct.currency == "USD") return "domiciled";
    else return "international";
}


//======================================== FILE FUNCTIONS =========================================
//===========
//QUESTION 6 
//===========
//FUNCTION TO WRITE TO FILES
void writeToFile(userList ul) {
    ofstream file(userFile);
    if (!file) {
        cout << "Error: File could not be opened." << endl;
        return;
    }
    user* currentUser = ul.head;
    while (currentUser != NULL) {
        file << "-" << currentUser->userID << "," << currentUser->fname << "," << currentUser->lname << endl;

        account* currentAccount = currentUser->acct;
        while (currentAccount != NULL) {
            file << "#" << currentAccount->IBAN << "," << currentAccount->accountName << "," << currentAccount->balance << currentAccount->currency << "," << currentAccount->limitDepositPerDay << currentAccount->currency << "," << currentAccount->limitWithdrawPerMonth << currentAccount->currency << "," << currentAccount->depositTrack << "," << currentAccount->withdrawTrack << "," << currentAccount->lastDailyDeposit << "," << currentAccount->lastMonthlyWithdraw << endl;

            transaction* currentTransaction = currentAccount->txn;
            while (currentTransaction != NULL) {
                file << "*" << currentTransaction->date << "," << currentTransaction->amount << currentAccount->currency << endl;
                currentTransaction = currentTransaction->next;
            }
            currentAccount = currentAccount->next;
        }
        currentUser = currentUser->next;
    }
    file.close();
    cout << "Data saved successfully!" << endl;
    return;
}

//===========
//QUESTION 1
//===========
//FUNCTION TO READ FROM FILES
void loadDataFromFile(userList* uList) {
    ifstream file(userFile);
    if (!file) {
        cout << "Error: File could not be opened." << endl;
        return;
    }

    string line;
    user* currentUser = nullptr;
    account* currentAccount = nullptr;

    while (getline(file, line)) {
        if (line.empty()) continue;

        // If the line starts with "-", it's a user
        if (line[0] == '-') {
            if (currentUser != nullptr) {
                if (uList->tail == nullptr) {
                    uList->head = currentUser;
                }
                else {
                    uList->tail->next = currentUser;
                    currentUser->previous = uList->tail;
                }
                uList->tail = currentUser;
            }

            // Parse user data inline
            stringstream ss(line.substr(1)); // Remove the '-' for easier parsing
            currentUser = new user();
            string userIDStr;

            getline(ss, userIDStr, ',');
            currentUser->userID = stoi(userIDStr);
            getline(ss, currentUser->fname, ',');
            getline(ss, currentUser->lname);

            currentUser->acct = nullptr;
            currentUser->next = nullptr;
            currentUser->previous = nullptr;
        }

        // If the line starts with "#", it's an account
        else if (line[0] == '#') {
            if (currentUser != nullptr) {
                stringstream ss(line.substr(1)); // Remove the '#' for easier parsing
                account* acct = new account();
                string balanceStr, depositLimitStr, withdrawLimitStr, depositTrackStr, withdrawTrackStr;

                getline(ss, acct->IBAN, ',');
                getline(ss, acct->accountName, ',');
                getline(ss, balanceStr, ',');
                acct->balance = stod(balanceStr.substr(0, balanceStr.size() - 3)); // Remove currency
                acct->currency = balanceStr.substr(balanceStr.size() - 3);       // Extract currency

                getline(ss, depositLimitStr, ',');
                acct->limitDepositPerDay = stod(depositLimitStr.substr(0, depositLimitStr.size() - 3));
                getline(ss, withdrawLimitStr, ',');
                acct->limitWithdrawPerMonth = stod(withdrawLimitStr.substr(0, withdrawLimitStr.size() - 3));

                getline(ss, depositTrackStr, ',');
                acct->depositTrack = stod(depositTrackStr.empty() ? "0" : depositTrackStr);

                getline(ss, withdrawTrackStr, ',');
                acct->withdrawTrack = stod(withdrawTrackStr.empty() ? "0" : withdrawTrackStr);

                getline(ss, acct->lastDailyDeposit, ',');
                getline(ss, acct->lastMonthlyWithdraw);

                acct->txn = nullptr;
                acct->next = nullptr;

                if (currentUser->acct == nullptr) {
                    currentUser->acct = acct;
                }
                else {
                    account* temp = currentUser->acct;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = acct;
                }
                currentAccount = acct;
            }
        }

        // If the line starts with "*", it's a transaction
        else if (line[0] == '*') {
            if (currentAccount != nullptr) {
                stringstream ss(line.substr(1)); // Remove the '*' for easier parsing
                transaction* txn = new transaction();
                string amountStr;

                getline(ss, txn->date, ',');
                getline(ss, amountStr);
                txn->amount = stod(amountStr.substr(0, amountStr.size() - 3)); // Remove currency

                txn->next = nullptr;

                if (currentAccount->txn == nullptr) {
                    currentAccount->txn = txn;
                }
                else {
                    transaction* temp = currentAccount->txn;
                    while (temp->next != nullptr) {
                        temp = temp->next;
                    }
                    temp->next = txn;
                }
            }
        }
    }

    // Ensure the last user is added to the user list
    if (currentUser != nullptr) {
        if (uList->tail == nullptr) {
            uList->head = currentUser;
        }
        else {
            uList->tail->next = currentUser;
            currentUser->previous = uList->tail;
        }
        uList->tail = currentUser;
    }

    file.close();
    cout << "Data loaded successfully!" << endl;
}



//======================================== IBAN FUNCTIONS =========================================
//FUNCTION TO GENERATE UNIQUE IBAN IDENTIFIER
int generateIBANidentifier() {
    static int IBAN = 9;

    // read the last used ID from the file
    ifstream infile("IBAN.txt");
    if (infile) {
        infile >> IBAN;
    }
    infile.close();

    // increment the userID
    IBAN++;

    // write the new userID back to the file
    ofstream outfile("IBAN.txt");
    if (outfile) {
        outfile << IBAN;
    }
    outfile.close();

    return IBAN;
}

//FUNCTION TO GENERATE UNIQUE IBAN
string generateIBAN(account acct) {
    const string countryCodes[] = { "JP", "US", "FR", "GB" };
    int IBANidentifier = generateIBANidentifier();
    if (acct.currency == "USD") return countryCodes[1] + to_string(IBANidentifier) + bankName;
    else if (acct.currency == "EUR") return countryCodes[2] + to_string(IBANidentifier) + bankName;
    else if (acct.currency == "GBP") return countryCodes[3] + to_string(IBANidentifier) + bankName;
    else return countryCodes[0] + to_string(IBANidentifier) + bankName;
}

//FUNCTION TO VALIDATE IBAN INPUT
bool IBANexists(userList* ul, string IBAN) {
    user* temp = ul->head;
    while (temp != NULL) {
        account* temp1 = temp->acct;
        while (temp1 != NULL) {
            if (temp1->IBAN == IBAN) {
                return true;
            }
            temp1 = temp1->next;
        }
        temp = temp->next;
    }
    return false;
}

//FUNCTION TO GIVE AN ACCOUNT AN IBAN AND ACCOUNT TYPE IF IT DOESN'T EXIST
void giveIBAN_Type(account* acct) {
    acct->IBAN = generateIBAN(*acct);
    acct->accountName = findAccountType(*acct);
}


//====================================== CURRENCY FUNCTIONS =======================================
//FUNCTION FOR CURRENCY CONVERSION
double currencyConvert(account acct1, account acct2, double amount) {
    if (acct1.currency == acct2.currency) {
        return amount;
    }
    if (acct2.currency != "EUR" && acct2.currency != "USD" && acct2.currency != "GBP" && acct2.currency != "YEN") {
        return -1;
    }

    // USD conversion rates
    const double USD_to_EUR = 0.92;
    const double USD_to_GBP = 0.78;
    const double USD_to_YEN = 146.30;

    // EUR conversion rates
    const double EUR_to_USD = 1.09;
    const double EUR_to_GBP = 0.85;
    const double EUR_to_YEN = 159.20;

    // GBP conversion rates
    const double GBP_to_USD = 1.28;
    const double GBP_to_EUR = 1.18;
    const double GBP_to_YEN = 187.10;

    // YEN conversion rates
    const double YEN_to_USD = 0.0068;
    const double YEN_to_EUR = 0.0063;
    const double YEN_to_GBP = 0.0053;

    if (acct2.currency == "USD") {
        if (acct1.currency == "EUR") return amount * EUR_to_USD;
        else if (acct1.currency == "GBP") return amount * GBP_to_USD;
        else return amount * YEN_to_USD;
    }
    else if (acct2.currency == "EUR") {
        if (acct1.currency == "USD") return amount * USD_to_EUR;
        else if (acct1.currency == "GBP") return amount * GBP_to_EUR;
        else return amount * YEN_to_EUR;
    }
    else if (acct2.currency == "GBP") {
        if (acct1.currency == "USD") return amount * USD_to_GBP;
        else if (acct1.currency == "EUR") return amount * EUR_to_GBP;
        else return amount * YEN_to_GBP;
    }
    else if (acct2.currency == "YEN") {
        if (acct1.currency == "USD") return amount * USD_to_YEN;
        else if (acct1.currency == "EUR") return amount * EUR_to_YEN;
        else return amount * GBP_to_YEN;
    }

    return -1;
}

//FUNCTION FOR CURRENCY VALIDATION
bool isValidCurrency(string currency) {
    if (currency != "USD" && currency != "EUR" && currency != "GBP" && currency != "YEN") {
        return false;
    }
    return true;
}


//======================================= MONEY FUNCTIONS =========================================
//FUNCTION TO VALIDATE THE AMOUNT
bool isValidAmount(double amount) {
    if (amount <= 0) return false;
    return true;
}


//======================================== DATE FUNCTIONS =========================================
//FUNCTION TO GET THE CURRENT DATE
string getCurrentDateTime() {
    system_clock::time_point now = system_clock::now(); //get the current time
    time_t currentTime = system_clock::to_time_t(now); //convert to time_t (C style time - ctime)
    struct tm* localTime = localtime(&currentTime); //convert to local time
    stringstream ss;
    ss << put_time(localTime, "%d/%m/%y %H:%M:%S"); //format the time
    return ss.str();
}

//FUNCTION TO VALIDATE THE DATE
bool isValidDate(int day, int month, int year) {
    auto now = system_clock::now();
    time_t currentTime = system_clock::to_time_t(now);
    tm localTime = *localtime(&currentTime);

    int currentYear = localTime.tm_year + 1900;
    int currentMonth = localTime.tm_mon + 1;
    int currentDay = localTime.tm_mday;

    if (year > currentYear) {
        cout << "Error: Year cannot be in the future." << endl;
        return false;
    }

    if (month < 1 || month > 12) {
        cout << "Error: Month must be between 1 and 12." << endl;
        return false;
    }

    int daysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
        daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1]) {
        cout << "Error: Invalid day for the given month and year." << endl;
        return false;
    }

    if (year == currentYear) {
        if (month > currentMonth) {
            cout << "Error: Month cannot be in the future." << endl;
            return false;
        }
        if (month == currentMonth && day > currentDay) {
            cout << "Error: Day cannot be in the future." << endl;
            return false;
        }
    }

    return true;
}

//FUNCTION TO GET DATE FROM USER
string getDateFromUser() {
    int day, month, year;

    while (true) {
        cin.ignore();
        cout << "Enter a date (DD/MM/YY): ";
        string input;
        getline(cin, input);

        regex datePattern(R"(^(\d{2})/(\d{2})/(\d{2})$)");
        smatch match;

        if (regex_match(input, match, datePattern)) {
            day = stoi(match[1].str());
            month = stoi(match[2].str());
            year = stoi(match[3].str()) + 2000;

            if (isValidDate(day, month, year)) {
                return input; // Return the input as-is since it's already in the desired format
            }
        }
        else {
            cout << "Error: Please enter the date in the format DD/MM/YY." << endl;
        }
    }
}


//====================================== BALANCE FUNCTIONS ========================================
// FUNCTION TO CHECK IF ACCOUNT HAS ENOUGH BALANCE
bool hasEnoughBalance(account* acct, double amount) {
    if (acct->balance < amount) return false;
    return true;
}

// FUNCTION TO CHECK IF IT EXCEEDS THE DAILY DEPOSIT LIMIT
bool exceedsDailyDepositLimit(account* acct, double amount) {
    if (amount < acct->balance && amount + acct->depositTrack > acct->limitDepositPerDay) return true;
    return false;
}

// FUNCTION TO CHECK IF IT EXCEEDS THE MONTHLY WITHDRAWAL LIMIT
bool exceedsMonthlyWithdrawalLimit(account* acct, double amount) {
    if (amount>acct->balance && amount + acct->withdrawTrack > acct->limitWithdrawPerMonth) return true;
    return false;
}


// FUNCTION TO RESET DAILY DEPOSIT
void resetDailyDeposit(account* acct) {
    string currentDate = getCurrentDateTime();
    if (currentDate != acct->lastDailyDeposit) {
        acct->depositTrack = 0;
        acct->lastDailyDeposit = currentDate;
    }
}

// FUNCTION TO RESET MONTHLY WITHDRAWAL
void resetMonthlyWithdrawal(account& acct) {
    if (acct.lastMonthlyWithdraw.empty()) {
        return;
    }

    string currentDateTime = getCurrentDateTime();
    string currentMonthYear = currentDateTime.substr(3, 5);
    string lastMonthYear = acct.lastMonthlyWithdraw.substr(3, 5);

    if (currentMonthYear != lastMonthYear) {
        acct.withdrawTrack = 0;
        acct.lastMonthlyWithdraw = currentDateTime;
    }
}

// FUNCTION TO DECREASE BALANCE
void decreaseBalance(account* acct, double amount) {
    acct->balance -= amount;
}

// FUNCTION TO INCREASE BALANCE
void increaseBalance(account* acct, double amount) {
    acct->balance += amount;
}


//======================================== LIST FUNCTIONS =========================================
//================================== TRANSACTION LIST FUNCTIONS ===================================
//FUNCTION TO DISPLAY TRANSACTIONS
void displayTransactions(transaction* currentTransaction) {
    if (currentTransaction == NULL) {
        cout << "No transactions available for this account." << endl;
        return;
    }

    int j = 1;
    while (currentTransaction != NULL) {
        cout << "Transaction " << j++ << ": " << endl;
        cout << "Date: " << currentTransaction->date << endl;
        cout << "Amount: " << currentTransaction->amount << endl;
        cout << "-------------------" << endl;
        currentTransaction = currentTransaction->next;
    }
}

// FUNCTION TO RECORD NEGATIVE TRANSACTION
void addNegTransaction(account* acct, double amount) {
    transaction* newTransaction = new transaction;
    newTransaction->date = getCurrentDateTime();
    newTransaction->amount = -amount;
    newTransaction->next = NULL;

    if (acct->txn == NULL) {
        acct->txn = newTransaction;
    }
    else {
        transaction* temp = acct->txn;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newTransaction;
    }
}

// FUNCTION TO RECORD POSITIVE TRANSACTION
void addPosTransaction(account* acct, double amount) {
    transaction* newTransaction = new transaction;
    newTransaction->date = getCurrentDateTime();
    newTransaction->amount = amount;
    newTransaction->next = NULL;

    if (acct->txn == NULL) {
        acct->txn = newTransaction;
    }
    else {
        transaction* temp = acct->txn;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newTransaction;
    }
}

//===========
//QUESTION 2 
//===========
//FUNCTION FOR MONEY TRANSFER
void transferMoney(userList* ul, double amount, account* acct1, account* acct2) {
    if (acct1 == NULL || acct2 == NULL) {
        cout << "Error: Invalid account(s)." << endl;
        return;
    }

    // Deduct amount from source account
    addNegTransaction(acct1, amount);
    decreaseBalance(acct1, amount);

    // Add amount to target account
    addPosTransaction(acct2, amount);
    increaseBalance(acct2, amount);

    // Update daily and monthly limits
    acct1->withdrawTrack += amount;
    acct1->lastMonthlyWithdraw = getCurrentDateTime();
    acct2->depositTrack += amount;
    acct2->lastDailyDeposit = getCurrentDateTime();

    cout << "Transaction complete: " << amount << " transferred from "
        << acct1->IBAN << " to " << acct2->IBAN << "." << endl;
}

//FUNCTION TO DELETE TRANSACTIONS
void deleteTransactions(userList* ul, string date) {
    user *currentUser = ul->head;
    bool transactionsDeleted = false;
    while (currentUser != NULL) {
        account* currentAccount = currentUser->acct;
        while (currentAccount != NULL) {
            transaction* currentTransaction = currentAccount->txn;
            transaction* previousTransaction = NULL;
            while (currentTransaction != NULL) {
                if (currentTransaction->date < date) {
                    if (previousTransaction == NULL) {
                        currentAccount->txn = currentTransaction->next;
                    }
                    else
                        previousTransaction->next=currentTransaction->next;

                    transaction* temp = currentTransaction;
                    currentTransaction = currentTransaction->next;
                    delete temp;
                    transactionsDeleted = true;
                }
                else {
                    previousTransaction = currentTransaction;
                    currentTransaction = currentTransaction->next;
                }
            }
            currentAccount = currentAccount->next;
        }
        currentUser = currentUser->next;
    }
    if (transactionsDeleted) {
		cout << "Transactions deleted successfully!" << endl;
	}
	else {
		cout << "No transactions found before the given date." << endl;
	}
}

//FUNCTIONS NEEDED TO SORT TRANS BY DATE
//FUNCTION TO GET LINKED LIST'S CENTER
transaction* getCenter(transaction* start) { //One moves 2x faster than the other so when the fast one reaches the end , the slower one is at the middle
    if (start == NULL || start->next == NULL) return start;
    transaction* slow = start;
    transaction* fast = start->next;
    while (fast != NULL && fast->next != NULL) {
        slow = slow->next;
        fast = fast->next->next;
    }
    transaction* center = slow->next;
    slow->next = NULL;
    return center;
}

//FUNCTION TO MERGE TWO LINKED LISTS
transaction* merge(transaction* left, transaction* right) {
    if (left == NULL) return right;
    if (right == NULL) return left;
    if (left->date <= right->date) {
        left->next = merge(left->next, right);
        return left;
    }
    else {
        right->next = merge(left, right->next);
        return right;
    }
}

//MERGE SORT FUNCTION
transaction* mergeSortTransactions(transaction* start) {
    if (start == NULL || start->next == NULL)
        return start;

    transaction* center = getCenter(start);

    transaction* left = mergeSortTransactions(start);
    transaction* right = mergeSortTransactions(center);

    return merge(left, right);
}


//==================================== ACCOUNT LIST FUNCTIONS =====================================
//FUNCTION TO DISPLAY ACCOUNT INFORMATION
void displayAccountInfo(account* currentAccount) {
    if (currentAccount == NULL) {
        cout << "No accounts available for this user." << endl;
        return;
    }

    int i = 1;
    while (currentAccount != NULL) {
        cout<<endl;
        cout << "Account " << i++ << ": " << endl;
        cout << "IBAN: " << currentAccount->IBAN << endl;
        cout << "Type: " << currentAccount->accountName << endl;
        cout << "Balance: " << currentAccount->balance << currentAccount->currency << endl;
        cout << "Daily Deposit Limit: " << currentAccount->limitDepositPerDay << currentAccount->currency << endl;
        cout << "Monthly Withdrawal Limit: " << currentAccount->limitWithdrawPerMonth << currentAccount->currency << endl;
        cout << "Remaining Daily Deposit Allowance: "
            << currentAccount->limitDepositPerDay - currentAccount->depositTrack
            << currentAccount->currency << endl;
        cout << "Remaining Monthly Withdrawal Allowance: "
            << currentAccount->limitWithdrawPerMonth - currentAccount->withdrawTrack
            << currentAccount->currency << endl;
        cout << "Last Daily Deposit: " << currentAccount->lastDailyDeposit << endl;
        cout << "Last Monthly Withdrawal: " << currentAccount->lastMonthlyWithdraw << endl;
        cout << "-------------------" << endl;

        displayTransactions(currentAccount->txn);

        currentAccount = currentAccount->next;
    }
}

//FUNCTION TO CHECK IF AN ACCOUNT EXISTS ALREADY
bool accountExists(user* usr, account* acct) {
    if (usr->acct == NULL) {
        return false;
    }
    account* temp = usr->acct;
    while (temp != NULL) {
        if (temp->currency == acct->currency && temp->limitDepositPerDay == acct->limitDepositPerDay && temp->limitWithdrawPerMonth == acct->limitWithdrawPerMonth)
            return true;
        temp = temp->next;
    }
    return false;
}

//FUNCTION TO CREATE AN ACCOUNT AND ALLOW FOR MODIFICATIONS
account generateAccount() {
    account acct;
    cout << "Enter the account's preferred currency (USD - EUR - GBP - YEN): ";
    cin >> acct.currency;
    while (!isValidCurrency(acct.currency)) {
        cout << "Invalid currency. Please enter a valid currency: ";
        cin >> acct.currency;
    }

    cout << "Enter the amount you would like to deposit: ";
    cin >> acct.balance;
    while (!isValidAmount(acct.balance)) {
        cout << "Invalid amount. Please enter a valid amount: ";
        cin >> acct.balance;
    }

    cout << "Enter the daily deposit limit: ";
    cin >> acct.limitDepositPerDay;
    while (!isValidAmount(acct.limitDepositPerDay)) {
        cout << "Invalid amount. Please enter a valid amount: ";
        cin >> acct.limitDepositPerDay;
    }

    cout << "Enter the monthly withdrawal limit: ";
    cin >> acct.limitWithdrawPerMonth;
    while (!isValidAmount(acct.limitWithdrawPerMonth)) {
        cout << "Invalid amount. Please enter a valid amount: ";
        cin >> acct.limitWithdrawPerMonth;
    }

    bool done = false;
    while (!done) {
        cout << "\nAccount Information:\n";
        cout << "1. Currency: " << acct.currency << endl;
        cout << "2. Balance: " << acct.balance << endl;
        cout << "3. Daily Deposit Limit: " << acct.limitDepositPerDay << endl;
        cout << "4. Monthly Withdrawal Limit: " << acct.limitWithdrawPerMonth << endl;

        cout << "\nWould you like to modify any of the details? (Y/N): ";
        char choice;
        cin >> choice;

        if (choice == 'Y' || choice == 'y') {
            cout << "\nEnter the number of the item you want to modify (1-4): ";
            int modifyChoice;
            cin >> modifyChoice;

            switch (modifyChoice) {
            case 1:
                cout << "Enter the new currency: ";
                cin >> acct.currency;
                while (!isValidCurrency(acct.currency)) {
                    cout << "Invalid currency. Please enter a valid currency: ";
                    cin >> acct.currency;
                }
                break;
            case 2:
                cout << "Enter the new balance: ";
                cin >> acct.balance;
                while (!isValidAmount(acct.balance)) {
                    cout << "Invalid amount. Please enter a valid amount: ";
                    cin >> acct.balance;
                }
                break;
            case 3:
                cout << "Enter the new daily deposit limit: ";
                cin >> acct.limitDepositPerDay;
                while (!isValidAmount(acct.limitDepositPerDay)) {
                    cout << "Invalid amount. Please enter a valid amount: ";
                    cin >> acct.limitDepositPerDay;
                }
                break;
            case 4:
                cout << "Enter the new monthly withdrawal limit: ";
                cin >> acct.limitWithdrawPerMonth;
                while (!isValidAmount(acct.limitWithdrawPerMonth)) {
                    cout << "Invalid amount. Please enter a valid amount: ";
                    cin >> acct.limitWithdrawPerMonth;
                }
                break;
            default:
                cout << "Invalid choice. Please select a number between 1 and 4." << endl;
                continue;
            }

            cout << "\nWould you like to modify something else? (Y/N): ";
            cin >> choice;
            if (choice == 'N' || choice == 'n') {
                done = true;
            }
        }
        else {
            done = true;
        }
    }
    acct.depositTrack = 0;
    acct.withdrawTrack = 0;
    acct.lastDailyDeposit = "";
    acct.lastMonthlyWithdraw = "";
    acct.txn = NULL;
    acct.next = NULL;
    return acct;
}

//FUNCTION TO LOCATE A USER ACCOUNT
account* locateAccount(userList* ul, string IBAN) {
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        account* currentAccount = currentUser->acct;
        while (currentAccount != NULL) {
            if (currentAccount->IBAN == IBAN) {
                return currentAccount;
            }
            currentAccount = currentAccount->next;
        }
        currentUser = currentUser->next;
    }
    return NULL; 
}

//===========
//QUESTION 4 
//===========
//FUNCTION TO CREATE ACCOUNTS
void createNewAccout(userList* ul, user usr, account acct) {
    user* currentAccount = ul->head;
    if (currentAccount == NULL) {
        cout<<"User list is empty!"<<endl;
        return;
    }

    while (currentAccount != NULL) {
        if (currentAccount->userID == usr.userID) {
            account* newCurrentAccount = currentAccount->acct;

            if (newCurrentAccount == NULL) {
                currentAccount->acct = new account;
                *currentAccount->acct = acct;
                giveIBAN_Type(currentAccount->acct);
                currentAccount->acct->next = NULL;
                cout << "Account created successfully!" << endl;
                return;
            }
            else {
                while (newCurrentAccount != NULL) {
                    if(accountExists(currentAccount, &acct)){
						cout << "Account already exists!" << endl;
						return;
                    }
                    if (newCurrentAccount->next == NULL) break;
                    newCurrentAccount = newCurrentAccount->next;
                }
                newCurrentAccount->next = new account;
                *(newCurrentAccount->next) = acct;
                giveIBAN_Type(newCurrentAccount->next);
                newCurrentAccount->next->next = NULL;
                cout << "Account created successfully!" << endl;
                return;
            }
        }
        currentAccount = currentAccount->next;
    }
    cout << "User not found. Please create the user first." << endl;
}

//FUNCTION TO CHECK IF USER HAS MORE THAN 2 ACCOUNTS
bool hasMoreThanTwoAccounts(userList* ul, int ID) {
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        if (currentUser->userID == ID) {
            int accountCount = 0;
            account* tempAccount = currentUser->acct;
            while (tempAccount != NULL) {
                accountCount++;
                tempAccount = tempAccount->next;
            }
            if (accountCount >= 2)
                return true;
        }
        currentUser = currentUser->next;
    }
    return false;
}

//======================================== USER FUNCTIONS =========================================
//FUNCTION TO DISPLAY USER INFORMATION
void displayUserInfo(user* currentUser) {
    if (currentUser == NULL) {
        cout << "Invalid user!" << endl;
        return;
    }

    cout << endl;
    cout << "User ID: " << currentUser->userID << endl;
    cout << "First Name: " << currentUser->fname << endl;
    cout << "Last Name: " << currentUser->lname << endl;
    cout << endl << endl;

    cout << "Accounts: " << endl;
    displayAccountInfo(currentUser->acct);
    cout << "----------------------" << endl;
}

//FUNCTION TO DISPLAY USER INFORMATION BY ID
void displayUserInfoID(userList* ul, int ID) {
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        if (currentUser->userID == ID) {
            cout << "User ID: " << currentUser->userID << endl;
            cout << "First Name: " << currentUser->fname << endl;
            cout << "Last Name: " << currentUser->lname << endl;
            cout << "----------------------" << endl;

            account* currentAccount = currentUser->acct;
            int i = 1;
            while (currentAccount != nullptr) {
                cout << "Account " << i++ << ": " << endl;
                
                cout << "Account IBAN: " << currentAccount->IBAN << endl;
                cout << "Account Name: " << currentAccount->accountName << endl;
                cout << "Balance: " << currentAccount->balance << currentAccount->currency << endl;
                cout << "Deposit Limit per Day: " << currentAccount->limitDepositPerDay << currentAccount->currency << endl;
                cout << "Withdraw Limit per Month: " << currentAccount->limitWithdrawPerMonth << currentAccount->currency << endl;
                cout << "Last Daily Deposit: " << currentAccount->lastDailyDeposit << endl;
                cout << "Last Monthly Withdrawal: " << currentAccount->lastMonthlyWithdraw << endl;
                cout << "----------------------" << endl;

                transaction* currentTransaction = currentAccount->txn;
                int j = 1;
                while (currentTransaction != nullptr) {
                    cout << "Transaction " << j++ << ": " << endl;
                    cout << "Transaction Date: " << currentTransaction->date << endl;
                    cout << "Transaction Amount: " << currentTransaction->amount << endl;
                    cout << "----------------------" << endl;
                    currentTransaction = currentTransaction->next;
                }
                currentAccount = currentAccount->next;
            }
            break;
        }
        currentUser = currentUser->next;
    }
}

//FUNCTION TO GET USER INFORMATION
user getUserInfo() {
    user usr;
    cout << "Enter your ID: ";
    cin >> usr.userID;
    cout << "Enter your first name: ";
    cin.ignore();
    getline(cin, usr.fname);
    cout << "Enter your last name: ";
    cin.ignore();
    getline(cin, usr.lname);
    return usr;
}

//FUNCTION TO GET A NEW USER'S INFORMATION
user getNewUserInfo() {
    user usr;
    cin.ignore();
    cout << "Enter your first name: ";
    getline(cin, usr.fname);
    cout << "\nEnter your last name: ";
    getline(cin, usr.lname);
    return usr;
}

//FUNCTION TO CHECK IF USER ALREADY EXISTS
bool userExists(userList* ul, user usr) {
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        if (currentUser->userID == usr.userID) {
            return true;
        }
        currentUser = currentUser->next;
    }
    return false;
}

//FUNCTION TO ADD A NEW USER
void addUser(userList* ul, user usr) {
    user* newUser = new user;
    *newUser = usr;
    newUser->acct = NULL;
    newUser->next = NULL;
    newUser->previous = NULL;

    if (ul->head == NULL) {
        ul->head = newUser;
        ul->tail = newUser;
    }
    else {
        ul->tail->next = newUser;
        newUser->previous = ul->tail;
        ul->tail = newUser;
    }
    cout<<"User added successfully!" << endl;
}

//FINAL FUNCTION FOR USER CREATION
void createUser(userList* ul) {
    cout << "===== User Creation ====" << endl;
    user usr = getNewUserInfo();
    if (userExists(ul, usr)) {
        cout << "User already exists." << endl;
        return;
    }
    usr.userID = generateUserID();
    cout << "Your ID is: " << usr.userID << endl;
    addUser(ul, usr);
    return;
}


//====================================== USER LIST FUNCTIONS ======================================
//FUNCTION TO INITIALIZE USER LIST IN MAIN
void initialiseUserList(userList* ul) {
    ul->head = NULL;
    ul->tail = NULL;
}

//FUNCTION TO DISPLAY USERS
void displayUserList(userList* ul) {
    if (ul->head == NULL) {
        cout << "No users found!" << endl;
        return;
    }

    user* currentUser = ul->head;
    while (currentUser != NULL) {
        displayUserInfo(currentUser);
        currentUser = currentUser->next;
    }
}


//====================================== FINALIZED FUNCTIONS ======================================
//FINAL FUNCTION FOR ACCOUNT CREATION
void accountCreation(userList* ul) {
    if (ul->head == NULL) {
        cout << "User list is empty. Please create a user first." << endl;
        return;
    }
    cout << "===== Account Creation ====" << endl << endl;
    cout << "==== User Information ====" << endl;
    user user1 = getUserInfo();
    cout << "===== Account Creation ====" << endl << endl;
    cout << "=== Account Information ===" << endl;
    account acct1 = generateAccount();
    createNewAccout(ul, user1, acct1);
    return;
}

// FINAL FUNCTION FOR TRANSACTIONS
void makeTransaction(userList* ul) {
    string IBAN1, IBAN2;
    double amount;
    int ID;
    cout << "===== Transaction =====" << endl << endl;
    cout << "Enter your ID: ";
    cin >> ID;
    while (ID < 1 || !IDexists(ul, ID)) {
        cout << "Error: Enter a valid ID: ";
        cin >> ID;
    }
    cout << endl;

    if (!hasMoreThanTwoAccounts(ul, ID)) {
        cout << "Error: You must have more than 2 accounts to make a transaction." << endl;
        return;
    }

    cout << "Your current accounts are: " << endl;
    displayUserInfoID(ul, ID);

    cout << "Enter the IBAN of the account you want to transfer from: ";
    cin >> IBAN1;
    while (!IBANexists(ul, IBAN1)) {
        cout << "Error: Enter valid IBAN." << endl;
        cin >> IBAN1;
    }

    account* acct = locateAccount(ul, IBAN1);

    cout << "Enter the IBAN of the account you want to transfer to: ";
    cin >> IBAN2;
    while (!IBANexists(ul, IBAN2) || IBAN2 == IBAN1) {
        cout << "Error: Enter valid IBAN." << endl;
        cin >> IBAN2;
    }

    account* acct1 = locateAccount(ul, IBAN2);


    cout << "Enter the amount you want to transfer: ";
    cin >> amount;

    if (!isValidAmount(amount)) {
        while (!isValidAmount(amount)) {
            cout << "Error: Enter a valid amount: ";
            cin >> amount;
        }
    }
    else if (!hasEnoughBalance(acct, amount)) {
        while (!hasEnoughBalance(acct, amount)) {
            cout << "Error: Amount exceeds your account's balance. Enter a valid amount: ";
            cin >> amount;
        }
    }
    else if (exceedsMonthlyWithdrawalLimit(acct, amount)) {
        while (exceedsMonthlyWithdrawalLimit(acct, amount)) {
            cout << "Error: Amount exceeds the monthly withdrawal limit. Enter a valid amount: ";
            cin >> amount;
        }
    }
    else if (exceedsDailyDepositLimit(acct1, amount)) {
        while (exceedsDailyDepositLimit(acct1, amount)) {
            cout << "Error: Amount exceeds the daily deposit limit. Enter a valid amount: ";
            cin >> amount;
        }
    }
    transferMoney(ul, amount, acct, acct1);
    cout << "Transaction successful." << endl;
}


//FINAL FUNCTION TO DELE TRANSACTIONS
void transactionDeletion(userList* ul) {
	string date;
    date = getDateFromUser();
    deleteTransactions(ul, date);
    return;
}

//===========
//QUESTION 3 
//===========
//FINAL FUNCTION TO SORT TRANSACTIONS
void sortTransactions(userList* ul) {
    user* currentUser = ul->head;
    while (currentUser != NULL) {
        account* currentAccount = currentUser->acct;
        while (currentAccount != NULL) {
            if (currentAccount->txn != NULL) {  // Check if there are transactions to sort
                currentAccount->txn = mergeSortTransactions(currentAccount->txn);
            }
            currentAccount = currentAccount->next;
        }
        currentUser = currentUser->next;
    }
    cout << "Transactions sorted successfully." << endl;
}

//======================================== MENU FUNCTIONS =========================================
void mainMenu(userList* ul) {
    cout << "============== MAIN MENU =============" << endl;
    char ans = 0;
    while (ans != '*') {
        char ans;
        if (ul->head == NULL) {
            cout << "There are no users yet!" << endl;
            cout << "========================" << endl;
            cout << "What Would You Like to Do?" << endl;
            cout << "========================" << endl;
            cout << "1. Create User" << endl;
            cout << "* To Save & Exit" << endl;
            cout << "Enter your choice: ";
            cin >> ans;
            while (ans != '*' && ans < 1 && ans>6) {
                cout << "Error: Enter a valid choice." << endl;
                cin >> ans;
            }

            switch (ans) {
            case '1':
                createUser(ul);
                break;
            case '*':
                writeToFile(*ul);
                cout << "Changes saved, exiting program. Goodbye!" << endl;
                return;
            default:
                cout << "Error: Invalid choice. Please try again." << endl;
            }
        }
        cout << "========================" << endl;
        cout << "What Would You Like to Do?" << endl;
        cout << "========================" << endl;
        cout << "1. Create User" << endl;
        cout << "2. Create Account" << endl;
        cout << "3. Perform Transaction" << endl;
        cout << "4. Sort Transactions" << endl;
        cout << "5. Delete Transactions" << endl;
        cout << "* To Save & Exit" << endl;
        cout << "Enter your choice: ";
        cin >> ans;
        while (ans != '*' && ans < 1 && ans>6) {
            cout << "Error: Enter a valid choice." << endl;
            cin >> ans;
        }

        switch (ans) {
        case '1':
            createUser(ul);
            break;
        case '2':
            accountCreation(ul);
			break;
        case '3':
			makeTransaction(ul);
			break;
        case '4':
            sortTransactions(ul);
            break;
        case '5':
            transactionDeletion(ul);
            break;
        case '*':
			writeToFile(*ul);
			cout << "Changes saved, exiting program. Goodbye!" << endl;
			return;
        default:
			cout << "Error: Invalid choice. Please try again." << endl;
        }
    }
}


//======================================== MAIN FUNCTION =========================================
int main(int argc, char** argv) {
    cout << "================= Welcome To The Banking System! =================" << endl;

    userList ul;
    initialiseUserList(&ul);

    cout << "Loading data..." << endl;
    loadDataFromFile(&ul);

    mainMenu(&ul);
    return 0;
}



