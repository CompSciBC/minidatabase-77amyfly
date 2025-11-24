#ifndef ENGINE_H
#define ENGINE_H

#include <iostream>
#include <vector>
#include "BST.h"
#include "Record.h"
#include <algorithm>
// add header files as needed

using namespace std;

// Converts a string to lowercase (used for case-insensitive searches)
static inline string toLower(string s)
{
    for (char &c : s)
        c = (char)tolower((unsigned char)c);
    return s;
}

// ================== Index Engine ==================
// Acts like a small "database engine" that manages records and two BST indexes:
// 1) idIndex: maps student_id → record index (unique key)
// 2) lastIndex: maps lowercase(last_name) → list of record indices (non-unique key)
struct Engine
{
    private:
    vector<Record> heap;                // the main data store (simulates a heap file)
    BST<int, int> idIndex;              // index by student ID
    BST<string, vector<int>> lastIndex; // index by last name (can have duplicates)
    int insertRec(const Record &recIn);
    bool deleteId(int id);
    const Record *findId(int id, int &cmpOut);
    void addToLastIndex(const string& lastname, int position);
    void deleteFromlastIndex(const string& lastname, int position);
    vector<const Record *> rangeId(int lo, int hi, int &cmpOut);
    vector<const Record *> prefixLast(const string &prefix, int &cmpOut);

    public:

    // Inserts a new record and updates both indexes.
    // Returns the record ID (RID) in the heap.
    int insertRecord(const Record &recIn)
    {
        return insertRec(recIn);
    }

    // Deletes a record logically (marks as deleted and updates indexes)
    // Returns true if deletion succeeded.
    bool deleteById(int id)
    {
       return deleteId(id);
    }

    // Finds a record by student ID.
    // Returns a pointer to the record, or nullptr if not found.
    // Outputs the number of comparisons made in the search.
    const Record *findById(int id, int &cmpOut)
    {
        const Record * rec = findId(id, cmpOut);
        cout<< "comparisons : " << cmpOut << endl;
        return rec;    
    }

        // Returns all records with ID in the range [lo, hi].
        // Also reports the number of key comparisons performed.
    vector<const Record *> rangeById(int lo, int hi, int &cmpOut)
        {
            vector<const Record *> range = rangeId(lo, hi, cmpOut);
            cout<< "comparisons : " << cmpOut << endl;
            return range;
        }

        // Returns all records whose last name begins with a given prefix.
        // Case-insensitive using lowercase comparison.
    vector<const Record *> prefixByLast(const string &prefix, int &cmpOut)
        {
            return prefixLast(prefix, cmpOut);
        }
    
};

// private functions

    int Engine::insertRec(const Record &recIn)
    {
        int id = recIn.id;
        if (idIndex.find(id))
        {
            return id;
        }
        heap.push_back(recIn);
        int pos = heap.size() - 1;
        idIndex.insert(id, pos);
        addToLastIndex(recIn.last, pos);
        
        return id;
    }

    bool Engine::deleteId(int id){
        auto pos = idIndex.find(id);
        if (!pos){
            return false;
        }
        deleteFromlastIndex(heap[*pos].last, *pos);
        heap[*pos].deleted = true;
        idIndex.erase(id);
        return true;
    }
    

    void Engine::addToLastIndex(const string &lastname, int position){
        string last = toLower(lastname);
        auto indexList = lastIndex.find(last);
        if (indexList)
        {
            indexList->push_back(position);
        }
        else
        {
            lastIndex.insert(last, vector<int>{position});
        }
    }

    void Engine::deleteFromlastIndex(const string& lastname, int position){
        string last = toLower(lastname);
        auto indexList = lastIndex.find(last);
        if (!indexList){
            return;
        }
        if(indexList->size() >1){
            indexList->erase(remove(indexList->begin(), indexList->end(), position), indexList->end());
        }else{
            lastIndex.erase(last);
        }
    }

    vector<const Record *> Engine::rangeId(int lo, int hi, int &cmpOut){
        idIndex.resetMetrics();
        cmpOut = 0;
        vector<const Record *> Records;
        idIndex.rangeApply(lo, hi,[&](int key , int value){
            const Record * recPtr = &heap[value];
            if (!heap[value].deleted){
                Records.push_back(recPtr);
            }
        });
        cmpOut = idIndex.comparisons;
        return Records;
    }

    vector<const Record *> Engine::prefixLast(const string &prefix, int &cmpOut){
        string pre = toLower(prefix);
        lastIndex.resetMetrics();
        cmpOut = 0;
        vector<const Record *> Records;
        string lo = pre;
        string hi = pre + '{';
        lastIndex.rangeApply(lo, hi, [&](const string &key , const vector<int>& value){
            for(int val : value){
            const Record * recPtr = &heap[val];
            if (!heap[val].deleted){
                Records.push_back(recPtr);
            }
            }
        });
        cmpOut = lastIndex.comparisons;
        return Records;
    }

    const Record *Engine::findId(int id, int &cmpOut){
        idIndex.resetMetrics();
        cmpOut = 0;
        auto pos = idIndex.find(id);
        cmpOut = idIndex.comparisons;
        if(!pos){
            return nullptr;
        }
        const Record* record = &heap[*pos];
        return record;
    }


#endif
