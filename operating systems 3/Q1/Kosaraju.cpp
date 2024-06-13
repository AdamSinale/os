#include <vector>
#include <iostream>
#include <stack>

#define invalid 0
#define valid 1

using namespace std;

int checkValid(int n, int m){
    if(n <= 0 || m < 0){ // should be atleast 1 vertex and non-negative amount of edges
        return invalid;
    }

    if(m > n*(n-1)){// m should be less than n*(n-1)
        return invalid;
    }

    return valid;
}

// finding all vertices reachable from v
void fillOrder(const std::vector<std::vector<int>>& matrix, int v, std::vector<bool>& visited, std::stack<int>& Stack) {
    visited[v] = true;
    for (int i = 0; i < matrix[v].size(); i++) {
        if (matrix[v][i] != 0 && !visited[i]) {
            fillOrder(matrix, i, visited, Stack);
        }
    }
    Stack.push(v);
}

// all edges are reversed
std::vector<std::vector<int>> getTranspose(const std::vector<std::vector<int>>& matrix, int n) {
    std::vector<std::vector<int>> transpose(n, vector<int>(n, 0));
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
void DFS(const std::vector<std::vector<int>>& matrix, int v, std::vector<bool>& visited) {
    visited[v] = true;
    std::cout << v << " ";
    for (int i = 0; i < matrix[v].size(); i++){
        if (matrix[v][i] != 0 && !visited[i]) {
            DFS(matrix, i, visited);
        }
    }
}

void findSCCs(const std::vector<std::vector<int>>& matrix, int n) {
    std::stack<int> Stack;                                              // an empty stack
    std::vector<bool> visited(n, false);                                // array of visited with falses

    for (int i = 0; i < n; i++) {                                       // for each vertex
        if (!visited[i]) {                                              // if it wasn't visited 
            fillOrder(matrix, i, visited, Stack);                       // DFS to fill the stack with vertices reachable from him
        }
    }
    std::vector<std::vector<int>> transpose = getTranspose(matrix, n);  // flip the edges direction (rows are columns respectfully)

    for (int i = 0; i < n; i++) { visited[i] = false; }                 // set each vertex to not visited again

    while (!Stack.empty()) {                                            // while the stack has verices
        int v = Stack.top();                                            // take the last inserted vertex
        Stack.pop();                                                    // remove it from the stack
        if (!visited[v]) {                                              // if it wasnt visited
            DFS(transpose, v, visited);                                 // print strongly connected component
            std::cout << std::endl;
        }
    }
}

int main(){
    int n, m;
    cin >> n >> m;

    int ans = checkValid(n, m);
    if(ans == invalid){
        cout << "Invalid Input" << endl;
        return 0;
    }

    vector<vector<int>> mat(n, vector<int>(n, 0));                 // matrix n*m of 0s;

    for(int i = 0; i < m; i++){ // input edges
        int u, v;
        cin >> u >> v;
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

    findSCCs(mat, n); // find strongly connected components and print them


    return 0;
}