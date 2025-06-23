/*=========================================================================

                                vtkbone

  VTK classes for building and analyzing Numerics88 finite element models.

  Copyright (c) 2010-2025, Numerics88 Solutions.
  All rights reserved.

=========================================================================*/

#ifndef __CommandStyleFileReader_h
#define __CommandStyleFileReader_h

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <stack>

#define FSDF_OK 0
#define FSDF_ERROR 1

/** @class CommandStyleFileReader

  An abstract base class for reading data files that are structured as text
  with a fixed maximum line length, and consist of a series of commands.

  Error reporting functionality is also provided.  The class works on a
  generic istream object, which can be either a file stream or a string
  stream.

  You must provide the actual functionality to process any commands that
  you desire in a concrete derived class.

  This class uses many virtual methods that can be reimplemented in your
  derived class in order to customize functionality.

  BASIC INSTRUCTIONS on implementing a derived class to process a specific
  file type:

    1. Implement IsCommandBegin, IsCommandEnd and IsCommentLine in your
       derived class.  These methods identify command beginnings and endings,
       as well as comment lines.  There are no default implementations of
       these methods, because file syntax is so variable.

    2. Implement methods in your derived class to handle particular commands.
       Once implented, register them with the base class.
       This is done with a call like following, which you can put in
       your constructor:

         RegisterCommandHandler ("SOME SECTION NAME",
             reinterpret_cast<CommandHandler_t>(&MyClass::MyHandlerMethod));

    3. ("Somewhat" optional).  Implement CallCommandHandler in your derived
       class.  It only needs to cast its argument back to a member function
       pointer for your derived class, and call it.  Like this:

         int MyClass::CallCommandHandler (CommandHandler_t handler)
         {
           typedef int (MyClass::*DerivedHandler_t)();
           DerivedHandler_t f = reinterpret_cast<DerivedHandler_t>(handler);
           return (this->*f)();
         }

       See the discussion below on why you can (usually) skip this step.

  ADDITIONAL TIPS:

    - You should only access the input stream through the method GetLine.
      This will ensure that line counting, error handling, etc... are
      maintained correctly.  You can access the current line through the
      member variable "line".

    - You can access the command stack through the member variable
      "commandNameStack".  You can inspect the complete list of nesting
      commands.

    - If you choose to handle (process) any command, then you are responsible
      for identifying the end of the command (use IsCommandEnd), and for
      popping off the handled command name from commandNameStack before
      returning.

    - Your methods should return FSDF_OK on success and FSDF_ERROR on failure.

    - frSetErrorMsgMacro can be used to report errors.  Note that this
      macro passes your argument to a C++ stream objects, so arguments such
      as "text" << variable are allowed.  After reporting an error, return FSDF_ERROR
      for failure.  This will cause the class to unwind. The caller can use
      GetErrorStatus and GetErrorMsg.

    - frSetDebugMsgMacro can be used to report debug messages.  The
      default behaviour is to print the message to stderr immediately
      if built with the compiler define DEFINE (and do nothing if not).
      If you prefer different behaviour, reimplement the virtual method
      "DebugMessage".

  NOTE and WARNING about CallCommandHandler:

    The Command Handlers table is implemented with member function pointers.
    This is a little-used (although occasionally useful) corner of C++,
    with some unfortunate quirks.  One being, that according to the standard,
    member function pointers are specific to a particular class, and cannot
    even be converted between base and derived classes (this is controversial,
    since they would be more useful if they could be).  Hence the necessity of
    reinterpret_cast<CommandHandler_t> in passing the method as an argument
    to RegisterCommandHandler.

    Strictly speaking, the member function pointer must be cast back to
    be a member function pointer of the derived class before being called.
    In practice however, providing your derived class uses only single
    inheritance (rather than multiple inheritance), it works to simply
    call the member function pointer cast to the base class (which is how
    it is stored in the table).  This is convenient, because it can then
    be called directly from the base class.  This works because if B inherits
    from A, then in practice all compilers store A at the beginning of B.
    This allows the same "this" pointer to be used for both.  Obviously this
    no longer works in the case of multiple inheritance (B inherits from A
    and some other class C - they can't both be at the beginning of A).

    Therefore, for lazy people (me included), I have provided a default
    CallCommandHandler in the base class that calls the command handler
    without the nominally required recast back to the derived class.  If
    you are pedantic or want to safely use of multiple inheritance in your
    derived class, then you should add CallCommandHandler to your derived
    class using the template above.

    One more annoyance is that Microsoft compilers, depending on compiler
    flags, might use different sizes for base and derived class member
    function pointers. (Yes! Really!)  This breaks the C++ standard and it
    also breaks this code.  The relevant compiler flag is /vmg, and it
    should be set - always.

  For an example of a file reader implemented using this class, see
  AbaqusReaderHelper.
*/
class CommandStyleFileReader
{
public:

  // Defines a pointer to a member function that takes no arguments
  // and returns an int.
  typedef int (CommandStyleFileReader::*CommandHandler_t)();

  // A set of commmand handler methods mapped to command key words.
  typedef std::map<std::string,CommandHandler_t> CommandContext_t;

  // Constructor
  // stream - an already-opened input stream (file or string)
  CommandStyleFileReader (std::istream& stream);

  // Method that the user calls to read and process input stream.
  //
  // Returns FSDF_OK on success and FSDF_ERROR on failure.
  virtual int Read();

  virtual void DebugOn() {debug = 1;}
  virtual void DebugOff() {debug = 0;}
  virtual void SetDebug(int d) {debug = d;}

  int GetErrorStatus() {return errorStatus;}
  const char * GetErrorMsg() {return errorMsg.c_str();}

protected:

  // Register a method of your derived class to handle a particular command.
  //
  // commandName - identifying name of the command that you want to handle.
  // handler - member function pointer to your method.
  //           NOTE: You must use reinterpret_cast<CommandHandler_t> on this
  //                 argument when calling this function.  Refer to the above
  //                 discussion.
  int RegisterCommandHandler (const char* commandName,
                              CommandHandler_t handler);

  // Refer to the above discussion for the purpose, implementation, and
  // calling method of this function.
  virtual int CallCommandHandler (CommandHandler_t handler);

  // Called after all file lines have been read and processed.
  virtual int Finish() { return FSDF_OK; }

  // Reads the next line from the file and stores it in "line".
  // Checks for error conditions and increments lineCount.
  //
  // Returns 1 if a line is successfully obtained, and 0 otherwise.
  virtual int GetLine();

  // Returns 1 if the current line is a command.
  //
  // If a command is found, commandName should be set to the command name.
  virtual int IsCommand () = 0;

  // Returns 1 if the line is a comment line.
  // No default implementation, since comment line syntax varies considerably.
  // You don't need to implement this if you don't use it, since it is not
  // currently used by CommandStyleFileReader.
  virtual int IsCommentLine() = 0;

  // Search the input stream for a command.
  // If a command is found, commandName is set and ProcessCommand is called.
  //
  // Returns FSDF_OK on success and FSDF_ERROR on failure.
  virtual int FindCommand();

  // Check if the current command has been registered for any command
  // handlers, and if so, call that handler.
  // If no match is found, call FindCommandBoundaries (recursively) to
  // search for sub-commands.
  //
  // Returns FSDF_OK on success and FSDF_ERROR on failure.
  virtual int ProcessCommand();

  // Sets errorStatus to FSDF_ERROR and stores msg in erroMsg.
  // Will not overwrite a pre-existing error message.
  virtual void SetError (const std::string& msg);

  // Does nothing if debug is 0, otherwise prints msg to std::cerr.
  // Implement in derived class to provide your own debug message functionality.
  virtual void DebugMessage (const std::string& msg);

  // Prints msg to std::cerr.
  // Implement in derived class to provide your own debug message functionality.
  virtual void WarningMessage (const std::string& msg);

  std::istream&              stream;
  std::string                commandName;
  long                       lineCount;
  int                        repeatLastCommand;

  std::string                line;

  std::stack<CommandContext_t> commandContextStack;

  int                        debug;

  int                        errorStatus;
  std::string                errorMsg;

private:
  // Prevent compiler from making public versions of these.
  CommandStyleFileReader (const CommandStyleFileReader&);
  void operator= (const CommandStyleFileReader&);

};

//----------------------------------------------------------------------------
// This macro is used to record errors
// frSetErrorMsgMacro("Error message" << variable);
//
#define frSetErrorMsgMacro(x)                                \
   {                                                            \
   std::ostringstream msg;                                      \
   msg << "ERROR: In " << __FILE__ << ", line " << __LINE__     \
          << "\n" << this << ": " << x << "\n";                 \
   SetError(msg.str());                                         \
   }

//----------------------------------------------------------------------------
// This macro is used to print out debugging info
// frSetDebugMsgMacro("Debug message" << variable);
//
#define frSetDebugMsgMacro(x)                                \
   {                                                            \
   if (debug)                                                   \
   {                                                          \
     std::ostringstream msg;                                    \
     msg << "Debug: In " << __FILE__ << ", line " << __LINE__   \
         << "\n" << this << ": " << x << "\n";                  \
     DebugMessage (msg.str());                                  \
   }                                                          \
   }

//----------------------------------------------------------------------------
// This macro is used to print out debugging info
// frSetDebugMsgMacro("Debug message" << variable);
//
#define frSetWarningMsgMacro(x)                              \
   {                                                            \
   std::ostringstream msg;                                      \
   msg << "WARNING: In " << __FILE__ << ", line " << __LINE__   \
         << "\n" << this << ": " << x << "\n";                  \
   WarningMessage (msg.str());                                  \
   }

#endif  // __CommandStyleFileReader_h
