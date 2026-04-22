#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cctype>
#include <vector>
#include <algorithm>

using namespace std;

// ================== STRUCTS ==================

struct User {
    string username;
    string password;
    string role; // admin or employee
    string gender;
	string phone;
    string status;
};

struct Book {
    int id;
    string name;
    string author;
    int quantity;
};

//void borrowBook(string username);

// ================== FUNCTIONS ==================

bool login(string &role,string &username) {
    string u, p;
    ifstream file("users.txt");
    if(!file.is_open()){
    	ofstream createFile("users.txt");
    	createFile<<"admin"<<" "<<"1234"<<" "<<"admin"<<" "<<"777777777"<<" "<<"male"<<" "<<"active"<<endl;
    	createFile.close();
    	file.open("users.txt");
	}
    cout << "Username: ";
    cin >> u;
    cout << "Password: ";
    cin >> p;

    User user;
    while (file >> user.username >> user.password >> user.role>>user.phone>>user.gender>>user.status) {
        if (u == user.username && p == user.password&&user.status=="active") {
            role = user.role;
            username=user.username;
            file.close();
            return true;
        }
    }
    file.close();
    cout<<"Login failed - Check username/ password or account status"<<endl;
    return false;
}

// ---------- BOOK FUNCTIONS ----------

bool validID(int id){
	
	return id>=1;
}

bool bookIDExists(int id){
	ifstream file("books.txt");
	if(!file.is_open()){
		return false;
	}
	Book b;
	while(file>>b.id){
		file.ignore();
		getline(file,b.name);
		getline(file,b.author);
		file>>b.quantity;
		if(b.id==id){
			file.close();
			return true;
		}
	}
		file.close();
		return false;
}

void printInvoice(string employee,string bookname,int bookID,string borrower,string phone){
	cout<<"\n==============BORROWER INVOICE================"<<endl;
	cout<<"Employee: "<<employee<<endl;
	cout<<"Book ID: "<<bookID<<endl;
	cout<<"Book Name: "<<bookname<<endl;
	cout<<"Borrower: "<<borrower<<endl;
	cout<<"Phone: "<<phone<<endl;
	cout<<"================================================"<<endl;
	
	
}

bool decreaseQuantity(int bookid,string &bookname){
	ifstream file("books.txt");
	ofstream temp("temp_books.txt");
	if(!file.is_open()||!temp.is_open()){
		return false;
	}
	Book b;
	bool found =false;
	
	while(file>>b.id){
		file.ignore();
		getline(file,b.name);
		getline(file,b.author);
		file>>b.quantity;
		if(b.id==bookid){
			found=true;
			bookname=b.name;
			if(b.quantity<=0){
				file.close();
				temp.close();
				remove("temp_books.txt");
				return false;
				
			}
			b.quantity--;
		}
		temp<<b.id<<endl<<b.name<<endl<<b.author<<endl<<b.quantity<<endl;
	}
	file.close();
	temp.close();
	if(found==true){
	remove("books.txt");
	rename("temp_books.txt","books.txt");
	}
	return found;
}

void topFiveBooks(){
	ifstream file("borrow.txt");
	if(!file.is_open()){
    	cout<<"Error: Cannot open borrow file"<<endl;
    	return;
	}
	vector<string>book;
	vector<int>count;
	int id;
	string bookname, borrower, phone,user;
	
	
	
	while(file>>id>>bookname>>borrower>>phone>>user){
		
		
		bool found =false;
		
		for(int i=0;i<book.size();i++){
			if(book[i]==bookname){
				count[i]++;
				found= true;
				break;
			}
		}
		
		if(!found){
			book.push_back(bookname);
			count.push_back(1);
			
			}
		
	}
	file.close();
	
	if(book.size()==0){
		cout<<"No borrowed books yet"<<endl;
		return;
	}
	
	for(int i=0;i<count.size()-1;i++){
		for(int j=i+1;j<count.size();j++){
			if(count[j]>count[i]){
				swap(count[i],count[j]);
				swap(book[i],book[j]);
			}
		}
	}
	
	cout<<"\n----TOP 5 MOST BORROWED BOOKS ----"<<endl;
	cout<<"Rank\tName\tQuantity"<<endl;
	cout<<"----------------------------------------------"<<endl;
	for(int i=0;i<book.size()&&i<5;i++){
		cout<<i+1<<"\t"<<book[i]<<"\t\t"<<count[i]<<endl;
		
	}
}

void addBook() {
    Book b;
    ofstream file("books.txt", ios::app);
    if(!file.is_open()){
    	cout<<"Error: Cannot open books file"<<endl;
    	return;
	}
	do{
    	cout << "Book ID: ";
    	cin >> b.id;
		if(!validID(b.id))
		cout<<"ID must be natural number"<<endl;
		else if(bookIDExists(b.id))
		cout<<"ID already exists"<<endl;
	
	}while(!validID(b.id)||bookIDExists(b.id));
    
    cin.ignore();
    cout << "Book Name: ";
    getline(cin, b.name);
    cout << "Author Name: ";
    getline(cin, b.author);
    
    do{
    	cout<<"Quantity : ";
    	cin>>b.quantity;
	}while(b.quantity<0);

   file<<b.id<<endl<<b.name<<endl<<b.author<<endl<<b.quantity<<endl;
    file.close();
    cout << "Book Added Successfully\n";
}

void viewBooks() {
    Book b;
    ifstream file("books.txt");
    if(!file.is_open()){
    	cout<<"Error: Cannot open books file"<<endl;
    	return;
	}
   
    string line;
   // int total=0;
    cout << "\nID\tBookName\tAuthor\tQuantity\n";
    cout<<"------------------------------------"<<endl;
    
    while (file >> b.id ) {
    	file.ignore();
    	getline(file, b.name);
    	getline(file, b.author);
    	file>>b.quantity;
       
       
       cout<<b.id<<"\t"<<b.name<<"\t\t"<<b.author<<"\t"<<b.quantity<<endl;
	    };
    file.close();
}

void UpdataBook(){
	bool update=false;
	while(!update){
	
	int d;
	Book b;
	cout<<"Enter Book ID to updata (-1 to cancelled): ";
	cin>>d;
	cin.ignore();
	if(d==-1){
		cout<<"Update cancelled "<<endl;
		return;
	}
	
	ifstream file("books.txt");
	ofstream temp("temp_books.txt");
	if(!file.is_open()||!temp.is_open()){
		cout<<"No found file"<<endl;
		return;
	}
	bool found=false;
	string line;
	cout << "\nID\tBook Name\tAuthor\n";
	while(file >> b.id ){
		 file.ignore();
		 getline(file,b.name);
		 getline(file,b.author);
		 file>>b.quantity;
		 file.ignore();
		 
      if(b.id==d){
      	found= true;
      	cout<<"New Book Name ";
      	getline(cin,b.name);
      	cout<<"New Book Author ";
      	getline(cin,b.author);
      	cout<<"New Quantity : ";
      	cin>>b.quantity;
      	cin.ignore();
      	
	  }
    temp<<b.id<<endl<<b.name<<endl<<b.author<<endl<<b.quantity<<endl;
	}
	file.close();
	temp.close();
	
	
	if(found){
		cout<<"Book updata successfully"<<endl;
		update=true;
	remove("books.txt");
	rename("temp_books.txt","books.txt");
		}
	else
	cout<<"Book not foun, try again..."<<endl;
	}
}
		  	



void searchBook(){
	string key;
	bool found=false;
	Book b;
	cout<<"Enter book name or author:";
	cin.ignore();
	getline(cin,key);
	
	ifstream file("books.txt");
	if(!file.is_open()){
    	cout<<" No  books file found"<<endl;
    	return;
	}
	cout << "\nID\tBook Name\tAuthor\tQuantity\n";
	while(file >> b.id){
		file.ignore();
		getline(file,b.name);
		getline(file,b.author);
		file>>b.quantity;
		
      if(b.name==key||b.author==key){
      	cout<<b.id<<"\t"<<b.name<<"\t\t"<<b.author<<"\t"<<b.quantity<<endl;;
      	found=true;
	  }
}
	file.close();
	
	if(!found){
		cout<<"No matching Books found "<<endl;
	}

	
}


void deleteBook() {
    int id;
    bool found = false;
    Book b;
    ifstream file("books.txt");
    ofstream temp("temp_books.txt");
   
    if(!file.is_open()||!temp.is_open()){
    	cout<<" No  books file found"<<endl;
    	return;
	}

    cout << "Enter Book ID to delete: ";
    cin >> id;

    while (file >> b.id ) {
    	file.ignore();
    	getline(file,b.name);
    	getline(file,b.author);
    	file>>b.quantity;
    	
        if (b.id == id){
            found = true;
		}
        else
            temp<<b.id<<endl<<b.name<<endl<<b.author<<endl<<b.quantity<<endl;;
    }

    file.close();
    temp.close();
    

    if (found){
    remove("books.txt");
    rename("temp_books.txt", "books.txt");
        cout << "Book Deleted succesfully\n";
	}
    else{
        cout << "Book Not Found\n";
       remove("temp_books.txt");
	}
}

// ---------- USER FUNCTIONS (ADMIN ONLY) ----------

bool usernameExists(string uname){
	ifstream file("users.txt");
	if(!file.is_open()){
		cout<<"not found file"<<endl;
		return false;
	}
	User u ;
	while(file >>u.username >> u.password >> u.role>>u.phone>>u.gender>>u.status){
		if(u.username == uname){
			file.close();
			return true;
		}
	}
	file.close();
	return false;
	
}


bool validPhone(string phone){
	if(phone.length()!=9){
		return false;
	}
	if(phone[0]!='7'){
		return false;
	}
	
	if(phone[1]!='7'&&phone[1]!='8'&&phone[1]!='1'&&phone[1]!='3'&&phone[1]!='0'){
		return false;
	}
	for(int i=0;i<9;i++){
		if(!isdigit(phone[i])){
			return false;
		}
	}
	return true;
	
	
}

bool validUsername(string name){
	for(int i=0;i<name.length();i++){
		if(!isalpha(name[i]))
		return false;
	}
	return true;
}

void addUser() {
    User u;
    fstream file("users.txt", ios::app);
    if(!file.is_open()){
    	cout<<"Error: Cannot open users file"<<endl;
    	return;
	}
	do{
    cout << "Username: ";
    cin >> u.username;
    if(!validUsername(u.username))
    cout<<"'username must cotain letters only"<<endl;
    
	else if(usernameExists(u.username)){
		cout<<"The username is found"<<endl;
		file.close();
		return;
	}
	
	}while(!validUsername(u.username)||usernameExists(u.username));
	
	
    cout << "Password: ";
    cin >> u.password;
    
	
    cout << "Role (admin/employee): ";
    cin >> u.role;
    
    do{
    	cout<<"Phone number: ";
    	cin>>u.phone;
    	if(!validPhone(u.phone)){
    		cout<<"Invalid phone number format"<<endl;
		}
	}while(!validPhone(u.phone));
	
	do{
		cout<<"Gender (male/female) : ";
		cin>>u.gender;
	}while(u.gender !="male"&& u.gender !="female");
    
    u.status="active";
    

    file << u.username << " " << u.password << " " << u.role <<" "<<u.phone<<" "<<u.gender<<" "<<u.status<< endl;
    file.close();
    cout << "User Added Successfully\n";
}


void toggleUserStatus() {
	string username;
	cout<<"Enter username to toggle status: ";
	cin>>username;
	
	ifstream file("users.txt");
	ofstream temp("temp_users.txt");
	User u;
	if(!file.is_open()||!temp.is_open()){
    	cout<<"Error: Cannot open users file"<<endl;
    	return;
	}
	bool found =false;
	string newStatus;
	while(file >>u.username >> u.password >> u.role>>u.phone>>u.gender>>u.status){
		if(u.username == username){
			found=true;
			if(u.status=="active"){
				u.status="inactive";
				newStatus="inactive";
				cout<<"User deactivated syccessfully"<<endl;
				
			}
			else{
				u.status="active";
				newStatus="active";
				cout<<"User activated syccessfully"<<endl;	
			}
		}
		temp<<u.username << " " << u.password << " " << u.role <<" "<<u.phone<<" "<<u.gender<<" "<<u.status<< endl;
	}
	file.close();
	temp.close();
	
	if(!found){
		cout<<"User not found"<<endl;
	}
	remove("users.txt");
	rename("temp_users.txt", "users.txt");
	
}

void viewAllUsers(){
	ifstream file("users.txt");
	User u;
	if(!file.is_open()){
    	cout<<"No users found"<<endl;
    	return;
	}
	
	cout<<"\n=========================================="<<endl;
	cout<<"Username\tg\tRole\t\tPhone\t\tStatus"<<endl;
	cout<<"\n======================================="<<endl;
	while(file >>u.username >> u.password >> u.role>>u.phone>>u.gender>>u.status){
		cout<<u.username<<"\t\t"<<u.role<<"\t\t"<<u.phone<<"\t"<<u.gender<<"\t"<<u.status<<endl;
	}
	file.close();
	cout<<"\n======================================="<<endl;
}

//bool getBookName(int id,string& bookname){
//	ifstream file("books.txt");
//	if(!file.is_open()){
//    	cout<<"No users found"<<endl;
//    	return false;
//	}
//	Book b;
//	while(file>>b.id){
//		file.ignore();
//		getline(file,b.name);
//		getline(file,b.author);
//		file>>b.quantity;
//		
//		if(b.id==id){
//			bookname=b.name;
//			file.close();
//			return true;
//			
//		}
//	}
//	file.close();
//	return false;
//	
//}




void borrowBook(string username){
	int bookId;
	string borrowname,phone,bookName;
	cout<<"Enter Book ID to borrow :";
	cin>>bookId;
	cin.ignore();
	
	if(!decreaseQuantity(bookId,bookName)){
		cout<<"book not available"<<endl;
		return;
	}
	cout<<"Book name :"<<bookName<<endl;
	cout<<"Enter Borrower Name : ";
	getline(cin,borrowname);
	do{
		cout<<"Enter Phone Number : ";
		cin>>phone;
		
		if(!validPhone(phone)){
			cout<<"Invlid phone number, try again"<<endl;
		}
	}while(!validPhone(phone));
	

	ofstream out("borrow.txt",ios::app);
	out<<bookId<<" "<<bookName<<" "<<borrowname<<" "<<phone<<" "<<username<<endl;
	out.close();
	cout<<"Book borrowed successfully "<<endl;
	printInvoice(username,bookName,bookId,borrowname,phone);// دالة طباعة الاشعار
	
	
}

bool increaseQuantity(int bookid){
	ifstream file("books.txt");
	ofstream temp("temp_books.txt");
	if(!file.is_open()||!temp.is_open()){
    	cout<<"No found"<<endl;
    	return false;
	}
		Book b;
		bool found=false; 
	while(file>>b.id){
		file.ignore();
		getline(file,b.name);
		getline(file,b.author);
		file>>b.quantity;
		
		if(b.id==bookid){
			found= true;
			b.quantity++;
			
		}
			temp<<b.id<<endl<<b.name<<endl<<b.author<<endl<<b.quantity<<endl;
	}
	file.close();
	temp.close();
		if(found==true){
	remove("books.txt");
	rename("temp_books.txt","books.txt");
		}
	return found;
}

void returnBook(string username){
	int bookid;
	cout<<"Enter Book ID to return: ";
	cin>>bookid;
	ifstream file("borrow.txt");
	ofstream temp("temp_borrow.txt");
	if(!file.is_open()||!temp.is_open()){
		cout<<"NO found file"<<endl;
	}
	
	int bid;
	string user,bookname,name,phone;
	bool found = false;
	while(file>>bid>>bookname>>name>>phone>>user){
		if(!found&&bid==bookid){
			found=true;
			continue;
		}
		else{
			temp<<bid<<" "<<bookname<<" "<<name<<" "<<phone<<" "<<user<<endl;
		}
		
	}
	file.close();
	temp.close();
	if(!found){
		cout<<"No borrowed book found "<<endl;
		remove("temp_borrow.txt");
		return;
	}
		else{
		increaseQuantity(bookid);
			
	remove("borrow.txt");
	rename("temp_borrow.txt","borrow.txt");
	cout<<"Book returned successfully "<<endl;
		}
	
}

void viewBorrowedBooks(){
	ifstream file("borrow.txt");
	int bid;
	string user,bookname,name,phone;
	if(!file.is_open()){
		cout<<"No borrowed books found"<<endl;
		return;
	}
	
	cout<<"\nID\tBook\tBorrower\tPhone\t\tUser"<<endl;
	cout<<"============================================================"<<endl;
	
	while(file>>bid>>bookname>>name>>phone>>user){
		cout<<bid<<"\t"<<bookname<<"\t"<<name<<"\t"<<phone<<"\t\t"<<user<<endl;
	}
	file.close();
}

void statistics(){
	ifstream users("users.txt");
	ifstream books("books.txt");
	
	if(!users.is_open()||!books.is_open()){
		cout<<"No  found file"<<endl;
	}
	Book b;
	int userCount=0;
	int bookCount=0;
	string line;
	while(getline(users,line))
	userCount++;
	
	while(books>>b.id){
	bookCount++;
	books.ignore();
	getline(books,line);
	getline(books,line);
	books>>b.quantity;
	}
	
	cout<<"System statisticss"<<endl;
	cout<<"Total users:"<<userCount<<endl;
	cout<<"Total books:"<<bookCount<<endl;
	
}

// ================== MENUS ==================

void adminMenu(string username) {
    int choice;
    do {
        cout << "\n--- ADMIN MENU ---\n";
        cout << "1. Add Book\n";
        cout << "2. View Books\n";
        cout << "3. Update Books\n";
        cout << "4. Sreach Books\n";
        cout << "5. Delete Book\n";
        cout << "6. Add User\n";
        cout << "7. View All User\n";
        cout << "8. Active/Deactivate User\n";
        cout << "9. borrow Book\n";
        cout << "10. return Book\n";
        cout << "11. View Borrowed Books\n";
        cout << "12. Statistics\n";
        cout << "13. Top 5 Books\n";
        cout << "14. Logout\n";
        cout<<"Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1: addBook(); break;
        case 2: viewBooks(); break;
        case 3: UpdataBook(); break;
        case 4: searchBook(); break;
        case 5: deleteBook(); break;
        case 6: addUser(); break;
        case 7: viewAllUsers(); break;
        case 8: toggleUserStatus(); break;
        case 9: borrowBook(username); break;
        case 10: returnBook(username); break;
        case 11: viewBorrowedBooks(); break;
        case 12: statistics(); break;
        case 13: topFiveBooks(); break;
        case 14:cout<<"Logging out ..."<<endl;break;
        default:cout<<"Invalid choice! "<<endl;
        }
    } while (choice != 14);
}

void employeeMenu(string username) {
	
    int choice;
    do {
        cout << "\n--- EMPLOYEE MENU ---\n";
        cout << "1. Add Book\n";
        cout << "2. View Books\n";
        cout << "3. Update Books\n";
        cout << "4. Sreach Books\n";
        cout << "5. borrow Book\n";
        cout << "6. return Book\n";
        cout << "7. Logout\n";
        cout<<"Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1: addBook(); break;
        case 2: viewBooks(); break;
        case 3: UpdataBook(); break;
        case 4: searchBook(); break;
        case 5: borrowBook(username); break;
        case 6: returnBook(username); break;
        case 7:cout<<"Logging out ..."<<endl;break;
        default:cout<<"Invalid choice! "<<endl;
        }
    } while (choice != 7);
}

// ================== MAIN ==================

int main() {
    string role,username;
    cout << "=== LIBRARY MANAGEMENT SYSTEM ===\n";

    if (login(role,username)) {
        if (role == "admin")
            adminMenu(username);
        else
            employeeMenu(username);
    } 
	else {
        cout << "Login Failed\n";
    }

    return 0;
    
}