// Utilities for programming

#ifndef COMMON_UTIL_H__
#define COMMON_UTIL_H__

#include <climits>
#include <iostream>
#include <string>
#include <vector>

#include <common/common.h>

// Read a line and split it into string arrays
std::istream& readline(std::istream& is, // the input stream for the command
        Vector<String>& str_array); // the string array to store the result

// Split a string
// Returns: the number of substrings
int split(const String& str, // The source string
        const String& delimeters, // The delimeters
        Vector<String>& str_array,
        int start_index = 0,
        int max_count = INT_MAX); // Output

// Is source a prefix of the target?
bool IsPrefix(const String& source, const String& target);

// Check whether a node in a DAG is an ancestor of another node using BFS
// "Is A an ancestor of B?"
// @param GetParentList The function to retrieve all the parents of a node.
template<typename Node>
bool IsAncestor(Node* A, Node* B, Vector<Node*> (*GetParentList) (Node*));

#endif // COMMON_UTIL_H__

