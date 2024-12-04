#define _CRT_SECURE_NO_WARNINGS // To avoid the error of localtime function

//Libraries
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
#include <regex>
#include <fstream>
#include <sstream>

//namespaces
using namespace std;
using namespace chrono;

//Structures
//Singly Linked Lists
struct transaction {
	string date;
	double amount;
	transaction* next;
};

struct account {
	string IBAN;
	string accountName;
	double balance;
	string currency; //Adopted Currencies: USD, GBP, EUR, JPY, CNY
	double limitDepositPerDay;
	double limitWithdrawPerMonth;
	transaction* txn;
	account* next;
};

//Doubly Linked Lists
struct user {
	int userID;
	string fname;
	string lname;
	account* acct;
	user* next, *previous;
};

struct userList {
	user* head, *tail;
};

//global variables
//IBAN 
const string countryCode= "US";
const string bankCode = "JPMC";
const string usersFile= "userList.txt";

//function to generate user ID
int generateUserID() {
	static int userID = 0;

	// Read the last used ID from the file
	ifstream infile("userID.txt");
	if (infile) {
		infile >> userID;
	}
	infile.close();

	// Increment the userID
	userID++;

	// Write the new userID back to the file
	ofstream outfile("userID.txt");
	if (outfile) {
		outfile << userID;
	}
	outfile.close();

	return userID;
}

//currency conversion function
double currencyConversion(double amount, account acct1, account acct2) {
	if (acct1.currency == acct2.currency)
		return amount;
	else if (acct2.currency != "USD" && acct2.currency!="EUR" && acct2.currency!="GBP" && acct2.currency != "JPY" && acct2.currency != "CNY") {
		return -1;
	}
	else if (acct1.currency == "USD") {
		if (acct2.currency == "EUR") return amount * 0.92;
		else if (acct2.currency == "GBP") return amount * 0.78;
		else if (acct2.currency == "JPY") return amount * 150.35;
		else return amount * 7.08;
	}
	else if (acct1.currency == "EUR") {
		if (acct2.currency == "USD") return amount * 1.09;
		else if (acct2.currency == "GBP") return amount * 0.85;
		else if (acct2.currency == "JPY") return amount * 120.52;
		else return amount * 7.08;
	}
	else if (acct1.currency == "GBP") {
		if (acct2.currency == "USD") return amount * 1.29;
		else if (acct2.currency == "EUR") return amount * 1.17;
		else if (acct2.currency == "JPY") return amount * 141.47;
		else return amount * 8.37;
	}
	else if (acct1.currency == "JPY") {
		if (acct2.currency == "USD") return amount * 0.0066;
		else if (acct2.currency == "EUR") return amount * 0.0083;
		else if (acct2.currency == "GBP") return amount * 0.0071;
		else return amount * 0.059;
	}
	else{
		if (acct2.currency == "USD") return amount * 0.14;
		else if (acct2.currency == "EUR") return amount * 0.13;
		else if (acct2.currency == "GBP") return amount * 0.12;
		else return amount * 16.92;
	}
}

//function to get transaction date/time 
string getCurrentDateTime() {
	system_clock::time_point now = system_clock::now(); //get the current time
	time_t currentTime = system_clock::to_time_t(now); //convert to time_t (C style time - ctime)
	struct tm* localTime = localtime(&currentTime); //convert to local time
	stringstream ss;
	ss << put_time(localTime, "%Y-%m-%d %H:%M:%S"); //format the time
	return ss.str();
}

//function to get the linked list's center
transaction* getCenter(transaction* start) {
	if (start == NULL || start->next == NULL)
		return start;

	transaction* slow = start;
	transaction* fast = start->next;

	while (fast && fast->next) {
		slow = slow->next;
		fast = fast->next->next;
	}

	transaction* center = slow->next;
	slow->next = NULL;

	return center;
}

//function to merge two linked lists
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

//function to sort the linked list
transaction* mergeSortTransactions(transaction* start) {
	if (start == NULL || start->next == NULL)
		return start;

	transaction* center = getCenter(start);

	transaction* left = mergeSortTransactions(start);
	transaction* right = mergeSortTransactions(center);

	return merge(left, right);
}

//function for user date input
string getUserDateInput() {
	string date;
	cout << "Enter the date of the transaction you want to delete (Format: Y-M-D): ";
	getline(cin, date);

	regex datePattern(R"(\d{4}-\d{2}-\d{2})");
	while (!regex_match(date, datePattern)) {
		cout << "Invalid format. Please enter the date in y-m-d format (e.g., 2024-12-05): ";
		getline(cin, date);
	}

	return date;
}

//function to check if date is valid
bool isValidDate(const string date) {
	smatch match;

	int year = stoi(match[1].str());
	int month = stoi(match[2].str());
	int day = stoi(match[3].str());

	if (month < 1 || month > 12) {
		return false;
	}

	if (month == 2) {
		bool isLeap = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
		if (day < 1 || day >(isLeap ? 29 : 28)) {
			return false;
		}
	}
	else if (month == 4 || month == 6 || month == 9 || month == 11) {
		if (day < 1 || day > 30) {
			return false;
		}
	}
	else {
		if (day < 1 || day > 31) {
			return false;
		}
	}

	return true;
}


//function to remove a transaction from the linked list
void deleteTransactions(userList& ul, int userID){
	cout<<"===========Welcome to the Transaction Deletion Menu==========="<<endl;
	string date=getUserDateInput();
	while(!isValidDate(date)){
		cout<<"Invalid date. The date you entered is in the future."<<endl;
		date=getUserDateInput();
	}

	user *currentUser= ul.head;
	while (currentUser != NULL) {
		if (currentUser->userID == userID) {
			account *userAccount= currentUser->acct;

			if (userAccount != NULL) {
				transaction *currentTransaction= userAccount->txn;
				transaction *previousTransaction= NULL;
				
				while (currentTransaction != NULL) {
					if (currentTransaction->date < date) {
						if (previousTransaction == NULL) {
							userAccount->txn = currentTransaction->next;
						}
						else {
							previousTransaction->next = currentTransaction->next;
						}
						transaction *temp = currentTransaction;
						currentTransaction = currentTransaction->next;
						delete temp;
					}
					else {
						previousTransaction= currentTransaction;	
						currentTransaction= currentTransaction->next;
					}
				}
			}
			cout<<"Transactions deleted successfully."<<endl;
			return;
		}
		currentUser= currentUser->next;
	}
	cout<<"User not found"<<endl;
}


//function to write to files
void writeToFile(userList ul) {
	ofstream file(usersFile); 
	if(!file){
		cout << "Error: File not found or could not be opened." << endl;
		return;
	}
	user *currentUser= ul.head;
	while (currentUser != NULL) {
		file<<"-"<<currentUser->userID<<","<<currentUser->fname<<","<<currentUser->lname<<endl;

		account *currentAccount= currentUser->acct;
		while (currentAccount != NULL) {
			file<<"#"<<currentAccount->IBAN<<","<<currentAccount->accountName<<","<<currentAccount->balance<<currentAccount->currency<<","<<currentAccount->limitDepositPerDay<<currentAccount->currency<<","<<currentAccount->limitWithdrawPerMonth<<currentAccount->currency<<endl;
		
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
	cout<<"Data saved successfully"<<endl;
}

//function to read from files
/*void readFromFile(userList& ul) {
	ifstream file(usersFile); // Ensure the correct file path is used
	if (!file) {
		cout << "Error: File not found or could not be opened." << endl;
		return;
	}

	string line;
	user* currentUser = NULL;
	account* currentAccount = NULL;

	while (getline(file, line)) {
		if (line.empty()) continue;

		if (line[0] == '-') {
			user* newUser = new user;
			stringstream ss(line.substr(1));
			ss >> newUser->userID;
			getline(ss, newUser->fname, ',');
			getline(ss, newUser->lname, ',');

			newUser->acct = NULL;
			newUser->next = NULL;
			newUser->previous = NULL;

			if(ul.head == NULL) {
				ul.head = newUser;
				ul.tail = newUser;
			}
			else {
				ul.tail->next = newUser;
				newUser->previous = ul.tail;
				ul.tail = newUser;
			}

			currentUser = newUser;
		}
		else if (line[0] == '#') {
			account* newAccount = new account;
			stringstream ss(line.substr(1));
			getline(ss, newAccount->IBAN, ',');
			getline(ss, newAccount->accountName, ',');
			string bal, depositLimit, withdrawLimit;
			getline(ss, bal, ',');
			getline(ss, newAccount->currency, ',');
			getline(ss, depositLimit, ',');
			getline(ss, newAccount->currency, ',');
			getline(ss, withdrawLimit, ',');
			getline(ss, withdrawLimit, ',');

			newAccount->balance = stod(bal.substr(0, bal.size() - 1));
			newAccount->limitDepositPerDay = stod(depositLimit.substr(0, depositLimit.size() - 1));
			newAccount->limitWithdrawPerMonth = stod(withdrawLimit.substr(0, withdrawLimit.size() - 1));
		}
	}
}*/


int main() {
	int n1, n2;
	n1=generateUserID();
	n2=generateUserID();
	cout<<"User ID 1: "<<n1<<endl;
	cout<<"User ID 2: "<<n2<<endl;
	return 0;
}
