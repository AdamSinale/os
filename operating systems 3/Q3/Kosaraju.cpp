#include <vector>
#include <iostream>
#include <stack>
#include <string>
#include <sstream>

#define invalid 0
#define valid 1

using namespace std;

vector<string> split(const string& s, char delimiter) {
    vector<string> tokens;
    string token;
    istringstream tokenStream(s);
    while (getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int checkValid(int n, int m){
    if(n <= 0 || m < 0){ // should be atleast 1 vertex and non-negative amount of edges
        return invalid;
    }

    if(m > n*(n-1)){// m should be less than n*(n-1)
        return invalid;
    }

    return valid;
}

void printMat(const vector<vector<int>>& mat){
    for(int i=0; i<mat.size(); i++){
        for(int j=0; j<mat[0].size()-1; j++){
            cout << mat[i][j] << " |";
        }
        cout << mat[i][mat[0].size()-1] << endl;
    }
}

// finding all vertices reachable from v
void fillOrder(const vector<vector<int>>& matrix, int v, vector<bool>& visited, stack<int>& Stack) {
    visited[v] = true;
    for (int i = 0; i < matrix[v].size(); i++) {
        if (matrix[v][i] != 0 && !visited[i]) {
            fillOrder(matrix, i, visited, Stack);
        }
    }
    Stack.push(v);
}

// all edges are reversed
vector<vector<int>> getTranspose(const vector<vector<int>>& matrix, int n) {
    vector<vector<int>> transpose(n, vector<int>(n, 0));
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < n; i++){
            if (matrix[j][i] != 0) {
                transpose[i][j] = 1;
            }
        }
    }
    return transpose;
}

// DFS until all vertices cannot be visited
void DFS(const vector<vector<int>>& matrix, int v, vector<bool>& visited) {
    visited[v] = true;
    cout << v+1 << " ";
    for (int i = 0; i < matrix[v].size(); i++){
        if (matrix[v][i] != 0 && !visited[i]) {
            DFS(matrix, i, visited);
        }
    }
}

void kosaraju(const vector<vector<int>>& matrix, int n) {
    stack<int> Stack;                                              // an empty stack
    vector<bool> visited(n, false);                                // array of visited with falses

    for (int i = 0; i < n; i++) {                                       // for each vertex
        if (!visited[i]) {                                              // if it wasn't visited 
            fillOrder(matrix, i, visited, Stack);                       // DFS to fill the stack with vertices reachable from him
        }
    }
    vector<vector<int>> transpose = getTranspose(matrix, n);  // flip the edges direction (rows are columns respectfully)

    for (int i = 0; i < n; i++) { visited[i] = false; }                 // set each vertex to not visited again

    while (!Stack.empty()) {                                            // while the stack has verices
        int v = Stack.top();                                            // take the last inserted vertex
        Stack.pop();                                                    // remove it from the stack
        if (!visited[v]) {                                              // if it wasnt visited
            DFS(transpose, v, visited);                                 // print strongly connected component
            cout << endl;
        }
    }
}

vector<vector<int>> newGraph(int n, int m){
    vector<vector<int>> mat(n, vector<int>(n, 0));                 // matrix n*m of 0s;
    cout << "Insert " << m << " edges in the form of 'X Y' as X->Y" << endl;
    for(int i = 0; i < m; i++){ // input edges
        int u, v;
        cin >> u >> v;
        u--; v--;
        if(u < 0 || u >= n || v < 0 || v >= n){ // vertex should be in range
            cout << "Invalid Input" << endl;
            i--;
        }
        else if(u == v){ // self loop
            cout << "Invalid Input" << endl;
            i--;
        }
        else if(mat[u][v] == 1){ // multiple edges
            cout << "Invalid Input" << endl;
            i--;
        }
        else { mat[u][v] = 1; };
    }
    return mat;
}

void newEdge(vector<vector<int>>& mat, int i, int j){
    i--;
    j--;
    if(mat[i][j] != 0){
        cout << "There is already an edge between " << i << " and " << j << endl;
        return;
    }
    mat[i][j] = 1;
    cout << "Successfuly added the edge" << endl;
}
                
void removeEdge(vector<vector<int>>& mat, int i, int j){
    i--;
    j--;
    if(mat[i][j] == 0){
        cout << "There isn't an edge between " << i << " and " << j << " already" << endl;
        return;
    }
    mat[i][j] = 0;
    cout << "Successfuly removed the edge" << endl;
}
                
int main(){
    cout << "First, insert number of vertices then edges" << endl;
    int n, m;
    int i, j;
    cin >> n >> m;
    while(checkValid(n, m) == invalid){
        cout << "Invalid Input, try again" << endl;
        cin >> n >> m;
    }
    vector<vector<int>> mat = newGraph(n,m);                 // matrix n*m of 0s;
    printMat(mat);

    string choice;
    while(1){
        cout << "Choose what you wish to do now" << endl;
        cout << "Newgraph n,m - Create a new graph" << endl;
        cout << "Kosaraju - Run Kosaraju algorithm to see the SCCs" << endl;
        cout << "Newedge i,j - Enter a new edge" << endl;
        cout << "Removeedge i,j - Remove an existing edge" << endl;
        cout << "other string - Exit program" << endl;
        cin >> choice;
        if(choice == "Newgraph"){
            cin >> choice;
            vector<string> args = split(choice, ',');
            if(args.size() != 2){ cout << "too many commas" << endl; return 0; }
            n = stoi(args[0]);
            m = stoi(args[1]);
            if(checkValid(n,m) == invalid){ cout << "invalid input" << endl; return 0; }
            mat = newGraph(n,m);
            printMat(mat);
        }
        else if(choice == "Kosaraju"){
            kosaraju(mat, n); // find strongly connected components and print them
        }
        else if(choice == "Newedge"){
            cin >> choice;
            vector<string> args = split(choice, ',');
            if(args.size() != 2){ cout << "too many commas" << endl; return 0; }
            i = stoi(args[0]);
            j = stoi(args[1]);
            if(i < 1 || j < 1 || j > n || i > n){
                cout << "not in bounds of 0 to " << n << endl;
                return 0;
            }
            newEdge(mat, i, j);
            printMat(mat);
        }
        else if(choice == "Removeedge"){
            cin >> choice;
            vector<string> args = split(choice, ',');
            if(args.size() != 2){ cout << "too many commas" << endl; return 0; }
            i = stoi(args[0]);
            j = stoi(args[1]);
            if(i < 1 || j < 1 || j > n || i > n){
                cout << "not in bounds of 0 to " << n << endl;
                return 0;
            }
            removeEdge(mat, i, j);
            printMat(mat);
        }
        else{ cout << "exiting program" << endl; return 0; }
    }


    return 0;
}