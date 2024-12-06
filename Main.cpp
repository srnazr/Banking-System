#define _CRT_SECURE_NO_WARNINGS //FOR CHRONO

//LIBRARIES
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <regex>
#include <fstream>
#include <sstream>
#include <thread>


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
//MISCEALLENOUS FUNCTIONS
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

//FUNCTION TO CLEAR CONSOLE
void clearConsole() {
    system("cls");
}

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

//FUNCTION TO VALIDATE THE AMOUNT
bool isValidAmount(double amount) {
    if (amount <= 0) return false;
    return true;
}

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


//FUNCTION FOR ACCOUNT TYPE
string findAccountType(account acct) {
    if (acct.currency == "USD") return "domiciled";
    else return "international";
}

//FILE MANIPULATION FUNCTIONS
//FUNCTION TO READ FROM FILES


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
            file << "#" << currentAccount->IBAN << "," << currentAccount->accountName << "," << currentAccount->balance << currentAccount->currency << "," << currentAccount->limitDepositPerDay << currentAccount->currency << "," << currentAccount->limitWithdrawPerMonth << currentAccount->currency <<","<< currentAccount->depositTrack << "," << currentAccount->withdrawTrack << "," << currentAccount->lastDailyDeposit << "," << currentAccount->lastMonthlyWithdraw << endl;

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


//LIST FUNCTIONS
//FUNCTION TO INITIALIZE USER LIST IN MAIN
void initializeUserList(userList& ul) {
    ul.head = NULL;
    ul.tail = NULL;
}

//FUNCTION TO DISPLAY USER LIST CONTENT
void displayUserList(const userList& ul) {
    if (ul.head == nullptr) {
        cout << "No users found." << endl;
        return;
    }

    user* temp = ul.head;
    while (temp != nullptr) {
        cout << "User ID: " << temp->userID << endl;
        cout << "First Name: " << temp->fname << endl;
        cout << "Last Name: " << temp->lname << endl;

        cout << "Accounts:" << endl;
        if (temp->acct != nullptr) {
            account* acctTemp = temp->acct;
            int i = 1;
            while (acctTemp != nullptr) {
                cout<<"Account " << i << endl;
                cout << "Account Currency: " << acctTemp->currency << endl;
                cout << "Account Balance: " << acctTemp->balance << endl;
                cout << "Daily Deposit Limit: " << acctTemp->limitDepositPerDay << endl;
                cout << "Monthly Withdrawal Limit: " << acctTemp->limitWithdrawPerMonth << endl;
                cout << "Remaining Deposit Allowance: " << acctTemp->depositTrack << endl;
                cout << "Remaining Withdraw Allowance: "<<acctTemp->withdrawTrack<<endl;
                cout << "Last Daily Deposit: " << acctTemp->lastDailyDeposit << endl;
                cout << "Last Monthly Withdraw: " << acctTemp->lastMonthlyWithdraw << endl;
                cout << "Account IBAN: " << acctTemp->IBAN << endl;
                cout << "Account Type: " << acctTemp->accountName << endl;
                cout << "----------------------" << endl;
                acctTemp = acctTemp->next;
                i++;
            }
        }
        else {
            cout << "No accounts available for this user." << endl;
        }

        cout << "----------------------" << endl;
        temp = temp->next;
    }
    return;
}

void displayUserInfo(const userList& ul, int ID) {
    if (ul.head == nullptr) {
        cout << "No users found." << endl;
        return;
    }

    user* temp = ul.head;
    while (temp != nullptr) {
        if (temp->userID == ID) {
            cout << "User ID: " << temp->userID << endl;
            cout << "First Name: " << temp->fname << endl;
            cout << "Last Name: " << temp->lname << endl;

            cout << "Accounts:" << endl;
            if (temp->acct != nullptr) {
                account* acctTemp = temp->acct;
                int i = 1;
                while (acctTemp != nullptr) {
                    cout << "Account " << i << endl;
                    cout << "Account Currency: " << acctTemp->currency << endl;
                    cout << "Account Balance: " << acctTemp->balance << endl;
                    cout << "Daily Deposit Limit: " << acctTemp->limitDepositPerDay << endl;
                    cout << "Monthly Withdrawal Limit: " << acctTemp->limitWithdrawPerMonth << endl;
                    cout << "Remaining Deposit Allowance: " << acctTemp->depositTrack << endl;
                    cout << "Remaining Withdraw Allowance: " << acctTemp->withdrawTrack << endl;
                    cout << "Last Daily Deposit: " << acctTemp->lastDailyDeposit << endl;
                    cout << "Last Monthly Withdraw: " << acctTemp->lastMonthlyWithdraw << endl;
                    cout << "Account IBAN: " << acctTemp->IBAN << endl;
                    cout << "Account Type: " << acctTemp->accountName << endl;
                    cout << "----------------------" << endl;
                    acctTemp = acctTemp->next;
                    i++;
                }
            }
            else {
                cout << "No accounts available for this user." << endl;
            }

            cout << "----------------------" << endl;
            return;
        }
        temp = temp->next;
    }
    cout << "User not found." << endl;
}

//FUNCTION TO LOCATE ACCOUNT
account* locateAccount(userList ul, string IBAN) {
    user* temp = ul.head;
    while (temp != nullptr) {
        account* temp1 = temp->acct;
        while (temp1 != nullptr) {
            if (temp1->IBAN == IBAN) {
                return temp1;  // Return a pointer to the matching account
            }
            temp1 = temp1->next;
        }
        temp = temp->next;
    }
    return nullptr;  // Return nullptr if no account with matching IBAN is found
}


//USER FUNCTIONS
// FUNCTION TO GENETRATE ACCOUNT AND ALLOW MODIFICATIONS
account generateAccount() {
    account acct;
    cout << "Enter the account's preferred currency (USD - EUR - GBP - YEN): ";
    cin >> acct.currency;
    while (!isValidCurrency(acct.currency)) {
        cout << "Invalid currency. Please enter a valid currency: ";
        cin >> acct.currency;
    }

    cout << "\nEnter the amount you would like to deposit: ";
    cin >> acct.balance;
    while (!isValidAmount(acct.balance)) {
        cout << "Invalid amount. Please enter a valid amount: ";
        cin >> acct.balance;
    }

    cout << "\nEnter the daily deposit limit: ";
    cin >> acct.limitDepositPerDay;
    while (!isValidAmount(acct.limitDepositPerDay)) {
        cout << "Invalid amount. Please enter a valid amount: ";
        cin >> acct.limitDepositPerDay;
    }

    cout << "\nEnter the monthly withdrawal limit: ";
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
    acct.next= NULL;
    return acct;
}

//FUNCTION TO GET USER INFORMATION
user getUserInfo() {
    user usr;
    cout<<"Enter your ID: ";
    cin>>usr.userID;
    cout<<"Enter your first name: ";
    cin.ignore();
    getline(cin, usr.fname);
    cout<<"Enter your last name: ";
    cin.ignore();
    getline(cin, usr.lname);
    return usr;
}

//FUNCTION TO GET NEW USER INFORMATION
user getNewUserInfo() {
    user usr;
    cout << "Enter your first name: ";
    getline(cin, usr.fname);
    cout << "\nEnter your last name: ";
    getline(cin, usr.lname);
    return usr;
}

//FUNCTION TO CHECK IF USER ACCOUNT ALREADY EXISTS
bool accountExists(user usr, account acct) {
    if (usr.acct == NULL) {
		return false;
	}
    account* temp = usr.acct;
    while (temp != NULL) {
        if (temp->currency == acct.currency && temp->limitDepositPerDay == acct.limitDepositPerDay && temp->limitWithdrawPerMonth == acct.limitWithdrawPerMonth)
            return true;
        temp = temp->next;
    }
    return false;
}

//FUNCTION TO GIVE AN ACCOUNT AN IBAN AND ACCOUNT TYPE IF IT DOESN'T EXIST
void giveIBAN_Type(account& acct) {
    acct.IBAN = generateIBAN(acct);
    acct.accountName = findAccountType(acct);
}

//FUNCTION TO VALIDATE IBAN INPUT
bool IBANexists(userList ul, string IBAN) {
	user* temp = ul.head;
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

//FUNCTION TO CREATE ACCOUNTS
void createNewAccount(userList& ul, user usr, account acct) {
    user* temp = ul.head;
    if (temp == NULL) {
        cout << "User list is empty. Please create a user first." << endl;
        return;
    }

    while (temp != NULL) {
        if (temp->userID == usr.userID) {
            account* temp1 = temp->acct;

            if (temp1 == NULL) {
                temp->acct = new account;
                *temp->acct = acct;
                giveIBAN_Type(*temp->acct);
                temp->acct->next = NULL;
                cout << "Account created successfully." << endl;
                return;
            }
            else {
                while (temp1 != NULL) {
                    if (accountExists(*temp, acct)) {
                        cout << "Account already exists. Try with a different account." << endl;
                        return;
                    }
                    if (temp1->next == NULL) break;
                    temp1 = temp1->next;
                }
                temp1->next = new account;
                *(temp1->next) = acct;
                giveIBAN_Type(*(temp1->next));
                temp1->next->next = NULL;
                cout << "Account created successfully." << endl;
                return;
            }
        }
        temp = temp->next;
    }

    cout << "User not found. Please create the user first." << endl;
}

//FUNCTION TO CHECK IF USER ALREADY EXISTS
bool userExists(userList ul, user usr) {
	user* temp = ul.head;
	while (temp != NULL) {
		if (temp->userID == usr.userID) {
			return true;
		}
		temp = temp->next;
	}
	return false;
}

//FUNCTION TO ADD USER TO USER LIST
void addUser(userList& ul, user usr) {
    user* newUser = new user;
    *newUser = usr;
    newUser->acct = NULL;
    newUser->next = NULL;
    newUser->previous = NULL;

    if (ul.head == NULL) {
        ul.head = newUser;
        ul.tail = newUser;
    }
    else {
        ul.tail->next = newUser;
        newUser->previous = ul.tail;
        ul.tail = newUser;
    }
    cout << "User added successfully." << endl;
}

// FUNCTION TO CHECK IF ACCOUNT HAS ENOUGH BALANCE
bool hasEnoughBalance(account* acct, double amount) {
    if (acct->balance < amount) return false;
    return true;
}

// FUNCTION TO CHECK IF IT EXCEEDS THE DAILY DEPOSIT LIMIT
bool exceedsDailyDepositLimit(account* acct, double amount) {
    if (amount + acct->depositTrack > acct->limitDepositPerDay) return true;
    return false;
}

// FUNCTION TO CHECK IF IT EXCEEDS THE MONTHLY WITHDRAWAL LIMIT
bool exceedsMonthlyWithdrawalLimit(account* acct, double amount) {
    if (amount + acct->withdrawTrack > acct->limitWithdrawPerMonth) return true;
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

// FUNCTION TO INCREASE BALANCE
void increaseBalance(account* acct, double amount) {
    acct->balance += amount;
    acct->depositTrack += amount;
    acct->lastDailyDeposit = getCurrentDateTime();
}

// FUNCTION TO DECREASE BALANCE
void decreaseBalance(account* acct, double amount) {
    acct->balance -= amount;
    acct->withdrawTrack += amount;
    acct->lastMonthlyWithdraw = getCurrentDateTime();
}

// FUNCTION FOR MONEY TRANSFER
void transferMoney(userList& ul, double amount, account* acct1, account* acct2) {
    user* temp = ul.head;
    while (temp != NULL) {
        account* temp1 = temp->acct;
        while (temp1 != NULL) {
            if (temp1->IBAN == acct1->IBAN) {
                addNegTransaction(temp1, amount);  // Record negative transaction in acct1
                decreaseBalance(temp1, amount);    // Decrease balance in acct1
            }
            if (temp1->IBAN == acct2->IBAN) {
                addPosTransaction(temp1, amount);  // Record positive transaction in acct2
                increaseBalance(temp1, amount);    // Increase balance in acct2
            }
            temp1 = temp1->next;
        }
        temp = temp->next;
    }
    return;
}

//FUNCTION TO DELETE TRANSACTIONS BEFORE A CERTAIN DATE
void deleteTransactions(userList& ul, string date) {
    user* currentUser = ul.head;
    bool transactionsDeleted = false;
    while (currentUser != NULL) {
        account* currentAccount = currentUser->acct;
        while (currentAccount != NULL) {
            transaction* currentTrans = currentAccount->txn;
            transaction* prevTrans = NULL;
            while (currentTrans != NULL) {
                if (currentTrans->date < date) {
                    if (prevTrans == NULL) {
                        currentAccount->txn = currentTrans->next;
                    }
                    else
                        prevTrans->next = currentTrans->next;

                    transaction* temp = currentTrans;
                    currentTrans = currentTrans->next;
                    delete temp;
                    transactionsDeleted = true;
                }
                else {
                    prevTrans = currentTrans;
                    currentTrans = currentTrans->next;
                }
            }
            currentAccount = currentAccount->next;
        }
        currentUser = currentUser->next;
    }
    if (transactionsDeleted) {
        cout << "Transactions deleted successfully." << endl;
    }
    else {
        cout << "No transactions found to delete." << endl;
    }
}

//FINAL TRANSACTION DELETION FUNCTION
void transactionDeletion(userList& ul) {
    string date = getDateFromUser();
    deleteTransactions(ul, date);
    return;
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


//====================================== FINALIZED FUNCTIONS ======================================
//FINAL FUNCTION FOR ACCOUNT CREATION
void accountCreation(userList& ul) {
    if (ul.head == NULL) {
        cout << "User list is empty. Please create a user first." << endl;
        return;
    }
    cout << "===== Account Creation ====" << endl << endl;
    cout << "==== User Information ====" << endl;
    user user1 = getUserInfo();
    cout << "===== Account Creation ====" << endl << endl;
    cout << "=== Account Information ===" << endl;
    account acct1 = generateAccount();
    createNewAccount(ul, user1, acct1);
    return;
}

//FINAL FUNCTION FOR USER CREATION
void createUser(userList& ul) {
    cout << "===== User Creation ====" << endl << endl;
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

// FINAL FUNCTION FOR TRANSACTIONS
void makeTransaction(userList& ul) {
    string IBAN1, IBAN2;
    double amount;
    int ID;
    cout << "===== Transaction ====" << endl << endl;
    cout << "Enter your ID: ";
    cin >> ID;
    while (ID < 1) {
        cout << "Error: Enter a valid ID: ";
        cin >> ID;
    }

    cout << "Your current accounts are: " << endl;
    displayUserInfo(ul, ID);

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
    else {
        transferMoney(ul, amount, acct, acct1);
        cout << "Transaction successful." << endl;
    }
}

// FINAL TRANSACTION DELETION FUNCTION
void transactionDeletion(userList & ul) {
    cout<<"===== Transaction Deletion ===="<<endl;
    string date = getDateFromUser();
    deleteTransactions(ul, date);
    return;
}

//FINAL FUNCTION TO SORT TRANSACTIONS
void sortTransactions(userList& ul) {
    user* currentUser = ul.head;
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

