//Libraries
#include <iostream>
#include <string>
#include <ctime>
#include <chrono>
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
	string balance;
	string currency;
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

//Functions List
/*
1- Currency conversion function that takes the amount and the currency and returns the converted amount
2- File parsing function that reads the file and stores the data in the doubly linked list userList
3- Money transfer function that takes userList, monetary amount, 2 accounts (transfer from 1st acc to 2nd)
-> Check if the amount has enough balance
-> Currency conversion if the accounts have different currencies
-> Check if the amount exceeds the daily limit
-> Store the transaction in the transaction singly linked list
4- Function that sorts the transactions in ascending order, chaging links between nodes
5- User creation function that takes the user list and the user's first and last name and checks if user already exists
6- Transaction deletion function that takes the user list and the transaction date and deletes the transaction if it is less than the current date
7- Function that writes all users in a file while ensuring data isn't repeated
8- Main function that calls all the functions

//Additional Functions
1- User account search that displays account info if it exists
2- Transaction Enhancement: Filter by date
3- User Authentication: User ID and Password
+ GUI
*/

//global variables
//gets the current time
time_t currentTime = time(nullptr);
struct tm* localTime = localtime(&currentTime);

//currency conversion function
double currencyConversion(double amount, string currency) {
	
}