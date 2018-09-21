//
// Created by Michal Young on 9/15/18.
//

#include "Messages.h"
#include "location.hh"

namespace report {

/* The error count is global */
static int error_count = 0;           // How many errors so far? */
const int  error_limit = 5;           // Should be configurable

void bail()
{
    std::cerr << "Too many errors, bailing" << std::endl;;
}

/* An error that we can locate in the input */
/* Note: An error message should look like this to work well
 * with IDEs and other tools:
 * /path/to/file:32:9: error: expression is not assignable
 */
void error_at(const yy::location& loc, const std::string& msg)
{
    std::cerr << msg << " at " << loc << std::endl;
    if (++error_count > error_limit) {
        bail();
    }
}

/* An error that we can't locate in the input */
void error(const std::string& msg)
{
    std::cerr << msg << std::endl;
    if (++error_count > error_limit) {
        bail();
    }
}

/* Additional diagnostic message, does not count against error limit */
void note(const std::string& msg) {
    std::cerr << msg << std::endl;
}

/* Are we ok? */
bool ok() {
    return (error_count == 0);
}

};