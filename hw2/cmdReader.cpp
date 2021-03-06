/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar getChar(istream&);


//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();

   while (1) {
      ParseChar pch = getChar(istr);
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : if(_readBufPtr != _readBuf) { // when current pointer point to the head of the array, then it stops
                                 
                                 char* tmp = _readBufPtr;
                                 string s = tmp;
                                 tmp--;
                                 _readBufPtr--;
                                 // clear the current position character on the screen
                                 cout << '\b' << s << ' ' << '\b';
                                 
                                 for(int i = 0; i < s.length(); i++) { // delete the char on the screen
                                    cout << '\b';
                                 }

                                 for(int i = 0; i < s.length(); i++) { // delete the char in the array
                                    *tmp = s[i];
                                    tmp++;
                                 }
                                 
                                 _readBufEnd--;
                                 *_readBufEnd = '\0'; // make sure the end of the string is '\0'
                               } else {
                                  mybeep();
                               }
                               break; /* TODO */ 
                               // may have errors recheck ==================================================
                               // not yet to move all chars from right to left
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               resetBufAndPrintPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: if(_readBufPtr != _readBufEnd) { // when current pointer meet end pointer, then it stops
                                 char *tmp = _readBufPtr;
                                 string s = tmp;

                                 cout << s;
                                 _readBufPtr++;
                                 for(int i = 0; i < s.length() - 1; i++) {
                                    cout << '\b';
                                 }

                               } else {
                                  mybeep();
                               }
                               break; /* TODO */
                               // still have to handle how to move all chars to right
         case ARROW_LEFT_KEY : if(_readBufPtr != _readBuf) { // when current pointer reach index 0, then it stops
                                 cout << '\b'; // set cursor back
                                 _readBufPtr--; // set current pointer back
                               } else {
                                  mybeep();
                               }
                               break; /* TODO */
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : if(true) { // to prevent some case weird behavior
                                 string s1 = _readBuf;
                                 string s2 = _readBufPtr;
                                 int mod = (s1.length() - s2.length()) % TAB_POSITION;
                                 int remain = TAB_POSITION - mod;
                                 insertChar(' ', remain);
                               }
                               break; /* TODO */ // may have error //=========================
         case INSERT_KEY     : // insertChar(char(pch));// not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool
CmdParser::moveBufPtr(char* const ptr)
{
   // TODO... // still not know how to check whether the pointer is within head and end
   string check = _readBuf;

   if(_readBuf <= _readBufPtr && _readBufPtr <= _readBuf + check.length()) { // check if in the domain of the array
      if(ptr == _readBuf) { // return to the head of line
         if(_readBufPtr != _readBuf) { // if current pointer is not the same as head pointer
         string s1 = _readBufPtr;
         _readBufPtr = _readBuf;
         string s2 = _readBufPtr;
         for(int i = 0; i < s2.length() - s1.length(); i++) {
            cout << '\b';
         }
         return true;
         }
      } else if(ptr == _readBufEnd) {
         char* tmp = _readBufPtr;
         string s = tmp;

         cout << s;

         _readBufPtr = _readBufEnd;

         return true;
      } else { // inside the array
         deleteLine();
         cout << check;
         _readBufPtr = ptr;
         for(int i = 0; i < check.length() - (_readBufPtr - _readBuf); i++) {
            cout << '\b';
         }
      }

   } else {
      mybeep();
      return false;
   }
   


}


// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool
CmdParser::deleteChar()
{
   // TODO...
   // the current pointer is at the end of the array
   if(_readBufPtr == _readBufEnd) {
      mybeep();
      return false;
   }

   // else
   _readBufPtr++;
   string s = _readBufPtr;
   
   // delete a char on the screen
   cout << s << ' ';

   for(int i = 0; i < s.length() + 1; i++) {
      cout << '\b';
   }

   // delete a char in the array
   _readBufPtr--;
   _readBufEnd--;
   char* tmp = _readBufPtr;
   for(int i = 0; i < s.length(); i++) {
      *tmp = s[i];
      tmp++;
   }


   *_readBufEnd = '\0';


   return true;
}

// 1. Insert character 'ch' for "repeat" times at _readBufPtr
// 2. Move the remaining string right for "repeat" characters
// 3. The cursor should move right for "repeats" positions afterwards
// 4. Default value for "repeat" is 1. You should assert that (repeat >= 1).
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k', 3) ---
//
// cmd> This is kkkthe command
//                 ^
//
void
CmdParser::insertChar(char ch, int repeat)
{
   // TODO...
   assert(repeat >= 1);
   *_readBufEnd = '\0'; // make sure the end of the array is null
   string s = "";
   // *** normal case
   char* tmp = _readBufPtr; // for counting the characters after the current pointer
   s = tmp; // save pointer to a string

   // repeating cout the char based on the repeat time
   for(int i = 0; i < repeat; i++) {
      // print on the screen
      cout << ch;        

      *_readBufPtr = ch; // store ch in the array
      _readBufPtr++; // move current pointer right
      _readBufEnd++; // move end pointer right

   }

   // print the original characters back
   cout << s;

   // recover the characters back and set end pointer back too
   for(int i = 0; i < s.length(); i++) {
      *_readBufPtr = s[i];
      _readBufPtr++;
   }
   _readBufEnd = _readBufPtr; // set the end pointer the the end og the array
   for(int i = 0; i < s.length(); i++) {
      cout << '\b';
      _readBufPtr--; // set the current pointer back
   }

   *_readBufEnd = '\0'; // set the end of string
   
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void
CmdParser::deleteLine()
{
   // TODO...
   string tmp = _readBufPtr;
   cout << tmp;
   _readBufPtr = _readBufEnd = _readBuf; // set current pointer to the head of the array
   string s = _readBuf; // record the length of the array
   // clear all characters
   for(int i = 0; i < s.length(); i++) {
      cout << '\b';
   }
   for(int i = 0; i < s.length(); i++) {
      cout << ' ';
   }
   for(int i = 0; i < s.length(); i++) {
      cout << '\b';
   }

   // reset _readBuf
   _readBuf[0] = '\0';
   

}


// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//
void
CmdParser::moveToHistory(int index)
{
   // TODO...
   int temp_index = index;
   
   _tempCmdStored = false;
   if(_history.size() == 0) { // no element
      mybeep();
      return ;
   } else if(index == _historyIdx - PG_OFFSET && index < 0 && _historyIdx != 0) { // page up key
      index = 0;
   } else if(index < 0) { // up arrow key , have elements but out of range
      mybeep();
      index = 0;
   } else if(index == _historyIdx + PG_OFFSET && index > _history.size() - 1 && _historyIdx == _history.size()) { // page down key
      index = _history.size() - 1;
      mybeep();
      return ;
   } else if(index == _historyIdx + PG_OFFSET && index > _history.size() - 1 && _historyIdx != _history.size() - 1) { // page down key
      index = _history.size() - 1;
   } else if (index > _history.size() - 1) { // down arrow key , have element but out of range
      mybeep();
      index = _history.size() - 1;
      return ;
   } 
   // within the range
   if(_history.size() > 0 && temp_index == _historyIdx - PG_OFFSET && _historyIdx == _history.size()) { // page up
      if(_readBuf == "") {
        _history.push_back("");
        // _tempCmdStored = true;

      } else {
         char* tmp = _readBuf;
         string s1 = tmp;
         // cout << endl << s1 << endl;
         _history.push_back(s1); // only push something back in the array
        _tempCmdStored = true;

      }
   } else if(_history.size() > 0 && index == _history.size() - 1 && index == _historyIdx - 1) { // when pressing up_arrow and index is at the end of the array
      if(_readBuf == "") {
        _history.push_back("");
        // _tempCmdStored = true;
      } else {
         char* tmp = _readBuf;
         string s1 = tmp;
         // cout << endl << s1 << endl;
         _history.push_back(s1); // only push something back in the array
        _tempCmdStored = true;
      }
   } else if(_history.size() > 0 && index == _historyIdx - PG_OFFSET && index == _history.size() - 2 && _history[_history.size() - 1] == "") {
      if(_readBuf == "") {
         _history[_history.size() - 1] = "";
       // _tempCmdStored = true;
      } else {
         char* tmp = _readBuf;
         string s1 = tmp;
         _history[_history.size() - 1] = s1; 
        //_tempCmdStored = true;
      }
   } else if(_history.size() > 0 && index == _historyIdx - 1 && index == _history.size() - 2 && _history[_history.size() - 1] == "") {
      if(_readBuf == "") {
         _history[_history.size() - 1] = "";
        //_tempCmdStored = true;
      } else {
         char* tmp = _readBuf;
         string s1 = tmp;
         _history[_history.size() - 1] = s1; 
        //_tempCmdStored = true;

      }
      
   }

   string s = _history[index];
   
   deleteLine(); // reset the cmd
   for(int i = 0; i < s.length(); i++) {
      _readBuf[i] = s[i];
      _readBufPtr++;
      _readBufEnd++;
   }
   *_readBufEnd = '\0';
      
   // if(_historyIdx > index) _historyIdx--;
   // else _historyIdx++;
   _historyIdx = index;

   cout << s;
   
}


// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether 
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
//
void
CmdParser::addHistory()
{
   // TODO...
   // add to history
   // if(_history.size() > 0) { // if there is some space in the array, clear it
   //    for(int i = _history.size() - 1; i >= 0; i--) {
   //       if(_history[i] == "") {
   //         _history.pop_back();
   //       }
   //    }
   // }
   
   //cout << endl << _readBuf << endl;
   // cout << endl << _tempCmdStored << endl;
   if(_tempCmdStored) _history.pop_back();  

   _tempCmdStored = false;

   if(_history.size() > 0) { // if there is some space in the array, clear it
      for(int i = _history.size() - 1; i >= 0; i--) {
         if(_history[i] == "") {
           _history.pop_back();
         }
      }
   }
   
   char* tmp = _readBuf;

   string s1 = tmp;
   string s2 = "";
   int lead_space = 0, last_space = 0, count = 0;;
   if(s1 == "") { // empty string, break
      // _history.pop_back();
      _historyIdx = _history.size(); // set history index to the lastest object      
      return ;
   }

   for(int i = 0; i < s1.length(); i++) { // count leading space
      if(s1[i] == ' ') {
         count++;
      }
   }

   if(count == s1.length()) { // all-space string, break
      _history.pop_back();
      _historyIdx = _history.size(); // set history index to the lastest object      
      return ;
   } 

   for(int i = 0; i < s1.length(); i++) { // count leading space
      if(s1[i] == ' ') {
         lead_space++;
      } else {
         break;
      }
   }

   for(int i = s1.length() - 1; i >= 0; i--) {
      if(s1[i] == ' ') {
         last_space++;
      } else {
         break;
      }
   }

   for(int i = lead_space; i < s1.length() - last_space; i++) {
      s2 += s1[i];
   }

   // if(_history.size() > 0) {
   //    cout << endl;
   //    for(int i = 0; i < _history.size(); i++) {
   //       cout << _history[i] << endl;;
   //    }
   // }
   

   if(_history.size() == 0) { // the history is empty
      _history.push_back(s2);
      _historyIdx = _history.size(); // set history index to the lastest object
   } else { // the history is not empty
       if(s2 == _history[_history.size() - 1]) { // the same as previous object
         // _history.pop_back();
         _historyIdx = _history.size(); // set history index to the lastest object                
         return ;
       }      
      _history.push_back(s2);
      _historyIdx = _history.size(); // set history index to the lastest object
   }

   if(_history.size() > 0) { // if there is some space in the array, clear it
      for(int i = _history.size() - 1; i >= 0; i--) {
         if(_history[i] == "") {
           _history.pop_back();
         }
      }
   }
   
}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}
