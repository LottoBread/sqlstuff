#include <iostream>
#include <tuple>
#include <stdlib.h>
#include </usr/include/mariadb/mysql.h>

using std::string;
using std::cout;
using std::cin;

string username;
string password;
bool login = false;

struct SQLConnection
{
    string server, user, password, database;

    SQLConnection(string server, string user, string password, string database)
    {
        this->server = server;
        this->user = user;
        this->password = password;
        this->database = database;
    }
};

/**
 * @brief
 * Will try to connect to SQL, return a tuple, first value will be true if connection was a success
 *
 * @param mysql_details Details to connect ot SQL db
 * @return std::tuple<bool, MYSQL *>
 */
std::tuple<bool, MYSQL *> sqlConnectionSetup(struct SQLConnection mysql_details)
{
    // there are multiple ways to return multiple values, here we use a tuple
    MYSQL *connection = mysql_init(NULL); // mysql instance
    bool success = true;

    // connect database
    // c_str -> converts string to char
    if (!mysql_real_connect(connection, mysql_details.server.c_str(), mysql_details.user.c_str(), mysql_details.password.c_str(), mysql_details.database.c_str(), 0, NULL, 0))
    {
        success = false;
        cout << "Connection Error: " << mysql_error(connection) << std::endl;
    }

    return std::make_tuple(success, connection);
}

/**
 * @brief
 * Execute a SQL query, will return a tuple, first value will be true if success
 *
 * @param connection SQL connection
 * @param query SQL query
 * @return result struct with success and res (result)
 */
auto execSQLQuery(MYSQL *connection, string query)
{
    // instead of returning a tuple, you could return a struct instead
    struct result
    {
        bool success;
        MYSQL_RES *res;
    };
    bool success = true;

    // send query to db
    if (mysql_query(connection, query.c_str()))
    {
        cout << "MySQL Query Error: " << mysql_error(connection) << std::endl;
        success = false;
    }

    return result{success, mysql_store_result(connection)};
}


void dbUpdate(MYSQL* con, string column, string username, string input){
    auto result = execSQLQuery(con, "UPDATE usr\n SET " + column +  "= \"" + input + "\"\n WHERE username = \"" + username + "\";" );
    mysql_free_result(result.res);
}

auto dbPull(MYSQL* con, string column, string username){ //when getting a null result from DB it causes segmentation fault
    auto result = execSQLQuery(con, "SELECT " + column + " FROM usr WHERE username = \"" + username + "\";");
    MYSQL_ROW row;
    row = mysql_fetch_row(result.res);
    mysql_free_result(result.res);
    return row[0];        
}

void dbRegister(MYSQL* con, string username, string password){
    execSQLQuery(con, "INSERT INTO usr (username, password, balance) VALUES (\"" + username + "\", \"" + password + "\", 10000);");
}


int main(int argc, char const *argv[])
{

    bool success;
    string newstring;
    bool menu = true;
    
    MYSQL *con; // the connection
    // MYSQL_RES *res; // the results
    MYSQL_ROW row; // the results rows (array)

    struct SQLConnection sqlDetails("76.21.16.133", "lotto", "Astral89$", "exampledb");

    // connect to the mysql database
    std::tie(success, con) = sqlConnectionSetup(sqlDetails);

    if (!success)
    {
        // you can handle any errors here
        return 1;
    }
    while(menu == true){
        int choice;
        cout << "1) Register a new account.\n2) Log in to an existing account.\n3) Exit the program.";
        cin >> choice;
            switch(choice){
                case 1:{

                    cout << "Register a new account\n";
                    cout << "Enter a username\n";
                    cin >> username;
                        if(dbPull(con, "username", username) == username){ //username already exists works, when user check returns null, program throws segmentation fault.
                            cout << "User already exists.\n";
                    }
                        else{
                            cout << "Enter a password\n";
                            cin >> password;
                            dbRegister(con, username, password);
                            cout << "Account registered, given default balance of 10,000\n";
                            menu = false;
                    }
                    break;
                }
                    
                case 2:{
                    cout << "Type a username:\n";
                    cin >> username;
                    cout << "Enter password\n";
                    cout << dbPull(con, "username", username);
                    cin >> newstring;
                    auto fetched = dbPull(con, "password", username);
                    if(fetched == newstring)
                    {
                        cout << "Signed in\n";
                        login = true;
                        menu = false;
                    }
                    else if (fetched != newstring)
                    {
                        cout << "Password does not match\n";
                        login = false;
                        menu = false;
                    }
                    break;
                }
                case 3:{
                    menu = false;
                    break;
                }
            
            }
            if(login == true){
                cout << "Your Balance is: " << dbPull(con, "balance", username);
            }
        }

    mysql_close(con);

    return 0;
}





    //if (!result.success)
    //{
    //     handle any errors
    //    return 1;
    //}

    //cout << ("Database Output:\n") << std::endl;


     //clean up the database result
    //mysql_free_result(result.res);

     //close database connection