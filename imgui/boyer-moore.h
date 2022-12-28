#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
typedef long long int LLI;
#define ASCII_RANGE 128
#define BUFFER_SIZE 512 * 1024

int caseSensitive;  // the case sensitive option was added in the end, quite hurriedly and in a rough fashion :P
bool print;

class boyerMoore {
   private:
    std::vector<string> inputFromFile;
    string pat;
    LLI patLen;
    LLI d1[ASCII_RANGE];
    LLI *d2;

    LLI max(LLI a, LLI b)  // function to find max of two numbers
    {
        if (a >= b)
            return a;
        else
            return b;
    }

    void genD1()  // function to generate the delta1 table
    {
        for (int i = 0; i < ASCII_RANGE; i++) {
            d1[i] = patLen;
        }

        char pat1;
        for (LLI i = 0; i < patLen - 1; i++) {
            if (caseSensitive == 2) {
                pat1 = (char)tolower(pat[i]);
            } else {
                pat1 = pat[i];
            }

            d1[pat1] = patLen - 1 - i;
        }
    }

    bool checkPrefix(LLI pos) {
        char pat1, pat2;
        LLI suffixLen = patLen - pos;
        for (LLI i = 0; i < suffixLen; i++) {
            if (caseSensitive == 2) {
                pat1 = (char)tolower(pat[pos + i]);
                pat2 = (char)tolower(pat[i]);
            } else {
                pat1 = pat[pos + i];
                pat2 = pat[i];
            }

            if (pat1 != pat2) {
                return false;
            }
        }
        return true;
    }

    LLI findSuffixLen(LLI pos)  // function to find length of the suffix matching the substring starting at pos
    {
        char pat1, pat2;
        LLI i = 0;
        if (caseSensitive == 2) {
            pat1 = (char)tolower(pat[patLen - 1 - i]);
            pat2 = (char)tolower(pat[pos - i]);
        } else {
            pat1 = pat[patLen - 1 - i];
            pat2 = pat[pos - i];
        }

        while (pat1 == pat2) {
            if (i < pos)
                i++;
            else
                break;
        }
        return i;
    }

    void genD2()  // function to generate the delata2 table
    {
        char pat1, pat2;
        LLI endPrefixPos = patLen - 1;
        LLI p = patLen - 1;
        while (p >= 0) {
            if (checkPrefix(p + 1)) {
                endPrefixPos = p + 1;
            }
            d2[p] = endPrefixPos + (patLen - 1 - p);
            p--;
        }

        for (LLI p = 0; p < patLen - 1; p++) {
            LLI suffixLen = findSuffixLen(p);
            if (caseSensitive == 2) {
                pat1 = (char)tolower(pat[patLen - 1 - suffixLen]);
                pat2 = (char)tolower(pat[p - suffixLen]);
            } else {
                pat1 = pat[patLen - 1 - suffixLen];
                pat2 = pat[p - suffixLen];
            }

            if (pat1 != pat2) {
                d2[patLen - 1 - suffixLen] = patLen - 1 + suffixLen - p;
            }
        }
    }

    void printNeighbours(int fileNo, LLI pos, LLI count)  // for printing the location of matched pattern
    {
        LLI j;
        if (pos >= 6) {
            j = pos - 6;
            cout << "#" << count << ".\t........";
        } else
            j = 0;

        while (j <= inputFromFile[fileNo].size() && j <= pos + 15) {
            if (j >= pos && j < pos + patLen)
                cout << "\033[31m" << inputFromFile[fileNo][j++] << "\033[0m";  // coulouring the matched pattern
            else
                cout << inputFromFile[fileNo][j++];
        }
        if (j != inputFromFile[fileNo].size() + 1)
            cout << "........" << endl;
    }

   public:
    boyerMoore() {
        patLen = 0;
        d2 = NULL;
        caseSensitive = 2;
    }

    void fileIn(string filename) {
        string input;

        ifstream fin;
        fin.open(filename.c_str());
        char *buffer;
        while (fin) {
            buffer = new char[BUFFER_SIZE];
            fin.read(buffer, BUFFER_SIZE);  // reading the fileas chuncks of size=BUFFER_SIZE
            input += buffer;
            delete (buffer);
        }
        inputFromFile.push_back(input);
        fin.close();
        input.clear();
    }

    void patIn(char *s) {
        pat = s;
        patLen = pat.size();
        d2 = new LLI[patLen];
        genD1();
        genD2();
        // printarr(d2,patLen);
        // printarr(d1,ASCII_RANGE);
    }

    void clean() {
        inputFromFile.clear();
    }

    void search(vector<LLI> *ans)  // the "main" function
    {
        LLI i;

        for (int fileNo = 0; fileNo < inputFromFile.size(); fileNo++) {
            char charFromFile, charFromPat;
            // cout << "FILE #" << fileNo + 1 << endl;
            i = patLen - 1;
            LLI count = 0;
            LLI fileSize = inputFromFile[fileNo].size();
            while (i < fileSize) {
                LLI j = patLen - 1;
                while (j >= 0) {
                    if (caseSensitive == 2) {
                        charFromFile = (char)tolower(inputFromFile[fileNo][i]);
                        charFromPat = (char)tolower(pat[j]);
                    } else {
                        charFromFile = inputFromFile[fileNo][i];
                        charFromPat = pat[j];
                    }
                    if (charFromFile == charFromPat) {
                        i--;
                        j--;
                    } else
                        break;
                }
                if (j < 0) {
                    count++;
                    if (print)
                        printNeighbours(fileNo, i + 1, count);
                    i += patLen + 1;
                    continue;
                }
                i += max(d1[charFromFile], d2[j]);
            }
            ans->push_back(count);
            /*if (count == 0) {
                cout << "Pattern Not Found!!" << endl;
            } else {
                cout << "No. Of Occurences : " << count << endl;
            }

            if (fileNo < inputFromFile.size() - 1) {
                cout << "PRESS ANY KEY TO SEE OCCURRENCES IN NEXT FILE...";
                getchar();
            }
            */
        }
        delete (d2);
    }
};
