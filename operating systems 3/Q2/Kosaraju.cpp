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
    std::stack<int> Stack;

    std::vector<bool> visited(n, false);

    for (int i = 0; i < n; i++) {
        if (!visited[i]) {
            fillOrder(matrix, i, visited, Stack); // DFS to fill the stack
        }
    }

    std::vector<std::vector<int>> transpose = getTranspose(matrix, n); 

    for (int i = 0; i < n; i++) {
        visited[i] = false;
    }

    while (!Stack.empty()) {
        int v = Stack.top();
        Stack.pop();

        if (!visited[v]) {
            DFS(transpose, v, visited); // print strongly connected component
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

    vector<vector<int>> mat(n, vector<int>(n, 0));

    for(int i = 0; i < m; i++){ // input edges
        int u, v;
        cin >> u >> v;
        if(u < 0 || u >= n || v < 0 || v >= n){ // vertex should be in range
            cout << "Invalid Input" << endl;
            return 0;
        }
        if(u == v){ // self loop
            cout << "Invalid Input" << endl;
            return 0;
        }
        if(mat[u][v] == 1){ // multiple edges
            cout << "Invalid Input" << endl;
            return 0;
        }
        mat[u][v] = 1;
    }

    findSCCs(mat, n); // find strongly connected components and print them


    return 0;
}