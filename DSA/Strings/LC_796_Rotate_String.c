/*# LeetCode 796 - Rotate String

## Problem
Given two strings `s` and `goal`, determine whether `goal` can be obtained by performing any number of cyclic (left or right) rotations on `s`.

Example:

Input:
s = "abcde"
goal = "cdeab"

Output:
true

---

## Approach

1. Check whether both strings have the same length.
2. Concatenate the original string with itself.
3. Search for `goal` as a substring inside the concatenated string.
4. If found, `goal` is a valid rotation; otherwise, it is not.

Example:

s = "abcde"

Concatenated string:

abcdeabcde

Possible rotations appear as contiguous substrings:

- abcde
- bcdea
- cdeab
- deabc
- eabcd

Since `"cdeab"` exists in the concatenated string, it is a valid rotation.

---

## Algorithm

1. Compute the lengths of both strings.
2. If the lengths differ, return `false`.
3. Create a new string by concatenating `s` with itself.
4. Use `strstr()` to check whether `goal` exists within the concatenated string.
5. Return the result.

---

## Time Complexity

- String concatenation: **O(n)**
- Substring search (`strstr`): **O(n × m)** in the general case (implementation-dependent)
- Overall: **O(n²)** in the worst case using a naive substring search.

> Note: Some standard library implementations may use optimized search algorithms, but the worst-case complexity is generally considered O(n²) for interview and competitive programming discussions.

---

## Space Complexity

- Concatenated string: **O(n)**

---

## Concepts Practiced

- String manipulation
- Cyclic rotation
- Concatenation
- Substring search (`strstr`)
- Time and space complexity analysis

---

## Future Improvements

- Implement substring search using the **Knuth-Morris-Pratt (KMP)** algorithm.
- Solve the problem using the **Z Algorithm**.
- Compare different string matching algorithms and their complexities.
*/



bool rotateString(char* s, char* goal) {

    int l1 = strlen(s);
    int l2 = strlen(goal);

    if(l1 != l2){
        return false;
    }

    if(l1 == 0){
        return false;
    }

    char con[2*l1 + 1];
    strcpy(con, s);
    strcat(con, s);

    return(strstr(con,goal) != NULL);
}
