//
// Created by Michal Young on 9/14/18.
//

// This should probably be functions (with a static variable for the count)
// rather than a class, or else it has to be a singleton because we want
// one global count. 
//

#ifndef AST_MESSAGES_H
#define AST_MESSAGES_H

# include "location.hh"
# include <string>

// Error reporting in one place, so that we can count number of errors,
// potentially killing the program if there are too many, and also
// localize decisions like where the error reports go (stdout, stderr, etc)
//


namespace report {

    // Halt execution if there are too many errors
    void bail();

    /* An error that we can locate in the input */
    void error_at(const yy::location& loc, const std::string& msg);

    /* An error that we can't locate in the input */
    void error(const std::string& msg);

    /* Additional diagnostic message, does not count against error limit */
    void note(const std::string& msg);

    /* Is everything ok, or have we encountered errors? */
    bool ok();

};


#endif //AST_MESSAGES_H
